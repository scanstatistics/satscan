// error.cpp

#include <stdio.h>
#include <stdlib.h>
#include "error.h"
#include "display.h"

//*********************************************************************
// Display error message based on error number.
// Also display "Program will be canceled" message.
// ReportError is intended to be used for Fatal Errors after
// which the program will be exited.
//*********************************************************************
/*void ReportError(int nError)
{
  switch(nError)
  {
    case INVALIDNUMBEROFARGUMENTS  :
      printf("ERROR 1: Invalid Number of Arguments on Command Line.\n"); break;
    case UNABLETOOPENPARAMETERFILE :
      printf("ERROR 2: Unable to Open Paramter File.\n");                break;
    case INVALIDANALYSISTYPE       :
      printf("ERROR 3: Invalid Analysis Type in Paramter FIle.\n");      break;
    case OUTOFMEMORY               :
      printf("ERROR 4: Unable to allocate required memory.\n");          break;
    case MAXCLUSTERS               :
      printf("ERROR 5: Attempt to access more clusters than allowed.\n");break;
    case DATAERROR                 :
      printf("ERROR 6: Problem encountered reading in data.\n");         break;
    default                        :
      printf("Unknown Error Encountered.\n");
  }

  printf("PROGRAM WILL BE CANCELED.\n");
  HoldForEnter("Press <Enter> to exit program.");
}
*/

//*********************************************************************
// ReportWarning is intended to be used for errors that will not
// halt program execution.
//*********************************************************************
/*void ReportWarning(int nWarning)
{
  switch(nWarning)
  {
    default                        :
      printf("Unknown Warning Encountered\n");
  }

}
*/

/**********************************************************************
 Prints an error message to stdout, then exits the program with
 value 1.
 **********************************************************************/
void FatalError(char *s)
{
   char c;

	if (s)
		fprintf(stderr, s);

   printf("\nPress <Enter> to exit the SaTScan program.");
   c = getc(stdin);

	exit(1);
} /* FatalError() */


