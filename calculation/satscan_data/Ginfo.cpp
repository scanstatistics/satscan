/**********************************************************************
 file: ginfo.c
 This file abstracts data on a special grid as it is read from a file.

 DATA REPRESENTATION (internal):
 Each special grid geographic tract is represented by a "struct gnode" in an
 array, sorted by increasing grid tract-id.
 The gnode records the grid tract-id and coordinates.
 **********************************************************************/

#include <string.h>
#include "spatscan.h"
#include "ginfo.h"
#include "basic.h"
#include <stdlib.h>
/*#include "sort.h"*/

struct gnode {              /* grid tract record    */
   char*         gid;       /* grid tract id string */
   float*        pCoords;   /* coordinates          */
};

static struct gnode* GridTractInfo = 0;  /* grid tract info vector           */
static tract_t       gi_length     = 0;  /* allocated len of GridTractInfo   */
static tract_t       NumGridTracts = 0;  /* number of grid tracts filled     */
static int 					 nDimensions = 0;


/* ==========================Grid  Tract Routines =========================== */

/**********************************************************************
 Insert a grid tract into the vector.
 Parameters:
   gid   - grid tract ID
   x, y  - grid tract coordinates
 Return value:
   0 = duplicate tract ID
   1 = success
 **********************************************************************/
int giInsertGnode(char *gid, float* pCoords)
{
  tract_t t = NumGridTracts - 1;
  int i;
  /* Grow GridTractInfo, if needed */
  if (NumGridTracts >= gi_length)
  {
    gi_length += TI_GROW;
    GridTractInfo = (gnode*)Srealloc(GridTractInfo, gi_length * sizeof(struct gnode));
  }

  /* Find insertion point */
  while (t >= 0 && strcmp(gid, GridTractInfo[t].gid) < 0)
  {
    GridTractInfo[t + 1] = GridTractInfo[t];
    --t;
  }

  /* if duplicate */
  if (t >= 0 && !strcmp(gid, GridTractInfo[t].gid))
    return 0;

  t++;

  GridTractInfo[t].gid = (char*) malloc(strlen(gid)+1);
  strcpy(GridTractInfo[t].gid, gid);
  GridTractInfo[t].pCoords = (float*)Smalloc(nDimensions * sizeof(float));
  for (i=0; i<nDimensions; i++)
  {
  	GridTractInfo[t].pCoords[i] = pCoords[i];
  }
  //  GridTractInfo[t].y    = (float)y;
  //  GridTractInfo[t].z    = (float)z;

  NumGridTracts++;

  return(1);
} /* giInsertGnode() */

/**********************************************************************
 Returns the number of tracts observed
 **********************************************************************/
tract_t giGetNumTracts(void)
{
   return(NumGridTracts);
} /* giGetNumTracts() */

/**********************************************************************
 Searches GridTractInfo for tract-id "gid".  Returns the index,
 or -1 if not found.
 Uses binary search.
 **********************************************************************/
tract_t giGetTractNum(char *gid)
{
  tract_t a = 0;                               /* start of array to search */
  tract_t b = NumGridTracts - 1;                 /* end of array to search */
  tract_t m;                                                   /* midpoint */

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

  return(-1);
} /* giGetTractNum() */


/**********************************************************************
 Returns the tract name (gid) for the given tract_t index.
 **********************************************************************/
char* giGetGid(tract_t t)
{
  if (0 <= t && t < NumGridTracts)
    return(GridTractInfo[t].gid);
  else
    return(0);
} /* giGetGid() */

/**********************************************************************
 Sets the number of dimensions in Ginfo.
 **********************************************************************/
void giSetDimensions(int nDim)
{
	nDimensions = nDim;
}


/**********************************************************************
 Returns the tract coords for the given tract_t index.  The allocation
 of pCoords MUST be deallocated in the calling function.
 **********************************************************************/
void giGetCoords(tract_t t, float** pCoord)
{
  int i;
//  pCoords = (float**)Smalloc(nDimensions * sizeof(float*));
  if (0 <= t && t < NumGridTracts)
  {
    //*x = GridTractInfo[t].x;
    //*y = GridTractInfo[t].y;
    //*z = GridTractInfo[t].z;
    *pCoord = (float*)Smalloc(nDimensions * sizeof(float));
    for (i=0; i<nDimensions; i++)
    {
    	(*pCoord)[i] = GridTractInfo[t].pCoords[i];
    }
  }
} /* giGetCoords() */


/* =========================== Display Routines ============================ */

/**********************************************************************
 Display tract info - x and y coordinates.
 **********************************************************************/
void giDisplayGridTractInfo()
{
  int i;

  printf("GID        x           y\n");
  for (i=0; i<NumGridTracts; i++)
  {
     printf("%s   %f   %f\n", GridTractInfo[i].gid, GridTractInfo[i].pCoords[0], GridTractInfo[i].pCoords[1]);
  }
  printf("\n");
} /* giDisplayTractInfo */


/**********************************************************************
 Cleanup all tract storage.
 **********************************************************************/
void giCleanup()
{
  int i;
  //int j;

  for (i=0; i<NumGridTracts; i++)
  {
    free(GridTractInfo[i].gid);
    free(GridTractInfo[i].pCoords);
  }

  free(GridTractInfo);

} /*giCleanup */


/**********************************************************************
Look for grid tract with identical coordinates.  Copied from tinfo
version, 7/6/98, G. Gherman
 **********************************************************************/
BOOL giFindDuplicateCoords(FILE* pDisplay)
{
  BOOL bStop = FALSE;
  float* pCoords;
  float* pCoords2;
  tract_t i=0;
  tract_t j;
  int nDims;							// Counter for dimensions

  while (!bStop && i<NumGridTracts)
  {
    giGetCoords(i, &pCoords);
    j = i+1;

    while (!bStop && j<NumGridTracts)
    {
      giGetCoords(j, &pCoords2);
      nDims = 0;
      while (nDims < nDimensions && (pCoords[nDims] == pCoords2[nDims]))
      {
      	nDims++;
      }
      if (nDims == nDimensions)
      	bStop=TRUE;
      else
      	j++;

      free(pCoords2);
    }

    if (!bStop)
    {
    	free(pCoords);
      i++;
    }
  }

  if (bStop)
  {
    fprintf(pDisplay, "  Error: Duplicate coordinates found for grid tracts %s and %s Coords=(",
    									GridTractInfo[i].gid, GridTractInfo[j].gid);
    for (nDims=0; nDims<(nDimensions-1); nDims++)
    {
    	fprintf(pDisplay, "%.0f, ", pCoords[nDims]);
    }
    fprintf(pDisplay, "%.0f).\n\n", pCoords[nDimensions-1]);
    free(pCoords);
  }
  return (bStop);
}



