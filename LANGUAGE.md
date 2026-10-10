# Mini-C Language

Mini-C is a tiny imperative language with one type (`int`, stored as a 64-bit signed integer) and no functions.

## Lexical structure

- Identifiers: `[A-Za-z_][A-Za-z0-9_]*`
- Integer literals: decimal digits only
- Keywords: `int`, `print`, `if`, `else`, `while`
- Operators: `+ - * / = == != < > <= >=`
- Delimiters: `( ) { } ;`
- Comments: `//` to end of line
- Whitespace is ignored

## Grammar (simplified)

```
program     ::= stmt*
stmt        ::= decl | assign | print-stmt | if-stmt | while-stmt | block
decl        ::= "int" IDENT ("=" expr)? ";"
assign      ::= IDENT "=" expr ";"
print-stmt  ::= "print" "(" expr ")" ";"
if-stmt     ::= "if" "(" expr ")" stmt ("else" stmt)?
while-stmt  ::= "while" "(" expr ")" stmt
block       ::= "{" stmt* "}"
expr        ::= comparison
comparison  ::= additive (("=="|"!="|"<"|">"|"<="|">=") additive)*
additive    ::= multiplicative (("+"|"-") multiplicative)*
multiplicative ::= unary (("*"|"/") unary)*
unary       ::= "-" unary | primary
primary     ::= NUMBER | IDENT | "(" expr ")"
```

Operator precedence (highest first): `()` and unary `-`, then `*` `/`, then `+` `-`, then comparisons. `else` binds to the nearest unmatched `if`.

## Semantics

- A program is a sequence of statements in a global scope.
- `{ ... }` opens a nested scope. Names declared inside a block are not visible after it.
- Redeclaring a name in the same scope is an error. Shadowing an outer name in an inner block is allowed.
- Using or assigning a name that is not in scope is an error.
- Uninitialized variables start at `0`.
- Comparisons yield `1` (true) or `0` (false). Any non-zero value is true in `if`/`while`.
- `/` is integer division toward zero. Division by zero is a runtime error.
- `print` writes one integer per line.

## What is not supported

- Types other than `int` (no `char`, pointers, arrays, structs)
- Functions, `return`, `for`, `break`, `continue`
- Boolean operators `&&` `||` `!` as distinct operators (`!=` is comparison only)
- Strings, I/O besides `print`
- Floating-point numbers
- Preprocessor / `#include`
