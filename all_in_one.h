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

typedef int GameState;
#define MAIN_STATE_MENU 0
#define MAIN_STATE_GAME 1
#define MAIN_STATE_SCORE 2
#define MAIN_STATE_HISTORY 3
#define MAIN_STATE_PLAYER 4

typedef int GameSubState;
#define STATE_MENU 0
#define STATE_OPTIONS 1
#define STATE_SAVE 2
#define STATE_SAVE_CHOICE 3
#define STATE_PLAYER 4
#define STATE_PLAYER_CONFIG 5
#define STATE_SCORES_INPUT 6
#define STATE_SCORES_LIST 7
#define STATE_START_PLAY 8
#define STATE_ENIGME 9
#define STATE_ENIGME_QUIZ 10
#define STATE_HISTOIRE 11
#define STATE_DISPLAY_CHOICE 12
#define STATE_GAME 13
#define STATE_QUIT 14
#define STATE_SKIN_SELECT 15
#define STATE_SCORES STATE_SCORES_INPUT
#define STATE_PLAYER_SELECT STATE_PLAYER_CONFIG
#define STATE_GAMES STATE_ENIGME
#define STATE_CREDITS STATE_HISTOIRE

typedef struct {
    Uint8 r, g, b, a;
} Color;

typedef struct {
    SDL_Texture *idleTexture;
    SDL_Texture *idleBackTexture;
    SDL_Texture *walkTexture;
    SDL_Texture *walkBackTexture;
    SDL_Texture *runTexture;
    SDL_Texture *runBackTexture;
    SDL_Texture *jumpTexture;
    SDL_Texture *jumpBackTexture;
    SDL_Texture *damageTexture;
    SDL_Texture *layDownTexture;
    SDL_Texture *tiredTexture;
    SDL_Texture *pickupTexture;
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
    int damageActive;
    Uint32 damageStartTick;
    Uint32 damageInvulnUntil;
    double energy;
    Uint32 tiredUntil;
    int pickupActive;
    Uint32 pickupStartTick;
    int pickupPendingSnowball;
    int hasSnowball;
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

typedef int MenuCommand;
#define MENU_COMMAND_NONE 0
#define MENU_COMMAND_PLAY 1
#define MENU_COMMAND_OPTIONS 2
#define MENU_COMMAND_SAVE 3
#define MENU_COMMAND_SCORES 4
#define MENU_COMMAND_GIFT 5
#define MENU_COMMAND_QUIT 6

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

typedef int OptionsCommand;
#define OPTIONS_COMMAND_NONE 0
#define OPTIONS_COMMAND_BACK 1
#define OPTIONS_COMMAND_VOLUME_DOWN 2
#define OPTIONS_COMMAND_VOLUME_MUTE 3
#define OPTIONS_COMMAND_VOLUME_UP 4
#define OPTIONS_COMMAND_FULLSCREEN 5

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

typedef int KeyAction;
#define KEY_ACTION_WALK 0
#define KEY_ACTION_JUMP 1
#define KEY_ACTION_RUN 2
#define KEY_ACTION_DOWN 3
#define KEY_ACTION_DANCE 4
#define KEY_ACTION_COUNT 5

#define GAME_CONTROL_KEYBOARD 0
#define GAME_CONTROL_CONTROLLER 1
#define GAME_CONTROL_MOUSE 2

typedef int GameMovement;
#define GAME_MOVE_STOP 0
#define GAME_MOVE_WALK 1
#define GAME_MOVE_RUN 2
#define GAME_MOVE_RUN_BACK 3
#define GAME_MOVE_WALK_BACK 4
#define GAME_MOVE_JUMP 5
#define GAME_MOVE_JUMP_BACK 6
#define GAME_MOVE_DAMAGE 7
#define GAME_MOVE_LAY_DOWN 8
#define GAME_MOVE_TIRED 9
#define GAME_MOVE_PICKUP 10

#define GAME_OBSTACLE_COUNT 2

typedef struct {
    SDL_Texture *texture;
    SDL_Rect position;
    int active;
    int collidingPlayer1;
    int collidingPlayer2;
    int direction;
    int minX;
    int maxX;
    int speed;
    int baseY;
    double amplitude;
    double frequency;
    double phase;
    int state;
    Uint32 stateTick;
} GameObstacle;

typedef struct {
    SDL_Texture *texture;
    SDL_Texture *standTexture;
    SDL_Texture *walkTexture;
    SDL_Texture *runTexture;
    SDL_Rect position;
    SDL_Rect sprite;
    int active;
    int frameWidth;
    int frameHeight;
    int frame;
    int nbCols;
    int nbRows;
    int direction;
    int animationState;
    Uint32 lastFrameTick;
    Uint32 playerTouchUntil;
} GameEnemy;

typedef struct {
    SDL_Texture *walk_tex;
    SDL_Texture *walk_back_tex;
    SDL_Texture *run_tex;
    SDL_Texture *run_back_tex;
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
    int run_rows;
    int run_cols;
    int run_frame_w;
    int run_frame_h;
    int run_back_rows;
    int run_back_cols;
    int run_back_frame_w;
    int run_back_frame_h;
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
    int running;
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

typedef int MovementType;
#define MOVEMENT_WALK 0
#define MOVEMENT_RUN 1
#define MOVEMENT_DANCE 2
#define MOVEMENT_JUMP 3
#define MOVEMENT_COUNT 4

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
    SDL_Texture *miniMapFrameTex;
    SDL_Texture *miniMapLockClosedTex;
    SDL_Texture *miniMapLockOpenTex;
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
    int solo_selected_player; /* 0: first player, 1: second player */
    int duo_display_mode;     /* 0: same screen, 1: vertical split, 2: horizontal split */
    int duo_background_mode;  /* 0: fixed background, 1: scrolling background */
    float minimap_zoom;       /* mini-map zoom factor (0.5 -> 2.0) */
    int playerControls[2];    /* GAME_CONTROL_* selected in player setup */
    SDL_Scancode keyBindings[2][KEY_ACTION_COUNT];

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
    SDL_Texture *puzzlePictureTex[2];
    SDL_Texture *puzzlePieceTex[2][3];
    TTF_Font *quizFont;
    Mix_Music *quizMusic;
    Mix_Chunk *quizBeep;
    Mix_Chunk *quizBeep2;
    Mix_Chunk *quizLaugh;

    Personnage gameCharacter;
    Personnage gameCharacter2;
    SDL_Texture *gameEnemyTex;
    SDL_Texture *gameEnemyStandTex;
    SDL_Texture *gameEnemyWalkTex;
    SDL_Texture *gameEnemyRunTex;
    SDL_Texture *gameSpiderTex;
    SDL_Texture *gameFallingTex;
    Mix_Chunk *gameObstacleHitSound;
    GameEnemy gameEnemy;
    GameObstacle gameObstacles[GAME_OBSTACLE_COUNT];
    int gameLoaded;
    Uint32 gameLastTick;
    int skinSelectLoaded;
    int selectedEnemySkinIndex;
    char selectedEnemySkinPath[256];
    GameSubState skinSelectReturnState;

    SDL_Window *window;
    int volume;
    int fullscreen;

    GameState currentState;
    GameSubState currentSubState;
    int running;
} Game;

#include "allinone2.h"

void box_message(SDL_Renderer *renderer, TTF_Font *font, const char *message, SDL_Rect box);

int Initialisation(Game *game, SDL_Window **window, SDL_Renderer **renderer);
void Liberation(Game *game, SDL_Window *window, SDL_Renderer *renderer);
void Game_ResetRuntime(Game *game);
GameState Game_MainStateFromSubState(GameSubState subState);
void Game_SetSubState(Game *game, GameSubState subState);

void GameLoop_ModuleInitialisationEtat(Game *game, SDL_Renderer *renderer);
void GameLoop_ModuleInput(Game *game, SDL_Renderer *renderer);
void GameLoop_ModuleUpdate(Game *game);
void GameLoop_ModuleAffichage(Game *game, SDL_Renderer *renderer);

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

void DisplayChoice_LectureEntree(Game *game);
void DisplayChoice_Affichage(Game *game, SDL_Renderer *renderer);
void DisplayChoice_MiseAJour(Game *game);

int StartPlay_Charger(Game *game, SDL_Renderer *renderer);
void StartPlay_LectureEntree(Game *game);
void StartPlay_Affichage(Game *game, SDL_Renderer *renderer);
void StartPlay_MiseAJour(Game *game);
void StartPlay_Cleanup(void);

int Options_Charger(Game *game, SDL_Renderer *renderer);
void Options_LectureEntree(Game *game);
void Options_Affichage(Game *game, SDL_Renderer *renderer);
void Options_MiseAJour(Game *game);

int SkinSelect_Charger(Game *game, SDL_Renderer *renderer);
void SkinSelect_LectureEntree(Game *game);
void SkinSelect_Affichage(Game *game, SDL_Renderer *renderer);
void SkinSelect_MiseAJour(Game *game);
void SkinSelect_Cleanup(void);

int Games_Charger(Game *game, SDL_Renderer *renderer);
void Games_LectureEntree(Game *game);
void Games_Affichage(Game *game, SDL_Renderer *renderer);
void Games_MiseAJour(Game *game);

const AnimationMovement *animation_get_movement(int type);
const AnimationMovement *animation_find_movement(const char *name);
size_t animation_movement_count(void);

#endif
