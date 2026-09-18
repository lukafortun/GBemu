#ifndef CPU_H
#define CPU_H

#include "../common.h"
#include "../memory.h"
#include <stdbool.h>

#define Z_FLAG 0x80  // Bit 7
#define N_FLAG 0x40  // Bit 6
#define H_FLAG 0x20  // Bit 5
#define C_FLAG 0x10  // Bit 4

// CPU registers.
//
// Each pair (AF, BC, DE, HL) is really a single 16-bit register that the
// real hardware also lets you address as two independent 8-bit halves -
// e.g. INC B only touches the top byte of BC. The anonymous union/struct
// trick reproduces that exactly: `cpu.bc` and the pair `{cpu.c, cpu.b}`
// alias the same two bytes, so writing one is instantly visible through
// the other with no copying. Struct member order matters here - on this
// little-endian target, the first-declared field is the low byte, so `c`
// (low) must come before `b` (high) for `bc` to read as expected.
typedef struct {
    union {
        struct {
            u8 f; // Flags register (low byte of AF)
            u8 a; // Accumulator (high byte of AF)
        };
        u16 af;
    };

    union {
        struct {
            u8 c;
            u8 b;
        };
        u16 bc;
    };

    union {
        struct {
            u8 e;
            u8 d;
        };
        u16 de;
    };

    union {
        struct {
            u8 l;
            u8 h;
        };
        u16 hl;
    };

    u16 sp; // Stack pointer
    u16 pc; // Program counter
  
    bool ime;          // Interrupt Master Enable
    bool ime_pending;  // EI takes effect after the instruction following it
    bool halted;       // Set by HALT, cleared when an interrupt wakes the CPU
    // IE (0xFFFF) and IF (0xFF0F) are memory-mapped registers, not CPU state
    // - see include/interrupts.h.

} CPU;

// Fetches and executes one instruction, servicing pending interrupts first.
// Returns the number of T-states this step actually took (interrupt
// dispatch, a halted idle tick, or the executed opcode's own cost).
u8 fetch_cycle(CPU *cpu, Memory *mem);

#endif
