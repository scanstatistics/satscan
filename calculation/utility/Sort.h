/* Sort.h */

#ifndef _SORT_H
#define _SORT_H

#include "SSException.h"

void Sort(unsigned long** pData, int low, int high);
void Swap(unsigned long** pData, int index1, int index2);
int  Partition(unsigned long** pData, int low, int high);
//int  Search(unsigned long** pData, unsigned long* pValue, int nItems);

#endif
