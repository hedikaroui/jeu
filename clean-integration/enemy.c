#include "game.h"

void enemy_init(Enemy *e, SDL_Renderer *renderer, int x, int y)
{
    e->texture = IMG_LoadTexture(renderer, "characters/enemy.png");
    e->position.x = x;
    e->position.y = y;
    e->position.w = 48;
    e->position.h = 48;
    e->active = 1;
    e->direction = 1;
    e->speed = 2;
}

void enemy_update(Enemy *e)
{
    if (!e->active) return;

    e->position.x += e->direction * e->speed;

    if (e->position.x < 0 || e->position.x > WIDTH)
        e->direction = -e->direction;

    if (e->position.y < 0 || e->position.y > HEIGHT)
        e->active = 0;
}

void enemy_render(SDL_Renderer *renderer, Enemy *e)
{
    if (e->texture && e->active)
        SDL_RenderCopy(renderer, e->texture, NULL, &e->position);
}
