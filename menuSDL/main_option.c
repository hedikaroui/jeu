#include <stdio.h>
#include "menu.h"
#include "sdl_utils.h"

int main(int argc, char *argv[])
{
    SDL_Window *window = NULL;
    SDL_Renderer *renderer = NULL;

    if (sdl_init(&window, &renderer, "Menu", 1280, 720) != 0) {
        fprintf(stderr, "Failed to initialize SDL subsystems\n");
        return 1;
    }

    /* run the options menu; return value could be used for error handling */
    if (menu(window, renderer) != 0) {
        fprintf(stderr, "Menu returned an error\n");
    }

    sdl_cleanup(window, renderer);
    return 0;
}
