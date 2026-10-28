#include "orders.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void orders_init(OrderList *l)
{
    l->head = NULL;
    l->tail = NULL;
    l->next_id = 1;
}

static void order_destroy(Order *o)
{
    free(o->lines);
    free(o);
}

void orders_free(OrderList *l)
{
    Order *cur = l->head;
    while (cur) {
        Order *next = cur->next;
        order_destroy(cur);
        cur = next;
    }
    orders_init(l);
}

/* Ajoute une commande vide en fin de liste chaînée */
static Order *order_append(OrderList *l, int id, int table_number, OrderStatus status)
{
    Order *o = calloc(1, sizeof *o);
    if (!o)
        return NULL;
    o->id = id;
    o->table_number = table_number;
    o->status = status;
    if (l->tail)
        l->tail->next = o;
    else
        l->head = o;
    l->tail = o;
    if (id >= l->next_id)
        l->next_id = id + 1;
    return o;
}

Order *order_create(OrderList *l, int table_number)
{
    return order_append(l, l->next_id, table_number, ORDER_OPEN);
}

int order_remove(OrderList *l, int order_id)
{
    Order *prev = NULL;
    for (Order *cur = l->head; cur; prev = cur, cur = cur->next) {
        if (cur->id == order_id) {
            if (prev)
                prev->next = cur->next;
            else
                l->head = cur->next;
            if (l->tail == cur)
                l->tail = prev;
            order_destroy(cur);
            return 1;
        }
    }
    return 0;
}

Order *order_find(const OrderList *l, int order_id)
{
    for (Order *o = l->head; o; o = o->next)
        if (o->id == order_id)
            return o;
    return NULL;
}

Order *order_find_open_for_table(const OrderList *l, int table_number)
{
    for (Order *o = l->head; o; o = o->next)
        if (o->status == ORDER_OPEN && o->table_number == table_number)
            return o;
    return NULL;
}

static int line_reserve(Order *o)
{
    if (o->line_count < o->line_capacity)
        return 1;
    int new_cap = o->line_capacity ? o->line_capacity * 2 : 4;
    OrderLine *p = realloc(o->lines, (size_t)new_cap * sizeof *p);
    if (!p)
        return 0;
    o->lines = p;
    o->line_capacity = new_cap;
    return 1;
}

int order_add_item(Order *o, const MenuItem *item, int quantity)
{
    if (quantity <= 0)
        return 0;
    for (int i = 0; i < o->line_count; i++) {          /* même plat : on augmente la quantité */
        if (o->lines[i].item_id == item->id) {
            o->lines[i].quantity += quantity;
            return 1;
        }
    }
    if (!line_reserve(o))
        return 0;
    OrderLine *l = &o->lines[o->line_count++];
    l->item_id = item->id;
    snprintf(l->name, sizeof l->name, "%s", item->name);
    l->unit_price = item->price;
    l->quantity = quantity;
    return 1;
}

int order_remove_line(Order *o, int line_index)
{
    if (line_index < 0 || line_index >= o->line_count)
        return 0;
    memmove(&o->lines[line_index], &o->lines[line_index + 1],
            (size_t)(o->line_count - line_index - 1) * sizeof *o->lines);
    o->line_count--;
    return 1;
}

float order_total(const Order *o)
{
    float total = 0.0f;
    for (int i = 0; i < o->line_count; i++)
        total += o->lines[i].unit_price * (float)o->lines[i].quantity;
    return total;
}

void order_print(const Order *o)
{
    printf("\n=== Commande n°%d — Table %d (%s) ===\n", o->id, o->table_number,
           o->status == ORDER_OPEN ? "en cours" : "payée");
    if (o->line_count == 0) {
        printf("(aucun article)\n");
        return;
    }
    for (int i = 0; i < o->line_count; i++) {
        const OrderLine *l = &o->lines[i];
        printf("%2d. %2d x ", i + 1, l->quantity);
        print_padded(l->name, 26);
        printf(" %6.2f € = %7.2f €\n", l->unit_price, l->unit_price * (float)l->quantity);
    }
    printf("--------------------------------------------------\n");
    printf("TOTAL : %.2f €\n", order_total(o));
}

