//---------------------------------------------------------------------------
#ifndef stsUtilityFunctionsH
#define stsUtilityFunctionsH
//---------------------------------------------------------------------------
#include <io.h>

bool File_Exists(char *filename);
bool FileCanBeCreated(char *filename);
bool ValidateFileExists(AnsiString szFileName, char *Description);
bool ValidateFileCanCreate(AnsiString szFileName, char *Description);
//---------------------------------------------------------------------------
#endif
