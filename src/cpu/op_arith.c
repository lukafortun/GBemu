#include "../../include/cpu/op_arith.h"


/*
---------Add instructions---------
*/

// Adds to the 8-bit A register, the 8-bit register r, and stores the result back into the A register.
void add_r8(CPU *cpu, u8 r){
    cpu->f &= ~(N_FLAG | H_FLAG | C_FLAG); // Clear N, H, C flags
    if((cpu->a & 0x0F) + (r & 0x0F) > 0x0F) cpu->f |= H_FLAG; // Set H flag if there is a carry from bit 3
    if(cpu->a + r > 0xFF) cpu->f |= C_FLAG; // Set C flag if there is a carry from bit 7
    cpu->a += r;
    if(cpu->a == 0) cpu->f |= Z_FLAG; // Set Z flag if result is zero
}

void add_b(CPU *cpu, Memory *memory){ add_r8(cpu, cpu->b); }; // 0x80
void add_c(CPU *cpu, Memory *memory) { add_r8(cpu, cpu->c); } // 0x81
void add_d(CPU *cpu, Memory *memory) { add_r8(cpu, cpu->d); } // 0x82
void add_e(CPU *cpu, Memory *memory) { add_r8(cpu, cpu->e); } // 0x83
void add_h(CPU *cpu, Memory *memory) { add_r8(cpu, cpu->h); } // 0x84
void add_l(CPU *cpu, Memory *memory) { add_r8(cpu, cpu->l); } // 0x85
void add_a(CPU *cpu, Memory *memory) { add_r8(cpu, cpu->a); } // 0x87

// Adds to the 8-bit A register, data from the absolute address specified by the 16-bit register HL, and stores the result back into the A register.
void add_hl(CPU *cpu, Memory *memory) { add_r8(cpu, memory_read(memory, cpu->hl)); } // 0x86

// Adds to the 8-bit A register, the immediate data n, and stores the result back into the A register.
void add_n(CPU *cpu, Memory *memory) { add_r8(cpu, memory_read(memory, cpu->pc++)); } // 0xC6



// Adds to the 8-bit A register, the carry flag and the 8-bit register r, and stores the result back into the A register.
void adc_r(CPU *cpu, u8 r){
    cpu->f &= ~(N_FLAG | H_FLAG | C_FLAG); // Clear N, H, C flags
    if((cpu->a & 0x0F) + (r & 0x0F) + (cpu->f & C_FLAG) > 0x0F) cpu->f |= H_FLAG; // Set H flag if there is a carry from bit 3
    if(cpu->a + r + (cpu->f & C_FLAG) > 0xFF) cpu->f |= C_FLAG; // Set C flag if there is a carry from bit 7
    cpu->a += r + (cpu->f & C_FLAG);
    if(cpu->a == 0) cpu->f |= Z_FLAG; // Set Z flag if result is zero
}

void adc_b(CPU *cpu, Memory *memory) { adc_r(cpu, cpu->b); } // 0x88
void adc_c(CPU *cpu, Memory *memory) { adc_r(cpu, cpu->c); } // 0x89
void adc_d(CPU *cpu, Memory *memory) { adc_r(cpu, cpu->d); } // 0x8A
void adc_e(CPU *cpu, Memory *memory) { adc_r(cpu, cpu->e); } // 0x8B
void adc_h(CPU *cpu, Memory *memory) { adc_r(cpu, cpu->h); } // 0x8C
void adc_l(CPU *cpu, Memory *memory) { adc_r(cpu, cpu->l); } // 0x8D
void adc_a(CPU *cpu, Memory *memory) { adc_r(cpu, cpu->a); } // 0x8F

void adc_hl(CPU *cpu, Memory *memory) { adc_r(cpu, memory_read(memory, cpu->hl)); } // 0x8E

void adc_n(CPU *cpu, Memory *memory) { adc_r(cpu, memory_read(memory, cpu->pc++)); } // 0xCE



/*
---------Subtract instructions---------
*/

