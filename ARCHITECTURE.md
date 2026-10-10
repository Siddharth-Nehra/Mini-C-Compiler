# Architecture

The compiler is a classic multi-pass pipeline implemented in C11 with no parser generators and no external compiler libraries.

```
source.mc
    │
    ▼
 lexer.c        tokens (keywords, idents, numbers, operators)
    │
    ▼
 parser.c       recursive-descent parse → AST (ast.c)
    │
    ▼
 semantic.c     scopes + checks via symbol_table.c
    │
    ▼
 ir.c           three-address code / VM opcodes
    │
    ▼
 vm.c           stackless register (slot) machine
    │
    ▼
 printed integers / trace
```

`main.c` reads the file, drives the passes, and implements `--tokens`, `--ast`, `--ir`, `--trace`.

## Lexer

`lex_source` scans the entire buffer and produces a `TokenList`. It tracks line and column, skips `//` comments, classifies keywords, and reports unexpected characters (for example `@`) as lexical errors. Multi-character operators `== != <= >=` are recognized before the single-character forms.

## Parser and AST

The parser is recursive descent and matches the precedence in LANGUAGE.md:

- `parse_comparison` → `parse_additive` → `parse_multiplicative` → `parse_unary` → `parse_primary`

Statements are parsed into AST nodes:

| Node        | Meaning                                      |
|-------------|----------------------------------------------|
| `AST_PROGRAM` / `AST_BLOCK` | list of statements                  |
| `AST_VAR_DECL` | `name`, optional initializer in `left`    |
| `AST_ASSIGN`   | `name`, value in `left`                   |
| `AST_PRINT`    | expression in `left`                      |
| `AST_IF`       | cond `left`, then `right`, else `extra`   |
| `AST_WHILE`    | cond `left`, body `right`                 |
| `AST_BINOP`    | operator in `op`, operands `left`/`right` |
| `AST_UNARY`    | unary minus                               |
| `AST_INT` / `AST_IDENT` | literals and names                 |

On syntax errors the parser reports `file:line:col` and stops.

## Symbol table and semantics

The symbol table is a stack of scopes. Each successful `int` declaration allocates a numeric **slot** (VM register) that is never reused, so inner and outer variables with the same name stay distinct.

Checks:

- duplicate names in the current scope
- undeclared names in expressions
- assignment to undeclared names
- use of a name after its declaring block has closed

Resolved slots are stored on the AST for the IR pass.

## IR (three-address code)

Expressions are lowered to temps `t1`, `t2`, …:

```
c = a + b * d;

t1 = b * d
t2 = a + t1
c = t2
```

Control flow uses explicit jumps:

- `if`: evaluate cond, `JUMP_IF_FALSE` over the then-branch; with `else`, a `JUMP` skips the else-branch
- `while`: jump back to the condition; `JUMP_IF_FALSE` exits the loop

Uninitialized `int x;` becomes `x = 0`. Unary minus is `t = 0 - x` because the VM has no `NEG` opcode.

The same instruction stream is both pretty-printed TAC and VM bytecode.

## Virtual machine

Each slot holds an `int64_t`. The program counter walks the instruction array.

| Opcode          | Effect                                      |
|-----------------|---------------------------------------------|
| `LOAD_CONST`    | dest ← immediate                            |
| `LOAD` / `STORE`| dest ← src (copy between slots)             |
| `ADD` `SUB` `MUL` `DIV` | dest ← a ⊕ b                      |
| `CMP_*`         | dest ← 0 or 1                               |
| `JUMP`          | pc ← target                                 |
| `JUMP_IF_FALSE` | if src == 0 then pc ← target                |
| `PRINT`         | write src as decimal line                   |
| `HALT`          | stop successfully                           |

`DIV` with a zero divisor reports a runtime error and exits non-zero. `--trace` prints each instruction to stderr as it runs.

## Why Mini-C is not compiled with GCC

`mini-c` is the compiler and the runtime. GCC/Clang only produce the `mini-c` binary from the C sources in `src/`. Mini-C programs never become native object files.
