#include "app.h"

#include <stdio.h>

#include "utils.h"

#define DATA_DIR    "data"
#define MENU_FILE   DATA_DIR "/menu.txt"
#define TABLES_FILE DATA_DIR "/tables.txt"
#define ORDERS_FILE DATA_DIR "/orders.txt"

static void seed_menu(Menu *m)
{
    menu_add(m, "Salade César", "Entrée", 8.50f);
    menu_add(m, "Soupe du jour", "Entrée", 6.00f);
    menu_add(m, "Burger maison", "Plat", 14.50f);
    menu_add(m, "Pizza Margherita", "Plat", 12.00f);
    menu_add(m, "Pâtes carbonara", "Plat", 13.00f);
    menu_add(m, "Steak frites", "Plat", 18.50f);
    menu_add(m, "Tiramisu", "Dessert", 6.50f);
    menu_add(m, "Crème brûlée", "Dessert", 6.00f);
    menu_add(m, "Eau minérale", "Boisson", 3.00f);
    menu_add(m, "Soda", "Boisson", 3.50f);
    menu_add(m, "Café", "Boisson", 2.50f);
}

static void seed_tables(TableList *t)
{
    const int seats[] = {2, 2, 4, 4, 6, 8};
    for (int i = 0; i < 6; i++)
        tables_add(t, i + 1, seats[i]);
}

int app_load(App *app)
{
    menu_init(&app->menu);
    tables_init(&app->tables);
    orders_init(&app->orders);

    if (!ensure_dir(DATA_DIR)) {
        fprintf(stderr, "Impossible de créer le dossier '%s'.\n", DATA_DIR);
        return 0;
    }
    if (!menu_load(&app->menu, MENU_FILE))
        seed_menu(&app->menu);                 /* premier lancement : données d'exemple */
    if (!tables_load(&app->tables, TABLES_FILE))
        seed_tables(&app->tables);
    orders_load(&app->orders, ORDERS_FILE);    /* pas de fichier = pas encore de commandes */
    return app_save(app);
}

int app_save(const App *app)
{
    int ok = menu_save(&app->menu, MENU_FILE);
    ok &= tables_save(&app->tables, TABLES_FILE);
    ok &= orders_save(&app->orders, ORDERS_FILE);
    return ok;
}

void app_free(App *app)
{
    menu_free(&app->menu);
    tables_free(&app->tables);
    orders_free(&app->orders);
}
