#!/bin/sh
set -eu

ROOT="$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)"
BIN="$ROOT/mini-c"
PASS=0
FAIL=0

assert_ok() {
    name="$1"
    file="$2"
    expected="$3"
    out="$(mktemp)"
    err="$(mktemp)"
    if "$BIN" "$file" >"$out" 2>"$err"; then
        got="$(cat "$out")"
        if [ "$got" = "$expected" ]; then
            echo "PASS  $name"
            PASS=$((PASS + 1))
        else
            echo "FAIL  $name (output mismatch)"
            echo "  expected:"
            printf '%s\n' "$expected" | sed 's/^/    /'
            echo "  got:"
            printf '%s\n' "$got" | sed 's/^/    /'
            FAIL=$((FAIL + 1))
        fi
    else
        echo "FAIL  $name (compiler/runtime exited non-zero)"
        cat "$err" | sed 's/^/    /'
        FAIL=$((FAIL + 1))
    fi
    rm -f "$out" "$err"
}

assert_fail() {
    name="$1"
    file="$2"
    needle="$3"
    extra="${4:-}"
    out="$(mktemp)"
    err="$(mktemp)"
    set +e
    "$BIN" $extra "$file" >"$out" 2>"$err"
    code=$?
    set -e
    if [ "$code" -ne 0 ] && grep -q "$needle" "$err"; then
        echo "PASS  $name"
        PASS=$((PASS + 1))
    else
        echo "FAIL  $name (expected non-zero exit and error containing: $needle)"
        echo "  exit=$code"
        echo "  stderr:"
        cat "$err" | sed 's/^/    /'
        FAIL=$((FAIL + 1))
    fi
    rm -f "$out" "$err"
}

# Original backward compatibility tests
assert_ok "declarations/assignment/print" "$ROOT/examples/add.mc" "30"
assert_ok "while loop" "$ROOT/examples/while.mc" "1
2
3
4
5"
assert_ok "if/else" "$ROOT/examples/ifelse.mc" "1"
assert_ok "precedence and parentheses" "$ROOT/examples/precedence.mc" "14
20"
assert_ok "nested if/while" "$ROOT/examples/nested.mc" "1
20
3"

assert_ok "arithmetic ops" "$ROOT/tests/arithmetic.mc" "15
5
50
2"
assert_ok "comparisons" "$ROOT/tests/comparisons.mc" "1
0
1
0
1
1"
assert_ok "if without else" "$ROOT/tests/if_only.mc" "7"
assert_ok "block scope" "$ROOT/tests/block_scope.mc" "1
2"
assert_ok "unary minus" "$ROOT/tests/unary.mc" "-6"

# Extended C tests (#include, main, printf, for, strings, logical ops)
assert_ok "hello world (#include <stdio.h> + main + printf)" "$ROOT/tests/hello_world.mc" "Hello World"
assert_ok "printf with integers" "$ROOT/tests/printf_int.mc" "42"
assert_ok "printf with expressions" "$ROOT/tests/printf_expr.mc" "Sum = 30"
assert_ok "for loop" "$ROOT/tests/for_loop.mc" "1
2
3"
assert_ok "logical operators (&&, ||, !)" "$ROOT/tests/logical_ops.mc" "1"
assert_ok "printf with strings (%s)" "$ROOT/tests/strings.mc" "Hello world"

# Error Handling tests
assert_fail "invalid #include <math.h>" "$ROOT/tests/invalid_include.mc" "unsupported library"
assert_fail "unsupported C library <stdlib.h>" "$ROOT/tests/unsupported_lib.mc" "unsupported library"
assert_fail "undeclared variable" "$ROOT/tests/undeclared.mc" "undeclared"
assert_fail "duplicate declaration" "$ROOT/tests/duplicate.mc" "duplicate"
assert_fail "syntax error" "$ROOT/tests/syntax_error.mc" "syntax error"
assert_fail "invalid token" "$ROOT/tests/invalid_token.mc" "lexical error"
assert_fail "division by zero" "$ROOT/tests/divzero.mc" "division by zero"
assert_fail "assignment to undeclared" "$ROOT/tests/assign_undeclared.mc" "undeclared"
assert_fail "out of scope" "$ROOT/tests/scope_error.mc" "undeclared"

echo
echo "Passed: $PASS"
echo "Failed: $FAIL"
if [ "$FAIL" -ne 0 ]; then
    exit 1
fi
exit 0
