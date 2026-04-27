#include "game.h"
#include <stdio.h>

#define SKIN_SELECT_COUNT 9
#define SKIN_SELECT_COLS 5
#define SKIN_SELECT_ROWS 5

static SDL_Texture *skinSelectBackground = NULL;
static SDL_Texture *skinSelectPreview[SKIN_SELECT_COUNT] = {0};
static SDL_Texture *skinSelectNameTex[SKIN_SELECT_COUNT] = {0};
static SDL_Texture *skinSelectTitleTex = NULL;
static SDL_Texture *skinSelectContinueTex = NULL;
static TTF_Font *skinSelectFont = NULL;
static int skinSelectCurrentIndex = 0;
static int skinSelectTargetIndex = 0;
static int skinSelectStepsDone = 0;
static int skinSelectTotalSteps = 0;
static int skinSelectLocked = 0;
static int skinSelectPreviewFrame = 0;
static Uint32 skinSelectLastStepTick = 0;
static Uint32 skinSelectLastPreviewTick = 0;
static const char *skinSelectNames[SKIN_SELECT_COUNT] = {
    "Waikiki Draft",
    "Sandveil Prince",
    "Mariviosi",
    "Dragon Alley Kid",
    "Neon Ronin",
    "Vice Shadow",
    "Dust Rebellion",
    "Iron Pankster",
    "Kevin"
};

static int skin_select_resolve_asset_path(int index, char *output, size_t output_size) {
    if (!output || output_size == 0 || index < 0 || index >= SKIN_SELECT_COUNT) return 0;
    return snprintf(output, output_size, "kevin_spritesheets/image%d.png", index + 1) < (int)output_size;
}

static SDL_Texture *skin_select_load_texture(SDL_Renderer *renderer, const char *path) {
    SDL_Surface *surface;
    SDL_Texture *texture;

    if (!renderer || !path || !*path) return NULL;

    surface = IMG_Load(path);
    if (!surface) return NULL;

    texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);
    return texture;
}

static SDL_Texture *skin_select_create_text(SDL_Renderer *renderer, const char *text,
                                            SDL_Color color, int *w, int *h) {
    SDL_Surface *surface;
    SDL_Texture *texture;

    if (!renderer || !skinSelectFont || !text) return NULL;

    surface = TTF_RenderUTF8_Blended(skinSelectFont, text, color);
    if (!surface) return NULL;

    texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (w) *w = surface->w;
    if (h) *h = surface->h;
    SDL_FreeSurface(surface);
    return texture;
}

static SDL_Rect skin_select_frame_rect(SDL_Texture *texture, int frame_index) {
    SDL_Rect rect = {0, 0, 0, 0};
    int tex_w = 0;
    int tex_h = 0;
    int cell_w;
    int cell_h;
    int frame_count;
    int frame;

    if (!texture) return rect;
    if (SDL_QueryTexture(texture, NULL, NULL, &tex_w, &tex_h) != 0 || tex_w <= 0 || tex_h <= 0) {
        return rect;
    }

    cell_w = tex_w / SKIN_SELECT_COLS;
    cell_h = tex_h / SKIN_SELECT_ROWS;
    if (cell_w <= 0 || cell_h <= 0) return rect;

    frame_count = SKIN_SELECT_COLS * SKIN_SELECT_ROWS;
    frame = frame_index % frame_count;
    if (frame < 0) frame += frame_count;

    rect.x = (frame % SKIN_SELECT_COLS) * cell_w;
    rect.y = (frame / SKIN_SELECT_COLS) * cell_h;
    rect.w = cell_w;
    rect.h = cell_h;
    return rect;
}

static void skin_select_accept_choice(Game *game) {
    if (!game) return;

    game->selectedEnemySkinIndex = skinSelectTargetIndex;
    snprintf(game->selectedEnemySkinPath,
             sizeof(game->selectedEnemySkinPath),
             "kevin_spritesheets/image%d",
             skinSelectTargetIndex + 1);
    if (game->skinSelectReturnState == STATE_GAME || game->skinSelectReturnState == STATE_START_PLAY) {
        Game_SetSubState(game, game->skinSelectReturnState);
    } else {
        Game_SetSubState(game, STATE_START_PLAY);
    }
}

