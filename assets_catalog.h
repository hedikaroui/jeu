#ifndef ASSETS_CATALOG_H
#define ASSETS_CATALOG_H

extern const char *BG_MENU;
extern const char *BG_SAVE_PRIMARY;
extern const char *BG_SAVE_FALLBACK;
extern const char *BG_MAIN;
extern const char *BG_LEADERBOARD;
extern const char *BG_OPTIONS;
extern const char *BG_GIFT;
extern const char *BG_QUIZ_1;
extern const char *BG_QUIZ_2;
extern const char *BG_START_HEART;

extern const char *MENU_BUTTON_NORMAL[5];
extern const char *MENU_BUTTON_HOVER[5];
extern const char *SAVE_BUTTON_NORMAL[4];
extern const char *SAVE_BUTTON_CLICKED[4];
extern const char *SCORE_BUTTON_NORMAL;
extern const char *SCORE_BUTTON_HOVER;
extern const char *SCORE_BACK_NORMAL;
extern const char *SCORE_BACK_HOVER;
extern const char *PLAYER_SELECT_SCORE_BUTTON;
extern const char *OPTION_BUTTON_VOLUME_PLUS;
extern const char *OPTION_BUTTON_VOLUME_MINUS;
extern const char *OPTION_BUTTON_VOLUME_MUTE;
extern const char *OPTION_BUTTON_VOLUME_PLUS_HOVER_1;
extern const char *OPTION_BUTTON_VOLUME_PLUS_HOVER_2;
extern const char *OPTION_BUTTON_VOLUME_MINUS_HOVER;
extern const char *OPTION_BUTTON_VOLUME_MUTE_HOVER_1;
extern const char *OPTION_BUTTON_VOLUME_MUTE_HOVER_2;
extern const char *OPTION_BUTTON_FULLSCREEN;
extern const char *OPTION_BUTTON_NORMALSCREEN;
extern const char *QUIZ_BUTTON_A;
extern const char *QUIZ_BUTTON_B;
extern const char *QUIZ_BUTTON_C;

extern const char *CHAR_LOGO;
extern const char *CHAR_PLAYER_1;
extern const char *CHAR_PLAYER_2;
extern const char *CHAR_KEYBOARD_NORMAL_1;
extern const char *CHAR_KEYBOARD_NORMAL_2;
extern const char *CHAR_KEYBOARD_NORMAL_3;
extern const char *CHAR_KEYBOARD_HOVER;
extern const char *CHAR_MANETTE_NORMAL_1;
extern const char *CHAR_MANETTE_NORMAL_2;
extern const char *CHAR_MANETTE_NORMAL_3;
extern const char *CHAR_MANETTE_HOVER;
extern const char *CHAR_SOURIS_NORMAL_1;
extern const char *CHAR_SOURIS_NORMAL_2;
extern const char *CHAR_SOURIS_NORMAL_3;
extern const char *CHAR_SOURIS_HOVER;

extern const char *FONT_SYSTEM_BOLD;
extern const char *FONT_SYSTEM_REGULAR;
extern const char *FONT_HELLO;
extern const char *FONT_QUIZ_PRIMARY;
extern const char *FONT_QUIZ_FALLBACK;

extern const char *SONG_MENU_MUSIC;
extern const char *SONG_SAVE_MUSIC_PRIMARY;
extern const char *SONG_SAVE_MUSIC_FALLBACK;
extern const char *SONG_QUIZ_MUSIC;
extern const char *SOUND_HOVER;
extern const char *SOUND_CLICK;
extern const char *SOUND_SAVE_CLICK;
extern const char *SOUND_BACKGROUND_LOOP;
extern const char *SOUND_OPTION_CLICK;
extern const char *SOUND_QUIZ_BEEP_1;
extern const char *SOUND_QUIZ_BEEP_2;
extern const char *SOUND_QUIZ_LAUGH;

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

extern const AssetCatalog GAME_ASSETS;

#endif
