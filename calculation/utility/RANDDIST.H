/**********************************************************************
 file: randdist.h
 Header file for randdist.c
 **********************************************************************/

#ifndef RANDDIST_H
#define RANDDIST_H

long Bernoulli(float p);
long Binomial(long n, float p);
long Equilikely(long a, long b);
double gammln(double xx);

#endif
