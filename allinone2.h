#ifndef ALLINONE2_H
#define ALLINONE2_H

#if defined(__GNUC__)
#define ASSET_UNUSED __attribute__((unused))
#else
#define ASSET_UNUSED
#endif

static const char *SONG_MENU_MUSIC ASSET_UNUSED = "songs/jingle.mp3";
static const char *SONG_SAVE_MUSIC_PRIMARY ASSET_UNUSED = "songs/save_music.mp3";
static const char *SONG_SAVE_MUSIC_FALLBACK ASSET_UNUSED = "songs/music.mp3";
static const char *SONG_QUIZ_MUSIC ASSET_UNUSED = "songs/quiz_music.mp3";

static const char *SOUND_HOVER ASSET_UNUSED = "songs/magic.wav";
static const char *SOUND_CLICK ASSET_UNUSED = "songs/click.mp3";
static const char *SOUND_SAVE_CLICK ASSET_UNUSED = "songs/cliq.wav";
static const char *SOUND_BACKGROUND_LOOP ASSET_UNUSED = "songs/background_sound.wav";
static const char *SOUND_OPTION_CLICK ASSET_UNUSED = "songs/sonbref.wav";
static const char *SOUND_QUIZ_BEEP_1 ASSET_UNUSED = "songs/quiz_beep.wav";
static const char *SOUND_QUIZ_BEEP_2 ASSET_UNUSED = "songs/quiz_beep2.wav";
static const char *SOUND_QUIZ_LAUGH ASSET_UNUSED = "songs/quiz_laugh.wav";

static const char *CHAR_LOGO ASSET_UNUSED = "characters/logo.png";
static const char *CHAR_PLAYER_1 ASSET_UNUSED = "characters/first_player.png";
static const char *CHAR_PLAYER_2 ASSET_UNUSED = "characters/second_player.png";

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

static const char *BG_MENU ASSET_UNUSED = "backgrounds/background.png";
static const char *BG_SAVE_PRIMARY ASSET_UNUSED = "backgrounds/BG.png";
static const char *BG_SAVE_FALLBACK ASSET_UNUSED = "backgrounds/LB.jpg";
static const char *BG_MAIN ASSET_UNUSED = "backgrounds/background_main.jpg";
static const char *BG_LEADERBOARD ASSET_UNUSED = "backgrounds/option_bg.png";
static const char *BG_OPTIONS ASSET_UNUSED = "backgrounds/options.png";
static const char *BG_GIFT ASSET_UNUSED = "backgrounds/gift-box_808639.png";
static const char *BG_QUIZ_1 ASSET_UNUSED = "backgrounds/quiz_bg1.png";
static const char *BG_QUIZ_2 ASSET_UNUSED = "backgrounds/quiz_bg2.png";
static const char *BG_START_HEART ASSET_UNUSED = "buttons/life_game_heart.png";

static const char *MENU_BUTTON_NORMAL[5] ASSET_UNUSED = {"buttons/j1.png", "buttons/o1.png", "buttons/q1.png", "buttons/m1.png", "buttons/h1.png"};
static const char *MENU_BUTTON_HOVER[5] ASSET_UNUSED = {"buttons/j2.png", "buttons/o2.png", "buttons/q2.png", "buttons/m2.png", "buttons/h2.png"};

static const char *SAVE_BUTTON_NORMAL[4] ASSET_UNUSED = {
    "buttons/yesnoncliq.png",
    "buttons/nonocliq.png",
    "buttons/sauvnocliq.png",
    "buttons/newnocliq.png"
};

static const char *SAVE_BUTTON_CLICKED[4] ASSET_UNUSED = {
    "buttons/yescliq.png",
    "buttons/nocliq.png",
    "buttons/sauvcliq.png",
    "buttons/newcliq.png"
};

