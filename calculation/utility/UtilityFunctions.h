//*****************************************************************************
#ifndef __UTILITYFUNCTIONS_H
#define __UTILITYFUNCTIONS_H
//*****************************************************************************
#include "SaTScan.h"

#define DEBUGLATLONG 0

double 			CalcLogLikelihood(count_t n, measure_t u, count_t N, measure_t U);
void 			ConvertFromLatLong(float Latitude, float Longitude, std::vector<double>& vCoordinates);
void 			ConvertToLatLong(float* Latitude, float* Longitude, double* pCoords);
void 			DisplayVersion(FILE* fp, int nPos);
double                  GetDuczmalCorrection(double dEllipseShape);
const char	      * GetWord(const char *s, int num, BasePrint *pPrintDirection);

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
    const ZdString    & gsSource;
    char              * gpWord;
    int                 giSizeOfWordBuffer;
    short               gwCurrentWordIndex;

  public:
    StringParser(const ZdString & sSource);
    ~StringParser();

    void                ClearWordIndex() {gwCurrentWordIndex=-1;}
    bool                HasWords();
    int                 GetNumberWords();
    const char        * GetWord(short wWordIndex);
    const char        * GetString() const {return gsSource.GetCString();}
    void                StringReloaded();
};
//*****************************************************************************
#endif 
