#include "SaTScan.h"
#pragma hdrstop
#include "GridTractCoordinates.h"

/**********************************************************************
 file: GridTractCoordinates.cpp
 This file abstracts data on a special grid as it is read from a file.

 DATA REPRESENTATION (internal):
 Each special grid geographic tract is represented by a "struct gnode" in an
 array, sorted by increasing grid tract-id.
 The gnode records the grid tract-id and coordinates.
 **********************************************************************/

/* ==========================Grid  Tract Routines =========================== */
GInfo::GInfo() {
  Init();
}

GInfo::~GInfo() {
   Free();
}

void GInfo::Free() {
   giCleanup();
}

/**********************************************************************
 Insert a grid tract into the vector.
 Parameters:
   gid   - grid tract ID
   x, y  - grid tract coordinates
**********************************************************************/

void GInfo::giInsertGnode(const char *gid, std::vector<double>& vCoordinates) {
  tract_t      tGridPoint, t = NumGridTracts - 1;
  int          i;

  try {
    //validate that passed coordinates have same dimensions as class has defined
    if (vCoordinates.size() != (unsigned int)nDimensions)
      ZdGenerateException("Passed coordinates have %u dimensions, wanted %i.",
                          "giInsertGnode()", vCoordinates.size(), nDimensions);

    //duplicate coordinates are ignored
    for (tGridPoint=0; tGridPoint < NumGridTracts; ++tGridPoint)
       if (!memcmp(GridTractInfo[tGridPoint].pCoords, &vCoordinates[0], nDimensions * sizeof(double)))
         return;

    /* Grow GridTractInfo, if needed */
    if (NumGridTracts >= gi_length) {
      gi_length += TI_GROW;
      GridTractInfo = (gnode*)Srealloc(GridTractInfo, gi_length * sizeof(struct gnode));
    }

    /* Find insertion point */
    while (t >= 0 && strcmp(gid, GridTractInfo[t].gid) < 0) {
         GridTractInfo[t + 1] = GridTractInfo[t];
         --t;
    }

    //A duplicate location identifier should never occur. Code before function call should ensure this.
    if (t >= 0 && !strcmp(gid, GridTractInfo[t].gid))
      ZdGenerateException("Duplicate tract identifier specified \"%s\".", "giInsertGnode()", gid);

    t++;
    GridTractInfo[t].gid = (char*) malloc(strlen(gid)+1);
    strcpy(GridTractInfo[t].gid, gid);
    GridTractInfo[t].pCoords = (double*)Smalloc(nDimensions * sizeof(double));
    for (i=0; i<nDimensions; i++)
       GridTractInfo[t].pCoords[i] = vCoordinates[i];
    NumGridTracts++;
  }
  catch (ZdException &x) {
    x.AddCallpath("giInsertGnode()", "GInfo");
    throw;
  }
}

/**********************************************************************
 Returns the number of tracts observed
 **********************************************************************/
int GInfo::giGetNumDimensions() const
{
   return nDimensions;
}
/**********************************************************************
 Returns the number of tracts observed
 **********************************************************************/
tract_t GInfo::giGetNumTracts(void)
{
   return(NumGridTracts);
} /* giGetNumTracts() */

/**********************************************************************
 Searches GridTractInfo for tract-id "gid".  Returns the index,
 or -1 if not found.
 Uses binary search.
 **********************************************************************/
tract_t GInfo::giGetTractNum(const char *gid)
{
   tract_t a = 0;                               /* start of array to search */
   tract_t b = NumGridTracts - 1;                 /* end of array to search */
   tract_t m;                                                   /* midpoint */

   try
      {
      while (b - a > 1)
         {
         m = (a + b) / 2;
         if (strcmp(gid, GridTractInfo[m].gid) < 0)
            b = m - 1;
         else
            a = m;
         }

      if (!strcmp(gid, GridTractInfo[a].gid))
         return(a);
      if (!strcmp(gid, GridTractInfo[b].gid))
         return(b);
      }
   catch (ZdException & x)
      {
      x.AddCallpath("giGetTractNum()", "GInfo");
      throw;
      }
  return(-1);
} /* giGetTractNum() */

/**********************************************************************
 Sets the number of dimensions in Ginfo.
 **********************************************************************/
void GInfo::giSetDimensions(int nDim)
{
   nDimensions = nDim;
}


/**********************************************************************
 Returns the tract coords for the given tract_t index.  The allocation
 of pCoords MUST be deallocated in the calling function.
 **********************************************************************/
