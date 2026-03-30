#include "pipeline.h"
#include <string>

Pipeline *pipeline = new Pipeline();

void single_clock_cycle(std::string instructionAddr){

    bool is_fwaiting = pipeline->fetch(instructionAddr);
    bool is_dwaiting = pipeline->decode();
    pipeline->execute();
    bool is_mwaiting = pipeline->memory_access();
    pipeline->write_back();

    if(is_mwaiting){
        if(!pipeline->wInstr.is_blocked){
            pipeline->wInstr = {.is_stalled = true};
        }
        return;
    }
    else if(is_dwaiting){
        if(!pipeline->eInstr.is_blocked){
            pipeline->eInstr = {.is_stalled = true};
        }
        pipeline->wInstr = pipeline->mInstr;
        pipeline->mInstr = pipeline->eInstr;

        return;
    }
    else if(is_fwaiting){
        if(!pipeline->dInstr.is_blocked){
            pipeline->dInstr = {.is_stalled = true};
        }
        pipeline->eInstr = pipeline->dInstr;
        pipeline->mInstr = pipeline->eInstr;
        pipeline->wInstr = pipeline->mInstr;
        return;
    }
    else{
        pipeline->dInstr = pipeline->fInstr;
        pipeline->eInstr = pipeline->dInstr;
        pipeline->mInstr = pipeline->eInstr;
        pipeline->wInstr = pipeline->mInstr;
        return;
    }


    (pipeline->global_clock)++;

}


