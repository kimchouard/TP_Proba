#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include "Generateurs.h"

#define NBVALEURS 1024

int rdtsc()
{
    __asm__ __volatile__("rdtsc");
	return 0;
}

double frequency ( word32 rand_array[], int array_size, size_t word_size )
{
	double frequency = 0;
	for ( int i = 0 ; i < array_size ; i++ ) 
	{
		word32 tmp = rand_array[i];
		for ( int j = 0 ; j < word_size ; j++ )
		{
			frequency += 2 * ( tmp & 0x01 ) - 1;
			tmp >>= 1;
		}
	}

	double sobs = abs(frequency) / sqrt(array_size*word_size);
	
	double p = erfc ( sobs / sqrt(2) );
	
	return p;
}

double runs ( word32 rand_array[], int array_size, size_t word_size )
{
	double n_size = array_size * word_size;
	double runs = 0;
	word32 tmp;
	
	for ( int i = 0 ; i < array_size ; i++ ) 
	{
		tmp = rand_array[i];
		for ( int j = 0 ; j < word_size ; j++ )
		{
			
			if ( ( tmp & 0x01 ) != 0x00 )
			{
				runs += 1;
			}
			tmp >>= 1;
		}
	}
	double pi = runs / n_size;
		
	if ( abs ( pi - 0.5 ) >= 2 / sqrt ( n_size ) )
	{
		return 0;
	}
	
	int runs_sum = 0;
	for ( int i = 0 ; i < array_size ; i++ ) 
	{
		word32 tmp = rand_array[i];
		
		for ( int j = 0 ; j < word_size -1 ; j++ )
		{
			
			if ( ( tmp & 0x01 ) != ( (tmp >> 1) & 0x01 ) )
			{
			
				runs_sum += 1;
			}
			tmp >>= 1;
		}
			if( i < array_size-1 )
			{
				word32 next_tmp = rand_array[i+1];
				if ( (next_tmp & 1) != (tmp & 1) )
				{
					runs_sum += 1;
				}
			}
	}
	runs_sum += 1;
	return erfc ( abs ( runs_sum - 2 * n_size * pi  * ( 1 - pi ) ) / ( 2 * sqrt ( 2 * n_size  ) * pi * ( 1 - pi ) ) );
	
}

