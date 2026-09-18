#include "../include/ppu.h"
#include "../include/interrupts.h"

// The four STAT mode values, and how long the PPU spends in each per
// visible scanline (in T-states, at the fixed timing this emulator uses -
// real hardware's mode 3 length actually varies with sprite/window count,
// but a fixed split is a common, good-enough simplification):
//   OAM scan (80) -> pixel transfer (172) -> HBlank (204) = 456 per line.
// VBlank (10 extra scanlines, 144-153) also takes 456 T-states per line,
// during which the CPU can freely rewrite VRAM/OAM without visible glitches.
#define MODE_HBLANK   0
#define MODE_VBLANK   1
#define MODE_OAM      2
#define MODE_TRANSFER 3

#define CYCLES_OAM      80
#define CYCLES_TRANSFER 172
#define CYCLES_HBLANK   204
#define CYCLES_PER_LINE 456

// Classic DMG greenish palette, indexed by 2-bit color id (0=lightest).
static const u8 palette[4][3] = {
  {155, 188, 15},
  {139, 172, 15},
  {48, 98, 48},
  {15, 56, 15},
};

void ppu_init(PPU *ppu){
  for (int y = 0; y < SCREEN_HEIGHT; y++) {
    for (int x = 0; x < SCREEN_WIDTH; x++) {
      ppu->framebuffer[y][x][0] = palette[0][0];
      ppu->framebuffer[y][x][1] = palette[0][1];
      ppu->framebuffer[y][x][2] = palette[0][2];
    }
  }
  ppu->dot_counter = 0;
  ppu->window_line = 0;
  ppu->frame_ready = false;
}

// Palettes never store colors directly - they store, per raw 2-bit color
// id (0-3), which of the 4 fixed shades above to actually display. Each
// palette byte packs its four 2-bit mappings back to back: color id 0's
// shade lives in bits 0-1, id 1's in bits 2-3, and so on.
static u8 bgp_shade(u8 bgp, u8 color_index){
  return (bgp >> (color_index * 2)) & 0x03;
}

// Writes one already-resolved shade into the framebuffer.
static void put_pixel(PPU *ppu, u8 ly, int x, u8 shade){
  ppu->framebuffer[ly][x][0] = palette[shade][0];
  ppu->framebuffer[ly][x][1] = palette[shade][1];
  ppu->framebuffer[ly][x][2] = palette[shade][2];
}

// Fills bg_color_index[] (raw 0-3 color ids, before BGP mapping - sprites
// need these to implement "behind background" priority) and draws the
// background into the framebuffer for one scanline.
//
// The background is a 32x32 grid of 8x8-pixel tiles (256x256 pixels total,
// of which only a 160x144 window starting at SCX,SCY is ever visible - the
// rest wraps around, which is what makes SCX/SCY a cheap way to scroll).
// Each tile's pixel data is 16 bytes: 2 bytes per row, forming two bit
// planes - bit `7-px` of the low byte is a pixel's low color bit, the same
// bit of the high byte is its high color bit. That gives 4 raw color ids
// per pixel, only meaningful once run through BGP.
static void render_background(PPU *ppu, Memory *mem, u8 ly, u8 lcdc, u8 bg_color_index[SCREEN_WIDTH]){
  if (!(lcdc & 0x01)) {
    // LCDC bit0 clear blanks BOTH background and window (sprites are
    // unaffected - see render_sprites). "Blank" reads as color id 0.
    for (int x = 0; x < SCREEN_WIDTH; x++) {
      bg_color_index[x] = 0;
      put_pixel(ppu, ly, x, 0);
    }
    return;
  }

  u8 scy = memory_read(mem, SCY_ADDR);
  u8 scx = memory_read(mem, SCX_ADDR);
  u8 bgp = memory_read(mem, BGP_ADDR);
  u16 map_base = (lcdc & 0x08) ? 0x9C00 : 0x9800;
  bool unsigned_addressing = (lcdc & 0x10) != 0;

  for (int x = 0; x < SCREEN_WIDTH; x++) {
    // Wrapping u8 addition: scrolling past the 256-pixel edge of the
    // background map wraps back to the other side, for free.
    u8 bg_x = (u8)(scx + x);
    u8 bg_y = (u8)(scy + ly);
    u16 tile_col = bg_x / 8;
    u16 tile_row = bg_y / 8;
    u16 map_addr = (u16)(map_base + tile_row * 32 + tile_col);
    u8 tile_index = memory_read(mem, map_addr);

    // Two incompatible ways to turn a tile index into a VRAM address:
    // "unsigned" treats it as 0-255 straight into 0x8000+; "signed"
    // treats it as -128..127 relative to 0x9000, so index 0 there means
    // 0x9000 and index -1 means 0x8FF0. Sprites always use the unsigned
    // scheme regardless of this bit (see render_sprites).
    u16 tile_addr;
    if (unsigned_addressing) {
      tile_addr = (u16)(0x8000 + (u16)tile_index * 16);
    } else {
      tile_addr = (u16)(0x9000 + (int8_t)tile_index * 16);
    }

    u8 py = bg_y % 8; // which row within the tile
    u8 px = bg_x % 8; // which column within the tile
    u8 lo = memory_read(mem, (u16)(tile_addr + py * 2));
    u8 hi = memory_read(mem, (u16)(tile_addr + py * 2 + 1));
    u8 bit = 7 - px; // bit 7 of a tile row is its leftmost pixel
    u8 color_index = (u8)((((hi >> bit) & 1) << 1) | ((lo >> bit) & 1));

    bg_color_index[x] = color_index;
    put_pixel(ppu, ly, x, bgp_shade(bgp, color_index));
  }
}

