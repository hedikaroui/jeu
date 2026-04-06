#include "game.h"
#include "assets_catalog.h"
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

static SDL_Rect menu_gift_rect(int w, int h, Uint32 ticks);
static SDL_Rect scoreStateJBtnRect = {0, 0, 0, 0};
static int scoreStateJBtnHover = 0;

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

static StartPlayAnimation startPlayAnim = {.dance_loop_channel = -1};

/* ═══════════════════════════════════════════════
   INITIALISATION GLOBALE
═══════════════════════════════════════════════ */
int Initialisation(Game *game, SDL_Window **window, SDL_Renderer **renderer) {
    memset(game, 0, sizeof(*game));

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        printf("Erreur SDL_Init: %s\n", SDL_GetError());
        return 0;
    }
    IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);
    TTF_Init();
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048);

    *window = SDL_CreateWindow("Menu Principal",
                               SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                               WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
    if (!*window) { printf("Erreur fenetre: %s\n", SDL_GetError()); return 0; }

    *renderer = SDL_CreateRenderer(*window, -1,
                                   SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!*renderer) { printf("Erreur renderer: %s\n", SDL_GetError()); return 0; }

    /* ── Assets menu ── */
    game->background   = IMG_LoadTexture(*renderer, GAME_ASSETS.backgrounds.menu);

    /* Logo top right – 300x300 */
    game->logoTexture  = IMG_LoadTexture(*renderer, GAME_ASSETS.characters.logo);
    game->logoRect     = (SDL_Rect){WIDTH - 320, 20, 300, 300};

    game->titleTexture = NULL;
    game->trapTexture  = NULL;

    game->music = Mix_LoadMUS(GAME_ASSETS.songs.menu_music);
    game->Sound = Mix_LoadWAV(SOUND_HOVER);

    /* Boutons : 5 empiles verticalement, LEFT side, taille augmentée 450x130 */
    int bw = 450, bh = 130, bsp = 25;
    int bx  = 40;
    int by0 = 180;

    for (int i = 0; i < 5; i++) {
        game->buttons[i].normalTex = IMG_LoadTexture(*renderer, MENU_BUTTON_NORMAL[i]);
        game->buttons[i].hoverTex  = IMG_LoadTexture(*renderer, MENU_BUTTON_HOVER[i]);
        game->buttons[i].rect      = (SDL_Rect){bx, by0 + i*(bh+bsp), bw, bh};
        game->buttons[i].selected  = 0;
    }

    /* ── Init sauvegarde ── */
    game->saveBg       = NULL;
    game->titleSurface = NULL;
    game->titleTexSave = NULL;
    game->saveMusic    = NULL;
    game->saveSound    = NULL;
    game->saveEtat     = 0;
    game->clic_bouton  = -1;
    for (int i = 0; i < 4; i++) {
        game->saveButtons[i].texture     = NULL;
        game->saveButtons[i].textureCliq = NULL;
        game->saveButtons[i].selected    = 0;
    }

    /* ── Init selection joueurs ── */
    game->ps_loaded         = 0;
    game->player1_name[0]   = '\0';
    game->player2_name[0]   = '\0';
    game->player1Tex        = NULL;
    game->player2Tex        = NULL;
    game->gameBgTex         = NULL;
    game->psJ1Tex           = NULL;
    game->psJ2Tex           = NULL;
    game->psKeyboardTex     = NULL;
    game->psKeyboardHoverTex= NULL;
    game->psManetteTex      = NULL;
    game->psManetteHoverTex = NULL;
    game->psSourisTex       = NULL;
    game->psSourisHoverTex  = NULL;
    game->psScoreBtnTex     = NULL;
    memset(&game->ps_bg, 0, sizeof(game->ps_bg));

    /* ── Leaderboard assets ── */
    game->msGreTex  = IMG_LoadTexture(*renderer, SCORE_BACK_HOVER);
    game->msRedTex  = IMG_LoadTexture(*renderer, SCORE_BACK_NORMAL);
    game->leaderTex = IMG_LoadTexture(*renderer, GAME_ASSETS.backgrounds.leaderboard);
    game->click     = Mix_LoadWAV(SOUND_CLICK);
    game->scoreJ1Tex = IMG_LoadTexture(*renderer, SCORE_BUTTON_NORMAL);
    game->scoreJ2Tex = IMG_LoadTexture(*renderer, SCORE_BUTTON_HOVER);
    game->startHeartTex = NULL;
    game->startTextTex  = NULL;
    game->startTextRect = (SDL_Rect){0, 0, 0, 0};
    game->startPlayLoaded = 0;

    game->backBtn.rect      = (SDL_Rect){WIDTH - 220, 20, 200, 70};
    game->backBtn.selected  = 0;
    game->backBtn.normalTex = game->msRedTex;
    game->backBtn.hoverTex  = game->msGreTex;

    game->searchBox    = (SDL_Rect){(WIDTH - 400) / 2, 140, 400, 50};
    scoreStateJBtnRect = (SDL_Rect){(WIDTH - 320) / 2, HEIGHT - 130, 320, 90};
    scoreStateJBtnHover = 0;
    game->inputActive  = 0;
    game->inputText[0] = '\0';

    /* ── Options init ── */
    game->optionsBg           = NULL;
    game->optionsTitle        = NULL;
    game->volumePlusBtn       = NULL;
    game->volumeMinusBtn      = NULL;
    game->volumeMuteBtn       = NULL;
    game->volumePlusHoverBtn  = NULL;
    game->volumeMinusHoverBtn = NULL;
    game->volumeMuteHoverBtn  = NULL;
    game->fullscreenBtn       = NULL;
    game->normalscreenBtn     = NULL;
    game->optionsClick        = NULL;
    game->optionsFullscreen   = 0;
    game->optionsLoaded       = 0;

    /* ── Games init ── */
    game->menuGiftTex         = IMG_LoadTexture(*renderer, GAME_ASSETS.backgrounds.gift);
    game->gamesImg            = NULL;
    game->gamesLoaded         = 0;
    memset(&game->gamesRect, 0, sizeof(game->gamesRect));
    game->quizBg1             = NULL;
    game->quizBg2             = NULL;
    game->quizBtnA            = NULL;
    game->quizBtnB            = NULL;
    game->quizBtnC            = NULL;
    game->quizFont            = NULL;
    game->quizMusic           = NULL;
    game->quizBeep            = NULL;
    game->quizBeep2           = NULL;
    game->quizLaugh           = NULL;

    /* Police leaderboard */
    game->font = TTF_OpenFont(GAME_ASSETS.fonts.system_bold, 32);
    if (!game->font)
        game->font = TTF_OpenFont(GAME_ASSETS.fonts.system_regular, 32);

    if (game->music) Mix_PlayMusic(game->music, -1);

    game->window       = *window;
    game->volume       = Mix_VolumeMusic(-1);
    game->fullscreen   = 0;
    game->currentState = STATE_MENU;
    game->running      = 1;
    return 1;
}

