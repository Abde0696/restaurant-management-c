#include <stdlib.h>

#include "app.h"
#include "ui.h"

int main(void)
{
    App app;
    if (!app_load(&app)) {
        app_free(&app);
        return EXIT_FAILURE;
    }
    ui_main_loop(&app);
    app_save(&app);
    app_free(&app);
    return EXIT_SUCCESS;
}
