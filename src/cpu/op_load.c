#include "../../include/cpu/op_load.h"

/*
---------16-bit load instructions---------
*/

// Load to 16-bit register (rr) the immediate 16-bit data (nn) 
void ld_rr_n16(u16 rr, CPU *cpu, Memory *memory){
  u8 nn_lsb = memory_read(memory, cpu->pc); cpu->pc++;
  u8 nn_msb = memory_read(memory, cpu->pc); cpu->pc++;
  u16 nn = ((u16)nn_lsb << 8) | nn_msb;
  rr = nn; 

}

void ld_bc_n16(CPU *cpu, Memory *memory){
  ld_rr_n16(cpu->bc, cpu, memory);
}; //0x01

void ld_de_n16(CPU *cpu, Memory *memory){
  ld_rr_n16(cpu->de, cpu, memory);
}; //0x11
void ld_hl_n16(CPU *cpu, Memory *memory){
  ld_rr_n16(cpu->hl, cpu, memory);
}; //0x21
void ld_sp_n16(CPU *cpu, Memory *memory){
  ld_rr_n16(cpu->sp, cpu, memory);
}; //0x31


// Load to the adress specified in 16-bit immediate data (nn) to the 1--bit SP register
void ld_nn_sp(CPU *cpu, Memory *memory); // 0x08

// Pops to the 16-bit register rr data from the stack memory
void pop_bc(CPU *cpu, Memory *memory); // 0xC1
void pop_de(CPU *cpu, Memory *memory); // 0xD1
void pop_hl(CPU *cpu, Memory *memory); // 0xE1
void pop_af(CPU *cpu, Memory *memory); // 0xF1

// Push to the stack memory, data from the 16-bit register rr
void push_bc(CPU *cpu, Memory *memory); // 0xC5
void push_de(CPU *cpu, Memory *memory); // 0xD5
void push_hl(CPU *cpu, Memory *memory); // 0xE5
void push_af(CPU *cpu, Memory *memory); // 0xF5

// Load to the HL register, 16-bit data calculated by adding the signed 8-bit operand e to the 16-bit value of the SP register
void ld_hl_sp(CPU *cpu, Memory *memory); // 0xF8

// Load to the 16-bit SP register, data from the 16-bit HL register
void ld_sp_hl(CPU *cpu, Memory *memory); //0xF9

