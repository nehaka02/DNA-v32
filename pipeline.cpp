#include <string>
#include "pipeline.h"
#include "main.h"
#include "cache.h"
#include "registers.h"

Pipeline::Pipeline() {
    this->fcontrol = {false, false};
    this->dcontrol = {false, false};
    this->econtrol = {false, false};
    this->mcontrol = {false, false};
    this->wcontrol = {false, false};
    this->fInstr = {};
    this->dInstr = {};
    this->eInstr = {};
    this->mInstr = {};
    this->wInstr = {};
}

Memory newMemory;
Cache* newCache = new Cache(&newMemory);

Registers::IntegerRegs intRegs;
Registers::PendIntegerRegs pendRegs;


void Pipeline::fetch(std::string memoryAddress){
    std::string bin_instr = parseInput({"R", memoryAddress, "0"}, newCache);
    this->fInstr.address = stoi(memoryAddress);
    this->fInstr.bin_instr = stoi(bin_instr);
}

void Pipeline::decode(){
    // get 2 most significant bits
    int bin = this->dInstr.bin_instr;

    switch(bin >> 30){
        // ALU Ops
        case 0:{
            int opcode = (bin >> 25) & 0b11111;
            // integer add
            if(opcode == 0){
                // reminder these are all register numbers
                int dest = (bin >> 21) & 0b1111;
                int src1 = (bin >> 17) & 0b1111;
                int src2 = (bin >> 13) & 0b1111;
                if (!pendRegs.r[dest].empty() || !pendRegs.r[src1].empty() || !pendRegs.r[src2].empty()) {
                    this->dcontrol.is_stalled = true;
                    return;
                }
                this->dInstr.destv.push_back(intRegs.r[dest]);
                this->dInstr.src1v.push_back(intRegs.r[src1]);
                this->dInstr.src2v.push_back(intRegs.r[src2]);

            }
            // compare
            if(opcode == 28){
                int src1 = (bin >> 17) & 0b1111;
                int src2 = (bin >> 13) & 0b1111;
                while (!pendRegs.r[src1].empty() || !pendRegs.r[src2].empty()) {
                    continue;
                }
                this->dInstr.src1v.push_back(intRegs.r[src1]);
                this->dInstr.src2v.push_back(intRegs.r[src2]);
            }

            break;
        }

        // Branching
        case 1:{
            int opcode = (bin >> 25) & 0b1111;
            //everything besides BX instruction
            if(opcode == 0 || opcode == 1 || opcode == 2 || opcode == 3 || opcode == 4 || opcode == 5 || opcode == 6 || opcode == 7){
                this->dInstr.branch_offset = bin & 0x3FFFFFF;
            }

            break;
        }

        // Miscellaneous
        case 2:{
            int opcode = (bin >> 25) & 0b1111;
            // load base + offset
            if(opcode == 6){
                int dest = (bin >> 26) & 0b1111;
                int base = (bin >> 22) & 0b1111; // this will go in src1 field of instruction object (base is a register!)
                int offset = (bin >> 13) & 0b1111; // this will go in immediate field of instruction object

                if (!pendRegs.r[dest].empty() || !pendRegs.r[base].empty()) {
                    this->dcontrol.is_stalled = true;
                    return;
                }

                this->dInstr.destv.push_back(intRegs.r[dest]);
                this->dInstr.src1v.push_back(intRegs.r[base]);
                this->dInstr.immediate = offset;
            }
            // store base + offset
            if(opcode == 7){
                int src1 = (bin >> 26) & 0b1111;
                int base = (bin >> 22) & 0b1111; // this will go in src2 field of instruction object (base is a register!)
                int offset = (bin >> 13) & 0b1111; // this will go in immediate field of instruction object

                if (!pendRegs.r[src1].empty() || !pendRegs.r[base].empty()) {
                    this->dcontrol.is_stalled = true;
                    return;
                }

                this->dInstr.src1v.push_back(intRegs.r[src1]);
                this->dInstr.src2v.push_back(intRegs.r[base]);
                this->dInstr.immediate = offset;

            }

            break;

        }

        default:
            break;


    }
}

