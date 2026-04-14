CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11
LIBS    = -lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer -lm
TARGET  = quiz_puzzle

SRCS    = main.c source.c puzzle_source.c

$(TARGET): $(SRCS) header.h
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) $(LIBS)

clean:
	rm -f $(TARGET)
