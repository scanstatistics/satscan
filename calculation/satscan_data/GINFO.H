/**********************************************************************
 file: ginfo.h
 Header file for ginfo.c, which abstracts grid tract coordinates
 **********************************************************************/

#ifndef GINFO_H
#define GINFO_H
#include "spatscan.h"
#include "date.h"

/* Tract Routines */
int     giInsertGnode(char *gid, float* pCoords);
tract_t giGetNumTracts(void);
tract_t giGetTractNum(char *gid);
char*   giGetGid(tract_t t);
void    giGetCoords(tract_t t, float** pCoords);

/* Display Routines */
void giDisplayGridTractInfo();

/* Misc. Routines */
void giCleanup();
void giSetDimensions(int nDim);
BOOL giFindDuplicateCoords(FILE* pDisplay);

#endif






