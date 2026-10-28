#ifndef MENU_H
#define MENU_H

#include "utils.h"

typedef struct {
    int   id;
    char  name[MAX_NAME];
    char  category[MAX_CAT];
    float price;
} MenuItem;

/* Tableau dynamique de plats (agrandi avec realloc) */
typedef struct {
    MenuItem *items;
    int       count;
    int       capacity;
    int       next_id;
} Menu;

void      menu_init(Menu *m);
void      menu_free(Menu *m);
int       menu_add(Menu *m, const char *name, const char *category, float price); /* id ou 0 si erreur */
MenuItem *menu_find(const Menu *m, int id);
int       menu_remove(Menu *m, int id);                                           /* 1 si supprimé */
void      menu_print(const Menu *m);
int       menu_save(const Menu *m, const char *path);
int       menu_load(Menu *m, const char *path);

#endif
