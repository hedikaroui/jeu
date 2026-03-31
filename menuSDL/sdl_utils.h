#ifndef SDL_UTILS_H
#define SDL_UTILS_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>

/*
 * Initialize SDL subsystems, create a window and renderer.
 * Returns 0 on success, non‑zero on failure.
 */
int sdl_init(SDL_Window **window, SDL_Renderer **renderer,
             const char *title, int width, int height);

/*
 * Clean up everything created by sdl_init.
 */
void sdl_cleanup(SDL_Window *window, SDL_Renderer *renderer);

#endif /* SDL_UTILS_H */
