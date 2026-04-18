#ifndef ALL_IN_ONE_H
#define ALL_IN_ONE_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>

#include <math.h>
#include <stddef.h>
#include <string.h>

#define WIDTH 1280
#define HEIGHT 720

typedef enum {
    MAIN_STATE_MENU,
    MAIN_STATE_GAME,
    MAIN_STATE_SCORE,
    MAIN_STATE_HISTORY,
    MAIN_STATE_PLAYER
} GameState;

typedef enum {
    STATE_MENU,
    STATE_OPTIONS,
    STATE_SAVE,
    STATE_SAVE_CHOICE,
    STATE_PLAYER,
    STATE_PLAYER_CONFIG,
    STATE_SCORES_INPUT,
    STATE_SCORES_LIST,
    STATE_START_PLAY,
    STATE_ENIGME,
    STATE_ENIGME_QUIZ,
    STATE_HISTOIRE,
    STATE_GAME,
    STATE_QUIT,

    STATE_SCORES = STATE_SCORES_INPUT,
    STATE_PLAYER_SELECT = STATE_PLAYER_CONFIG,
    STATE_GAMES = STATE_ENIGME,
    STATE_CREDITS = STATE_HISTOIRE
} GameSubState;

typedef struct {
    Uint8 r, g, b, a;
} Color;

typedef struct {
    SDL_Texture *idleTexture;
    SDL_Texture *idleBackTexture;
    SDL_Texture *walkTexture;
    SDL_Texture *walkBackTexture;
    SDL_Texture *runTexture;
    SDL_Texture *jumpTexture;
    SDL_Texture *jumpBackTexture;
    SDL_Rect position;
    int up;
    int jumpPhase;
    int posinit;
    int posinitX;
    double jumpRelX;
    double jumpRelY;
    double jumpProgress;
    int jumpDir;
    int groundY;
    int jumpSpeed;
    int jumpHeight;
    int moveSpeed;
    int gravity;
    int facing;
    int moving;
    int movementState;
    int pendingJump;
    int frameIndex;
    Uint32 lastFrameTick;
} Personnage;

typedef struct {
    SDL_Texture *texture;
    SDL_Rect dest_rect;
    int width;
    int height;
    Mix_Music *music;
    int music_volume;
} Background;

typedef struct {
    SDL_Rect rect;
    int selected;
    SDL_Texture *normalTex;
    SDL_Texture *hoverTex;
} Button;

typedef struct {
    SDL_Rect rect;
    int selected;
    SDL_Texture *texture;
    SDL_Texture *textureCliq;
} SaveButton;

typedef enum {
    MENU_COMMAND_NONE,
    MENU_COMMAND_PLAY,
    MENU_COMMAND_OPTIONS,
    MENU_COMMAND_SAVE,
    MENU_COMMAND_SCORES,
    MENU_COMMAND_GIFT,
    MENU_COMMAND_QUIT
} MenuCommand;

typedef struct {
    SDL_Rect backgroundRect;
    SDL_Rect giftRect;
    int hoveredButton;
    int clickedButton;
    int hoverGift;
    int clickGift;
    int hoverSoundPending;
    MenuCommand pendingCommand;
} MenuUiState;

typedef enum {
    OPTIONS_COMMAND_NONE,
    OPTIONS_COMMAND_BACK,
    OPTIONS_COMMAND_VOLUME_DOWN,
    OPTIONS_COMMAND_VOLUME_MUTE,
    OPTIONS_COMMAND_VOLUME_UP,
    OPTIONS_COMMAND_FULLSCREEN
} OptionsCommand;

typedef struct {
    SDL_Rect backgroundRect;
    SDL_Rect minusRect;
    SDL_Rect muteRect;
    SDL_Rect plusRect;
    SDL_Rect fullscreenRect;
    int hoverMinus;
    int hoverMute;
    int hoverPlus;
    int hoverFullscreen;
    OptionsCommand pendingCommand;
} OptionsUiState;

typedef enum {
    GAME_MOVE_STOP = 0,
    GAME_MOVE_WALK,
    GAME_MOVE_RUN,
    GAME_MOVE_RUN_BACK,
    GAME_MOVE_WALK_BACK,
    GAME_MOVE_JUMP,
    GAME_MOVE_JUMP_BACK
} GameMovement;