// Draws the window layer over the background for one scanline, if active.
// The window is a second, non-scrolling background layer used for things
// like HUDs and menus - it always starts drawing from its own tile (0,0)
// rather than tracking SCX/SCY. It anchors at (WX-7, WY) on screen; the
// "-7" is a long-standing hardware quirk of how WX is defined, not a bug.
//
// Its *own* line counter (ppu->window_line) only advances on rows where
// the window actually draws something - so if a game enables it partway
// down the screen, or toggles it off and back on, the window's internal
// tile row still starts from wherever it last left off rather than
// jumping to match LY. That's exactly how the real hardware behaves.
static void render_window(PPU *ppu, Memory *mem, u8 ly, u8 lcdc, u8 bg_color_index[SCREEN_WIDTH]){
  if (!(lcdc & 0x20)) return; // window disabled

  u8 wy = memory_read(mem, WY_ADDR);
  if (ly < wy) return; // hasn't started yet on this frame

  int wx = (int)memory_read(mem, WX_ADDR) - 7;
  if (wx >= SCREEN_WIDTH) return; // pushed entirely off the right edge

  u8 bgp = memory_read(mem, BGP_ADDR);
  u16 map_base = (lcdc & 0x40) ? 0x9C00 : 0x9800; // independent of the BG's own map bit (LCDC bit3)
  bool unsigned_addressing = (lcdc & 0x10) != 0;  // same tile data area as the background
  int win_y = ppu->window_line;

  bool drew_any = false;
  // Start at wx clamped to 0 (a negative wx just means the window's left
  // edge is scrolled off-screen; nothing to draw there, but the visible
  // part still starts at its own tile 0 - hence `win_x = x - wx` below,
  // not `x`).
  for (int x = (wx < 0 ? 0 : wx); x < SCREEN_WIDTH; x++) {
    int win_x = x - wx; // position within the window, independent of wx
    u16 tile_col = (u16)(win_x / 8);
    u16 tile_row = (u16)(win_y / 8);
    u16 map_addr = (u16)(map_base + tile_row * 32 + tile_col);
    u8 tile_index = memory_read(mem, map_addr);

    u16 tile_addr;
    if (unsigned_addressing) {
      tile_addr = (u16)(0x8000 + (u16)tile_index * 16);
    } else {
      tile_addr = (u16)(0x9000 + (int8_t)tile_index * 16);
    }

    u8 py = (u8)(win_y % 8);
    u8 px = (u8)(win_x % 8);
    u8 lo = memory_read(mem, (u16)(tile_addr + py * 2));
    u8 hi = memory_read(mem, (u16)(tile_addr + py * 2 + 1));
    u8 bit = 7 - px;
    u8 color_index = (u8)((((hi >> bit) & 1) << 1) | ((lo >> bit) & 1));

    bg_color_index[x] = color_index;
    put_pixel(ppu, ly, x, bgp_shade(bgp, color_index));
    drew_any = true;
  }

  if (drew_any) ppu->window_line++;
}