/* ═══════════════════════════════════════════════
   LIBERATION
═══════════════════════════════════════════════ */
void Liberation(Game *game, SDL_Window *window, SDL_Renderer *renderer) {
    if (game->music)        Mix_FreeMusic(game->music);
    if (game->Sound)        Mix_FreeChunk(game->Sound);
    if (game->font)         TTF_CloseFont(game->font);
    if (game->background)   SDL_DestroyTexture(game->background);
    if (game->logoTexture)  SDL_DestroyTexture(game->logoTexture);
    if (game->titleTexture) SDL_DestroyTexture(game->titleTexture);
    if (game->trapTexture)  SDL_DestroyTexture(game->trapTexture);
    for (int i = 0; i < 5; i++) {
        if (game->buttons[i].normalTex) SDL_DestroyTexture(game->buttons[i].normalTex);
        if (game->buttons[i].hoverTex)  SDL_DestroyTexture(game->buttons[i].hoverTex);
    }
    if (game->saveMusic)    Mix_FreeMusic(game->saveMusic);
    if (game->saveSound)    Mix_FreeChunk(game->saveSound);
    if (game->titleSurface) SDL_FreeSurface(game->titleSurface);
    if (game->titleTexSave) SDL_DestroyTexture(game->titleTexSave);
    if (game->saveBg)       SDL_DestroyTexture(game->saveBg);
    for (int i = 0; i < 4; i++) {
        if (game->saveButtons[i].texture)     SDL_DestroyTexture(game->saveButtons[i].texture);
        if (game->saveButtons[i].textureCliq) SDL_DestroyTexture(game->saveButtons[i].textureCliq);
    }
    if (game->ps_bg.texture) SDL_DestroyTexture(game->ps_bg.texture);
    if (game->ps_bg.music)   { Mix_HaltMusic(); Mix_FreeMusic(game->ps_bg.music); }
    if (game->player1Tex)    SDL_DestroyTexture(game->player1Tex);
    if (game->player2Tex)    SDL_DestroyTexture(game->player2Tex);
    if (game->gameBgTex)     SDL_DestroyTexture(game->gameBgTex);
    if (game->psJ1Tex)       SDL_DestroyTexture(game->psJ1Tex);
    if (game->psJ2Tex)       SDL_DestroyTexture(game->psJ2Tex);
    if (game->psKeyboardTex) SDL_DestroyTexture(game->psKeyboardTex);
    if (game->psKeyboardHoverTex) SDL_DestroyTexture(game->psKeyboardHoverTex);
    if (game->psManetteTex)  SDL_DestroyTexture(game->psManetteTex);
    if (game->psManetteHoverTex) SDL_DestroyTexture(game->psManetteHoverTex);
    if (game->psSourisTex)   SDL_DestroyTexture(game->psSourisTex);
    if (game->psSourisHoverTex) SDL_DestroyTexture(game->psSourisHoverTex);
    if (game->psScoreBtnTex) SDL_DestroyTexture(game->psScoreBtnTex);
    if (game->msGreTex)  SDL_DestroyTexture(game->msGreTex);
    if (game->msRedTex)  SDL_DestroyTexture(game->msRedTex);
    if (game->leaderTex) SDL_DestroyTexture(game->leaderTex);
    if (game->scoreJ1Tex) SDL_DestroyTexture(game->scoreJ1Tex);
    if (game->scoreJ2Tex) SDL_DestroyTexture(game->scoreJ2Tex);
    if (game->startHeartTex) SDL_DestroyTexture(game->startHeartTex);
    if (game->startTextTex) SDL_DestroyTexture(game->startTextTex);
    if (startPlayAnim.walk_tex) SDL_DestroyTexture(startPlayAnim.walk_tex);
    if (startPlayAnim.walk_back_tex) SDL_DestroyTexture(startPlayAnim.walk_back_tex);
    if (startPlayAnim.stop_tex) SDL_DestroyTexture(startPlayAnim.stop_tex);
    if (startPlayAnim.coin_tex) SDL_DestroyTexture(startPlayAnim.coin_tex);
    if (startPlayAnim.dance_tex) SDL_DestroyTexture(startPlayAnim.dance_tex);
    if (startPlayAnim.wave_tex) SDL_DestroyTexture(startPlayAnim.wave_tex);
    if (startPlayAnim.dance_loop_channel >= 0) Mix_HaltChannel(startPlayAnim.dance_loop_channel);
    if (startPlayAnim.dance_loop_sfx) Mix_FreeChunk(startPlayAnim.dance_loop_sfx);
    if (game->click)     Mix_FreeChunk(game->click);

    /* Options cleanup */
    if (game->optionsBg)           SDL_DestroyTexture(game->optionsBg);
    if (game->optionsTitle)        SDL_DestroyTexture(game->optionsTitle);
    if (game->volumePlusBtn)       SDL_DestroyTexture(game->volumePlusBtn);
    if (game->volumeMinusBtn)      SDL_DestroyTexture(game->volumeMinusBtn);
    if (game->volumeMuteBtn)       SDL_DestroyTexture(game->volumeMuteBtn);
    if (game->volumePlusHoverBtn)  SDL_DestroyTexture(game->volumePlusHoverBtn);
    if (game->volumeMinusHoverBtn) SDL_DestroyTexture(game->volumeMinusHoverBtn);
    if (game->volumeMuteHoverBtn)  SDL_DestroyTexture(game->volumeMuteHoverBtn);
    if (game->fullscreenBtn)       SDL_DestroyTexture(game->fullscreenBtn);
    if (game->normalscreenBtn)     SDL_DestroyTexture(game->normalscreenBtn);
    if (game->optionsClick)        Mix_FreeChunk(game->optionsClick);

    /* Games cleanup */
    if (game->menuGiftTex)        SDL_DestroyTexture(game->menuGiftTex);
    if (game->gamesImg)            SDL_DestroyTexture(game->gamesImg);
    if (game->quizBg1)            SDL_DestroyTexture(game->quizBg1);
    if (game->quizBg2)            SDL_DestroyTexture(game->quizBg2);
    if (game->quizBtnA)           SDL_DestroyTexture(game->quizBtnA);
    if (game->quizBtnB)           SDL_DestroyTexture(game->quizBtnB);
    if (game->quizBtnC)           SDL_DestroyTexture(game->quizBtnC);
    if (game->quizFont)           TTF_CloseFont(game->quizFont);
    if (game->quizMusic)          Mix_FreeMusic(game->quizMusic);
    if (game->quizBeep)           Mix_FreeChunk(game->quizBeep);
    if (game->quizBeep2)          Mix_FreeChunk(game->quizBeep2);
    if (game->quizLaugh)          Mix_FreeChunk(game->quizLaugh);

    Mix_CloseAudio();
    TTF_Quit();
    IMG_Quit();
    if (renderer) SDL_DestroyRenderer(renderer);
    if (window)   SDL_DestroyWindow(window);
    SDL_Quit();
}

/* ═══════════════════════════════════════════════
   MENU – INPUT
═══════════════════════════════════════════════ */
void Menu_LectureEntree(Game *game) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) { game->running = 0; return; }

        if (event.type == SDL_KEYDOWN) {
            if (event.key.keysym.sym == SDLK_ESCAPE) { game->currentState = STATE_QUIT; return; }
            if (event.key.keysym.sym == SDLK_j) { game->currentState = STATE_SAVE; return; }
            if (event.key.keysym.sym == SDLK_o) { game->currentState = STATE_OPTIONS; return; }
            if (event.key.keysym.sym == SDLK_m) { game->currentState = STATE_SCORES_INPUT; return; }
        }

        if (event.type == SDL_MOUSEMOTION) {
            int mx = event.motion.x, my = event.motion.y;
            for (int i = 0; i < 5; i++) {
                int prev = game->buttons[i].selected;
                game->buttons[i].selected =
                    SDL_PointInRect(&(SDL_Point){mx, my}, &game->buttons[i].rect);
                if (game->buttons[i].selected && !prev && game->Sound)
                    Mix_PlayChannel(-1, game->Sound, 0);
            }
        }

        if (event.type == SDL_MOUSEBUTTONDOWN) {
            int mx = event.button.x, my = event.button.y;
            int w = WIDTH, h = HEIGHT;
            if (game->window) SDL_GetWindowSize(game->window, &w, &h);
            SDL_Rect gift = menu_gift_rect(w, h, SDL_GetTicks());

            if (mx >= gift.x && mx <= gift.x + gift.w &&
                my >= gift.y && my <= gift.y + gift.h) {
                if (game->click) Mix_PlayChannel(-1, game->click, 0);
                game->currentState = STATE_ENIGME_QUIZ;
                return;
            }

            if (SDL_PointInRect(&(SDL_Point){mx,my}, &game->buttons[0].rect))
                game->currentState = STATE_GAME; /* directly enter game which hosts player-select */
            if (SDL_PointInRect(&(SDL_Point){mx,my}, &game->buttons[1].rect))
                game->currentState = STATE_OPTIONS;
            if (SDL_PointInRect(&(SDL_Point){mx,my}, &game->buttons[2].rect))
                game->currentState = STATE_SAVE;
            if (SDL_PointInRect(&(SDL_Point){mx,my}, &game->buttons[3].rect)) {
                if (game->click) Mix_PlayChannel(-1, game->click, 0);
                game->currentState = STATE_SCORES_INPUT;
            }
            if (SDL_PointInRect(&(SDL_Point){mx,my}, &game->buttons[4].rect))
                game->currentState = STATE_QUIT;
        }
    }
}