// Subtracts from the 8-bit A register, the 8-bit register r, and stores the result back into the A register.
void sub_r(CPU *cpu, u8 r) {
    cpu->f = N_FLAG; // Set N flag
    if((cpu->a & 0x0F) < (r & 0x0F)) cpu->f |= H_FLAG; // Set H flag if there is a borrow from bit 4
    if(cpu->a < r) cpu->f |= C_FLAG; // Set C flag if there is a borrow from bit 8
    cpu->a -= r;
    if(cpu->a == 0) cpu->f |= Z_FLAG; // Set Z flag if result is zero
}

void sub_b(CPU *cpu, Memory *memory){ sub_r(cpu, cpu->b); }; // 0x90
void sub_c(CPU *cpu, Memory *memory){ sub_r(cpu, cpu->c); }; // 0x91
void sub_d(CPU *cpu, Memory *memory){ sub_r(cpu, cpu->d); }; // 0x92
void sub_e(CPU *cpu, Memory *memory){ sub_r(cpu, cpu->e); }; // 0x93
void sub_h(CPU *cpu, Memory *memory){ sub_r(cpu, cpu->h); }; // 0x94
void sub_l(CPU *cpu, Memory *memory){ sub_r(cpu, cpu->l); }; // 0x95
void sub_a(CPU *cpu, Memory *memory){ sub_r(cpu, cpu->a); }; // 0x97

// Subtracts from the 8-bit A register, the 8-bit data from the absolute address specified by the 16-bit register HL, and stores the result back into the A register.
void sub_hl(CPU *cpu, Memory *memory) { sub_r(cpu, memory_read(memory, cpu->hl)); }; // 0x96

// Subtracts from the 8-bit A register, the immediate data n, and stores the result back into the A register.
void sub_n(CPU *cpu, Memory *memory) { sub_r(cpu, memory_read(memory, cpu->pc++)); }; // 0xD6


// Subtracts from the 8-bit A register, the carry flag and the 8-bit register r, and stores the result back into the A register.
void sbc_r(CPU *cpu, u8 r) {
    cpu->f = N_FLAG; // Set N flag
    if((cpu->a & 0x0F) < (r & 0x0F) + (cpu->f & C_FLAG)) cpu->f |= H_FLAG; // Set H flag if there is a borrow from bit 4
    if(cpu->a < r + (cpu->f & C_FLAG)) cpu->f |= C_FLAG; // Set C flag if there is a borrow from bit 8
    cpu->a -= r + (cpu->f & C_FLAG);
    if(cpu->a == 0) cpu->f |= Z_FLAG; // Set Z flag if result is zero
}

void sbc_b(CPU *cpu, Memory *memory){ sbc_r(cpu, cpu->b); }; // 0x98
void sbc_c(CPU *cpu, Memory *memory){ sbc_r(cpu, cpu->c); }; // 0x99
void sbc_d(CPU *cpu, Memory *memory){ sbc_r(cpu, cpu->d); }; // 0x9A
void sbc_e(CPU *cpu, Memory *memory){ sbc_r(cpu, cpu->e); }; // 0x9B
void sbc_h(CPU *cpu, Memory *memory){ sbc_r(cpu, cpu->h); }; // 0x9C
void sbc_l(CPU *cpu, Memory *memory){ sbc_r(cpu, cpu->l); }; // 0x9D
void sbc_a(CPU *cpu, Memory *memory){ sbc_r(cpu, cpu->a); }; // 0x9F

// Subtracts from the 8-bit A register, the carry flag and data from the absolute address specified by the 16-bit register HL, and stores the result back into the A register.
void sbc_hl(CPU *cpu, Memory *memory){ sbc_r(cpu, memory_read(memory, cpu->hl)); }; // 0x9E

// Subtracts from the 8-bit A register, the carry flag and the immediate data n, and stores the result back into the A register.
void sbc_n(CPU *cpu, Memory *memory){ sbc_r(cpu, memory_read(memory, cpu->pc++)); }; // 0xDE


/*
---------Other arithmetical instructions---------
*/

// Performs a logical AND between the A register and the 8-bit register r, stores the result back into the A register.
void and_r(CPU *cpu, u8 r){
    cpu->f = H_FLAG; // Set H flag
    cpu->a &= r;
    if(cpu->a == 0) cpu->f |= Z_FLAG; // Set Z flag if result is zero
}

