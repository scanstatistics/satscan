/* basic.h */

#ifndef _BASIC_H
#define _BASIC_H


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <time.h>

#ifndef TRUE
  #ifndef FALSE
    #ifndef BOOL
      #define BOOL unsigned int
      #define TRUE   1
      #define FALSE  0
    #endif
  #endif
#endif

#define MAX_STR_LEN 150//150

/*#define PARAMFILENAME "STSDOS.TXT"*/
#define PARAMFILENAME "STSPARAM.PRM"

enum { STRING, INTEGER, ENUM, FILENAME, DATE, REPLICA };

enum { NONE, YEAR=1, MONTH, DAY };

typedef unsigned int UInt;

#endif
