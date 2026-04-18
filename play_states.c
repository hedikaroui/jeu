#include "game.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static StartPlayAnimation startPlayAnim = {.dance_loop_channel = -1};
static StartPlayMover startPlayMover = {0};
static Uint32 startPlayIntroStart = 0;
static SDL_Rect startPlayPlayerRect = {0, 0, 120, 120};
static Uint32 startPlayLastTick = 0;
static int startPlayJumping = 0;
static int startPlayPendingJump = 0;
static double startPlayJumpRelX = -50.0;
static double startPlayJumpRelY = 0.0;
static double startPlayJumpProgress = 0.0;
static int startPlayJumpBaseX = 0;
static int startPlayJumpBaseY = 0;
static int startPlayJumpDir = 0;
static Uint32 startPlayLastJumpDebugTick = 0;

#define HARRY_SHEET_ROWS 5
#define HARRY_SHEET_COLS 5
#define STARTPLAY_JUMP_DURATION_MS 620.0
#define STARTPLAY_IDLE_NONE 0
#define STARTPLAY_IDLE_DANCE 1
#define STARTPLAY_IDLE_WAVE 2
#define STARTPLAY_IDLE_TRIGGER_MS 20000
#define STARTPLAY_HOLD_MAX_MS 1200
#define STARTPLAY_HOLD_FACTOR_DIV 2600.0
#define STARTPLAY_HOLD_FACTOR_MAX 1.35
#define STARTPLAY_BASE_ACCEL 420.0
#define STARTPLAY_FRICTION 0.78
#define STARTPLAY_STOP_EPSILON 3.0
#define STARTPLAY_BASE_MAX_SPEED 95.0
#define STARTPLAY_MAX_SPEED_BONUS 45.0
#define STARTPLAY_BASE_Y_SPEED 4
#define STARTPLAY_Y_SPEED_BONUS 5.0
#define STARTPLAY_WALK_FRAME_BASE_MS 140
#define STARTPLAY_WALK_FRAME_BONUS_DIV 28
#define STARTPLAY_WALK_FRAME_BONUS_CAP 95
#define STARTPLAY_WALK_FRAME_MIN_MS 40

static const char *startplay_wave_msgs[] = {
    "HEEY ,YOU ",
    "YES I M TALKING TO YOU ",
    "ARE YOU PLAYING OR WHAT ? "
};

static int ps_point_in_rect(SDL_Rect r, int x, int y) {
    return x >= r.x && x <= r.x + r.w && y >= r.y && y <= r.y + r.h;
}

static void start_play_reload_texture(SDL_Renderer *renderer, SDL_Texture **dst, const char *path) {
    if (!dst) return;
    if (*dst) {
        SDL_DestroyTexture(*dst);
        *dst = NULL;
    }
    if (renderer && path && *path) {
        *dst = IMG_LoadTexture(renderer, path);
    }
}

static void start_play_setup_sheet(SDL_Texture *tex, int wanted_rows, int wanted_cols,
                                   int *rows, int *cols, int *frame_w, int *frame_h) {
    int w = 0, h = 0;
    if (!rows || !cols || !frame_w || !frame_h) return;

    *rows = 1;
    *cols = 1;
    *frame_w = startPlayPlayerRect.w;
    *frame_h = startPlayPlayerRect.h;

    if (!tex) return;
    if (SDL_QueryTexture(tex, NULL, NULL, &w, &h) != 0 || w <= 0 || h <= 0) return;

    int r = wanted_rows;
    int c = wanted_cols;
    if (r < 1) r = 1;
    if (c < 1) c = 1;

    if ((w % c) != 0 || (h % r) != 0) {
        r = 1;
        c = (h > 0) ? (w / h) : 1;
        if (c < 1) c = 1;
    }

    *rows = r;
    *cols = c;
    *frame_w = w / c;
    *frame_h = h / r;
}

static void start_play_reset_mover(void) {
    Uint32 now = SDL_GetTicks();
    startPlayMover.x = (WIDTH - 120) / 2.0;
    startPlayMover.y = (HEIGHT - 120) / 2.0 + 40.0;
    startPlayMover.vitesse = 0.0;
    startPlayMover.acceleration = 0.0;
    startPlayMover.position_acc = (SDL_Rect){(int)startPlayMover.x, (int)startPlayMover.y, 120, 120};
    startPlayPlayerRect = startPlayMover.position_acc;
    startPlayAnim.frame_index = 0;
    startPlayAnim.facing = 1;
    startPlayAnim.moving = 0;
    startPlayAnim.idle_state = STARTPLAY_IDLE_NONE;
    startPlayAnim.idle_msg_index = 0;
    startPlayAnim.move_hold_ms = 0;
    startPlayAnim.last_input_tick = now;
    startPlayAnim.idle_msg_tick = now;
    startPlayAnim.last_anim_tick = now;
    startPlayJumping = 0;
    startPlayPendingJump = 0;
    startPlayJumpRelX = -50.0;
    startPlayJumpRelY = 0.0;
    startPlayJumpProgress = 0.0;
    startPlayJumpBaseX = startPlayPlayerRect.x;
    startPlayJumpBaseY = startPlayPlayerRect.y;
    startPlayJumpDir = 0;
    startPlayLastJumpDebugTick = 0;
}

