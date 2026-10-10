# Mini-C Compiler

A small C-like language compiled by a hand-written C11 compiler and executed on a custom virtual machine. Clang/GCC are used only to build this compiler, never to run Mini-C programs.

## Build

```bash
make
```

Requires Clang or GCC with C11 support. The build uses `-Wall -Wextra -pedantic -std=c11` and should produce zero warnings.

```bash
make test
make clean
```

## Run

```bash
./mini-c examples/add.mc
./mini-c --tokens examples/add.mc
./mini-c --ast examples/add.mc
./mini-c --ir examples/add.mc
./mini-c --trace examples/add.mc
```

## Example

Input (`examples/add.mc`):

```c
int a = 10;
int b = 20;
int c;

c = a + b;

print(c);
```

Output:

```
30
```

Loop example (`examples/while.mc`):

```c
int a = 1;

while (a <= 5) {
    print(a);
    a = a + 1;
}
```

Output:

```
1
2
3
4
5
```

## Pipeline

Mini-C source → lexer → parser (AST) → semantic analysis + symbol table → three-address code → virtual machine → output

See [LANGUAGE.md](LANGUAGE.md) for the language and [ARCHITECTURE.md](ARCHITECTURE.md) for the compiler internals.

## Features

- `int` declarations, optional initializers, assignment
- Arithmetic `+ - * /` and parentheses
- Comparisons `== != < > <= >=`
- `if`, `if/else`, `while`, nested blocks
- `print(expr);`
- `//` line comments
- Scoped declarations
- Compile-time checks for undeclared, duplicate, and out-of-scope names
- Runtime division-by-zero detection
- CLI dumps for tokens, AST, IR, and VM traces

## Web IDE

The browser IDE talks to a small Node API that shells out to `./mini-c`. It does not fake compile results.

```bash
make
cd server && npm install && npm start
# in another terminal
cd web && npm install && npm run dev
```

Open [http://localhost:5173](http://localhost:5173). Register, create a program, then **Run**. Output, tokens, AST, TAC, and VM trace come from the real compiler.

API base URL is configured in `web/src/api/config.js` (`VITE_API_BASE_URL` or `/api` via the Vite proxy).
