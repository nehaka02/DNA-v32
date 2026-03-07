#include "cache.h"

Cache::Cache(Memory* memory) {
    this->dramDelay=0;
    //0 is in an invalid pipeline stage
    //pipeline stages are 1-5
    this->currentlyServicing=0;
    this->memory=memory;
    this->clock=0;
}

// this method needs to check cache first
// if address doesn't exist in cache, we need to load from memory and possibly evict a line from cache
 std::string Cache::readMemory(int address, int pipelineStage){
    return "";
}

std::string Cache::writeMemory(int address, int data, int pipelineStage){
    if(this->currentlyServicing != 0 && this->currentlyServicing != pipelineStage){
        return "Wait";
    }
    else{
        //Currently servicing no pipeline stage -> service requested pipeline stage
        //Do not decrement delay count on first attempt
        if(this->currentlyServicing == 0){
            this->currentlyServicing=pipelineStage;
            this->dramDelay=3;
            return "Wait";
        }
        //Currently servicing requested pipeline stage but delay has not expired
        else if(this->currentlyServicing == pipelineStage && this->dramDelay!=0){
            this->dramDelay -= 1;
            return "Wait";
        }
        //Delay has expired for requested pipeline stage (i.e., dramClock=0)
        else{

            int line   = (address / 4) % 8192;  // which DRAM line
            int index  = line % 8;              // which cache slot
            int offset = address % 4;           // which word within line
            int tag    = line / 8;              // tag

            //write hit (write through)
            //must check valid bit
            if(this->cache_memory[index][0] == tag && this->cache_memory[index][2] == 1){
                //set dirty bit to 0 and valid bit to 1
                this->cache_memory[index][2] = 0;
                this->cache_memory[index][3] = 1;
                this->cache_memory[index][4 + offset] = data;
                this->memory->dram[line][offset] = data;
            }
            //write miss (no allocate)
            else{
                this->memory->dram[line][offset] = data;
            }

            //Reset currentlyServicing variable to 0 to indicate no pipeline stage is being serviced
            this->currentlyServicing=0;
            return "Done";
        }
    }
}

// side door interface
int Cache::viewMemory(int address){
    return 0;

}
