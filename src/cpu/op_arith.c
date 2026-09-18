#include "../../include/cpu/op_arith.h"


/*
---------Add instructions---------
*/

// Shared core for every 8-bit ADD (register, (HL), and immediate all
// eventually call this). H and C are computed *before* the add actually
// happens, by checking the low nibble and full byte in a wider (u16) type
// so the "did this overflow" comparison can't itself overflow.
void add_r8(CPU *cpu, u8 r){
    u8 h = ((cpu->a & 0x0F) + (r & 0x0F)) > 0x0F; // Carry from bit 3
    u8 c = ((u16)cpu->a + (u16)r) > 0xFF; // Carry from bit 7
    cpu->a += r;
    cpu->f = 0; // ADD always recomputes every flag from scratch, unlike INC/DEC
    if(h) cpu->f |= H_FLAG;
    if(c) cpu->f |= C_FLAG;
    if(cpu->a == 0) cpu->f |= Z_FLAG;
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



// Same as add_r8, but also adds in the previous carry. The one detail that
// actually matters here: `carry_in` must be normalized to 0 or 1 before
// use. C_FLAG is bit 4 (value 0x10) of F, so `cpu->f & C_FLAG` alone is
// either 0 or 0x10 - adding *that* directly would add 16 instead of 1
// whenever the carry was set, which was a real bug this project had.
void adc_r(CPU *cpu, u8 r){
    u8 carry_in = (cpu->f & C_FLAG) ? 1 : 0;
    u8 h = ((cpu->a & 0x0F) + (r & 0x0F) + carry_in) > 0x0F; // Carry from bit 3
    u8 c = ((u16)cpu->a + (u16)r + carry_in) > 0xFF; // Carry from bit 7
    cpu->a = cpu->a + r + carry_in;
    cpu->f = 0;
    if(h) cpu->f |= H_FLAG;
    if(c) cpu->f |= C_FLAG;
    if(cpu->a == 0) cpu->f |= Z_FLAG;
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

// Shared core for every 8-bit SUB. `cpu->f = N_FLAG` resets every other
// flag to 0 up front, so - unlike add_r8's separate booleans-then-OR
// approach - H/C only ever get set, never need an explicit "else clear".
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


// Same as sub_r, but also subtracts the previous carry - normalized to 0/1
// for the same reason described in adc_r above.
void sbc_r(CPU *cpu, u8 r) {
    u8 carry_in = (cpu->f & C_FLAG) ? 1 : 0;
    u8 h = (cpu->a & 0x0F) < ((r & 0x0F) + carry_in); // Borrow from bit 4
    u8 c = (u16)cpu->a < ((u16)r + carry_in); // Borrow from bit 8
    cpu->a = cpu->a - r - carry_in;
    cpu->f = N_FLAG;
    if(h) cpu->f |= H_FLAG;
    if(c) cpu->f |= C_FLAG;
    if(cpu->a == 0) cpu->f |= Z_FLAG;
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
void xor_hl(CPU *cpu, Memory *memory){ xor_r(cpu, memory_read(memory, cpu->hl));}; // 0xAE

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
void or_hl(CPU *cpu, Memory *memory){ or_r(cpu, memory_read(memory, cpu->hl));}; // 0xB6

// Performs a bitwise OR operation between the 8-bit A register and immediate data n, and stores the result back into the A register.
void or_n(CPU *cpu, Memory *memory){ or_r(cpu, memory_read(memory, cpu->pc++));}; // 0xF6



// Subtracts from the 8-bit A register, the 8-bit register r, and updates flags based on the result. This instruction is basically identical to SUB r, but does not update the A register.
void cp_r(CPU *cpu, u8 r){
    u8 result = cpu->a - r; 
    cpu->f = N_FLAG; // Set N flag
    if((cpu->a & 0x0F) < (r & 0x0F)) cpu->f |= H_FLAG; // Set H flag if there is a borrow from bit 4
    if(cpu->a < r) cpu->f |= C_FLAG; // Set C flag if there is a borrow from bit 8
    if(result == 0) cpu->f |= Z_FLAG; // Set Z flag if result is zero
}

void cp_b(CPU *cpu, Memory *memory){cp_r(cpu, cpu->b);}; // 0xB8
void cp_c(CPU *cpu, Memory *memory){cp_r(cpu, cpu->c);}; // 0xB9
void cp_d(CPU *cpu, Memory *memory){cp_r(cpu, cpu->d);}; // 0xBA
void cp_e(CPU *cpu, Memory *memory){cp_r(cpu, cpu->e);}; // 0xBB
void cp_h(CPU *cpu, Memory *memory){cp_r(cpu, cpu->h);}; // 0xBC
void cp_l(CPU *cpu, Memory *memory){cp_r(cpu, cpu->l);}; // 0xBD
void cp_a(CPU *cpu, Memory *memory){cp_r(cpu, cpu->a);}; // 0xBF

// Subtracts from the 8-bit A register, data from the absolute address specified by the 16-bit register HL, and updates flags based on the result. This instruction is basically identical to SUB (HL), but does not update the A register
void cp_hl(CPU *cpu, Memory *memory){ cp_r(cpu, memory_read(memory, cpu->hl));}; // 0xBE

// Subtracts from the 8-bit A register, the immediate data n, and updates flags based on the result. This instruction is basically identical to SUB n, but does not update the A register.
void cp_n(CPU *cpu, Memory *memory){ cp_r(cpu, memory_read(memory, cpu->pc++));}; // 0xFE



// Increments the 8-bit register r by 1. Unlike ADD, INC doesn't touch C -
// so this can't just reset cpu->f to 0 first like add_r8 does; H and Z
// must each be explicitly set *and* cleared (both branches of both `if`s
// below), otherwise a flag left set by some earlier instruction would
// incorrectly "leak" into this one's result whenever the new condition is
// false. (An earlier version of this code only ever set these flags, never
// cleared them, which was a real bug.)
void inc_r(CPU *cpu, u8 *r){
    cpu->f &= ~N_FLAG; // Clear N flag
    if((*r & 0x0F) == 0x0F) cpu->f |= H_FLAG; else cpu->f &= ~H_FLAG; // H flag if carry from bit 3
    (*r)++;
    if(*r == 0) cpu->f |= Z_FLAG; else cpu->f &= ~Z_FLAG; // Z flag if result is zero
}

void inc_b(CPU *cpu, Memory *memory){inc_r(cpu, &cpu->b);}; // 0x04
void inc_c(CPU *cpu, Memory *memory){inc_r(cpu, &cpu->c);}; // 0x0C
void inc_d(CPU *cpu, Memory *memory){inc_r(cpu, &cpu->d);}; // 0x14
void inc_e(CPU *cpu, Memory *memory){inc_r(cpu, &cpu->e);}; // 0x1C
void inc_h(CPU *cpu, Memory *memory){inc_r(cpu, &cpu->h);}; // 0x24
void inc_l(CPU *cpu, Memory *memory){inc_r(cpu, &cpu->l);}; // 0x2C
void inc_a(CPU *cpu, Memory *memory){inc_r(cpu, &cpu->a);}; // 0x3C

// Increments the data at the memory address specified by the 16-bit HL register pair.
void inc_hl(CPU *cpu, Memory *memory){
  cpu->f &= ~N_FLAG; // Clear N flag
  u8 data = memory_read(memory, cpu->hl);
  if((data & 0x0F) == 0x0F) cpu->f |= H_FLAG; else cpu->f &= ~H_FLAG; // H flag if carry from bit 3
  data += 1;
  memory_write(memory, cpu->hl, data);
  if(data == 0) cpu->f |= Z_FLAG; else cpu->f &= ~Z_FLAG; // Z flag if result is zero
}; // 0x34

// Decrements the 8-bit register r by 1.
void dec_r(CPU *cpu, u8 *r){
    cpu->f |= N_FLAG; 
    if((*r & 0x0F) == 0x00) cpu->f |= H_FLAG; 
    else cpu->f &= ~H_FLAG;
    (*r)--;
    if(*r == 0) cpu->f |= Z_FLAG; 
    else cpu->f &= ~Z_FLAG; 
}

void dec_b(CPU *cpu, Memory *memory){dec_r(cpu, &cpu->b);}; // 0x05
void dec_c(CPU *cpu, Memory *memory){dec_r(cpu, &cpu->c);}; // 0x0D
void dec_d(CPU *cpu, Memory *memory){dec_r(cpu, &cpu->d);}; // 0x15
void dec_e(CPU *cpu, Memory *memory){dec_r(cpu, &cpu->e);}; // 0x1D
void dec_h(CPU *cpu, Memory *memory){dec_r(cpu, &cpu->h);}; // 0x25
void dec_l(CPU *cpu, Memory *memory){dec_r(cpu, &cpu->l);}; // 0x2D
void dec_a(CPU *cpu, Memory *memory){dec_r(cpu, &cpu->a);}; // 0x3D

// Decrements the data at the memory address specified by the 16-bit HL register pair.
void dec_hl(CPU *cpu, Memory *memory){ 
  cpu->f |= N_FLAG; // Set N flag
  u8 data = memory_read(memory, cpu->hl);
  if((data & 0x0F) == 0x00) cpu->f |= H_FLAG; // Set H flag if there is a carry from bit 3
  data -= 1;
  memory_write(memory, cpu->hl, data);
  if(data == 0) cpu->f |= Z_FLAG; // Set Z flag if result is zero
}; // 0x35

/*
---------16-bit arithmetic instructions---------
*/

// ADD HL,rr: unlike the 8-bit ADD above, this leaves Z untouched entirely
// (per the real instruction set - only N, H, C are affected), and the
// half-carry checks bit 11 -> 12 (the boundary between the two *bytes* of
// a 16-bit value), not bit 3 -> 4 like the 8-bit versions.
void add_hl_rr(CPU *cpu, u16 rr){
  cpu->f &= ~N_FLAG;
  if(((cpu->hl & 0x0FFF) + (rr & 0x0FFF)) > 0x0FFF) cpu->f |= H_FLAG; else cpu->f &= ~H_FLAG;
  if(((u32)cpu->hl + (u32)rr) > 0xFFFF) cpu->f |= C_FLAG; else cpu->f &= ~C_FLAG;
  cpu->hl += rr;
}

void add_hl_bc(CPU *cpu, Memory *memory){ add_hl_rr(cpu, cpu->bc); }; // 0x09
void add_hl_de(CPU *cpu, Memory *memory){ add_hl_rr(cpu, cpu->de); }; // 0x19
void add_hl_hl(CPU *cpu, Memory *memory){ add_hl_rr(cpu, cpu->hl); }; // 0x29
void add_hl_sp(CPU *cpu, Memory *memory){ add_hl_rr(cpu, cpu->sp); }; // 0x39

// Adds to the 16-bit SP register the signed 8-bit operand e.
//
// The offset is genuinely signed for the actual addition (a negative e
// must be able to move SP downward), but H/C are - by the real hardware's
// own defined behavior - computed treating e as *unsigned*, i.e. as if
// this were an 8-bit add of SP's low byte and the raw operand byte. This
// looks inconsistent but matches real SM83 flag behavior for both this
// instruction and LD HL,SP+e below, so it's intentional, not a shortcut.
void add_sp_e(CPU *cpu, Memory *memory){
  int8_t e = (int8_t)memory_read(memory, cpu->pc); cpu->pc++;
  u8 h = ((cpu->sp & 0xF) + (e & 0xF)) > 0xF;
  u8 c = ((cpu->sp & 0xFF) + (e & 0xFF)) > 0xFF;
  cpu->sp = (u16)(cpu->sp + e);

  cpu->f = 0; // Z and N are always cleared for this instruction
  cpu->f |= (h << 5);
  cpu->f |= (c << 4);
}; // 0xE8

void inc_bc(CPU *cpu, Memory *memory){ cpu->bc++; }; // 0x03
void inc_de(CPU *cpu, Memory *memory){ cpu->de++; }; // 0x13
void inc_hl16(CPU *cpu, Memory *memory){ cpu->hl++; }; // 0x23
void inc_sp(CPU *cpu, Memory *memory){ cpu->sp++; }; // 0x33

void dec_bc(CPU *cpu, Memory *memory){ cpu->bc--; }; // 0x0B
void dec_de(CPU *cpu, Memory *memory){ cpu->de--; }; // 0x1B
void dec_hl16(CPU *cpu, Memory *memory){ cpu->hl--; }; // 0x2B
void dec_sp(CPU *cpu, Memory *memory){ cpu->sp--; }; // 0x3B

// Decimal Adjust Accumulator - probably the single most confusing opcode
// on this CPU, so worth spelling out. Old software often stored numbers
// in BCD (binary-coded decimal): each nibble of a byte holds one decimal
// digit 0-9, so 0x47 means the decimal number "47", not 71. The CPU's
// ADD/SUB instructions don't know anything about BCD - they just do
// normal binary arithmetic - so after adding or subtracting two BCD
// bytes, the raw binary result can land on a value that isn't a valid
// BCD number (e.g. 0x09 + 0x01 = 0x0A in binary, but the correct BCD
// answer is 0x10). DAA is meant to run immediately after that ADD/SUB and
// nudge the result back into valid BCD by adding or subtracting 0x06
// and/or 0x60 wherever a nibble spilled over 9 (or a half/full carry
// already flagged that it did).
//
// It has to know whether the *previous* instruction was an add or a
// subtract (N_FLAG, set by SUB/DEC and cleared by ADD/INC) because the
// correction goes the opposite direction each way.
void daa(CPU *cpu, Memory *memory){
  u8 a = cpu->a;
  u8 adjust = 0;
  bool carry = (cpu->f & C_FLAG) != 0;

  if (cpu->f & N_FLAG) {
    // Previous op was a subtraction: undo an over-correction downward.
    if (cpu->f & H_FLAG) adjust |= 0x06;
    if (carry) adjust |= 0x60;
    a -= adjust;
  } else {
    // Previous op was an addition: a nibble > 9 (or a flagged carry into
    // it) means that digit needs +6 to skip the 6 non-decimal values
    // (0xA-0xF) it could otherwise land on.
    if ((cpu->f & H_FLAG) || (a & 0x0F) > 0x09) adjust |= 0x06;
    if (carry || a > 0x99) { adjust |= 0x60; carry = true; }
    a += adjust;
  }

  cpu->f &= ~(H_FLAG | Z_FLAG | C_FLAG); // H is always cleared; N is left as-is
  if (carry) cpu->f |= C_FLAG;
  if (a == 0) cpu->f |= Z_FLAG;
  cpu->a = a;
}; // 0x27

// Sets the carry flag, and clears the N and H flags.
void scf(CPU *cpu, Memory *memory){
  cpu->f |= C_FLAG;
  cpu->f &= ~N_FLAG;
  cpu->f &= ~H_FLAG;
}; // 0x37

// Flips all the bits in the 8-bit A register, and sets the N and H flags.
void cpl(CPU *cpu, Memory *memory){
  cpu->a = ~cpu->a;
  cpu->f |= N_FLAG;
  cpu->f |= H_FLAG;
}; // 0x2f

// Flips all the bits in the 8-bit A register, and sets the N and H flags.
void ccf(CPU *cpu, Memory *memory){
  cpu->f ^= C_FLAG;
  cpu->f &= ~N_FLAG;
  cpu->f &= ~H_FLAG;
}; // 0x3f







