#include "lexer.h"

const char *token_kind_name(TokenKind kind)
{
    switch (kind) {
    case TOK_EOF:
        return "EOF";
    case TOK_INT_KW:
        return "int";
    case TOK_CHAR_KW:
        return "char";
    case TOK_VOID:
        return "void";
    case TOK_PRINT:
        return "print";
    case TOK_PRINTF:
        return "printf";
    case TOK_SCANF:
        return "scanf";
    case TOK_IF:
        return "if";
    case TOK_ELSE:
        return "else";
    case TOK_WHILE:
        return "while";
    case TOK_FOR:
        return "for";
    case TOK_RETURN:
        return "return";
    case TOK_MAIN:
        return "main";
    case TOK_IDENT:
        return "identifier";
    case TOK_NUMBER:
        return "number";
    case TOK_STRING:
        return "string";
    case TOK_CHAR_LIT:
        return "char_lit";
    case TOK_PLUS:
        return "+";
    case TOK_MINUS:
        return "-";
    case TOK_STAR:
        return "*";
    case TOK_SLASH:
        return "/";
    case TOK_EQ:
        return "=";
    case TOK_EQEQ:
        return "==";
    case TOK_NE:
        return "!=";
    case TOK_LT:
        return "<";
    case TOK_GT:
        return ">";
    case TOK_LE:
        return "<=";
    case TOK_GE:
        return ">=";
    case TOK_ANDAND:
        return "&&";
    case TOK_OROR:
        return "||";
    case TOK_BANG:
        return "!";
    case TOK_INC:
        return "++";
    case TOK_DEC:
        return "--";
    case TOK_AMP:
        return "&";
    case TOK_COMMA:
        return ",";
    case TOK_LPAREN:
        return "(";
    case TOK_RPAREN:
        return ")";
    case TOK_LBRACE:
        return "{";
    case TOK_RBRACE:
        return "}";
    case TOK_SEMI:
        return ";";
    case TOK_ERROR:
        return "error";
    }
    return "?";
}

void token_list_free(TokenList *list)
{
    int i;
    if (list == NULL) {
        return;
    }
    for (i = 0; i < list->count; i++) {
        free(list->items[i].lexeme);
    }
    free(list->items);
    list->items = NULL;
    list->count = 0;
    list->capacity = 0;
}

static void token_list_push(TokenList *list, Token token)
{
    if (list->count >= list->capacity) {
        int cap = list->capacity == 0 ? 64 : list->capacity * 2;
        list->items = xrealloc(list->items, (size_t)cap * sizeof(Token));
        list->capacity = cap;
    }
    list->items[list->count++] = token;
}

static Token make_token(TokenKind kind, int line, int col, const char *lexeme, int64_t value)
{
    Token t;
    t.kind = kind;
    t.line = line;
    t.col = col;
    t.lexeme = xstrdup(lexeme);
    t.value = value;
    return t;
}

static TokenKind keyword_kind(const char *s)
{
    if (strcmp(s, "int") == 0) return TOK_INT_KW;
    if (strcmp(s, "char") == 0) return TOK_CHAR_KW;
    if (strcmp(s, "void") == 0) return TOK_VOID;
    if (strcmp(s, "print") == 0) return TOK_PRINT;
    if (strcmp(s, "printf") == 0) return TOK_PRINTF;
    if (strcmp(s, "scanf") == 0) return TOK_SCANF;
    if (strcmp(s, "if") == 0) return TOK_IF;
    if (strcmp(s, "else") == 0) return TOK_ELSE;
    if (strcmp(s, "while") == 0) return TOK_WHILE;
    if (strcmp(s, "for") == 0) return TOK_FOR;
    if (strcmp(s, "return") == 0) return TOK_RETURN;
    if (strcmp(s, "main") == 0) return TOK_MAIN;
    return TOK_IDENT;
}

static void lex_error(TokenList *list, const char *filename, int line, int col, const char *fmt, ...)
{
    va_list ap;
    fprintf(stderr, "%s:%d:%d: lexical error: ", filename, line, col);
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fputc('\n', stderr);
    list->had_error = true;
}

