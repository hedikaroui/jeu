CC = gcc
CFLAGS = -Wall -Wextra -g $(shell sdl2-config --cflags)
LIBS = $(shell sdl2-config --libs) -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lm

TARGET = game
SRCS = main.c game.c player.c enemy.c puzzle.c quiz.c background.c minimap.c button.c
OBJS = $(SRCS:.c=.o)
HEADER = game.h

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LIBS)

%.o: %.c $(HEADER)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

re: clean all

.PHONY: all clean re
