#include "SaTScan.h"
#pragma hdrstop
#include "RandomNumberGenerator.h"
/**********************************************************************
 file: RandomNumberGenerator.cpp
 This file contains functions for random number generation.  It is
 based on "rng.pas", by Steve Park, as are the following comments.

 The generator upon which it is based is a "Lehmer random number
 generator" which returns a pseudo-random real number uniformly distributed
 between 0.0 and 1.0.  The period is (m - 1) where m = 2,147,483,647 and
 the smallest and largest possible values are (1 / m) and 1 - (1 / m)
 respectively.  For more details see -
   "Random Number Generators: Good Ones Are Hard To Find"
               Steve Park & Keith Miller
         Communications of the ACM, October, 1988
 **********************************************************************/

#define M      2147483647                      /* DO NOT CHANGE THIS VALUE */
#define MAX    (M - 1)
#define A      48271
#define CHECK  399268537
#define Q      (M / A)                 /* Used for stepping through values */
#define R      (M % A)

static long seed = 12345678;                               /* initial seed */

 
double rngRand(void)
{
   long t;

   t = A * (seed % Q) - R * (seed / Q);
   if (t > 0)
      seed = t;
   else
      seed = t + M;
   return (double) seed /  (double) M;
} /* rngRand() */


long rngGetSeed(void)
{
   return seed;
} /* rngGetSeed() */


void rngPutSeed(long newseed)
{
   if (0 < newseed && newseed < M)
      seed = newseed;
} /* rngPutSeed() */

int rngTestRand(void)
{
   long i;

   rngPutSeed(1);
   for (i = 1; i <= 10000; i++)
      rngRand();
   return (rngGetSeed() == CHECK);
} /* rngTestRand() */