static void start_play_begin_jump(int dir, Uint32 now) {
    if (startPlayJumping) return;
    startPlayJumping = 1;
    startPlayPendingJump = 0;
    startPlayJumpRelX = -50.0;
    startPlayJumpRelY = 0.0;
    startPlayJumpProgress = 0.0;
    startPlayJumpBaseX = startPlayPlayerRect.x;
    startPlayJumpBaseY = startPlayPlayerRect.y;
    startPlayJumpDir = dir;
    if (dir != 0) startPlayAnim.facing = dir;
    startPlayAnim.idle_state = STARTPLAY_IDLE_NONE;
    startPlayAnim.moving = 0;
    startPlayAnim.frame_index = 0;
    startPlayAnim.last_anim_tick = now;
    printf("[START_PLAY JUMP] start base=(%d,%d) dir=%d facing=%d\n",
           startPlayJumpBaseX, startPlayJumpBaseY, startPlayJumpDir, startPlayAnim.facing);
    fflush(stdout);
}

static void start_play_update_jump(Uint32 dt_ms, Uint32 now) {
    double t;
    double smooth_t;
    int dir;

    if (!startPlayJumping) return;

    startPlayJumpProgress += (double)dt_ms / STARTPLAY_JUMP_DURATION_MS;
    if (startPlayJumpProgress > 1.0) startPlayJumpProgress = 1.0;
    t = startPlayJumpProgress;
    smooth_t = t * t * (3.0 - 2.0 * t);
    startPlayJumpRelX = -50.0 + (100.0 * smooth_t);
    startPlayJumpRelY = (-0.04 * (startPlayJumpRelX * startPlayJumpRelX)) + 100.0;

    dir = startPlayJumpDir;
    startPlayPlayerRect.x = startPlayJumpBaseX + (int)lround((startPlayJumpRelX + 50.0) * dir);
    startPlayPlayerRect.y = startPlayJumpBaseY - (int)lround(startPlayJumpRelY);

    startPlayMover.x = (double)startPlayPlayerRect.x;
    startPlayMover.y = (double)startPlayPlayerRect.y;
    startPlayMover.position_acc = startPlayPlayerRect;

    if (now - startPlayLastJumpDebugTick >= 60u) {
        printf("[START_PLAY JUMP] relX=%.2f relY=%.2f pos=(%d,%d)\n",
               startPlayJumpRelX, startPlayJumpRelY, startPlayPlayerRect.x, startPlayPlayerRect.y);
        fflush(stdout);
        startPlayLastJumpDebugTick = now;
    }

    if (startPlayJumpProgress >= 1.0) {
        startPlayJumping = 0;
        startPlayJumpRelX = -50.0;
        startPlayJumpRelY = 0.0;
        startPlayJumpProgress = 0.0;
        startPlayJumpDir = 0;
        startPlayPlayerRect.y = startPlayJumpBaseY;
        startPlayMover.y = (double)startPlayJumpBaseY;
        startPlayMover.position_acc = startPlayPlayerRect;
        printf("[START_PLAY JUMP] end pos=(%d,%d)\n", startPlayPlayerRect.x, startPlayPlayerRect.y);
        fflush(stdout);
    }
}

static void start_play_move_mover(Uint32 dt_ms) {
    double dt = (double)dt_ms / 1000.0;
    double dx = 0.5 * startPlayMover.acceleration * dt * dt + startPlayMover.vitesse * dt;
    startPlayMover.x += dx;
    startPlayMover.vitesse += startPlayMover.acceleration * dt;
    startPlayMover.position_acc.x = (int)lround(startPlayMover.x);
    startPlayMover.position_acc.y = (int)lround(startPlayMover.y);
    startPlayPlayerRect = startPlayMover.position_acc;
}

void box_message(SDL_Renderer *renderer, TTF_Font *font, const char *message, SDL_Rect box) {
    if (!renderer || box.w <= 0 || box.h <= 0) return;

    Uint8 old_r, old_g, old_b, old_a;
    SDL_BlendMode old_blend = SDL_BLENDMODE_NONE;
    SDL_GetRenderDrawColor(renderer, &old_r, &old_g, &old_b, &old_a);
    SDL_GetRenderDrawBlendMode(renderer, &old_blend);

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
    SDL_RenderFillRect(renderer, &box);
    SDL_SetRenderDrawColor(renderer, 245, 245, 245, 255);
    SDL_RenderDrawRect(renderer, &box);

    if (font && message && *message) {
        SDL_Color color = {255, 255, 255, 255};
        int wrap_w = box.w - 20;
        if (wrap_w < 10) wrap_w = box.w;

        SDL_Surface *surf = TTF_RenderUTF8_Blended_Wrapped(font, message, color, (Uint32)wrap_w);
        if (surf) {
            SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, surf);
            if (tex) {
                SDL_Rect dst = {
                    box.x + (box.w - surf->w) / 2,
                    box.y + (box.h - surf->h) / 2,
                    surf->w,
                    surf->h
                };
                SDL_RenderCopy(renderer, tex, NULL, &dst);
                SDL_DestroyTexture(tex);
            }
            SDL_FreeSurface(surf);
        }
    }

    SDL_SetRenderDrawColor(renderer, old_r, old_g, old_b, old_a);
    SDL_SetRenderDrawBlendMode(renderer, old_blend);
}

