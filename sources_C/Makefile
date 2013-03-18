OBJ = mt19937p.o Rijndael.o 
#expo: main.c $(OBJ)
#	gcc -Wall -std=c99 -pedantic -o B3146 main.c $(OBJ) -lm
expo: file_mm1.c $(OBJ)
	gcc -Wall -std=c99 -pedantic -o B3146 file_mm1.c $(OBJ) -lm

# Ces 2 lignes définissent la méthode de création d'un .o
.SUFFIXES: .o

.c.o:; gcc -Wall -std=c99 -pedantic -c -o $@ -lm $<

mt19937p.o: mt19937p.h mt19937p.c

Rijndael.o: Rijndael.h Rijndael.c

clean:
	rm *.o B3XXX
