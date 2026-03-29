#ifndef PIPELINE_H
#define PIPELINE_H
#include <vector>

struct ControlBlock {
    bool is_blocked;
    bool is_stalled;
};

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
};


class Pipeline
{
public:
    ControlBlock fcontrol;
    ControlBlock dcontrol;
    ControlBlock econtrol;
    ControlBlock mcontrol;
    ControlBlock wcontrol;

    InstructionObject fInstr;
    InstructionObject dInstr;
    InstructionObject eInstr;
    InstructionObject mInstr;
    InstructionObject wInstr;

    Pipeline();

    void fetch(std::string memoryAddress);
    void decode();

};

#endif // PIPELINE_H