int StartPlay_Charger(Game *game, SDL_Renderer *renderer) {
    static int startplay_rng_seeded = 0;
    if (game->startPlayLoaded) return 1;

    if (startPlayAnim.dance_loop_channel >= 0) {
        Mix_HaltChannel(startPlayAnim.dance_loop_channel);
        startPlayAnim.dance_loop_channel = -1;
    }

    if (!game->startPlayer1LifeTex)
        game->startPlayer1LifeTex = IMG_LoadTexture(renderer, "characters/first_player_icon_life.png");
    if (!game->startPlayer2LifeTex)
        game->startPlayer2LifeTex = IMG_LoadTexture(renderer, "characters/second_player_icon_life.png");

    if (!game->startTextTex) {
        TTF_Font *f = TTF_OpenFont(GAME_ASSETS.fonts.hello, 68);
        if (!f) f = TTF_OpenFont(GAME_ASSETS.fonts.system_bold, 68);
        if (!f) f = TTF_OpenFont(GAME_ASSETS.fonts.system_regular, 68);
        if (f) {
            SDL_Color white = {255, 255, 255, 255};
            SDL_Surface *surf = TTF_RenderUTF8_Blended(f, "the game start,go...", white);
            if (surf) {
                game->startTextTex = SDL_CreateTextureFromSurface(renderer, surf);
                game->startTextRect = (SDL_Rect){
                    (WIDTH - surf->w) / 2,
                    (HEIGHT - surf->h) / 2,
                    surf->w,
                    surf->h
                };
                SDL_FreeSurface(surf);
            }
            TTF_CloseFont(f);
        }
    }

    if (!game->player1Tex)
        game->player1Tex = IMG_LoadTexture(renderer, GAME_ASSETS.characters.player1);

    start_play_reload_texture(renderer, &startPlayAnim.walk_tex,
                              "spritesheet_characters/mr_harry_walk_cycle_transparent.png");
    start_play_reload_texture(renderer, &startPlayAnim.walk_back_tex,
                              "spritesheet_characters/mr_harry_walk_cycle_back_transparent.png");
    start_play_reload_texture(renderer, &startPlayAnim.jump_tex,
                              "spritesheet_characters/mr_harry_jump_transparent.png");
    start_play_reload_texture(renderer, &startPlayAnim.jump_back_tex,
                              "spritesheet_characters/mr_harry_jump_back_transparent.png");
    start_play_reload_texture(renderer, &startPlayAnim.stop_tex,
                              "spritesheet_characters/mr_harry_stand_up.png");
    start_play_reload_texture(renderer, &startPlayAnim.stop_back_tex,
                              "spritesheet_characters/mr_harry_stand_up_back.png");
    start_play_reload_texture(renderer, &startPlayAnim.coin_tex,
                              "buttons/coin.png");
    start_play_reload_texture(renderer, &startPlayAnim.dance_tex,
                              "spritesheet_characters/mr_harry_dance_animation_transparent.png");
    start_play_reload_texture(renderer, &startPlayAnim.wave_tex,
                              "spritesheet_characters/mr_harry_wave_animation_transparent.png");
    if (!startPlayAnim.dance_loop_sfx) {
        startPlayAnim.dance_loop_sfx = Mix_LoadWAV("songs/sahara-dans-song.wav");
        if (!startPlayAnim.dance_loop_sfx)
            startPlayAnim.dance_loop_sfx = Mix_LoadWAV("songs/sahara-dance-song.wav");
    }

    if (!startplay_rng_seeded) {
        srand((unsigned int)time(NULL));
        startplay_rng_seeded = 1;
    }
    startPlayAnim.dance_loop_channel = -1;

    start_play_setup_sheet(startPlayAnim.walk_tex, HARRY_SHEET_ROWS, HARRY_SHEET_COLS,
                           &startPlayAnim.walk_rows, &startPlayAnim.walk_cols,
                           &startPlayAnim.walk_frame_w, &startPlayAnim.walk_frame_h);
    start_play_setup_sheet(startPlayAnim.walk_back_tex, HARRY_SHEET_ROWS, HARRY_SHEET_COLS,
                           &startPlayAnim.walk_back_rows, &startPlayAnim.walk_back_cols,
                           &startPlayAnim.walk_back_frame_w, &startPlayAnim.walk_back_frame_h);
    start_play_setup_sheet(startPlayAnim.jump_tex, HARRY_SHEET_ROWS, HARRY_SHEET_COLS,
                           &startPlayAnim.jump_rows, &startPlayAnim.jump_cols,
                           &startPlayAnim.jump_frame_w, &startPlayAnim.jump_frame_h);
    start_play_setup_sheet(startPlayAnim.jump_back_tex, HARRY_SHEET_ROWS, HARRY_SHEET_COLS,
                           &startPlayAnim.jump_back_rows, &startPlayAnim.jump_back_cols,
                           &startPlayAnim.jump_back_frame_w, &startPlayAnim.jump_back_frame_h);
    start_play_setup_sheet(startPlayAnim.stop_tex, HARRY_SHEET_ROWS, HARRY_SHEET_COLS,
                           &startPlayAnim.stop_rows, &startPlayAnim.stop_cols,
                           &startPlayAnim.stop_frame_w, &startPlayAnim.stop_frame_h);
    start_play_setup_sheet(startPlayAnim.stop_back_tex, HARRY_SHEET_ROWS, HARRY_SHEET_COLS,
                           &startPlayAnim.stop_back_rows, &startPlayAnim.stop_back_cols,
                           &startPlayAnim.stop_back_frame_w, &startPlayAnim.stop_back_frame_h);
    start_play_setup_sheet(startPlayAnim.dance_tex, HARRY_SHEET_ROWS, HARRY_SHEET_COLS,
                           &startPlayAnim.dance_rows, &startPlayAnim.dance_cols,
                           &startPlayAnim.dance_frame_w, &startPlayAnim.dance_frame_h);
    start_play_setup_sheet(startPlayAnim.wave_tex, HARRY_SHEET_ROWS, HARRY_SHEET_COLS,
                           &startPlayAnim.wave_rows, &startPlayAnim.wave_cols,
                           &startPlayAnim.wave_frame_w, &startPlayAnim.wave_frame_h);

    startPlayAnim.frame_index = 0;
    startPlayAnim.facing = 1;
    startPlayAnim.moving = 0;
    startPlayAnim.idle_state = STARTPLAY_IDLE_NONE;
    startPlayAnim.idle_msg_index = 0;
    startPlayAnim.move_hold_ms = 0;
    startPlayAnim.last_input_tick = SDL_GetTicks();
    startPlayAnim.idle_msg_tick = startPlayAnim.last_input_tick;
    startPlayAnim.last_anim_tick = SDL_GetTicks();

    startPlayIntroStart = SDL_GetTicks();
    startPlayLastTick = startPlayIntroStart;
    start_play_reset_mover();

    game->startPlayLoaded = 1;
    return 1;
}

