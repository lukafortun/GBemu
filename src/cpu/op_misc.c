#include "../../include/cpu/opcodes.h"

// Does nothing.
void op_nop(CPU *cpu, Memory *mem){}; // 0x00

// STOP is a 2-byte instruction (opcode + one padding byte the real
// hardware also fetches but ignores) - that byte still has to be skipped
// here or the next fetch would decode it as a totally different opcode.
// NOTE: this does not yet implement the actual stop/low-power behavior
// (freezing the CPU and LCD until a button press); it's currently a NOP
// with the mandatory operand skip.
void op_stop(CPU *cpu, Memory *mem){
  cpu->pc++;
}; // 0x10

// Enters low-power mode until an interrupt occurs.
void op_halt(CPU *cpu, Memory *mem){
  cpu->halted = true;
}; // 0x76

// Enables interrupts (takes effect after the next instruction).
void op_ei(CPU *cpu, Memory *mem){
  cpu->ime_pending = true;
}; // 0xFB

// Disables interrupts immediately.
void op_di(CPU *cpu, Memory *mem){
  cpu->ime = false;
  cpu->ime_pending = false;
}; // 0xF3

// Dispatches a CB-prefixed (rotate/shift/bit) opcode.
u8 op_prefix(CPU *cpu, Memory *mem){
  u8 cb_opcode = memory_read(mem, cpu->pc); cpu->pc++;
  return cpu_execute_cb_opcode(cpu, mem, cb_opcode);
}; // 0xCB
