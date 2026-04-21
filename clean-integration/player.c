#include "game.h"

void player_init(Player *p, SDL_Renderer *renderer, int x, int y)
{
    p->texture = IMG_LoadTexture(renderer, "characters/player.png");
    p->position.x = x;
    p->position.y = y;
    p->position.w = 64;
    p->position.h = 64;
    p->health = 100;
    p->score = 0;
    p->facing = 1;
    p->jumping = 0;
}

void player_move(Player *p, int dx, int dy)
{
    p->position.x += dx;
    p->position.y += dy;

    if (p->position.x < 0) p->position.x = 0;
    if (p->position.y < 0) p->position.y = 0;
    if (p->position.x + p->position.w > WIDTH)
        p->position.x = WIDTH - p->position.w;
    if (p->position.y + p->position.h > HEIGHT)
        p->position.y = HEIGHT - p->position.h;
}

void player_jump(Player *p)
{
    if (!p->jumping)
    {
        p->jumping = 1;
        p->position.y -= 50;
    }
}

void player_render(SDL_Renderer *renderer, Player *p)
{
    if (p->texture)
        SDL_RenderCopy(renderer, p->texture, NULL, &p->position);
}
