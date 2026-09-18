#ifndef OP_ROT_SHIFT_H
#define OP_ROT_SHIFT_H

#include "./cpu.h"
#include "../memory.h"

// Rotates the 8-bit A register left (bit 7 wraps into both the carry flag and bit 0).
void rlca(CPU *cpu, Memory *memory); // 0x07

// Rotates the 8-bit A register right (bit 0 wraps into both the carry flag and bit 7).
void rrca(CPU *cpu, Memory *memory); // 0x0F

// Rotates the 8-bit A register left, through the carry flag.
void rla(CPU *cpu, Memory *memory); // 0x17

// Rotates the 8-bit A register right, through the carry flag.
void rra(CPU *cpu, Memory *memory); // 0x1F

#endif