void Pipeline::execute(){

    switch(this->eInstr.type_code){

        case 0: // Data Processing
            this->eInstr.result = ALU_helper(
                this->eInstr.opcode,
                this->eInstr.src1v[0],
                this->eInstr.src2v[0]  // D puts immediate into src2v[0] for immediate instructions
                );
            break;

        case 2: // Miscellaneous
            switch(this->eInstr.opcode){
                case 0b0000: // NOT
                    this->eInstr.result = ~this->eInstr.src1v[0];
                    break;
                case 0b0001: // LD
                case 0b0010: // STR
                    this->eInstr.result = this->eInstr.src1v[0];
                    break;
                case 0b0110: // LDB
                case 0b0111: // STRB
                    this->eInstr.result = this->eInstr.src1v[0] + this->eInstr.immediate;
                    break;
                case 0b1000: // LDI
                    this->eInstr.result = this->eInstr.immediate;
                    break;
            }
            break;

        case 1: // Branch, E does nothing
            break;

    }
}

void Pipeline::memory_access(){
    switch(this->mInstr.type_code){

        case 0: // ALU, do nothing
            break;

        case 2: // Miscellaneous
            switch(this->mInstr.opcode){
                case 0b0001: // LD
                case 0b0110:{ // LDB
                    // result holds the address, go fetch the value
                    std::string val = parseInput(
                        {"R", std::to_string(this->mInstr.result), "0"},
                        newCache
                    );
                    this->mInstr.result = stoi(val);
                    break;
                }
                case 0b0010: // STR
                case 0b0111: { // STRB
                    parseInput(
                        {"W", std::to_string(this->mInstr.result), std::to_string(this->mInstr.src1v[0])},
                        newCache
                    );
                    break;
                }
                case 0b0000: // NOT
                case 0b1000: // LDI
                    break;   // nothing to do
            }
            break;

        case 1: // Branch
            break;

    }
}

void Pipeline::write_back(){
    switch(this->wInstr.type_code){
        case 0: {  // ALU
            int opcode = this->wInstr.opcode;

            // CMP and CMPI update Condition Register instead of writing to a register
            if(opcode == 0b11100 || opcode == 0b11111){
                int result = this->wInstr.result;
                // N = (result < 0) ? 1 : 0;
                // Z = (result == 0) ? 1 : 0;
                // // V bit — check for underflow
                // V = (result > this->wInstr.src1v[0]) ? 1 : 0;
                break;
            }

            // All other ALU ops write to dest register
            intRegs.r[this->wInstr.destv[0]] = this->wInstr.result;
            break;
        }

        case 2: {  // Miscellaneous
            int opcode = this->wInstr.opcode;
            if(opcode == 0b0010 || opcode == 0b0111){
                // STR / STRB, already written in M, nothing to do
            }
            else{
                // LD, LDB, LDI, NOT — all write result to dest register
                intRegs.r[this->wInstr.destv[0]] = this->wInstr.result;
            }
            break;
        }

        case 1: { // Branch   update PC

        }
    }
}

// FIXME: Move to separate file if needed
int Pipeline::ALU_helper(int opcode, int a, int b) {

    switch(opcode){

        case 0b00000: return a + b;                                          // ADD
        case 0b00001: return a - b;                                          // SUB
        case 0b00010: return (b == 0) ? 0xFFFFFFFF : a / b;                  // DIV
        case 0b00011: return a * b;                                          // MUL
        case 0b00100: return (b == 0) ? a : a % b;                           // MOD
        case 0b00101: return a >> b;                                         // ASR
        case 0b00111: return a << b;                                         // ASL
        case 0b01000: return (unsigned int)a >> b;                           // LSR
        case 0b01001: return a << b;                                         // LSL
        case 0b01010: return a & b;                                          // AND
        case 0b01011: return a | b;                                          // OR
        case 0b01100: return a ^ b;                                          // XOR
        case 0b10000: return a + b;                                          // ADDI
        case 0b10001: return a - b;                                          // SUBI
        case 0b10010: return a * b;                                          // MULI
        case 0b11000: return (b == 0) ? 0xFFFFFFFF : a / b;                  // DIVI
        case 0b11001: return (b == 0) ? a : a % b;                           // MODI
        case 0b10011: return a >> b;                                         // ASRI
        case 0b10100: return a << b;                                         // ASLI
        case 0b11010: return (unsigned int)a >> b;                           // LSRI
        case 0b11011: return a << b;                                         // LSLI
        case 0b10101: return a & b;                                          // ANDI
        case 0b10110: return a | b;                                          // ORI
        case 0b10111: return a ^ b;                                          // XORI
        case 0b11100: return a - b;                                          // CMP
        case 0b11111: return a - b;                                          // CMPI
        default:      return 0;

    }
}
