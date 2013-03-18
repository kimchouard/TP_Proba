#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <limits.h>
#include <unistd.h>
#include "Generateurs.h"

#define NBVALEURS 1024

#define ARRAY_MAX_SIZE 1000


//------------------------------------------------------------
//				Test des fréquences
//------------------------------------------------------------

double frequency ( word32 rand_array[], int array_size, size_t word_size )
{
	int frequency = 0;
	for ( int i = 0 ; i < array_size ; i++ ) 
	{
		word32 tmp = rand_array[i];
		for ( int j = 0 ; j < word_size ; j++ )
		{
			frequency += 2 * ( tmp & 0x01 ) - 1;
			tmp >>= 1;
		}
	}

	double sobs = ( (double) abs((double) frequency) ) / sqrt(array_size*word_size);
	
	double p = erfc ( sobs / sqrt(2) );
	
	return p;
}


//------------------------------------------------------------
//					Test des runs
//------------------------------------------------------------

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


//------------------------------------------------------------
//						Aléa
//------------------------------------------------------------

double Alea()
{
    u32 Kx[NK]; // pour l'AES
	u32 Kex[NB*NR]; // pour l'AES
	u32 Px[NB]; // pour l'AES 
    srand(time(NULL));   //INIT RAND     
    int tmp =rand();

	// Initialisation de la clé et du plaintext pour l'AES // 45 est un paramètre qui doit changer à chaque initialisation
    init_rand(Kx, Px, NK, NB, tmp);
    // construction des sous-clés pour l'AES
	//KeyExpansion(Kex,Kx);

	// Generation d'un nombre aléatoire avec AES (sortie sur 32 bits)
    word32 result = AES(Px, Kex);

	return (result/(double) UINT_MAX);
}


//------------------------------------------------------------
//						Exponentielle
//------------------------------------------------------------

double Exponentielle (double lambda)
{
	return -((double) log(1 - Alea())/log(10))/lambda;
}


//------------------------------------------------------------
//						FileMM1
//------------------------------------------------------------

//Structure pour stocker la file d'attente
struct file_attente {
	double *arr;
	int nb_arr;
	double *dep;
	int nb_dep;
};
typedef struct file_attente file_attente;

file_attente FileMM1 ( double lambda, double mu, double D, FILE *f) {
	//Initialisation pour la file d'attente à renvoyer
	file_attente res;
  	res.arr = (double*) calloc (ARRAY_MAX_SIZE, sizeof(double));
  	res.nb_arr = 0;
  	res.dep = (double*) calloc (ARRAY_MAX_SIZE, sizeof(double));
  	res.nb_dep = 0;

  	//----------------------------------------------------------------
  	//				Boucle qui fait arriver tous les clients

	//Variable pour stocker le temps actuel
	double dTempsArr = 0;

	//En-tete pour le fichier
	fprintf(f, "Arrivee\n");

  	while (dTempsArr <= D)
  	{
  		//On incrémente le temps d'arrivée
  		dTempsArr += Exponentielle(lambda);
		
  		//On fait arriver un nouveau client (on l'écrit aussi dans un fichier)
  		res.arr[res.nb_arr++] = dTempsArr;
  		fprintf(f, "%f, ", dTempsArr);
  	}
  	//On annule le dernier client arrivé car on à dépassé le temps impartit.
  	res.nb_arr--;
	fprintf(f, "\n\n");

  	//----------------------------------------------------------------
  	//				Boucle qui fait partir tous les clients

	//Le premier client part au moment ou il arrive + le temps de traitement
	res.dep[res.nb_dep] = res.arr[res.nb_dep] + Exponentielle(mu);
	res.nb_dep++;

	//Variable utiles
	double t_arr_suiv;
	double t_dep;
	double dTempsDep = res.dep[res.nb_dep-1];
	double Rep;

	//En-tete pour le fichier
	fprintf(f, "Depart\n");

  	while (dTempsDep <= D)
  	{
  		//Temps d'arrivée du client suivant
  		t_arr_suiv = res.arr[res.nb_dep];

  		//Temps de départ du client actuel
  		t_dep = res.dep[res.nb_dep-1];

  		//Temps de traitement du serveur
  		Rep = Exponentielle(mu);

  		//Si le client actuel n'est pas partit quand le suivant arrive
  		if (t_dep > t_arr_suiv)
  		{
  			//Le temps de départ du nouveau est le temps du précédent + temps de traitement
  			res.dep[res.nb_dep] = t_dep + Rep;
  		}
  		//Si le client actuel est partit
  		else 
  		{
  			//Le temps de départ du nouveau est son temps d'arrivé + temps de traitement
  			res.dep[res.nb_dep] = t_arr_suiv + Rep;
  		}
		
		dTempsDep = res.dep[res.nb_dep++];
  		fprintf(f, "%f, ", dTempsDep);
		
		//printf("Nouveau départ : %f, temps de rep du server: %f\n", res.dep[res.nb_dep], Rep);
  	}
  	//On annule le dernier client partit, car on à dépassé le temps impartit.
  	res.nb_dep--;
	fprintf(f, "\n\n");
	
	return res;
}


