#include "game.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define LOT1_PLAYER_W 108
#define LOT1_PLAYER_H 148
#define LOT1_GROUND_Y (HEIGHT - 174)
#define LOT1_WALK_SPEED 4
#define LOT1_RUN_SPEED 8
#define LOT1_JUMP_MS 650.0
#define LOT1_JUMP_HEIGHT 145.0
#define LOT1_SHEET_ROWS 5
#define LOT1_SHEET_COLS 5
#define LOT1_FRAME_COUNT (LOT1_SHEET_ROWS * LOT1_SHEET_COLS)
#define LOT1_IDLE_FRAME_MS 180u
#define LOT1_WALK_FRAME_MS 95u
#define LOT1_RUN_FRAME_MS 55u
#define LOT1_DANCE_FRAME_MS 85u
#define LOT1_IDLE_DANCE_DELAY_MS 20000u

static Uint32 lot1_p1_last_action_tick = 0;

int lot1_key_pressed(const Uint8 *keys, SDL_Scancode sc) {
    if (!keys || sc == SDL_SCANCODE_UNKNOWN) return 0;
    return keys[sc] != 0;
}

int lot1_mouse_owner(const Game *game) {
    int player_count;

    if (!game) return -1;
    player_count = (game->player_mode == 1) ? 1 : 2;
    for (int i = 0; i < player_count; i++) {
        if (game->playerControls[i] == GAME_CONTROL_MOUSE) return i;
    }
    return -1;
}

void lot1_mouse_direction(SDL_Rect rect, int *left, int *right, int *run) {
    int mx = 0;
    int my = 0;
    int dx;

    SDL_GetMouseState(&mx, &my);
    (void)my;

    dx = mx - (rect.x + rect.w / 2);
    if (left) *left = dx < -28;
    if (right) *right = dx > 28;
    if (run) *run = abs(dx) > 210;
}

SDL_Texture *lot1_load_texture(SDL_Renderer *renderer, const char *path) {
    SDL_Texture *tex;

    if (!renderer || !path || !*path) return NULL;
    tex = IMG_LoadTexture(renderer, path);
    if (!tex) {
        printf("[lot1] texture missing: %s (%s)\n", path, IMG_GetError());
    }
    return tex;
}

SDL_Texture *lot1_load_texture_first(SDL_Renderer *renderer,
                                            const char *a,
                                            const char *b) {
    SDL_Texture *tex = lot1_load_texture(renderer, a);
    if (!tex) tex = lot1_load_texture(renderer, b);
    return tex;
}

void lot1_destroy_character_textures(Personnage *p) {
    if (!p) return;

    if (p->idleTexture) SDL_DestroyTexture(p->idleTexture);
    if (p->idleBackTexture) SDL_DestroyTexture(p->idleBackTexture);
    if (p->walkTexture) SDL_DestroyTexture(p->walkTexture);
    if (p->walkBackTexture) SDL_DestroyTexture(p->walkBackTexture);
    if (p->runTexture) SDL_DestroyTexture(p->runTexture);
    if (p->runBackTexture) SDL_DestroyTexture(p->runBackTexture);
    if (p->jumpTexture) SDL_DestroyTexture(p->jumpTexture);
    if (p->jumpBackTexture) SDL_DestroyTexture(p->jumpBackTexture);
    if (p->damageTexture) SDL_DestroyTexture(p->damageTexture);
    if (p->layDownTexture) SDL_DestroyTexture(p->layDownTexture);
    if (p->danceTexture) SDL_DestroyTexture(p->danceTexture);

    memset(p, 0, sizeof(*p));
}

void lot1_load_harry(SDL_Renderer *renderer, Personnage *p) {
    if (!p) return;

    p->idleTexture = lot1_load_texture(renderer, "spritesheet_characters/mr_harry_stand_up.png");
    p->idleBackTexture = lot1_load_texture(renderer, "spritesheet_characters/mr_harry_stand_up_back.png");
    p->walkTexture = lot1_load_texture(renderer, "spritesheet_characters/mr_harry_walk_cycle_transparent.png");
    p->walkBackTexture = lot1_load_texture(renderer, "spritesheet_characters/mr_harry_walk_cycle_back_transparent.png");
    p->runTexture = lot1_load_texture(renderer, "spritesheet_characters/mr_harry_run.png");
    p->runBackTexture = lot1_load_texture(renderer, "spritesheet_characters/mr harry-run -reverse.png");
    p->jumpTexture = lot1_load_texture(renderer, "spritesheet_characters/mr_harry_jump_transparent.png");
    p->jumpBackTexture = lot1_load_texture(renderer, "spritesheet_characters/mr_harry_jump_back_transparent.png");
    p->danceTexture = lot1_load_texture(renderer, "spritesheet_characters/mr_harry_dance_animation_transparent.png");
}

