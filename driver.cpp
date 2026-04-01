#include "driver.h"
#include "pipeline.h"
#include "registers.h"
#include <string>

void single_clock_cycle(Pipeline* pipeline) {
    pipeline->print_state();
    extern Registers::IntegerRegs intRegs;

    (pipeline->global_clock)++;

    // 1. Execute logic for each stage (Internal state changes only)
    bool is_fwaiting = pipeline->fetch();
    bool is_dwaiting = pipeline->decode();
    pipeline->execute();
    bool is_mwaiting = pipeline->memory_access();
    pipeline->write_back();

    // 2. Movement Logic (Bottom-Up Shifting)

    // --- Move M to W ---
    pipeline->wInstr = pipeline->mInstr;

    // --- Move E to M ---
    if (is_mwaiting) {
        pipeline->wInstr = {.is_stalled = true}; // Send bubble to WB
        pipeline->mInstr.is_blocked = true;
    } else {
        pipeline->mInstr = pipeline->eInstr;
        pipeline->mInstr.is_blocked = false;
    }

    // --- Move D to E ---
    if (is_mwaiting || is_dwaiting) {
        if (!is_mwaiting) pipeline->eInstr = {.is_stalled = true}; // Send bubble to EX
        pipeline->eInstr.is_blocked = true;
    } else {
        pipeline->eInstr = pipeline->dInstr;
        pipeline->eInstr.is_blocked = false;
    }

    // --- Move F to D ---
    if (is_mwaiting || is_dwaiting || is_fwaiting) {
        if (!is_mwaiting && !is_dwaiting) pipeline->dInstr = {.is_stalled = true}; // Send bubble to ID
        pipeline->dInstr.is_blocked = true;
    } else {
        pipeline->dInstr = pipeline->fInstr;
        pipeline->dInstr.is_blocked = false;

        // CRITICAL: Increment PC only if F successfully moved to D
        intRegs.r[13]++;
    }

}


