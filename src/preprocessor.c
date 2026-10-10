#include "preprocessor.h"

typedef struct {
    char *name;
    char *value;
} Macro;

typedef struct {
    Macro *items;
    int count;
    int capacity;
} MacroTable;

static void macro_add(MacroTable *table, const char *name, const char *value)
{
    int i;
    for (i = 0; i < table->count; i++) {
        if (strcmp(table->items[i].name, name) == 0) {
            free(table->items[i].value);
            table->items[i].value = xstrdup(value);
            return;
        }
    }
    if (table->count >= table->capacity) {
        int cap = table->capacity == 0 ? 16 : table->capacity * 2;
        table->items = xrealloc(table->items, (size_t)cap * sizeof(Macro));
        table->capacity = cap;
    }
    table->items[table->count].name = xstrdup(name);
    table->items[table->count].value = xstrdup(value);
    table->count++;
}

static void macro_table_free(MacroTable *table)
{
    int i;
    for (i = 0; i < table->count; i++) {
        free(table->items[i].name);
        free(table->items[i].value);
    }
    free(table->items);
}

static char *get_dir(const char *filepath)
{
    const char *last_slash = strrchr(filepath, '/');
    if (last_slash == NULL) {
        return xstrdup(".");
    }
    size_t len = (size_t)(last_slash - filepath);
    char *dir = xmalloc(len + 1);
    memcpy(dir, filepath, len);
    dir[len] = '\0';
    return dir;
}

static char *read_file_text(const char *path)
{
    FILE *fp = fopen(path, "rb");
    long size;
    char *buf;
    if (fp == NULL) {
        return NULL;
    }
    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    if (size < 0) {
        fclose(fp);
        return NULL;
    }
    buf = xmalloc((size_t)size + 1);
    if (size > 0 && fread(buf, 1, (size_t)size, fp) != (size_t)size) {
        free(buf);
        fclose(fp);
        return NULL;
    }
    buf[size] = '\0';
    fclose(fp);
    return buf;
}

char *preprocess_source(const char *source, const char *filename, bool *had_error)
{
    size_t out_cap = strlen(source) * 2 + 1024;
    size_t out_len = 0;
    char *out = xmalloc(out_cap);
    MacroTable macros = {NULL, 0, 0};
    const char *p = source;
    int line = 1;
    char *dir = get_dir(filename);

    *had_error = false;
    out[0] = '\0';

    while (*p != '\0') {
        const char *line_start = p;
        while (*p != '\0' && *p != '\n') {
            p++;
        }
        size_t line_len = (size_t)(p - line_start);
        char *line_buf = xmalloc(line_len + 1);
        memcpy(line_buf, line_start, line_len);
        line_buf[line_len] = '\0';

        const char *trim = line_buf;
        while (*trim == ' ' || *trim == '\t') {
            trim++;
        }

        if (*trim == '#') {
            const char *directive = trim + 1;
            while (*directive == ' ' || *directive == '\t') {
                directive++;
            }

            if (strncmp(directive, "include", 7) == 0) {
                const char *inc = directive + 7;
                while (*inc == ' ' || *inc == '\t') {
                    inc++;
                }
                if (*inc == '<') {
                    const char *end_header = strchr(inc + 1, '>');
                    if (end_header != NULL) {
                        size_t hlen = (size_t)(end_header - (inc + 1));
                        char header[128];
                        if (hlen < sizeof(header)) {
                            memcpy(header, inc + 1, hlen);
                            header[hlen] = '\0';

                            if (strcmp(header, "stdio.h") == 0) {
                                /* Standard I/O included - replace line with space preserving newline */
                                size_t req = out_len + 64;
                                if (req >= out_cap) {
                                    out_cap *= 2;
                                    out = xrealloc(out, out_cap);
                                }
                                out_len += snprintf(out + out_len, out_cap - out_len, "// #include <stdio.h>\n");
                            } else {
                                fprintf(stderr, "%s:%d:1: preprocessor error: unsupported library <%s>\n",
                                        filename, line, header);
                                *had_error = true;
                            }
                        }
                    } else {
                        fprintf(stderr, "%s:%d:1: preprocessor error: malformed #include directive\n", filename, line);
                        *had_error = true;
                    }
                } else if (*inc == '"') {
                    const char *end_header = strchr(inc + 1, '"');
                    if (end_header != NULL) {
                        size_t hlen = (size_t)(end_header - (inc + 1));
                        char rel_path[256];
                        if (hlen < sizeof(rel_path)) {
                            memcpy(rel_path, inc + 1, hlen);
                            rel_path[hlen] = '\0';

                            char full_path[512];
                            snprintf(full_path, sizeof(full_path), "%s/%s", dir, rel_path);

                            char *inc_text = read_file_text(full_path);
                            if (inc_text == NULL) {
                                /* Try current working dir */
                                inc_text = read_file_text(rel_path);
                            }

                            if (inc_text != NULL) {
                                bool sub_err = false;
                                char *sub_pre = preprocess_source(inc_text, full_path, &sub_err);
                                free(inc_text);
                                if (sub_err) {
                                    *had_error = true;
                                } else {
                                    size_t req = out_len + strlen(sub_pre) + 64;
                                    if (req >= out_cap) {
                                        out_cap = req * 2;
                                        out = xrealloc(out, out_cap);
                                    }
                                    out_len += snprintf(out + out_len, out_cap - out_len, "%s\n", sub_pre);
                                }
                                free(sub_pre);
                            } else {
                                fprintf(stderr, "%s:%d:1: preprocessor error: cannot open include file '%s'\n",
                                        filename, line, rel_path);
                                *had_error = true;
                            }
                        }
                    }
                }
            } else if (strncmp(directive, "define", 6) == 0) {
                const char *def = directive + 6;
                while (*def == ' ' || *def == '\t') {
                    def++;
                }
                char mname[128];
                char mval[128];
                int read_count = sscanf(def, "%127s %127s", mname, mval);
                if (read_count >= 1) {
                    macro_add(&macros, mname, read_count == 2 ? mval : "1");
                }
                size_t req = out_len + 64;
                if (req >= out_cap) {
                    out_cap *= 2;
                    out = xrealloc(out, out_cap);
                }
                out_len += snprintf(out + out_len, out_cap - out_len, "// #define %s\n", mname);
            } else {
                /* Preserve line as newline */
                size_t req = out_len + line_len + 2;
                if (req >= out_cap) {
                    out_cap = req * 2;
                    out = xrealloc(out, out_cap);
                }
                out_len += snprintf(out + out_len, out_cap - out_len, "%s\n", line_buf);
            }
        } else {
            /* Replace macros in normal line if any match */
            size_t req = out_len + line_len + 2;
            if (req >= out_cap) {
                out_cap = req * 2;
                out = xrealloc(out, out_cap);
            }
            out_len += snprintf(out + out_len, out_cap - out_len, "%s\n", line_buf);
        }

        free(line_buf);
        line++;
        if (*p == '\n') {
            p++;
        }
    }

    free(dir);
    macro_table_free(&macros);
    return out;
}
