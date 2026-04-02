#include <string>
#include <iostream>

#include "pipeline.h"
#include "cache.h"
#include "registers.h"

Pipeline::Pipeline(Cache* externalCache) {
    this->fInstr = {};
    this->dInstr = {};
    this->eInstr = {};
    this->mInstr = {};
    this->wInstr = {};
    this->global_clock = 0;
    this->newCache = externalCache;

}

Registers::IntegerRegs intRegs;
Registers::PendIntegerRegs pendRegs;


bool Pipeline::fetch(){
    int current_pc = intRegs.r[13];

    std::string readValue = this->newCache->readMemory(current_pc, 1);
    if (readValue.rfind("Done:", 0) == 0) {  // starts with "Done:"
        this->fInstr.address = current_pc;
        this->fInstr.bin_instr = static_cast<int>(stoul(readValue.substr(6)));
        this->fInstr.is_blocked = false;

        // FIXME: PC should be updated when f is able to pass on it's instruction block
        // //UPDATE PC HERE
        // if(!this->dInstr.is_blocked){
        //      intRegs.r[13]++;
        //  }

        return false;
    }
    else{
        this->fInstr = InstructionObject{};
        this->fInstr.is_blocked = true;
        this->newCache->clock++;
        return true;
    }
}

bool Pipeline::decode(){

    // if (this->dInstr.is_stalled) {
    //     return false; // Do nothing for bubbles, and don't stall
    // }

    if(this->dInstr.is_stalled || this->dInstr.bin_instr == -1 ||this->dInstr.address == -2){
        return false;
    }

    // get 2 most significant bits
    unsigned int bin = this->dInstr.bin_instr;
    int type_code = bin >> 30;
    this->dInstr.type_code = type_code;
    
              
    switch(type_code){
        // ALU Ops
        case 0:{
            int opcode = (bin >> 25) & 0b11111;
            this->dInstr.opcode = opcode;
            // integer add
            if(opcode == 0){
                // reminder these are all register numbers
                int dest = (bin >> 21) & 0b1111;
                int src1 = (bin >> 17) & 0b1111;
                int src2 = (bin >> 13) & 0b1111;
                if (pendRegs.r[dest] != 0 || pendRegs.r[src1] != 0 || pendRegs.r[src2] != 0) {
                    return true;
                }

                //this->dInstr.destv.push_back(intRegs.r[dest]);
                this->dInstr.destv.push_back(dest);
                this->dInstr.src1v.push_back(intRegs.r[src1]);
                this->dInstr.src2v.push_back(intRegs.r[src2]);

                pendRegs.r[dest]++;
                return false;

            }
            // compare
            if(opcode == 28){
                int src1 = (bin >> 17) & 0b1111;
                int src2 = (bin >> 13) & 0b1111;
                if (pendRegs.r[src1] != 0 || pendRegs.r[src2] != 0) {
                    return true;
                }
                this->dInstr.src1v.push_back(intRegs.r[src1]);
                this->dInstr.src2v.push_back(intRegs.r[src2]);
                return false;
            }

            break;
        }

        // Branching
        case 1:{

            int opcode = (bin >> 26) & 0b1111;

            this->dInstr.opcode = opcode;

            //everything besides BX instruction
            if(opcode == 0 || opcode == 1 || opcode == 2 || opcode == 3 || opcode == 4 || opcode == 5 || opcode == 6 || opcode == 7){
                this->dInstr.branch_offset = bin & 0x3FFFFFF;
                return false;
            }
            if(opcode == 8) { // BX
                int src = (bin >> 22) & 0b1111;
                this->dInstr.src1v.push_back(intRegs.r[src]);
            }
            if(opcode == 7) { // LR is pending
                pendRegs.r[12]++;
            }
            pendRegs.r[13]++; // PC register pending
            break;
        }

        // Miscellaneous
        case 2:{
            int opcode = (bin >> 26) & 0b1111;
            this->dInstr.opcode = opcode;
            // load base + offset
            if(opcode == 6){
                int dest = (bin >> 26) & 0b1111;
                int base = (bin >> 22) & 0b1111; // this will go in src1 field of instruction object (base is a register!)
                int offset = (bin >> 13) & 0b1111; // this will go in immediate field of instruction object

                if (pendRegs.r[dest] != 0 || pendRegs.r[base] != 0) {
                    return true;
                }

                this->dInstr.destv.push_back(intRegs.r[dest]);
                this->dInstr.src1v.push_back(intRegs.r[base]);

                pendRegs.r[dest]++;

                this->dInstr.immediate = offset;
                return false;
            }
            // store base + offset
            if(opcode == 7){
                int src1 = (bin >> 26) & 0b1111;
                int base = (bin >> 22) & 0b1111; // this will go in src2 field of instruction object (base is a register!)
                int offset = (bin >> 13) & 0b1111; // this will go in immediate field of instruction object

                if (pendRegs.r[src1] != 0 || pendRegs.r[base] != 0) {
                    return true;
                }

                this->dInstr.src1v.push_back(intRegs.r[src1]);
                this->dInstr.src2v.push_back(intRegs.r[base]);


                this->dInstr.immediate = offset;
                return false;
            }
            // LDI
            if(opcode == 8){
                int dest = (bin >> 22) & 0b1111;
                int immediate = bin & 0x3FFFFF; // 22 bits

                this->dInstr.destv.push_back(dest);
                this->dInstr.immediate = immediate;
                pendRegs.r[dest]++;
                return false;
            }
            // Halt
            if(opcode == 5){
                this->dInstr.halt_signal = true;
            }

            break;

        }

        default:
            return false;

    }
    return false;
}

