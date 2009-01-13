//***************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//***************************************************************************
#include "stsASCIIFileWriter.h"
#include "AbstractDataFileWriter.h"

const char * ASCIIDataFileWriter::ASCII_FILE_EXT = ".txt";

/** constructor */
ASCIIDataFileWriter::ASCIIDataFileWriter(const CParameters& Parameters, const std::string& sFileExtension, bool bAppend) {
  try {
    Setup(Parameters, sFileExtension, bAppend);
  }
  catch (prg_exception& x) {
    x.addTrace("constructor()","ASCIIDataFileWriter");
    throw;
  }
}

/** destructor */
ASCIIDataFileWriter::~ASCIIDataFileWriter() {
  try {
    if (gpFile) fclose(gpFile);
  }
  catch (...){}
}

// creates a blank string of the length of the specified field
// pre: 0 <= iFieldNumber < vFields.size()
// post: sFormatString conatins field length number of spaces
void ASCIIDataFileWriter::CreateBlankString(std::string& sFormatString, const FieldDef& Field) {
  sFormatString += "n/a";
  while (sFormatString.size() < static_cast<unsigned long>(Field.GetLength()))
       sFormatString += " ";
}

// creates the formatted string from the precision and type of the field value and stores the formatted
//  output value in sValue
// pre : none
// post : formats the string sValue based upon the settings of fieldValue
void ASCIIDataFileWriter::CreateFormatString(std::string& sValue, const FieldDef& FieldDef, const FieldValue& fv) {
  std::string          sFormat, sTemp;
  unsigned long        ulStringLength = 0;

  switch(fv.GetType()) {
    case FieldValue::ALPHA_FLD  :
      sTemp = fv.AsString();
      ulStringLength = sTemp.size();
      if (ulStringLength < (unsigned long)FieldDef.GetLength())
        for (int i = sTemp.size(); i < FieldDef.GetLength() + 1; ++i)
           sTemp += " ";
      else if(ulStringLength > (unsigned long)FieldDef.GetLength())
        sTemp.resize(FieldDef.GetLength());
      sValue += sTemp;
      break;
    case FieldValue::NUMBER_FLD :
        sTemp = getValueAsString(fv.AsDouble(), sTemp ,FieldDef.GetAsciiDecimals());
        if (sTemp.size() > static_cast<size_t>(FieldDef.GetLength())) {
            printString(sFormat, "%%-0%dg", FieldDef.GetLength());
            printString(sValue, sFormat.c_str(), fv.AsDouble());
        }
        else {
            if (sTemp.size() < static_cast<size_t>(FieldDef.GetLength()))
                for (int i = sTemp.size(); i < FieldDef.GetLength() + 1; ++i)
                    sTemp += " ";
            sValue = sTemp;
        }
      break;
    default : throw prg_error("Unsupported field type %c", "Error!", fv.GetType());
  }
}

/** Writes record buffer to file stream. */
void ASCIIDataFileWriter::WriteRecord(const RecordBuffer& Record) {
  std::string             sFormatString;

  for (unsigned int j=0; j < Record.GetNumFields(); ++j) {
    sFormatString.clear();
    if (!Record.GetFieldIsBlank(j))
      CreateFormatString(sFormatString, Record.GetFieldDefinition(j), Record.GetFieldValue(j));
    else
      CreateBlankString(sFormatString, Record.GetFieldDefinition(j));
    fprintf(gpFile, "%s ", sFormatString.c_str());
  }
  fprintf(gpFile, "\n");
}

/** internal setup - opens file stream for writing */
void ASCIIDataFileWriter::Setup(const CParameters& Parameters, const std::string& sFileExtension, bool bAppend) {
   std::string  buffer, ext(sFileExtension);

  try {
    ext += ASCII_FILE_EXT;
    gsFileName.setFullPath(Parameters.GetOutputFileName().c_str());
    gsFileName.setExtension(ext.c_str());

    if ((gpFile = fopen(gsFileName.getFullPath(buffer).c_str(), (bAppend ? "a" : "w"))) == NULL)
      throw resolvable_error("Unable to open/create file %s", "Setup()", gsFileName.getFullPath(buffer).c_str());
  }
  catch (prg_exception& x) {
    x.addTrace("Setup()","ASCIIDataFileWriter");
    throw;
  }
}

