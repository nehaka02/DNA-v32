#include "driver.h"
#include "pipeline.h"
#include "registers.h"
#include <iostream>

extern Registers::IntegerRegs intRegs;
int curClockCycle = 0;

void single_clock_cycle(Pipeline* pipeline) {
    // Fetch --> Decode --> Execute --> Memory Access --> Write Back
    //extern Registers::IntegerRegs intRegs;
    if (pipeline -> global_clock == 0) { // Initialize first PC to 0
        intRegs.r[13] = 0;
    }

    curClockCycle = pipeline -> global_clock;
    (pipeline->global_clock)++;

    // Execute logic for each stage (Internal state changes only)
    // pipeline->write_back();
    // bool is_mwaiting = pipeline->memory_access();
    // pipeline->execute();
    // bool is_dwaiting = pipeline->decode();
    // bool is_fwaiting = pipeline->fetch();
    std::cout << "Clock = " << curClockCycle << ", PC = " << intRegs.r[13] << std::endl;
    std::cout << "Running W..." << std::endl;
    pipeline->write_back();
    std::cout << "Running M..." << std::endl;
    bool is_mwaiting = pipeline->memory_access();
    std::cout << "Running E..." << std::endl;
    pipeline->execute();
    std::cout << "Running D..." << std::endl;
    bool is_dwaiting = pipeline->decode();
    std::cout << "Running F..." << std::endl;
    bool is_fwaiting = pipeline->fetch();
    std::cout << "Done with stages..." << std::endl;


    /************************DEBUG PRINTS***************************/
    // Print state of all instruction structs after stage execution
    auto printInstr = [](const std::string& name, const InstructionObject& instr){
        std::cout << name << ": ";
        if(instr.is_stalled)       std::cout << "[BUBBLE]";
        else if(instr.is_blocked)  std::cout << "[BLOCKED]";
        else if(instr.bin_instr == -1) std::cout << "[EMPTY]";
        else {
            std::cout << "bin=" << instr.bin_instr
                      << " type=" << instr.type_code
                      << " opcode=" << instr.opcode
                      << " imm=" << instr.immediate
                      << " result=" << instr.result;
            if(!instr.src1v.empty()) std::cout << " src1=" << instr.src1v[0];
            if(!instr.src2v.empty()) std::cout << " src2=" << instr.src2v[0];
            if(!instr.destv.empty()) std::cout << " dest=" << instr.destv[0];
        }
        std::cout << std::endl;
    };
    std::cout << "\n--- AFTER STAGE EXECUTION ---" << std::endl;
    printInstr("F", pipeline->fInstr);
    printInstr("D", pipeline->dInstr);
    printInstr("E", pipeline->eInstr);
    printInstr("M", pipeline->mInstr);
    printInstr("W", pipeline->wInstr);
    std::cout << "is_fwaiting=" << is_fwaiting
              << " is_dwaiting=" << is_dwaiting
              << " is_mwaiting=" << is_mwaiting << std::endl;
    std::cout << "\n" << std::endl;

    /************************DEBUG PRINTS***************************/

    // Set status and instruction struct for the next clock cycle

    // Assign blocks and stalls: blocks propagate backwards, stalls go forward
    // Forward instruction struct when possible

    // Handle squash after write_back
    if(pipeline->squashed){
        pipeline->fInstr = InstructionObject{};
        pipeline->fInstr.bin_instr = -1;
        //pipeline->fInstr.is_stalled = true;
        pipeline->dInstr = InstructionObject{};
        //pipeline->dInstr.is_stalled = true;
        pipeline->eInstr = InstructionObject{};
        //pipeline->eInstr.is_stalled = true;
        pipeline->mInstr = InstructionObject{};
        //pipeline->mInstr.is_stalled = true;
        pipeline->wInstr = InstructionObject{};  // ADD THIS
        //pipeline->wInstr.is_stalled = true;
        pipeline->squashed = false;
    }
    else {
        // Handles stage M
        if (is_mwaiting) {
            pipeline->mInstr.is_blocked = true;
            pipeline->eInstr.is_blocked = true;
            if (!pipeline->wInstr.is_blocked) {
                pipeline->wInstr = InstructionObject{};
                pipeline->wInstr.is_stalled = true;
            }
        }
        else { // M pass data on, W is never blocked
            pipeline->mInstr.is_blocked = false;
            pipeline->eInstr.is_blocked = false;
            pipeline->wInstr = pipeline->mInstr;
        }

        // Handles stage E
        if(pipeline->eInstr.is_blocked){ 
            // E can only become blocked from M, if E is blocked, send back block to D
            pipeline->dInstr.is_blocked = true;
            if (!pipeline->mInstr.is_blocked) {
                // send stall forward if M is not blocked
                pipeline->mInstr = InstructionObject{};
                pipeline->mInstr.is_stalled = true;
            }
        }
        else { // if E is not blocked 
            pipeline->dInstr.is_blocked = false; // Unblock D (D can block itself again if waiting)
            if (!pipeline->mInstr.is_blocked) { // M is not blocked, forward instruction struct
                pipeline->mInstr = pipeline->eInstr;
                pipeline->dInstr.is_blocked = false;
            }
        }

        // Handles stage D 
        if(is_dwaiting) {
            pipeline->dInstr.is_blocked = true;
            pipeline->fInstr.is_blocked = true; 
            if (!pipeline->eInstr.is_blocked) {//Forward stall if E unblocked
                pipeline->eInstr.is_stalled = true;
            }
        }
        else { // if D is not blocked, then E is not blocked, forward instruction struct
            pipeline->fInstr.is_blocked = false; // Unblock F (F can block itself again if waiting)
            pipeline->dInstr.is_blocked = false;
            pipeline->eInstr = pipeline->dInstr; 
        }

        // Handles stage F 
        if(is_fwaiting) {
            pipeline->fInstr.is_blocked = true;
            // Check for blocking not waiting, D might not be waiting for anything but blocked by another stage
            if (!pipeline->dInstr.is_blocked) {
                pipeline->dInstr.is_stalled = true;
            }
        }
        else {
            pipeline->fInstr.is_blocked = false;
            if (!pipeline->dInstr.is_blocked) {
                pipeline->dInstr = pipeline-> fInstr;
                // Only increment PC if we forwarded a real instruction
                if(pipeline->fInstr.bin_instr != -1){
                    intRegs.r[13]++;
                }
                // fInstr will be updated in the next fetch if successful 
            }

        }
    }
}



