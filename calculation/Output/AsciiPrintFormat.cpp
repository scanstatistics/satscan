//***************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//***************************************************************************
#include "AsciiPrintFormat.h"
#include "Toolkit.h"
#include "UtilityFunctions.h"

/** width of label with one dataset for cluster section */
const unsigned int AsciiPrintFormat::giOneDataSetClusterLabelWidth   = 22;
/** width of label for summary section with one data set */
const unsigned int AsciiPrintFormat::giOneDataSetSummuaryLabelWidth  = 25;
/** width of label for cluster section with multiple datasets */
const unsigned int AsciiPrintFormat::giMultiDataSetClusterLabelWidth = 32;
/** width of label for summary section with multiple datasets */
const unsigned int AsciiPrintFormat::giMultiDataSetSummaryLabelWidth = 38;
/** width of label for run-time components section */
const unsigned int AsciiPrintFormat::giRunTimeComponentsLabelWidth   = 35;
/** width of output area - this value is untested below 64 */
const unsigned int AsciiPrintFormat::giRightMargin                   = 64;
/** width of version header section */
const unsigned int AsciiPrintFormat::giVersionHeaderWidth            = 29;
/** text appended to label for multiple dataset */
const char * AsciiPrintFormat::gsPerDataSetText                      = "per data set";

/** constructor */
AsciiPrintFormat::AsciiPrintFormat(bool bOneDataSet) : gbOneDataSet(bOneDataSet) {
  SetMarginsAsOverviewSection();
}

/** destructor */
AsciiPrintFormat::~AsciiPrintFormat() {}

/** Prints data supplied by sDataString parameter to file in a manner
    such that data is aligned with left margin of section label and wraps when
    data will exceed right margin.
    NOTE: It is assumed that the corresponding section label has been printed to
          file, so that the first line of data string is printed to the
          (giRightMargin - giDataLeftMargin)'th character beyond beginning of a
          line. Subsequent lines that wrap do incorporate appropriate padding of
          blanks. */
void AsciiPrintFormat::PrintAlignedMarginsDataString(FILE* fp, std::string& sDataString, unsigned int iPostNewlines) const {
  unsigned int  iStart, iScan, iPrint, iDataPrintWidth;

  iStart = 0;
  //calculate number of characters in the data print area
  iDataPrintWidth = giRightMargin - giDataLeftMargin;
  //if data string is wider than print area then cause it to wrap
  while (iStart + iDataPrintWidth < sDataString.size()) {
      //scan backwards from iStart + iDataPrintWidth, looking for blank to replace
      iScan = iStart + iDataPrintWidth;
      while (iScan > iStart) {
           if (sDataString[iScan] == ' ' || sDataString[iScan] == '\n') {
             //found insertion point - first print characters up to iScan
             for (iPrint=iStart; iPrint < iScan; ++iPrint)
               putc(sDataString[iPrint], fp);
             //print newline character - wrap
             putc('\n', fp);
             //pad with blanks to align data
             for (iPrint=0; iPrint < giDataLeftMargin; ++iPrint)
               putc(' ', fp);
             iStart = iScan + 1/*replaced blank character*/;
             break;
           }
           iScan--;
      }
      //no blank found, so cause data to wrap at right margin
      if (iScan == iStart) {
        //print characters up to iDataPrintWidth
        for (iPrint=iStart; iPrint < iStart + iDataPrintWidth; ++iPrint)
           putc(sDataString[iPrint], fp);
        //print newline - wrap
        putc('\n', fp);
        //pad with blanks to align data
        for (iPrint=0; iPrint < giDataLeftMargin; ++iPrint)
           putc(' ', fp);
        iStart += iDataPrintWidth;
      }
  }
  //print remaining characters of sDataString
  for (iPrint=iStart; iPrint < sDataString.size(); ++iPrint)
     putc(sDataString[iPrint], fp);
  //append newlines as requested
  while (iPostNewlines-- > 0)
     putc('\n', fp);
}

/** Prints section label to file stream. */
void AsciiPrintFormat::PrintSectionLabel(FILE* fp, const char * sText, bool bDataSetParticular, bool bPadLeftMargin) const {
  unsigned int   iStringLength, iFillLength, iPad=0;

  iStringLength = 0;
  //add left margin spacing if requested
  while (bPadLeftMargin && iPad++ < giLeftMargin) {
       putc(' ', fp);
       ++iStringLength;
  }
  //add label
  iStringLength += fprintf(fp, sText);
  //add 'per data set' text if requested and there is more than one dataset
  if (bDataSetParticular && !gbOneDataSet)
    iStringLength += fprintf(fp, " %s", gsPerDataSetText);
  //check that created label isn't greater than defined maximum width of label
  if (iStringLength > (bPadLeftMargin ? giLabelWidth + giLeftMargin : giLabelWidth))
    ZdGenerateException("Label text has length of %u, but defined max length is %u.\n", "PrintSectionLabel()",
                        iStringLength, (bPadLeftMargin ? giLabelWidth + giLeftMargin : giLabelWidth));
  //calculate fill length
  iFillLength = (bPadLeftMargin ? giLeftMargin + giLabelWidth : giLabelWidth);
  //fill remaining label space with '.'
  while (iStringLength < iFillLength)
       iStringLength += fprintf(fp, ".");
  //append label colon
  iStringLength += fprintf(fp, ": ");
}

