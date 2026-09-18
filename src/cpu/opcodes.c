#include "../../include/cpu/opcodes.h"

// Reaching this means either a real bug (PC ended up somewhere that isn't
// actual code - see the endianness/ROM-corruption bugs this project has
// hit before) or one of the 11 byte values the SM83 simply has no
// instruction for (0xD3, 0xDB, 0xDD, 0xE3, 0xE4, 0xEB, 0xEC, 0xED, 0xF4,
// 0xFC, 0xFD). Either way there's nothing sane to do but stop loudly
// rather than silently execute garbage.
static void op_unimplemented(CPU *cpu, Memory *mem, u8 opcode){
  fprintf(stderr, "Unimplemented opcode 0x%02X at PC=0x%04X\n", opcode, (unsigned)(cpu->pc - 1));
  exit(1);
}

// T-state cost of each non-CB opcode. For the 16 conditional jump/call/
// return opcodes this is the "not taken" cost - cpu_execute_opcode adds the
// branch bonus itself when the branch is actually taken. Unlisted (invalid)
// opcodes default to 0 and are never actually charged since they hit
// op_unimplemented, which exits.
static const u8 base_cycles[256] = {
  [0x00]=4,  [0x01]=12, [0x02]=8,  [0x03]=8,  [0x04]=4,  [0x05]=4,  [0x06]=8,  [0x07]=4,
  [0x08]=20, [0x09]=8,  [0x0A]=8,  [0x0B]=8,  [0x0C]=4,  [0x0D]=4,  [0x0E]=8,  [0x0F]=4,

  [0x10]=4,  [0x11]=12, [0x12]=8,  [0x13]=8,  [0x14]=4,  [0x15]=4,  [0x16]=8,  [0x17]=4,
  [0x18]=12, [0x19]=8,  [0x1A]=8,  [0x1B]=8,  [0x1C]=4,  [0x1D]=4,  [0x1E]=8,  [0x1F]=4,

  [0x20]=8,  [0x21]=12, [0x22]=8,  [0x23]=8,  [0x24]=4,  [0x25]=4,  [0x26]=8,  [0x27]=4,
  [0x28]=8,  [0x29]=8,  [0x2A]=8,  [0x2B]=8,  [0x2C]=4,  [0x2D]=4,  [0x2E]=8,  [0x2F]=4,

  [0x30]=8,  [0x31]=12, [0x32]=8,  [0x33]=8,  [0x34]=12, [0x35]=12, [0x36]=12, [0x37]=4,
  [0x38]=8,  [0x39]=8,  [0x3A]=8,  [0x3B]=8,  [0x3C]=4,  [0x3D]=4,  [0x3E]=8,  [0x3F]=4,

  [0x40]=4,  [0x41]=4,  [0x42]=4,  [0x43]=4,  [0x44]=4,  [0x45]=4,  [0x46]=8,  [0x47]=4,
  [0x48]=4,  [0x49]=4,  [0x4A]=4,  [0x4B]=4,  [0x4C]=4,  [0x4D]=4,  [0x4E]=8,  [0x4F]=4,

  [0x50]=4,  [0x51]=4,  [0x52]=4,  [0x53]=4,  [0x54]=4,  [0x55]=4,  [0x56]=8,  [0x57]=4,
  [0x58]=4,  [0x59]=4,  [0x5A]=4,  [0x5B]=4,  [0x5C]=4,  [0x5D]=4,  [0x5E]=8,  [0x5F]=4,

  [0x60]=4,  [0x61]=4,  [0x62]=4,  [0x63]=4,  [0x64]=4,  [0x65]=4,  [0x66]=8,  [0x67]=4,
  [0x68]=4,  [0x69]=4,  [0x6A]=4,  [0x6B]=4,  [0x6C]=4,  [0x6D]=4,  [0x6E]=8,  [0x6F]=4,

  [0x70]=8,  [0x71]=8,  [0x72]=8,  [0x73]=8,  [0x74]=8,  [0x75]=8,  [0x76]=4,  [0x77]=8,
  [0x78]=4,  [0x79]=4,  [0x7A]=4,  [0x7B]=4,  [0x7C]=4,  [0x7D]=4,  [0x7E]=8,  [0x7F]=4,

  [0x80]=4,  [0x81]=4,  [0x82]=4,  [0x83]=4,  [0x84]=4,  [0x85]=4,  [0x86]=8,  [0x87]=4,
  [0x88]=4,  [0x89]=4,  [0x8A]=4,  [0x8B]=4,  [0x8C]=4,  [0x8D]=4,  [0x8E]=8,  [0x8F]=4,

  [0x90]=4,  [0x91]=4,  [0x92]=4,  [0x93]=4,  [0x94]=4,  [0x95]=4,  [0x96]=8,  [0x97]=4,
  [0x98]=4,  [0x99]=4,  [0x9A]=4,  [0x9B]=4,  [0x9C]=4,  [0x9D]=4,  [0x9E]=8,  [0x9F]=4,

  [0xA0]=4,  [0xA1]=4,  [0xA2]=4,  [0xA3]=4,  [0xA4]=4,  [0xA5]=4,  [0xA6]=8,  [0xA7]=4,
  [0xA8]=4,  [0xA9]=4,  [0xAA]=4,  [0xAB]=4,  [0xAC]=4,  [0xAD]=4,  [0xAE]=8,  [0xAF]=4,

  [0xB0]=4,  [0xB1]=4,  [0xB2]=4,  [0xB3]=4,  [0xB4]=4,  [0xB5]=4,  [0xB6]=8,  [0xB7]=4,
  [0xB8]=4,  [0xB9]=4,  [0xBA]=4,  [0xBB]=4,  [0xBC]=4,  [0xBD]=4,  [0xBE]=8,  [0xBF]=4,

  [0xC0]=8,  [0xC1]=12, [0xC2]=12, [0xC3]=16, [0xC4]=12, [0xC5]=16, [0xC6]=8,  [0xC7]=16,
  [0xC8]=8,  [0xC9]=16, [0xCA]=12, [0xCB]=4,  [0xCC]=12, [0xCD]=24, [0xCE]=8,  [0xCF]=16,

  [0xD0]=8,  [0xD1]=12, [0xD2]=12, [0xD4]=12, [0xD5]=16, [0xD6]=8,  [0xD7]=16,
  [0xD8]=8,  [0xD9]=16, [0xDA]=12, [0xDC]=12, [0xDE]=8,  [0xDF]=16,

  [0xE0]=12, [0xE1]=12, [0xE2]=8,  [0xE5]=16, [0xE6]=8,  [0xE7]=16, [0xE8]=16, [0xE9]=4,
  [0xEA]=16, [0xEE]=8,  [0xEF]=16,

  [0xF0]=12, [0xF1]=12, [0xF2]=8,  [0xF3]=4,  [0xF5]=16, [0xF6]=8,  [0xF7]=16,
  [0xF8]=12, [0xF9]=8,  [0xFA]=16, [0xFB]=4,  [0xFE]=8,  [0xFF]=16,
};