// One OAM entry (0xFE00 + 4 bytes per sprite): Y, X (both offset from the
// real screen position - see below), tile index, and an attribute byte
// (bit7 BG priority, bit6 Y-flip, bit5 X-flip, bit4 palette; bits 0-2 are
// CGB-only and unused here).
typedef struct { u8 y, x, tile, attr; int oam_index; } SpriteEntry;

// Draws OAM sprites intersecting this scanline on top of the background
// (up to the hardware's 10-sprites-per-line limit).
static void render_sprites(PPU *ppu, Memory *mem, u8 ly, u8 lcdc, const u8 bg_color_index[SCREEN_WIDTH]){
  if (!(lcdc & 0x02)) return; // OBJ disabled (background can still be blanked independently)

  bool tall = (lcdc & 0x04) != 0;
  int sprite_height = tall ? 16 : 8;

  // Scan all 40 OAM entries in order and keep the first 10 whose vertical
  // span covers this line - exactly what a real Game Boy does, including
  // the "only the first 10 in OAM order get drawn" limit (sprite #11+ on a
  // crowded line just doesn't render, even if there'd be room).
  SpriteEntry sprites[10];
  int count = 0;
  for (int i = 0; i < 40 && count < 10; i++) {
    u16 base = (u16)(0xFE00 + i * 4);
    u8 sy = memory_read(mem, base);
    // OAM Y is stored offset by +16 (and X by +8, below) so that sprites
    // can be smoothly scrolled fully off the top/left edge using only
    // unsigned byte values - Y=0 means "16 pixels above the screen",
    // i.e. completely hidden, rather than needing a signed coordinate.
    int screen_y = (int)sy - 16;
    if ((int)ly >= screen_y && (int)ly < screen_y + sprite_height) {
      sprites[count].y = sy;
      sprites[count].x = memory_read(mem, (u16)(base + 1));
      sprites[count].tile = memory_read(mem, (u16)(base + 2));
      sprites[count].attr = memory_read(mem, (u16)(base + 3));
      sprites[count].oam_index = i;
      count++;
    }
  }

  // Real hardware priority when sprites overlap: smaller X wins; ties go
  // to the smaller OAM index. This insertion sort orders the array so the
  // *lowest*-priority sprite comes first - since the drawing loop below
  // just overwrites pixels in order, drawing lowest priority first and
  // highest priority last naturally leaves the highest priority sprite
  // visually on top, with no separate "z-buffer" needed.
  for (int a = 1; a < count; a++) {
    SpriteEntry tmp = sprites[a];
    int b = a - 1;
    while (b >= 0 && (sprites[b].x < tmp.x ||
           (sprites[b].x == tmp.x && sprites[b].oam_index < tmp.oam_index))) {
      sprites[b + 1] = sprites[b];
      b--;
    }
    sprites[b + 1] = tmp;
  }

  for (int s = 0; s < count; s++) {
    u8 sy = sprites[s].y, sx = sprites[s].x, tile = sprites[s].tile, attr = sprites[s].attr;
    int screen_y = (int)sy - 16;
    int screen_x = (int)sx - 8;
    int row = (int)ly - screen_y; // which row of the sprite this scanline hits

    bool yflip = (attr & 0x40) != 0;
    bool xflip = (attr & 0x20) != 0;
    bool behind_bg = (attr & 0x80) != 0;
    u8 obp = memory_read(mem, (attr & 0x10) ? OBP1_ADDR : OBP0_ADDR);

    int use_row = yflip ? (sprite_height - 1 - row) : row;
    // 8x16 sprites always use an even tile index for their top half; the
    // hardware ignores bit 0 of the stored index rather than requiring the
    // game to pre-clear it. Sprites also always use unsigned addressing
    // from 0x8000, unlike the background/window's LCDC-bit4-dependent mode.
    u8 eff_tile = tall ? (u8)(tile & 0xFE) : tile;
    u16 tile_addr = (u16)(0x8000 + (u16)eff_tile * 16 + (u16)use_row * 2);
    u8 lo = memory_read(mem, tile_addr);
    u8 hi = memory_read(mem, (u16)(tile_addr + 1));

    for (int px = 0; px < 8; px++) {
      int screen_px = screen_x + px;
      if (screen_px < 0 || screen_px >= SCREEN_WIDTH) continue;

      // Normally bit 7 is the leftmost pixel (px=0); X-flip just reads the
      // bits in the opposite order instead of physically reversing them.
      int bit = xflip ? px : (7 - px);
      u8 color_index = (u8)((((hi >> bit) & 1) << 1) | ((lo >> bit) & 1));
      if (color_index == 0) continue; // color id 0 is always transparent for sprites
      if (behind_bg && bg_color_index[screen_px] != 0) continue; // BG priority: only shows over BG color 0

      u8 shade = bgp_shade(obp, color_index);
      put_pixel(ppu, ly, screen_px, shade);
    }
  }
}