static void skin_select_destroy_textures(void) {
    if (skinSelectBackground) {
        SDL_DestroyTexture(skinSelectBackground);
        skinSelectBackground = NULL;
    }
    if (skinSelectTitleTex) {
        SDL_DestroyTexture(skinSelectTitleTex);
        skinSelectTitleTex = NULL;
    }
    if (skinSelectContinueTex) {
        SDL_DestroyTexture(skinSelectContinueTex);
        skinSelectContinueTex = NULL;
    }
    for (int i = 0; i < SKIN_SELECT_COUNT; i++) {
        if (skinSelectPreview[i]) {
            SDL_DestroyTexture(skinSelectPreview[i]);
            skinSelectPreview[i] = NULL;
        }
        if (skinSelectNameTex[i]) {
            SDL_DestroyTexture(skinSelectNameTex[i]);
            skinSelectNameTex[i] = NULL;
        }
    }
}

void SkinSelect_Cleanup(void) {
    skin_select_destroy_textures();
    if (skinSelectFont) {
        TTF_CloseFont(skinSelectFont);
        skinSelectFont = NULL;
    }
}

int SkinSelect_Charger(Game *game, SDL_Renderer *renderer) {
    SDL_Color white = {255, 255, 255, 255};
    char skin_path[256];

    if (!game || !renderer) return 0;
    if (game->skinSelectLoaded) return 1;

    SkinSelect_Cleanup();

    skinSelectFont = TTF_OpenFont("fonts/arial.ttf", 34);
    if (!skinSelectFont) skinSelectFont = TTF_OpenFont(GAME_ASSETS.fonts.system_bold, 34);
    if (!skinSelectFont) skinSelectFont = TTF_OpenFont(GAME_ASSETS.fonts.system_regular, 34);

    skinSelectBackground = skin_select_load_texture(renderer, "backgrounds/skin_background.png");
    if (!skinSelectBackground) {
        skinSelectBackground = skin_select_load_texture(renderer, "backgrounds/BG.png");
    }

    for (int i = 0; i < SKIN_SELECT_COUNT; i++) {
        if (skin_select_resolve_asset_path(i, skin_path, sizeof(skin_path))) {
            skinSelectPreview[i] = skin_select_load_texture(renderer, skin_path);
        }
        skinSelectNameTex[i] = skin_select_create_text(renderer, skinSelectNames[i], white, NULL, NULL);
    }

    skinSelectTitleTex = skin_select_create_text(renderer, "Enemy Skin Roulette", white, NULL, NULL);
    skinSelectContinueTex = skin_select_create_text(renderer, "Press Enter or click to continue", white, NULL, NULL);

    skinSelectCurrentIndex = 0;
    skinSelectTargetIndex = rand() % SKIN_SELECT_COUNT;
    skinSelectStepsDone = 0;
    skinSelectTotalSteps = (SKIN_SELECT_COUNT * 2) + skinSelectTargetIndex + 1 + (rand() % SKIN_SELECT_COUNT);
    skinSelectLocked = 0;
    skinSelectPreviewFrame = 0;
    skinSelectLastStepTick = SDL_GetTicks();
    skinSelectLastPreviewTick = SDL_GetTicks();

    game->skinSelectLoaded = 1;
    game->selectedEnemySkinIndex = -1;
    game->selectedEnemySkinPath[0] = '\0';
    return 1;
}

void SkinSelect_LectureEntree(Game *game) {
    SDL_Event event;

    if (!game) return;

    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) {
            game->running = 0;
            return;
        }

        if (!skinSelectLocked) continue;

        if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
            skin_select_accept_choice(game);
            return;
        }

        if (event.type == SDL_KEYDOWN &&
            (event.key.keysym.sym == SDLK_RETURN ||
             event.key.keysym.sym == SDLK_SPACE ||
             event.key.keysym.sym == SDLK_RIGHT)) {
            skin_select_accept_choice(game);
            return;
        }
    }
}

