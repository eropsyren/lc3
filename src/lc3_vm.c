#include "lc3_vm.h"

#include <stdint.h>
#include <stdlib.h>

#define REGISTERS_COUNT 8
#define PC_START 0x3000

#define OP_BRANCH 0
#define OP_ADD 1
#define OP_LOAD 2
#define OP_STORE 3
#define OP_JUMP_REGISTER 4
#define OP_AND 5
#define OP_LOAD_REGISTER 6
#define OP_STORE_REGISTER 7
#define OP_UNUSED 8
#define OP_NOT 9
#define OP_LOAD_INDIRECT 10
#define OP_STORE_INDIRECT 11
#define OP_JUMP 12
#define OP_RESERVED 13
#define OP_LOAD_EFFECTIVE_ADDRESS 14
#define OP_TRAP 15

#define COND_POSITIVE 0b0001
#define COND_ZERO 0b0010
#define COND_NEGATIVE 0b0100

typedef struct LC3VM {
    // Memory locations
    uint16_t memory[UINT16_MAX];
    // General purpose registers
    uint16_t registers[REGISTERS_COUNT];
    // Program counter
    uint16_t program_counter;
    // Condition flag register
    uint16_t condition;
} LC3VM;

LC3VM* lc3_vm_new() {
    LC3VM* vm = calloc(1, sizeof(LC3VM));

    vm->program_counter = PC_START;

    return vm;
}

void lc3_vm_execute(LC3VM* vm, uint16_t instruction) {
    switch (instruction) {
        case OP_BRANCH:
            break;

        case OP_ADD:
            break;

        case OP_LOAD:
            break;

        case OP_STORE:
            break;

        case OP_JUMP_REGISTER:
            break;

        case OP_AND:
            break;

        case OP_LOAD_REGISTER:
            break;

        case OP_STORE_REGISTER:
            break;

        case OP_UNUSED:
            break;

        case OP_NOT:
            break;

        case OP_LOAD_INDIRECT:
            break;

        case OP_STORE_INDIRECT:
            break;

        case OP_JUMP:
            break;

        case OP_RESERVED:
            break;

        case OP_LOAD_EFFECTIVE_ADDRESS:
            break;

        case OP_TRAP:
            break;
    }
}

void update_condition_register(LC3VM* vm, uint16_t register_number) {
    if (vm->registers[register_number] == 0) {
        vm->condition = COND_ZERO;
    } else if (vm->registers[register_number] >> 15) {
        vm->condition = COND_NEGATIVE;
    } else {
        vm->condition = COND_POSITIVE;
    }
}

void lc3_vm_free(LC3VM const* vm) { free(vm); }
