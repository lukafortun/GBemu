#include "../include/joypad.h"

// The "real" input state, set by the platform layer from actual keyboard
// polling. joypad_step never reads input directly - it just reflects
// whatever was last recorded here into the JOYP register.
static JoypadButtons g_buttons = {0};

// The low nibble JOYP reported on the previous call, kept only to detect
// released->pressed transitions for the joypad interrupt (see below).
static u8 g_last_lower = 0x0F;

void joypad_set_buttons(JoypadButtons buttons){
  g_buttons = buttons;
}

// Rebuilds JOYP's low nibble from the current button state and the
// selection bits the game last wrote, and fires the joypad interrupt on
// any newly-pressed button. Intended to run once per instruction so JOYP
// is always fresh whenever the game happens to read it.
void joypad_step(Memory *mem){
  u8 v = memory_read(mem, JOYP_ADDR);
  u8 lower = 0x0F; // active-low: 1 = released, cleared bit by bit below

  bool select_dpad    = !(v & 0x10); // P14: 0 = direction keys selected
  bool select_buttons = !(v & 0x20); // P15: 0 = A/B/Select/Start selected

  // Both groups share the same 4 output bits, so if a game selects both at
  // once (some do, to detect "any button") a press in either group clears
  // the matching bit - the two blocks below just both get a chance to.
  if (select_dpad) {
    if (g_buttons.right) lower &= (u8)~0x01;
    if (g_buttons.left)  lower &= (u8)~0x02;
    if (g_buttons.up)    lower &= (u8)~0x04;
    if (g_buttons.down)  lower &= (u8)~0x08;
  }
  if (select_buttons) {
    if (g_buttons.a)      lower &= (u8)~0x01;
    if (g_buttons.b)      lower &= (u8)~0x02;
    if (g_buttons.select) lower &= (u8)~0x04;
    if (g_buttons.start)  lower &= (u8)~0x08;
  }

  // The joypad interrupt fires on any 1->0 (released->pressed) transition
  // of the selected lines - many games (menus especially) HALT and wait
  // for it instead of actively polling JOYP. `g_last_lower & ~lower` is
  // nonzero exactly where a bit that used to be 1 (released) is now 0
  // (pressed).
  if ((g_last_lower & ~lower) != 0) {
    interrupt_request(mem, INT_JOYPAD);
  }
  g_last_lower = lower;

  // Bits 6-7 are unused and always read as 1; bits 4-5 (the selection the
  // game wrote) are preserved as-is; bits 0-3 are the result computed above.
  v = (u8)((v & 0xF0) | 0xC0 | lower);
  memory_write(mem, JOYP_ADDR, v);
}