static const char *SCORE_BUTTON_NORMAL ASSET_UNUSED = "buttons/j1.png";
static const char *SCORE_BUTTON_HOVER ASSET_UNUSED = "buttons/j2.png";
static const char *SCORE_BACK_NORMAL ASSET_UNUSED = "buttons/MSred.png";
static const char *SCORE_BACK_HOVER ASSET_UNUSED = "buttons/MSgre.png";
static const char *PLAYER_SELECT_SCORE_BUTTON ASSET_UNUSED = "buttons/s1.png";

static const char *OPTION_BUTTON_VOLUME_PLUS ASSET_UNUSED = "buttons/volumeplus.png";
static const char *OPTION_BUTTON_VOLUME_MINUS ASSET_UNUSED = "buttons/volumeminus.png";
static const char *OPTION_BUTTON_VOLUME_MUTE ASSET_UNUSED = "buttons/volumemute.png";
static const char *OPTION_BUTTON_VOLUME_PLUS_HOVER_1 ASSET_UNUSED = "buttons/sound_greend.png";
static const char *OPTION_BUTTON_VOLUME_PLUS_HOVER_2 ASSET_UNUSED = "buttons/sound_green.png";
static const char *OPTION_BUTTON_VOLUME_MINUS_HOVER ASSET_UNUSED = "buttons/sound_red.png";
static const char *OPTION_BUTTON_VOLUME_MUTE_HOVER_1 ASSET_UNUSED = "buttons/sound_yelllow.png";
static const char *OPTION_BUTTON_VOLUME_MUTE_HOVER_2 ASSET_UNUSED = "buttons/sound_yellow.png";
static const char *OPTION_BUTTON_FULLSCREEN ASSET_UNUSED = "buttons/fullscreen.png";
static const char *OPTION_BUTTON_NORMALSCREEN ASSET_UNUSED = "buttons/normalscreen.png";

static const char *QUIZ_BUTTON_A ASSET_UNUSED = "buttons/quiz_A.png";
static const char *QUIZ_BUTTON_B ASSET_UNUSED = "buttons/quiz_B.png";
static const char *QUIZ_BUTTON_C ASSET_UNUSED = "buttons/quiz_C.png";

static const char *FONT_SYSTEM_BOLD ASSET_UNUSED = "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf";
static const char *FONT_SYSTEM_REGULAR ASSET_UNUSED = "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf";
static const char *FONT_HELLO ASSET_UNUSED = "fonts/hello.ttf";
static const char *FONT_QUIZ_PRIMARY ASSET_UNUSED = "fonts/quiz_arial.ttf";
static const char *FONT_QUIZ_FALLBACK ASSET_UNUSED = "fonts/arial.ttf";

static const AssetCatalog GAME_ASSETS ASSET_UNUSED = {
    .songs = {
        .menu_music = "songs/jingle.mp3",
        .save_music_primary = "songs/save_music.mp3",
        .save_music_fallback = "songs/music.mp3",
        .quiz_music = "songs/quiz_music.mp3",
    },
    .characters = {
        .logo = "characters/logo.png",
        .player1 = "characters/first_player.png",
        .player2 = "characters/second_player.png",
    },
    .backgrounds = {
        .menu = "backgrounds/background.png",
        .save_primary = "backgrounds/BG.png",
        .save_fallback = "backgrounds/LB.jpg",
        .main = "backgrounds/background_main.jpg",
        .leaderboard = "backgrounds/option_bg.png",
        .options = "backgrounds/options.png",
        .gift = "backgrounds/gift-box_808639.png",
        .quiz_1 = "backgrounds/quiz_bg1.png",
        .quiz_2 = "backgrounds/quiz_bg2.png",
        .start_heart = "buttons/life_game_heart.png",
    },
    .fonts = {
        .system_bold = "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
        .system_regular = "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        .hello = "fonts/hello.ttf",
        .quiz_primary = "fonts/quiz_arial.ttf",
        .quiz_fallback = "fonts/arial.ttf",
    }
};

#undef ASSET_UNUSED

#endif
