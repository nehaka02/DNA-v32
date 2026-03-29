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

    switch(bin >> 30){
        // ALU Ops
        case 0:{
            int opcode = (bin >> 25) & 0b11111;
            // integer add
            if(opcode == 0){
                // reminder these are all register numbers
                int dest = (bin >> 21) & 0b1111;
                int src1 = (bin >> 17) & 0b1111;
                int src2 = (bin >> 13) & 0b1111;
                if (!pendRegs.r[dest].empty() || !pendRegs.r[src1].empty() || !pendRegs.r[src2].empty()) {
                    this->dcontrol.is_stalled = true;
                    return;
                }
                this->dInstr.destv.push_back(intRegs.r[dest]);
                this->dInstr.src1v.push_back(intRegs.r[src1]);
                this->dInstr.src2v.push_back(intRegs.r[src2]);

            }
            // compare
            if(opcode == 28){
                int src1 = (bin >> 17) & 0b1111;
                int src2 = (bin >> 13) & 0b1111;
                while (!pendRegs.r[src1].empty() || !pendRegs.r[src2].empty()) {
                    continue;
                }
                this->dInstr.src1v.push_back(intRegs.r[src1]);
                this->dInstr.src2v.push_back(intRegs.r[src2]);
            }

            break;
        }

        // Branching
        case 1:{
            int opcode = (bin >> 25) & 0b1111;
            //everything besides BX instruction
            if(opcode == 0 || opcode == 1 || opcode == 2 || opcode == 3 || opcode == 4 || opcode == 5 || opcode == 6 || opcode == 7){
                this->dInstr.branch_offset = bin & 0x3FFFFFF;
            }

            break;
        }

        // Miscellaneous
        case 2:{
            int opcode = (bin >> 25) & 0b1111;
            // load base + offset
            if(opcode == 6){
                int dest = (bin >> 26) & 0b1111;
                int base = (bin >> 22) & 0b1111; // this will go in src1 field of instruction object (base is a register!)
                int offset = (bin >> 13) & 0b1111; // this will go in immediate field of instruction object

                if (!pendRegs.r[dest].empty() || !pendRegs.r[base].empty()) {
                    this->dcontrol.is_stalled = true;
                    return;
                }

                this->dInstr.destv.push_back(intRegs.r[dest]);
                this->dInstr.src1v.push_back(intRegs.r[base]);
                this->dInstr.immediate = offset;
            }
            // store base + offset
            if(opcode == 7){
                int src1 = (bin >> 26) & 0b1111;
                int base = (bin >> 22) & 0b1111; // this will go in src2 field of instruction object (base is a register!)
                int offset = (bin >> 13) & 0b1111; // this will go in immediate field of instruction object

                if (!pendRegs.r[src1].empty() || !pendRegs.r[base].empty()) {
                    this->dcontrol.is_stalled = true;
                    return;
                }

                this->dInstr.src1v.push_back(intRegs.r[src1]);
                this->dInstr.src2v.push_back(intRegs.r[base]);
                this->dInstr.immediate = offset;

            }

            break;

        }

        default:
            break;


    }
}