typedef struct {
    SDL_Texture *walk_tex;
    SDL_Texture *walk_back_tex;
    SDL_Texture *jump_tex;
    SDL_Texture *jump_back_tex;
    SDL_Texture *stop_tex;
    SDL_Texture *stop_back_tex;
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
    int jump_rows;
    int jump_cols;
    int jump_frame_w;
    int jump_frame_h;
    int jump_back_rows;
    int jump_back_cols;
    int jump_back_frame_w;
    int jump_back_frame_h;
    int stop_rows;
    int stop_cols;
    int stop_frame_w;
    int stop_frame_h;
    int stop_back_rows;
    int stop_back_cols;
    int stop_back_frame_w;
    int stop_back_frame_h;
    int dance_rows;
    int dance_cols;
    int dance_frame_w;
    int dance_frame_h;
    int wave_rows;
    int wave_cols;
    int wave_frame_w;
    int wave_frame_h;
    int frame_index;
    int facing;
    int moving;
    int idle_state;
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

typedef enum {
    MOVEMENT_WALK = 0,
    MOVEMENT_RUN,
    MOVEMENT_DANCE,
    MOVEMENT_JUMP,
    MOVEMENT_COUNT
} MovementType;

typedef struct {
    const char *name;
    const char *sprite_sheet;
    int rows;
    int cols;
    int frame_duration_ms;
    float speed_x;
    float speed_y;
    int loops;
} AnimationMovement;

typedef struct {
    const char *menu_music;
    const char *save_music_primary;
    const char *save_music_fallback;
    const char *quiz_music;
} SongCatalog;

typedef struct {
    const char *logo;
    const char *player1;
    const char *player2;
} CharacterCatalog;

typedef struct {
    const char *menu;
    const char *save_primary;
    const char *save_fallback;
    const char *main;
    const char *leaderboard;
    const char *options;
    const char *gift;
    const char *quiz_1;
    const char *quiz_2;
    const char *start_heart;
} BackgroundCatalog;

typedef struct {
    const char *system_bold;
    const char *system_regular;
    const char *hello;
    const char *quiz_primary;
    const char *quiz_fallback;
} FontCatalog;

typedef struct {
    SongCatalog songs;
    CharacterCatalog characters;
    BackgroundCatalog backgrounds;
    FontCatalog fonts;
} AssetCatalog;

typedef struct {
    SDL_Texture *background;
    TTF_Font *font;
    SDL_Texture *titleTexture;
    SDL_Rect titleRect;
    SDL_Texture *logoTexture;
    SDL_Rect logoRect;
    SDL_Texture *trapTexture;
    SDL_Rect trapRect;
    Mix_Music *music;
    Mix_Chunk *Sound;
    Button buttons[5];

    SDL_Texture *msGreTex;
    SDL_Texture *msRedTex;
    SDL_Texture *leaderTex;
    Mix_Chunk *click;
    Button backBtn;
    SDL_Texture *scoreJ1Tex;
    SDL_Texture *scoreJ2Tex;
    SDL_Texture *startPlayer1LifeTex;
    SDL_Texture *startPlayer2LifeTex;
    SDL_Texture *startTextTex;
    SDL_Rect startTextRect;
    int startPlayLoaded;

    SDL_Rect searchBox;
    int inputActive;
    char inputText[256];

    SDL_Texture *saveBg;
    SDL_Surface *titleSurface;
    SDL_Texture *titleTexSave;
    SDL_Rect titleRectSave;
    Mix_Music *saveMusic;
    Mix_Chunk *saveSound;
    SaveButton saveButtons[4];
    int saveEtat;
    int clic_bouton;

    Background ps_bg;
    int ps_loaded;
    char player1_name[256];
    char player2_name[256];
    SDL_Texture *player1Tex;
    SDL_Texture *player2Tex;
    SDL_Texture *gameBgTex;
    SDL_Texture *psJ1Tex;
    SDL_Texture *psJ2Tex;
    SDL_Texture *psKeyboardTex;
    SDL_Texture *psKeyboardHoverTex;
    SDL_Texture *psManetteTex;
    SDL_Texture *psManetteHoverTex;
    SDL_Texture *psSourisTex;
    SDL_Texture *psSourisHoverTex;
    SDL_Texture *psScoreBtnTex;
    SDL_Texture *psMonoBtnTex;
    SDL_Texture *psMonoBtnHoverTex;
    SDL_Texture *psMultiBtnTex;
    SDL_Texture *psMultiBtnHoverTex;
    SDL_Texture *psNamePlayer1Tex;
    SDL_Texture *psNamePlayer2Tex;
    SDL_Texture *psHelpIconTex;
    SDL_Texture *psHelpIconHoverTex;
    SDL_Texture *psHelpButtonTex;
    int player_mode;

    SDL_Texture *optionsBg;
    SDL_Texture *optionsTitle;
    SDL_Rect optionsTitleRect;
    SDL_Texture *volumePlusBtn;
    SDL_Texture *volumeMinusBtn;
    SDL_Texture *volumeMuteBtn;
    SDL_Texture *volumePlusHoverBtn;
    SDL_Texture *volumeMinusHoverBtn;
    SDL_Texture *volumeMuteHoverBtn;
    SDL_Texture *fullscreenBtn;
    SDL_Texture *normalscreenBtn;
    Mix_Chunk *optionsClick;
    int optionsFullscreen;
    int optionsLoaded;

    SDL_Texture *menuGiftTex;
    SDL_Texture *gamesImg;
    SDL_Rect gamesRect;
    int gamesLoaded;
    SDL_Texture *quizBg1;
    SDL_Texture *quizBg2;
    SDL_Texture *quizBtnA;
    SDL_Texture *quizBtnB;
    SDL_Texture *quizBtnC;
    TTF_Font *quizFont;
    Mix_Music *quizMusic;
    Mix_Chunk *quizBeep;
    Mix_Chunk *quizBeep2;
    Mix_Chunk *quizLaugh;

    Personnage gameCharacter;
    int gameLoaded;
    Uint32 gameLastTick;

    SDL_Window *window;
    int volume;
    int fullscreen;

    GameState currentState;
    GameSubState currentSubState;
    int running;
} Game;

extern Background background;
#include "allinone2.h"

SDL_Texture *ChargerTexture(SDL_Renderer *renderer, const char *fichier);
Mix_Music *ChargerMusique(const char *fichier);
Mix_Chunk *ChargerSon(const char *fichier);

void box_message(SDL_Renderer *renderer, TTF_Font *font, const char *message, SDL_Rect box);

int Initialisation(Game *game, SDL_Window **window, SDL_Renderer **renderer);
void Liberation(Game *game, SDL_Window *window, SDL_Renderer *renderer);
void Game_ResetRuntime(Game *game);
GameState Game_MainStateFromSubState(GameSubState subState);
void Game_SetSubState(Game *game, GameSubState subState);

void GameLoop_ModuleInitialisationEtat(Game *game, SDL_Renderer *renderer);
void GameLoop_ModuleInput(Game *game);
void GameLoop_ModuleUpdate(Game *game);
void GameLoop_ModuleAffichage(Game *game, SDL_Renderer *renderer);

int init_background(SDL_Renderer *renderer, const char *image_path);
void draw_background(SDL_Renderer *renderer);
void update_background(void);
void cleanup_background(void);
int init_background_music(const char *music_path, int volume);
void play_background_music(void);
void stop_background_music(void);
void pause_background_music(void);
void resume_background_music(void);
void set_background_music_volume(int volume);
int get_background_music_volume(void);
int is_music_playing(void);
int is_music_paused(void);

void Menu_Preparer(Game *game, SDL_Renderer *renderer);
void Menu_LectureEntree(Game *game);
void Menu_MiseAJour(Game *game);
void Menu_Affichage(Game *game, SDL_Renderer *renderer);

void Leaderboard_LectureEntree(Game *game);
void Leaderboard_Affichage(Game *game, SDL_Renderer *renderer);

int Save_Charger(Game *game, SDL_Renderer *renderer);
void Save_LectureEntree(Game *game);
void Save_Affichage(Game *game, SDL_Renderer *renderer);
void Save_MiseAJour(Game *game);

int PlayerSelect_Charger(Game *game, SDL_Renderer *renderer);
void PlayerSelect_LectureEntree(Game *game);
void PlayerSelect_Affichage(Game *game, SDL_Renderer *renderer);
void PlayerSelect_MiseAJour(Game *game);

void Game_LectureEntree(Game *game);
void Game_Affichage(Game *game, SDL_Renderer *renderer);
int Game_Charger(Game *game, SDL_Renderer *renderer);
void Game_MiseAJour(Game *game);

int StartPlay_Charger(Game *game, SDL_Renderer *renderer);
void StartPlay_LectureEntree(Game *game);
void StartPlay_Affichage(Game *game, SDL_Renderer *renderer);
void StartPlay_MiseAJour(Game *game);
void StartPlay_Cleanup(void);

int Options_Charger(Game *game, SDL_Renderer *renderer);
void Options_LectureEntree(Game *game);
void Options_Affichage(Game *game, SDL_Renderer *renderer);
void Options_MiseAJour(Game *game);

int Games_Charger(Game *game, SDL_Renderer *renderer);
void Games_LectureEntree(Game *game);
void Games_Affichage(Game *game, SDL_Renderer *renderer);
void Games_MiseAJour(Game *game);

const AnimationMovement *animation_get_movement(MovementType type);
const AnimationMovement *animation_find_movement(const char *name);
size_t animation_movement_count(void);

int main(void);

#endif
