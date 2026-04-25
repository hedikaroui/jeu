#include "game.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

StartPlayAnimation startPlayAnim = {.dance_loop_channel = -1};
StartPlayMover startPlayMover = {0};
Uint32 startPlayIntroStart = 0;
SDL_Rect startPlayPlayerRect = {0, 0, 120, 120};
Uint32 startPlayLastTick = 0;
int startPlayJumping = 0;
int startPlayPendingJump = 0;
double startPlayJumpRelX = -50.0;
double startPlayJumpRelY = 0.0;
double startPlayJumpProgress = 0.0;
int startPlayJumpBaseX = 0;
int startPlayJumpBaseY = 0;
int startPlayJumpDir = 0;
Uint32 startPlayLastJumpDebugTick = 0;
int startPlayJumpPhase = 0;
int startPlayRngSeeded = 0;
int gameHazardsRngSeeded = 0;

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
#define STARTPLAY_PLAYER_Y 544.0
#define STARTPLAY_RUN_MAX_SPEED 1000.0
#define STARTPLAY_RUN_ACCEL 6000.0
#define STARTPLAY_WALK_FRAME_BASE_MS 140
#define STARTPLAY_WALK_FRAME_BONUS_DIV 28
#define STARTPLAY_WALK_FRAME_BONUS_CAP 95
#define STARTPLAY_WALK_FRAME_MIN_MS 40
#define GAME_ENIGME_DELAY_MIN_MS 9000u
#define GAME_ENIGME_DELAY_MAX_MS 25000u
#define GAME_ENERGY_MAX 100.0
#define GAME_ENERGY_RECHARGE_PER_SEC 17.0
#define GAME_ENERGY_RECHARGE_REST_PER_SEC 28.0
#define GAME_ENERGY_DRAIN_FAST_PER_SEC 44.0
#define GAME_ENERGY_DRAIN_SLOW_PER_SEC 18.0
#define GAME_ENERGY_TIRED_MS 1800u
#define GAME_PICKUP_MS 780u
#define GAME_SNOWBALL_STATE_FALLING 1
#define GAME_SNOWBALL_STATE_GROUNDED 2
#define GAME_SNOWBALL_BOX_W 60
#define GAME_SNOWBALL_BOX_H 60
#define GAME_SNOWBALL_FALL_SPEED_MIN 12
#define GAME_SNOWBALL_FALL_SPEED_MAX 18
#define GAME_SNOWBALL_SPAWN_MIN_MS 2200u
#define GAME_SNOWBALL_SPAWN_MAX_MS 5600u

Uint32 game_enigme_intro_tick = 0;
int game_enigme_intro_shown = 0;
Uint32 game_next_snowball_spawn_tick = 0;

const char *startplay_wave_msgs[] = {
    "HEEY ,YOU ",
    "YES I M TALKING TO YOU ",
    "ARE YOU PLAYING OR WHAT ? "
};

int start_play_point_in_rect(SDL_Rect r, int x, int y) {
    return x >= r.x && x <= r.x + r.w && y >= r.y && y <= r.y + r.h;
}

int game_key_pressed(const Uint8 *keys, SDL_Scancode sc) {
    if (!keys || sc == SDL_SCANCODE_UNKNOWN) return 0;
    return keys[sc] != 0;
}

int game_player_uses_mouse(const Game *game, int player_index) {
    if (!game || player_index < 0 || player_index > 1) return 0;
    return game->playerControls[player_index] == GAME_CONTROL_MOUSE;
}

int game_mouse_owner(const Game *game) {
    int player_count;

    if (!game) return -1;
    player_count = (game->player_mode != 1) ? 2 : 1;
    for (int i = 0; i < player_count; i++) {
        if (game_player_uses_mouse(game, i)) return i;
    }
    return -1;
}

void game_mouse_direction_for_rect(SDL_Rect rect, int *left_pressed,
                                          int *right_pressed, int *run_modifier) {
    int mx;
    int my;
    int dx;
    const int deadzone = 28;
    const int run_distance = 210;

    SDL_GetMouseState(&mx, &my);
    (void)my;
    dx = mx - (rect.x + rect.w / 2);

    if (left_pressed) *left_pressed = (dx < -deadzone);
    if (right_pressed) *right_pressed = (dx > deadzone);
    if (run_modifier) *run_modifier = (abs(dx) > run_distance);
}

void game_minimap_ensure_layout(void);
int game_handle_minimap_mouse_event(const SDL_Event *e);
void game_update_minimap_spark(Uint32 now);
void game_render_background(Game *game, SDL_Renderer *renderer, SDL_Rect dst,
                                   int focus_x, int focus_y);
void game_render_minimap_overlay(Game *game, SDL_Renderer *renderer, int include_second_player,
                                        SDL_Texture *primary_marker_tex, const SDL_Rect *primary_pos_override);
void game_reset_enemy_obstacles(Game *game);
void game_sync_start_play_character(Game *game);
void game_update_world_hazards_motion(Game *game, Uint32 dt, Uint32 now);
void game_update_world_hazards(Game *game, Uint32 dt, Uint32 now);
void game_render_world_hazards(Game *game, SDL_Renderer *renderer);
void game_destroy_character_textures(Personnage *p);
void game_load_harry_character(SDL_Renderer *renderer, Personnage *p);
void game_load_marvin_character(SDL_Renderer *renderer, Personnage *p);
void game_load_enemy_textures(Game *game, SDL_Renderer *renderer);
void game_load_obstacle_assets(Game *game, SDL_Renderer *renderer);
int game_update_damage_state(Personnage *p, Uint32 now);
int game_update_pickup_state(Personnage *p, Uint32 now);
void game_draw_character(SDL_Renderer *renderer, Personnage *p);
void game_draw_enemy_sprite(SDL_Renderer *renderer, const GameEnemy *enemy,
                                   SDL_Rect dst_rect);
void game_draw_minimap_player_marker(SDL_Renderer *renderer, const Personnage *player,
                                            SDL_Texture *fallback_texture,
                                            SDL_Rect marker, SDL_Color fallback,
                                            int move_dir, Uint32 now);
void games_reset_menu_state(void);
void game_schedule_next_enigme(Uint32 now);
void game_wrap_horizontal_position(int *x, int obj_width, int world_width);
double game_wrapped_delta_x(double from_x, double to_x, int world_width);
int game_collision_trigonometric_wrapped(SDL_Rect a, SDL_Rect b, int world_width);
void game_reset_character_energy(Personnage *p);
int game_character_is_tired(const Personnage *p, Uint32 now);
int game_update_character_stamina(Personnage *p, int wants_run, int has_move_input, Uint32 dt, Uint32 now);
int game_find_pickable_snowball_slot(const Game *game, const Personnage *p,
                                            Uint32 now, double *distance_sq);
int game_handle_snowball_pickup_input(Game *game, Uint32 now);
void game_schedule_next_snowball_spawn(Uint32 now);
void game_draw_snowball_inventory(Game *game, SDL_Renderer *renderer,
                                         const Personnage *p,
                                         int x, int y, int size);
void game_render_pickup_prompt_for_player(Game *game, SDL_Renderer *renderer,
                                                 const Personnage *p,
                                                 SDL_Rect draw_rect);
void game_draw_energy_bar(Game *game, SDL_Renderer *renderer, const Personnage *p,
                                 int x, int y, int width, int height);
void game_move_animation(Personnage *p, Uint32 now);

void start_play_reload_texture(SDL_Renderer *renderer, SDL_Texture **dst, const char *path) {
    if (!dst) return;
    if (*dst) {
        SDL_DestroyTexture(*dst);
        *dst = NULL;
    }
    if (renderer && path && *path) {
        *dst = IMG_LoadTexture(renderer, path);
    }
}

void start_play_reload_texture_first(SDL_Renderer *renderer, SDL_Texture **dst,
                                            const char *a, const char *b) {
    if (!dst) return;
    if (*dst) {
        SDL_DestroyTexture(*dst);
        *dst = NULL;
    }
    if (renderer && a && *a) *dst = IMG_LoadTexture(renderer, a);
    if (!*dst && renderer && b && *b) *dst = IMG_LoadTexture(renderer, b);
}

void start_play_setup_sheet(SDL_Texture *tex, int wanted_rows, int wanted_cols,
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

void start_play_reset_mover(void) {
    Uint32 now = SDL_GetTicks();
    startPlayMover.x = (WIDTH - 120) / 2.0;
    startPlayMover.y = STARTPLAY_PLAYER_Y;
    startPlayMover.vitesse = 0.0;
    startPlayMover.acceleration = 0.0;
    startPlayMover.position_acc = (SDL_Rect){(int)startPlayMover.x, (int)startPlayMover.y, 120, 120};
    startPlayPlayerRect = startPlayMover.position_acc;
    startPlayAnim.frame_index = 0;
    startPlayAnim.facing = 1;
    startPlayAnim.moving = 0;
    startPlayAnim.running = 0;
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

void start_play_trigger_dance(Uint32 now) {
    if (startPlayJumping) return;
    startPlayMover.acceleration = 0.0;
    startPlayMover.vitesse = 0.0;
    startPlayAnim.moving = 0;
    startPlayAnim.running = 0;
    startPlayAnim.idle_state = STARTPLAY_IDLE_DANCE;
    startPlayAnim.frame_index = 0;
    startPlayAnim.last_input_tick = now;
    startPlayAnim.idle_msg_tick = now;
    startPlayAnim.last_anim_tick = now;
    if (startPlayAnim.dance_loop_sfx && startPlayAnim.dance_loop_channel < 0) {
        startPlayAnim.dance_loop_channel = Mix_PlayChannel(-1, startPlayAnim.dance_loop_sfx, -1);
    }
}

void start_play_begin_jump(int dir, Uint32 now) {
    if (startPlayJumping) return;
    startPlayJumping = 1;
    startPlayPendingJump = 0;
    startPlayJumpRelX = -50.0;
    startPlayJumpRelY = 0.0;
    startPlayJumpProgress = 0.0;
    startPlayJumpBaseX = startPlayPlayerRect.x;
    startPlayJumpBaseY = startPlayPlayerRect.y;
    startPlayJumpDir = dir;
    startPlayJumpPhase = 1;
    if (dir != 0) startPlayAnim.facing = dir;
    startPlayAnim.idle_state = STARTPLAY_IDLE_NONE;
    startPlayAnim.moving = 0;
    startPlayAnim.running = 0;
    startPlayAnim.frame_index = 0;
    startPlayAnim.last_anim_tick = now;
    printf("[START_PLAY JUMP] start base=(%d,%d) dir=%d facing=%d\n",
           startPlayJumpBaseX, startPlayJumpBaseY, startPlayJumpDir, startPlayAnim.facing);
    fflush(stdout);
}

void start_play_update_jump(Uint32 dt_ms, Uint32 now) {
    double t;
    double smooth_t;
    int dir;

    if (!startPlayJumping) return;

    if (startPlayJumpDir == 0) {
        const int threshold = startPlayJumpBaseY - 100;
        int step = (int)lround(7.0 * ((double)dt_ms / 16.0));
        if (step < 1) step = 1;

        startPlayPlayerRect.x = startPlayJumpBaseX;
        if (startPlayJumpPhase != 2) {
            startPlayJumpPhase = 1;
            startPlayPlayerRect.y -= step;
            if (startPlayPlayerRect.y <= threshold) {
                startPlayPlayerRect.y = threshold;
                startPlayJumpPhase = 2;
            }
        } else {
            startPlayPlayerRect.y += step;
        }

        if (startPlayPlayerRect.y >= startPlayJumpBaseY && startPlayJumpPhase == 2) {
            startPlayJumping = 0;
            startPlayJumpPhase = 0;
            startPlayJumpRelX = -50.0;
            startPlayJumpRelY = 0.0;
            startPlayJumpProgress = 0.0;
            startPlayJumpDir = 0;
            startPlayPlayerRect.y = startPlayJumpBaseY;
        } else {
            int offset = startPlayJumpBaseY - startPlayPlayerRect.y;
            if (offset < 0) offset = 0;
            startPlayJumpRelY = (double)offset;
            startPlayJumpProgress = startPlayJumpRelY / 100.0;
            if (startPlayJumpProgress > 1.0) startPlayJumpProgress = 1.0;
        }

        startPlayMover.x = (double)startPlayPlayerRect.x;
        startPlayMover.y = (double)startPlayPlayerRect.y;
        startPlayMover.position_acc = startPlayPlayerRect;
        return;
    }

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
        startPlayJumpPhase = 0;
        startPlayPlayerRect.y = startPlayJumpBaseY;
        startPlayMover.y = (double)startPlayJumpBaseY;
        startPlayMover.position_acc = startPlayPlayerRect;
        printf("[START_PLAY JUMP] end pos=(%d,%d)\n", startPlayPlayerRect.x, startPlayPlayerRect.y);
        fflush(stdout);
    }
}

void start_play_move_mover(Uint32 dt_ms) {
    double dt = (double)dt_ms / 1000.0;
    double dx = 0.5 * startPlayMover.acceleration * dt * dt + startPlayMover.vitesse * dt;
    startPlayMover.x += dx;
    startPlayMover.vitesse += startPlayMover.acceleration * dt;
    startPlayMover.position_acc.x = (int)lround(startPlayMover.x);
    startPlayMover.position_acc.y = (int)lround(startPlayMover.y);
    startPlayPlayerRect = startPlayMover.position_acc;
}

void game_sync_start_play_character(Game *game) {
    Personnage *p;
    Uint32 now;

    if (!game) return;

    p = &game->gameCharacter;
    now = SDL_GetTicks();
    p->position = startPlayPlayerRect;
    p->groundY = (int)lround(STARTPLAY_PLAYER_Y);
    p->facing = startPlayAnim.facing;

    if (p->damageActive) {
        p->up = 0;
        p->moving = 0;
        p->position.y = p->groundY;
        return;
    }

    if (game_character_is_tired(p, now)) {
        p->up = 0;
        p->moving = 0;
        p->position.y = p->groundY;
        p->movementState = GAME_MOVE_TIRED;
        return;
    }

    p->up = startPlayJumping;
    p->moving = startPlayAnim.moving;
    p->frameIndex = startPlayAnim.frame_index;

    if (startPlayJumping) {
        p->movementState = (startPlayAnim.facing < 0) ? GAME_MOVE_JUMP_BACK : GAME_MOVE_JUMP;
    } else if (startPlayAnim.moving && startPlayAnim.running) {
        p->movementState = (startPlayAnim.facing < 0) ? GAME_MOVE_RUN_BACK : GAME_MOVE_RUN;
    } else if (startPlayAnim.moving) {
        p->movementState = (startPlayAnim.facing < 0) ? GAME_MOVE_WALK_BACK : GAME_MOVE_WALK;
    } else {
        p->movementState = GAME_MOVE_STOP;
    }
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
    int use_marvin;
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
    if (!game->miniMapFrameTex)
        game->miniMapFrameTex = IMG_LoadTexture(renderer, "backgrounds/cadre_mini_map.png");
    if (!game->miniMapLockClosedTex)
        game->miniMapLockClosedTex = IMG_LoadTexture(renderer, "buttons/ferme_lock_1.png");
    if (!game->miniMapLockOpenTex)
        game->miniMapLockOpenTex = IMG_LoadTexture(renderer, "buttons/ouvert_lock_2.png");
    game_load_enemy_textures(game, renderer);
    game_load_obstacle_assets(game, renderer);

    use_marvin = (game->player_mode == 1 && game->solo_selected_player == 1);
    if (use_marvin) {
        start_play_reload_texture(renderer, &startPlayAnim.walk_tex,
                                  "spritesheet_characters/marvin-walk-v1.png");
        start_play_reload_texture_first(renderer, &startPlayAnim.walk_back_tex,
                                        "spritesheet_characters/marvin-walk-v1-reverse.png",
                                        "spritesheet_characters/marvin-walk-v1 -reverse.png");
        start_play_reload_texture(renderer, &startPlayAnim.run_tex,
                                  "spritesheet_characters/marvin-run.png");
        start_play_reload_texture(renderer, &startPlayAnim.run_back_tex,
                                  "spritesheet_characters/marvin-run-reverse.png");
        start_play_reload_texture(renderer, &startPlayAnim.jump_tex,
                                  "spritesheet_characters/marvin-jump.png");
        start_play_reload_texture_first(renderer, &startPlayAnim.jump_back_tex,
                                        "spritesheet_characters/marvin-jump-reverse.png",
                                        "spritesheet_characters/marvin-jump -reverse.png");
        start_play_reload_texture(renderer, &startPlayAnim.stop_tex,
                                  "spritesheet_characters/marvin-stand-up.png");
        start_play_reload_texture(renderer, &startPlayAnim.stop_back_tex,
                                  "spritesheet_characters/marvin-stand-up-reverse.png");
    } else {
        start_play_reload_texture(renderer, &startPlayAnim.walk_tex,
                                  "spritesheet_characters/mr_harry_walk_cycle_transparent.png");
        start_play_reload_texture(renderer, &startPlayAnim.walk_back_tex,
                                  "spritesheet_characters/mr_harry_walk_cycle_back_transparent.png");
        start_play_reload_texture(renderer, &startPlayAnim.run_tex,
                                  "spritesheet_characters/mr_harry_run.png");
        start_play_reload_texture_first(renderer, &startPlayAnim.run_back_tex,
                                        "spritesheet_characters/mr harry-run-reverse.png",
                                        "spritesheet_characters/mr harry-run -reverse.png");
        start_play_reload_texture(renderer, &startPlayAnim.jump_tex,
                                  "spritesheet_characters/mr_harry_jump_transparent.png");
        start_play_reload_texture(renderer, &startPlayAnim.jump_back_tex,
                                  "spritesheet_characters/mr_harry_jump_back_transparent.png");
        start_play_reload_texture(renderer, &startPlayAnim.stop_tex,
                                  "spritesheet_characters/mr_harry_stand_up.png");
        start_play_reload_texture(renderer, &startPlayAnim.stop_back_tex,
                                  "spritesheet_characters/mr_harry_stand_up_back.png");
    }
    start_play_reload_texture(renderer, &startPlayAnim.coin_tex,
                              "buttons/coin.png");
    start_play_reload_texture(renderer, &startPlayAnim.dance_tex,
                              use_marvin
                                  ? "spritesheet_characters/marvin-dance.png"
                                  : "spritesheet_characters/mr_harry_dance_animation_transparent.png");
    start_play_reload_texture(renderer, &startPlayAnim.wave_tex,
                              use_marvin
                                  ? "spritesheet_characters/marvin-stand-up.png"
                                  : "spritesheet_characters/mr_harry_wave_animation_transparent.png");
    game_destroy_character_textures(&game->gameCharacter);
    if (use_marvin) game_load_marvin_character(renderer, &game->gameCharacter);
    else game_load_harry_character(renderer, &game->gameCharacter);
    game->gameCharacter.damageActive = 0;
    game->gameCharacter.damageStartTick = 0;
    game->gameCharacter.damageInvulnUntil = 0;
    game_reset_character_energy(&game->gameCharacter);
    game->gameCharacter.frameIndex = 0;
    game->gameCharacter.lastFrameTick = SDL_GetTicks();
    if (!startPlayAnim.dance_loop_sfx) {
        startPlayAnim.dance_loop_sfx = Mix_LoadWAV("songs/sahara-dans-song.wav");
        if (!startPlayAnim.dance_loop_sfx)
            startPlayAnim.dance_loop_sfx = Mix_LoadWAV("songs/sahara-dance-song.wav");
    }

    if (!startPlayRngSeeded) {
        srand((unsigned int)time(NULL));
        startPlayRngSeeded = 1;
    }
    startPlayAnim.dance_loop_channel = -1;

    start_play_setup_sheet(startPlayAnim.walk_tex, HARRY_SHEET_ROWS, HARRY_SHEET_COLS,
                           &startPlayAnim.walk_rows, &startPlayAnim.walk_cols,
                           &startPlayAnim.walk_frame_w, &startPlayAnim.walk_frame_h);
    start_play_setup_sheet(startPlayAnim.walk_back_tex, HARRY_SHEET_ROWS, HARRY_SHEET_COLS,
                           &startPlayAnim.walk_back_rows, &startPlayAnim.walk_back_cols,
                           &startPlayAnim.walk_back_frame_w, &startPlayAnim.walk_back_frame_h);
    start_play_setup_sheet(startPlayAnim.run_tex, HARRY_SHEET_ROWS, HARRY_SHEET_COLS,
                           &startPlayAnim.run_rows, &startPlayAnim.run_cols,
                           &startPlayAnim.run_frame_w, &startPlayAnim.run_frame_h);
    start_play_setup_sheet(startPlayAnim.run_back_tex, HARRY_SHEET_ROWS, HARRY_SHEET_COLS,
                           &startPlayAnim.run_back_rows, &startPlayAnim.run_back_cols,
                           &startPlayAnim.run_back_frame_w, &startPlayAnim.run_back_frame_h);
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
    startPlayAnim.running = 0;
    startPlayAnim.idle_state = STARTPLAY_IDLE_NONE;
    startPlayAnim.idle_msg_index = 0;
    startPlayAnim.move_hold_ms = 0;
    startPlayAnim.last_input_tick = SDL_GetTicks();
    startPlayAnim.idle_msg_tick = startPlayAnim.last_input_tick;
    startPlayAnim.last_anim_tick = SDL_GetTicks();
    game_minimap_ensure_layout();

    startPlayIntroStart = SDL_GetTicks();
    startPlayLastTick = startPlayIntroStart;
    start_play_reset_mover();
    if (game->player_mode == 1) {
        game_schedule_next_enigme(startPlayIntroStart);
    }
    game_sync_start_play_character(game);
    game_reset_enemy_obstacles(game);

    game->startPlayLoaded = 1;
    return 1;
}

void StartPlay_LectureEntree(Game *game) {
    SDL_Event e;
    if (startPlayIntroStart == 0) {
        startPlayIntroStart = SDL_GetTicks();
        startPlayLastTick = startPlayIntroStart;
        start_play_reset_mover();
        if (game && game->player_mode == 1) {
            game_schedule_next_enigme(startPlayIntroStart);
        }
    }

    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            game->running = 0;
            return;
        }
        if (game->player_mode == 1 &&
            SDL_GetTicks() - startPlayIntroStart >= 2000 &&
            game_handle_minimap_mouse_event(&e)) {
            continue;
        }
        if (e.type == SDL_MOUSEBUTTONDOWN &&
            e.button.button == SDL_BUTTON_LEFT &&
            game_mouse_owner(game) == 0) {
            startPlayPendingJump = 1;
            continue;
        }
        if (e.type == SDL_KEYDOWN) {
            SDL_Scancode scan = e.key.keysym.scancode;
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
            if (scan == game->keyBindings[0][KEY_ACTION_JUMP]) {
                startPlayPendingJump = 1;
                printf("[START_PLAY INPUT] jump key pressed: %s\n", SDL_GetKeyName(e.key.keysym.sym));
                fflush(stdout);
            } else if (scan == game->keyBindings[0][KEY_ACTION_DANCE]) {
                start_play_trigger_dance(SDL_GetTicks());
            }
        }
    }

    if (SDL_GetTicks() - startPlayIntroStart < 2000) return;

    Uint32 now = SDL_GetTicks();
    Uint32 dt = (startPlayLastTick == 0) ? 16u : (now - startPlayLastTick);
    startPlayLastTick = now;
    game_update_minimap_spark(now);

    if (game && game->player_mode == 1 &&
        game_enigme_intro_shown &&
        game_enigme_intro_tick == 0) {
        game_schedule_next_enigme(now);
    }

    if (game && game->player_mode == 1 &&
        !game_enigme_intro_shown &&
        game_enigme_intro_tick != 0 &&
        now >= game_enigme_intro_tick) {
        game_enigme_intro_shown = 1;
        game_enigme_intro_tick = 0;
        game_sync_start_play_character(game);
        games_reset_menu_state();
        if (startPlayAnim.dance_loop_channel >= 0) {
            Mix_HaltChannel(startPlayAnim.dance_loop_channel);
            startPlayAnim.dance_loop_channel = -1;
        }
        Game_SetSubState(game, STATE_ENIGME);
        return;
    }

    if (game && game->gameCharacter.damageActive) {
        int still_damaged = game_update_damage_state(&game->gameCharacter, now);
        startPlayPlayerRect = game->gameCharacter.position;
        startPlayMover.x = (double)startPlayPlayerRect.x;
        startPlayMover.y = (double)startPlayPlayerRect.y;
        startPlayMover.vitesse = 0.0;
        startPlayMover.acceleration = 0.0;
        startPlayMover.position_acc = startPlayPlayerRect;
        startPlayJumping = 0;
        startPlayPendingJump = 0;
        startPlayAnim.moving = 0;
        startPlayAnim.running = 0;
        startPlayAnim.idle_state = STARTPLAY_IDLE_NONE;
        if (still_damaged) {
            game_update_character_stamina(&game->gameCharacter, 0, 0, dt, now);
            game_update_world_hazards_motion(game, dt, now);
            return;
        }
    }

    const Uint8 *keys = SDL_GetKeyboardState(NULL);
    int left_pressed = game_key_pressed(keys, game->keyBindings[0][KEY_ACTION_DOWN]);
    int right_pressed = game_key_pressed(keys, game->keyBindings[0][KEY_ACTION_WALK]);
    int run_pressed = game_key_pressed(keys, game->keyBindings[0][KEY_ACTION_RUN]);
    int dance_pressed = game_key_pressed(keys, game->keyBindings[0][KEY_ACTION_DANCE]);
    int jump_pressed = startPlayPendingJump ||
                       game_key_pressed(keys, game->keyBindings[0][KEY_ACTION_JUMP]);
    int has_move_input = ((left_pressed && !right_pressed) || (right_pressed && !left_pressed));
    int effective_run_pressed;

    if (game_player_uses_mouse(game, 0)) {
        game_mouse_direction_for_rect(startPlayPlayerRect,
                                      &left_pressed,
                                      &right_pressed,
                                      &run_pressed);
        jump_pressed = startPlayPendingJump;
        has_move_input = ((left_pressed && !right_pressed) || (right_pressed && !left_pressed));
    }

    effective_run_pressed = game_update_character_stamina(&game->gameCharacter, run_pressed, has_move_input, dt, now);
    if (game_character_is_tired(&game->gameCharacter, now)) {
        startPlayMover.acceleration = 0.0;
        startPlayMover.vitesse = 0.0;
        startPlayJumping = 0;
        startPlayPendingJump = 0;
        startPlayAnim.moving = 0;
        startPlayAnim.running = 0;
        startPlayAnim.idle_state = STARTPLAY_IDLE_NONE;
        startPlayPlayerRect.y = (int)lround(STARTPLAY_PLAYER_Y);
        startPlayMover.y = STARTPLAY_PLAYER_Y;
        startPlayMover.position_acc = startPlayPlayerRect;
        game_sync_start_play_character(game);
        game_move_animation(&game->gameCharacter, now);
        game_update_world_hazards(game, dt, now);
        return;
    }

    if (jump_pressed && !startPlayJumping) {
        int jump_dir = 0;
        if (left_pressed && !right_pressed) jump_dir = -1;
        if (right_pressed && !left_pressed) jump_dir = 1;
        start_play_begin_jump(jump_dir, now);
    }
    startPlayPendingJump = 0;

    if (dance_pressed && !has_move_input && !startPlayJumping) {
        start_play_trigger_dance(now);
    }

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
    if (effective_run_pressed && has_move_input) hold_factor = STARTPLAY_HOLD_FACTOR_MAX;
    if (hold_factor > STARTPLAY_HOLD_FACTOR_MAX) hold_factor = STARTPLAY_HOLD_FACTOR_MAX;
    double accel = STARTPLAY_BASE_ACCEL * hold_factor;
    if (effective_run_pressed && has_move_input) accel = STARTPLAY_RUN_ACCEL;

    if (startPlayJumping) {
        startPlayMover.acceleration = 0.0;
        startPlayMover.vitesse = 0.0;
    } else if (left_pressed && !right_pressed) {
        startPlayMover.acceleration = -accel;
        if (effective_run_pressed) startPlayMover.vitesse = -STARTPLAY_RUN_MAX_SPEED;
        startPlayAnim.facing = -1;
    } else if (right_pressed && !left_pressed) {
        startPlayMover.acceleration = accel;
        if (effective_run_pressed) startPlayMover.vitesse = STARTPLAY_RUN_MAX_SPEED;
        startPlayAnim.facing = 1;
    } else {
        startPlayMover.acceleration = 0.0;
        startPlayMover.vitesse *= STARTPLAY_FRICTION;
        if (fabs(startPlayMover.vitesse) < STARTPLAY_STOP_EPSILON) startPlayMover.vitesse = 0.0;
    }

    if (!startPlayJumping) start_play_move_mover(dt);

    {
        double max_speed = (effective_run_pressed && has_move_input)
            ? STARTPLAY_RUN_MAX_SPEED
            : STARTPLAY_BASE_MAX_SPEED + hold_factor * STARTPLAY_MAX_SPEED_BONUS;
        if (startPlayMover.vitesse > max_speed) startPlayMover.vitesse = max_speed;
        if (startPlayMover.vitesse < -max_speed) startPlayMover.vitesse = -max_speed;
    }

    if (startPlayJumping) {
        start_play_update_jump(dt, now);
    } else {
        /* Keep the character on a fixed horizontal line when not jumping. */
        startPlayMover.y = STARTPLAY_PLAYER_Y;
        startPlayMover.position_acc.y = (int)lround(startPlayMover.y);
        startPlayPlayerRect.y = startPlayMover.position_acc.y;
    }

    startPlayAnim.moving = !startPlayJumping && (has_move_input || fabs(startPlayMover.vitesse) > 8.0);
    startPlayAnim.running = startPlayAnim.moving && has_move_input && effective_run_pressed;
    if (startPlayMover.vitesse < -8.0) startPlayAnim.facing = -1;
    if (startPlayMover.vitesse > 8.0) startPlayAnim.facing = 1;

    game_sync_start_play_character(game);
    game_update_world_hazards(game, dt, now);

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
        startPlayAnim.running = 0;
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
        startPlayAnim.running = 0;
        if (frames < 1) frames = 1;
        if (now - startPlayAnim.last_anim_tick >= 90u) {
            startPlayAnim.frame_index = (startPlayAnim.frame_index + 1) % frames;
            startPlayAnim.last_anim_tick = now;
        }
    } else if (startPlayAnim.moving) {
        int use_run_sheet = startPlayAnim.running &&
            ((startPlayAnim.facing < 0 && startPlayAnim.run_back_tex) ||
             (startPlayAnim.facing >= 0 && startPlayAnim.run_tex));
        int rows = use_run_sheet
            ? ((startPlayAnim.facing < 0) ? startPlayAnim.run_back_rows : startPlayAnim.run_rows)
            : ((startPlayAnim.facing < 0) ? startPlayAnim.walk_back_rows : startPlayAnim.walk_rows);
        int cols = use_run_sheet
            ? ((startPlayAnim.facing < 0) ? startPlayAnim.run_back_cols : startPlayAnim.run_cols)
            : ((startPlayAnim.facing < 0) ? startPlayAnim.walk_back_cols : startPlayAnim.walk_cols);
        int frames = rows * cols;
        Uint32 speed_bonus = startPlayAnim.move_hold_ms / STARTPLAY_WALK_FRAME_BONUS_DIV;
        Uint32 frame_ms = use_run_sheet ? 65u : STARTPLAY_WALK_FRAME_BASE_MS;
        if (speed_bonus > STARTPLAY_WALK_FRAME_BONUS_CAP) speed_bonus = STARTPLAY_WALK_FRAME_BONUS_CAP;
        if (!use_run_sheet) {
            frame_ms = (frame_ms > speed_bonus) ? (frame_ms - speed_bonus) : STARTPLAY_WALK_FRAME_MIN_MS;
            if (frame_ms < STARTPLAY_WALK_FRAME_MIN_MS) frame_ms = STARTPLAY_WALK_FRAME_MIN_MS;
        }
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
        startPlayAnim.running = 0;
        if (frames < 1) frames = 1;
        if (now - startPlayAnim.last_anim_tick >= 140u) {
            startPlayAnim.frame_index = (startPlayAnim.frame_index + 1) % frames;
            startPlayAnim.last_anim_tick = now;
        }
    }
}