void StartPlay_LectureEntree(Game *game) {
    SDL_Event e;
    if (startPlayIntroStart == 0) {
        startPlayIntroStart = SDL_GetTicks();
        startPlayLastTick = startPlayIntroStart;
        start_play_reset_mover();
    }

    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            game->running = 0;
            return;
        }
        if (e.type == SDL_KEYDOWN) {
            if (e.key.keysym.sym == SDLK_ESCAPE) {
                if (startPlayAnim.dance_loop_channel >= 0) {
                    Mix_HaltChannel(startPlayAnim.dance_loop_channel);
                    startPlayAnim.dance_loop_channel = -1;
                }
                startPlayAnim.idle_state = STARTPLAY_IDLE_NONE;
                game->startPlayLoaded = 0;
                Game_SetSubState(game, STATE_MENU);
                if (game->music) Mix_PlayMusic(game->music, -1);
                startPlayIntroStart = 0;
                startPlayLastTick = 0;
                return;
            }
            if (e.key.keysym.scancode == SDL_SCANCODE_SPACE ||
                e.key.keysym.scancode == SDL_SCANCODE_UP ||
                e.key.keysym.scancode == SDL_SCANCODE_W) {
                startPlayPendingJump = 1;
                printf("[START_PLAY INPUT] jump key pressed: %s\n", SDL_GetKeyName(e.key.keysym.sym));
                fflush(stdout);
            }
        }
    }

    if (SDL_GetTicks() - startPlayIntroStart < 2000) return;

    Uint32 now = SDL_GetTicks();
    Uint32 dt = (startPlayLastTick == 0) ? 16u : (now - startPlayLastTick);
    startPlayLastTick = now;

    const Uint8 *keys = SDL_GetKeyboardState(NULL);
    int left_pressed = keys[SDL_SCANCODE_LEFT] || keys[SDL_SCANCODE_A];
    int right_pressed = keys[SDL_SCANCODE_RIGHT] || keys[SDL_SCANCODE_D];
    int jump_pressed = startPlayPendingJump ||
                       keys[SDL_SCANCODE_SPACE] ||
                       keys[SDL_SCANCODE_UP] ||
                       keys[SDL_SCANCODE_W];
    int has_move_input = left_pressed || right_pressed;

    if (jump_pressed && !startPlayJumping) {
        int jump_dir = 0;
        if (left_pressed && !right_pressed) jump_dir = -1;
        if (right_pressed && !left_pressed) jump_dir = 1;
        start_play_begin_jump(jump_dir, now);
    }
    startPlayPendingJump = 0;

    if (!startPlayJumping && has_move_input) {
        int was_idle = (startPlayAnim.idle_state != STARTPLAY_IDLE_NONE);
        if (startPlayAnim.move_hold_ms < STARTPLAY_HOLD_MAX_MS) {
            startPlayAnim.move_hold_ms += dt;
            if (startPlayAnim.move_hold_ms > STARTPLAY_HOLD_MAX_MS) startPlayAnim.move_hold_ms = STARTPLAY_HOLD_MAX_MS;
        }
        startPlayAnim.last_input_tick = now;
        if (startPlayAnim.idle_state == STARTPLAY_IDLE_DANCE && startPlayAnim.dance_loop_channel >= 0) {
            Mix_HaltChannel(startPlayAnim.dance_loop_channel);
            startPlayAnim.dance_loop_channel = -1;
        }
        startPlayAnim.idle_state = STARTPLAY_IDLE_NONE;
        if (was_idle) startPlayAnim.frame_index = 0;
    } else {
        startPlayAnim.move_hold_ms = 0;
    }

    double hold_factor = 1.0 + ((double)startPlayAnim.move_hold_ms / STARTPLAY_HOLD_FACTOR_DIV);
    if (hold_factor > STARTPLAY_HOLD_FACTOR_MAX) hold_factor = STARTPLAY_HOLD_FACTOR_MAX;
    double accel = STARTPLAY_BASE_ACCEL * hold_factor;

    if (startPlayJumping) {
        startPlayMover.acceleration = 0.0;
        startPlayMover.vitesse = 0.0;
    } else if (left_pressed && !right_pressed) {
        startPlayMover.acceleration = -accel;
        startPlayAnim.facing = -1;
    } else if (right_pressed && !left_pressed) {
        startPlayMover.acceleration = accel;
        startPlayAnim.facing = 1;
    } else {
        startPlayMover.acceleration = 0.0;
        startPlayMover.vitesse *= STARTPLAY_FRICTION;
        if (fabs(startPlayMover.vitesse) < STARTPLAY_STOP_EPSILON) startPlayMover.vitesse = 0.0;
    }

    if (!startPlayJumping) start_play_move_mover(dt);

    {
        double max_speed = STARTPLAY_BASE_MAX_SPEED + hold_factor * STARTPLAY_MAX_SPEED_BONUS;
        if (startPlayMover.vitesse > max_speed) startPlayMover.vitesse = max_speed;
        if (startPlayMover.vitesse < -max_speed) startPlayMover.vitesse = -max_speed;
    }

    if (startPlayJumping) {
        start_play_update_jump(dt, now);
    } else {
        /* Keep the character on a fixed horizontal line when not jumping. */
        startPlayMover.y = (HEIGHT - 120) / 2.0 + 40.0;
        startPlayMover.position_acc.y = (int)lround(startPlayMover.y);
        startPlayPlayerRect.y = startPlayMover.position_acc.y;
    }

    startPlayAnim.moving = !startPlayJumping && (has_move_input || fabs(startPlayMover.vitesse) > 8.0);
    if (startPlayMover.vitesse < -8.0) startPlayAnim.facing = -1;
    if (startPlayMover.vitesse > 8.0) startPlayAnim.facing = 1;

    if (!startPlayJumping && !has_move_input && fabs(startPlayMover.vitesse) < 4.0 &&
        now - startPlayAnim.last_input_tick >= STARTPLAY_IDLE_TRIGGER_MS &&
        startPlayAnim.idle_state == STARTPLAY_IDLE_NONE) {
        startPlayAnim.idle_state = (rand() % 2 == 0) ? STARTPLAY_IDLE_DANCE : STARTPLAY_IDLE_WAVE;
        startPlayAnim.frame_index = 0;
        startPlayAnim.idle_msg_index = 0;
        startPlayAnim.idle_msg_tick = now;
        startPlayAnim.last_anim_tick = now;
        if (startPlayAnim.idle_state == STARTPLAY_IDLE_DANCE &&
            startPlayAnim.dance_loop_sfx && startPlayAnim.dance_loop_channel < 0) {
            startPlayAnim.dance_loop_channel = Mix_PlayChannel(-1, startPlayAnim.dance_loop_sfx, -1);
        }
    }

    if (startPlayAnim.idle_state == STARTPLAY_IDLE_WAVE &&
        now - startPlayAnim.idle_msg_tick >= 2200) {
        startPlayAnim.idle_msg_index = (startPlayAnim.idle_msg_index + 1) % 3;
        startPlayAnim.idle_msg_tick = now;
    }

    if (startPlayAnim.idle_state != STARTPLAY_IDLE_NONE) {
        int rows = (startPlayAnim.idle_state == STARTPLAY_IDLE_DANCE) ? startPlayAnim.dance_rows : startPlayAnim.wave_rows;
        int cols = (startPlayAnim.idle_state == STARTPLAY_IDLE_DANCE) ? startPlayAnim.dance_cols : startPlayAnim.wave_cols;
        int frames = rows * cols;
        Uint32 frame_ms = (startPlayAnim.idle_state == STARTPLAY_IDLE_DANCE) ? 80 : 120;
        if (frames < 1) frames = 1;
        if (now - startPlayAnim.last_anim_tick >= frame_ms) {
            startPlayAnim.frame_index = (startPlayAnim.frame_index + 1) % frames;
            startPlayAnim.last_anim_tick = now;
        }
        return;
    }

    if (startPlayJumping) {
        int rows = (startPlayAnim.facing < 0) ? startPlayAnim.jump_back_rows : startPlayAnim.jump_rows;
        int cols = (startPlayAnim.facing < 0) ? startPlayAnim.jump_back_cols : startPlayAnim.jump_cols;
        int frames = rows * cols;
        if (frames < 1) frames = 1;
        if (now - startPlayAnim.last_anim_tick >= 90u) {
            startPlayAnim.frame_index = (startPlayAnim.frame_index + 1) % frames;
            startPlayAnim.last_anim_tick = now;
        }
    } else if (startPlayAnim.moving) {
        int rows = (startPlayAnim.facing < 0) ? startPlayAnim.walk_back_rows : startPlayAnim.walk_rows;
        int cols = (startPlayAnim.facing < 0) ? startPlayAnim.walk_back_cols : startPlayAnim.walk_cols;
        int frames = rows * cols;
        Uint32 speed_bonus = startPlayAnim.move_hold_ms / STARTPLAY_WALK_FRAME_BONUS_DIV;
        Uint32 frame_ms = STARTPLAY_WALK_FRAME_BASE_MS;
        if (speed_bonus > STARTPLAY_WALK_FRAME_BONUS_CAP) speed_bonus = STARTPLAY_WALK_FRAME_BONUS_CAP;
        frame_ms = (frame_ms > speed_bonus) ? (frame_ms - speed_bonus) : STARTPLAY_WALK_FRAME_MIN_MS;
        if (frame_ms < STARTPLAY_WALK_FRAME_MIN_MS) frame_ms = STARTPLAY_WALK_FRAME_MIN_MS;
        if (frames < 1) frames = 1;
        if (now - startPlayAnim.last_anim_tick >= frame_ms) {
            startPlayAnim.frame_index = (startPlayAnim.frame_index + 1) % frames;
            startPlayAnim.last_anim_tick = now;
        }
    } else {
        int rows = (startPlayAnim.facing < 0 && startPlayAnim.stop_back_tex)
            ? startPlayAnim.stop_back_rows
            : startPlayAnim.stop_rows;
        int cols = (startPlayAnim.facing < 0 && startPlayAnim.stop_back_tex)
            ? startPlayAnim.stop_back_cols
            : startPlayAnim.stop_cols;
        int frames = rows * cols;
        if (frames < 1) frames = 1;
        if (now - startPlayAnim.last_anim_tick >= 140u) {
            startPlayAnim.frame_index = (startPlayAnim.frame_index + 1) % frames;
            startPlayAnim.last_anim_tick = now;
        }
    }
}

