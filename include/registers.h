#ifndef REGISTERS_H
#define REGISTERS_H


namespace Registers
{
    struct IntegerRegs {
        int r[16] = {};  // zero-initializes all 16 to 0
    };

    //have vectors because a register can be on the pending list multiple times
    struct PendIntegerRegs{
        int r[16] = {}; // index 0-15 maps to r0-r15
    };

    // we want this to be a 2D array with 16 rows and 4 columns (each vector is max 4 words)
    struct VectorRegs{
        int q[16][4] = {};
    };

    struct PendVectorRegs{
        int q[16] = {};
    };
};

#endif // REGISTERS_H