void and_b(CPU *cpu, Memory *memory) { and_r(cpu, cpu->b); }; // 0xA0
void and_c(CPU *cpu, Memory *memory) { and_r(cpu, cpu->c); }; // 0xA1
void and_d(CPU *cpu, Memory *memory) { and_r(cpu, cpu->d); }; // 0xA2
void and_e(CPU *cpu, Memory *memory) { and_r(cpu, cpu->e); }; // 0xA3
void and_h(CPU *cpu, Memory *memory) { and_r(cpu, cpu->h); }; // 0xA4
void and_l(CPU *cpu, Memory *memory) { and_r(cpu, cpu->l); }; // 0xA5
void and_a(CPU *cpu, Memory *memory) { and_r(cpu, cpu->a); }; // 0xA7

// Performs a bitwise AND operation between the 8-bit A register and data from the absolute address specified by the 16-bit register HL, and stores the result back into the A register.
void and_hl(CPU *cpu, Memory *memory) { and_r(cpu, memory_read(memory, cpu->hl)); }; // 0xA6

//Performs a bitwise AND operation between the 8-bit A register and immediate data n, and stores the result back into the A register.
void and_n(CPU *cpu, Memory *memory) { and_r(cpu, memory_read(memory, cpu->pc++));}; // 0xE6



// Performs a logical XOR between the A register and the 8-bit register r, stores the result back into the A register.
void xor_r(CPU *cpu, u8 r){
    cpu->f = 0; // Clear all flags
    cpu->a ^= r;
    if(cpu->a == 0) cpu->f |= Z_FLAG; // Set Z flag if result is zero
}

void xor_b(CPU *cpu, Memory *memory) { xor_r(cpu, cpu->b); }; // 0xA8
void xor_c(CPU *cpu, Memory *memory) { xor_r(cpu, cpu->c); }; // 0xA9
void xor_d(CPU *cpu, Memory *memory) { xor_r(cpu, cpu->d); }; // 0xAA
void xor_e(CPU *cpu, Memory *memory) { xor_r(cpu, cpu->e); }; // 0xAB
void xor_h(CPU *cpu, Memory *memory) { xor_r(cpu, cpu->h); }; // 0xAC
void xor_l(CPU *cpu, Memory *memory) { xor_r(cpu, cpu->l); }; // 0xAD
void xor_a(CPU *cpu, Memory *memory) { xor_r(cpu, cpu->a); }; // 0xAF

// Performs a bitwise XOR operation between the 8-bit A register and data from the absolute address specified by the 16-bit register HL, and stores the result back into the A register.
void xor_hl(CPU *cpu, Memory *memory){ xor_r(cpu, memory_read(memory, cpu->pc++));}; // 0xAE

// Performs a bitwise XOR operation between the 8-bit A register and immediate data n, and stores the result back into the A register.
void xor_n(CPU *cpu, Memory *memory){ xor_r(cpu, memory_read(memory, cpu->pc++));}; // 0xEE



// Performs a logical OR between the A register and the 8-bit register r, stores the result back into the A register.
void or_r(CPU *cpu, u8 r){
    cpu->f = 0; // Clear all flags
    cpu->a |= r;
    if(cpu->a == 0) cpu->f |= Z_FLAG; // Set Z flag if result is zero
}

void or_b(CPU *cpu, Memory *memory){or_r(cpu, cpu->b);}; // 0xB0
void or_c(CPU *cpu, Memory *memory){or_r(cpu, cpu->c);}; // 0xB1
void or_d(CPU *cpu, Memory *memory){or_r(cpu, cpu->d);}; // 0xB2
void or_e(CPU *cpu, Memory *memory){or_r(cpu, cpu->e);}; // 0xB3
void or_h(CPU *cpu, Memory *memory){or_r(cpu, cpu->h);}; // 0xB4
void or_l(CPU *cpu, Memory *memory){or_r(cpu, cpu->l);}; // 0xB5
void or_a(CPU *cpu, Memory *memory){or_r(cpu, cpu->a);}; // 0xB7

// Performs a bitwise OR operation between the 8-bit A register and data from the absolute address specified by the 16-bit register HL, and stores the result back into the A register.
void or_hl(CPU *cpu, Memory *memory){ or_r(cpu, memory_read(memory, cpu->pc++));}; // 0xB6

