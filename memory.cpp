#include "memory.h"

Memory::Memory() {
    for(int i = 0; i < 32768; i++){
        this->memory[i][0] = i;
    }

}

int Memory::readMemory(){
    return 0;
}

void Memory::writeMemory(int address, int data){
    this->memory[address][1] = data;
}
