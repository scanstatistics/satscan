#include "SaTScan.h"
#pragma hdrstop
#include "Tracts.h"

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

/* ============================ Tract Routines ============================= */

TInfo::TInfo(Cats *pCats, BasePrint *pPrintDirection)
{
   Init();
   gpCats = pCats;
   gpPrintDirection = pPrintDirection;
}
TInfo::~TInfo()
{
   Free();
}
void TInfo::Init()
{
   TractInfo     = 0;
   ti_length     = 0;
   NumTracts     = 0;
   pPopDates     = 0;
   nPopDates     = 0;
   bStartAsPopDt = false;
   bEndAsPopDt   = false;
   nDimensions   = 0;
}
void TInfo::Free()
{
   // DO NOT need to delete gpCats;
   tiCleanup();
}
/**********************************************************************
 Insert a tract into the vector.
 Parameters:
   tid   - tract ID
   x, y, z  - tract coordinates
 Return value:
   0 = duplicate tract ID
   1 = success
 **********************************************************************/
int TInfo::tiInsertTnode(char *tid, double* pCoords)
{
   try
      {
      tract_t t = NumTracts - 1;
      int i;
    
       /* Grow TractInfo, if needed */
      if (NumTracts >= ti_length)
      {
        ti_length += TI_GROW;
        TractInfo = (tnode*)Srealloc(TractInfo, ti_length * sizeof(struct tnode), gpPrintDirection);
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
      /*   TractInfo[t].x    = (double)x;
      TractInfo[t].y    = (double)y;
      TractInfo[t].z    = (double)z;  */
      TractInfo[t].pCoords = (double*)malloc(nDimensions * sizeof(double));
      for (i=0; i < nDimensions; i++)
      {
      	TractInfo[t].pCoords[i] =  pCoords[i];
      }
      TractInfo[t].cats = 0;
    
      NumTracts++;
      }
   catch (SSException & x)
      {
      x.AddCallpath("tiInsertTnode()", "TInfo");
      throw;
      }
   return(1);
} /* tiInsertTnode() */

/**********************************************************************
 Returns the number of tracts observed
 **********************************************************************/
tract_t TInfo::tiGetNumTracts(void) const
{
   return(NumTracts);
} /* tiGetNumTracts() */

/**********************************************************************
 Searches TractInfo for tract-id "tid".  Returns the index,
 or -1 if not found.
 Uses binary search.
 **********************************************************************/
tract_t TInfo::tiGetTractNum(char *tid)
{
   tract_t a = 0;                              /* start of array to search */
   tract_t b = NumTracts - 1;                    /* end of array to search */
   tract_t m;                                                  /* midpoint */

   try
      {
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
      }
   catch (SSException & x)
      {
      x.AddCallpath("tiGetTractNum()", "TInfo");
      throw;
      }
  return(-1);
} /* tiGetTractNum() */


/**********************************************************************
 Returns the tract name (tid) for the given tract_t index.
 **********************************************************************/
char *TInfo::tiGetTid(tract_t t) const
{
   char *Tid = 0;

   try
     {
     if (0 <= t && t < NumTracts)
        Tid = (TractInfo[t].tid);
     }
   catch (SSException & x)
      {
      x.AddCallpath("tiGetTractNum()", "TInfo");
      throw;
      }
   return Tid;
} /* tiGetTid() */

/**********************************************************************
 Returns the tract coords for the given tract_t index.
 **********************************************************************/
void TInfo::tiGetCoords(tract_t t, double** pCoords) const
{
   try
      {
      *pCoords = (double*)Smalloc(nDimensions * sizeof(double), gpPrintDirection);             // DTG nDimensions is a global value... Rename as "gnDimensions"
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
      }
   catch (SSException & x)
      {
      x.AddCallpath("tiGetCoords()", "TInfo");
      throw;
      }
} /* tiGetCoords() */

/**********************************************************************
 Returns the tract coords for the given tract_t index.
 **********************************************************************/
void TInfo::tiGetCoords2(tract_t t, double* pCoords) const
{
   int i;

   try
      {
      if (0 <= t && t < NumTracts)
         {
         for (i=0; i < nDimensions; i++)
            pCoords[i] = TractInfo[t].pCoords[i];
         }
      }
   catch (SSException & x)
      {
      x.AddCallpath("tiGetCoords2()", "TInfo");
      throw;
      }
} /* tiGetCoords2() */

/* =========================== Category Routines =========================== */

/**********************************************************************
 Adds a category to the tract info structure.
 **********************************************************************/
void TInfo::tiAddCat(tract_t t, int cat, Julian popDate, long pop)
{
   int i;
   //int popIndex;

   try
      {
      struct cnode* node = TractInfo[t].cats;
    
      while (node && node->cat != cat)
        node = node->next;
      if (node)
        {
        tiAssignPop(&node->pPopList, popDate, pop);
        }
      else
        {
        node = (cnode*)Smalloc(sizeof(struct cnode), gpPrintDirection);
        node->cat = cat;
        node->count = 0;
    
        node->pPopList = (long*)Smalloc(nPopDates * sizeof(long), gpPrintDirection);
        for (i=0; i<nPopDates; i++)
        {
          node->pPopList[i] = 0L;
        }
    
        tiAssignPop(&node->pPopList, popDate, pop);
    
        node->next = TractInfo[t].cats;
        TractInfo[t].cats = node;
        }
      }
   catch (SSException & x)
      {
      x.AddCallpath("tiAddCat()", "TInfo");
      throw;
      }
} /* tiAddCat() */


/* ========================== Population Routines ========================== */

/**********************************************************************
 Initializes the Population dates vector.
 **********************************************************************/
void TInfo::tiSetupPopDates(Julian* pDates, int nDates, Julian StartDate, Julian EndDate)
{
   //char c;
   int  n;
   int  nSourceOffset;
   int  nDestOffset;
   int  nDatesUsed;
   //struct cnode* cats;
   //char szDate[MAX_DT_STR];

   try
      {
      Sort(&pDates, 0, nDates-1);
    
      tiFindPopDatesToUse(pDates, nDates, StartDate, EndDate,
                          &nSourceOffset, &nDestOffset, &bStartAsPopDt, &bEndAsPopDt,
                          &nDatesUsed, &nPopDates);
    
      pPopDates = (unsigned long*)Smalloc(nPopDates * sizeof(Julian), gpPrintDirection);
    
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
      gpPrintDirection->SatScanPrintf("\nStart Date = %s\n", szDate);
       JulianToChar(szDate, EndDate);
      gpPrintDirection->SatScanPrintf("End Date   = %s\n", szDate);
     
      gpPrintDirection->SatScanPrintf("\nSource offset      = %i\n", nSourceOffset);
      gpPrintDirection->SatScanPrintf("Destination offset = %i\n", nDestOffset);
      gpPrintDirection->SatScanPrintf("Add Start          = %i\n", bStartAsPopDt);
      gpPrintDirection->SatScanPrintf("Add End            = %i\n", bEndAsPopDt);
      gpPrintDirection->SatScanPrintf("Number Dates Used  = %i\n", nDatesUsed);
      gpPrintDirection->SatScanPrintf("Number Pop Dates   = %i\n", nPopDates);
      gpPrintDirection->SatScanPrintf("\n");
     
      gpPrintDirection->SatScanPrintf("<Press any key to continue>");
       c = getc(stdin);
     
       DisplayDatesArray(pPopDates, nPopDates, "Array of Selected Pop Dates", stdout);
#endif
      }
   catch (SSException & x)
      {
      x.AddCallpath("tiSetupPopDates()", "TInfo");
      throw;
      }
} /* tiSetupPopDates */

/**********************************************************************
 Determines which available population years should be used.
 **********************************************************************/
void TInfo::tiFindPopDatesToUse(Julian* pDates, int nDates, Julian StartDate, Julian EndDate,
                         int* pnSourceOffset, int* pnDestOffset, bool* pbAddStart, bool* pbAddEnd,
                         int* pnDatesUsed, int* pnPopDates)
{
   int    n;
   int    nLastIndex;
   bool   bStartFound = false;
   bool   bEndFound   = false;

   try
      {
      *pnSourceOffset = 0;
      *pnDestOffset   = 0;
      *pbAddStart     = false;
      *pbAddEnd       = false;
    
      /* Determine which pop dates to use */
      for (n=0; n<nDates; n++)
      {
        if (!bStartFound)
        {
          if (pDates[n] > StartDate)
          {
            bStartFound = true;
            if (n==0)
            {
              *pbAddStart = true;
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
            bEndFound = true;
        }
      }
    
      *pnDatesUsed = nLastIndex - *pnSourceOffset + 1;
      *pnPopDates  = *pnDatesUsed;
    
      if (*pbAddStart)
        *pnPopDates = *pnPopDates+1;
    
      if (!bEndFound && nLastIndex==(nDates-1))
        {
        *pbAddEnd = true;
        *pnPopDates = *pnPopDates+1;
        }
      }
   catch (SSException & x)
      {
      x.AddCallpath("tiFindPopDatesToUse()", "TInfo");
      throw;
      }
} /* tiFindPopDatesToUse */

/******************************************************************************
 Assigns a population count to the appropriate location in the population list.
 ******************************************************************************/
int TInfo::tiAssignPop(long** pPopList, Julian nPopDate, long nPop)
{
   int nPopIndex;

   try
      {
      nPopIndex = tiGetPopDateIndex(nPopDate);
      if (nPopIndex == -1)
      {
      /* gpPrintDirection->SatScanPrintf("Population Year is not in the index.\n"); */
        return(-1);
      }
    
      (*pPopList)[nPopIndex] += nPop;
    
      if ((nPopIndex == 1) && (bStartAsPopDt))
        (*pPopList)[0] += nPop;
    
      if ((nPopIndex == nPopDates-2) && (bEndAsPopDt))
        (*pPopList)[nPopDates-1] += nPop;
      }
   catch (SSException & x)
      {
      x.AddCallpath("tiAssignPop()", "TInfo");
      throw;
      }
  return(0);
} /* tiAssignPop */

/**********************************************************************
 Returns the population for a given year and category in a given tract
 (0 if the cell does not exist).
 **********************************************************************/
unsigned long TInfo::tiGetPop(tract_t t, int cat, int nPopIndex) const
{
  struct cnode *node = TractInfo[t].cats;
  unsigned long ulValue;
  try
     {
     while (node && node->cat != cat)
       node = node->next;

     if (node && nPopIndex>=0 && nPopIndex<nPopDates)
       ulValue = (node->pPopList[nPopIndex]);
     else
       ulValue = 0;
      }
   catch (SSException & x)
      {
      x.AddCallpath("tiGetPop()", "TInfo");
      throw;
      }
   return ulValue;
} /* tiGetPop() */

/**********************************************************************
 Returns the population date for a given index into the Pop date array.
***********************************************************************/
Julian TInfo::tiGetPopDate(int nIndex) const
{
  if (!(nIndex >= 0 && nIndex < nPopDates))
    return(-1);

  return(pPopDates[nIndex]);
} /* tiGetPopDate */

/**********************************************************************
 Returns the number of dates in the population date array.
***********************************************************************/
int TInfo::tiGetNumPopDates() const
{
  return(nPopDates);
} /* tiGetNumPopDates */

/**********************************************************************
 Returns the index into the Pop date array for a given date.
***********************************************************************/
int TInfo::tiGetPopDateIndex(Julian nDate)
{
   int  retVal;
   int i = 0;
   bool bDateFound = false;

   try
      {
      while (i < nPopDates && !bDateFound)
        {
        bDateFound = (nDate == pPopDates[i]);
        i++;
        }

      if (bDateFound)
         retVal = i-1;
      else
         retVal = -1;
      }
   catch (SSException & x)
      {
      x.AddCallpath("tiGetPopDateIndex()", "TInfo");
      throw;
      }
  return (retVal);
} /* tiGetPopDateIndex */

/*************************************************************************
 Returns the indeces to population dates that bound a given interval date.
**************************************************************************/
int TInfo::tiGetPopUpLowIndex(Julian* pDates, int nDateIndex, int nMaxDateIndex, int* nUpIndex, int* nLowIndex) const
{
   int i, index;
   bool bUpFound = false;

   try
      {
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
          bUpFound = true;
          *nUpIndex = i;
          }
        }
      }
   catch (SSException & x)
      {
      x.AddCallpath("tiGetPopUpLowIndex()", "TInfo");
      throw;
      }
   return(1);
} /* tiGetPopUpLowIndex */

/*************************************************************************
 Calculates....
 These calculations assumes that the population of a given day refers to
 the beginning of that day.
**************************************************************************/
/** returns array that indicates, for each population date, population dates
    percentage of the whole study period - Scott Hostovich @ July 16,2002*/
void TInfo::tiCalcAlpha(double** pAlpha, Julian StartDate, Julian EndDate) const
{
   int  n;
   int  N           = nPopDates-2;
   long nTotalYears = TimeBetween(StartDate, EndDate, DAY)/*EndDate-StartDate*/ ;
   double sumalpha;
   //char szDate[MAX_DT_STR];

   try
     {
     *pAlpha = (double*)Smalloc((nPopDates+1) * sizeof(double), gpPrintDirection);
   
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
      gpPrintDirection->SatScanPrintf("Pop\nIndex   PopDates        Alpha\n");
       for (n=0;n<=N+1;n++) {
         JulianToChar(szDate,pPopDates[n]);
        gpPrintDirection->SatScanPrintf("%i       %s        %f\n",n, szDate, (*pAlpha)[n]);
         }
      gpPrintDirection->SatScanPrintf("\n");
#endif


      /* Bug check, seeing that alpha values add to one. */
      sumalpha = 0;
      for (n = 0; n <= N+1; n++) sumalpha = sumalpha + (*pAlpha)[n];
      if(sumalpha>1.0001 || sumalpha<0.9999)
         {
         char sMessage[200], sTmp[50];
         strcpy(sMessage, "\n\n  Error: Alpha values not calculated correctly in");
         strcat(sMessage, "\n  file tinfo.c, function tiCalcAlpha. The sum of the ");
         sprintf(sTmp, "\n  alpha values is %8.6lf rather than 1.\n", sumalpha);
         strcat(sMessage, sTmp);
         SSGenerateException(sMessage, "tiCalcAlpha");
         }
      }
   catch (SSException & x)
      {
      x.AddCallpath("tiCalcAlpha()", "TInfo");
      throw;
      }
} /* tiCalcAlpha */

/* ========================== Case Count Routines ========================== */

/**********************************************************************
 Sets the case count for a given cell.
 Return value:
   1 = OK
   0 = cell does not exist; not added
 **********************************************************************/
int TInfo::tiSetCount(tract_t t, int cat, count_t count)
{
   struct cnode *node = TractInfo[t].cats;
   int iValue;

   try
      {
      while (node && node->cat != cat)
         node = node->next;

      if (node)
         {
         node->count = count;
         iValue = 1;
         }
      else
         iValue = 0;
      }
   catch (SSException & x)
      {
      x.AddCallpath("tiSetCount()", "TInfo");
      throw;
      }
   return iValue;
} /* tiSetCount() */

/**********************************************************************
 Returns the case count for a given category in a given tract
 (0 if the cell does not exist).
 **********************************************************************/
count_t TInfo::tiGetCount(tract_t t, int cat) const
{
   struct cnode *node = TractInfo[t].cats;
   count_t tValue;

   try
      {
      while (node && node->cat != cat)
         node = node->next;

      if (node)
         tValue = node->count;
      else
         tValue = 0;
      }
   catch (SSException & x)
      {
      x.AddCallpath("tiGetCount()", "TInfo");
      throw;
      }
   return tValue;
} /* tiGetCount() */


/* =========================== Display Routines ============================ */

/**********************************************************************
 Display an array of Julian dates.
 **********************************************************************/
void TInfo::DisplayDatesArray(Julian* pDates, int nDates, char* szTitle, FILE* pDisplay)
{
   int n;
   //char c;
   char szDate[MAX_DT_STR];

   try
      {
      fprintf(pDisplay, "\n%s\n", szTitle);
      for (n=0; n<nDates; n++)
        {
        JulianToChar(szDate, pDates[n]);
        fprintf(pDisplay, "%i  %s\n", n, szDate);
        }
    
      if (pDisplay == stdout || pDisplay == stderr)
        {
        fprintf(pDisplay, "<Press any key to continue>\n");
        getc(stdin);
        }
      else
        fprintf(pDisplay, "\n");
      }
   catch (SSException & x)
      {
      x.AddCallpath("DisplayDatesArray()", "TInfo");
      throw;
      }
}

/**********************************************************************
 Display the array of Population dates.
 **********************************************************************/
void TInfo::DisplayPopDatesArray(FILE* pDisplay)
{
   int n;
   //char c;
   char szDate[MAX_DT_STR];

   try
      {
      fprintf(pDisplay, "\nPopulation Dates\n");
      for (n=0; n<nPopDates; n++)
        {
        JulianToChar(szDate, pPopDates[n]);
        fprintf(pDisplay, "%i  %s\n", n, szDate);
        }
    
      if (pDisplay == stdout || pDisplay == stderr)
        {
        fprintf(pDisplay, "<Press any key to continue>\n");
        getc(stdin);
        }
      else
        fprintf(pDisplay, "\n");
      }
   catch (SSException & x)
      {
      x.AddCallpath("DisplayPopDatesArray()", "TInfo");
      throw;
      }
}

/**********************************************************************
 Display tract info - x, y, z coordinates.
 **********************************************************************/
void TInfo::DisplayTractInfo()
{
   //int i;
   try
      {
      gpPrintDirection->SatScanPrintf("TID        x           y          z\n");
      // Temporarily disable this
      //for (i=0; i<NumTracts; i++)
      //{
      //  gpPrintDirection->SatScanPrintf("%s   %f   %f   %f\n", TractInfo[i].tid, TractInfo[i].x,
      //                                 TractInfo[i].y, TractInfo[i].z);
      //}
      gpPrintDirection->SatScanPrintf("\n");
      }
   catch (SSException & x)
      {
      x.AddCallpath("DisplayTractInfo()", "TInfo");
      throw;
      }
} /* DisplayTractInfo */

/**********************************************************************
 Loop through and display all tract info - categories, case counts and
 populations.
 **********************************************************************/
void TInfo::DisplayAllTractInfo(FILE* pDisplay)
{
   int i, j;
   struct cnode* cats;
   //char c;

   try
      {
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
            getc(stdin);
          else
            fprintf(pDisplay, "\n");
          }
        }
      fprintf(pDisplay, "\n");
      }
   catch (SSException & x)
      {
      x.AddCallpath("DisplayAllTractInfo()", "TInfo");
      throw;
      }
} /* Display All Info */


