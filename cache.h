#ifndef CACHE_H
#define CACHE_H
#include "memory.h"
#include<string>

class Cache
{
public:

    //cache has a pointer to dram
    //cache memory contains: tag, index, dirty, valid, 4 offsets
    int cache_memory[8][8];
    Memory* memory;

    int currentlyServicing;
    int dramDelay;
    int clock;

    Cache(Memory* memory);
    std::string writeMemory(int address, int data, int pipelineStage);
    std::string readMemory(int address, int pipelineStage);
    int viewMemory(int address);
    void editCache(int address, int data);
    void printCache();
    void printMemory(int startAddress, int endAddress);
private:
    void decodeAddress(int address, int &line, int &index, int &offset, int &tag);
};

#endif // CACHE_H