void StartPlay_Affichage(Game *game, SDL_Renderer *renderer) {
    Uint32 elapsed = (startPlayIntroStart == 0) ? 0 : (SDL_GetTicks() - startPlayIntroStart);
    int is_duo = (game && game->player_mode != 1);
    int solo_second = (!is_duo && game && game->solo_selected_player == 1);
    SDL_Texture *solo_tex = (solo_second && game && game->player2Tex) ? game->player2Tex : (game ? game->player1Tex : NULL);

    if (elapsed < 2000) {
        if (game->startTextTex)
            SDL_RenderCopy(renderer, game->startTextTex, NULL, &game->startTextRect);
    } else {
        SDL_Rect scene_rect = {0, 0, WIDTH, HEIGHT};
        int focus_x = startPlayPlayerRect.x + startPlayPlayerRect.w / 2;
        int focus_y = startPlayPlayerRect.y + startPlayPlayerRect.h / 2;
        int ground_line_y = startPlayPlayerRect.y + startPlayPlayerRect.h;
        SDL_Rect ground_rect = {0, ground_line_y, WIDTH, HEIGHT - ground_line_y};
        SDL_Texture *active_sheet = NULL;
        int rows = 1, cols = 1;
        int frame_w = startPlayPlayerRect.w, frame_h = startPlayPlayerRect.h;
        int reverse_sheet = 0;

        game_sync_start_play_character(game);
        game_render_background(game, renderer, scene_rect, focus_x, focus_y);

        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 18, 18, 18, 170);
        SDL_RenderFillRect(renderer, &ground_rect);
        SDL_SetRenderDrawColor(renderer, 255, 214, 10, 255);
        SDL_RenderDrawLine(renderer, 0, ground_line_y, WIDTH, ground_line_y);

        if (!is_duo) {
            game_render_world_hazards(game, renderer);
        }

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
            int use_run_sheet = startPlayAnim.running &&
                ((startPlayAnim.facing < 0 && startPlayAnim.run_back_tex) ||
                 (startPlayAnim.facing >= 0 && startPlayAnim.run_tex));
            active_sheet = use_run_sheet
                ? ((startPlayAnim.facing < 0) ? startPlayAnim.run_back_tex : startPlayAnim.run_tex)
                : ((startPlayAnim.facing < 0) ? startPlayAnim.walk_back_tex : startPlayAnim.walk_tex);
            rows = use_run_sheet
                ? ((startPlayAnim.facing < 0) ? startPlayAnim.run_back_rows : startPlayAnim.run_rows)
                : ((startPlayAnim.facing < 0) ? startPlayAnim.walk_back_rows : startPlayAnim.walk_rows);
            cols = use_run_sheet
                ? ((startPlayAnim.facing < 0) ? startPlayAnim.run_back_cols : startPlayAnim.run_cols)
                : ((startPlayAnim.facing < 0) ? startPlayAnim.walk_back_cols : startPlayAnim.walk_cols);
            frame_w = use_run_sheet
                ? ((startPlayAnim.facing < 0) ? startPlayAnim.run_back_frame_w : startPlayAnim.run_frame_w)
                : ((startPlayAnim.facing < 0) ? startPlayAnim.walk_back_frame_w : startPlayAnim.walk_frame_w);
            frame_h = use_run_sheet
                ? ((startPlayAnim.facing < 0) ? startPlayAnim.run_back_frame_h : startPlayAnim.run_frame_h)
                : ((startPlayAnim.facing < 0) ? startPlayAnim.walk_back_frame_h : startPlayAnim.walk_frame_h);
            reverse_sheet = (startPlayAnim.facing < 0);
        }

        if (game && (game->gameCharacter.damageActive || game->gameCharacter.movementState == GAME_MOVE_TIRED)) {
            game_draw_character(renderer, &game->gameCharacter);
        } else if (active_sheet && rows > 0 && cols > 0 && frame_w > 0 && frame_h > 0) {
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
        } else if (solo_tex) {
            SDL_RenderCopy(renderer, solo_tex, NULL, &startPlayPlayerRect);
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

        if (is_duo) {
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
        } else {
            SDL_Texture *solo_life_tex = (solo_second && game->startPlayer2LifeTex)
                ? game->startPlayer2LifeTex
                : game->startPlayer1LifeTex;
            if (solo_life_tex) {
                SDL_Rect solo_life = {x0, y0, icon_w, icon_h};
                SDL_RenderCopy(renderer, solo_life_tex, NULL, &solo_life);
                bottom_icons_y = solo_life.y + solo_life.h;
            }
        }

        if (game) {
            game_draw_energy_bar(game, renderer, &game->gameCharacter,
                                 x0, bottom_icons_y + 12, 240, 22);
            bottom_icons_y += 42;
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
            int coin_hover = start_play_point_in_rect(coin_base, mx, my);
            int bob = coin_hover ? (int)lround(3.0 * sin((double)SDL_GetTicks() / 120.0)) : 0;
            SDL_Rect coin_draw = coin_base;
            coin_draw.y += bob;

            SDL_RenderCopy(renderer, startPlayAnim.coin_tex, NULL, &coin_draw);
        }
    }

    if (elapsed >= 2000 && !is_duo) {
        game_render_minimap_overlay(game, renderer, 0, solo_tex, &startPlayPlayerRect);
    }
}

void StartPlay_MiseAJour(Game *game) {
    if (startPlayPlayerRect.y < 0) startPlayPlayerRect.y = 0;
    game_wrap_horizontal_position(&startPlayPlayerRect.x, startPlayPlayerRect.w, WIDTH);
    if (startPlayPlayerRect.y + startPlayPlayerRect.h > HEIGHT)
        startPlayPlayerRect.y = HEIGHT - startPlayPlayerRect.h;

    startPlayMover.x = (double)startPlayPlayerRect.x;
    startPlayMover.y = (double)startPlayPlayerRect.y;
    startPlayMover.position_acc = startPlayPlayerRect;
    game_sync_start_play_character(game);
    SDL_Delay(16);
}

void StartPlay_Cleanup(void) {
    if (startPlayAnim.walk_tex) SDL_DestroyTexture(startPlayAnim.walk_tex);
    if (startPlayAnim.walk_back_tex) SDL_DestroyTexture(startPlayAnim.walk_back_tex);
    if (startPlayAnim.run_tex) SDL_DestroyTexture(startPlayAnim.run_tex);
    if (startPlayAnim.run_back_tex) SDL_DestroyTexture(startPlayAnim.run_back_tex);
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

int game_jump_latch = 0;

#define GAME_SHEET_ROWS 5
#define GAME_SHEET_COLS 5
#define GAME_JUMP_DURATION_MS 620.0
#define GAME_JUMP_REL_MIN -50.0
#define GAME_JUMP_REL_MAX 50.0
#define GAME_JUMP_PARABOLA_A -0.04
#define GAME_JUMP_PARABOLA_C 100.0
#define GAME_SPRITE_FRAME_COUNT (GAME_SHEET_ROWS * GAME_SHEET_COLS)
#define GAME_SPRITE_CACHE_MAX 48
#define GAME_CHARACTER_BOX_W 170
#define GAME_CHARACTER_BOX_H 180
#define GAME_MONO_GROUND_LINE_Y 544
#define GAME_RUN_SPEED_BONUS 12
#define GAME_DAMAGE_MS 850u
#define GAME_LAY_DOWN_MS 1250u
#define GAME_DAMAGE_INVULN_MS 1100u
#define GAME_ENEMY_SHEET_ROWS 5
#define GAME_ENEMY_SHEET_COLS 5
#define GAME_ENEMY_BOX_W 155
#define GAME_ENEMY_BOX_H 155
#define GAME_ENEMY_ALERT_DISTANCE 340
#define GAME_ENEMY_KEEP_DISTANCE 260
#define GAME_ENEMY_FOLLOW_SPEED 18
#define GAME_ENEMY_FLEE_SPEED 24
#define GAME_ENEMY_ANIM_STAND 0
#define GAME_ENEMY_ANIM_WALK 1
#define GAME_ENEMY_ANIM_RUN 2
#define GAME_ENEMY_STAND_FRAME_MS 140u
#define GAME_ENEMY_WALK_FRAME_MS 110u
#define GAME_ENEMY_RUN_FRAME_MS 70u
#define GAME_OBSTACLE_RIGHT 0
#define GAME_OBSTACLE_LEFT 1

const SDL_Rect game_walk_crop = {88, 48, 88, 167};
const SDL_Rect game_walk_back_crop = {80, 46, 91, 169};
const SDL_Rect game_jump_crop = {88, 48, 80, 160};
const SDL_Rect game_jump_back_crop = {95, 64, 77, 144};
Uint32 duoStartTime = 0;

#define DUO_DISPLAY_SAME 0
#define DUO_DISPLAY_VERTICAL 1
#define DUO_DISPLAY_HORIZONTAL 2
#define DUO_BACKGROUND_FIXED 0
#define DUO_BACKGROUND_SCROLLING 1
#define GAME_MINIMAP_ZOOM_MIN 0.5f
#define GAME_MINIMAP_ZOOM_MAX 2.0f
#define GAME_MINIMAP_ZOOM_STEP 0.1f
#define GAME_MINIMAP_MARGIN 18
#define GAME_MINIMAP_W 220
#define GAME_MINIMAP_H 130
#define GAME_MINIMAP_FRAME_PAD 18
#define GAME_MINIMAP_PADDING 2
#define GAME_MINIMAP_MARKER_MIN 6
#define GAME_MINIMAP_MARKER_MAX 24
#define GAME_MINIMAP_SPLIT_DISTANCE 320
#define GAME_MINIMAP_LOCK_BOX 42
#define GAME_MINIMAP_LOCK_ICON 26
#define GAME_MINIMAP_LOCK_INSET 6
#define GAME_MINIMAP_SPARK_FRAMES 10
#define GAME_MINIMAP_SPARK_FRAME_MS 45u
#define GAME_MINIMAP_SPARK_MAX_LEN 26

SDL_Rect display_choice_horizontal_rect = {0, 0, 0, 0};
SDL_Rect display_choice_vertical_rect = {0, 0, 0, 0};
SDL_Rect display_choice_same_rect = {0, 0, 0, 0};
SDL_Rect display_choice_bg_fixed_rect = {0, 0, 0, 0};
SDL_Rect display_choice_bg_scroll_rect = {0, 0, 0, 0};
int display_choice_hover_horizontal = 0;
int display_choice_hover_vertical = 0;
int display_choice_hover_same = 0;
int display_choice_hover_bg_fixed = 0;
int display_choice_hover_bg_scroll = 0;
int display_choice_last_hover_horizontal = 0;
int display_choice_last_hover_vertical = 0;
int display_choice_last_hover_same = 0;
int display_choice_last_hover_bg_fixed = 0;
int display_choice_last_hover_bg_scroll = 0;

typedef struct {
    SDL_Texture *texture;
    SDL_Rect frames[GAME_SPRITE_FRAME_COUNT];
    int frame_count;
} GameSpriteCacheEntry;

typedef struct {
    int active;
    int frame;
    int owner;      /* 0: player1, 1: player2 */
    int direction;  /* -1 left, +1 right */
    SDL_Rect world_pos;
    Uint32 last_tick;
} GameMinimapSpark;

GameSpriteCacheEntry game_sprite_cache[GAME_SPRITE_CACHE_MAX];
SDL_Point game_minimap_top_left = {0, 0};
int game_minimap_layout_initialized = 0;
int game_minimap_drag_unlocked = 0;
int game_minimap_dragging = 0;
int game_minimap_drag_offset_x = 0;
int game_minimap_drag_offset_y = 0;
GameMinimapSpark game_minimap_spark = {0};
Uint32 game_minimap_border_cooldown[2] = {0u, 0u};
int game_enemy_collision_latch[2] = {0, 0};

void game_draw_center_text(SDL_Renderer *renderer, TTF_Font *font, const char *text,
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

void game_draw_text_at(SDL_Renderer *renderer, TTF_Font *font, const char *text,
                              SDL_Color color, int x, int y) {
    SDL_Surface *surf;
    SDL_Texture *tex;
    SDL_Rect dst;

    if (!renderer || !font || !text || !*text) return;

    surf = TTF_RenderUTF8_Blended(font, text, color);
    if (!surf) return;

    tex = SDL_CreateTextureFromSurface(renderer, surf);
    if (tex) {
        dst = (SDL_Rect){x, y, surf->w, surf->h};
        SDL_RenderCopy(renderer, tex, NULL, &dst);
        SDL_DestroyTexture(tex);
    }
    SDL_FreeSurface(surf);
}

void game_draw_snowball_inventory(Game *game, SDL_Renderer *renderer,
                                         const Personnage *p,
                                         int x, int y, int size) {
    SDL_Rect badge;
    SDL_Rect icon;

    if (!game || !renderer || !p || !p->hasSnowball || !game->gameFallingTex || size <= 0) return;

    badge = (SDL_Rect){x - 4, y - 4, size + 8, size + 8};
    icon = (SDL_Rect){x, y, size, size};

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 12, 18, 26, 215);
    SDL_RenderFillRect(renderer, &badge);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 80);
    SDL_RenderDrawRect(renderer, &badge);
    SDL_RenderCopy(renderer, game->gameFallingTex, NULL, &icon);
}

void game_render_pickup_prompt_for_player(Game *game, SDL_Renderer *renderer,
                                                 const Personnage *p,
                                                 SDL_Rect draw_rect) {
    SDL_Rect viewport;
    SDL_Rect box;
    int slot;

    if (!game || !renderer || !p || !game->font) return;

    slot = game_find_pickable_snowball_slot(game, p, SDL_GetTicks(), NULL);
    if (slot < 0) return;

    SDL_RenderGetViewport(renderer, &viewport);
    box = (SDL_Rect){draw_rect.x + draw_rect.w / 2 - 132, draw_rect.y - 44, 264, 30};
    if (box.x < 10) box.x = 10;
    if (box.x + box.w > viewport.w - 10) box.x = viewport.w - box.w - 10;
    if (box.y < 10) box.y = draw_rect.y + draw_rect.h + 8;
    if (box.y + box.h > viewport.h - 10) box.y = viewport.h - box.h - 10;

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 12, 18, 26, 215);
    SDL_RenderFillRect(renderer, &box);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 90);
    SDL_RenderDrawRect(renderer, &box);
    game_draw_center_text(renderer, game->font, "Press X to pick snowball",
                          (SDL_Color){255, 255, 255, 255}, box);
}

void game_draw_energy_bar(Game *game, SDL_Renderer *renderer, const Personnage *p,
                                 int x, int y, int width, int height) {
    SDL_Rect outer;
    SDL_Rect inner;
    SDL_Rect fill;
    double ratio;
    SDL_Color label_color = {245, 245, 245, 255};
    SDL_Color fill_color;

    if (!renderer || !p || width <= 6 || height <= 6) return;

    ratio = p->energy / GAME_ENERGY_MAX;
    if (ratio < 0.0) ratio = 0.0;
    if (ratio > 1.0) ratio = 1.0;

    if (ratio > 0.65) fill_color = (SDL_Color){70, 220, 120, 255};
    else if (ratio > 0.3) fill_color = (SDL_Color){245, 190, 55, 255};
    else fill_color = (SDL_Color){235, 80, 70, 255};

    outer = (SDL_Rect){x, y, width, height};
    inner = (SDL_Rect){x + 3, y + 3, width - 6, height - 6};
    fill = inner;
    fill.w = (int)lround((double)inner.w * ratio);
    if (fill.w < 0) fill.w = 0;

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 8, 12, 18, 210);
    SDL_RenderFillRect(renderer, &outer);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 70);
    SDL_RenderDrawRect(renderer, &outer);

    SDL_SetRenderDrawColor(renderer, 22, 32, 40, 220);
    SDL_RenderFillRect(renderer, &inner);
    if (fill.w > 0) {
        SDL_SetRenderDrawColor(renderer, fill_color.r, fill_color.g, fill_color.b, fill_color.a);
        SDL_RenderFillRect(renderer, &fill);
    }

    if (game && game->font) {
        game_draw_text_at(renderer, game->font, "Energy", label_color, x, y - 24);
        if (game_character_is_tired(p, SDL_GetTicks())) {
            game_draw_text_at(renderer, game->font, "Resting...", (SDL_Color){255, 220, 120, 255},
                              x + width + 12, y + 2);
        }
    }
}

void duo_render_time(Game *game, SDL_Renderer *renderer, int x, int y) {
    char buf[32];
    Uint32 seconds = (SDL_GetTicks() - duoStartTime) / 1000u;
    snprintf(buf, sizeof(buf), "Time: %02u:%02u", seconds / 60u, seconds % 60u);
    game_draw_text_at(renderer, game->font, buf, (SDL_Color){255, 255, 255, 255}, x, y);
}

SDL_Texture *game_active_background(Game *game) {
    if (!game) return NULL;
    if (game->gameBgTex) return game->gameBgTex;
    return game->background;
}

