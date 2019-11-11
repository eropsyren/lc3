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
#define OP_RETURN_FROM_INTERRUPT 8
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

void update_condition_register(LC3VM* vm, uint16_t register_number);

// Sign extend the rightmost bit_count bits in x
uint16_t sign_extend(uint16_t x, int bit_count);

// Get the vaule from instruction that starts at index (starting from right) and
// is length bit long
uint16_t get_value(uint16_t instruction, uint16_t index, uint16_t length);

// Write to memory
void write(uint16_t address, uint16_t value);
// Read form memory
uint16_t read(uint16_t address);

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

void lc3_vm_free(LC3VM const* vm) { free(vm); }

void lc3_vm_execute(LC3VM* vm, uint16_t instruction) {
    uint16_t opcode = instruction >> 12;

    switch (opcode) {
        case OP_BRANCH:
            uint16_t pc_offset = get_value(instruction, 8, 9);
            uint16_t condition = get_value(instruction, 11, 3);

            if (condition & vm->condition) {
                vm->program_counter += pc_offset;
            }

            break;

        case OP_ADD:
            // Destination register
            uint16_t dr = get_value(instruction, 11, 3);
            // First operand
            uint16_t sr1 = get_value(instruction, 8, 3);
            // Immediate mode flag
            uint16_t imm = get_value(instruction, 5, 1);

            if (imm) {
                // Second operand
                uint16_t imm5 = get_value(instruction, 4, 5);

                imm5 = sign_extend(imm5, 5);
                vm->registers[dr] = vm->registers[sr1] + imm5;
            } else {
                // Second operand
                uint16_t sr2 = get_value(instruction, 2, 3);

                vm->registers[dr] = vm->registers[sr1] + vm->registers[sr2];
            }

            update_condition_register(vm, dr);

            break;

        case OP_LOAD:
            break;

        case OP_STORE:
            break;

        case OP_JUMP_REGISTER:
            break;

        case OP_AND:
            uint16_t dr = get_value(instruction, 11, 3);
            uint16_t sr1 = get_value(instruction, 8, 3);
            uint16_t imm = get_value(instruction, 5, 1);

            if (imm) {
                uint16_t imm5 = get_value(instruction, 4, 5);

                imm5 = sign_extend(imm5, 5);
                vm->registers[dr] = vm->registers[sr1] & imm5;
            } else {
                uint16_t sr2 = get_value(instruction, 2, 3);

                vm->registers[dr] = vm->registers[sr1] & vm->registers[sr2];
            }

            update_condition_register(vm, dr);

            break;

        case OP_LOAD_REGISTER:
            break;

        case OP_STORE_REGISTER:
            break;

        case OP_RETURN_FROM_INTERRUPT:
            // Do nothing
            break;

        case OP_NOT:
            uint16_t dr = get_value(instruction, 11, 3);
            uint16_t sr = get_value(instruction, 8, 3);

            vm->registers[dr] = ~vm->registers[sr];

            update_condition_register(vm, dr);

            break;

        case OP_LOAD_INDIRECT:
            uint16_t dr = get_value(instruction, 11, 3);
            uint16_t pc_offset = get_value(instruction, 8, 9);

            vm->registers[dr] =
                mem_read(mem_read(vm->program_counter + pc_offset));

            update_condition_register(vm, dr);

            break;

        case OP_STORE_INDIRECT:
            break;

        case OP_JUMP:
            break;

        case OP_RESERVED:
            // Do nothing
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

uint16_t sign_extend(uint16_t x, int bit_count) {
    if ((x >> (bit_count - 1)) & 1) {
        x |= (0xFFFF << bit_count);
    }

    return x;
}

uint16_t get_value(uint16_t instruction, uint16_t index, uint16_t length) {
    uint16_t tmp = instruction >> (index - length + 1);
    uint16_t bitmask = (1 << length) - 1;

    return tmp & bitmask;
}
