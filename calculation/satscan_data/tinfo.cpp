/**********************************************************************
 file: tinfo.c
 This file abstracts data as it is read from a file:  separate geographic
 tracts, each containing multiple populations (different categories).
 Each population cell has a population and a case count.

 DATA REPRESENTATION (internal):
 Each geographic tract is represented by a "struct tnode" in an array,
 sorted by increasing tract-id.
 The tnode records the tract-id, coordinates, and the head of a linked list
 of category records for that tract.

 Each category record ("struct cnode") contains the category number, the
 population, and the case count for that node.  They are stored as linked
 lists within each tract, on the theory that there will not be many
 categories in a given analysis.
 **********************************************************************/
// Changes made 2/98 - K.Rand
//   1) Calls to salloc routines cast to appropriate types.

#include <string.h>
#include <stdlib.h>
#include "spatscan.h"
#include "tinfo.h"
#include "basic.h"
#include "sort.h"
#include "analysis.h"
#include "error.h"
#include "cats.h"

struct cnode {           /* record for one pop. category in a single tract */
  int           cat;        /* category number   */
  long*         pPopList;   /* population vector */
  count_t       count;      /* case count        */
  struct cnode* next;       /* list link         */
};

struct tnode {              /* tract record              */
   char*         tid;       /* tract id string           */
   /*float         x, y, z;   /* coordinates               */
   float*				 pCoords;		/* coordinates               */
   struct cnode* cats;      /* pops & counts by category */
};

static struct tnode* TractInfo     = 0;  /* tract info vector                */
static tract_t       ti_length     = 0;  /* allocated length of TractInfo    */
static tract_t       NumTracts     = 0;  /* number of tracts filled          */

static Julian*       pPopDates     = 0;  /* Pop dates vector                 */
static int           nPopDates     = 0;  /* Number of population years       */
static BOOL          bStartAsPopDt = FALSE;
static BOOL          bEndAsPopDt   = FALSE;
static int 				nDimensions = 0;

/* ============================ Tract Routines ============================= */

/**********************************************************************
 Insert a tract into the vector.
 Parameters:
   tid   - tract ID
   x, y, z  - tract coordinates
 Return value:
   0 = duplicate tract ID
   1 = success
 **********************************************************************/
int tiInsertTnode(char *tid, float* pCoords)
{
  tract_t t = NumTracts - 1;
  int i;

   /* Grow TractInfo, if needed */
  if (NumTracts >= ti_length)
  {
    ti_length += TI_GROW;
    TractInfo = (tnode*)Srealloc(TractInfo, ti_length * sizeof(struct tnode));
  }

  /* Find insertion point */
  while (t >= 0 && strcmp(tid, TractInfo[t].tid) < 0)
  {
    TractInfo[t + 1] = TractInfo[t];
    --t;
  }

  /* if duplicate */
  if (t >= 0 && !strcmp(tid, TractInfo[t].tid))
    return 0;

  t++;

  TractInfo[t].tid = (char*) malloc(strlen(tid)+1);
  strcpy(TractInfo[t].tid, tid);

  /*   TractInfo[t].tid  = Sstrdup(tid); */
  /*   TractInfo[t].x    = (float)x;
  TractInfo[t].y    = (float)y;
  TractInfo[t].z    = (float)z;  */
  TractInfo[t].pCoords = (float*)malloc(nDimensions * sizeof(float));
  for (i=0; i < nDimensions; i++)
  {
  	TractInfo[t].pCoords[i] =  pCoords[i];
  }
  TractInfo[t].cats = 0;

  NumTracts++;

  return(1);
} /* tiInsertTnode() */

/**********************************************************************
 Returns the number of tracts observed
 **********************************************************************/
tract_t tiGetNumTracts(void)
{
   return(NumTracts);
} /* tiGetNumTracts() */

/**********************************************************************
 Searches TractInfo for tract-id "tid".  Returns the index,
 or -1 if not found.
 Uses binary search.
 **********************************************************************/