/* ============================ Misc. Routines ============================= */

/**********************************************************************
 Cleanup all tract storage.
 **********************************************************************/
void TInfo::tiCleanup()
{
   struct cnode* pCurrCat;
   struct cnode* pNextCat;

   int i;
   //int j, k;

   /*  for (i=0; i<ti_length; i++)*/
   if (TractInfo)
      {
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
bool TInfo::tiFindDuplicateCoords(FILE* pDisplay)
{
   bool bStop = false;
   double* pCoords;
   double* pCoords2;
   tract_t i=0;
   tract_t j;
   int nDims;							// Counter for dimensions

   try
      {
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
          	bStop=true;
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
        gpPrintDirection->SatScanPrintWarning("  Error: Duplicate coordinates found for tracts %s and %s Coords=(",
             	TractInfo[i].tid, TractInfo[j].tid);
        for (nDims=0; nDims<(nDimensions-1); nDims++)
          {
        	fprintf(pDisplay, "%.0f, ", pCoords[nDims]);
          }
        fprintf(pDisplay, "%.0f).\n\n", pCoords[nDimensions-1]);
        gpPrintDirection->SatScanPrintWarning("%.0f).\n\n", pCoords[nDimensions-1]);
        //NO NEED TO THROW AN EXCEPTION HERE... DONE UPON RETURN TO MAIN FUNCTION...
        free(pCoords);
        }
      }
   catch (SSException & x)
      {
      x.AddCallpath("tiFindDuplicateCoords()", "TInfo");
      throw;
      }
  return (bStop);
}

/**********************************************************************
Look for and display tracts with zero population for any population year.  7/8/97, K. Rand
 **********************************************************************/
void TInfo::tiReportZeroPops(FILE *pDisplay) const
{
   int i, j;
   UInt month, day, year;
   bool bZeroFound = false;
   struct cnode* cats;
   long* PopTotalsArray = 0;
   int   nPStartIndex = 0;
   int   nPEndIndex;

   try
      {
      if (bStartAsPopDt)
        nPStartIndex = 1;
      if (bEndAsPopDt)
        nPEndIndex = nPopDates-2;
      else
        nPEndIndex = nPopDates-1;
    
      PopTotalsArray = (long*)Smalloc(nPopDates *sizeof(long), gpPrintDirection);
    
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
              bZeroFound = true;
              fprintf(pDisplay,"\n________________________________________________________________\n\n");
              fprintf(pDisplay,"Warning: According to the input data, the following tracts have a \n");
              fprintf(pDisplay,"         population totaling zero for the specified year(s).\n\n");
              gpPrintDirection->SatScanPrintWarning("\n________________________________________________________________\n\n");
              gpPrintDirection->SatScanPrintWarning("Warning: According to the input data, the following tracts have a \n");
              gpPrintDirection->SatScanPrintWarning("         population totaling zero for the specified year(s).\n\n");
            }
            JulianToMDY(&month, &day, &year, pPopDates[j]);
            fprintf(pDisplay,"         Tract %s, %d\n", TractInfo[i].tid, year);
          }
        }
    
      } /* i=0-<NumTracts */
    
      free (PopTotalsArray);
      }
   catch (SSException & x)
      {
      free (PopTotalsArray);
      x.AddCallpath("tiReportZeroPops()", "TInfo");
      throw;
      }
}

