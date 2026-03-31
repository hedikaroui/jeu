# Makefile for the standalone menuSDL application

CC      = gcc
TARGET  = menuSDL_app

SRCS    = main_option.c menu_option.c sdl_utils.c
OBJS    = $(SRCS:.c=.o)

CFLAGS  = -Wall -Wextra -g $(shell sdl2-config --cflags)
LIBS    = $(shell sdl2-config --libs) \
          -lSDL2_image -lSDL2_ttf -lSDL2_mixer

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $(TARGET) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
