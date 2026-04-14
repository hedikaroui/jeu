
prog: bg.o minimap.o enemi.o main.o
	gcc bg.o minimap.o enemi.o main.o -o prog -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf -lm

main.o: main.c minimap.h bg.h enemi.h
	gcc -c main.c

bg.o: bg.c bg.h
	gcc -c bg.c

minimap.o: minimap.c minimap.h
	gcc -c minimap.c

enemi.o: enemi.c enemi.h
	gcc -c enemi.c

clean:
	rm -f *.o prog

.PHONY: clean exe

exe: prog
	@echo "Exécutable généré : ./prog"