void game_render_background(Game *game, SDL_Renderer *renderer, SDL_Rect dst, int focus_x, int focus_y) {
    SDL_Texture *bg;
    int tex_w = 0;
    int tex_h = 0;
    int render_w;
    int render_h;
    int offset_x = 0;
    int offset_y = 0;
    int start_x;
    int start_y;
    int scrolling_enabled;
    int clip_was_enabled;
    SDL_Rect old_clip = {0, 0, 0, 0};

    if (!renderer || !game) return;
    bg = game_active_background(game);
    if (!bg) return;

    clip_was_enabled = SDL_RenderIsClipEnabled(renderer);
    if (clip_was_enabled) SDL_RenderGetClipRect(renderer, &old_clip);
    SDL_RenderSetClipRect(renderer, &dst);

    scrolling_enabled = (game->player_mode == 1) ||
                        (game->duo_background_mode == DUO_BACKGROUND_SCROLLING);
    if (!scrolling_enabled) {
        SDL_RenderCopy(renderer, bg, NULL, &dst);
        if (clip_was_enabled) SDL_RenderSetClipRect(renderer, &old_clip);
        else SDL_RenderSetClipRect(renderer, NULL);
        return;
    }

    if (SDL_QueryTexture(bg, NULL, NULL, &tex_w, &tex_h) != 0 || tex_w <= 0 || tex_h <= 0) {
        SDL_RenderCopy(renderer, bg, NULL, &dst);
        if (clip_was_enabled) SDL_RenderSetClipRect(renderer, &old_clip);
        else SDL_RenderSetClipRect(renderer, NULL);
        return;
    }

    render_w = (int)lround(((double)tex_w * (double)dst.h) / (double)tex_h);
    render_h = dst.h;
    if (render_w < dst.w) render_w = dst.w;
    if (render_w < 1) render_w = dst.w;
    if (render_h < dst.h) render_h = dst.h;
    if (render_h < 1) render_h = dst.h;

    if (focus_y < 0) focus_y = 0;
    if (focus_y > HEIGHT) focus_y = HEIGHT;
    if (render_w > 1 && WIDTH > 0) {
        offset_x = (int)lround(((double)focus_x / (double)WIDTH) * (double)(render_w - 1));
        offset_x %= render_w;
        if (offset_x < 0) offset_x += render_w;
    }
    if (render_h > 1) {
        offset_y = (int)lround(((double)focus_y / (double)HEIGHT) * (double)(render_h - 1));
        offset_y %= render_h;
        if (offset_y < 0) offset_y += render_h;
    }

    start_x = dst.x - offset_x;
    start_y = dst.y - offset_y;
    while (start_x > dst.x) start_x -= render_w;
    while (start_y > dst.y) start_y -= render_h;

    for (int y = start_y; y < dst.y + dst.h; y += render_h) {
        for (int x = start_x; x < dst.x + dst.w; x += render_w) {
            SDL_Rect tile = {x, y, render_w, render_h};
            SDL_RenderCopy(renderer, bg, NULL, &tile);
        }
    }

    if (clip_was_enabled) SDL_RenderSetClipRect(renderer, &old_clip);
    else SDL_RenderSetClipRect(renderer, NULL);
}

int game_clampi(int value, int min_value, int max_value) {
    if (value < min_value) return min_value;
    if (value > max_value) return max_value;
    return value;
}

void game_wrap_horizontal_position(int *x, int obj_width, int world_width) {
    int span;

    if (!x || obj_width <= 0 || world_width <= 0) return;

    span = world_width + obj_width;
    while (*x > world_width) *x -= span;
    while (*x + obj_width < 0) *x += span;
}

void game_minimap_ensure_layout(void) {
    if (game_minimap_layout_initialized) return;
    game_minimap_top_left.x = WIDTH - GAME_MINIMAP_W - GAME_MINIMAP_MARGIN;
    game_minimap_top_left.y = GAME_MINIMAP_MARGIN;
    game_minimap_layout_initialized = 1;
}

void game_minimap_set_top_left(int x, int y) {
    int min_x = GAME_MINIMAP_FRAME_PAD;
    int min_y = GAME_MINIMAP_FRAME_PAD;
    int max_x = WIDTH - GAME_MINIMAP_W - GAME_MINIMAP_FRAME_PAD;
    int max_y = HEIGHT - GAME_MINIMAP_H - GAME_MINIMAP_FRAME_PAD;
    if (max_x < min_x) max_x = min_x;
    if (max_y < min_y) max_y = min_y;
    game_minimap_top_left.x = game_clampi(x, min_x, max_x);
    game_minimap_top_left.y = game_clampi(y, min_y, max_y);
}

SDL_Rect game_minimap_rect(void) {
    game_minimap_ensure_layout();
    return (SDL_Rect){game_minimap_top_left.x, game_minimap_top_left.y, GAME_MINIMAP_W, GAME_MINIMAP_H};
}

SDL_Rect game_minimap_frame_rect(SDL_Rect minimap_rect) {
    return (SDL_Rect){
        minimap_rect.x - GAME_MINIMAP_FRAME_PAD,
        minimap_rect.y - GAME_MINIMAP_FRAME_PAD,
        minimap_rect.w + GAME_MINIMAP_FRAME_PAD * 2,
        minimap_rect.h + GAME_MINIMAP_FRAME_PAD * 2
    };
}

SDL_Rect game_minimap_lock_rect(SDL_Rect frame_rect) {
    return (SDL_Rect){
        frame_rect.x + GAME_MINIMAP_LOCK_INSET,
        frame_rect.y + GAME_MINIMAP_LOCK_INSET,
        GAME_MINIMAP_LOCK_BOX,
        GAME_MINIMAP_LOCK_BOX
    };
}

int game_handle_minimap_mouse_event(const SDL_Event *e) {
    SDL_Rect minimap_rect;
    SDL_Rect frame_rect;
    SDL_Rect lock_rect;
    int mx;
    int my;

    if (!e) return 0;

    if (e->type == SDL_MOUSEBUTTONDOWN && e->button.button == SDL_BUTTON_LEFT) {
        mx = e->button.x;
        my = e->button.y;
        minimap_rect = game_minimap_rect();
        frame_rect = game_minimap_frame_rect(minimap_rect);
        lock_rect = game_minimap_lock_rect(frame_rect);

        if (start_play_point_in_rect(lock_rect, mx, my)) {
            game_minimap_drag_unlocked = !game_minimap_drag_unlocked;
            if (!game_minimap_drag_unlocked) game_minimap_dragging = 0;
            return 1;
        }
        if (game_minimap_drag_unlocked && start_play_point_in_rect(frame_rect, mx, my)) {
            game_minimap_dragging = 1;
            game_minimap_drag_offset_x = mx - minimap_rect.x;
            game_minimap_drag_offset_y = my - minimap_rect.y;
            return 1;
        }
    }
    if (e->type == SDL_MOUSEBUTTONUP && e->button.button == SDL_BUTTON_LEFT) {
        game_minimap_dragging = 0;
        return 1;
    }
    if (e->type == SDL_MOUSEMOTION && game_minimap_dragging && game_minimap_drag_unlocked) {
        game_minimap_set_top_left(e->motion.x - game_minimap_drag_offset_x,
                                  e->motion.y - game_minimap_drag_offset_y);
        return 1;
    }
    return 0;
}

SDL_Rect game_world_to_minimap_rect(SDL_Rect world_pos, SDL_Rect minimap_rect, float zoom) {
    SDL_Rect marker;
    double scale_x;
    double scale_y;
    int max_x;
    int max_y;

    if (zoom < GAME_MINIMAP_ZOOM_MIN) zoom = GAME_MINIMAP_ZOOM_MIN;
    if (zoom > GAME_MINIMAP_ZOOM_MAX) zoom = GAME_MINIMAP_ZOOM_MAX;

    scale_x = ((double)minimap_rect.w / (double)WIDTH) * (double)zoom;
    scale_y = ((double)minimap_rect.h / (double)HEIGHT) * (double)zoom;

    marker.x = minimap_rect.x + (int)lround((double)world_pos.x * scale_x);
    marker.y = minimap_rect.y + (int)lround((double)world_pos.y * scale_y);
    marker.w = (int)lround((double)world_pos.w * scale_x);
    marker.h = (int)lround((double)world_pos.h * scale_y);

    if (marker.w < GAME_MINIMAP_MARKER_MIN) marker.w = GAME_MINIMAP_MARKER_MIN;
    if (marker.h < GAME_MINIMAP_MARKER_MIN) marker.h = GAME_MINIMAP_MARKER_MIN;
    if (marker.w > GAME_MINIMAP_MARKER_MAX) marker.w = GAME_MINIMAP_MARKER_MAX;
    if (marker.h > GAME_MINIMAP_MARKER_MAX) marker.h = GAME_MINIMAP_MARKER_MAX;

    max_x = minimap_rect.x + minimap_rect.w - marker.w - GAME_MINIMAP_PADDING;
    max_y = minimap_rect.y + minimap_rect.h - marker.h - GAME_MINIMAP_PADDING;
    if (max_x < minimap_rect.x + GAME_MINIMAP_PADDING) {
        max_x = minimap_rect.x + GAME_MINIMAP_PADDING;
    }
    if (max_y < minimap_rect.y + GAME_MINIMAP_PADDING) {
        max_y = minimap_rect.y + GAME_MINIMAP_PADDING;
    }

    marker.x = game_clampi(marker.x, minimap_rect.x + GAME_MINIMAP_PADDING, max_x);
    marker.y = game_clampi(marker.y, minimap_rect.y + GAME_MINIMAP_PADDING, max_y);
    return marker;
}

int game_minimap_player_move_dir(const Personnage *p) {
    if (!p || p->damageActive) return 0;
    switch (p->movementState) {
        case GAME_MOVE_WALK:
        case GAME_MOVE_RUN:
        case GAME_MOVE_JUMP:
            return 1;
        case GAME_MOVE_WALK_BACK:
        case GAME_MOVE_RUN_BACK:
        case GAME_MOVE_JUMP_BACK:
            return -1;
        default:
            return 0;
    }
}

void game_draw_minimap_marker(SDL_Renderer *renderer, SDL_Texture *texture,
                                     SDL_Rect marker, SDL_Color fallback) {
    if (!renderer) return;
    if (texture) {
        SDL_RenderCopy(renderer, texture, NULL, &marker);
        return;
    }
    SDL_SetRenderDrawColor(renderer, fallback.r, fallback.g, fallback.b, fallback.a);
    SDL_RenderFillRect(renderer, &marker);
}

void game_trigger_minimap_spark(int owner, SDL_Rect world_pos, int direction, Uint32 now) {
    game_minimap_spark.active = 1;
    game_minimap_spark.frame = 0;
    game_minimap_spark.owner = owner;
    game_minimap_spark.direction = (direction < 0) ? -1 : 1;
    game_minimap_spark.world_pos = world_pos;
    game_minimap_spark.last_tick = now;
}

void game_update_minimap_spark(Uint32 now) {
    Uint32 steps;

    if (!game_minimap_spark.active) return;
    if (now <= game_minimap_spark.last_tick) return;

    steps = (now - game_minimap_spark.last_tick) / GAME_MINIMAP_SPARK_FRAME_MS;
    if (steps == 0) return;

    game_minimap_spark.last_tick += steps * GAME_MINIMAP_SPARK_FRAME_MS;
    game_minimap_spark.frame += (int)steps;
    if (game_minimap_spark.frame >= GAME_MINIMAP_SPARK_FRAMES) {
        game_minimap_spark.active = 0;
        game_minimap_spark.frame = 0;
    }
}

void game_render_minimap_spark(SDL_Renderer *renderer, SDL_Rect marker,
                                      int direction, int frame) {
    const int rays[8][2] = {
        {1, 0}, {1, 1}, {0, 1}, {-1, 1},
        {-1, 0}, {-1, -1}, {0, -1}, {1, -1}
    };
    int cx;
    int cy;
    int len;
    int alpha;

    if (!renderer) return;

    cx = marker.x + marker.w / 2;
    cy = marker.y + marker.h / 2;
    len = 6 + (frame * 2);
    if (len > GAME_MINIMAP_SPARK_MAX_LEN) len = GAME_MINIMAP_SPARK_MAX_LEN;
    alpha = 255 - (frame * 22);
    if (alpha < 40) alpha = 40;

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 255, 224, 60, (Uint8)alpha);
    for (int i = 0; i < 8; i++) {
        int dx = rays[i][0];
        int dy = rays[i][1];
        int sx = cx + (direction < 0 ? -2 : 2);
        int ex = sx + dx * len;
        int ey = cy + dy * len;
        SDL_RenderDrawLine(renderer, sx, cy, ex, ey);
    }

    SDL_SetRenderDrawColor(renderer, 255, 110, 35, (Uint8)alpha);
    SDL_Rect core = {cx - 2, cy - 2, 4, 4};
    SDL_RenderFillRect(renderer, &core);
}

void game_render_minimap_overlay(Game *game, SDL_Renderer *renderer, int include_second_player,
                                        SDL_Texture *primary_marker_tex, const SDL_Rect *primary_pos_override) {
    SDL_Rect frame_rect;
    SDL_Rect minimap_rect;
    SDL_Rect lock_rect;
    SDL_Rect lock_icon_rect;
    SDL_Rect panel_a;
    SDL_Rect panel_b;
    SDL_Rect p1_marker;
    SDL_Rect p2_marker;
    SDL_Rect spark_marker;
    SDL_Rect spark_panel;
    SDL_Rect enemy_marker;
    SDL_Rect p1_pos;
    SDL_Texture *lock_tex;
    int focus_x;
    int focus_y;
    int split_panels = 0;
    int p1_center_x;
    int p1_center_y;
    int p2_center_x;
    int p2_center_y;
    int p1_move_dir;
    int p2_move_dir;
    int dx;
    int dy;
    Uint32 now;

    if (!game || !renderer) return;
    now = SDL_GetTicks();

    minimap_rect = game_minimap_rect();
    frame_rect = game_minimap_frame_rect(minimap_rect);
    p1_pos = primary_pos_override ? *primary_pos_override : game->gameCharacter.position;
    p1_center_x = p1_pos.x + p1_pos.w / 2;
    p1_center_y = p1_pos.y + p1_pos.h / 2;
    p2_center_x = game->gameCharacter2.position.x + game->gameCharacter2.position.w / 2;
    p2_center_y = game->gameCharacter2.position.y + game->gameCharacter2.position.h / 2;
    focus_x = p1_center_x;
    focus_y = p1_center_y;
    if (include_second_player) focus_x = (p1_center_x + p2_center_x) / 2;
    if (include_second_player) focus_y = (p1_center_y + p2_center_y) / 2;
    p1_move_dir = game_minimap_player_move_dir(&game->gameCharacter);
    p2_move_dir = game_minimap_player_move_dir(&game->gameCharacter2);

    if (game->miniMapFrameTex) SDL_RenderCopy(renderer, game->miniMapFrameTex, NULL, &frame_rect);

    panel_a = minimap_rect;
    panel_b = minimap_rect;
    if (include_second_player) {
        dx = p1_center_x - p2_center_x;
        dy = p1_center_y - p2_center_y;
        split_panels = ((dx * dx + dy * dy) > (GAME_MINIMAP_SPLIT_DISTANCE * GAME_MINIMAP_SPLIT_DISTANCE));
    }
    if (split_panels) {
        panel_a.w = minimap_rect.w / 2;
        panel_b.x = minimap_rect.x + panel_a.w;
        panel_b.w = minimap_rect.w - panel_a.w;
        if (panel_a.w < 8 || panel_b.w < 8) {
            split_panels = 0;
            panel_a = minimap_rect;
            panel_b = minimap_rect;
        }
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    if (split_panels) {
        SDL_SetRenderDrawColor(renderer, 12, 14, 20, 210);
        SDL_RenderFillRect(renderer, &panel_a);
        SDL_RenderFillRect(renderer, &panel_b);
        game_render_background(game, renderer, panel_a, p1_center_x, p1_center_y);
        game_render_background(game, renderer, panel_b, p2_center_x, p2_center_y);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 65);
        SDL_RenderFillRect(renderer, &panel_a);
        SDL_RenderFillRect(renderer, &panel_b);
        SDL_SetRenderDrawColor(renderer, 235, 235, 235, 190);
        SDL_RenderDrawLine(renderer, panel_b.x, minimap_rect.y, panel_b.x, minimap_rect.y + minimap_rect.h);
    } else {
        SDL_SetRenderDrawColor(renderer, 12, 14, 20, 210);
        SDL_RenderFillRect(renderer, &minimap_rect);
        game_render_background(game, renderer, minimap_rect, focus_x, focus_y);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 65);
        SDL_RenderFillRect(renderer, &minimap_rect);
    }

    if (game->gameEnemy.active && game->gameEnemy.texture) {
        enemy_marker = game_world_to_minimap_rect(game->gameEnemy.position,
                                                  split_panels ? panel_a : minimap_rect,
                                                  game->minimap_zoom);
        game_draw_enemy_sprite(renderer, &game->gameEnemy, enemy_marker);
        if (split_panels) {
            enemy_marker = game_world_to_minimap_rect(game->gameEnemy.position,
                                                      panel_b,
                                                      game->minimap_zoom);
            game_draw_enemy_sprite(renderer, &game->gameEnemy, enemy_marker);
        }
    }

    p1_marker = game_world_to_minimap_rect(p1_pos,
                                           split_panels ? panel_a : minimap_rect,
                                           game->minimap_zoom);
    game_draw_minimap_player_marker(renderer, &game->gameCharacter,
                                    primary_marker_tex ? primary_marker_tex : game->player1Tex,
                                    p1_marker, (SDL_Color){235, 84, 84, 255},
                                    p1_move_dir, now);

    if (include_second_player) {
        p2_marker = game_world_to_minimap_rect(game->gameCharacter2.position,
                                               split_panels ? panel_b : minimap_rect,
                                               game->minimap_zoom);
        game_draw_minimap_player_marker(renderer, &game->gameCharacter2,
                                        game->player2Tex,
                                        p2_marker, (SDL_Color){74, 140, 255, 255},
                                        p2_move_dir, now);
    }

    if (game_minimap_spark.active) {
        spark_panel = minimap_rect;
        if (include_second_player && split_panels) {
            spark_panel = (game_minimap_spark.owner == 1) ? panel_b : panel_a;
        }
        spark_marker = game_world_to_minimap_rect(game_minimap_spark.world_pos,
                                                  spark_panel, game->minimap_zoom);
        game_render_minimap_spark(renderer, spark_marker,
                                  game_minimap_spark.direction,
                                  game_minimap_spark.frame);
    }

    if (!game->miniMapFrameTex) {
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        for (int i = 0; i < 3; i++) {
            SDL_Rect border = {
                frame_rect.x - i, frame_rect.y - i,
                frame_rect.w + i * 2, frame_rect.h + i * 2
            };
            SDL_RenderDrawRect(renderer, &border);
        }
    }

    lock_rect = game_minimap_lock_rect(frame_rect);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 235);
    SDL_RenderFillRect(renderer, &lock_rect);
    SDL_SetRenderDrawColor(renderer, 25, 25, 25, 210);
    SDL_RenderDrawRect(renderer, &lock_rect);

    lock_tex = game_minimap_drag_unlocked ? game->miniMapLockOpenTex : game->miniMapLockClosedTex;
    lock_icon_rect = (SDL_Rect){
        lock_rect.x + (lock_rect.w - GAME_MINIMAP_LOCK_ICON) / 2,
        lock_rect.y + (lock_rect.h - GAME_MINIMAP_LOCK_ICON) / 2,
        GAME_MINIMAP_LOCK_ICON,
        GAME_MINIMAP_LOCK_ICON
    };
    if (lock_tex) SDL_RenderCopy(renderer, lock_tex, NULL, &lock_icon_rect);
}

void display_choice_layout(void) {
    const int box_w = 270;
    const int box_h = 180;
    const int spacing = 70;
    const int top_y = 238;
    const int bg_w = 220;
    const int bg_h = 44;
    const int bg_gap = 28;
    const int bg_y = 184;

    display_choice_bg_fixed_rect = (SDL_Rect){WIDTH / 2 - bg_w - (bg_gap / 2), bg_y, bg_w, bg_h};
    display_choice_bg_scroll_rect = (SDL_Rect){WIDTH / 2 + (bg_gap / 2), bg_y, bg_w, bg_h};
    display_choice_horizontal_rect = (SDL_Rect){WIDTH / 2 - box_w - spacing, top_y, box_w, box_h};
    display_choice_vertical_rect = (SDL_Rect){WIDTH / 2 + spacing, top_y, box_w, box_h};
    display_choice_same_rect = (SDL_Rect){WIDTH / 2 - box_w / 2, top_y + box_h + 48, box_w, box_h};
}

void display_choice_fill_circle(SDL_Renderer *renderer, int cx, int cy, int radius,
                                       Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    if (!renderer || radius <= 0) return;
    SDL_SetRenderDrawColor(renderer, r, g, b, a);
    for (int dy = -radius; dy <= radius; dy++) {
        int extent = (int)lround(sqrt((double)(radius * radius - dy * dy)));
        SDL_RenderDrawLine(renderer, cx - extent, cy + dy, cx + extent, cy + dy);
    }
}

void display_choice_draw_card(SDL_Renderer *renderer, TTF_Font *font, SDL_Rect card,
                                     int hover, int mode, const char *title) {
    SDL_Rect preview;
    SDL_Rect label_box;
    SDL_Color white = {255, 255, 255, 255};
    SDL_Color soft = {225, 230, 235, 255};
    Uint8 fill_r = 35;
    Uint8 fill_g = 95;
    Uint8 fill_b = 150;

    if (!renderer) return;

    if (mode == DUO_DISPLAY_HORIZONTAL) {
        fill_r = 62;
        fill_g = 145;
        fill_b = 85;
    } else if (mode == DUO_DISPLAY_SAME) {
        fill_r = 190;
        fill_g = 86;
        fill_b = 64;
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 5, 10, 18, hover ? 220 : 185);
    SDL_RenderFillRect(renderer, &card);
    SDL_SetRenderDrawColor(renderer, fill_r, fill_g, fill_b, hover ? 255 : 220);
    SDL_RenderDrawRect(renderer, &card);

    preview = (SDL_Rect){card.x + 34, card.y + 28, card.w - 68, 94};
    SDL_SetRenderDrawColor(renderer, fill_r, fill_g, fill_b, hover ? 235 : 195);
    SDL_RenderFillRect(renderer, &preview);
    SDL_SetRenderDrawColor(renderer, 245, 245, 245, 255);
    SDL_RenderDrawRect(renderer, &preview);

    if (mode == DUO_DISPLAY_VERTICAL) {
        SDL_RenderDrawLine(renderer, preview.x + preview.w / 2, preview.y,
                           preview.x + preview.w / 2, preview.y + preview.h);
        game_draw_center_text(renderer, font, "P1", white,
                              (SDL_Rect){preview.x, preview.y, preview.w / 2, preview.h});
        game_draw_center_text(renderer, font, "P2", white,
                              (SDL_Rect){preview.x + preview.w / 2, preview.y, preview.w / 2, preview.h});
    } else if (mode == DUO_DISPLAY_HORIZONTAL) {
        SDL_RenderDrawLine(renderer, preview.x, preview.y + preview.h / 2,
                           preview.x + preview.w, preview.y + preview.h / 2);
        game_draw_center_text(renderer, font, "P1", white,
                              (SDL_Rect){preview.x, preview.y, preview.w, preview.h / 2});
        game_draw_center_text(renderer, font, "P2", white,
                              (SDL_Rect){preview.x, preview.y + preview.h / 2, preview.w, preview.h / 2});
    } else {
        display_choice_fill_circle(renderer, preview.x + preview.w / 2 - 28,
                                   preview.y + preview.h / 2, 18, 255, 255, 255, 245);
        display_choice_fill_circle(renderer, preview.x + preview.w / 2 + 28,
                                   preview.y + preview.h / 2, 18, 255, 255, 255, 205);
    }

    label_box = (SDL_Rect){card.x, card.y + card.h - 54, card.w, 38};
    game_draw_center_text(renderer, font, title, hover ? white : soft, label_box);
}

void display_choice_draw_bg_toggle(SDL_Renderer *renderer, TTF_Font *font, SDL_Rect rect,
                                          const char *label, int hover, int selected) {
    SDL_Color text_color = selected ? (SDL_Color){255, 255, 255, 255} : (SDL_Color){220, 230, 238, 255};
    Uint8 base_r = selected ? 66 : 22;
    Uint8 base_g = selected ? 150 : 32;
    Uint8 base_b = selected ? 95 : 42;

    if (!renderer) return;

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, base_r, base_g, base_b, hover ? 240 : 195);
    SDL_RenderFillRect(renderer, &rect);
    SDL_SetRenderDrawColor(renderer, selected ? 255 : 188, selected ? 255 : 206, selected ? 255 : 216, 255);
    SDL_RenderDrawRect(renderer, &rect);

    game_draw_center_text(renderer, font, label, text_color, rect);
}

void display_choice_start_game(Game *game, int mode) {
    if (!game) return;
    game->duo_display_mode = mode;
    Game_ResetRuntime(game);
    Game_SetSubState(game, STATE_GAME);
}

