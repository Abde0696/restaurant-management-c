#include "ui.h"

#include <stdio.h>
#include <string.h>

#include "utils.h"

static void save(const App *app)
{
    if (!app_save(app))
        printf("Attention : la sauvegarde dans le dossier data/ a échoué.\n");
}

/* ------------------------------------------------------------------ menu */
static void menu_screen(App *app)
{
    for (;;) {
        printf("\n--- Gestion du menu ---\n"
               "1. Afficher le menu\n2. Ajouter un plat\n3. Modifier le prix d'un plat\n"
               "4. Supprimer un plat\n0. Retour\n");
        switch (read_int("Choix : ", 0, 4)) {
        case 1:
            menu_print(&app->menu);
            break;
        case 2: {
            char name[MAX_NAME], category[MAX_CAT];
            read_line("Nom du plat : ", name, sizeof name);
            sanitize_text(name);
            if (!name[0]) {
                printf("Le nom ne peut pas être vide.\n");
                break;
            }
            read_line("Catégorie (Entrée, Plat, Dessert, Boisson...) : ", category, sizeof category);
            sanitize_text(category);
            if (!category[0])
                snprintf(category, sizeof category, "Plat");
            float price = read_float("Prix en € : ", 0.0f);
            if (menu_add(&app->menu, name, category, price)) {
                printf("Plat ajouté.\n");
                save(app);
            } else {
                printf("Erreur mémoire : plat non ajouté.\n");
            }
            break;
        }
        case 3: {
            menu_print(&app->menu);
            int id = read_int("ID du plat (0 pour annuler) : ", 0, 1000000);
            if (id == 0)
                break;
            MenuItem *it = menu_find(&app->menu, id);
            if (!it) {
                printf("Plat introuvable.\n");
                break;
            }
            it->price = read_float("Nouveau prix en € : ", 0.0f);
            printf("Prix modifié.\n");
            save(app);
            break;
        }
        case 4: {
            menu_print(&app->menu);
            int id = read_int("ID du plat à supprimer (0 pour annuler) : ", 0, 1000000);
            if (id == 0)
                break;
            if (menu_remove(&app->menu, id)) {
                printf("Plat supprimé.\n");
                save(app);
            } else {
                printf("Plat introuvable.\n");
            }
            break;
        }
        case 0:
            return;
        }
    }
}

/* ---------------------------------------------------------------- tables */
static void tables_print(const App *app)
{
    if (app->tables.count == 0) {
        printf("Aucune table.\n");
        return;
    }
    printf("\n%-8s %-8s %-10s %s\n", "Table", "Places", "Statut", "Commande");
    printf("--------------------------------------------\n");
    for (int i = 0; i < app->tables.count; i++) {
        const Table *t = &app->tables.tables[i];
        printf("%-8d %-8d %-10s ", t->number, t->seats, t->status == TABLE_FREE ? "libre" : "occupée");
        Order *o = order_find_open_for_table(&app->orders, t->number);
        if (o)
            printf("n°%d (%.2f €)", o->id, order_total(o));
        putchar('\n');
    }
}

static void tables_screen(App *app)
{
    for (;;) {
        printf("\n--- Gestion des tables ---\n"
               "1. Afficher les tables\n2. Ajouter une table\n3. Supprimer une table\n0. Retour\n");
        switch (read_int("Choix : ", 0, 3)) {
        case 1:
            tables_print(app);
            break;
        case 2: {
            int number = read_int("Numéro de la table : ", 1, 999);
            int seats = read_int("Nombre de places : ", 1, 50);
            if (tables_add(&app->tables, number, seats)) {
                printf("Table ajoutée.\n");
                save(app);
            } else {
                printf("Ce numéro de table existe déjà.\n");
            }
            break;
        }
        case 3: {
            tables_print(app);
            int number = read_int("Numéro de la table à supprimer (0 pour annuler) : ", 0, 999);
            if (number == 0)
                break;
            Table *t = tables_find(&app->tables, number);
            if (!t)
                printf("Table introuvable.\n");
            else if (t->status == TABLE_OCCUPIED)
                printf("Impossible : la table est occupée.\n");
            else if (tables_remove(&app->tables, number)) {
                printf("Table supprimée.\n");
                save(app);
            }
            break;
        }
        case 0:
            return;
        }
    }
}

/* ------------------------------------------------------------- commandes */
static Order *ask_open_order(App *app)
{
    printf("\nCommandes en cours :\n");
    orders_print_open(&app->orders);
    int id = read_int("N° de commande (0 pour annuler) : ", 0, 1000000);
    if (id == 0)
        return NULL;
    Order *o = order_find(&app->orders, id);
    if (!o || o->status != ORDER_OPEN) {
        printf("Aucune commande en cours avec ce numéro.\n");
        return NULL;
    }
    return o;
}

static void add_items_loop(App *app, Order *o)
{
    menu_print(&app->menu);
    for (;;) {
        int id = read_int("ID du plat à ajouter (0 pour terminer) : ", 0, 1000000);
        if (id == 0)
            break;
        MenuItem *it = menu_find(&app->menu, id);
        if (!it) {
            printf("Plat introuvable.\n");
            continue;
        }
        int qty = read_int("Quantité : ", 1, 99);
        if (order_add_item(o, it, qty))
            printf("%d x %s ajouté(s).\n", qty, it->name);
        else
            printf("Erreur : article non ajouté.\n");
    }
    save(app);
}

