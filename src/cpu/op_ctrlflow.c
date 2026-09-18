#include "../../include/cpu/op_ctrlflow.h"
#include <stdbool.h>


/*
---------Jump instructions---------
*/

// Conditional jump to the relative address specified by the signed 8-bit
// operand e, depending on the condition cc.
//
// Every conditional jr/jp/call/ret in this file shares one shape: always
// consume the operand bytes (PC has to advance past them regardless), then
// only apply the effect if cc holds, and finally return cc itself so
// cpu_execute_opcode (in opcodes.c) knows whether to charge the extra
// cycles a taken branch costs on top of this opcode's base cost.
bool jr_cc_e(CPU *cpu, Memory *memory, bool cc){
  int8_t e = (int8_t)memory_read(memory, cpu->pc); cpu->pc++;
  if (cc){cpu->pc+=e;}
  return cc;
};

bool jr_nc_e(CPU * cpu, Memory *memory){return jr_cc_e(cpu, memory, !((cpu->f & C_FLAG)) );}; // 0x30
bool jr_nz_e(CPU * cpu, Memory *memory){return jr_cc_e(cpu, memory, !((cpu->f & Z_FLAG)) );}; // 0x20
bool jr_c_e(CPU * cpu, Memory *memory){return jr_cc_e(cpu, memory, ((cpu->f & C_FLAG)) );}; // 0x38
bool jr_z_e(CPU * cpu, Memory *memory){return jr_cc_e(cpu, memory, ((cpu->f & Z_FLAG)) );}; // 0x28

// Conditional jump to the absolute address specified by the 16-bit operand
// nn (reassembled little-endian, see ld_rr_n16 in op_load.c), depending on
// the condition cc.
bool jp_cc_nn(CPU *cpu, Memory *memory, bool cc){
  u8 nn_lsb = memory_read(memory, cpu->pc); cpu->pc++;
  u8 nn_msb = memory_read(memory, cpu->pc); cpu->pc++;
  u16 nn = ((u16)nn_msb << 8) | nn_lsb;
  if (cc){cpu->pc=nn;}
  return cc;
};

bool jp_nc_nn(CPU * cpu, Memory *memory){return jp_cc_nn(cpu, memory, !((cpu->f & C_FLAG)) );}; // 0xD2
bool jp_nz_nn(CPU * cpu, Memory *memory){return jp_cc_nn(cpu, memory, !((cpu->f & Z_FLAG)) );}; // 0xC2
bool jp_c_nn(CPU *cpu, Memory *memory){return jp_cc_nn(cpu, memory, ((cpu->f & C_FLAG)) );}; // 0xDA
bool jp_z_nn(CPU *cpu, Memory *memory){return jp_cc_nn(cpu, memory, ((cpu->f & Z_FLAG)) );}; // 0xCA

// Unconditional jump to the absolute address specified by the 16-bit immediate operand nn.
void jp_nn(CPU *cpu, Memory *memory){jp_cc_nn(cpu, memory, true);}; // 0xC3

// Unconditional jump to the absolute address specified by the 16-bit register HL.
void jp_hl(CPU *cpu, Memory *memory){cpu->pc = cpu->hl;}; // 0xE9

// Unconditional jump to the relative address specified by the signed 8-bit operand e.
void jr_e(CPU *cpu, Memory *memory){jr_cc_e(cpu, memory, true);}; // 0x18 


/*
---------Function call instructions---------
*/

// Conditional function call to the absolute address specified by the
// 16-bit operand nn, depending on the condition cc.
//
// Note this pushes cpu->pc *after* both operand bytes have already been
// consumed above - i.e. the address of the instruction right after this
// CALL, which is exactly the return address a later RET needs to resume
// at. Pushing anything else here (this project once had a bug that pushed
// the stack pointer's own value by mistake) silently sends the eventual
// RET to a nonsense address.
bool call_cc_nn(CPU *cpu, Memory *memory, bool cc){
  u8 nn_lsb = memory_read(memory, cpu->pc); cpu->pc++;
  u8 nn_msb = memory_read(memory, cpu->pc); cpu->pc++;
  u16 nn = ((u16)nn_msb << 8) | nn_lsb;
  if (cc){
    cpu->sp-=1;
    memory_write(memory, cpu->sp, (cpu->pc >> 8) & 0xFF); cpu->sp-=1;
    memory_write(memory, cpu->sp, cpu->pc & 0xFF);
    cpu->pc = nn;
  }
  return cc;
}


