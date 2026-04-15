#include "game.h"
#include "assets_catalog.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct {
    SDL_Texture *walk_tex;
    SDL_Texture *walk_back_tex;
    SDL_Texture *stop_tex;
    SDL_Texture *coin_tex;
    SDL_Texture *dance_tex;
    SDL_Texture *wave_tex;
    int walk_rows;
    int walk_cols;
    int walk_frame_w;
    int walk_frame_h;
    int walk_back_rows;
    int walk_back_cols;
    int walk_back_frame_w;
    int walk_back_frame_h;
    int dance_rows;
    int dance_cols;
    int dance_frame_w;
    int dance_frame_h;
    int wave_rows;
    int wave_cols;
    int wave_frame_w;
    int wave_frame_h;
    int frame_index;
    int facing; /* 1 right/forward, -1 left/back */
    int moving;
    int idle_state; /* 0 none, 1 dance, 2 wave */
    int idle_msg_index;
    Uint32 move_hold_ms;
    Uint32 last_input_tick;
    Uint32 idle_msg_tick;
    Uint32 last_anim_tick;
    Mix_Chunk *dance_loop_sfx;
    int dance_loop_channel;
} StartPlayAnimation;

typedef struct {
    double x, y;
    double vitesse;
    double acceleration;
    SDL_Rect position_acc;
} StartPlayMover;

static StartPlayAnimation startPlayAnim = {.dance_loop_channel = -1};
static StartPlayMover startPlayMover = {0};
static Uint32 startPlayIntroStart = 0;
static SDL_Rect startPlayPlayerRect = {0, 0, 120, 120};
static Uint32 startPlayLastTick = 0;