tract_t tiGetTractNum(char *tid)
{
  tract_t a = 0;                              /* start of array to search */
  tract_t b = NumTracts - 1;                    /* end of array to search */
  tract_t m;                                                  /* midpoint */

  while (b - a > 1)
  {
    m = (a + b) / 2;
    if (strcmp(tid, TractInfo[m].tid) < 0)
       b = m - 1;
    else
       a = m;
  }

  if (!strcmp(tid, TractInfo[a].tid))
    return(a);
  if (!strcmp(tid, TractInfo[b].tid))
    return(b);

  return(-1);
} /* tiGetTractNum() */


/**********************************************************************
 Returns the tract name (tid) for the given tract_t index.
 **********************************************************************/
char *tiGetTid(tract_t t)
{
  if (0 <= t && t < NumTracts)
    return(TractInfo[t].tid);
  else
    return(0);
} /* tiGetTid() */

/**********************************************************************
 Returns the tract coords for the given tract_t index.
 **********************************************************************/
void tiGetCoords(tract_t t, float** pCoords)
{
	*pCoords = (float*)Smalloc(nDimensions * sizeof(float));
	int i;

  if (0 <= t && t < NumTracts)
  {
    //  *x = TractInfo[t].x;
    //  *y = TractInfo[t].y;
    //  *z = TractInfo[t].z;
    for (i=0; i < nDimensions; i++)
    {
			(*pCoords)[i] = TractInfo[t].pCoords[i];
    }
  }
} /* tiGetCoords() */


/* =========================== Category Routines =========================== */

/**********************************************************************
 Adds a category to the tract info structure.
 **********************************************************************/
void tiAddCat(tract_t t, int cat, Julian popDate, long pop)
{
  int i;
  //int popIndex;

  struct cnode* node = TractInfo[t].cats;

  while (node && node->cat != cat)
    node = node->next;
  if (node)
  {
    tiAssignPop(&node->pPopList, popDate, pop);
  }
  else
  {
    node = (cnode*)Smalloc(sizeof(struct cnode));
    node->cat = cat;
    node->count = 0;

    node->pPopList = (long*)Smalloc(nPopDates * sizeof(long));
    for (i=0; i<nPopDates; i++)
    {
      node->pPopList[i] = 0L;
    }

    tiAssignPop(&node->pPopList, popDate, pop);

    node->next = TractInfo[t].cats;
    TractInfo[t].cats = node;
  }
} /* tiAddCat() */


/* ========================== Population Routines ========================== */

/**********************************************************************
 Initializes the Population dates vector.
 **********************************************************************/
void tiSetupPopDates(Julian* pDates, int nDates, Julian StartDate, Julian EndDate)
{
  //char c;
  int  n;
  int  nSourceOffset;
  int  nDestOffset;
  int  nDatesUsed;
  //struct cnode* cats;
  //char szDate[MAX_DT_STR];

  Sort(&pDates, 0, nDates-1);

  tiFindPopDatesToUse(pDates, nDates, StartDate, EndDate,
                      &nSourceOffset, &nDestOffset, &bStartAsPopDt, &bEndAsPopDt,
                      &nDatesUsed, &nPopDates);

  pPopDates = (unsigned long*)Smalloc(nPopDates * sizeof(Julian));

  if (bStartAsPopDt)
    pPopDates[0] = StartDate;

  for (n=0; n<nDatesUsed; n++)
    pPopDates[n + nDestOffset] = pDates[n + nSourceOffset];

  if (bEndAsPopDt)
    pPopDates[nPopDates-1] = EndDate+1;

/* debug */
#if 0
  DisplayDatesArray(pDates, nDates, "Array of Potential Pop Dates", stdout);

  JulianToChar(szDate, StartDate);
  printf("\nStart Date = %s\n", szDate);
  JulianToChar(szDate, EndDate);
  printf("End Date   = %s\n", szDate);

  printf("\nSource offset      = %i\n", nSourceOffset);
  printf("Destination offset = %i\n", nDestOffset);
  printf("Add Start          = %i\n", bStartAsPopDt);
  printf("Add End            = %i\n", bEndAsPopDt);
  printf("Number Dates Used  = %i\n", nDatesUsed);
  printf("Number Pop Dates   = %i\n", nPopDates);
  printf("\n");

  printf("<Press any key to continue>");
  c = getc(stdin);

  DisplayDatesArray(pPopDates, nPopDates, "Array of Selected Pop Dates", stdout);
#endif

} /* tiSetupPopDates */

