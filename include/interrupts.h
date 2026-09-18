#ifndef INTERRUPTS_H
#define INTERRUPTS_H

#include "cpu/cpu.h"
#include "memory.h"

// Three separate things gate an interrupt, easy to mix up:
//   - IME (CPU.ime, not memory-mapped): the master on/off switch, toggled
//     by EI/DI/RETI - nothing fires at all while it's clear.
//   - IE (0xFFFF): which sources the program has *enabled*.
//   - IF (0xFF0F): which sources are currently *pending* - set by hardware
//     (interrupt_request) the instant the triggering event happens, cleared
//     by interrupt_step once it actually dispatches that source.
// A source only fires when its bit is set in both IE and IF, and IME=1.
#define IF_ADDR 0xFF0F
#define IE_ADDR 0xFFFF

#define INT_VBLANK   0x01
#define INT_LCD_STAT 0x02
#define INT_TIMER    0x04
#define INT_SERIAL   0x08
#define INT_JOYPAD   0x10

// Sets the corresponding bit in IF (0xFF0F) to request an interrupt.
void interrupt_request(Memory *mem, u8 flag);

// Services the highest-priority pending, enabled interrupt, if any: wakes
// the CPU from HALT, and if IME is set, pushes PC and jumps to the vector.
// Must be called once per fetch cycle, before fetching the next opcode.
// Returns 20 (the T-state cost of interrupt dispatch) if one was actually
// serviced (pushed + jumped), 0 otherwise.
u8 interrupt_step(CPU *cpu, Memory *mem);

#endif
