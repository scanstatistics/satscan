#pragma hdrstop
#include "PopulationCategories.h"

/**********************************************************************
 file: cats.c
 This file abstracts "categories" of populations.
 Each data and case record has a variable number of fields which specify
 arbitrary string values.  Each combination of these values defines a
 separate category, by which the data is stratified.  This unit contains
 operations to manipulate these categories.
 All external function names have a "cat" prefix.

 NOTE:
 Currently, the mechanism for translating lists of values into category
 numbers is extremely primitive.  It could be improved by using less
 dynamic memory, by using a better data structure than a linked list,
 and by merging identical cati value strings.
 **********************************************************************/

static int CompList(char *dv1[], char *dv2[], int len);


Cats::Cats(BasePrint *pPrintDirection)
{
   Init();
   gpPrintDirection = pPrintDirection;
}
Cats::~Cats()
{
   Free();
}
void Cats::Free()
{
   catCleanup();
}
void Cats::Init()
{
   CatList = 0;
   //CatVecLength;  /* = 0; (KR 1/14/97) */

   CatVecLength = 0;
}
/**********************************************************************
 Translate a list of values into its category number (-1 if none)
 **********************************************************************/
int Cats::catGetCat(char *dvec[])
{
   struct catnode *node = CatList;
   try
      {
      while (node && CompList(dvec, node->dvec, CatVecLength))
         node = node->next;
      }
   catch (SSException & x)
      {
      x.AddCallpath("catGetCat(char *)", "Cats");
      throw;
      }
   return node ? (node->num) : -1;
} /* catGetCat() */


/**********************************************************************
 If the value list is unique, make a new category for it.
 Return the category number.
 **********************************************************************/
int Cats::catMakeCat(char *dvec[])
{
   struct catnode *node;
   int i;

   try
      {
      if ((i = catGetCat(dvec)) != -1)
         return i;
   
      node = (catnode*)Smalloc(sizeof(struct catnode), gpPrintDirection);
      node->num = CatList ? CatList->num + 1 : 0;
      node->dvec = (char**)Smalloc(CatVecLength * sizeof(char *), gpPrintDirection);
      for (i = 0; i < CatVecLength; i++)
   /*      node->dvec[i] = Sstrdup(dvec[i]); */
         Sstrcpy(&(node->dvec[i]), dvec[i], gpPrintDirection);
   
      node->next = CatList;
      CatList = node;
      }
   catch (SSException & x)
      {
      x.AddCallpath("catMakeCat(char *)", "Cats");
      throw;
      }
   return node->num;
} /* catMakeCat() */


/**********************************************************************
 Returns the number of categories found so far
 **********************************************************************/
int Cats::catNumCats(void)
{
   return CatList ? (CatList->num + 1) : 0;
} /* catNumCats() */


/**********************************************************************
 Sets the number of elements in a category vector.
 This should only be called once.  Future calls will have no effect
 **********************************************************************/
void Cats::catSetNumEls(int n)
{
/*   if (CatVecLength == 0)  (KR 1/14/97) */
   CatVecLength = n;
} /* catSetNumEls() */

/**********************************************************************
 Returns the number of elements per category vector.
 **********************************************************************/
int Cats::catGetNumEls(void)
{
   return CatVecLength;
} /* catGetNumEls() */


/**********************************************************************
 Compare two lists of char pointers.
 Return value:
   0   = they point to identical strings
   > 0 = dv2 > dv1
   < 0 = dv2 < dv1
 **********************************************************************/
static int CompList(char *dv1[], char *dv2[], int len)
{
   int i;
   
   try
      {
      while (len--)
         if ((i = strcmp(*dv1++, *dv2++)) != 0)
            return i;
      }
   catch (SSException & x)
      {
      x.AddCallpath("CompList(char *, char *, int)", "Cats");
      throw;
      }
   return 0;
} /* CompList() */

/**********************************************************************
 Return Category String
 **********************************************************************/
char* Cats::catGetCategoriesString(int n, char* dvec)
{
   int    i;
   struct catnode *node = CatList;

   try
      {
      while (node && n != node->num)
         node = node->next;

      if (node)
         {
         for (i = 0; i < CatVecLength; i++)
            {
            strcat(dvec, node->dvec[i]);
            strcat(dvec, "  ");
            }
         }
      }
   catch (SSException & x)
      {
      x.AddCallpath("catGetCategoriesString(int, char *)", "Cats");
      throw;
      }
  return dvec;
} /* DisplayCats() */

/**********************************************************************
 Display categories in the list
 **********************************************************************/
void Cats::catDisplay(void)
{
   //int i;
   int j;
   //  int nCatCombs = catNumCats();
   int nCatVars  = catGetNumEls();
   struct catnode *node = CatList;

   try
      {
      gpPrintDirection->SatScanPrintf("DISPLAY:CatVecLength=%i\n",CatVecLength);
      gpPrintDirection->SatScanPrintf("\n#   Category Combination\n");
     
      while (node)
         {
         gpPrintDirection->SatScanPrintf("%d     ",  node->num);
          for (j=0; j<nCatVars; j++)
           gpPrintDirection->SatScanPrintf("%s  ", node->dvec[j]);
         gpPrintDirection->SatScanPrintf("\n");
          node = node->next;
         }
     
      gpPrintDirection->SatScanPrintf("\n");
      }
   catch (SSException & x)
      {
      x.AddCallpath("catDisplay()", "Cats");
      throw;
      }
} /* DisplayCats() */

void Cats::catCleanup()
{
   struct catnode* pCurrCat;
   struct catnode* pNextCat;
   int    i;
  
   try
      {
      pCurrCat = CatList;
    
      while (pCurrCat != NULL)
        {
        pNextCat = pCurrCat->next;
        for (i = 0; i < CatVecLength; i++)
          free(pCurrCat->dvec[i]);
        free(pCurrCat->dvec);
        free(pCurrCat);
        pCurrCat = pNextCat;
        }
      }
   catch (...)
      {
      }
}

