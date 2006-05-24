//******************************************************************************
#ifndef __UTILITYFUNCTIONS_H
#define __UTILITYFUNCTIONS_H
//******************************************************************************
#include "SaTScan.h"

void 	                        ConvertFromLatLong(double Latitude, double Longitude, std::vector<double>& vCoordinates);
std::pair<double, double>       ConvertToLatLong(const std::vector<double>& vCoordinates);
double                          CalculateNonCompactnessPenalty(double dEllipseShape, double dPower);
const char                    * GetDatePrecisionAsString(DatePrecisionType eType, ZdString& sString, bool bPlural=true, bool bCapitalizeFirstLetter=false);
unsigned int                    GetNumSystemProcessors();
void                            ReportTimeEstimate(boost::posix_time::ptime StartTime, int nRepetitions, int nRepsCompleted, BasePrint *pPrintDirection);
boost::posix_time::ptime        GetCurrentTime_HighResolution();
double                          GetUnbiasedVariance(count_t tObservations, measure_t tSumMeasure, measure_t tSumSqMeasure);

/** The data read routines of CSaTScanData used to use function:
    const char * GetWord(const char *s, int num, BasePrint *pPrintDirection)
    but it relied on a static character array to store word and use as return
    value. In a threaded enviroment, this could be trouble some.
    This class wraps that functionality into an object.

    Note: The routines could be designed to read from a ZdFile interface,
          permitting direct reading from dBase or any other file type driver
          we want to create. The reason this has not been implemented is:
          1 - the overhead might cause an overall increase in time for data read rountines
          2 - no one has asked for direct read for other file types
          3 - there may be other concerns/problems as this hasn't been completely thought through */
class StringParser {
  private:
    static const int            MAX_LINESIZE = 4096; /** Maximum length of a line in input file (scanf). */
    static const int            MAX_LINEITEMSIZE = 512; /** Maximum string length*/
    char                        gsReadBuffer[MAX_LINESIZE];
    char                      * gpWord;
    int                         giSizeOfWordBuffer;
    short                       gwCurrentWordIndex;
    long                        glReadCount;
    BasePrint                 & gPrint;


    void                        ClearWordIndex() {gwCurrentWordIndex=-1;}
    void                        ThrowAsciiException();
    void                        ThrowUnicodeException();

  public:
    StringParser(BasePrint& Print);
    ~StringParser();

    bool                        HasWords();
    short                       GetNumberWords();
    long                        GetReadCount() const {return glReadCount;}
    const char                * GetString() const {return gsReadBuffer;}
    const char                * GetWord(short wWordIndex);
    const char                * ReadString(FILE * pSourceFile);
    void                        Reset();
};
//******************************************************************************
#endif

