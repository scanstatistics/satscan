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
double                          GetUnbiasedVariance(count_t tObservations, measure_t tSumMeasure, measure_t tSumSqMeasure);
double                          GetUnbiasedVariance(count_t tCases, measure_t tMeasure, measure_t tSqMeasure, count_t tTotalCases, measure_t tTotalMeasure, measure_t tTotalSqMeasure);
bool                            ValidateFileAccess(const std::string& filename, bool bWriteEnable=false);
void                            trimString(std::string &source, const std::string &t=" ");
std::string                   & printString(std::string& s, const char * format, ...);
std::string                   & printStringArgs(std::string& s, va_list varArgs, const char * format);
//******************************************************************************
#endif

