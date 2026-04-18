#include "game.h"
#include <stdio.h>

static int game_jump_latch = 0;
static Uint32 game_last_jump_debug_tick = 0;

#define GAME_SHEET_ROWS 5
#define GAME_SHEET_COLS 5
#define GAME_JUMP_DURATION_MS 620.0

static const SDL_Rect game_walk_crop = {88, 48, 88, 167};
static const SDL_Rect game_walk_back_crop = {80, 46, 91, 169};
static const SDL_Rect game_jump_crop = {88, 48, 80, 160};
static const SDL_Rect game_jump_back_crop = {95, 64, 77, 144};

static void game_draw_center_text(SDL_Renderer *renderer, TTF_Font *font, const char *text,
                                  SDL_Color color, SDL_Rect box) {
    SDL_Surface *surf;
    SDL_Texture *tex;
    SDL_Rect dst;

    if (!renderer || !font || !text || !*text) return;

    surf = TTF_RenderUTF8_Blended(font, text, color);
    if (!surf) return;

    tex = SDL_CreateTextureFromSurface(renderer, surf);
    if (!tex) {
        SDL_FreeSurface(surf);
        return;
    }

    dst = (SDL_Rect){
        box.x + (box.w - surf->w) / 2,
        box.y + (box.h - surf->h) / 2,
        surf->w,
        surf->h
    };
    SDL_RenderCopy(renderer, tex, NULL, &dst);
    SDL_DestroyTexture(tex);
    SDL_FreeSurface(surf);
}

static void game_draw_sheet_frame(SDL_Renderer *renderer, SDL_Texture *tex, int frame_index,
                                  SDL_Rect crop, SDL_Rect dst_rect) {
    int tex_w = 0;
    int tex_h = 0;
    int frame_w;
    int frame_h;
    int frame_col;
    int frame_row;
    SDL_Rect src;

    if (!renderer || !tex) return;
    if (SDL_QueryTexture(tex, NULL, NULL, &tex_w, &tex_h) != 0 || tex_w <= 0 || tex_h <= 0) return;

    frame_w = tex_w / GAME_SHEET_COLS;
    frame_h = tex_h / GAME_SHEET_ROWS;
    frame_col = frame_index % GAME_SHEET_COLS;
    frame_row = frame_index / GAME_SHEET_COLS;

    src = (SDL_Rect){
        frame_col * frame_w + crop.x,
        frame_row * frame_h + crop.y,
        crop.w,
        crop.h
    };

    SDL_RenderCopy(renderer, tex, &src, &dst_rect);
}

static void game_draw_sheet_frame_reverse(SDL_Renderer *renderer, SDL_Texture *tex, int frame_index,
                                          SDL_Rect crop, SDL_Rect dst_rect) {
    int frame_count = GAME_SHEET_ROWS * GAME_SHEET_COLS;
    if (frame_count < 1) frame_count = 1;
    game_draw_sheet_frame(renderer, tex, frame_count - 1 - (frame_index % frame_count),
                          crop, dst_rect);
}

static void game_draw_sheet_full_cell(SDL_Renderer *renderer, SDL_Texture *tex, int frame_index,
                                      SDL_Rect dst_rect) {
    int tex_w = 0;
    int tex_h = 0;
    int frame_w;
    int frame_h;
    int frame_col;
    int frame_row;
    SDL_Rect src;

    if (!renderer || !tex) return;
    if (SDL_QueryTexture(tex, NULL, NULL, &tex_w, &tex_h) != 0 || tex_w <= 0 || tex_h <= 0) return;

    frame_w = tex_w / GAME_SHEET_COLS;
    frame_h = tex_h / GAME_SHEET_ROWS;
    if (frame_w <= 0 || frame_h <= 0) return;

    frame_col = frame_index % GAME_SHEET_COLS;
    frame_row = frame_index / GAME_SHEET_COLS;

    src = (SDL_Rect){
        frame_col * frame_w,
        frame_row * frame_h,
        frame_w,
        frame_h
    };
    SDL_RenderCopy(renderer, tex, &src, &dst_rect);
}

