// ebdary.c - Memory Management Routines for Poroelastic Model Boundary element

/* HISTORY:
     XIULI GAI: 06/30/2002
   NOTES:
     1)  ERROR NUMBERS 461-500 ARE RESERVED FOR MEMORY MANAGEMENT */

#include "memory.h"

FORTSUB ealcbea_ (PCHAR varynam, PINT4 kind, PINT4 ndim4,
                             PINT4 arynum,  PINT4 nface, PINT4 err);
/********************************************************************/
FORTSUB ealcbea_ (PCHAR varynam, PINT4 kind, PINT4 ndim4,
                             PINT4 arynum,  PINT4 nface, PINT4 err)
/******************************************************************
 Boundary-element array creation.  Call this routine after DIVIDE but
 before any other reference to the array.

  varynam = Array name (20 charcters max) (must be terminated
  with a blank or [ if longer than 20 charcters (input)

  kind = Data type of the array (input)
       = 1 ==> REAL*4
       = 2 ==> REAL*8
       = 3 ==> INTEGER
       = 4 ==> INTEGER
       = 5 ==> LOGICAL
       = 6 ==> LOGICAL

  ndim4 = Product of the 4th and higher dimensions (input)
       = 0 ==> no higher dimensions

  arynum = Array number (output)

  nface = Boundary face number (input)
        = 1 (-X OR +X FACE)
        = 2 (-Y OR +Y FACE)
        = 3 (-Z OR +Z FACE)

  err = Error number (output)
      = 0 ==> no error
      = 464 ==> too many arrays
      = 462 ==> insufficient memory
      = 465 ==> invalid data type
*****************************************************************/
{
int i, k, na, nb, ne;
//long nm;
int nm;
char *cstring;
cstring = varynam;
for (na = 0; (na < MAXARY) && (arytyp[na] != 0); na++);
if (na == MAXARY)
   {
   *err = 464;
   return;
   }
aryfrm[na] = *kind;
switch(*kind)
   {
   case 1:
      varlen[na] = sizeof(float);
      break;
   case 2:
      varlen[na] = sizeof(double);
      break;
   case 3:
//      varlen[na] = sizeof(long);
      varlen[na] = sizeof(int);
      break;
   case 4:
//      varlen[na] = sizeof(long);
      varlen[na] = sizeof(int);
      break;
   case 5:
//      varlen[na] = sizeof(long);
      varlen[na] = sizeof(int);
      break;
   case 6:
//      varlen[na] = sizeof(long);
      varlen[na] = sizeof(int);
      break;
   default:
      *err = 465;
      return;
   }
dimext[na] = max (1, *ndim4);
arytyp[na] = 2;
arypmod[na] = *modact;  //bw
arymodel[na]=CurrentModel;
for (i = 0; i < MAXANAM; i++)
   {
   arynam[na][i] = ' ';
   if ((cstring[i] == ' ') || (cstring[i] == '[')) break;
   arynam[na][i] = cstring[i];
   }

for (nb = 0; nb < numblks; nb++)
   {
//bw  if ((myelem[nb] > 0) && ((CurrentModel==0)
//bw        || (blkmodel_c[nb] == CurrentModel)))
   if ((myelem[nb] > 0) &&                                 //bw
      (((*modact) == 0L) || ((*modact) == modblk[nb]))     //bw
      || ((*modact) == (fmodblk[nb])))                     //bw
      {
      if (*nface == 1) ne = jdim[nb] * kdim[nb];
      if (*nface == 2) ne = idim[nb] * kdim[nb];
      if (*nface == 3) ne = idim[nb] * jdim[nb];
      nm = ne * dimext[na] * varlen[na];
      if ((aryadd[nb][na] = (PADD) malloc(nm)) == NULL)
         {
         *err = 462;
         return;
         }
      memset((void*)(aryadd[nb][na]), (int) 0, (size_t) nm );
      }
   }

if (numarys <= na) numarys++;
*arynum = na;
*err = 0;
return;
}

