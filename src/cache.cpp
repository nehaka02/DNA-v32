#include "cache.h"
#include <iostream>
#include <iomanip>

Cache::Cache(Memory* memory) {
    this->dramDelay=0;
    //0 is in an invalid pipeline stage
    //pipeline stages are 1-5
    this->currentlyServicing=0;
    this->memory=memory;
    this->clock=0;

    // initializing cache to 0

    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            cache_memory[i][j] = 0;
        }
    }
}

void Cache::decodeAddress(int address, int &line, int &index, int &offset, int &tag) {
    line   = (address / 4) % 8192;
    index  = line % 8;
    offset = address % 4;
    tag    = line / 8;
}

// this method needs to check cache first
// if address doesn't exist in cache, we need to load from memory and possibly evict a line from cache
 std::string Cache::readMemory(int address, int pipelineStage, bool isVector){
    // Currently servicing another pipeline stage
    if(this->currentlyServicing != 0 && this->currentlyServicing != pipelineStage) {
        return "Wait: memory is currently servicing another pipeline stage";
    }
    else {

        // Map out values
        int line, index, offset, tag;
        decodeAddress(address, line, index, offset, tag);

        // Cache hit: If valid data found in cache, 0 delay, return value directly
        if (this->cache_memory[index][3] == 1 && this->cache_memory[index][0] == tag) {
            this->dramDelay = 0;
            this->currentlyServicing = 0;
            std::cout << "Cache Hit!" << std::endl;
            if(!isVector){
                return "Done: " + std::to_string(this->cache_memory[index][4 + offset]);
            }
            else{
                // we will enforce 4-word alignment of vectors in pipeline.cpp
                std::string result = "Done: ";
                for (int i = 0; i < 4; i++) {
                    result += std::to_string(this->cache_memory[index][4 + i]);
                    if (i < 3) result += ", ";
                }
                return result;
            }
        }

        //Currently servicing no pipeline stage -> service requested pipeline stage
        if(this->currentlyServicing == 0){
            this->dramDelay = 3;
            this->currentlyServicing=pipelineStage;
            std::cout << "Cache miss. Starting new access cycle... Wait" << std::endl;
            return "Wait";
        }

        // No valid memory found in cache, load from memory and direct eviction from cache
        // Remember 3 tick delay
        else if (this->currentlyServicing == pipelineStage && this->dramDelay != 0) {
            this->dramDelay -= 1;
            std::cout << "Cache miss, loading from memory... Wait" << std::endl;
            return "Wait";
        }

        //Delay has expired for requested pipeline stage (i.e., dramClock=0)
        else {
            // Update value in cache with value from memory (Don't forget tag, idx, valid, dirty)
            this->cache_memory[index][0] = tag;
            this->cache_memory[index][2] = 0; // Dirty bit
            this->cache_memory[index][3] = 1; // Valid bit
            // Update all 4 blocks in cache
            for (int i = 0; i < 4; i ++) {
                this->cache_memory[index][4 + i] = this->memory->dram[line][i];
            }
            this->currentlyServicing = 0;

            // Return result (same logic as cache hit)
            if (!isVector) {
                return "Done: " + std::to_string(this->cache_memory[index][4 + offset]);
            }
            else {
                std::string result = "Done: ";
                for (int i = 0; i < 4; i++) {
                    result += std::to_string(this->cache_memory[index][4 + i]);
                    if (i < 3) result += ", ";
                }
                return result;
            }

        }
    }
}

