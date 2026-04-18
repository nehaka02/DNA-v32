#include <string>
#include <iostream>
#include <sstream>

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

Registers::VectorRegs vectorRegs;
Registers::PendVectorRegs pendVectorRegs;


bool Pipeline::fetch(bool cacheEnabled){
    int current_pc = intRegs.r[13];

    std::string readValue = this->newCache->readMemory(current_pc, 1, false, cacheEnabled);
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

    if(this->dInstr.is_squashed || this->dInstr.is_stalled || this->dInstr.bin_instr == -1 ||this->dInstr.address == -2){
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

            // scalar instruction (except CMP and immediate shifts)
            if(opcode >= 0 && opcode <= 11){
                // reminder these are all register numbers
                int dest = (bin >> 21) & 0b1111;
                int src1 = (bin >> 17) & 0b1111;
                int src2 = (bin >> 13) & 0b1111;
                if (pendRegs.r[dest] != 0 || pendRegs.r[src1] != 0 || pendRegs.r[src2] != 0) {
                    return true;
                }
                //this->dInstr.destv.push_back(dest);
                this->dInstr.destv[0] = dest;
                this->dInstr.src1v[0] = intRegs.r[src1];
                this->dInstr.src2v[0] = intRegs.r[src2];

                pendRegs.r[dest]++;
                return false;

            }
            // vector instructions
            if(opcode == 12 || opcode == 13 || opcode == 14){
                // TODO
                break;

            }
            // immediate ops (excluding CMPI)
            if(opcode >= 15 && opcode <= 26){
                int dest = (bin >> 21) & 0b1111;
                int src1 = (bin >> 17) & 0b1111;
                if (pendRegs.r[dest] != 0 || pendRegs.r[src1] != 0) {
                    return true;
                }
                this->dInstr.destv[0] = dest;
                this->dInstr.src1v[0] = intRegs.r[src1];

                pendRegs.r[dest]++;
                return false;
            }
            // compare (note compare got moved from opcode 28 --> 27!)
            if(opcode == 27){
                int src1 = (bin >> 21) & 0b1111;
                int src2 = (bin >> 17) & 0b1111;
                if (pendRegs.r[src1] != 0 || pendRegs.r[src2] != 0) {
                    return true;
                }
                this->dInstr.src1v[0] = intRegs.r[src1];
                this->dInstr.src2v[0] = intRegs.r[src2];

                pendRegs.r[14]++; // flags register
                return false;
            }
            // VEQ
            if(opcode == 28){
                // TODO
                break;

            }
            // VSUM
            if(opcode == 29){
                // TODO
                break;
            }
            // CMPI
            if(opcode == 30){
                int src1 = (bin >> 21) & 0b1111;
                int immediate = (bin & 0x1FFFFF);
                if(pendRegs.r[src1] != 0 ) {
                    return true;
                }
                this->dInstr.src1v[0] = intRegs.r[src1];
                this->dInstr.immediate = immediate;
                return false;
            }

            break;
        }

        // Branching
        case 1:{
            int opcode = (bin >> 26) & 0b1111;
            this->dInstr.opcode = opcode;

            //everything besides BX instruction
            if(opcode >= 0 && opcode <= 7){
                this->dInstr.branch_offset = bin & 0x3FFFFFF;
                return false;
            }
            if(opcode == 8) { // BX
                int src = (bin >> 22) & 0b1111;
                this->dInstr.src1v[0] = intRegs.r[src];
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

            // NOT, LD
            if(opcode == 0 || opcode == 1 ){
                int dest = (bin >> 22) & 0b1111;
                int src = (bin >> 18) & 0b1111;
                if (pendRegs.r[dest] != 0 || pendRegs.r[src] != 0) {
                    return true;
                }
                this->dInstr.destv[0] = dest; // only putting register number in destv
                this->dInstr.src1v[0] = intRegs.r[src];

                pendRegs.r[dest]++;
                return false;
            }

            // STR
            if(opcode == 2){
                int dest = (bin >> 22) & 0b1111;
                int src = (bin >> 18) & 0b1111;
                if (pendRegs.r[dest] != 0 || pendRegs.r[src] != 0) {
                    return true;
                }
                this->dInstr.destv[0] = intRegs.r[dest]; // dest contains memory address
                this->dInstr.src1v[0] = intRegs.r[src]; // src contains value to write to memory

                // nothing gets put in pending registers because we aren't writing to any registers

                return false;
            }

            // VLD
            // IMPORTANT: The src register contains the memory address of a vector, so it is an INT register! Due to register indirect addressing
            if(opcode == 3){
                int dest = (bin >> 22) & 0b1111;
                int src = (bin >> 18) & 0b1111;
                if (pendVectorRegs.q[dest] != 0 || pendRegs.r[src] != 0) {
                    return true;
                }
                this->dInstr.destv[0] = dest; // only putting register number in destv
                this->dInstr.src1v[0] = intRegs.r[src]; // putting the memory address of the source vector in src1v

                pendVectorRegs.q[dest]++;
                return false;

            }

            // VSTR
            if(opcode == 4){
                int dest = (bin >> 22) & 0b1111;
                int src = (bin >> 18) & 0b1111;
                if (pendRegs.r[dest] != 0 || pendVectorRegs.q[src] != 0) {
                    return true;
                }
                this->dInstr.destv[0] = intRegs.r[dest]; // dest contains memory address
                for(int i = 0; i < 4; i++){this->dInstr.src1v[i] = vectorRegs.q[src][i];} // src contains value to write to memory

                // nothing gets put in pending registers because we aren't writing to any registers

                return false;
            }

            // Halt
            if(opcode == 5){
                this->dInstr.halt_signal = true;
            }

            // load base + offset
            if(opcode == 6){
                int dest = (bin >> 22) & 0b1111;
                int base = (bin >> 18) & 0b1111; // this will go in src1 field of instruction object (base is a register!)
                int offset = (bin & 0x3FFFF); // this will go in immediate field of instruction object

                if (pendRegs.r[dest] != 0 || pendRegs.r[base] != 0) {
                    return true;
                }

                this->dInstr.destv[0] = dest;
                this->dInstr.src1v[0] = intRegs.r[base];
                this->dInstr.immediate = offset;

                pendRegs.r[dest]++;
                return false;
            }
            // store base + offset
            if(opcode == 7){
                int src1 = (bin >> 22) & 0b1111;
                int base = (bin >> 18) & 0b1111; // this will go in src2 field of instruction object (base is a register!)
                int offset = (bin & 0x3FFFF); // this will go in immediate field of instruction object

                if (pendRegs.r[src1] != 0 || pendRegs.r[base] != 0) {
                    return true;
                }

                this->dInstr.src1v[0] = intRegs.r[src1];
                this->dInstr.src2v[0] = intRegs.r[base];
                this->dInstr.immediate = offset;

                return false;
            }
            // LDI
            if(opcode == 8){
                int dest = (bin >> 22) & 0b1111;
                int immediate = bin & 0x3FFFFF; // 22 bits

                this->dInstr.destv[0] = dest;
                this->dInstr.immediate = immediate;

                pendRegs.r[dest]++;
                return false;
            }

            break;

        }

        default:
            return false;

    }
    return false;
}

