#include "header.h"

int main(int argc, char *argv[])
{
    (void)argc; (void)argv;

    Game g;
    memset(&g, 0, sizeof(Game));
    g.state = STATE_MENU;

    if (!InitSDL(&g, "START", SCREEN_WIDTH, SCREEN_HEIGHT))
        return 1;

    LoadBackground(&g, "background lvl 1.jpg");
    LoadNameBackground(&g, "saisir nom bG1.png");
    InitBackground(&g, SCREEN_WIDTH, SCREEN_HEIGHT);
    InitPlatforms(&g, 1);
    InitPlayer(&g.player1, g.renderer, NULL, 100, 300);
    InitPlayer(&g.player2, g.renderer, NULL, 1000, 300);

    SDL_StartTextInput();

    int running = 1, keys[SDL_NUM_SCANCODES] = {0};
    SDL_Event e;

    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) { running = 0; break; }
            switch (g.state) {
                case STATE_MENU:
                    if (e.type == SDL_MOUSEBUTTONDOWN)
                        HandleMenuClick(&g, e.button.x, e.button.y);
                    break;
                case STATE_NAME_INPUT:
                    HandleNameInput(&g, &e);
                    break;
                case STATE_GAME:
                    if (e.type == SDL_MOUSEBUTTONDOWN)
                        HandleGuideClick(&g, e.button.x, e.button.y);
                    if (e.type == SDL_KEYDOWN) {
                        SDL_Scancode sc = e.key.keysym.scancode;
                        if (sc == SDL_SCANCODE_H || sc == SDL_SCANCODE_F1)
                            g.showGuide = !g.showGuide;
                        else if (sc == SDL_SCANCODE_ESCAPE) {
                            if (g.showGuide) g.showGuide = 0;
                            else { memset(keys, 0, sizeof(keys)); g.state = STATE_MENU; }
                        } else keys[sc] = 1;
                    }
                    if (e.type == SDL_KEYUP) keys[e.key.keysym.scancode] = 0;
                    break;
            }
        }

        switch (g.state) {
            case STATE_MENU:       RenderMenu(&g);       break;
            case STATE_NAME_INPUT: RenderNameInput(&g);  break;
            case STATE_GAME:
                if (g.splitScreen) {
                    UpdatePlayersSplit(&g, keys, LEVEL_WIDTH, LEVEL_HEIGHT);
                    RenderGameSplit(&g, keys);
                } else {
                    UpdatePlayersSingle(&g, keys, LEVEL_WIDTH, LEVEL_HEIGHT);
                    RenderGameSingle(&g, keys);
                }
                break;
        }
        SDL_Delay(16);
    }

    SDL_StopTextInput();
    CleanupSDL(&g);
    return 0;
}
