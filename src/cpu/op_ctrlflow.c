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









/*
---------8-bit load instructions---------
*/

// Load an immediate 8-bit value (n) into an 8-bit register (r)
void ld_r_n(u8 *r,CPU *cpu, Memory *memory){
  *r = memory_read(memory, cpu->pc); cpu->pc++;
};

void ld_a_n(CPU *cpu, Memory *memory){
  ld_r_n(&cpu->a, cpu, memory);
}; // 0x3E
void ld_b_n(CPU *cpu, Memory *memory){
  ld_r_n(&cpu->b, cpu, memory);
}; // 0x06
void ld_c_n(CPU *cpu, Memory *memory){
  ld_r_n(&cpu->c, cpu, memory);
}; // 0x0E
void ld_d_n(CPU *cpu, Memory *memory){
  ld_r_n(&cpu->d, cpu, memory);
}; // 0x16
void ld_e_n(CPU *cpu, Memory *memory){
  ld_r_n(&cpu->e, cpu, memory);
}; // 0x1E
void ld_h_n(CPU *cpu, Memory *memory){
  ld_r_n(&cpu->h, cpu, memory);
}; // 0x26
void ld_l_n(CPU *cpu, Memory *memory){
  ld_r_n(&cpu->l, cpu, memory);
}; // 0x2E

// Load an immediate 8-bit value (n) into HL 16-bit address
void ld_hl_n(CPU *cpu, Memory *memory){
  u8 n = memory_read(memory, cpu->pc); cpu->pc++;
  memory_write(memory, cpu->hl, n);
}; // 0x36


// Load the value from a register (r2) into another register (r1)
void ld_a_a(CPU *cpu, Memory *memory) { cpu->a = cpu->a; } // 0x7F
void ld_a_b(CPU *cpu, Memory *memory) { cpu->a = cpu->b; } // 0x78
void ld_a_c(CPU *cpu, Memory *memory) { cpu->a = cpu->c; } // 0x79
void ld_a_d(CPU *cpu, Memory *memory) { cpu->a = cpu->d; } // 0x7A
void ld_a_e(CPU *cpu, Memory *memory) { cpu->a = cpu->e; } // 0x7B
void ld_a_h(CPU *cpu, Memory *memory) { cpu->a = cpu->h; } // 0x7C
void ld_a_l(CPU *cpu, Memory *memory) { cpu->a = cpu->l; } // 0x7D

void ld_b_a(CPU *cpu, Memory *memory) { cpu->b = cpu->a; } // 0x47
void ld_b_b(CPU *cpu, Memory *memory) { cpu->b = cpu->b; } // 0x40 (inutile)
void ld_b_c(CPU *cpu, Memory *memory) { cpu->b = cpu->c; } // 0x41
void ld_b_d(CPU *cpu, Memory *memory) { cpu->b = cpu->d; } // 0x42
void ld_b_e(CPU *cpu, Memory *memory) { cpu->b = cpu->e; } // 0x43
void ld_b_h(CPU *cpu, Memory *memory) { cpu->b = cpu->h; } // 0x44
void ld_b_l(CPU *cpu, Memory *memory) { cpu->b = cpu->l; } // 0x45

void ld_c_a(CPU *cpu, Memory *memory) { cpu->c = cpu->a; } // 0x4F
void ld_c_b(CPU *cpu, Memory *memory) { cpu->c = cpu->b; } // 0x48
void ld_c_c(CPU *cpu, Memory *memory) { cpu->c = cpu->c; } // 0x49 (inutile)
void ld_c_d(CPU *cpu, Memory *memory) { cpu->c = cpu->d; } // 0x4A
void ld_c_e(CPU *cpu, Memory *memory) { cpu->c = cpu->e; } // 0x4B
void ld_c_h(CPU *cpu, Memory *memory) { cpu->c = cpu->h; } // 0x4C
void ld_c_l(CPU *cpu, Memory *memory) { cpu->c = cpu->l; } // 0x4D