std::string Cache::writeMemory(int address, const int data[4], int pipelineStage, bool isVector){
    if(this->currentlyServicing != 0 && this->currentlyServicing != pipelineStage){
        return "Wait: memory is currently servicing another pipeline stage";
    }
    else{
        //Currently servicing no pipeline stage -> service requested pipeline stage
        //Do not decrement delay count on first attempt
        if(this->currentlyServicing == 0){
            this->currentlyServicing=pipelineStage;
            this->dramDelay=3;
            std::cout << "Starting new access cycle... Wait" << std::endl;
            return "Wait";
        }
        //Currently servicing requested pipeline stage but delay has not expired
        else if(this->currentlyServicing == pipelineStage && this->dramDelay!=0){
            this->dramDelay -= 1;
            std::cout << "Writing to memory... Wait" << std::endl;
            return "Wait";
        }
        //Delay has expired for requested pipeline stage (i.e., dramClock=0)
        else{
            // Map out values
            int line, index, offset, tag;
            decodeAddress(address, line, index, offset, tag);

            //write hit (write through)
            //must check valid bit
            if(this->cache_memory[index][0] == tag && this->cache_memory[index][3] == 1){
                //set dirty bit to 0 and valid bit to 1
                this->cache_memory[index][2] = 0;
                this->cache_memory[index][3] = 1;

                if(!isVector){
                    this->cache_memory[index][4 + offset] = data[0];
                    this->memory->dram[line][offset] = data[0];
                }
                else{
                    for (int i = 0; i < 4; i++) {
                        this->cache_memory[index][4 + i] = data[i];
                        this->memory->dram[line][i] = data[i];
                    }
                }

            }
            //write miss (no allocate)
            else{
                if(!isVector){
                    this->memory->dram[line][offset] = data[0];
                }
                else{
                    for (int i = 0; i < 4; i++) {
                        this->memory->dram[line][i] = data[i];
                    }
                }
            }

            //Reset currentlyServicing variable to 0 to indicate no pipeline stage is being serviced
            this->currentlyServicing=0;
            return "Done";
        }
    }
}

// side door interface
int Cache::viewMemory(int address){
    // Map out values
    int line, index, offset, tag;
    decodeAddress(address, line, index, offset, tag);
    std::cout << "Address : " << address                    << std::endl;
    std::cout << "Tag     : " << cache_memory[index][0]     << std::endl;
    std::cout << "Index   : " << index                      << std::endl;
    std::cout << "Dirty   : " << cache_memory[index][2]     << std::endl;
    std::cout << "Valid   : " << cache_memory[index][3]     << std::endl;
    std::cout << "Offset  : " << offset                     << std::endl;
    std::cout << "Data    : " << cache_memory[index][4 + offset] << std::endl;

    return cache_memory[index][4 + offset];

}

// Edit cache? (set dirty bit to 1 after edit)
void Cache::editCache(int address, int data) {
    int line, index, offset, tag;
    decodeAddress(address, line, index, offset, tag);
    this->cache_memory[index][4 + offset] = data;
}


// Print entire cache?
void Cache::printCache(){
    std::cout << "===== CACHE STATE =====" << std::endl;
    std::cout << std::left
              << std::setw(6)  << "Line"
              << std::setw(6)  << "Tag"
              << std::setw(8)  << "Dirty"
              << std::setw(8)  << "Valid"
              << std::setw(30) << "Words [0,1,2,3]" << std::endl;
    std::cout << std::string(58, '-') << std::endl;

    for(int i = 0; i < 8; i++){
        std::cout << std::left
                  << std::setw(6)  << i
                  << std::setw(6)  << cache_memory[i][0]
                  << std::setw(8)  << cache_memory[i][2]
                  << std::setw(8)  << cache_memory[i][3]
                  << "[" << cache_memory[i][4] << ", "
                  << cache_memory[i][5] << ", "
                  << cache_memory[i][6] << ", "
                  << cache_memory[i][7] << "]" << std::endl;
    }
    std::cout << "=======================" << std::endl;
    std::cout << std::endl;
}


// For demo, only use the first 9 lines of memory
void Cache:: printMemory(int startLine, int endLine) {
    std::cout << "===== DRAM =====" << std::endl;
    std::cout << std::left
              << std::setw(15)  << "Address"
              << std::setw(30) << "Words [0,1,2,3]" << std::endl;
    std::cout << std::string(58, '-') << std::endl;

    for(int i = startLine; i <= endLine; i++){
        std::cout << std::left
                  << std::setw(8) << i
                  << "[" << memory->dram[i][0] << ", "
                  << memory->dram[i][1] << ", "
                  << memory->dram[i][2] << ", "
                  << memory->dram[i][3] << "]" << std::endl;
    }
    std::cout << "========================" << std::endl;
    std::cout << std::endl;
}
