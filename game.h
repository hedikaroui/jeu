#ifndef HPR_H
#define HPR_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_mixer.h>
#include <string.h>
#include <math.h>

/* ══════════════════════════════════════
   CONFIGURATION
══════════════════════════════════════ */
#define WIDTH  1280
#define HEIGHT 720

/* ══════════════════════════════════════
   ÉTATS DU JEU
══════════════════════════════════════ */
typedef enum {
    STATE_MENU,          /* menu principal */
    STATE_OPTIONS,       /* sous-menu options */
    STATE_SAVE,          /* sauvegarde: OUI / NON */
    STATE_SAVE_CHOICE,   /* sauvegarde: CHARGER / NOUVELLE */
    STATE_PLAYER,        /* choix mono / multi */
    STATE_PLAYER_CONFIG, /* configuration joueurs */
    STATE_SCORES_INPUT,  /* saisie scores */
    STATE_SCORES_LIST,   /* top scores */
    STATE_START_PLAY,    /* launch screen before game */
    STATE_ENIGME,        /* enigme menu */
    STATE_ENIGME_QUIZ,   /* quiz actif */
    STATE_HISTOIRE,      /* histoire */
    STATE_GAME,          /* jeu */
    STATE_QUIT,          /* sortie */

    /* Compatibility aliases for existing modules */
    STATE_SCORES        = STATE_SCORES_INPUT,
    STATE_PLAYER_SELECT = STATE_PLAYER_CONFIG,
    STATE_GAMES         = STATE_ENIGME,
    STATE_CREDITS       = STATE_HISTOIRE
} GameState;

/* ══════════════════════════════════════
   COULEUR RGBA
══════════════════════════════════════ */
typedef struct {
    Uint8 r, g, b, a;
} Color;

/* ══════════════════════════════════════
   BACKGROUND
══════════════════════════════════════ */
typedef struct {
    SDL_Texture* texture;
    SDL_Rect     dest_rect;
    int          width;
    int          height;
    Mix_Music*   music;
    int          music_volume;
} Background;

/* ══════════════════════════════════════
   BOUTON MENU (normal + hover)
══════════════════════════════════════ */
typedef struct {
    SDL_Rect      rect;
    int           selected;
    SDL_Texture  *normalTex;
    SDL_Texture  *hoverTex;
} Button;

/* ══════════════════════════════════════
   BOUTON SAUVEGARDE (normal + cliqué)
══════════════════════════════════════ */
typedef struct {
    SDL_Rect     rect;
    int          selected;
    SDL_Texture *texture;
    SDL_Texture *textureCliq;
} SaveButton;

/* ══════════════════════════════════════════════════════════════
   STRUCTURE PRINCIPALE DU JEU (fusionné projets 1, 2 et 3)
══════════════════════════════════════════════════════════════ */
typedef struct {

    /* ── PROJET 1: Menu principal ── */
    SDL_Texture  *background;       /* background du menu */
    TTF_Font     *font;             /* police générale */
    SDL_Texture  *titleTexture;
    SDL_Rect      titleRect;
    SDL_Texture  *logoTexture;
    SDL_Rect      logoRect;
    SDL_Texture  *trapTexture;
    SDL_Rect      trapRect;
    Mix_Music    *music;            /* musique de fond */
    Mix_Chunk    *Sound;            /* son hover */
    Button        buttons[5];       /* Play / Options / Scores / Save / Quit */

    /* ── PROJET 2: Leaderboard / Scores ── */
    SDL_Texture  *msGreTex;         /* bouton scores surligné */
    SDL_Texture  *msRedTex;         /* bouton scores normal */
    SDL_Texture  *leaderTex;        /* fond leaderboard */
    Mix_Chunk    *click;            /* son clic */
    Button        backBtn;          /* bouton retour */
    SDL_Texture  *scoreJ1Tex;       /* button in score state */
    SDL_Texture  *scoreJ2Tex;       /* hover for score button */
    SDL_Texture  *startHeartTex;    /* heart icon for start screen */
    SDL_Texture  *startTextTex;     /* "the game start,go..." */
    SDL_Rect      startTextRect;
    int           startPlayLoaded;
    
    SDL_Rect      searchBox;        /* zone de recherche */
    int           inputActive;      /* 1 si input focus */
    char          inputText[256];   /* texte recherche */

    /* ── PROJET 3: Écran sauvegarde ── */
    SDL_Texture  *saveBg;
    SDL_Surface  *titleSurface;
    SDL_Texture  *titleTexSave;
    SDL_Rect      titleRectSave;
    Mix_Music    *saveMusic;
    Mix_Chunk    *saveSound;
    SaveButton    saveButtons[4];
    int           saveEtat;
    int           clic_bouton;

    /* ── PROJET 3: Écran sélection joueurs ── */
    Background    ps_bg;            /* background pixel art */
    int           ps_loaded;        /* 1 = assets déjà chargés */
    char          player1_name[256];
    char          player2_name[256];
    SDL_Texture  *player1Tex;
    SDL_Texture  *player2Tex;
    SDL_Texture  *gameBgTex;
    SDL_Texture  *psJ1Tex;
    SDL_Texture  *psJ2Tex;
    SDL_Texture  *psKeyboardTex;
    SDL_Texture  *psKeyboardHoverTex;
    SDL_Texture  *psManetteTex;
    SDL_Texture  *psManetteHoverTex;
    SDL_Texture  *psSourisTex;
    SDL_Texture  *psSourisHoverTex;
    SDL_Texture  *psScoreBtnTex;

    /* ── OPTIONS: Écran options ── */
    SDL_Texture  *optionsBg;        /* options.png */
    SDL_Texture  *optionsTitle;
    SDL_Rect      optionsTitleRect;
    SDL_Texture  *volumePlusBtn;
    SDL_Texture  *volumeMinusBtn;
    SDL_Texture  *volumeMuteBtn;
    SDL_Texture  *volumePlusHoverBtn;
    SDL_Texture  *volumeMinusHoverBtn;
    SDL_Texture  *volumeMuteHoverBtn;
    SDL_Texture  *fullscreenBtn;
    SDL_Texture  *normalscreenBtn;
    Mix_Chunk    *optionsClick;
    int           optionsFullscreen;
    int           optionsLoaded;

    /* ── GAMES: Écran games/gifts ── */
    SDL_Texture  *menuGiftTex;      /* gift-box shown on menu */
    SDL_Texture  *gamesImg;         /* gift-box image */
    SDL_Rect      gamesRect;        /* positioned bottom-right */
    int           gamesLoaded;
    SDL_Texture  *quizBg1;
    SDL_Texture  *quizBg2;
    SDL_Texture  *quizBtnA;
    SDL_Texture  *quizBtnB;
    SDL_Texture  *quizBtnC;
    TTF_Font     *quizFont;
    Mix_Music    *quizMusic;
    Mix_Chunk    *quizBeep;
    Mix_Chunk    *quizBeep2;
    Mix_Chunk    *quizLaugh;

    /* ── Shared runtime state ── */
    SDL_Window   *window;
    int           volume;          /* 0..128 */
    int           fullscreen;      /* 0 windowed, 1 fullscreen */

    /* ── Machine à états ── */
    GameState     currentState;
    int           running;

} Game;

