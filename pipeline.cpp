#include <string>
#include "pipeline.h"
#include "main.h"
#include "cache.h"
#include "registers.h"

Pipeline::Pipeline() {
    this->fcontrol = {false, false};
    this->dcontrol = {false, false};
    this->econtrol = {false, false};
    this->mcontrol = {false, false};
    this->wcontrol = {false, false};
    this->fInstr = {};
    this->dInstr = {};
    this->eInstr = {};
    this->mInstr = {};
    this->wInstr = {};
}

Memory newMemory;
Cache* newCache = new Cache(&newMemory);

Registers::IntegerRegs intRegs;
Registers::PendIntegerRegs pendRegs;


void Pipeline::fetch(std::string memoryAddress){
    std::string bin_instr = parseInput({"R", memoryAddress, "0"}, newCache);
    this->fInstr.address = stoi(memoryAddress);
    this->fInstr.bin_instr = stoi(bin_instr);
}

void Pipeline::decode(){
    // get 2 most significant bits
    int bin = this->dInstr.bin_instr;

    int opcode = (bin >> 25) & 0b11111;

    switch(bin >> 30){
        case 0:
            // integer add
            if(opcode == 0){
                // reminder these are all register numbers
                int dest = (bin >> 21) & 0b1111;
                int src1 = (bin >> 17) & 0b1111;
                int src2 = (bin >> 13) & 0b1111;
                while (!pendRegs.r[dest].empty() || !pendRegs.r[src1].empty() || !pendRegs.r[src2].empty()) {
                    continue;
                }
                this->dInstr.destv[0] = intRegs.r[dest];
                this->dInstr.src1v[0] = intRegs.r[src1];
                this->dInstr.src2v[0] = intRegs.r[src2];

            }

        default:
            break;


    }
}
