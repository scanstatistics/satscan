/**********************************************************************
 file: cats.h
 Header file for category facility (cats.c)
 **********************************************************************/

#ifndef CATS_H
#define CATS_H

#include "baseprint.h"

struct catnode {     /* associates a combination of values with cat number */
   int num;                /* category number       */
   char **dvec;            /* array of values       */
   struct catnode *next;   /* link to next category */
};

class Cats
{
private:
 BasePrint *gpPrintDirection;

 struct catnode *CatList;             /* linked list of categories */
 int CatVecLength;  /* = 0; (KR 1/14/97) */         /* length of "dvec" */

   void Init();
   void Free();
public:
   Cats(BasePrint *pPrintDirection);
   ~Cats();
int catGetCat(char *dvec[]);
int catMakeCat(char *dvec[]);
int catNumCats(void);
void catSetNumEls(int n);
int catGetNumEls(void);
char* catGetCategoriesString(int n, char* dvec);
void catDisplay(void);
void catCleanup();
};
#endif
