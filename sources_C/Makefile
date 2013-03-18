OBJ = mt19937p.o Rijndael.o 
expo: main.c $(OBJ)
	gcc -Wall -std=c99 -pedantic -o essai -lm main.c $(OBJ)

# Ces 2 lignes définissent la méthode de création d'un .o
.SUFFIXES: .o

.c.o:; gcc -Wall -std=c99 -pedantic -c -o $@ -lm $<

mt19937p.o: mt19937p.h mt19937p.c

Rijndael.o: Rijndael.h Rijndael.c

clean:
	rm *.o expo
