//*****************************************************************************
#ifndef __TRACTS_H
#define __TRACTS_H
//*****************************************************************************
#include "SaTScan.h"
#include "Sort.h"
#include "JulianDates.h"
#include "PopulationCategories.h"

/**********************************************************************
 file: Tracts.h
 Header file for tinfo.c, which abstracts tract, population & case
 count information
 **********************************************************************/

struct cnode {           /* record for one pop. category in a single tract */
  int           cat;        /* category number   */
  long*         pPopList;   /* population vector */
  count_t       count;      /* case count        */
  struct cnode* next;       /* list link         */
};

struct tnode {              /* tract record              */
   char*         tid;       /* tract id string           */
   /*double         x, y, z;   /* coordinates               */
   double*	 pCoords;   /* coordinates               */
   struct cnode* cats;      /* pops & counts by category */
};

class TInfo
{
private:
   Cats *gpCats;
   BasePrint *gpPrintDirection;

 struct tnode* TractInfo ;  /* tract info vector                */
  tract_t       ti_length   ;  /* allocated length of TractInfo    */
  tract_t       NumTracts    ;  /* number of tracts filled          */

 Julian*       pPopDates     ;  /* Pop dates vector                 */
 int           nPopDates     ;  /* Number of population years       */
 bool          bStartAsPopDt ;
 bool          bEndAsPopDt   ;
 int 	     nDimensions ;

 void   Free();
 void   Init();
 
public:
     TInfo(Cats *pCats, BasePrint *pPrintDirection);
    ~TInfo();

/* Tract Routines */
int     tiInsertTnode(char *tid, double* pCoords);
tract_t tiGetNumTracts(void) const;
tract_t tiGetTractNum(char *tid);
char*   tiGetTid(tract_t t) const;
void    tiGetCoords(tract_t t, double** pCoords) const;
void    tiGetCoords2(tract_t t, double* pCoords) const;

/* Category Routines */
void    tiAddCat(tract_t t, int cat, Julian popDate, long pop);

/* Population Routines */
void    tiSetupPopDates(Julian* pDates, int nDates, Julian StartDate, Julian EndDate);
void    tiFindPopDatesToUse(Julian* pDates, int nDates, Julian StartDate, Julian EndDate,
                            int* pnSourceOffset, int* pnDestOffset, bool* pbAddStart, bool* pbAddEnd,
                            int* pnDatesUsed, int* pnTotalPopDates);
int     tiAssignPop(long** pPopList, Julian nPopDate, long nPop);
unsigned long    tiGetPop(tract_t t, int cat, int nPopIndex) const;
Julian  tiGetPopDate(int nIndex) const;
int     tiGetNumPopDates(void) const;
int     tiGetPopDateIndex(Julian nDate);
int     tiGetPopUpLowIndex(Julian* pDates, int nDateIndex, int nMaxDateIndex, int* nUpIndex, int* nLowIndex) const;
void    tiCalcAlpha(double** pAlpha, Julian StartDate, Julian EndDate) const;

/* Case Count Routines */
int     tiSetCount(tract_t t, int cat, count_t count);
count_t tiGetCount(tract_t t, int cat) const;

/* Display Routines */
void DisplayDatesArray(Julian* pDates, int nDates, char* szTitle, FILE* pDisplay);
void DisplayPopDatesArray(FILE* pDisplay);
void DisplayTractInfo();
void DisplayAllTractInfo(FILE* pDisplay);
void DisplayPopDates();

/* Misc. Routines */
void    tiCleanup();
int     sort_function(const void *a, const void *b);
bool    tiFindDuplicateCoords(FILE* pDisplay);
void    tiReportZeroPops(FILE *pDisplay) const;
void	tiSetDimensions(int nDim);
double   tiGetDistanceSq(double* pCoords, double* pCoords2) const;

bool    tiCheckCasesHavePop() const;
bool    tiCheckZeroPops(FILE *pDisplay) const;
};
#endif