static void game_draw_sheet_full_cell_reverse(SDL_Renderer *renderer, SDL_Texture *tex,
                                              int frame_index, SDL_Rect dst_rect) {
    int frame_count = GAME_SHEET_ROWS * GAME_SHEET_COLS;
    if (frame_count < 1) frame_count = 1;
    game_draw_sheet_full_cell(renderer, tex, frame_count - 1 - (frame_index % frame_count),
                              dst_rect);
}

static void game_state_reset_character(Personnage *p) {
    if (!p) return;

    p->position = (SDL_Rect){0, 0, 90, 170};
    p->groundY = HEIGHT - 70 - p->position.h;
    p->position.x = (WIDTH - p->position.w) / 2;
    p->position.y = p->groundY;
    p->up = 0;
    p->jumpPhase = 0;
    p->posinit = p->position.y;
    p->posinitX = p->position.x;
    p->jumpRelX = -50.0;
    p->jumpRelY = 0.0;
    p->jumpProgress = 0.0;
    p->jumpDir = 0;
    p->jumpSpeed = 16;
    p->jumpHeight = 170;
    p->moveSpeed = 4;
    p->gravity = 18;
    p->facing = 1;
    p->moving = 0;
    p->movementState = GAME_MOVE_STOP;
    p->pendingJump = 0;
    p->frameIndex = 0;
    p->lastFrameTick = SDL_GetTicks();
    game_last_jump_debug_tick = 0;
    game_jump_latch = 0;
}

static void game_set_movement(Personnage *p, GameMovement movement, Uint32 now) {
    if (!p) return;
    if ((GameMovement)p->movementState != movement) {
        p->movementState = (int)movement;
        p->frameIndex = 0;
        p->lastFrameTick = now;
    }
}

static void game_move_stop(Personnage *p, Uint32 now) {
    if (!p || p->up) return;
    p->moving = 0;
    game_set_movement(p, GAME_MOVE_STOP, now);
}

static void game_move_walk(Personnage *p, int step, Uint32 now) {
    if (!p) return;
    p->position.x += step;
    p->facing = 1;
    p->moving = 1;
    if (!p->up) game_set_movement(p, GAME_MOVE_WALK, now);
}

static void game_move_walk_back(Personnage *p, int step, Uint32 now) {
    if (!p) return;
    p->position.x -= step;
    p->facing = -1;
    p->moving = 1;
    if (!p->up) game_set_movement(p, GAME_MOVE_WALK_BACK, now);
}

static void game_move_run(Personnage *p, int step, Uint32 now) {
    if (!p) return;
    p->position.x += step;
    p->facing = 1;
    p->moving = 1;
    if (!p->up) game_set_movement(p, GAME_MOVE_RUN, now);
}

static void game_move_run_back(Personnage *p, int step, Uint32 now) {
    if (!p) return;
    p->position.x -= step;
    p->facing = -1;
    p->moving = 1;
    if (!p->up) game_set_movement(p, GAME_MOVE_RUN_BACK, now);
}

static void game_move_jump(Personnage *p, Uint32 now) {
    if (!p || p->up) return;
    p->up = 1;
    p->jumpPhase = 1;
    p->moving = 0;
    p->facing = 1;
    p->posinitX = p->position.x;
    p->posinit = p->position.y;
    p->jumpRelX = -50.0;
    p->jumpRelY = 0.0;
    p->jumpProgress = 0.0;
    p->jumpDir = 1;
    game_set_movement(p, GAME_MOVE_JUMP, now);
    printf("[JUMP] start forward base=(%d,%d)\n", p->posinitX, p->posinit);
    fflush(stdout);
}

