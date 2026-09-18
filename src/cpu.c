#include "../include/cpu/cpu.h"
#include "../include/cpu/opcodes.h"
#include "../include/interrupts.h"

// Runs one step of the classic fetch-decode-execute loop, in four stages:
//
//   1. Service a pending interrupt, if any (this can happen even while
//      halted - see interrupts.c). If it does, that dispatch *is* this
//      step; we don't also fetch an opcode in the same call.
//   2. If HALTed and no interrupt fired, do nothing but still report 4
//      T-states elapsed - the CPU is idle, but the rest of the machine
//      (timer, PPU) keeps running in real time and something on that
//      side is usually what eventually wakes it back up.
//   3. Otherwise, fetch the opcode byte at PC (advancing PC by one) and
//      hand it to the dispatcher, which reads any further operand bytes
//      itself and reports how many T-states the whole instruction cost.
//   4. Apply a pending EI: the real hardware only turns interrupts on
//      after the instruction *following* EI has finished, not EI itself -
//      that's why `ime_pending` is captured before executing and only
//      applied afterward.
//
// ref: https://gekkio.fi/files/gb-docs/gbctr.pdf, page 17
u8 fetch_cycle(CPU *cpu, Memory *mem){
  u8 interrupt_cycles = interrupt_step(cpu, mem);
  if (interrupt_cycles > 0) return interrupt_cycles; // dispatch used this slot

  if (cpu->halted) return 4; // idles, but the clock (and other components) keep running

  // EI enables interrupts only after the instruction following it has run.
  bool enable_ime_after_this = cpu->ime_pending;
  cpu->ime_pending = false;

  u8 opcode = memory_read(mem, cpu->pc);
  cpu->pc++;

  u8 cycles = cpu_execute_opcode(cpu, mem, opcode);

  if (enable_ime_after_this) cpu->ime = true;

  return cycles;
}