/*******************************************************************************
Add dimension size (of coordinates) to model
*******************************************************************************/
void TInfo::tiSetDimensions(int nDim)
{
   nDimensions = nDim;
}

/*******************************************************************************
Compute distance sqaured between 2 tracts
*******************************************************************************/
double TInfo::tiGetDistanceSq(double* pCoords, double* pCoords2) const
{
   int i;
   double sum=0;

   try
      {
      for (i=0; i<nDimensions; i++)
        sum += (pCoords[i] - pCoords2[i]) * (pCoords[i] - pCoords2[i]);
      }
   catch (SSException & x)
      {
      x.AddCallpath("tiGetDistanceSq()", "TInfo");
      throw;
      }
  return(sum);
}

/*******************************************************************************
Check to see that no cases were found for tracts that have no population
*******************************************************************************/
bool TInfo::tiCheckCasesHavePop() const
{
   int i, j;
   //UInt month, day, year;
   bool bValid = true;
   struct cnode* cats;
   long  nPopTotal;
   int   nPStartIndex = 0;
   int   nPEndIndex, iTractPopulation;
   char  szCategories[100], sMessage[100];

   try
      {
      if (bStartAsPopDt)
        nPStartIndex = 1;
      if (bEndAsPopDt)
        nPEndIndex = nPopDates-2;
      else
        nPEndIndex = nPopDates-1;
    
      for (i=0; i<NumTracts; i++)
        {
        iTractPopulation = 0;
        cats = TractInfo[i].cats;
        while (cats)
          {
          strcpy(szCategories, "\0");
          nPopTotal   = 0;
          for (j=nPStartIndex; j<=nPEndIndex; j++)
            nPopTotal += cats->pPopList[j];
          iTractPopulation += nPopTotal;
          if (nPopTotal==0 && cats->count>0)
            {
            //DTG -- was asked to set this as a warning and not an error
            // Categories can have zero pops, but pop has to be greater than zero for entire tract
            //
           // bValid = false;
            gpPrintDirection->SatScanPrintWarning("  Warning: %s %s has cases but zero population.\n",
            TractInfo[i].tid, gpCats->catGetCategoriesString(cats->cat, szCategories));
            }

          cats = cats->next;
          }
        if (iTractPopulation == 0)
           {
           sprintf(sMessage,"Total population is zero for tract %i", TractInfo[i].tid);
           SSGenerateException(sMessage, "TInfo");
           }
    
        } /* i=0-<NumTracts */
      }
   catch (SSException & x)
      {
      x.AddCallpath("tiCheckCasesHavePop()", "TInfo");
      throw;
      }
  return bValid;
}