static void game_move_jump_back(Personnage *p, Uint32 now) {
    if (!p || p->up) return;
    p->up = 1;
    p->jumpPhase = 1;
    p->moving = 0;
    p->facing = -1;
    p->posinitX = p->position.x;
    p->posinit = p->position.y;
    p->jumpRelX = -50.0;
    p->jumpRelY = 0.0;
    p->jumpProgress = 0.0;
    p->jumpDir = -1;
    game_set_movement(p, GAME_MOVE_JUMP_BACK, now);
    printf("[JUMP] start back base=(%d,%d)\n", p->posinitX, p->posinit);
    fflush(stdout);
}

static void game_move_jump_up(Personnage *p, Uint32 now) {
    if (!p || p->up) return;
    p->up = 1;
    p->jumpPhase = 1;
    p->moving = 0;
    p->posinitX = p->position.x;
    p->posinit = p->position.y;
    p->jumpRelX = -50.0;
    p->jumpRelY = 0.0;
    p->jumpProgress = 0.0;
    p->jumpDir = 0;
    game_set_movement(p, (p->facing < 0) ? GAME_MOVE_JUMP_BACK : GAME_MOVE_JUMP, now);
    printf("[JUMP] start vertical base=(%d,%d)\n", p->posinitX, p->posinit);
    fflush(stdout);
}

static void game_update_jump(Personnage *p, Uint32 dt, Uint32 now) {
    double t;
    double smooth_t;
    double rel_y;
    int dir;

    if (!p || !p->up) return;

    p->jumpProgress += (double)dt / GAME_JUMP_DURATION_MS;
    if (p->jumpProgress > 1.0) p->jumpProgress = 1.0;

    t = p->jumpProgress;
    smooth_t = t * t * (3.0 - 2.0 * t);
    p->jumpRelX = -50.0 + (100.0 * smooth_t);

    rel_y = (-0.04 * (p->jumpRelX * p->jumpRelX)) + 100.0;
    p->jumpRelY = rel_y;

    dir = p->jumpDir;
    p->position.x = p->posinitX + (int)lround((p->jumpRelX + 50.0) * dir);
    p->position.y = p->posinit - (int)lround(p->jumpRelY);

    if (now - game_last_jump_debug_tick >= 60u) {
        printf("[JUMP] relX=%.2f relY=%.2f pos=(%d,%d) facing=%d up=%d\n",
               p->jumpRelX, p->jumpRelY, p->position.x, p->position.y, p->facing, p->up);
        fflush(stdout);
        game_last_jump_debug_tick = now;
    }

    if (p->jumpProgress >= 1.0) {
        p->jumpRelX = -50.0;
        p->jumpRelY = 0.0;
        p->jumpProgress = 0.0;
        p->up = 0; /* fin du saut */
        p->jumpPhase = 0;
        p->moving = 0;
        p->position.y = p->groundY;
        printf("[JUMP] end pos=(%d,%d)\n", p->position.x, p->position.y);
        fflush(stdout);
        game_set_movement(p, GAME_MOVE_STOP, now);
    }
}

static void game_move_animation(Personnage *p, Uint32 now) {
    Uint32 frame_delay;
    int frame_count;

    if (!p) return;

    switch ((GameMovement)p->movementState) {
        case GAME_MOVE_STOP:
            frame_delay = 140u;
            frame_count = GAME_SHEET_ROWS * GAME_SHEET_COLS;
            break;
        case GAME_MOVE_RUN:
            frame_delay = 70u;
            frame_count = GAME_SHEET_ROWS * GAME_SHEET_COLS;
            break;
        case GAME_MOVE_RUN_BACK:
            frame_delay = 70u;
            frame_count = GAME_SHEET_COLS;
            break;
        case GAME_MOVE_JUMP:
        case GAME_MOVE_JUMP_BACK:
            frame_delay = 90u;
            frame_count = GAME_SHEET_COLS;
            break;
        case GAME_MOVE_WALK:
        case GAME_MOVE_WALK_BACK:
        default:
            frame_delay = 110u;
            frame_count = GAME_SHEET_COLS;
            break;
    }

    if (now - p->lastFrameTick < frame_delay) return;
    if (frame_count < 1) frame_count = 1;
    p->frameIndex = (p->frameIndex + 1) % frame_count;
    p->lastFrameTick = now;
}

