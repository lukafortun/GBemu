#ifndef MEMORY_H
#define MEMORY_H

#include "common.h"

typedef struct {
    u8 data[GB_MEMORY_SIZE];
} Memory;

void memory_init(Memory *mem);
u8 memory_read(Memory *mem, u16 address);
void memory_write(Memory *mem, u16 address, u8 value);

#endif
