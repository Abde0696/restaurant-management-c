#ifndef APP_H
#define APP_H

#include "menu.h"
#include "tables.h"
#include "orders.h"

typedef struct {
    Menu      menu;
    TableList tables;
    OrderList orders;
} App;

int  app_load(App *app);          /* charge les fichiers (ou crée des données d'exemple) */
int  app_save(const App *app);
void app_free(App *app);

#endif
