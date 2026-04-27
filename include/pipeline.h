#ifndef PIPELINE_H
#define PIPELINE_H
#include <vector>
#include <string>
#include "cache.h"

// note that all fields don't need to be used for every instruction
// scalars are one-element vectors
struct InstructionObject {
    // std::vector<int> src1v;
    // std::vector<int> src2v;
    // std::vector<int> destv;

    int src1v[4] = {};
    int src2v[4] = {};
    int destv[4] = {};

    int address    = -1;
    int branch_offset = -1;
    int bin_instr  = -1;
    int type_code  = -1;
    int opcode     = -1;
    int immediate  = -1;
    int vlen       = -1;
    int result[4]     = {};
    bool is_stalled = false;
    bool is_blocked = false;
    bool is_squashed = false;
    bool halt_signal = false;
    bool pend_incremented = false;
};

class Pipeline
{

#ifdef TESTING
    friend class PipelineTest;
#endif

public:

    InstructionObject fInstr;
    InstructionObject dInstr;
    InstructionObject eInstr;
    InstructionObject mInstr;
    InstructionObject wInstr;

    // To preserve state before fowarding for correct rendering in UI
    InstructionObject displayF{}, displayD{}, displayE{}, displayM{}, displayW{};

    Cache *newCache;
    int global_clock;
    bool squashed = false;

    Pipeline(Cache* externalCache);

    bool fetch(bool cacheEnabled, int pc_to_fetch);
    bool decode();
    void execute();
    bool memory_access(bool cacheEnabled);
    void write_back();
    void print_state();
    std::string instrToString(const InstructionObject& instr);

private :
    int ALU_helper(int opcode, int a, int b);
    int branch_helper(int addr);
    int helper_unsigned_to_signed(int val, int bits);


};

#endif // PIPELINE_H
