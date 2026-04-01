#ifndef PIPELINE_H
#define PIPELINE_H
#include <vector>
#include <string>
#include "cache.h"

// note that all fields don't need to be used for every instruction
// scalars are one-element vectors
struct InstructionObject {
    std::vector<int> src1v;
    std::vector<int> src2v;
    std::vector<int> destv;
    int address    = 0;
    int branch_offset = 0;
    int bin_instr  = 0;
    int type_code  = 0;
    int opcode     = 0;
    int immediate  = 0;
    int vlen       = 0;
    int result     = 0;
    bool is_stalled = false;
    bool is_blocked = false;
    bool halt_signal = false;
};


class Pipeline
{
public:

    InstructionObject fInstr;
    InstructionObject dInstr;
    InstructionObject eInstr;
    InstructionObject mInstr;
    InstructionObject wInstr;
    Cache *newCache;
    int global_clock;

    Pipeline(Cache* externalCache);

    bool fetch();
    bool decode();
    void execute();
    bool memory_access();
    void write_back();
    void print_state();

private :
    int ALU_helper(int opcode, int a, int b);
    int branch_helper();

};

#endif // PIPELINE_H