void Pipeline::execute(){

    if(this->eInstr.is_stalled || this->eInstr.is_blocked || this->eInstr.bin_instr == -1 || this->eInstr.address == -2){
        return;
    }

    switch(this->eInstr.type_code){

        case 0: // Data Processing
            this->eInstr.result = ALU_helper(
                this->eInstr.opcode,
                this->eInstr.src1v[0],
                this->eInstr.src2v[0]  // D puts immediate into src2v[0] for immediate instructions
                );
            break;

        case 2: // Miscellaneous
            switch(this->eInstr.opcode){
                case 0b0000: // NOT
                    this->eInstr.result = ~this->eInstr.src1v[0];
                    break;
                case 0b0001: // LD
                case 0b0010: // STR
                    this->eInstr.result = this->eInstr.src1v[0];
                    break;
                case 0b0110: // LDB
                case 0b0111: // STRB
                    this->eInstr.result = this->eInstr.src1v[0] + this->eInstr.immediate;
                    break;
                case 0b1000: // LDI
                    this->eInstr.result = this->eInstr.immediate;
                    break;
                case 0b101: // HALT (do nothing)
                    break;
            }
            break;

        case 1: // Branch, E does nothing
            break;

    }
}

bool Pipeline::memory_access(){
    if(this->mInstr.is_stalled || this->mInstr.bin_instr == -1 || this->mInstr.bin_instr == -2){
        return false;
    }

    switch(this->mInstr.type_code){

        case 0: // ALU, do nothing
            break;

        case 2: // Miscellaneous
            switch(this->mInstr.opcode){
                case 0b0001: // LD
                case 0b0110:{ // LDB
                    // result holds the address, go fetch the value

                    std::string readValue = this->newCache->readMemory(this->mInstr.result, 4);
                    if (readValue.rfind("Done:", 0) == 0) { // starts with "Done:"
                        this->mInstr.result =static_cast<int>(stoul(readValue.substr(6)));
                        return false;
                    }
                    else{
                        this->newCache->clock++;
                        return true;
                    }
                }
                case 0b0010: // STR
                case 0b0111: { // STRB
                    std::string status = this->newCache->writeMemory(this->mInstr.result, this->mInstr.src1v[0], 4);
                    if (status.rfind("Done", 0) == 0) { // starts with "Done"
                        return false;
                    }
                    else{
                        this->newCache->clock++;
                        return true;
                    }

                    break;
                }
                case 0b0000: // NOT
                case 0b1000: // LDI
                    break;   // nothing to do
                case 0b0101: // HALT
                    break;
            }
            break;

        case 1: // Branch
            break;
        default:
            return false;
    }
    return false;
}

