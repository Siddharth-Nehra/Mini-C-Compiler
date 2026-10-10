#include "parser.h"

void parser_init(Parser *parser, const TokenList *tokens, const char *filename)
{
    parser->tokens = tokens;
    parser->filename = filename;
    parser->current = 0;
    parser->had_error = false;
}

static Token peek(Parser *p)
{
    return p->tokens->items[p->current];
}

static Token previous(Parser *p)
{
    return p->tokens->items[p->current - 1];
}

static bool is_at_end(Parser *p)
{
    return peek(p).kind == TOK_EOF;
}

static Token advance(Parser *p)
{
    if (!is_at_end(p)) {
        p->current++;
    }
    return previous(p);
}

static bool check(Parser *p, TokenKind kind)
{
    if (is_at_end(p)) {
        return kind == TOK_EOF;
    }
    return peek(p).kind == kind;
}

static bool match_tok(Parser *p, TokenKind kind)
{
    if (check(p, kind)) {
        advance(p);
        return true;
    }
    return false;
}

static void parse_error(Parser *p, Token tok, const char *fmt, ...)
{
    va_list ap;
    fprintf(stderr, "%s:%d:%d: syntax error: ", p->filename, tok.line, tok.col);
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fputc('\n', stderr);
    p->had_error = true;
}

static Token consume(Parser *p, TokenKind kind, const char *message)
{
    if (check(p, kind)) {
        return advance(p);
    }
    parse_error(p, peek(p), "%s (found '%s')", message, token_kind_name(peek(p).kind));
    return peek(p);
}

static AstNode *parse_stmt(Parser *p);
static AstNode *parse_expr(Parser *p);

static AstNode *parse_primary(Parser *p)
{
    Token t;
    AstNode *node;

    if (match_tok(p, TOK_NUMBER)) {
        t = previous(p);
        node = ast_new(AST_INT, t.line, t.col);
        node->value = t.value;
        return node;
    }
    if (match_tok(p, TOK_STRING)) {
        t = previous(p);
        node = ast_new(AST_STRING, t.line, t.col);
        node->name = xstrdup(t.lexeme);
        return node;
    }
    if (match_tok(p, TOK_CHAR_LIT)) {
        t = previous(p);
        node = ast_new(AST_CHAR, t.line, t.col);
        node->value = t.value;
        return node;
    }
    if (match_tok(p, TOK_IDENT) || match_tok(p, TOK_MAIN)) {
        t = previous(p);
        node = ast_new(AST_IDENT, t.line, t.col);
        node->name = xstrdup(t.lexeme);

        /* Handle postfix ++ and -- */
        if (match_tok(p, TOK_INC) || match_tok(p, TOK_DEC)) {
            Token op = previous(p);
            AstNode *inc_node = ast_new(AST_BINOP, op.line, op.col);
            inc_node->op = op.kind == TOK_INC ? TOK_PLUS : TOK_MINUS;
            inc_node->left = node;
            
            AstNode *one = ast_new(AST_INT, op.line, op.col);
            one->value = 1;
            inc_node->right = one;

            AstNode *assign = ast_new(AST_ASSIGN, op.line, op.col);
            assign->name = xstrdup(node->name);
            assign->left = inc_node;
            return assign;
        }
        return node;
    }
    if (match_tok(p, TOK_LPAREN)) {
        AstNode *expr;
        t = previous(p);
        expr = parse_expr(p);
        consume(p, TOK_RPAREN, "expected ')' after expression");
        return expr;
    }

    t = peek(p);
    parse_error(p, t, "expected expression, found '%s'", token_kind_name(t.kind));
    return ast_new(AST_INT, t.line, t.col);
}

