#include "../include/interrupts.h"

// Marks one interrupt source as pending by setting its bit in IF. Called by
// the hardware-side modules (timer, PPU, joypad) the instant their event
// happens; whether it actually fires is decided later, by interrupt_step.
void interrupt_request(Memory *mem, u8 flag){
  u8 iflag = memory_read(mem, IF_ADDR);
  memory_write(mem, IF_ADDR, iflag | flag);
}

// Fixed dispatch addresses, in priority order (index 0 = highest priority,
// checked first): VBlank, LCD STAT, Timer, Serial, Joypad. The CPU jumps
// here exactly as if a CALL had targeted this address.
static const u16 vectors[5] = {0x40, 0x48, 0x50, 0x58, 0x60};

// Checks for a pending, enabled interrupt and services the highest-priority
// one, if any. See interrupts.h for the full return-value contract.
u8 interrupt_step(CPU *cpu, Memory *mem){
  u8 ie = memory_read(mem, IE_ADDR);
  u8 iflag = memory_read(mem, IF_ADDR);
  u8 pending = ie & iflag & 0x1F;

  if (pending == 0) return 0;

  // A pending interrupt always wakes the CPU from HALT, even if IME is off.
  cpu->halted = false;

  if (!cpu->ime) return 0;

  for (int i = 0; i < 5; i++) {
    if (pending & (1 << i)) {
      cpu->ime = false;                             // re-armed by the handler's own EI/RETI
      memory_write(mem, IF_ADDR, iflag & ~(1 << i)); // acknowledge: stop requesting this one

      // Push the current PC (the return address) the same way CALL does:
      // high byte at the higher address, low byte at the lower/final one.
      cpu->sp--;
      memory_write(mem, cpu->sp, (cpu->pc >> 8) & 0xFF); cpu->sp--;
      memory_write(mem, cpu->sp, cpu->pc & 0xFF);
      cpu->pc = vectors[i];
      return 20;
    }
  }
  return 0; // unreachable: `pending` was already confirmed nonzero above
}