void DisplayChoice_LectureEntree(Game *game) {
    SDL_Event e;

    display_choice_layout();
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            game->running = 0;
            return;
        }

        if (e.type == SDL_MOUSEMOTION) {
            int mx = e.motion.x;
            int my = e.motion.y;
            display_choice_hover_horizontal = start_play_point_in_rect(display_choice_horizontal_rect, mx, my);
            display_choice_hover_vertical = start_play_point_in_rect(display_choice_vertical_rect, mx, my);
            display_choice_hover_same = start_play_point_in_rect(display_choice_same_rect, mx, my);
            display_choice_hover_bg_fixed = start_play_point_in_rect(display_choice_bg_fixed_rect, mx, my);
            display_choice_hover_bg_scroll = start_play_point_in_rect(display_choice_bg_scroll_rect, mx, my);

            if (game->click &&
                ((display_choice_hover_horizontal && !display_choice_last_hover_horizontal) ||
                 (display_choice_hover_vertical && !display_choice_last_hover_vertical) ||
                 (display_choice_hover_same && !display_choice_last_hover_same) ||
                 (display_choice_hover_bg_fixed && !display_choice_last_hover_bg_fixed) ||
                 (display_choice_hover_bg_scroll && !display_choice_last_hover_bg_scroll))) {
                Mix_PlayChannel(-1, game->click, 0);
            }

            display_choice_last_hover_horizontal = display_choice_hover_horizontal;
            display_choice_last_hover_vertical = display_choice_hover_vertical;
            display_choice_last_hover_same = display_choice_hover_same;
            display_choice_last_hover_bg_fixed = display_choice_hover_bg_fixed;
            display_choice_last_hover_bg_scroll = display_choice_hover_bg_scroll;
        }

        if (e.type == SDL_KEYDOWN) {
            SDL_Keycode sym = e.key.keysym.sym;
            if (sym == SDLK_ESCAPE) {
                Game_SetSubState(game, STATE_PLAYER_CONFIG);
                return;
            }
            if (sym == SDLK_f || sym == SDLK_4 || sym == SDLK_KP_4) {
                game->duo_background_mode = DUO_BACKGROUND_FIXED;
                continue;
            }
            if (sym == SDLK_r || sym == SDLK_5 || sym == SDLK_KP_5) {
                game->duo_background_mode = DUO_BACKGROUND_SCROLLING;
                continue;
            }
            if (sym == SDLK_1 || sym == SDLK_KP_1 || sym == SDLK_h) {
                display_choice_start_game(game, DUO_DISPLAY_HORIZONTAL);
                return;
            }
            if (sym == SDLK_2 || sym == SDLK_KP_2 || sym == SDLK_v) {
                display_choice_start_game(game, DUO_DISPLAY_VERTICAL);
                return;
            }
            if (sym == SDLK_3 || sym == SDLK_KP_3 || sym == SDLK_s || sym == SDLK_RETURN) {
                display_choice_start_game(game, DUO_DISPLAY_SAME);
                return;
            }
        }

        if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
            int mx = e.button.x;
            int my = e.button.y;
            if (game->click) Mix_PlayChannel(-1, game->click, 0);
            if (start_play_point_in_rect(display_choice_bg_fixed_rect, mx, my)) {
                game->duo_background_mode = DUO_BACKGROUND_FIXED;
                continue;
            }
            if (start_play_point_in_rect(display_choice_bg_scroll_rect, mx, my)) {
                game->duo_background_mode = DUO_BACKGROUND_SCROLLING;
                continue;
            }
            if (start_play_point_in_rect(display_choice_horizontal_rect, mx, my)) {
                display_choice_start_game(game, DUO_DISPLAY_HORIZONTAL);
                return;
            }
            if (start_play_point_in_rect(display_choice_vertical_rect, mx, my)) {
                display_choice_start_game(game, DUO_DISPLAY_VERTICAL);
                return;
            }
            if (start_play_point_in_rect(display_choice_same_rect, mx, my)) {
                display_choice_start_game(game, DUO_DISPLAY_SAME);
                return;
            }
        }
    }
}

void DisplayChoice_MiseAJour(Game *game) {
    (void)game;
}

void DisplayChoice_Affichage(Game *game, SDL_Renderer *renderer) {
    SDL_Rect overlay = {0, 0, WIDTH, HEIGHT};
    SDL_Rect title_box = {(WIDTH - 760) / 2, 68, 760, 58};
    SDL_Rect hint_box = {(WIDTH - 960) / 2, 124, 960, 44};
    SDL_Rect bg_label_box = {(WIDTH - 420) / 2, 154, 420, 34};
    SDL_Color white = {255, 255, 255, 255};
    SDL_Color soft = {220, 230, 238, 255};

    if (!game || !renderer) return;
    display_choice_layout();

    if (game->gameBgTex)
        SDL_RenderCopy(renderer, game->gameBgTex, NULL, NULL);
    else if (game->ps_bg.texture)
        SDL_RenderCopy(renderer, game->ps_bg.texture, NULL, &game->ps_bg.dest_rect);
    else if (game->background)
        SDL_RenderCopy(renderer, game->background, NULL, NULL);

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 135);
    SDL_RenderFillRect(renderer, &overlay);

    game_draw_center_text(renderer, game->font, "Choose display mode", white, title_box);
    game_draw_center_text(renderer, game->font,
                          "Pick a display mode, then choose fixed or scrolling background",
                          soft, hint_box);
    game_draw_center_text(renderer, game->font, "Background behavior", soft, bg_label_box);

    display_choice_draw_bg_toggle(renderer, game->font, display_choice_bg_fixed_rect,
                                  "Fixed", display_choice_hover_bg_fixed,
                                  game->duo_background_mode == DUO_BACKGROUND_FIXED);
    display_choice_draw_bg_toggle(renderer, game->font, display_choice_bg_scroll_rect,
                                  "Scrolling", display_choice_hover_bg_scroll,
                                  game->duo_background_mode == DUO_BACKGROUND_SCROLLING);

    display_choice_draw_card(renderer, game->font, display_choice_horizontal_rect,
                             display_choice_hover_horizontal,
                             DUO_DISPLAY_HORIZONTAL, "Horizontal");
    display_choice_draw_card(renderer, game->font, display_choice_vertical_rect,
                             display_choice_hover_vertical,
                             DUO_DISPLAY_VERTICAL, "Vertical");
    display_choice_draw_card(renderer, game->font, display_choice_same_rect,
                             display_choice_hover_same,
                             DUO_DISPLAY_SAME, "Same Screen");

    game_draw_center_text(renderer, game->font, "1/2/3 select display | F fixed | R scrolling | ESC back",
                          soft, (SDL_Rect){(WIDTH - 860) / 2, HEIGHT - 54, 860, 36});
}

GameSpriteCacheEntry *game_find_sprite_entry(SDL_Texture *texture) {
    if (!texture) return NULL;
    for (int i = 0; i < GAME_SPRITE_CACHE_MAX; i++) {
        if (game_sprite_cache[i].texture == texture) return &game_sprite_cache[i];
    }
    return NULL;
}

void game_forget_sprite_texture(SDL_Texture *texture) {
    GameSpriteCacheEntry *entry = game_find_sprite_entry(texture);
    if (!entry) return;
    memset(entry, 0, sizeof(*entry));
}

GameSpriteCacheEntry *game_alloc_sprite_entry(SDL_Texture *texture) {
    GameSpriteCacheEntry *entry = NULL;

    if (!texture) return NULL;
    entry = game_find_sprite_entry(texture);
    if (entry) return entry;

    for (int i = 0; i < GAME_SPRITE_CACHE_MAX; i++) {
        if (!game_sprite_cache[i].texture) {
            game_sprite_cache[i].texture = texture;
            game_sprite_cache[i].frame_count = GAME_SPRITE_FRAME_COUNT;
            return &game_sprite_cache[i];
        }
    }
    return NULL;
}

SDL_Rect game_fallback_sprite_cell(SDL_Surface *surface, int frame_index) {
    int cell_w = surface ? surface->w / GAME_SHEET_COLS : 1;
    int cell_h = surface ? surface->h / GAME_SHEET_ROWS : 1;
    int frame = frame_index % GAME_SPRITE_FRAME_COUNT;
    if (frame < 0) frame += GAME_SPRITE_FRAME_COUNT;
    if (cell_w < 1) cell_w = 1;
    if (cell_h < 1) cell_h = 1;
    return (SDL_Rect){(frame % GAME_SHEET_COLS) * cell_w,
                      (frame / GAME_SHEET_COLS) * cell_h,
                      cell_w, cell_h};
}

void game_register_sprite_frames(SDL_Texture *texture, SDL_Surface *surface) {
    GameSpriteCacheEntry *entry;
    SDL_Surface *rgba;
    int cell_w;
    int cell_h;

    if (!texture || !surface) return;
    entry = game_alloc_sprite_entry(texture);
    if (!entry) return;

    for (int i = 0; i < GAME_SPRITE_FRAME_COUNT; i++) {
        entry->frames[i] = game_fallback_sprite_cell(surface, i);
    }

    rgba = SDL_ConvertSurfaceFormat(surface, SDL_PIXELFORMAT_RGBA32, 0);
    if (!rgba) return;

    cell_w = rgba->w / GAME_SHEET_COLS;
    cell_h = rgba->h / GAME_SHEET_ROWS;
    if (cell_w < 1 || cell_h < 1) {
        SDL_FreeSurface(rgba);
        return;
    }

    if (SDL_MUSTLOCK(rgba) && SDL_LockSurface(rgba) != 0) {
        SDL_FreeSurface(rgba);
        return;
    }

    for (int frame = 0; frame < GAME_SPRITE_FRAME_COUNT; frame++) {
        int base_x = (frame % GAME_SHEET_COLS) * cell_w;
        int base_y = (frame / GAME_SHEET_COLS) * cell_h;
        int min_x = cell_w;
        int min_y = cell_h;
        int max_x = -1;
        int max_y = -1;

        for (int y = 0; y < cell_h; y++) {
            Uint32 *row = (Uint32 *)((Uint8 *)rgba->pixels + (base_y + y) * rgba->pitch);
            for (int x = 0; x < cell_w; x++) {
                Uint8 r, g, b, a;
                SDL_GetRGBA(row[base_x + x], rgba->format, &r, &g, &b, &a);
                if (a > 16) {
                    if (x < min_x) min_x = x;
                    if (x > max_x) max_x = x;
                    if (y < min_y) min_y = y;
                    if (y > max_y) max_y = y;
                }
            }
        }

        if (max_x >= min_x && max_y >= min_y) {
            const int margin = 2;
            min_x -= margin;
            min_y -= margin;
            max_x += margin;
            max_y += margin;
            if (min_x < 0) min_x = 0;
            if (min_y < 0) min_y = 0;
            if (max_x >= cell_w) max_x = cell_w - 1;
            if (max_y >= cell_h) max_y = cell_h - 1;
            entry->frames[frame] = (SDL_Rect){
                base_x + min_x,
                base_y + min_y,
                max_x - min_x + 1,
                max_y - min_y + 1
            };
        }
    }

    if (SDL_MUSTLOCK(rgba)) SDL_UnlockSurface(rgba);
    SDL_FreeSurface(rgba);
}

SDL_Texture *game_load_sprite_texture(SDL_Renderer *renderer, const char *path) {
    SDL_Surface *surface;
    SDL_Texture *texture;

    if (!renderer || !path || !*path) return NULL;
    surface = IMG_Load(path);
    if (!surface) return NULL;

    texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture) game_register_sprite_frames(texture, surface);
    SDL_FreeSurface(surface);
    return texture;
}

SDL_Texture *game_load_sprite_texture_first(SDL_Renderer *renderer, const char *a, const char *b) {
    SDL_Texture *texture = game_load_sprite_texture(renderer, a);
    if (!texture) texture = game_load_sprite_texture(renderer, b);
    return texture;
}

void game_load_enemy_textures(Game *game, SDL_Renderer *renderer) {
    if (!game || !renderer) return;

    if (!game->gameEnemyStandTex) {
        game->gameEnemyStandTex =
            game_load_sprite_texture(renderer, "spritesheet_characters/kevin-stand_up.png");
    }
    if (!game->gameEnemyWalkTex) {
        game->gameEnemyWalkTex =
            game_load_sprite_texture(renderer, "spritesheet_characters/kevin-walk.png");
    }
    if (!game->gameEnemyRunTex) {
        game->gameEnemyRunTex =
            game_load_sprite_texture(renderer, "spritesheet_characters/kevin-run.png");
    }

    if (!game->gameEnemyTex &&
        !game->gameEnemyStandTex &&
        !game->gameEnemyWalkTex &&
        !game->gameEnemyRunTex) {
        game->gameEnemyTex = game_load_sprite_texture(renderer, "enemyobtacle/enemy.png");
    }
}

void game_load_obstacle_assets(Game *game, SDL_Renderer *renderer) {
    if (!game || !renderer) return;

    if (!game->gameFallingTex) {
        game->gameFallingTex = IMG_LoadTexture(renderer, "obstacles/snow_ball.png");
    }
}

int game_get_sprite_frame(SDL_Texture *texture, int frame_index, SDL_Rect *src) {
    GameSpriteCacheEntry *entry = game_find_sprite_entry(texture);
    int frame = frame_index % GAME_SPRITE_FRAME_COUNT;

    if (frame < 0) frame += GAME_SPRITE_FRAME_COUNT;
    if (entry && src) {
        *src = entry->frames[frame];
        return 1;
    }
    return 0;
}

void game_draw_normalized_frame(SDL_Renderer *renderer, SDL_Texture *tex,
                                       SDL_Rect src, SDL_Rect box) {
    SDL_Rect dst;

    if (!renderer || !tex || src.w <= 0 || src.h <= 0 || box.w <= 0 || box.h <= 0) return;

    dst.h = box.h;
    dst.w = (int)lround((double)src.w * ((double)dst.h / (double)src.h));
    if (dst.w < 1) dst.w = 1;
    if (dst.w > box.w) dst.w = box.w;
    dst.x = box.x + (box.w - dst.w) / 2;
    dst.y = box.y + box.h - dst.h;

    SDL_RenderCopy(renderer, tex, &src, &dst);
}

void game_draw_normalized_frame_flip(SDL_Renderer *renderer, SDL_Texture *tex,
                                            SDL_Rect src, SDL_Rect box,
                                            SDL_RendererFlip flip) {
    SDL_Rect dst;

    if (!renderer || !tex || src.w <= 0 || src.h <= 0 || box.w <= 0 || box.h <= 0) return;

    dst.h = box.h;
    dst.w = (int)lround((double)src.w * ((double)dst.h / (double)src.h));
    if (dst.w < 1) dst.w = 1;
    if (dst.w > box.w) dst.w = box.w;
    dst.x = box.x + (box.w - dst.w) / 2;
    dst.y = box.y + box.h - dst.h;

    SDL_RenderCopyEx(renderer, tex, &src, &dst, 0.0, NULL, flip);
}

SDL_Texture *game_minimap_player_texture(const Personnage *player,
                                                SDL_RendererFlip *flip) {
    SDL_Texture *texture = NULL;

    if (flip) *flip = SDL_FLIP_NONE;
    if (!player) return NULL;

    switch (player->movementState) {
        case GAME_MOVE_LAY_DOWN:
            texture = player->layDownTexture ? player->layDownTexture : player->damageTexture;
            break;
        case GAME_MOVE_TIRED:
            texture = player->tiredTexture ? player->tiredTexture : player->layDownTexture;
            break;
        case GAME_MOVE_PICKUP:
            texture = player->pickupTexture ? player->pickupTexture : player->idleTexture;
            if (player->facing < 0 && flip) *flip = SDL_FLIP_HORIZONTAL;
            break;
        case GAME_MOVE_DAMAGE:
            texture = player->damageTexture;
            break;
        case GAME_MOVE_JUMP_BACK:
            if (player->jumpBackTexture) texture = player->jumpBackTexture;
            else {
                texture = player->jumpTexture;
                if (flip) *flip = SDL_FLIP_HORIZONTAL;
            }
            break;
        case GAME_MOVE_JUMP:
            texture = player->jumpTexture;
            break;
        case GAME_MOVE_RUN_BACK:
            if (player->runBackTexture) texture = player->runBackTexture;
            else {
                texture = player->runTexture;
                if (flip) *flip = SDL_FLIP_HORIZONTAL;
            }
            break;
        case GAME_MOVE_RUN:
            texture = player->runTexture;
            break;
        case GAME_MOVE_WALK_BACK:
            if (player->walkBackTexture) texture = player->walkBackTexture;
            else {
                texture = player->walkTexture;
                if (flip) *flip = SDL_FLIP_HORIZONTAL;
            }
            break;
        case GAME_MOVE_WALK:
            texture = player->walkTexture;
            break;
        case GAME_MOVE_STOP:
        default:
            if (player->facing < 0 && player->idleBackTexture) texture = player->idleBackTexture;
            else {
                texture = player->idleTexture;
                if (player->facing < 0 && !player->idleBackTexture && flip) {
                    *flip = SDL_FLIP_HORIZONTAL;
                }
            }
            break;
    }

    if (!texture) texture = player->idleTexture ? player->idleTexture : player->walkTexture;
    return texture;
}

int game_get_sheet_cell_src(SDL_Texture *texture, int frame_index, SDL_Rect *src) {
    int tex_w = 0;
    int tex_h = 0;
    int frame_w;
    int frame_h;
    int frame;

    if (!texture || !src) return 0;
    if (game_get_sprite_frame(texture, frame_index, src)) return 1;

    if (SDL_QueryTexture(texture, NULL, NULL, &tex_w, &tex_h) != 0 ||
        tex_w <= 0 || tex_h <= 0) {
        return 0;
    }

    frame_w = tex_w / GAME_SHEET_COLS;
    frame_h = tex_h / GAME_SHEET_ROWS;
    if (frame_w <= 0 || frame_h <= 0) {
        *src = (SDL_Rect){0, 0, tex_w, tex_h};
        return 1;
    }

    frame = frame_index % GAME_SPRITE_FRAME_COUNT;
    if (frame < 0) frame += GAME_SPRITE_FRAME_COUNT;
    *src = (SDL_Rect){
        (frame % GAME_SHEET_COLS) * frame_w,
        (frame / GAME_SHEET_COLS) * frame_h,
        frame_w,
        frame_h
    };
    return 1;
}

void game_draw_minimap_player_marker(SDL_Renderer *renderer, const Personnage *player,
                                            SDL_Texture *fallback_texture,
                                            SDL_Rect marker, SDL_Color fallback,
                                            int move_dir, Uint32 now) {
    SDL_Rect draw = marker;
    SDL_Rect src;
    SDL_Texture *texture;
    SDL_RendererFlip flip = SDL_FLIP_NONE;

    if (!renderer) return;

    if (move_dir != 0) {
        int grow = 1 + (int)lround((sin((double)now / 95.0) + 1.0) * 1.5);
        int bob = (int)lround(sin((double)now / 80.0) * 1.5);
        draw.x -= grow / 2;
        draw.y -= (grow / 2) + bob;
        draw.w += grow;
        draw.h += grow;

        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        for (int i = 1; i <= 3; i++) {
            SDL_Rect t = draw;
            t.w -= i;
            t.h -= i;
            if (t.w < 2) t.w = 2;
            if (t.h < 2) t.h = 2;
            t.x -= move_dir * i * 3;
            t.y += i;
            SDL_SetRenderDrawColor(renderer, fallback.r, fallback.g, fallback.b,
                                   (Uint8)(150 - i * 35));
            SDL_RenderFillRect(renderer, &t);
        }

        SDL_SetRenderDrawColor(renderer, 255, 245, 140, 190);
        {
            int cx = draw.x + draw.w / 2;
            int cy = draw.y + draw.h / 2;
            int fx = cx + move_dir * (draw.w / 2 + 5);
            SDL_RenderDrawLine(renderer, cx, cy, fx, cy);
            SDL_RenderDrawLine(renderer, fx, cy, fx - move_dir * 3, cy - 2);
            SDL_RenderDrawLine(renderer, fx, cy, fx - move_dir * 3, cy + 2);
        }
    }

    texture = game_minimap_player_texture(player, &flip);
    if (texture && game_get_sheet_cell_src(texture, player ? player->frameIndex : 0, &src)) {
        game_draw_normalized_frame_flip(renderer, texture, src, draw, flip);
        return;
    }

    game_draw_minimap_marker(renderer, fallback_texture, draw, fallback);
}

void game_draw_sheet_frame(SDL_Renderer *renderer, SDL_Texture *tex, int frame_index,
                                  SDL_Rect crop, SDL_Rect dst_rect) {
    int tex_w = 0;
    int tex_h = 0;
    int frame_w;
    int frame_h;
    int frame_col;
    int frame_row;
    SDL_Rect src;

    if (!renderer || !tex) return;
    if (game_get_sprite_frame(tex, frame_index, &src)) {
        game_draw_normalized_frame(renderer, tex, src, dst_rect);
        return;
    }

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

    game_draw_normalized_frame(renderer, tex, src, dst_rect);
}

void game_draw_sheet_frame_reverse(SDL_Renderer *renderer, SDL_Texture *tex, int frame_index,
                                          SDL_Rect crop, SDL_Rect dst_rect) {
    int frame_count = GAME_SHEET_ROWS * GAME_SHEET_COLS;
    if (frame_count < 1) frame_count = 1;
    game_draw_sheet_frame(renderer, tex, frame_count - 1 - (frame_index % frame_count),
                          crop, dst_rect);
}

void game_draw_sheet_full_cell(SDL_Renderer *renderer, SDL_Texture *tex, int frame_index,
                                      SDL_Rect dst_rect) {
    int tex_w = 0;
    int tex_h = 0;
    int frame_w;
    int frame_h;
    int frame_col;
    int frame_row;
    SDL_Rect src;

    if (!renderer || !tex) return;
    if (game_get_sprite_frame(tex, frame_index, &src)) {
        game_draw_normalized_frame(renderer, tex, src, dst_rect);
        return;
    }

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
    game_draw_normalized_frame(renderer, tex, src, dst_rect);
}

void game_draw_sheet_full_cell_reverse(SDL_Renderer *renderer, SDL_Texture *tex,
                                              int frame_index, SDL_Rect dst_rect) {
    int frame_count = GAME_SHEET_ROWS * GAME_SHEET_COLS;
    if (frame_count < 1) frame_count = 1;
    game_draw_sheet_full_cell(renderer, tex, frame_count - 1 - (frame_index % frame_count),
                              dst_rect);
}

void game_destroy_character_textures(Personnage *p) {
    if (!p) return;
    if (p->idleTexture) {
        game_forget_sprite_texture(p->idleTexture);
        SDL_DestroyTexture(p->idleTexture);
    }
    if (p->idleBackTexture) {
        game_forget_sprite_texture(p->idleBackTexture);
        SDL_DestroyTexture(p->idleBackTexture);
    }
    if (p->walkTexture) {
        game_forget_sprite_texture(p->walkTexture);
        SDL_DestroyTexture(p->walkTexture);
    }
    if (p->walkBackTexture) {
        game_forget_sprite_texture(p->walkBackTexture);
        SDL_DestroyTexture(p->walkBackTexture);
    }
    if (p->runTexture) {
        game_forget_sprite_texture(p->runTexture);
        SDL_DestroyTexture(p->runTexture);
    }
    if (p->runBackTexture) {
        game_forget_sprite_texture(p->runBackTexture);
        SDL_DestroyTexture(p->runBackTexture);
    }
    if (p->jumpTexture) {
        game_forget_sprite_texture(p->jumpTexture);
        SDL_DestroyTexture(p->jumpTexture);
    }
    if (p->jumpBackTexture) {
        game_forget_sprite_texture(p->jumpBackTexture);
        SDL_DestroyTexture(p->jumpBackTexture);
    }
    if (p->damageTexture) {
        game_forget_sprite_texture(p->damageTexture);
        SDL_DestroyTexture(p->damageTexture);
    }
    if (p->layDownTexture) {
        game_forget_sprite_texture(p->layDownTexture);
        SDL_DestroyTexture(p->layDownTexture);
    }
    if (p->tiredTexture) {
        game_forget_sprite_texture(p->tiredTexture);
        SDL_DestroyTexture(p->tiredTexture);
    }
    if (p->pickupTexture) {
        game_forget_sprite_texture(p->pickupTexture);
        SDL_DestroyTexture(p->pickupTexture);
    }
    p->idleTexture = NULL;
    p->idleBackTexture = NULL;
    p->walkTexture = NULL;
    p->walkBackTexture = NULL;
    p->runTexture = NULL;
    p->runBackTexture = NULL;
    p->jumpTexture = NULL;
    p->jumpBackTexture = NULL;
    p->damageTexture = NULL;
    p->layDownTexture = NULL;
    p->tiredTexture = NULL;
    p->pickupTexture = NULL;
}

void game_load_harry_character(SDL_Renderer *renderer, Personnage *p) {
    if (!p) return;
    p->idleTexture = game_load_sprite_texture(renderer, "spritesheet_characters/mr_harry_stand_up.png");
    p->idleBackTexture = game_load_sprite_texture(renderer, "spritesheet_characters/mr_harry_stand_up_back.png");
    p->walkTexture = game_load_sprite_texture(renderer, "spritesheet_characters/mr_harry_walk_cycle_transparent.png");
    p->walkBackTexture = game_load_sprite_texture(renderer, "spritesheet_characters/mr_harry_walk_cycle_back_transparent.png");
    p->runTexture = game_load_sprite_texture(renderer, "spritesheet_characters/mr_harry_run.png");
    p->runBackTexture = game_load_sprite_texture(renderer, "spritesheet_characters/mr harry-run -reverse.png");
    p->jumpTexture = game_load_sprite_texture(renderer, "spritesheet_characters/mr_harry_jump_transparent.png");
    p->jumpBackTexture = game_load_sprite_texture(renderer, "spritesheet_characters/mr_harry_jump_back_transparent.png");
    p->damageTexture = game_load_sprite_texture(renderer, "spritesheet_characters/harry-damage.png");
    p->layDownTexture = game_load_sprite_texture(renderer, "spritesheet_characters/harry-lay_down.png");
    p->tiredTexture = game_load_sprite_texture(renderer, "spritesheet_characters/mr harry-kneel.png");
    p->pickupTexture = game_load_sprite_texture(renderer, "spritesheet_characters/mr harry-pickup.png");
}