/* ═══════════════════════════════════════════════
   MENU – RENDU
═══════════════════════════════════════════════ */
void Menu_Affichage(Game *game, SDL_Renderer *renderer) {
    if (game->background)
        SDL_RenderCopy(renderer, game->background, NULL, NULL);
    if (game->logoTexture)
        SDL_RenderCopy(renderer, game->logoTexture, NULL, &game->logoRect);
    for (int i = 0; i < 5; i++) {
        SDL_Texture *t = game->buttons[i].selected
                         ? game->buttons[i].hoverTex
                         : game->buttons[i].normalTex;
        if (t) SDL_RenderCopy(renderer, t, NULL, &game->buttons[i].rect);
    }

    if (game->menuGiftTex) {
        int w = WIDTH, h = HEIGHT;
        if (game->window) SDL_GetWindowSize(game->window, &w, &h);
        SDL_Rect gift = menu_gift_rect(w, h, SDL_GetTicks());
        SDL_RenderCopy(renderer, game->menuGiftTex, NULL, &gift);
    }
}

/* ═══════════════════════════════════════════════
   LEADERBOARD – INPUT
═══════════════════════════════════════════════ */
void Leaderboard_LectureEntree(Game *game) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) { game->running = 0; return; }

        if (e.type == SDL_KEYDOWN) {
            if (e.key.keysym.sym == SDLK_ESCAPE) {
                game->inputActive  = 0;
                SDL_StopTextInput();
                if (game->currentState == STATE_SCORES_LIST) {
                    game->currentState = STATE_QUIT;
                } else {
                    game->currentState = STATE_MENU;
                    if (game->music) Mix_PlayMusic(game->music, -1);
                }
                return;
            }
            if (e.key.keysym.sym == SDLK_e) {
                game->currentState = STATE_ENIGME_QUIZ;
                return;
            }
        }

        if (e.type == SDL_MOUSEBUTTONDOWN) {
            int mx = e.button.x, my = e.button.y;
            if (SDL_PointInRect(&(SDL_Point){mx,my}, &game->backBtn.rect)) {
                if (game->click) Mix_PlayChannel(-1, game->click, 0);
                game->currentState = STATE_MENU;
                game->inputActive  = 0;
                SDL_StopTextInput();
                if (game->music) Mix_PlayMusic(game->music, -1);
                return;
            }
            if (SDL_PointInRect(&(SDL_Point){mx,my}, &scoreStateJBtnRect)) {
                if (game->click) Mix_PlayChannel(-1, game->click, 0);
                game->currentState = STATE_START_PLAY;
                return;
            }
            if (SDL_PointInRect(&(SDL_Point){mx,my}, &game->searchBox)) {
                game->inputActive = 1; SDL_StartTextInput();
            } else {
                game->inputActive = 0; SDL_StopTextInput();
            }
        }
        if (e.type == SDL_MOUSEMOTION) {
            scoreStateJBtnHover = SDL_PointInRect(&(SDL_Point){e.motion.x, e.motion.y}, &scoreStateJBtnRect);
        }

        if (game->inputActive && e.type == SDL_TEXTINPUT) {
            if (strlen(game->inputText) + strlen(e.text.text) < sizeof(game->inputText)-1)
                strcat(game->inputText, e.text.text);
        }
        if (game->inputActive && e.type == SDL_KEYDOWN) {
            if (e.key.keysym.sym == SDLK_BACKSPACE && strlen(game->inputText) > 0)
                game->inputText[strlen(game->inputText)-1] = '\0';
            if (e.key.keysym.sym == SDLK_RETURN) {
                printf("Recherche joueur: %s\n", game->inputText);
                game->currentState = STATE_SCORES_LIST;
                return;
            }
        }
    }
}

/* ═══════════════════════════════════════════════
   LEADERBOARD – RENDU
═══════════════════════════════════════════════ */
void Leaderboard_Affichage(Game *game, SDL_Renderer *renderer) {
    if (game->leaderTex)
        SDL_RenderCopy(renderer, game->leaderTex, NULL, NULL);

    /* Bouton retour */
    SDL_Texture *bt = game->backBtn.selected ? game->msGreTex : game->msRedTex;
    if (bt) SDL_RenderCopy(renderer, bt, NULL, &game->backBtn.rect);

    /* Fond searchbox */
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 160);
    SDL_RenderFillRect(renderer, &game->searchBox);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderDrawRect(renderer, &game->searchBox);

    const char *display = strlen(game->inputText) > 0
                          ? game->inputText
                          : (game->inputActive ? "" : "Rechercher un joueur...");
    if (game->font && strlen(display) > 0) {
        SDL_Color col = strlen(game->inputText) > 0
                        ? (SDL_Color){255,255,255,255}
                        : (SDL_Color){180,180,180,255};
        SDL_Surface *surf = TTF_RenderUTF8_Blended(game->font, display, col);
        if (surf) {
            SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, surf);
            if (tex) {
                SDL_Rect tr = {
                    game->searchBox.x + 10,
                    game->searchBox.y + (game->searchBox.h - surf->h) / 2,
                    surf->w < game->searchBox.w-20 ? surf->w : game->searchBox.w-20,
                    surf->h
                };
                SDL_RenderCopy(renderer, tex, NULL, &tr);
                SDL_DestroyTexture(tex);
            }
            SDL_FreeSurface(surf);
        }
    }

    /* Curseur */
    static Uint32 last_blink = 0;
    static int    cursor_on  = 1;
    Uint32 now = SDL_GetTicks();
    if (now - last_blink > 500) { cursor_on = !cursor_on; last_blink = now; }
    if (game->inputActive && cursor_on) {
        int cx = game->searchBox.x + 10;
        if (strlen(game->inputText) > 0 && game->font) {
            int tw; TTF_SizeUTF8(game->font, game->inputText, &tw, NULL);
            cx += tw;
        }
        SDL_SetRenderDrawColor(renderer, 255,255,255,255);
        SDL_RenderDrawLine(renderer, cx, game->searchBox.y+6, cx, game->searchBox.y+game->searchBox.h-6);
    }

    {
        SDL_Rect drawRect = scoreStateJBtnRect;
        SDL_Texture *t = scoreStateJBtnHover ? game->scoreJ2Tex : game->scoreJ1Tex;
        if (t) {
            if (scoreStateJBtnHover) drawRect.y -= 4;
            SDL_RenderCopy(renderer, t, NULL, &drawRect);
        }
    }
}

