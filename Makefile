CC = gcc
TARGET = jeu

SRCS = main.c \
       core.c \
       menu.c \
       screens_menu.c \
       player_select_state.c \
       play_states.c

OBJS = $(SRCS:.c=.o)

CFLAGS = -Wall -Wextra -g $(shell sdl2-config --cflags)
LIBS = $(shell sdl2-config --libs) -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lm

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LIBS)

%.o: %.c game.h all_in_one.h allinone2.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

re: clean all

.PHONY: all clean re
