CC     = gcc
TARGET = puzzle_game
SRCS   = main.c source.c
CFLAGS = -Wall -O2
LIBS   = $(shell pkg-config --cflags --libs sdl2 SDL2_image SDL2_ttf SDL2_mixer 2>/dev/null) -lm

all: $(TARGET)

$(TARGET): $(SRCS) header.h
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET) $(LIBS)

clean:
	rm -f $(TARGET)