/**********************************************************************
 Determines which available population years should be used.
 **********************************************************************/
void tiFindPopDatesToUse(Julian* pDates, int nDates, Julian StartDate, Julian EndDate,
                         int* pnSourceOffset, int* pnDestOffset, BOOL* pbAddStart, BOOL* pbAddEnd,
                         int* pnDatesUsed, int* pnPopDates)
{
  int    n;
  int    nLastIndex;
  BOOL   bStartFound = FALSE;
  BOOL   bEndFound   = FALSE;

  *pnSourceOffset = 0;
  *pnDestOffset   = 0;
  *pbAddStart     = FALSE;
  *pbAddEnd       = FALSE;

  /* Determine which pop dates to use */
  for (n=0; n<nDates; n++)
  {
    if (!bStartFound)
    {
      if (pDates[n] > StartDate)
      {
        bStartFound = TRUE;
        if (n==0)
        {
          *pbAddStart = TRUE;
          *pnDestOffset = 1;
        }
      }
      else
        *pnSourceOffset = n;
    }

    if (!bEndFound)
    {
      nLastIndex = n;
      if (pDates[n] > EndDate)
        bEndFound = TRUE;
    }
  }

  *pnDatesUsed = nLastIndex - *pnSourceOffset + 1;
  *pnPopDates  = *pnDatesUsed;

  if (*pbAddStart)
    *pnPopDates = *pnPopDates+1;

  if (!bEndFound && nLastIndex==(nDates-1))
  {
    *pbAddEnd = TRUE;
    *pnPopDates = *pnPopDates+1;
  }
} /* tiFindPopDatesToUse */

/******************************************************************************
 Assigns a population count to the appropriate location in the population list.
 ******************************************************************************/
int tiAssignPop(long** pPopList, Julian nPopDate, long nPop)
{
  int nPopIndex;

  nPopIndex = tiGetPopDateIndex(nPopDate);
  if (nPopIndex == -1)
  {
/*  printf("Population Year is not in the index.\n"); */
    return(-1);
  }

  (*pPopList)[nPopIndex] += nPop;

  if ((nPopIndex == 1) && (bStartAsPopDt))
    (*pPopList)[0] += nPop;

  if ((nPopIndex == nPopDates-2) && (bEndAsPopDt))
    (*pPopList)[nPopDates-1] += nPop;

  return(0);
} /* tiAssignPop */

/**********************************************************************
 Returns the population for a given year and category in a given tract
 (0 if the cell does not exist).
 **********************************************************************/
unsigned long tiGetPop(tract_t t, int cat, int nPopIndex)
{
  struct cnode *node = TractInfo[t].cats;

  while (node && node->cat != cat)
    node = node->next;

  if (node && nPopIndex>=0 && nPopIndex<nPopDates)
    return(node->pPopList[nPopIndex]);
  else
    return(0);
} /* tiGetPop() */

/**********************************************************************
 Returns the population date for a given index into the Pop date array.
***********************************************************************/
Julian tiGetPopDate(int nIndex)
{
  if (!(nIndex >= 0 && nIndex < nPopDates))
    return(-1);

  return(pPopDates[nIndex]);
} /* tiGetPopDate */

/**********************************************************************
 Returns the number of dates in the population date array.
***********************************************************************/
int tiGetNumPopDates()
{
  return(nPopDates);
} /* tiGetNumPopDates */

/**********************************************************************
 Returns the index into the Pop date array for a given date.
***********************************************************************/
int tiGetPopDateIndex(Julian nDate)
{
  int  retVal;
  int i = 0;
  BOOL bDateFound = FALSE;

  while (i < nPopDates && !bDateFound)
  {
     bDateFound = (nDate == pPopDates[i]);
     i++;
  }

  if (bDateFound)
    retVal = i-1;
  else
    retVal = -1;

  return (retVal);
} /* tiGetPopDateIndex */

