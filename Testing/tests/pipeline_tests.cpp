#include <gtest/gtest.h>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include "pipeline.h"
#include "cache.h"
#include "memory.h"
#include "registers.h"
#include "driver.h"
#include "parseInput.h"


extern Registers::IntegerRegs intRegs;
extern Registers::PendIntegerRegs pendRegs;
extern Registers::VectorRegs vectorRegs;
extern Registers::PendVectorRegs pendVectorRegs;



class PipelineTest : public ::testing::Test {
protected:
    Memory* mem;
    Cache* cache;
    Pipeline* pipeline;

    void SetUp() override {
        mem = new Memory();
        cache = new Cache(mem);
        pipeline = new Pipeline(cache);

        // Reset registers
        for (int i = 0; i < 16; i++) {
            intRegs.r[i] = 0;
            pendRegs.r[i] = 0;
        }
    }

    void TearDown() override {
        delete pipeline;
        delete cache;
        delete mem;
    }

    // FIXME
    void loadAssembly(const std::string& assembly) {
        // Write assembly to temp file
        std::ofstream tempFile("test_temp.s");
        tempFile << assembly;
        tempFile.close();

        // Run python assembler
        int ret = system("python3  assembler/assembler.py test_temp.s test_output.txt");
        if (ret != 0) {
            FAIL() << "Assembler failed";
            return;
        }

        // Parse output and load into cache/memory
        std::ifstream outFile("test_output.txt");
        std::string line;
        while (std::getline(outFile, line)) {
            if (line.empty()) continue;
            std::istringstream iss(line);
            std::vector<std::string> tokens;
            std::string token;
            while (iss >> token) tokens.push_back(token);
            if (!tokens.empty()) parseInput(tokens, cache);
        }
        outFile.close();
    }

    void preloadToCache(int address, int value) {
        int line, index, offset, tag;
        cache->decodeAddress(address, line, index, offset, tag);
        cache->cache_memory[index][0] = tag;   // tag
        cache->cache_memory[index][2] = 0;     // dirty bit
        cache->cache_memory[index][3] = 1;     // valid bit
        cache->cache_memory[index][4 + offset] = value;  // data
    }

    // Expose private ALU helper via friend
    int ALU(int opcode, int a, int b) {
        return pipeline->ALU_helper(opcode, a, b);
    }
};

// ===== MEMORY ACCESS TESTS =====

// Fetch was fetching memory when marked with squashed
// should initiate new fetch cycle instead of building of previous count down
TEST_F(PipelineTest, F_MemAccess_Reinitiated_After_Squash) {
    loadAssembly(R"(
        ADDI R0, R1, 5
        ADDI R1, R1, 1
    )");

    intRegs.r[13] = 0;

    // Cycle 1: Fetch will be blocked
    single_clock_cycle(pipeline, false, true);
    EXPECT_GT(pipeline->newCache->dramDelay, 0); // Start of count down
    int delayAfterCycle1 = pipeline->newCache->dramDelay;

    // Squash fInstr, simulating a branch squash
    pipeline->fInstr.is_squashed = true;
    pipeline->squashed = true;

    // Cycle 2: fetch runs again but for a new PC after squash
    // countdown should reset, not continue from previous
    intRegs.r[13] = 1;  // new PC after squash
    single_clock_cycle(pipeline, false, true);

    EXPECT_EQ(pipeline->newCache->dramDelay, 3);  // reset not decremented
    EXPECT_EQ(pipeline->newCache->currentlyServicing, 0);  // servicing new fetch
}

// If Fetch and Mem Access both try to access memory at the same time
// Mem finishes by cycle


// ===== ALU TESTS =====


// Testing for corret truncation of binary if overflow
// TEST_F(PipelineTest, ALU_MUL) {

// }

// TEST_F(PipelineTest, ALU_DIV) {
//     // divide by zero
// }

// TEST_F(PipelineTest, ALU_MOD) {

// }