static void new_order(App *app)
{
    tables_print(app);
    int number = read_int("Numéro de la table (0 pour annuler) : ", 0, 999);
    if (number == 0)
        return;
    Table *t = tables_find(&app->tables, number);
    if (!t) {
        printf("Table introuvable.\n");
        return;
    }
    if (t->status == TABLE_OCCUPIED) {
        printf("Cette table est déjà occupée.\n");
        return;
    }
    Order *o = order_create(&app->orders, number);
    if (!o) {
        printf("Erreur mémoire.\n");
        return;
    }
    t->status = TABLE_OCCUPIED;
    printf("Commande n°%d créée pour la table %d.\n", o->id, number);
    add_items_loop(app, o);
}

static void remove_item(App *app)
{
    Order *o = ask_open_order(app);
    if (!o)
        return;
    order_print(o);
    if (o->line_count == 0)
        return;
    int n = read_int("N° de la ligne à retirer (0 pour annuler) : ", 0, o->line_count);
    if (n > 0 && order_remove_line(o, n - 1)) {
        printf("Ligne retirée.\n");
        save(app);
    }
}

static void show_order(const App *app)
{
    int id = read_int("N° de commande (0 pour annuler) : ", 0, 1000000);
    if (id == 0)
        return;
    Order *o = order_find(&app->orders, id);
    if (o)
        order_print(o);
    else
        printf("Commande introuvable.\n");
}

static void pay_order(App *app)
{
    Order *o = ask_open_order(app);
    if (!o)
        return;
    if (o->line_count == 0) {
        printf("Commande vide : ajoutez des articles ou annulez-la.\n");
        return;
    }
    order_print(o);
    if (!confirm("Confirmer l'encaissement ? (o/n) : "))
        return;
    o->status = ORDER_PAID;
    Table *t = tables_find(&app->tables, o->table_number);
    if (t)
        t->status = TABLE_FREE;
    printf("Paiement enregistré. La table %d est libre.\n", o->table_number);
    save(app);
}

static void cancel_order(App *app)
{
    Order *o = ask_open_order(app);
    if (!o)
        return;
    if (!confirm("Annuler cette commande ? (o/n) : "))
        return;
    int table_number = o->table_number;
    order_remove(&app->orders, o->id);
    Table *t = tables_find(&app->tables, table_number);
    if (t)
        t->status = TABLE_FREE;
    printf("Commande annulée.\n");
    save(app);
}

static void orders_screen(App *app)
{
    for (;;) {
        printf("\n--- Gestion des commandes ---\n"
               "1. Nouvelle commande\n2. Ajouter des articles à une commande\n"
               "3. Retirer un article d'une commande\n4. Afficher une commande\n"
               "5. Commandes en cours\n6. Encaisser une commande\n"
               "7. Annuler une commande\n0. Retour\n");
        switch (read_int("Choix : ", 0, 7)) {
        case 1:
            new_order(app);
            break;
        case 2: {
            Order *o = ask_open_order(app);
            if (o)
                add_items_loop(app, o);
            break;
        }
        case 3:
            remove_item(app);
            break;
        case 4:
            show_order(app);
            break;
        case 5:
            orders_print_open(&app->orders);
            break;
        case 6:
            pay_order(app);
            break;
        case 7:
            cancel_order(app);
            break;
        case 0:
            return;
        }
    }
}

/* --------------------------------------------------------------- rapport */
static void report_screen(const App *app)
{
    int paid = orders_paid_count(&app->orders);
    float revenue = orders_revenue(&app->orders);
    printf("\n--- Rapport ---\n");
    printf("Commandes payées : %d\n", paid);
    printf("Chiffre d'affaires : %.2f €\n", revenue);
    if (paid > 0)
        printf("Panier moyen : %.2f €\n", revenue / (float)paid);

    char best[MAX_NAME];
    int qty;
    if (orders_best_seller(&app->orders, best, sizeof best, &qty))
        printf("Plat le plus vendu : %s (%d)\n", best, qty);

    int occupied = 0;
    for (int i = 0; i < app->tables.count; i++)
        if (app->tables.tables[i].status == TABLE_OCCUPIED)
            occupied++;
    printf("Tables occupées : %d / %d\n", occupied, app->tables.count);
}

/* ------------------------------------------------------------------ main */
void ui_main_loop(App *app)
{
    printf("=== Gestion de restaurant ===\n");
    for (;;) {
        printf("\n--- Menu principal ---\n"
               "1. Menu (plats et prix)\n2. Tables\n3. Commandes\n4. Rapport\n0. Quitter\n");
        switch (read_int("Choix : ", 0, 4)) {
        case 1:
            menu_screen(app);
            break;
        case 2:
            tables_screen(app);
            break;
        case 3:
            orders_screen(app);
            break;
        case 4:
            report_screen(app);
            break;
        case 0:
            printf("Données sauvegardées. À bientôt !\n");
            return;
        }
    }
}
