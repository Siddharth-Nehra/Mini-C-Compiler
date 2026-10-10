#ifndef MINI_C_LEXER_H
#define MINI_C_LEXER_H

#include "common.h"

typedef enum {
    TOK_EOF = 0,
    TOK_INT_KW,
    TOK_CHAR_KW,
    TOK_VOID,
    TOK_PRINT,
    TOK_PRINTF,
    TOK_SCANF,
    TOK_IF,
    TOK_ELSE,
    TOK_WHILE,
    TOK_FOR,
    TOK_RETURN,
    TOK_MAIN,
    TOK_IDENT,
    TOK_NUMBER,
    TOK_STRING,
    TOK_CHAR_LIT,
    TOK_PLUS,
    TOK_MINUS,
    TOK_STAR,
    TOK_SLASH,
    TOK_EQ,
    TOK_EQEQ,
    TOK_NE,
    TOK_LT,
    TOK_GT,
    TOK_LE,
    TOK_GE,
    TOK_ANDAND,
    TOK_OROR,
    TOK_BANG,
    TOK_INC,
    TOK_DEC,
    TOK_AMP,
    TOK_COMMA,
    TOK_LPAREN,
    TOK_RPAREN,
    TOK_LBRACE,
    TOK_RBRACE,
    TOK_SEMI,
    TOK_ERROR
} TokenKind;

typedef struct {
    TokenKind kind;
    int line;
    int col;
    char *lexeme;
    int64_t value;
} Token;

typedef struct {
    Token *items;
    int count;
    int capacity;
    bool had_error;
} TokenList;

const char *token_kind_name(TokenKind kind);
void token_list_free(TokenList *list);
TokenList lex_source(const char *source, const char *filename);
void print_tokens(const TokenList *list);

#endif