/*************************************************************************
 Returns the indeces to population dates that bound a given interval date.
**************************************************************************/
int tiGetPopUpLowIndex(Julian* pDates, int nDateIndex, int nMaxDateIndex, int* nUpIndex, int* nLowIndex)
{
  int i, index;
  BOOL bUpFound = FALSE;

/*  if (nDateIndex == nMaxDateIndex)
    return(0);
*/
  for (i=0; i<nPopDates; i++)
  {
    if (pPopDates[i] <= pDates[nDateIndex])
      *nLowIndex = i;

    if (nDateIndex == nMaxDateIndex)
      index = nDateIndex;
    else
      index = nDateIndex+1;

    if (!bUpFound && (pPopDates[i] >= pDates[index]))
    {
      bUpFound = TRUE;
      *nUpIndex = i;
    }
  }
  return(1);
} /* tiGetPopUpLowIndex */

/*************************************************************************
 Calculates....
 These calculations assumes that the population of a given day refers to
 the beginning of that day.
**************************************************************************/
void tiCalcAlpha(double** pAlpha, Julian StartDate, Julian EndDate)
{
  int  n;
  int  N           = nPopDates-2;
  long nTotalYears = TimeBetween(StartDate, EndDate, DAY)/*EndDate-StartDate*/ ;
  double sumalpha;
  //char szDate[MAX_DT_STR];

  *pAlpha = (double*)Smalloc((nPopDates+1) * sizeof(double));

  if(N==0) {
     (*pAlpha)[0] = 0.5*((pPopDates[1]-StartDate)+(pPopDates[1]-(EndDate+1)))/(double)(pPopDates[1]-pPopDates[0]);
     (*pAlpha)[1] = 0.5*((StartDate-pPopDates[0])+((EndDate+1)-pPopDates[0]))/(double)(pPopDates[1]-pPopDates[0]);
     }
  else if(N==1) {
     (*pAlpha)[0] = (0.5*((double)(pPopDates[1]-StartDate)/(pPopDates[1]-pPopDates[0]))*(pPopDates[1]-StartDate)) / (double)nTotalYears;
     (*pAlpha)[1] = (0.5*(pPopDates[1]-StartDate)*(1+((double)(StartDate-pPopDates[0])/(pPopDates[1]-pPopDates[0])))) / (double)nTotalYears
                  + (0.5*(double)(EndDate+1-pPopDates[N])*(1+((double)(pPopDates[N+1]-(EndDate+1))/(pPopDates[N+1]-pPopDates[N])))) /  (double)nTotalYears;
     (*pAlpha)[N+1] = (0.5*((double)(EndDate+1-pPopDates[N])/(pPopDates[N+1]-pPopDates[N]))*(EndDate+1-pPopDates[N])) / nTotalYears;
     }
  else {
     (*pAlpha)[0] = (0.5*((double)(pPopDates[1]-StartDate)/(pPopDates[1]-pPopDates[0]))*(pPopDates[1]-StartDate)) / (double)nTotalYears;
     (*pAlpha)[1] = (0.5*(pPopDates[2]-pPopDates[1]) + 0.5*(pPopDates[1]-StartDate)*(1+((double)(StartDate-pPopDates[0])/(pPopDates[1]-pPopDates[0])))) / (double)nTotalYears;
     for (n = 2; n < N; n++) {
       (*pAlpha)[n] = 0.5*(double)(pPopDates[n+1] - pPopDates[n-1]) / (double)nTotalYears;
       }
     (*pAlpha)[N]   = (0.5*(pPopDates[N]-pPopDates[N-1]) + 0.5*(double)(EndDate+1-pPopDates[N])*(1+((double)(pPopDates[N+1]-(EndDate+1))/(pPopDates[N+1]-pPopDates[N])))) /  (double)nTotalYears;
     (*pAlpha)[N+1] = (0.5*((double)(EndDate+1-pPopDates[N])/(pPopDates[N+1]-pPopDates[N]))*(EndDate+1-pPopDates[N])) / nTotalYears;
     }

#if 0 /* DEBUG */
printf("\nTotal years = %ld, N=%ld\n", nTotalYears, N);
  printf("Pop\nIndex   PopDates        Alpha\n");
  for (n=0;n<=N+1;n++) {
    JulianToChar(szDate,pPopDates[n]);
    printf("%i       %s        %f\n",n, szDate, (*pAlpha)[n]);
    }
  printf("\n");
#endif


/* Bug check, seeing that alpha values add to one. */
  sumalpha = 0;
  for (n = 0; n <= N+1; n++) sumalpha = sumalpha + (*pAlpha)[n];
  if(sumalpha>1.0001 || sumalpha<0.9999) {
     printf("\n\n  Error: Alpha values not calculated correctly in");
     printf("\n  file tinfo.c, function tiCalcAlpha. The sum of the ");
     printf("\n  alpha values is %8.6lf rather than 1.\n", sumalpha);

     FatalError("Program cancelled.\n");
     }

} /* tiCalcAlpha */