static AstNode *parse_unary(Parser *p)
{
    if (match_tok(p, TOK_MINUS)) {
        Token t = previous(p);
        AstNode *node = ast_new(AST_UNARY, t.line, t.col);
        node->op = TOK_MINUS;
        node->left = parse_unary(p);
        return node;
    }
    if (match_tok(p, TOK_BANG)) {
        Token t = previous(p);
        AstNode *node = ast_new(AST_UNARY, t.line, t.col);
        node->op = TOK_BANG;
        node->left = parse_unary(p);
        return node;
    }
    if (match_tok(p, TOK_AMP)) {
        Token t = previous(p);
        Token var = consume(p, TOK_IDENT, "expected variable name after '&'");
        AstNode *node = ast_new(AST_ADDR_OF, t.line, t.col);
        node->name = xstrdup(var.lexeme);
        return node;
    }
    if (match_tok(p, TOK_INC) || match_tok(p, TOK_DEC)) {
        Token op = previous(p);
        Token var = consume(p, TOK_IDENT, "expected variable after prefix operator");
        AstNode *ident = ast_new(AST_IDENT, var.line, var.col);
        ident->name = xstrdup(var.lexeme);

        AstNode *inc_node = ast_new(AST_BINOP, op.line, op.col);
        inc_node->op = op.kind == TOK_INC ? TOK_PLUS : TOK_MINUS;
        inc_node->left = ident;
        
        AstNode *one = ast_new(AST_INT, op.line, op.col);
        one->value = 1;
        inc_node->right = one;

        AstNode *assign = ast_new(AST_ASSIGN, op.line, op.col);
        assign->name = xstrdup(var.lexeme);
        assign->left = inc_node;
        return assign;
    }
    return parse_primary(p);
}

static AstNode *parse_multiplicative(Parser *p)
{
    AstNode *left = parse_unary(p);
    while (check(p, TOK_STAR) || check(p, TOK_SLASH)) {
        Token op = advance(p);
        AstNode *node = ast_new(AST_BINOP, op.line, op.col);
        node->op = op.kind;
        node->left = left;
        node->right = parse_unary(p);
        left = node;
    }
    return left;
}

static AstNode *parse_additive(Parser *p)
{
    AstNode *left = parse_multiplicative(p);
    while (check(p, TOK_PLUS) || check(p, TOK_MINUS)) {
        Token op = advance(p);
        AstNode *node = ast_new(AST_BINOP, op.line, op.col);
        node->op = op.kind;
        node->left = left;
        node->right = parse_multiplicative(p);
        left = node;
    }
    return left;
}

static bool is_comparison(TokenKind k)
{
    return k == TOK_EQEQ || k == TOK_NE || k == TOK_LT || k == TOK_GT || k == TOK_LE || k == TOK_GE;
}

static AstNode *parse_comparison(Parser *p)
{
    AstNode *left = parse_additive(p);
    while (is_comparison(peek(p).kind)) {
        Token op = advance(p);
        AstNode *node = ast_new(AST_BINOP, op.line, op.col);
        node->op = op.kind;
        node->left = left;
        node->right = parse_additive(p);
        left = node;
    }
    return left;
}

static AstNode *parse_logical_and(Parser *p)
{
    AstNode *left = parse_comparison(p);
    while (match_tok(p, TOK_ANDAND)) {
        Token op = previous(p);
        AstNode *node = ast_new(AST_BINOP, op.line, op.col);
        node->op = TOK_ANDAND;
        node->left = left;
        node->right = parse_comparison(p);
        left = node;
    }
    return left;
}

static AstNode *parse_logical_or(Parser *p)
{
    AstNode *left = parse_logical_and(p);
    while (match_tok(p, TOK_OROR)) {
        Token op = previous(p);
        AstNode *node = ast_new(AST_BINOP, op.line, op.col);
        node->op = TOK_OROR;
        node->left = left;
        node->right = parse_logical_and(p);
        left = node;
    }
    return left;
}

static AstNode *parse_expr(Parser *p)
{
    return parse_logical_or(p);
}

