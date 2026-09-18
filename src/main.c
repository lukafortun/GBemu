#include "../include/common.h"
#include "../include/memory.h"
#include "../include/cpu/cpu.h"
#include "../include/cpu/opcodes.h"
#include "../include/interrupts.h"
#include "../include/timer.h"
#include "../include/ppu.h"
#include "../include/joypad.h"
#include "../include/platform.h"

// Window size multiplier applied to the native 160x144 Game Boy resolution.
#define WINDOW_SCALE 3

// blargg's test ROMs report progress/results over the serial port: a byte
// written to 0xFF01 followed by 0x81 written to 0xFF02 means "print this
// character". Mirroring that to stdout is the standard way to read results.
static void handle_serial_output(Memory *mem){
  if (memory_read(mem, 0xFF02) == 0x81) {
    putchar(memory_read(mem, 0xFF01));
    fflush(stdout);
    memory_write(mem, 0xFF02, 0x00);
  }
}

// Loads a ROM (0x0000-0x7FFF, no MBC support so only the first 32 KB of
// larger cartridges is usable) and runs it in a live window until closed.
int main(int argc, char **argv){
  if (argc < 2) {
    fprintf(stderr, "Usage: %s <rom.gb>\n", argv[0]);
    return 1;
  }

  FILE *f = fopen(argv[1], "rb");
  if (!f) {
    fprintf(stderr, "Could not open %s\n", argv[1]);
    return 1;
  }
  fseek(f, 0, SEEK_END);
  long size = ftell(f);
  fseek(f, 0, SEEK_SET);

  u8 *rom = malloc((size_t)size);
  if (fread(rom, 1, (size_t)size, f) != (size_t)size) {
    fprintf(stderr, "Failed to read %s\n", argv[1]);
    fclose(f);
    free(rom);
    return 1;
  }
  fclose(f);

  Memory mem;
  memory_init(&mem);
  memory_load_rom(&mem, rom, (u32)size);
  free(rom);

  // We don't emulate the Nintendo logo boot ROM, so instead of starting at
  // 0x0000 and letting it run, jump straight to 0x0100 (where every
  // cartridge's own code begins) with the exact register values real
  // hardware leaves behind once that boot ROM finishes. Games rely on
  // these specific values (nothing reinitializes registers before using
  // them), so skipping the boot animation but faking its *effects* is
  // what lets execution continue correctly from here.
  CPU cpu = {0};
  cpu.af = 0x01B0;
  cpu.bc = 0x0013;
  cpu.de = 0x00D8;
  cpu.hl = 0x014D;
  cpu.sp = 0xFFFE;
  cpu.pc = 0x0100;

  PPU ppu;
  ppu_init(&ppu);

  if (!platform_init(WINDOW_SCALE)) {
    fprintf(stderr, "Failed to create the display window\n");
    return 1;
  }

  PlatformInput input = {0};
  bool running = true;
  long instr_count = 0;

  // The Windows message pump must run often regardless of what the emulated
  // game is doing (LCD off, a slow loop, a hang in our own code) - otherwise
  // the OS marks the window "Not Responding" within a couple of seconds.
  // Pumping every 256 instructions is cheap and keeps it responsive; the
  // heavier work (blit + frame pacing) still only happens once per frame.
  while (running) {
    // This is the whole machine: the CPU executes one instruction and
    // reports its real cost in T-states, and every other component -
    // timer, PPU, joypad - is advanced by that same number of cycles so
    // they all stay in lockstep, exactly as they would driven by one
    // shared hardware clock.
    u8 cycles = fetch_cycle(&cpu, &mem);
    timer_step(&mem, cycles);
    ppu_step(&ppu, &mem, cycles);
    joypad_step(&mem);
    handle_serial_output(&mem);
    instr_count++;

    if (ppu.frame_ready || (instr_count % 256) == 0) {
      running = platform_poll(&input);
      joypad_set_buttons((JoypadButtons){
        .right = input.right, .left = input.left,
        .up = input.up, .down = input.down,
        .a = input.a, .b = input.b,
        .select = input.select, .start = input.start,
      });
      if (!running) break;
    }

    // Only actually redraw and pace to real-time once a full frame is
    // done rendering - not on every one of the polls above, which run far
    // more often just to keep the window responsive.
    if (ppu.frame_ready) {
      platform_present(ppu.framebuffer);
    }
  }

  platform_shutdown();
  return 0;
}