/* ========================== Case Count Routines ========================== */

/**********************************************************************
 Sets the case count for a given cell.
 Return value:
   1 = OK
   0 = cell does not exist; not added
 **********************************************************************/
int tiSetCount(tract_t t, int cat, count_t count)
{
  struct cnode *node = TractInfo[t].cats;

  while (node && node->cat != cat)
    node = node->next;

  if (node)
  {
    node->count = count;
    return(1);
  }
  else
    return(0);
} /* tiSetCount() */

/**********************************************************************
 Returns the case count for a given category in a given tract
 (0 if the cell does not exist).
 **********************************************************************/
count_t tiGetCount(tract_t t, int cat)
{
  struct cnode *node = TractInfo[t].cats;

  while (node && node->cat != cat)
     node = node->next;

  if (node)
    return node->count;
  else
    return 0;
} /* tiGetCount() */


/* =========================== Display Routines ============================ */

/**********************************************************************
 Display an array of Julian dates.
 **********************************************************************/
void DisplayDatesArray(Julian* pDates, int nDates, char* szTitle, FILE* pDisplay)
{
  int n;
  char c;
  char szDate[MAX_DT_STR];

  fprintf(pDisplay, "\n%s\n", szTitle);
  for (n=0; n<nDates; n++)
  {
    JulianToChar(szDate, pDates[n]);
    fprintf(pDisplay, "%i  %s\n", n, szDate);
  }

  if (pDisplay == stdout || pDisplay == stderr)
  {
    fprintf(pDisplay, "<Press any key to continue>\n");
    c = getc(stdin);
  }
  else
    fprintf(pDisplay, "\n");

}

/**********************************************************************
 Display the array of Population dates.
 **********************************************************************/
void DisplayPopDatesArray(FILE* pDisplay)
{
  int n;
  char c;
  char szDate[MAX_DT_STR];

  fprintf(pDisplay, "\nPopulation Dates\n");
  for (n=0; n<nPopDates; n++)
  {
    JulianToChar(szDate, pPopDates[n]);
    fprintf(pDisplay, "%i  %s\n", n, szDate);
  }

  if (pDisplay == stdout || pDisplay == stderr)
  {
    fprintf(pDisplay, "<Press any key to continue>\n");
    c = getc(stdin);
  }
  else
    fprintf(pDisplay, "\n");

}

/**********************************************************************
 Display tract info - x, y, z coordinates.
 **********************************************************************/
void DisplayTractInfo()
{
  //int i;

  printf("TID        x           y          z\n");
  // Temporarily disable this
  //for (i=0; i<NumTracts; i++)
  //{
  //   printf("%s   %f   %f   %f\n", TractInfo[i].tid, TractInfo[i].x,
  //                                 TractInfo[i].y, TractInfo[i].z);
  //}
  printf("\n");
} /* DisplayTractInfo */

/**********************************************************************
 Loop through and display all tract info - categories, case counts and
 populations.
 **********************************************************************/
