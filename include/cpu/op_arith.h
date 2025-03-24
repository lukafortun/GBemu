#ifndef OP_ARITH_H
#define OP_ARITH_H

#include "./cpu.h"
#include "../memory.h"

/*
---------Add instructions---------
*/

// Adds to the 8-bit A register, the 8-bit register r, and stores the result back into the A register.
void add_b(CPU *cpu, Memory *memory); // 0x80
void add_c(CPU *cpu, Memory *memory); // 0x81
void add_d(CPU *cpu, Memory *memory); // 0x82
void add_e(CPU *cpu, Memory *memory); // 0x83
void add_h(CPU *cpu, Memory *memory); // 0x84
void add_l(CPU *cpu, Memory *memory); // 0x85
void add_a(CPU *cpu, Memory *memory); // 0x87

// Adds to the 8-bit A register, data from the absolute address specified by the 16-bit register HL, and stores the result back into the A register.
void add_hl(CPU *cpu, Memory *memory); // 0x86

void add_n(CPU *cpu, Memory *memory); // 0xC6



// Adds to the 8-bit A register, the carry flag and the 8-bit register r, and stores the result back into the A register.
void adc_b(CPU *cpu, Memory *memory); // 0x88
void adc_c(CPU *cpu, Memory *memory); // 0x89
void adc_d(CPU *cpu, Memory *memory); // 0x8A
void adc_e(CPU *cpu, Memory *memory); // 0x8B
void adc_h(CPU *cpu, Memory *memory); // 0x8C
void adc_l(CPU *cpu, Memory *memory); // 0x8D
void adc_a(CPU *cpu, Memory *memory); // 0x8F

// Adds to the 8-bit A register, the carry flag and data from the absolute address specified by the 16-bit register HL, and stores the result back into the A register. 
void adc_hl(CPU *cpu, Memory *memory); // 0x8E

void adc_n(CPU *cpu, Memory *memory); // 0xCE



/*
---------Subtract instructions---------
*/

// Subtracts from the 8-bit A register, the 8-bit register r, and stores the result back into the A register.
void sub_b(CPU *cpu, Memory *memory); // 0x90
void sub_c(CPU *cpu, Memory *memory); // 0x91
void sub_d(CPU *cpu, Memory *memory); // 0x92
void sub_e(CPU *cpu, Memory *memory); // 0x93
void sub_h(CPU *cpu, Memory *memory); // 0x94
void sub_l(CPU *cpu, Memory *memory); // 0x95
void sub_a(CPU *cpu, Memory *memory); // 0x97

// Subtracts from the 8-bit A register, the 8-bit data from the absolute address specified by the 16-bit register HL, and stores the result back into the A register.
void sub_hl(CPU *cpu, Memory *memory); // 0x96

void sub_n(CPU *cpu, Memory *memory); // 0xD6


// Subtracts from the 8-bit A register, the carry flag and the 8-bit register r, and stores the result back into the A register.
void sbc_b(CPU *cpu, Memory *memory); // 0x98
void sbc_c(CPU *cpu, Memory *memory); // 0x99
void sbc_d(CPU *cpu, Memory *memory); // 0x9A
void sbc_e(CPU *cpu, Memory *memory); // 0x9B
void sbc_h(CPU *cpu, Memory *memory); // 0x9C
void sbc_l(CPU *cpu, Memory *memory); // 0x9D
void sbc_a(CPU *cpu, Memory *memory); // 0x9F

// Subtracts from the 8-bit A register, the carry flag and data from the absolute address specified by the 16-bit register HL, and stores the result back into the A register.
void sbc_hl(CPU *cpu, Memory *memory); // 0x9E

void sbc_n(CPU *cpu, Memory *memory); // 0xDE


/*
---------Other arithmetical instructions---------
*/

// Performs a logical AND between the A register and the 8-bit register r, stores the result back into the A register.
void and_b(CPU *cpu, Memory *memory); // 0xA0
void and_c(CPU *cpu, Memory *memory); // 0xA1
void and_d(CPU *cpu, Memory *memory); // 0xA2
void and_e(CPU *cpu, Memory *memory); // 0xA3
void and_h(CPU *cpu, Memory *memory); // 0xA4
void and_l(CPU *cpu, Memory *memory); // 0xA5
void and_a(CPU *cpu, Memory *memory); // 0xA7

void and_hl(CPU *cpu, Memory *memory); // 0xA6

void and_n(CPU *cpu, Memory *memory); // 0xE6



// Performs a logical XOR between the A register and the 8-bit register r, stores the result back into the A register.
void xor_b(CPU *cpu, Memory *memory); // 0xA8
void xor_c(CPU *cpu, Memory *memory); // 0xA9
void xor_d(CPU *cpu, Memory *memory); // 0xAA
void xor_e(CPU *cpu, Memory *memory); // 0xAB
void xor_h(CPU *cpu, Memory *memory); // 0xAC
void xor_l(CPU *cpu, Memory *memory); // 0xAD
void xor_a(CPU *cpu, Memory *memory); // 0xAF

void xor_hl(CPU *cpu, Memory *memory); // 0xAE

void xor_n(CPU *cpu, Memory *memory); // 0xEE



// Performs a logical OR between the A register and the 8-bit register r, stores the result back into the A register.
void or_b(CPU *cpu, Memory *memory); // 0xB0
void or_c(CPU *cpu, Memory *memory); // 0xB1
void or_d(CPU *cpu, Memory *memory); // 0xB2
void or_e(CPU *cpu, Memory *memory); // 0xB3
void or_h(CPU *cpu, Memory *memory); // 0xB4
void or_l(CPU *cpu, Memory *memory); // 0xB5
void or_a(CPU *cpu, Memory *memory); // 0xB7

void or_hl(CPU *cpu, Memory *memory); // 0xB6

void or_n(CPU *cpu, Memory *memory); // 0xF6



// Complements the A register (bitwise NOT operation).
void cp_b(CPU *cpu, Memory *memory); // 0xB8
void cp_c(CPU *cpu, Memory *memory); // 0xB9
void cp_d(CPU *cpu, Memory *memory); // 0xBA
void cp_e(CPU *cpu, Memory *memory); // 0xBB
void cp_h(CPU *cpu, Memory *memory); // 0xBC
void cp_l(CPU *cpu, Memory *memory); // 0xBD
void cp_a(CPU *cpu, Memory *memory); // 0xBF

void cp_hl(CPU *cpu, Memory *memory); // 0xBE

void cp_n(CPU *cpu, Memory *memory); // 0xFE



// Increments the 8-bit register r by 1.
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






#endif // !



