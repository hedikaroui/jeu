#ifndef ASSETS_CATALOG_H
#define ASSETS_CATALOG_H

#include "backgrounds.h"
#include "buttons.h"
#include "characters.h"
#include "fonts.h"
#include "songs.h"

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
