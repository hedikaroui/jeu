# Makefile - integrated SDL2 game build

CC      = gcc
TARGET  = jeu

SRCS    = main.c \
          game_loop.c \
          game_core.c \
          player_select_state.c \
          start_play_state.c \
          games_state.c \
          menu.c \
          leaderboard.c \
          save.c \
          options.c \
          animation.c \
          songs.c \
          characters.c \
          backgrounds.c \
          buttons.c \
          fonts.c \
          assets_catalog.c

OBJS    = $(SRCS:.c=.o)

# ── Flags de compilation ──
CFLAGS  = -Wall -Wextra -g \
          $(shell sdl2-config --cflags)

# ── Bibliothèques SDL2 ──
LIBS    = $(shell sdl2-config --libs) \
          -lSDL2_image \
          -lSDL2_ttf \
          -lSDL2_mixer \
          -lm

# ───────────────────────────────────────────────
# Règle principale
# ───────────────────────────────────────────────
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LIBS)

# ───────────────────────────────────────────────
# Règles de compilation individuelles
# ───────────────────────────────────────────────
main.o: main.c game.h
	$(CC) $(CFLAGS) -c main.c -o main.o

game_core.o: game_core.c game.h
	$(CC) $(CFLAGS) -c game_core.c -o game_core.o

# ───────────────────────────────────────────────
# Nettoyage
# ───────────────────────────────────────────────
clean:
	rm -f $(OBJS) $(TARGET)

re: clean all

.PHONY: all clean re
