//*****************************************************************************
#ifndef __RANDOMDISTRIBUTION_H
#define __RANDOMDISTRIBUTION_H
//*****************************************************************************
#include <math.h>
#include <stdio.h>
#include "RandomNumberGenerator.h"

/**********************************************************************
 file: RandomDistribution.h
 Header file for RandomDistribution.cpp
 **********************************************************************/
long    Bernoulli(float p, RandomNumberGenerator & rng);
long    Binomial(long n, float p, RandomNumberGenerator & rng);
long    Equilikely(long a, long b, RandomNumberGenerator & rng);
double  gammln(double xx);
//*****************************************************************************
#endif