void lot1_load_marvin(SDL_Renderer *renderer, Personnage *p) {
    if (!p) return;

    p->idleTexture = lot1_load_texture(renderer, "spritesheet_characters/marvin-stand-up.png");
    p->idleBackTexture = lot1_load_texture(renderer, "spritesheet_characters/marvin-stand-up-reverse.png");
    p->walkTexture = lot1_load_texture(renderer, "spritesheet_characters/marvin-walk-v1.png");
    p->walkBackTexture = lot1_load_texture_first(renderer,
                                                 "spritesheet_characters/marvin-walk-v1-reverse.png",
                                                 "spritesheet_characters/marvin-walk-v1 -reverse.png");
    p->runTexture = lot1_load_texture(renderer, "spritesheet_characters/marvin-run.png");
    p->runBackTexture = lot1_load_texture(renderer, "spritesheet_characters/marvin-run-reverse.png");
    p->jumpTexture = lot1_load_texture(renderer, "spritesheet_characters/marvin-jump.png");
    p->jumpBackTexture = lot1_load_texture_first(renderer,
                                                 "spritesheet_characters/marvin-jump-reverse.png",
                                                 "spritesheet_characters/marvin-jump -reverse.png");
}

void lot1_reset_character(Personnage *p, int x, int facing) {
    if (!p) return;

    p->position = (SDL_Rect){x, LOT1_GROUND_Y, LOT1_PLAYER_W, LOT1_PLAYER_H};
    p->groundY = LOT1_GROUND_Y;
    p->facing = facing;
    p->moving = 0;
    p->movementState = GAME_MOVE_STOP;
    p->pendingJump = 0;
    p->up = 0;
    p->jumpPhase = 0;
    p->jumpProgress = 0.0;
    p->jumpDir = 0;
    p->posinitX = x;
    p->posinit = LOT1_GROUND_Y;
    p->jumpHeight = LOT1_JUMP_HEIGHT;
    p->jumpSpeed = 7;
    p->frameIndex = 0;
    p->lastFrameTick = SDL_GetTicks();
}

void lot1_set_movement(Personnage *p, GameMovement movement, Uint32 now) {
    if (!p) return;
    if (p->movementState == (int)movement) return;

    p->movementState = movement;
    p->frameIndex = 0;
    p->lastFrameTick = now;
}

SDL_Texture *lot1_character_texture(const Personnage *p) {
    if (!p) return NULL;

    switch (p->movementState) {
        case GAME_MOVE_WALK:
            return p->walkTexture ? p->walkTexture : p->idleTexture;
        case GAME_MOVE_WALK_BACK:
            return p->walkBackTexture ? p->walkBackTexture : p->walkTexture;
        case GAME_MOVE_RUN:
            return p->runTexture ? p->runTexture : p->walkTexture;
        case GAME_MOVE_RUN_BACK:
            return p->runBackTexture ? p->runBackTexture : p->runTexture;
        case GAME_MOVE_JUMP:
            return p->jumpTexture ? p->jumpTexture : p->idleTexture;
        case GAME_MOVE_JUMP_BACK:
            return p->jumpBackTexture ? p->jumpBackTexture : p->jumpTexture;
        case GAME_MOVE_DANCE:
            return p->danceTexture ? p->danceTexture : p->idleTexture;
        case GAME_MOVE_STOP:
        default:
            return (p->facing < 0 && p->idleBackTexture) ? p->idleBackTexture : p->idleTexture;
    }
}

Uint32 lot1_frame_delay(const Personnage *p) {
    if (!p) return LOT1_IDLE_FRAME_MS;
    if (p->movementState == GAME_MOVE_RUN || p->movementState == GAME_MOVE_RUN_BACK) {
        return LOT1_RUN_FRAME_MS;
    }
    if (p->movementState == GAME_MOVE_WALK || p->movementState == GAME_MOVE_WALK_BACK) {
        return LOT1_WALK_FRAME_MS;
    }
    if (p->movementState == GAME_MOVE_DANCE) return LOT1_DANCE_FRAME_MS;
    return LOT1_IDLE_FRAME_MS;
}