// ===== PENDING REGISTER TESTS =====
TEST_F(PipelineTest, PendReg_BlocksOnHazard) {
    // Simulate a pending register on r1
    pendRegs.r[1] = 1;

    // Set up a decode instruction that reads r1
    // ADDI r0, r1, 5 -> opcode 15, type 0
    // Encode: type(00) opcode(01111) dest(0000) src1(0001) imm(5)
    unsigned int bin = (0b00 << 30) | (15 << 25) | (0 << 21) | (1 << 17) | 5;
    pipeline->dInstr.bin_instr = static_cast<int>(bin);
    pipeline->dInstr.pend_incremented = false;

    bool stalled = pipeline->decode();
    EXPECT_TRUE(stalled); // should stall due to hazard
    EXPECT_EQ(pendRegs.r[0], 0); // dest pending should NOT have been incremented
}

TEST_F(PipelineTest, PendReg_PassesWhenClear) {
    // No pending registers
    pendRegs.r[1] = 0;

    // ADDI r0, r1, 5
    unsigned int bin = (0b00 << 30) | (15 << 25) | (0 << 21) | (1 << 17) | 5;
    pipeline->dInstr.bin_instr = static_cast<int>(bin);
    pipeline->dInstr.pend_incremented = false;

    bool stalled = pipeline->decode();
    EXPECT_FALSE(stalled);
    EXPECT_EQ(pendRegs.r[0], 1); // dest pending should be incremented
}

TEST_F(PipelineTest, PendReg_SelfBlock) {
    // r3 pending due to previous instruction, ADDI r3 r3 1 should block
    pendRegs.r[3] = 1;

    // ADDI r3, r3, 1 -> opcode 15, dest=3, src1=3, imm=1
    unsigned int bin = (0b00 << 30) | (15 << 25) | (3 << 21) | (3 << 17) | 1;
    pipeline->dInstr.bin_instr = static_cast<int>(bin);
    pipeline->dInstr.pend_incremented = false;

    bool stalled = pipeline->decode();
    EXPECT_TRUE(stalled);
}

// ===== SQUASH TESTS =====
TEST_F(PipelineTest, SquashedInstr_DoesNotWriteBack) {
    intRegs.r[0] = 0;

    // Set up a squashed ADDI r0, r0, 5 in writeback
    pipeline->wInstr.type_code = 0;
    pipeline->wInstr.opcode = 15;
    pipeline->wInstr.destv[0] = 0;
    pipeline->wInstr.result[0] = 5;
    pipeline->wInstr.is_squashed = true;
    pipeline->wInstr.bin_instr = 1;

    pipeline->write_back();
    EXPECT_EQ(intRegs.r[0], 0); // should not have written
}

// ===== HALT TESTS =====
TEST_F(PipelineTest, HaltSignal_NotSquashed) {
    pipeline->wInstr.halt_signal = true;
    pipeline->wInstr.is_squashed = false;
    EXPECT_TRUE(pipeline->wInstr.halt_signal && !pipeline->wInstr.is_squashed);
}

TEST_F(PipelineTest, HaltSignal_Squashed) {
    pipeline->wInstr.halt_signal = true;
    pipeline->wInstr.is_squashed = true;
    // A squashed halt should NOT terminate
    EXPECT_FALSE(pipeline->wInstr.halt_signal && !pipeline->wInstr.is_squashed);
}


// ===== DECODE TESTS =====

// ADDI r0, r1, 5 - basic immediate, no hazard
TEST_F(PipelineTest, Decode_ADDI_NoHazard) {
    pendRegs.r[1] = 0;
    intRegs.r[1] = 10;

    unsigned int bin = (0b00 << 30) | (15 << 25) | (0 << 21) | (1 << 17) | 5;
    pipeline->dInstr.bin_instr = static_cast<int>(bin);
    pipeline->dInstr.pend_incremented = false;

    bool stalled = pipeline->decode();
    EXPECT_FALSE(stalled);
    EXPECT_EQ(pipeline->dInstr.src1v[0], 10);  // captured r1 value
    EXPECT_EQ(pipeline->dInstr.immediate, 5);
    EXPECT_EQ(pipeline->dInstr.destv[0], 0);
    EXPECT_EQ(pendRegs.r[0], 1);               // dest pending incremented
}

