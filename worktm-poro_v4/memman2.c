// memman2.c - Memory Management Routines in C, part 2 of 3

// HISTORY:

// JOHN WHEELER       9/10/95   ORIGINAL BETA CODE
// SUNIL G THOMAS     -/--/--   MODS FOR TRCHEM, 32-64 BITS CONVERSION

#include "memory.h"

// ROUTINE DECLARATIONS

FORTSUB  ralcgea_   (PINT4 blknum, PINT4 dnxh, PINT4 dnyh,
         PINT4 dnzh, PINT4 dnxl, PINT4 dnyl, PINT4 dnzl, PINT4 err);
FORTSUB  kilary_    (void);
FORTSUB  callwork_  (FORTSUB (*subadd) (), PINT4 subdat);
FORTSUB  callwork_cofout_ (PINT4 nstep,PINT4 newt,PINT4 stcof,
                          PINT4 neq,PINT4 n_cof,PINT4 n_resid);

FORTSUB transferdata_ (FORTSUB (*subadd) (), PINT4 d);
/* Template for transfers added by Saumik and Ben Ganis */

FORTSUB transferhetero_ (FORTSUB (*subadd) (), PINT4 d);
/* Template for transfers in heterogeneous medium added by Saumik */

FORTSUB projections_ (FORTSUB (*subadd) (), PINT4 d);
/* Template for computing projection operators added by Saumik and Ben Ganis */

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
FORTSUB ralcgea_ (PINT4 blknum, PINT4 dnxh, PINT4 dnyh,
        PINT4 dnzh, PINT4 dnxl, PINT4 dnyl, PINT4 dnzl, PINT4 err)
{
// *******************************************************************

// Changes the dimensions of grid-element arrays for one block.  Existing
// data in the arrays is preserved.  The grid data IDIM, JDIM, KDIM, IL1,
// IL2, JL1V, JL2V, KL1, KL2, and KEYOUT may be changed in the process.

// blknum = block number (modulo 1) (input)

// dnxh,dnyh,dnzh = Changes in the number of grid elements in the x, y, and z
// directions respectively for the block (input).  These locations will be
// added (or subtracted) at the high end of the index range(s).

// dnxl,dnyl,dnzl = Changes in the number of grid elements in the x, y, and z
// directions respectively for the block (input).  These locations will be
// added (or subtracted) at the low end of the index range(s).

// err = Error number (output)
//     = 0 ==> no error
//     = 462 ==> insufficient memory
//     = 463 ==> invalid block number
//     = 467 ==> invalid dimension change

// *******************************************************************
int na, nb;
int nm, nmn, inew, jnew, knew, nnew, i, j, k, m, n, iold, jold, kold,
    nold, indm, ihgh, jhgh, khgh, ilow, jlow, klow, dil, djl, dkl,
    kmo, kmn, mo, mn, kl1, kl2;
PINT2 anew, aold, anewi, aoldi;
PINT4 a1old, a1new, a2old, a2new, a1o, a1n, a2o, a2n;

//  Test block number

nb = (*blknum) - 1;

if ((nb < 0) || (nb >= numblks))
   {
   *err = 463;
   return;
   }

// Update grid data

iold = idim[nb];
jold = jdim[nb];
kold = kdim[nb];
dil = *dnxl;
djl = *dnyl;
dkl = *dnzl;
idim[nb] = inew = iold + (*dnxh) + dil;
jdim[nb] = jnew = jold + (*dnyh) + djl;
kdim[nb] = knew = kold + (*dnzh) + dkl;
nmn = inew * jnew * knew;
if ((inew < (1 + 2 * layer[0])) || (jnew < (1 + 2 * layer[1])) ||
   (knew < (1 + 2 * layer[2]))) goto baddat;
if ((iloc1[nb] = iloc1[nb] + dil) < 1) goto baddat;
if ((iloc2[nb] = iloc2[nb] + dil) > inew) goto baddat;
kl1 = kloc1[nb];
kl2 = kloc2[nb];
if ((kloc1[nb] = kl1 + dkl) < 1) goto baddat;
if ((kloc2[nb] = kl2 + dkl) > knew) goto baddat;
n0map[nb] += jnew - jold;
nymap[nb] += jold - jnew;

//  Update grid-element arrays

ilow = max (0, -dil);
jlow = max (0, -djl);
klow = max (0, -dkl);
ihgh = min (iold, iold + (*dnxh));
jhgh = min (jold, jold + (*dnyh));
khgh = min (kold, kold + (*dnzh));
for (na = 0; na < numarys; na++)
   {
   aold = (PINT2) aryadd[nb][na];
   if ((arytyp[na] == 2) && (aold != NULL))
      {
      nm = nmn * dimext[na] * varlen[na];
      if ((anew = (PINT2) malloc(nm)) == NULL) goto errex;
      indm = varlen[na] / sizeof(int);
      for (m = 0; m < dimext[na]; m++)
         {
         mo = kold * m;
         mn = dkl + knew * m;
         for (k = klow; k < khgh; k++)
            {
            kmo = jold * (k + mo);
            kmn = djl + jnew * (k + mn);
            for (j = jlow; j < jhgh; j++)
               {
               nold = iold * (j + kmo);
               nnew = dil + inew * (j + kmn);
               for (i = ilow; i < ihgh; i++)
                  {
                  aoldi = aold + indm * (nold + i);
                  anewi = anew + indm * (nnew + i);
                  for (n = 0; n < indm; n++) *(anewi++) = *(aoldi++);
                  }
               }
            }
         }
      free (aold);
      aryadd[nb][na] = (PADD) anew;
      }
   }

//  Update keyout address

keyout[nb] = aryadd[nb][0];

//  Revise jloc1 and jloc2 vectors

a1old = jloc1[nb];
a2old = jloc2[nb];
nm = knew * sizeof(int);
if ((a1new = (PINT4) malloc(nm)) == NULL) goto errex;
if ((a2new = (PINT4) malloc(nm)) == NULL)
   {
   free (a1new);
   goto errex;
   }
for (k = 0; k < knew; k++)
   {
   a1new[k] = jnew + 1;
   a2new[k] = 1;
   }
klow = max (kl1 - 1, -dkl);
khgh = min (kl2, knew - dkl);
for (k = klow; k < khgh; k++)
   {
   a1new[k] = a1old[k + dkl] + djl;
   a2new[k] = a2old[k + dkl] + djl;
   }
jloc1[nb] = a1new;
jloc2[nb] = a2new;
free(a1old);
free(a2old);

// Exits

*err = 0;
return;
errex: *err = 462;
return;
baddat: *err = 467;
return;
}

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
FORTSUB kilary_ (void)
{
// *******************************************************************

//  Frees all memory allocated to grid-element and grid-refinement
//  arrays.  Call before exiting the simulator.

// *******************************************************************
int nb, na;

for (nb = 0; nb < numblks; nb++)
   {
   if (jloc1[nb] != NULL) free (jloc1[nb]);
   jloc1[nb] = NULL;
   if (jloc2[nb] != NULL) free (jloc2[nb]);
   jloc2[nb] = NULL;
   keyout[nb] = NULL;
   for (na = 0; na < numarys; na++)
      {
      if ((arytyp[na] > 1) && (aryadd[nb][na] != NULL))
         {
         free (aryadd[nb][na]);
         aryadd[nb][na] = NULL;
         arytyp[na] = 0;
         }
      }
   }

return;
}

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
FORTSUB callwork_ (FORTSUB (*subadd) (), PINT4 d)
{
// *******************************************************************

// Calls a work routine for all grid block for which a processor has
// grid elements.

// subadd = Name of the work routine (input).  The FORTRAN statement
// EXTERNAL WORK
// must appear in the subroutine that calls CALLWORK.

// d[ ]= Work routine data vector. (input)
// d[0] = na = Number of arguments after the 12 standard arguments.
// d[1] to d(na) = numary values set by ALCGEA, or PNTVAR.

// Note:  allblk = 0 ==> Call work routine even if processor has no elements
//                       in the fault block
//        allblk = 1 ==> Call work routine only if processor has elements
//                       in the fault block

// The first 12 arguments of all work routines are:

// IDIM, JDIM, KDIM = The first three local dimensions of grid-element arrays
// (input).  Space for communication layers are included in these dimensions.

// LDIM = The first dimension of grid-refinement arrays (input).

// IL1, IL2 = the smallest and largest local I indexes for which the work
// routine is responsible (input).

// JL1V(K), JL2V(K) = the smallest and largest local J indexes for which
// the work routine is responsible in row K. (input).

// KL1, KL2 = the smallest and largest local K indexes for which the work
// routine is responsible (input).

// KEYOUT(I,J,K) = an array defining element type.  Local indexes are
// used as subscripts for KEYOUT. (input)
// 0  ==> The grid element does not exist.  It may be outside the boundary
//        of the reservoir or may represent a shale.
// 1  ==> The grid element exists and belongs to the current processor.
//        The element is not refined.
// -1 ==> The grid element exists but belongs to a neighboring processor.
//        The element is not refined.
// N  ==> The grid element exists and belongs to the current processor.
//        The element is refined and N points to the 1st refinement index.
// -N ==> The grid element exists but belongs to a neighboring processor.
//        The element is refined and N points to the 1st refinement index.

// NBLK = Grid-block number (input) (mod 1).

// *******************************************************************
int i,narg,nbp;
void **a;

/* bag8 - check to make sure array numbers are not undefined (equal to 0),
// in which case they would be treated as the KEYOUT array and cause
// segmentation faults and/or difficult to debug stack corruption errors.
if (allblk != 0) {
for (i=1; i<=d[0]; i++) {
  if (d[i]==0) {
    printf("CALLWORK Error: Argument %d has pointer 0!\n",i);
    exit(1);
  }
}} */

narg = d[0] + 12;

for (nbcw = 0; nbcw < numblks; nbcw++)
   {
   if ((((*modact) == 0) || ((*modact) == modblk[nbcw])
     || ((*modact) == (fmodblk[nbcw]))
   ) && ((myelem[nbcw] > 0) || (allblk == 0)))
      {
      nbp = nbcw + 1;
      a = &(aryadd[nbcw][0]);
      if (narg < 23)
      {
      switch(narg)
         {
         case 12:
            (*subadd) (&(idim[nbcw]),&(jdim[nbcw]),&(kdim[nbcw]),&dimr,
               &(iloc1[nbcw]),&(iloc2[nbcw]),jloc1[nbcw],jloc2[nbcw],
               &(kloc1[nbcw]),&(kloc2[nbcw]),keyout[nbcw],&nbp);
            break;
         case 13:
            (*subadd) (&(idim[nbcw]),&(jdim[nbcw]),&(kdim[nbcw]),&dimr,
               &(iloc1[nbcw]),&(iloc2[nbcw]),jloc1[nbcw],jloc2[nbcw],
               &(kloc1[nbcw]),&(kloc2[nbcw]),keyout[nbcw],&nbp,
               *(a+d[1]));
            break;
         case 14:
            (*subadd) (&(idim[nbcw]),&(jdim[nbcw]),&(kdim[nbcw]),&dimr,
               &(iloc1[nbcw]),&(iloc2[nbcw]),jloc1[nbcw],jloc2[nbcw],
               &(kloc1[nbcw]),&(kloc2[nbcw]),keyout[nbcw],&nbp,
               *(a+d[1]),*(a+d[2]));
            break;
         case 15:
            (*subadd) (&(idim[nbcw]),&(jdim[nbcw]),&(kdim[nbcw]),&dimr,
               &(iloc1[nbcw]),&(iloc2[nbcw]),jloc1[nbcw],jloc2[nbcw],
               &(kloc1[nbcw]),&(kloc2[nbcw]),keyout[nbcw],&nbp,
               *(a+d[1]),*(a+d[2]),*(a+d[3]));
            break;
         case 16:
            (*subadd) (&(idim[nbcw]),&(jdim[nbcw]),&(kdim[nbcw]),&dimr,
               &(iloc1[nbcw]),&(iloc2[nbcw]),jloc1[nbcw],jloc2[nbcw],
               &(kloc1[nbcw]),&(kloc2[nbcw]),keyout[nbcw],&nbp,
               *(a+d[1]),*(a+d[2]),*(a+d[3]),*(a+d[4]));
            break;
         case 17:
            (*subadd) (&(idim[nbcw]),&(jdim[nbcw]),&(kdim[nbcw]),&dimr,
               &(iloc1[nbcw]),&(iloc2[nbcw]),jloc1[nbcw],jloc2[nbcw],
               &(kloc1[nbcw]),&(kloc2[nbcw]),keyout[nbcw],&nbp,
               *(a+d[1]),*(a+d[2]),*(a+d[3]),*(a+d[4]),*(a+d[5]));
            break;
         case 18:
            (*subadd) (&(idim[nbcw]),&(jdim[nbcw]),&(kdim[nbcw]),&dimr,
               &(iloc1[nbcw]),&(iloc2[nbcw]),jloc1[nbcw],jloc2[nbcw],
               &(kloc1[nbcw]),&(kloc2[nbcw]),keyout[nbcw],&nbp,
               *(a+d[1]),*(a+d[2]),*(a+d[3]),*(a+d[4]),*(a+d[5]),
               *(a+d[6]));
            break;
         case 19:
            (*subadd) (&(idim[nbcw]),&(jdim[nbcw]),&(kdim[nbcw]),&dimr,
               &(iloc1[nbcw]),&(iloc2[nbcw]),jloc1[nbcw],jloc2[nbcw],
               &(kloc1[nbcw]),&(kloc2[nbcw]),keyout[nbcw],&nbp,
               *(a+d[1]),*(a+d[2]),*(a+d[3]),*(a+d[4]),*(a+d[5]),
               *(a+d[6]),*(a+d[7]));
            break;
         case 20:
            (*subadd) (&(idim[nbcw]),&(jdim[nbcw]),&(kdim[nbcw]),&dimr,
               &(iloc1[nbcw]),&(iloc2[nbcw]),jloc1[nbcw],jloc2[nbcw],
               &(kloc1[nbcw]),&(kloc2[nbcw]),keyout[nbcw],&nbp,
               *(a+d[1]),*(a+d[2]),*(a+d[3]),*(a+d[4]),*(a+d[5]),
               *(a+d[6]),*(a+d[7]),*(a+d[8]));
            break;
         case 21:
            (*subadd) (&(idim[nbcw]),&(jdim[nbcw]),&(kdim[nbcw]),&dimr,
               &(iloc1[nbcw]),&(iloc2[nbcw]),jloc1[nbcw],jloc2[nbcw],
               &(kloc1[nbcw]),&(kloc2[nbcw]),keyout[nbcw],&nbp,
               *(a+d[1]),*(a+d[2]),*(a+d[3]),*(a+d[4]),*(a+d[5]),
               *(a+d[6]),*(a+d[7]),*(a+d[8]),*(a+d[9]));
            break;
         case 22:
            (*subadd) (&(idim[nbcw]),&(jdim[nbcw]),&(kdim[nbcw]),&dimr,
               &(iloc1[nbcw]),&(iloc2[nbcw]),jloc1[nbcw],jloc2[nbcw],
               &(kloc1[nbcw]),&(kloc2[nbcw]),keyout[nbcw],&nbp,
               *(a+d[1]),*(a+d[2]),*(a+d[3]),*(a+d[4]),*(a+d[5]),
               *(a+d[6]),*(a+d[7]),*(a+d[8]),*(a+d[9]),*(a+d[10]));
            break;
         default:
            break;
         }
      }
      else if (narg < 43)
      {
      switch(narg)
         {
         case 23:
            (*subadd) (&(idim[nbcw]),&(jdim[nbcw]),&(kdim[nbcw]),&dimr,
               &(iloc1[nbcw]),&(iloc2[nbcw]),jloc1[nbcw],jloc2[nbcw],
               &(kloc1[nbcw]),&(kloc2[nbcw]),keyout[nbcw],&nbp,
               *(a+d[1]),*(a+d[2]),*(a+d[3]),*(a+d[4]),*(a+d[5]),
               *(a+d[6]),*(a+d[7]),*(a+d[8]),*(a+d[9]),*(a+d[10]),
               *(a+d[11]));
            break;
         case 24:
            (*subadd) (&(idim[nbcw]),&(jdim[nbcw]),&(kdim[nbcw]),&dimr,
               &(iloc1[nbcw]),&(iloc2[nbcw]),jloc1[nbcw],jloc2[nbcw],
               &(kloc1[nbcw]),&(kloc2[nbcw]),keyout[nbcw],&nbp,
               *(a+d[1]),*(a+d[2]),*(a+d[3]),*(a+d[4]),*(a+d[5]),
               *(a+d[6]),*(a+d[7]),*(a+d[8]),*(a+d[9]),*(a+d[10]),
               *(a+d[11]),*(a+d[12]));
            break;
         case 25:
            (*subadd) (&(idim[nbcw]),&(jdim[nbcw]),&(kdim[nbcw]),&dimr,
               &(iloc1[nbcw]),&(iloc2[nbcw]),jloc1[nbcw],jloc2[nbcw],
               &(kloc1[nbcw]),&(kloc2[nbcw]),keyout[nbcw],&nbp,
               *(a+d[1]),*(a+d[2]),*(a+d[3]),*(a+d[4]),*(a+d[5]),
               *(a+d[6]),*(a+d[7]),*(a+d[8]),*(a+d[9]),*(a+d[10]),
               *(a+d[11]),*(a+d[12]),*(a+d[13]));
            break;
         case 26:
            (*subadd) (&(idim[nbcw]),&(jdim[nbcw]),&(kdim[nbcw]),&dimr,
               &(iloc1[nbcw]),&(iloc2[nbcw]),jloc1[nbcw],jloc2[nbcw],
               &(kloc1[nbcw]),&(kloc2[nbcw]),keyout[nbcw],&nbp,
               *(a+d[1]),*(a+d[2]),*(a+d[3]),*(a+d[4]),*(a+d[5]),
               *(a+d[6]),*(a+d[7]),*(a+d[8]),*(a+d[9]),*(a+d[10]),
               *(a+d[11]),*(a+d[12]),*(a+d[13]),*(a+d[14]));
            break;
         case 27:
            (*subadd) (&(idim[nbcw]),&(jdim[nbcw]),&(kdim[nbcw]),&dimr,
               &(iloc1[nbcw]),&(iloc2[nbcw]),jloc1[nbcw],jloc2[nbcw],
               &(kloc1[nbcw]),&(kloc2[nbcw]),keyout[nbcw],&nbp,
               *(a+d[1]),*(a+d[2]),*(a+d[3]),*(a+d[4]),*(a+d[5]),
               *(a+d[6]),*(a+d[7]),*(a+d[8]),*(a+d[9]),*(a+d[10]),
               *(a+d[11]),*(a+d[12]),*(a+d[13]),*(a+d[14]),*(a+d[15]));
            break;
         case 28:
            (*subadd) (&(idim[nbcw]),&(jdim[nbcw]),&(kdim[nbcw]),&dimr,
               &(iloc1[nbcw]),&(iloc2[nbcw]),jloc1[nbcw],jloc2[nbcw],
               &(kloc1[nbcw]),&(kloc2[nbcw]),keyout[nbcw],&nbp,
               *(a+d[1]),*(a+d[2]),*(a+d[3]),*(a+d[4]),*(a+d[5]),
               *(a+d[6]),*(a+d[7]),*(a+d[8]),*(a+d[9]),*(a+d[10]),
               *(a+d[11]),*(a+d[12]),*(a+d[13]),*(a+d[14]),*(a+d[15]),
               *(a+d[16]));
            break;
         case 29:
            (*subadd) (&(idim[nbcw]),&(jdim[nbcw]),&(kdim[nbcw]),&dimr,
               &(iloc1[nbcw]),&(iloc2[nbcw]),jloc1[nbcw],jloc2[nbcw],
               &(kloc1[nbcw]),&(kloc2[nbcw]),keyout[nbcw],&nbp,
               *(a+d[1]),*(a+d[2]),*(a+d[3]),*(a+d[4]),*(a+d[5]),
               *(a+d[6]),*(a+d[7]),*(a+d[8]),*(a+d[9]),*(a+d[10]),
               *(a+d[11]),*(a+d[12]),*(a+d[13]),*(a+d[14]),*(a+d[15]),
               *(a+d[16]),*(a+d[17]));
            break;
         case 30:
            (*subadd) (&(idim[nbcw]),&(jdim[nbcw]),&(kdim[nbcw]),&dimr,
               &(iloc1[nbcw]),&(iloc2[nbcw]),jloc1[nbcw],jloc2[nbcw],
               &(kloc1[nbcw]),&(kloc2[nbcw]),keyout[nbcw],&nbp,
               *(a+d[1]),*(a+d[2]),*(a+d[3]),*(a+d[4]),*(a+d[5]),
               *(a+d[6]),*(a+d[7]),*(a+d[8]),*(a+d[9]),*(a+d[10]),
               *(a+d[11]),*(a+d[12]),*(a+d[13]),*(a+d[14]),*(a+d[15]),
               *(a+d[16]),*(a+d[17]),*(a+d[18]));
            break;
         case 31:
            (*subadd) (&(idim[nbcw]),&(jdim[nbcw]),&(kdim[nbcw]),&dimr,
               &(iloc1[nbcw]),&(iloc2[nbcw]),jloc1[nbcw],jloc2[nbcw],
               &(kloc1[nbcw]),&(kloc2[nbcw]),keyout[nbcw],&nbp,
               *(a+d[1]),*(a+d[2]),*(a+d[3]),*(a+d[4]),*(a+d[5]),
               *(a+d[6]),*(a+d[7]),*(a+d[8]),*(a+d[9]),*(a+d[10]),
               *(a+d[11]),*(a+d[12]),*(a+d[13]),*(a+d[14]),*(a+d[15]),
               *(a+d[16]),*(a+d[17]),*(a+d[18]),*(a+d[19]));
            break;
         case 32:
            (*subadd) (&(idim[nbcw]),&(jdim[nbcw]),&(kdim[nbcw]),&dimr,
               &(iloc1[nbcw]),&(iloc2[nbcw]),jloc1[nbcw],jloc2[nbcw],
               &(kloc1[nbcw]),&(kloc2[nbcw]),keyout[nbcw],&nbp,
               *(a+d[1]),*(a+d[2]),*(a+d[3]),*(a+d[4]),*(a+d[5]),
               *(a+d[6]),*(a+d[7]),*(a+d[8]),*(a+d[9]),*(a+d[10]),
               *(a+d[11]),*(a+d[12]),*(a+d[13]),*(a+d[14]),*(a+d[15]),
               *(a+d[16]),*(a+d[17]),*(a+d[18]),*(a+d[19]),*(a+d[20]));
            break;
         case 33:
            (*subadd) (&(idim[nbcw]),&(jdim[nbcw]),&(kdim[nbcw]),&dimr,
               &(iloc1[nbcw]),&(iloc2[nbcw]),jloc1[nbcw],jloc2[nbcw],
               &(kloc1[nbcw]),&(kloc2[nbcw]),keyout[nbcw],&nbp,
               *(a+d[1]),*(a+d[2]),*(a+d[3]),*(a+d[4]),*(a+d[5]),
               *(a+d[6]),*(a+d[7]),*(a+d[8]),*(a+d[9]),*(a+d[10]),
               *(a+d[11]),*(a+d[12]),*(a+d[13]),*(a+d[14]),*(a+d[15]),
               *(a+d[16]),*(a+d[17]),*(a+d[18]),*(a+d[19]),*(a+d[20]),
               *(a+d[21]));
            break;
         case 34:
            (*subadd) (&(idim[nbcw]),&(jdim[nbcw]),&(kdim[nbcw]),&dimr,
               &(iloc1[nbcw]),&(iloc2[nbcw]),jloc1[nbcw],jloc2[nbcw],
               &(kloc1[nbcw]),&(kloc2[nbcw]),keyout[nbcw],&nbp,
               *(a+d[1]),*(a+d[2]),*(a+d[3]),*(a+d[4]),*(a+d[5]),
               *(a+d[6]),*(a+d[7]),*(a+d[8]),*(a+d[9]),*(a+d[10]),
               *(a+d[11]),*(a+d[12]),*(a+d[13]),*(a+d[14]),*(a+d[15]),
               *(a+d[16]),*(a+d[17]),*(a+d[18]),*(a+d[19]),*(a+d[20]),
               *(a+d[21]),*(a+d[22]));
            break;
         case 35:
            (*subadd) (&(idim[nbcw]),&(jdim[nbcw]),&(kdim[nbcw]),&dimr,
               &(iloc1[nbcw]),&(iloc2[nbcw]),jloc1[nbcw],jloc2[nbcw],
               &(kloc1[nbcw]),&(kloc2[nbcw]),keyout[nbcw],&nbp,
               *(a+d[1]),*(a+d[2]),*(a+d[3]),*(a+d[4]),*(a+d[5]),
               *(a+d[6]),*(a+d[7]),*(a+d[8]),*(a+d[9]),*(a+d[10]),
               *(a+d[11]),*(a+d[12]),*(a+d[13]),*(a+d[14]),*(a+d[15]),
               *(a+d[16]),*(a+d[17]),*(a+d[18]),*(a+d[19]),*(a+d[20]),
               *(a+d[21]),*(a+d[22]),*(a+d[23]));
            break;
         case 36:
            (*subadd) (&(idim[nbcw]),&(jdim[nbcw]),&(kdim[nbcw]),&dimr,
               &(iloc1[nbcw]),&(iloc2[nbcw]),jloc1[nbcw],jloc2[nbcw],
               &(kloc1[nbcw]),&(kloc2[nbcw]),keyout[nbcw],&nbp,
               *(a+d[1]),*(a+d[2]),*(a+d[3]),*(a+d[4]),*(a+d[5]),
               *(a+d[6]),*(a+d[7]),*(a+d[8]),*(a+d[9]),*(a+d[10]),
               *(a+d[11]),*(a+d[12]),*(a+d[13]),*(a+d[14]),*(a+d[15]),
               *(a+d[16]),*(a+d[17]),*(a+d[18]),*(a+d[19]),*(a+d[20]),
               *(a+d[21]),*(a+d[22]),*(a+d[23]),*(a+d[24]));
            break;
         case 37:
            (*subadd) (&(idim[nbcw]),&(jdim[nbcw]),&(kdim[nbcw]),&dimr,
               &(iloc1[nbcw]),&(iloc2[nbcw]),jloc1[nbcw],jloc2[nbcw],
               &(kloc1[nbcw]),&(kloc2[nbcw]),keyout[nbcw],&nbp,
               *(a+d[1]),*(a+d[2]),*(a+d[3]),*(a+d[4]),*(a+d[5]),
               *(a+d[6]),*(a+d[7]),*(a+d[8]),*(a+d[9]),*(a+d[10]),
               *(a+d[11]),*(a+d[12]),*(a+d[13]),*(a+d[14]),*(a+d[15]),
               *(a+d[16]),*(a+d[17]),*(a+d[18]),*(a+d[19]),*(a+d[20]),
               *(a+d[21]),*(a+d[22]),*(a+d[23]),*(a+d[24]),*(a+d[25]));
            break;
         case 38:
            (*subadd) (&(idim[nbcw]),&(jdim[nbcw]),&(kdim[nbcw]),&dimr,
               &(iloc1[nbcw]),&(iloc2[nbcw]),jloc1[nbcw],jloc2[nbcw],
               &(kloc1[nbcw]),&(kloc2[nbcw]),keyout[nbcw],&nbp,
               *(a+d[1]),*(a+d[2]),*(a+d[3]),*(a+d[4]),*(a+d[5]),
               *(a+d[6]),*(a+d[7]),*(a+d[8]),*(a+d[9]),*(a+d[10]),
               *(a+d[11]),*(a+d[12]),*(a+d[13]),*(a+d[14]),*(a+d[15]),
               *(a+d[16]),*(a+d[17]),*(a+d[18]),*(a+d[19]),*(a+d[20]),
               *(a+d[21]),*(a+d[22]),*(a+d[23]),*(a+d[24]),*(a+d[25]),
               *(a+d[26]));
            break;
         case 39:
            (*subadd) (&(idim[nbcw]),&(jdim[nbcw]),&(kdim[nbcw]),&dimr,
               &(iloc1[nbcw]),&(iloc2[nbcw]),jloc1[nbcw],jloc2[nbcw],
               &(kloc1[nbcw]),&(kloc2[nbcw]),keyout[nbcw],&nbp,
               *(a+d[1]),*(a+d[2]),*(a+d[3]),*(a+d[4]),*(a+d[5]),
               *(a+d[6]),*(a+d[7]),*(a+d[8]),*(a+d[9]),*(a+d[10]),
               *(a+d[11]),*(a+d[12]),*(a+d[13]),*(a+d[14]),*(a+d[15]),
               *(a+d[16]),*(a+d[17]),*(a+d[18]),*(a+d[19]),*(a+d[20]),
               *(a+d[21]),*(a+d[22]),*(a+d[23]),*(a+d[24]),*(a+d[25]),
               *(a+d[26]),*(a+d[27]));
            break;
         case 40:
            (*subadd) (&(idim[nbcw]),&(jdim[nbcw]),&(kdim[nbcw]),&dimr,
               &(iloc1[nbcw]),&(iloc2[nbcw]),jloc1[nbcw],jloc2[nbcw],
               &(kloc1[nbcw]),&(kloc2[nbcw]),keyout[nbcw],&nbp,
               *(a+d[1]),*(a+d[2]),*(a+d[3]),*(a+d[4]),*(a+d[5]),
               *(a+d[6]),*(a+d[7]),*(a+d[8]),*(a+d[9]),*(a+d[10]),
               *(a+d[11]),*(a+d[12]),*(a+d[13]),*(a+d[14]),*(a+d[15]),
               *(a+d[16]),*(a+d[17]),*(a+d[18]),*(a+d[19]),*(a+d[20]),
               *(a+d[21]),*(a+d[22]),*(a+d[23]),*(a+d[24]),*(a+d[25]),
               *(a+d[26]),*(a+d[27]),*(a+d[28]));
            break;
         case 41:
            (*subadd) (&(idim[nbcw]),&(jdim[nbcw]),&(kdim[nbcw]),&dimr,
               &(iloc1[nbcw]),&(iloc2[nbcw]),jloc1[nbcw],jloc2[nbcw],
               &(kloc1[nbcw]),&(kloc2[nbcw]),keyout[nbcw],&nbp,
               *(a+d[1]),*(a+d[2]),*(a+d[3]),*(a+d[4]),*(a+d[5]),
               *(a+d[6]),*(a+d[7]),*(a+d[8]),*(a+d[9]),*(a+d[10]),
               *(a+d[11]),*(a+d[12]),*(a+d[13]),*(a+d[14]),*(a+d[15]),
               *(a+d[16]),*(a+d[17]),*(a+d[18]),*(a+d[19]),*(a+d[20]),
               *(a+d[21]),*(a+d[22]),*(a+d[23]),*(a+d[24]),*(a+d[25]),
               *(a+d[26]),*(a+d[27]),*(a+d[28]),*(a+d[29]));
            break;
         case 42:
            (*subadd) (&(idim[nbcw]),&(jdim[nbcw]),&(kdim[nbcw]),&dimr,
               &(iloc1[nbcw]),&(iloc2[nbcw]),jloc1[nbcw],jloc2[nbcw],
               &(kloc1[nbcw]),&(kloc2[nbcw]),keyout[nbcw],&nbp,
               *(a+d[1]),*(a+d[2]),*(a+d[3]),*(a+d[4]),*(a+d[5]),
               *(a+d[6]),*(a+d[7]),*(a+d[8]),*(a+d[9]),*(a+d[10]),
               *(a+d[11]),*(a+d[12]),*(a+d[13]),*(a+d[14]),*(a+d[15]),
               *(a+d[16]),*(a+d[17]),*(a+d[18]),*(a+d[19]),*(a+d[20]),
               *(a+d[21]),*(a+d[22]),*(a+d[23]),*(a+d[24]),*(a+d[25]),
               *(a+d[26]),*(a+d[27]),*(a+d[28]),*(a+d[29]),*(a+d[30]));
            break;
         default:
            break;
         }
      }
      else
      {
      switch(narg)
         {
         case 43:
            (*subadd) (&(idim[nbcw]),&(jdim[nbcw]),&(kdim[nbcw]),&dimr,
               &(iloc1[nbcw]),&(iloc2[nbcw]),jloc1[nbcw],jloc2[nbcw],
               &(kloc1[nbcw]),&(kloc2[nbcw]),keyout[nbcw],&nbp,
               *(a+d[1]),*(a+d[2]),*(a+d[3]),*(a+d[4]),*(a+d[5]),
               *(a+d[6]),*(a+d[7]),*(a+d[8]),*(a+d[9]),*(a+d[10]),
               *(a+d[11]),*(a+d[12]),*(a+d[13]),*(a+d[14]),*(a+d[15]),
               *(a+d[16]),*(a+d[17]),*(a+d[18]),*(a+d[19]),*(a+d[20]),
               *(a+d[21]),*(a+d[22]),*(a+d[23]),*(a+d[24]),*(a+d[25]),
               *(a+d[26]),*(a+d[27]),*(a+d[28]),*(a+d[29]),*(a+d[30]),
               *(a+d[31]));
            break;
         case 44:
            (*subadd) (&(idim[nbcw]),&(jdim[nbcw]),&(kdim[nbcw]),&dimr,
               &(iloc1[nbcw]),&(iloc2[nbcw]),jloc1[nbcw],jloc2[nbcw],
               &(kloc1[nbcw]),&(kloc2[nbcw]),keyout[nbcw],&nbp,
               *(a+d[1]),*(a+d[2]),*(a+d[3]),*(a+d[4]),*(a+d[5]),
               *(a+d[6]),*(a+d[7]),*(a+d[8]),*(a+d[9]),*(a+d[10]),
               *(a+d[11]),*(a+d[12]),*(a+d[13]),*(a+d[14]),*(a+d[15]),
               *(a+d[16]),*(a+d[17]),*(a+d[18]),*(a+d[19]),*(a+d[20]),
               *(a+d[21]),*(a+d[22]),*(a+d[23]),*(a+d[24]),*(a+d[25]),
               *(a+d[26]),*(a+d[27]),*(a+d[28]),*(a+d[29]),*(a+d[30]),
               *(a+d[31]),*(a+d[32]));
            break;
         case 45:
            (*subadd) (&(idim[nbcw]),&(jdim[nbcw]),&(kdim[nbcw]),&dimr,
               &(iloc1[nbcw]),&(iloc2[nbcw]),jloc1[nbcw],jloc2[nbcw],
               &(kloc1[nbcw]),&(kloc2[nbcw]),keyout[nbcw],&nbp,
               *(a+d[1]),*(a+d[2]),*(a+d[3]),*(a+d[4]),*(a+d[5]),
               *(a+d[6]),*(a+d[7]),*(a+d[8]),*(a+d[9]),*(a+d[10]),
               *(a+d[11]),*(a+d[12]),*(a+d[13]),*(a+d[14]),*(a+d[15]),
               *(a+d[16]),*(a+d[17]),*(a+d[18]),*(a+d[19]),*(a+d[20]),
               *(a+d[21]),*(a+d[22]),*(a+d[23]),*(a+d[24]),*(a+d[25]),
               *(a+d[26]),*(a+d[27]),*(a+d[28]),*(a+d[29]),*(a+d[30]),
               *(a+d[31]),*(a+d[32]),*(a+d[33]));
            break;
         case 46:
            (*subadd) (&(idim[nbcw]),&(jdim[nbcw]),&(kdim[nbcw]),&dimr,
               &(iloc1[nbcw]),&(iloc2[nbcw]),jloc1[nbcw],jloc2[nbcw],
               &(kloc1[nbcw]),&(kloc2[nbcw]),keyout[nbcw],&nbp,
               *(a+d[1]),*(a+d[2]),*(a+d[3]),*(a+d[4]),*(a+d[5]),
               *(a+d[6]),*(a+d[7]),*(a+d[8]),*(a+d[9]),*(a+d[10]),
               *(a+d[11]),*(a+d[12]),*(a+d[13]),*(a+d[14]),*(a+d[15]),
               *(a+d[16]),*(a+d[17]),*(a+d[18]),*(a+d[19]),*(a+d[20]),
               *(a+d[21]),*(a+d[22]),*(a+d[23]),*(a+d[24]),*(a+d[25]),
               *(a+d[26]),*(a+d[27]),*(a+d[28]),*(a+d[29]),*(a+d[30]),
               *(a+d[31]),*(a+d[32]),*(a+d[33]),*(a+d[34]));
            break;
         case 47:
            (*subadd) (&(idim[nbcw]),&(jdim[nbcw]),&(kdim[nbcw]),&dimr,
               &(iloc1[nbcw]),&(iloc2[nbcw]),jloc1[nbcw],jloc2[nbcw],
               &(kloc1[nbcw]),&(kloc2[nbcw]),keyout[nbcw],&nbp,
               *(a+d[1]),*(a+d[2]),*(a+d[3]),*(a+d[4]),*(a+d[5]),
               *(a+d[6]),*(a+d[7]),*(a+d[8]),*(a+d[9]),*(a+d[10]),
               *(a+d[11]),*(a+d[12]),*(a+d[13]),*(a+d[14]),*(a+d[15]),
               *(a+d[16]),*(a+d[17]),*(a+d[18]),*(a+d[19]),*(a+d[20]),
               *(a+d[21]),*(a+d[22]),*(a+d[23]),*(a+d[24]),*(a+d[25]),
               *(a+d[26]),*(a+d[27]),*(a+d[28]),*(a+d[29]),*(a+d[30]),
               *(a+d[31]),*(a+d[32]),*(a+d[33]),*(a+d[34]),*(a+d[35]));
            break;
         case 48:
            (*subadd) (&(idim[nbcw]),&(jdim[nbcw]),&(kdim[nbcw]),&dimr,
               &(iloc1[nbcw]),&(iloc2[nbcw]),jloc1[nbcw],jloc2[nbcw],
               &(kloc1[nbcw]),&(kloc2[nbcw]),keyout[nbcw],&nbp,
               *(a+d[1]),*(a+d[2]),*(a+d[3]),*(a+d[4]),*(a+d[5]),
               *(a+d[6]),*(a+d[7]),*(a+d[8]),*(a+d[9]),*(a+d[10]),
               *(a+d[11]),*(a+d[12]),*(a+d[13]),*(a+d[14]),*(a+d[15]),
               *(a+d[16]),*(a+d[17]),*(a+d[18]),*(a+d[19]),*(a+d[20]),
               *(a+d[21]),*(a+d[22]),*(a+d[23]),*(a+d[24]),*(a+d[25]),
               *(a+d[26]),*(a+d[27]),*(a+d[28]),*(a+d[29]),*(a+d[30]),
               *(a+d[31]),*(a+d[32]),*(a+d[33]),*(a+d[34]),*(a+d[35]),
               *(a+d[36]));
            break;
         case 49:
            (*subadd) (&(idim[nbcw]),&(jdim[nbcw]),&(kdim[nbcw]),&dimr,
               &(iloc1[nbcw]),&(iloc2[nbcw]),jloc1[nbcw],jloc2[nbcw],
               &(kloc1[nbcw]),&(kloc2[nbcw]),keyout[nbcw],&nbp,
               *(a+d[1]),*(a+d[2]),*(a+d[3]),*(a+d[4]),*(a+d[5]),
               *(a+d[6]),*(a+d[7]),*(a+d[8]),*(a+d[9]),*(a+d[10]),
               *(a+d[11]),*(a+d[12]),*(a+d[13]),*(a+d[14]),*(a+d[15]),
               *(a+d[16]),*(a+d[17]),*(a+d[18]),*(a+d[19]),*(a+d[20]),
               *(a+d[21]),*(a+d[22]),*(a+d[23]),*(a+d[24]),*(a+d[25]),
               *(a+d[26]),*(a+d[27]),*(a+d[28]),*(a+d[29]),*(a+d[30]),
               *(a+d[31]),*(a+d[32]),*(a+d[33]),*(a+d[34]),*(a+d[35]),
               *(a+d[36]),*(a+d[37]));
            break;
         case 50:
            (*subadd) (&(idim[nbcw]),&(jdim[nbcw]),&(kdim[nbcw]),&dimr,
               &(iloc1[nbcw]),&(iloc2[nbcw]),jloc1[nbcw],jloc2[nbcw],
               &(kloc1[nbcw]),&(kloc2[nbcw]),keyout[nbcw],&nbp,
               *(a+d[1]),*(a+d[2]),*(a+d[3]),*(a+d[4]),*(a+d[5]),
               *(a+d[6]),*(a+d[7]),*(a+d[8]),*(a+d[9]),*(a+d[10]),
               *(a+d[11]),*(a+d[12]),*(a+d[13]),*(a+d[14]),*(a+d[15]),
               *(a+d[16]),*(a+d[17]),*(a+d[18]),*(a+d[19]),*(a+d[20]),
               *(a+d[21]),*(a+d[22]),*(a+d[23]),*(a+d[24]),*(a+d[25]),
               *(a+d[26]),*(a+d[27]),*(a+d[28]),*(a+d[29]),*(a+d[30]),
               *(a+d[31]),*(a+d[32]),*(a+d[33]),*(a+d[34]),*(a+d[35]),
               *(a+d[36]),*(a+d[37]),*(a+d[38]));
            break;
         case 51:
            (*subadd) (&(idim[nbcw]),&(jdim[nbcw]),&(kdim[nbcw]),&dimr,
               &(iloc1[nbcw]),&(iloc2[nbcw]),jloc1[nbcw],jloc2[nbcw],
               &(kloc1[nbcw]),&(kloc2[nbcw]),keyout[nbcw],&nbp,
               *(a+d[1]),*(a+d[2]),*(a+d[3]),*(a+d[4]),*(a+d[5]),
               *(a+d[6]),*(a+d[7]),*(a+d[8]),*(a+d[9]),*(a+d[10]),
               *(a+d[11]),*(a+d[12]),*(a+d[13]),*(a+d[14]),*(a+d[15]),
               *(a+d[16]),*(a+d[17]),*(a+d[18]),*(a+d[19]),*(a+d[20]),
               *(a+d[21]),*(a+d[22]),*(a+d[23]),*(a+d[24]),*(a+d[25]),
               *(a+d[26]),*(a+d[27]),*(a+d[28]),*(a+d[29]),*(a+d[30]),
               *(a+d[31]),*(a+d[32]),*(a+d[33]),*(a+d[34]),*(a+d[35]),
               *(a+d[36]),*(a+d[37]),*(a+d[38]),*(a+d[39]));
            break;
         case 52:
            (*subadd) (&(idim[nbcw]),&(jdim[nbcw]),&(kdim[nbcw]),&dimr,
               &(iloc1[nbcw]),&(iloc2[nbcw]),jloc1[nbcw],jloc2[nbcw],
               &(kloc1[nbcw]),&(kloc2[nbcw]),keyout[nbcw],&nbp,
               *(a+d[1]),*(a+d[2]),*(a+d[3]),*(a+d[4]),*(a+d[5]),
               *(a+d[6]),*(a+d[7]),*(a+d[8]),*(a+d[9]),*(a+d[10]),
               *(a+d[11]),*(a+d[12]),*(a+d[13]),*(a+d[14]),*(a+d[15]),
               *(a+d[16]),*(a+d[17]),*(a+d[18]),*(a+d[19]),*(a+d[20]),
               *(a+d[21]),*(a+d[22]),*(a+d[23]),*(a+d[24]),*(a+d[25]),
               *(a+d[26]),*(a+d[27]),*(a+d[28]),*(a+d[29]),*(a+d[30]),
               *(a+d[31]),*(a+d[32]),*(a+d[33]),*(a+d[34]),*(a+d[35]),
               *(a+d[36]),*(a+d[37]),*(a+d[38]),*(a+d[39]),*(a+d[40]));
            break;
         case 53:
            (*subadd) (&(idim[nbcw]),&(jdim[nbcw]),&(kdim[nbcw]),&dimr,
               &(iloc1[nbcw]),&(iloc2[nbcw]),jloc1[nbcw],jloc2[nbcw],
               &(kloc1[nbcw]),&(kloc2[nbcw]),keyout[nbcw],&nbp,
               *(a+d[1]),*(a+d[2]),*(a+d[3]),*(a+d[4]),*(a+d[5]),
               *(a+d[6]),*(a+d[7]),*(a+d[8]),*(a+d[9]),*(a+d[10]),
               *(a+d[11]),*(a+d[12]),*(a+d[13]),*(a+d[14]),*(a+d[15]),
               *(a+d[16]),*(a+d[17]),*(a+d[18]),*(a+d[19]),*(a+d[20]),
               *(a+d[21]),*(a+d[22]),*(a+d[23]),*(a+d[24]),*(a+d[25]),
               *(a+d[26]),*(a+d[27]),*(a+d[28]),*(a+d[29]),*(a+d[30]),
               *(a+d[31]),*(a+d[32]),*(a+d[33]),*(a+d[34]),*(a+d[35]),
               *(a+d[36]),*(a+d[37]),*(a+d[38]),*(a+d[39]),*(a+d[40]),
               *(a+d[41]));
            break;
         case 54:
            (*subadd) (&(idim[nbcw]),&(jdim[nbcw]),&(kdim[nbcw]),&dimr,
               &(iloc1[nbcw]),&(iloc2[nbcw]),jloc1[nbcw],jloc2[nbcw],
               &(kloc1[nbcw]),&(kloc2[nbcw]),keyout[nbcw],&nbp,
               *(a+d[1]),*(a+d[2]),*(a+d[3]),*(a+d[4]),*(a+d[5]),
               *(a+d[6]),*(a+d[7]),*(a+d[8]),*(a+d[9]),*(a+d[10]),
               *(a+d[11]),*(a+d[12]),*(a+d[13]),*(a+d[14]),*(a+d[15]),
               *(a+d[16]),*(a+d[17]),*(a+d[18]),*(a+d[19]),*(a+d[20]),
               *(a+d[21]),*(a+d[22]),*(a+d[23]),*(a+d[24]),*(a+d[25]),
               *(a+d[26]),*(a+d[27]),*(a+d[28]),*(a+d[29]),*(a+d[30]),
               *(a+d[31]),*(a+d[32]),*(a+d[33]),*(a+d[34]),*(a+d[35]),
               *(a+d[36]),*(a+d[37]),*(a+d[38]),*(a+d[39]),*(a+d[40]),
               *(a+d[41]),*(a+d[42]));
            break;
         case 55:
            (*subadd) (&(idim[nbcw]),&(jdim[nbcw]),&(kdim[nbcw]),&dimr,
               &(iloc1[nbcw]),&(iloc2[nbcw]),jloc1[nbcw],jloc2[nbcw],
               &(kloc1[nbcw]),&(kloc2[nbcw]),keyout[nbcw],&nbp,
               *(a+d[1]),*(a+d[2]),*(a+d[3]),*(a+d[4]),*(a+d[5]),
               *(a+d[6]),*(a+d[7]),*(a+d[8]),*(a+d[9]),*(a+d[10]),
               *(a+d[11]),*(a+d[12]),*(a+d[13]),*(a+d[14]),*(a+d[15]),
               *(a+d[16]),*(a+d[17]),*(a+d[18]),*(a+d[19]),*(a+d[20]),
               *(a+d[21]),*(a+d[22]),*(a+d[23]),*(a+d[24]),*(a+d[25]),
               *(a+d[26]),*(a+d[27]),*(a+d[28]),*(a+d[29]),*(a+d[30]),
               *(a+d[31]),*(a+d[32]),*(a+d[33]),*(a+d[34]),*(a+d[35]),
               *(a+d[36]),*(a+d[37]),*(a+d[38]),*(a+d[39]),*(a+d[40]),
               *(a+d[41]),*(a+d[42]),*(a+d[43]));
            break;
         case 56:
            (*subadd) (&(idim[nbcw]),&(jdim[nbcw]),&(kdim[nbcw]),&dimr,
               &(iloc1[nbcw]),&(iloc2[nbcw]),jloc1[nbcw],jloc2[nbcw],
               &(kloc1[nbcw]),&(kloc2[nbcw]),keyout[nbcw],&nbp,
               *(a+d[1]),*(a+d[2]),*(a+d[3]),*(a+d[4]),*(a+d[5]),
               *(a+d[6]),*(a+d[7]),*(a+d[8]),*(a+d[9]),*(a+d[10]),
               *(a+d[11]),*(a+d[12]),*(a+d[13]),*(a+d[14]),*(a+d[15]),
               *(a+d[16]),*(a+d[17]),*(a+d[18]),*(a+d[19]),*(a+d[20]),
               *(a+d[21]),*(a+d[22]),*(a+d[23]),*(a+d[24]),*(a+d[25]),
               *(a+d[26]),*(a+d[27]),*(a+d[28]),*(a+d[29]),*(a+d[30]),
               *(a+d[31]),*(a+d[32]),*(a+d[33]),*(a+d[34]),*(a+d[35]),
               *(a+d[36]),*(a+d[37]),*(a+d[38]),*(a+d[39]),*(a+d[40]),
               *(a+d[41]),*(a+d[42]),*(a+d[43]),*(a+d[44]));
            break;
         case 57:
            (*subadd) (&(idim[nbcw]),&(jdim[nbcw]),&(kdim[nbcw]),&dimr,
               &(iloc1[nbcw]),&(iloc2[nbcw]),jloc1[nbcw],jloc2[nbcw],
               &(kloc1[nbcw]),&(kloc2[nbcw]),keyout[nbcw],&nbp,
               *(a+d[1]),*(a+d[2]),*(a+d[3]),*(a+d[4]),*(a+d[5]),
               *(a+d[6]),*(a+d[7]),*(a+d[8]),*(a+d[9]),*(a+d[10]),
               *(a+d[11]),*(a+d[12]),*(a+d[13]),*(a+d[14]),*(a+d[15]),
               *(a+d[16]),*(a+d[17]),*(a+d[18]),*(a+d[19]),*(a+d[20]),
               *(a+d[21]),*(a+d[22]),*(a+d[23]),*(a+d[24]),*(a+d[25]),
               *(a+d[26]),*(a+d[27]),*(a+d[28]),*(a+d[29]),*(a+d[30]),
               *(a+d[31]),*(a+d[32]),*(a+d[33]),*(a+d[34]),*(a+d[35]),
               *(a+d[36]),*(a+d[37]),*(a+d[38]),*(a+d[39]),*(a+d[40]),
               *(a+d[41]),*(a+d[42]),*(a+d[43]),*(a+d[44]),*(a+d[45]));
            break;
         case 58:
            (*subadd) (&(idim[nbcw]),&(jdim[nbcw]),&(kdim[nbcw]),&dimr,
               &(iloc1[nbcw]),&(iloc2[nbcw]),jloc1[nbcw],jloc2[nbcw],
               &(kloc1[nbcw]),&(kloc2[nbcw]),keyout[nbcw],&nbp,
               *(a+d[1]),*(a+d[2]),*(a+d[3]),*(a+d[4]),*(a+d[5]),
               *(a+d[6]),*(a+d[7]),*(a+d[8]),*(a+d[9]),*(a+d[10]),
               *(a+d[11]),*(a+d[12]),*(a+d[13]),*(a+d[14]),*(a+d[15]),
               *(a+d[16]),*(a+d[17]),*(a+d[18]),*(a+d[19]),*(a+d[20]),
               *(a+d[21]),*(a+d[22]),*(a+d[23]),*(a+d[24]),*(a+d[25]),
               *(a+d[26]),*(a+d[27]),*(a+d[28]),*(a+d[29]),*(a+d[30]),
               *(a+d[31]),*(a+d[32]),*(a+d[33]),*(a+d[34]),*(a+d[35]),
               *(a+d[36]),*(a+d[37]),*(a+d[38]),*(a+d[39]),*(a+d[40]),
               *(a+d[41]),*(a+d[42]),*(a+d[43]),*(a+d[44]),*(a+d[45]),
               *(a+d[46]));
            break;
         case 59:
            (*subadd) (&(idim[nbcw]),&(jdim[nbcw]),&(kdim[nbcw]),&dimr,
               &(iloc1[nbcw]),&(iloc2[nbcw]),jloc1[nbcw],jloc2[nbcw],
               &(kloc1[nbcw]),&(kloc2[nbcw]),keyout[nbcw],&nbp,
               *(a+d[1]),*(a+d[2]),*(a+d[3]),*(a+d[4]),*(a+d[5]),
               *(a+d[6]),*(a+d[7]),*(a+d[8]),*(a+d[9]),*(a+d[10]),
               *(a+d[11]),*(a+d[12]),*(a+d[13]),*(a+d[14]),*(a+d[15]),
               *(a+d[16]),*(a+d[17]),*(a+d[18]),*(a+d[19]),*(a+d[20]),
               *(a+d[21]),*(a+d[22]),*(a+d[23]),*(a+d[24]),*(a+d[25]),
               *(a+d[26]),*(a+d[27]),*(a+d[28]),*(a+d[29]),*(a+d[30]),
               *(a+d[31]),*(a+d[32]),*(a+d[33]),*(a+d[34]),*(a+d[35]),
               *(a+d[36]),*(a+d[37]),*(a+d[38]),*(a+d[39]),*(a+d[40]),
               *(a+d[41]),*(a+d[42]),*(a+d[43]),*(a+d[44]),*(a+d[45]),
               *(a+d[46]),*(a+d[47]));
            break;
         case 60:
            (*subadd) (&(idim[nbcw]),&(jdim[nbcw]),&(kdim[nbcw]),&dimr,
               &(iloc1[nbcw]),&(iloc2[nbcw]),jloc1[nbcw],jloc2[nbcw],
               &(kloc1[nbcw]),&(kloc2[nbcw]),keyout[nbcw],&nbp,
               *(a+d[1]),*(a+d[2]),*(a+d[3]),*(a+d[4]),*(a+d[5]),
               *(a+d[6]),*(a+d[7]),*(a+d[8]),*(a+d[9]),*(a+d[10]),
               *(a+d[11]),*(a+d[12]),*(a+d[13]),*(a+d[14]),*(a+d[15]),
               *(a+d[16]),*(a+d[17]),*(a+d[18]),*(a+d[19]),*(a+d[20]),
               *(a+d[21]),*(a+d[22]),*(a+d[23]),*(a+d[24]),*(a+d[25]),
               *(a+d[26]),*(a+d[27]),*(a+d[28]),*(a+d[29]),*(a+d[30]),
               *(a+d[31]),*(a+d[32]),*(a+d[33]),*(a+d[34]),*(a+d[35]),
               *(a+d[36]),*(a+d[37]),*(a+d[38]),*(a+d[39]),*(a+d[40]),
               *(a+d[41]),*(a+d[42]),*(a+d[43]),*(a+d[44]),*(a+d[45]),
               *(a+d[46]),*(a+d[47]),*(a+d[48]));
            break;
         case 61:
            (*subadd) (&(idim[nbcw]),&(jdim[nbcw]),&(kdim[nbcw]),&dimr,
               &(iloc1[nbcw]),&(iloc2[nbcw]),jloc1[nbcw],jloc2[nbcw],
               &(kloc1[nbcw]),&(kloc2[nbcw]),keyout[nbcw],&nbp,
               *(a+d[1]),*(a+d[2]),*(a+d[3]),*(a+d[4]),*(a+d[5]),
               *(a+d[6]),*(a+d[7]),*(a+d[8]),*(a+d[9]),*(a+d[10]),
               *(a+d[11]),*(a+d[12]),*(a+d[13]),*(a+d[14]),*(a+d[15]),
               *(a+d[16]),*(a+d[17]),*(a+d[18]),*(a+d[19]),*(a+d[20]),
               *(a+d[21]),*(a+d[22]),*(a+d[23]),*(a+d[24]),*(a+d[25]),
               *(a+d[26]),*(a+d[27]),*(a+d[28]),*(a+d[29]),*(a+d[30]),
               *(a+d[31]),*(a+d[32]),*(a+d[33]),*(a+d[34]),*(a+d[35]),
               *(a+d[36]),*(a+d[37]),*(a+d[38]),*(a+d[39]),*(a+d[40]),
               *(a+d[41]),*(a+d[42]),*(a+d[43]),*(a+d[44]),*(a+d[45]),
               *(a+d[46]),*(a+d[47]),*(a+d[48]),*(a+d[49]));
            break;
         case 62:
            (*subadd) (&(idim[nbcw]),&(jdim[nbcw]),&(kdim[nbcw]),&dimr,
               &(iloc1[nbcw]),&(iloc2[nbcw]),jloc1[nbcw],jloc2[nbcw],
               &(kloc1[nbcw]),&(kloc2[nbcw]),keyout[nbcw],&nbp,
               *(a+d[1]),*(a+d[2]),*(a+d[3]),*(a+d[4]),*(a+d[5]),
               *(a+d[6]),*(a+d[7]),*(a+d[8]),*(a+d[9]),*(a+d[10]),
               *(a+d[11]),*(a+d[12]),*(a+d[13]),*(a+d[14]),*(a+d[15]),
               *(a+d[16]),*(a+d[17]),*(a+d[18]),*(a+d[19]),*(a+d[20]),
               *(a+d[21]),*(a+d[22]),*(a+d[23]),*(a+d[24]),*(a+d[25]),
               *(a+d[26]),*(a+d[27]),*(a+d[28]),*(a+d[29]),*(a+d[30]),
               *(a+d[31]),*(a+d[32]),*(a+d[33]),*(a+d[34]),*(a+d[35]),
               *(a+d[36]),*(a+d[37]),*(a+d[38]),*(a+d[39]),*(a+d[40]),
               *(a+d[41]),*(a+d[42]),*(a+d[43]),*(a+d[44]),*(a+d[45]),
               *(a+d[46]),*(a+d[47]),*(a+d[48]),*(a+d[49]),*(a+d[50]));
            break;
         case 63:
            (*subadd) (&(idim[nbcw]),&(jdim[nbcw]),&(kdim[nbcw]),&dimr,
               &(iloc1[nbcw]),&(iloc2[nbcw]),jloc1[nbcw],jloc2[nbcw],
               &(kloc1[nbcw]),&(kloc2[nbcw]),keyout[nbcw],&nbp,
               *(a+d[1]),*(a+d[2]),*(a+d[3]),*(a+d[4]),*(a+d[5]),
               *(a+d[6]),*(a+d[7]),*(a+d[8]),*(a+d[9]),*(a+d[10]),
               *(a+d[11]),*(a+d[12]),*(a+d[13]),*(a+d[14]),*(a+d[15]),
               *(a+d[16]),*(a+d[17]),*(a+d[18]),*(a+d[19]),*(a+d[20]),
               *(a+d[21]),*(a+d[22]),*(a+d[23]),*(a+d[24]),*(a+d[25]),
               *(a+d[26]),*(a+d[27]),*(a+d[28]),*(a+d[29]),*(a+d[30]),
               *(a+d[31]),*(a+d[32]),*(a+d[33]),*(a+d[34]),*(a+d[35]),
               *(a+d[36]),*(a+d[37]),*(a+d[38]),*(a+d[39]),*(a+d[40]),
               *(a+d[41]),*(a+d[42]),*(a+d[43]),*(a+d[44]),*(a+d[45]),
               *(a+d[46]),*(a+d[47]),*(a+d[48]),*(a+d[49]),*(a+d[50]),
               *(a+d[51]));
            break;
         case 67:
            (*subadd) (&(idim[nbcw]),&(jdim[nbcw]),&(kdim[nbcw]),&dimr,
               &(iloc1[nbcw]),&(iloc2[nbcw]),jloc1[nbcw],jloc2[nbcw],
               &(kloc1[nbcw]),&(kloc2[nbcw]),keyout[nbcw],&nbp,
               *(a+d[1]),*(a+d[2]),*(a+d[3]),*(a+d[4]),*(a+d[5]),
               *(a+d[6]),*(a+d[7]),*(a+d[8]),*(a+d[9]),*(a+d[10]),
               *(a+d[11]),*(a+d[12]),*(a+d[13]),*(a+d[14]),*(a+d[15]),
               *(a+d[16]),*(a+d[17]),*(a+d[18]),*(a+d[19]),*(a+d[20]),
               *(a+d[21]),*(a+d[22]),*(a+d[23]),*(a+d[24]),*(a+d[25]),
               *(a+d[26]),*(a+d[27]),*(a+d[28]),*(a+d[29]),*(a+d[30]),
               *(a+d[31]),*(a+d[32]),*(a+d[33]),*(a+d[34]),*(a+d[35]),
               *(a+d[36]),*(a+d[37]),*(a+d[38]),*(a+d[39]),*(a+d[40]),
               *(a+d[41]),*(a+d[42]),*(a+d[43]),*(a+d[44]),*(a+d[45]),
               *(a+d[46]),*(a+d[47]),*(a+d[48]),*(a+d[49]),*(a+d[50]),
               *(a+d[51]),*(a+d[52]),*(a+d[53]),*(a+d[54]),*(a+d[55]));
            break;
         case 71:
            (*subadd) (&(idim[nbcw]),&(jdim[nbcw]),&(kdim[nbcw]),&dimr,
               &(iloc1[nbcw]),&(iloc2[nbcw]),jloc1[nbcw],jloc2[nbcw],
               &(kloc1[nbcw]),&(kloc2[nbcw]),keyout[nbcw],&nbp,
               *(a+d[1]),*(a+d[2]),*(a+d[3]),*(a+d[4]),*(a+d[5]),
               *(a+d[6]),*(a+d[7]),*(a+d[8]),*(a+d[9]),*(a+d[10]),
               *(a+d[11]),*(a+d[12]),*(a+d[13]),*(a+d[14]),*(a+d[15]),
               *(a+d[16]),*(a+d[17]),*(a+d[18]),*(a+d[19]),*(a+d[20]),
               *(a+d[21]),*(a+d[22]),*(a+d[23]),*(a+d[24]),*(a+d[25]),
               *(a+d[26]),*(a+d[27]),*(a+d[28]),*(a+d[29]),*(a+d[30]),
               *(a+d[31]),*(a+d[32]),*(a+d[33]),*(a+d[34]),*(a+d[35]),
               *(a+d[36]),*(a+d[37]),*(a+d[38]),*(a+d[39]),*(a+d[40]),
               *(a+d[41]),*(a+d[42]),*(a+d[43]),*(a+d[44]),*(a+d[45]),
               *(a+d[46]),*(a+d[47]),*(a+d[48]),*(a+d[49]),*(a+d[50]),
               *(a+d[51]),*(a+d[52]),*(a+d[53]),*(a+d[54]),*(a+d[55]),
               *(a+d[56]),*(a+d[57]),*(a+d[58]),*(a+d[59]));
            break;
         case 72:
            (*subadd) (&(idim[nbcw]),&(jdim[nbcw]),&(kdim[nbcw]),&dimr,
               &(iloc1[nbcw]),&(iloc2[nbcw]),jloc1[nbcw],jloc2[nbcw],
               &(kloc1[nbcw]),&(kloc2[nbcw]),keyout[nbcw],&nbp,
               *(a+d[1]),*(a+d[2]),*(a+d[3]),*(a+d[4]),*(a+d[5]),
               *(a+d[6]),*(a+d[7]),*(a+d[8]),*(a+d[9]),*(a+d[10]),
               *(a+d[11]),*(a+d[12]),*(a+d[13]),*(a+d[14]),*(a+d[15]),
               *(a+d[16]),*(a+d[17]),*(a+d[18]),*(a+d[19]),*(a+d[20]),
               *(a+d[21]),*(a+d[22]),*(a+d[23]),*(a+d[24]),*(a+d[25]),
               *(a+d[26]),*(a+d[27]),*(a+d[28]),*(a+d[29]),*(a+d[30]),
               *(a+d[31]),*(a+d[32]),*(a+d[33]),*(a+d[34]),*(a+d[35]),
               *(a+d[36]),*(a+d[37]),*(a+d[38]),*(a+d[39]),*(a+d[40]),
               *(a+d[41]),*(a+d[42]),*(a+d[43]),*(a+d[44]),*(a+d[45]),
               *(a+d[46]),*(a+d[47]),*(a+d[48]),*(a+d[49]),*(a+d[50]),
               *(a+d[51]),*(a+d[52]),*(a+d[53]),*(a+d[54]),*(a+d[55]),
               *(a+d[56]),*(a+d[57]),*(a+d[58]),*(a+d[59]),*(a+d[60]));
            break;
         case 73:
            (*subadd) (&(idim[nbcw]),&(jdim[nbcw]),&(kdim[nbcw]),&dimr,
               &(iloc1[nbcw]),&(iloc2[nbcw]),jloc1[nbcw],jloc2[nbcw],
               &(kloc1[nbcw]),&(kloc2[nbcw]),keyout[nbcw],&nbp,
               *(a+d[1]),*(a+d[2]),*(a+d[3]),*(a+d[4]),*(a+d[5]),
               *(a+d[6]),*(a+d[7]),*(a+d[8]),*(a+d[9]),*(a+d[10]),
               *(a+d[11]),*(a+d[12]),*(a+d[13]),*(a+d[14]),*(a+d[15]),
               *(a+d[16]),*(a+d[17]),*(a+d[18]),*(a+d[19]),*(a+d[20]),
               *(a+d[21]),*(a+d[22]),*(a+d[23]),*(a+d[24]),*(a+d[25]),
               *(a+d[26]),*(a+d[27]),*(a+d[28]),*(a+d[29]),*(a+d[30]),
               *(a+d[31]),*(a+d[32]),*(a+d[33]),*(a+d[34]),*(a+d[35]),
               *(a+d[36]),*(a+d[37]),*(a+d[38]),*(a+d[39]),*(a+d[40]),
               *(a+d[41]),*(a+d[42]),*(a+d[43]),*(a+d[44]),*(a+d[45]),
               *(a+d[46]),*(a+d[47]),*(a+d[48]),*(a+d[49]),*(a+d[50]),
               *(a+d[51]),*(a+d[52]),*(a+d[53]),*(a+d[54]),*(a+d[55]),
               *(a+d[56]),*(a+d[57]),*(a+d[58]),*(a+d[59]),*(a+d[60]),
               *(a+d[61]));
            break;
         case 75:
            (*subadd) (&(idim[nbcw]),&(jdim[nbcw]),&(kdim[nbcw]),&dimr,
               &(iloc1[nbcw]),&(iloc2[nbcw]),jloc1[nbcw],jloc2[nbcw],
               &(kloc1[nbcw]),&(kloc2[nbcw]),keyout[nbcw],&nbp,
               *(a+d[1]),*(a+d[2]),*(a+d[3]),*(a+d[4]),*(a+d[5]),
               *(a+d[6]),*(a+d[7]),*(a+d[8]),*(a+d[9]),*(a+d[10]),
               *(a+d[11]),*(a+d[12]),*(a+d[13]),*(a+d[14]),*(a+d[15]),
               *(a+d[16]),*(a+d[17]),*(a+d[18]),*(a+d[19]),*(a+d[20]),
               *(a+d[21]),*(a+d[22]),*(a+d[23]),*(a+d[24]),*(a+d[25]),
               *(a+d[26]),*(a+d[27]),*(a+d[28]),*(a+d[29]),*(a+d[30]),
               *(a+d[31]),*(a+d[32]),*(a+d[33]),*(a+d[34]),*(a+d[35]),
               *(a+d[36]),*(a+d[37]),*(a+d[38]),*(a+d[39]),*(a+d[40]),
               *(a+d[41]),*(a+d[42]),*(a+d[43]),*(a+d[44]),*(a+d[45]),
               *(a+d[46]),*(a+d[47]),*(a+d[48]),*(a+d[49]),*(a+d[50]),
               *(a+d[51]),*(a+d[52]),*(a+d[53]),*(a+d[54]),*(a+d[55]),
               *(a+d[56]),*(a+d[57]),*(a+d[58]),*(a+d[59]),*(a+d[60]),
               *(a+d[61]),*(a+d[62]),*(a+d[63]));
            break;
         case 77:
            (*subadd) (&(idim[nbcw]),&(jdim[nbcw]),&(kdim[nbcw]),&dimr,
               &(iloc1[nbcw]),&(iloc2[nbcw]),jloc1[nbcw],jloc2[nbcw],
               &(kloc1[nbcw]),&(kloc2[nbcw]),keyout[nbcw],&nbp,
               *(a+d[1]),*(a+d[2]),*(a+d[3]),*(a+d[4]),*(a+d[5]),
               *(a+d[6]),*(a+d[7]),*(a+d[8]),*(a+d[9]),*(a+d[10]),
               *(a+d[11]),*(a+d[12]),*(a+d[13]),*(a+d[14]),*(a+d[15]),
               *(a+d[16]),*(a+d[17]),*(a+d[18]),*(a+d[19]),*(a+d[20]),
               *(a+d[21]),*(a+d[22]),*(a+d[23]),*(a+d[24]),*(a+d[25]),
               *(a+d[26]),*(a+d[27]),*(a+d[28]),*(a+d[29]),*(a+d[30]),
               *(a+d[31]),*(a+d[32]),*(a+d[33]),*(a+d[34]),*(a+d[35]),
               *(a+d[36]),*(a+d[37]),*(a+d[38]),*(a+d[39]),*(a+d[40]),
               *(a+d[41]),*(a+d[42]),*(a+d[43]),*(a+d[44]),*(a+d[45]),
               *(a+d[46]),*(a+d[47]),*(a+d[48]),*(a+d[49]),*(a+d[50]),
               *(a+d[51]),*(a+d[52]),*(a+d[53]),*(a+d[54]),*(a+d[55]),
               *(a+d[56]),*(a+d[57]),*(a+d[58]),*(a+d[59]),*(a+d[60]),
               *(a+d[61]),*(a+d[62]),*(a+d[63]),*(a+d[64]),*(a+d[65]));
            break;
         case 78:
            (*subadd) (&(idim[nbcw]),&(jdim[nbcw]),&(kdim[nbcw]),&dimr,
               &(iloc1[nbcw]),&(iloc2[nbcw]),jloc1[nbcw],jloc2[nbcw],
               &(kloc1[nbcw]),&(kloc2[nbcw]),keyout[nbcw],&nbp,
               *(a+d[1]),*(a+d[2]),*(a+d[3]),*(a+d[4]),*(a+d[5]),
               *(a+d[6]),*(a+d[7]),*(a+d[8]),*(a+d[9]),*(a+d[10]),
               *(a+d[11]),*(a+d[12]),*(a+d[13]),*(a+d[14]),*(a+d[15]),
               *(a+d[16]),*(a+d[17]),*(a+d[18]),*(a+d[19]),*(a+d[20]),
               *(a+d[21]),*(a+d[22]),*(a+d[23]),*(a+d[24]),*(a+d[25]),
               *(a+d[26]),*(a+d[27]),*(a+d[28]),*(a+d[29]),*(a+d[30]),
               *(a+d[31]),*(a+d[32]),*(a+d[33]),*(a+d[34]),*(a+d[35]),
               *(a+d[36]),*(a+d[37]),*(a+d[38]),*(a+d[39]),*(a+d[40]),
               *(a+d[41]),*(a+d[42]),*(a+d[43]),*(a+d[44]),*(a+d[45]),
               *(a+d[46]),*(a+d[47]),*(a+d[48]),*(a+d[49]),*(a+d[50]),
               *(a+d[51]),*(a+d[52]),*(a+d[53]),*(a+d[54]),*(a+d[55]),
               *(a+d[56]),*(a+d[57]),*(a+d[58]),*(a+d[59]),*(a+d[60]),
               *(a+d[61]),*(a+d[62]),*(a+d[63]),*(a+d[64]),*(a+d[65]),
               *(a+d[66]));
            break;
         case 79:
            (*subadd) (&(idim[nbcw]),&(jdim[nbcw]),&(kdim[nbcw]),&dimr,
               &(iloc1[nbcw]),&(iloc2[nbcw]),jloc1[nbcw],jloc2[nbcw],
               &(kloc1[nbcw]),&(kloc2[nbcw]),keyout[nbcw],&nbp,
               *(a+d[1]),*(a+d[2]),*(a+d[3]),*(a+d[4]),*(a+d[5]),
               *(a+d[6]),*(a+d[7]),*(a+d[8]),*(a+d[9]),*(a+d[10]),
               *(a+d[11]),*(a+d[12]),*(a+d[13]),*(a+d[14]),*(a+d[15]),
               *(a+d[16]),*(a+d[17]),*(a+d[18]),*(a+d[19]),*(a+d[20]),
               *(a+d[21]),*(a+d[22]),*(a+d[23]),*(a+d[24]),*(a+d[25]),
               *(a+d[26]),*(a+d[27]),*(a+d[28]),*(a+d[29]),*(a+d[30]),
               *(a+d[31]),*(a+d[32]),*(a+d[33]),*(a+d[34]),*(a+d[35]),
               *(a+d[36]),*(a+d[37]),*(a+d[38]),*(a+d[39]),*(a+d[40]),
               *(a+d[41]),*(a+d[42]),*(a+d[43]),*(a+d[44]),*(a+d[45]),
               *(a+d[46]),*(a+d[47]),*(a+d[48]),*(a+d[49]),*(a+d[50]),
               *(a+d[51]),*(a+d[52]),*(a+d[53]),*(a+d[54]),*(a+d[55]),
               *(a+d[56]),*(a+d[57]),*(a+d[58]),*(a+d[59]),*(a+d[60]),
               *(a+d[61]),*(a+d[62]),*(a+d[63]),*(a+d[64]),*(a+d[65]),
               *(a+d[66]),*(a+d[67]));
            break;
         case 80:
            (*subadd) (&(idim[nbcw]),&(jdim[nbcw]),&(kdim[nbcw]),&dimr,
               &(iloc1[nbcw]),&(iloc2[nbcw]),jloc1[nbcw],jloc2[nbcw],
               &(kloc1[nbcw]),&(kloc2[nbcw]),keyout[nbcw],&nbp,
               *(a+d[1]),*(a+d[2]),*(a+d[3]),*(a+d[4]),*(a+d[5]),
               *(a+d[6]),*(a+d[7]),*(a+d[8]),*(a+d[9]),*(a+d[10]),
               *(a+d[11]),*(a+d[12]),*(a+d[13]),*(a+d[14]),*(a+d[15]),
               *(a+d[16]),*(a+d[17]),*(a+d[18]),*(a+d[19]),*(a+d[20]),
               *(a+d[21]),*(a+d[22]),*(a+d[23]),*(a+d[24]),*(a+d[25]),
               *(a+d[26]),*(a+d[27]),*(a+d[28]),*(a+d[29]),*(a+d[30]),
               *(a+d[31]),*(a+d[32]),*(a+d[33]),*(a+d[34]),*(a+d[35]),
               *(a+d[36]),*(a+d[37]),*(a+d[38]),*(a+d[39]),*(a+d[40]),
               *(a+d[41]),*(a+d[42]),*(a+d[43]),*(a+d[44]),*(a+d[45]),
               *(a+d[46]),*(a+d[47]),*(a+d[48]),*(a+d[49]),*(a+d[50]),
               *(a+d[51]),*(a+d[52]),*(a+d[53]),*(a+d[54]),*(a+d[55]),
               *(a+d[56]),*(a+d[57]),*(a+d[58]),*(a+d[59]),*(a+d[60]),
               *(a+d[61]),*(a+d[62]),*(a+d[63]),*(a+d[64]),*(a+d[65]),
               *(a+d[66]),*(a+d[67]),*(a+d[68]));
            break;
         case 81:
            (*subadd) (&(idim[nbcw]),&(jdim[nbcw]),&(kdim[nbcw]),&dimr,
               &(iloc1[nbcw]),&(iloc2[nbcw]),jloc1[nbcw],jloc2[nbcw],
               &(kloc1[nbcw]),&(kloc2[nbcw]),keyout[nbcw],&nbp,
               *(a+d[1]),*(a+d[2]),*(a+d[3]),*(a+d[4]),*(a+d[5]),
               *(a+d[6]),*(a+d[7]),*(a+d[8]),*(a+d[9]),*(a+d[10]),
               *(a+d[11]),*(a+d[12]),*(a+d[13]),*(a+d[14]),*(a+d[15]),
               *(a+d[16]),*(a+d[17]),*(a+d[18]),*(a+d[19]),*(a+d[20]),
               *(a+d[21]),*(a+d[22]),*(a+d[23]),*(a+d[24]),*(a+d[25]),
               *(a+d[26]),*(a+d[27]),*(a+d[28]),*(a+d[29]),*(a+d[30]),
               *(a+d[31]),*(a+d[32]),*(a+d[33]),*(a+d[34]),*(a+d[35]),
               *(a+d[36]),*(a+d[37]),*(a+d[38]),*(a+d[39]),*(a+d[40]),
               *(a+d[41]),*(a+d[42]),*(a+d[43]),*(a+d[44]),*(a+d[45]),
               *(a+d[46]),*(a+d[47]),*(a+d[48]),*(a+d[49]),*(a+d[50]),
               *(a+d[51]),*(a+d[52]),*(a+d[53]),*(a+d[54]),*(a+d[55]),
               *(a+d[56]),*(a+d[57]),*(a+d[58]),*(a+d[59]),*(a+d[60]),
               *(a+d[61]),*(a+d[62]),*(a+d[63]),*(a+d[64]),*(a+d[65]),
               *(a+d[66]),*(a+d[67]),*(a+d[68]),*(a+d[69]));
            break;
         case 82:
            (*subadd) (&(idim[nbcw]),&(jdim[nbcw]),&(kdim[nbcw]),&dimr,
               &(iloc1[nbcw]),&(iloc2[nbcw]),jloc1[nbcw],jloc2[nbcw],
               &(kloc1[nbcw]),&(kloc2[nbcw]),keyout[nbcw],&nbp,
               *(a+d[1]),*(a+d[2]),*(a+d[3]),*(a+d[4]),*(a+d[5]),
               *(a+d[6]),*(a+d[7]),*(a+d[8]),*(a+d[9]),*(a+d[10]),
               *(a+d[11]),*(a+d[12]),*(a+d[13]),*(a+d[14]),*(a+d[15]),
               *(a+d[16]),*(a+d[17]),*(a+d[18]),*(a+d[19]),*(a+d[20]),
               *(a+d[21]),*(a+d[22]),*(a+d[23]),*(a+d[24]),*(a+d[25]),
               *(a+d[26]),*(a+d[27]),*(a+d[28]),*(a+d[29]),*(a+d[30]),
               *(a+d[31]),*(a+d[32]),*(a+d[33]),*(a+d[34]),*(a+d[35]),
               *(a+d[36]),*(a+d[37]),*(a+d[38]),*(a+d[39]),*(a+d[40]),
               *(a+d[41]),*(a+d[42]),*(a+d[43]),*(a+d[44]),*(a+d[45]),
               *(a+d[46]),*(a+d[47]),*(a+d[48]),*(a+d[49]),*(a+d[50]),
               *(a+d[51]),*(a+d[52]),*(a+d[53]),*(a+d[54]),*(a+d[55]),
               *(a+d[56]),*(a+d[57]),*(a+d[58]),*(a+d[59]),*(a+d[60]),
               *(a+d[61]),*(a+d[62]),*(a+d[63]),*(a+d[64]),*(a+d[65]),
               *(a+d[66]),*(a+d[67]),*(a+d[68]),*(a+d[69]),*(a+d[70]));
            break;
         case 83:
            (*subadd) (&(idim[nbcw]),&(jdim[nbcw]),&(kdim[nbcw]),&dimr,
               &(iloc1[nbcw]),&(iloc2[nbcw]),jloc1[nbcw],jloc2[nbcw],
               &(kloc1[nbcw]),&(kloc2[nbcw]),keyout[nbcw],&nbp,
               *(a+d[1]),*(a+d[2]),*(a+d[3]),*(a+d[4]),*(a+d[5]),
               *(a+d[6]),*(a+d[7]),*(a+d[8]),*(a+d[9]),*(a+d[10]),
               *(a+d[11]),*(a+d[12]),*(a+d[13]),*(a+d[14]),*(a+d[15]),
               *(a+d[16]),*(a+d[17]),*(a+d[18]),*(a+d[19]),*(a+d[20]),
               *(a+d[21]),*(a+d[22]),*(a+d[23]),*(a+d[24]),*(a+d[25]),
               *(a+d[26]),*(a+d[27]),*(a+d[28]),*(a+d[29]),*(a+d[30]),
               *(a+d[31]),*(a+d[32]),*(a+d[33]),*(a+d[34]),*(a+d[35]),
               *(a+d[36]),*(a+d[37]),*(a+d[38]),*(a+d[39]),*(a+d[40]),
               *(a+d[41]),*(a+d[42]),*(a+d[43]),*(a+d[44]),*(a+d[45]),
               *(a+d[46]),*(a+d[47]),*(a+d[48]),*(a+d[49]),*(a+d[50]),
               *(a+d[51]),*(a+d[52]),*(a+d[53]),*(a+d[54]),*(a+d[55]),
               *(a+d[56]),*(a+d[57]),*(a+d[58]),*(a+d[59]),*(a+d[60]),
               *(a+d[61]),*(a+d[62]),*(a+d[63]),*(a+d[64]),*(a+d[65]),
               *(a+d[66]),*(a+d[67]),*(a+d[68]),*(a+d[69]),*(a+d[70]),
               *(a+d[71]));
            break;
         case 84:
            (*subadd) (&(idim[nbcw]),&(jdim[nbcw]),&(kdim[nbcw]),&dimr,
               &(iloc1[nbcw]),&(iloc2[nbcw]),jloc1[nbcw],jloc2[nbcw],
               &(kloc1[nbcw]),&(kloc2[nbcw]),keyout[nbcw],&nbp,
               *(a+d[1]),*(a+d[2]),*(a+d[3]),*(a+d[4]),*(a+d[5]),
               *(a+d[6]),*(a+d[7]),*(a+d[8]),*(a+d[9]),*(a+d[10]),
               *(a+d[11]),*(a+d[12]),*(a+d[13]),*(a+d[14]),*(a+d[15]),
               *(a+d[16]),*(a+d[17]),*(a+d[18]),*(a+d[19]),*(a+d[20]),
               *(a+d[21]),*(a+d[22]),*(a+d[23]),*(a+d[24]),*(a+d[25]),
               *(a+d[26]),*(a+d[27]),*(a+d[28]),*(a+d[29]),*(a+d[30]),
               *(a+d[31]),*(a+d[32]),*(a+d[33]),*(a+d[34]),*(a+d[35]),
               *(a+d[36]),*(a+d[37]),*(a+d[38]),*(a+d[39]),*(a+d[40]),
               *(a+d[41]),*(a+d[42]),*(a+d[43]),*(a+d[44]),*(a+d[45]),
               *(a+d[46]),*(a+d[47]),*(a+d[48]),*(a+d[49]),*(a+d[50]),
               *(a+d[51]),*(a+d[52]),*(a+d[53]),*(a+d[54]),*(a+d[55]),
               *(a+d[56]),*(a+d[57]),*(a+d[58]),*(a+d[59]),*(a+d[60]),
               *(a+d[61]),*(a+d[62]),*(a+d[63]),*(a+d[64]),*(a+d[65]),
               *(a+d[66]),*(a+d[67]),*(a+d[68]),*(a+d[69]),*(a+d[70]),
               *(a+d[71]),*(a+d[72]));
            break;
         case 85:
            (*subadd) (&(idim[nbcw]),&(jdim[nbcw]),&(kdim[nbcw]),&dimr,
               &(iloc1[nbcw]),&(iloc2[nbcw]),jloc1[nbcw],jloc2[nbcw],
               &(kloc1[nbcw]),&(kloc2[nbcw]),keyout[nbcw],&nbp,
               *(a+d[1]),*(a+d[2]),*(a+d[3]),*(a+d[4]),*(a+d[5]),
               *(a+d[6]),*(a+d[7]),*(a+d[8]),*(a+d[9]),*(a+d[10]),
               *(a+d[11]),*(a+d[12]),*(a+d[13]),*(a+d[14]),*(a+d[15]),
               *(a+d[16]),*(a+d[17]),*(a+d[18]),*(a+d[19]),*(a+d[20]),
               *(a+d[21]),*(a+d[22]),*(a+d[23]),*(a+d[24]),*(a+d[25]),
               *(a+d[26]),*(a+d[27]),*(a+d[28]),*(a+d[29]),*(a+d[30]),
               *(a+d[31]),*(a+d[32]),*(a+d[33]),*(a+d[34]),*(a+d[35]),
               *(a+d[36]),*(a+d[37]),*(a+d[38]),*(a+d[39]),*(a+d[40]),
               *(a+d[41]),*(a+d[42]),*(a+d[43]),*(a+d[44]),*(a+d[45]),
               *(a+d[46]),*(a+d[47]),*(a+d[48]),*(a+d[49]),*(a+d[50]),
               *(a+d[51]),*(a+d[52]),*(a+d[53]),*(a+d[54]),*(a+d[55]),
               *(a+d[56]),*(a+d[57]),*(a+d[58]),*(a+d[59]),*(a+d[60]),
               *(a+d[61]),*(a+d[62]),*(a+d[63]),*(a+d[64]),*(a+d[65]),
               *(a+d[66]),*(a+d[67]),*(a+d[68]),*(a+d[69]),*(a+d[70]),
               *(a+d[71]),*(a+d[72]),*(a+d[73]));
            break;
         default:
            printf("Callwork case not coded with %d arguments\n",narg);
            exit(2);
            break;
         }
      }
      }
// bag8
     else
     {
// gp
//       printf("callwork exiting without calling fortran\n");
//       exit(3);
     }
   }
nbcw = -1;
allblk = 1;
return;
}

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
FORTSUB callwork_cofout_(PINT4 nstep,PINT4 newt,PINT4 stcof,
                          PINT4 neq,PINT4 n_cof,PINT4 n_resid)
{
// *******************************************************************
  int nbp,ioff,joff,koff,id,jd,kd,nb,i1,i2,j1,j2,k1,k2,ig,jg,kg,ns,ne;
  int nt,nwt,ie,je,ngrid,nvar,ist,nst,i,j,k,l,ll;
  PREAL4  COF;
  PREAL8  RES;
  FILE *fp;
  char COFfilename[50];
  char RESfilename[50];

  nt=*nstep;
  nwt=*newt;
  ns=*stcof;
  ne=*neq;

  nb = 0;
  COF = aryadd[nb][*n_cof];
  RES = aryadd[nb][*n_resid];
  ioff = iofflg[nb];
  joff = jofflg[nb];
  koff = kofflg[nb];

  id = idim[nb];
  jd = jdim[nb];
  kd = kdim[nb];

  i1=iloc1[nb]-1;
  i2=iloc2[nb]-1;
  k1=kloc1[nb]-1;
  k2=kloc2[nb]-1;
//printf("ne======%d",ne);
  ngrid = id * jd * kd;
  nst = ngrid * ns;
  nvar = nst * ne ;

// creat file name
  sprintf(COFfilename,"MATRIX_COF_%d_%d.txt",nt,nwt);
  fp=fopen(COFfilename,"w");

  for(k=k1;k<=k2;k++) {
      j1=*(jloc1[nb]+k)-1;
      j2=*(jloc2[nb]+k)-1;
      for(j=j1;j<=j2;j++) {
          for(i=i1;i<=i2;i++){
              l=k*id*jd+j*id+i;
              if(*(keyout[nb]+l)==1) {
                 ig=i+ioff+1;
                 jg=j+joff+1;
                 kg=k+koff+1;
                 for(ist=0;ist<ns;ist++) {
                    fprintf(fp,"%d %d %d %d ",ig,jg,kg,ist+1);
//                    printf("%d %d %d %d ",ig,jg,kg,ist+1);
                    ll=k*id*jd+j*id+i;
                    for(je=0;je<ne;je++) {
                        for(ie=0;ie<ne;ie++) {
                            ll=ie*nvar+je*nst+ist*ngrid+k*id*jd+j*id+i;
                            fprintf(fp,"%g ",*(COF+ll));
//                            printf("%g ",*(COF+ll));
                        }
                    }
                    fprintf(fp,"\n");
//                    printf("\n");
                 }
              }
           }
       }
   }
   sprintf(RESfilename,"RESIDUAL_%d_%d.txt",nt,nwt);
   fp=fopen(RESfilename,"w");
   for(k=k1;k<=k2;k++) {
      j1=*(jloc1[nb]+k)-1;
      j2=*(jloc2[nb]+k)-1;
      for(j=j1;j<=j2;j++) {
          for(i=i1;i<=i2;i++){
              l=k*id*jd+j*id+i;
              if(*(keyout[nb]+l)==1) {
                 ig=i+ioff+1;
                 jg=j+joff+1;
                 kg=k+koff+1;
                 fprintf(fp,"%d  %d  %d ",ig,jg,kg);
//                 printf("%d  %d  %d ",ig,jg,kg);
                 for(ie=0;ie<ne;ie++) {
                     ll=ie*ngrid+k*id*jd+j*id+i;
                     fprintf(fp,"%g  ",*(RES+ll));
//                     printf("%g  ",*(RES+ll));
                 }
                 fprintf(fp,"\n");
//                 printf("\n");
              }
          }
       }
   }
 return;
}

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
FORTSUB transferdata_ (FORTSUB (*subadd) (), PINT4 d)
{
// *******************************************************************
void **a;
void **b;
int nbp;
a = &(aryadd[0][0]);//mechanics block
for (nbcw = 1; nbcw < numblks; nbcw++)
{
//if(*modact!=modblk[nbcw]) continue;//added for multiple flow models
nbp = nbcw + 1;
if (myelem[nbcw] > 0) //if current process has elements of flow block
{
   b = &(aryadd[nbcw][0]);//flow block

   (*subadd) (&(idim[0]),&(jdim[0]),&(kdim[0]),&(iloc1[0]),
      &(iloc2[0]),
      jloc1[0],jloc2[0],&(kloc1[0]),&(kloc2[0]),keyout[0],
      &(idim[nbcw]),&(jdim[nbcw]),&(kdim[nbcw]),&(iloc1[nbcw]),
      &(iloc2[nbcw]),
      jloc1[nbcw],jloc2[nbcw],&(kloc1[nbcw]),&(kloc2[nbcw]),
      keyout[nbcw],
      *(a+d[1]),*(a+d[2]),*(a+d[3]),*(a+d[4]),*(a+d[6]),
      *(b+d[5]),*(b+d[2]),*(b+d[3]),*(b+d[4]),*(b+d[6]),&nbp);
}
}
return;
}

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
FORTSUB projections_ (FORTSUB (*subadd) (), PINT4 d)
{
// *******************************************************************
void **a;
void **b;
int nbp;
a = &(aryadd[0][0]);//mechanics block
for (nbcw = 1; nbcw < numblks; nbcw++)
{
//if(*modact!=modblk[nbcw]) continue;//added for multiple flow models
nbp = nbcw + 1;
if (myelem[nbcw] > 0)     //if current process has elements of flow block
{
   b = &(aryadd[nbcw][0]);//flow block

   (*subadd) (&(idim[0]),&(jdim[0]),&(kdim[0]),&(iloc1[0]),&(iloc2[0]),
      jloc1[0],jloc2[0],&(kloc1[0]),&(kloc2[0]),keyout[0],
      &(idim[nbcw]),&(jdim[nbcw]),&(kdim[nbcw]),&(iloc1[nbcw]),&(iloc2[nbcw]),
      jloc1[nbcw],jloc2[nbcw],&(kloc1[nbcw]),&(kloc2[nbcw]),keyout[nbcw],
      *(a+d[1]),*(a+d[2]),*(a+d[3]),*(b+d[1]),*(b+d[2]),*(b+d[3]),&nbp);
}
}
return;
}

