//*****************************************************************************
#ifndef __RANDOMNUMBERGENERATOR_H
#define __RANDOMNUMBERGENERATOR_H
//*****************************************************************************

/**********************************************************************
 file: RandomNumberGenerator.h
 Header file for "RandomNumberGenerator.cpp"
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



/**********************************************************************
 Lehmer generator which returns a pseudo-random real number uniformly
 distributed between 0 and 1.
 **********************************************************************/
double rngRand(void);
float rngRandFloat(void);

/**********************************************************************
 Returns the current random number generator seed.
 **********************************************************************/
long rngGetSeed(void);

/**********************************************************************
 Sets the random number generator seed.  Note:
   0 < newseed < m (see top of file)
 **********************************************************************/
void rngPutSeed(long newseed);

/**********************************************************************
 Tests for a correct implementation.
 Return value:
   1 = Correct
   0 = incorrect
 **********************************************************************/
int rngTestRand(void);

//*****************************************************************************
#endif
