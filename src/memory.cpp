#include "memory.h"
Memory::Memory() {
    // In memory.cpp
    // Initialize all DRAM to 0
    for (int i = 0; i < 8192; i++) {
        for (int j = 0; j < 4; j++) {
            dram[i][j] = 0;
        }
    }
}



