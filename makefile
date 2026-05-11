prog:fonctions.o main.o
	gcc fonctions.o main.o -o prog -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf
main.o:main.c header.h
	gcc -c main.c
fonctions.o:fonctions.c header.h
	gcc -c fonctions.c
	