void GInfo::giGetCoords(tract_t t, double** pCoord) const
{
   int i;

   try
      {
      //pCoords = (float**)Smalloc(nDimensions * sizeof(float*));
      if (0 <= t && t < NumGridTracts)
         {
         //*x = GridTractInfo[t].x;
         //*y = GridTractInfo[t].y;
         //*z = GridTractInfo[t].z;
         *pCoord = (double*)Smalloc(nDimensions * sizeof(double));
         for (i=0; i<nDimensions; i++)
            {
    	    (*pCoord)[i] = GridTractInfo[t].pCoords[i];
            }
         }
      }
   catch (ZdException & x)
      {
      x.AddCallpath("giGetCoords()", "GInfo");
      throw;
      }
} /* giGetCoords() */

/**
Get the tract coords for the given tract_t index.
*/
void GInfo::giRetrieveCoords(tract_t t, std::vector<double> & vRepository) const {
  if ((t < 0) || (t >= NumGridTracts))
    ZdException::Generate("index, %d, is out of bounds: [0, %d].", "TractHandler", t, NumGridTracts - 1);
  vRepository.resize(nDimensions);
  std::copy(GridTractInfo[t].pCoords, GridTractInfo[t].pCoords + nDimensions, vRepository.begin());
}

/**********************************************************************
 Returns the tract coords for the given tract_t index.  The allocation
 of pCoords is made in the calling function.
 **********************************************************************/
void GInfo::giGetCoords2(tract_t t, double* pCoord) const
{
   int i;

   try
      {
      if (0 <= t && t < NumGridTracts)
         {
         for (i=0; i<nDimensions; i++)
    	    pCoord[i] = GridTractInfo[t].pCoords[i];
         }
      }
   catch (ZdException & x)
      {
      x.AddCallpath("giGetCoords2()", "GInfo");
      throw;
      }
} /* giGetCoords2() */

/* =========================== Display Routines ============================ */

/**********************************************************************
 Display tract info - x and y coordinates.
 **********************************************************************/
void GInfo::giDisplayGridTractInfo(FILE* pDisplay)
{
   int i;

   try
      {
      fprintf(pDisplay, "GID        x           y\n");
      for (i=0; i<NumGridTracts; i++)
         {
        fprintf(pDisplay, "%s   %f   %f\n", GridTractInfo[i].gid, GridTractInfo[i].pCoords[0], GridTractInfo[i].pCoords[1]);
        }
      fprintf(pDisplay, "\n");
      }
   catch (ZdException & x)
      {
      x.AddCallpath("giDisplayGridTractInfo()", "GInfo");
      throw;
      }
} /* giDisplayTractInfo */


/**********************************************************************
 Cleanup all tract storage.
 **********************************************************************/
void GInfo::giCleanup()
{
  int i;
  //int j;

  if (GridTractInfo)
     {
     for (i=0; i<NumGridTracts; i++)
        {
        free(GridTractInfo[i].gid);
        free(GridTractInfo[i].pCoords);
        }
     free(GridTractInfo);
     }
} /*giCleanup */


/**********************************************************************
Look for grid tract with identical coordinates.  Copied from tinfo
version, 7/6/98, G. Gherman
 **********************************************************************/
bool GInfo::giFindDuplicateCoords(FILE* pDisplay) {
   bool bFirstTime = true;
   bool bFoundDuplicates = false;
   double* pCoords = 0;
   double* pCoords2 = 0;
   tract_t i=0;
   tract_t j;
   int nDims;							// Counter for dimensions

   try {
     while (i<NumGridTracts)
        {
        giGetCoords(i, &pCoords);
        j = i+1;

        while ((j<NumGridTracts) && !bFoundDuplicates)
          {
          giGetCoords(j, &pCoords2);
          nDims = 0;
          while (nDims < nDimensions && (pCoords[nDims] == pCoords2[nDims]))
             {
             nDims++;
             }
          if (nDims == nDimensions)
             bFoundDuplicates=true;
          j++;

          free(pCoords2);
          }

        if (bFoundDuplicates && bFirstTime)
           {
           bFirstTime = false;
           fprintf(pDisplay, "  Error: Duplicate coordinates found for grid tracts %s and %s Coords=(",
             GridTractInfo[i].gid, GridTractInfo[j].gid);
           for (nDims=0; nDims<(nDimensions-1); nDims++)
           {
              fprintf(pDisplay, "%.0f, ", pCoords[nDims]);
            }
           fprintf(pDisplay, "%.0f).\n\n", pCoords[nDimensions-1]);
           }
        free(pCoords);
        i++;
        }

      }
   catch (ZdException & x)
      {
      if (pCoords)  free(pCoords);
      if (pCoords2)  free(pCoords2);
      x.AddCallpath("giFindDuplicateCoords(File *)", "GInfo");
      throw;
      }
  //return (bStop);
  return false;
}

void GInfo::Init() {
 GridTractInfo = 0;
 gi_length     = 0;
 NumGridTracts = 0;
 nDimensions   = 0;
}

