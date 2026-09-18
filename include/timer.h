#ifndef TIMER_H
#define TIMER_H

#include "common.h"
#include "memory.h"

// DIV free-runs forever and is visible to the program only as the upper
// byte of a hidden 16-bit counter (see timer.c) - that's why it appears to
// tick much slower than the CPU clock. TIMA increments at a rate chosen by
// TAC and, unlike DIV, reloads from TMA (instead of wrapping to 0) and
// requests an interrupt whenever it overflows past 0xFF.
#define DIV_ADDR  0xFF04
#define TIMA_ADDR 0xFF05
#define TMA_ADDR  0xFF06
#define TAC_ADDR  0xFF07

// Advances DIV/TIMA by `cycles` T-states and requests a timer interrupt on
// TIMA overflow.
void timer_step(Memory *mem, int cycles);

#endif
