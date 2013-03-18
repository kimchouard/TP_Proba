
#include <stdio.h>      /* printf, scanf, NULL */
#include <stdlib.h>     /* calloc, exit, free */
#include <math.h>
#include <limits.h>
#include "Generateurs.h" //Include contentant les g�n�rateurs al�atoires

#define ARRAY_MAX_SIZE 1000

//------------------------------------------------------------
//						Al�a
//------------------------------------------------------------

double Alea()
{
    u32 Kx[NK]; // pour l'AES
	u32 Kex[NB*NR]; // pour l'AES
	u32 Px[NB]; // pour l'AES 
    srand(time(NULL));   //INIT RAND     
    int tmp =rand();

	// Initialisation de la cl� et du plaintext pour l'AES // 45 est un param�tre qui doit changer � chaque initialisation
    init_rand(Kx, Px, NK, NB, tmp);
    // construction des sous-cl�s pour l'AES
	//KeyExpansion(Kex,Kx);

	// Generation d'un nombre al�atoire avec AES (sortie sur 32 bits)
    word32 result = AES(Px, Kex);

    /*struct mt19937p mt; // Pour Mersenne-Twister
    srand(time(NULL));   //INIT RAND     
    int tmp =rand();
    // initialisation de la graine pour Mersenne-Twister    
    sgenrand(time(NULL)+(tmp), &mt);
    word32 result = genrand(&mt); */

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
	//Initialisation pour la file d'attente � renvoyer
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
  		//On incr�mente le temps d'arriv�e
  		dTempsArr += Exponentielle(lambda);
		
  		//On fait arriver un nouveau client (on l'�crit aussi dans un fichier)
  		res.arr[res.nb_arr++] = dTempsArr;
  		fprintf(f, "%f, ", dTempsArr);
  	}
  	//On annule le dernier client arriv� car on � d�pass� le temps impartit.
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
  		//Temps d'arriv�e du client suivant
  		t_arr_suiv = res.arr[res.nb_dep];

  		//Temps de d�part du client actuel
  		t_dep = res.dep[res.nb_dep-1];

  		//Temps de traitement du serveur
  		Rep = Exponentielle(mu);

  		//Si le client actuel n'est pas partit quand le suivant arrive
  		if (t_dep > t_arr_suiv)
  		{
  			//Le temps de d�part du nouveau est le temps du pr�c�dent + temps de traitement
  			res.dep[res.nb_dep] = t_dep + Rep;
  		}
  		//Si le client actuel est partit
  		else 
  		{
  			//Le temps de d�part du nouveau est son temps d'arriv� + temps de traitement
  			res.dep[res.nb_dep] = t_arr_suiv + Rep;
  		}
		
		dTempsDep = res.dep[res.nb_dep++];
  		fprintf(f, "%f, ", dTempsDep);
		
		//printf("Nouveau d�part : %f, temps de rep du server: %f\n", res.dep[res.nb_dep], Rep);
  	}
  	//On annule le dernier client partit, car on � d�pass� le temps impartit.
  	res.nb_dep--;
	fprintf(f, "\n\n");
	
	return res;
}

//------------------------------------------------------------
//					Evolution nombre client
//------------------------------------------------------------

//Structure permettant de stocker les �volutions
struct evolution
{
	double *temps;
	unsigned int *nombre;
};
typedef struct evolution evolution;

evolution evol_client(file_attente file, FILE *f)
{
	//Initialisation pour l'�volution
	evolution evo;
  	evo.temps = (double*) calloc (ARRAY_MAX_SIZE, sizeof(double));
  	evo.nombre = (unsigned int *) calloc (ARRAY_MAX_SIZE, sizeof(double));

	//En-tete pour le fichier
	fprintf(f, "Evolution des clients\n");
	
	//Parcours des 2 tableau simultan�ment
	int iArr = 0;
	int iDep = 0;
	int nbClient = 0;
	double dDep, dArr;

	//Tant que l'on est pas arriv� � la fin des 2 tableaux
  	while ( (iArr < file.nb_arr) || (iDep < file.nb_dep) )
  	{
  		//R�cup�ration du temps d'arriv� et de d�part � l'index actuel
		dArr = file.arr[iArr];
		dDep = file.dep[iDep];

		//Ajout d'une l'arriv�e
		if ((dArr < dDep) && (iArr < file.nb_arr))
		{
			evo.temps[iArr] = dArr;
			evo.nombre[iArr] = ++nbClient;
			iArr++;

			//Log de l'arriv�e dans le fichier
			fprintf(f, "%f %i\n", dArr, nbClient);
		}
		//Ajout du d�part (si on est pas � la fin du tableau !)
		else if (iDep < file.nb_dep)
		{
			evo.temps[iDep] = dDep;
			evo.nombre[iDep] = --nbClient;
			iDep++;

			//Log du d�part dans le fichier
			fprintf(f, "%f %i\n", dDep, nbClient);
		}
	}

	fprintf(f, "\n\n");
	
	return evo;
}


//------------------------------------------------------------
//					Temps pass� dans le syst�me
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
	
	//Parcours des 2 tableau simultan�ment
	int i;

	//Tant que l'on est pas arriv� � la fin des 2 tableaux
  	for ( i = 0 ; i < max ; i++ )
  	{
		//Ajout de l'heure d'arriv�e
		att.temps[i] = file.arr[i];

		//Calcul du temps total

		//0 si le client n'est pas partit
		if (i > file.nb_dep)
		{
			att.temps[i] = 0;
		}
		//Si il est partit
		else
		{
			att.temps[i] = file.dep[i] - att.temps[i];

			//On ajoute le temps � la moyenne, et on incr�ment le nombre de gens partis
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

int main(int argc, char* argv[])
{

//------------------------------------------------------------
//					Partie 4
//------------------------------------------------------------

	//Fichier dans lequel �crire
	char * urlFile;

	//Temps d'observation
	double temps = 3*60;

	//Nombre moyen d'arriv�e par unit� de temps (minute)
	double lambda = 0.20;//(double) ((double) 18) / ((double) 60);

	//Nombre moyen de traitement par minute
	double mu = 0.33;//(double) ((double) 20) / ((double) 60);	

	//Quel fichier ?
	if (argc > 1) urlFile = argv[1];
	else urlFile = "B3XXX.txt";

	//Quel temps ?
	if (argc > 2) temps = atof(argv[2]);

	//Quel lambda ?	
	if (argc > 3) lambda = atof(argv[3]);

	//Quel capacit� de traitement ?
	if (argc > 4) mu = atof(argv[4]);

	//Cr�ation du fichier
	FILE *fichier = fopen(urlFile, "w+");
	
	if (fichier != NULL)
	{
		file_attente file_test = FileMM1(lambda, mu, temps, fichier);
		
		evol_client(file_test, fichier);

		attente att = temps_attente(file_test, fichier);
		
		printf("Nombre de gens arriv�s : %i;\nNombre de gens partis : %i;\nMoyenne de temps pass� : %f\n", file_test.nb_arr, file_test.nb_dep, att.moyenne);

		fclose(fichier);
	}
	else
	{
		printf("Erreur avec le fichier %s !!", urlFile);
	}
}