int Game_Charger(Game *game, SDL_Renderer *renderer) {
    if (!game || !renderer) return 0;
    if (game->gameLoaded) return 1;

    if (!game->gameCharacter.idleTexture)
        game->gameCharacter.idleTexture = IMG_LoadTexture(renderer, "spritesheet_characters/mr_harry_stand_up.png");
    if (!game->gameCharacter.idleBackTexture)
        game->gameCharacter.idleBackTexture = IMG_LoadTexture(renderer, "spritesheet_characters/mr_harry_stand_up_back.png");
    if (!game->gameCharacter.walkTexture)
        game->gameCharacter.walkTexture = IMG_LoadTexture(renderer, "spritesheet_characters/mr_harry_walk_cycle_transparent.png");
    if (!game->gameCharacter.walkBackTexture)
        game->gameCharacter.walkBackTexture = IMG_LoadTexture(renderer, "spritesheet_characters/mr_harry_walk_cycle_back_transparent.png");
    if (!game->gameCharacter.runTexture)
        game->gameCharacter.runTexture = IMG_LoadTexture(renderer, "spritesheet_characters/mr_harry_run.png");
    if (!game->gameCharacter.jumpTexture)
        game->gameCharacter.jumpTexture = IMG_LoadTexture(renderer, "spritesheet_characters/mr_harry_jump_transparent.png");
    if (!game->gameCharacter.jumpBackTexture)
        game->gameCharacter.jumpBackTexture = IMG_LoadTexture(renderer, "spritesheet_characters/mr_harry_jump_back_transparent.png");

    game_state_reset_character(&game->gameCharacter);
    game->gameLastTick = SDL_GetTicks();
    game->gameLoaded = 1;
    printf("[GAME] loaded. Jump keys: SPACE / UP / W\n");
    fflush(stdout);
    return 1;
}

void Game_LectureEntree(Game *game) {
    SDL_Event e;

    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            game->running = 0;
            return;
        }
        if (e.type == SDL_KEYDOWN) {
            SDL_Keycode sym = e.key.keysym.sym;
            if (sym == SDLK_ESCAPE) {
                Game_ResetRuntime(game);
                Game_SetSubState(game, STATE_MENU);
                if (game->music) Mix_PlayMusic(game->music, -1);
                return;
            }
            if (e.key.keysym.scancode == SDL_SCANCODE_SPACE ||
                e.key.keysym.scancode == SDL_SCANCODE_UP ||
                e.key.keysym.scancode == SDL_SCANCODE_W) {
                game->gameCharacter.pendingJump = 1;
                printf("[INPUT] jump key pressed: %s\n", SDL_GetKeyName(sym));
                fflush(stdout);
            }
        }
    }
}