//------------------------------------------------------------
//					Evolution nombre client
//------------------------------------------------------------

//Structure permettant de stocker les évolutions
struct evolution
{
	double *temps;
	unsigned int *nombre;
};
typedef struct evolution evolution;

evolution evol_client(file_attente file, FILE *f)
{
	//Initialisation pour l'évolution
	evolution evo;
  	evo.temps = (double*) calloc (ARRAY_MAX_SIZE, sizeof(double));
  	evo.nombre = (unsigned int *) calloc (ARRAY_MAX_SIZE, sizeof(double));

	//En-tete pour le fichier
	fprintf(f, "Evolution des clients\n");
	
	//Parcours des 2 tableau simultanément
	int iArr = 0;
	int iDep = 0;
	int nbClient = 0;
	double dDep, dArr;

	//Tant que l'on est pas arrivé à la fin des 2 tableaux
  	while ( (iArr < file.nb_arr) || (iDep < file.nb_dep) )
  	{
  		//Récupération du temps d'arrivé et de départ à l'index actuel
		dArr = file.arr[iArr];
		dDep = file.dep[iDep];

		//Ajout d'une l'arrivée
		if ((dArr < dDep) && (iArr < file.nb_arr))
		{
			evo.temps[iArr] = dArr;
			evo.nombre[iArr] = ++nbClient;
			iArr++;

			//Log de l'arrivée dans le fichier
			fprintf(f, "%f %i\n", dArr, nbClient);
		}
		//Ajout du départ (si on est pas à la fin du tableau !)
		else if (iDep < file.nb_dep)
		{
			evo.temps[iDep] = dDep;
			evo.nombre[iDep] = --nbClient;
			iDep++;

			//Log du départ dans le fichier
			fprintf(f, "%f %i\n", dDep, nbClient);
		}
	}

	fprintf(f, "\n\n");
	
	return evo;
}


//------------------------------------------------------------
//					Temps passé dans le système
//------------------------------------------------------------

//Structure permettant de stocker les temps d'attente des clients
struct attente
{
	double *temps;
	double moyenne;
	unsigned int nombre;
};
typedef struct attente attente;

attente temps_attente(file_attente file, FILE *f)
{
	//Initialisation pour les temps d'attente
	attente att;
  	att.temps = (double*) calloc (ARRAY_MAX_SIZE, sizeof(double));
  	att.moyenne = 0;
  	att.nombre = 0;

	//En-tete pour le fichier
	fprintf(f, "Temps d'attente des clients\n");
	int max = (file.nb_arr < file.nb_dep) ? file.nb_dep : file.nb_arr;
	
	//Parcours des 2 tableau simultanément
	int i;

	//Tant que l'on est pas arrivé à la fin des 2 tableaux
  	for ( i = 0 ; i < max ; i++ )
  	{
		//Calcul du temps total

		//0 si le client n'est pas partit
		if (i > file.nb_dep)
		{
			att.temps[i] = 0;
		}
		//Si il est partit
		else
		{
			att.temps[i] = file.dep[i] - file.arr[i];

			//On ajoute le temps à la moyenne, et on incrément le nombre de gens partis
			att.moyenne += att.temps[i];
			att.nombre++;
		}

		//Log du temps total dans le fichier
		fprintf(f, "%i %f\n", i, att.temps[i]);
	}

	fprintf(f, "\n\n");

	//Calcul de la moyenne
	att.moyenne = att.moyenne / att.nombre;
	fprintf(f, "Moyenne du temps d'attente : %f\n", att.moyenne);

	fprintf(f, "\n\n");
	
	return att;
}


