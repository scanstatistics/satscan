#include "SaTScan.h"
#pragma hdrstop
#include "MakeNeighbors.h"

short min2(short v1, short v2)
{
  if (v1 < v2)
    return(v1);
  else
    return(v2);
}

/**********************************************************************
 Compare two struct td's, according to dsq.
 **********************************************************************/
static int CompTd(const void *td1, const void *td2)
{
   double rtn = (((struct tdist *)td1)->dsq - ((struct tdist *)td2)->dsq);
   if (rtn < 0)   return -1;
   if (rtn > 0)   return 1;
   return 0;
} /* CompTd() */

//---------------------------------------------------------- MK 5.2001 -
// This function transforms the x and y coordinates so that circles 
//   in the transformed space represent ellipsoids in the original space.
// The input is the old X and Y coordinates, the angle of the longest
//   axis of the ellipsoid (0<=EllipseAngle<pi), and the EllipseShape (>1),
//   which is defined as the length divided by the width.
// The output is the new X and Y coordinates.
//----------------------------------------------------------------------

void Transform(double Xold, double Yold, float EllipseAngle, float EllipseShape, double* pXnew, double* pYnew)
{
   double Xnew;
   double Ynew;
   float beta;    // slope of the line perpendicular to the ellipsoids longest axis, y=beta*x
   double Xp,Yp;   // projection of (Xold,Yold) onto the line perpendicular to the ellipsoids longest axis
   float Weight;

   try
      {
      Weight=1/EllipseShape;

      if(EllipseAngle==0)
         {
         Xnew=Xold*Weight;
         Ynew=Yold;
         }
      else
         {
         beta=-cos(EllipseAngle)/sin(EllipseAngle);
         Xp=(Xold+beta*Yold)/(1+beta*beta);
         Yp=beta*Xp;
         Xnew=Xold*Weight+Xp*(1-Weight);
         Ynew=Yold*Weight+Yp*(1-Weight);
         }
      *pXnew=Xnew;
      *pYnew=Ynew;
      }
   catch (SSException & x)
      {
      x.AddCallpath("Transform()", "MakeNeighbors.cpp");
      throw;
      }
} /* void Transform */

/**********************************************************************
 For the circle [e = 0] and each ellipsoid [e = 1, 2, ... n], calculate
 the Sorted[] matrix, such that Sorted[e][a][b] is the b-th
 closest neighbor to a, and Sorted[a][0] == a.
 e = circle or ellipse1, ellipse2, etc.
 a = grid point
 b = neighbor tacts ( sorted closest to farthest.. up to maxcirclesize)
 **********************************************************************/