void Pipeline::execute(){

    if(this->eInstr.is_squashed || this->eInstr.is_stalled || this->eInstr.is_blocked || this->eInstr.bin_instr == -1 || this->eInstr.address == -2){
        return;
    }

    int opcode = this->eInstr.opcode;

    switch(this->eInstr.type_code){
        case 0:{
            // scalars
            if(opcode >= 0 && opcode <= 11 || opcode == 27){
                this->eInstr.result[0] = ALU_helper(
                    this->eInstr.opcode,
                    this->eInstr.src1v[0],
                    this->eInstr.src2v[0]
                    );
            }
            // vector instructions
            if(opcode == 12 || opcode == 13 || opcode == 14){
                // TODO
                break;
            }
            // scalar immediates
            if(opcode >= 15 && opcode <= 26 || opcode == 30){
                this->eInstr.result[0] = ALU_helper(
                    this->eInstr.opcode,
                    this->eInstr.src1v[0],
                    this->eInstr.immediate
                    );
            }

            // VEQ and VSUM
            if(opcode == 28){
                // TODO
                break;
            }
            if(opcode == 29){
                // TODO
                break;
            }

            break;
        }

        case 1: // Branch, E does nothing
            break;

        case 2:{ // Miscellaneous
            switch(this->eInstr.opcode){
                case 0: // NOT
                    this->eInstr.result[0] = ~this->eInstr.src1v[0];
                    break;
                case 1: // LD
                    this->eInstr.result[0] = this->eInstr.src1v[0];
                case 2: // STR
                    // do nothing
                    break;
                case 3: // VLD
                    this->eInstr.result[0] = this->eInstr.src1v[0];
                    break;
                case 4: // VSTR
                    // do nothing
                    break;
                case 5: // HALT (do nothing)
                    break;
                case 6: // LDB
                    this->eInstr.result[0] = this->eInstr.src1v[0] + this->eInstr.immediate;
                case 7: // STRB
                    this->eInstr.result[0] = this->eInstr.src2v[0] + this->eInstr.immediate;
                    break;
                case 8: // LDI
                    this->eInstr.result[0] = this->eInstr.immediate;
                    break;
            }
            break;
        }

        default:
            break;

    }
}

