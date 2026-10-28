#include "menu.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void menu_init(Menu *m)
{
    m->items = NULL;
    m->count = 0;
    m->capacity = 0;
    m->next_id = 1;
}

void menu_free(Menu *m)
{
    free(m->items);
    menu_init(m);
}

static int menu_reserve(Menu *m)
{
    if (m->count < m->capacity)
        return 1;
    int new_cap = m->capacity ? m->capacity * 2 : 8;
    MenuItem *p = realloc(m->items, (size_t)new_cap * sizeof *p);
    if (!p)
        return 0;
    m->items = p;
    m->capacity = new_cap;
    return 1;
}

int menu_add(Menu *m, const char *name, const char *category, float price)
{
    if (!menu_reserve(m))
        return 0;
    MenuItem *it = &m->items[m->count++];
    it->id = m->next_id++;
    snprintf(it->name, sizeof it->name, "%s", name);
    snprintf(it->category, sizeof it->category, "%s", category);
    it->price = price;
    return it->id;
}

MenuItem *menu_find(const Menu *m, int id)
{
    for (int i = 0; i < m->count; i++)
        if (m->items[i].id == id)
            return &m->items[i];
    return NULL;
}

int menu_remove(Menu *m, int id)
{
    for (int i = 0; i < m->count; i++) {
        if (m->items[i].id == id) {
            memmove(&m->items[i], &m->items[i + 1], (size_t)(m->count - i - 1) * sizeof *m->items);
            m->count--;
            return 1;
        }
    }
    return 0;
}

void menu_print(const Menu *m)
{
    if (m->count == 0) {
        printf("Le menu est vide.\n");
        return;
    }
    printf("\n%-4s ", "ID");
    print_padded("Catégorie", 12);
    putchar(' ');
    print_padded("Plat", 28);
    printf(" %8s\n", "Prix");
    printf("-----------------------------------------------------\n");
    for (int i = 0; i < m->count; i++) {
        const MenuItem *it = &m->items[i];
        printf("%-4d ", it->id);
        print_padded(it->category, 12);
        putchar(' ');
        print_padded(it->name, 28);
        printf(" %6.2f €\n", it->price);
    }
}

int menu_save(const Menu *m, const char *path)
{
    FILE *f = fopen(path, "w");
    if (!f)
        return 0;
    for (int i = 0; i < m->count; i++)
        fprintf(f, "%d|%s|%s|%.2f\n", m->items[i].id, m->items[i].category, m->items[i].name,
                m->items[i].price);
    fclose(f);
    return 1;
}

int menu_load(Menu *m, const char *path)
{
    FILE *f = fopen(path, "r");
    if (!f)
        return 0;
    char line[MAX_LINE];
    while (fgets(line, sizeof line, f)) {
        strip_newline(line);
        char *fields[4];
        if (split_fields(line, '|', fields, 4) != 4)
            continue;                              /* ligne invalide : ignorée */
        int id = atoi(fields[0]);
        float price = (float)atof(fields[3]);
        if (id <= 0 || price < 0 || !fields[2][0] || !menu_reserve(m))
            continue;
        MenuItem *it = &m->items[m->count++];
        it->id = id;
        snprintf(it->category, sizeof it->category, "%s", fields[1]);
        snprintf(it->name, sizeof it->name, "%s", fields[2]);
        it->price = price;
        if (id >= m->next_id)
            m->next_id = id + 1;
    }
    fclose(f);
    return 1;
}
