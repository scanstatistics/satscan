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
     mutable unsigned int       giLeftMargin;
     mutable unsigned int       giDataLeftMargin;
     mutable unsigned int       giLabelWidth;
     unsigned int               _label_extra;
     bool                       gbOneDataSet;
     static const char *        gsPerDataSetText;
     static const char *        gsPerDayText;
     static const unsigned int  giOneDataSetClusterLabelWidth;
     static const unsigned int  giOneDataSetSummuaryLabelWidth;
     static const unsigned int  giMultiDataSetClusterLabelWidth;
     static const unsigned int  giMultiDataSetSummaryLabelWidth;
     static const unsigned int  giRunTimeComponentsLabelWidth;
     static const unsigned int  giRightMargin;
     static const unsigned int  giVersionHeaderWidth;

     static void putChar(char c, FILE* fp, unsigned int num = 1);

   public:
     AsciiPrintFormat(bool bOneDataSet=true, unsigned int labelextra=0);
     virtual ~AsciiPrintFormat();

     std::string& getSectionSuffix(std::string& s, bool bernoulliDrilldown) const;
     void PrintAlignedMarginsDataString(FILE* fp, const std::string& sDataString, unsigned int iPostNewlines=1) const;
     void PrintNonRightMarginedDataString(FILE* fp, const std::string& sDataString, bool bPadLeftMargin, unsigned int iPostNewlines=1) const;
     void PrintSectionLabel(FILE* fp, const char* sText, bool bDataSetParticular, bool bPadLeftMargin, bool bernoulliDrilldown=false) const;
     void PrintSummaryEntries(FILE* fp, const std::vector<std::pair<std::string, std::string>>& summaryEntries) const;
     void PrintSectionStatement(FILE* fp, const char* sText, unsigned int iPostNewlines = 1) const;
     static void PrintSectionSeparatorString(FILE* fp, unsigned int iPreNewlines=0, unsigned int iPostNewlines=1, char cSeparator='_');
     static void PrintVersionHeader(FILE* fp);
     static void printPadRight(FILE* fp, const char * s, unsigned int width, char pad=' ');
     void SetMarginsAsClusterSection(unsigned int iNumber);
     void SetMarginsAsOverviewSection();
     void SetMarginsAsRunTimeReportSection();
};
//***************************************************************************
#endif
