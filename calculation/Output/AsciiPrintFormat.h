//***************************************************************************
#ifndef AsciiPrintFormatH
#define AsciiPrintFormatH
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
     bool                       gbOneStream;
     static const char *        gsPerStreamText;
     static const unsigned int  giOneStreamLabelWidth;
     static const unsigned int  giMultiStreamClusterLabelWidth;
     static const unsigned int  giMultiStreamSummaryLabelWidth;
     static const unsigned int  giRightMargin;
     static const unsigned int  giVersionHeaderWidth;

   public:
     AsciiPrintFormat(bool bOneStream=true);
     virtual ~AsciiPrintFormat();

     void                       PrintAlignedMarginsDataString(FILE* fp, ZdString& sDataString, unsigned int iPostNewlines=1) const;
     void                       PrintSectionLabel(FILE* fp, const char* sText, bool bStreamParticular, bool bPadLeftMargin) const;
     static void                PrintSectionSeparatorString(FILE* fp, unsigned int iPreNewlines=0, unsigned int iPostNewlines=1);
     static void                PrintVersionHeader(FILE* fp);
     void                       SetMarginsAsClusterSection(unsigned int iNumber);
     void                       SetMarginsAsOverviewSection();
     void                       SetMarginsAsSummarySection();
};
//***************************************************************************
#endif
