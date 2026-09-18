#ifndef PPU_H
#define PPU_H

#include "common.h"
#include "memory.h"
#include <stdbool.h>

// LCDC: master control. bit7 LCD on/off, bit6 window tile map (0=0x9800,
// 1=0x9C00), bit5 window on/off, bit4 BG/window tile data addressing mode
// (0=signed, based at 0x9000; 1=unsigned, based at 0x8000), bit3 BG tile
// map (same encoding as bit6), bit2 sprite height (0=8x8, 1=8x16), bit1
// sprites on/off, bit0 BG/window on/off.
#define LCDC_ADDR 0xFF40
// STAT: bits 0-1 current mode (see MODE_* below), bit2 "LY==LYC" flag.
#define STAT_ADDR 0xFF41
#define SCY_ADDR  0xFF42 // background scroll Y
#define SCX_ADDR  0xFF43 // background scroll X
#define LY_ADDR   0xFF44 // current scanline (0-153); read-only from the CPU's view
#define LYC_ADDR  0xFF45 // compare value for the STAT bit2 flag
#define BGP_ADDR  0xFF47 // background/window palette: four 2-bit shade slots
#define OBP0_ADDR 0xFF48 // sprite palette 0 (slot 0 is always transparent)
#define OBP1_ADDR 0xFF49 // sprite palette 1
#define WY_ADDR   0xFF4A // window top-left Y
#define WX_ADDR   0xFF4B // window top-left X, offset by +7 for hardware reasons

typedef struct {
  u8 framebuffer[SCREEN_HEIGHT][SCREEN_WIDTH][3]; // RGB, top-down
  int dot_counter; // T-states elapsed in the current scanline phase
  int window_line; // internal window line counter, advances only on rows it draws
  bool frame_ready; // true for the call in which VBlank (LY==144) begins
} PPU;

// Clears the framebuffer to the lightest palette shade and resets internal
// timing state. Call once before the first ppu_step.
void ppu_init(PPU *ppu);

// Advances the PPU by `cycles` T-states: steps LCDC mode/LY, renders
// completed background scanlines into the framebuffer, and requests the
// VBlank interrupt once per frame.
void ppu_step(PPU *ppu, Memory *mem, int cycles);

#endif
