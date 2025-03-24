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

#endif
