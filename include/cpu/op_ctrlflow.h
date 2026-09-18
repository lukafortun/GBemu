#ifndef OP_CTRLFLOW_H
#define OP_CTRLFLOW_H

#include "./cpu.h"
#include "../memory.h"


/*
---------Jump instructions---------
*/

// Conditional jump to the relative address specified by the signed 8-bit
// operand e, depending on the condition cc. Returns whether it branched
// (the two cases cost a different number of cycles).
bool jr_nc_e(CPU * cpu, Memory *memory); // 0x30
bool jr_nz_e(CPU * cpu, Memory *memory); // 0x20
bool jr_c_e(CPU * cpu, Memory *memory); // 0x38
bool jr_z_e(CPU * cpu, Memory *memory); // 0x28

// Conditional jump to the absolute address specified by the 16-bit operand
// nn, depending on the condition cc. Returns whether it branched.
bool jp_nc_nn(CPU * cpu, Memory *memory); // 0xD2
bool jp_nz_nn(CPU * cpu, Memory *memory); // 0xC2
bool jp_c_nn(CPU *cpu, Memory *memory); // 0xDA
bool jp_z_nn(CPU *cpu, Memory *memory); // 0xCA

// Unconditional jump to the absolute address specified by the 16-bit immediate operand nn.
void jp_nn(CPU *cpu, Memory *memory); // 0xC3

// Unconditional jump to the absolute address specified by the 16-bit register HL.
void jp_hl(CPU *cpu, Memory *memory); // 0xE9

// Unconditional jump to the relative address specified by the signed 8-bit operand e.
void jr_e(CPU *cpu, Memory *memory); // 0x18 


/*
---------Function call instructions---------
*/

// Conditional function call to the absolute address specified by the 16-bit
// operand nn, depending on the condition cc. Returns whether it branched.
bool call_nz_nn(CPU *cpu, Memory *memory); // 0xC4
bool call_nc_nn(CPU *cpu, Memory *memory); // 0xD4
bool call_z_nn(CPU *cpu, Memory *memory); // 0xCC
bool call_c_nn(CPU *cpu, Memory *memory); // 0xDC


// Unconditional function call to the absolute address specified by the 16-bit operand nn.
void call_nn(CPU *cpu, Memory *memory); // 0xCD


/*
---------Function return instructions---------
*/

// Conditional return from a function, depending on the condition cc.
// Returns whether it branched.
bool ret_nz(CPU *cpu, Memory *memory); // 0xC0
bool ret_nc(CPU *cpu, Memory *memory); // 0xD0
bool ret_z(CPU *cpu, Memory *memory); // 0xC8
bool ret_c(CPU *cpu, Memory *memory); // 0xD8

// Unconditional return from a function.
void ret(CPU *cpu, Memory *memory); // 0xC9


// Unconditional return from a function. Also enables interrupts by setting IME=1.
void reti(CPU *cpu, Memory *memory); // 0xD9


// Unconditional function call to the absolute fixed address defined by the opcode.
void rst_0x00(CPU *cpu, Memory *memory); // 0xC7
void rst_0x10(CPU *cpu, Memory *memory); // 0xD7
void rst_0x20(CPU *cpu, Memory *memory); // 0xE7
void rst_0x30(CPU *cpu, Memory *memory); // 0xF7
void rst_0x08(CPU *cpu, Memory *memory); // 0xCF
void rst_0x18(CPU *cpu, Memory *memory); // 0xDF
void rst_0x28(CPU *cpu, Memory *memory); // 0xEF
void rst_0x38(CPU *cpu, Memory *memory); // 0xFF


#endif
