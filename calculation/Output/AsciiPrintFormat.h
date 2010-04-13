//***************************************************************************
#ifndef __AsciiPrintFormat_H
#define __AsciiPrintFormat_H
//***************************************************************************
/** Print formatter for aiding in the process of creating the primary ASCII
    results output file.

    NOTE: The values of the static variables are somewhat determined
          empirically. The code has been written to be tested to be adjustable
          only in repects to expanding the width of the output area. */
class AsciiPrintFormat {
   private:
     unsigned int               giLeftMargin;
     unsigned int               giDataLeftMargin;
     unsigned int               giLabelWidth;
     bool                       gbOneDataSet;
     static const char *        gsPerDataSetText;
     static const unsigned int  giOneDataSetClusterLabelWidth;
     static const unsigned int  giOneDataSetSummuaryLabelWidth;
     static const unsigned int  giMultiDataSetClusterLabelWidth;
     static const unsigned int  giMultiDataSetSummaryLabelWidth;
     static const unsigned int  giRunTimeComponentsLabelWidth;
     static const unsigned int  giRightMargin;
     static const unsigned int  giVersionHeaderWidth;

     static void                putChar(char c, FILE* fp);

   public:
     AsciiPrintFormat(bool bOneDataSet=true);
     virtual ~AsciiPrintFormat();

     void                       PrintAlignedMarginsDataString(FILE* fp, std::string& sDataString, unsigned int iPostNewlines=1) const;
     void                       PrintNonRightMarginedDataString(FILE* fp, std::string& sDataString, bool bPadLeftMargin, unsigned int iPostNewlines=1) const;
     void                       PrintSectionLabel(FILE* fp, const char* sText, bool bDataSetParticular, bool bPadLeftMargin) const;
     void                       PrintSectionLabelAtDataColumn(FILE* fp, const char* sText, unsigned int iPostNewlines=1) const;
     static void                PrintSectionSeparatorString(FILE* fp, unsigned int iPreNewlines=0, unsigned int iPostNewlines=1, char cSeparator='_');
     static void                PrintVersionHeader(FILE* fp);
     void                       SetMarginsAsClusterSection(unsigned int iNumber);
     void                       SetMarginsAsOverviewSection();
     void                       SetMarginsAsRunTimeReportSection();
     void                       SetMarginsAsSummarySection();
};
//***************************************************************************
#endif
