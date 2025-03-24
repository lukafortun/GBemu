#ifndef CPU_H
#define CPU_H

#include "../common.h"
#include <stdbool.h>

#define Z_FLAG 0x80  // Bit 7
#define N_FLAG 0x40  // Bit 6
#define H_FLAG 0x20  // Bit 5
#define C_FLAG 0x10  // Bit 4

// CPU registers
typedef struct {
    union {
        struct {
            u8 f; // Flags register
            u8 a; // Accumulator
        };
        u16 af;
    };

    union {
        struct {
            u8 c;
            u8 b;
        };
        u16 bc;
    };

    union {
        struct {
            u8 e;
            u8 d;
        };
        u16 de;
    };

    union {
        struct {
            u8 l;
            u8 h;
        };
        u16 hl;
    };

    u16 sp; // Stack pointer
    u16 pc; // Program counter
  
    bool ime;       // Interrupt Master Enable 
    uint8_t ie;     // Interrupt Enable Register 
    uint8_t ir; // Interrupt Request Register 

} CPU;



#endif
