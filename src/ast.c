#include "ast.h"

const char *ast_kind_name(AstKind kind)
{
    switch (kind) {
    case AST_PROGRAM:
        return "Program";
    case AST_BLOCK:
        return "Block";
    case AST_VAR_DECL:
        return "VarDecl";
    case AST_ASSIGN:
        return "Assign";
    case AST_PRINT:
        return "Print";
    case AST_PRINTF:
        return "Printf";
    case AST_SCANF:
        return "Scanf";
    case AST_IF:
        return "If";
    case AST_WHILE:
        return "While";
    case AST_FOR:
        return "For";
    case AST_RETURN:
        return "Return";
    case AST_BINOP:
        return "BinOp";
    case AST_UNARY:
        return "Unary";
    case AST_INT:
        return "Int";
    case AST_IDENT:
        return "Ident";
    case AST_STRING:
        return "String";
    case AST_CHAR:
        return "Char";
    case AST_ADDR_OF:
        return "AddrOf";
    }
    return "?";
}

AstNode *ast_new(AstKind kind, int line, int col)
{
    AstNode *node = xcalloc(1, sizeof(AstNode));
    node->kind = kind;
    node->line = line;
    node->col = col;
    node->slot = -1;
    return node;
}

void ast_add_child(AstNode *parent, AstNode *child)
{
    if (child == NULL) return;
    if (parent->count >= parent->capacity) {
        int cap = parent->capacity == 0 ? 4 : parent->capacity * 2;
        parent->children = xrealloc(parent->children, (size_t)cap * sizeof(AstNode *));
        parent->capacity = cap;
    }
    parent->children[parent->count++] = child;
}

void ast_free(AstNode *node)
{
    int i;
    if (node == NULL) return;
    free(node->name);
    ast_free(node->left);
    ast_free(node->right);
    ast_free(node->extra);
    for (i = 0; i < node->count; i++) {
        ast_free(node->children[i]);
    }
    free(node->children);
    free(node);
}

void ast_print(const AstNode *node, int indent)
{
    int i;
    if (node == NULL) return;

    for (i = 0; i < indent; i++) {
        printf("  ");
    }

    printf("%s", ast_kind_name(node->kind));
    if (node->name != NULL) {
        printf(" %s", node->name);
    }
    if (node->kind == AST_INT) {
        printf(" %" PRId64, node->value);
    } else if (node->kind == AST_STRING) {
        printf(" \"%s\"", node->name ? node->name : "");
    } else if (node->kind == AST_CHAR) {
        printf(" '%c'", (char)node->value);
    } else if (node->kind == AST_BINOP || node->kind == AST_UNARY) {
        printf(" %s", token_kind_name(node->op));
    }
    printf("\n");

    if (node->left) ast_print(node->left, indent + 1);
    if (node->right) ast_print(node->right, indent + 1);
    if (node->extra) ast_print(node->extra, indent + 1);
    for (i = 0; i < node->count; i++) {
        ast_print(node->children[i], indent + 1);
    }
}
