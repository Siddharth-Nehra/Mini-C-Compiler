#ifndef MINI_C_PARSER_H
#define MINI_C_PARSER_H

#include "ast.h"

typedef struct {
    const TokenList *tokens;
    const char *filename;
    int current;
    bool had_error;
} Parser;

AstNode *parse_program(Parser *parser);
void parser_init(Parser *parser, const TokenList *tokens, const char *filename);

#endif