/* ══════════════════════════════════════
   VARIABLES GLOBALES
══════════════════════════════════════ */
extern Background background;

/* ══════════════════════════════════════
   PROTOTYPES – Helpers Asset Loading
══════════════════════════════════════ */
SDL_Texture* ChargerTexture(SDL_Renderer *renderer, const char *fichier);
Mix_Music*   ChargerMusique(const char *fichier);
Mix_Chunk*   ChargerSon    (const char *fichier);

/* ══════════════════════════════════════
   PROTOTYPES – Lifecycle (Initialisation / Liberation)
══════════════════════════════════════ */
int  Initialisation(Game *game, SDL_Window **window, SDL_Renderer **renderer);
void Liberation    (Game *game, SDL_Window  *window, SDL_Renderer  *renderer);

/* ══════════════════════════════════════
   PROTOTYPES – background.c
══════════════════════════════════════ */
int  init_background(SDL_Renderer* renderer, const char* image_path);
void draw_background(SDL_Renderer* renderer);
void update_background(void);
void cleanup_background(void);
int  init_background_music(const char* music_path, int volume);
void play_background_music(void);
void stop_background_music(void);
void pause_background_music(void);
void resume_background_music(void);
void set_background_music_volume(int volume);
int  get_background_music_volume(void);
int  is_music_playing(void);
int  is_music_paused(void);

/* ══════════════════════════════════════
   PROTOTYPES – game_states.c (Menu principal)
══════════════════════════════════════ */
void Menu_LectureEntree(Game *game);
void Menu_Affichage    (Game *game, SDL_Renderer *renderer);

/* ══════════════════════════════════════
   PROTOTYPES – game_states.c (Leaderboard)
══════════════════════════════════════ */
void Leaderboard_LectureEntree(Game *game);
void Leaderboard_Affichage    (Game *game, SDL_Renderer *renderer);

/* ══════════════════════════════════════
   PROTOTYPES – game_states.c (Sauvegarde)
══════════════════════════════════════ */
int  Save_Charger       (Game *game, SDL_Renderer *renderer);
void Save_LectureEntree (Game *game);
void Save_Affichage     (Game *game, SDL_Renderer *renderer);
void Save_MiseAJour     (Game *game);

/* ══════════════════════════════════════
   PROTOTYPES – game_states.c (Sélection joueurs)
══════════════════════════════════════ */
int  PlayerSelect_Charger       (Game *game, SDL_Renderer *renderer);
void PlayerSelect_LectureEntree (Game *game);
void PlayerSelect_Affichage     (Game *game, SDL_Renderer *renderer);
void PlayerSelect_MiseAJour     (Game *game);

/* ══════════════════════════════════════
   PROTOTYPES – game_states.c (Écran jeu)
══════════════════════════════════════ */
void Game_LectureEntree (Game *game);
void Game_Affichage     (Game *game, SDL_Renderer *renderer);
int  StartPlay_Charger       (Game *game, SDL_Renderer *renderer);
void StartPlay_LectureEntree (Game *game);
void StartPlay_Affichage     (Game *game, SDL_Renderer *renderer);
void StartPlay_MiseAJour     (Game *game);

/* ══════════════════════════════════════
   PROTOTYPES – game_states.c (Options)
══════════════════════════════════════ */
int  Options_Charger       (Game *game, SDL_Renderer *renderer);
void Options_LectureEntree (Game *game);
void Options_Affichage     (Game *game, SDL_Renderer *renderer);
void Options_MiseAJour     (Game *game);

/* ══════════════════════════════════════
   PROTOTYPES – game_states.c (Games/Gifts)
══════════════════════════════════════ */
int  Games_Charger       (Game *game, SDL_Renderer *renderer);
void Games_LectureEntree (Game *game);
void Games_Affichage     (Game *game, SDL_Renderer *renderer);
void Games_MiseAJour     (Game *game);

#endif /* HPR_H */
