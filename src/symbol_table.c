#include "symbol_table.h"

void symtab_init(SymbolTable *table)
{
    table->scopes = NULL;
    table->depth = 0;
    table->capacity = 0;
    table->next_slot = 0;
    table->slot_names = NULL;
    table->slot_names_cap = 0;
    symtab_enter_scope(table);
}

void symtab_free(SymbolTable *table)
{
    int i;
    int j;
    for (i = 0; i < table->depth; i++) {
        for (j = 0; j < table->scopes[i].count; j++) {
            free(table->scopes[i].items[j].name);
        }
        free(table->scopes[i].items);
    }
    free(table->scopes);
    for (i = 0; i < table->next_slot; i++) {
        free(table->slot_names[i]);
    }
    free(table->slot_names);
    table->scopes = NULL;
    table->depth = 0;
    table->capacity = 0;
    table->next_slot = 0;
    table->slot_names = NULL;
    table->slot_names_cap = 0;
}

void symtab_enter_scope(SymbolTable *table)
{
    Scope *scope;
    if (table->depth >= table->capacity) {
        int cap = table->capacity == 0 ? 8 : table->capacity * 2;
        table->scopes = xrealloc(table->scopes, (size_t)cap * sizeof(Scope));
        table->capacity = cap;
    }
    scope = &table->scopes[table->depth++];
    scope->items = NULL;
    scope->count = 0;
    scope->capacity = 0;
}

void symtab_leave_scope(SymbolTable *table)
{
    Scope *scope;
    int i;
    if (table->depth <= 0) {
        return;
    }
    scope = &table->scopes[--table->depth];
    for (i = 0; i < scope->count; i++) {
        free(scope->items[i].name);
    }
    free(scope->items);
}

static bool scope_contains(const Scope *scope, const char *name)
{
    int i;
    for (i = 0; i < scope->count; i++) {
        if (strcmp(scope->items[i].name, name) == 0) {
            return true;
        }
    }
    return false;
}

bool symtab_declare(SymbolTable *table, const char *name, int line, int col, int *slot_out)
{
    Scope *scope;
    Symbol *sym;

    if (table->depth <= 0) {
        return false;
    }
    scope = &table->scopes[table->depth - 1];
    if (scope_contains(scope, name)) {
        return false;
    }
    if (scope->count >= scope->capacity) {
        int cap = scope->capacity == 0 ? 8 : scope->capacity * 2;
        scope->items = xrealloc(scope->items, (size_t)cap * sizeof(Symbol));
        scope->capacity = cap;
    }
    sym = &scope->items[scope->count++];
    sym->name = xstrdup(name);
    sym->slot = table->next_slot;
    if (table->next_slot >= table->slot_names_cap) {
        int cap = table->slot_names_cap == 0 ? 16 : table->slot_names_cap * 2;
        table->slot_names = xrealloc(table->slot_names, (size_t)cap * sizeof(char *));
        table->slot_names_cap = cap;
    }
    table->slot_names[table->next_slot] = xstrdup(name);
    table->next_slot++;
    sym->line = line;
    sym->col = col;
    sym->depth = table->depth - 1;
    if (slot_out != NULL) {
        *slot_out = sym->slot;
    }
    return true;
}

bool symtab_lookup(SymbolTable *table, const char *name, int *slot_out)
{
    int i;
    int j;
    for (i = table->depth - 1; i >= 0; i--) {
        Scope *scope = &table->scopes[i];
        for (j = 0; j < scope->count; j++) {
            if (strcmp(scope->items[j].name, name) == 0) {
                if (slot_out != NULL) {
                    *slot_out = scope->items[j].slot;
                }
                return true;
            }
        }
    }
    return false;
}

int symtab_slot_count(const SymbolTable *table)
{
    return table->next_slot;
}

const char *symtab_slot_name(const SymbolTable *table, int slot)
{
    if (slot < 0 || slot >= table->next_slot) {
        return "?";
    }
    return table->slot_names[slot];
}