void DisplayAllTractInfo(FILE* pDisplay)
{
  int i, j;
  struct cnode* cats;
  char c;

  fprintf(pDisplay, "\nTID   Cat Count   Populations\n");

  for (i=0; i<NumTracts; i++)
  {
    cats = TractInfo[i].cats;
    while (cats)
    {
      fprintf(pDisplay, "%s   %d   %i       ", TractInfo[i].tid, cats->cat, cats->count);
      for (j=0; j<nPopDates; j++)
      {
         fprintf(pDisplay, "%li ", cats->pPopList[j]);     /* change to Julian */
      }
      cats = cats->next;
      if (pDisplay == stdout)
        c = getc(stdin);
      else
        fprintf(pDisplay, "\n");
    }
  }
  fprintf(pDisplay, "\n");

} /* Display All Info */


/* ============================ Misc. Routines ============================= */

/**********************************************************************
 Cleanup all tract storage.
 **********************************************************************/
void tiCleanup()
{
  struct cnode* pCurrCat;
  struct cnode* pNextCat;

  int i;
  //int j, k;

/*  for (i=0; i<ti_length; i++)*/
  for (i=0; i<NumTracts; i++)
  {
    pCurrCat = TractInfo[i].cats;

    while (pCurrCat != NULL)
    {
      pNextCat = pCurrCat->next;
		free(pCurrCat->pPopList);
      free(pCurrCat);
      pCurrCat = pNextCat;
    }
    free(TractInfo[i].tid);
    free(TractInfo[i].pCoords);
/*    free(TractInfo + i);*/
  }

  free(TractInfo);
  free(pPopDates);


} /*tiCleanup */

/**********************************************************************
 Sort function comparison.
 **********************************************************************/
/*KR-12/9/97int sort_function( const void *a, const void *b)
{
   return( ! strcmp((char *)a,(char *)b) );
} /* sort_function */

/**********************************************************************
Look for tracts with identical x,y,z coordinates.  7/8/97, K. Rand
 **********************************************************************/