// The full opcode -> handler mapping. Most cases are a single call whose
// cycle cost is just whatever base_cycles[opcode] already says; the three
// exceptions are commented at their case labels below:
//   - the 16 conditional jump/call/return opcodes, which report back
//     (via their bool return value) whether they actually branched, so we
//     can add the extra cycles a taken branch costs;
//   - 0xCB, whose own handler (op_prefix) reads a second opcode byte and
//     returns *that* instruction's cost, to be added on top of the
//     4-cycle base_cycles[0xCB] (the prefix fetch itself).
u8 cpu_execute_opcode(CPU *cpu, Memory *mem, u8 opcode){
  u8 cycles = base_cycles[opcode];

  switch (opcode) {
    // 0x0_
    case 0x00: op_nop(cpu, mem); break;
    case 0x01: ld_bc_n16(cpu, mem); break;
    case 0x02: ld_bc_a(cpu, mem); break;
    case 0x03: inc_bc(cpu, mem); break;
    case 0x04: inc_b(cpu, mem); break;
    case 0x05: dec_b(cpu, mem); break;
    case 0x06: ld_b_n(cpu, mem); break;
    case 0x07: rlca(cpu, mem); break;
    case 0x08: ld_nn_sp(cpu, mem); break;
    case 0x09: add_hl_bc(cpu, mem); break;
    case 0x0A: ld_a_bc(cpu, mem); break;
    case 0x0B: dec_bc(cpu, mem); break;
    case 0x0C: inc_c(cpu, mem); break;
    case 0x0D: dec_c(cpu, mem); break;
    case 0x0E: ld_c_n(cpu, mem); break;
    case 0x0F: rrca(cpu, mem); break;

    // 0x1_
    case 0x10: op_stop(cpu, mem); break;
    case 0x11: ld_de_n16(cpu, mem); break;
    case 0x12: ld_de_a(cpu, mem); break;
    case 0x13: inc_de(cpu, mem); break;
    case 0x14: inc_d(cpu, mem); break;
    case 0x15: dec_d(cpu, mem); break;
    case 0x16: ld_d_n(cpu, mem); break;
    case 0x17: rla(cpu, mem); break;
    case 0x18: jr_e(cpu, mem); break;
    case 0x19: add_hl_de(cpu, mem); break;
    case 0x1A: ld_a_de(cpu, mem); break;
    case 0x1B: dec_de(cpu, mem); break;
    case 0x1C: inc_e(cpu, mem); break;
    case 0x1D: dec_e(cpu, mem); break;
    case 0x1E: ld_e_n(cpu, mem); break;
    case 0x1F: rra(cpu, mem); break;

    // 0x2_
    case 0x20: if (jr_nz_e(cpu, mem)) cycles += 4; break;
    case 0x21: ld_hl_n16(cpu, mem); break;
    case 0x22: ld_hl_inc_a(cpu, mem); break;
    case 0x23: inc_hl16(cpu, mem); break;
    case 0x24: inc_h(cpu, mem); break;
    case 0x25: dec_h(cpu, mem); break;
    case 0x26: ld_h_n(cpu, mem); break;
    case 0x27: daa(cpu, mem); break;
    case 0x28: if (jr_z_e(cpu, mem)) cycles += 4; break;
    case 0x29: add_hl_hl(cpu, mem); break;
    case 0x2A: ld_a_hl_inc(cpu, mem); break;
    case 0x2B: dec_hl16(cpu, mem); break;
    case 0x2C: inc_l(cpu, mem); break;
    case 0x2D: dec_l(cpu, mem); break;
    case 0x2E: ld_l_n(cpu, mem); break;
    case 0x2F: cpl(cpu, mem); break;

    // 0x3_
    case 0x30: if (jr_nc_e(cpu, mem)) cycles += 4; break;
    case 0x31: ld_sp_n16(cpu, mem); break;
    case 0x32: ld_hl_dec_a(cpu, mem); break;
    case 0x33: inc_sp(cpu, mem); break;
    case 0x34: inc_hl(cpu, mem); break;
    case 0x35: dec_hl(cpu, mem); break;
    case 0x36: ld_hl_n(cpu, mem); break;
    case 0x37: scf(cpu, mem); break;
    case 0x38: if (jr_c_e(cpu, mem)) cycles += 4; break;
    case 0x39: add_hl_sp(cpu, mem); break;
    case 0x3A: ld_a_hl_dec(cpu, mem); break;
    case 0x3B: dec_sp(cpu, mem); break;
    case 0x3C: inc_a(cpu, mem); break;
    case 0x3D: dec_a(cpu, mem); break;
    case 0x3E: ld_a_n(cpu, mem); break;
    case 0x3F: ccf(cpu, mem); break;

    // 0x4_ - 0x7_ : 8-bit register loads (0x76 = HALT)
    case 0x40: ld_b_b(cpu, mem); break;
    case 0x41: ld_b_c(cpu, mem); break;
    case 0x42: ld_b_d(cpu, mem); break;
    case 0x43: ld_b_e(cpu, mem); break;
    case 0x44: ld_b_h(cpu, mem); break;
    case 0x45: ld_b_l(cpu, mem); break;
    case 0x46: ld_b_hl(cpu, mem); break;
    case 0x47: ld_b_a(cpu, mem); break;
    case 0x48: ld_c_b(cpu, mem); break;
    case 0x49: ld_c_c(cpu, mem); break;
    case 0x4A: ld_c_d(cpu, mem); break;
    case 0x4B: ld_c_e(cpu, mem); break;
    case 0x4C: ld_c_h(cpu, mem); break;
    case 0x4D: ld_c_l(cpu, mem); break;
    case 0x4E: ld_c_hl(cpu, mem); break;
    case 0x4F: ld_c_a(cpu, mem); break;

    case 0x50: ld_d_b(cpu, mem); break;
    case 0x51: ld_d_c(cpu, mem); break;
    case 0x52: ld_d_d(cpu, mem); break;
    case 0x53: ld_d_e(cpu, mem); break;
    case 0x54: ld_d_h(cpu, mem); break;
    case 0x55: ld_d_l(cpu, mem); break;
    case 0x56: ld_d_hl(cpu, mem); break;
    case 0x57: ld_d_a(cpu, mem); break;
    case 0x58: ld_e_b(cpu, mem); break;
    case 0x59: ld_e_c(cpu, mem); break;
    case 0x5A: ld_e_d(cpu, mem); break;
    case 0x5B: ld_e_e(cpu, mem); break;
    case 0x5C: ld_e_h(cpu, mem); break;
    case 0x5D: ld_e_l(cpu, mem); break;
    case 0x5E: ld_e_hl(cpu, mem); break;
    case 0x5F: ld_e_a(cpu, mem); break;

    case 0x60: ld_h_b(cpu, mem); break;
    case 0x61: ld_h_c(cpu, mem); break;
    case 0x62: ld_h_d(cpu, mem); break;
    case 0x63: ld_h_e(cpu, mem); break;
    case 0x64: ld_h_h(cpu, mem); break;
    case 0x65: ld_h_l(cpu, mem); break;
    case 0x66: ld_h_hl(cpu, mem); break;
    case 0x67: ld_h_a(cpu, mem); break;
    case 0x68: ld_l_b(cpu, mem); break;
    case 0x69: ld_l_c(cpu, mem); break;
    case 0x6A: ld_l_d(cpu, mem); break;
    case 0x6B: ld_l_e(cpu, mem); break;
    case 0x6C: ld_l_h(cpu, mem); break;
    case 0x6D: ld_l_l(cpu, mem); break;
    case 0x6E: ld_l_hl(cpu, mem); break;
    case 0x6F: ld_l_a(cpu, mem); break;

    case 0x70: ld_hl_b(cpu, mem); break;
    case 0x71: ld_hl_c(cpu, mem); break;
    case 0x72: ld_hl_d(cpu, mem); break;
    case 0x73: ld_hl_e(cpu, mem); break;
    case 0x74: ld_hl_h(cpu, mem); break;
    case 0x75: ld_hl_l(cpu, mem); break;
    case 0x76: op_halt(cpu, mem); break;
    case 0x77: ld_hl_a(cpu, mem); break;
    case 0x78: ld_a_b(cpu, mem); break;
    case 0x79: ld_a_c(cpu, mem); break;
    case 0x7A: ld_a_d(cpu, mem); break;
    case 0x7B: ld_a_e(cpu, mem); break;
    case 0x7C: ld_a_h(cpu, mem); break;
    case 0x7D: ld_a_l(cpu, mem); break;
    case 0x7E: ld_a_hl(cpu, mem); break;
    case 0x7F: ld_a_a(cpu, mem); break;

    // 0x8_ - 0xB_ : ALU A,r
    case 0x80: add_b(cpu, mem); break;
    case 0x81: add_c(cpu, mem); break;
    case 0x82: add_d(cpu, mem); break;
    case 0x83: add_e(cpu, mem); break;
    case 0x84: add_h(cpu, mem); break;
    case 0x85: add_l(cpu, mem); break;
    case 0x86: add_hl(cpu, mem); break;
    case 0x87: add_a(cpu, mem); break;
    case 0x88: adc_b(cpu, mem); break;
    case 0x89: adc_c(cpu, mem); break;
    case 0x8A: adc_d(cpu, mem); break;
    case 0x8B: adc_e(cpu, mem); break;
    case 0x8C: adc_h(cpu, mem); break;
    case 0x8D: adc_l(cpu, mem); break;
    case 0x8E: adc_hl(cpu, mem); break;
    case 0x8F: adc_a(cpu, mem); break;

    case 0x90: sub_b(cpu, mem); break;
    case 0x91: sub_c(cpu, mem); break;
    case 0x92: sub_d(cpu, mem); break;
    case 0x93: sub_e(cpu, mem); break;
    case 0x94: sub_h(cpu, mem); break;
    case 0x95: sub_l(cpu, mem); break;
    case 0x96: sub_hl(cpu, mem); break;
    case 0x97: sub_a(cpu, mem); break;
    case 0x98: sbc_b(cpu, mem); break;
    case 0x99: sbc_c(cpu, mem); break;
    case 0x9A: sbc_d(cpu, mem); break;
    case 0x9B: sbc_e(cpu, mem); break;
    case 0x9C: sbc_h(cpu, mem); break;
    case 0x9D: sbc_l(cpu, mem); break;
    case 0x9E: sbc_hl(cpu, mem); break;
    case 0x9F: sbc_a(cpu, mem); break;

    case 0xA0: and_b(cpu, mem); break;
    case 0xA1: and_c(cpu, mem); break;
    case 0xA2: and_d(cpu, mem); break;
    case 0xA3: and_e(cpu, mem); break;
    case 0xA4: and_h(cpu, mem); break;
    case 0xA5: and_l(cpu, mem); break;
    case 0xA6: and_hl(cpu, mem); break;
    case 0xA7: and_a(cpu, mem); break;
    case 0xA8: xor_b(cpu, mem); break;
    case 0xA9: xor_c(cpu, mem); break;
    case 0xAA: xor_d(cpu, mem); break;
    case 0xAB: xor_e(cpu, mem); break;
    case 0xAC: xor_h(cpu, mem); break;
    case 0xAD: xor_l(cpu, mem); break;
    case 0xAE: xor_hl(cpu, mem); break;
    case 0xAF: xor_a(cpu, mem); break;

    case 0xB0: or_b(cpu, mem); break;
    case 0xB1: or_c(cpu, mem); break;
    case 0xB2: or_d(cpu, mem); break;
    case 0xB3: or_e(cpu, mem); break;
    case 0xB4: or_h(cpu, mem); break;
    case 0xB5: or_l(cpu, mem); break;
    case 0xB6: or_hl(cpu, mem); break;
    case 0xB7: or_a(cpu, mem); break;
    case 0xB8: cp_b(cpu, mem); break;
    case 0xB9: cp_c(cpu, mem); break;
    case 0xBA: cp_d(cpu, mem); break;
    case 0xBB: cp_e(cpu, mem); break;
    case 0xBC: cp_h(cpu, mem); break;
    case 0xBD: cp_l(cpu, mem); break;
    case 0xBE: cp_hl(cpu, mem); break;
    case 0xBF: cp_a(cpu, mem); break;

    // 0xC_
    case 0xC0: if (ret_nz(cpu, mem)) cycles += 12; break;
    case 0xC1: pop_bc(cpu, mem); break;
    case 0xC2: if (jp_nz_nn(cpu, mem)) cycles += 4; break;
    case 0xC3: jp_nn(cpu, mem); break;
    case 0xC4: if (call_nz_nn(cpu, mem)) cycles += 12; break;
    case 0xC5: push_bc(cpu, mem); break;
    case 0xC6: add_n(cpu, mem); break;
    case 0xC7: rst_0x00(cpu, mem); break;
    case 0xC8: if (ret_z(cpu, mem)) cycles += 12; break;
    case 0xC9: ret(cpu, mem); break;
    case 0xCA: if (jp_z_nn(cpu, mem)) cycles += 4; break;
    case 0xCB: cycles += op_prefix(cpu, mem); break;
    case 0xCC: if (call_z_nn(cpu, mem)) cycles += 12; break;
    case 0xCD: call_nn(cpu, mem); break;
    case 0xCE: adc_n(cpu, mem); break;
    case 0xCF: rst_0x08(cpu, mem); break;

    // 0xD_
    case 0xD0: if (ret_nc(cpu, mem)) cycles += 12; break;
    case 0xD1: pop_de(cpu, mem); break;
    case 0xD2: if (jp_nc_nn(cpu, mem)) cycles += 4; break;
    case 0xD4: if (call_nc_nn(cpu, mem)) cycles += 12; break;
    case 0xD5: push_de(cpu, mem); break;
    case 0xD6: sub_n(cpu, mem); break;
    case 0xD7: rst_0x10(cpu, mem); break;
    case 0xD8: if (ret_c(cpu, mem)) cycles += 12; break;
    case 0xD9: reti(cpu, mem); break;
    case 0xDA: if (jp_c_nn(cpu, mem)) cycles += 4; break;
    case 0xDC: if (call_c_nn(cpu, mem)) cycles += 12; break;
    case 0xDE: sbc_n(cpu, mem); break;
    case 0xDF: rst_0x18(cpu, mem); break;

    // 0xE_
    case 0xE0: ld_ff00_n_a(cpu, mem); break;
    case 0xE1: pop_hl(cpu, mem); break;
    case 0xE2: ld_ff00_c_a(cpu, mem); break;
    case 0xE5: push_hl(cpu, mem); break;
    case 0xE6: and_n(cpu, mem); break;
    case 0xE7: rst_0x20(cpu, mem); break;
    case 0xE8: add_sp_e(cpu, mem); break;
    case 0xE9: jp_hl(cpu, mem); break;
    case 0xEA: ld_nn_a(cpu, mem); break;
    case 0xEE: xor_n(cpu, mem); break;
    case 0xEF: rst_0x28(cpu, mem); break;

    // 0xF_
    case 0xF0: ld_a_ff00_n(cpu, mem); break;
    case 0xF1: pop_af(cpu, mem); break;
    case 0xF2: ld_a_ff00_c(cpu, mem); break;
    case 0xF3: op_di(cpu, mem); break;
    case 0xF5: push_af(cpu, mem); break;
    case 0xF6: or_n(cpu, mem); break;
    case 0xF7: rst_0x30(cpu, mem); break;
    case 0xF8: ld_hl_sp(cpu, mem); break;
    case 0xF9: ld_sp_hl(cpu, mem); break;
    case 0xFA: ld_a_nn(cpu, mem); break;
    case 0xFB: op_ei(cpu, mem); break;
    case 0xFE: cp_n(cpu, mem); break;
    case 0xFF: rst_0x38(cpu, mem); break;

    // 0xD3, 0xDB, 0xDD, 0xE3, 0xE4, 0xEB, 0xEC, 0xED, 0xF4, 0xFC, 0xFD do not
    // exist on the SM83 and fall through to the unimplemented handler below.
    default:
      op_unimplemented(cpu, mem, opcode);
      return 0; // unreachable: op_unimplemented exits
  }

  return cycles;
}
