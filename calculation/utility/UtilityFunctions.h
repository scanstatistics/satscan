//******************************************************************************
#ifndef __UTILITYFUNCTIONS_H
#define __UTILITYFUNCTIONS_H
//******************************************************************************
#include "SaTScan.h"

void 	                        ConvertFromLatLong(double Latitude, double Longitude, std::vector<double>& vCoordinates);
std::pair<double, double>       ConvertToLatLong(const std::vector<double>& vCoordinates);
double                          CalculateNonCompactnessPenalty(double dEllipseShape, double dPower);
const char                    * GetDatePrecisionAsString(DatePrecisionType eType, std::string& sString, bool bPlural=true, bool bCapitalizeFirstLetter=false);
unsigned int                    GetNumSystemProcessors();
void                            ReportTimeEstimate(boost::posix_time::ptime StartTime, int nRepetitions, int nRepsCompleted, BasePrint *pPrintDirection);
boost::posix_time::ptime        GetCurrentTime_HighResolution();
double                          GetUnbiasedVariance(count_t tObservations, measure_t tSumMeasure, measure_t tSumMeasureAux);
double                          GetUnbiasedVariance(count_t tCases, measure_t tMeasure, measure_t tMeasureAux, count_t tTotalCases, measure_t tTotalMeasure, measure_t tTotalMeasureAux);
bool                            ValidateFileAccess(const std::string& filename, bool bWriteEnable=false);
void                            trimString(std::string &source, const char * t=" ");
std::string                   & lowerString(std::string &source); 
std::string                   & printString(std::string& s, const char * format, ...);
unsigned int                    getFormatPrecision(double value, unsigned int iSignificant=2);
std::string                   & getValueAsString(double value, std::string& s, unsigned int iSignificant=2);
std::string                   & GetUserDocumentsDirectory(std::string& s, const std::string& defaultPath);
//******************************************************************************
#endif

