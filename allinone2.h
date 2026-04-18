#ifndef ALLINONE2_H
#define ALLINONE2_H

static const char *SONG_MENU_MUSIC = "songs/jingle.mp3";
static const char *SONG_SAVE_MUSIC_PRIMARY = "songs/save_music.mp3";
static const char *SONG_SAVE_MUSIC_FALLBACK = "songs/music.mp3";
static const char *SONG_QUIZ_MUSIC = "songs/quiz_music.mp3";

static const char *SOUND_HOVER = "songs/magic.wav";
static const char *SOUND_CLICK = "songs/click.mp3";
static const char *SOUND_SAVE_CLICK = "songs/cliq.wav";
static const char *SOUND_BACKGROUND_LOOP = "songs/background_sound.wav";
static const char *SOUND_OPTION_CLICK = "songs/sonbref.wav";
static const char *SOUND_QUIZ_BEEP_1 = "songs/quiz_beep.wav";
static const char *SOUND_QUIZ_BEEP_2 = "songs/quiz_beep2.wav";
static const char *SOUND_QUIZ_LAUGH = "songs/quiz_laugh.wav";

static const char *CHAR_LOGO = "characters/logo.png";
static const char *CHAR_PLAYER_1 = "characters/first_player.png";
static const char *CHAR_PLAYER_2 = "characters/second_player.png";

static const char *CHAR_KEYBOARD_NORMAL_1 = "buttons/keyboard_transparent.png";
static const char *CHAR_KEYBOARD_NORMAL_2 = "buttons/keyboard_transparent.png";
static const char *CHAR_KEYBOARD_NORMAL_3 = "buttons/keyboard_white.png";
static const char *CHAR_KEYBOARD_HOVER = "buttons/keyboard_yellow.png";

static const char *CHAR_MANETTE_NORMAL_1 = "buttons/manette_transparent.png";
static const char *CHAR_MANETTE_NORMAL_2 = "buttons/manette_transparent.png";
static const char *CHAR_MANETTE_NORMAL_3 = "buttons/manette_transparent.png";
static const char *CHAR_MANETTE_HOVER = "buttons/manette_yellow.png";

static const char *CHAR_SOURIS_NORMAL_1 = "buttons/souris_transparent.png";
static const char *CHAR_SOURIS_NORMAL_2 = "buttons/souris_transparent.png";
static const char *CHAR_SOURIS_NORMAL_3 = "buttons/souris_transparent.png";
static const char *CHAR_SOURIS_HOVER = "buttons/souris_yellow.png";

static const char *BG_MENU = "backgrounds/background.png";
static const char *BG_SAVE_PRIMARY = "backgrounds/BG.png";
static const char *BG_SAVE_FALLBACK = "backgrounds/LB.jpg";
static const char *BG_MAIN = "backgrounds/background_main.jpg";
static const char *BG_LEADERBOARD = "backgrounds/option_bg.png";
static const char *BG_OPTIONS = "backgrounds/options.png";
static const char *BG_GIFT = "backgrounds/gift-box_808639.png";
static const char *BG_QUIZ_1 = "backgrounds/quiz_bg1.png";
static const char *BG_QUIZ_2 = "backgrounds/quiz_bg2.png";
static const char *BG_START_HEART = "buttons/life_game_heart.png";

static const char *MENU_BUTTON_NORMAL[5] = {"buttons/j1.png", "buttons/o1.png", "buttons/q1.png", "buttons/m1.png", "buttons/h1.png"};
static const char *MENU_BUTTON_HOVER[5] = {"buttons/j2.png", "buttons/o2.png", "buttons/q2.png", "buttons/m2.png", "buttons/h2.png"};

static const char *SAVE_BUTTON_NORMAL[4] = {
    "buttons/yesnoncliq.png",
    "buttons/nonocliq.png",
    "buttons/sauvnocliq.png",
    "buttons/newnocliq.png"
};

static const char *SAVE_BUTTON_CLICKED[4] = {
    "buttons/yescliq.png",
    "buttons/nocliq.png",
    "buttons/sauvcliq.png",
    "buttons/newcliq.png"
};

static const char *SCORE_BUTTON_NORMAL = "buttons/j1.png";
static const char *SCORE_BUTTON_HOVER = "buttons/j2.png";
static const char *SCORE_BACK_NORMAL = "buttons/MSred.png";
static const char *SCORE_BACK_HOVER = "buttons/MSgre.png";
static const char *PLAYER_SELECT_SCORE_BUTTON = "buttons/s1.png";

static const char *OPTION_BUTTON_VOLUME_PLUS = "buttons/volumeplus.png";
static const char *OPTION_BUTTON_VOLUME_MINUS = "buttons/volumeminus.png";
static const char *OPTION_BUTTON_VOLUME_MUTE = "buttons/volumemute.png";
static const char *OPTION_BUTTON_VOLUME_PLUS_HOVER_1 = "buttons/sound_greend.png";
static const char *OPTION_BUTTON_VOLUME_PLUS_HOVER_2 = "buttons/sound_green.png";
static const char *OPTION_BUTTON_VOLUME_MINUS_HOVER = "buttons/sound_red.png";
static const char *OPTION_BUTTON_VOLUME_MUTE_HOVER_1 = "buttons/sound_yelllow.png";
static const char *OPTION_BUTTON_VOLUME_MUTE_HOVER_2 = "buttons/sound_yellow.png";
static const char *OPTION_BUTTON_FULLSCREEN = "buttons/fullscreen.png";
static const char *OPTION_BUTTON_NORMALSCREEN = "buttons/normalscreen.png";

static const char *QUIZ_BUTTON_A = "buttons/quiz_A.png";
static const char *QUIZ_BUTTON_B = "buttons/quiz_B.png";
static const char *QUIZ_BUTTON_C = "buttons/quiz_C.png";

static const char *FONT_SYSTEM_BOLD = "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf";
static const char *FONT_SYSTEM_REGULAR = "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf";
static const char *FONT_HELLO = "fonts/hello.ttf";
static const char *FONT_QUIZ_PRIMARY = "fonts/quiz_arial.ttf";
static const char *FONT_QUIZ_FALLBACK = "fonts/arial.ttf";

static const AssetCatalog GAME_ASSETS = {
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

#endif
