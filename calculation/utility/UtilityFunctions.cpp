#include "SaTScan.h"
#pragma hdrstop
#include "UtilityFunctions.h"

double CalcLogLikelihood(count_t n, measure_t u,
                         count_t N, measure_t U)
{
   double nLogLikelihood;

   try
      {
      if (n != N && n != 0)
         nLogLikelihood = n*log(n/u) + (N-n)*log((N-n)/(U-u));
      else if (n == 0)
         nLogLikelihood = (N-n) * log((N-n)/(U-u));
      else
         nLogLikelihood = n*log(n/u);
      }
   catch (SSException & x)
      {
      x.AddCallpath("CalcLogLikelihood()", "UtilityFunctions.cpp");
      throw;
      }
   return (nLogLikelihood);
}

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
void ConvertFromLatLong(float Latitude, float Longitude, double* pCoords)
{
   float RADIUS = 6367; // Constant; radius of earth in km)

   try
      {
      // Need to allocate pCoords
      //*pCoords = (double*)Smalloc(3 * sizeof(double), gpPrintDirection);

      pCoords[0] = (double)(RADIUS * cos(Longitude*PI/180.0) * cos(Latitude*PI/180.0)); // x coordinate
      pCoords[1] = (double)(RADIUS * sin(Longitude*PI/180.0) * cos(Latitude*PI/180.0)); // y coordinate
      pCoords[2] = (double)(RADIUS * sin(Latitude*PI/180.0));														// z coordinate
      }
   catch (SSException & x)
      {
      x.AddCallpath("ConvertFromLatLong(float, float, double *)", "UtilityFunctions.cpp");
      throw;
      }
}

void ConvertToLatLong(float* Latitude, float* Longitude, double* pCoords)
{
   float RADIUS = 6367; // Constant; radius of earth in km)

   try
      {
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
      //      	/(RADIUS*RADIUS)));
      }
   catch (SSException & x)
      {
      x.AddCallpath("ConvertToLatLong(float *, float *, double *)", "UtilityFunctions.cpp");
      throw;
      }
}

void DisplayVersion(FILE* fp=stdout, int nPos=0)
{
  if (nPos==1)
    fprintf(fp,"                        ");
  fprintf(fp, "SaTScan V.3\n");
}

/**********************************************************************
 Returns a pointer to the "num"-th whitespace-delimited word in a string
 (starting at word number 0).
 The value returned is a pointer to a static string.
 If there are not "num" words, NULL is returned.
 **********************************************************************/
/*KR-6/20/97staticKR-6/20/97*/
char* GetWord(char *s, int num, BasePrint *pPrintDirection)
{
   int inwd;
   char *cp = s;
   char *cp2;
   int wdlen;
  // static int   buflen = 0;
   static char buf[MAX_LINESIZE]/*KR-6/22/97 = 0*/;

   try
      {
      /* ignore spaces at start of line */
      while(isspace(*cp)) cp++;

      /* find start of word */
      inwd = !isspace(*cp);
      while (*cp != '\0' && (num > 0 || !inwd)) {
         if (inwd == !!(isspace(*cp))) {              /* entered or exited a word */
            inwd = !inwd;
            if (inwd)                          /* if entered a word, count it */
               if (--num == 0)
                  break;
            }
         cp++;                                              /* next character */
         }

      /* handle underflow */
      if (*cp == '\0')
         return 0;

      /* find end of word */
      cp2 = cp + 1;
      while (!isspace(*cp2)) cp2++;
      wdlen = cp2 - cp;
      if (wdlen>MAX_LINESIZE)
         {
         fprintf(stderr, "\n  Error: Data Buffer too small\n");
         //FatalError(0, pPrintDirection);
         SSGenerateException("\n  Error: Data Buffer too small\n","UtilityFunctions()");
         }
      /*KR-6/20/97   if (buflen <= wdlen) {
         buf = Srealloc(buf, wdlen + 10);
         buflen = wdlen + 10;
         }KR-6/20/97*/
      memcpy(buf, cp, wdlen);
      buf[wdlen] = '\0';
      }
   catch (SSException & x)
      {
      x.AddCallpath("GetWord()", "UtilityFunctions.cpp");
      throw;
      }
   return buf;
} /* GetWord() */