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

#include <string.h>
#include <stdio.h>
#include "cats.h"
#include "salloc.h"

struct catnode {     /* associates a combination of values with cat number */
   int num;                /* category number       */
   char **dvec;            /* array of values       */
   struct catnode *next;   /* link to next category */
};

static struct catnode *CatList=0;             /* linked list of categories */
static int CatVecLength;  /* = 0; (KR 1/14/97) */         /* length of "dvec" */

static int CompList(char *dv1[], char *dv2[], int len);

/**********************************************************************
 Translate a list of values into its category number (-1 if none)
 **********************************************************************/
int catGetCat(char *dvec[])
{
   struct catnode *node = CatList;

   while (node && CompList(dvec, node->dvec, CatVecLength))
      node = node->next;

   return node ? (node->num) : -1;
} /* catGetCat() */


/**********************************************************************
 If the value list is unique, make a new category for it.
 Return the category number.
 **********************************************************************/
int catMakeCat(char *dvec[])
{
   struct catnode *node;
   int i;

   if ((i = catGetCat(dvec)) != -1)
      return i;

   node = (catnode*)Smalloc(sizeof(struct catnode));
   node->num = CatList ? CatList->num + 1 : 0;
   node->dvec = (char**)Smalloc(CatVecLength * sizeof(char *));
   for (i = 0; i < CatVecLength; i++)
/*      node->dvec[i] = Sstrdup(dvec[i]); */
      Sstrcpy(&(node->dvec[i]), dvec[i]);

   node->next = CatList;
   CatList = node;
   return node->num;
} /* catMakeCat() */


/**********************************************************************
 Returns the number of categories found so far
 **********************************************************************/
int catNumCats(void)
{
   return CatList ? (CatList->num + 1) : 0;
} /* catNumCats() */


/**********************************************************************
 Sets the number of elements in a category vector.
 This should only be called once.  Future calls will have no effect
 **********************************************************************/
void catSetNumEls(int n)
{
/*   if (CatVecLength == 0)  (KR 1/14/97) */
   CatVecLength = n;
} /* catSetNumEls() */

/**********************************************************************
 Returns the number of elements per category vector.
 **********************************************************************/
int catGetNumEls(void)
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
   while (len--)
      if ((i = strcmp(*dv1++, *dv2++)) != 0)
         return i;
   return 0;
} /* CompList() */

/**********************************************************************
 Return Category String
 **********************************************************************/
char* catGetCategoriesString(int n, char* dvec)
{
  int    i;
  struct catnode *node = CatList;

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

  return dvec;
} /* DisplayCats() */

/**********************************************************************
 Display categories in the list
 **********************************************************************/
void catDisplay(void)
{
  //int i;
  int j;
  int nCatCombs = catNumCats();
  int nCatVars  = catGetNumEls();
  struct catnode *node = CatList;

  printf("DISPLAY:CatVecLength=%i\n",CatVecLength);
  printf("\n#   Category Combination\n");

   while (node)
   {
     printf("%d     ",  node->num);
     for (j=0; j<nCatVars; j++)
       printf("%s  ", node->dvec[j]);
     printf("\n");
     node = node->next;
   }

   printf("\n");

} /* DisplayCats() */

void catCleanup()
{
  struct catnode* pCurrCat;
  struct catnode* pNextCat;
  int    i;
  
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

