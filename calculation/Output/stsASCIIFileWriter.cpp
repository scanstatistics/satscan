//***************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//***************************************************************************
#include "stsASCIIFileWriter.h"
//#include "AbstractDataFileWriter.h"
#include "stsLogLikelihood.h"

const char * ASCIIFileWriter::ASCII_FILE_EXT		= ".txt";

// constructor
ASCIIFileWriter::ASCIIFileWriter(BaseOutputStorageClass& OutputFileData, BasePrint& PrintDirection,
                                 const CParameters& Parameters, bool bAppend)
                 : OutputFileWriter(OutputFileData, PrintDirection) {
  try {
    Setup(Parameters, bAppend);
    Print();
  }
  catch (ZdException &x) {
    gPrintDirection.SatScanPrintWarning(x.GetErrorMessage());
    gPrintDirection.SatScanPrintWarning("\nWarning - Unable to create the ASCII output file.\n");
  }
}

/** class destructor */
ASCIIFileWriter::~ASCIIFileWriter() {}

// creates a blank string of the length of the specified field
// pre: 0 <= iFieldNumber < vFields.size()
// post: sFormatString conatins field length number of spaces
void ASCIIFileWriter::CreateBlankString(ZdString& sFormatString, int iFieldNumber) {
   try {
      const ZdField* pField = gOutputFileData.GetField(iFieldNumber);

      sFormatString << "n/a";
      while (sFormatString.GetLength() < static_cast<unsigned long>(pField->GetLength()))
           sFormatString << " ";
   }
   catch (ZdException &x) {
      x.AddCallpath("CreateBlankString()", "ASCIIFileWriter");
      throw;
   }
}