bool Pipeline::memory_access(bool cacheEnabled){
    if(this->mInstr.is_squashed || this->mInstr.is_stalled || this->mInstr.bin_instr == -1 || this->mInstr.bin_instr == -2){
        return false;
    }

    switch(this->mInstr.type_code){
        case 0: // ALU, do nothing
            break;

        case 1: // Branch
            break;

        case 2: // Miscellaneous
            switch(this->mInstr.opcode){
                case 1:{ // LD
                    // result holds the address, go fetch the value
                    std::string readValue = this->newCache->readMemory(this->mInstr.result[0], 4, false, cacheEnabled);
                    if (readValue.rfind("Done:", 0) == 0) { // starts with "Done:"
                        this->mInstr.result[0] =static_cast<int>(stoul(readValue.substr(6)));
                        return false;
                    }
                    else{
                        this->newCache->clock++;
                        return true;
                    }

                    break;
                }
                case 6:{ // LDB
                    // result holds the address, go fetch the value
                    std::string readValue = this->newCache->readMemory(this->mInstr.result[0], 4, false, cacheEnabled);
                    if (readValue.rfind("Done:", 0) == 0) { // starts with "Done:"
                        this->mInstr.result[0] =static_cast<int>(stoul(readValue.substr(6)));
                        return false;
                    }
                    else{
                        this->newCache->clock++;
                        return true;
                    }

                    break;
                }
                case 2:{ // STR
                    std::string status = this->newCache->writeMemory(this->mInstr.destv[0], this->mInstr.src1v, 4, false, cacheEnabled);
                    if (status.rfind("Done", 0) == 0) { // starts with "Done"
                        return false;
                    }
                    else{
                        this->newCache->clock++;
                        return true;
                    }
                    break;
                }
                case 7: { // STRB
                    std::string status = this->newCache->writeMemory(this->mInstr.result[0], this->mInstr.src1v, 4, false, cacheEnabled);
                    if (status.rfind("Done", 0) == 0) { // starts with "Done"
                        return false;
                    }
                    else{
                        this->newCache->clock++;
                        return true;
                    }
                    break;
                }
                case 0: // NOT
                    break;
                case 8: // LDI
                    break;
                case 5: // HALT
                    break;
                // Enforced 4-word alignment for vector loads and stores!
                case 4:{ // VSTR
                    std::string status = this->newCache->writeMemory((this->mInstr.destv[0])%4, this->mInstr.src1v, 4, true, cacheEnabled);
                    if (status.rfind("Done", 0) == 0) { // starts with "Done"
                        return false;
                    }
                    else{
                        this->newCache->clock++;
                        return true;
                    }
                    break;
                }
                case 3:{ // VLD
                    // result holds the address, go fetch the value
                    std::string readValue = this->newCache->readMemory((this->mInstr.result[0])%4, 4, true, cacheEnabled);

                    if (readValue.rfind("Done:", 0) == 0) {

                        // Remove "Done: "
                        std::string values = readValue.substr(6);

                        std::stringstream ss(values);
                        std::string token;
                        int i = 0;

                        while (std::getline(ss, token, ',') && i < 4) {
                            // Remove leading space if present
                            if (!token.empty() && token[0] == ' ') {
                                token = token.substr(1);
                            }

                            this->mInstr.result[i++] = std::stoi(token);
                        }

                        // Now result contains vector
                        return false;
                    }
                    else{
                        this->newCache->clock++;
                        return true;
                    }
                }
            }
            break;

        default:
            return false;
    }
    return false;
}