BOOL tiFindDuplicateCoords(FILE* pDisplay)
{
  BOOL bStop = FALSE;
  float* pCoords;
  float* pCoords2;
  tract_t i=0;
  tract_t j;
  int nDims;							// Counter for dimensions

  while (!bStop && i<NumTracts)
  {
    tiGetCoords(i, &pCoords);
    j = i+1;

    while (!bStop && j<NumTracts)
    {
      tiGetCoords(j, &pCoords2);
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
    fprintf(pDisplay, "  Error: Duplicate coordinates found for tracts %s and %s Coords=(",
    									TractInfo[i].tid, TractInfo[j].tid);
    for (nDims=0; nDims<(nDimensions-1); nDims++)
    {
    	fprintf(pDisplay, "%.0f, ", pCoords[nDims]);
    }
    fprintf(pDisplay, "%.0f).\n\n", pCoords[nDimensions-1]);
    free(pCoords);
  }
  return (bStop);
}

/**********************************************************************
Look for and display tracts with zero population for any population year.  7/8/97, K. Rand
 **********************************************************************/
void tiReportZeroPops(FILE *pDisplay)
{
  int i, j;
  UInt month, day, year;
  BOOL bZeroFound = FALSE;
  struct cnode* cats;
  long* PopTotalsArray;
  int   nPStartIndex = 0;
  int   nPEndIndex;

  if (bStartAsPopDt)
    nPStartIndex = 1;
  if (bEndAsPopDt)
    nPEndIndex = nPopDates-2;
  else
    nPEndIndex = nPopDates-1;

  PopTotalsArray = (long*)Smalloc(nPopDates *sizeof(long));

  for (i=0; i<NumTracts; i++)
  {
    for (j=nPStartIndex; j<=nPEndIndex; j++)
      PopTotalsArray[j]=0;

    cats = TractInfo[i].cats;
    while (cats)
    {
      for (j=nPStartIndex; j<=nPEndIndex; j++)
        PopTotalsArray[j]+= cats->pPopList[j];
      cats = cats->next;
    }

    for (j=nPStartIndex; j<=nPEndIndex; j++)
    {
      if (PopTotalsArray[j]==0)
      {
        if (!bZeroFound)
        {
          bZeroFound = TRUE;
          fprintf(pDisplay,"\n________________________________________________________________\n\n");
          fprintf(pDisplay,"Warning: According to the input data, the following tracts have a \n");
          fprintf(pDisplay,"         population totaling zero for the specified year(s).\n\n");
        }
        JulianToMDY(&month, &day, &year, pPopDates[j]);
        fprintf(pDisplay,"         Tract %s, %d\n", TractInfo[i].tid, year);
      }
    }

  } /* i=0-<NumTracts */

  free (PopTotalsArray);
}

/*******************************************************************************
Add dimension size (of coordinates) to model
*******************************************************************************/
void tiSetDimensions(int nDim)
{
	nDimensions = nDim;
}

/*******************************************************************************
Compute distance sqaured between 2 tracts
*******************************************************************************/
float tiGetDistanceSq(float* pCoords, float* pCoords2)
{
	int i;
  float sum=0;
  //float *pCoords = (float*)malloc(nDimensions * sizeof(float));   coordinates */
  //float *pCoords2 = (float*)malloc(nDimensions * sizeof(float));   coordinates */

  for (i=0; i<nDimensions; i++)
  {
  	//sum += (TractInfo[t1].pCoords[i] - TractInfo[t2].pCoords[i])*
    //         (TractInfo[t1].pCoords[i] - TractInfo[t2].pCoords[i]);
    sum += (pCoords[i] - pCoords2[i]) * (pCoords[i] - pCoords2[i]);
  }
  return(sum);
}

/*******************************************************************************
Check to see that no cases were found for tracts that have no population
*******************************************************************************/
BOOL tiCheckCasesHavePop()
{
  int i, j;
  //UInt month, day, year;
  BOOL bValid = TRUE;
  struct cnode* cats;
  long  nPopTotal;
  int   nPStartIndex = 0;
  int   nPEndIndex;
  char  szCategories[100];

  if (bStartAsPopDt)
    nPStartIndex = 1;
  if (bEndAsPopDt)
    nPEndIndex = nPopDates-2;
  else
    nPEndIndex = nPopDates-1;

  for (i=0; i<NumTracts; i++)
  {

    cats = TractInfo[i].cats;
    while (cats)
    {
      strcpy(szCategories, "\0");
      nPopTotal   = 0;
      for (j=nPStartIndex; j<=nPEndIndex; j++)
        nPopTotal += cats->pPopList[j];

      if (nPopTotal==0 && cats->count>0)
      {
        bValid = FALSE;
        printf("  Error: %s %s has cases but zero population.\n",
        TractInfo[i].tid, catGetCategoriesString(cats->cat, szCategories));
      }

      cats = cats->next;
    }

  } /* i=0-<NumTracts */

  return bValid;
}

BOOL tiCheckZeroPops(FILE *pDisplay)
/*******************************************************************************
Check to see that no years have a total population of zero.
*******************************************************************************/
{
  int i, j;
  UInt month, day, year;
  BOOL bValid = TRUE;
  struct cnode* cats;
  long* PopTotalsArray;
  int   nPStartIndex = 0;
  int   nPEndIndex;

  if (bStartAsPopDt)
    nPStartIndex = 1;
  if (bEndAsPopDt)
    nPEndIndex = nPopDates-2;
  else
    nPEndIndex = nPopDates-1;

  PopTotalsArray = (long*)Smalloc(nPopDates *sizeof(long));

  for (j=nPStartIndex; j<=nPEndIndex; j++)
    PopTotalsArray[j]=0;

  for (i=0; i<NumTracts; i++)
  {
    cats = TractInfo[i].cats;
    while (cats)
    {
      for (j=nPStartIndex; j<=nPEndIndex; j++)
        PopTotalsArray[j]+= cats->pPopList[j];
      cats = cats->next;
    }
  }

  for (j=nPStartIndex; j<=nPEndIndex; j++)
  {
    if (PopTotalsArray[j]==0)
    {
      bValid = FALSE;
      JulianToMDY(&month, &day, &year, pPopDates[j]);
      fprintf(pDisplay, "  Error: Population of zero found for all tracts in %d.\n", year);
    }
  }

  free (PopTotalsArray);
  return bValid;
}