/* ═══════════════════════════════════════════════
   SAUVEGARDE – CHARGEMENT DES ASSETS
═══════════════════════════════════════════════ */
int Save_Charger(Game *game, SDL_Renderer *renderer) {
    if (game->saveBg) return 1;

    game->saveBg = IMG_LoadTexture(renderer, GAME_ASSETS.backgrounds.save_primary);
    if (!game->saveBg)
        game->saveBg = IMG_LoadTexture(renderer, GAME_ASSETS.backgrounds.save_fallback);
    if (!game->saveBg) { printf("Erreur BG.png/LB.jpg: %s\n", IMG_GetError()); return 0; }

    game->saveMusic = Mix_LoadMUS(GAME_ASSETS.songs.save_music_primary);
    if (!game->saveMusic)
        game->saveMusic = Mix_LoadMUS(GAME_ASSETS.songs.save_music_fallback);
    if (game->saveMusic) Mix_PlayMusic(game->saveMusic, -1);
    game->saveSound = Mix_LoadWAV(SOUND_SAVE_CLICK);

    TTF_Font *f = TTF_OpenFont(GAME_ASSETS.fonts.system_bold, 36);
    if (!f) f = TTF_OpenFont(GAME_ASSETS.fonts.system_regular, 36);
    if (f) {
        SDL_Color white = {255,255,255,255};
        SDL_Color red   = {178,34,34,255};
        game->titleSurface = TTF_RenderUTF8_Shaded(f,
                             "Voulez-vous sauvegarder la partie ?", white, red);
        TTF_CloseFont(f);
    }
    if (game->titleSurface) {
        game->titleTexSave  = SDL_CreateTextureFromSurface(renderer, game->titleSurface);
        game->titleRectSave = (SDL_Rect){
            (WIDTH  - game->titleSurface->w) / 2, 30,
             game->titleSurface->w, game->titleSurface->h
        };
    }

    /* Boutons plus grands et mieux centres */
    game->saveButtons[0].rect        = (SDL_Rect){200, 300, 380, 200};
    game->saveButtons[0].texture     = IMG_LoadTexture(renderer, SAVE_BUTTON_NORMAL[0]);
    game->saveButtons[0].textureCliq = IMG_LoadTexture(renderer, SAVE_BUTTON_CLICKED[0]);
    game->saveButtons[1].rect        = (SDL_Rect){700, 300, 380, 200};
    game->saveButtons[1].texture     = IMG_LoadTexture(renderer, SAVE_BUTTON_NORMAL[1]);
    game->saveButtons[1].textureCliq = IMG_LoadTexture(renderer, SAVE_BUTTON_CLICKED[1]);
    game->saveButtons[2].rect        = (SDL_Rect){240, 220, 420, 180};
    game->saveButtons[2].texture     = IMG_LoadTexture(renderer, SAVE_BUTTON_NORMAL[2]);
    game->saveButtons[2].textureCliq = IMG_LoadTexture(renderer, SAVE_BUTTON_CLICKED[2]);
    game->saveButtons[3].rect        = (SDL_Rect){620, 220, 420, 180};
    game->saveButtons[3].texture     = IMG_LoadTexture(renderer, SAVE_BUTTON_NORMAL[3]);
    game->saveButtons[3].textureCliq = IMG_LoadTexture(renderer, SAVE_BUTTON_CLICKED[3]);

    game->saveEtat    = 0;
    game->clic_bouton = -1;
    return 1;
}

/* ═══════════════════════════════════════════════
   SAUVEGARDE – INPUT
═══════════════════════════════════════════════ */
void Save_LectureEntree(Game *game) {
    SDL_Event event;
    game->clic_bouton = -1;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) { game->running = 0; return; }
        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
            game->currentState = STATE_MENU;
            if (game->music) Mix_PlayMusic(game->music, -1);
            return;
        }
        if (event.type == SDL_MOUSEMOTION) {
            int mx = event.motion.x, my = event.motion.y;
            for (int i = 0; i < 4; i++) game->saveButtons[i].selected = 0;
            if (game->saveEtat == 0) {
                if (SDL_PointInRect(&(SDL_Point){mx,my}, &game->saveButtons[0].rect)) game->saveButtons[0].selected = 1;
                if (SDL_PointInRect(&(SDL_Point){mx,my}, &game->saveButtons[1].rect)) game->saveButtons[1].selected = 1;
            } else {
                if (SDL_PointInRect(&(SDL_Point){mx,my}, &game->saveButtons[2].rect)) game->saveButtons[2].selected = 1;
                if (SDL_PointInRect(&(SDL_Point){mx,my}, &game->saveButtons[3].rect)) game->saveButtons[3].selected = 1;
            }
        }
        if (event.type == SDL_MOUSEBUTTONDOWN) {
            int mx = event.button.x, my = event.button.y;
            if (game->saveEtat == 0) {
                if (SDL_PointInRect(&(SDL_Point){mx,my}, &game->saveButtons[0].rect)) game->clic_bouton = 0;
                if (SDL_PointInRect(&(SDL_Point){mx,my}, &game->saveButtons[1].rect)) game->clic_bouton = 1;
            } else {
                if (SDL_PointInRect(&(SDL_Point){mx,my}, &game->saveButtons[2].rect)) game->clic_bouton = 2;
                if (SDL_PointInRect(&(SDL_Point){mx,my}, &game->saveButtons[3].rect)) game->clic_bouton = 3;
            }
        }
    }
}

/* ═══════════════════════════════════════════════
   SAUVEGARDE – RENDU
═══════════════════════════════════════════════ */
void Save_Affichage(Game *game, SDL_Renderer *renderer) {
    SDL_RenderClear(renderer);
    if (game->saveBg) SDL_RenderCopy(renderer, game->saveBg, NULL, NULL);
    if (game->saveEtat == 0) {
        if (game->titleTexSave)
            SDL_RenderCopy(renderer, game->titleTexSave, NULL, &game->titleRectSave);
        for (int i = 0; i < 2; i++) {
            SDL_Texture *t = game->saveButtons[i].selected
                             ? game->saveButtons[i].textureCliq
                             : game->saveButtons[i].texture;
            if (t) SDL_RenderCopy(renderer, t, NULL, &game->saveButtons[i].rect);
        }
    } else {
        for (int i = 2; i < 4; i++) {
            SDL_Texture *t = game->saveButtons[i].selected
                             ? game->saveButtons[i].textureCliq
                             : game->saveButtons[i].texture;
            if (t) SDL_RenderCopy(renderer, t, NULL, &game->saveButtons[i].rect);
        }
    }
    SDL_RenderPresent(renderer);
}

/* ═══════════════════════════════════════════════
   SAUVEGARDE – LOGIQUE
═══════════════════════════════════════════════ */
void Save_MiseAJour(Game *game) {
    if (game->clic_bouton == 0) {
        if (game->saveSound) Mix_PlayChannel(-1, game->saveSound, 0);
        game->saveEtat = 1;
        game->currentState = STATE_SAVE_CHOICE;
    }
    if (game->clic_bouton == 1) {
        if (game->saveSound) Mix_PlayChannel(-1, game->saveSound, 0);
        game->saveEtat = 0; game->currentState = STATE_MENU;
        if (game->music) Mix_PlayMusic(game->music, -1);
    }
    if (game->clic_bouton == 2) {
        if (game->saveSound) Mix_PlayChannel(-1, game->saveSound, 0);
        printf("Chargement sauvegarde\n");
        game->saveEtat = 0;
        game->currentState = STATE_PLAYER;
    }
    if (game->clic_bouton == 3) {
        if (game->saveSound) Mix_PlayChannel(-1, game->saveSound, 0);
        printf("Nouvelle partie\n");
        game->saveEtat = 0;
        game->currentState = STATE_PLAYER;
    }
    SDL_Delay(16);
}

/* ═══════════════════════════════════════════════════════════════════
   SELECTION JOUEURS – UI rectangulaire (sans Sequence)
═══════════════════════════════════════════════════════════════════ */

static SDL_Rect ps_p1_frame, ps_p2_frame;
static SDL_Rect ps_j1_btn, ps_j2_btn;
static SDL_Rect ps_input1, ps_input2;
static SDL_Rect ps_tools[3];
static SDL_Rect ps_score_btn;
static int ps_focus_field = 0;   /* 0 none, 1 input1, 2 input2 */
static int ps_cursor_on = 1;
static Uint32 ps_cursor_timer = 0;
static int ps_hover_j1 = 0, ps_hover_j2 = 0;
static int ps_last_hover_j1 = 0, ps_last_hover_j2 = 0;
static int ps_hover_tools[3] = {0, 0, 0};
static int ps_last_hover_tools[3] = {0, 0, 0};
static int ps_hover_score = 0;
static int ps_last_hover_score = 0;
static char ps_name1[256];
static char ps_name2[256];
static int ps_cursor1 = 0, ps_cursor2 = 0;

static SDL_Rect menu_gift_rect(int w, int h, Uint32 ticks) {
    int shakeX = (int)(3.0 * sin((double)ticks / 55.0));
    int shakeY = (int)(2.0 * cos((double)ticks / 70.0));
    return (SDL_Rect){w - 170 + shakeX, h - 170 + shakeY, 140, 140};
}