// ADDI blocked because src pending
TEST_F(PipelineTest, Decode_ADDI_Hazard) {
    pendRegs.r[1] = 1;

    unsigned int bin = (0b00 << 30) | (15 << 25) | (0 << 21) | (1 << 17) | 5;
    pipeline->dInstr.bin_instr = static_cast<int>(bin);
    pipeline->dInstr.pend_incremented = false;

    bool stalled = pipeline->decode();
    EXPECT_TRUE(stalled);
    EXPECT_EQ(pendRegs.r[0], 0);  // dest pending should NOT be incremented
}

// ADDI r3, r3, 1 - self block, dest == src
TEST_F(PipelineTest, Decode_ADDI_SelfBlock) {
    pendRegs.r[3] = 1;

    unsigned int bin = (0b00 << 30) | (15 << 25) | (3 << 21) | (3 << 17) | 1;
    pipeline->dInstr.bin_instr = static_cast<int>(bin);
    pipeline->dInstr.pend_incremented = false;

    bool stalled = pipeline->decode();
    EXPECT_TRUE(stalled);
}

// ADDI r3, r3, 1 - pend_incremented guard, should not re-block on own pending
TEST_F(PipelineTest, Decode_ADDI_PendIncrementedGuard) {
    pendRegs.r[3] = 1;  // already incremented by this instruction

    unsigned int bin = (0b00 << 30) | (15 << 25) | (3 << 21) | (3 << 17) | 1;
    pipeline->dInstr.bin_instr = static_cast<int>(bin);
    pipeline->dInstr.pend_incremented = true;  // already incremented

    bool stalled = pipeline->decode();
    EXPECT_FALSE(stalled);  // should NOT self block
}

// ADD r0, r1, r2 - scalar, no hazard
TEST_F(PipelineTest, Decode_ADD_NoHazard) {
    intRegs.r[1] = 3;
    intRegs.r[2] = 7;
    pendRegs.r[1] = 0;
    pendRegs.r[2] = 0;

    unsigned int bin = (0b00 << 30) | (0 << 25) | (0 << 21) | (1 << 17) | (2 << 13);
    pipeline->dInstr.bin_instr = static_cast<int>(bin);
    pipeline->dInstr.pend_incremented = false;

    bool stalled = pipeline->decode();
    EXPECT_FALSE(stalled);
    EXPECT_EQ(pipeline->dInstr.src1v[0], 3);
    EXPECT_EQ(pipeline->dInstr.src2v[0], 7);
    EXPECT_EQ(pendRegs.r[0], 1);
}

// ADD r0, r1, r2 - src2 has hazard
TEST_F(PipelineTest, Decode_ADD_Src2Hazard) {
    pendRegs.r[1] = 0;
    pendRegs.r[2] = 1;

    unsigned int bin = (0b00 << 30) | (0 << 25) | (0 << 21) | (1 << 17) | (2 << 13);
    pipeline->dInstr.bin_instr = static_cast<int>(bin);
    pipeline->dInstr.pend_incremented = false;

    bool stalled = pipeline->decode();
    EXPECT_TRUE(stalled);
    EXPECT_EQ(pendRegs.r[0], 0);
}

// CMP r1, r2 - no hazard, should increment CR pending
TEST_F(PipelineTest, Decode_CMP_NoHazard) {
    intRegs.r[1] = 5;
    intRegs.r[2] = 3;
    pendRegs.r[1] = 0;
    pendRegs.r[2] = 0;
    pendRegs.r[14] = 0;

    unsigned int bin = (0b00 << 30) | (27 << 25) | (1 << 21) | (2 << 17);
    pipeline->dInstr.bin_instr = static_cast<int>(bin);
    pipeline->dInstr.pend_incremented = false;

    bool stalled = pipeline->decode();
    EXPECT_FALSE(stalled);
    EXPECT_EQ(pipeline->dInstr.src1v[0], 5);
    EXPECT_EQ(pipeline->dInstr.src2v[0], 3);
    EXPECT_EQ(pendRegs.r[14], 1);  // CR pending incremented
}

