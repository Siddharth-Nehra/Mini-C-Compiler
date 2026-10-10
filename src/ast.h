#ifndef MINI_C_AST_H
#define MINI_C_AST_H

#include "lexer.h"

typedef enum {
    AST_PROGRAM,
    AST_BLOCK,
    AST_VAR_DECL,
    AST_ASSIGN,
    AST_PRINT,
    AST_PRINTF,
    AST_SCANF,
    AST_IF,
    AST_WHILE,
    AST_FOR,
    AST_RETURN,
    AST_BINOP,
    AST_UNARY,
    AST_INT,
    AST_IDENT,
    AST_STRING,
    AST_CHAR,
    AST_ADDR_OF
} AstKind;

typedef struct AstNode AstNode;

struct AstNode {
    AstKind kind;
    int line;
    int col;
    TokenKind op;
    char *name;
    int slot;
    int64_t value;
    AstNode *left;
    AstNode *right;
    AstNode *extra;
    AstNode **children;
    int count;
    int capacity;
};

AstNode *ast_new(AstKind kind, int line, int col);
void ast_add_child(AstNode *parent, AstNode *child);
void ast_free(AstNode *node);
void ast_print(const AstNode *node, int indent);
const char *ast_kind_name(AstKind kind);

#endif
