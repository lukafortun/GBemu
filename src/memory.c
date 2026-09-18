#include "../include/memory.h"

// Zeroes every byte of the address space (RAM, registers, and the ROM area
// too - the real ROM image gets copied in afterward by memory_load_rom).
void memory_init(Memory *mem) {
    for (int i = 0; i < GB_MEMORY_SIZE; i++) {
        mem->data[i] = 0;
    }
}

// 0xE000-0xFDFF (echo RAM) mirrors 0xC000-0xDDFF.
static u16 resolve_address(u16 address){
    if (address >= 0xE000 && address <= 0xFDFF) return address - 0x2000;
    return address;
}

// Plain read - the only special case is the echo RAM redirect handled by
// resolve_address; every other address (ROM, VRAM, I/O registers, HRAM...)
// is read exactly as stored.
u8 memory_read(Memory *mem, u16 address) {
    return mem->data[resolve_address(address)];
}

// Plain write, plus the two pieces of hardware behavior a flat byte array
// can't express on its own: ROM is read-only, and writing the DMA register
// (0xFF46) is itself an instruction to the memory controller, not just data
// to store.
void memory_write(Memory *mem, u16 address, u8 value) {
    u16 resolved = resolve_address(address);

    // ROM (0x0000-0x7FFF) is physically read-only. Real MBC-equipped
    // cartridges intercept writes here for bank switching instead of
    // storing them; without MBC support yet, the correct behavior for a
    // plain ROM is to simply ignore the write rather than corrupt the
    // program by "writing" into it.
    if (resolved < 0x8000) return;

    mem->data[resolved] = value;

    // Writing to the DMA register (0xFF46) triggers a 160-byte transfer
    // from (value << 8) into OAM (0xFE00-0xFE9F). Real hardware takes
    // ~160 M-cycles and restricts other memory access meanwhile; this
    // simplified version completes it instantly on the write.
    if (resolved == 0xFF46) {
        u16 src = (u16)((u16)value << 8);
        for (u16 i = 0; i < 0xA0; i++) {
            // resolve_address on the *source* only: it can legally be
            // anywhere in the 64 KB space, including echo RAM. The
            // destination is always plain OAM, never echo-mirrored.
            mem->data[0xFE00 + i] = mem->data[resolve_address((u16)(src + i))];
        }
    }
}

// Copies up to 32 KB of cartridge data starting at 0x0000. This bypasses
// memory_write (and its ROM write-protection) since we're installing the
// ROM image itself, not something the emulated CPU wrote.
void memory_load_rom(Memory *mem, const u8 *rom, u32 size) {
    u32 n = size < 0x8000 ? size : 0x8000;
    for (u32 i = 0; i < n; i++) {
        mem->data[i] = rom[i];
    }
}
