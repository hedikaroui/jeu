#include "header.h"

int main(void) {
    Game puzzle;
    if (!game_init(&puzzle)) return 1;
    game_load(&puzzle);

    SDL_Event e;
    int running = 1;
    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = 0;
            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) running = 0;
            game_event(&puzzle, &e);
        }
        game_update(&puzzle);
        game_draw(&puzzle);
    }

    game_free(&puzzle);
    return 0;
}
