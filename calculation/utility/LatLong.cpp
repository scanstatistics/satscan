// LatLong.cpp
#include "LatLong.h"
#include <math.h>
#define PI 3.1415926535897932384626433832795028841972  /*KR-7/11/97*/
//others

// Conversion routines for Latitude/Longitude option for data input
// and output based on the following formulas:
//
//		d = latitude
//		l = longitude
//		a = radius (fixed = 6367 km)
//
//		x = a cos(l) cos(d)
//		y = a sin(l) cos(d)
//		z = a sin(d)
//
//		Latitude and Longitude values should be float, ie, real numbers
//		with ranges [-90,90] and [-180,180] (in degrees) respectively.

void ConvertFromLatLong(float Latitude, float Longitude, float* pCoords)
{
	float RADIUS = 6367; // Constant; radius of earth in km)

  // Need to allocate pCoords
  //*pCoords = (float*)Smalloc(3 * sizeof(float));

   pCoords[0] = (float)(RADIUS * cos(Longitude*PI/180.0) * cos(Latitude*PI/180.0)); // x coordinate
   pCoords[1] = (float)(RADIUS * sin(Longitude*PI/180.0) * cos(Latitude*PI/180.0)); // y coordinate
   pCoords[2] = (float)(RADIUS * sin(Latitude*PI/180.0));														// z coordinate
}

void ConvertToLatLong(float* Latitude, float* Longitude, float* pCoords)
{
	float RADIUS = 6367; // Constant; radius of earth in km)

  if (pCoords[0] != 0)
  {
  	*Longitude = (float)(atan(pCoords[1] / pCoords[0]) * 180.0 / PI);
    if (pCoords[0] < 0 && pCoords[1] > 0)
    	*Longitude += 180.0;
    else if (pCoords[0] < 0 && pCoords[1] < 0)
			*Longitude -= 180.0;
  }
  else if (pCoords[1] > 0)
  	*Longitude = 90.0;
  else if (pCoords[1] < 0)
    *Longitude = -90.0;
  else if (pCoords[1] == 0)
  	*Longitude = 0.0;

  float tmp = (float)sqrt((pCoords[0]*pCoords[0] + pCoords[1]*pCoords[1])
  									/(RADIUS*RADIUS));
  *Latitude = (float)((pCoords[2] >= 0 ? (1.0) : (-1.0)) * acos(tmp) * 180.0 / PI);
//  *Latitude = acos(sqrt((pCoords[0]*pCoords[0] + pCoords[1]*pCoords[1])
//  									/(RADIUS*RADIUS)));
}
