#ifndef MINI_C_VM_H
#define MINI_C_VM_H

#include "ir.h"

int vm_run(const IrProgram *prog, bool trace);

#endif