// Renders one full scanline: background, then window on top, then sprites.
static void render_scanline(PPU *ppu, Memory *mem, u8 ly){
  u8 lcdc = memory_read(mem, LCDC_ADDR);
  u8 bg_color_index[SCREEN_WIDTH];
  render_background(ppu, mem, ly, lcdc, bg_color_index);
  render_window(ppu, mem, ly, lcdc, bg_color_index);
  render_sprites(ppu, mem, ly, lcdc, bg_color_index);
}

// The PPU's whole job, per call: add `cycles` T-states to a running
// counter, and while that counter has accumulated enough to complete the
// *current* mode's phase, consume that phase's length from it and advance
// to the next mode (looping, since a single call - especially now that
// opcodes report their real, sometimes-large cycle cost - can span more
// than one phase transition at once). LY and STAT are written back to
// memory as ordinary registers so the CPU can read them normally.
void ppu_step(PPU *ppu, Memory *mem, int cycles){
  ppu->frame_ready = false;

  u8 lcdc = memory_read(mem, LCDC_ADDR);
  // While the LCD is off, the whole PPU is frozen - LY, STAT and the
  // internal dot counter simply don't move, exactly like on hardware.
  if (!(lcdc & 0x80)) return;

  ppu->dot_counter += cycles;

  u8 ly = memory_read(mem, LY_ADDR);
  u8 stat = memory_read(mem, STAT_ADDR);
  u8 mode = stat & 0x03;

  for (;;) {
    int phase_length;
    switch (mode) {
      case MODE_OAM:      phase_length = CYCLES_OAM; break;
      case MODE_TRANSFER: phase_length = CYCLES_TRANSFER; break;
      case MODE_HBLANK:   phase_length = CYCLES_HBLANK; break;
      default:             phase_length = CYCLES_PER_LINE; break; // VBLANK
    }

    if (ppu->dot_counter < phase_length) break; // not enough accumulated yet - done for this call
    ppu->dot_counter -= phase_length;

    if (mode == MODE_OAM) {
      mode = MODE_TRANSFER;
    } else if (mode == MODE_TRANSFER) {
      // The whole scanline is rendered in one shot exactly here, at the
      // instant pixel transfer "finishes" - a simplification compared to
      // real hardware's pixel-by-pixel FIFO, but it produces the same
      // final image for the vast majority of effects.
      render_scanline(ppu, mem, ly);
      mode = MODE_HBLANK;
    } else if (mode == MODE_HBLANK) {
      ly++;
      if (ly == SCREEN_HEIGHT) {
        // All 144 visible lines done: enter VBlank and tell the CPU a
        // full frame is ready - this is the signal main.c uses to present
        // the framebuffer and poll input.
        mode = MODE_VBLANK;
        interrupt_request(mem, INT_VBLANK);
        ppu->frame_ready = true;
      } else {
        mode = MODE_OAM; // next visible line
      }
    } else { // MODE_VBLANK, advances one line at a time
      ly++;
      if (ly == SCREEN_HEIGHT + 10) { // 10 VBlank lines (144-153) elapsed
        ly = 0;
        mode = MODE_OAM;
        ppu->window_line = 0; // new frame: window restarts from its own tile 0
      }
    }

    memory_write(mem, LY_ADDR, ly);
  }

  // STAT's low 2 bits always reflect the current mode; bit 2 is a
  // convenience flag some games poll (or get an interrupt from, though
  // that's not wired up yet) instead of comparing LY to LYC themselves.
  stat = (u8)((stat & 0xFC) | mode);
  u8 lyc = memory_read(mem, LYC_ADDR);
  if (ly == lyc) stat |= 0x04; else stat &= (u8)~0x04;
  memory_write(mem, STAT_ADDR, stat);
}