// CMPI r1, 5 - no hazard
TEST_F(PipelineTest, Decode_CMPI_NoHazard) {
    intRegs.r[1] = 10;
    pendRegs.r[1] = 0;

    unsigned int bin = (0b00 << 30) | (30 << 25) | (1 << 21) | (5 & 0x1FFFFF);
    pipeline->dInstr.bin_instr = static_cast<int>(bin);
    pipeline->dInstr.pend_incremented = false;

    bool stalled = pipeline->decode();
    EXPECT_FALSE(stalled);
    EXPECT_EQ(pipeline->dInstr.src1v[0], 10);
    EXPECT_EQ(pipeline->dInstr.immediate, 5);
    EXPECT_EQ(pendRegs.r[14], 1);
}

// BEQ - conditional branch blocked on CR pending
TEST_F(PipelineTest, Decode_BEQ_CRPending) {
    pendRegs.r[14] = 1;

    unsigned int bin = (0b01 << 30) | (1 << 26) | (5 & 0x3FFFFFF);
    pipeline->dInstr.bin_instr = static_cast<int>(bin);
    pipeline->dInstr.pend_incremented = false;

    bool stalled = pipeline->decode();
    EXPECT_TRUE(stalled);
}

// BEQ - conditional branch passes when CR clear
TEST_F(PipelineTest, Decode_BEQ_CRClear) {
    pendRegs.r[14] = 0;

    unsigned int bin = (0b01 << 30) | (1 << 26) | (5 & 0x3FFFFFF);
    pipeline->dInstr.bin_instr = static_cast<int>(bin);
    pipeline->dInstr.pend_incremented = false;

    bool stalled = pipeline->decode();
    EXPECT_FALSE(stalled);
    EXPECT_EQ(pipeline->dInstr.branch_offset, 5);
    EXPECT_EQ(pendRegs.r[13], 1);  // PC pending incremented
}

// LDB r0, r1, 4 - no hazard
TEST_F(PipelineTest, Decode_LDB_NoHazard) {
    intRegs.r[1] = 32;
    pendRegs.r[1] = 0;

    unsigned int bin = (0b10 << 30) | (6 << 26) | (0 << 22) | (1 << 18) | (4 & 0x3FFFF);
    pipeline->dInstr.bin_instr = static_cast<int>(bin);
    pipeline->dInstr.pend_incremented = false;

    bool stalled = pipeline->decode();
    EXPECT_FALSE(stalled);
    EXPECT_EQ(pipeline->dInstr.src1v[0], 32);  // base register value
    EXPECT_EQ(pipeline->dInstr.immediate, 4);   // offset
    EXPECT_EQ(pendRegs.r[0], 1);
}

// Squashed instruction should not decode
TEST_F(PipelineTest, Decode_Squashed_DoesNothing) {
    pendRegs.r[1] = 0;
    intRegs.r[1] = 10;

    unsigned int bin = (0b00 << 30) | (15 << 25) | (0 << 21) | (1 << 17) | 5;
    pipeline->dInstr.bin_instr = static_cast<int>(bin);
    pipeline->dInstr.is_squashed = true;
    pipeline->dInstr.pend_incremented = false;

    bool stalled = pipeline->decode();
    EXPECT_FALSE(stalled);
    EXPECT_EQ(pendRegs.r[0], 0);  // nothing should have been incremented
}