void lot1_update_animation(Personnage *p, Uint32 now) {
    Uint32 delay;

    if (!p) return;
    delay = lot1_frame_delay(p);

    if (p->up) {
        int frame = (int)(p->jumpProgress * (LOT1_FRAME_COUNT - 1));
        if (frame < 0) frame = 0;
        if (frame >= LOT1_FRAME_COUNT) frame = LOT1_FRAME_COUNT - 1;
        p->frameIndex = frame;
        return;
    }

    if (now - p->lastFrameTick >= delay) {
        p->frameIndex = (p->frameIndex + 1) % LOT1_FRAME_COUNT;
        p->lastFrameTick = now;
    }
}

void lot1_update_character(Personnage *p, int left, int right, int run,
                                  int idle_dance, Uint32 dt, Uint32 now) {
    int speed;
    int move_dir = 0;

    if (!p) return;

    if (left && !right) move_dir = -1;
    if (right && !left) move_dir = 1;

    if (p->pendingJump && !p->up) {
        p->up = 1;
        p->jumpPhase = 1;
        p->jumpProgress = 0.0;
        p->jumpDir = move_dir;
        p->posinitX = p->position.x;
        p->posinit = p->position.y;
        p->pendingJump = 0;
    }

    speed = run ? LOT1_RUN_SPEED : LOT1_WALK_SPEED;
    if (move_dir != 0) {
        p->facing = move_dir;
        p->position.x += move_dir * speed;
        if (p->position.x < 0) p->position.x = 0;
        if (p->position.x > WIDTH - p->position.w) {
            p->position.x = WIDTH - p->position.w;
        }
    }

    if (p->up) {
        if (p->jumpDir == 0) {
            int step = (int)lround((double)p->jumpSpeed * ((double)dt / 16.0));
            int threshold = p->posinit - p->jumpHeight;
            int offset;
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
                p->jumpProgress = 0.0;
                p->jumpPhase = 0;
                p->up = 0;
                p->position.x = p->posinitX;
                p->position.y = p->groundY;
            } else {
                offset = p->posinit - p->position.y;
                if (offset < 0) offset = 0;
                if (offset > p->jumpHeight) offset = p->jumpHeight;
                p->jumpProgress = (double)offset / (double)p->jumpHeight;
            }
        } else {
            p->jumpProgress += (double)dt / LOT1_JUMP_MS;
            if (p->jumpProgress >= 1.0) {
                p->jumpProgress = 0.0;
                p->jumpPhase = 0;
                p->up = 0;
                p->position.y = p->groundY;
            } else {
                double arc = sin(p->jumpProgress * M_PI);
                p->position.y = p->groundY - (int)(arc * LOT1_JUMP_HEIGHT);
                p->position.x += p->jumpDir * (run ? 5 : 3);
                if (p->position.x < 0) p->position.x = 0;
                if (p->position.x > WIDTH - p->position.w) {
                    p->position.x = WIDTH - p->position.w;
                }
            }
        }
        if (p->jumpDir < 0) {
            lot1_set_movement(p, GAME_MOVE_JUMP_BACK, now);
        } else if (p->jumpDir > 0) {
            lot1_set_movement(p, GAME_MOVE_JUMP, now);
        } else {
            lot1_set_movement(p, p->facing < 0 ? GAME_MOVE_JUMP_BACK : GAME_MOVE_JUMP, now);
        }
    } else if (move_dir != 0) {
        lot1_set_movement(p,
                          run
                              ? (move_dir < 0 ? GAME_MOVE_RUN_BACK : GAME_MOVE_RUN)
                              : (move_dir < 0 ? GAME_MOVE_WALK_BACK : GAME_MOVE_WALK),
                          now);
    } else if (idle_dance && p->danceTexture) {
        lot1_set_movement(p, GAME_MOVE_DANCE, now);
    } else {
        lot1_set_movement(p, GAME_MOVE_STOP, now);
    }

    p->moving = move_dir != 0;
    lot1_update_animation(p, now);
}