void game_load_marvin_character(SDL_Renderer *renderer, Personnage *p) {
    if (!p) return;
    p->idleTexture = game_load_sprite_texture(renderer, "spritesheet_characters/marvin-stand-up.png");
    p->idleBackTexture = game_load_sprite_texture(renderer, "spritesheet_characters/marvin-stand-up-reverse.png");
    p->walkTexture = game_load_sprite_texture(renderer, "spritesheet_characters/marvin-walk-v1.png");
    p->walkBackTexture = game_load_sprite_texture_first(renderer,
        "spritesheet_characters/marvin-walk-v1-reverse.png",
        "spritesheet_characters/marvin-walk-v1 -reverse.png");
    p->runTexture = game_load_sprite_texture(renderer, "spritesheet_characters/marvin-run.png");
    p->runBackTexture = game_load_sprite_texture(renderer, "spritesheet_characters/marvin-run-reverse.png");
    p->jumpTexture = game_load_sprite_texture(renderer, "spritesheet_characters/marvin-jump.png");
    p->jumpBackTexture = game_load_sprite_texture_first(renderer,
        "spritesheet_characters/marvin-jump-reverse.png",
        "spritesheet_characters/marvin-jump -reverse.png");
    p->damageTexture = game_load_sprite_texture(renderer, "spritesheet_characters/marvin-damage.png");
    p->layDownTexture = game_load_sprite_texture(renderer, "spritesheet_characters/marvin-lay_down.png");
    p->tiredTexture = game_load_sprite_texture(renderer, "spritesheet_characters/marvin-kneel.png");
    p->pickupTexture = game_load_sprite_texture(renderer, "spritesheet_characters/marvin-pickup.png");
}

void game_state_reset_character(Personnage *p) {
    if (!p) return;

    p->position = (SDL_Rect){0, 0, GAME_CHARACTER_BOX_W, GAME_CHARACTER_BOX_H};
    p->groundY = HEIGHT - 70 - p->position.h;
    p->position.x = (WIDTH - p->position.w) / 2;
    p->position.y = p->groundY;
    p->up = 0;
    p->jumpPhase = 0;
    p->posinit = p->position.y;
    p->posinitX = p->position.x;
    p->jumpRelX = GAME_JUMP_REL_MIN;
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
    p->damageActive = 0;
    p->damageStartTick = 0;
    p->damageInvulnUntil = 0;
    p->pickupActive = 0;
    p->pickupStartTick = 0;
    p->pickupPendingSnowball = 0;
    p->hasSnowball = 0;
    game_reset_character_energy(p);
    game_jump_latch = 0;
}

void game_state_reset_character_at(Personnage *p, int x, int facing) {
    game_state_reset_character(p);
    if (!p) return;
    p->position.x = x;
    p->facing = facing;
    p->posinitX = p->position.x;
}

void game_set_character_ground_line(Personnage *p, int ground_line_y) {
    if (!p) return;
    p->groundY = ground_line_y - p->position.h;
    if (p->groundY < 0) p->groundY = 0;
    if (!p->up) p->position.y = p->groundY;
    p->posinit = p->position.y;
}

void game_set_movement(Personnage *p, int movement, Uint32 now) {
    if (!p) return;
    if (p->movementState != movement) {
        p->movementState = (int)movement;
        p->frameIndex = 0;
        p->lastFrameTick = now;
    }
}

void game_move_stop(Personnage *p, Uint32 now) {
    if (!p || p->up) return;
    p->moving = 0;
    game_set_movement(p, GAME_MOVE_STOP, now);
}

void game_move_walk(Personnage *p, int step, Uint32 now) {
    if (!p) return;
    p->position.x += step;
    p->facing = 1;
    p->moving = 1;
    if (!p->up) game_set_movement(p, GAME_MOVE_WALK, now);
}

void game_move_walk_back(Personnage *p, int step, Uint32 now) {
    if (!p) return;
    p->position.x -= step;
    p->facing = -1;
    p->moving = 1;
    if (!p->up) game_set_movement(p, GAME_MOVE_WALK_BACK, now);
}

void game_move_run(Personnage *p, int step, Uint32 now) {
    if (!p) return;
    p->position.x += step;
    p->facing = 1;
    p->moving = 1;
    if (!p->up) game_set_movement(p, GAME_MOVE_RUN, now);
}

void game_move_run_back(Personnage *p, int step, Uint32 now) {
    if (!p) return;
    p->position.x -= step;
    p->facing = -1;
    p->moving = 1;
    if (!p->up) game_set_movement(p, GAME_MOVE_RUN_BACK, now);
}

void game_move_jump(Personnage *p, Uint32 now) {
    if (!p || p->up) return;
    p->up = 1;
    p->jumpPhase = 1; /* 1 = montee, 2 = descente */
    p->moving = 0;
    p->facing = 1;
    p->posinitX = p->position.x;
    p->posinit = p->position.y;
    p->jumpRelX = GAME_JUMP_REL_MIN;
    p->jumpRelY = 0.0;
    p->jumpProgress = 0.0;
    p->jumpDir = 1;
    game_set_movement(p, GAME_MOVE_JUMP, now);
}

void game_move_jump_back(Personnage *p, Uint32 now) {
    if (!p || p->up) return;
    p->up = 1;
    p->jumpPhase = 1;
    p->moving = 0;
    p->facing = -1;
    p->posinitX = p->position.x;
    p->posinit = p->position.y;
    p->jumpRelX = GAME_JUMP_REL_MIN;
    p->jumpRelY = 0.0;
    p->jumpProgress = 0.0;
    p->jumpDir = -1;
    game_set_movement(p, GAME_MOVE_JUMP_BACK, now);
}

void game_move_jump_up(Personnage *p, Uint32 now) {
    if (!p || p->up) return;
    p->up = 1;
    p->jumpPhase = 1;
    p->moving = 0;
    p->posinitX = p->position.x;
    p->posinit = p->position.y;
    p->jumpRelX = GAME_JUMP_REL_MIN;
    p->jumpRelY = 0.0;
    p->jumpProgress = 0.0;
    p->jumpDir = 0;
    game_set_movement(p, (p->facing < 0) ? GAME_MOVE_JUMP_BACK : GAME_MOVE_JUMP, now);
}

void game_finish_jump(Personnage *p, Uint32 now) {
    if (!p) return;
    p->jumpRelX = GAME_JUMP_REL_MIN;
    p->jumpRelY = 0.0;
    p->jumpProgress = 0.0;
    p->up = 0;
    p->jumpPhase = 0;
    p->moving = 0;
    p->position.y = p->groundY;
    game_set_movement(p, GAME_MOVE_STOP, now);
}

void game_update_vertical_jump(Personnage *p, Uint32 dt, Uint32 now) {
    int step;
    int threshold;
    int height;
    int offset;

    if (!p || !p->up) return;

    height = p->jumpHeight;
    if (height < 1) height = 1;
    threshold = p->posinit - height;
    step = (int)lround((double)p->jumpSpeed * ((double)dt / 16.0));
    if (step < 1) step = 1;

    p->position.x = p->posinitX;
    if (p->jumpPhase != 2) {
        p->jumpPhase = 1;
        p->position.y -= step;
        if (p->position.y <= threshold) {
            p->position.y = threshold;
            p->jumpPhase = 2;
        }
    } else {
        p->position.y += step;
    }

    if (p->position.y >= p->posinit && p->jumpPhase == 2) {
        game_finish_jump(p, now);
        return;
    }

    offset = p->posinit - p->position.y;
    if (offset < 0) offset = 0;
    if (offset > height) offset = height;
    p->jumpRelX = 0.0;
    p->jumpRelY = (double)offset;
    p->jumpProgress = (double)offset / (double)height;
}

void game_update_jump(Personnage *p, Uint32 dt, Uint32 now) {
    double rel_range;
    double rel_step;
    int dir;

    if (!p || !p->up) return;

    if (p->jumpDir == 0) {
        game_update_vertical_jump(p, dt, now);
        return;
    }

    rel_range = GAME_JUMP_REL_MAX - GAME_JUMP_REL_MIN;
    rel_step = (rel_range * (double)dt) / GAME_JUMP_DURATION_MS;
    if (rel_step < 0.0) rel_step = 0.0;
    p->jumpRelX += rel_step;
    if (p->jumpRelX > GAME_JUMP_REL_MAX) p->jumpRelX = GAME_JUMP_REL_MAX;

    p->jumpRelY = (GAME_JUMP_PARABOLA_A * (p->jumpRelX * p->jumpRelX)) + GAME_JUMP_PARABOLA_C;
    if (p->jumpRelY < 0.0) p->jumpRelY = 0.0;
    p->jumpProgress = (p->jumpRelX - GAME_JUMP_REL_MIN) / rel_range;

    if (p->jumpRelX >= 0.0) p->jumpPhase = 2;

    dir = p->jumpDir;
    p->position.x = p->posinitX + (int)lround((p->jumpRelX - GAME_JUMP_REL_MIN) * dir);
    p->position.y = p->posinit - (int)lround(p->jumpRelY);

    if (p->jumpRelX >= GAME_JUMP_REL_MAX || p->position.y >= p->posinit) {
        game_finish_jump(p, now);
    }
}

int game_trigger_character_damage(Game *game, Personnage *p, int owner, Uint32 now) {
    if (!p) return 0;
    if (p->damageActive || now < p->damageInvulnUntil) return 0;

    p->damageActive = 1;
    p->damageStartTick = now;
    p->damageInvulnUntil = now + GAME_DAMAGE_INVULN_MS;
    p->up = 0;
    p->jumpPhase = 0;
    p->pendingJump = 0;
    p->moving = 0;
    p->position.y = p->groundY;
    if (game) {
        game_trigger_minimap_spark(owner, p->position, p->facing, now);
    }
    game_set_movement(p, GAME_MOVE_DAMAGE, now);
    return 1;
}

int game_update_damage_state(Personnage *p, Uint32 now) {
    Uint32 elapsed;

    if (!p || !p->damageActive) return 0;

    elapsed = now - p->damageStartTick;
    p->up = 0;
    p->jumpPhase = 0;
    p->pendingJump = 0;
    p->moving = 0;
    p->position.y = p->groundY;

    if (elapsed >= GAME_DAMAGE_MS + GAME_LAY_DOWN_MS) {
        p->damageActive = 0;
        game_set_movement(p, GAME_MOVE_STOP, now);
        return 0;
    }

    if (elapsed >= GAME_DAMAGE_MS) {
        game_set_movement(p, GAME_MOVE_LAY_DOWN, now);
    } else {
        game_set_movement(p, GAME_MOVE_DAMAGE, now);
    }

    game_move_animation(p, now);
    return 1;
}

