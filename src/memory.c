#include "../include/memory.h"

void memory_init(Memory *mem) {
    for (int i = 0; i < GB_MEMORY_SIZE; i++) {
        mem->data[i] = 0;
    }
}

u8 memory_read(Memory *mem, u16 address) {
    return mem->data[address];
}

void memory_write(Memory *mem, u16 address, u8 value) {
    mem->data[address] = value;
}
