#include "SaTScan.h"
#pragma hdrstop
#include "MakeNeighbors.h"

/** Counts neighbors through expected number of cases using measure array. */
tract_t CountNeighborsByMeasure(std::vector<TractDistance>& vTractDistances,
			        measure_t Measure[], measure_t MaxCircleSize, measure_t nMaxMeasure)
{
   measure_t cummeasure=0;
   tract_t   tCount=0;
   size_t    i;

   for (i=0; (i < vTractDistances.size()) &&
             (cummeasure + Measure[vTractDistances[i].GetTractNumber()] <= nMaxMeasure); i++)
     {
     cummeasure += Measure[vTractDistances[i].GetTractNumber()];
     if (cummeasure <= MaxCircleSize)
       tCount++;
     }
   return tCount;
}

/** Counts neighbors through accumulated distance. */
tract_t CountNeighborsByDistance(std::vector<TractDistance>& vTractDistances,
                                 measure_t MaxCircleSize)
{
   float     fCummulatedDistance=0;
   size_t    i;
   tract_t   tCount=0;

   for (i=0; (i < vTractDistances.size()) &&
             (fCummulatedDistance + vTractDistances[i].GetDistance() <= MaxCircleSize/*nMaxMeasure*/); i++)
     {
     fCummulatedDistance += vTractDistances[i].GetDistance();
     if (fCummulatedDistance <= MaxCircleSize)
       tCount++;
     }
   return tCount;
}

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
                   int       iSpatialMaxType,
                   BasePrint *pPrintDirection)
{
   tract_t t;                                         /* reference tract */
   tract_t j, k;                                      /* destination tract */
   double *pCoords = 0;  /* coordinates */
   double *pCoords2 = 0;  /* coordinates */
   measure_t cummeasure;
   clock_t nStartTime;
   measure_t nMaxMeasure;
   double *pNewXCoord = 0, *pNewYCoord = 0;
   float EllipseAngle;
   double Xold,Yold;
   double Xnew,Ynew;
   long   lTotalIterations = 0, lCurrentEllipse = 0;
   int es,ea;

   std::vector<TractDistance> vTractDistances;
   vTractDistances.reserve(NumTracts);


   try
      {
      pPrintDirection->SatScanPrintf("Constructing the circles\n\n");

      // Actually, if MaxMeasure to be kept, neighbors don't need to be counted...KR-980327
      if (nMaxMeasureToKeep > MaxCircleSize)
        nMaxMeasure = nMaxMeasureToKeep;
      else
        nMaxMeasure = MaxCircleSize;

      // Initialize Tract Distamce vector
      for (tract_t t=0; t < NumTracts; t++)
         vTractDistances.push_back(TractDistance());

      nStartTime = clock();

      pCoords = (double*)Smalloc(pGInfo->giGetNumDimensions() * sizeof(double), pPrintDirection);
      pCoords2 = (double*)Smalloc(pGInfo->giGetNumDimensions() * sizeof(double), pPrintDirection);
    
      //Circle Calculations
      for (t = 0; t < NumGridTracts; t++)  // for each tract, ...
      {
        pGInfo->giGetCoords2(t, pCoords);
        for (k=0; k < NumTracts; k++)  // find distances
        {
          vTractDistances[k].SetTractNumber(k);
          pTInfo->tiGetCoords2(k, pCoords2);
          vTractDistances[k].SetDistance(pTInfo->tiGetDistanceSq(pCoords, pCoords2));
        }
        std::sort(vTractDistances.begin(), vTractDistances.end(), CompareTractDistance());
        if (iSpatialMaxType == PERCENTAGEOFMEASURETYPE)
          NeighborCounts[0][t] += CountNeighborsByMeasure(vTractDistances, Measure, MaxCircleSize, nMaxMeasure);
        else if (iSpatialMaxType == DISTANCETYPE)
          NeighborCounts[0][t] += CountNeighborsByDistance(vTractDistances, MaxCircleSize);
        else
          SSGenerateException("Unknown spatial maximum type.", "MakeNeighbors()" );

        if (SortedInt)
           {
           SortedInt[0][t] = (tract_t*)Smalloc(NeighborCounts[0][t] * sizeof(tract_t), pPrintDirection);
           /* copy tract numbers */
           for (j = NeighborCounts[0][t]-1; j >= 0; j--)   
             SortedInt[0][t][j] = vTractDistances[j].GetTractNumber();
           }
        else
           {
           SortedUShort[0][t] = (unsigned short*)Smalloc(NeighborCounts[0][t] * sizeof(unsigned short), pPrintDirection);
           /* copy tract numbers */
           for (j = NeighborCounts[0][t]-1; j >= 0; j--)
             SortedUShort[0][t][j] = vTractDistances[j].GetTractNumber();
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
                for(k=0; k<NumTracts;k++)
                   {
                   pTInfo->tiGetCoords2(k, pCoords2);
                   //Xold=pCoords2[0];
                   //Yold=pCoords2[1];
                   Transform(pCoords2[0],pCoords2[1],EllipseAngle,pdEShapes[es],&pNewXCoord[k],&pNewYCoord[k]);      // pointers????????
                   }
                for (t = 0; t < NumGridTracts; t++)  // for each grid tract, ...
                   {
                   pGInfo->giGetCoords2(t, pCoords);
                  // Xold=pCoords[0];
                  // Yold=pCoords[1];
                   Transform(pCoords[0],pCoords[1],EllipseAngle,pdEShapes[es],&pCoords[0],&pCoords[1]);

                   for (k=0; k < NumTracts; k++)  // find distances
                      {
                      vTractDistances[k].SetTractNumber(k);
                      pCoords2[0] = pNewXCoord[k];
                      pCoords2[1] = pNewYCoord[k];
                      vTractDistances[k].SetDistance(pTInfo->tiGetDistanceSq(pCoords, pCoords2));
                      }
                   std::sort(vTractDistances.begin(), vTractDistances.end(), CompareTractDistance());
                   if (iSpatialMaxType == PERCENTAGEOFMEASURETYPE)
                     NeighborCounts[lCurrentEllipse][t] += CountNeighborsByMeasure(vTractDistances, Measure, MaxCircleSize, nMaxMeasure);
                   else if (iSpatialMaxType == DISTANCETYPE)
                     NeighborCounts[lCurrentEllipse][t] += CountNeighborsByDistance(vTractDistances, MaxCircleSize);
                   else
                     SSGenerateException("Unknown spatial maximum type.", "MakeNeighbors()" );

                   if (SortedInt)
                      {
                      SortedInt[lCurrentEllipse][t] = (tract_t*)Smalloc(NeighborCounts[lCurrentEllipse][t] * sizeof(tract_t), pPrintDirection);
                      /* copy tract numbers */
                      for (j = NeighborCounts[lCurrentEllipse][t]-1; j >= 0; j--)
                         SortedInt[lCurrentEllipse][t][j] = vTractDistances[j].GetTractNumber();
                      }
                   else
                      {
                      SortedUShort[lCurrentEllipse][t] = (unsigned short*)Smalloc(NeighborCounts[lCurrentEllipse][t] * sizeof(unsigned short), pPrintDirection);
                      /* copy tract numbers */
                      for (j = NeighborCounts[lCurrentEllipse][t]-1; j >= 0; j--)
                         SortedUShort[lCurrentEllipse][t][j] = vTractDistances[j].GetTractNumber();
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

     //temporary print function to view contents of sorted array
     // PrintNeighbors((iNumEllipses + 1), NumTracts, Sorted);
     }
   catch (SSException & x)
      {
      delete [] pNewXCoord;
      delete [] pNewYCoord;
      free(pCoords);
      free(pCoords2);
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