void StartPlay_Affichage(Game *game, SDL_Renderer *renderer) {
    Uint32 elapsed = (startPlayIntroStart == 0) ? 0 : (SDL_GetTicks() - startPlayIntroStart);
    if (elapsed < 2000) {
        if (game->startTextTex)
            SDL_RenderCopy(renderer, game->startTextTex, NULL, &game->startTextRect);
    } else {
        if (game->gameBgTex) SDL_RenderCopy(renderer, game->gameBgTex, NULL, NULL);
        SDL_Texture *active_sheet = NULL;
        int rows = 1, cols = 1;
        int frame_w = startPlayPlayerRect.w, frame_h = startPlayPlayerRect.h;
        int reverse_sheet = 0;

        if (startPlayJumping) {
            active_sheet = (startPlayAnim.facing < 0) ? startPlayAnim.jump_back_tex : startPlayAnim.jump_tex;
            rows = (startPlayAnim.facing < 0) ? startPlayAnim.jump_back_rows : startPlayAnim.jump_rows;
            cols = (startPlayAnim.facing < 0) ? startPlayAnim.jump_back_cols : startPlayAnim.jump_cols;
            frame_w = (startPlayAnim.facing < 0) ? startPlayAnim.jump_back_frame_w : startPlayAnim.jump_frame_w;
            frame_h = (startPlayAnim.facing < 0) ? startPlayAnim.jump_back_frame_h : startPlayAnim.jump_frame_h;
            reverse_sheet = (startPlayAnim.facing < 0);
        } else if (startPlayAnim.idle_state == STARTPLAY_IDLE_DANCE) {
            active_sheet = startPlayAnim.dance_tex;
            rows = startPlayAnim.dance_rows;
            cols = startPlayAnim.dance_cols;
            frame_w = startPlayAnim.dance_frame_w;
            frame_h = startPlayAnim.dance_frame_h;
        } else if (startPlayAnim.idle_state == STARTPLAY_IDLE_WAVE) {
            active_sheet = startPlayAnim.wave_tex;
            rows = startPlayAnim.wave_rows;
            cols = startPlayAnim.wave_cols;
            frame_w = startPlayAnim.wave_frame_w;
            frame_h = startPlayAnim.wave_frame_h;
        } else if (!startPlayAnim.moving && (startPlayAnim.stop_tex || startPlayAnim.stop_back_tex)) {
            if (startPlayAnim.facing < 0 && startPlayAnim.stop_back_tex) {
                active_sheet = startPlayAnim.stop_back_tex;
                rows = startPlayAnim.stop_back_rows;
                cols = startPlayAnim.stop_back_cols;
                frame_w = startPlayAnim.stop_back_frame_w;
                frame_h = startPlayAnim.stop_back_frame_h;
                reverse_sheet = 1;
            } else {
                active_sheet = startPlayAnim.stop_tex;
                rows = startPlayAnim.stop_rows;
                cols = startPlayAnim.stop_cols;
                frame_w = startPlayAnim.stop_frame_w;
                frame_h = startPlayAnim.stop_frame_h;
            }
        } else {
            active_sheet = (startPlayAnim.facing < 0) ? startPlayAnim.walk_back_tex : startPlayAnim.walk_tex;
            rows = (startPlayAnim.facing < 0) ? startPlayAnim.walk_back_rows : startPlayAnim.walk_rows;
            cols = (startPlayAnim.facing < 0) ? startPlayAnim.walk_back_cols : startPlayAnim.walk_cols;
            frame_w = (startPlayAnim.facing < 0) ? startPlayAnim.walk_back_frame_w : startPlayAnim.walk_frame_w;
            frame_h = (startPlayAnim.facing < 0) ? startPlayAnim.walk_back_frame_h : startPlayAnim.walk_frame_h;
            reverse_sheet = (startPlayAnim.facing < 0);
        }

        if (active_sheet && rows > 0 && cols > 0 && frame_w > 0 && frame_h > 0) {
            int frame_count = rows * cols;
            int draw_frame = startPlayAnim.frame_index % frame_count;
            if (reverse_sheet) draw_frame = frame_count - 1 - draw_frame;
            int frame_col = draw_frame % cols;
            int frame_row = draw_frame / cols;
            SDL_Rect src = {
                frame_col * frame_w,
                frame_row * frame_h,
                frame_w,
                frame_h
            };
            SDL_RenderCopy(renderer, active_sheet, &src, &startPlayPlayerRect);
        } else if (game->player1Tex) {
            SDL_RenderCopy(renderer, game->player1Tex, NULL, &startPlayPlayerRect);
        }

        if (startPlayAnim.idle_state == STARTPLAY_IDLE_WAVE) {
            const char *msg = startplay_wave_msgs[startPlayAnim.idle_msg_index];
            SDL_Rect msg_box = {(WIDTH - 650) / 2, 50, 650, 110};
            box_message(renderer, game->font, msg, msg_box);
        }
    }

    {
        int icon_w = 100;
        int icon_h = 100;
        int x0 = 16;
        int y0 = 16;
        int icon_gap = 10;
        int bottom_icons_y = y0;

        if (game->startPlayer1LifeTex) {
            SDL_Rect p1_life = {x0, y0, icon_w, icon_h};
            SDL_RenderCopy(renderer, game->startPlayer1LifeTex, NULL, &p1_life);
            bottom_icons_y = p1_life.y + p1_life.h;
        }

        if (game->startPlayer2LifeTex) {
            SDL_Rect p2_life = {x0, bottom_icons_y + icon_gap, icon_w, icon_h};
            SDL_RenderCopy(renderer, game->startPlayer2LifeTex, NULL, &p2_life);
            bottom_icons_y = p2_life.y + p2_life.h;
        }

        if (startPlayAnim.coin_tex) {
            int coin_w = 42;
            int coin_h = 42;
            int space_from_icons = 14;
            int left_border_space = 18;
            SDL_Rect coin_base = {
                left_border_space,
                bottom_icons_y + space_from_icons,
                coin_w,
                coin_h
            };

            int mx, my;
            SDL_GetMouseState(&mx, &my);
            int coin_hover = ps_point_in_rect(coin_base, mx, my);
            int bob = coin_hover ? (int)lround(3.0 * sin((double)SDL_GetTicks() / 120.0)) : 0;
            SDL_Rect coin_draw = coin_base;
            coin_draw.y += bob;

            SDL_RenderCopy(renderer, startPlayAnim.coin_tex, NULL, &coin_draw);
        }
    }
}

