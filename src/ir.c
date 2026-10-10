#include "ir.h"

const char *ir_op_name(IrOp op)
{
    switch (op) {
    case IR_LOAD_CONST:
        return "LOAD_CONST";
    case IR_LOAD:
        return "LOAD";
    case IR_STORE:
        return "STORE";
    case IR_ADD:
        return "ADD";
    case IR_SUB:
        return "SUB";
    case IR_MUL:
        return "MUL";
    case IR_DIV:
        return "DIV";
    case IR_CMP_EQ:
        return "CMP_EQ";
    case IR_CMP_NE:
        return "CMP_NE";
    case IR_CMP_LT:
        return "CMP_LT";
    case IR_CMP_GT:
        return "CMP_GT";
    case IR_CMP_LE:
        return "CMP_LE";
    case IR_CMP_GE:
        return "CMP_GE";
    case IR_LOGICAL_AND:
        return "LOGICAL_AND";
    case IR_LOGICAL_OR:
        return "LOGICAL_OR";
    case IR_LOGICAL_NOT:
        return "LOGICAL_NOT";
    case IR_JUMP:
        return "JUMP";
    case IR_JUMP_IF_FALSE:
        return "JUMP_IF_FALSE";
    case IR_PRINT:
        return "PRINT";
    case IR_PRINTF:
        return "PRINTF";
    case IR_SCANF:
        return "SCANF";
    case IR_RETURN:
        return "RETURN";
    case IR_HALT:
        return "HALT";
    }
    return "?";
}

void ir_program_free(IrProgram *prog)
{
    int i;
    if (prog == NULL) {
        return;
    }
    for (i = 0; i < prog->nslots; i++) {
        free(prog->slot_names[i]);
    }
    free(prog->slot_names);

    for (i = 0; i < prog->nstrings; i++) {
        free(prog->strings[i]);
    }
    free(prog->strings);

    for (i = 0; i < prog->count; i++) {
        free(prog->insts[i].arg_slots);
    }
    free(prog->insts);
    free(prog);
}

static void ir_emit(IrProgram *prog, IrInst inst)
{
    if (prog->count >= prog->capacity) {
        int cap = prog->capacity == 0 ? 64 : prog->capacity * 2;
        prog->insts = xrealloc(prog->insts, (size_t)cap * sizeof(IrInst));
        prog->capacity = cap;
    }
    prog->insts[prog->count++] = inst;
}

static int ir_emit_op(IrProgram *prog, IrOp op, int dest, int src1, int src2, int64_t imm, int target)
{
    IrInst inst;
    memset(&inst, 0, sizeof(IrInst));
    inst.op = op;
    inst.dest = dest;
    inst.src1 = src1;
    inst.src2 = src2;
    inst.imm = imm;
    inst.target = target;
    inst.string_id = -1;
    ir_emit(prog, inst);
    return prog->count - 1;
}

static int alloc_string(IrProgram *prog, const char *str)
{
    int i;
    for (i = 0; i < prog->nstrings; i++) {
        if (strcmp(prog->strings[i], str) == 0) {
            return i;
        }
    }
    if (prog->nstrings >= prog->string_cap) {
        int cap = prog->string_cap == 0 ? 16 : prog->string_cap * 2;
        prog->strings = xrealloc(prog->strings, (size_t)cap * sizeof(char *));
        prog->string_cap = cap;
    }
    int id = prog->nstrings++;
    prog->strings[id] = xstrdup(str);
    return id;
}

typedef struct {
    IrProgram *prog;
    int temp_id;
} IrGen;

static bool name_taken(IrProgram *prog, const char *name)
{
    int i;
    for (i = 0; i < prog->nslots; i++) {
        if (prog->slot_names[i] != NULL && strcmp(prog->slot_names[i], name) == 0) {
            return true;
        }
    }
    return false;
}

static int alloc_slot(IrProgram *prog, const char *name)
{
    int slot;
    if (prog->nslots >= prog->slot_cap) {
        int cap = prog->slot_cap == 0 ? 32 : prog->slot_cap * 2;
        char **names = xrealloc(prog->slot_names, (size_t)cap * sizeof(char *));
        prog->slot_names = names;
        while (prog->slot_cap < cap) {
            prog->slot_names[prog->slot_cap] = NULL;
            prog->slot_cap++;
        }
    }
    slot = prog->nslots++;
    prog->slot_names[slot] = xstrdup(name);
    return slot;
}

static int alloc_temp(IrGen *g)
{
    char name[32];
    do {
        g->temp_id++;
        snprintf(name, sizeof(name), "t%d", g->temp_id);
    } while (name_taken(g->prog, name));
    return alloc_slot(g->prog, name);
}

