#pragma once
#include <stdint.h>

typedef struct LC3VM;

// Allocates a new LC3 virual machine
LC3VM* lc3_vm_new();
// Frees resources allocated by lc3_vm_new function.
void lc3_vm_free(LC3VM const* vm);
