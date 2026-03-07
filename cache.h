#ifndef CACHE_H
#define CACHE_H
#include "memory.h"
#include<string>

class Cache
{
public:

    //cache has a pointer to dram
    //cache memory contains: tag, index, valid, dirty, 4 offsets
    int cache_memory[8][8];
    Memory* memory;

    int currentlyServicing;
    int dramDelay;
    int clock;

    Cache(Memory* memory);
    std::string writeMemory(int address, int data, int pipelineStage);
    std::string readMemory(int address, int pipelineStage);
    int viewMemory(int address);
};

#endif // CACHE_H