void lot1_draw_sheet_frame(SDL_Renderer *renderer, SDL_Texture *tex,
                                  int frame, SDL_Rect dst) {
    SDL_Rect src;
    int tw = 0;
    int th = 0;
    int frame_w;
    int frame_h;

    if (!renderer || !tex) return;
    if (SDL_QueryTexture(tex, NULL, NULL, &tw, &th) != 0 || tw <= 0 || th <= 0) {
        SDL_RenderCopy(renderer, tex, NULL, &dst);
        return;
    }

    frame_w = tw / LOT1_SHEET_COLS;
    frame_h = th / LOT1_SHEET_ROWS;
    if (frame_w <= 0 || frame_h <= 0) {
        SDL_RenderCopy(renderer, tex, NULL, &dst);
        return;
    }

    frame %= LOT1_FRAME_COUNT;
    if (frame < 0) frame = 0;

    src.x = (frame % LOT1_SHEET_COLS) * frame_w;
    src.y = (frame / LOT1_SHEET_COLS) * frame_h;
    src.w = frame_w;
    src.h = frame_h;

    SDL_RenderCopy(renderer, tex, &src, &dst);
}

void lot1_draw_character(SDL_Renderer *renderer, const Personnage *p,
                                SDL_Rect dst) {
    SDL_Texture *tex;

    if (!renderer || !p) return;
    tex = lot1_character_texture(p);
    if (!tex) {
        SDL_SetRenderDrawColor(renderer, 245, 214, 89, 255);
        SDL_RenderFillRect(renderer, &dst);
        return;
    }
    lot1_draw_sheet_frame(renderer, tex, p->frameIndex, dst);
}

SDL_Rect lot1_map_rect_to_panel(SDL_Rect world, SDL_Rect panel) {
    SDL_Rect out;

    out.x = panel.x + (world.x * panel.w) / WIDTH;
    out.y = panel.y + (world.y * panel.h) / HEIGHT;
    out.w = (world.w * panel.w) / WIDTH;
    out.h = (world.h * panel.h) / HEIGHT;
    if (out.w < 42) out.w = 42;
    if (out.h < 58) out.h = 58;
    return out;
}

void lot1_draw_background(Game *game, SDL_Renderer *renderer, SDL_Rect viewport) {
    if (game && game->gameBgTex) {
        SDL_RenderCopy(renderer, game->gameBgTex, NULL, &viewport);
        return;
    }

    SDL_SetRenderDrawColor(renderer, 23, 42, 54, 255);
    SDL_RenderFillRect(renderer, &viewport);
    SDL_SetRenderDrawColor(renderer, 63, 104, 72, 255);
    SDL_Rect ground = {viewport.x, viewport.y + (viewport.h * LOT1_GROUND_Y) / HEIGHT,
                       viewport.w, viewport.h};
    SDL_RenderFillRect(renderer, &ground);
}

void lot1_render_scene(Game *game, SDL_Renderer *renderer,
                              SDL_Rect viewport, int draw_p1, int draw_p2) {
    SDL_Rect p1;
    SDL_Rect p2;

    lot1_draw_background(game, renderer, viewport);

    if (draw_p1) {
        p1 = lot1_map_rect_to_panel(game->gameCharacter.position, viewport);
        lot1_draw_character(renderer, &game->gameCharacter, p1);
    }
    if (draw_p2 && game->player_mode != 1) {
        p2 = lot1_map_rect_to_panel(game->gameCharacter2.position, viewport);
        lot1_draw_character(renderer, &game->gameCharacter2, p2);
    }
}

void lot1_draw_center_text(SDL_Renderer *renderer, TTF_Font *font,
                                  const char *text, SDL_Color color,
                                  SDL_Rect box) {
    SDL_Surface *surface;
    SDL_Texture *texture;
    SDL_Rect dst;

    if (!renderer || !font || !text || !*text) return;
    surface = TTF_RenderUTF8_Blended(font, text, color);
    if (!surface) return;

    texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (!texture) {
        SDL_FreeSurface(surface);
        return;
    }

    dst.w = surface->w;
    dst.h = surface->h;
    if (dst.w > box.w - 20) {
        double scale = (double)(box.w - 20) / (double)dst.w;
        dst.w = box.w - 20;
        dst.h = (int)(dst.h * scale);
    }
    if (dst.h > box.h - 12) {
        double scale = (double)(box.h - 12) / (double)dst.h;
        dst.h = box.h - 12;
        dst.w = (int)(dst.w * scale);
    }
    dst.x = box.x + (box.w - dst.w) / 2;
    dst.y = box.y + (box.h - dst.h) / 2;

    SDL_RenderCopy(renderer, texture, NULL, &dst);
    SDL_DestroyTexture(texture);
    SDL_FreeSurface(surface);
}