static SDL_Rect quizBtnARect = {120, 430, 150, 120};
static SDL_Rect quizBtnBRect = {325, 430, 150, 120};
static SDL_Rect quizBtnCRect = {530, 430, 150, 120};
static int quizHoverA = 0, quizHoverB = 0, quizHoverC = 0;
static int quizSelected = -1;

static Uint32 startPlayIntroStart = 0;
static SDL_Rect startPlayPlayerRect = {0, 0, 120, 120};
static Uint32 startPlayLastTick = 0;

typedef struct {
    double x, y;
    double vitesse;
    double acceleration;
    SDL_Rect position_acc;
} StartPlayMover;

static StartPlayMover startPlayMover = {0};

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

    if (SDL_QueryTexture(tex, NULL, NULL, &w, &h) != 0 || w <= 0 || h <= 0) {
        return;
    }

    int r = wanted_rows;
    int c = wanted_cols;
    if (r < 1) r = 1;
    if (c < 1) c = 1;

    /* PDF method: matrix with regular frames (rows x cols). */
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
            surf->w, surf->h
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

/* ═══════════════════════════════════════════════
   SELECTION JOUEURS – CHARGEMENT DES ASSETS
═══════════════════════════════════════════════ */
int PlayerSelect_Charger(Game *game, SDL_Renderer *renderer) {
    if (game->ps_loaded) return 1;

    SDL_Surface *surf = IMG_Load(GAME_ASSETS.backgrounds.main);
    if (surf) {
        game->ps_bg.texture = SDL_CreateTextureFromSurface(renderer, surf);
        game->ps_bg.width   = surf->w; game->ps_bg.height = surf->h;
        game->ps_bg.dest_rect = (SDL_Rect){0, 0, WIDTH, HEIGHT};
        SDL_FreeSurface(surf);
    } else printf("Avertissement: background_main.jpg introuvable\n");

    game->ps_bg.music = Mix_LoadMUS(SOUND_BACKGROUND_LOOP);
    if (game->ps_bg.music) {
        game->ps_bg.music_volume = 48;
        Mix_VolumeMusic(48);
        Mix_PlayMusic(game->ps_bg.music, -1);
    }

    /* Load hover sound if not already loaded */
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

    ps_p1_frame = (SDL_Rect){40, 80, 360, 300};
    ps_p2_frame = (SDL_Rect){WIDTH - 400, 80, 360, 300};
    ps_j1_btn   = (SDL_Rect){ps_p1_frame.x + (ps_p1_frame.w - 140) / 2, ps_p1_frame.y - 52, 140, 44};
    ps_j2_btn   = (SDL_Rect){ps_p2_frame.x + (ps_p2_frame.w - 140) / 2, ps_p2_frame.y - 52, 140, 44};
    ps_input1   = (SDL_Rect){40, 410, 360, 56};
    ps_input2   = (SDL_Rect){WIDTH - 400, 410, 360, 56};
    {
        int iconW = 130;
        int iconH = 88;
        int gap = 34;
        int rowY = 500;
        int totalW = iconW * 3 + gap * 2;
        int startX = (WIDTH - totalW) / 2;
        ps_tools[0] = (SDL_Rect){startX, rowY, iconW, iconH};
        ps_tools[1] = (SDL_Rect){startX + iconW + gap, rowY, iconW, iconH};
        ps_tools[2] = (SDL_Rect){startX + 2 * (iconW + gap), rowY, iconW, iconH};
    }
    ps_score_btn = (SDL_Rect){(WIDTH - 360) / 2, HEIGHT - 100, 360, 76};

    strncpy(ps_name1, game->player1_name, sizeof(ps_name1) - 1);
    strncpy(ps_name2, game->player2_name, sizeof(ps_name2) - 1);
    ps_name1[sizeof(ps_name1) - 1] = '\0';
    ps_name2[sizeof(ps_name2) - 1] = '\0';
    ps_cursor1 = (int)strlen(ps_name1);
    ps_cursor2 = (int)strlen(ps_name2);
    ps_focus_field = 0;
    ps_hover_j1 = ps_hover_j2 = 0;
    ps_last_hover_j1 = ps_last_hover_j2 = 0;
    ps_hover_tools[0] = ps_hover_tools[1] = ps_hover_tools[2] = 0;
    ps_last_hover_tools[0] = ps_last_hover_tools[1] = ps_last_hover_tools[2] = 0;
    ps_hover_score = 0;
    ps_last_hover_score = 0;
    ps_cursor_timer = SDL_GetTicks();
    ps_cursor_on = 1;

    game->ps_loaded = 1;
    printf("PlayerSelect: assets charges\n");
    return 1;
}

/* ═══════════════════════════════════════════════
   SELECTION JOUEURS – INPUT
═══════════════════════════════════════════════ */
void PlayerSelect_LectureEntree(Game *game) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) { game->running = 0; return; }

        if (e.type == SDL_MOUSEMOTION) {
            int mx = e.motion.x, my = e.motion.y;
            ps_hover_j1 = ps_point_in_rect(ps_j1_btn, mx, my);
            ps_hover_j2 = ps_point_in_rect(ps_j2_btn, mx, my);
            for (int i = 0; i < 3; i++)
                ps_hover_tools[i] = ps_point_in_rect(ps_tools[i], mx, my);
            ps_hover_score = ps_point_in_rect(ps_score_btn, mx, my);

            if (game->click && (
                (ps_hover_j1 && !ps_last_hover_j1) ||
                (ps_hover_j2 && !ps_last_hover_j2) ||
                (ps_hover_tools[0] && !ps_last_hover_tools[0]) ||
                (ps_hover_tools[1] && !ps_last_hover_tools[1]) ||
                (ps_hover_tools[2] && !ps_last_hover_tools[2]) ||
                (ps_hover_score && !ps_last_hover_score)))
                Mix_PlayChannel(-1, game->click, 0);
            ps_last_hover_j1 = ps_hover_j1;
            ps_last_hover_j2 = ps_hover_j2;
            ps_last_hover_tools[0] = ps_hover_tools[0];
            ps_last_hover_tools[1] = ps_hover_tools[1];
            ps_last_hover_tools[2] = ps_hover_tools[2];
            ps_last_hover_score = ps_hover_score;
        }

        if (e.type == SDL_KEYDOWN) {
            SDL_Keycode sym = e.key.keysym.sym;

            if (sym == SDLK_ESCAPE) {
                if (ps_focus_field != 0) {
                    ps_focus_field = 0;
                    SDL_StopTextInput();
                } else {
                    game->currentState = STATE_MENU;
                    if (game->music) Mix_PlayMusic(game->music, -1);
                }
                return;
            }

            if (ps_focus_field == 0) {
                if (sym == SDLK_PLUS || sym == SDLK_EQUALS) {
                    int v = game->ps_bg.music_volume + 5; if (v>128) v=128;
                    game->ps_bg.music_volume = v; Mix_VolumeMusic(v);
                }
                if (sym == SDLK_MINUS || sym == SDLK_UNDERSCORE) {
                    int v = game->ps_bg.music_volume - 5; if (v<0) v=0;
                    game->ps_bg.music_volume = v; Mix_VolumeMusic(v);
                }
                if (sym == SDLK_p) Mix_PauseMusic();
                if (sym == SDLK_r) Mix_ResumeMusic();
            }

            if (sym == SDLK_TAB) {
                ps_focus_field = (ps_focus_field == 1) ? 2 : 1;
                SDL_StartTextInput();
            }

            if (sym == SDLK_RETURN) {
                strncpy(game->player1_name, ps_name1, sizeof(game->player1_name) - 1);
                strncpy(game->player2_name, ps_name2, sizeof(game->player2_name) - 1);
                game->player1_name[sizeof(game->player1_name) - 1] = '\0';
                game->player2_name[sizeof(game->player2_name) - 1] = '\0';
                if (strlen(game->player1_name) == 0) strcpy(game->player1_name, "Player1");
                if (strlen(game->player2_name) == 0) strcpy(game->player2_name, "Player2");
                ps_focus_field = 0;
                SDL_StopTextInput();
                printf("Joueur 1: %s | Joueur 2: %s\n", game->player1_name, game->player2_name);
                game->currentState = STATE_GAME;
                return;
            }

            if (ps_focus_field == 1) ps_handle_text_input(ps_name1, &ps_cursor1, sym);
            if (ps_focus_field == 2) ps_handle_text_input(ps_name2, &ps_cursor2, sym);
        }

        if (e.type == SDL_TEXTINPUT) {
            char *buf = NULL;
            int *cursor = NULL;
            if (ps_focus_field == 1) { buf = ps_name1; cursor = &ps_cursor1; }
            if (ps_focus_field == 2) { buf = ps_name2; cursor = &ps_cursor2; }
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
            int mx = e.button.x, my = e.button.y;
            if (ps_point_in_rect(ps_input1, mx, my)) { ps_focus_field = 1; SDL_StartTextInput(); }
            else if (ps_point_in_rect(ps_input2, mx, my)) { ps_focus_field = 2; SDL_StartTextInput(); }
            else { ps_focus_field = 0; SDL_StopTextInput(); }

            if (ps_point_in_rect(ps_j1_btn, mx, my) || ps_point_in_rect(ps_j2_btn, mx, my)) {
                if (game->click) Mix_PlayChannel(-1, game->click, 0);
            }
            if (ps_point_in_rect(ps_score_btn, mx, my)) {
                if (game->click) Mix_PlayChannel(-1, game->click, 0);
                game->currentState = STATE_SCORES_INPUT;
                return;
            }
        }
    }
}

