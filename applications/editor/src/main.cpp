//
// Created by Jonathan Richard on 2024-01-29.
//

#include "application.h"

int main(int argc, char* argv[])
{
    application app;

    app.init();

    while (!app.shouldClose())
    {
        app.update();
    }

    app.exit();
}