void game_move_animation(Personnage *p, Uint32 now) {
    Uint32 frame_delay;
    int frame_count;

    if (!p) return;

    switch (p->movementState) {
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
        case GAME_MOVE_DAMAGE:
            frame_delay = 140u;
            frame_count = GAME_SHEET_ROWS * GAME_SHEET_COLS;
            break;
        case GAME_MOVE_LAY_DOWN:
            frame_delay = 180u;
            frame_count = GAME_SHEET_ROWS * GAME_SHEET_COLS;
            break;
        case GAME_MOVE_TIRED:
            frame_delay = 110u;
            frame_count = GAME_SHEET_ROWS * GAME_SHEET_COLS;
            break;
        case GAME_MOVE_PICKUP:
            frame_delay = 80u;
            frame_count = GAME_SHEET_ROWS * GAME_SHEET_COLS;
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

void game_update_character_controls(Personnage *p, int left_pressed, int right_pressed,
                                           int jump_pressed, int run_modifier,
                                           int walk_step, int run_step,
                                           Uint32 dt, Uint32 now,
                                           int owner_index) {
    int hit_border = 0;
    int has_move_input;
    int effective_run_modifier;

    if (!p) return;

    if (game_update_damage_state(p, now)) {
        game_update_character_stamina(p, 0, 0, dt, now);
        game_wrap_horizontal_position(&p->position.x, p->position.w, WIDTH);
        return;
    }

    if (game_update_pickup_state(p, now)) {
        game_update_character_stamina(p, 0, 0, dt, now);
        game_wrap_horizontal_position(&p->position.x, p->position.w, WIDTH);
        return;
    }

    has_move_input = ((left_pressed && !right_pressed) || (right_pressed && !left_pressed));
    effective_run_modifier = game_update_character_stamina(p, run_modifier, has_move_input, dt, now);
    if (game_character_is_tired(p, now)) {
        p->pendingJump = 0;
        p->up = 0;
        p->jumpPhase = 0;
        p->moving = 0;
        p->position.y = p->groundY;
        game_set_movement(p, GAME_MOVE_TIRED, now);
        game_wrap_horizontal_position(&p->position.x, p->position.w, WIDTH);
        game_move_animation(p, now);
        return;
    }

    if (jump_pressed && !p->up) {
        if (left_pressed && !right_pressed) game_move_jump_back(p, now);
        else if (right_pressed && !left_pressed) game_move_jump(p, now);
        else game_move_jump_up(p, now);
    }
    p->pendingJump = 0;

    if (p->up) {
        game_set_movement(p, (p->facing < 0) ? GAME_MOVE_JUMP_BACK : GAME_MOVE_JUMP, now);
        game_update_jump(p, dt, now);
    } else {
        p->position.y = p->groundY;
        p->moving = 0;
        if (left_pressed && !right_pressed) {
            if (effective_run_modifier) game_move_run_back(p, run_step, now);
            else game_move_walk_back(p, walk_step, now);
        } else if (right_pressed && !left_pressed) {
            if (effective_run_modifier) game_move_run(p, run_step, now);
            else game_move_walk(p, walk_step, now);
        } else {
            game_move_stop(p, now);
        }
    }

    game_wrap_horizontal_position(&p->position.x, p->position.w, WIDTH);
    if (p->position.y < 0) {
        p->position.y = 0;
        hit_border = 1;
    }
    if (p->position.y > p->groundY) {
        p->position.y = p->groundY;
        hit_border = 1;
    }

    if (hit_border && owner_index >= 0 && owner_index < 2 &&
        now >= game_minimap_border_cooldown[owner_index]) {
        game_trigger_minimap_spark(owner_index, p->position, p->facing, now);
        game_minimap_border_cooldown[owner_index] = now + 180u;
    }
    game_move_animation(p, now);
}

void game_draw_character(SDL_Renderer *renderer, Personnage *p) {
    if (!renderer || !p) return;

    switch (p->movementState) {
        case GAME_MOVE_LAY_DOWN:
            if (p->layDownTexture)
                game_draw_sheet_full_cell(renderer, p->layDownTexture,
                                          p->frameIndex, p->position);
            else if (p->damageTexture)
                game_draw_sheet_full_cell(renderer, p->damageTexture,
                                          p->frameIndex, p->position);
            else if (p->idleTexture)
                game_draw_sheet_full_cell(renderer, p->idleTexture, 0, p->position);
            break;
        case GAME_MOVE_DAMAGE:
            if (p->damageTexture)
                game_draw_sheet_full_cell(renderer, p->damageTexture,
                                          p->frameIndex, p->position);
            else if (p->idleTexture)
                game_draw_sheet_full_cell(renderer, p->idleTexture, 0, p->position);
            break;
        case GAME_MOVE_TIRED:
            if (p->tiredTexture)
                game_draw_sheet_full_cell(renderer, p->tiredTexture,
                                          p->frameIndex, p->position);
            else if (p->layDownTexture)
                game_draw_sheet_full_cell(renderer, p->layDownTexture,
                                          p->frameIndex, p->position);
            else if (p->idleTexture)
                game_draw_sheet_full_cell(renderer, p->idleTexture, 0, p->position);
            break;
        case GAME_MOVE_PICKUP:
            if (p->pickupTexture) {
                if (p->facing < 0) {
                    game_draw_sheet_full_cell_reverse(renderer, p->pickupTexture,
                                                      p->frameIndex, p->position);
                } else {
                    game_draw_sheet_full_cell(renderer, p->pickupTexture,
                                              p->frameIndex, p->position);
                }
            } else if (p->idleTexture) {
                game_draw_sheet_full_cell(renderer, p->idleTexture, 0, p->position);
            }
            break;
        case GAME_MOVE_JUMP_BACK:
            if (p->jumpBackTexture)
                game_draw_sheet_frame_reverse(renderer, p->jumpBackTexture,
                                              p->frameIndex, game_jump_back_crop,
                                              p->position);
            break;
        case GAME_MOVE_JUMP:
            if (p->jumpTexture)
                game_draw_sheet_frame(renderer, p->jumpTexture,
                                      p->frameIndex, game_jump_crop,
                                      p->position);
            break;
        case GAME_MOVE_RUN_BACK:
            if (p->runBackTexture)
                game_draw_sheet_full_cell(renderer, p->runBackTexture,
                                          p->frameIndex,
                                          p->position);
            else if (p->walkBackTexture)
                game_draw_sheet_frame_reverse(renderer, p->walkBackTexture,
                                              p->frameIndex, game_walk_back_crop,
                                              p->position);
            break;
        case GAME_MOVE_RUN:
            if (p->runTexture)
                game_draw_sheet_full_cell(renderer, p->runTexture,
                                          p->frameIndex,
                                          p->position);
            else if (p->walkTexture)
                game_draw_sheet_frame(renderer, p->walkTexture,
                                      p->frameIndex, game_walk_crop,
                                      p->position);
            break;
        case GAME_MOVE_WALK_BACK:
            if (p->walkBackTexture)
                game_draw_sheet_frame_reverse(renderer, p->walkBackTexture,
                                              p->frameIndex, game_walk_back_crop,
                                              p->position);
            break;
        case GAME_MOVE_WALK:
            if (p->walkTexture)
                game_draw_sheet_frame(renderer, p->walkTexture,
                                      p->frameIndex, game_walk_crop,
                                      p->position);
            break;
        case GAME_MOVE_STOP:
        default:
            {
                SDL_Texture *idle_tex = (p->facing < 0 && p->idleBackTexture)
                    ? p->idleBackTexture
                    : p->idleTexture;
                if (idle_tex) {
                    if (p->facing < 0 && p->idleBackTexture) {
                        game_draw_sheet_full_cell_reverse(renderer, idle_tex,
                                                          p->frameIndex,
                                                          p->position);
                    } else {
                        game_draw_sheet_full_cell(renderer, idle_tex,
                                                  p->frameIndex,
                                                  p->position);
                    }
                } else if (p->walkTexture) {
                    game_draw_sheet_frame(renderer, p->walkTexture,
                                          0, game_walk_crop, p->position);
                }
            }
            break;
    }
}

void game_seed_hazards_random_once(void) {
    if (!gameHazardsRngSeeded) {
        srand((unsigned int)time(NULL) ^ (unsigned int)SDL_GetTicks());
        gameHazardsRngSeeded = 1;
    }
}

Uint32 game_random_between_u32(Uint32 min_value, Uint32 max_value) {
    Uint32 span;

    if (max_value <= min_value) return min_value;
    span = max_value - min_value + 1u;
    return min_value + (Uint32)(rand() % (int)span);
}

void game_schedule_next_enigme(Uint32 now) {
    Uint32 delay;

    game_seed_hazards_random_once();
    delay = game_random_between_u32(GAME_ENIGME_DELAY_MIN_MS, GAME_ENIGME_DELAY_MAX_MS);
    game_enigme_intro_tick = now + delay;
    game_enigme_intro_shown = 0;
}

double game_rect_center_x(SDL_Rect rect) {
    return (double)rect.x + (double)rect.w / 2.0;
}

double game_rect_center_y(SDL_Rect rect) {
    return (double)rect.y + (double)rect.h / 2.0;
}

double game_wrapped_delta_x(double from_x, double to_x, int world_width) {
    double dx = to_x - from_x;
    double half_width;

    if (world_width <= 0) return dx;

    half_width = (double)world_width / 2.0;
    while (dx > half_width) dx -= (double)world_width;
    while (dx < -half_width) dx += (double)world_width;
    return dx;
}

double game_collision_circle_radius(SDL_Rect rect, double factor) {
    double half_w = (double)rect.w / 2.0;
    double half_h = (double)rect.h / 2.0;
    return sqrt((half_w * half_w) + (half_h * half_h)) * factor;
}

int game_collision_trigonometric(SDL_Rect a, SDL_Rect b) {
    double dx = game_rect_center_x(a) - game_rect_center_x(b);
    double dy = game_rect_center_y(a) - game_rect_center_y(b);
    double radius_sum = game_collision_circle_radius(a, 0.36) +
                        game_collision_circle_radius(b, 0.42);
    return (dx * dx + dy * dy) <= (radius_sum * radius_sum);
}

int game_collision_trigonometric_wrapped(SDL_Rect a, SDL_Rect b, int world_width) {
    double dx = game_wrapped_delta_x(game_rect_center_x(a), game_rect_center_x(b), world_width);
    double dy = game_rect_center_y(a) - game_rect_center_y(b);
    double radius_sum = game_collision_circle_radius(a, 0.36) +
                        game_collision_circle_radius(b, 0.42);
    return (dx * dx + dy * dy) <= (radius_sum * radius_sum);
}

void game_schedule_next_snowball_spawn(Uint32 now) {
    Uint32 delay;

    game_seed_hazards_random_once();
    delay = game_random_between_u32(GAME_SNOWBALL_SPAWN_MIN_MS,
                                    GAME_SNOWBALL_SPAWN_MAX_MS);
    game_next_snowball_spawn_tick = now + delay;
}

int game_find_free_snowball_slot(Game *game) {
    if (!game) return -1;

    for (int i = 0; i < GAME_OBSTACLE_COUNT; i++) {
        if (!game->gameObstacles[i].active) return i;
    }
    return -1;
}

void game_spawn_snowball(Game *game, int slot, int ground_line, Uint32 now) {
    GameObstacle *obs;
    int max_x;
    int max_y;

    if (!game || slot < 0 || slot >= GAME_OBSTACLE_COUNT || !game->gameFallingTex) return;

    obs = &game->gameObstacles[slot];
    memset(obs, 0, sizeof(*obs));

    max_x = WIDTH - GAME_SNOWBALL_BOX_W - 36;
    if (max_x < 36) max_x = 36;
    max_y = ground_line - GAME_SNOWBALL_BOX_H;
    if (max_y < 0) max_y = 0;

    obs->texture = game->gameFallingTex;
    obs->position = (SDL_Rect){
        (int)game_random_between_u32(36u, (Uint32)max_x),
        -(int)game_random_between_u32(120u, 320u),
        GAME_SNOWBALL_BOX_W,
        GAME_SNOWBALL_BOX_H
    };
    obs->active = 1;
    obs->speed = (int)game_random_between_u32(GAME_SNOWBALL_FALL_SPEED_MIN,
                                              GAME_SNOWBALL_FALL_SPEED_MAX);
    obs->baseY = max_y;
    obs->state = GAME_SNOWBALL_STATE_FALLING;
    obs->stateTick = now;
}

void game_update_snowball(GameObstacle *obs, Uint32 dt, int ground_line, Uint32 now) {
    int step;
    int ground_y;

    if (!obs || !obs->active || !obs->texture) return;
    if (obs->state != GAME_SNOWBALL_STATE_FALLING) return;

    step = (int)lround((double)obs->speed * ((double)dt / 16.0));
    if (step < 1) step = 1;

    ground_y = ground_line - obs->position.h;
    if (ground_y < 0) ground_y = 0;

    obs->position.y += step;
    if (obs->position.y >= ground_y) {
        obs->position.y = ground_y;
        obs->state = GAME_SNOWBALL_STATE_GROUNDED;
        obs->stateTick = now;
    }
}

double game_snowball_pickup_distance_sq(const Personnage *p, const GameObstacle *obs) {
    double dx;
    double dy;

    if (!p || !obs) return 1000000000.0;

    dx = game_rect_center_x(p->position) - game_rect_center_x(obs->position);
    dy = game_rect_center_y(p->position) - game_rect_center_y(obs->position);
    return dx * dx + dy * dy;
}

int game_player_near_grounded_snowball(const Personnage *p, const GameObstacle *obs) {
    SDL_Rect player_zone;
    SDL_Rect pickup_zone;

    if (!p || !obs || !obs->active || obs->state != GAME_SNOWBALL_STATE_GROUNDED) return 0;

    player_zone = p->position;
    player_zone.x -= 16;
    player_zone.w += 32;
    player_zone.y += player_zone.h / 3;
    player_zone.h -= player_zone.h / 3;
    if (player_zone.h < 24) player_zone.h = 24;

    pickup_zone = obs->position;
    pickup_zone.x -= 10;
    pickup_zone.y -= 8;
    pickup_zone.w += 20;
    pickup_zone.h += 16;

    if (SDL_HasIntersection(&player_zone, &pickup_zone)) return 1;
    return game_snowball_pickup_distance_sq(p, obs) <= 9800.0;
}

int game_find_pickable_snowball_slot(const Game *game, const Personnage *p, Uint32 now,
                                            double *distance_sq) {
    int best_slot = -1;
    double best_distance = 1000000000.0;

    if (distance_sq) *distance_sq = best_distance;
    if (!game || !p) return -1;
    if (p->hasSnowball || p->pickupActive || p->damageActive || game_character_is_tired(p, now)) {
        return -1;
    }

    for (int i = 0; i < GAME_OBSTACLE_COUNT; i++) {
        const GameObstacle *obs = &game->gameObstacles[i];
        double candidate_distance;

        if (!game_player_near_grounded_snowball(p, obs)) continue;

        candidate_distance = game_snowball_pickup_distance_sq(p, obs);
        if (candidate_distance < best_distance) {
            best_distance = candidate_distance;
            best_slot = i;
        }
    }

    if (distance_sq) *distance_sq = best_distance;
    return best_slot;
}

int game_trigger_character_pickup(Personnage *p, Uint32 now) {
    if (!p) return 0;
    if (p->hasSnowball || p->pickupActive || p->damageActive || game_character_is_tired(p, now)) {
        return 0;
    }

    p->pickupActive = 1;
    p->pickupStartTick = now;
    p->pickupPendingSnowball = 1;
    p->up = 0;
    p->jumpPhase = 0;
    p->pendingJump = 0;
    p->moving = 0;
    p->position.y = p->groundY;
    game_set_movement(p, GAME_MOVE_PICKUP, now);
    return 1;
}

int game_update_pickup_state(Personnage *p, Uint32 now) {
    Uint32 elapsed;

    if (!p || !p->pickupActive) return 0;

    elapsed = now - p->pickupStartTick;
    p->up = 0;
    p->jumpPhase = 0;
    p->pendingJump = 0;
    p->moving = 0;
    p->position.y = p->groundY;

    if (elapsed >= GAME_PICKUP_MS) {
        p->pickupActive = 0;
        if (p->pickupPendingSnowball) {
            p->hasSnowball = 1;
            p->pickupPendingSnowball = 0;
        }
        game_set_movement(p, GAME_MOVE_STOP, now);
        return 0;
    }

    game_set_movement(p, GAME_MOVE_PICKUP, now);
    game_move_animation(p, now);
    return 1;
}

int game_handle_snowball_pickup_input(Game *game, Uint32 now) {
    Personnage *target = NULL;
    int slot = -1;
    double p1_distance = 1000000000.0;
    double p2_distance = 1000000000.0;
    int p1_slot;
    int p2_slot = -1;

    if (!game) return 0;

    p1_slot = game_find_pickable_snowball_slot(game, &game->gameCharacter, now, &p1_distance);
    if (game->player_mode != 1) {
        p2_slot = game_find_pickable_snowball_slot(game, &game->gameCharacter2, now, &p2_distance);
    }

    if (p1_slot < 0 && p2_slot < 0) return 0;

    if (p2_slot >= 0 && (p1_slot < 0 || p2_distance < p1_distance)) {
        target = &game->gameCharacter2;
        slot = p2_slot;
    } else {
        target = &game->gameCharacter;
        slot = p1_slot;
    }

    if (!target || slot < 0 || slot >= GAME_OBSTACLE_COUNT) return 0;
    if (!game_trigger_character_pickup(target, now)) return 0;

    memset(&game->gameObstacles[slot], 0, sizeof(game->gameObstacles[slot]));
    return 1;
}

void game_reset_character_energy(Personnage *p) {
    if (!p) return;
    p->energy = GAME_ENERGY_MAX;
    p->tiredUntil = 0;
}

int game_character_is_tired(const Personnage *p, Uint32 now) {
    if (!p) return 0;
    return now < p->tiredUntil;
}

int game_update_character_stamina(Personnage *p, int wants_run, int has_move_input, Uint32 dt, Uint32 now) {
    double dt_sec;
    double fullness;
    double drain_per_sec;

    if (!p) return 0;

    if (p->energy < 0.0) p->energy = 0.0;
    if (p->energy > GAME_ENERGY_MAX) p->energy = GAME_ENERGY_MAX;

    dt_sec = (double)dt / 1000.0;
    if (dt_sec < 0.0) dt_sec = 0.0;

    if (game_character_is_tired(p, now)) {
        p->energy += GAME_ENERGY_RECHARGE_REST_PER_SEC * dt_sec;
        if (p->energy > GAME_ENERGY_MAX) p->energy = GAME_ENERGY_MAX;
        return 0;
    }

    if (wants_run && has_move_input && p->energy > 0.0) {
        fullness = p->energy / GAME_ENERGY_MAX;
        if (fullness < 0.0) fullness = 0.0;
        if (fullness > 1.0) fullness = 1.0;
        drain_per_sec = GAME_ENERGY_DRAIN_FAST_PER_SEC -
                        fullness * (GAME_ENERGY_DRAIN_FAST_PER_SEC - GAME_ENERGY_DRAIN_SLOW_PER_SEC);
        p->energy -= drain_per_sec * dt_sec;
        if (p->energy <= 0.0) {
            p->energy = 0.0;
            p->tiredUntil = now + GAME_ENERGY_TIRED_MS;
            p->frameIndex = 0;
            p->lastFrameTick = now;
            return 0;
        }
        return 1;
    }

    p->energy += GAME_ENERGY_RECHARGE_PER_SEC * dt_sec;
    if (p->energy > GAME_ENERGY_MAX) p->energy = GAME_ENERGY_MAX;
    return 0;
}

int game_character_moving_right(const Personnage *p) {
    if (!p || p->damageActive) return 0;
    return p->movementState == GAME_MOVE_WALK ||
           p->movementState == GAME_MOVE_RUN ||
           p->movementState == GAME_MOVE_JUMP;
}

int game_character_moving_back(const Personnage *p) {
    if (!p || p->damageActive) return 0;
    return p->movementState == GAME_MOVE_WALK_BACK ||
           p->movementState == GAME_MOVE_RUN_BACK ||
           p->movementState == GAME_MOVE_JUMP_BACK;
}

void game_init_obstacle(GameObstacle *obs, SDL_Texture *texture,
                               int x, int y, int w, int h, int speed) {
    if (!obs) return;

    game_seed_hazards_random_once();
    obs->texture = texture;
    obs->position = (SDL_Rect){x, y, w, h};
    obs->active = (texture != NULL);
    obs->collidingPlayer1 = 0;
    obs->collidingPlayer2 = 0;
    obs->direction = (rand() % 2 == 0) ? GAME_OBSTACLE_RIGHT : GAME_OBSTACLE_LEFT;
    obs->minX = 40;
    obs->maxX = WIDTH - w - 40;
    if (obs->maxX < obs->minX) obs->maxX = obs->minX;
    obs->speed = speed;
    obs->baseY = y;
    obs->amplitude = 18.0 + (double)(rand() % 28);
    obs->frequency = 0.012 + ((double)(rand() % 20) / 1000.0);
    obs->phase = (double)(rand() % 628) / 100.0;
}

SDL_Texture *game_enemy_texture_for_state(GameEnemy *enemy, int animation_state) {
    if (!enemy) return NULL;
    if (animation_state == GAME_ENEMY_ANIM_RUN && enemy->runTexture) return enemy->runTexture;
    if (animation_state == GAME_ENEMY_ANIM_WALK && enemy->walkTexture) return enemy->walkTexture;
    if (animation_state == GAME_ENEMY_ANIM_STAND && enemy->standTexture) return enemy->standTexture;
    if (enemy->standTexture) return enemy->standTexture;
    if (enemy->walkTexture) return enemy->walkTexture;
    if (enemy->runTexture) return enemy->runTexture;
    return enemy->texture;
}

Uint32 game_enemy_frame_delay(int animation_state) {
    if (animation_state == GAME_ENEMY_ANIM_RUN) return GAME_ENEMY_RUN_FRAME_MS;
    if (animation_state == GAME_ENEMY_ANIM_WALK) return GAME_ENEMY_WALK_FRAME_MS;
    return GAME_ENEMY_STAND_FRAME_MS;
}

void game_enemy_refresh_frame_geometry(GameEnemy *enemy) {
    int tex_w = 0;
    int tex_h = 0;

    if (!enemy) return;

    enemy->nbCols = GAME_ENEMY_SHEET_COLS;
    enemy->nbRows = GAME_ENEMY_SHEET_ROWS;
    enemy->frameWidth = 1;
    enemy->frameHeight = 1;

    if (enemy->texture &&
        SDL_QueryTexture(enemy->texture, NULL, NULL, &tex_w, &tex_h) == 0 &&
        tex_w > 0 && tex_h > 0) {
        enemy->frameWidth = tex_w / GAME_ENEMY_SHEET_COLS;
        enemy->frameHeight = tex_h / GAME_ENEMY_SHEET_ROWS;
        if (enemy->frameWidth < 1) enemy->frameWidth = tex_w;
        if (enemy->frameHeight < 1) enemy->frameHeight = tex_h;
    }
    enemy->sprite = (SDL_Rect){0, 0, enemy->frameWidth, enemy->frameHeight};
}

void game_enemy_update_sprite(GameEnemy *enemy) {
    SDL_Rect src;
    int frame_count;
    int frame;

    if (!enemy) return;

    frame_count = enemy->nbCols * enemy->nbRows;
    if (frame_count < 1) frame_count = 1;
    frame = enemy->frame % frame_count;
    if (frame < 0) frame += frame_count;

    if (game_get_sprite_frame(enemy->texture, frame, &src)) {
        enemy->sprite = src;
        return;
    }

    enemy->sprite = (SDL_Rect){
        (frame % enemy->nbCols) * enemy->frameWidth,
        (frame / enemy->nbCols) * enemy->frameHeight,
        enemy->frameWidth,
        enemy->frameHeight
    };
}

void game_enemy_set_animation(GameEnemy *enemy, int animation_state, Uint32 now) {
    SDL_Texture *next_texture;

    if (!enemy) return;

    next_texture = game_enemy_texture_for_state(enemy, animation_state);
    if (!next_texture) {
        enemy->active = 0;
        return;
    }

    if (enemy->texture != next_texture || enemy->animationState != animation_state) {
        enemy->texture = next_texture;
        enemy->animationState = animation_state;
        enemy->frame = 0;
        enemy->lastFrameTick = now;
        game_enemy_refresh_frame_geometry(enemy);
    }
    enemy->active = 1;
    game_enemy_update_sprite(enemy);
}

void game_draw_enemy_sprite(SDL_Renderer *renderer, const GameEnemy *enemy,
                                   SDL_Rect dst_rect) {
    SDL_RendererFlip flip;

    if (!renderer || !enemy || !enemy->texture || dst_rect.w <= 0 || dst_rect.h <= 0) return;

    flip = (enemy->direction == GAME_OBSTACLE_LEFT) ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE;
    game_draw_normalized_frame_flip(renderer, enemy->texture, enemy->sprite, dst_rect, flip);
}

void game_reset_enemy_obstacles(Game *game) {
    int ground_line;

    if (!game) return;

    ground_line = game->gameCharacter.groundY + game->gameCharacter.position.h;
    if (ground_line <= 0) ground_line = GAME_MONO_GROUND_LINE_Y;

    memset(&game->gameEnemy, 0, sizeof(game->gameEnemy));
    game->gameEnemy.standTexture = game->gameEnemyStandTex ? game->gameEnemyStandTex : game->gameEnemyTex;
    game->gameEnemy.walkTexture = game->gameEnemyWalkTex ? game->gameEnemyWalkTex : game->gameEnemy.standTexture;
    game->gameEnemy.runTexture = game->gameEnemyRunTex ? game->gameEnemyRunTex : game->gameEnemy.walkTexture;
    game->gameEnemy.texture = game_enemy_texture_for_state(&game->gameEnemy,
                                                           GAME_ENEMY_ANIM_STAND);
    game->gameEnemy.active = (game->gameEnemy.texture != NULL);
    game->gameEnemy.position = (SDL_Rect){WIDTH - 360,
                                          ground_line - GAME_ENEMY_BOX_H,
                                          GAME_ENEMY_BOX_W,
                                          GAME_ENEMY_BOX_H};
    game->gameEnemy.direction = GAME_OBSTACLE_RIGHT;
    game->gameEnemy.animationState = GAME_ENEMY_ANIM_STAND;
    game->gameEnemy.lastFrameTick = SDL_GetTicks();
    game_enemy_refresh_frame_geometry(&game->gameEnemy);
    game_enemy_update_sprite(&game->gameEnemy);
    for (int i = 0; i < GAME_OBSTACLE_COUNT; i++) {
        memset(&game->gameObstacles[i], 0, sizeof(game->gameObstacles[i]));
    }
    game_schedule_next_snowball_spawn(SDL_GetTicks());
}

void game_update_enemy(Game *game, Uint32 dt, Uint32 now) {
    GameEnemy *enemy;
    Personnage *players[2];
    int player_count;
    int any_player_moving = 0;
    int move_dir = 0;
    int speed = 0;
    int step;
    int animation_state = GAME_ENEMY_ANIM_STAND;
    int frame_count;
    Uint32 frame_delay;
    double enemy_center;
    double nearest_delta_x = 0.0;
    double nearest_distance = 1000000.0;

    if (!game) return;
    enemy = &game->gameEnemy;
    if (!enemy->active || !enemy->texture) return;

    players[0] = &game->gameCharacter;
    players[1] = (game->player_mode != 1) ? &game->gameCharacter2 : NULL;
    player_count = (game->player_mode != 1) ? 2 : 1;
    enemy_center = game_rect_center_x(enemy->position);

    for (int i = 0; i < player_count; i++) {
        Personnage *p = players[i];
        double px;
        double dx;
        double dist;
        int moving;
        if (!p) continue;
        px = game_rect_center_x(p->position);
        dx = game_wrapped_delta_x(enemy_center, px, WIDTH);
        dist = fabs(dx);
        moving = game_character_moving_right(p) || game_character_moving_back(p);
        if (moving) any_player_moving = 1;
        if (dist < nearest_distance) {
            nearest_distance = dist;
            nearest_delta_x = dx;
        }
    }

    if (any_player_moving || nearest_distance <= GAME_ENEMY_ALERT_DISTANCE) {
        move_dir = (nearest_delta_x >= 0.0) ? -1 : 1;
    }

    if (move_dir != 0) {
        speed = (nearest_distance <= GAME_ENEMY_ALERT_DISTANCE)
            ? GAME_ENEMY_FLEE_SPEED
            : GAME_ENEMY_FOLLOW_SPEED;
        if (nearest_distance < GAME_ENEMY_KEEP_DISTANCE) {
            double pressure = (double)(GAME_ENEMY_KEEP_DISTANCE - nearest_distance);
            speed += (int)lround(pressure / 18.0);
        }
        animation_state = (nearest_distance < GAME_ENEMY_KEEP_DISTANCE || speed >= GAME_ENEMY_FLEE_SPEED)
            ? GAME_ENEMY_ANIM_RUN
            : GAME_ENEMY_ANIM_WALK;
        step = (int)lround((double)speed * ((double)dt / 16.0));
        if (step < 1) step = 1;
        enemy->position.x += move_dir * step;
        enemy->direction = (move_dir > 0) ? GAME_OBSTACLE_RIGHT : GAME_OBSTACLE_LEFT;
    }

    game_enemy_set_animation(enemy, animation_state, now);
    if (!enemy->active || !enemy->texture) return;

    frame_count = enemy->nbCols * enemy->nbRows;
    if (frame_count < 1) frame_count = 1;
    frame_delay = game_enemy_frame_delay(enemy->animationState);
    if (now - enemy->lastFrameTick >= frame_delay) {
        enemy->frame = (enemy->frame + 1) % frame_count;
        enemy->lastFrameTick = now;
    }

    game_wrap_horizontal_position(&enemy->position.x, enemy->position.w, WIDTH);
    enemy->position.y = game->gameCharacter.groundY + game->gameCharacter.position.h - enemy->position.h;
    if (enemy->position.y < 0) enemy->position.y = 0;
    game_enemy_update_sprite(enemy);
}

void game_update_obstacle_motion(GameObstacle *obs, Uint32 dt, int ground_line) {
    int step;
    int max_y;

    if (!obs || !obs->active) return;

    if (obs->position.x >= obs->maxX) obs->direction = GAME_OBSTACLE_LEFT;
    if (obs->position.x <= obs->minX) obs->direction = GAME_OBSTACLE_RIGHT;

    step = (int)lround((double)obs->speed * ((double)dt / 16.0));
    if (step < 1) step = 1;
    if (obs->direction == GAME_OBSTACLE_RIGHT) obs->position.x += step;
    else obs->position.x -= step;
    obs->position.x = game_clampi(obs->position.x, obs->minX, obs->maxX);

    max_y = ground_line - obs->position.h;
    if (max_y < 0) max_y = 0;
    obs->position.y = obs->baseY +
        (int)lround(sin((double)obs->position.x * obs->frequency + obs->phase) *
                    obs->amplitude);
    obs->position.y = game_clampi(obs->position.y, 40, max_y);
}

void game_update_obstacle_player_collision(Game *game, GameObstacle *obs,
                                                  Personnage *player,
                                                  int *colliding_flag,
                                                  Uint32 now) {
    int colliding;

    if (!game || !obs || !obs->active || !player || !colliding_flag) return;

    colliding = game_collision_trigonometric(player->position, obs->position);
    if (colliding) {
        if (!*colliding_flag &&
            game_trigger_character_damage(game, player,
                                          (player == &game->gameCharacter2) ? 1 : 0,
                                          now) &&
            game->gameObstacleHitSound) {
            Mix_PlayChannel(-1, game->gameObstacleHitSound, 0);
        }
        *colliding_flag = 1;
    } else {
        *colliding_flag = 0;
    }
}

void game_update_world_hazards_motion(Game *game, Uint32 dt, Uint32 now) {
    int ground_line;
    int slot;

    if (!game) return;

    game_update_enemy(game, dt, now);

    ground_line = game->gameCharacter.groundY + game->gameCharacter.position.h;
    if (ground_line <= 0) ground_line = GAME_MONO_GROUND_LINE_Y;

    if (game->gameFallingTex && game_next_snowball_spawn_tick != 0 &&
        now >= game_next_snowball_spawn_tick) {
        slot = game_find_free_snowball_slot(game);
        if (slot >= 0) {
            game_spawn_snowball(game, slot, ground_line, now);
        }
        game_schedule_next_snowball_spawn(now);
    }

    for (int i = 0; i < GAME_OBSTACLE_COUNT; i++) {
        game_update_snowball(&game->gameObstacles[i], dt, ground_line, now);
    }
}

void game_update_enemy_player_minimap_collision(Game *game, Personnage *player,
                                                       int owner_index, Uint32 now) {
    int colliding;

    if (!game || !player) return;
    if (owner_index < 0 || owner_index > 1) return;
    if (!game->gameEnemy.active || !game->gameEnemy.texture) {
        game_enemy_collision_latch[owner_index] = 0;
        return;
    }

    colliding = game_collision_trigonometric_wrapped(player->position, game->gameEnemy.position, WIDTH);
    if (colliding) {
        if (!game_enemy_collision_latch[owner_index]) {
            game_trigger_minimap_spark(owner_index, player->position, player->facing, now);
            game_enemy_collision_latch[owner_index] = 1;
        }
    } else {
        game_enemy_collision_latch[owner_index] = 0;
    }
}

void game_update_world_hazards(Game *game, Uint32 dt, Uint32 now) {
    if (!game) return;

    game_update_world_hazards_motion(game, dt, now);
    game_update_enemy_player_minimap_collision(game, &game->gameCharacter, 0, now);
    if (game->player_mode != 1) {
        game_update_enemy_player_minimap_collision(game, &game->gameCharacter2, 1, now);
    } else {
        game_enemy_collision_latch[1] = 0;
    }
}

void game_render_world_hazards(Game *game, SDL_Renderer *renderer) {
    if (!game || !renderer) return;

    if (game->gameEnemy.active && game->gameEnemy.texture) {
        game_draw_enemy_sprite(renderer, &game->gameEnemy, game->gameEnemy.position);
    }

    for (int i = 0; i < GAME_OBSTACLE_COUNT; i++) {
        GameObstacle *obs = &game->gameObstacles[i];
        if (!obs->active || !obs->texture) continue;
        SDL_RenderCopy(renderer, obs->texture, NULL, &obs->position);
    }
}

SDL_Rect game_world_rect_to_panel_rect(SDL_Rect world_rect,
                                              SDL_Rect viewport,
                                              double scale,
                                              int panel_ground_line_y,
                                              int world_ground_line) {
    SDL_Rect dst;
    int world_span;
    int viewport_span;
    int bottom_offset;
    double x_scale;

    dst.w = (int)lround((double)world_rect.w * scale);
    dst.h = (int)lround((double)world_rect.h * scale);
    if (dst.w < 16) dst.w = 16;
    if (dst.h < 16) dst.h = 16;

    world_span = WIDTH - world_rect.w;
    viewport_span = viewport.w - dst.w;
    if (world_span < 1) world_span = 1;
    if (viewport_span < 0) viewport_span = 0;
    x_scale = (double)viewport_span / (double)world_span;
    dst.x = (int)lround((double)world_rect.x * x_scale);

    bottom_offset = world_ground_line - (world_rect.y + world_rect.h);
    if (bottom_offset < 0) bottom_offset = 0;
    dst.y = panel_ground_line_y - dst.h - (int)lround((double)bottom_offset * scale);

    return dst;
}

void game_draw_world_texture_in_panel(SDL_Renderer *renderer,
                                             SDL_Texture *texture,
                                             const SDL_Rect *src,
                                             SDL_Rect world_rect,
                                             SDL_Rect viewport,
                                             double scale,
                                             int panel_ground_line_y,
                                             int world_ground_line) {
    SDL_Rect dst;

    if (!renderer || !texture) return;

    dst = game_world_rect_to_panel_rect(world_rect, viewport, scale,
                                        panel_ground_line_y, world_ground_line);
    SDL_RenderCopy(renderer, texture, src, &dst);
}

void game_draw_enemy_in_panel(SDL_Renderer *renderer,
                                     const GameEnemy *enemy,
                                     SDL_Rect viewport,
                                     double scale,
                                     int panel_ground_line_y,
                                     int world_ground_line) {
    SDL_Rect dst;

    if (!renderer || !enemy || !enemy->active || !enemy->texture) return;

    dst = game_world_rect_to_panel_rect(enemy->position, viewport, scale,
                                        panel_ground_line_y, world_ground_line);
    game_draw_enemy_sprite(renderer, enemy, dst);
}

void game_render_hazards_in_panel(Game *game, SDL_Renderer *renderer,
                                         SDL_Rect viewport, double scale,
                                         int panel_ground_line_y) {
    int world_ground_line;

    if (!game || !renderer) return;

    world_ground_line = game->gameCharacter.groundY + game->gameCharacter.position.h;
    if (world_ground_line <= 0) world_ground_line = GAME_MONO_GROUND_LINE_Y;

    if (game->gameEnemy.active && game->gameEnemy.texture) {
        game_draw_enemy_in_panel(renderer, &game->gameEnemy,
                                 viewport, scale,
                                 panel_ground_line_y,
                                 world_ground_line);
    }

    for (int i = 0; i < GAME_OBSTACLE_COUNT; i++) {
        GameObstacle *obs = &game->gameObstacles[i];
        if (!obs->active || !obs->texture) continue;
        game_draw_world_texture_in_panel(renderer, obs->texture, NULL,
                                         obs->position, viewport, scale,
                                         panel_ground_line_y, world_ground_line);
    }
}

void duo_reset_runtime(Game *game) {
    if (!game) return;
    game_state_reset_character_at(&game->gameCharacter, WIDTH / 2 - 180, 1);
    game_state_reset_character_at(&game->gameCharacter2, WIDTH / 2 + 90, -1);
    duoStartTime = SDL_GetTicks();
}

void duo_update_runtime(Game *game, Uint32 dt, Uint32 now) {
    const Uint8 *keys = SDL_GetKeyboardState(NULL);
    int walk_step;
    int run_step;
    int p1_left;
    int p1_right;
    int p1_jump;
    int p1_run;
    int p2_left;
    int p2_right;
    int p2_jump;
    int p2_run;

    if (!game) return;

    walk_step = (int)lround((double)game->gameCharacter.moveSpeed * ((double)dt / 16.0));
    run_step = (int)lround((double)(game->gameCharacter.moveSpeed + GAME_RUN_SPEED_BONUS) * ((double)dt / 16.0));
    if (walk_step < 1) walk_step = 1;
    if (run_step < 1) run_step = 1;

    p1_left = game_key_pressed(keys, game->keyBindings[0][KEY_ACTION_DOWN]);
    p1_right = game_key_pressed(keys, game->keyBindings[0][KEY_ACTION_WALK]);
    p1_jump = game->gameCharacter.pendingJump;
    p1_run = game_key_pressed(keys, game->keyBindings[0][KEY_ACTION_RUN]);

    p2_left = game_key_pressed(keys, game->keyBindings[1][KEY_ACTION_DOWN]);
    p2_right = game_key_pressed(keys, game->keyBindings[1][KEY_ACTION_WALK]);
    p2_jump = game->gameCharacter2.pendingJump;
    p2_run = game_key_pressed(keys, game->keyBindings[1][KEY_ACTION_RUN]);
    if (game_player_uses_mouse(game, 0)) {
        game_mouse_direction_for_rect(game->gameCharacter.position,
                                      &p1_left,
                                      &p1_right,
                                      &p1_run);
    }
    if (game_player_uses_mouse(game, 1)) {
        game_mouse_direction_for_rect(game->gameCharacter2.position,
                                      &p2_left,
                                      &p2_right,
                                      &p2_run);
    }

    game_update_character_controls(&game->gameCharacter,
                                   p1_left,
                                   p1_right,
                                   p1_jump,
                                   p1_run,
                                   walk_step, run_step, dt, now, 0);
    game_update_character_controls(&game->gameCharacter2,
                                   p2_left,
                                   p2_right,
                                   p2_jump,
                                   p2_run,
                                   walk_step, run_step, dt, now, 1);
}

double duo_panel_scale(SDL_Rect viewport) {
    double scale_x = (double)viewport.w / (double)WIDTH;
    double scale_y = (double)viewport.h / (double)HEIGHT;
    double scale = (scale_x < scale_y) ? scale_x : scale_y;
    if (scale <= 0.0) scale = 1.0;
    if (scale > 1.0) scale = 1.0;
    return scale;
}

void duo_render_panel_background(Game *game, SDL_Renderer *renderer, SDL_Rect viewport,
                                        double scale, int focus_x, int focus_y, int *ground_line_y) {
    SDL_Rect bg_dst = {0, 0, viewport.w, viewport.h};
    int line_y = viewport.h - (int)lround(70.0 * scale);
    SDL_Rect ground_rect;

    if (line_y < viewport.h / 2) line_y = viewport.h - 45;
    if (line_y > viewport.h) line_y = viewport.h;
    ground_rect = (SDL_Rect){0, line_y, viewport.w, viewport.h - line_y};

    game_render_background(game, renderer, bg_dst, focus_x, focus_y);

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 18, 18, 18, 170);
    SDL_RenderFillRect(renderer, &ground_rect);
    SDL_SetRenderDrawColor(renderer, 255, 214, 10, 255);
    SDL_RenderDrawLine(renderer, 0, line_y, viewport.w, line_y);

    if (ground_line_y) *ground_line_y = line_y;
}

void duo_draw_character_in_panel(SDL_Renderer *renderer, Personnage *p,
                                        SDL_Rect viewport, double scale, int ground_line_y) {
    SDL_Rect saved_position;
    int draw_w;
    int draw_h;
    int jump_offset;
    int world_span;
    int viewport_span;
    double x_scale;

    if (!renderer || !p) return;

    saved_position = p->position;
    draw_w = (int)lround((double)saved_position.w * scale);
    draw_h = (int)lround((double)saved_position.h * scale);
    if (draw_w < 24) draw_w = 24;
    if (draw_h < 48) draw_h = 48;

    jump_offset = p->groundY - saved_position.y;
    if (jump_offset < 0) jump_offset = 0;

    p->position.w = draw_w;
    p->position.h = draw_h;

    world_span = WIDTH - saved_position.w;
    viewport_span = viewport.w - draw_w;
    if (world_span < 1) world_span = 1;
    if (viewport_span < 0) viewport_span = 0;
    x_scale = (double)viewport_span / (double)world_span;

    p->position.x = (int)lround((double)saved_position.x * x_scale);
    p->position.y = ground_line_y - draw_h - (int)lround((double)jump_offset * scale);
    game_draw_character(renderer, p);
    p->position = saved_position;
}

void duo_render_player_panel(Game *game, SDL_Renderer *renderer, SDL_Rect viewport,
                                    Personnage *p, SDL_Texture *life_icon) {
    double scale;
    int ground_line_y = viewport.h;
    int focus_x;
    int focus_y;
    int world_ground_line;
    SDL_Rect draw_rect;

    if (!game || !renderer || !p) return;

    scale = duo_panel_scale(viewport);
    focus_x = p->position.x + p->position.w / 2;
    focus_y = p->position.y + p->position.h / 2;
    SDL_RenderSetViewport(renderer, &viewport);
    duo_render_panel_background(game, renderer, viewport, scale, focus_x, focus_y, &ground_line_y);
    game_render_hazards_in_panel(game, renderer, viewport, scale, ground_line_y);
    duo_draw_character_in_panel(renderer, p, viewport, scale, ground_line_y);
    world_ground_line = game->gameCharacter.groundY + game->gameCharacter.position.h;
    if (world_ground_line <= 0) world_ground_line = GAME_MONO_GROUND_LINE_Y;
    draw_rect = game_world_rect_to_panel_rect(p->position, viewport, scale,
                                              ground_line_y, world_ground_line);
    game_render_pickup_prompt_for_player(game, renderer, p, draw_rect);

    if (life_icon) {
        SDL_Rect icon_dst = {18, 10, 60, 60};
        SDL_RenderCopy(renderer, life_icon, NULL, &icon_dst);
    }
    game_draw_snowball_inventory(game, renderer, p, 30, 78, 34);
    if (game->font) duo_render_time(game, renderer, 92, 24);
    game_draw_energy_bar(game, renderer, p, 18, 118, viewport.w > 210 ? 180 : viewport.w - 36, 18);

    SDL_RenderSetViewport(renderer, NULL);
}

void duo_render_game(Game *game, SDL_Renderer *renderer) {
    int ground_line_y;
    int focus_x;
    int focus_y;
    SDL_Rect scene_rect = {0, 0, WIDTH, HEIGHT};
    SDL_Rect ground_rect;
    if (!game || !renderer) return;

    if (game->duo_display_mode == DUO_DISPLAY_VERTICAL) {
        SDL_Rect left_view = {0, 0, WIDTH / 2, HEIGHT};
        SDL_Rect right_view = {WIDTH / 2, 0, WIDTH - WIDTH / 2, HEIGHT};
        duo_render_player_panel(game, renderer, left_view, &game->gameCharacter,
                                game->startPlayer1LifeTex);
        duo_render_player_panel(game, renderer, right_view, &game->gameCharacter2,
                                game->startPlayer2LifeTex);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawLine(renderer, WIDTH / 2, 0, WIDTH / 2, HEIGHT);
        game_render_minimap_overlay(game, renderer, 1, game->player1Tex, NULL);
        return;
    }

    if (game->duo_display_mode == DUO_DISPLAY_HORIZONTAL) {
        SDL_Rect top_view = {0, 0, WIDTH, HEIGHT / 2};
        SDL_Rect bottom_view = {0, HEIGHT / 2, WIDTH, HEIGHT - HEIGHT / 2};
        duo_render_player_panel(game, renderer, top_view, &game->gameCharacter,
                                game->startPlayer1LifeTex);
        duo_render_player_panel(game, renderer, bottom_view, &game->gameCharacter2,
                                game->startPlayer2LifeTex);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawLine(renderer, 0, HEIGHT / 2, WIDTH, HEIGHT / 2);
        game_render_minimap_overlay(game, renderer, 1, game->player1Tex, NULL);
        return;
    }

    ground_line_y = game->gameCharacter.groundY + game->gameCharacter.position.h;
    ground_rect = (SDL_Rect){0, ground_line_y, WIDTH, HEIGHT - ground_line_y};
    focus_x = (game->gameCharacter.position.x + game->gameCharacter.position.w / 2 +
               game->gameCharacter2.position.x + game->gameCharacter2.position.w / 2) / 2;
    focus_y = (game->gameCharacter.position.y + game->gameCharacter.position.h / 2 +
               game->gameCharacter2.position.y + game->gameCharacter2.position.h / 2) / 2;

    game_render_background(game, renderer, scene_rect, focus_x, focus_y);

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 18, 18, 18, 170);
    SDL_RenderFillRect(renderer, &ground_rect);
    SDL_SetRenderDrawColor(renderer, 255, 214, 10, 255);
    SDL_RenderDrawLine(renderer, 0, ground_rect.y, WIDTH, ground_rect.y);

    game_render_world_hazards(game, renderer);
    game_draw_character(renderer, &game->gameCharacter);
    game_draw_character(renderer, &game->gameCharacter2);
    game_render_pickup_prompt_for_player(game, renderer, &game->gameCharacter,
                                         game->gameCharacter.position);
    game_render_pickup_prompt_for_player(game, renderer, &game->gameCharacter2,
                                         game->gameCharacter2.position);

    if (game->startPlayer1LifeTex) {
        SDL_Rect p1_icon = {18, 10, 60, 60};
        SDL_RenderCopy(renderer, game->startPlayer1LifeTex, NULL, &p1_icon);
        game_draw_snowball_inventory(game, renderer, &game->gameCharacter, 30, 78, 34);
    }
    if (game->startPlayer2LifeTex) {
        SDL_Rect p2_icon = {18, 128, 60, 60};
        SDL_RenderCopy(renderer, game->startPlayer2LifeTex, NULL, &p2_icon);
        game_draw_snowball_inventory(game, renderer, &game->gameCharacter2, 30, 196, 34);
    }

    duo_render_time(game, renderer, 104, 20);
    game_render_minimap_overlay(game, renderer, 1, game->player1Tex, NULL);
}

