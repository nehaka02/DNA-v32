#ifndef MEMORY_H
#define MEMORY_H

class Memory
{
public:
    int memory[32768][2];
    int cache[8][7];
    Memory();
    void writeMemory(int address, int data);
    int readMemory();

};

#endif // MEMORY_H