void StartPlay_MiseAJour(Game *game) {
    (void)game;
    if (startPlayPlayerRect.x < 0) startPlayPlayerRect.x = 0;
    if (startPlayPlayerRect.y < 0) startPlayPlayerRect.y = 0;
    if (startPlayPlayerRect.x + startPlayPlayerRect.w > WIDTH)
        startPlayPlayerRect.x = WIDTH - startPlayPlayerRect.w;
    if (startPlayPlayerRect.y + startPlayPlayerRect.h > HEIGHT)
        startPlayPlayerRect.y = HEIGHT - startPlayPlayerRect.h;

    startPlayMover.x = (double)startPlayPlayerRect.x;
    startPlayMover.y = (double)startPlayPlayerRect.y;
    startPlayMover.position_acc = startPlayPlayerRect;
    SDL_Delay(16);
}

void StartPlay_Cleanup(void) {
    if (startPlayAnim.walk_tex) SDL_DestroyTexture(startPlayAnim.walk_tex);
    if (startPlayAnim.walk_back_tex) SDL_DestroyTexture(startPlayAnim.walk_back_tex);
    if (startPlayAnim.jump_tex) SDL_DestroyTexture(startPlayAnim.jump_tex);
    if (startPlayAnim.jump_back_tex) SDL_DestroyTexture(startPlayAnim.jump_back_tex);
    if (startPlayAnim.stop_tex) SDL_DestroyTexture(startPlayAnim.stop_tex);
    if (startPlayAnim.stop_back_tex) SDL_DestroyTexture(startPlayAnim.stop_back_tex);
    if (startPlayAnim.coin_tex) SDL_DestroyTexture(startPlayAnim.coin_tex);
    if (startPlayAnim.dance_tex) SDL_DestroyTexture(startPlayAnim.dance_tex);
    if (startPlayAnim.wave_tex) SDL_DestroyTexture(startPlayAnim.wave_tex);
    if (startPlayAnim.dance_loop_channel >= 0) Mix_HaltChannel(startPlayAnim.dance_loop_channel);
    if (startPlayAnim.dance_loop_sfx) Mix_FreeChunk(startPlayAnim.dance_loop_sfx);
    memset(&startPlayAnim, 0, sizeof(startPlayAnim));
    startPlayAnim.dance_loop_channel = -1;
    startPlayIntroStart = 0;
    startPlayLastTick = 0;
}

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

