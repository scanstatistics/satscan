#pragma hdrstop
#include "Sort.h"

void Sort(unsigned long** pData, int low, int high)
{
   int i_pivotLocation;

   try
      {
      if (low < high)
         {
         i_pivotLocation = Partition(pData, low, high);
         Sort(pData, low, i_pivotLocation-1);
         Sort(pData, i_pivotLocation+1, high);
         }
      }
   catch (SSException & x)
      {
      x.AddCallpath("Sort()", "Sort.cpp");
      throw;
      }
}

int Partition(unsigned long** pData, int low, int high)
{
   int i, i_mid, i_lastSmall;

   try
      {
      i_mid = low + (high-low)/2;
      i_lastSmall = low;

      if (low != i_mid)
         Swap(pData, low, i_mid);

      for (i=low+1; i<=high; i++)
        {
        if ((*pData)[i] < (*pData)[low])
          {
          if (i != (i_lastSmall+1))
            Swap(pData, i_lastSmall+1, i);
          i_lastSmall++;
          }
        }

      if (low != i_lastSmall)
        Swap(pData, low, i_lastSmall);
      }
   catch (SSException & x)
      {
      x.AddCallpath("Partition()", "Sort.cpp");
      throw;
      }
  return(i_lastSmall);
}

void Swap(unsigned long** pData, int index1, int index2)
{
   unsigned long l;

   try
      {
      l                = (*pData)[index1];
      (*pData)[index1] = (*pData)[index2];
      (*pData)[index2] = l;
      }
   catch (SSException & x)
      {
      x.AddCallpath("Swap()", "Sort.cpp");
      throw;
      }
}

/*int Search(unsigned long** pData, unsigned long* pValue, int nItems)
{
  int  target;
  int  bottom, top, mid;
  bool found;

  bottom = 0;
  top    = nItems-1;
  found  = false;

  if (*pValue != 0)
  {
    while (!found && (bottom<=top))
    {
      mid = ((top + bottom) / 2);

      if ((*pData)[mid] == target)
        found = true;
      else if (target < (*pData)[mid])
        top = mid-1;
      else if (target > (*pData)[mid])
        bottom = mid+1;
    }
  }

  return(target);
}*/

