/**********************************************************************
 file: cats.h
 Header file for category facility (cats.c)
 **********************************************************************/

#ifndef CATS_H
#define CATS_H

int catGetCat(char *dvec[]);
int catMakeCat(char *dvec[]);
int catNumCats(void);
void catSetNumEls(int n);
int catGetNumEls(void);
char* catGetCategoriesString(int n, char* dvec);
void catDisplay(void);
void catCleanup();

#endif