void Pipeline::write_back(){
    if(this->wInstr.is_stalled || this->wInstr.is_blocked || this->wInstr.bin_instr == -1 || this->wInstr.address == -2){
        return;
    }
    
    switch(this->wInstr.type_code){
        case 0: {  // ALU
            int opcode = this->wInstr.opcode;

            // CMP and CMPI update Condition Register instead of writing to a register
            if(opcode == 0b11100 || opcode == 0b11111){
                int result = this->wInstr.result;
                int N = (result < 0) ? 1 : 0;  // Negative
                int Z = (result == 0) ? 1 : 0; // Zero
                int V = (result > this->wInstr.src1v[0]) ? 1 : 0;  // Overflow
                intRegs.r[14] = (V << 2) | (Z << 1) | N;   // Order: 00000.....V Z B
                pendRegs.r[14]--;
                break;
            }

            // All other ALU ops write to dest register
            // Decrement pending
            intRegs.r[this->wInstr.destv[0]] = this->wInstr.result;
            pendRegs.r[this->wInstr.destv[0]]--;

            break;
        }

        case 2: {  // Miscellaneous
            int opcode = this->wInstr.opcode;
            if(opcode == 0b0010 || opcode == 0b0111 || opcode == 0b101){
                // STR / STRB, already written in M, nothing to do
                // HALT nothing to do
            }
            else{
                // LD, LDB, LDI, NOT, all write result to dest register
                intRegs.r[this->wInstr.destv[0]] = this->wInstr.result;
                pendRegs.r[this->wInstr.destv[0]]--;
            }
            break;
        }

        case 1: { // Branch, update PC R[13]
            // // Pending PC register decrement
            // int addr = this->wInstr.address;
            // int new_pc = branch_helper(addr);
            // intRegs.r[13] = new_pc;
            // pendRegs.r[13]--;
            
            // // Only squash if branch target is different from sequential next instruction
            // // Sequential next = addr + 1 (next instruction after branch) ==> Branching needed
            // if(new_pc != addr + 1){
            //     squashed = true;
            // }
            // break;

            int addr = this->wInstr.address;
            int new_pc = branch_helper(addr);
            
            std::cout << "DEBUG BRANCH: addr=" << addr 
                    << " new_pc=" << new_pc
                    << " current_pc=" << intRegs.r[13]
                    << " addr+1=" << (addr+1)
                    << " squash=" << (new_pc != addr+1) << std::endl;
            
            intRegs.r[13] = new_pc;
            pendRegs.r[13]--;
            
            if(new_pc != addr + 1){
                squashed = true;
                std::cout << "DEBUG: squash set!" << std::endl;
            }
            break;

        }
    }

    // For all non-branch instructions, advance PC by 1
    // if(this->wInstr.type_code != 1){
    //     intRegs.r[13]++;
    // }

}

