#include "header.h"

int main(int argc, char *argv[])
{
    (void)argc; (void)argv;

    Game g;
    int  keys[SDL_NUM_SCANCODES] = {0};

    if (!InitGame(&g, "Mon Jeu SDL2", SCREEN_WIDTH, SCREEN_HEIGHT))
        return 1;
    if (!LoadRessources(&g, "background lvl 1.jpg", "saisir nom bG1.png"))
        return 1;

    while (g.running) {
        HandleEvents(&g, keys);
        Update(&g, keys);
        Render(&g, keys);
        SDL_Delay(16);
    }

    Cleanup(&g);
    return 0;
}