void box_message(SDL_Renderer *renderer, TTF_Font *font,
                 const char *message, SDL_Rect box) {
    SDL_Color white = {255, 255, 255, 255};

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 170);
    SDL_RenderFillRect(renderer, &box);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 220);
    SDL_RenderDrawRect(renderer, &box);
    lot1_draw_center_text(renderer, font, message, white, box);
}

void lot1_draw_icon_fit(SDL_Renderer *renderer, SDL_Texture *texture,
                               SDL_Rect slot) {
    SDL_Rect dst = slot;
    int tw = 0;
    int th = 0;
    double scale;

    if (!renderer || !texture) return;
    if (SDL_QueryTexture(texture, NULL, NULL, &tw, &th) != 0 || tw <= 0 || th <= 0) {
        SDL_RenderCopy(renderer, texture, NULL, &slot);
        return;
    }

    scale = (double)slot.w / (double)tw;
    if ((double)slot.h / (double)th < scale) {
        scale = (double)slot.h / (double)th;
    }
    dst.w = (int)(tw * scale);
    dst.h = (int)(th * scale);
    dst.x = slot.x + (slot.w - dst.w) / 2;
    dst.y = slot.y + (slot.h - dst.h) / 2;
    SDL_RenderCopy(renderer, texture, NULL, &dst);
}

void lot1_draw_player_icon(SDL_Renderer *renderer, SDL_Texture *icon,
                                  SDL_Rect icon_slot) {
    if (!renderer || !icon) return;
    lot1_draw_icon_fit(renderer, icon, icon_slot);
}

void lot1_draw_side_view_lives(SDL_Renderer *renderer, SDL_Texture *side_view_tex,
                               SDL_Rect icon_slot, int draw_to_left) {
    const int life_count = 5;
    const int life_w = 36;
    const int life_h = 36;
    const int gap = 6;
    const int icon_gap = 10;
    int y;

    if (!renderer || !side_view_tex) return;
    y = icon_slot.y + (icon_slot.h - life_h) / 2;

    for (int i = 0; i < life_count; i++) {
        SDL_Rect slot = {0, y, life_w, life_h};
        if (draw_to_left) {
            slot.x = icon_slot.x - icon_gap - life_w - i * (life_w + gap);
        } else {
            slot.x = icon_slot.x + icon_slot.w + icon_gap + i * (life_w + gap);
        }
        lot1_draw_icon_fit(renderer, side_view_tex, slot);
    }
}

void lot1_draw_player_icons(Game *game, SDL_Renderer *renderer) {
    SDL_Rect p1_icon = {18, 12, 68, 68};
    SDL_Rect p2_icon = {WIDTH - 86, 12, 68, 68};
    SDL_Texture *solo_icon;
    SDL_Texture *solo_side_view;

    if (!game || !renderer) return;

    if (game->player_mode == 1) {
        solo_icon = game->solo_selected_player == 1
            ? game->startPlayer2LifeTex
            : game->startPlayer1LifeTex;
        solo_side_view = game->solo_selected_player == 1
            ? game->startPlayer2SideViewTex
            : game->startPlayer1SideViewTex;
        lot1_draw_player_icon(renderer, solo_icon, p1_icon);
        lot1_draw_side_view_lives(renderer, solo_side_view, p1_icon, 0);
        return;
    }

    lot1_draw_player_icon(renderer, game->startPlayer1LifeTex, p1_icon);
    lot1_draw_side_view_lives(renderer, game->startPlayer1SideViewTex, p1_icon, 0);
    lot1_draw_player_icon(renderer, game->startPlayer2LifeTex, p2_icon);
    lot1_draw_side_view_lives(renderer, game->startPlayer2SideViewTex, p2_icon, 1);
}

