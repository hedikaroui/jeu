#include "game.h"

void puzzle_init(Puzzle *pz, SDL_Renderer *renderer, int x, int y)
{
    pz->texture = IMG_LoadTexture(renderer, "puzzles/puzzle.png");
    pz->position.x = x;
    pz->position.y = y;
    pz->position.w = 80;
    pz->position.h = 80;
    pz->solved = 0;
}

void puzzle_check(Puzzle *pz, Player *p)
{
    if (pz->solved) return;

    if (p->position.x < pz->position.x + pz->position.w &&
        p->position.x + p->position.w > pz->position.x &&
        p->position.y < pz->position.y + pz->position.h &&
        p->position.y + p->position.h > pz->position.y)
    {
        pz->solved = 1;
        p->score += 100;
    }
}

void puzzle_render(SDL_Renderer *renderer, Puzzle *pz)
{
    if (pz->texture && !pz->solved)
        SDL_RenderCopy(renderer, pz->texture, NULL, &pz->position);
}
