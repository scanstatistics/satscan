//*****************************************************************************
#ifndef __UTILITYFUNCTIONS_H
#define __UTILITYFUNCTIONS_H
//*****************************************************************************
#include "SaTScan.h"

#define DEBUGLATLONG 0

double 			CalcLogLikelihood(count_t n, measure_t u, count_t N, measure_t U);
void 			ConvertFromLatLong(float Latitude, float Longitude, double* pCoords);
void 			ConvertToLatLong(float* Latitude, float* Longitude, double* pCoords);
void 			DisplayVersion(FILE* fp, int nPos);
double                  GetDuczmalCorrection(double dEllipseShape);
char		      * GetWord(char *s, int num, BasePrint *pPrintDirection);

//*****************************************************************************
#endif 