int main()
{
	//Ouverture du fichier de resultats
	FILE* f = fopen ( "./resultats.txt", "w+" );

	//Résultats des 20 tests de frequence 
	double freq_newmann [20];
	double freq_mt [20];
	double freq_aes [20]; 
	double freq_rand_fort [20];	
	double freq_rand_faible [20]; 

	//Résultats des 20 tests des runs 
	double rand_newmann [20];
	double rand_mt [20];
	double rand_aes [20]; 
	double rand_rand_fort [20];	
	double rand_rand_faible [20]; 

	int j;
	for ( j = 0 ; j < 20 ; j++)
    {
	    int tmp;
	    word16 x = 1111;
	    struct mt19937p mt; // Pour Mersenne-Twister
	    u32 Kx[NK]; // pour l'AES
		u32 Kex[NB*NR]; // pour l'AES
		u32 Px[NB]; // pour l'AES 

		word32 array_newmann [NBVALEURS]; // sortie pour l'AES ( SORITE sur 16 bits)
		word32 array_mt [NBVALEURS]; // sortie pour pour Von Neumann ( SORITE sur 32 bits)
		word32 array_aes [NBVALEURS]; // sortie pour Mersenne-Twister ( SORITE sur 32 bits) 
		word32 array_rand_fort [NBVALEURS];	// sortie pour les 4 bits de poids faible de rand ( SORITE sur 8 bits)
		word32 array_rand_faible [NBVALEURS]; // sortie pour les 4 bits de poids fort de rand ( SORITE sur 8 bits)

    	srand(time(NULL));   //INIT RAND     
        tmp =rand();
        
	    x = rand()*8999+1000; /* a rentrer un nombre entre 1000 et 9999 pour Von Neumann*/

        // initialisation de la graine pour Mersenne-Twister    
        sgenrand(time(NULL)+(tmp), &mt);
         
    	// Initialisation de la clé et du plaintext pour l'AES // 45 est un paramètre qui doit changer à chaque initialisation
    	init_rand(Kx, Px, NK, NB, tmp);
    	// construction des sous-clés pour l'AES
    	KeyExpansion(Kex,Kx);
    
    	// Pour obtenir 1024 valeurs
    	for ( int i = 0 ; i < NBVALEURS ; i++ )
    	{
    		// Generation d'un nombre aléatoire avec Von Neumann (sortie sur 16 bits)
    		array_newmann[i] = Von_Neumann(&x);
    
    		// Generation d'un nombre aléatoire avec Mersenne-Twister (sortie sur 32 bits)
    		array_mt[i] = genrand(&mt); 
    
    		// Generation d'un nombre aléatoire avec AES (sortie sur 32 bits)
    		array_aes[i] = AES(Px, Kex);
    
    		// Generation d'un nombre aléatoire avec rand 4 bits poids fort (sortie sur 32 bits)
    		array_rand_fort[i] = rand ( ) >> 27;
    
    		// Generation d'un nombre aléatoire avec rand 4 bits poids faible (sortie sur 32 bits)
    		array_rand_faible[i] =  rand ( ) & 0x0F;
    	}
    	
    	//Génération des tests de fréquence monobit
    	freq_newmann[j] = frequency ( array_newmann, NBVALEURS, 16 ); 
    	freq_mt[j] = frequency ( array_mt, NBVALEURS, 32 ); 
    	freq_aes[j] = frequency ( array_aes, NBVALEURS, 32 ); 
    	freq_rand_fort[j] = frequency ( array_rand_fort, NBVALEURS, 4 ); 
    	freq_rand_faible[j] = frequency ( array_rand_faible, NBVALEURS, 4 ); 
    		
    		
    	printf( "Frequene monobit VNeumann: %lf\n", freq_newmann[j] );
    	printf( "Frequene monobit MT: %lf\n", freq_mt[j] );
    	printf( "Frequene monobit AES: %lf\n", freq_aes[j] );
    	printf( "Frequene monobit rand fort: %lf\n", freq_rand_fort[j] );
    	printf( "Frequene monobit rand faible: %lf\n", freq_rand_faible[j] );
    
    	//Génération des tests des runs
    	rand_newmann[j] = runs ( array_newmann, NBVALEURS, 16 ); 
    	rand_mt[j] = runs ( array_mt, NBVALEURS, 32 ); 
    	rand_aes[j] = runs ( array_aes, NBVALEURS, 32 ); 
    	rand_rand_fort[j] = runs ( array_rand_fort, NBVALEURS, 4 ); 
    	rand_rand_faible[j] = runs ( array_rand_faible, NBVALEURS, 4 ); 
        
    	printf( "Test des runs VNeumann: %lf\n", rand_newmann[j] );
    	printf( "Test des runs MT: %lf\n", rand_mt[j] );
    	printf( "Test des runs AES: %lf\n", rand_aes[j] );
    	printf( "Test des runs rand fort: %lf\n", rand_rand_fort[j] );
    	printf( "Test des runs rand faible: %lf\n", rand_rand_faible[j] );

    	//Permet de mettre à jour la graine du random !! ;)
    	sleep(1);
    }
    
    //Stockage des résultats dans le fichier
    fprintf(f, "Resultats des tests de fréquence\n" );
    fprintf(f, "VN, MT, AES, RANDFORT, RANDFAIBLE\n");
    for ( j = 0 ; j < 20 ; j++ )
    {
    	fprintf(f, "%lf ", freq_newmann[j] );
    	fprintf(f, "%lf ", freq_mt[j] );
    	fprintf(f, "%lf ", freq_aes[j] );
    	fprintf(f, "%lf ", freq_rand_fort[j] );
    	fprintf(f, "%lf\n", freq_rand_faible[j] );
    }

    fprintf(f, "\n\nResultats des tests des runs\n" );
    fprintf(f, "VN, MT, AES, RANDFORT, RANDFAIBLE\n");
    for ( j = 0 ; j < 20 ; j++ )
    {
    	fprintf(f, "%lf ", rand_newmann[j] );
    	fprintf(f, "%lf ", rand_mt[j] );
    	fprintf(f, "%lf ", rand_aes[j] );
    	fprintf(f, "%lf ", rand_rand_fort[j] );
    	fprintf(f, "%lf\n", rand_rand_faible[j] );
    }
    fprintf(f, "\n\n" );
		
	//Fermeture du fichier
	fclose ( f );
	
	
    return 1;
}
