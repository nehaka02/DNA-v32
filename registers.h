#ifndef REGISTERS_H
#define REGISTERS_H
#include <vector>

namespace Registers
{
    struct IntegerRegs {
        int r[16] = {};  // zero-initializes all 16 to 0
    };

    //have vectors because a register can be on the pending list multiple times
    struct PendIntegerRegs{
        std::vector<int> r[16]; // index 0-15 maps to r0-r15
    };
};

#endif // REGISTERS_H
