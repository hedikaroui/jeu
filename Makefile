# Makefile - integrated SDL2 game build

CC      = gcc
TARGET  = jeu

SRCS    = main.c \
          game_states.c

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

game_states.o: game_states.c game.h
	$(CC) $(CFLAGS) -c game_states.c -o game_states.o

# ───────────────────────────────────────────────
# Nettoyage
# ───────────────────────────────────────────────
clean:
	rm -f $(OBJS) $(TARGET)

re: clean all

.PHONY: all clean re menu

menu:
	@echo "Building SDL menu example"
	$(MAKE) -C menuSDL