bool call_nz_nn(CPU *cpu, Memory *memory){return call_cc_nn(cpu, memory, !((cpu->f & Z_FLAG)));}; // 0xC4
bool call_nc_nn(CPU *cpu, Memory *memory){return call_cc_nn(cpu, memory, !((cpu->f & C_FLAG)));}; // 0xD4
bool call_z_nn(CPU *cpu, Memory *memory){return call_cc_nn(cpu, memory, ((cpu->f & Z_FLAG)));}; // 0xCC
bool call_c_nn(CPU *cpu, Memory *memory){return call_cc_nn(cpu, memory, ((cpu->f & C_FLAG)));}; // 0xDC


// Unconditional function call to the absolute address specified by the
// 16-bit operand nn. Implemented as call_cc_nn with cc always true, rather
// than duplicating the push+jump logic - same pattern jp_nn/jr_e use below.
void call_nn(CPU *cpu, Memory *memory){call_cc_nn(cpu, memory, true);}; // 0xCD

/*
---------Function return instructions---------
*/

// Conditional return from a function, depending on the condition cc.
bool ret_cc(CPU *cpu, Memory *memory, bool cc){
  if (cc){
    u8 lsb = memory_read(memory, cpu->sp); cpu->sp++;
    u8 msb = memory_read(memory, cpu->sp); cpu->sp++;
    cpu->pc = ((u16)msb << 8) | lsb;
  }
  return cc;
}

bool ret_nz(CPU *cpu, Memory *memory){return ret_cc(cpu, memory, !(cpu->f & Z_FLAG));}; // 0xC0
bool ret_nc(CPU *cpu, Memory *memory){return ret_cc(cpu, memory, !(cpu->f & C_FLAG));}; // 0xD0
bool ret_z(CPU *cpu, Memory *memory){return ret_cc(cpu, memory, cpu->f & Z_FLAG);}; // 0xC8
bool ret_c(CPU *cpu, Memory *memory){return ret_cc(cpu, memory, cpu->f & C_FLAG);}; // 0xD8

// Unconditional return from a function.
void ret(CPU *cpu, Memory *memory){ret_cc(cpu, memory, true);}; // 0xC9


// Unconditional return from a function. Also enables interrupts by setting
// IME=1 - unlike EI, this takes effect immediately rather than being
// delayed to after the next instruction, since RETI is specifically meant
// to end an interrupt handler and hand control straight back.
void reti(CPU *cpu, Memory *memory){
  ret_cc(cpu, memory, true);
  cpu->ime = true;
}; // 0xD9


// Unconditional function call to one of 8 fixed addresses baked into the
// opcode itself (n is 0x00/0x08/.../0x38 - see the wrappers below), same
// push as CALL/interrupt dispatch. RST is really just a compact,
// single-byte CALL to a small set of common targets, historically used for
// frequently-called routines to save ROM space.
void rst_n(CPU *cpu, Memory *memory, u8 n){
    cpu->sp-=1;
    memory_write(memory, cpu->sp, (cpu->pc >> 8) & 0xFF); cpu->sp-=1;
    memory_write(memory, cpu->sp, cpu->pc & 0xFF);
    cpu->pc = (u16)n;
}

void rst_0x00(CPU *cpu, Memory *memory){rst_n(cpu, memory, 0x00);}; // 0xC7
void rst_0x10(CPU *cpu, Memory *memory){rst_n(cpu, memory, 0x10);}; // 0xD7
void rst_0x20(CPU *cpu, Memory *memory){rst_n(cpu, memory, 0x20);}; // 0xE7
void rst_0x30(CPU *cpu, Memory *memory){rst_n(cpu, memory, 0x30);}; // 0xF7
void rst_0x08(CPU *cpu, Memory *memory){rst_n(cpu, memory, 0x08);}; // 0xCF
void rst_0x18(CPU *cpu, Memory *memory){rst_n(cpu, memory, 0x18);}; // 0xDF
void rst_0x28(CPU *cpu, Memory *memory){rst_n(cpu, memory, 0x28);}; // 0xEF
void rst_0x38(CPU *cpu, Memory *memory){rst_n(cpu, memory, 0x38);}; // 0xFF


