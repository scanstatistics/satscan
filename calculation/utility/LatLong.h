// LatLong.h

#ifndef __LATLONG_H
#define __LATLONG_H

//#include

#define DEBUGLATLONG 0

void ConvertFromLatLong(float Latitude, float Longitude, float* pCoords);
void ConvertToLatLong(float* Latitude, float* Longitude, float* pCoords);

#endif

