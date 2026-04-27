#include "game.h"

int game_init(GameState *gs, SDL_Renderer *renderer)
{
    int i;

    gs->game_running = 1;
    gs->game_state = 0;
    gs->current_level = 1;

    player_init(&gs->players[0], renderer, 100, 100);
    if (MAX_PLAYERS > 1)
        player_init(&gs->players[1], renderer, 150, 100);

    background_init(&gs->background, renderer, "backgrounds/bg.png");
    minimap_init(&gs->minimap, renderer);

    for (i = 0; i < MAX_ENEMIES && i < 10; i++)
        enemy_init(&gs->enemies[i], renderer, 300 + i * 100, 200);

    for (i = 0; i < MAX_PUZZLES && i < 5; i++)
        puzzle_init(&gs->puzzles[i], renderer, 400 + i * 150, 300);

    for (i = 0; i < MAX_QUIZZES && i < 3; i++)
        quiz_init(&gs->quizzes[i], "Question?", "Option 1", "Option 2", "Option 3", "Option 4", 0);

    return 1;
}

void game_update(GameState *gs)
{
    int i;

    if (!gs->game_running) return;

    background_update(&gs->background);

    for (i = 0; i < MAX_ENEMIES; i++)
        enemy_update(&gs->enemies[i]);

    for (i = 0; i < MAX_PUZZLES; i++)
        puzzle_check(&gs->puzzles[i], &gs->players[0]);

    minimap_update(&gs->minimap, &gs->players[0]);
}

void game_render(SDL_Renderer *renderer, GameState *gs)
{
    int i;

    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
    SDL_RenderClear(renderer);

    background_render(renderer, &gs->background);

    for (i = 0; i < MAX_ENEMIES; i++)
        enemy_render(renderer, &gs->enemies[i]);

    for (i = 0; i < MAX_PUZZLES; i++)
        puzzle_render(renderer, &gs->puzzles[i]);

    player_render(renderer, &gs->players[0]);
    if (MAX_PLAYERS > 1)
        player_render(renderer, &gs->players[1]);

    minimap_render(renderer, &gs->minimap);

    SDL_RenderPresent(renderer);
}

void game_cleanup(GameState *gs)
{
    int i;

    for (i = 0; i < MAX_PLAYERS; i++)
        if (gs->players[i].texture)
            SDL_DestroyTexture(gs->players[i].texture);

    for (i = 0; i < MAX_ENEMIES; i++)
        if (gs->enemies[i].texture)
            SDL_DestroyTexture(gs->enemies[i].texture);

    for (i = 0; i < MAX_PUZZLES; i++)
        if (gs->puzzles[i].texture)
            SDL_DestroyTexture(gs->puzzles[i].texture);

    if (gs->background.texture)
        SDL_DestroyTexture(gs->background.texture);
}
