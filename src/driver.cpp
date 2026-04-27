#include "driver.h"
#include "pipeline.h"
#include "registers.h"
#include <iostream>

extern Registers::IntegerRegs intRegs;
extern Registers::PendIntegerRegs pendRegs;
extern Registers::PendVectorRegs pendVectorRegs;

int curClockCycle = 0;
int pipelineStage = 1; // 1: fetch, 2: decode, 3: execute, 4: memory_access, 5: write_back_to_register

// Helper function to capture pipeline stage states before forwarding. This captured state is what the UI displays
void helper_snapshot(Pipeline* pipeline){
    pipeline->displayF = pipeline->fInstr;
    pipeline->displayD = pipeline->dInstr;
    pipeline->displayE = pipeline->eInstr;
    pipeline->displayM = pipeline->mInstr;
    pipeline->displayW = pipeline->wInstr;
}


void single_clock_cycle(Pipeline* pipeline, bool cacheEnabled, bool pipelineEnabled) {

    // FIXME
    std::cout << "PEND: r0=" << pendRegs.r[0]
              << " r1=" << pendRegs.r[1]
              << " r2=" << pendRegs.r[2] << std::endl;

    // Fetch --> Decode --> Execute --> Memory Access --> Write Back
    //extern Registers::IntegerRegs intRegs;
    if (pipeline -> global_clock == 0) { // Initialize first PC to 0
        intRegs.r[13] = 0;
    }

    curClockCycle = pipeline -> global_clock;
    (pipeline->global_clock)++;


    int pc_to_fetch = intRegs.r[13];// This is specifically used by fetch

    if (pipelineEnabled) {
        // Execute logic for each stage (Internal state changes only)
        std::cout << "Clock = " << curClockCycle << ", PC = " << intRegs.r[13] << std::endl;
        pipeline->write_back();
        bool is_mwaiting = pipeline->memory_access(cacheEnabled);
        pipeline->execute();
        bool is_dwaiting = pipeline->decode();
        bool is_fwaiting = pipeline->fetch(cacheEnabled, pc_to_fetch);
        std::cout << "Done with pipeline stages..." << std::endl;

        // Snapshot pipeline state here, match UI to debug print out
        helper_snapshot(pipeline);

        // pipeline->displayF = pipeline->fInstr;
        // pipeline->displayD = pipeline->dInstr;
        // pipeline->displayE = pipeline->eInstr;
        // pipeline->displayM = pipeline->mInstr;
        // pipeline->displayW = pipeline->wInstr;

        /************************DEBUG PRINTS***************************/
        // Print state of all instruction structs after stage execution
        auto printInstr = [](const std::string& name, const InstructionObject& instr){
            std::cout << name << ": ";
            if(instr.is_squashed){
                std::cout << "[SQUASHED]";
                if(instr.is_stalled == false){
                    std::cout << "bin=" << instr.bin_instr
                              << " type=" << instr.type_code
                              << " opcode=" << instr.opcode
                              << " imm=" << instr.immediate
                              << " result=" << instr.result;
                    if(std::size(instr.src1v) != 0) std::cout << " src1=" << instr.src1v[0];
                    if(std::size(instr.src2v) != 0) std::cout << " src2=" << instr.src2v[0];
                    if(std::size(instr.destv) != 0) std::cout << " dest=" << instr.destv[0];
                }
                else{
                    std::cout << "[BUBBLE]";
                }
            }
            else if(instr.is_stalled)       std::cout << "[BUBBLE]";
            else if(instr.is_blocked)  std::cout << "[BLOCKED]";
            else if(instr.bin_instr == -1) std::cout << "[EMPTY]";
            else {
                std::cout << "bin=" << instr.bin_instr
                          << " type=" << instr.type_code
                          << " opcode=" << instr.opcode
                          << " imm=" << instr.immediate
                          << " result=" << instr.result;
                if(std::size(instr.src1v) != 0) std::cout << " src1=" << instr.src1v[0];
                if(std::size(instr.src2v) != 0) std::cout << " src2=" << instr.src2v[0];
                if(std::size(instr.destv) != 0) std::cout << " dest=" << instr.destv[0];
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


        // After current cycle execution, set squash tags for next cycle
        if(pipeline->squashed){

            // Undo pending reg increments for squashed instructions still in D/E/M
            auto undoPending = [](InstructionObject& instr){
                if(instr.bin_instr == -1 || instr.is_stalled || instr.is_squashed || !instr.pend_incremented) return;

                if(instr.type_code == 0){
                    int op = instr.opcode;
                    if((op >= 0 && op <= 11) || (op >= 15 && op <= 26)){
                        pendRegs.r[instr.destv[0]]--;
                    }
                    if(op == 27 || op == 30) pendRegs.r[14]--;  // CMP
                }
                if(instr.type_code == 1){
                    pendRegs.r[13]--;  // PC
                    if(instr.opcode == 7) pendRegs.r[12]--;  // BL also sets LR
                }
                if(instr.type_code == 2){
                    int op = instr.opcode;
                    if(op == 0 || op == 1 || op == 6 || op == 8){  // NOT, LD, LDB, LDI
                        pendRegs.r[instr.destv[0]]--;
                    }
                    if(op == 3) pendVectorRegs.q[instr.destv[0]]--;  // VLD
                }
            };

            undoPending(pipeline->dInstr);
            undoPending(pipeline->eInstr);
            undoPending(pipeline->mInstr);


            pipeline->fInstr.is_squashed=true;

            pipeline->dInstr.is_squashed=true;
            pipeline->dInstr.is_blocked=false;

            pipeline->eInstr.is_squashed=true;
            pipeline->eInstr.is_blocked=false;

            pipeline->mInstr.is_squashed=true;
            pipeline->mInstr.is_blocked=false;

            // pipeline->wInstr.is_squashed=true;
            // pipeline->wInstr.is_blocked=false;

            pipeline->squashed = false;

        }



        // Set status and instruction struct for the next clock cycle

        // Assign blocks and stalls: blocks propagate backwards, stalls go forward
        // Forward instruction struct when possible

        // Before M handling
        std::cout << "PRE-M: mInstr.is_blocked=" << pipeline->mInstr.is_blocked
                  << " eInstr.is_blocked=" << pipeline->eInstr.is_blocked << std::endl;

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

        // Before E handling
        std::cout << "PRE-E: eInstr.is_blocked=" << pipeline->eInstr.is_blocked
                  << " mInstr.is_blocked=" << pipeline->mInstr.is_blocked << std::endl;

        // Handles stage E
        if(pipeline->eInstr.is_squashed){
            // if E is squashed then all other stages are squashed, forward squashed instruction
            pipeline->mInstr = pipeline->eInstr;
        }
        else if(pipeline->eInstr.is_blocked){
            // E can only become blocked from M, if E is blocked, send back block to D
            pipeline->dInstr.is_blocked = true;
            if (!pipeline->mInstr.is_blocked) {
                // send stall forward if M is not blocked
                pipeline->mInstr = InstructionObject{};
                pipeline->mInstr.is_stalled = true;
            }
        }
        else { // if E is not blocked and not squashed
            pipeline->dInstr.is_blocked = false; // Unblock D (D can block itself again if waiting)
            if (!pipeline->mInstr.is_blocked) { // M is not blocked, forward instruction struct
                pipeline->mInstr = pipeline->eInstr;
            }
        }
        // Before D handling
        std::cout << "PRE-D: eInstr.is_blocked=" << pipeline->eInstr.is_blocked
                  << " is_dwaiting=" << is_dwaiting << std::endl;

        // Handles stage D
        if(is_dwaiting) {
            pipeline->dInstr.is_blocked = true;
            pipeline->fInstr.is_blocked = true;
            if (!pipeline->eInstr.is_blocked) {//Forward stall if E unblocked
                pipeline->eInstr.is_stalled = true;
            }
        }
        else { // if D is not blocked, then E is not blocked, forward instruction struct THIS IS NOT NECESSARILY TRUE!
            pipeline->fInstr.is_blocked = false; // Unblock F (F can block itself again if waiting)
            if(!pipeline->eInstr.is_blocked){
                pipeline->dInstr.is_blocked = false;
                pipeline->eInstr = pipeline->dInstr;
            }

        }

        // Handles stage F
        if(is_fwaiting && !pipeline->fInstr.is_squashed) {
            pipeline->fInstr.is_blocked = true;
            // Check for blocking not waiting, D might not be waiting for anything but blocked by another stage
            if (!pipeline->dInstr.is_blocked) {
                pipeline->dInstr = InstructionObject{};
                pipeline->dInstr.is_stalled = true;
            }
        }
        else {
            pipeline->fInstr.is_blocked = false;
            if (!pipeline->dInstr.is_blocked && !pipeline->eInstr.is_blocked) {
                pipeline->dInstr = pipeline-> fInstr;
                // Only increment PC if we forwarded a real instruction
                if(pipeline->fInstr.bin_instr != -1 && !pipeline->fInstr.is_squashed){
                    intRegs.r[13]++;
                }
                // fInstr will be updated in the next fetch if successful
            }

        }

        // FIXME

        std::cout << "POST-FORWARD SQUASH FLAGS: F=" << pipeline->fInstr.is_squashed
                  << " D=" << pipeline->dInstr.is_squashed
                  << " E=" << pipeline->eInstr.is_squashed
                  << " M=" << pipeline->mInstr.is_squashed
                  << " W=" << pipeline->wInstr.is_squashed << std::endl;

    }

    else{ // If pipeline not enabled, go through instruction stage by stage

        switch (pipelineStage){
        // Corresonding stage will run and forward
        case 1: {// Fetch
            bool is_fwaiting = pipeline->fetch(cacheEnabled, pc_to_fetch);
            helper_snapshot(pipeline);
            // Forward
            if (is_fwaiting) {
                pipeline->fInstr.is_blocked = true;
                helper_snapshot(pipeline);
            }
            else {
                pipeline->fInstr.is_blocked = false;
                helper_snapshot(pipeline);
                pipeline -> dInstr = pipeline -> fInstr;
                pipeline -> fInstr = InstructionObject();
                pipelineStage++;
                if (pipelineStage > 5) pipelineStage = 1;
                intRegs.r[13]++;
            }
            break;
        }
        case 2: // Decode
            pipeline -> decode();
            helper_snapshot(pipeline);
            pipeline -> eInstr = pipeline -> dInstr;
            pipeline -> dInstr = InstructionObject();
            pipelineStage++;
            if (pipelineStage > 5) pipelineStage = 1;
            break;
        case 3: // Execute
            pipeline -> execute();
            helper_snapshot(pipeline);
            pipeline -> mInstr = pipeline -> eInstr;
            pipeline -> eInstr = InstructionObject();
            pipelineStage++;
            if (pipelineStage > 5) pipelineStage = 1;
            break;
        case 4: {// Memory Access
            bool is_mwaiting = pipeline->memory_access(cacheEnabled);
            if (is_mwaiting) {
                pipeline->mInstr.is_blocked = true;
                helper_snapshot(pipeline);
            }
            else {
                pipeline->mInstr.is_blocked = false;
                helper_snapshot(pipeline);
                pipeline -> wInstr = pipeline -> mInstr;
                pipeline -> mInstr = InstructionObject();
                pipelineStage++;
                if (pipelineStage > 5) pipelineStage = 1;
            }
            break;
        }
        case 5: // Write Back
            pipeline -> write_back();
            helper_snapshot(pipeline);
            pipeline -> wInstr = InstructionObject();
            pipelineStage++;
            if (pipelineStage > 5) pipelineStage = 1;
            break;
        default:
            std::cout << "invalid pipeline stage" << std::endl;
        }
    }
}