void orders_print_open(const OrderList *l)
{
    int found = 0;
    for (Order *o = l->head; o; o = o->next) {
        if (o->status == ORDER_OPEN) {
            printf("Commande n°%d — Table %d — %d article(s) — %.2f €\n", o->id, o->table_number,
                   o->line_count, order_total(o));
            found = 1;
        }
    }
    if (!found)
        printf("Aucune commande en cours.\n");
}

int orders_paid_count(const OrderList *l)
{
    int n = 0;
    for (Order *o = l->head; o; o = o->next)
        if (o->status == ORDER_PAID)
            n++;
    return n;
}

float orders_revenue(const OrderList *l)
{
    float total = 0.0f;
    for (Order *o = l->head; o; o = o->next)
        if (o->status == ORDER_PAID)
            total += order_total(o);
    return total;
}

/* Plat le plus vendu parmi les commandes payées (comparaison par nom). */
int orders_best_seller(const OrderList *l, char *name, int size, int *quantity)
{
    int best = 0;
    for (Order *o = l->head; o; o = o->next) {
        if (o->status != ORDER_PAID)
            continue;
        for (int i = 0; i < o->line_count; i++) {
            int qty = 0;
            for (Order *o2 = l->head; o2; o2 = o2->next) {
                if (o2->status != ORDER_PAID)
                    continue;
                for (int j = 0; j < o2->line_count; j++)
                    if (strcmp(o2->lines[j].name, o->lines[i].name) == 0)
                        qty += o2->lines[j].quantity;
            }
            if (qty > best) {
                best = qty;
                snprintf(name, (size_t)size, "%s", o->lines[i].name);
            }
        }
    }
    if (quantity)
        *quantity = best;
    return best > 0;
}

/* Format : une ligne ORDER|id|table|statut puis une ligne LINE|id_plat|nom|prix|quantité par article */
int orders_save(const OrderList *l, const char *path)
{
    FILE *f = fopen(path, "w");
    if (!f)
        return 0;
    for (Order *o = l->head; o; o = o->next) {
        fprintf(f, "ORDER|%d|%d|%d\n", o->id, o->table_number, (int)o->status);
        for (int i = 0; i < o->line_count; i++)
            fprintf(f, "LINE|%d|%s|%.2f|%d\n", o->lines[i].item_id, o->lines[i].name,
                    o->lines[i].unit_price, o->lines[i].quantity);
    }
    fclose(f);
    return 1;
}

int orders_load(OrderList *l, const char *path)
{
    FILE *f = fopen(path, "r");
    if (!f)
        return 0;
    char line[MAX_LINE];
    Order *current = NULL;
    while (fgets(line, sizeof line, f)) {
        strip_newline(line);
        char *fields[5];
        int n = split_fields(line, '|', fields, 5);
        if (n == 4 && strcmp(fields[0], "ORDER") == 0) {
            int id = atoi(fields[1]), table = atoi(fields[2]);
            current = (id > 0 && table > 0)
                          ? order_append(l, id, table, atoi(fields[3]) == ORDER_PAID ? ORDER_PAID : ORDER_OPEN)
                          : NULL;
        } else if (n == 5 && strcmp(fields[0], "LINE") == 0 && current) {
            MenuItem tmp;
            tmp.id = atoi(fields[1]);
            snprintf(tmp.name, sizeof tmp.name, "%s", fields[2]);
            tmp.price = (float)atof(fields[3]);
            tmp.category[0] = '\0';
            order_add_item(current, &tmp, atoi(fields[4]));
        }
    }
    fclose(f);
    return 1;
}