static const char *slot_name(const IrProgram *prog, int slot)
{
    if (slot < 0 || slot >= prog->nslots || prog->slot_names[slot] == NULL) {
        return "?";
    }
    return prog->slot_names[slot];
}

static int gen_expr(IrGen *g, AstNode *node);
static void gen_stmt(IrGen *g, AstNode *node);

static IrOp binop_to_ir(TokenKind op)
{
    switch (op) {
    case TOK_PLUS:
        return IR_ADD;
    case TOK_MINUS:
        return IR_SUB;
    case TOK_STAR:
        return IR_MUL;
    case TOK_SLASH:
        return IR_DIV;
    case TOK_EQEQ:
        return IR_CMP_EQ;
    case TOK_NE:
        return IR_CMP_NE;
    case TOK_LT:
        return IR_CMP_LT;
    case TOK_GT:
        return IR_CMP_GT;
    case TOK_LE:
        return IR_CMP_LE;
    case TOK_GE:
        return IR_CMP_GE;
    case TOK_ANDAND:
        return IR_LOGICAL_AND;
    case TOK_OROR:
        return IR_LOGICAL_OR;
    default:
        return IR_ADD;
    }
}

static int gen_expr(IrGen *g, AstNode *node)
{
    int dest, left, right, zero;

    if (node == NULL) {
        dest = alloc_temp(g);
        ir_emit_op(g->prog, IR_LOAD_CONST, dest, -1, -1, 0, -1);
        return dest;
    }

    switch (node->kind) {
    case AST_INT:
    case AST_CHAR:
        dest = alloc_temp(g);
        ir_emit_op(g->prog, IR_LOAD_CONST, dest, -1, -1, node->value, -1);
        return dest;
    case AST_STRING:
        dest = alloc_temp(g);
        int str_id = alloc_string(g->prog, node->name ? node->name : "");
        ir_emit_op(g->prog, IR_LOAD_CONST, dest, -1, -1, str_id, -1);
        return dest;
    case AST_IDENT:
        return node->slot;
    case AST_ADDR_OF:
        return node->slot;
    case AST_UNARY:
        if (node->op == TOK_MINUS) {
            zero = alloc_temp(g);
            ir_emit_op(g->prog, IR_LOAD_CONST, zero, -1, -1, 0, -1);
            right = gen_expr(g, node->left);
            dest = alloc_temp(g);
            ir_emit_op(g->prog, IR_SUB, dest, zero, right, 0, -1);
            return dest;
        } else if (node->op == TOK_BANG) {
            right = gen_expr(g, node->left);
            dest = alloc_temp(g);
            ir_emit_op(g->prog, IR_LOGICAL_NOT, dest, right, -1, 0, -1);
            return dest;
        }
        return gen_expr(g, node->left);
    case AST_BINOP:
        left = gen_expr(g, node->left);
        right = gen_expr(g, node->right);
        dest = alloc_temp(g);
        ir_emit_op(g->prog, binop_to_ir(node->op), dest, left, right, 0, -1);
        return dest;
    case AST_ASSIGN:
        right = gen_expr(g, node->left);
        ir_emit_op(g->prog, IR_STORE, node->slot, right, -1, 0, -1);
        return node->slot;
    default:
        dest = alloc_temp(g);
        ir_emit_op(g->prog, IR_LOAD_CONST, dest, -1, -1, 0, -1);
        return dest;
    }
}

