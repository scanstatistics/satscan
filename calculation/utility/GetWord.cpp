// GetWord.cpp

#include "GetWord.h"
#include "Spatscan.h"
#include "error.h"

/**********************************************************************
 Returns a pointer to the "num"-th whitespace-delimited word in a string
 (starting at word number 0).
 The value returned is a pointer to a static string.
 If there are not "num" words, NULL is returned.
 **********************************************************************/
/*KR-6/20/97staticKR-6/20/97*/
char* GetWord(char *s, int num)
{
   int inwd;
   char *cp = s;
   char *cp2;
   int wdlen;
   static int   buflen = 0;
   static char buf[MAX_LINESIZE]/*KR-6/22/97 = 0*/;

/* ignore spaces at start of line */
while(isspace(*cp)) cp++;


/* find start of word */
   inwd = !isspace(*cp);
   while (*cp != '\0' && (num > 0 || !inwd)) {
      if (inwd == !!(isspace(*cp))) {              /* entered or exited a word */
         inwd = !inwd;
         if (inwd)                          /* if entered a word, count it */
            if (--num == 0)
               break;

      }
      cp++;                                              /* next character */
   }

/* handle underflow */
   if (*cp == '\0')
      return 0;

/* find end of word */
   cp2 = cp + 1;
   while (!isspace(*cp2)) cp2++;
   wdlen = cp2 - cp;
   if (wdlen>MAX_LINESIZE)
   {
     fprintf(stderr, "\n  Error: Data Buffer too small\n");
     FatalError(0);
   }
   /*KR-6/20/97   if (buflen <= wdlen) {
      buf = Srealloc(buf, wdlen + 10);
      buflen = wdlen + 10;
   }KR-6/20/97*/
   memcpy(buf, cp, wdlen);
   buf[wdlen] = '\0';
   return buf;
} /* GetWord() */

