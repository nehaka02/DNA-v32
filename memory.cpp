#include "memory.h"
#include <iostream>


Memory::Memory() {
    for(int i = 0; i < 32768; i++){
        this->memory[i][0] = i;
    }
    this->prevWriteAddr=-1;
    this->prevWriteStage=-1;
}

// this method needs to check cache first
// if address doesn't exist in cache, we need to load from memory and possibly evict a line from cache
int Memory::readMemory(int address, int pipelineStage){
    return 0;
}

void Memory::writeMemory(int address, int data, int pipelineStage){

    if(this->fClock != 0 && this->mClock == 0){
        std::cout << "Wait" << std::endl;
    }
    else if(this->fClock == 0 && this->mClock == 0){
        this->prevWriteAddr=address;
        this->prevWriteStage=pipelineStage;

        this->mClock = 3;
        std::cout << "Wait" << std::endl;
    }
    else if(this->fClock == 0 && this->mClock != 0){
        if(this->prevWriteAddr==address && this->prevWriteStage==pipelineStage){
            this->mClock -= 1;
            std::cout << "Wait" << std::endl;
        }
        else{
            std::cout << "Error" << std::endl;
        }
    }
    else if(this->mClock == 0){
        std::cout << "Done" << std::endl;
    }

    this->memory[address][1] = data;
}

// side door interface
int Memory::viewMemory(int address){
    return 0;

}
