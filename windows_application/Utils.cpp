//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "Utils.h"

//------------------------------------------------------------------------------
// Does the give file exits ??
//------------------------------------------------------------------------------
bool File_Exists(char *filename)
 {
   return (access(filename, 0) == 0);
 }

//------------------------------------------------------------------------------
// Do you have write priveleges to the give disk
//------------------------------------------------------------------------------
bool FileCanBeCreated(char *filename)
{
   bool bCanCreate = true;

   if (File_Exists(filename))
      bCanCreate = (access(filename, 02) == 0);
   //else
     // MAKE SURE YOU HAVE WRITE PRIVELEDGES TO THAT DIRECTORY !!!!

   return bCanCreate;
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ValidateFileExists(AnsiString szFileName, char *Description)
{
   bool bExists = true;
   AnsiString szTitle, szMessage;

   szTitle = Description;
   szTitle += " File Error";
   szMessage = "File ";
   szMessage += szFileName.c_str();
   szMessage += " could not be opened.";
   if (szFileName.IsEmpty() || ! File_Exists(szFileName.c_str()))
      {
      Application->MessageBox(szMessage.c_str(), szTitle.c_str(), MB_OK);
      bExists = false;
      }

   return bExists;
}
//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
bool ValidateFileCanCreate(AnsiString szFileName, char *Description)
{
   bool bExists = true;
   AnsiString szTitle, szMessage;

   szTitle = Description;
   szTitle += " File Error";
   szMessage = "File ";
   szMessage += szFileName.c_str();
   szMessage += " could not be created.";
   if (szFileName.IsEmpty() || ! FileCanBeCreated(szFileName.c_str()))
      {
      Application->MessageBox(szMessage.c_str(), szTitle.c_str(), MB_OK);
      bExists = false;
      }
   return bExists;
}

