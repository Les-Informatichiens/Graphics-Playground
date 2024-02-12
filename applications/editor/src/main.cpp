//
// Created by Jonathan Richard on 2024-01-29.
//

#include "application.h"
#include "renderer/renderAPI/openGLRenderer.h"
int main(int argc, char* argv[])
{
    // Renderer init
    openGLRenderer renderer;
    renderer.init();

    // Engine init
    engine engine(renderer);

    // Application init
    application app(engine);
    if (!app.init())
        return -1;
    app.run();
}