// Performs a bitwise OR operation between the 8-bit A register and immediate data n, and stores the result back into the A register.
void or_n(CPU *cpu, Memory *memory){ or_r(cpu, memory_read(memory, cpu->pc++));}; // 0xF6



// Complements the A register (bitwise NOT operation).
void cp_r(CPU *cpu, Memory *memory){
    cpu->a = ~cpu->a;
    cpu->f |= N_FLAG | H_FLAG; // Set N and H flags
}

void cp_b(CPU *cpu, Memory *memory){cp_r(cpu, cpu->b);}; // 0xB8
void cp_c(CPU *cpu, Memory *memory){cp_r(cpu, cpu->c);}; // 0xB9
void cp_d(CPU *cpu, Memory *memory){cp_r(cpu, cpu->d);}; // 0xBA
void cp_e(CPU *cpu, Memory *memory){cp_r(cpu, cpu->e);}; // 0xBB
void cp_h(CPU *cpu, Memory *memory){cp_r(cpu, cpu->h);}; // 0xBC
void cp_l(CPU *cpu, Memory *memory){cp_r(cpu, cpu->l);}; // 0xBD
void cp_a(CPU *cpu, Memory *memory){cp_r(cpu, cpu->a);}; // 0xBF

// Subtracts from the 8-bit A register, data from the absolute address specified by the 16-bit register HL, and updates flags based on the result. This instruction is basically identical to SUB (HL), but does not update the A register
void cp_hl(CPU *cpu, Memory *memory){ cp_r(cpu, memory_read(memory, cpu->pc++));}; // 0xBE

// Subtracts from the 8-bit A register, the immediate data n, and updates flags based on the result. This instruction is basically identical to SUB n, but does not update the A register.
void cp_n(CPU *cpu, Memory *memory){ cp_r(cpu, memory_read(memory, cpu->pc++));}; // 0xFE



// Increments the 8-bit register r by 1.
void inc_r(CPU *cpu, u8 *r){
    cpu->f &= ~N_FLAG; // Clear N flag
    if((*r & 0x0F) == 0x0F) cpu->f |= H_FLAG; // Set H flag if there is a carry from bit 3
    (*r)++;
    if(*r == 0) cpu->f |= Z_FLAG; // Set Z flag if result is zero
}

void inc_b(CPU *cpu, Memory *memory); // 0x04
void inc_c(CPU *cpu, Memory *memory); // 0x0C
void inc_d(CPU *cpu, Memory *memory); // 0x14
void inc_e(CPU *cpu, Memory *memory); // 0x1C
void inc_h(CPU *cpu, Memory *memory); // 0x24
void inc_l(CPU *cpu, Memory *memory); // 0x2C
void inc_a(CPU *cpu, Memory *memory); // 0x3C

// Increments the data at the memory address specified by the 16-bit HL register pair.
void inc_hl(CPU *cpu, Memory *memory); // 0x34

// Decrements the 8-bit register r by 1.
void dec_b(CPU *cpu, Memory *memory); // 0x05
void dec_c(CPU *cpu, Memory *memory); // 0x0D
void dec_d(CPU *cpu, Memory *memory); // 0x15
void dec_e(CPU *cpu, Memory *memory); // 0x1D
void dec_h(CPU *cpu, Memory *memory); // 0x25
void dec_l(CPU *cpu, Memory *memory); // 0x2D
void dec_a(CPU *cpu, Memory *memory); // 0x3D

// Decrements the data at the memory address specified by the 16-bit HL register pair.
void dec_hl(CPU *cpu, Memory *memory); // 0x35

// Decimal adjust accumulator
void daa(CPU *cpu, Memory *memory); // 0x28

// Sets the carry flag, and clears the N and H flags.
void scf(CPU *cpu, Memory *memory); // 0x38

// Flips all the bits in the 8-bit A register, and sets the N and H flags.
void cpl(CPU *cpu, Memory *memory); // 0x2f

// Flips all the bits in the 8-bit A register, and sets the N and H flags.
void ccf(CPU *cpu, Memory *memory); // 0x3f







