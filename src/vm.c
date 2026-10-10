#include "vm.h"

static const char *vm_slot_name(const IrProgram *prog, int slot)
{
    if (slot < 0 || slot >= prog->nslots || prog->slot_names[slot] == NULL) {
        return "?";
    }
    return prog->slot_names[slot];
}

static void execute_printf(const IrProgram *prog, const IrInst *ins, const int64_t *slots)
{
    if (ins->string_id < 0 || ins->string_id >= prog->nstrings) {
        return;
    }
    const char *fmt = prog->strings[ins->string_id];
    int arg_idx = 0;

    for (const char *p = fmt; *p != '\0'; p++) {
        if (*p == '%' && p[1] != '\0') {
            p++;
            if (*p == '%') {
                putchar('%');
            } else if (*p == 'd' || *p == 'i') {
                if (arg_idx < ins->arg_count) {
                    int slot = ins->arg_slots[arg_idx++];
                    printf("%" PRId64, slots[slot]);
                }
            } else if (*p == 'f') {
                if (arg_idx < ins->arg_count) {
                    int slot = ins->arg_slots[arg_idx++];
                    printf("%f", (double)slots[slot]);
                }
            } else if (*p == 'c') {
                if (arg_idx < ins->arg_count) {
                    int slot = ins->arg_slots[arg_idx++];
                    printf("%c", (char)slots[slot]);
                }
            } else if (*p == 's') {
                if (arg_idx < ins->arg_count) {
                    int slot = ins->arg_slots[arg_idx++];
                    int64_t val = slots[slot];
                    if (val >= 0 && val < prog->nstrings) {
                        printf("%s", prog->strings[val]);
                    } else {
                        printf("(null)");
                    }
                }
            } else {
                putchar('%');
                putchar(*p);
            }
        } else {
            putchar(*p);
        }
    }
}

static void execute_scanf(const IrProgram *prog, const IrInst *ins, int64_t *slots)
{
    if (ins->string_id < 0 || ins->string_id >= prog->nstrings) {
        return;
    }
    const char *fmt = prog->strings[ins->string_id];
    int arg_idx = 0;

    for (const char *p = fmt; *p != '\0'; p++) {
        if (*p == '%' && p[1] != '\0') {
            p++;
            if (*p == 'd' || *p == 'i') {
                if (arg_idx < ins->arg_count) {
                    int slot = ins->arg_slots[arg_idx++];
                    long long input_val = 0;
                    if (scanf("%lld", &input_val) == 1) {
                        slots[slot] = (int64_t)input_val;
                    }
                }
            }
        }
    }
}

