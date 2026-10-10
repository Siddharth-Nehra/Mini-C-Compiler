#ifndef MINI_C_SEMANTIC_H
#define MINI_C_SEMANTIC_H

#include "ast.h"
#include "symbol_table.h"

typedef struct {
    const char *filename;
    SymbolTable *table;
    bool had_error;
} SemanticContext;

bool semantic_analyze(AstNode *program, SymbolTable *table, const char *filename);

#endif
