#ifndef OP_CB_H
#define OP_CB_H

#include "./cpu.h"
#include "../memory.h"

// Executes a single CB-prefixed opcode (rotate/shift/swap on 0x00-0x3F,
// BIT on 0x40-0x7F, RES on 0x80-0xBF, SET on 0xC0-0xFF).
// The low 3 bits of the opcode select the operand: B,C,D,E,H,L,(HL),A.
// Returns the T-state cost of this CB opcode alone (the 0xCB prefix byte's
// own 4-cycle fetch is accounted for separately by the caller).
u8 cpu_execute_cb_opcode(CPU *cpu, Memory *mem, u8 opcode);

#endif