//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
FORTSUB transferhetero_ (FORTSUB (*subadd) (), PINT4 d)
{
// *******************************************************************
void **a;
void **b;
int nbp;
a = &(aryadd[0][0]);//mechanics block
for (nbcw = 1; nbcw < numblks; nbcw++)
{
//if(*modact!=modblk[nbcw]) continue;//added for multiple flow models
nbp = nbcw + 1;
if (myelem[nbcw] > 0) //if current process has elements of flow block
{
   b = &(aryadd[nbcw][0]);//flow block

   (*subadd) (&(idim[0]),&(jdim[0]),&(kdim[0]),&(iloc1[0]),
      &(iloc2[0]),
      jloc1[0],jloc2[0],&(kloc1[0]),&(kloc2[0]),keyout[0],
      &(idim[nbcw]),&(jdim[nbcw]),&(kdim[nbcw]),&(iloc1[nbcw]),
      &(iloc2[nbcw]),
      jloc1[nbcw],jloc2[nbcw],&(kloc1[nbcw]),&(kloc2[nbcw]),
      keyout[nbcw],
      *(a+d[1]),*(a+d[2]),*(a+d[3]),*(a+d[4]),*(a+d[6]),
      *(b+d[5]),*(b+d[2]),*(b+d[3]),*(b+d[4]),*(b+d[6]),&nbp,
      *(a+d[7]),*(b+d[8]),*(a+d[9]),*(b+d[10]),*(a+d[11]),*(b+d[12]));
}
}
return;
}


