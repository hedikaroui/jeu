CC = gcc
CFLAGS = -Wall -Wextra -g `sdl2-config --cflags` -I.
LIBS = `sdl2-config --libs` -lSDL2_ttf -lSDL2_image

SRC = main.c source.c
OBJ = $(SRC:.c=.o)
EXEC = jeu

all: $(EXEC)

$(EXEC): $(OBJ)
	$(CC) -o $@ $^ $(LIBS)

%.o: %.c header.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f *.o $(EXEC)
