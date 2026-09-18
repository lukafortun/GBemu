#ifndef JOYPAD_H
#define JOYPAD_H

#include "common.h"
#include "memory.h"
#include "interrupts.h"
#include <stdbool.h>

// JOYP (0xFF00) is a two-stage register, not a direct button readout. The
// game first *selects* a group by clearing bit 4 (direction keys) and/or
// bit 5 (A/B/Select/Start) - both can be selected at once - then reads the
// low nibble, which reports that group's state, active-low (0 = pressed,
// 1 = released). This mirrors real hardware, which multiplexes 8 buttons
// over one set of 4 lines rather than wiring one line per button.
#define JOYP_ADDR 0xFF00

typedef struct {
  bool right, left, up, down;
  bool a, b, select, start;
} JoypadButtons;

// Records the current button state, read by joypad_step. Call once per
// frame from the platform layer's polled input (or leave unset for "nothing
// pressed", the previous default).
void joypad_set_buttons(JoypadButtons buttons);

// Updates the JOYP register (0xFF00) to reflect the last recorded button
// state for whichever group(s) the game has selected via bits 4-5. Should
// be called once per instruction, like timer_step/ppu_step.
void joypad_step(Memory *mem);

#endif
