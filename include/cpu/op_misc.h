#ifndef OP_MISC_H
#define OP_MISC_H

#include "cpu.h"
#include "../memory.h"

// Does nothing but consume a cycle.
void op_nop(CPU *cpu, Memory *mem); // 0x00

// Stops the CPU and LCD until a button is pressed (very low power mode).
// NOTE: only the mandatory operand-byte skip is implemented (see op_stop
// in op_misc.c) - actually halting execution and the LCD is not.
void op_stop(CPU *cpu, Memory *mem); // 0x10

// Dispatches a CB-prefixed opcode. Returns that opcode's own cycle cost
// (see cpu_execute_cb_opcode) - the 0xCB prefix byte's cost is separate.
u8 op_prefix(CPU *cpu, Memory *mem); // 0xCB

// Sets ime_pending; IME itself turns on after the *next* instruction.
void op_ei(CPU *cpu, Memory *mem); // 0xFB

// Clears IME (and any still-pending EI) immediately.
void op_di(CPU *cpu, Memory *mem); // 0xF3

// Enters low-power mode until an interrupt occurs.
void op_halt(CPU *cpu, Memory *mem); // 0x76


#endif

