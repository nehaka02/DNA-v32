#ifndef MEMORY_H
#define MEMORY_H

class Memory
{
public:
    int memory[32768][2];
    int cache[8][7];

    int fClock;
    int mClock;

    int prevWriteAddr;
    int prevWriteStage;

    Memory();
    void writeMemory(int address, int data, int pipelineStage);
    int readMemory(int address, int pipelineStage);
    int viewMemory(int address);

};

#endif // MEMORY_H