void ld_d_a(CPU *cpu, Memory *memory) { cpu->d = cpu->a; } // 0x57
void ld_d_b(CPU *cpu, Memory *memory) { cpu->d = cpu->b; } // 0x50
void ld_d_c(CPU *cpu, Memory *memory) { cpu->d = cpu->c; } // 0x51
void ld_d_d(CPU *cpu, Memory *memory) { cpu->d = cpu->d; } // 0x52 (inutile)
void ld_d_e(CPU *cpu, Memory *memory) { cpu->d = cpu->e; } // 0x53
void ld_d_h(CPU *cpu, Memory *memory) { cpu->d = cpu->h; } // 0x54
void ld_d_l(CPU *cpu, Memory *memory) { cpu->d = cpu->l; } // 0x55

void ld_e_a(CPU *cpu, Memory *memory) { cpu->e = cpu->a; } // 0x5F
void ld_e_b(CPU *cpu, Memory *memory) { cpu->e = cpu->b; } // 0x58
void ld_e_c(CPU *cpu, Memory *memory) { cpu->e = cpu->c; } // 0x59
void ld_e_d(CPU *cpu, Memory *memory) { cpu->e = cpu->d; } // 0x5A
void ld_e_e(CPU *cpu, Memory *memory) { cpu->e = cpu->e; } // 0x5B (inutile)
void ld_e_h(CPU *cpu, Memory *memory) { cpu->e = cpu->h; } // 0x5C
void ld_e_l(CPU *cpu, Memory *memory) { cpu->e = cpu->l; } // 0x5D

void ld_h_a(CPU *cpu, Memory *memory) { cpu->h = cpu->a; } // 0x67
void ld_h_b(CPU *cpu, Memory *memory) { cpu->h = cpu->b; } // 0x60
void ld_h_c(CPU *cpu, Memory *memory) { cpu->h = cpu->c; } // 0x61
void ld_h_d(CPU *cpu, Memory *memory) { cpu->h = cpu->d; } // 0x62
void ld_h_e(CPU *cpu, Memory *memory) { cpu->h = cpu->e; } // 0x63
void ld_h_h(CPU *cpu, Memory *memory) { cpu->h = cpu->h; } // 0x64 (inutile)
void ld_h_l(CPU *cpu, Memory *memory) { cpu->h = cpu->l; } // 0x65

void ld_l_a(CPU *cpu, Memory *memory) { cpu->l = cpu->a; } // 0x6F
void ld_l_b(CPU *cpu, Memory *memory) { cpu->l = cpu->b; } // 0x68
void ld_l_c(CPU *cpu, Memory *memory) { cpu->l = cpu->c; } // 0x69
void ld_l_d(CPU *cpu, Memory *memory) { cpu->l = cpu->d; } // 0x6A
void ld_l_e(CPU *cpu, Memory *memory) { cpu->l = cpu->e; } // 0x6B
void ld_l_h(CPU *cpu, Memory *memory) { cpu->l = cpu->h; } // 0x6C
void ld_l_l(CPU *cpu, Memory *memory) { cpu->l = cpu->l; } // 0x6D (inutile)



// Load from memory at address HL into a register
void ld_b_hl(CPU *cpu, Memory *memory) { cpu->b = memory_read(memory, cpu->hl); }; // 0x46
void ld_c_hl(CPU *cpu, Memory *memory) { cpu->c = memory_read(memory, cpu->hl); }; // 0x4E
void ld_d_hl(CPU *cpu, Memory *memory) { cpu->d = memory_read(memory, cpu->hl); }; // 0x56
void ld_e_hl(CPU *cpu, Memory *memory) { cpu->e = memory_read(memory, cpu->hl); }; // 0x5E
void ld_h_hl(CPU *cpu, Memory *memory) { cpu->h = memory_read(memory, cpu->hl); }; // 0x66
void ld_l_hl(CPU *cpu, Memory *memory) { cpu->l = memory_read(memory, cpu->hl); }; // 0x6E
void ld_a_hl(CPU *cpu, Memory *memory) { cpu->a = memory_read(memory, cpu->hl); }; // 0x7E

