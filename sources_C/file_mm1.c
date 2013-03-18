
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

	// Initialisation de la cl� et du plaintext pour l'AES // 45 est un param�tre qui doit changer � chaque initialisation
    init_rand(Kx, Px, NK, NB, 45);
    // construction des sous-cl�s pour l'AES
	KeyExpansion(Kex,Kx);

	// Generation d'un nombre al�atoire avec AES (sortie sur 32 bits)
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

file_attente FileMM1 ( double lambda, double mu, double D) {
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

  	while (dTempsArr <= D)
  	{
  		//On incr�mente le temps d'arriv�e
  		dTempsArr += Exponentielle(lambda);
		
  		//On fait arriver un nouveau client
  		res.arr[res.nb_arr++] = dTempsArr;
  	}
  	//On annule le dernier client arriv� car on � d�pass� le temps impartit.
  	res.nb_arr--;

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
		
		//printf("Nouveau d�part : %f, temps de rep du server: %f\n", res.dep[res.nb_dep], Rep);
  	}
	
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

evolution evol_client(file_attente file)
{
	//Initialisation pour l'�volution
	evolution evo;
  	evo.temps = (double*) calloc (ARRAY_MAX_SIZE, sizeof(double));
  	evo.nombre = (unsigned int *) calloc (ARRAY_MAX_SIZE, sizeof(double));
	
	//D�termination de la taille du plus grand tableau
	int max = (file.nb_arr < file.nb_dep) ? file.nb_dep : file.nb_arr;
	
	//Parcours des 2 tableau simultan�ment
	int i, nbClient;
	double dDep, dArr;
  	for (i = 0 ; i <= max; i++)
  	{
		//Ajout du d�part
		dDep = file.dep[i];
		if (dDep != 0)
		{
			evo.temps[i] = dDep;
			evo.nombre[i] = ++nbClient;
		}
		
		//Ajout de l'arriv�e
		dArr = file.arr[i];
		if (dArr != 0)
		{
			evo.temps[i] = dArr;
			evo.nombre[i] = --nbClient;
		}
	}
	
	return evo;
}

int main()
{
	//Temps d'observation
	double temps = 10;

	//Nombre moyen d'arriv�e par unit� de temps (minute)
	double lambda = 1;

	//Nombre moyen de traitement par minute
	double mu = 1;
	
	file_attente file_test = FileMM1(lambda, mu, temps);
	
	evolution evol_text = evol_client(file_test);
	
	printf("Nombre de gens arriv�s : %i;\nNombre de gens partis : %i;\n", file_test.nb_arr, file_test.nb_dep);
}