#define HARRY_SHEET_ROWS 5
#define HARRY_SHEET_COLS 5
#define STARTPLAY_IDLE_NONE 0
#define STARTPLAY_IDLE_DANCE 1
#define STARTPLAY_IDLE_WAVE 2
#define STARTPLAY_IDLE_TRIGGER_MS 20000
#define STARTPLAY_HOLD_MAX_MS 5000
#define STARTPLAY_HOLD_FACTOR_DIV 2000.0
#define STARTPLAY_HOLD_FACTOR_MAX 2.8
#define STARTPLAY_BASE_ACCEL 1350.0
#define STARTPLAY_FRICTION 0.90
#define STARTPLAY_STOP_EPSILON 5.0
#define STARTPLAY_BASE_MAX_SPEED 250.0
#define STARTPLAY_MAX_SPEED_BONUS 260.0
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

    if (!game->startHeartTex)
        game->startHeartTex = IMG_LoadTexture(renderer, GAME_ASSETS.backgrounds.start_heart);

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
    start_play_reload_texture(renderer, &startPlayAnim.stop_tex,
                              "spritesheet_characters/mr_harry_stops.png");
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
        if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
            if (startPlayAnim.dance_loop_channel >= 0) {
                Mix_HaltChannel(startPlayAnim.dance_loop_channel);
                startPlayAnim.dance_loop_channel = -1;
            }
            startPlayAnim.idle_state = STARTPLAY_IDLE_NONE;
            game->startPlayLoaded = 0;
            game->currentState = STATE_MENU;
            if (game->music) Mix_PlayMusic(game->music, -1);
            startPlayIntroStart = 0;
            startPlayLastTick = 0;
            return;
        }
    }

    if (SDL_GetTicks() - startPlayIntroStart < 2000) return;

    Uint32 now = SDL_GetTicks();
    Uint32 dt = (startPlayLastTick == 0) ? 16u : (now - startPlayLastTick);
    startPlayLastTick = now;

    const Uint8 *keys = SDL_GetKeyboardState(NULL);
    int left_pressed = keys[SDL_SCANCODE_LEFT] || keys[SDL_SCANCODE_A];
    int right_pressed = keys[SDL_SCANCODE_RIGHT] || keys[SDL_SCANCODE_D];
    int up_pressed = keys[SDL_SCANCODE_UP] || keys[SDL_SCANCODE_W];
    int down_pressed = keys[SDL_SCANCODE_DOWN] || keys[SDL_SCANCODE_S];
    int has_move_input = left_pressed || right_pressed || up_pressed || down_pressed;

    if (has_move_input) {
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

    if (left_pressed && !right_pressed) {
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

    start_play_move_mover(dt);

    {
        double max_speed = STARTPLAY_BASE_MAX_SPEED + hold_factor * STARTPLAY_MAX_SPEED_BONUS;
        if (startPlayMover.vitesse > max_speed) startPlayMover.vitesse = max_speed;
        if (startPlayMover.vitesse < -max_speed) startPlayMover.vitesse = -max_speed;
    }

    {
        int speedY = STARTPLAY_BASE_Y_SPEED + (int)lround(STARTPLAY_Y_SPEED_BONUS * hold_factor);
        if (up_pressed) startPlayMover.y -= speedY;
        if (down_pressed) startPlayMover.y += speedY;
    }
    startPlayMover.position_acc.y = (int)lround(startPlayMover.y);
    startPlayPlayerRect.y = startPlayMover.position_acc.y;

    startPlayAnim.moving = has_move_input || fabs(startPlayMover.vitesse) > 8.0;
    if (startPlayMover.vitesse < -8.0) startPlayAnim.facing = -1;
    if (startPlayMover.vitesse > 8.0) startPlayAnim.facing = 1;

    if (!has_move_input && fabs(startPlayMover.vitesse) < 4.0 &&
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

    if (startPlayAnim.moving) {
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
        startPlayAnim.frame_index = 0;
        startPlayAnim.last_anim_tick = now;
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

        if (startPlayAnim.idle_state == STARTPLAY_IDLE_DANCE) {
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
        } else if (!startPlayAnim.moving && startPlayAnim.stop_tex) {
            active_sheet = startPlayAnim.stop_tex;
            rows = 1;
            cols = 1;
            if (SDL_QueryTexture(startPlayAnim.stop_tex, NULL, NULL, &frame_w, &frame_h) != 0 ||
                frame_w <= 0 || frame_h <= 0) {
                frame_w = startPlayPlayerRect.w;
                frame_h = startPlayPlayerRect.h;
            }
        } else {
            active_sheet = (startPlayAnim.facing < 0) ? startPlayAnim.walk_back_tex : startPlayAnim.walk_tex;
            rows = (startPlayAnim.facing < 0) ? startPlayAnim.walk_back_rows : startPlayAnim.walk_rows;
            cols = (startPlayAnim.facing < 0) ? startPlayAnim.walk_back_cols : startPlayAnim.walk_cols;
            frame_w = (startPlayAnim.facing < 0) ? startPlayAnim.walk_back_frame_w : startPlayAnim.walk_frame_w;
            frame_h = (startPlayAnim.facing < 0) ? startPlayAnim.walk_back_frame_h : startPlayAnim.walk_frame_h;
        }

        if (active_sheet && rows > 0 && cols > 0 && frame_w > 0 && frame_h > 0) {
            int frame_count = rows * cols;
            int draw_frame = startPlayAnim.frame_index % frame_count;
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

    if (game->startHeartTex) {
        int hearts = 10;
        int size = 32;
        int gap = 6;
        int x0 = 16;
        int y0 = 16;
        for (int i = 0; i < hearts; i++) {
            SDL_Rect r = {x0 + i * (size + gap), y0, size, size};
            SDL_RenderCopy(renderer, game->startHeartTex, NULL, &r);
        }

        if (startPlayAnim.coin_tex) {
            int coin_w = 42;
            int coin_h = 42;
            int space_from_hearts = 14;
            int left_border_space = 18;
            SDL_Rect coin_base = {
                left_border_space,
                y0 + size + space_from_hearts,
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
    if (startPlayAnim.stop_tex) SDL_DestroyTexture(startPlayAnim.stop_tex);
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