/* ═══════════════════════════════════════════════
   SELECTION JOUEURS – RENDU
═══════════════════════════════════════════════ */
void PlayerSelect_Affichage(Game *game, SDL_Renderer *renderer) {
    SDL_SetRenderDrawColor(renderer, 10,10,20,255);
    SDL_RenderClear(renderer);

    if (game->ps_bg.texture)
        SDL_RenderCopy(renderer, game->ps_bg.texture, NULL, &game->ps_bg.dest_rect);

    /* Overlay sombre pour lisibilite */
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0,0,0,90);
    SDL_Rect full = {0,0,WIDTH,HEIGHT};
    SDL_RenderFillRect(renderer, &full);

    TTF_Font *font = game->font;
    SDL_Color white = {255, 255, 255, 255};
    SDL_Color muted = {180, 180, 180, 255};

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 190, 60, 60, 70);
    SDL_RenderFillRect(renderer, &ps_p1_frame);
    SDL_SetRenderDrawColor(renderer, 200, 80, 80, 255);
    SDL_RenderDrawRect(renderer, &ps_p1_frame);

    SDL_SetRenderDrawColor(renderer, 60, 140, 220, 70);
    SDL_RenderFillRect(renderer, &ps_p2_frame);
    SDL_SetRenderDrawColor(renderer, 80, 170, 240, 255);
    SDL_RenderDrawRect(renderer, &ps_p2_frame);

    ps_draw_text(renderer, font, "PLAYER 1", white, ps_p1_frame.x + 12, ps_p1_frame.y + 12);
    ps_draw_text(renderer, font, "PLAYER 2", white, ps_p2_frame.x + 12, ps_p2_frame.y + 12);

    if (game->player1Tex) {
        SDL_Rect c1 = {ps_p1_frame.x + 20, ps_p1_frame.y + 50, ps_p1_frame.w - 40, ps_p1_frame.h - 70};
        SDL_RenderCopy(renderer, game->player1Tex, NULL, &c1);
    }
    if (game->player2Tex) {
        SDL_Rect c2 = {ps_p2_frame.x + 20, ps_p2_frame.y + 50, ps_p2_frame.w - 40, ps_p2_frame.h - 70};
        SDL_RenderCopy(renderer, game->player2Tex, NULL, &c2);
    }

    if (ps_hover_j1) {
        SDL_SetRenderDrawColor(renderer, 255, 220, 60, 120);
        SDL_RenderFillRect(renderer, &ps_j1_btn);
    }
    if (game->psJ1Tex) SDL_RenderCopy(renderer, game->psJ1Tex, NULL, &ps_j1_btn);
    else ps_draw_center_text(renderer, font, "J1", white, ps_j1_btn);

    if (ps_hover_j2) {
        SDL_SetRenderDrawColor(renderer, 255, 220, 60, 120);
        SDL_RenderFillRect(renderer, &ps_j2_btn);
    }
    if (game->psJ2Tex) SDL_RenderCopy(renderer, game->psJ2Tex, NULL, &ps_j2_btn);
    else ps_draw_center_text(renderer, font, "J2", white, ps_j2_btn);

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
    SDL_RenderFillRect(renderer, &ps_input1);
    SDL_RenderFillRect(renderer, &ps_input2);
    SDL_SetRenderDrawColor(renderer, ps_focus_field == 1 ? 255 : 170, ps_focus_field == 1 ? 220 : 170, 70, 255);
    SDL_RenderDrawRect(renderer, &ps_input1);
    SDL_SetRenderDrawColor(renderer, ps_focus_field == 2 ? 255 : 170, ps_focus_field == 2 ? 220 : 170, 70, 255);
    SDL_RenderDrawRect(renderer, &ps_input2);

    ps_draw_text(renderer, font, strlen(ps_name1) ? ps_name1 : "Player 1 name...", strlen(ps_name1) ? white : muted, ps_input1.x + 8, ps_input1.y + 14);
    ps_draw_text(renderer, font, strlen(ps_name2) ? ps_name2 : "Player 2 name...", strlen(ps_name2) ? white : muted, ps_input2.x + 8, ps_input2.y + 14);

    for (int i = 0; i < 3; i++) {
        SDL_Rect r = ps_tools[i];
        SDL_Texture *tex = NULL;
        if (i == 0) tex = ps_hover_tools[i] ? game->psKeyboardHoverTex : game->psKeyboardTex;
        if (i == 1) tex = ps_hover_tools[i] ? game->psManetteHoverTex : game->psManetteTex;
        if (i == 2) tex = ps_hover_tools[i] ? game->psSourisHoverTex : game->psSourisTex;
        if (ps_hover_tools[i]) {
            r.y -= 4;
        } else {
            SDL_SetRenderDrawColor(renderer, 30, 30, 30, 120);
            SDL_RenderFillRect(renderer, &r);
        }
        SDL_SetRenderDrawColor(renderer, ps_hover_tools[i] ? 255 : 230, ps_hover_tools[i] ? 220 : 230, 80, 255);
        SDL_RenderDrawRect(renderer, &r);
        if (tex) SDL_RenderCopy(renderer, tex, NULL, &r);
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

    SDL_Rect vol_rect = {WIDTH - 160, 20, 140, 42};
    char vol_text[32];
    snprintf(vol_text, sizeof(vol_text), "VOL %d", game->ps_bg.music_volume);
    SDL_SetRenderDrawColor(renderer, 40, 40, 40, 180);
    SDL_RenderFillRect(renderer, &vol_rect);
    SDL_SetRenderDrawColor(renderer, 220, 220, 220, 255);
    SDL_RenderDrawRect(renderer, &vol_rect);
    ps_draw_center_text(renderer, font, vol_text, white, vol_rect);

    SDL_Rect score_btn_draw = ps_score_btn;
    if (ps_hover_score) {
        SDL_SetRenderDrawColor(renderer, 255, 230, 60, 140);
        SDL_RenderFillRect(renderer, &score_btn_draw);
    }
    SDL_SetRenderDrawColor(renderer, 230, 230, 230, 255);
    SDL_RenderDrawRect(renderer, &score_btn_draw);
    if (game->psScoreBtnTex) {
        SDL_RenderCopy(renderer, game->psScoreBtnTex, NULL, &score_btn_draw);
    } else {
        ps_draw_center_text(renderer, font, "SCORES", white, score_btn_draw);
    }

    SDL_RenderPresent(renderer);
}

