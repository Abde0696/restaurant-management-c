#ifndef ORDERS_H
#define ORDERS_H

#include "menu.h"

typedef struct {
    int   item_id;
    char  name[MAX_NAME];   /* copie du nom et du prix au moment de la commande */
    float unit_price;
    int   quantity;
} OrderLine;

typedef enum { ORDER_OPEN = 0, ORDER_PAID = 1 } OrderStatus;

/* Une commande = tableau dynamique de lignes, chaînée avec la suivante (liste chaînée) */
typedef struct Order {
    int           id;
    int           table_number;
    OrderStatus   status;
    OrderLine    *lines;
    int           line_count;
    int           line_capacity;
    struct Order *next;
} Order;

typedef struct {
    Order *head;
    Order *tail;
    int    next_id;
} OrderList;

void   orders_init(OrderList *l);
void   orders_free(OrderList *l);
Order *order_create(OrderList *l, int table_number);
int    order_remove(OrderList *l, int order_id);                 /* supprime de la liste chaînée */
Order *order_find(const OrderList *l, int order_id);
Order *order_find_open_for_table(const OrderList *l, int table_number);

int    order_add_item(Order *o, const MenuItem *item, int quantity);
int    order_remove_line(Order *o, int line_index);              /* index à partir de 0 */
float  order_total(const Order *o);
void   order_print(const Order *o);
void   orders_print_open(const OrderList *l);

/* Statistiques */
int    orders_paid_count(const OrderList *l);
float  orders_revenue(const OrderList *l);
int    orders_best_seller(const OrderList *l, char *name, int size, int *quantity);

int    orders_save(const OrderList *l, const char *path);
int    orders_load(OrderList *l, const char *path);

#endif
