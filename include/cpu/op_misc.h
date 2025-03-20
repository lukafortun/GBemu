#ifndef OP_MISC_H
#define OP_MISC_H

#include "cpu.h"
#include "../memory.h"

// 0x00
void op_nop(CPU *cpu, Memory *mem);
// 0x10
void op_stop(CPU *cpu, Memory *mem);
// 0xCB
void op_prefix(CPU *cpu, Memory *mem);
// 0xFB
void op_ei(CPU *cpu, Memory *mem);
// 0xF3
void op_di(CPU *cpu, Memory *mem);
// 0x76
void op_halt(CPU *cpu, Memory *mem);


#endif