void Game_MiseAJour(Game *game) {
    Uint32 now;
    Uint32 dt;
    const Uint8 *keys;
    int walk_step;
    int run_step;
    int left_pressed;
    int right_pressed;
    int jump_pressed;
    int run_modifier;

    if (!game || !game->gameLoaded) return;

    SDL_PumpEvents();
    now = SDL_GetTicks();
    dt = (game->gameLastTick == 0) ? 16u : (now - game->gameLastTick);
    game->gameLastTick = now;

    keys = SDL_GetKeyboardState(NULL);
    walk_step = (int)lround((double)game->gameCharacter.moveSpeed * ((double)dt / 16.0));
    run_step = (int)lround((double)(game->gameCharacter.moveSpeed + 8) * ((double)dt / 16.0));
    if (walk_step < 1) walk_step = 1;
    if (run_step < 1) run_step = 1;

    left_pressed = (keys[SDL_SCANCODE_LEFT] || keys[SDL_SCANCODE_A]);
    right_pressed = (keys[SDL_SCANCODE_RIGHT] || keys[SDL_SCANCODE_D]);
    jump_pressed = game->gameCharacter.pendingJump ||
                   keys[SDL_SCANCODE_SPACE] ||
                   keys[SDL_SCANCODE_UP] ||
                   keys[SDL_SCANCODE_W];
    run_modifier = (keys[SDL_SCANCODE_LSHIFT] || keys[SDL_SCANCODE_RSHIFT]);

    if (jump_pressed && !game->gameCharacter.up) {
        if (left_pressed && !right_pressed) game_move_jump_back(&game->gameCharacter, now);
        else if (right_pressed && !left_pressed) game_move_jump(&game->gameCharacter, now);
        else game_move_jump_up(&game->gameCharacter, now);
    }
    game_jump_latch = jump_pressed;
    game->gameCharacter.pendingJump = 0;

    if (game->gameCharacter.up) {
        if (game->gameCharacter.facing < 0)
            game_set_movement(&game->gameCharacter, GAME_MOVE_JUMP_BACK, now);
        else
            game_set_movement(&game->gameCharacter, GAME_MOVE_JUMP, now);
        game_update_jump(&game->gameCharacter, dt, now);
    } else {
        game->gameCharacter.moving = 0;
        if (left_pressed && !right_pressed) {
            if (run_modifier) game_move_run_back(&game->gameCharacter, run_step, now);
            else game_move_walk_back(&game->gameCharacter, walk_step, now);
        } else if (right_pressed && !left_pressed) {
            if (run_modifier) game_move_run(&game->gameCharacter, run_step, now);
            else game_move_walk(&game->gameCharacter, walk_step, now);
        } else {
            game_move_stop(&game->gameCharacter, now);
        }
    }

    if (game->gameCharacter.position.x < 0)
        game->gameCharacter.position.x = 0;
    if (game->gameCharacter.position.x + game->gameCharacter.position.w > WIDTH)
        game->gameCharacter.position.x = WIDTH - game->gameCharacter.position.w;
    if (game->gameCharacter.position.y < 0)
        game->gameCharacter.position.y = 0;
    if (game->gameCharacter.position.y > game->gameCharacter.groundY)
        game->gameCharacter.position.y = game->gameCharacter.groundY;

    game_move_animation(&game->gameCharacter, now);

    SDL_Delay(16);
}

