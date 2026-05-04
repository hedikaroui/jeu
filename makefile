prog:fonctions.o main.o
	gcc fonctions.o main.o -o prog -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf
main.o:main.c
	gcc -c main.c
fonctions.o:fonctions.c
	gcc -c fonctions.c
	
