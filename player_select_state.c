#include "game.h"
#include <stdio.h>
#include <string.h>

static SDL_Rect ps_p1_frame, ps_p2_frame;
static SDL_Rect ps_j1_btn, ps_j2_btn;
static SDL_Rect ps_input1, ps_input2;
static SDL_Rect ps_controls[2][3]; /* [player][0 keyboard, 1 controller, 2 mouse] */
static SDL_Rect ps_score_btn;
static SDL_Rect ps_mode_mono_btn, ps_mode_multi_btn;
static SDL_Rect ps_player1_photo_rect;
static SDL_Rect ps_help_top_right_rect;
static int ps_focus_field = 0;   /* 0 none, 1 input1, 2 input2 */
static int ps_cursor_on = 1;
static Uint32 ps_cursor_timer = 0;
static int ps_hover_j1 = 0, ps_hover_j2 = 0;
static int ps_last_hover_j1 = 0, ps_last_hover_j2 = 0;
static int ps_hover_controls[2][3] = {{0, 0, 0}, {0, 0, 0}};
static int ps_last_hover_controls[2][3] = {{0, 0, 0}, {0, 0, 0}};
static int ps_selected_controls[2] = {-1, -1};
static int ps_hover_score = 0;
static int ps_last_hover_score = 0;
static int ps_hover_mode_mono = 0, ps_hover_mode_multi = 0;
static int ps_last_hover_mode_mono = 0, ps_last_hover_mode_multi = 0;
static char ps_name1[256];
static char ps_name2[256];
static int ps_cursor1 = 0, ps_cursor2 = 0;
static Uint32 ps_control_warn_until = 0;
static SDL_Texture *ps_dance_tex = NULL;
static int ps_dance_rows = 5, ps_dance_cols = 5;
static int ps_dance_frame_w = 0, ps_dance_frame_h = 0;
static int ps_dance_frame = 0;
static Uint32 ps_dance_last_tick = 0;
static int ps_hover_player1_photo = 0;

static int ps_point_in_rect(SDL_Rect r, int x, int y) {
    return x >= r.x && x <= r.x + r.w && y >= r.y && y <= r.y + r.h;
}

static SDL_Texture *load_texture_first(SDL_Renderer *renderer, const char *a, const char *b, const char *c) {
    SDL_Texture *t = NULL;
    if (a) t = IMG_LoadTexture(renderer, a);
    if (!t && b) t = IMG_LoadTexture(renderer, b);
    if (!t && c) t = IMG_LoadTexture(renderer, c);
    return t;
}

static void ps_draw_texture_in_rect(SDL_Renderer *renderer, SDL_Texture *tex, SDL_Rect dst_rect) {
    if (!renderer || !tex || dst_rect.w <= 0 || dst_rect.h <= 0) return;
    int tw = 0, th = 0;
    if (SDL_QueryTexture(tex, NULL, NULL, &tw, &th) != 0 || tw <= 0 || th <= 0) {
        SDL_RenderCopy(renderer, tex, NULL, &dst_rect);
        return;
    }

    SDL_Rect draw = dst_rect;
    double scale_x = (double)dst_rect.w / (double)tw;
    double scale_y = (double)dst_rect.h / (double)th;
    double scale = (scale_x < scale_y) ? scale_x : scale_y;
    if (scale <= 0.0) scale = 1.0;

    draw.w = (int)(tw * scale);
    draw.h = (int)(th * scale);
    draw.x = dst_rect.x + (dst_rect.w - draw.w) / 2;
    draw.y = dst_rect.y + (dst_rect.h - draw.h) / 2;
    SDL_RenderCopy(renderer, tex, NULL, &draw);
}

static void ps_draw_text(SDL_Renderer *renderer, TTF_Font *font, const char *text,
                         SDL_Color color, int x, int y) {
    if (!font || !text || !*text) return;
    SDL_Surface *surf = TTF_RenderUTF8_Blended(font, text, color);
    if (!surf) return;
    SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, surf);
    if (tex) {
        SDL_Rect dst = {x, y, surf->w, surf->h};
        SDL_RenderCopy(renderer, tex, NULL, &dst);
        SDL_DestroyTexture(tex);
    }
    SDL_FreeSurface(surf);
}

