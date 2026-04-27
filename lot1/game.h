#ifndef HPR_H
#define HPR_H

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
    STATE_HISTOIRE,
    STATE_GAME,
    STATE_QUIT,

    STATE_SCORES = STATE_SCORES_INPUT,
    STATE_PLAYER_SELECT = STATE_PLAYER_CONFIG,
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
    SDL_Texture *runBackTexture;
    SDL_Texture *jumpTexture;
    SDL_Texture *jumpBackTexture;
    SDL_Texture *damageTexture;
    SDL_Texture *layDownTexture;
    SDL_Texture *danceTexture;
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
    KEY_ACTION_WALK = 0,
    KEY_ACTION_JUMP,
    KEY_ACTION_RUN,
    KEY_ACTION_DOWN,
    KEY_ACTION_DANCE,
    KEY_ACTION_COUNT
} KeyAction;

#define GAME_CONTROL_KEYBOARD 0
#define GAME_CONTROL_CONTROLLER 1
#define GAME_CONTROL_MOUSE 2

typedef enum {
    GAME_MOVE_STOP = 0,
    GAME_MOVE_WALK,
    GAME_MOVE_RUN,
    GAME_MOVE_RUN_BACK,
    GAME_MOVE_WALK_BACK,
    GAME_MOVE_JUMP,
    GAME_MOVE_JUMP_BACK,
    GAME_MOVE_DANCE,
    GAME_MOVE_DAMAGE,
    GAME_MOVE_LAY_DOWN
} GameMovement;

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
} BackgroundCatalog;

typedef struct {
    const char *system_bold;
    const char *system_regular;
    const char *hello;
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
    SDL_Texture *startPlayer1SideViewTex;
    SDL_Texture *startPlayer2SideViewTex;
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
    int solo_selected_player; /* 0: first player, 1: second player */
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

    Personnage gameCharacter;
    Personnage gameCharacter2;
    int gameLoaded;
    Uint32 gameLastTick;

    SDL_Window *window;
    int volume;
    int fullscreen;

    GameState currentState;
    GameSubState currentSubState;
    int running;
} Game;


#if defined(__GNUC__)
#define ASSET_UNUSED __attribute__((unused))
#else
#define ASSET_UNUSED
#endif

static const char *SOUND_CLICK ASSET_UNUSED = "songs/click.mp3";
static const char *SOUND_BACKGROUND_LOOP ASSET_UNUSED = "songs/background_sound.wav";

static const char *CHAR_KEYBOARD_NORMAL_1 ASSET_UNUSED = "buttons/keyboard_transparent.png";
static const char *CHAR_KEYBOARD_NORMAL_2 ASSET_UNUSED = "buttons/keyboard_transparent.png";
static const char *CHAR_KEYBOARD_NORMAL_3 ASSET_UNUSED = "buttons/keyboard_white.png";
static const char *CHAR_KEYBOARD_HOVER ASSET_UNUSED = "buttons/keyboard_yellow.png";

static const char *CHAR_MANETTE_NORMAL_1 ASSET_UNUSED = "buttons/manette_transparent.png";
static const char *CHAR_MANETTE_NORMAL_2 ASSET_UNUSED = "buttons/manette_transparent.png";
static const char *CHAR_MANETTE_NORMAL_3 ASSET_UNUSED = "buttons/manette_transparent.png";
static const char *CHAR_MANETTE_HOVER ASSET_UNUSED = "buttons/manette_yellow.png";

static const char *CHAR_SOURIS_NORMAL_1 ASSET_UNUSED = "buttons/souris_transparent.png";
static const char *CHAR_SOURIS_NORMAL_2 ASSET_UNUSED = "buttons/souris_transparent.png";
static const char *CHAR_SOURIS_NORMAL_3 ASSET_UNUSED = "buttons/souris_transparent.png";
static const char *CHAR_SOURIS_HOVER ASSET_UNUSED = "buttons/souris_yellow.png";

static const char *SCORE_BUTTON_NORMAL ASSET_UNUSED = "buttons/j1.png";
static const char *SCORE_BUTTON_HOVER ASSET_UNUSED = "buttons/j2.png";
static const char *PLAYER_SELECT_SCORE_BUTTON ASSET_UNUSED = "buttons/s1.png";

static const char *FONT_SYSTEM_BOLD ASSET_UNUSED = "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf";
static const char *FONT_SYSTEM_REGULAR ASSET_UNUSED = "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf";
static const char *FONT_HELLO ASSET_UNUSED = "fonts/hello.ttf";

static const AssetCatalog GAME_ASSETS ASSET_UNUSED = {
    .songs = {
        .menu_music = "",
        .save_music_primary = "",
        .save_music_fallback = "",
    },
    .characters = {
        .logo = "",
        .player1 = "characters/first_player.png",
        .player2 = "characters/second_player.png",
    },
    .backgrounds = {
        .menu = "",
        .save_primary = "",
        .save_fallback = "",
        .main = "backgrounds/background_main.jpg",
        .leaderboard = "",
        .options = "",
        .gift = "",
    },
    .fonts = {
        .system_bold = "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
        .system_regular = "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        .hello = "fonts/hello.ttf",
    }
};

#undef ASSET_UNUSED


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

int StartPlay_Charger(Game *game, SDL_Renderer *renderer);
void StartPlay_LectureEntree(Game *game);
void StartPlay_Affichage(Game *game, SDL_Renderer *renderer);
void StartPlay_MiseAJour(Game *game);
void StartPlay_Cleanup(void);

int Options_Charger(Game *game, SDL_Renderer *renderer);
void Options_LectureEntree(Game *game);
void Options_Affichage(Game *game, SDL_Renderer *renderer);
void Options_MiseAJour(Game *game);

const AnimationMovement *animation_get_movement(MovementType type);
const AnimationMovement *animation_find_movement(const char *name);
size_t animation_movement_count(void);

#endif /* HPR_H */