bool TInfo::tiCheckZeroPops(FILE *pDisplay) const
/*******************************************************************************
Check to see that no years have a total population of zero.
*******************************************************************************/
{
   int i, j;
   UInt month, day, year;
   bool bValid = true;
   struct cnode* cats;
   long* PopTotalsArray = 0;
   int   nPStartIndex = 0;
   int   nPEndIndex;

   try
      {
      if (bStartAsPopDt)
        nPStartIndex = 1;
      if (bEndAsPopDt)
        nPEndIndex = nPopDates-2;
      else
        nPEndIndex = nPopDates-1;
    
      PopTotalsArray = (long*)Smalloc(nPopDates *sizeof(long), gpPrintDirection);
    
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
          bValid = false;
          JulianToMDY(&month, &day, &year, pPopDates[j]);
          fprintf(pDisplay, "  Error: Population of zero found for all tracts in %d.\n", year);
          gpPrintDirection->SatScanPrintWarning("  Error: Population of zero found for all tracts in %d.\n", year);
          }
        }
    
      free (PopTotalsArray);
      }
   catch (SSException & x)
      {
      free (PopTotalsArray);
      x.AddCallpath("tiCheckZeroPops()", "TInfo");
      throw;
      }
  return bValid;
}

/* Returns coordinate for tract at specified dimension. */
double TInfo::tiGetTractCoordinate(tract_t t, int iDimension) const {
  try {
    if ((0 > t && t >= NumTracts) || (0 > iDimension && iDimension >= nDimensions))
      ZdGenerateException("Index out of range","tiGetTractCoordinate()");
  }
  catch (SSException & x) {
    x.AddCallpath("tiGetTractCoordinate()", "TInfo");
    throw;
  }
  return TractInfo[t].pCoords[iDimension];
}