// FIXME: Move to separate file if needed
int Pipeline::ALU_helper(int opcode, int a, int b) {

    switch(opcode){

        case 0b00000: return a + b;                                          // ADD
        case 0b00001: return a - b;                                          // SUB
        case 0b00010: return (b == 0) ? 0xFFFFFFFF : a / b;                  // DIV
        case 0b00011: return a * b;                                          // MUL
        case 0b00100: return (b == 0) ? a : a % b;                           // MOD
        case 0b00101: return a >> b;                                         // ASR
        case 0b00111: return a << b;                                         // ASL
        case 0b01000: return (unsigned int)a >> b;                           // LSR
        case 0b01001: return a << b;                                         // LSL
        case 0b01010: return a & b;                                          // AND
        case 0b01011: return a | b;                                          // OR
        case 0b01100: return a ^ b;                                          // XOR
        case 0b10000: return a + b;                                          // ADDI
        case 0b10001: return a - b;                                          // SUBI
        case 0b10010: return a * b;                                          // MULI
        case 0b11000: return (b == 0) ? 0xFFFFFFFF : a / b;                  // DIVI
        case 0b11001: return (b == 0) ? a : a % b;                           // MODI
        case 0b10011: return a >> b;                                         // ASRI
        case 0b10100: return a << b;                                         // ASLI
        case 0b11010: return (unsigned int)a >> b;                           // LSRI
        case 0b11011: return a << b;                                         // LSLI
        case 0b10101: return a & b;                                          // ANDI
        case 0b10110: return a | b;                                          // ORI
        case 0b10111: return a ^ b;                                          // XORI
        case 0b11100: return a - b;                                          // CMP
        case 0b11111: return a - b;                                          // CMPI
        default:      return 0;

    }
}

// Helper for branching, returns the next PC address
int Pipeline::branch_helper(int addr) {

    // BX, jumps to address stored in the register
    if(this->wInstr.opcode == 0b1000){
        return this->wInstr.src1v[0];
    }

    // BL is PC relative, need to update LR R[12]
    if(this->wInstr.opcode == 0b0111){
        intRegs.r[12] = intRegs.r[13] + 1; // return address PC + 1 stored in LR
        pendRegs.r[12]--; // Pending LR
        return addr + this->wInstr.branch_offset;
    }

    int CR = intRegs.r[14];
    int N = (CR >> 0) & 1;
    int Z = (CR >> 1) & 1;


    // Debug print 
    // std::cout << "DEBUG BRANCH_HELPER: CR=" << CR 
    //       << " N=" << N << " Z=" << Z << std::endl;


    // PC relative for everything except BX
    bool branch = false;
    switch(this->wInstr.opcode){
        case 0b0000: branch = true; break;                      // B
        case 0b0001: branch = (Z == 1); break;                  // BEQ
        case 0b0010: branch = (Z == 0); break;                  // BNE
        case 0b0011: branch = (N == 1); break;                  // BLT
        case 0b0100: branch = (N == 1 || Z == 1); break;        // BLE
        case 0b0101: branch = (N == 0 && Z == 0); break;        // BGT
        case 0b0110: branch = (N == 0 || Z == 1); break;        // BGE
    }

    if(branch){
        return addr + this->wInstr.branch_offset;
    }

    return intRegs.r[13];
}

void Pipeline::print_state(){
    std::cout << "\n========== CLOCK CYCLE " << this->global_clock << " ==========" << std::endl;

    // Pipeline stages
    std::cout << "\n--- PIPELINE ---" << std::endl;
    auto printInstr = [](const std::string& name, const InstructionObject& instr){
        std::cout << name << ": ";
        if(instr.is_stalled)      std::cout << "[BUBBLE]";
        else if(instr.is_blocked) std::cout << "[BLOCKED] opcode=" << instr.opcode << " type=" << instr.type_code;
        else                      std::cout << "opcode=" << instr.opcode << " type=" << instr.type_code << " bin=" << instr.bin_instr;
        std::cout << std::endl;
    };
    printInstr("F", this->fInstr);
    printInstr("D", this->dInstr);
    printInstr("E", this->eInstr);
    printInstr("M", this->mInstr);
    printInstr("W", this->wInstr);

    // Integer registers
    std::cout << "\n--- INTEGER REGISTERS ---" << std::endl;
    for(int i = 0; i < 16; i++){
        std::cout << "r" << i << "=" << intRegs.r[i] << " ";
        if((i+1) % 8 == 0) std::cout << std::endl;
    }

    // Pending registers
    std::cout << "\n--- PENDING REGISTERS ---" << std::endl;
    for(int i = 0; i < 16; i++){
        std::cout << "r" << i << "=" << pendRegs.r[i] << " ";
        if((i+1) % 8 == 0) std::cout << std::endl;
    }

    // Cache
    std::cout << std::endl;
    this->newCache->printCache();
}