//------------------------------------------------------------
//					Nombre moyen de client
//------------------------------------------------------------

double nb_moyen_client ( double lambda, double mu )
{
	double alpha = lambda / mu;

	return alpha/(1-alpha);
}


//------------------------------------------------------------
//					Main
//------------------------------------------------------------

int main(int argc, char* argv[])
{
//------------------------------------------------------------
//		Initialisations et gestion des paramètres
// Dans l'ordre :
//	- lien fichier externe (default B3146.txt)
//	- temps d'étude (défault 3h)
//	- lambda (défault 18/h)
//	- mu (défault 20/h)
//------------------------------------------------------------


	//Fichier dans lequel écrire
	char * urlFile;

	//Temps d'observation
	double temps = 3*60;

	//Nombre moyen d'arrivée par unité de temps (minute)
	double lambda = 0.20;//(double) ((double) 18) / ((double) 60);

	//Nombre moyen de traitement par minute
	double mu = 0.33;//(double) ((double) 20) / ((double) 60);	

	//Quel fichier ?
	if (argc > 1) urlFile = argv[1];
	else urlFile = "B3146.txt";

	//Quel temps ?
	if (argc > 2) temps = atof(argv[2]);

	//Quel lambda ?	
	if (argc > 3) lambda = atof(argv[3]);

	//Quel capacité de traitement ?
	if (argc > 4) mu = atof(argv[4]);

	//Création du fichier
	FILE *f = fopen(urlFile, "w+");
	
	if (f != NULL)
	{

//------------------------------------------------------------
//					Partie 2
//------------------------------------------------------------
		printf("Debut partie 2 -----------------\n");

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

		printf("Generation des sequences de 1024 blocs pour les 5 generateurs.\n");
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
	        
		    x = tmp*8999+1000; /* un nombre entre 1000 et 9999 pour Von Neumann*/

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
	    
	    	//Génération des tests des runs
	    	rand_newmann[j] = runs ( array_newmann, NBVALEURS, 16 ); 
	    	rand_mt[j] = runs ( array_mt, NBVALEURS, 32 ); 
	    	rand_aes[j] = runs ( array_aes, NBVALEURS, 32 ); 
	    	rand_rand_fort[j] = runs ( array_rand_fort, NBVALEURS, 4 ); 
	    	rand_rand_faible[j] = runs ( array_rand_faible, NBVALEURS, 4 );

			printf("Partie %i/20\n", j);

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


//------------------------------------------------------------
//					Partie 4
//------------------------------------------------------------
		printf("Debut partie 4 -----------------\n");

		file_attente file_test = FileMM1(lambda, mu, temps, f);
		
		evol_client(file_test, f);

		attente att = temps_attente(file_test, f);

		double moy_client = nb_moyen_client(lambda, mu);
		fprintf(f, "Nombre moyen de client dans le système : %f\n", moy_client);

		printf("Formule de little (doit etre le plus proche de 0) : %f\n", moy_client - lambda*att.moyenne);
		
		printf("Nombre de gens arrivés : %i;\nNombre de gens partis : %i;\nMoyenne de temps passe : %f\nNombre moyen de client dans le systeme : %f\n", file_test.nb_arr, file_test.nb_dep, att.moyenne, moy_client);



//------------------------------------------------------------
//					Finalisation
//------------------------------------------------------------

		//Fermeture du fichier
		fclose(f);
	}
	else
	{
		printf("Erreur avec le fichier %s !!", urlFile);
	}

	printf("Programme termine. A plus sur le campus ! -----------------\n");
	
	//Tout s'est bien passé ! :p
    return 1;
}