/** Prints section label to file stream, but starts label text at data column. */
void AsciiPrintFormat::PrintSectionLabelAtDataColumn(FILE* fp, const char* sText, unsigned int iPostNewlines) const {
  unsigned int  iStringLength=0, iPad=0;

  //add left margin spacing til data column left margin
  while (iPad++ < giDataLeftMargin) {
       putc(' ', fp);
       ++iStringLength;
  }
  //add label
  iStringLength += fprintf(fp, sText);
  //check that created label fits in data section
  if (iStringLength > giRightMargin)
    ZdGenerateException("Label text extended beyond defined max length is %u.\n", "PrintSectionLabelAtDataColumn()", giRightMargin);
  //append newlines as requested
  while (iPostNewlines-- > 0)
     putc('\n', fp);
}

/** Prints character cSeparator giRightMargin'th times. Prefixes/postfixes separator
    string newline character as specified by parameters, respectively. */
void AsciiPrintFormat::PrintSectionSeparatorString(FILE* fp, unsigned int iPreNewlines, unsigned int iPostNewlines, char cSeparator) {
  unsigned int iPrint = 0;

  while (iPreNewlines-- > 0)
     putc('\n', fp);

  while (iPrint++ < giRightMargin)
     putc(cSeparator, fp);

  while (iPostNewlines-- > 0)
     putc('\n', fp);
}

/** Prints version header to file stream */
void AsciiPrintFormat::PrintVersionHeader(FILE* fp) {
  unsigned int  iSeparatorsMargin, iTextMargin, iPrint;
  std::string   buffer;

  //calculate padding to center separators
  iSeparatorsMargin = (giRightMargin - giVersionHeaderWidth)/2;

  iPrint=0;
  while (iPrint++ < iSeparatorsMargin)
     putc(' ', fp);
  iPrint=0;
  while (iPrint++ < giVersionHeaderWidth)
     putc('_', fp);
  fprintf(fp, "\n\n");

  printString(buffer, "SaTScan v%s", AppToolkit::getToolkit().GetVersion());
  iTextMargin = (giRightMargin - buffer.size())/2;
  iPrint=0;
  while (iPrint++ < iTextMargin)
     putc(' ', fp);
  fprintf(fp, "%s\n", buffer.c_str());

  iPrint=0;
  while (iPrint++ < iSeparatorsMargin)
     putc(' ', fp);
  iPrint=0;
  while (iPrint++ < giVersionHeaderWidth)
     putc('_', fp);
  fprintf(fp, "\n\n");
}

/** Adjusts left margin give width of iNumber in text. */
void AsciiPrintFormat::SetMarginsAsClusterSection(unsigned int iNumber) {
  giLeftMargin=2;
  int n = (int)floor(((double)iNumber)/10);
  while (n > 0) {
      ++giLeftMargin;
      n = (int)floor(((double)n)/10);
  }
  //set margin for data print
  giDataLeftMargin = (gbOneDataSet ? giOneDataSetClusterLabelWidth : giMultiDataSetClusterLabelWidth) + giLeftMargin + strlen(": ");
  giLabelWidth = (gbOneDataSet ? giOneDataSetClusterLabelWidth : giMultiDataSetClusterLabelWidth);
}

/** Adjusts margins for run overview section. The overview section contains
    no labels, only text. */
void AsciiPrintFormat::SetMarginsAsOverviewSection() {
  //no labels in overview section - purely wrapping text
  giLeftMargin = giDataLeftMargin = giLabelWidth = 0;
}

/** Adjusts margins for run-time components section. */
void AsciiPrintFormat::SetMarginsAsRunTimeReportSection() {
  giLeftMargin = 2;
  giLabelWidth = giRunTimeComponentsLabelWidth;
  giDataLeftMargin = giLabelWidth + giLeftMargin + strlen(": ");
}

/** Adjusts margins for summary of data section. The left margin is zero and the
    label width and data margins are calculated. */
void AsciiPrintFormat::SetMarginsAsSummarySection() {
  giLeftMargin = 0;
  giLabelWidth = (gbOneDataSet ? giOneDataSetSummuaryLabelWidth : giMultiDataSetSummaryLabelWidth);
  giDataLeftMargin = giLabelWidth + giLeftMargin + strlen(": ");
}

