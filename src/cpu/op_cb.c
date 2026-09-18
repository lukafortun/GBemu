#include "../../include/cpu/op_cb.h"

// Reads the operand selected by the low 3 bits of a CB opcode:
// 0=B 1=C 2=D 3=E 4=H 5=L 6=(HL) 7=A
static u8 cb_read(CPU *cpu, Memory *mem, u8 r){
  switch (r) {
    case 0: return cpu->b;
    case 1: return cpu->c;
    case 2: return cpu->d;
    case 3: return cpu->e;
    case 4: return cpu->h;
    case 5: return cpu->l;
    case 6: return memory_read(mem, cpu->hl);
    default: return cpu->a; // case 7
  }
}

static void cb_write(CPU *cpu, Memory *mem, u8 r, u8 value){
  switch (r) {
    case 0: cpu->b = value; break;
    case 1: cpu->c = value; break;
    case 2: cpu->d = value; break;
    case 3: cpu->e = value; break;
    case 4: cpu->h = value; break;
    case 5: cpu->l = value; break;
    case 6: memory_write(mem, cpu->hl, value); break;
    default: cpu->a = value; break; // case 7
  }
}

// Rotate left (circular): bit 7 -> carry and bit 0.
static u8 op_rlc(CPU *cpu, u8 v){
  u8 carry = (v >> 7) & 1;
  u8 res = (u8)((v << 1) | carry);
  cpu->f = 0;
  if (carry) cpu->f |= C_FLAG;
  if (res == 0) cpu->f |= Z_FLAG;
  return res;
}

// Rotate right (circular): bit 0 -> carry and bit 7.
static u8 op_rrc(CPU *cpu, u8 v){
  u8 carry = v & 1;
  u8 res = (u8)((v >> 1) | (carry << 7));
  cpu->f = 0;
  if (carry) cpu->f |= C_FLAG;
  if (res == 0) cpu->f |= Z_FLAG;
  return res;
}

// Rotate left through carry: old carry -> bit 0, bit 7 -> new carry.
static u8 op_rl(CPU *cpu, u8 v){
  u8 old_carry = (cpu->f & C_FLAG) ? 1 : 0;
  u8 new_carry = (v >> 7) & 1;
  u8 res = (u8)((v << 1) | old_carry);
  cpu->f = 0;
  if (new_carry) cpu->f |= C_FLAG;
  if (res == 0) cpu->f |= Z_FLAG;
  return res;
}

// Rotate right through carry: old carry -> bit 7, bit 0 -> new carry.
static u8 op_rr(CPU *cpu, u8 v){
  u8 old_carry = (cpu->f & C_FLAG) ? 1 : 0;
  u8 new_carry = v & 1;
  u8 res = (u8)((v >> 1) | (old_carry << 7));
  cpu->f = 0;
  if (new_carry) cpu->f |= C_FLAG;
  if (res == 0) cpu->f |= Z_FLAG;
  return res;
}

// Shift left, bit 0 <- 0, bit 7 -> carry.
static u8 op_sla(CPU *cpu, u8 v){
  u8 carry = (v >> 7) & 1;
  u8 res = (u8)(v << 1);
  cpu->f = 0;
  if (carry) cpu->f |= C_FLAG;
  if (res == 0) cpu->f |= Z_FLAG;
  return res;
}

// Arithmetic shift right: bit 7 unchanged, bit 0 -> carry.
static u8 op_sra(CPU *cpu, u8 v){
  u8 carry = v & 1;
  u8 res = (u8)((v >> 1) | (v & 0x80));
  cpu->f = 0;
  if (carry) cpu->f |= C_FLAG;
  if (res == 0) cpu->f |= Z_FLAG;
  return res;
}

// Swaps the low and high nibbles.
static u8 op_swap(CPU *cpu, u8 v){
  u8 res = (u8)((v << 4) | (v >> 4));
  cpu->f = 0;
  if (res == 0) cpu->f |= Z_FLAG;
  return res;
}

// Logical shift right: bit 7 <- 0, bit 0 -> carry.
static u8 op_srl(CPU *cpu, u8 v){
  u8 carry = v & 1;
  u8 res = (u8)(v >> 1);
  cpu->f = 0;
  if (carry) cpu->f |= C_FLAG;
  if (res == 0) cpu->f |= Z_FLAG;
  return res;
}

// Tests bit `bit` of v: Z set if the bit is 0. N=0, H=1, C unaffected.
static void op_bit(CPU *cpu, u8 bit, u8 v){
  cpu->f &= C_FLAG; // clear Z and N, preserve C
  cpu->f |= H_FLAG;
  if (((v >> bit) & 1) == 0) cpu->f |= Z_FLAG;
}

u8 cpu_execute_cb_opcode(CPU *cpu, Memory *mem, u8 opcode){
  u8 r = opcode & 0x07;         // operand: B,C,D,E,H,L,(HL),A
  u8 group = (opcode >> 3) & 0x07; // rotate/shift kind, or bit index for BIT/RES/SET
  u8 block = opcode >> 6;       // 0=rotate/shift 1=BIT 2=RES 3=SET
  bool is_hl = (r == 6);

  u8 value = cb_read(cpu, mem, r);

  switch (block) {
    case 0: {
      u8 result;
      switch (group) {
        case 0: result = op_rlc(cpu, value); break;
        case 1: result = op_rrc(cpu, value); break;
        case 2: result = op_rl(cpu, value); break;
        case 3: result = op_rr(cpu, value); break;
        case 4: result = op_sla(cpu, value); break;
        case 5: result = op_sra(cpu, value); break;
        case 6: result = op_swap(cpu, value); break;
        default: result = op_srl(cpu, value); break; // 7
      }
      cb_write(cpu, mem, r, result);
      break;
    }
    case 1: // BIT b,r
      op_bit(cpu, group, value);
      break;
    case 2: // RES b,r
      cb_write(cpu, mem, r, (u8)(value & ~(1u << group)));
      break;
    default: // 3: SET b,r
      cb_write(cpu, mem, r, (u8)(value | (1u << group)));
      break;
  }

  if (!is_hl) return 8;
  return (block == 1) ? 12 : 16; // BIT b,(HL) reads only: 12; others read+write: 16
}
