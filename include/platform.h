#ifndef PLATFORM_H
#define PLATFORM_H

#include "common.h"
#include <stdbool.h>

// The Windows-specific display/input backend (platform_win32.c). Kept
// behind this small interface so the emulator core (cpu/memory/ppu/timer/
// joypad) never touches a Win32 type directly - a future SDL2 or other
// backend would only need to implement these five functions.
typedef struct {
  bool right, left, up, down;
  bool a, b, select, start;
} PlatformInput;

// Creates the display window, scaled up `scale` times from 160x144.
// Returns false on failure.
bool platform_init(int scale);

// Pumps window messages and reads the current keyboard state into
// `out_input`. Returns false once the user has closed the window.
bool platform_poll(PlatformInput *out_input);

// Blits the framebuffer to the window and sleeps out the rest of the frame
// budget so the window updates at roughly 59.7 FPS instead of as fast as
// the host can run the (not yet cycle-accurate) emulation loop.
void platform_present(u8 framebuffer[SCREEN_HEIGHT][SCREEN_WIDTH][3]);

// Destroys the window. Safe to call even if platform_init was never called
// or already failed.
void platform_shutdown(void);

#endif