static void gen_stmt(IrGen *g, AstNode *node)
{
    int i, src, cond, jf, jmp, loop;

    if (node == NULL) return;

    switch (node->kind) {
    case AST_VAR_DECL:
        if (node->left != NULL) {
            src = gen_expr(g, node->left);
            ir_emit_op(g->prog, IR_STORE, node->slot, src, -1, 0, -1);
        } else {
            ir_emit_op(g->prog, IR_LOAD_CONST, node->slot, -1, -1, 0, -1);
        }
        break;
    case AST_ASSIGN:
        src = gen_expr(g, node->left);
        ir_emit_op(g->prog, IR_STORE, node->slot, src, -1, 0, -1);
        break;
    case AST_PRINT:
        src = gen_expr(g, node->left);
        ir_emit_op(g->prog, IR_PRINT, -1, src, -1, 0, -1);
        break;
    case AST_PRINTF: {
        IrInst inst;
        memset(&inst, 0, sizeof(IrInst));
        inst.op = IR_PRINTF;
        inst.dest = -1;
        inst.src1 = -1;
        inst.src2 = -1;
        inst.target = -1;

        if (node->left != NULL && node->left->kind == AST_STRING) {
            inst.string_id = alloc_string(g->prog, node->left->name ? node->left->name : "");
        } else if (node->left != NULL) {
            int str_slot = gen_expr(g, node->left);
            inst.string_id = (int)g->prog->insts[str_slot].imm;
        }

        inst.arg_count = node->count;
        if (node->count > 0) {
            inst.arg_slots = xcalloc((size_t)node->count, sizeof(int));
            for (i = 0; i < node->count; i++) {
                inst.arg_slots[i] = gen_expr(g, node->children[i]);
            }
        }
        ir_emit(g->prog, inst);
        break;
    }
    case AST_SCANF: {
        IrInst inst;
        memset(&inst, 0, sizeof(IrInst));
        inst.op = IR_SCANF;
        inst.dest = -1;
        inst.src1 = -1;
        inst.src2 = -1;
        inst.target = -1;

        if (node->left != NULL && node->left->kind == AST_STRING) {
            inst.string_id = alloc_string(g->prog, node->left->name ? node->left->name : "");
        }

        inst.arg_count = node->count;
        if (node->count > 0) {
            inst.arg_slots = xcalloc((size_t)node->count, sizeof(int));
            for (i = 0; i < node->count; i++) {
                inst.arg_slots[i] = gen_expr(g, node->children[i]);
            }
        }
        ir_emit(g->prog, inst);
        break;
    }
    case AST_IF:
        cond = gen_expr(g, node->left);
        jf = ir_emit_op(g->prog, IR_JUMP_IF_FALSE, -1, cond, -1, 0, -1);
        gen_stmt(g, node->right);
        if (node->extra != NULL) {
            jmp = ir_emit_op(g->prog, IR_JUMP, -1, -1, -1, 0, -1);
            g->prog->insts[jf].target = g->prog->count;
            gen_stmt(g, node->extra);
            g->prog->insts[jmp].target = g->prog->count;
        } else {
            g->prog->insts[jf].target = g->prog->count;
        }
        break;
    case AST_WHILE:
        loop = g->prog->count;
        cond = gen_expr(g, node->left);
        jf = ir_emit_op(g->prog, IR_JUMP_IF_FALSE, -1, cond, -1, 0, -1);
        gen_stmt(g, node->right);
        ir_emit_op(g->prog, IR_JUMP, -1, -1, -1, 0, loop);
        g->prog->insts[jf].target = g->prog->count;
        break;
    case AST_FOR:
        if (node->left != NULL) gen_stmt(g, node->left);
        loop = g->prog->count;
        jf = -1;
        if (node->right != NULL) {
            cond = gen_expr(g, node->right);
            jf = ir_emit_op(g->prog, IR_JUMP_IF_FALSE, -1, cond, -1, 0, -1);
        }
        for (i = 0; i < node->count; i++) {
            gen_stmt(g, node->children[i]);
        }
        if (node->extra != NULL) {
            gen_expr(g, node->extra);
        }
        ir_emit_op(g->prog, IR_JUMP, -1, -1, -1, 0, loop);
        if (jf != -1) {
            g->prog->insts[jf].target = g->prog->count;
        }
        break;
    case AST_RETURN:
        if (node->left != NULL) {
            src = gen_expr(g, node->left);
            ir_emit_op(g->prog, IR_RETURN, -1, src, -1, 0, -1);
        } else {
            ir_emit_op(g->prog, IR_HALT, -1, -1, -1, 0, -1);
        }
        break;
    case AST_BLOCK:
    case AST_PROGRAM:
        for (i = 0; i < node->count; i++) {
            gen_stmt(g, node->children[i]);
        }
        break;
    default:
        gen_expr(g, node);
        break;
    }
}

IrProgram *ir_generate(AstNode *ast, SymbolTable *table)
{
    IrProgram *prog = xcalloc(1, sizeof(IrProgram));
    IrGen gen;
    int i;
    int nvars = symtab_slot_count(table);

    gen.prog = prog;
    gen.temp_id = 0;

    for (i = 0; i < nvars; i++) {
        alloc_slot(prog, symtab_slot_name(table, i));
    }

    gen_stmt(&gen, ast);
    ir_emit_op(prog, IR_HALT, -1, -1, -1, 0, -1);
    return prog;
}

static const char *binop_sym(IrOp op)
{
    switch (op) {
    case IR_ADD: return "+";
    case IR_SUB: return "-";
    case IR_MUL: return "*";
    case IR_DIV: return "/";
    case IR_CMP_EQ: return "==";
    case IR_CMP_NE: return "!=";
    case IR_CMP_LT: return "<";
    case IR_CMP_GT: return ">";
    case IR_CMP_LE: return "<=";
    case IR_CMP_GE: return ">=";
    case IR_LOGICAL_AND: return "&&";
    case IR_LOGICAL_OR: return "||";
    default: return "?";
    }
}

