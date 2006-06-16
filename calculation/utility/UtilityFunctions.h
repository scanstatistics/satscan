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
//******************************************************************************
#endif

