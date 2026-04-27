#include "game.h"
#include <math.h>
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
static int ps_mouse_conflict_warn = 0;
static SDL_Texture *ps_dance_tex = NULL;
static SDL_Texture *ps_settings_p1_tex = NULL;
static SDL_Texture *ps_settings_p2_tex = NULL;
static SDL_Texture *ps_not_ready_tex = NULL;
static SDL_Texture *ps_ready_tex = NULL;
static SDL_Texture *ps_arrow_right_tex = NULL;
static SDL_Texture *ps_movement_settings_tex = NULL;
static SDL_Texture *ps_send_button_tex = NULL;
static SDL_Texture *ps_cancel_button_tex = NULL;
static int ps_show_movement_settings = 0;
static int ps_movement_settings_target_player = 0;
static int ps_movement_settings_active_action = -1;
static SDL_Scancode ps_movement_settings_keys[KEY_ACTION_COUNT];
static SDL_Rect ps_movement_settings_key_rects[KEY_ACTION_COUNT];
static SDL_Rect ps_movement_settings_send_rect;
static SDL_Rect ps_movement_settings_cancel_rect;
static SDL_Rect ps_movement_settings_close_rect;
static int ps_movement_settings_hover_send = 0;
static int ps_movement_settings_hover_cancel = 0;
static int ps_movement_settings_hover_close = 0;
static int ps_dance_rows = 5, ps_dance_cols = 5;
static int ps_dance_frame_w = 0, ps_dance_frame_h = 0;
static int ps_dance_frame = 0;
static Uint32 ps_dance_last_tick = 0;
static int ps_hover_player1_photo = 0;
static SDL_Rect ps_settings_rect;
static SDL_Rect ps_not_ready_rect;
static SDL_Rect ps_mono_arrow_rect;
static int ps_hover_not_ready = 0;
static int ps_mono_character_index = 0; /* 0 -> first player, 1 -> second player */
static SDL_Rect ps_player1_help_icon_rect = {-1000, -1000, 0, 0};
static SDL_Texture *ps_skin_background_tex = NULL;
static SDL_Texture *ps_skin_preview_tex[6] = {NULL, NULL, NULL, NULL, NULL, NULL};
static SDL_Rect ps_skin_slot_draw_rects[7];
static SDL_Rect ps_skin_close_rect = {-1000, -1000, 0, 0};
static int ps_show_skin_gallery = 0;

static const char *ps_skin_preview_paths[6] = {
    "skin/mr_harry_beach_50px.png",
    "skin/mr_harry_cheff_50px.png",
    "skin/mr_harry_fire_50px.png",
    "skin/mr_harry_fire_pro_max_50px.png",
    "skin/mr_harry_matrix_50px.png",
    "skin/mr_harry_santa_50px.png"
};

static const SDL_Rect ps_skin_slot_src_rects[7] = {
    {13, 65, 50, 50},
    {81, 65, 50, 50},
    {149, 65, 50, 50},
    {13, 131, 50, 50},
    {81, 131, 50, 50},
    {149, 131, 50, 50},
    {13, 197, 50, 50}
};

int ps_point_in_rect(SDL_Rect r, int x, int y) {
    return x >= r.x && x <= r.x + r.w && y >= r.y && y <= r.y + r.h;
}

SDL_Rect ps_control_help_icon_rect(int player, int control, int active) {
    SDL_Rect r = ps_controls[player][control];
    if (active) r.y -= 4;
    return (SDL_Rect){r.x + r.w - 24, r.y + 4, 20, 20};
}

int ps_keyboard_help_icon_hit(int player_count, int x, int y, int *player_out) {
    for (int p = 0; p < player_count; p++) {
        SDL_Rect normal = ps_control_help_icon_rect(p, 0, 0);
        SDL_Rect shifted = ps_control_help_icon_rect(p, 0, 1);
        if (ps_point_in_rect(normal, x, y) || ps_point_in_rect(shifted, x, y)) {
            if (player_out) *player_out = p;
            return 1;
        }
    }
    if (player_out) *player_out = -1;
    return 0;
}

SDL_Texture *load_texture_first(SDL_Renderer *renderer, const char *a, const char *b, const char *c) {
    SDL_Texture *t = NULL;
    if (a) t = IMG_LoadTexture(renderer, a);
    if (!t && b) t = IMG_LoadTexture(renderer, b);
    if (!t && c) t = IMG_LoadTexture(renderer, c);
    return t;
}

