#include "../../include/cpu/op_load.h"

/*
---------16-bit load instructions---------
*/

// Load to 16-bit register (rr) the immediate 16-bit data (nn) 
void ld_rr_n16(u16 *rr, CPU *cpu, Memory *memory){
  u8 nn_lsb = memory_read(memory, cpu->pc); cpu->pc++;
  u8 nn_msb = memory_read(memory, cpu->pc); cpu->pc++;
  u16 nn = ((u16)nn_lsb << 8) | nn_msb;
  *rr = nn; 

}

void ld_bc_n16(CPU *cpu, Memory *memory){
  ld_rr_n16(&cpu->bc, cpu, memory);
}; //0x01

void ld_de_n16(CPU *cpu, Memory *memory){
  ld_rr_n16(&cpu->de, cpu, memory);
}; //0x11
void ld_hl_n16(CPU *cpu, Memory *memory){
  ld_rr_n16(&cpu->hl, cpu, memory);
}; //0x21
void ld_sp_n16(CPU *cpu, Memory *memory){
  ld_rr_n16(&cpu->sp, cpu, memory);
}; //0x31


// Load to the adress specified in 16-bit immediate data (nn) to the 16-bit SP register
void ld_nn_sp(CPU *cpu, Memory *memory){
  u8 nn_lsb = memory_read(memory, cpu->pc); cpu->pc++;
  u8 nn_msb = memory_read(memory, cpu->pc); cpu->pc++;
  u16 nn = ((u16)nn_lsb << 8) | nn_msb;
  memory_write(memory,nn,(cpu->sp >> 8) & 0xFF); nn++;
  memory_write(memory,nn,cpu->sp & 0xFF);
}; // 0x08


// Pops to the 16-bit register rr data from the stack memory
void pop(u16 *rr, CPU *cpu, Memory *memory){
  u8 lsb = memory_read(memory, cpu->sp); cpu->sp++;
  u8 msb = memory_read(memory, cpu->sp); cpu->sp++;
  *rr = ((u16)lsb << 8) | msb;
};

void pop_bc(CPU *cpu, Memory *memory){
  pop(&cpu->bc, cpu, memory);
}; // 0xC1
void pop_de(CPU *cpu, Memory *memory){
  pop(&cpu->de, cpu, memory);
}; // 0xD1
void pop_hl(CPU *cpu, Memory *memory){
  pop(&cpu->hl, cpu, memory);
}; // 0xE1
void pop_af(CPU *cpu, Memory *memory){
  pop(&cpu->af, cpu, memory);
}; // 0xF1

// Push to the stack memory, data from the 16-bit register rr
void push(u16 *rr, CPU *cpu, Memory *memory){
  cpu->sp--;
  memory_write(memory,cpu->sp,(*rr >> 8) & 0xFF); cpu->sp--;
  memory_write(memory,cpu->sp,*rr & 0xFF);
};

void push_bc(CPU *cpu, Memory *memory){
  push(&cpu->bc, cpu, memory); 
}; // 0xC5
void push_de(CPU *cpu, Memory *memory){
  push(&cpu->de, cpu, memory); 
}; // 0xD5
void push_hl(CPU *cpu, Memory *memory){
  push(&cpu->hl, cpu, memory); 
}; // 0xE5
void push_af(CPU *cpu, Memory *memory){
  push(&cpu->af, cpu, memory); 
}; // 0xF5


// Load to the HL register, 16-bit data calculated by adding the signed 8-bit operand e to the 16-bit value of the SP register
void ld_hl_sp(CPU *cpu, Memory *memory){
  int8_t e = (int8_t)memory_read(memory, cpu->pc); cpu->pc++;
  u8 result = cpu->sp + e;
  u8 h = ((cpu->sp & 0xF) + (e & 0xF)) > 0xF;  
  u8 c = ((cpu->sp & 0xFF) + (e & 0xFF)) > 0xFF;    
  
  cpu->hl = result;

  cpu->f = 0;
  cpu->f |= (h << 5);
  cpu->f |= (c << 4);
}; // 0xF8

// Load to the 16-bit SP register, data from the 16-bit HL register
void ld_sp_hl(CPU *cpu, Memory *memory){
  cpu->sp = cpu->hl;
}; //0xF9

