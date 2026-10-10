#include "semantic.h"

static void sem_error(SemanticContext *ctx, int line, int col, const char *fmt, ...)
{
    va_list ap;
    fprintf(stderr, "%s:%d:%d: semantic error: ", ctx->filename, line, col);
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fputc('\n', stderr);
    ctx->had_error = true;
}

static void analyze_stmt(SemanticContext *ctx, AstNode *node);

static void analyze_expr(SemanticContext *ctx, AstNode *node)
{
    if (node == NULL) {
        return;
    }
    switch (node->kind) {
    case AST_INT:
    case AST_STRING:
    case AST_CHAR:
        break;
    case AST_IDENT:
        if (!symtab_lookup(ctx->table, node->name, &node->slot)) {
            sem_error(ctx, node->line, node->col, "undeclared variable '%s'", node->name);
        }
        break;
    case AST_ADDR_OF:
        if (!symtab_lookup(ctx->table, node->name, &node->slot)) {
            sem_error(ctx, node->line, node->col, "undeclared variable '%s' in address-of operand", node->name);
        }
        break;
    case AST_BINOP:
        analyze_expr(ctx, node->left);
        analyze_expr(ctx, node->right);
        break;
    case AST_UNARY:
        analyze_expr(ctx, node->left);
        break;
    case AST_ASSIGN:
        analyze_stmt(ctx, node);
        break;
    default:
        sem_error(ctx, node->line, node->col, "invalid expression");
        break;
    }
}

static void analyze_stmt(SemanticContext *ctx, AstNode *node)
{
    int i;
    if (node == NULL) {
        return;
    }
    switch (node->kind) {
    case AST_VAR_DECL:
        if (!symtab_declare(ctx->table, node->name, node->line, node->col, &node->slot)) {
            sem_error(ctx, node->line, node->col, "duplicate declaration of '%s' in this scope",
                      node->name);
        }
        if (node->left != NULL) {
            analyze_expr(ctx, node->left);
        }
        break;
    case AST_ASSIGN:
        if (!symtab_lookup(ctx->table, node->name, &node->slot)) {
            sem_error(ctx, node->line, node->col, "assignment to undeclared variable '%s'", node->name);
        }
        if (node->left == NULL) {
            sem_error(ctx, node->line, node->col, "invalid assignment to '%s'", node->name);
        } else {
            analyze_expr(ctx, node->left);
        }
        break;
    case AST_PRINT:
        analyze_expr(ctx, node->left);
        break;
    case AST_PRINTF:
        analyze_expr(ctx, node->left);
        for (i = 0; i < node->count; i++) {
            analyze_expr(ctx, node->children[i]);
        }
        break;
    case AST_SCANF:
        analyze_expr(ctx, node->left);
        for (i = 0; i < node->count; i++) {
            analyze_expr(ctx, node->children[i]);
        }
        break;
    case AST_IF:
        analyze_expr(ctx, node->left);
        analyze_stmt(ctx, node->right);
        if (node->extra != NULL) {
            analyze_stmt(ctx, node->extra);
        }
        break;
    case AST_WHILE:
        analyze_expr(ctx, node->left);
        analyze_stmt(ctx, node->right);
        break;
    case AST_FOR:
        symtab_enter_scope(ctx->table);
        if (node->left != NULL) analyze_stmt(ctx, node->left);
        if (node->right != NULL) analyze_expr(ctx, node->right);
        if (node->extra != NULL) analyze_expr(ctx, node->extra);
        for (i = 0; i < node->count; i++) {
            analyze_stmt(ctx, node->children[i]);
        }
        symtab_leave_scope(ctx->table);
        break;
    case AST_RETURN:
        if (node->left != NULL) {
            analyze_expr(ctx, node->left);
        }
        break;
    case AST_BLOCK:
        symtab_enter_scope(ctx->table);
        for (i = 0; i < node->count; i++) {
            analyze_stmt(ctx, node->children[i]);
        }
        symtab_leave_scope(ctx->table);
        break;
    case AST_BINOP:
    case AST_UNARY:
    case AST_INT:
    case AST_STRING:
    case AST_CHAR:
    case AST_IDENT:
        analyze_expr(ctx, node);
        break;
    default:
        sem_error(ctx, node->line, node->col, "invalid statement");
        break;
    }
}

static void analyze_node(SemanticContext *ctx, AstNode *node)
{
    int i;
    if (node == NULL || node->kind != AST_PROGRAM) {
        sem_error(ctx, 1, 1, "internal error: expected program");
        return;
    }
    for (i = 0; i < node->count; i++) {
        analyze_stmt(ctx, node->children[i]);
    }
}

bool semantic_analyze(AstNode *program, SymbolTable *table, const char *filename)
{
    SemanticContext ctx;
    ctx.filename = filename;
    ctx.table = table;
    ctx.had_error = false;
    analyze_node(&ctx, program);
    return !ctx.had_error;
}