static void ps_draw_center_text(SDL_Renderer *renderer, TTF_Font *font, const char *text,
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

static void ps_handle_text_input(char *buf, int *cursor_pos, SDL_Keycode key) {
    int len = (int)strlen(buf);
    if (key == SDLK_BACKSPACE && *cursor_pos > 0) {
        memmove(buf + *cursor_pos - 1, buf + *cursor_pos, len - *cursor_pos + 1);
        (*cursor_pos)--;
    } else if (key == SDLK_DELETE && *cursor_pos < len) {
        memmove(buf + *cursor_pos, buf + *cursor_pos + 1, len - *cursor_pos);
    } else if (key == SDLK_LEFT && *cursor_pos > 0) {
        (*cursor_pos)--;
    } else if (key == SDLK_RIGHT && *cursor_pos < len) {
        (*cursor_pos)++;
    } else if (key == SDLK_HOME) {
        *cursor_pos = 0;
    } else if (key == SDLK_END) {
        *cursor_pos = len;
    }
}

int PlayerSelect_Charger(Game *game, SDL_Renderer *renderer) {
    if (game->ps_loaded) return 1;

    SDL_Surface *surf = IMG_Load(GAME_ASSETS.backgrounds.main);
    if (surf) {
        game->ps_bg.texture = SDL_CreateTextureFromSurface(renderer, surf);
        game->ps_bg.width = surf->w;
        game->ps_bg.height = surf->h;
        game->ps_bg.dest_rect = (SDL_Rect){0, 0, WIDTH, HEIGHT};
        SDL_FreeSurface(surf);
    } else {
        printf("Avertissement: background_main.jpg introuvable\n");
    }

    game->ps_bg.music = Mix_LoadMUS(SOUND_BACKGROUND_LOOP);
    if (game->ps_bg.music) {
        game->ps_bg.music_volume = 48;
        Mix_VolumeMusic(48);
        Mix_PlayMusic(game->ps_bg.music, -1);
    }

    if (!game->click) {
        game->click = Mix_LoadWAV(SOUND_CLICK);
    }

    if (!game->player1Tex)
        game->player1Tex = IMG_LoadTexture(renderer, GAME_ASSETS.characters.player1);
    if (!game->player2Tex)
        game->player2Tex = IMG_LoadTexture(renderer, GAME_ASSETS.characters.player2);
    if (!game->gameBgTex)
        game->gameBgTex = IMG_LoadTexture(renderer, GAME_ASSETS.backgrounds.main);
    if (!game->psJ1Tex)
        game->psJ1Tex = IMG_LoadTexture(renderer, SCORE_BUTTON_NORMAL);
    if (!game->psJ2Tex)
        game->psJ2Tex = IMG_LoadTexture(renderer, SCORE_BUTTON_HOVER);
    if (!game->psKeyboardTex)
        game->psKeyboardTex = load_texture_first(renderer, CHAR_KEYBOARD_NORMAL_1, CHAR_KEYBOARD_NORMAL_2, CHAR_KEYBOARD_NORMAL_3);
    if (!game->psKeyboardHoverTex)
        game->psKeyboardHoverTex = IMG_LoadTexture(renderer, CHAR_KEYBOARD_HOVER);
    if (!game->psManetteTex)
        game->psManetteTex = load_texture_first(renderer, CHAR_MANETTE_NORMAL_1, CHAR_MANETTE_NORMAL_2, CHAR_MANETTE_NORMAL_3);
    if (!game->psManetteHoverTex)
        game->psManetteHoverTex = IMG_LoadTexture(renderer, CHAR_MANETTE_HOVER);
    if (!game->psSourisTex)
        game->psSourisTex = load_texture_first(renderer, CHAR_SOURIS_NORMAL_1, CHAR_SOURIS_NORMAL_2, CHAR_SOURIS_NORMAL_3);
    if (!game->psSourisHoverTex)
        game->psSourisHoverTex = IMG_LoadTexture(renderer, CHAR_SOURIS_HOVER);
    if (!game->psScoreBtnTex)
        game->psScoreBtnTex = IMG_LoadTexture(renderer, PLAYER_SELECT_SCORE_BUTTON);
    if (!game->psMonoBtnTex)
        game->psMonoBtnTex = IMG_LoadTexture(renderer, "buttons/mono_joueur_button.png");
    if (!game->psMonoBtnHoverTex)
        game->psMonoBtnHoverTex = IMG_LoadTexture(renderer, "buttons/mono_joueur_button_green.png");
    if (!game->psMultiBtnTex)
        game->psMultiBtnTex = IMG_LoadTexture(renderer, "buttons/multi_joueurs_button.png");
    if (!game->psMultiBtnHoverTex)
        game->psMultiBtnHoverTex = IMG_LoadTexture(renderer, "buttons/multi_joueurs_button_green.png");
    if (!ps_dance_tex) {
        ps_dance_tex = IMG_LoadTexture(renderer, "spritesheet_characters/mr_harry_dance_animation_transparent.png");
        if (ps_dance_tex) {
            int tw = 0, th = 0;
            if (SDL_QueryTexture(ps_dance_tex, NULL, NULL, &tw, &th) == 0 && tw > 0 && th > 0) {
                ps_dance_frame_w = tw / ps_dance_cols;
                ps_dance_frame_h = th / ps_dance_rows;
            }
        }
    }
    if (!game->psNamePlayer1Tex)
        game->psNamePlayer1Tex = IMG_LoadTexture(renderer, "buttons/harry_name_pixels.png");
    if (!game->psNamePlayer2Tex)
        game->psNamePlayer2Tex = IMG_LoadTexture(renderer, "buttons/marvin_name_pixel_no_bg.png");
    if (!game->psHelpIconTex)
        game->psHelpIconTex = IMG_LoadTexture(renderer, "buttons/help_icon.png");
    if (!game->psHelpButtonTex)
        game->psHelpButtonTex = IMG_LoadTexture(renderer, "buttons/help_button.png");

    ps_p1_frame = (SDL_Rect){40, 80, 360, 300};
    ps_p2_frame = (SDL_Rect){WIDTH - 400, 80, 360, 300};
    ps_help_top_right_rect = (SDL_Rect){WIDTH - 70, 18, 48, 48};
    ps_j1_btn = (SDL_Rect){ps_p1_frame.x + (ps_p1_frame.w - 140) / 2, ps_p1_frame.y - 52, 140, 44};
    ps_j2_btn = (SDL_Rect){ps_p2_frame.x + (ps_p2_frame.w - 140) / 2, ps_p2_frame.y - 52, 140, 44};
    ps_input1 = (SDL_Rect){40, 410, 360, 56};
    ps_input2 = (SDL_Rect){WIDTH - 400, 410, 360, 56};
    {
        int iconW = 96;
        int iconH = 74;
        int gap = 12;
        int y = ps_input1.y + ps_input1.h + 16;
        int totalW = iconW * 3 + (gap * 2);

        int p1StartX = ps_input1.x + (ps_input1.w - totalW) / 2;
        int p2StartX = ps_input2.x + (ps_input2.w - totalW) / 2;

        ps_controls[0][0] = (SDL_Rect){p1StartX, y, iconW, iconH};
        ps_controls[0][1] = (SDL_Rect){p1StartX + iconW + gap, y, iconW, iconH};
        ps_controls[0][2] = (SDL_Rect){p1StartX + (iconW + gap) * 2, y, iconW, iconH};
        ps_controls[1][0] = (SDL_Rect){p2StartX, y, iconW, iconH};
        ps_controls[1][1] = (SDL_Rect){p2StartX + iconW + gap, y, iconW, iconH};
        ps_controls[1][2] = (SDL_Rect){p2StartX + (iconW + gap) * 2, y, iconW, iconH};
    }
    ps_score_btn = (SDL_Rect){(WIDTH - 360) / 2, HEIGHT - 100, 360, 76};
    {
        int mode_w = 430;
        int mode_h = 210;
        int mode_gap = 50;
        int total_w = mode_w * 2 + mode_gap;
        int start_x = (WIDTH - total_w) / 2;
        int center_y = HEIGHT / 2;
        ps_mode_mono_btn = (SDL_Rect){start_x, center_y - (mode_h / 2) + 30, mode_w, mode_h};
        ps_mode_multi_btn = (SDL_Rect){start_x + mode_w + mode_gap, center_y - (mode_h / 2) + 30, mode_w, mode_h};
    }

    strncpy(ps_name1, game->player1_name, sizeof(ps_name1) - 1);
    strncpy(ps_name2, game->player2_name, sizeof(ps_name2) - 1);
    ps_name1[sizeof(ps_name1) - 1] = '\0';
    ps_name2[sizeof(ps_name2) - 1] = '\0';
    ps_cursor1 = (int)strlen(ps_name1);
    ps_cursor2 = (int)strlen(ps_name2);
    ps_focus_field = 0;
    ps_hover_j1 = ps_hover_j2 = 0;
    ps_last_hover_j1 = ps_last_hover_j2 = 0;
    for (int p = 0; p < 2; p++) {
        for (int c = 0; c < 3; c++) {
            ps_hover_controls[p][c] = 0;
            ps_last_hover_controls[p][c] = 0;
        }
    }
    ps_selected_controls[0] = -1;
    ps_selected_controls[1] = -1;
    ps_hover_score = 0;
    ps_last_hover_score = 0;
    ps_hover_mode_mono = 0;
    ps_hover_mode_multi = 0;
    ps_last_hover_mode_mono = 0;
    ps_last_hover_mode_multi = 0;
    ps_control_warn_until = 0;
    ps_player1_photo_rect = (SDL_Rect){ps_p1_frame.x + 18, ps_p1_frame.y + 40, ps_p1_frame.w - 36, ps_p1_frame.h - 12};
    ps_hover_player1_photo = 0;
    ps_dance_frame = 0;
    ps_dance_last_tick = SDL_GetTicks();
    ps_cursor_timer = SDL_GetTicks();
    ps_cursor_on = 1;

    game->ps_loaded = 1;
    printf("PlayerSelect: assets charges\n");
    return 1;
}

void PlayerSelect_LectureEntree(Game *game) {
    SDL_Event e;
    if (game->currentSubState == STATE_PLAYER) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                game->running = 0;
                return;
            }

            if (e.type == SDL_MOUSEMOTION) {
                int mx = e.motion.x;
                int my = e.motion.y;
                ps_hover_mode_mono = ps_point_in_rect(ps_mode_mono_btn, mx, my);
                ps_hover_mode_multi = ps_point_in_rect(ps_mode_multi_btn, mx, my);

                if (game->click && ((ps_hover_mode_mono && !ps_last_hover_mode_mono) ||
                                    (ps_hover_mode_multi && !ps_last_hover_mode_multi))) {
                    Mix_PlayChannel(-1, game->click, 0);
                }
                ps_last_hover_mode_mono = ps_hover_mode_mono;
                ps_last_hover_mode_multi = ps_hover_mode_multi;
            }

            if (e.type == SDL_KEYDOWN) {
                SDL_Keycode sym = e.key.keysym.sym;
                if (sym == SDLK_ESCAPE) {
                    Game_SetSubState(game, STATE_MENU);
                    if (game->music) Mix_PlayMusic(game->music, -1);
                    return;
                }
                if (sym == SDLK_1 || sym == SDLK_KP_1) {
                    game->player_mode = 1;
                    if (game->click) Mix_PlayChannel(-1, game->click, 0);
                    Game_SetSubState(game, STATE_PLAYER_CONFIG);
                    return;
                }
                if (sym == SDLK_2 || sym == SDLK_KP_2) {
                    game->player_mode = 2;
                    if (game->click) Mix_PlayChannel(-1, game->click, 0);
                    Game_SetSubState(game, STATE_PLAYER_CONFIG);
                    return;
                }
            }

            if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
                int mx = e.button.x;
                int my = e.button.y;
                if (ps_point_in_rect(ps_mode_mono_btn, mx, my)) {
                    game->player_mode = 1;
                    if (game->click) Mix_PlayChannel(-1, game->click, 0);
                    Game_SetSubState(game, STATE_PLAYER_CONFIG);
                    return;
                }
                if (ps_point_in_rect(ps_mode_multi_btn, mx, my)) {
                    game->player_mode = 2;
                    if (game->click) Mix_PlayChannel(-1, game->click, 0);
                    Game_SetSubState(game, STATE_PLAYER_CONFIG);
                    return;
                }
            }
        }
        return;
    }

    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) {
            game->running = 0;
            return;
        }

        if (e.type == SDL_MOUSEMOTION) {
            int mx = e.motion.x;
            int my = e.motion.y;
            ps_hover_player1_photo = ps_point_in_rect(ps_player1_photo_rect, mx, my);
            for (int p = 0; p < 2; p++) {
                for (int c = 0; c < 3; c++) {
                    ps_hover_controls[p][c] = ps_point_in_rect(ps_controls[p][c], mx, my);
                }
            }
            ps_hover_score = ps_point_in_rect(ps_score_btn, mx, my);

            int hovered_new_control = 0;
            for (int p = 0; p < 2 && !hovered_new_control; p++) {
                for (int c = 0; c < 3; c++) {
                    if (ps_hover_controls[p][c] && !ps_last_hover_controls[p][c]) {
                        hovered_new_control = 1;
                        break;
                    }
                }
            }

            if (game->click && (hovered_new_control || (ps_hover_score && !ps_last_hover_score))) {
                Mix_PlayChannel(-1, game->click, 0);
            }

            for (int p = 0; p < 2; p++) {
                for (int c = 0; c < 3; c++) {
                    ps_last_hover_controls[p][c] = ps_hover_controls[p][c];
                }
            }
            ps_last_hover_score = ps_hover_score;
        }

        if (e.type == SDL_KEYDOWN) {
            SDL_Keycode sym = e.key.keysym.sym;

            if (sym == SDLK_ESCAPE) {
                if (ps_focus_field != 0) {
                    ps_focus_field = 0;
                    SDL_StopTextInput();
                } else {
                    Game_SetSubState(game, STATE_MENU);
                    if (game->music) Mix_PlayMusic(game->music, -1);
                }
                return;
            }

            if (ps_focus_field == 0) {
                if (sym == SDLK_PLUS || sym == SDLK_EQUALS) {
                    int v = game->ps_bg.music_volume + 5;
                    if (v > 128) v = 128;
                    game->ps_bg.music_volume = v;
                    Mix_VolumeMusic(v);
                }
                if (sym == SDLK_MINUS || sym == SDLK_UNDERSCORE) {
                    int v = game->ps_bg.music_volume - 5;
                    if (v < 0) v = 0;
                    game->ps_bg.music_volume = v;
                    Mix_VolumeMusic(v);
                }
                if (sym == SDLK_p) Mix_PauseMusic();
                if (sym == SDLK_r) Mix_ResumeMusic();
            }

            if (sym == SDLK_TAB) {
                ps_focus_field = (ps_focus_field == 1) ? 2 : 1;
                SDL_StartTextInput();
            }

            if (sym == SDLK_RETURN) {
                if (ps_selected_controls[0] < 0 || ps_selected_controls[1] < 0) {
                    ps_control_warn_until = SDL_GetTicks() + 2000;
                    if (game->click) Mix_PlayChannel(-1, game->click, 0);
                    continue;
                }
                strncpy(game->player1_name, ps_name1, sizeof(game->player1_name) - 1);
                strncpy(game->player2_name, ps_name2, sizeof(game->player2_name) - 1);
                game->player1_name[sizeof(game->player1_name) - 1] = '\0';
                game->player2_name[sizeof(game->player2_name) - 1] = '\0';
                if (strlen(game->player1_name) == 0) strcpy(game->player1_name, "Player1");
                if (strlen(game->player2_name) == 0) strcpy(game->player2_name, "Player2");
                ps_focus_field = 0;
                SDL_StopTextInput();
                printf("Joueur 1: %s | Joueur 2: %s\n", game->player1_name, game->player2_name);
                Game_ResetRuntime(game);
                Game_SetSubState(game, STATE_GAME);
                return;
            }

            if (ps_focus_field == 1) ps_handle_text_input(ps_name1, &ps_cursor1, sym);
            if (ps_focus_field == 2) ps_handle_text_input(ps_name2, &ps_cursor2, sym);
        }

        if (e.type == SDL_TEXTINPUT) {
            char *buf = NULL;
            int *cursor = NULL;
            if (ps_focus_field == 1) {
                buf = ps_name1;
                cursor = &ps_cursor1;
            }
            if (ps_focus_field == 2) {
                buf = ps_name2;
                cursor = &ps_cursor2;
            }
            if (buf && cursor) {
                int len = (int)strlen(buf);
                int tlen = (int)strlen(e.text.text);
                if (len + tlen < 255) {
                    memmove(buf + *cursor + tlen, buf + *cursor, len - *cursor + 1);
                    memcpy(buf + *cursor, e.text.text, tlen);
                    *cursor += tlen;
                }
            }
        }

        if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
            int mx = e.button.x;
            int my = e.button.y;
            if (ps_point_in_rect(ps_input1, mx, my)) {
                ps_focus_field = 1;
                SDL_StartTextInput();
            } else if (ps_point_in_rect(ps_input2, mx, my)) {
                ps_focus_field = 2;
                SDL_StartTextInput();
            } else {
                ps_focus_field = 0;
                SDL_StopTextInput();
            }

            int picked_control = 0;
            for (int p = 0; p < 2 && !picked_control; p++) {
                for (int c = 0; c < 3; c++) {
                    if (ps_point_in_rect(ps_controls[p][c], mx, my)) {
                        ps_selected_controls[p] = c;
                        picked_control = 1;
                        break;
                    }
                }
            }
            if (picked_control) {
                if (game->click) Mix_PlayChannel(-1, game->click, 0);
                continue;
            }
            if (ps_point_in_rect(ps_score_btn, mx, my)) {
                if (game->click) Mix_PlayChannel(-1, game->click, 0);
                Game_SetSubState(game, STATE_SCORES_INPUT);
                return;
            }
        }
    }
}

