#include <stdlib.h>
#include <stdio.h>
#include <math.h>
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
    word16 x=1111; /* a rentrer un nombre entre 1000 et 9999 pour Von Neumann*/
    int i,k,l, tmp;
    struct mt19937p mt; // Pour Mersenne-Twister
    unsigned long truc;
    word32 output_AES; // sortie pour l'AES
    word16 output_Vneumann; // sortie pour pour Von Neumann
    word32 output_MT; // sortie pour Mersenne-Twister
	word8 output_rand_faible; // sortie pour les 4 bits de poids faible de rand
	word8 output_rand_fort;   // sortie pour les 4 bits de poids fort de rand
    u32 Kx[NK]; // pour l'AES
	u32 Kex[NB*NR]; // pour l'AES
	u32 Px[NB]; // pour l'AES 
	
	//Ouverture du fichier de resultats
	FILE* f = fopen ( "./resultats.out", "a+" /*"w+"*/ ); 
    
    srand(time(NULL));   //INIT RAND     
    tmp =rand();
     
    // initialisation de la graine pour Mersenne-Twister    
    sgenrand(time(NULL)+(tmp), &mt);
     
	// Initialisation de la clé et du plaintext pour l'AES // 45 est un paramètre qui doit changer à chaque initialisation
	init_rand(Kx, Px, NK, NB, tmp);
	// construction des sous-clés pour l'AES
	KeyExpansion(Kex,Kx);

	//fprintf( f, "Random = [");

	word32 array_newmann [NBVALEURS]; // sortie pour l'AES ( SORITE sur 16 bits)
	word32 array_mt [NBVALEURS]; // sortie pour pour Von Neumann ( SORITE sur 32 bits)
	word32 array_aes [NBVALEURS]; // sortie pour Mersenne-Twister ( SORITE sur 32 bits) 
	word32 array_rand_fort [NBVALEURS];	// sortie pour les 4 bits de poids faible de rand ( SORITE sur 8 bits)
	word32 array_rand_faible [NBVALEURS]; // sortie pour les 4 bits de poids fort de rand ( SORITE sur 8 bits)

	// Pour obtenir 1024 valeurs
	for ( int i = 0 ; i < NBVALEURS ; i++ )
	{
		// Generation d'un nombre aléatoire avec Von Neumann (sortie sur 16 bits)
		output_Vneumann = Von_Neumann(&x);
		array_newmann[i] = Von_Neumann(&x);

		// Generation d'un nombre aléatoire avec Mersenne-Twister (sortie sur 32 bits)
		output_MT = genrand(&mt); 
		array_mt[i] = genrand(&mt); 

		// Generation d'un nombre aléatoire avec AES (sortie sur 32 bits)
		output_AES = AES(Px, Kex);
		array_aes[i] = AES(Px, Kex);

		// Generation d'un nombre aléatoire avec rand 4 bits poids fort (sortie sur 32 bits)
		output_rand_fort = rand ( ) >> 27;
		array_rand_fort[i] = rand ( ) >> 27;

		// Generation d'un nombre aléatoire avec rand 4 bits poids faible (sortie sur 32 bits)
		output_rand_faible =  rand ( ) & 0x0F; 
		array_rand_faible[i] =  rand ( ) & 0x0F; 


		//printf("SORTIES RESPECTIVES: VN: %u, MT: %u, AES: %u, RANDFORT: %u, RANDFAIBLE: %u\n", output_Vneumann, output_MT, output_AES, output_rand_fort, output_rand_faible );
		//fprintf( f, "%u %u %u %u %u; ", output_Vneumann, output_MT, output_AES, output_rand_fort, output_rand_faible );
	}
	
	//Génération des tests de fréquence monobit
	/*double res_neumann = frequency ( array_newmann, NBVALEURS, 16 ); 
	double res_mt = frequency ( array_mt, NBVALEURS, 32 ); 
	double res_aes = frequency ( array_aes, NBVALEURS, 32 ); 
	double res_rand_fort = frequency ( array_rand_fort, NBVALEURS, 4 ); 
	double res_rand_faible = frequency ( array_rand_faible, NBVALEURS, 4 ); 
		
		
	printf( "Frequene monobit VNeumann: %lf\n", res_neumann );
	printf( "Frequene monobit MT: %lf\n", res_mt );
	printf( "Frequene monobit AES: %lf\n", res_aes );
	printf( "Frequene monobit rand fort: %lf\n", res_rand_fort );
	printf( "Frequene monobit rand faible: %lf\n", res_rand_faible );*/
	
	//Génération des tests des runs
	double res_neumann = runs ( array_newmann, NBVALEURS, 16 ); 
	double res_mt = runs ( array_mt, NBVALEURS, 32 ); 
	double res_aes = runs ( array_aes, NBVALEURS, 32 ); 
	double res_rand_fort = runs ( array_rand_fort, NBVALEURS, 4 ); 
	double res_rand_faible = runs ( array_rand_faible, NBVALEURS, 4 ); 
    
	printf( "Test des runs VNeumann: %lf\n", res_neumann );
	printf( "Test des runs MT: %lf\n", res_mt );
	printf( "Test des runs AES: %lf\n", res_aes );
	printf( "Test des runs rand fort: %lf\n", res_rand_fort );
	printf( "Test des runs rand faible: %lf\n", res_rand_faible );
	
	fprintf(f, "%lf ", res_neumann );
	fprintf(f, "%lf ", res_mt );
	fprintf(f, "%lf ", res_aes );
	fprintf(f, "%lf ", res_rand_fort );
	fprintf(f, "%lf\n", res_rand_faible );
		
	
	//fseek (f, -2, SEEK_END); 
	//fprintf( f, "]");
	
	//Fermeture du fichier
	fclose ( f );
	
	
    return 1;
}