static AstNode *parse_block(Parser *p, Token lbrace)
{
    AstNode *block = ast_new(AST_BLOCK, lbrace.line, lbrace.col);
    while (!check(p, TOK_RBRACE) && !is_at_end(p)) {
        ast_add_child(block, parse_stmt(p));
        if (p->had_error) {
            break;
        }
    }
    consume(p, TOK_RBRACE, "expected '}' after block");
    return block;
}

static AstNode *parse_decl(Parser *p, Token type_tok)
{
    Token name;
    AstNode *node;

    name = consume(p, TOK_IDENT, "expected variable name after type specifier");
    node = ast_new(AST_VAR_DECL, type_tok.line, type_tok.col);
    node->name = xstrdup(name.lexeme[0] == '\0' ? "<invalid>" : name.lexeme);

    if (match_tok(p, TOK_EQ)) {
        node->left = parse_expr(p);
    }
    consume(p, TOK_SEMI, "expected ';' after declaration");
    return node;
}

static AstNode *parse_assign(Parser *p)
{
    Token name = advance(p);
    AstNode *node = ast_new(AST_ASSIGN, name.line, name.col);
    node->name = xstrdup(name.lexeme);
    consume(p, TOK_EQ, "expected '=' in assignment");
    node->left = parse_expr(p);
    consume(p, TOK_SEMI, "expected ';' after assignment");
    return node;
}

static AstNode *parse_print(Parser *p, Token print_tok)
{
    AstNode *node = ast_new(AST_PRINT, print_tok.line, print_tok.col);
    consume(p, TOK_LPAREN, "expected '(' after 'print'");
    node->left = parse_expr(p);
    consume(p, TOK_RPAREN, "expected ')' after print argument");
    consume(p, TOK_SEMI, "expected ';' after print statement");
    return node;
}

static AstNode *parse_printf(Parser *p, Token printf_tok)
{
    AstNode *node = ast_new(AST_PRINTF, printf_tok.line, printf_tok.col);
    consume(p, TOK_LPAREN, "expected '(' after 'printf'");

    /* First argument is string literal */
    node->left = parse_expr(p);

    while (match_tok(p, TOK_COMMA)) {
        ast_add_child(node, parse_expr(p));
    }

    consume(p, TOK_RPAREN, "expected ')' after printf arguments");
    consume(p, TOK_SEMI, "expected ';' after printf statement");
    return node;
}

static AstNode *parse_scanf(Parser *p, Token scanf_tok)
{
    AstNode *node = ast_new(AST_SCANF, scanf_tok.line, scanf_tok.col);
    consume(p, TOK_LPAREN, "expected '(' after 'scanf'");

    /* First argument is format string */
    node->left = parse_expr(p);

    while (match_tok(p, TOK_COMMA)) {
        ast_add_child(node, parse_expr(p));
    }

    consume(p, TOK_RPAREN, "expected ')' after scanf arguments");
    consume(p, TOK_SEMI, "expected ';' after scanf statement");
    return node;
}

static AstNode *parse_if(Parser *p, Token if_tok)
{
    AstNode *node = ast_new(AST_IF, if_tok.line, if_tok.col);
    consume(p, TOK_LPAREN, "expected '(' after 'if'");
    node->left = parse_expr(p);
    consume(p, TOK_RPAREN, "expected ')' after if condition");
    node->right = parse_stmt(p);
    if (match_tok(p, TOK_ELSE)) {
        node->extra = parse_stmt(p);
    }
    return node;
}

static AstNode *parse_while(Parser *p, Token while_tok)
{
    AstNode *node = ast_new(AST_WHILE, while_tok.line, while_tok.col);
    consume(p, TOK_LPAREN, "expected '(' after 'while'");
    node->left = parse_expr(p);
    consume(p, TOK_RPAREN, "expected ')' after while condition");
    node->right = parse_stmt(p);
    return node;
}

