//***************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//***************************************************************************
#include "stsASCIIFileWriter.h"
#include "AbstractDataFileWriter.h"

const char * ASCIIDataFileWriter::ASCII_FILE_EXT = ".txt";

/** constructor */
ASCIIDataFileWriter::ASCIIDataFileWriter(const CParameters& Parameters, const ZdString& sFileExtension, bool bAppend) {
  try {
    Setup(Parameters, sFileExtension, bAppend);
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor()","ASCIIDataFileWriter");
    throw;
  }
}

/** destructor */
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

/** Writes record buffer to file stream. */
void ASCIIDataFileWriter::WriteRecord(const RecordBuffer& Record) {
  ZdString             sFormatString;

  try {
    for (unsigned int j=0; j < Record.GetNumFields(); ++j) {
      sFormatString << ZdString::reset;
      if (!Record.GetFieldIsBlank(j))
        CreateFormatString(sFormatString, Record.GetFieldDefinition(j), Record.GetFieldValue(j));
      else
        CreateBlankString(sFormatString, Record.GetFieldDefinition(j));
      fprintf(gpFile, "%s ", sFormatString.GetCString());
    }
    fprintf(gpFile, "\n");
  }
  catch (ZdException &x) {
    x.AddCallpath("WriteRecord()","ASCIIDataFileWriter");
    throw;
  }
}

/** internal setup - opens file stream for writing */
void ASCIIDataFileWriter::Setup(const CParameters& Parameters, const ZdString& sFileExtension, bool bAppend) {
   ZdString sExt(sFileExtension);

  try {
    sExt << ASCII_FILE_EXT;
    gsFileName.SetFullPath(Parameters.GetOutputFileName().c_str());
    gsFileName.SetExtension(sExt.GetCString());

    if ((gpFile = fopen(gsFileName.GetFullPath(), (bAppend ? "a" : "w"))) == NULL)
      ZdGenerateException("Unable to open/create file %s", "Error!", gsFileName.GetFullPath());
  }
  catch (ZdException &x) {
    x.AddCallpath("Setup()","ASCIIDataFileWriter");
    throw;
  }
}


