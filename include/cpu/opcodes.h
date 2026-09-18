#ifndef OPCODES_H
#define OPCODES_H

#include "op_arith.h"
#include "op_cb.h"
#include "op_ctrlflow.h"
#include "op_load.h"
#include "op_misc.h"
#include "op_rot_shift.h"

// Executes a single non-prefixed opcode already fetched from memory.
// Returns its T-state cost (branch-dependent for conditional jumps/calls/
// returns; the 0xCB prefix's cost includes the CB opcode that follows it).
u8 cpu_execute_opcode(CPU *cpu, Memory *mem, u8 opcode);

#endif