static AstNode *parse_for(Parser *p, Token for_tok)
{
    AstNode *node = ast_new(AST_FOR, for_tok.line, for_tok.col);
    consume(p, TOK_LPAREN, "expected '(' after 'for'");

    /* Init statement / declaration */
    if (!check(p, TOK_SEMI)) {
        node->left = parse_stmt(p);
    } else {
        consume(p, TOK_SEMI, "expected ';'");
    }

    /* Condition expression */
    if (!check(p, TOK_SEMI)) {
        node->right = parse_expr(p);
    }
    consume(p, TOK_SEMI, "expected ';' after for condition");

    /* Increment expression */
    if (!check(p, TOK_RPAREN)) {
        node->extra = parse_expr(p);
    }
    consume(p, TOK_RPAREN, "expected ')' after for header");

    /* Body statement */
    ast_add_child(node, parse_stmt(p));
    return node;
}

static AstNode *parse_return(Parser *p, Token ret_tok)
{
    AstNode *node = ast_new(AST_RETURN, ret_tok.line, ret_tok.col);
    if (!check(p, TOK_SEMI)) {
        node->left = parse_expr(p);
    }
    consume(p, TOK_SEMI, "expected ';' after return statement");
    return node;
}

static AstNode *parse_stmt(Parser *p)
{
    if (match_tok(p, TOK_SEMI)) {
        return ast_new(AST_BLOCK, previous(p).line, previous(p).col);
    }
    if (match_tok(p, TOK_INT_KW) || match_tok(p, TOK_CHAR_KW) || match_tok(p, TOK_VOID)) {
        Token type_tok = previous(p);
        
        /* Check if function declaration: int main() { ... } */
        if (check(p, TOK_MAIN) || (check(p, TOK_IDENT) && (p->tokens->items[p->current + 1].kind == TOK_LPAREN))) {
            Token fname = advance(p);
            (void)fname;
            consume(p, TOK_LPAREN, "expected '(' after function name");
            /* Optional parameters void or (int argc, char** argv) */
            while (!check(p, TOK_RPAREN) && !is_at_end(p)) {
                advance(p);
            }
            consume(p, TOK_RPAREN, "expected ')' after function parameters");
            Token lbrace = consume(p, TOK_LBRACE, "expected '{' to start function body");
            return parse_block(p, lbrace);
        }
        
        return parse_decl(p, type_tok);
    }
    if (match_tok(p, TOK_PRINT)) {
        return parse_print(p, previous(p));
    }
    if (match_tok(p, TOK_PRINTF)) {
        return parse_printf(p, previous(p));
    }
    if (match_tok(p, TOK_SCANF)) {
        return parse_scanf(p, previous(p));
    }
    if (match_tok(p, TOK_IF)) {
        return parse_if(p, previous(p));
    }
    if (match_tok(p, TOK_WHILE)) {
        return parse_while(p, previous(p));
    }
    if (match_tok(p, TOK_FOR)) {
        return parse_for(p, previous(p));
    }
    if (match_tok(p, TOK_RETURN)) {
        return parse_return(p, previous(p));
    }
    if (match_tok(p, TOK_LBRACE)) {
        return parse_block(p, previous(p));
    }
    if (check(p, TOK_IDENT) && p->tokens->items[p->current + 1].kind == TOK_EQ) {
        return parse_assign(p);
    }
    if (check(p, TOK_IDENT) || check(p, TOK_INC) || check(p, TOK_DEC)) {
        AstNode *expr = parse_expr(p);
        consume(p, TOK_SEMI, "expected ';' after expression statement");
        return expr;
    }

    parse_error(p, peek(p), "expected statement, found '%s'", token_kind_name(peek(p).kind));
    if (!is_at_end(p)) {
        advance(p);
    }
    return ast_new(AST_BLOCK, peek(p).line, peek(p).col);
}

AstNode *parse_program(Parser *parser)
{
    AstNode *program = ast_new(AST_PROGRAM, 1, 1);

    while (!is_at_end(parser) && !parser->had_error) {
        ast_add_child(program, parse_stmt(parser));
    }

    if (!parser->had_error && !is_at_end(parser)) {
        parse_error(parser, peek(parser), "unexpected tokens after program");
    }
    return program;
}
