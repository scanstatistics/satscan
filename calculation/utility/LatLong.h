// LatLong.h

#ifndef __LATLONG_H
#define __LATLONG_H

#include "SSException.h"

#define DEBUGLATLONG 0

void ConvertFromLatLong(float Latitude, float Longitude, double* pCoords);
void ConvertToLatLong(float* Latitude, float* Longitude, double* pCoords);

#endif