int Game_Charger(Game *game, SDL_Renderer *renderer) {
    int use_marvin;
    if (!game || !renderer) return 0;
    if (game->gameLoaded) return 1;

    if (!game->player1Tex)
        game->player1Tex = IMG_LoadTexture(renderer, GAME_ASSETS.characters.player1);
    if (!game->player2Tex)
        game->player2Tex = IMG_LoadTexture(renderer, GAME_ASSETS.characters.player2);
    if (!game->startPlayer1LifeTex)
        game->startPlayer1LifeTex = IMG_LoadTexture(renderer, "characters/first_player_icon_life.png");
    if (!game->startPlayer2LifeTex)
        game->startPlayer2LifeTex = IMG_LoadTexture(renderer, "characters/second_player_icon_life.png");
    if (!game->gameBgTex)
        game->gameBgTex = IMG_LoadTexture(renderer, GAME_ASSETS.backgrounds.main);
    if (!game->gamesLoaded)
        Games_Charger(game, renderer);
    if (!game->miniMapFrameTex)
        game->miniMapFrameTex = IMG_LoadTexture(renderer, "backgrounds/cadre_mini_map.png");
    if (!game->miniMapLockClosedTex)
        game->miniMapLockClosedTex = IMG_LoadTexture(renderer, "buttons/ferme_lock_1.png");
    if (!game->miniMapLockOpenTex)
        game->miniMapLockOpenTex = IMG_LoadTexture(renderer, "buttons/ouvert_lock_2.png");
    game_load_enemy_textures(game, renderer);
    game_load_obstacle_assets(game, renderer);

    game_destroy_character_textures(&game->gameCharacter);
    game_destroy_character_textures(&game->gameCharacter2);

    use_marvin = (game->player_mode == 1 && game->solo_selected_player == 1);
    if (game->player_mode != 1) {
        game_load_harry_character(renderer, &game->gameCharacter);
        game_load_marvin_character(renderer, &game->gameCharacter2);
        duo_reset_runtime(game);
    } else {
        if (use_marvin) game_load_marvin_character(renderer, &game->gameCharacter);
        else game_load_harry_character(renderer, &game->gameCharacter);
        game_state_reset_character(&game->gameCharacter);
        game_set_character_ground_line(&game->gameCharacter, GAME_MONO_GROUND_LINE_Y);
    }
    duoStartTime = SDL_GetTicks();
    game_reset_enemy_obstacles(game);
    game_minimap_spark.active = 0;
    game_minimap_spark.frame = 0;
    game_minimap_border_cooldown[0] = 0u;
    game_minimap_border_cooldown[1] = 0u;
    game_enemy_collision_latch[0] = 0;
    game_enemy_collision_latch[1] = 0;
    game_minimap_layout_initialized = 0;
    game_minimap_drag_unlocked = 0;
    game_minimap_dragging = 0;
    game_minimap_drag_offset_x = 0;
    game_minimap_drag_offset_y = 0;
    game_minimap_ensure_layout();
    game->gameLastTick = SDL_GetTicks();
    game_schedule_next_enigme(game->gameLastTick);
    game->gameLoaded = 1;
    printf("[GAME] loaded. Custom key bindings enabled.\n");
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
        if (game_handle_minimap_mouse_event(&e)) {
            continue;
        }
        if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
            int owner = game_mouse_owner(game);
            if (owner == 0) {
                game->gameCharacter.pendingJump = 1;
                continue;
            }
            if (owner == 1 && game->player_mode != 1) {
                game->gameCharacter2.pendingJump = 1;
                continue;
            }
        }
        if (e.type == SDL_KEYDOWN) {
            SDL_Keycode sym = e.key.keysym.sym;
            SDL_Scancode scan = e.key.keysym.scancode;
            int is_player_jump_key = 0;

            if (sym == SDLK_x && game_handle_snowball_pickup_input(game, SDL_GetTicks())) {
                continue;
            }

            if (scan == game->keyBindings[0][KEY_ACTION_JUMP]) is_player_jump_key = 1;
            if (game->player_mode != 1 &&
                scan == game->keyBindings[1][KEY_ACTION_JUMP]) is_player_jump_key = 1;

            if (!is_player_jump_key &&
                (sym == SDLK_z || sym == SDLK_KP_PLUS || sym == SDLK_EQUALS)) {
                game->minimap_zoom += GAME_MINIMAP_ZOOM_STEP;
                if (game->minimap_zoom > GAME_MINIMAP_ZOOM_MAX) {
                    game->minimap_zoom = GAME_MINIMAP_ZOOM_MAX;
                }
                continue;
            }
            if (!is_player_jump_key &&
                (sym == SDLK_KP_MINUS || sym == SDLK_MINUS)) {
                game->minimap_zoom -= GAME_MINIMAP_ZOOM_STEP;
                if (game->minimap_zoom < GAME_MINIMAP_ZOOM_MIN) {
                    game->minimap_zoom = GAME_MINIMAP_ZOOM_MIN;
                }
                continue;
            }
            if (sym == SDLK_ESCAPE) {
                Game_ResetRuntime(game);
                Game_SetSubState(game, STATE_MENU);
                if (game->music) Mix_PlayMusic(game->music, -1);
                return;
            }
            if (game->player_mode != 1) {
                if (scan == game->keyBindings[0][KEY_ACTION_JUMP]) {
                    game->gameCharacter.pendingJump = 1;
                }
                if (scan == game->keyBindings[1][KEY_ACTION_JUMP]) {
                    game->gameCharacter2.pendingJump = 1;
                }
                continue;
            }
            if (scan == game->keyBindings[0][KEY_ACTION_JUMP]) {
                game->gameCharacter.pendingJump = 1;
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
    game_update_minimap_spark(now);

    if (game_enigme_intro_shown && game_enigme_intro_tick == 0) {
        game_schedule_next_enigme(now);
    }

    if (!game_enigme_intro_shown &&
        game_enigme_intro_tick != 0 &&
        now >= game_enigme_intro_tick) {
        game_enigme_intro_shown = 1;
        game_enigme_intro_tick = 0;
        game->gameLastTick = now;
        games_reset_menu_state();
        Game_SetSubState(game, STATE_ENIGME);
        SDL_Delay(16);
        return;
    }

    if (game->player_mode != 1) {
        duo_update_runtime(game, dt, now);
        game_update_world_hazards(game, dt, now);
        SDL_Delay(16);
        return;
    }

    keys = SDL_GetKeyboardState(NULL);
    walk_step = (int)lround((double)game->gameCharacter.moveSpeed * ((double)dt / 16.0));
    run_step = (int)lround((double)(game->gameCharacter.moveSpeed + GAME_RUN_SPEED_BONUS) * ((double)dt / 16.0));
    if (walk_step < 1) walk_step = 1;
    if (run_step < 1) run_step = 1;

    left_pressed = game_key_pressed(keys, game->keyBindings[0][KEY_ACTION_DOWN]);
    right_pressed = game_key_pressed(keys, game->keyBindings[0][KEY_ACTION_WALK]);
    jump_pressed = game->gameCharacter.pendingJump;
    run_modifier = game_key_pressed(keys, game->keyBindings[0][KEY_ACTION_RUN]);
    if (game_player_uses_mouse(game, 0)) {
        game_mouse_direction_for_rect(game->gameCharacter.position,
                                      &left_pressed,
                                      &right_pressed,
                                      &run_modifier);
    }

    game_jump_latch = jump_pressed;
    game_update_character_controls(&game->gameCharacter,
                                   left_pressed,
                                   right_pressed,
                                   jump_pressed,
                                   run_modifier,
                                   walk_step, run_step, dt, now, 0);
    game_update_world_hazards(game, dt, now);

    SDL_Delay(16);
}

void Game_Affichage(Game *game, SDL_Renderer *renderer) {
    int ground_line_y;
    int focus_x;
    int focus_y;
    SDL_Rect scene_rect = {0, 0, WIDTH, HEIGHT};
    SDL_Rect ground_rect;
    int is_duo;
    int solo_second;
    SDL_Texture *solo_tex;
    SDL_Texture *solo_life_tex;

    if (!game || !renderer) return;
    is_duo = (game->player_mode != 1);
    if (is_duo) {
        duo_render_game(game, renderer);
        return;
    }

    solo_second = (!is_duo && game->solo_selected_player == 1);
    solo_tex = (solo_second && game->player2Tex) ? game->player2Tex : game->player1Tex;

    ground_line_y = game->gameCharacter.groundY + game->gameCharacter.position.h;
    ground_rect = (SDL_Rect){0, ground_line_y, WIDTH, HEIGHT - ground_line_y};
    focus_x = game->gameCharacter.position.x + game->gameCharacter.position.w / 2;
    focus_y = game->gameCharacter.position.y + game->gameCharacter.position.h / 2;
    game_render_background(game, renderer, scene_rect, focus_x, focus_y);

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 18, 18, 18, 170);
    SDL_RenderFillRect(renderer, &ground_rect);
    SDL_SetRenderDrawColor(renderer, 255, 214, 10, 255);
    SDL_RenderDrawLine(renderer, 0, ground_rect.y, WIDTH, ground_rect.y);

    game_render_world_hazards(game, renderer);
    game_draw_character(renderer, &game->gameCharacter);
    game_render_pickup_prompt_for_player(game, renderer, &game->gameCharacter,
                                         game->gameCharacter.position);

    solo_life_tex = (solo_second && game->startPlayer2LifeTex)
        ? game->startPlayer2LifeTex
        : game->startPlayer1LifeTex;
    if (solo_life_tex) {
        SDL_Rect icon_dst = {18, 10, 60, 60};
        SDL_RenderCopy(renderer, solo_life_tex, NULL, &icon_dst);
    }
    game_draw_snowball_inventory(game, renderer, &game->gameCharacter, 30, 78, 34);
    if (game->font) duo_render_time(game, renderer, 92, 24);
    game_draw_energy_bar(game, renderer, &game->gameCharacter, 18, 122, 240, 22);

    game_render_minimap_overlay(game, renderer, 0, solo_tex, NULL);
}

typedef struct {
    const char *question;
    const char *answers[3];
    int correct;
} ImportedQuizQuestion;

const ImportedQuizQuestion quizQuestions[] = {
    {"Quel objet Kevin met-il sur la poignee pour bruler Harry ?",
     {"Bouilloire", "Fer a repasser", "Radiateur"}, 1},
    {"Dans quel film apparait Kevin McCallister ?",
     {"Maman j'ai rate l'avion", "Gremlins", "Hook"}, 0},
    {"Quel acteur joue Kevin dans Maman j'ai rate l'avion ?",
     {"Macaulay Culkin", "Haley Joel Osment", "Jake Lloyd"}, 0},
    {"Comment s'appelle le duo de cambrioleurs dans le film ?",
     {"Les Bandits du Sud", "Les Bandits Mouilles", "Les Fantomes"}, 1},
    {"Dans quelle ville se deroule l'action du premier film ?",
     {"New York", "Chicago", "Los Angeles"}, 1},
    {"Combien de freres et soeurs a Kevin dans le premier film ?",
     {"2", "4", "6"}, 1},
    {"Quel piege Kevin pose-t-il sur le sol du sous-sol ?",
     {"De l'huile", "Des clous", "Du verre brise"}, 0},
    {"Dans quel pays la famille de Kevin devait partir en vacances ?",
     {"France", "Mexique", "Italie"}, 0},
    {"Quel voisin aide Kevin a la fin du premier film ?",
     {"Le facteur", "Le vieux Marley", "Le policier"}, 1},
    {"Quel est le prenom de la mere de Kevin ?",
     {"Kate", "Julie", "Susan"}, 0},
    {"Combien de fois Kevin crie-t-il en se mettant de l'apres-rasage ?",
     {"1", "2", "3"}, 1},
    {"Quel est le nom de famille de Kevin ?",
     {"Miller", "McCallister", "Johnson"}, 1},
    {"Ou la famille de Kevin part-elle en vacances dans le 2eme film ?",
     {"Paris", "Miami", "New York"}, 2},
    {"Quel acteur joue le cambrioleur Harry ?",
     {"Joe Pesci", "Daniel Stern", "John Candy"}, 0},
    {"Quel acteur joue le cambrioleur Marv ?",
     {"Joe Pesci", "Daniel Stern", "Rob Schneider"}, 1},
    {"Quel est le surnom donne aux deux cambrioleurs ?",
     {"Les Bandits de Noel", "Les Bandits Mouilles", "Les Voleurs du Soir"}, 1},
    {"Comment Kevin commande-t-il sa pizza au debut du film ?",
     {"Par courrier", "Par telephone", "En personne"}, 1},
    {"Quel film Kevin regarde-t-il seul a la maison ?",
     {"Freddy", "Angels with Filthy Souls", "Halloween"}, 1},
    {"Quelle est la reaction de Kevin quand il se retrouve seul ?",
     {"Il pleure", "Il appelle la police", "Il crie de joie"}, 2},
    {"Quel instrument joue le voisin Marley dans le film ?",
     {"Violon", "Piano", "Guitare"}, 0}
};

#define QUIZ_TIME_LIMIT_MS 15000u
#define GAMES_MODE_MENU 0
#define GAMES_MODE_QUIZ 1
#define GAMES_MODE_PUZZLE 2
#define PUZZLE_LEVEL_COUNT 2
#define PUZZLE_PIECE_COUNT 3
#define PUZZLE_TIME_LIMIT_MS 30000u

typedef struct {
    SDL_Rect dst;
    SDL_Rect home;
    int correct;
    int dragging;
    int ox;
    int oy;
} IntegratedPuzzlePiece;

SDL_Rect quizBtnARect = {120, 430, 150, 100};
SDL_Rect quizBtnBRect = {325, 430, 150, 100};
SDL_Rect quizBtnCRect = {530, 430, 150, 100};
SDL_Rect gamesMenuQuizRect = {330, 320, 220, 70};
SDL_Rect gamesMenuPuzzleRect = {730, 320, 220, 70};
int gamesMode = GAMES_MODE_MENU;
int gamesMenuHoverQuiz = 0;
int gamesMenuHoverPuzzle = 0;
int quizHoverA = 0, quizHoverB = 0, quizHoverC = 0;
int quizSelected = -1;
int quizTimedOut = 0;
int quizAnsweredWrong = 0;
int quizQuestionIndex = -1;
int quizAsked[50] = {0};
Uint32 quizQuestionStart = 0;
Uint32 quizAnswerTick = 0;
int quizReturnState = STATE_ENIGME;
IntegratedPuzzlePiece puzzlePieces[PUZZLE_PIECE_COUNT];
SDL_Rect puzzleImageRect = {0, 0, 0, 0};
SDL_Rect puzzleHoleRect = {0, 0, 0, 0};
int puzzleLevel = 0;
int puzzleSolved = 0;
int puzzleGameOver = 0;
int puzzleAnsweredWrong = 0;
int puzzleWrongFlash = 0;
Uint32 puzzleStartTick = 0;
Uint32 puzzleDoneTick = 0;

int games_point_in_rect(SDL_Rect r, int x, int y) {
    return x >= r.x && x <= r.x + r.w && y >= r.y && y <= r.y + r.h;
}

void games_reset_menu_state(void) {
    gamesMode = GAMES_MODE_MENU;
    gamesMenuHoverQuiz = 0;
    gamesMenuHoverPuzzle = 0;
    quizHoverA = quizHoverB = quizHoverC = 0;
    quizSelected = -1;
    quizTimedOut = 0;
    quizAnsweredWrong = 0;
    puzzleSolved = 0;
    puzzleGameOver = 0;
    puzzleAnsweredWrong = 0;
    puzzleWrongFlash = 0;
}

