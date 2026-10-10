#include "preprocessor.h"
#include "lexer.h"
#include "parser.h"
#include "semantic.h"
#include "ir.h"
#include "vm.h"

typedef enum {
    MODE_RUN,
    MODE_TOKENS,
    MODE_AST,
    MODE_IR,
    MODE_TRACE
} RunMode;

static void usage(const char *argv0)
{
    fprintf(stderr, "Usage: %s [--tokens|--ast|--ir|--trace] <file.mc>\n", argv0);
}

static char *read_file(const char *path)
{
    FILE *fp = fopen(path, "rb");
    long size;
    char *buf;

    if (fp == NULL) {
        fprintf(stderr, "error: cannot open '%s': %s\n", path, strerror(errno));
        return NULL;
    }
    if (fseek(fp, 0, SEEK_END) != 0) {
        fprintf(stderr, "error: cannot seek '%s'\n", path);
        fclose(fp);
        return NULL;
    }
    size = ftell(fp);
    if (size < 0) {
        fprintf(stderr, "error: cannot determine size of '%s'\n", path);
        fclose(fp);
        return NULL;
    }
    if (fseek(fp, 0, SEEK_SET) != 0) {
        fprintf(stderr, "error: cannot rewind '%s'\n", path);
        fclose(fp);
        return NULL;
    }
    buf = xmalloc((size_t)size + 1);
    if (size > 0 && fread(buf, 1, (size_t)size, fp) != (size_t)size) {
        fprintf(stderr, "error: failed to read '%s'\n", path);
        free(buf);
        fclose(fp);
        return NULL;
    }
    buf[size] = '\0';
    fclose(fp);
    return buf;
}

int main(int argc, char **argv)
{
    RunMode mode = MODE_RUN;
    const char *path = NULL;
    int i;
    char *raw_source;
    char *source;
    bool pre_error = false;
    TokenList tokens;
    Parser parser;
    AstNode *ast;
    SymbolTable table;
    IrProgram *ir;
    int status = 0;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--tokens") == 0) {
            mode = MODE_TOKENS;
        } else if (strcmp(argv[i], "--ast") == 0) {
            mode = MODE_AST;
        } else if (strcmp(argv[i], "--ir") == 0) {
            mode = MODE_IR;
        } else if (strcmp(argv[i], "--trace") == 0) {
            mode = MODE_TRACE;
        } else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
            usage(argv[0]);
            return 0;
        } else if (argv[i][0] == '-') {
            fprintf(stderr, "error: unknown option '%s'\n", argv[i]);
            usage(argv[0]);
            return 1;
        } else if (path == NULL) {
            path = argv[i];
        } else {
            fprintf(stderr, "error: extra argument '%s'\n", argv[i]);
            usage(argv[0]);
            return 1;
        }
    }

    if (path == NULL) {
        usage(argv[0]);
        return 1;
    }

    raw_source = read_file(path);
    if (raw_source == NULL) {
        return 1;
    }

    source = preprocess_source(raw_source, path, &pre_error);
    free(raw_source);
    if (pre_error || source == NULL) {
        free(source);
        return 1;
    }

    tokens = lex_source(source, path);
    if (mode == MODE_TOKENS) {
        print_tokens(&tokens);
        status = tokens.had_error ? 1 : 0;
        token_list_free(&tokens);
        free(source);
        return status;
    }
    if (tokens.had_error) {
        token_list_free(&tokens);
        free(source);
        return 1;
    }

    parser_init(&parser, &tokens, path);
    ast = parse_program(&parser);
    if (parser.had_error) {
        ast_free(ast);
        token_list_free(&tokens);
        free(source);
        return 1;
    }

    if (mode == MODE_AST) {
        ast_print(ast, 0);
        ast_free(ast);
        token_list_free(&tokens);
        free(source);
        return 0;
    }

    symtab_init(&table);
    if (!semantic_analyze(ast, &table, path)) {
        ast_free(ast);
        token_list_free(&tokens);
        symtab_free(&table);
        free(source);
        return 1;
    }

    ir = ir_generate(ast, &table);
    if (mode == MODE_IR) {
        ir_print(ir);
        ir_program_free(ir);
        ast_free(ast);
        token_list_free(&tokens);
        symtab_free(&table);
        free(source);
        return 0;
    }

    status = vm_run(ir, mode == MODE_TRACE);

    ir_program_free(ir);
    ast_free(ast);
    token_list_free(&tokens);
    symtab_free(&table);
    free(source);
    return status;
}
