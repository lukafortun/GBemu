#include "../../include/cpu/op_rot_shift.h"

// These four are the "legacy" accumulator-only rotates, distinct from the
// CB-prefixed RLC/RRC/RL/RR in op_cb.c which do the exact same bit
// operations but on any register (and also set Z when the result is 0).
// These always clear Z unconditionally regardless of the result - a quirk
// of the original Z80/8080 instruction set the Game Boy inherited.
//
// "Circular" (RLCA/RRCA) means the bit that falls off one end reappears on
// the other *and* is copied into the carry flag - carry is a side effect,
// not part of the rotation itself. "Through carry" (RLA/RRA) instead
// treats the carry flag as a 9th bit of the rotation: the old carry feeds
// in one end while the bit falling off the other end becomes the new
// carry, so a value can be rotated across multiple RLA calls without
// ever losing bits, as long as nothing else touches the carry flag
// in between.

void rlca(CPU *cpu, Memory *memory){
  u8 carry = (cpu->a >> 7) & 0x01;
  cpu->a = (cpu->a << 1) | carry;
  cpu->f = 0;
  if (carry) cpu->f |= C_FLAG;
}; // 0x07

void rrca(CPU *cpu, Memory *memory){
  u8 carry = cpu->a & 0x01;
  cpu->a = (cpu->a >> 1) | (carry << 7);
  cpu->f = 0;
  if (carry) cpu->f |= C_FLAG;
}; // 0x0F

void rla(CPU *cpu, Memory *memory){
  u8 old_carry = (cpu->f & C_FLAG) ? 1 : 0;
  u8 new_carry = (cpu->a >> 7) & 0x01;
  cpu->a = (cpu->a << 1) | old_carry;
  cpu->f = 0;
  if (new_carry) cpu->f |= C_FLAG;
}; // 0x17

void rra(CPU *cpu, Memory *memory){
  u8 old_carry = (cpu->f & C_FLAG) ? 1 : 0;
  u8 new_carry = cpu->a & 0x01;
  cpu->a = (cpu->a >> 1) | (old_carry << 7);
  cpu->f = 0;
  if (new_carry) cpu->f |= C_FLAG;
}; // 0x1F
