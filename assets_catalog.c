#include "assets_catalog.h"

const AssetCatalog GAME_ASSETS = {
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
        .start_heart = "backgrounds/life_game_heart.png",
    },
    .fonts = {
        .system_bold = "/usr/share/fonts/truetype/dejavu/DejaVuSans-Bold.ttf",
        .system_regular = "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        .hello = "fonts/hello.ttf",
        .quiz_primary = "fonts/quiz_arial.ttf",
        .quiz_fallback = "fonts/arial.ttf",
    }
};