// creates the formatted string from the precision and type of the field value and stores the formatted
//  output value in sValue
// pre : none
// post : formats the string sValue based upon the settings of fieldValue
void ASCIIFileWriter::CreateFormatString(ZdString& sValue, const int iFieldNumber, const ZdFieldValue& fv) {
   ZdString             sFormat, sTemp;
   unsigned long        ulStringLength = 0;
   const ZdField      * pField = 0;

   try {
      sFormat << "%";
      pField = gOutputFileData.GetField(iFieldNumber);

      switch(fv.GetType()) {
         case ZD_ALPHA_FLD :
            sTemp = fv.AsZdString();
            ulStringLength = sTemp.GetLength();
            if (ulStringLength < (unsigned long)pField->GetLength())
               for(int i = sTemp.GetLength(); i < pField->GetLength() + 1; ++i)
                  sTemp << " ";
            else if(ulStringLength > (unsigned long)pField->GetLength())
               sTemp.Truncate(pField->GetLength());
            sValue << sTemp;
            break;
         case ZD_NUMBER_FLD :
            sFormat << "-0" << pField->GetLength() << "." << pField->GetPrecision() << "f";
            sValue.printf(sFormat.GetCString(), fv.AsDouble());
            break;
         default :
            ZdGenerateException("Unsupported field type %c", "Error!", fv.GetType());
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

// prints the data from the global output file data pointer to the
// dBase file      
void ASCIIFileWriter::Print() {
   ZdString             sFormatString;
   const OutputRecord * pRecord = 0;
   FILE*                pFile = 0;

   try {
      if ((pFile = fopen(gsFileName, "a")) == NULL)
         ZdGenerateException("Unable to open/create file %s", "Error!", gsFileName.GetCString());
      for(unsigned int i = 0; i < gOutputFileData.GetNumRecords(); ++i) {
         pRecord = gOutputFileData.GetRecord(i);
         for(unsigned int j = 0; j < pRecord->GetNumFields(); ++j) {
            sFormatString << ZdString::reset;
            if (!pRecord->GetFieldIsBlank(j))
               CreateFormatString(sFormatString, j, pRecord->GetFieldValue(j));
            else
               CreateBlankString(sFormatString, j);
            fprintf(pFile, "%s ", sFormatString.GetCString());
         }
         fprintf(pFile, "\n");
      }
      fclose(pFile); pFile = 0;
   }
   catch (ZdException &x) {
      fclose(pFile);
      gPrintDirection.SatScanPrintWarning(x.GetErrorMessage());
      gPrintDirection.SatScanPrintWarning("\nWarning - Unable to write to ASCII file: %s.\n", gsFileName.GetCString());
   }
}

// setup
void ASCIIFileWriter::Setup(const CParameters& Parameters, bool bAppend) {
   SetOutputFileName(Parameters.GetOutputFileName().c_str(), ASCII_FILE_EXT);
   if (!bAppend)
     CreateOutputFile();
}






const char * ASCIIDataFileWriter::ASCII_FILE_EXT		= ".txt";

// constructor
ASCIIDataFileWriter::ASCIIDataFileWriter(const CParameters& Parameters, const char * sFileExtension, bool bAppend)
                    :DataFileWriter() {
  try {
    Setup(Parameters, sFileExtension, bAppend);
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor()","ASCIIDataFileWriter");
    throw;
  }
}

/** class destructor */
ASCIIDataFileWriter::~ASCIIDataFileWriter() {
  try {
    fclose(gpFile);
  }
  catch (...){}
}

// creates a blank string of the length of the specified field
// pre: 0 <= iFieldNumber < vFields.size()
// post: sFormatString conatins field length number of spaces
void ASCIIDataFileWriter::CreateBlankString(ZdString& sFormatString, const ZdField& FieldDef) {
  try {
    sFormatString << "n/a";
    while (sFormatString.GetLength() < static_cast<unsigned long>(FieldDef.GetLength()))
         sFormatString << " ";
  }
  catch (ZdException &x) {
    x.AddCallpath("CreateBlankString()","ASCIIDataFileWriter");
    throw;
  }
}

// creates the formatted string from the precision and type of the field value and stores the formatted
//  output value in sValue
// pre : none
// post : formats the string sValue based upon the settings of fieldValue
void ASCIIDataFileWriter::CreateFormatString(ZdString& sValue, const ZdField& FieldDef, const ZdFieldValue& fv) {
  ZdString             sFormat, sTemp;
  unsigned long        ulStringLength = 0;

  try {
    sFormat << "%";
    switch(fv.GetType()) {
      case ZD_ALPHA_FLD  : sTemp = fv.AsZdString();
                           ulStringLength = sTemp.GetLength();
                           if (ulStringLength < (unsigned long)FieldDef.GetLength())
                           for (int i = sTemp.GetLength(); i < FieldDef.GetLength() + 1; ++i)
                              sTemp << " ";
                           else if(ulStringLength > (unsigned long)FieldDef.GetLength())
                              sTemp.Truncate(FieldDef.GetLength());
                           sValue << sTemp;
                           break;
      case ZD_NUMBER_FLD : sFormat << "-0" << FieldDef.GetLength() << "." << FieldDef.GetPrecision() << "f";
                           sValue.printf(sFormat.GetCString(), fv.AsDouble());
                           break;
      default : ZdGenerateException("Unsupported field type %c", "Error!", fv.GetType());
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("CreateFormatString()", "ASCIIDataFileWriter");
    throw;
  }
}
// checks to see if file already exists, if it does delete it for overwrite
// pre : none
// post : if file already existed, deletes the file
void ASCIIDataFileWriter::CreateOutputFile() {
   try {
      if(ZdIO::Exists(gsFileName.GetFullPath()))
        ZdIO::Delete(gsFileName.GetFullPath());
   }
   catch (ZdException &x) {
      x.AddCallpath("CreateOutputFile()", "ASCIIDataFileWriter");
      throw;
   }
}

// prints the data from the global output file data pointer to the
// dBase file
void ASCIIDataFileWriter::WriteRecord(const ZdPointerVector<ZdField>& vFieldDefs, const RecordBuffer& Record) {
  ZdString             sFormatString;

  try {
    for (unsigned int j=0; j < Record.GetNumFields(); ++j) {
      sFormatString << ZdString::reset;
      if (!Record.GetFieldIsBlank(j))
        CreateFormatString(sFormatString, *vFieldDefs[j], Record.GetFieldValue(j));
      else
        CreateBlankString(sFormatString, *vFieldDefs[j]);
      fprintf(gpFile, "%s ", sFormatString.GetCString());
    }
    fprintf(gpFile, "\n");
  }
  catch (ZdException &x) {
    x.AddCallpath("WriteRecord()","ASCIIDataFileWriter");
    throw;
  }
}

// setup
void ASCIIDataFileWriter::Setup(const CParameters& Parameters, const char * sFileExtension, bool bAppend) {
   ZdString sExt(sFileExtension);

   sExt << ASCII_FILE_EXT;
   SetOutputFileName(Parameters.GetOutputFileName().c_str(), sExt.GetCString());

   if (!bAppend)
     CreateOutputFile();
   if ((gpFile = fopen(gsFileName.GetFullPath(), "a")) == NULL)
     ZdGenerateException("Unable to open/create file %s", "Setup()", gsFileName.GetFullPath());
}