void PlayerSelect_Affichage(Game *game, SDL_Renderer *renderer) {
    if (game->ps_bg.texture)
        SDL_RenderCopy(renderer, game->ps_bg.texture, NULL, &game->ps_bg.dest_rect);

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 90);
    SDL_Rect full = {0, 0, WIDTH, HEIGHT};
    SDL_RenderFillRect(renderer, &full);

    TTF_Font *font = game->font;
    SDL_Color white = {255, 255, 255, 255};
    SDL_Color muted = {180, 180, 180, 255};

    if (game->currentSubState == STATE_PLAYER) {
        if (game->psHelpButtonTex) {
            SDL_RenderCopy(renderer, game->psHelpButtonTex, NULL, &ps_help_top_right_rect);
        }

        SDL_Rect prompt_box = {(WIDTH - 1000) / 2, 120, 1000, 80};
        ps_draw_center_text(renderer, font, "Do you want mono player or duo player ?", white, prompt_box);

        SDL_Texture *mono_tex = ps_hover_mode_mono ? game->psMonoBtnHoverTex : game->psMonoBtnTex;
        SDL_Texture *multi_tex = ps_hover_mode_multi ? game->psMultiBtnHoverTex : game->psMultiBtnTex;

        if (mono_tex) ps_draw_texture_in_rect(renderer, mono_tex, ps_mode_mono_btn);
        else ps_draw_center_text(renderer, font, "MONO PLAYER", white, ps_mode_mono_btn);

        if (multi_tex) ps_draw_texture_in_rect(renderer, multi_tex, ps_mode_multi_btn);
        else ps_draw_center_text(renderer, font, "DUO PLAYER", white, ps_mode_multi_btn);
        return;
    }

    if (game->psHelpButtonTex) {
        SDL_RenderCopy(renderer, game->psHelpButtonTex, NULL, &ps_help_top_right_rect);
    }

    {
        SDL_Rect name1 = {ps_p1_frame.x + 10, ps_p1_frame.y - 16, ps_p1_frame.w - 20, 84};
        SDL_Rect name2 = {ps_p2_frame.x + 10, ps_p2_frame.y - 16, ps_p2_frame.w - 20, 84};
        if (game->psNamePlayer1Tex) ps_draw_texture_in_rect(renderer, game->psNamePlayer1Tex, name1);
        if (game->psNamePlayer2Tex) ps_draw_texture_in_rect(renderer, game->psNamePlayer2Tex, name2);
    }

    if (game->player1Tex) {
        SDL_Rect c1 = {ps_p1_frame.x + 18, ps_p1_frame.y + 40, ps_p1_frame.w - 36, ps_p1_frame.h - 12};
        ps_player1_photo_rect = c1;
        if (ps_hover_player1_photo && ps_dance_tex && ps_dance_frame_w > 0 && ps_dance_frame_h > 0) {
            SDL_Rect src = {
                (ps_dance_frame % ps_dance_cols) * ps_dance_frame_w,
                (ps_dance_frame / ps_dance_cols) * ps_dance_frame_h,
                ps_dance_frame_w,
                ps_dance_frame_h
            };
            SDL_RenderCopy(renderer, ps_dance_tex, &src, &c1);
        } else {
            SDL_RenderCopy(renderer, game->player1Tex, NULL, &c1);
        }
        if (game->psHelpIconTex) {
            SDL_Rect help_on_p1 = {c1.x + c1.w - 44, c1.y + 6, 38, 38};
            SDL_RenderCopy(renderer, game->psHelpIconTex, NULL, &help_on_p1);
        }
    }
    if (game->player2Tex) {
        SDL_Rect c2 = {ps_p2_frame.x + 18, ps_p2_frame.y + 40, ps_p2_frame.w - 36, ps_p2_frame.h - 12};
        SDL_RenderCopy(renderer, game->player2Tex, NULL, &c2);
        if (game->psHelpIconTex) {
            SDL_Rect help_on_p2 = {c2.x + c2.w - 44, c2.y + 6, 38, 38};
            SDL_RenderCopy(renderer, game->psHelpIconTex, NULL, &help_on_p2);
        }
    }

    ps_draw_text(renderer, font, strlen(ps_name1) ? ps_name1 : "Player 1 name...", strlen(ps_name1) ? white : muted, ps_input1.x + 8, ps_input1.y + 14);
    ps_draw_text(renderer, font, strlen(ps_name2) ? ps_name2 : "Player 2 name...", strlen(ps_name2) ? white : muted, ps_input2.x + 8, ps_input2.y + 14);

    for (int p = 0; p < 2; p++) {
        for (int c = 0; c < 3; c++) {
            SDL_Rect r = ps_controls[p][c];
            int active = ps_hover_controls[p][c] || (ps_selected_controls[p] == c);
            SDL_Texture *tex = NULL;
            if (c == 0) tex = active ? game->psKeyboardHoverTex : game->psKeyboardTex;
            if (c == 1) tex = active ? game->psManetteHoverTex : game->psManetteTex;
            if (c == 2) tex = active ? game->psSourisHoverTex : game->psSourisTex;
            if (active) r.y -= 4;
            if (tex) SDL_RenderCopy(renderer, tex, NULL, &r);
            if (game->psHelpIconTex && (c == 0 || c == 1)) {
                SDL_Rect help_on_control = {r.x + r.w - 24, r.y + 4, 20, 20};
                SDL_RenderCopy(renderer, game->psHelpIconTex, NULL, &help_on_control);
            }
        }
    }

    if (SDL_GetTicks() < ps_control_warn_until) {
        SDL_Rect hint = {(WIDTH - 760) / 2, HEIGHT - 150, 760, 34};
        ps_draw_center_text(renderer, font, "Choose keyboard, controller or mouse for each player first", white, hint);
    }

    if (ps_cursor_on && (ps_focus_field == 1 || ps_focus_field == 2) && font) {
        char tmp[256];
        int tw = 0;
        SDL_Rect box = (ps_focus_field == 1) ? ps_input1 : ps_input2;
        const char *active = (ps_focus_field == 1) ? ps_name1 : ps_name2;
        int cursor = (ps_focus_field == 1) ? ps_cursor1 : ps_cursor2;
        if (cursor > 0) {
            strncpy(tmp, active, (size_t)cursor);
            tmp[cursor] = '\0';
            TTF_SizeUTF8(font, tmp, &tw, NULL);
        }
        SDL_SetRenderDrawColor(renderer, 255, 220, 70, 255);
        SDL_RenderDrawLine(renderer, box.x + 8 + tw, box.y + 8, box.x + 8 + tw, box.y + box.h - 8);
    }

    SDL_Rect score_btn_draw = ps_score_btn;
    if (game->psScoreBtnTex) {
        SDL_RenderCopy(renderer, game->psScoreBtnTex, NULL, &score_btn_draw);
    } else {
        ps_draw_center_text(renderer, font, "SCORES", white, score_btn_draw);
    }
}

void PlayerSelect_MiseAJour(Game *game) {
    (void)game;
    Uint32 now = SDL_GetTicks();
    int mx = 0, my = 0;
    SDL_GetMouseState(&mx, &my);
    ps_hover_player1_photo = ps_point_in_rect(ps_player1_photo_rect, mx, my);

    if (ps_hover_player1_photo && ps_dance_tex && ps_dance_frame_w > 0 && ps_dance_frame_h > 0) {
        if (now - ps_dance_last_tick >= 85) {
            int frames = ps_dance_rows * ps_dance_cols;
            if (frames < 1) frames = 1;
            ps_dance_frame = (ps_dance_frame + 1) % frames;
            ps_dance_last_tick = now;
        }
    } else {
        ps_dance_frame = 0;
        ps_dance_last_tick = now;
    }
    if (now - ps_cursor_timer >= 500) {
        ps_cursor_on = !ps_cursor_on;
        ps_cursor_timer = now;
    }
    SDL_Delay(16);
}
