#ifndef TABLES_H
#define TABLES_H

typedef enum { TABLE_FREE = 0, TABLE_OCCUPIED = 1 } TableStatus;

typedef struct {
    int         number;
    int         seats;
    TableStatus status;
} Table;

typedef struct {
    Table *tables;
    int    count;
    int    capacity;
} TableList;

void   tables_init(TableList *t);
void   tables_free(TableList *t);
int    tables_add(TableList *t, int number, int seats);  /* 1 si ajoutée, 0 si numéro déjà pris */
Table *tables_find(const TableList *t, int number);
int    tables_remove(TableList *t, int number);          /* 1 si supprimée */
int    tables_save(const TableList *t, const char *path);
int    tables_load(TableList *t, const char *path);

#endif