static bool is_alu(IrOp op)
{
    return op == IR_ADD || op == IR_SUB || op == IR_MUL || op == IR_DIV || op == IR_CMP_EQ ||
           op == IR_CMP_NE || op == IR_CMP_LT || op == IR_CMP_GT || op == IR_CMP_LE || op == IR_CMP_GE ||
           op == IR_LOGICAL_AND || op == IR_LOGICAL_OR;
}

void ir_print(const IrProgram *prog)
{
    int i;
    printf("=== Three-Address Code ===\n");
    for (i = 0; i < prog->count; i++) {
        const IrInst *ins = &prog->insts[i];
        printf("%4d: ", i);
        if (ins->op == IR_LOAD_CONST) {
            printf("%s = %" PRId64 "\n", slot_name(prog, ins->dest), ins->imm);
        } else if (ins->op == IR_LOAD || ins->op == IR_STORE) {
            printf("%s = %s\n", slot_name(prog, ins->dest), slot_name(prog, ins->src1));
        } else if (is_alu(ins->op)) {
            printf("%s = %s %s %s\n", slot_name(prog, ins->dest), slot_name(prog, ins->src1),
                   binop_sym(ins->op), slot_name(prog, ins->src2));
        } else if (ins->op == IR_LOGICAL_NOT) {
            printf("%s = !%s\n", slot_name(prog, ins->dest), slot_name(prog, ins->src1));
        } else if (ins->op == IR_JUMP) {
            printf("goto %d\n", ins->target);
        } else if (ins->op == IR_JUMP_IF_FALSE) {
            printf("if_false %s goto %d\n", slot_name(prog, ins->src1), ins->target);
        } else if (ins->op == IR_PRINT) {
            printf("print %s\n", slot_name(prog, ins->src1));
        } else if (ins->op == IR_PRINTF) {
            printf("printf \"%s\"", ins->string_id >= 0 ? prog->strings[ins->string_id] : "");
            for (int k = 0; k < ins->arg_count; k++) {
                printf(", %s", slot_name(prog, ins->arg_slots[k]));
            }
            printf("\n");
        } else if (ins->op == IR_SCANF) {
            printf("scanf \"%s\"", ins->string_id >= 0 ? prog->strings[ins->string_id] : "");
            for (int k = 0; k < ins->arg_count; k++) {
                printf(", &%s", slot_name(prog, ins->arg_slots[k]));
            }
            printf("\n");
        } else if (ins->op == IR_RETURN) {
            printf("return %s\n", slot_name(prog, ins->src1));
        } else if (ins->op == IR_HALT) {
            printf("halt\n");
        } else {
            printf("%s\n", ir_op_name(ins->op));
        }
    }

    printf("\n=== VM Instructions ===\n");
    for (i = 0; i < prog->count; i++) {
        const IrInst *ins = &prog->insts[i];
        printf("%4d: %-14s", i, ir_op_name(ins->op));
        switch (ins->op) {
        case IR_LOAD_CONST:
            printf(" dest=%s imm=%" PRId64 "\n", slot_name(prog, ins->dest), ins->imm);
            break;
        case IR_LOAD:
        case IR_STORE:
            printf(" dest=%s src=%s\n", slot_name(prog, ins->dest), slot_name(prog, ins->src1));
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
            printf(" dest=%s a=%s b=%s\n", slot_name(prog, ins->dest), slot_name(prog, ins->src1),
                   slot_name(prog, ins->src2));
            break;
        case IR_LOGICAL_NOT:
            printf(" dest=%s src=%s\n", slot_name(prog, ins->dest), slot_name(prog, ins->src1));
            break;
        case IR_JUMP:
            printf(" target=%d\n", ins->target);
            break;
        case IR_JUMP_IF_FALSE:
            printf(" cond=%s target=%d\n", slot_name(prog, ins->src1), ins->target);
            break;
        case IR_PRINT:
            printf(" src=%s\n", slot_name(prog, ins->src1));
            break;
        case IR_PRINTF:
            printf(" str=\"%s\" args=%d\n", ins->string_id >= 0 ? prog->strings[ins->string_id] : "", ins->arg_count);
            break;
        case IR_SCANF:
            printf(" str=\"%s\" args=%d\n", ins->string_id >= 0 ? prog->strings[ins->string_id] : "", ins->arg_count);
            break;
        case IR_RETURN:
            printf(" val=%s\n", slot_name(prog, ins->src1));
            break;
        case IR_HALT:
            printf("\n");
            break;
        }
    }
}
