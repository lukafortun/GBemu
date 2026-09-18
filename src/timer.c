#include "../include/timer.h"
#include "../include/interrupts.h"

// Both counters are internal implementation state, not memory - the game
// only ever sees them indirectly through the DIV/TIMA registers this file
// keeps in sync. div_counter is the real 16-bit hardware counter; DIV is
// just its top 8 bits. tima_counter tracks how many T-states have
// accumulated toward the *next* TIMA tick, whose size depends on TAC.
static u16 div_counter = 0;
static u16 tima_counter = 0;

// How many T-states TIMA takes to advance by 1, per the TAC frequency
// selection (bits 0-1). Bit 2 of TAC (checked separately in timer_step)
// enables/disables TIMA entirely; DIV always runs regardless.
static int tac_cycles_per_tick(u8 tac){
  switch (tac & 0x03) {
    case 0: return 1024; // 4096 Hz
    case 1: return 16;   // 262144 Hz
    case 2: return 64;   // 65536 Hz
    default: return 256; // 16384 Hz
  }
}

// Advances DIV and (if enabled) TIMA by `cycles` T-states, requesting a
// timer interrupt whenever TIMA wraps past 0xFF.
void timer_step(Memory *mem, int cycles){
  div_counter = (u16)(div_counter + cycles);
  memory_write(mem, DIV_ADDR, (u8)(div_counter >> 8));

  u8 tac = memory_read(mem, TAC_ADDR);
  if (!(tac & 0x04)) return; // TIMA disabled (DIV above still ran)

  int threshold = tac_cycles_per_tick(tac);
  tima_counter = (u16)(tima_counter + cycles);

  // A `while`, not `if`: a single call can carry enough cycles (e.g. a
  // 24-cycle CALL at the fastest TAC setting) to cross more than one TIMA
  // threshold, so we drain every tick it's actually owed.
  while (tima_counter >= (u16)threshold) {
    tima_counter = (u16)(tima_counter - threshold);
    u8 tima = memory_read(mem, TIMA_ADDR);
    if (tima == 0xFF) {
      // Overflow: reload from TMA (not 0) and let the game know via the
      // timer interrupt - this is what lets TIMA count at any period, not
      // just powers of two.
      memory_write(mem, TIMA_ADDR, memory_read(mem, TMA_ADDR));
      interrupt_request(mem, INT_TIMER);
    } else {
      memory_write(mem, TIMA_ADDR, (u8)(tima + 1));
    }
  }
}
