#include "../include/cpu.h"

int process_opcode(int opcode){

}

int fetch_cycle(CPU *cpu, Memory *mem){
  u8 opcode = memory_read(mem, cpu->pc);
  cpu->pc++;
  
  switch (opcode) {
    case 0x00:
      break;

  }

}


// In a first time we have to fetch the next opcode to execute
// When there is only one cycle to process, we can start to fetch 
// next opcode. (fetch/execute overlap) 
// ref : https://gekkio.fi/files/gb-docs/gbctr.pdf, page 17