/* ═══════════════════════════════════════════════
   SELECTION JOUEURS – MISE A JOUR
═══════════════════════════════════════════════ */
void PlayerSelect_MiseAJour(Game *game) {
    (void)game;
    Uint32 now = SDL_GetTicks();
    if (now - ps_cursor_timer >= 500) {
        ps_cursor_on = !ps_cursor_on;
        ps_cursor_timer = now;
    }
    SDL_Delay(16);
}

void Game_LectureEntree(Game *game) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) { game->running = 0; return; }
        if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
            game->currentState = STATE_MENU;
            if (game->music) Mix_PlayMusic(game->music, -1);
            return;
        }
    }
}

void Game_Affichage(Game *game, SDL_Renderer *renderer) {
    SDL_SetRenderDrawColor(renderer, 15, 15, 25, 255);
    SDL_RenderClear(renderer);

    if (game->gameBgTex) SDL_RenderCopy(renderer, game->gameBgTex, NULL, NULL);
    else if (game->background) SDL_RenderCopy(renderer, game->background, NULL, NULL);

    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_Rect leftCard = {80, 140, 420, 500};
    SDL_Rect rightCard = {WIDTH - 500, 140, 420, 500};
    SDL_SetRenderDrawColor(renderer, 10, 10, 10, 140);
    SDL_RenderFillRect(renderer, &leftCard);
    SDL_RenderFillRect(renderer, &rightCard);
    SDL_SetRenderDrawColor(renderer, 230, 230, 230, 255);
    SDL_RenderDrawRect(renderer, &leftCard);
    SDL_RenderDrawRect(renderer, &rightCard);

    if (game->player1Tex) {
        SDL_Rect p1 = {leftCard.x + 20, leftCard.y + 20, leftCard.w - 40, leftCard.h - 90};
        SDL_RenderCopy(renderer, game->player1Tex, NULL, &p1);
    }
    if (game->player2Tex) {
        SDL_Rect p2 = {rightCard.x + 20, rightCard.y + 20, rightCard.w - 40, rightCard.h - 90};
        SDL_RenderCopy(renderer, game->player2Tex, NULL, &p2);
    }

    if (game->font) {
        SDL_Color white = {255, 255, 255, 255};
        ps_draw_center_text(renderer, game->font, game->player1_name, white,
                            (SDL_Rect){leftCard.x, leftCard.y + leftCard.h - 58, leftCard.w, 40});
        ps_draw_center_text(renderer, game->font, game->player2_name, white,
                            (SDL_Rect){rightCard.x, rightCard.y + rightCard.h - 58, rightCard.w, 40});
        ps_draw_center_text(renderer, game->font, "GAME STATE", white,
                            (SDL_Rect){(WIDTH - 300) / 2, 40, 300, 50});
    }

    SDL_RenderPresent(renderer);
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
        if (e.type == SDL_QUIT) { game->running = 0; return; }
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

    /* Holding movement keys increases movement acceleration. */
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
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

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

    SDL_RenderPresent(renderer);
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

/* ═══════════════════════════════════════════════════════════════════
   OPTIONS – Integrated into main project (merged from menu_option.c)
═══════════════════════════════════════════════════════════════════ */

/* Helper: check if point is in rect */
static int options_mouse_over(SDL_Rect r, int x, int y) {
    return (x >= r.x && x <= r.x + r.w && y >= r.y && y <= r.y + r.h);
}

/* ═══════════════════════════════════════════════
   OPTIONS – CHARGEMENT DES ASSETS
═══════════════════════════════════════════════ */
int Options_Charger(Game *game, SDL_Renderer *renderer) {
    if (game->optionsLoaded) return 1;

    game->optionsBg          = IMG_LoadTexture(renderer, GAME_ASSETS.backgrounds.options);
    game->volumePlusBtn      = IMG_LoadTexture(renderer, OPTION_BUTTON_VOLUME_PLUS);
    game->volumeMinusBtn     = IMG_LoadTexture(renderer, OPTION_BUTTON_VOLUME_MINUS);
    game->volumeMuteBtn      = IMG_LoadTexture(renderer, OPTION_BUTTON_VOLUME_MUTE);
    game->volumeMinusHoverBtn= load_texture_first(renderer, OPTION_BUTTON_VOLUME_MINUS_HOVER, OPTION_BUTTON_VOLUME_MINUS_HOVER, NULL);
    game->volumePlusHoverBtn = load_texture_first(renderer, OPTION_BUTTON_VOLUME_PLUS_HOVER_1, OPTION_BUTTON_VOLUME_PLUS_HOVER_2, NULL);
    game->volumeMuteHoverBtn = load_texture_first(renderer, OPTION_BUTTON_VOLUME_MUTE_HOVER_1, OPTION_BUTTON_VOLUME_MUTE_HOVER_2, NULL);
    game->fullscreenBtn      = IMG_LoadTexture(renderer, OPTION_BUTTON_FULLSCREEN);
    game->normalscreenBtn    = IMG_LoadTexture(renderer, OPTION_BUTTON_NORMALSCREEN);
    game->optionsClick       = Mix_LoadWAV(SOUND_OPTION_CLICK);

    /* Create OPTIONS title texture */
    TTF_Font *f = TTF_OpenFont(GAME_ASSETS.fonts.system_bold, 60);
    if (!f) f = TTF_OpenFont(GAME_ASSETS.fonts.system_regular, 60);
    if (f) {
        SDL_Color white = {255, 255, 255, 255};
        SDL_Surface *surf = TTF_RenderUTF8_Blended(f, "OPTIONS", white);
        if (surf) {
            game->optionsTitle = SDL_CreateTextureFromSurface(renderer, surf);
            game->optionsTitleRect = (SDL_Rect){(WIDTH - surf->w) / 2, 40, surf->w, 80};
            SDL_FreeSurface(surf);
        }
        TTF_CloseFont(f);
    }

    game->optionsFullscreen = 0;
    game->optionsLoaded = 1;
    printf("Options: assets charges\n");
    return 1;
}

/* ═══════════════════════════════════════════════
   OPTIONS – INPUT
═══════════════════════════════════════════════ */
void Options_LectureEntree(Game *game) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) { game->running = 0; return; }

        if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
            game->currentState = STATE_MENU;
            if (game->music) Mix_PlayMusic(game->music, -1);
            return;
        }

        if (e.type == SDL_MOUSEBUTTONDOWN) {
            int mx = e.button.x, my = e.button.y;

            /* Button rects (centered) */
            int centerX = WIDTH / 2;
            int bw = 80, bh = 80;
            SDL_Rect btnMinus = {centerX - 180, HEIGHT / 2, bw, bh};
            SDL_Rect btnMute  = {centerX - 40,  HEIGHT / 2, bw, bh};
            SDL_Rect btnPlus  = {centerX + 100, HEIGHT / 2, bw, bh};
            SDL_Rect btnFull  = {centerX - 40,  HEIGHT / 2 + 120, bw, bh};

            if (game->optionsClick)
                Mix_PlayChannel(-1, game->optionsClick, 0);

            /* Volume minus */
            if (options_mouse_over(btnMinus, mx, my)) {
                int vol = Mix_VolumeMusic(-1) - 10;
                if (vol < 0) vol = 0;
                Mix_VolumeMusic(vol);
                game->volume = vol;
            }
            /* Volume mute */
            if (options_mouse_over(btnMute, mx, my)) {
                Mix_VolumeMusic(0);
                game->volume = 0;
            }
            /* Volume plus */
            if (options_mouse_over(btnPlus, mx, my)) {
                int vol = Mix_VolumeMusic(-1) + 10;
                if (vol > 128) vol = 128;
                Mix_VolumeMusic(vol);
                game->volume = vol;
            }
            /* Fullscreen toggle */
            if (options_mouse_over(btnFull, mx, my)) {
                game->optionsFullscreen = !game->optionsFullscreen;
                game->fullscreen = game->optionsFullscreen;
                if (game->window) {
                    Uint32 flags = game->optionsFullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0;
                    SDL_SetWindowFullscreen(game->window, flags);
                }
            }
        }
    }
}

