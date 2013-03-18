#include <stdlib.h>
#include<time.h>
#include <stdio.h>
#include<math.h>
#include "mt19937p.h"
#include "Rijndael.h"



typedef unsigned int word32;
typedef uint16_t word16;
typedef unsigned char word8;


// cette fonction permet de connaitre la taille décimale d'un nombre nécessaire pour le calcul de VON NEUMANN
inline int Dec_size(word32 e)
{
  unsigned i = 0;
  while (e != 0)
    {
	  ++i;
      e = e/10;
    }
  return i;
}

// =================================================================================VON NEUMANN ===============================================
// next est au premier clock la graine et ensuite l'état courant du générateur de Von Neumann. La graine devra faire 4 chiffres décimaux.
// la sortie de la fonction est également un mot de 4 chiffres décimaux.
word16 Von_Neumann(word16 *next)
{
   word32 result;    
   int pds,i;  
   result = (*next)*(*next);
   pds = Dec_size(result);
   pds=(pds-4)/2;
   for(i=0; i<pds; i++)
   {
    result = (result) / 10 ; 
   }
   result = result % 10000;
   *next = result;
   return ((word16) result);
}



