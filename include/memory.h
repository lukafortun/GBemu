#ifndef MEMORY_H
#define MEMORY_H

#include "common.h"

// The full 16-bit Game Boy address space, modeled as one flat 64 KB array.
// ROM, VRAM, WRAM, OAM, I/O registers and HRAM all just live at their
// real addresses in `data`; memory_read/memory_write apply the handful of
// special-case behaviors (echo RAM, ROM write protection, OAM DMA) that a
// flat array alone can't express - see memory.c.
typedef struct {
    u8 data[GB_MEMORY_SIZE];
} Memory;

// Zeroes the whole address space. Called once at startup, before the ROM
// is loaded in.
void memory_init(Memory *mem);

// Reads one byte, transparently redirecting echo RAM (0xE000-0xFDFF) to
// its backing WRAM address (0xC000-0xDDFF).
u8 memory_read(Memory *mem, u16 address);

// Writes one byte. Ignores writes to ROM (0x0000-0x7FFF), redirects echo
// RAM like memory_read, and triggers an OAM DMA transfer as a side effect
// of writing the DMA register (0xFF46).
void memory_write(Memory *mem, u16 address, u8 value);

// Copies a cartridge image into ROM space (0x0000-0x7FFF). No MBC banking
// yet, so only the first 32 KB of a ROM-only cartridge is usable.
void memory_load_rom(Memory *mem, const u8 *rom, u32 size);

#endif