// ADDI r0, r1, -1 - checks helper_unsigned_to_signed works correctly
TEST_F(PipelineTest, Decode_ADDI_NegativeImmediate) {
    intRegs.r[1] = 5;
    pendRegs.r[1] = 0;

    int imm = -1 & 0x1FFFF;  // -1 in 17 bits
    unsigned int bin = (0b00 << 30) | (15 << 25) | (0 << 21) | (1 << 17) | imm;
    pipeline->dInstr.bin_instr = static_cast<int>(bin);
    pipeline->dInstr.pend_incremented = false;

    bool stalled = pipeline->decode();
    EXPECT_FALSE(stalled);
    EXPECT_EQ(pipeline->dInstr.immediate, -1);  // should be sign extended to -1
}

// ADD where both src1 and src2 are pending
TEST_F(PipelineTest, Decode_ADD_BothSrcsPending) {
    pendRegs.r[1] = 1;
    pendRegs.r[2] = 1;

    unsigned int bin = (0b00 << 30) | (0 << 25) | (0 << 21) | (1 << 17) | (2 << 13);
    pipeline->dInstr.bin_instr = static_cast<int>(bin);
    pipeline->dInstr.pend_incremented = false;

    bool stalled = pipeline->decode();
    EXPECT_TRUE(stalled);
    EXPECT_EQ(pendRegs.r[0], 0);
}

TEST_F(PipelineTest, Decode_STR_NoPendingIncrement) {
    intRegs.r[0] = 32;  // address
    intRegs.r[1] = 99;  // value
    pendRegs.r[0] = 0;
    pendRegs.r[1] = 0;

    unsigned int bin = (0b10 << 30) | (2 << 26) | (0 << 22) | (1 << 18);
    pipeline->dInstr.bin_instr = static_cast<int>(bin);
    pipeline->dInstr.pend_incremented = false;

    bool stalled = pipeline->decode();
    EXPECT_FALSE(stalled);
    // No register should have pending incremented
    for (int i = 0; i < 16; i++) {
        EXPECT_EQ(pendRegs.r[i], 0);
    }
}

TEST_F(PipelineTest, Decode_STRB_BlockedOnBase) {
    pendRegs.r[1] = 1;  // base register pending

    unsigned int bin = (0b10 << 30) | (7 << 26) | (0 << 22) | (1 << 18) | (1 & 0x3FFFF);
    pipeline->dInstr.bin_instr = static_cast<int>(bin);
    pipeline->dInstr.pend_incremented = false;

    bool stalled = pipeline->decode();
    EXPECT_TRUE(stalled);
}


// Fetch blocks when decode is blocked
TEST_F(PipelineTest, Fetch_BlocksWhenDecodeBlocked) {
    // Load two instructions into memory
    // ADDI r0, r1, 5 at address 0
    // ADDI r1, r1, 1 at address 1
    loadAssembly(R"(
        ADDI R0, R1, 5
        ADDI R1, R1, 1
    )");

    // Pre-load into cache, no delay make it easier to debug
    preloadToCache(0, mem->dram[0][0]);
    preloadToCache(1, mem->dram[0][1]);

    // Make r1 pending so decode will block on second instruction
    pendRegs.r[1] = 1;
    intRegs.r[13] = 0;

    // Cycle 1:
    single_clock_cycle(pipeline, true, true);
    EXPECT_EQ(intRegs.r[13], 1);  // PC advanced

    // Cycle 2:
    single_clock_cycle(pipeline, true, true);
    EXPECT_EQ(intRegs.r[13], 2);

    // Cycle 3:
    single_clock_cycle(pipeline, true, true);
    // EXPECT_TRUE(pipeline->dInstr.is_blocked);   // D is blocked
    // EXPECT_TRUE(pipeline->fInstr.is_blocked);   // F should also be blocked
    // int pc_when_blocked = intRegs.r[13];
    // EXPECT_EQ(intRegs.r[13], pc_when_blocked);  // PC should not advance

    // Cycle 4: still blocked, PC still should not advance
    single_clock_cycle(pipeline, true, true);
    EXPECT_TRUE(pipeline->dInstr.is_blocked);
    // EXPECT_TRUE(pipeline->fInstr.is_blocked);
    // EXPECT_EQ(intRegs.r[13], pc_when_blocked);  // PC frozen
}

