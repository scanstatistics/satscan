/**********************************************************************
 file: tinfo.h
 Header file for tinfo.c, which abstracts tract, population & case
 count information
 **********************************************************************/

#ifndef TINFO_H
#define TINFO_H
#include "spatscan.h"
#include "date.h"

/* Tract Routines */
int     tiInsertTnode(char *tid, float* pCoords);
tract_t tiGetNumTracts(void);
tract_t tiGetTractNum(char *tid);
char*   tiGetTid(tract_t t);
void    tiGetCoords(tract_t t, float** pCoords);

/* Category Routines */
void    tiAddCat(tract_t t, int cat, Julian popDate, long pop);

/* Population Routines */
void    tiSetupPopDates(Julian* pDates, int nDates, Julian StartDate, Julian EndDate);
void    tiFindPopDatesToUse(Julian* pDates, int nDates, Julian StartDate, Julian EndDate,
                            int* pnSourceOffset, int* pnDestOffset, BOOL* pbAddStart, BOOL* pbAddEnd,
                            int* pnDatesUsed, int* pnTotalPopDates);
int     tiAssignPop(long** pPopList, Julian nPopDate, long nPop);
unsigned long    tiGetPop(tract_t t, int cat, int nPopIndex);
Julian  tiGetPopDate(int nIndex);
int     tiGetNumPopDates(void);
int     tiGetPopDateIndex(Julian nDate);
int     tiGetPopUpLowIndex(Julian* pDates, int nDateIndex, int nMaxDateIndex, int* nUpIndex, int* nLowIndex);
void    tiCalcAlpha(double** pAlpha, Julian StartDate, Julian EndDate);

/* Case Count Routines */
int     tiSetCount(tract_t t, int cat, count_t count);
count_t tiGetCount(tract_t t, int cat);

/* Display Routines */
void DisplayDatesArray(Julian* pDates, int nDates, char* szTitle, FILE* pDisplay);
void DisplayPopDatesArray(FILE* pDisplay);
void DisplayTractInfo();
void DisplayAllTractInfo(FILE* pDisplay);
void DisplayPopDates();

/* Misc. Routines */
void    tiCleanup();
int     sort_function(const void *a, const void *b);
BOOL    tiFindDuplicateCoords(FILE* pDisplay);
void    tiReportZeroPops(FILE *pDisplay);
void		tiSetDimensions(int nDim);
//float   tiGetDistance(tract_t t1, tract_t t2, int nDimensions);
float   tiGetDistanceSq(float* pCoords, float* pCoords2);

BOOL    tiCheckCasesHavePop();
BOOL    tiCheckZeroPops(FILE *pDisplay);

#endif