void Pipeline::write_back(){
    if(this->wInstr.is_squashed || this->wInstr.is_stalled || this->wInstr.is_blocked || this->wInstr.bin_instr == -1 || this->wInstr.address == -2){
        return;
    }
    
    switch(this->wInstr.type_code){
        case 0: {  // ALU
            int opcode = this->wInstr.opcode;

            // CMP and CMPI update Condition Register instead of writing to a register
            if(opcode == 27 || opcode == 30){
                int result = this->wInstr.result[0];
                int N = (result < 0) ? 1 : 0;  // Negative
                int Z = (result == 0) ? 1 : 0; // Zero
                int V = (result > this->wInstr.src1v[0]) ? 1 : 0;  // Overflow
                intRegs.r[14] = (V << 2) | (Z << 1) | N;   // Order: 00000.....V Z B
                pendRegs.r[14]--;
                break;
            }

            // All other ALU ops write to dest register
            // Decrement pending
            if((opcode >= 0 && opcode <= 11) || (opcode >= 15 && opcode <= 26)){
                intRegs.r[this->wInstr.destv[0]] = this->wInstr.result[0];
                pendRegs.r[this->wInstr.destv[0]]--;
                break;
            }

            if(opcode == 12 || opcode == 13 || opcode == 14){
                // TODO
                break;

            }
            if(opcode == 28){
                // TODO
                break;

            }
            // VSUM
            if(opcode == 29){
                // TODO
                break;
            }

            break;
        }

        case 2: {  // Miscellaneous
            int opcode = this->wInstr.opcode;
            if(opcode == 2 || opcode == 5 || opcode == 7 || opcode == 4){
                // STR / STRB, already written in M, nothing to do
                // HALT nothing to do
                // VSTR nothing to do
            }
            // VLD
            else if(opcode == 3){
                // TODO
                for(int i = 0; i < 4; i++){vectorRegs.q[this->wInstr.destv[0]][i] = this->wInstr.result[i];}
                pendVectorRegs.q[this->wInstr.destv[0]]--;
                break;
            }
            else{
                // LD, LDB, LDI, NOT all write result to dest register
                intRegs.r[this->wInstr.destv[0]] = this->wInstr.result[0];
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

        default:
            break;
    }

    // For all non-branch instructions, advance PC by 1
    // if(this->wInstr.type_code != 1){
    //     intRegs.r[13]++;
    // }

}

// FIXME: Move to separate file if needed
int Pipeline::ALU_helper(int opcode, int a, int b) {

    switch(opcode){

        case 0: return a + b;                                          // ADD
        case 1: return a - b;                                          // SUB
        case 2: return (b == 0) ? 0xFFFFFFFF : a / b;                  // DIV
        case 3: return a * b;                                          // MUL
        case 4: return (b == 0) ? a : a % b;                           // MOD
        case 5: return a >> b;                                         // ASR
        case 6: return a << b;                                         // ASL
        case 7: return (unsigned int)a >> b;                           // LSR
        case 8: return a << b;                                         // LSL
        case 9: return a & b;                                          // AND
        case 10: return a | b;                                          // OR
        case 11: return a ^ b;                                          // XOR
        case 15: return a + b;                                          // ADDI
        case 16: return a - b;                                          // SUBI
        case 17: return a * b;                                          // MULI
        case 23: return (b == 0) ? 0xFFFFFFFF : a / b;                  // DIVI
        case 24: return (b == 0) ? a : a % b;                           // MODI
        case 18: return a >> b;                                         // ASRI
        case 19: return a << b;                                         // ASLI
        case 25: return (unsigned int)a >> b;                           // LSRI
        case 26: return a << b;                                         // LSLI
        case 20: return a & b;                                          // ANDI
        case 21: return a | b;                                          // ORI
        case 22: return a ^ b;                                          // XORI
        case 27: return a - b;                                          // CMP
        case 30: return a - b;                                          // CMPI
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
