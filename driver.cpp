#include"driver.h"
#include <string>
#include "pipeline.h"
#include "registers.h"


// Note: No instructionAddr string needed anymore if fetching via PC
void single_clock_cycle(Pipeline* pipeline) {

    extern Registers::IntegerRegs intRegs;

    if(pipeline->global_clock == 0){
        intRegs.r[13] = 0;
    }


    (pipeline->global_clock)++;

    // Get PC from your register file (e.g., r13)
    // extern Registers::IntegerRegs intRegs; // If defined in pipeline.cpp


    // Stage execution
    bool is_fwaiting = pipeline->fetch();
    bool is_dwaiting = pipeline->decode();
    pipeline->execute();
    bool is_mwaiting = pipeline->memory_access();
    pipeline->write_back();

    if(is_mwaiting){
        if(!pipeline->wInstr.is_blocked)
            pipeline->wInstr = {.is_stalled = true};
        pipeline->mInstr.is_blocked = true;
        pipeline->eInstr.is_blocked = true;
        pipeline->dInstr.is_blocked = true;
        pipeline->fInstr.is_blocked = true;
    }

    if(is_dwaiting){
        if(!pipeline->wInstr.is_blocked)
            pipeline->wInstr = pipeline->mInstr;
        if(!pipeline->mInstr.is_blocked)
            pipeline->mInstr = pipeline->eInstr;
        if(!pipeline->eInstr.is_blocked)
            pipeline->eInstr = {.is_stalled = true};
        pipeline->dInstr.is_blocked = true;
        pipeline->fInstr.is_blocked = true;
    }

    if(is_fwaiting){
        if(!pipeline->wInstr.is_blocked)
            pipeline->wInstr = pipeline->mInstr;
        if(!pipeline->mInstr.is_blocked)
            pipeline->mInstr = pipeline->eInstr;
        if(!pipeline->eInstr.is_blocked)
            pipeline->eInstr = pipeline->dInstr;
        if(!pipeline->dInstr.is_blocked)
            pipeline->dInstr = {.is_stalled = true};
        pipeline->fInstr.is_blocked = true;
    }

    if(!is_mwaiting && !is_dwaiting && !is_fwaiting){
        pipeline->fInstr.is_blocked = false;
        pipeline->dInstr.is_blocked = false;
        pipeline->eInstr.is_blocked = false;
        pipeline->mInstr.is_blocked = false;
        pipeline->wInstr.is_blocked = false;
        if(!pipeline->wInstr.is_blocked)
            pipeline->wInstr = pipeline->mInstr;
        if(!pipeline->mInstr.is_blocked)
            pipeline->mInstr = pipeline->eInstr;
        if(!pipeline->eInstr.is_blocked)
            pipeline->eInstr = pipeline->dInstr;
        if(!pipeline->dInstr.is_blocked)
            pipeline->dInstr = pipeline->fInstr;
        // clear all blocks since pipeline is flowing freely

    }
     pipeline->print_state();
}


