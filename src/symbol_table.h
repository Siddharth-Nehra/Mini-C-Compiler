#ifndef MINI_C_SYMBOL_TABLE_H
#define MINI_C_SYMBOL_TABLE_H

#include "common.h"

typedef struct {
    char *name;
    int slot;
    int line;
    int col;
    int depth;
} Symbol;

typedef struct {
    Symbol *items;
    int count;
    int capacity;
} Scope;

typedef struct {
    Scope *scopes;
    int depth;
    int capacity;
    int next_slot;
    char **slot_names;
    int slot_names_cap;
} SymbolTable;

void symtab_init(SymbolTable *table);
void symtab_free(SymbolTable *table);
void symtab_enter_scope(SymbolTable *table);
void symtab_leave_scope(SymbolTable *table);
bool symtab_declare(SymbolTable *table, const char *name, int line, int col, int *slot_out);
bool symtab_lookup(SymbolTable *table, const char *name, int *slot_out);
int symtab_slot_count(const SymbolTable *table);
const char *symtab_slot_name(const SymbolTable *table, int slot);

#endif
