#ifndef MEMORY_H
#define MEMORY_H

class Memory
{
public:

    //memory has 4 words per line
    //memory address is calculated implicitly and doesn't need to be physically stored
    int dram[8192][4];

    Memory();

};

#endif // MEMORY_H