int vm_run(const IrProgram *prog, bool trace)
{
    int64_t *slots;
    int pc;
    int n;

    if (prog->nslots < 0) {
        fprintf(stderr, "runtime error: invalid program\n");
        return 1;
    }

    n = prog->nslots;
    slots = xcalloc((size_t)(n == 0 ? 1 : n), sizeof(int64_t));
    pc = 0;

    while (pc >= 0 && pc < prog->count) {
        const IrInst *ins = &prog->insts[pc];
        int next = pc + 1;

        if (trace) {
            fprintf(stderr, "pc=%d %-14s", pc, ir_op_name(ins->op));
            switch (ins->op) {
            case IR_LOAD_CONST:
                fprintf(stderr, "%s = %" PRId64, vm_slot_name(prog, ins->dest), ins->imm);
                break;
            case IR_LOAD:
            case IR_STORE:
                fprintf(stderr, "%s = %s (%" PRId64 ")", vm_slot_name(prog, ins->dest),
                        vm_slot_name(prog, ins->src1), slots[ins->src1]);
                break;
            case IR_ADD:
            case IR_SUB:
            case IR_MUL:
            case IR_DIV:
            case IR_CMP_EQ:
            case IR_CMP_NE:
            case IR_CMP_LT:
            case IR_CMP_GT:
            case IR_CMP_LE:
            case IR_CMP_GE:
            case IR_LOGICAL_AND:
            case IR_LOGICAL_OR:
                fprintf(stderr, "%s = %s(%" PRId64 ") %s %s(%" PRId64 ")", vm_slot_name(prog, ins->dest),
                        vm_slot_name(prog, ins->src1), slots[ins->src1], ir_op_name(ins->op),
                        vm_slot_name(prog, ins->src2), slots[ins->src2]);
                break;
            case IR_LOGICAL_NOT:
                fprintf(stderr, "%s = !%s(%" PRId64 ")", vm_slot_name(prog, ins->dest),
                        vm_slot_name(prog, ins->src1), slots[ins->src1]);
                break;
            case IR_JUMP:
                fprintf(stderr, "-> %d", ins->target);
                break;
            case IR_JUMP_IF_FALSE:
                fprintf(stderr, "%s=%" PRId64 " -> %d", vm_slot_name(prog, ins->src1), slots[ins->src1],
                        ins->target);
                break;
            case IR_PRINT:
                fprintf(stderr, "%s=%" PRId64, vm_slot_name(prog, ins->src1), slots[ins->src1]);
                break;
            case IR_PRINTF:
                fprintf(stderr, "printf fmt=\"%s\"", ins->string_id >= 0 ? prog->strings[ins->string_id] : "");
                break;
            case IR_SCANF:
                fprintf(stderr, "scanf fmt=\"%s\"", ins->string_id >= 0 ? prog->strings[ins->string_id] : "");
                break;
            case IR_RETURN:
                fprintf(stderr, "return %s=%" PRId64, vm_slot_name(prog, ins->src1), slots[ins->src1]);
                break;
            case IR_HALT:
                break;
            }
            fputc('\n', stderr);
        }

        switch (ins->op) {
        case IR_LOAD_CONST:
            slots[ins->dest] = ins->imm;
            break;
        case IR_LOAD:
            slots[ins->dest] = slots[ins->src1];
            break;
        case IR_STORE:
            slots[ins->dest] = slots[ins->src1];
            break;
        case IR_ADD:
            slots[ins->dest] = slots[ins->src1] + slots[ins->src2];
            break;
        case IR_SUB:
            slots[ins->dest] = slots[ins->src1] - slots[ins->src2];
            break;
        case IR_MUL:
            slots[ins->dest] = slots[ins->src1] * slots[ins->src2];
            break;
        case IR_DIV:
            if (slots[ins->src2] == 0) {
                fprintf(stderr, "runtime error: division by zero at instruction %d\n", pc);
                free(slots);
                return 1;
            }
            slots[ins->dest] = slots[ins->src1] / slots[ins->src2];
            break;
        case IR_CMP_EQ:
            slots[ins->dest] = slots[ins->src1] == slots[ins->src2] ? 1 : 0;
            break;
        case IR_CMP_NE:
            slots[ins->dest] = slots[ins->src1] != slots[ins->src2] ? 1 : 0;
            break;
        case IR_CMP_LT:
            slots[ins->dest] = slots[ins->src1] < slots[ins->src2] ? 1 : 0;
            break;
        case IR_CMP_GT:
            slots[ins->dest] = slots[ins->src1] > slots[ins->src2] ? 1 : 0;
            break;
        case IR_CMP_LE:
            slots[ins->dest] = slots[ins->src1] <= slots[ins->src2] ? 1 : 0;
            break;
        case IR_CMP_GE:
            slots[ins->dest] = slots[ins->src1] >= slots[ins->src2] ? 1 : 0;
            break;
        case IR_LOGICAL_AND:
            slots[ins->dest] = (slots[ins->src1] != 0 && slots[ins->src2] != 0) ? 1 : 0;
            break;
        case IR_LOGICAL_OR:
            slots[ins->dest] = (slots[ins->src1] != 0 || slots[ins->src2] != 0) ? 1 : 0;
            break;
        case IR_LOGICAL_NOT:
            slots[ins->dest] = (slots[ins->src1] == 0) ? 1 : 0;
            break;
        case IR_JUMP:
            next = ins->target;
            break;
        case IR_JUMP_IF_FALSE:
            if (slots[ins->src1] == 0) {
                next = ins->target;
            }
            break;
        case IR_PRINT:
            printf("%" PRId64 "\n", slots[ins->src1]);
            break;
        case IR_PRINTF:
            execute_printf(prog, ins, slots);
            break;
        case IR_SCANF:
            execute_scanf(prog, ins, slots);
            break;
        case IR_RETURN:
            free(slots);
            return 0;
        case IR_HALT:
            free(slots);
            return 0;
        }

        if (next < 0 || next >= prog->count) {
            fprintf(stderr, "runtime error: invalid jump to %d\n", next);
            free(slots);
            return 1;
        }
        pc = next;
    }

    fprintf(stderr, "runtime error: program counter out of range\n");
    free(slots);
    return 1;
}
