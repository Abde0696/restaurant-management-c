#include "tables.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "utils.h"

void tables_init(TableList *t)
{
    t->tables = NULL;
    t->count = 0;
    t->capacity = 0;
}

void tables_free(TableList *t)
{
    free(t->tables);
    tables_init(t);
}

Table *tables_find(const TableList *t, int number)
{
    for (int i = 0; i < t->count; i++)
        if (t->tables[i].number == number)
            return &t->tables[i];
    return NULL;
}

int tables_add(TableList *t, int number, int seats)
{
    if (tables_find(t, number))
        return 0;
    if (t->count == t->capacity) {
        int new_cap = t->capacity ? t->capacity * 2 : 8;
        Table *p = realloc(t->tables, (size_t)new_cap * sizeof *p);
        if (!p)
            return 0;
        t->tables = p;
        t->capacity = new_cap;
    }
    t->tables[t->count].number = number;
    t->tables[t->count].seats = seats;
    t->tables[t->count].status = TABLE_FREE;
    t->count++;
    return 1;
}

int tables_remove(TableList *t, int number)
{
    for (int i = 0; i < t->count; i++) {
        if (t->tables[i].number == number) {
            memmove(&t->tables[i], &t->tables[i + 1], (size_t)(t->count - i - 1) * sizeof *t->tables);
            t->count--;
            return 1;
        }
    }
    return 0;
}

int tables_save(const TableList *t, const char *path)
{
    FILE *f = fopen(path, "w");
    if (!f)
        return 0;
    for (int i = 0; i < t->count; i++)
        fprintf(f, "%d|%d|%d\n", t->tables[i].number, t->tables[i].seats, (int)t->tables[i].status);
    fclose(f);
    return 1;
}

int tables_load(TableList *t, const char *path)
{
    FILE *f = fopen(path, "r");
    if (!f)
        return 0;
    char line[MAX_LINE];
    while (fgets(line, sizeof line, f)) {
        strip_newline(line);
        char *fields[3];
        if (split_fields(line, '|', fields, 3) != 3)
            continue;
        int number = atoi(fields[0]), seats = atoi(fields[1]), status = atoi(fields[2]);
        if (number > 0 && seats > 0 && tables_add(t, number, seats))
            tables_find(t, number)->status = status == TABLE_OCCUPIED ? TABLE_OCCUPIED : TABLE_FREE;
    }
    fclose(f);
    return 1;
}
