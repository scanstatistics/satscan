/**********************************************************************
 file: ginfo.h
 Header file for ginfo.c, which abstracts grid tract coordinates
 **********************************************************************/

#ifndef GINFO_H
#define GINFO_H
#include "spatscan.h"
#include "SSException.h"
#include "date.h"
#include "baseprint.h"


struct gnode {              /* grid tract record    */
   char*         gid;       /* grid tract id string */
   double*        pCoords;   /* coordinates          */
};

class GInfo
{
private:
 BasePrint *gpPrintDirection;

 struct gnode* GridTractInfo ;  /* grid tract info vector           */
 tract_t       gi_length     ;  /* allocated len of GridTractInfo   */
 tract_t       NumGridTracts ;  /* number of grid tracts filled     */
 int 	        nDimensions  ;

 void Free();
 void Init();
public:
   GInfo(BasePrint *pPrintDirection);
   ~GInfo();

/* Tract Routines */
int     giInsertGnode(char *gid, double* pCoords);
tract_t giGetNumTracts(void);
tract_t giGetTractNum(char *gid);
char*   giGetGid(tract_t t);
void    giGetCoords(tract_t t, double** pCoords) const ;
void    giGetCoords2(tract_t t, double* pCoord) const ;
int     giGetNumDimensions();

/* Display Routines */
void giDisplayGridTractInfo();

/* Misc. Routines */
void giCleanup();
void giSetDimensions(int nDim);
bool giFindDuplicateCoords(FILE* pDisplay);
};
#endif