void MakeNeighbors(TInfo *pTInfo,
                   GInfo *pGInfo,
                   tract_t   ***SortedInt,
                   unsigned short ***SortedUShort,
                   tract_t   NumTracts,
                   tract_t   NumGridTracts,
                   measure_t Measure[],
                   measure_t MaxCircleSize,
                   measure_t nMaxMeasureToKeep,
                   tract_t** NeighborCounts,
                   int       nDimensions,
                   int       iNumEllipses,
                   double   *pdEShapes,
                   int      *piEAngles,
                   BasePrint *pPrintDirection)
{
   tract_t t;                                         /* reference tract */
   tract_t i,j;                                       /* destination tract */
   double *pCoords = 0;  /* coordinates */
   double *pCoords2 = 0;  /* coordinates */
   measure_t cummeasure;
   clock_t nStartTime;
   struct tdist *td = (tdist*)Smalloc(NumTracts * sizeof(struct tdist), pPrintDirection);
   measure_t nMaxMeasure;
   double *pNewXCoord = 0, *pNewYCoord = 0;
   float EllipseAngle;
   double Xold,Yold;
   double Xnew,Ynew;
   long   lTotalIterations = 0, lCurrentEllipse = 0;
   int es,ea;

   try
      {
      pPrintDirection->SatScanPrintf("Constructing the circles\n\n");

      // Actually, if MaxMeasure to be kept, neighbors don't need to be counted...KR-980327
      if (nMaxMeasureToKeep > MaxCircleSize)
        nMaxMeasure = nMaxMeasureToKeep;
      else
        nMaxMeasure = MaxCircleSize;
    
      nStartTime = clock();

      pCoords = (double*)Smalloc(pGInfo->giGetNumDimensions() * sizeof(double), pPrintDirection);
      pCoords2 = (double*)Smalloc(pGInfo->giGetNumDimensions() * sizeof(double), pPrintDirection);
    
      //Circle Calculations
      for (t = 0; t < NumGridTracts; t++)  // for each tract, ...
      {
        pGInfo->giGetCoords2(t, pCoords);
        for (i = 0; i < NumTracts; i++)  // find distances
        {
          td[i].t = i;
          pTInfo->tiGetCoords2(i, pCoords2);
          td[i].dsq = pTInfo->tiGetDistanceSq(pCoords, pCoords2);
          //free(pCoords2);    //DTG - create pCoords2 before loop and free after.
                               //Same size for all loops of the array
        }
        qsort(td, NumTracts, sizeof(struct tdist), CompTd);
    
        // find number of neighbors, allocate memory
        cummeasure = 0;
        for (i=0; i < NumTracts && cummeasure+Measure[td[i].t] <= nMaxMeasure; i++)
        {
          cummeasure += Measure[td[i].t];
          if (cummeasure <= MaxCircleSize)
            NeighborCounts[0][t]++;
        }
        if (SortedInt)
           {
           //SortedInt[0][t] = (tract_t*)Smalloc((i+1) * sizeof(tract_t), pPrintDirection);  /*NeighborCounts[t] could be used instead of i */
           SortedInt[0][t] = (tract_t*)Smalloc(i * sizeof(tract_t), pPrintDirection);
           /* copy tract numbers */
           for (j = i-1; j >= 0; j--)   /*NeighborCounts[t]-1 could be used instead of i-1 */
             SortedInt[0][t][j] = td[j].t;
           //SortedInt[0][t][i] = -1;           //DTG - temp line and also modified Smalloc to be ( i + 1)
           }
        else
           {
           //SortedUShort[0][t] = (unsigned short*)Smalloc((i+1) * sizeof(unsigned short), pPrintDirection);
           SortedUShort[0][t] = (unsigned short*)Smalloc(i * sizeof(unsigned short), pPrintDirection);
           /* copy tract numbers */
           for (j = i-1; j >= 0; j--)   /*NeighborCounts[t]-1 could be used instead of i-1 */
             SortedUShort[0][t][j] = td[j].t;
           //SortedUShort[0][t][i] = 0;           //DTG - USE TO BE -1 temp line and also modified Smalloc to be ( i + 1)
           }


        if(t==9)
          ReportTimeEstimate(nStartTime, NumGridTracts, t+1, pPrintDirection);

        //free(pCoords);
      } /* for t */


       //Ellipsoid calculations.
       if (iNumEllipses>0)
          {
         pPrintDirection->SatScanPrintf("Constructing the ellipsoids\n\n");
          nStartTime = clock();
          //For computation "time" - compute the number of iterations..
          for (es=0; es<iNumEllipses; es++)
             for (ea=0; ea<piEAngles[es]; ea++)
                lTotalIterations += NumGridTracts;

          pNewXCoord = new double[NumTracts];
          pNewYCoord = new double[NumTracts];
          for (es=0; es<iNumEllipses; es++)
             {
             for(ea=0; ea<piEAngles[es]; ea++)
                {
                lCurrentEllipse++;

                EllipseAngle=PI*ea/piEAngles[es];
                for(i=0; i<NumTracts;i++)
                   {
                   pTInfo->tiGetCoords2(i, pCoords2);
                   //Xold=pCoords2[0];
                   //Yold=pCoords2[1];
                   Transform(pCoords2[0],pCoords2[1],EllipseAngle,pdEShapes[es],&pNewXCoord[i],&pNewYCoord[i]);      // pointers????????
                   }
                for (t = 0; t < NumGridTracts; t++)  // for each grid tract, ...
                   {
                   pGInfo->giGetCoords2(t, pCoords);
                  // Xold=pCoords[0];
                  // Yold=pCoords[1];
                   Transform(pCoords[0],pCoords[1],EllipseAngle,pdEShapes[es],&pCoords[0],&pCoords[1]);

                   for (i = 0; i < NumTracts; i++)  // find distances
                      {
                      td[i].t = i;
                      // tiGetCoords2(i, pCoords2);
                      pCoords2[0] = pNewXCoord[i];
                      pCoords2[1] = pNewYCoord[i];
                      td[i].dsq = pTInfo->tiGetDistanceSq(pCoords, pCoords2);   //"pCoords" should be from Xnew and Ynew!!!!!!
                                                             //"pCoords2" should be from the NewXCoord and NewYCoord!!!!!
                      }
                   qsort(td, NumTracts, sizeof(struct tdist), CompTd);

                   // find number of neighbors, allocate memory
                   cummeasure = 0;
                   for (i=0; i < NumTracts && cummeasure+Measure[td[i].t] <= nMaxMeasure; i++)
                      {
                      cummeasure += Measure[td[i].t];
                      if (cummeasure <= MaxCircleSize)
                         NeighborCounts[lCurrentEllipse][t]++;               // Needs to be saved as a 2-dimensional array!!!!!
                      }
                   if (SortedInt)
                      {
                      //SortedInt[lCurrentEllipse][t] = (tract_t*)Smalloc((i + 1) * sizeof(tract_t), pPrintDirection); /*NeighborCounts[t] could be used instead of i */
                      SortedInt[lCurrentEllipse][t] = (tract_t*)Smalloc(i * sizeof(tract_t), pPrintDirection);
                      /* copy tract numbers */
                      for (j = i-1/*NeighborCounts[t]-1*/; j >= 0; j--)
                         SortedInt[lCurrentEllipse][t][j] = td[j].t;
                      //SortedInt[lCurrentEllipse][t][i] = 0;    //DTG - temp for printing sorted list - also updated Smalloc to be (i + 1)
                      }
                   else
                      {
                      //SortedUShort[lCurrentEllipse][t] = (unsigned short*)Smalloc((i + 1) * sizeof(unsigned short), pPrintDirection); /*NeighborCounts[t] could be used instead of i */
                      SortedUShort[lCurrentEllipse][t] = (unsigned short*)Smalloc(i * sizeof(unsigned short), pPrintDirection);
                      /* copy tract numbers */
                      for (j = i-1/*NeighborCounts[t]-1*/; j >= 0; j--)
                         SortedUShort[lCurrentEllipse][t][j] = td[j].t;
                      //SortedUShort[lCurrentEllipse][t][i] = 0;                // DTG - USE TO BE  -1   temp for printing sorted list - also updated Smalloc to be (i + 1)
                      }
                   //free(pCoords);    //    needed????
                   if ((t==9) && (es == 0) && (ea == 0))
                      ReportTimeEstimate(nStartTime, lTotalIterations, t+1, pPrintDirection); // Instead of passing in NumGridTracts as second parameter...
                   } /* for t */                                             // It is the total number of iterations for the Ellipses/Angles/Tracts
                } /* for ea */
             } /* for es */
          delete [] pNewXCoord; pNewXCoord = 0;
          delete [] pNewYCoord; pNewYCoord = 0;
          }
       free(pCoords); pCoords = 0;
       free(pCoords2);pCoords2 = 0;
       free(td);      td = 0;
    
     //temporary print function to view contents of sorted array
     // PrintNeighbors((iNumEllipses + 1), NumTracts, Sorted);
     }
   catch (SSException & x)
      {
      delete [] pNewXCoord;
      delete [] pNewYCoord;
      free(pCoords);
      free(pCoords2);
      free(td);
      x.AddCallpath("MakeNeighbors()", "MakeNeighbors.cpp");
      throw;
      }
}

void PrintNeighbors(long lTotalNumEllipses, tract_t GridTracts, tract_t ***Sorted, BasePrint *pPrintDirection)
{
   FILE* pFile;
   int i, j, k;

   try
      {
      if ((pFile = fopen("c:\\SatScan V.2.1.4\\Borland Calc\\neighbors.txt", "w")) == NULL)
         SSGenerateException("  Error: Unable to open neighbors file.\n", "PrintNeighbors()");
      else
         {
         for (i = 0; i <= lTotalNumEllipses; i ++)
            for (j = 0; j < GridTracts; j++)
               {
               k = 0;
               while (Sorted[i][j][k] >= 0)
                  fprintf(pFile, "was here %i \n", Sorted[i][j][k++]);
               }
         fclose(pFile);
         }
      }
   catch (SSException & x)
      {
      x.AddCallpath("PrintNeighbors()", "MakeNeighbors.cpp");
      throw;
      }
}