void SkinSelect_MiseAJour(Game *game) {
    Uint32 now = SDL_GetTicks();

    (void)game;

    if (!skinSelectLocked && now - skinSelectLastStepTick >= 120) {
        skinSelectCurrentIndex = (skinSelectCurrentIndex + 1) % SKIN_SELECT_COUNT;
        skinSelectStepsDone++;
        skinSelectLastStepTick = now;
        if (skinSelectStepsDone >= skinSelectTotalSteps) {
            skinSelectCurrentIndex = skinSelectTargetIndex;
            skinSelectLocked = 1;
        }
    }

    if (now - skinSelectLastPreviewTick >= 90) {
        skinSelectPreviewFrame = (skinSelectPreviewFrame + 1) % (SKIN_SELECT_COLS * SKIN_SELECT_ROWS);
        skinSelectLastPreviewTick = now;
    }
}

void SkinSelect_Affichage(Game *game, SDL_Renderer *renderer) {
    SDL_Color selected = {60, 220, 90, 255};
    SDL_Rect preview_panel = {760, 130, 420, 420};
    SDL_Rect title_box = {40, 36, 500, 60};
    SDL_Rect continue_box = {680, 610, 540, 36};
    SDL_Rect name_dst;

    (void)game;

    if (!renderer) return;

    if (skinSelectBackground) {
        SDL_Rect bg = {0, 0, WIDTH, HEIGHT};
        SDL_RenderCopy(renderer, skinSelectBackground, NULL, &bg);
    }

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 4, 10, 18, 135);
    SDL_RenderFillRect(renderer, NULL);

    if (skinSelectTitleTex) {
        SDL_Rect dst = title_box;
        SDL_QueryTexture(skinSelectTitleTex, NULL, NULL, &dst.w, &dst.h);
        SDL_RenderCopy(renderer, skinSelectTitleTex, NULL, &dst);
    }

    for (int i = 0; i < SKIN_SELECT_COUNT; i++) {
        int row = i / 3;
        int col = i % 3;
        SDL_Rect cell = {34 + (col * 180), 126 + (row * 170), 150, 150};
        SDL_Rect src = skin_select_frame_rect(skinSelectPreview[i], skinSelectPreviewFrame);

        if (i == skinSelectCurrentIndex) {
            SDL_Rect border = {cell.x - 5, cell.y - 5, cell.w + 10, cell.h + 10};
            SDL_SetRenderDrawColor(renderer, selected.r, selected.g, selected.b, selected.a);
            SDL_RenderDrawRect(renderer, &border);
        }

        if (skinSelectPreview[i] && src.w > 0 && src.h > 0) {
            SDL_RenderCopy(renderer, skinSelectPreview[i], &src, &cell);
        }
    }

    if (skinSelectPreview[skinSelectCurrentIndex]) {
        SDL_Rect src = skin_select_frame_rect(skinSelectPreview[skinSelectCurrentIndex], skinSelectPreviewFrame);
        SDL_Rect fitted = preview_panel;
        double scale_x;
        double scale_y;
        double scale;

        if (src.w > 0 && src.h > 0) {
            scale_x = (double)preview_panel.w / (double)src.w;
            scale_y = (double)preview_panel.h / (double)src.h;
            scale = (scale_x < scale_y) ? scale_x : scale_y;
            fitted.w = (int)(src.w * scale);
            fitted.h = (int)(src.h * scale);
            fitted.x = preview_panel.x + (preview_panel.w - fitted.w) / 2;
            fitted.y = preview_panel.y + (preview_panel.h - fitted.h) / 2;
            SDL_RenderCopy(renderer, skinSelectPreview[skinSelectCurrentIndex], &src, &fitted);
        }
    }

    if (skinSelectNameTex[skinSelectCurrentIndex]) {
        name_dst = (SDL_Rect){0, 0, 0, 0};
        SDL_QueryTexture(skinSelectNameTex[skinSelectCurrentIndex], NULL, NULL, &name_dst.w, &name_dst.h);
        name_dst.x = preview_panel.x + (preview_panel.w - name_dst.w) / 2;
        name_dst.y = preview_panel.y + preview_panel.h + 12;
        SDL_RenderCopy(renderer, skinSelectNameTex[skinSelectCurrentIndex], NULL, &name_dst);
    }

    if (skinSelectContinueTex && skinSelectLocked) {
        SDL_Rect dst = continue_box;
        SDL_QueryTexture(skinSelectContinueTex, NULL, NULL, &dst.w, &dst.h);
        dst.x = continue_box.x + (continue_box.w - dst.w) / 2;
        SDL_RenderCopy(renderer, skinSelectContinueTex, NULL, &dst);
    }
}
