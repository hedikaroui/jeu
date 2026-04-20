prog: source.o main.o
	gcc source.o main.o -o prog -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf -lm
	
main.o: main.c game.h
	gcc -c main.c

source.o: source.c game.h
	gcc -c source.c