static SDL_Rect quizBtnARect = {120, 430, 150, 120};
static SDL_Rect quizBtnBRect = {325, 430, 150, 120};
static SDL_Rect quizBtnCRect = {530, 430, 150, 120};
static int quizHoverA = 0, quizHoverB = 0, quizHoverC = 0;
static int quizSelected = -1;

static int point_in_rect(SDL_Rect r, int x, int y) {
    return x >= r.x && x <= r.x + r.w && y >= r.y && y <= r.y + r.h;
}

static void draw_center_text(SDL_Renderer *renderer, TTF_Font *font, const char *text,
                             SDL_Color color, SDL_Rect box) {
    if (!font || !text || !*text) return;
    SDL_Surface *surf = TTF_RenderUTF8_Blended(font, text, color);
    if (!surf) return;
    SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, surf);
    if (tex) {
        SDL_Rect dst = {
            box.x + (box.w - surf->w) / 2,
            box.y + (box.h - surf->h) / 2,
            surf->w,
            surf->h
        };
        SDL_RenderCopy(renderer, tex, NULL, &dst);
        SDL_DestroyTexture(tex);
    }
    SDL_FreeSurface(surf);
}

int Games_Charger(Game *game, SDL_Renderer *renderer) {
    if (game->gamesLoaded) return 1;

    game->quizBg1 = IMG_LoadTexture(renderer, GAME_ASSETS.backgrounds.quiz_1);
    game->quizBg2 = IMG_LoadTexture(renderer, GAME_ASSETS.backgrounds.quiz_2);
    game->quizBtnA = IMG_LoadTexture(renderer, QUIZ_BUTTON_A);
    game->quizBtnB = IMG_LoadTexture(renderer, QUIZ_BUTTON_B);
    game->quizBtnC = IMG_LoadTexture(renderer, QUIZ_BUTTON_C);
    game->quizMusic = Mix_LoadMUS(GAME_ASSETS.songs.quiz_music);
    game->quizBeep = Mix_LoadWAV(SOUND_QUIZ_BEEP_1);
    game->quizBeep2 = Mix_LoadWAV(SOUND_QUIZ_BEEP_2);
    game->quizLaugh = Mix_LoadWAV(SOUND_QUIZ_LAUGH);
    game->quizFont = TTF_OpenFont(GAME_ASSETS.fonts.quiz_primary, 26);
    if (!game->quizFont) game->quizFont = TTF_OpenFont(GAME_ASSETS.fonts.quiz_fallback, 26);

    quizSelected = -1;
    quizHoverA = quizHoverB = quizHoverC = 0;

    if (game->quizMusic) {
        Mix_VolumeMusic(40);
        Mix_PlayMusic(game->quizMusic, -1);
    }

    game->gamesLoaded = 1;
    printf("Enigme/Quiz assets charges\n");
    return 1;
}