void Game_Affichage(Game *game, SDL_Renderer *renderer) {
    int ground_line_y;
    SDL_Rect ground_rect;
    SDL_Rect hint_rect = {(WIDTH - 620) / 2, 88, 620, 42};
    SDL_Rect left_card = {80, 140, 420, 500};
    SDL_Rect right_card = {WIDTH - 500, 140, 420, 500};
    SDL_Color white = {255, 255, 255, 255};
    SDL_Color soft = {220, 220, 220, 255};

    if (!game || !renderer) return;

    ground_line_y = game->gameCharacter.groundY + game->gameCharacter.position.h;
    ground_rect = (SDL_Rect){0, ground_line_y, WIDTH, HEIGHT - ground_line_y};

    if (game->gameBgTex)
        SDL_RenderCopy(renderer, game->gameBgTex, NULL, NULL);
    else if (game->background)
        SDL_RenderCopy(renderer, game->background, NULL, NULL);

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 18, 18, 18, 170);
    SDL_RenderFillRect(renderer, &ground_rect);
    SDL_SetRenderDrawColor(renderer, 255, 214, 10, 255);
    SDL_RenderDrawLine(renderer, 0, ground_rect.y, WIDTH, ground_rect.y);

    switch ((GameMovement)game->gameCharacter.movementState) {
        case GAME_MOVE_JUMP_BACK:
            if (game->gameCharacter.jumpBackTexture)
                game_draw_sheet_frame_reverse(renderer, game->gameCharacter.jumpBackTexture,
                                              game->gameCharacter.frameIndex, game_jump_back_crop,
                                              game->gameCharacter.position);
            break;
        case GAME_MOVE_JUMP:
            if (game->gameCharacter.jumpTexture)
                game_draw_sheet_frame(renderer, game->gameCharacter.jumpTexture,
                                      game->gameCharacter.frameIndex, game_jump_crop,
                                      game->gameCharacter.position);
            break;
        case GAME_MOVE_RUN_BACK:
            if (game->gameCharacter.walkBackTexture)
                game_draw_sheet_frame_reverse(renderer, game->gameCharacter.walkBackTexture,
                                              game->gameCharacter.frameIndex, game_walk_back_crop,
                                              game->gameCharacter.position);
            break;
        case GAME_MOVE_RUN:
            if (game->gameCharacter.runTexture)
                game_draw_sheet_full_cell(renderer, game->gameCharacter.runTexture,
                                          game->gameCharacter.frameIndex,
                                          game->gameCharacter.position);
            else if (game->gameCharacter.walkTexture)
                game_draw_sheet_frame(renderer, game->gameCharacter.walkTexture,
                                      game->gameCharacter.frameIndex, game_walk_crop,
                                      game->gameCharacter.position);
            break;
        case GAME_MOVE_WALK_BACK:
            if (game->gameCharacter.walkBackTexture)
                game_draw_sheet_frame_reverse(renderer, game->gameCharacter.walkBackTexture,
                                              game->gameCharacter.frameIndex, game_walk_back_crop,
                                              game->gameCharacter.position);
            break;
        case GAME_MOVE_WALK:
            if (game->gameCharacter.walkTexture)
                game_draw_sheet_frame(renderer, game->gameCharacter.walkTexture,
                                      game->gameCharacter.frameIndex, game_walk_crop,
                                      game->gameCharacter.position);
            break;
        case GAME_MOVE_STOP:
        default:
            {
                SDL_Texture *idle_tex = (game->gameCharacter.facing < 0 && game->gameCharacter.idleBackTexture)
                    ? game->gameCharacter.idleBackTexture
                    : game->gameCharacter.idleTexture;
                if (idle_tex) {
                    if (game->gameCharacter.facing < 0 && game->gameCharacter.idleBackTexture) {
                        game_draw_sheet_full_cell_reverse(renderer, idle_tex,
                                                          game->gameCharacter.frameIndex,
                                                          game->gameCharacter.position);
                    } else {
                        game_draw_sheet_full_cell(renderer, idle_tex,
                                                  game->gameCharacter.frameIndex,
                                                  game->gameCharacter.position);
                    }
                } else if (game->gameCharacter.walkTexture) {
                    game_draw_sheet_frame(renderer, game->gameCharacter.walkTexture,
                                          0, game_walk_crop, game->gameCharacter.position);
                }
            }
            break;
    }

    SDL_SetRenderDrawColor(renderer, 10, 10, 10, 140);
    SDL_RenderFillRect(renderer, &left_card);
    SDL_RenderFillRect(renderer, &right_card);
    SDL_SetRenderDrawColor(renderer, 230, 230, 230, 255);
    SDL_RenderDrawRect(renderer, &left_card);
    SDL_RenderDrawRect(renderer, &right_card);

    if (game->player1Tex) {
        SDL_Rect p1 = {left_card.x + 20, left_card.y + 20, left_card.w - 40, left_card.h - 90};
        SDL_RenderCopy(renderer, game->player1Tex, NULL, &p1);
    }
    if (game->player2Tex) {
        SDL_Rect p2 = {right_card.x + 20, right_card.y + 20, right_card.w - 40, right_card.h - 90};
        SDL_RenderCopy(renderer, game->player2Tex, NULL, &p2);
    }

    if (game->font) {
        game_draw_center_text(renderer, game->font, game->player1_name, white,
                              (SDL_Rect){left_card.x, left_card.y + left_card.h - 58, left_card.w, 40});
        game_draw_center_text(renderer, game->font, game->player2_name, white,
                              (SDL_Rect){right_card.x, right_card.y + right_card.h - 58, right_card.w, 40});
        game_draw_center_text(renderer, game->font, "GAME STATE", white,
                              (SDL_Rect){(WIDTH - 300) / 2, 40, 300, 50});
        game_draw_center_text(renderer, game->font,
                              "LEFT/RIGHT walk | SHIFT run | SPACE jump", soft, hint_rect);
    }
}