TokenList lex_source(const char *source, const char *filename)
{
    TokenList list;
    const char *p = source;
    int line = 1;
    int col = 1;

    list.items = NULL;
    list.count = 0;
    list.capacity = 0;
    list.had_error = false;

    while (*p != '\0') {
        char c = *p;

        if (c == ' ' || c == '\t' || c == '\r') {
            p++;
            col++;
            continue;
        }
        if (c == '\n') {
            p++;
            line++;
            col = 1;
            continue;
        }

        if (c == '/' && p[1] == '/') {
            p += 2;
            col += 2;
            while (*p != '\0' && *p != '\n') {
                p++;
                col++;
            }
            continue;
        }

        if (c == '/' && p[1] == '*') {
            p += 2;
            col += 2;
            while (*p != '\0' && !(p[0] == '*' && p[1] == '/')) {
                if (*p == '\n') {
                    line++;
                    col = 1;
                } else {
                    col++;
                }
                p++;
            }
            if (*p == '*' && p[1] == '/') {
                p += 2;
                col += 2;
            } else {
                lex_error(&list, filename, line, col, "unterminated block comment");
            }
            continue;
        }

        /* String Literal */
        if (c == '"') {
            int start_col = col;
            p++;
            col++;
            char buf[1024];
            int len = 0;

            while (*p != '\0' && *p != '"' && *p != '\n') {
                if (*p == '\\') {
                    p++;
                    col++;
                    if (*p == 'n') buf[len++] = '\n';
                    else if (*p == 't') buf[len++] = '\t';
                    else if (*p == 'r') buf[len++] = '\r';
                    else if (*p == '"') buf[len++] = '"';
                    else if (*p == '\\') buf[len++] = '\\';
                    else buf[len++] = *p;
                } else {
                    buf[len++] = *p;
                }
                p++;
                col++;
                if (len >= (int)sizeof(buf) - 1) break;
            }

            if (*p == '"') {
                buf[len] = '\0';
                p++;
                col++;
                token_list_push(&list, make_token(TOK_STRING, line, start_col, buf, 0));
            } else {
                lex_error(&list, filename, line, start_col, "unterminated string literal");
                token_list_push(&list, make_token(TOK_ERROR, line, start_col, "<string>", 0));
            }
            continue;
        }

        /* Character Literal */
        if (c == '\'') {
            int start_col = col;
            p++;
            col++;
            char val = 0;
            if (*p == '\\') {
                p++;
                col++;
                if (*p == 'n') val = '\n';
                else if (*p == 't') val = '\t';
                else if (*p == '0') val = '\0';
                else val = *p;
            } else {
                val = *p;
            }
            p++;
            col++;
            if (*p == '\'') {
                p++;
                col++;
                char buf[4];
                snprintf(buf, sizeof(buf), "%c", val);
                token_list_push(&list, make_token(TOK_CHAR_LIT, line, start_col, buf, (int64_t)val));
            } else {
                lex_error(&list, filename, line, start_col, "unterminated character literal");
                token_list_push(&list, make_token(TOK_ERROR, line, start_col, "<char>", 0));
            }
            continue;
        }

        if (isalpha((unsigned char)c) || c == '_') {
            const char *start = p;
            int start_col = col;
            char buf[256];
            size_t len;
            TokenKind kind;

            while (isalnum((unsigned char)*p) || *p == '_') {
                p++;
                col++;
            }
            len = (size_t)(p - start);
            if (len >= sizeof(buf)) {
                lex_error(&list, filename, line, start_col, "identifier is too long");
                token_list_push(&list, make_token(TOK_ERROR, line, start_col, "<too-long-ident>", 0));
                continue;
            }
            memcpy(buf, start, len);
            buf[len] = '\0';
            kind = keyword_kind(buf);
            token_list_push(&list, make_token(kind, line, start_col, buf, 0));
            continue;
        }

        if (isdigit((unsigned char)c)) {
            const char *start = p;
            int start_col = col;
            char buf[64];
            size_t len;
            char *endptr;
            unsigned long long val;

            while (isdigit((unsigned char)*p)) {
                p++;
                col++;
            }
            if (isalpha((unsigned char)*p) || *p == '_') {
                lex_error(&list, filename, line, start_col,
                          "invalid number: identifier cannot start with a digit");
                while (isalnum((unsigned char)*p) || *p == '_') {
                    p++;
                    col++;
                }
                token_list_push(&list, make_token(TOK_ERROR, line, start_col, "<invalid-number>", 0));
                continue;
            }
            len = (size_t)(p - start);
            if (len >= sizeof(buf)) {
                lex_error(&list, filename, line, start_col, "integer literal is too long");
                token_list_push(&list, make_token(TOK_ERROR, line, start_col, "<overflow>", 0));
                continue;
            }
            memcpy(buf, start, len);
            buf[len] = '\0';
            errno = 0;
            val = strtoull(buf, &endptr, 10);
            if (errno == ERANGE || val > (unsigned long long)INT64_MAX) {
                lex_error(&list, filename, line, start_col, "integer literal '%s' is out of range", buf);
                token_list_push(&list, make_token(TOK_ERROR, line, start_col, buf, 0));
                continue;
            }
            token_list_push(&list, make_token(TOK_NUMBER, line, start_col, buf, (int64_t)val));
            continue;
        }

        /* Two-character operators */
        if (c == '+' && p[1] == '+') {
            token_list_push(&list, make_token(TOK_INC, line, col, "++", 0));
            p += 2; col += 2; continue;
        }
        if (c == '-' && p[1] == '-') {
            token_list_push(&list, make_token(TOK_DEC, line, col, "--", 0));
            p += 2; col += 2; continue;
        }
        if (c == '&' && p[1] == '&') {
            token_list_push(&list, make_token(TOK_ANDAND, line, col, "&&", 0));
            p += 2; col += 2; continue;
        }
        if (c == '|' && p[1] == '|') {
            token_list_push(&list, make_token(TOK_OROR, line, col, "||", 0));
            p += 2; col += 2; continue;
        }
        if (c == '=' && p[1] == '=') {
            token_list_push(&list, make_token(TOK_EQEQ, line, col, "==", 0));
            p += 2; col += 2; continue;
        }
        if (c == '!' && p[1] == '=') {
            token_list_push(&list, make_token(TOK_NE, line, col, "!=", 0));
            p += 2; col += 2; continue;
        }
        if (c == '<' && p[1] == '=') {
            token_list_push(&list, make_token(TOK_LE, line, col, "<=", 0));
            p += 2; col += 2; continue;
        }
        if (c == '>' && p[1] == '=') {
            token_list_push(&list, make_token(TOK_GE, line, col, ">=", 0));
            p += 2; col += 2; continue;
        }

        switch (c) {
        case '+':
            token_list_push(&list, make_token(TOK_PLUS, line, col, "+", 0));
            break;
        case '-':
            token_list_push(&list, make_token(TOK_MINUS, line, col, "-", 0));
            break;
        case '*':
            token_list_push(&list, make_token(TOK_STAR, line, col, "*", 0));
            break;
        case '/':
            token_list_push(&list, make_token(TOK_SLASH, line, col, "/", 0));
            break;
        case '=':
            token_list_push(&list, make_token(TOK_EQ, line, col, "=", 0));
            break;
        case '!':
            token_list_push(&list, make_token(TOK_BANG, line, col, "!", 0));
            break;
        case '&':
            token_list_push(&list, make_token(TOK_AMP, line, col, "&", 0));
            break;
        case ',':
            token_list_push(&list, make_token(TOK_COMMA, line, col, ",", 0));
            break;
        case '<':
            token_list_push(&list, make_token(TOK_LT, line, col, "<", 0));
            break;
        case '>':
            token_list_push(&list, make_token(TOK_GT, line, col, ">", 0));
            break;
        case '(':
            token_list_push(&list, make_token(TOK_LPAREN, line, col, "(", 0));
            break;
        case ')':
            token_list_push(&list, make_token(TOK_RPAREN, line, col, ")", 0));
            break;
        case '{':
            token_list_push(&list, make_token(TOK_LBRACE, line, col, "{", 0));
            break;
        case '}':
            token_list_push(&list, make_token(TOK_RBRACE, line, col, "}", 0));
            break;
        case ';':
            token_list_push(&list, make_token(TOK_SEMI, line, col, ";", 0));
            break;
        default: {
            char msg[8];
            msg[0] = c;
            msg[1] = '\0';
            lex_error(&list, filename, line, col, "unexpected character '%s'", msg);
            token_list_push(&list, make_token(TOK_ERROR, line, col, msg, 0));
            break;
        }
        }
        p++;
        col++;
    }

    token_list_push(&list, make_token(TOK_EOF, line, col, "", 0));
    return list;
}

void print_tokens(const TokenList *list)
{
    int i;
    for (i = 0; i < list->count; i++) {
        const Token *t = &list->items[i];
        if (t->kind == TOK_NUMBER) {
            printf("%4d:%-4d %-12s %s  (value=%" PRId64 ")\n", t->line, t->col, token_kind_name(t->kind),
                   t->lexeme, t->value);
        } else if (t->kind == TOK_IDENT || t->kind == TOK_STRING) {
            printf("%4d:%-4d %-12s %s\n", t->line, t->col, token_kind_name(t->kind), t->lexeme);
        } else if (t->kind == TOK_EOF) {
            printf("%4d:%-4d %-12s\n", t->line, t->col, token_kind_name(t->kind));
        } else {
            printf("%4d:%-4d %-12s %s\n", t->line, t->col, token_kind_name(t->kind), t->lexeme);
        }
    }
}