void Games_LectureEntree(Game *game) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            game->running = 0;
            return;
        }

        if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
            Game_SetSubState(game, STATE_MENU);
            return;
        }

        if (e.type == SDL_MOUSEMOTION && game->currentSubState == STATE_ENIGME_QUIZ) {
            int mx = e.motion.x, my = e.motion.y;
            quizHoverA = point_in_rect(quizBtnARect, mx, my);
            quizHoverB = point_in_rect(quizBtnBRect, mx, my);
            quizHoverC = point_in_rect(quizBtnCRect, mx, my);
        }

        if (e.type == SDL_MOUSEBUTTONDOWN) {
            int mx = e.button.x, my = e.button.y;
            if (game->currentSubState == STATE_ENIGME_QUIZ) {
                if (point_in_rect(quizBtnARect, mx, my)) {
                    quizSelected = 0;
                    if (game->quizBeep) Mix_PlayChannel(-1, game->quizBeep, 0);
                    if (game->quizLaugh) Mix_PlayChannel(-1, game->quizLaugh, 0);
                }
                if (point_in_rect(quizBtnBRect, mx, my)) {
                    quizSelected = 1;
                    if (game->quizBeep2) Mix_PlayChannel(-1, game->quizBeep2, 0);
                }
                if (point_in_rect(quizBtnCRect, mx, my)) {
                    quizSelected = 2;
                    if (game->quizBeep) Mix_PlayChannel(-1, game->quizBeep, 0);
                    if (game->quizLaugh) Mix_PlayChannel(-1, game->quizLaugh, 0);
                }
            } else {
                Game_SetSubState(game, STATE_ENIGME_QUIZ);
            }
        }
    }
}

void Games_Affichage(Game *game, SDL_Renderer *renderer) {
    if (game->currentSubState == STATE_ENIGME_QUIZ) {
        if (game->quizBg2) SDL_RenderCopy(renderer, game->quizBg2, NULL, NULL);
        else if (game->quizBg1) SDL_RenderCopy(renderer, game->quizBg1, NULL, NULL);

        if (game->quizFont) {
            SDL_Color black = {0, 0, 0, 255};
            SDL_Color white = {255, 255, 255, 255};
            draw_center_text(renderer, game->quizFont,
                             "QUEL OBJET KEVIN MET-IL SUR LA POIGNEE ?",
                             black, (SDL_Rect){100, 70, WIDTH - 200, 70});
            draw_center_text(renderer, game->quizFont,
                             "A: Bouilloire   B: Fer a repasser   C: Radiateur",
                             white, (SDL_Rect){50, 140, WIDTH - 100, 40});
        }

        SDL_Rect a = quizBtnARect, b = quizBtnBRect, c = quizBtnCRect;
        if (quizHoverA) a.y -= 4;
        if (quizHoverB) b.y -= 4;
        if (quizHoverC) c.y -= 4;

        if (game->quizBtnA) SDL_RenderCopy(renderer, game->quizBtnA, NULL, &a);
        if (game->quizBtnB) SDL_RenderCopy(renderer, game->quizBtnB, NULL, &b);
        if (game->quizBtnC) SDL_RenderCopy(renderer, game->quizBtnC, NULL, &c);

        if (quizSelected != -1 && game->quizFont) {
            SDL_Color col = (quizSelected == 1)
                ? (SDL_Color){30, 220, 30, 255}
                : (SDL_Color){240, 30, 30, 255};
            const char *msg = (quizSelected == 1) ? "BRAVO ! Bonne reponse." : "FAUX ! Reessayez.";
            draw_center_text(renderer, game->quizFont, msg, col, (SDL_Rect){120, 280, WIDTH - 240, 50});
        }
    } else {
        if (game->quizBg1) SDL_RenderCopy(renderer, game->quizBg1, NULL, NULL);
    }
}

void Games_MiseAJour(Game *game) {
    (void)game;
    SDL_Delay(16);
}

static const AnimationMovement kAnimationMovements[MOVEMENT_COUNT] = {
    [MOVEMENT_WALK] = {
        .name = "walk",
        .sprite_sheet = "spritesheet_characters/mr_harry_walk_cycle_transparent.png",
        .rows = 6,
        .cols = 5,
        .frame_duration_ms = 110,
        .speed_x = 2.5f,
        .speed_y = 0.0f,
        .loops = 1
    },
    [MOVEMENT_RUN] = {
        .name = "run",
        .sprite_sheet = "spritesheet_characters/mr_harry_walk_cycle_transparent.png",
        .rows = 6,
        .cols = 5,
        .frame_duration_ms = 60,
        .speed_x = 5.0f,
        .speed_y = 0.0f,
        .loops = 1
    },
    [MOVEMENT_DANCE] = {
        .name = "dance",
        .sprite_sheet = "spritesheet_characters/mr_harry_dance_animation_transparent.png",
        .rows = 6,
        .cols = 5,
        .frame_duration_ms = 80,
        .speed_x = 0.0f,
        .speed_y = 0.0f,
        .loops = 1
    },
    [MOVEMENT_JUMP] = {
        .name = "jump",
        .sprite_sheet = "spritesheet_characters/mr_harry_stops.png",
        .rows = 1,
        .cols = 1,
        .frame_duration_ms = 140,
        .speed_x = 1.0f,
        .speed_y = -8.0f,
        .loops = 0
    }
};

const AnimationMovement *animation_get_movement(MovementType type)
{
    if (type < 0 || type >= MOVEMENT_COUNT) return NULL;
    return &kAnimationMovements[type];
}

const AnimationMovement *animation_find_movement(const char *name)
{
    size_t i;

    if (!name) return NULL;

    for (i = 0; i < MOVEMENT_COUNT; ++i) {
        if (strcmp(kAnimationMovements[i].name, name) == 0) {
            return &kAnimationMovements[i];
        }
    }

    return NULL;
}

size_t animation_movement_count(void)
{
    return MOVEMENT_COUNT;
}
