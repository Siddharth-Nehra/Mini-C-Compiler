CC ?= clang
CFLAGS ?= -Wall -Wextra -pedantic -std=c11
CPPFLAGS += -Isrc
LDFLAGS ?=

SRCS = src/main.c src/preprocessor.c src/lexer.c src/parser.c src/ast.c src/semantic.c src/symbol_table.c src/ir.c src/vm.c
OBJS = $(SRCS:.c=.o)
TARGET = mini-c

.PHONY: all test clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(LDFLAGS)

src/%.o: src/%.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

test: $(TARGET)
	sh tests/run_tests.sh

clean:
	rm -f $(OBJS) $(TARGET)