int Game_Charger(Game *game, SDL_Renderer *renderer) {
    if (!game || !renderer) return 0;
    if (game->gameLoaded) return 1;

    if (!game->gameBgTex) {
        game->gameBgTex = lot1_load_texture(renderer, GAME_ASSETS.backgrounds.main);
    }
    if (!game->player1Tex) {
        game->player1Tex = lot1_load_texture(renderer, GAME_ASSETS.characters.player1);
    }
    if (!game->player2Tex) {
        game->player2Tex = lot1_load_texture(renderer, GAME_ASSETS.characters.player2);
    }
    if (!game->startPlayer1LifeTex) {
        game->startPlayer1LifeTex =
            lot1_load_texture(renderer, "characters/first_player_icon_life.png");
    }
    if (!game->startPlayer2LifeTex) {
        game->startPlayer2LifeTex =
            lot1_load_texture(renderer, "characters/second_player_icon_life.png");
    }
    if (!game->startPlayer1SideViewTex) {
        game->startPlayer1SideViewTex = lot1_load_texture_first(
            renderer,
            "characters/first_player_side_view.png",
            "characters/first_player.png");
    }
    if (!game->startPlayer2SideViewTex) {
        game->startPlayer2SideViewTex = lot1_load_texture_first(
            renderer,
            "characters/second_player_side_view.png",
            "characters/second_player.png");
    }

    lot1_destroy_character_textures(&game->gameCharacter);
    lot1_destroy_character_textures(&game->gameCharacter2);

    if (game->player_mode == 1) {
        if (game->solo_selected_player == 1) {
            lot1_load_marvin(renderer, &game->gameCharacter);
        } else {
            lot1_load_harry(renderer, &game->gameCharacter);
        }
        lot1_reset_character(&game->gameCharacter, (WIDTH - LOT1_PLAYER_W) / 2, 1);
    } else {
        lot1_load_harry(renderer, &game->gameCharacter);
        lot1_load_marvin(renderer, &game->gameCharacter2);
        lot1_reset_character(&game->gameCharacter, 280, 1);
        lot1_reset_character(&game->gameCharacter2, WIDTH - 390, -1);
    }

    game->gameLoaded = 1;
    game->gameLastTick = SDL_GetTicks();
    lot1_p1_last_action_tick = game->gameLastTick;
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
            SDL_Scancode scan = e.key.keysym.scancode;
            if (e.key.keysym.sym == SDLK_ESCAPE) {
                Game_ResetRuntime(game);
                Game_SetSubState(game, STATE_PLAYER);
                return;
            }
            if (scan == game->keyBindings[0][KEY_ACTION_JUMP]) {
                game->gameCharacter.pendingJump = 1;
            }
            if (game->player_mode != 1 &&
                scan == game->keyBindings[1][KEY_ACTION_JUMP]) {
                game->gameCharacter2.pendingJump = 1;
            }
        }

        if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
            int owner = lot1_mouse_owner(game);
            if (owner == 0) {
                game->gameCharacter.pendingJump = 1;
            } else if (owner == 1 && game->player_mode != 1) {
                game->gameCharacter2.pendingJump = 1;
            }
        }
    }
}

void Game_MiseAJour(Game *game) {
    Uint32 now;
    Uint32 dt;
    const Uint8 *keys;
    int p1_action;
    int p1_idle_dance;
    int p1_left;
    int p1_right;
    int p1_run;
    int p2_left;
    int p2_right;
    int p2_run;
    int mouse_owner;

    if (!game || !game->gameLoaded) return;

    now = SDL_GetTicks();
    dt = game->gameLastTick == 0 ? 16u : now - game->gameLastTick;
    if (dt > 40u) dt = 40u;
    game->gameLastTick = now;

    keys = SDL_GetKeyboardState(NULL);
    p1_left = lot1_key_pressed(keys, game->keyBindings[0][KEY_ACTION_DOWN]);
    p1_right = lot1_key_pressed(keys, game->keyBindings[0][KEY_ACTION_WALK]);
    p1_run = lot1_key_pressed(keys, game->keyBindings[0][KEY_ACTION_RUN]);
    p2_left = lot1_key_pressed(keys, game->keyBindings[1][KEY_ACTION_DOWN]);
    p2_right = lot1_key_pressed(keys, game->keyBindings[1][KEY_ACTION_WALK]);
    p2_run = lot1_key_pressed(keys, game->keyBindings[1][KEY_ACTION_RUN]);

    mouse_owner = lot1_mouse_owner(game);
    if (mouse_owner == 0) {
        lot1_mouse_direction(game->gameCharacter.position, &p1_left, &p1_right, &p1_run);
    } else if (mouse_owner == 1 && game->player_mode != 1) {
        lot1_mouse_direction(game->gameCharacter2.position, &p2_left, &p2_right, &p2_run);
    }

    p1_action = p1_left || p1_right || game->gameCharacter.pendingJump || game->gameCharacter.up;
    if (lot1_p1_last_action_tick == 0) lot1_p1_last_action_tick = now;
    if (p1_action) lot1_p1_last_action_tick = now;
    p1_idle_dance = !p1_action &&
                    now - lot1_p1_last_action_tick >= LOT1_IDLE_DANCE_DELAY_MS;

    lot1_update_character(&game->gameCharacter, p1_left, p1_right, p1_run,
                          p1_idle_dance, dt, now);
    if (game->player_mode != 1) {
        lot1_update_character(&game->gameCharacter2, p2_left, p2_right, p2_run,
                              0, dt, now);
    }
}

