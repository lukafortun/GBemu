#ifndef CPU_H
#define CPU_H

#include "../common.h"

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
} CPU;



#endif
