#ifndef MINI_C_IR_H
#define MINI_C_IR_H

#include "ast.h"
#include "symbol_table.h"

typedef enum {
    IR_LOAD_CONST,
    IR_LOAD,
    IR_STORE,
    IR_ADD,
    IR_SUB,
    IR_MUL,
    IR_DIV,
    IR_CMP_EQ,
    IR_CMP_NE,
    IR_CMP_LT,
    IR_CMP_GT,
    IR_CMP_LE,
    IR_CMP_GE,
    IR_LOGICAL_AND,
    IR_LOGICAL_OR,
    IR_LOGICAL_NOT,
    IR_JUMP,
    IR_JUMP_IF_FALSE,
    IR_PRINT,
    IR_PRINTF,
    IR_SCANF,
    IR_RETURN,
    IR_HALT
} IrOp;

typedef struct {
    IrOp op;
    int dest;
    int src1;
    int src2;
    int64_t imm;
    int target;
    int *arg_slots;
    int arg_count;
    int string_id;
} IrInst;

typedef struct {
    IrInst *insts;
    int count;
    int capacity;
    char **slot_names;
    int nslots;
    int slot_cap;
    char **strings;
    int nstrings;
    int string_cap;
} IrProgram;

void ir_program_free(IrProgram *prog);
IrProgram *ir_generate(AstNode *ast, SymbolTable *table);
void ir_print(const IrProgram *prog);
const char *ir_op_name(IrOp op);

#endif