void Game_Affichage(Game *game, SDL_Renderer *renderer) {
    SDL_Rect full = {0, 0, WIDTH, HEIGHT};

    if (!game || !renderer) return;

    lot1_render_scene(game, renderer, full, 1, game->player_mode != 1);
    lot1_draw_player_icons(game, renderer);
}

int StartPlay_Charger(Game *game, SDL_Renderer *renderer) {
    int ok = Game_Charger(game, renderer);
    if (game) game->startPlayLoaded = ok;
    return ok;
}

void StartPlay_LectureEntree(Game *game) {
    Game_LectureEntree(game);
}

void StartPlay_Affichage(Game *game, SDL_Renderer *renderer) {
    Game_Affichage(game, renderer);
}

void StartPlay_MiseAJour(Game *game) {
    Game_MiseAJour(game);
}

void StartPlay_Cleanup(void) {
}

static const AnimationMovement kAnimationMovements[MOVEMENT_COUNT] = {
    {
        .name = "walk",
        .sprite_sheet = "spritesheet_characters/mr_harry_walk_cycle_transparent.png",
        .rows = LOT1_SHEET_ROWS,
        .cols = LOT1_SHEET_COLS,
        .frame_duration_ms = LOT1_WALK_FRAME_MS,
        .speed_x = (float)LOT1_WALK_SPEED,
        .speed_y = 0.0f,
        .loops = 1,
    },
    {
        .name = "run",
        .sprite_sheet = "spritesheet_characters/mr_harry_run.png",
        .rows = LOT1_SHEET_ROWS,
        .cols = LOT1_SHEET_COLS,
        .frame_duration_ms = LOT1_RUN_FRAME_MS,
        .speed_x = (float)LOT1_RUN_SPEED,
        .speed_y = 0.0f,
        .loops = 1,
    },
    {
        .name = "dance",
        .sprite_sheet = "spritesheet_characters/mr_harry_stand_up.png",
        .rows = LOT1_SHEET_ROWS,
        .cols = LOT1_SHEET_COLS,
        .frame_duration_ms = LOT1_IDLE_FRAME_MS,
        .speed_x = 0.0f,
        .speed_y = 0.0f,
        .loops = 1,
    },
    {
        .name = "jump",
        .sprite_sheet = "spritesheet_characters/mr_harry_jump_transparent.png",
        .rows = LOT1_SHEET_ROWS,
        .cols = LOT1_SHEET_COLS,
        .frame_duration_ms = 45,
        .speed_x = 3.0f,
        .speed_y = (float)LOT1_JUMP_HEIGHT,
        .loops = 0,
    },
};

const AnimationMovement *animation_get_movement(MovementType type) {
    if (type < 0 || type >= MOVEMENT_COUNT) return NULL;
    return &kAnimationMovements[type];
}

const AnimationMovement *animation_find_movement(const char *name) {
    if (!name) return NULL;
    for (int i = 0; i < MOVEMENT_COUNT; i++) {
        if (strcmp(kAnimationMovements[i].name, name) == 0) {
            return &kAnimationMovements[i];
        }
    }
    return NULL;
}

size_t animation_movement_count(void) {
    return MOVEMENT_COUNT;
}
