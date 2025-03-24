#ifndef OP_LOAD_H 
#define OP_LOAD_H

#include "./cpu.h"
#include "../memory.h"


/*
---------Jump instructions---------
*/

// Conditional jump to the relative address specified by the signed 8-bit operand e, depending on the condition cc.
void jr_nc_e(CPU * cpu, Memory *memory); // 0x30
void jr_nz_e(CPU * cpu, Memory *memory); // 0x20
void jr_c_e(CPU * cpu, Memory *memory); // 0x38
void jr_z_e(CPU * cpu, Memory *memory); // 0x28

// Conditional jump to the absolute address specified by the 16-bit operand nn, depending on the condition cc.
void jp_nc_nn(CPU * cpu, Memory *memory); // 0xD2
void jp_nz_nn(CPU * cpu, Memory *memory); // 0xC2
void jp_c_nn(CPU *cpu, Memory *memory); // 0xDA
void jp_z_nn(CPU *cpu, Memory *memory); // 0xCA

// Unconditional jump to the absolute address specified by the 16-bit immediate operand nn.
void jp_nn(CPU *cpu, Memory *memory); // 0xC4

// Unconditional jump to the absolute address specified by the 16-bit register HL.
void jp_hl(CPU *cpu, Memory *memory); // 0xE9

// Unconditional jump to the relative address specified by the signed 8-bit operand e.
void jr_e(CPU *cpu, Memory *memory); // 0x18 


/*
---------Function call instructions---------
*/

// Conditional function call to the absolute address specified by the 16-bit operand nn, depending on the condition cc.
void call_nz_nn(CPU *cpu, Memory *memory); // 0xC4
void call_nc_nn(CPU *cpu, Memory *memory); // 0xD4
void call_z_nn(CPU *cpu, Memory *memory); // 0xCC
void call_c_nn(CPU *cpu, Memory *memory); // 0xDC


// Unconditional function call to the absolute address specified by the 16-bit operand nn.
void call_nn(CPU *cpu, Memory *memory); // 0xDD


/*
---------Function return instructions---------
*/

// Conditional return from a function, depending on the condition cc.
void ret_nz(CPU *cpu, Memory *memory); // 0xC0
void ret_nc(CPU *cpu, Memory *memory); // 0xD0
void ret_z(CPU *cpu, Memory *memory); // 0xC8
void ret_z(CPU *cpu, Memory *memory); // 0xD8

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
