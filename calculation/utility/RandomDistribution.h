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
long Bernoulli(float p);
long Binomial(long n, float p);
long Equilikely(long a, long b);
double gammln(double xx);

//*****************************************************************************
#endif
