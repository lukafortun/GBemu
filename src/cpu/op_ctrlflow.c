#include "../../include/cpu/op_ctrlflow.h"
#include <stdbool.h>


/*
---------Jump instructions---------
*/

// Conditional jump to the relative address specified by the signed 8-bit operand e, depending on the condition cc.
void jr_cc_e(CPU *cpu, Memory *memory, bool cc){
  int8_t e = (int8_t)memory_read(memory, cpu->pc); cpu->pc++;
  if (cc){cpu->pc+=e;} 
};

void jr_nc_e(CPU * cpu, Memory *memory){jr_cc_e(cpu, memory, !((cpu->f & C_FLAG)) );}; // 0x30
void jr_nz_e(CPU * cpu, Memory *memory){jr_cc_e(cpu, memory, !((cpu->f & Z_FLAG)) );}; // 0x20
void jr_c_e(CPU * cpu, Memory *memory){jr_cc_e(cpu, memory, ((cpu->f & C_FLAG)) );}; // 0x38
void jr_z_e(CPU * cpu, Memory *memory){jr_cc_e(cpu, memory, ((cpu->f & Z_FLAG)) );}; // 0x28

// Conditional jump to the absolute address specified by the 16-bit operand nn, depending on the condition cc.
void jp_cc_nn(CPU *cpu, Memory *memory, bool cc){
  u8 nn_lsb = memory_read(memory, cpu->pc); cpu->pc++;
  u8 nn_msb = memory_read(memory, cpu->pc); cpu->pc++;
  u16 nn = ((u16)nn_lsb << 8) | nn_msb;
  if (cc){cpu->pc=nn;}  
};

void jp_nc_nn(CPU * cpu, Memory *memory){jp_cc_nn(cpu, memory, !((cpu->f & C_FLAG)) );}; // 0xD2
void jp_nz_nn(CPU * cpu, Memory *memory){jp_cc_nn(cpu, memory, !((cpu->f & Z_FLAG)) );}; // 0xC2
void jp_c_nn(CPU *cpu, Memory *memory){jp_cc_nn(cpu, memory, ((cpu->f & C_FLAG)) );}; // 0xDA
void jp_z_nn(CPU *cpu, Memory *memory){jp_cc_nn(cpu, memory, ((cpu->f & Z_FLAG)) );}; // 0xCA

// Unconditional jump to the absolute address specified by the 16-bit immediate operand nn.
void jp_nn(CPU *cpu, Memory *memory){jp_cc_nn(cpu, memory, true);}; // 0xC4

// Unconditional jump to the absolute address specified by the 16-bit register HL.
void jp_hl(CPU *cpu, Memory *memory){cpu->pc = cpu->hl;}; // 0xE9

// Unconditional jump to the relative address specified by the signed 8-bit operand e.
void jr_e(CPU *cpu, Memory *memory){jr_cc_e(cpu, memory, true);}; // 0x18 


/*
---------Function call instructions---------
*/

// Conditional function call to the absolute address specified by the 16-bit operand nn, depending on the condition cc.
void call_cc_nn(CPU *cpu, Memory *memory, bool cc){
  u8 nn_lsb = memory_read(memory, cpu->pc); cpu->pc++;
  u8 nn_msb = memory_read(memory, cpu->pc); cpu->pc++;
  u16 nn = ((u16)nn_lsb << 8) | nn_msb;
  if (cc){
    cpu->sp-=1;
    memory_write(memory, cpu->sp, (cpu->sp >> 8) & 0xFF); cpu->sp-=1;
    memory_write(memory, cpu->sp, cpu->sp & 0xFF);
    cpu->pc = nn;
  }
}


void call_nz_nn(CPU *cpu, Memory *memory){call_cc_nn(cpu, memory, !((cpu->f & C_FLAG)));}; // 0xC4
void call_nc_nn(CPU *cpu, Memory *memory){call_cc_nn(cpu, memory, !((cpu->f & Z_FLAG)));}; // 0xD4
void call_z_nn(CPU *cpu, Memory *memory){call_cc_nn(cpu, memory, ((cpu->f & C_FLAG)));}; // 0xCC
void call_c_nn(CPU *cpu, Memory *memory){call_cc_nn(cpu, memory, ((cpu->f & Z_FLAG)));}; // 0xDC


// Unconditional function call to the absolute address specified by the 16-bit operand nn.
void call_nn(CPU *cpu, Memory *memory){call_cc_nn(cpu, memory, true);}; // 0xDD

/*
---------Function return instructions---------
*/

// Conditional return from a function, depending on the condition cc.
void ret_cc(CPU *cpu, Memory *memory, bool cc){
  if (cc){
    u8 lsb = memory_read(memory, cpu->sp); cpu->sp++;
    u8 msb = memory_read(memory, cpu->sp); cpu->sp++;
    cpu->pc = ((u16)lsb << 8) | msb;
  }
}

void ret_nz(CPU *cpu, Memory *memory){ret_cc(cpu, memory, !(cpu->f & Z_FLAG));}; // 0xC0
void ret_nc(CPU *cpu, Memory *memory){ret_cc(cpu, memory, !(cpu->f & C_FLAG));}; // 0xD0
void ret_z(CPU *cpu, Memory *memory){ret_cc(cpu, memory, cpu->f & Z_FLAG);}; // 0xC8
void ret_c(CPU *cpu, Memory *memory){ret_cc(cpu, memory, cpu->f & C_FLAG);}; // 0xD8

// Unconditional return from a function.
void ret(CPU *cpu, Memory *memory){ret_cc(cpu, memory, true);}; // 0xC9


// Unconditional return from a function. Also enables interrupts by setting IME=1.
void reti(CPU *cpu, Memory *memory){
  ret_cc(cpu, memory, true);
  cpu->ime = true;
}; // 0xD9


// Unconditional function call to the absolute fixed address defined by the opcode.
void rst_n(CPU *cpu, Memory *memory, u8 n){
    cpu->sp-=1;
    memory_write(memory, cpu->sp, (cpu->sp >> 8) & 0xFF); cpu->sp-=1;
    memory_write(memory, cpu->sp, cpu->sp & 0xFF);
    cpu->pc = ((u16)n << 8) | 0x00;
}

void rst_0x00(CPU *cpu, Memory *memory){rst_n(cpu, memory, 0x00);}; // 0xC7
void rst_0x10(CPU *cpu, Memory *memory){rst_n(cpu, memory, 0x10);}; // 0xD7
void rst_0x20(CPU *cpu, Memory *memory){rst_n(cpu, memory, 0x20);}; // 0xE7
void rst_0x30(CPU *cpu, Memory *memory){rst_n(cpu, memory, 0x30);}; // 0xF7
void rst_0x08(CPU *cpu, Memory *memory){rst_n(cpu, memory, 0x08);}; // 0xCF
void rst_0x18(CPU *cpu, Memory *memory){rst_n(cpu, memory, 0x18);}; // 0xDF
void rst_0x28(CPU *cpu, Memory *memory){rst_n(cpu, memory, 0x28);}; // 0xEF
void rst_0x38(CPU *cpu, Memory *memory){rst_n(cpu, memory, 0x38);}; // 0xFF


