#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#include "lc3_vm.h"
#include "lc3_opcodes.h"

#define REGISTERS_COUNT 8
#define PC_START 0x3000

#define COND_POSITIVE 0b0001
#define COND_ZERO 0b0010
#define COND_NEGATIVE 0b0100

void set_cond_reg(LC3VM* vm, uint16_t register_number);

// Get the vaule from instruction that starts at index i (starting from right)
// and is l bit long. Sign extend the vaule if extend is true.
uint16_t get_value(uint16_t instruction, uint16_t i, uint16_t l, bool extend);

// Write to memory
void write(uint16_t address, uint16_t value);
// Read form memory
uint16_t read(uint16_t address);

typedef struct LC3VM {
    // Memory locations
    uint16_t memory[UINT16_MAX];
    // General purpose registers
    uint16_t regs[REGISTERS_COUNT];
    // Program counter
    uint16_t pc;
    // Condition flag register
    uint16_t cond;
} LC3VM;

LC3VM* lc3_vm_new() {
    LC3VM* vm = calloc(1, sizeof(LC3VM));

    vm->pc = PC_START;

    return vm;
}

void lc3_vm_free(LC3VM const* vm) { free(vm); }

void lc3_vm_execute(LC3VM* vm, uint16_t instruction) {
    uint16_t opcode = instruction >> 12;

    switch (opcode) {
        case OP_BRANCH:
            uint16_t pc_offset = get_value(instruction, 8, 9, true);
            uint16_t condition = get_value(instruction, 11, 3, false);

            if (condition & vm->cond) {
                vm->pc += pc_offset;
            }

            break;

        case OP_ADD:
            // Destination register
            uint16_t dr = get_value(instruction, 11, 3, false);
            // First operand
            uint16_t sr1 = get_value(instruction, 8, 3, false);
            // Immediate mode flag
            uint16_t imm = get_value(instruction, 5, 1, false);

            if (imm) {
                // Second operand
                uint16_t imm5 = get_value(instruction, 4, 5, true);

                vm->regs[dr] = vm->regs[sr1] + imm5;
            } else {
                // Second operand
                uint16_t sr2 = get_value(instruction, 2, 3, false);

                vm->regs[dr] = vm->regs[sr1] + vm->regs[sr2];
            }

            set_cond_reg(vm, dr);

            break;

        case OP_LOAD:
            uint16_t dr = get_value(instruction, 11, 3, false);
            uint16_t pc_offset = get_value(instruction, 8, 9, true);

            vm->regs[dr] = mem_read();

            set_cond_reg(vm, dr);

            break;

        case OP_STORE:
            uint16_t sr = get_value(instruction, 11, 3, false);
            uint16_t pc_offset = get_value(instruction, 8, 9, true);

            write(vm->pc + pc_offset, vm->regs[sr]);

            break;

        case OP_JUMP_REGISTER:
            uint16_t long_flag = get_value(instruction, 11, 1, false);

            if (long_flag) {
                uint16_t pc_offset = get_value(instruction, 10, 11, true);

                vm->pc += pc_offset;
            } else {
                uint16_t base_r = get_value(instruction, 8, 3, false);

                vm->pc = vm->regs[base_r];
            }

            break;

        case OP_AND:
            uint16_t dr = get_value(instruction, 11, 3, false);
            uint16_t sr1 = get_value(instruction, 8, 3, false);
            uint16_t imm = get_value(instruction, 5, 1, false);

            if (imm) {
                uint16_t imm5 = get_value(instruction, 4, 5, true);

                vm->regs[dr] = vm->regs[sr1] & imm5;
            } else {
                uint16_t sr2 = get_value(instruction, 2, 3, false);

                vm->regs[dr] = vm->regs[sr1] & vm->regs[sr2];
            }

            set_cond_reg(vm, dr);

            break;

        case OP_LOAD_REGISTER:
            uint16_t dr = get_value(instruction, 11, 3, false);
            uint16_t base_r = get_value(instruction, 8, 3, false);
            uint16_t offset = get_value(instruction, 5, 6, true);

            vm->regs[dr] = mem_read(vm->regs[base_r] + offset);

            set_cond_reg(vm, dr);

            break;

        case OP_STORE_REGISTER:
            uint16_t sr = get_value(instruction, 11, 3, false);
            uint16_t base_r = get_value(instruction, 8, 3, false);
            uint16_t offset = get_value(instruction, 5, 6, true);

            write(vm->regs[base_r] + offset, vm->regs[sr]);

            break;

        case OP_RETURN_FROM_INTERRUPT:
            // Do nothing
            break;

        case OP_NOT:
            uint16_t dr = get_value(instruction, 11, 3, false);
            uint16_t sr = get_value(instruction, 8, 3, false);

            vm->regs[dr] = ~vm->regs[sr];

            set_cond_reg(vm, dr);

            break;

        case OP_LOAD_INDIRECT:
            uint16_t dr = get_value(instruction, 11, 3, false);
            uint16_t pc_offset = get_value(instruction, 8, 9, true);

            vm->regs[dr] = mem_read(mem_read(vm->pc + pc_offset));

            set_cond_reg(vm, dr);

            break;

        case OP_STORE_INDIRECT:
            uint16_t sr = get_value(instruction, 11, 3, false);
            uint16_t pc_offset = get_value(instruction, 8, 9, true);

            write(read(vm->pc + pc_offset), vm->regs[sr]);

            break;

        case OP_JUMP:
            uint16_t base_r = get_value(instruction, 8, 3, false);

            vm->pc = vm->regs[base_r];

            break;

        case OP_RESERVED:
            // Do nothing
            break;

        case OP_LOAD_EFFECTIVE_ADDRESS:
            uint16_t dr = get_value(instruction, 11, 3, false);
            uint16_t pc_offset = get_value(instruction, 8, 9, true);

            vm->regs[dr] = vm->pc + pc_offset;

            set_cond_reg(vm, dr);

            break;

        case OP_TRAP:
            break;
    }
}

void update_condition_register(LC3VM* vm, uint16_t register_number) {
    if (vm->regs[register_number] == 0) {
        vm->cond = COND_ZERO;
    } else if (vm->regs[register_number] >> 15) {
        vm->cond = COND_NEGATIVE;
    } else {
        vm->cond = COND_POSITIVE;
    }
}

uint16_t get_value(uint16_t instruction, uint16_t i, uint16_t l, bool extend) {
    uint16_t tmp = instruction >> (i - l + 1);
    uint16_t bitmask = (1 << l) - 1;

    tmp = tmp & bitmask;

    if (extend) {
        if ((tmp >> (l - 1)) & 1) {
            tmp |= (0xFFFF << l);
        }
    }

    return tmp;
}