void draw_center_text(SDL_Renderer *renderer, TTF_Font *font, const char *text,
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

void draw_wrapped_center_text(SDL_Renderer *renderer, TTF_Font *font, const char *text,
                                     SDL_Color color, SDL_Rect box) {
    SDL_Surface *surf;
    SDL_Texture *tex;
    SDL_Rect dst;
    Uint32 wrap_w;

    if (!renderer || !font || !text || !*text || box.w <= 0 || box.h <= 0) return;

    wrap_w = (Uint32)(box.w > 20 ? box.w - 20 : box.w);
    surf = TTF_RenderUTF8_Blended_Wrapped(font, text, color, wrap_w);
    if (!surf) return;

    tex = SDL_CreateTextureFromSurface(renderer, surf);
    if (tex) {
        dst = (SDL_Rect){
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

int quiz_question_count(void) {
    return (int)(sizeof(quizQuestions) / sizeof(quizQuestions[0]));
}

void quiz_pick_question(void) {
    int count = quiz_question_count();
    int remaining = 0;

    game_seed_hazards_random_once();

    if (count <= 0) {
        quizQuestionIndex = -1;
        return;
    }

    for (int i = 0; i < count; i++) {
        if (!quizAsked[i]) remaining++;
    }
    if (remaining == 0) {
        for (int i = 0; i < count; i++) quizAsked[i] = 0;
    }

    do {
        quizQuestionIndex = rand() % count;
    } while (quizAsked[quizQuestionIndex]);

    quizAsked[quizQuestionIndex] = 1;
}

void games_begin_quiz(Game *game, int return_state) {
    if (!game) return;

    gamesMode = GAMES_MODE_QUIZ;
    quiz_pick_question();
    quizSelected = -1;
    quizTimedOut = 0;
    quizAnsweredWrong = 0;
    quizHoverA = quizHoverB = quizHoverC = 0;
    quizQuestionStart = SDL_GetTicks();
    quizAnswerTick = 0;
    quizReturnState = return_state;
    if (game->quizMusic) {
        Mix_VolumeMusic(40);
        Mix_PlayMusic(game->quizMusic, -1);
    }

    Game_SetSubState(game, STATE_ENIGME_QUIZ);
}

void games_finish_quiz(Game *game) {
    if (!game) return;

    games_reset_menu_state();
    Mix_HaltMusic();
    if (quizReturnState == STATE_GAME || quizReturnState == STATE_START_PLAY) {
        game_schedule_next_enigme(SDL_GetTicks());
    }
    Game_SetSubState(game, quizReturnState);
}

int puzzle_overlap(SDL_Rect a, SDL_Rect b) {
    return !(a.x + a.w < b.x || b.x + b.w < a.x ||
             a.y + a.h < b.y || b.y + b.h < a.y);
}

void puzzle_setup_level(Game *game, int level) {
    int order[PUZZLE_PIECE_COUNT] = {0, 1, 2};
    int image_w = WIDTH - 360;
    int image_h = HEIGHT - 150;
    int piece_size = 140;

    game_seed_hazards_random_once();

    (void)game;
    if (image_w < 520) image_w = 520;
    if (image_h < 360) image_h = 360;

    puzzleImageRect = (SDL_Rect){60, 90, image_w, image_h};

    if (level == 0) {
        puzzleHoleRect = (SDL_Rect){
            puzzleImageRect.x + (int)lround((687.0 / 979.0) * puzzleImageRect.w),
            puzzleImageRect.y + (int)lround((114.0 / 645.0) * puzzleImageRect.h),
            (int)lround((98.0 / 979.0) * puzzleImageRect.w),
            (int)lround((99.0 / 645.0) * puzzleImageRect.h)
        };
    } else {
        puzzleHoleRect = (SDL_Rect){
            puzzleImageRect.x + (int)lround((524.0 / 1196.0) * puzzleImageRect.w),
            puzzleImageRect.y + (int)lround((371.0 / 896.0) * puzzleImageRect.h),
            (int)lround((151.0 / 1196.0) * puzzleImageRect.w),
            (int)lround((152.0 / 896.0) * puzzleImageRect.h)
        };
    }

    if (puzzleHoleRect.w < 60) puzzleHoleRect.w = 60;
    if (puzzleHoleRect.h < 60) puzzleHoleRect.h = 60;

    for (int i = PUZZLE_PIECE_COUNT - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int tmp = order[i];
        order[i] = order[j];
        order[j] = tmp;
    }

    for (int i = 0; i < PUZZLE_PIECE_COUNT; i++) {
        int piece_index = order[i];
        puzzlePieces[piece_index].home = (SDL_Rect){
            WIDTH - 205,
            130 + i * 170,
            piece_size,
            piece_size
        };
        puzzlePieces[piece_index].dst = puzzlePieces[piece_index].home;
        puzzlePieces[piece_index].correct = (piece_index == 0);
        puzzlePieces[piece_index].dragging = 0;
        puzzlePieces[piece_index].ox = 0;
        puzzlePieces[piece_index].oy = 0;
    }
}

void games_begin_puzzle(Game *game, int return_state) {
    if (!game) return;

    gamesMode = GAMES_MODE_PUZZLE;
    quizReturnState = return_state;
    puzzleLevel = 0;
    puzzleSolved = 0;
    puzzleGameOver = 0;
    puzzleAnsweredWrong = 0;
    puzzleWrongFlash = 0;
    puzzleStartTick = SDL_GetTicks();
    puzzleDoneTick = 0;
    puzzle_setup_level(game, puzzleLevel);
    if (game->quizMusic) {
        Mix_VolumeMusic(40);
        Mix_PlayMusic(game->quizMusic, -1);
    }
    Game_SetSubState(game, STATE_ENIGME_QUIZ);
}

void puzzle_handle_event(Game *game, const SDL_Event *e) {
    if (!game || !e) return;

    if (e->type == SDL_MOUSEBUTTONDOWN && e->button.button == SDL_BUTTON_LEFT) {
        int mx = e->button.x;
        int my = e->button.y;

        if (puzzleAnsweredWrong) {
            games_finish_quiz(game);
            return;
        }
        if ((puzzleSolved && puzzleLevel == PUZZLE_LEVEL_COUNT - 1) || puzzleGameOver) {
            games_finish_quiz(game);
            return;
        }
        if (puzzleSolved || puzzleGameOver) return;

        for (int i = PUZZLE_PIECE_COUNT - 1; i >= 0; i--) {
            if (games_point_in_rect(puzzlePieces[i].dst, mx, my)) {
                puzzlePieces[i].dragging = 1;
                puzzlePieces[i].ox = mx - puzzlePieces[i].dst.x;
                puzzlePieces[i].oy = my - puzzlePieces[i].dst.y;
                return;
            }
        }
    }

    if (e->type == SDL_MOUSEMOTION && !puzzleSolved && !puzzleGameOver && !puzzleAnsweredWrong) {
        for (int i = 0; i < PUZZLE_PIECE_COUNT; i++) {
            if (puzzlePieces[i].dragging) {
                puzzlePieces[i].dst.x = e->motion.x - puzzlePieces[i].ox;
                puzzlePieces[i].dst.y = e->motion.y - puzzlePieces[i].oy;
            }
        }
    }

    if (e->type == SDL_MOUSEBUTTONUP && e->button.button == SDL_BUTTON_LEFT &&
        !puzzleSolved && !puzzleGameOver && !puzzleAnsweredWrong) {
        for (int i = 0; i < PUZZLE_PIECE_COUNT; i++) {
            IntegratedPuzzlePiece *piece = &puzzlePieces[i];
            if (!piece->dragging) continue;

            piece->dragging = 0;
            if (puzzle_overlap(piece->dst, puzzleHoleRect)) {
                if (piece->correct) {
                    piece->dst = puzzleHoleRect;
                    puzzleSolved = 1;
                    puzzleDoneTick = SDL_GetTicks();
                    if (game->quizBeep2) Mix_PlayChannel(-1, game->quizBeep2, 0);
                } else {
                    piece->dst = piece->home;
                    puzzleAnsweredWrong = 1;
                    puzzleWrongFlash = 90;
                    puzzleDoneTick = SDL_GetTicks();
                    if (game->gameLoaded) {
                        quizReturnState = STATE_GAME;
                        game->gameLastTick = puzzleDoneTick;
                    } else if (game->startPlayLoaded) {
                        quizReturnState = STATE_START_PLAY;
                        startPlayLastTick = puzzleDoneTick;
                    }
                    if (game->quizLaugh) Mix_PlayChannel(-1, game->quizLaugh, 0);
                }
            } else {
                piece->dst = piece->home;
            }
        }
    }
}

int Games_Charger(Game *game, SDL_Renderer *renderer) {
    if (game->gamesLoaded) return 1;

    game->quizBg1 = IMG_LoadTexture(renderer, GAME_ASSETS.backgrounds.quiz_1);
    game->quizBg2 = IMG_LoadTexture(renderer, GAME_ASSETS.backgrounds.quiz_2);
    game->quizBtnA = IMG_LoadTexture(renderer, QUIZ_BUTTON_A);
    game->quizBtnB = IMG_LoadTexture(renderer, QUIZ_BUTTON_B);
    game->quizBtnC = IMG_LoadTexture(renderer, QUIZ_BUTTON_C);
    game->puzzlePictureTex[0] = IMG_LoadTexture(renderer, "picture1.png");
    game->puzzlePictureTex[1] = IMG_LoadTexture(renderer, "picture2.png");
    game->puzzlePieceTex[0][0] = IMG_LoadTexture(renderer, "correct1.png");
    game->puzzlePieceTex[0][1] = IMG_LoadTexture(renderer, "wrong1.png");
    game->puzzlePieceTex[0][2] = IMG_LoadTexture(renderer, "wrong11.png");
    game->puzzlePieceTex[1][0] = IMG_LoadTexture(renderer, "correct2.png");
    game->puzzlePieceTex[1][1] = IMG_LoadTexture(renderer, "wrong2.png");
    game->puzzlePieceTex[1][2] = IMG_LoadTexture(renderer, "wrong22.png");
    game->quizMusic = Mix_LoadMUS(GAME_ASSETS.songs.quiz_music);
    game->quizBeep = Mix_LoadWAV(SOUND_QUIZ_BEEP_1);
    game->quizBeep2 = Mix_LoadWAV(SOUND_QUIZ_BEEP_2);
    game->quizLaugh = Mix_LoadWAV(SOUND_QUIZ_LAUGH);
    game->quizFont = TTF_OpenFont(GAME_ASSETS.fonts.quiz_primary, 26);
    if (!game->quizFont) game->quizFont = TTF_OpenFont(GAME_ASSETS.fonts.quiz_fallback, 26);

    quizSelected = -1;
    quizTimedOut = 0;
    quizAnsweredWrong = 0;
    quizQuestionIndex = -1;
    games_reset_menu_state();

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
            if (game->currentSubState == STATE_ENIGME_QUIZ) {
                games_finish_quiz(game);
            } else {
                Game_SetSubState(game, STATE_MENU);
            }
            return;
        }

        if (gamesMode == GAMES_MODE_PUZZLE) {
            puzzle_handle_event(game, &e);
            continue;
        }

        if (e.type == SDL_MOUSEMOTION && gamesMode == GAMES_MODE_MENU) {
            int mx = e.motion.x, my = e.motion.y;
            gamesMenuHoverQuiz = games_point_in_rect(gamesMenuQuizRect, mx, my);
            gamesMenuHoverPuzzle = games_point_in_rect(gamesMenuPuzzleRect, mx, my);
        }

        if (e.type == SDL_MOUSEMOTION && gamesMode == GAMES_MODE_QUIZ) {
            int mx = e.motion.x, my = e.motion.y;
            quizHoverA = games_point_in_rect(quizBtnARect, mx, my);
            quizHoverB = games_point_in_rect(quizBtnBRect, mx, my);
            quizHoverC = games_point_in_rect(quizBtnCRect, mx, my);
        }

        if (e.type == SDL_MOUSEBUTTONDOWN) {
            int mx = e.button.x, my = e.button.y;
            if (gamesMode == GAMES_MODE_QUIZ) {
                const ImportedQuizQuestion *q;
                if (quizQuestionIndex < 0) quiz_pick_question();
                if (quizQuestionIndex < 0) return;
                q = &quizQuestions[quizQuestionIndex];

                if (quizSelected != -1 || quizTimedOut) {
                    games_finish_quiz(game);
                    return;
                }

                if (games_point_in_rect(quizBtnARect, mx, my)) {
                    quizSelected = 0;
                }
                if (games_point_in_rect(quizBtnBRect, mx, my)) {
                    quizSelected = 1;
                }
                if (games_point_in_rect(quizBtnCRect, mx, my)) {
                    quizSelected = 2;
                }

                if (quizSelected != -1) {
                    quizAnswerTick = SDL_GetTicks();
                    if (quizSelected == q->correct) {
                        quizAnsweredWrong = 0;
                        if (game->quizBeep2) Mix_PlayChannel(-1, game->quizBeep2, 0);
                    } else {
                        quizAnsweredWrong = 1;
                        if (game->quizBeep) Mix_PlayChannel(-1, game->quizBeep, 0);
                        if (game->quizLaugh) Mix_PlayChannel(-1, game->quizLaugh, 0);
                    }
                }
            } else {
                if (games_point_in_rect(gamesMenuQuizRect, mx, my)) {
                    games_begin_quiz(game, STATE_ENIGME);
                } else if (games_point_in_rect(gamesMenuPuzzleRect, mx, my)) {
                    games_begin_puzzle(game, STATE_ENIGME);
                }
            }
        }
    }
}

void games_draw_menu(Game *game, SDL_Renderer *renderer) {
    TTF_Font *font;
    SDL_Color white = {255, 255, 255, 255};
    SDL_Color dark = {20, 24, 28, 255};
    SDL_Color gold = {255, 214, 10, 255};
    SDL_Rect quiz_rect = gamesMenuQuizRect;
    SDL_Rect puzzle_rect = gamesMenuPuzzleRect;

    if (!game || !renderer) return;

    if (game->quizBg1) SDL_RenderCopy(renderer, game->quizBg1, NULL, NULL);
    else {
        SDL_SetRenderDrawColor(renderer, 35, 45, 52, 255);
        SDL_RenderClear(renderer);
    }

    if (gamesMenuHoverQuiz) quiz_rect.y -= 4;
    if (gamesMenuHoverPuzzle) puzzle_rect.y -= 4;

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 245, 245, 245, 220);
    SDL_RenderFillRect(renderer, &quiz_rect);
    SDL_RenderFillRect(renderer, &puzzle_rect);
    SDL_SetRenderDrawColor(renderer, gold.r, gold.g, gold.b, gold.a);
    SDL_RenderDrawRect(renderer, &quiz_rect);
    SDL_RenderDrawRect(renderer, &puzzle_rect);

    font = game->quizFont ? game->quizFont : game->font;
    if (font) {
        draw_center_text(renderer, font, "QUIZ", dark, quiz_rect);
        draw_center_text(renderer, font, "PUZZLE", dark, puzzle_rect);
        draw_center_text(renderer, font, "ENIGME", white, (SDL_Rect){100, 160, WIDTH - 200, 70});
    }
}

void puzzle_draw(Game *game, SDL_Renderer *renderer) {
    TTF_Font *font;
    Uint32 elapsed;
    int remaining_ms;
    int bar_w;
    SDL_Rect bar_bg = {80, 34, WIDTH - 160, 18};
    SDL_Rect bar_fg = bar_bg;
    SDL_Rect panel = {WIDTH - 250, 80, 210, HEIGHT - 130};
    SDL_Texture *picture_tex;

    if (!game || !renderer) return;

    if (game->quizBg2) SDL_RenderCopy(renderer, game->quizBg2, NULL, NULL);
    else if (game->quizBg1) SDL_RenderCopy(renderer, game->quizBg1, NULL, NULL);

    picture_tex = game->puzzlePictureTex[puzzleLevel];
    if (picture_tex) {
        SDL_RenderCopy(renderer, picture_tex, NULL, &puzzleImageRect);
    } else {
        SDL_SetRenderDrawColor(renderer, 28, 28, 32, 255);
        SDL_RenderFillRect(renderer, &puzzleImageRect);
    }

    if (!puzzleSolved) {
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        SDL_RenderFillRect(renderer, &puzzleHoleRect);
        SDL_SetRenderDrawColor(renderer, 255, 214, 10, 255);
        SDL_RenderDrawRect(renderer, &puzzleHoleRect);
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 10, 10, 20, 200);
    SDL_RenderFillRect(renderer, &panel);

    for (int i = 0; i < PUZZLE_PIECE_COUNT; i++) {
        SDL_Texture *tex = game->puzzlePieceTex[puzzleLevel][i];
        if (puzzlePieces[i].dragging) {
            SDL_Rect border = {
                puzzlePieces[i].dst.x - 3,
                puzzlePieces[i].dst.y - 3,
                puzzlePieces[i].dst.w + 6,
                puzzlePieces[i].dst.h + 6
            };
            SDL_SetRenderDrawColor(renderer, 255, 214, 10, 255);
            SDL_RenderFillRect(renderer, &border);
        }

        if (tex) {
            SDL_RenderCopy(renderer, tex, NULL, &puzzlePieces[i].dst);
        } else {
            SDL_SetRenderDrawColor(renderer, i == 0 ? 60 : 180, i == 0 ? 190 : 70, 90, 255);
            SDL_RenderFillRect(renderer, &puzzlePieces[i].dst);
        }
    }

    elapsed = SDL_GetTicks() - puzzleStartTick;
    remaining_ms = (elapsed >= PUZZLE_TIME_LIMIT_MS) ? 0 : (int)(PUZZLE_TIME_LIMIT_MS - elapsed);
    bar_w = (int)lround(((double)remaining_ms / (double)PUZZLE_TIME_LIMIT_MS) * (double)bar_bg.w);
    if (bar_w < 0) bar_w = 0;
    bar_fg.w = bar_w;

    SDL_SetRenderDrawColor(renderer, 20, 20, 20, 190);
    SDL_RenderFillRect(renderer, &bar_bg);
    SDL_SetRenderDrawColor(renderer, 245, remaining_ms > 9000 ? 205 : 70, 45, 255);
    SDL_RenderFillRect(renderer, &bar_fg);

    font = game->quizFont ? game->quizFont : game->font;
    if (font) {
        SDL_Color white = {255, 255, 255, 255};
        SDL_Color green = {50, 220, 80, 255};
        SDL_Color red = {240, 40, 40, 255};
        SDL_Color gold = {255, 214, 10, 255};
        char title[64];

        snprintf(title, sizeof(title), "PUZZLE %d/%d",
                 puzzleLevel + 1, PUZZLE_LEVEL_COUNT);
        draw_center_text(renderer, font, title, white, (SDL_Rect){80, 56, WIDTH - 160, 30});

        if (puzzleWrongFlash > 0) {
            draw_center_text(renderer, font, "WRONG!", red, (SDL_Rect){260, 285, 500, 80});
        }
        if (puzzleGameOver) {
            draw_center_text(renderer, font, "TIME'S UP!", gold, (SDL_Rect){260, 285, 500, 80});
        } else if (puzzleSolved && puzzleLevel == PUZZLE_LEVEL_COUNT - 1) {
            draw_center_text(renderer, font, "PUZZLE OK!", green, (SDL_Rect){260, 285, 500, 80});
        } else if (puzzleSolved) {
            draw_center_text(renderer, font, "NEXT!", green, (SDL_Rect){260, 285, 500, 80});
        }
    }
}

void Games_Affichage(Game *game, SDL_Renderer *renderer) {
    if (gamesMode == GAMES_MODE_PUZZLE) {
        puzzle_draw(game, renderer);
        return;
    }

    if (gamesMode == GAMES_MODE_QUIZ) {
        const ImportedQuizQuestion *q;
        TTF_Font *font;
        Uint32 elapsed;
        int remaining_ms;
        int bar_w;
        SDL_Rect bar_bg = {90, 54, WIDTH - 180, 18};
        SDL_Rect bar_fg = bar_bg;

        if (quizQuestionIndex < 0) quiz_pick_question();
        if (quizQuestionIndex < 0) return;
        q = &quizQuestions[quizQuestionIndex];
        font = game->quizFont ? game->quizFont : game->font;

        if (game->quizBg2) SDL_RenderCopy(renderer, game->quizBg2, NULL, NULL);
        else if (game->quizBg1) SDL_RenderCopy(renderer, game->quizBg1, NULL, NULL);

        elapsed = SDL_GetTicks() - quizQuestionStart;
        remaining_ms = (elapsed >= QUIZ_TIME_LIMIT_MS) ? 0 : (int)(QUIZ_TIME_LIMIT_MS - elapsed);
        bar_w = (int)lround(((double)remaining_ms / (double)QUIZ_TIME_LIMIT_MS) * (double)bar_bg.w);
        if (bar_w < 0) bar_w = 0;
        bar_fg.w = bar_w;

        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 20, 20, 20, 190);
        SDL_RenderFillRect(renderer, &bar_bg);
        SDL_SetRenderDrawColor(renderer, 245, remaining_ms > 5000 ? 205 : 70, 45, 255);
        SDL_RenderFillRect(renderer, &bar_fg);

        if (font) {
            SDL_Color black = {0, 0, 0, 255};
            SDL_Color white = {255, 255, 255, 255};
            SDL_Color soft = {255, 245, 210, 255};
            draw_center_text(renderer, font, "QUIZ",
                             white, (SDL_Rect){100, 78, WIDTH - 200, 36});
            draw_wrapped_center_text(renderer, font, q->question,
                                     black, (SDL_Rect){100, 120, WIDTH - 200, 95});
            draw_wrapped_center_text(renderer, font, q->answers[0],
                                     soft, (SDL_Rect){quizBtnARect.x - 25, 538, quizBtnARect.w + 50, 50});
            draw_wrapped_center_text(renderer, font, q->answers[1],
                                     soft, (SDL_Rect){quizBtnBRect.x - 25, 538, quizBtnBRect.w + 50, 50});
            draw_wrapped_center_text(renderer, font, q->answers[2],
                                     soft, (SDL_Rect){quizBtnCRect.x - 25, 538, quizBtnCRect.w + 50, 50});
        }

        SDL_Rect a = quizBtnARect, b = quizBtnBRect, c = quizBtnCRect;
        if (quizHoverA) a.y -= 4;
        if (quizHoverB) b.y -= 4;
        if (quizHoverC) c.y -= 4;

        if (game->quizBtnA) SDL_RenderCopy(renderer, game->quizBtnA, NULL, &a);
        if (game->quizBtnB) SDL_RenderCopy(renderer, game->quizBtnB, NULL, &b);
        if (game->quizBtnC) SDL_RenderCopy(renderer, game->quizBtnC, NULL, &c);

        if ((quizSelected != -1 || quizTimedOut) && font) {
            SDL_Color col = (!quizTimedOut && quizSelected == q->correct)
                ? (SDL_Color){30, 220, 30, 255}
                : (SDL_Color){240, 30, 30, 255};
            const char *msg = quizTimedOut
                ? "TEMPS ECOULE"
                : ((quizSelected == q->correct) ? "BRAVO ! Bonne reponse." : "FAUX ! Mauvaise reponse.");
            draw_center_text(renderer, font, msg, col, (SDL_Rect){120, 280, WIDTH - 240, 50});
        }
    } else {
        games_draw_menu(game, renderer);
    }
}

void Games_MiseAJour(Game *game) {
    Uint32 now;

    if (!game) return;
    if (gamesMode == GAMES_MODE_QUIZ) {
        now = SDL_GetTicks();
        if (quizSelected == -1 && !quizTimedOut &&
            now - quizQuestionStart >= QUIZ_TIME_LIMIT_MS) {
            quizTimedOut = 1;
            quizAnsweredWrong = 1;
            quizAnswerTick = now;
            if (game->quizLaugh) Mix_PlayChannel(-1, game->quizLaugh, 0);
        }
        if ((quizSelected != -1 || quizTimedOut) && now - quizAnswerTick >= 1400u) {
            if (quizAnsweredWrong && game->gameLoaded) {
                quizReturnState = STATE_GAME;
                game->gameLastTick = now;
            } else if (quizAnsweredWrong && game->startPlayLoaded) {
                quizReturnState = STATE_START_PLAY;
                startPlayLastTick = now;
            }
            games_finish_quiz(game);
            return;
        }
    }

    if (gamesMode == GAMES_MODE_PUZZLE) {
        now = SDL_GetTicks();
        if (puzzleWrongFlash > 0) puzzleWrongFlash--;

        if (puzzleAnsweredWrong) {
            if (now - puzzleDoneTick >= 1300u) games_finish_quiz(game);
            SDL_Delay(16);
            return;
        }

        if (!puzzleSolved && !puzzleGameOver &&
            now - puzzleStartTick >= PUZZLE_TIME_LIMIT_MS) {
            puzzleGameOver = 1;
            puzzleDoneTick = now;
            if (game->quizLaugh) Mix_PlayChannel(-1, game->quizLaugh, 0);
        }

        if (puzzleGameOver) {
            if (now - puzzleDoneTick >= 1600u) games_finish_quiz(game);
            SDL_Delay(16);
            return;
        }

        if (puzzleSolved && now - puzzleDoneTick >= 1300u) {
            if (puzzleLevel + 1 < PUZZLE_LEVEL_COUNT) {
                puzzleLevel++;
                puzzleSolved = 0;
                puzzleWrongFlash = 0;
                puzzleStartTick = now;
                puzzleDoneTick = 0;
                puzzle_setup_level(game, puzzleLevel);
            } else {
                games_finish_quiz(game);
                return;
            }
        }
    }

    SDL_Delay(16);
}

const AnimationMovement kAnimationMovements[MOVEMENT_COUNT] = {
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

const AnimationMovement *animation_get_movement(int type)
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
