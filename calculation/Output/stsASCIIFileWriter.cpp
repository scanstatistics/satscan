// Adam J Vaughn
// November 2002
//
// This class object is used to print out an OutputFileData type in ASCII format.

#include "SaTScan.h"
#pragma hdrstop

#include "stsASCIIFileWriter.h"

const char *	ASCII_FILE_EXT		= ".txt";

// constructor
ASCIIFileWriter::ASCIIFileWriter(BaseOutputStorageClass* pOutputFileData)
                 : OutputFileWriter(pOutputFileData) {
   try {
      Init();
      Setup();
   }
   catch (ZdException &x) {
      x.AddCallpath("Constructor", "ASCIIFileWriter");
      throw;
   }
}

ASCIIFileWriter::~ASCIIFileWriter() {
}

// creates the formatted string from the precision and type of the field value and stores the formatted
//  output value in sValue
// pre : none
// post : formats the string sValue based upon the settings of fieldValue
void ASCIIFileWriter::CreateFormatString(ZdString& sValue, const int iFieldNumber, const ZdFieldValue& fv) {
   ZdString             sFormat, sTemp;
   unsigned long        ulStringLength = 0;
   ZdField*             pField = 0;

   try {
      sFormat << "%";

      pField = gpOutputFileData->GetField(iFieldNumber);

      switch(fv.GetType()) {
         case ZD_ALPHA_FLD :
            ulStringLength = fv.AsZdString().GetLength();
            sTemp = fv.AsZdString();
            if (ulStringLength < (unsigned long)pField->GetLength())
               for(int i = sTemp.GetLength(); i < pField->GetLength() + 1; ++i)
                  sTemp << " ";
            else if(ulStringLength > (unsigned long)pField->GetLength())
               sTemp.Truncate(pField->GetLength());
            sValue << sTemp;
            break;
         case ZD_NUMBER_FLD :
            sFormat << "-0" << pField->GetLength() << "." << pField->GetPrecision();
            sFormat << "f";
            sValue.printf(sFormat.GetCString(), fv.AsDouble());
            break;
         default :
            ZdGenerateException("Unsupported field type %c", "Error!", pField->GetType());
      }
   }
   catch (ZdException &x) {
      x.AddCallpath("CreateFormatString()", "ASCIIFileWriter");
      throw;
   }
}
// checks to see if file already exists, if it does delete it for overwrite
// pre : none
// post : if file already existed, deletes the file
void ASCIIFileWriter::CreateOutputFile() {
   try {
      if(ZdIO::Exists(gsFileName))
        ZdIO::Delete(gsFileName);
   }
   catch (ZdException &x) {
      x.AddCallpath("CreateOutputFile()", "ASCIIFileWriter");
      throw;
   }
}

// initialize global variables
void ASCIIFileWriter::Init() {
}

// prints the data from the global output file data pointer to the
// dBase file      
void ASCIIFileWriter::Print() {
   ZdString             sFormatString;
   BaseOutputRecord*    pRecord = 0;
   FILE*                pFile = 0;

   try {
      if ((pFile = fopen(gsFileName, "w")) == NULL)
         ZdGenerateException("Unable to open/create file %s", "Error!", gsFileName);
      for(unsigned long i = 0; i < gpOutputFileData->GetNumRecords(); ++i) {
         pRecord = gpOutputFileData->GetRecord(i);
         for(unsigned short j = 0; j < pRecord->GetNumFields(); ++j) {
            sFormatString << ZdString::reset;
            CreateFormatString(sFormatString, j, pRecord->GetValue(j));
            fprintf(pFile, "%s ", sFormatString.GetCString());
         }
         fprintf(pFile, "\n");
      }
      fclose(pFile); pFile = 0;
   }
   catch (ZdException &x) {
      fclose(pFile);
      x.AddCallpath("Print()", "ASCIIFileWriter");
      throw;
   }
}

// setup
void ASCIIFileWriter::Setup() {
   gsFileName = gpOutputFileData->GetFileName();
   gsFileName << ASCII_FILE_EXT;
   CreateOutputFile();
}

   