// Load from a register into memory at address HL
void ld_hl_b(CPU *cpu, Memory *memory) { memory_write(memory, cpu->hl, cpu->b); }; // 0x70
void ld_hl_c(CPU *cpu, Memory *memory) { memory_write(memory, cpu->hl, cpu->c); }; // 0x71
void ld_hl_d(CPU *cpu, Memory *memory) { memory_write(memory, cpu->hl, cpu->d); }; // 0x72
void ld_hl_e(CPU *cpu, Memory *memory) { memory_write(memory, cpu->hl, cpu->e); }; // 0x73
void ld_hl_h(CPU *cpu, Memory *memory) { memory_write(memory, cpu->hl, cpu->h); }; // 0x74
void ld_hl_l(CPU *cpu, Memory *memory) { memory_write(memory, cpu->hl, cpu->l); }; // 0x75
void ld_hl_a(CPU *cpu, Memory *memory) { memory_write(memory, cpu->hl, cpu->a); }; // 0x77


// Load an 8-bit value from memory at address (BC) into register A
void ld_a_bc(CPU *cpu, Memory *memory) { cpu->a = memory_read(memory, cpu->bc); }; // 0x0A

// Load an 8-bit value from register A into memory at address (BC)
void ld_bc_a(CPU *cpu, Memory *memory) { memory_write(memory, cpu->bc, cpu->a); }; // 0x02

// Load an 8-bit value from memory at address (DE) into register A
void ld_a_de(CPU *cpu, Memory *memory) { cpu->a = memory_read(memory, cpu->de); }; // 0x1A

// Load an 8-bit value from register A into memory at address (DE)
void ld_de_a(CPU *cpu, Memory *memory) { memory_write(memory, cpu->de, cpu->a); }; // 0x12

// Load an 8-bit value from memory at address (nn) into register A
void ld_a_nn(CPU *cpu, Memory *memory) {
  u8 nn_lsb = memory_read(memory, cpu->pc); cpu->pc++;
  u8 nn_msb = memory_read(memory, cpu->pc); cpu->pc++;
  u16 nn = ((u16)nn_lsb << 8) | nn_msb;
  cpu->a = memory_read(memory, nn); 
}; // 0xFA

// Load an 8-bit value from register A into memory at address (nn)
void ld_nn_a(CPU *cpu, Memory *memory) {
  u8 nn_lsb = memory_read(memory, cpu->pc); cpu->pc++;
  u8 nn_msb = memory_read(memory, cpu->pc); cpu->pc++;
  u16 nn = ((u16)nn_lsb << 8) | nn_msb;
  memory_write(memory, nn, cpu->a); 
}; // 0xEA

// Load an 8-bit value from memory at address (0xFF00 + C) into register A
void ld_a_ff00_c(CPU *cpu, Memory *memory) { cpu->a = memory_read(memory, ((u16)cpu->c << 8) | 0xFF ); }; // 0xF2

// Load an 8-bit value from register A into memory at address (0xFF00 + C)
void ld_ff00_c_a(CPU *cpu, Memory *memory) { memory_write(memory, ((u16)cpu->c << 8) | 0xFF, cpu->a); }; // 0xE2

// Load an 8-bit value from memory at address (0xFF00 + n) into register A
void ld_a_ff00_n(CPU *cpu, Memory *memory) { 
  u8 n = memory_read(memory, cpu->pc); cpu->pc++;
  cpu->a = memory_read(memory,  ((u16)n << 8) | 0xFF);
}; // 0xF0

// Load an 8-bit value from register A into memory at address (0xFF00 + n)
void ld_ff00_n_a(CPU *cpu, Memory *memory){
  u8 n = memory_read(memory, cpu->pc); cpu->pc++;
  memory_write(memory, ((u16)n << 8) | 0xFF, cpu->a);
}; // 0xE0

// Load from memory at address HL into A, then decrement HL
void ld_a_hl_dec(CPU *cpu, Memory *memory){
  cpu->a = memory_read(memory, cpu->hl); cpu->hl--;
}; // 0x3A

// Load from A into memory at address HL, then decrement HL
void ld_hl_dec_a(CPU *cpu, Memory *memory){
  memory_write(memory, cpu->hl, cpu->a); cpu->hl--;
}; // 0x32

// Load from memory at address HL into A, then increment HL
void ld_a_hl_inc(CPU *cpu, Memory *memory){
  cpu->a = memory_read(memory, cpu->hl); cpu->hl++;
}; // 0x2A

// Load from A into memory at address HL, then increment HL
void ld_hl_inc_a(CPU *cpu, Memory *memory){
  memory_write(memory, cpu->hl, cpu->a); cpu->hl++;
}; // 0x22


