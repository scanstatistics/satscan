//*****************************************************************************
#ifndef __GRIDTRACTCOORDINATES_H
#define __GRIDTRACTCOORDINATES_H
//*****************************************************************************
#include "SaTScan.h"
#include "JulianDates.h"

/**********************************************************************
 file: GridTractCoordinates.h
 Header file for GridTractCoordinates.cpp, which abstracts grid tract coordinates
 **********************************************************************/

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
int     giInsertGnode(const char *gid, std::vector<double>& vCoordinates);
tract_t giGetNumTracts(void);
tract_t giGetTractNum(const char *gid);
char*   giGetGid(tract_t t) const;
void    giGetCoords(tract_t t, double** pCoords) const ;
std::vector<double> giGetCoords(tract_t t) const;
void    giGetCoords2(tract_t t, double* pCoord) const ;
int     giGetNumDimensions() const;

/* Display Routines */
void giDisplayGridTractInfo();

/* Misc. Routines */
void giCleanup();
void giSetDimensions(int nDim);
bool giFindDuplicateCoords(FILE* pDisplay);
};
//*****************************************************************************
#endif
