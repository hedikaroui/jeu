prog: source.o main.o
	gcc source.o main.o -o prog -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf
main.o: main.c minimap.h
	gcc -c main.c
source.o: source.c minimap.h
	gcc -c source.c