void ps_draw_texture_in_rect(SDL_Renderer *renderer, SDL_Texture *tex, SDL_Rect dst_rect) {
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

SDL_Rect ps_fit_texture_rect(SDL_Texture *tex, SDL_Rect dst_rect) {
    SDL_Rect draw = dst_rect;
    int tw = 0;
    int th = 0;
    if (!tex || dst_rect.w <= 0 || dst_rect.h <= 0) return draw;
    if (SDL_QueryTexture(tex, NULL, NULL, &tw, &th) != 0 || tw <= 0 || th <= 0) return draw;

    {
        double scale_x = (double)dst_rect.w / (double)tw;
        double scale_y = (double)dst_rect.h / (double)th;
        double scale = (scale_x < scale_y) ? scale_x : scale_y;
        if (scale <= 0.0) scale = 1.0;
        draw.w = (int)(tw * scale);
        draw.h = (int)(th * scale);
        draw.x = dst_rect.x + (dst_rect.w - draw.w) / 2;
        draw.y = dst_rect.y + (dst_rect.h - draw.h) / 2;
    }
    return draw;
}

SDL_Rect ps_scale_from_skin_background(SDL_Rect panel_draw, SDL_Rect src_rect) {
    SDL_Rect mapped = src_rect;
    mapped.x = panel_draw.x + ((src_rect.x * panel_draw.w) + 250) / 500;
    mapped.y = panel_draw.y + ((src_rect.y * panel_draw.h) + 155) / 310;
    mapped.w = (src_rect.w * panel_draw.w) / 500;
    mapped.h = (src_rect.h * panel_draw.h) / 310;
    if (mapped.w < 1) mapped.w = 1;
    if (mapped.h < 1) mapped.h = 1;
    return mapped;
}

SDL_Rect ps_skin_gallery_slot(void) {
    return (SDL_Rect){100, 40, WIDTH - 200, HEIGHT - 80};
}

SDL_Rect ps_skin_gallery_panel_draw(void) {
    SDL_Rect slot = ps_skin_gallery_slot();
    return ps_fit_texture_rect(ps_skin_background_tex, slot);
}

void ps_layout_skin_gallery_widgets(SDL_Rect panel_draw) {
    for (int i = 0; i < 7; i++) {
        ps_skin_slot_draw_rects[i] = ps_scale_from_skin_background(panel_draw, ps_skin_slot_src_rects[i]);
    }
    ps_skin_close_rect = (SDL_Rect){panel_draw.x + panel_draw.w - 38, panel_draw.y + 12, 24, 24};
}

SDL_Scancode ps_default_binding_for(int player_index, int action) {
    if (player_index == 0) {
        switch (action) {
            case KEY_ACTION_WALK: return SDL_SCANCODE_D;
            case KEY_ACTION_JUMP: return SDL_SCANCODE_W;
            case KEY_ACTION_RUN: return SDL_SCANCODE_LSHIFT;
            case KEY_ACTION_DOWN: return SDL_SCANCODE_A;
            case KEY_ACTION_DANCE: return SDL_SCANCODE_SPACE;
            default: return SDL_SCANCODE_UNKNOWN;
        }
    }
    switch (action) {
        case KEY_ACTION_WALK: return SDL_SCANCODE_RIGHT;
        case KEY_ACTION_JUMP: return SDL_SCANCODE_UP;
        case KEY_ACTION_RUN: return SDL_SCANCODE_RSHIFT;
        case KEY_ACTION_DOWN: return SDL_SCANCODE_LEFT;
        case KEY_ACTION_DANCE: return SDL_SCANCODE_RETURN;
        default: return SDL_SCANCODE_UNKNOWN;
    }
}

void ps_reset_movement_settings_labels(void) {
    for (int i = 0; i < KEY_ACTION_COUNT; i++) {
        ps_movement_settings_keys[i] = ps_default_binding_for(ps_movement_settings_target_player, i);
    }
    ps_movement_settings_active_action = -1;
}

void ps_open_movement_settings(Game *game, int player_index) {
    if (!game) return;
    if (player_index < 0) player_index = 0;
    if (player_index > 1) player_index = 1;
    ps_movement_settings_target_player = player_index;
    for (int i = 0; i < KEY_ACTION_COUNT; i++) {
        SDL_Scancode code = game->keyBindings[player_index][i];
        if (code == SDL_SCANCODE_UNKNOWN) code = ps_default_binding_for(player_index, i);
        ps_movement_settings_keys[i] = code;
    }
    ps_movement_settings_active_action = -1;
    ps_show_movement_settings = 1;
}

void ps_apply_movement_settings(Game *game) {
    if (!game) return;
    for (int i = 0; i < KEY_ACTION_COUNT; i++) {
        game->keyBindings[ps_movement_settings_target_player][i] = ps_movement_settings_keys[i];
    }
}

const char *ps_key_label(SDL_Scancode sc) {
    const char *name;
    if (sc == SDL_SCANCODE_UNKNOWN) return "...";
    name = SDL_GetScancodeName(sc);
    if (!name || !*name) return "...";
    return name;
}

void ps_layout_movement_settings_widgets(SDL_Rect panel_draw_rect) {
    int key_x = panel_draw_rect.x + (int)(panel_draw_rect.w * 0.60);
    int key_w = (int)(panel_draw_rect.w * 0.24);
    int row_h = (int)(panel_draw_rect.h * 0.055);
    int first_center_y = panel_draw_rect.y + (int)(panel_draw_rect.h * 0.432);
    int step_y = (int)(panel_draw_rect.h * 0.080);
    int btn_w = (int)(panel_draw_rect.w * 0.18);
    int btn_h;
    int btn_y;

    if (key_w < 160) key_w = 160;
    if (row_h < 34) row_h = 34;
    if (step_y < 42) step_y = 42;

    for (int i = 0; i < KEY_ACTION_COUNT; i++) {
        int cy = first_center_y + i * step_y;
        ps_movement_settings_key_rects[i] = (SDL_Rect){key_x, cy - row_h / 2, key_w, row_h};
    }

    if (btn_w < 160) btn_w = 160;
    btn_h = (btn_w * 50) / 200;
    if (btn_h < 40) btn_h = 40;
    btn_y = panel_draw_rect.y + panel_draw_rect.h - btn_h - (int)(panel_draw_rect.h * 0.065);
    ps_movement_settings_cancel_rect = (SDL_Rect){
        panel_draw_rect.x + (int)(panel_draw_rect.w * 0.13), btn_y, btn_w, btn_h
    };
    ps_movement_settings_send_rect = (SDL_Rect){
        panel_draw_rect.x + panel_draw_rect.w - (int)(panel_draw_rect.w * 0.13) - btn_w,
        btn_y, btn_w, btn_h
    };
    ps_movement_settings_close_rect = (SDL_Rect){
        panel_draw_rect.x + panel_draw_rect.w - 44, panel_draw_rect.y + 18, 26, 26
    };
}

void ps_draw_text(SDL_Renderer *renderer, TTF_Font *font, const char *text,
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

void ps_draw_center_text(SDL_Renderer *renderer, TTF_Font *font, const char *text,
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

void ps_draw_filled_circle(SDL_Renderer *renderer, int cx, int cy, int radius,
                                  Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
    if (!renderer || radius <= 0) return;
    SDL_SetRenderDrawColor(renderer, r, g, b, a);
    for (int dy = -radius; dy <= radius; dy++) {
        int x_extent = (int)(sqrt((double)(radius * radius - dy * dy)));
        SDL_RenderDrawLine(renderer, cx - x_extent, cy + dy, cx + x_extent, cy + dy);
    }
}

void ps_handle_text_input(char *buf, int *cursor_pos, SDL_Keycode key) {
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

int ps_player_count(const Game *game) {
    return (game && game->player_mode == 1) ? 1 : 2;
}

void ps_layout_controls_for_input(int player, SDL_Rect input) {
    int iconW = 96;
    int iconH = 74;
    int gap = 12;
    int y = input.y + input.h + 16;
    int totalW = iconW * 3 + (gap * 2);
    int startX = input.x + (input.w - totalW) / 2;

    ps_controls[player][0] = (SDL_Rect){startX, y, iconW, iconH};
    ps_controls[player][1] = (SDL_Rect){startX + iconW + gap, y, iconW, iconH};
    ps_controls[player][2] = (SDL_Rect){startX + (iconW + gap) * 2, y, iconW, iconH};
}

void ps_layout_player_config(const Game *game) {
    if (ps_player_count(game) == 1) {
        int panel_margin = 45;
        int right_panel_w = 500;
        int settings_h = 330;
        int icon_w = 108;
        int icon_h = 82;
        int icon_gap = 18;
        int controls_total_w = icon_w * 3 + icon_gap * 2;
        int controls_start_x = (WIDTH - right_panel_w - panel_margin) + (right_panel_w - controls_total_w) / 2;

        ps_p1_frame = (SDL_Rect){70, 100, 360, 300};
        ps_p2_frame = (SDL_Rect){-1000, 80, 360, 300};
        ps_j1_btn = (SDL_Rect){-1000, ps_p1_frame.y - 52, 140, 44};
        ps_j2_btn = (SDL_Rect){-1000, ps_p2_frame.y - 52, 140, 44};
        ps_input1 = (SDL_Rect){ps_p1_frame.x + (ps_p1_frame.w - 360) / 2, ps_p1_frame.y + ps_p1_frame.h + 28, 360, 56};
        ps_input2 = (SDL_Rect){-1000, 410, 360, 56};
        ps_settings_rect = (SDL_Rect){WIDTH - right_panel_w - panel_margin, 70, right_panel_w, settings_h};
        ps_not_ready_rect = (SDL_Rect){WIDTH - 220, HEIGHT - 70, 200, 50};
        ps_controls[0][0] = (SDL_Rect){controls_start_x, ps_settings_rect.y + ps_settings_rect.h + 22, icon_w, icon_h};
        ps_controls[0][1] = (SDL_Rect){controls_start_x + icon_w + icon_gap, ps_settings_rect.y + ps_settings_rect.h + 22, icon_w, icon_h};
        ps_controls[0][2] = (SDL_Rect){controls_start_x + (icon_w + icon_gap) * 2, ps_settings_rect.y + ps_settings_rect.h + 22, icon_w, icon_h};
        ps_controls[1][0] = (SDL_Rect){-1000, -1000, 96, 74};
        ps_controls[1][1] = (SDL_Rect){-1000, -1000, 96, 74};
        ps_controls[1][2] = (SDL_Rect){-1000, -1000, 96, 74};
    } else {
        ps_p1_frame = (SDL_Rect){40, 80, 360, 300};
        ps_p2_frame = (SDL_Rect){WIDTH - 400, 80, 360, 300};
        ps_j1_btn = (SDL_Rect){ps_p1_frame.x + (ps_p1_frame.w - 140) / 2, ps_p1_frame.y - 52, 140, 44};
        ps_j2_btn = (SDL_Rect){ps_p2_frame.x + (ps_p2_frame.w - 140) / 2, ps_p2_frame.y - 52, 140, 44};
        ps_input1 = (SDL_Rect){40, 410, 360, 56};
        ps_input2 = (SDL_Rect){WIDTH - 400, 410, 360, 56};
        ps_settings_rect = (SDL_Rect){-1000, -1000, 0, 0};
        ps_not_ready_rect = (SDL_Rect){-1000, -1000, 0, 0};
        ps_layout_controls_for_input(0, ps_input1);
        ps_layout_controls_for_input(1, ps_input2);
    }

    ps_score_btn = (SDL_Rect){(WIDTH - 360) / 2, HEIGHT - 100, 360, 76};
    ps_player1_photo_rect = (SDL_Rect){ps_p1_frame.x + 18, ps_p1_frame.y + 40, ps_p1_frame.w - 36, ps_p1_frame.h - 12};
}

void ps_clear_config_hover(void) {
    ps_hover_player1_photo = 0;
    ps_hover_not_ready = 0;
    ps_hover_score = 0;
    ps_last_hover_score = 0;
    for (int p = 0; p < 2; p++) {
        for (int c = 0; c < 3; c++) {
            ps_hover_controls[p][c] = 0;
            ps_last_hover_controls[p][c] = 0;
        }
    }
}

void ps_commit_selection(Game *game, int player_count) {
    if (!game) return;

    game->startPlayLoaded = 0;
    Game_ResetRuntime(game);

    if (player_count == 2) {
        game->solo_selected_player = 0;
    } else {
        game->solo_selected_player = (ps_mono_character_index == 1) ? 1 : 0;
    }

    strncpy(game->player1_name, ps_name1, sizeof(game->player1_name) - 1);
    game->player1_name[sizeof(game->player1_name) - 1] = '\0';
    if (strlen(game->player1_name) == 0) {
        strcpy(game->player1_name, (player_count == 1 && game->solo_selected_player == 1) ? "Player2" : "Player1");
    }

    if (player_count == 2) {
        strncpy(game->player2_name, ps_name2, sizeof(game->player2_name) - 1);
        game->player2_name[sizeof(game->player2_name) - 1] = '\0';
        if (strlen(game->player2_name) == 0) strcpy(game->player2_name, "Player2");
        game->solo_selected_player = 0;
    } else {
        game->player2_name[0] = '\0';
    }

    game->playerControls[0] = (ps_selected_controls[0] >= 0)
        ? ps_selected_controls[0]
        : GAME_CONTROL_KEYBOARD;
    game->playerControls[1] = (player_count == 2 && ps_selected_controls[1] >= 0)
        ? ps_selected_controls[1]
        : GAME_CONTROL_KEYBOARD;
    if (game->playerControls[0] == GAME_CONTROL_MOUSE &&
        game->playerControls[1] == GAME_CONTROL_MOUSE) {
        game->playerControls[1] = GAME_CONTROL_KEYBOARD;
    }
}

void ps_enter_player_config(Game *game, int player_mode) {
    game->player_mode = player_mode;
    ps_layout_player_config(game);
    ps_clear_config_hover();
    ps_show_movement_settings = 0;
    ps_show_skin_gallery = 0;
    ps_movement_settings_active_action = -1;
    ps_focus_field = 0;
    ps_control_warn_until = 0;
    ps_mouse_conflict_warn = 0;
    ps_mono_arrow_rect = (SDL_Rect){-1000, -1000, 0, 0};
    ps_player1_help_icon_rect = (SDL_Rect){-1000, -1000, 0, 0};
    SDL_StopTextInput();

    if (player_mode == 1) {
        ps_mono_character_index = 0;
        ps_name1[0] = '\0';
        ps_cursor1 = 0;
        ps_name2[0] = '\0';
        ps_cursor2 = 0;
        ps_selected_controls[1] = -1;
    }

    Game_SetSubState(game, STATE_PLAYER_CONFIG);
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
        game->psKeyboardTex = load_texture_first(renderer, "buttons/keyboard.png", CHAR_KEYBOARD_NORMAL_1, CHAR_KEYBOARD_NORMAL_3);
    if (!game->psKeyboardHoverTex)
        game->psKeyboardHoverTex = IMG_LoadTexture(renderer, CHAR_KEYBOARD_HOVER);
    if (!game->psManetteTex)
        game->psManetteTex = load_texture_first(renderer, "buttons/manette.png", CHAR_MANETTE_NORMAL_1, CHAR_MANETTE_NORMAL_3);
    if (!game->psManetteHoverTex)
        game->psManetteHoverTex = IMG_LoadTexture(renderer, CHAR_MANETTE_HOVER);
    if (!game->psSourisTex)
        game->psSourisTex = load_texture_first(renderer, "buttons/souris.png", CHAR_SOURIS_NORMAL_1, CHAR_SOURIS_NORMAL_3);
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
        game->psNamePlayer1Tex = load_texture_first(renderer, "buttons/mr_harry_name.png", "buttons/harry_name_pixels.png", NULL);
    if (!game->psNamePlayer2Tex)
        game->psNamePlayer2Tex = IMG_LoadTexture(renderer, "buttons/marvin_name_pixel_no_bg.png");
    if (!game->psHelpIconTex)
        game->psHelpIconTex = IMG_LoadTexture(renderer, "buttons/help_icon.png");
    if (!game->psHelpButtonTex)
        game->psHelpButtonTex = IMG_LoadTexture(renderer, "buttons/help_button.png");
    if (!ps_skin_background_tex)
        ps_skin_background_tex = IMG_LoadTexture(renderer, "backgrounds/skin_background.png");
    for (int i = 0; i < 6; i++) {
        if (!ps_skin_preview_tex[i]) {
            ps_skin_preview_tex[i] = IMG_LoadTexture(renderer, ps_skin_preview_paths[i]);
        }
    }
    if (!ps_settings_p1_tex)
        ps_settings_p1_tex = load_texture_first(renderer,
                                                "buttons/remake_player_stats_mr_harry.png",
                                                "buttons/try.jpg",
                                                NULL);
    if (!ps_settings_p2_tex)
        ps_settings_p2_tex = load_texture_first(renderer,
                                                "buttons/remake_stats_mr_marvin.png",
                                                "buttons/remake_player_stats_mr_marvin.png",
                                                "buttons/try.jpg");
    if (!ps_not_ready_tex)
        ps_not_ready_tex = IMG_LoadTexture(renderer, "buttons/not_ready_button.png");
    if (!ps_ready_tex)
        ps_ready_tex = load_texture_first(renderer, "buttons/ready.png", "buttons/ready_button.png", NULL);
    if (!ps_arrow_right_tex)
        ps_arrow_right_tex = load_texture_first(renderer, "buttons/arrow right.jpg", "buttons/arrow_right.jpg", NULL);
    if (!ps_movement_settings_tex)
        ps_movement_settings_tex = IMG_LoadTexture(renderer, "buttons/movement_settings.png");
    if (!ps_send_button_tex)
        ps_send_button_tex = IMG_LoadTexture(renderer, "buttons/send_button.png");
    if (!ps_cancel_button_tex)
        ps_cancel_button_tex = IMG_LoadTexture(renderer, "buttons/cancel_button.png");

    ps_help_top_right_rect = (SDL_Rect){WIDTH - 70, 18, 48, 48};
    ps_layout_player_config(game);
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
    ps_mouse_conflict_warn = 0;
    ps_hover_player1_photo = 0;
    ps_show_movement_settings = 0;
    ps_show_skin_gallery = 0;
    ps_movement_settings_target_player = 0;
    ps_movement_settings_active_action = -1;
    ps_player1_help_icon_rect = (SDL_Rect){-1000, -1000, 0, 0};
    ps_skin_close_rect = (SDL_Rect){-1000, -1000, 0, 0};
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
                    if (game->click) Mix_PlayChannel(-1, game->click, 0);
                    ps_enter_player_config(game, 1);
                    return;
                }
                if (sym == SDLK_2 || sym == SDLK_KP_2) {
                    if (game->click) Mix_PlayChannel(-1, game->click, 0);
                    ps_enter_player_config(game, 2);
                    return;
                }
            }

            if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
                int mx = e.button.x;
                int my = e.button.y;
                if (ps_point_in_rect(ps_mode_mono_btn, mx, my)) {
                    if (game->click) Mix_PlayChannel(-1, game->click, 0);
                    ps_enter_player_config(game, 1);
                    return;
                }
                if (ps_point_in_rect(ps_mode_multi_btn, mx, my)) {
                    if (game->click) Mix_PlayChannel(-1, game->click, 0);
                    ps_enter_player_config(game, 2);
                    return;
                }
            }
        }
        return;
    }

    while (SDL_PollEvent(&e)) {
        int player_count = ps_player_count(game);

        if (e.type == SDL_QUIT) {
            game->running = 0;
            return;
        }

        if (ps_show_skin_gallery) {
            SDL_Rect panel_draw = ps_skin_gallery_panel_draw();
            ps_layout_skin_gallery_widgets(panel_draw);

            if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
                ps_show_skin_gallery = 0;
                if (game->click) Mix_PlayChannel(-1, game->click, 0);
                continue;
            }

            if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
                int mx = e.button.x;
                int my = e.button.y;
                if (ps_point_in_rect(ps_skin_close_rect, mx, my) ||
                    !ps_point_in_rect(panel_draw, mx, my)) {
                    ps_show_skin_gallery = 0;
                    if (game->click) Mix_PlayChannel(-1, game->click, 0);
                }
            }
            continue;
        }

        if (ps_show_movement_settings) {
            SDL_Rect panel_slot = {(WIDTH - 1000) / 2, (HEIGHT - 666) / 2, 1000, 666};
            SDL_Rect panel_draw = ps_fit_texture_rect(ps_movement_settings_tex, panel_slot);
            ps_layout_movement_settings_widgets(panel_draw);

            if (e.type == SDL_MOUSEMOTION) {
                int mx = e.motion.x;
                int my = e.motion.y;
                ps_movement_settings_hover_send = ps_point_in_rect(ps_movement_settings_send_rect, mx, my);
                ps_movement_settings_hover_cancel = ps_point_in_rect(ps_movement_settings_cancel_rect, mx, my);
                ps_movement_settings_hover_close = ps_point_in_rect(ps_movement_settings_close_rect, mx, my);
            }

            if (e.type == SDL_KEYDOWN) {
                SDL_Keycode sym = e.key.keysym.sym;
                if (sym == SDLK_ESCAPE) {
                    ps_show_movement_settings = 0;
                    ps_movement_settings_active_action = -1;
                    continue;
                }
                if (sym == SDLK_TAB) {
                    ps_movement_settings_active_action =
                        (ps_movement_settings_active_action + 1 + KEY_ACTION_COUNT) % KEY_ACTION_COUNT;
                    continue;
                }
                if (ps_movement_settings_active_action >= 0 &&
                    ps_movement_settings_active_action < KEY_ACTION_COUNT) {
                    if (sym == SDLK_BACKSPACE || sym == SDLK_DELETE) {
                        ps_movement_settings_keys[ps_movement_settings_active_action] = SDL_SCANCODE_UNKNOWN;
                    } else {
                        ps_movement_settings_keys[ps_movement_settings_active_action] = e.key.keysym.scancode;
                    }
                }
                continue;
            }

            if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
                int mx = e.button.x;
                int my = e.button.y;
                if (ps_point_in_rect(ps_movement_settings_close_rect, mx, my)) {
                    ps_show_movement_settings = 0;
                    ps_movement_settings_active_action = -1;
                    if (game->click) Mix_PlayChannel(-1, game->click, 0);
                    continue;
                }
                if (ps_point_in_rect(ps_movement_settings_cancel_rect, mx, my)) {
                    ps_reset_movement_settings_labels();
                    if (game->click) Mix_PlayChannel(-1, game->click, 0);
                    continue;
                }
                if (ps_point_in_rect(ps_movement_settings_send_rect, mx, my)) {
                    ps_apply_movement_settings(game);
                    ps_show_movement_settings = 0;
                    ps_movement_settings_active_action = -1;
                    if (game->click) Mix_PlayChannel(-1, game->click, 0);
                    continue;
                }
                for (int i = 0; i < KEY_ACTION_COUNT; i++) {
                    if (ps_point_in_rect(ps_movement_settings_key_rects[i], mx, my)) {
                        ps_movement_settings_active_action = i;
                        if (game->click) Mix_PlayChannel(-1, game->click, 0);
                        break;
                    }
                }
            }
            continue;
        }

        if (e.type == SDL_MOUSEMOTION) {
            int mx = e.motion.x;
            int my = e.motion.y;
            ps_hover_player1_photo = ps_point_in_rect(ps_player1_photo_rect, mx, my);
            ps_hover_not_ready = (player_count == 1) && ps_point_in_rect(ps_not_ready_rect, mx, my);
            for (int p = 0; p < 2; p++) {
                for (int c = 0; c < 3; c++) {
                    ps_hover_controls[p][c] = (p < player_count) && ps_point_in_rect(ps_controls[p][c], mx, my);
                }
            }
            ps_hover_score = (player_count == 2) && ps_point_in_rect(ps_score_btn, mx, my);

            int hovered_new_control = 0;
            for (int p = 0; p < player_count && !hovered_new_control; p++) {
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
                    Game_SetSubState(game, STATE_PLAYER);
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
                ps_focus_field = (player_count == 1 || ps_focus_field == 2) ? 1 : 2;
                SDL_StartTextInput();
            }

            if (sym == SDLK_RETURN) {
                if (ps_selected_controls[0] < 0 || (player_count == 2 && ps_selected_controls[1] < 0)) {
                    ps_control_warn_until = SDL_GetTicks() + 2000;
                    ps_mouse_conflict_warn = 0;
                    if (game->click) Mix_PlayChannel(-1, game->click, 0);
                    continue;
                }
                ps_commit_selection(game, player_count);
                ps_focus_field = 0;
                SDL_StopTextInput();
                if (player_count == 2) {
                    printf("Joueur 1: %s | Joueur 2: %s\n", game->player1_name, game->player2_name);
                } else {
                    printf("Joueur 1: %s\n", game->player1_name);
                }
                Game_ResetRuntime(game);
                if (player_count == 2)
                    Game_SetSubState(game, STATE_GAME);
                else
                    Game_SetSubState(game, STATE_SCORES_INPUT);
                return;
            }

            if (ps_focus_field == 1) ps_handle_text_input(ps_name1, &ps_cursor1, sym);
            if (player_count == 2 && ps_focus_field == 2) ps_handle_text_input(ps_name2, &ps_cursor2, sym);
        }

        if (e.type == SDL_TEXTINPUT) {
            char *buf = NULL;
            int *cursor = NULL;
            if (ps_focus_field == 1) {
                buf = ps_name1;
                cursor = &ps_cursor1;
            }
            if (player_count == 2 && ps_focus_field == 2) {
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

            {
                int help_player = -1;
                if (ps_keyboard_help_icon_hit(player_count, mx, my, &help_player)) {
                    ps_open_movement_settings(game, help_player);
                    ps_show_skin_gallery = 0;
                    ps_movement_settings_hover_send = 0;
                    ps_movement_settings_hover_cancel = 0;
                    ps_movement_settings_hover_close = 0;
                    ps_focus_field = 0;
                    SDL_StopTextInput();
                    if (game->click) Mix_PlayChannel(-1, game->click, 0);
                    continue;
                }
            }

            if (ps_point_in_rect(ps_player1_help_icon_rect, mx, my)) {
                ps_show_skin_gallery = 1;
                ps_focus_field = 0;
                SDL_StopTextInput();
                if (game->click) Mix_PlayChannel(-1, game->click, 0);
                continue;
            }

            if (ps_point_in_rect(ps_input1, mx, my)) {
                ps_focus_field = 1;
                SDL_StartTextInput();
            } else if (player_count == 2 && ps_point_in_rect(ps_input2, mx, my)) {
                ps_focus_field = 2;
                SDL_StartTextInput();
            } else {
                ps_focus_field = 0;
                SDL_StopTextInput();
            }

            if (player_count == 1 && ps_point_in_rect(ps_mono_arrow_rect, mx, my)) {
                ps_mono_character_index = (ps_mono_character_index + 1) % 2;
                if (game->click) Mix_PlayChannel(-1, game->click, 0);
                continue;
            }

            int picked_control = 0;
            for (int p = 0; p < player_count && !picked_control; p++) {
                for (int c = 0; c < 3; c++) {
                    if (ps_point_in_rect(ps_controls[p][c], mx, my)) {
                        if (c == GAME_CONTROL_MOUSE) {
                            for (int other = 0; other < player_count; other++) {
                                if (other != p &&
                                    ps_selected_controls[other] == GAME_CONTROL_MOUSE) {
                                    ps_selected_controls[other] = -1;
                                    ps_control_warn_until = SDL_GetTicks() + 2000;
                                    ps_mouse_conflict_warn = 1;
                                }
                            }
                        }
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
            if (player_count == 1 && ps_point_in_rect(ps_not_ready_rect, mx, my)) {
                if (game->click) Mix_PlayChannel(-1, game->click, 0);
                ps_commit_selection(game, player_count);
                Game_SetSubState(game, STATE_SCORES_INPUT);
                return;
            }
            if (player_count == 2 && ps_point_in_rect(ps_score_btn, mx, my)) {
                if (game->click) Mix_PlayChannel(-1, game->click, 0);
                ps_commit_selection(game, player_count);
                Game_SetSubState(game, STATE_GAME);
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
    int player_count = ps_player_count(game);

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
    ps_mono_arrow_rect = (SDL_Rect){-1000, -1000, 0, 0};
    ps_player1_help_icon_rect = (SDL_Rect){-1000, -1000, 0, 0};

    if (player_count == 2) {
        SDL_Rect name1 = {ps_p1_frame.x + 10, ps_p1_frame.y - 16, ps_p1_frame.w - 20, 84};
        SDL_Rect name2 = {ps_p2_frame.x + 10, ps_p2_frame.y - 16, ps_p2_frame.w - 20, 84};
        if (game->psNamePlayer1Tex) ps_draw_texture_in_rect(renderer, game->psNamePlayer1Tex, name1);
        if (game->psNamePlayer2Tex) ps_draw_texture_in_rect(renderer, game->psNamePlayer2Tex, name2);
    }

    {
        SDL_Texture *mono_or_p1_tex = game->player1Tex;
        if (player_count == 1 && ps_mono_character_index == 1 && game->player2Tex) {
            mono_or_p1_tex = game->player2Tex;
        }
        if (mono_or_p1_tex) {
            SDL_Rect c1 = (player_count == 1)
                ? (SDL_Rect){ps_p1_frame.x + 30, ps_p1_frame.y + 14, ps_p1_frame.w - 80, ps_p1_frame.h - 20}
                : (SDL_Rect){ps_p1_frame.x + 18, ps_p1_frame.y + 40, ps_p1_frame.w - 36, ps_p1_frame.h - 12};
            ps_player1_photo_rect = c1;
            if (ps_hover_player1_photo &&
                (player_count == 2 || ps_mono_character_index == 0) &&
                ps_dance_tex && ps_dance_frame_w > 0 && ps_dance_frame_h > 0) {
                SDL_Rect src = {
                    (ps_dance_frame % ps_dance_cols) * ps_dance_frame_w,
                    (ps_dance_frame / ps_dance_cols) * ps_dance_frame_h,
                    ps_dance_frame_w,
                    ps_dance_frame_h
                };
                SDL_RenderCopy(renderer, ps_dance_tex, &src, &c1);
            } else {
                SDL_RenderCopy(renderer, mono_or_p1_tex, NULL, &c1);
            }
            if (game->psHelpIconTex) {
                SDL_Rect help_on_p1 = {c1.x + c1.w - 44, c1.y + 6, 38, 38};
                ps_player1_help_icon_rect = help_on_p1;
                SDL_RenderCopy(renderer, game->psHelpIconTex, NULL, &help_on_p1);
            }
            if (player_count == 1 && ps_arrow_right_tex) {
                ps_mono_arrow_rect = (SDL_Rect){c1.x + c1.w + 18, c1.y + (c1.h - 86) / 2, 130, 86};
                ps_draw_texture_in_rect(renderer, ps_arrow_right_tex, ps_mono_arrow_rect);
            }
            if (player_count == 1) {
                SDL_Texture *mono_name_tex = (ps_mono_character_index == 1 && game->psNamePlayer2Tex)
                    ? game->psNamePlayer2Tex
                    : game->psNamePlayer1Tex;
                SDL_Rect mono_name_top = {c1.x - 50, c1.y - 108, c1.w + 100, 96};
                if (mono_name_tex) {
                    ps_draw_texture_in_rect(renderer, mono_name_tex, mono_name_top);
                }
            }
        }
    }

    if (player_count == 2 && game->player2Tex) {
        SDL_Rect c2 = {ps_p2_frame.x + 18, ps_p2_frame.y + 40, ps_p2_frame.w - 36, ps_p2_frame.h - 12};
        SDL_RenderCopy(renderer, game->player2Tex, NULL, &c2);
        if (game->psHelpIconTex) {
            SDL_Rect help_on_p2 = {c2.x + c2.w - 44, c2.y + 6, 38, 38};
            SDL_RenderCopy(renderer, game->psHelpIconTex, NULL, &help_on_p2);
        }
    }

    {
        const char *default_name = "Player 1 name...";
        const char *display_name = strlen(ps_name1) ? ps_name1 : default_name;
        SDL_Color name_color = strlen(ps_name1) ? white : muted;
        ps_draw_text(renderer, font, display_name, name_color, ps_input1.x + 8, ps_input1.y + 14);
    }
    if (player_count == 2) {
        ps_draw_text(renderer, font, strlen(ps_name2) ? ps_name2 : "Player 2 name...", strlen(ps_name2) ? white : muted, ps_input2.x + 8, ps_input2.y + 14);
    }

    if (player_count == 1) {
        SDL_Texture *mono_settings_tex = (ps_mono_character_index == 1 && ps_settings_p2_tex)
            ? ps_settings_p2_tex
            : ps_settings_p1_tex;
        if (mono_settings_tex) ps_draw_texture_in_rect(renderer, mono_settings_tex, ps_settings_rect);
        else ps_draw_center_text(renderer, font, "SETTINGS", white, ps_settings_rect);

        if (ps_not_ready_tex) {
            SDL_Texture *ready_state_tex = (ps_hover_not_ready && ps_ready_tex) ? ps_ready_tex : ps_not_ready_tex;
            ps_draw_texture_in_rect(renderer, ready_state_tex, ps_not_ready_rect);
        }
    }

    for (int p = 0; p < player_count; p++) {
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
                SDL_Rect help_on_control = ps_control_help_icon_rect(p, c, active);
                SDL_RenderCopy(renderer, game->psHelpIconTex, NULL, &help_on_control);
            }
        }
    }

    if (SDL_GetTicks() < ps_control_warn_until) {
        SDL_Rect hint = {(WIDTH - 760) / 2, HEIGHT - 150, 760, 34};
        const char *message = (player_count == 1)
            ? "Choose keyboard, controller or mouse for your player first"
            : "Choose keyboard, controller or mouse for each player first";
        if (ps_mouse_conflict_warn) {
            message = "Only one player can use the mouse";
        }
        ps_draw_center_text(renderer, font, message, white, hint);
    }

    if (ps_cursor_on && (ps_focus_field == 1 || (player_count == 2 && ps_focus_field == 2)) && font) {
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

    if (player_count == 1) {
        Uint8 old_r = 255, old_g = 255, old_b = 255, old_a = 255;
        SDL_BlendMode old_blend = SDL_BLENDMODE_NONE;
        int center_x = ps_score_btn.x + (ps_score_btn.w / 2);
        int cy = ps_score_btn.y + (ps_score_btn.h / 2);
        int left_selected = (ps_mono_character_index == 0);
        int active_radius = 7;
        int inactive_radius = 4;
        int spacing = 12;
        int left_x = center_x - spacing;
        int right_x = center_x + spacing;

        SDL_GetRenderDrawColor(renderer, &old_r, &old_g, &old_b, &old_a);
        SDL_GetRenderDrawBlendMode(renderer, &old_blend);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

        ps_draw_filled_circle(renderer, left_x, cy,
                              left_selected ? active_radius : inactive_radius,
                              255, 255, 255, left_selected ? 255 : 95);
        ps_draw_filled_circle(renderer, right_x, cy,
                              left_selected ? inactive_radius : active_radius,
                              255, 255, 255, left_selected ? 95 : 255);

        SDL_SetRenderDrawBlendMode(renderer, old_blend);
        SDL_SetRenderDrawColor(renderer, old_r, old_g, old_b, old_a);
    } else if (player_count == 2) {
        if (ps_not_ready_tex) {
            SDL_Texture *ready_state_tex = (ps_hover_score && ps_ready_tex) ? ps_ready_tex : ps_not_ready_tex;
            ps_draw_texture_in_rect(renderer, ready_state_tex, ps_score_btn);
        } else {
            ps_draw_center_text(renderer, font, "SCORES", white, ps_score_btn);
        }
    }

    if (ps_show_movement_settings) {
        SDL_Rect overlay = {0, 0, WIDTH, HEIGHT};
        SDL_Rect panel_slot = {(WIDTH - 1000) / 2, (HEIGHT - 666) / 2, 1000, 666};
        SDL_Rect panel_draw = ps_fit_texture_rect(ps_movement_settings_tex, panel_slot);
        SDL_Color key_text = {20, 32, 30, 255};
        SDL_Color key_active = {200, 0, 0, 255};

        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 185);
        SDL_RenderFillRect(renderer, &overlay);
        ps_layout_movement_settings_widgets(panel_draw);

        if (ps_movement_settings_tex) {
            ps_draw_texture_in_rect(renderer, ps_movement_settings_tex, panel_slot);
        } else {
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderDrawRect(renderer, &panel_draw);
            ps_draw_center_text(renderer, font, "movement_settings.png not found", white, panel_draw);
        }

        for (int i = 0; i < KEY_ACTION_COUNT; i++) {
            SDL_Rect r = ps_movement_settings_key_rects[i];
            int active = (i == ps_movement_settings_active_action);
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, active ? 230 : 180);
            SDL_RenderFillRect(renderer, &r);
            SDL_SetRenderDrawColor(renderer, active ? 215 : 165, active ? 38 : 165, active ? 38 : 165, 255);
            SDL_RenderDrawRect(renderer, &r);
            ps_draw_center_text(renderer, font, ps_key_label(ps_movement_settings_keys[i]),
                                active ? key_active : key_text, r);
        }

        {
            SDL_Rect send_rect = ps_movement_settings_send_rect;
            SDL_Rect cancel_rect = ps_movement_settings_cancel_rect;
            if (ps_movement_settings_hover_send) send_rect.y -= 2;
            if (ps_movement_settings_hover_cancel) cancel_rect.y -= 2;

            if (ps_send_button_tex) ps_draw_texture_in_rect(renderer, ps_send_button_tex, send_rect);
            else ps_draw_center_text(renderer, font, "Send", white, send_rect);

            if (ps_cancel_button_tex) ps_draw_texture_in_rect(renderer, ps_cancel_button_tex, cancel_rect);
            else ps_draw_center_text(renderer, font, "Cancel", white, cancel_rect);
        }

        {
            SDL_Rect close = ps_movement_settings_close_rect;
            SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
            SDL_SetRenderDrawColor(renderer,
                                   ps_movement_settings_hover_close ? 210 : 165,
                                   38, 38, 245);
            SDL_RenderFillRect(renderer, &close);
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderDrawRect(renderer, &close);
            ps_draw_center_text(renderer, font, "X", white, close);
        }

        ps_draw_center_text(renderer, font, "Click a key label, press a key, then send",
                            white, (SDL_Rect){panel_draw.x + 130, panel_draw.y + panel_draw.h - 56, panel_draw.w - 260, 28});
    }

    if (ps_show_skin_gallery) {
        SDL_Rect overlay = {0, 0, WIDTH, HEIGHT};
        SDL_Rect panel_slot = ps_skin_gallery_slot();
        SDL_Rect panel_draw = ps_skin_gallery_panel_draw();

        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 185);
        SDL_RenderFillRect(renderer, &overlay);

        if (ps_skin_background_tex) {
            ps_draw_texture_in_rect(renderer, ps_skin_background_tex, panel_slot);
        } else {
            SDL_SetRenderDrawColor(renderer, 80, 58, 124, 255);
            SDL_RenderFillRect(renderer, &panel_draw);
            SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
            SDL_RenderDrawRect(renderer, &panel_draw);
        }

        ps_layout_skin_gallery_widgets(panel_draw);
        for (int i = 0; i < 7; i++) {
            SDL_Texture *preview = ps_skin_preview_tex[i % 6];
            SDL_Rect slot = ps_skin_slot_draw_rects[i];
            if (preview) {
                ps_draw_texture_in_rect(renderer, preview, slot);
            } else {
                SDL_SetRenderDrawColor(renderer, 140, 140, 140, 255);
                SDL_RenderFillRect(renderer, &slot);
            }
        }

        SDL_SetRenderDrawColor(renderer, 60, 20, 20, 235);
        SDL_RenderFillRect(renderer, &ps_skin_close_rect);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawRect(renderer, &ps_skin_close_rect);
        ps_draw_center_text(renderer, font, "X", white, ps_skin_close_rect);
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