/* ═══════════════════════════════════════════════
   OPTIONS – RENDU
═══════════════════════════════════════════════ */
void Options_Affichage(Game *game, SDL_Renderer *renderer) {
    SDL_SetRenderDrawColor(renderer, 20, 20, 40, 255);
    SDL_RenderClear(renderer);

    /* Draw background */
    if (game->optionsBg)
        SDL_RenderCopy(renderer, game->optionsBg, NULL, NULL);

    /* Draw title */
    if (game->optionsTitle)
        SDL_RenderCopy(renderer, game->optionsTitle, NULL, &game->optionsTitleRect);

    /* Button positions (centered) */
    int centerX = WIDTH / 2;
    int bw = 80, bh = 80;
    SDL_Rect btnMinus = {centerX - 180, HEIGHT / 2, bw, bh};
    SDL_Rect btnMute  = {centerX - 40,  HEIGHT / 2, bw, bh};
    SDL_Rect btnPlus  = {centerX + 100, HEIGHT / 2, bw, bh};
    SDL_Rect btnFull  = {centerX - 40,  HEIGHT / 2 + 120, bw, bh};
    int mx, my;
    SDL_GetMouseState(&mx, &my);
    int hovMinus = options_mouse_over(btnMinus, mx, my);
    int hovMute  = options_mouse_over(btnMute, mx, my);
    int hovPlus  = options_mouse_over(btnPlus, mx, my);

    /* Draw volume buttons */
    if (hovMinus && game->volumeMinusHoverBtn)
        SDL_RenderCopy(renderer, game->volumeMinusHoverBtn, NULL, &btnMinus);
    else if (game->volumeMinusBtn)
        SDL_RenderCopy(renderer, game->volumeMinusBtn, NULL, &btnMinus);

    if (hovMute && game->volumeMuteHoverBtn)
        SDL_RenderCopy(renderer, game->volumeMuteHoverBtn, NULL, &btnMute);
    else if (game->volumeMuteBtn)
        SDL_RenderCopy(renderer, game->volumeMuteBtn, NULL, &btnMute);

    if (hovPlus && game->volumePlusHoverBtn)
        SDL_RenderCopy(renderer, game->volumePlusHoverBtn, NULL, &btnPlus);
    else if (game->volumePlusBtn)
        SDL_RenderCopy(renderer, game->volumePlusBtn, NULL, &btnPlus);

    /* Draw fullscreen toggle button */
    if (game->optionsFullscreen && game->normalscreenBtn)
        SDL_RenderCopy(renderer, game->normalscreenBtn, NULL, &btnFull);
    else if (!game->optionsFullscreen && game->fullscreenBtn)
        SDL_RenderCopy(renderer, game->fullscreenBtn, NULL, &btnFull);

    SDL_RenderPresent(renderer);
}

/* ═══════════════════════════════════════════════
   OPTIONS – MISE A JOUR
═══════════════════════════════════════════════ */
void Options_MiseAJour(Game *game) {
    (void)game;
    SDL_Delay(16);
}

/* ═══════════════════════════════════════════════════════════════════
   GAMES/GIFTS – Display gift-box image in bottom-right
═══════════════════════════════════════════════════════════════════ */

/* ═══════════════════════════════════════════════
   GAMES – CHARGEMENT DES ASSETS
═══════════════════════════════════════════════ */
int Games_Charger(Game *game, SDL_Renderer *renderer) {
    if (game->gamesLoaded) return 1;

    game->quizBg1   = IMG_LoadTexture(renderer, GAME_ASSETS.backgrounds.quiz_1);
    game->quizBg2   = IMG_LoadTexture(renderer, GAME_ASSETS.backgrounds.quiz_2);
    game->quizBtnA  = IMG_LoadTexture(renderer, QUIZ_BUTTON_A);
    game->quizBtnB  = IMG_LoadTexture(renderer, QUIZ_BUTTON_B);
    game->quizBtnC  = IMG_LoadTexture(renderer, QUIZ_BUTTON_C);
    game->quizMusic = Mix_LoadMUS(GAME_ASSETS.songs.quiz_music);
    game->quizBeep  = Mix_LoadWAV(SOUND_QUIZ_BEEP_1);
    game->quizBeep2 = Mix_LoadWAV(SOUND_QUIZ_BEEP_2);
    game->quizLaugh = Mix_LoadWAV(SOUND_QUIZ_LAUGH);
    game->quizFont  = TTF_OpenFont(GAME_ASSETS.fonts.quiz_primary, 26);
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

/* ═══════════════════════════════════════════════
   GAMES – INPUT
═══════════════════════════════════════════════ */
void Games_LectureEntree(Game *game) {
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        if (e.type == SDL_QUIT) { game->running = 0; return; }

        if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE) {
            game->currentState = STATE_MENU;
            return;
        }

        if (e.type == SDL_MOUSEMOTION && game->currentState == STATE_ENIGME_QUIZ) {
            int mx = e.motion.x, my = e.motion.y;
            quizHoverA = ps_point_in_rect(quizBtnARect, mx, my);
            quizHoverB = ps_point_in_rect(quizBtnBRect, mx, my);
            quizHoverC = ps_point_in_rect(quizBtnCRect, mx, my);
        }

        if (e.type == SDL_MOUSEBUTTONDOWN) {
            int mx = e.button.x, my = e.button.y;
            if (game->currentState == STATE_ENIGME_QUIZ) {
                if (ps_point_in_rect(quizBtnARect, mx, my)) {
                    quizSelected = 0;
                    if (game->quizBeep) Mix_PlayChannel(-1, game->quizBeep, 0);
                    if (game->quizLaugh) Mix_PlayChannel(-1, game->quizLaugh, 0);
                }
                if (ps_point_in_rect(quizBtnBRect, mx, my)) {
                    quizSelected = 1;
                    if (game->quizBeep2) Mix_PlayChannel(-1, game->quizBeep2, 0);
                }
                if (ps_point_in_rect(quizBtnCRect, mx, my)) {
                    quizSelected = 2;
                    if (game->quizBeep) Mix_PlayChannel(-1, game->quizBeep, 0);
                    if (game->quizLaugh) Mix_PlayChannel(-1, game->quizLaugh, 0);
                }
            } else {
                game->currentState = STATE_ENIGME_QUIZ;
            }
        }
    }
}

/* ═══════════════════════════════════════════════
   GAMES – RENDU
═══════════════════════════════════════════════ */
void Games_Affichage(Game *game, SDL_Renderer *renderer) {
    SDL_SetRenderDrawColor(renderer, 20, 20, 30, 255);
    SDL_RenderClear(renderer);

    if (game->currentState == STATE_ENIGME_QUIZ) {
        if (game->quizBg2) SDL_RenderCopy(renderer, game->quizBg2, NULL, NULL);
        else if (game->quizBg1) SDL_RenderCopy(renderer, game->quizBg1, NULL, NULL);

        if (game->quizFont) {
            SDL_Color black = {0, 0, 0, 255};
            SDL_Color white = {255, 255, 255, 255};
            ps_draw_center_text(renderer, game->quizFont,
                                "QUEL OBJET KEVIN MET-IL SUR LA POIGNEE ?",
                                black, (SDL_Rect){100, 70, WIDTH - 200, 70});
            ps_draw_center_text(renderer, game->quizFont,
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
            ps_draw_center_text(renderer, game->quizFont, msg, col, (SDL_Rect){120, 280, WIDTH - 240, 50});
        }
    } else {
        if (game->quizBg1) SDL_RenderCopy(renderer, game->quizBg1, NULL, NULL);
    }

    SDL_RenderPresent(renderer);
}

/* ═══════════════════════════════════════════════
   GAMES – MISE A JOUR
═══════════════════════════════════════════════ */
void Games_MiseAJour(Game *game) {
    (void)game;
    SDL_Delay(16);
}
