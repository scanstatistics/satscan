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
   char               * gid;       /* grid tract id string */
   double             * pCoords;   /* coordinates          */
};

class GInfo {
  private:
    static const int    TI_GROW = 100; /** Growth increment for GInfo */

    struct gnode      * GridTractInfo;  /* grid tract info vector           */
    tract_t             gi_length;  /* allocated len of GridTractInfo   */
    tract_t             NumGridTracts;  /* number of grid tracts filled     */
    int 	        nDimensions;

    void                Free();
    void                Init();

  public:
    GInfo();
    ~GInfo();

    void                giCleanup();
    void                giDisplayGridTractInfo(FILE* pDisplay);
    bool                giFindDuplicateCoords(FILE* pDisplay);
    tract_t             giGetNumTracts();
    tract_t             giGetTractNum(const char *gid);
    void                giGetCoords(tract_t t, double** pCoords) const;
    void                giGetCoords2(tract_t t, double* pCoord) const;
    int                 giGetNumDimensions() const;
    void                giInsertGnode(const char *gid, std::vector<double>& vCoordinates);
    void                giRetrieveCoords(tract_t t, std::vector<double> & vRepository) const;
    void                giSetDimensions(int nDim);
};
//*****************************************************************************
#endif
