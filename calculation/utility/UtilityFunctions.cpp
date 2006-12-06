//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "UtilityFunctions.h"
#include "SSException.h"
#include <cmath>

// Conversion routines for Latitude/Longitude option for data input
// and output based on the following formulas:
//
//		d = latitude
//		l = longitude
//		a = radius (fixed = 6367 km)
//
//		x = a cos(l) cos(d)
//		y = a sin(l) cos(d)
//		z = a sin(d)
//
//		Latitude and Longitude values should be float, ie, real numbers
//		with ranges [-90,90] and [-180,180] (in degrees) respectively.
void ConvertFromLatLong(double Latitude, double Longitude, std::vector<double>& vCoordinates) {
  double RADIUS = 6367; // Constant; radius of earth in km)

  vCoordinates[0] = (RADIUS * cos(Longitude*PI/180.0) * cos(Latitude*PI/180.0)); // x coordinate
  vCoordinates[1] = (RADIUS * sin(Longitude*PI/180.0) * cos(Latitude*PI/180.0)); // y coordinate
  vCoordinates[2] = (RADIUS * sin(Latitude*PI/180.0));														// z coordinate
}

/** Converts passed coordinates in Cartesian system to latitude/longitude system.
    Returns pair of doubles <latitude, longtitude>. Throws prg_error if number of
    coordinates in passed vector is not 3. */
std::pair<double, double> ConvertToLatLong(const std::vector<double>& vCoordinates) {
  std::pair<double, double>     prLatitudeLongitude;
  double RADIUS = 6367; // Constant; radius of earth in km)

  if (vCoordinates.size() != 3)
    throw prg_error("Conversion to latitude/longitude requires a vector of 3 elements.\n"
                    "Passed vector contains %u elements.","ConvertToLatLong()", vCoordinates.size());

  if (vCoordinates[0] != 0) {
    prLatitudeLongitude.second = atan(vCoordinates[1] / vCoordinates[0]) * 180.0 / (double)PI;
    if (vCoordinates[0] < 0 && vCoordinates[1] > 0)
      prLatitudeLongitude.second += 180.0;
    else if (vCoordinates[0] < 0 && vCoordinates[1] < 0)
      prLatitudeLongitude.second -= 180.0;
  }
  else if (vCoordinates[1] > 0)
    prLatitudeLongitude.second = 90.0;
  else if (vCoordinates[1] < 0)
    prLatitudeLongitude.second = -90.0;
  else if (vCoordinates[1] == 0)
    prLatitudeLongitude.second = 0.0;

  double tmp = sqrt((vCoordinates[0]*vCoordinates[0] + vCoordinates[1]*vCoordinates[1])/(RADIUS*RADIUS));
  prLatitudeLongitude.first = (vCoordinates[2] >= 0 ? (1.0) : (-1.0)) * acos(tmp) * 180.0 / (double)PI;

  return prLatitudeLongitude;
}

/** Return non-compactness penalty coefficient for specified elliptic shape and
    compactness penalty power. */
double CalculateNonCompactnessPenalty(double dEllipseShape, double dPower) {
  return pow((4*dEllipseShape/(pow(dEllipseShape + 1, 2))), dPower);
}

//What is the current time? (UTC | Coordinated Universal Time)
#ifdef INTEL_BASED
boost::posix_time::ptime GetCurrentTime_HighResolution()
{
  using namespace boost::posix_time;
  using namespace boost::gregorian;
  SYSTEMTIME stm;
  GetSystemTime(&stm);
  time_duration::fractional_seconds_type frct_secs = stm.wMilliseconds * std::pow10(time_duration::num_fractional_digits()-3);
  return ptime(date(stm.wYear,stm.wMonth,stm.wDay), time_duration(stm.wHour,stm.wMinute,stm.wSecond,frct_secs));
}
#else
#include <sys/time.h>
boost::posix_time::ptime GetCurrentTime_HighResolution()
{
  using namespace boost::posix_time;
  using namespace boost::gregorian;
  struct timeval   tmStruct;
  gettimeofday(&tmStruct, 0);
  time_duration::fractional_seconds_type frct_secs = tmStruct.tv_usec * std::pow(static_cast<double>(10), time_duration::num_fractional_digits()-6);
  return ptime(date(1970,1,1), time_duration(0,0,tmStruct.tv_sec,frct_secs));
}
#endif

/** Returns date precision as string. */
const char * GetDatePrecisionAsString(DatePrecisionType eType, std::string& sString, bool bPlural, bool bCapitalizeFirstLetter) {
  sString.clear();
  switch (eType) {
    case YEAR  : sString = (bCapitalizeFirstLetter ? "Y" : "y"); sString += "ear"; break;
    case MONTH : sString = (bCapitalizeFirstLetter ? "M" : "m"); sString += "onth"; break;
    case DAY   : sString = (bCapitalizeFirstLetter ? "D" : "d"); sString += "ay"; break;
    default    : sString = "none"; break;
  };
  if (bPlural)
    sString += "s";

  return sString.c_str();
}

/** Returns number of processors in the system. */
unsigned int GetNumSystemProcessors() {
  unsigned int iNumProcessors;

#ifdef INTEL_BASED
   SYSTEM_INFO siSysInfo;
   GetSystemInfo(&siSysInfo);
   iNumProcessors = siSysInfo.dwNumberOfProcessors;
#else
  iNumProcessors = sysconf(_SC_NPROCESSORS_ONLN);
#endif

  //return at least one, system calls might have failed
  return (iNumProcessors > 0 ? iNumProcessors : 1);
}

/** Calculates an estimate for the time remaining to complete X repetition given Y completed. */
void ReportTimeEstimate(boost::posix_time::ptime StartTime, int nRepetitions, int nRepsCompleted, BasePrint *pPrintDirection) {
  boost::posix_time::ptime StopTime(GetCurrentTime_HighResolution());
  double dSecondsElapsed;

  //nothing to report if number of repetitions less than 2 or none have been completed
  if (nRepetitions <= 1 || nRepsCompleted <= 0) return;
  //nothing to report if start time greater than stop time -- error?
  if (StartTime > StopTime) return;

  boost::posix_time::time_duration ElapsedTime = StopTime - StartTime;
  dSecondsElapsed = ElapsedTime.fractional_seconds() / std::pow(static_cast<double>(10), ElapsedTime.num_fractional_digits());
  dSecondsElapsed += ElapsedTime.seconds();
  dSecondsElapsed += ElapsedTime.minutes() * 60;
  dSecondsElapsed += ElapsedTime.hours() * 60 * 60;
  double dEstimatedSecondsRemaining = dSecondsElapsed/nRepsCompleted * (nRepetitions - nRepsCompleted);

  //print an estimation only if estimated time will be 30 seconds or more
  if (dEstimatedSecondsRemaining >= 30) {
    if (dEstimatedSecondsRemaining < 60.0)
      pPrintDirection->Printf(".... this will take approximately %.0lf seconds.\n", BasePrint::P_STDOUT, dEstimatedSecondsRemaining);
    else if (dEstimatedSecondsRemaining < 3600.0) {
      double dMinutes = std::ceil(dEstimatedSecondsRemaining/60.0);
      pPrintDirection->Printf(".... this will take approximately %.0lf minute%s.\n",
                              BasePrint::P_STDOUT, dMinutes, (dMinutes == 1.0 ? "" : "s"));
    }
    else {
      double dHours = std::floor(dEstimatedSecondsRemaining/3600.0);
      double dMinutes = std::ceil((dEstimatedSecondsRemaining - dHours * 3600.0)/60.0);
      pPrintDirection->Printf(".... this will take approximately %.0lf hour%s %.0lf minute%s.\n",
                              BasePrint::P_STDOUT, dHours, (dHours == 1.0 ? "" : "s"), dMinutes, (dMinutes == 1.0 ? "" : "s"));
    }
  }
}

/** Returns estimated unbiased variance for the entire data set. */
double GetUnbiasedVariance(count_t tObservations, measure_t tSumMeasure, measure_t tSumSqMeasure) {
  double dUnbiasedVariance;

  if (tObservations < 1) return -1; //error condition

  dUnbiasedVariance = std::fabs((tObservations == 1 ? 0.0 : (tSumSqMeasure - std::pow(tSumMeasure, 2)/tObservations)/(tObservations - 1)));
  return (dUnbiasedVariance < DBL_CMP_TOLERANCE ? 0.0 : dUnbiasedVariance);
}

/** Returns estimated combined variance for all observations adjusting for the cluster.
    This can be thought of as the variance unexplained by the cluster. */
double GetUnbiasedVariance(count_t tCases, measure_t tMeasure, measure_t tSqMeasure, count_t tTotalCases, measure_t tTotalMeasure, measure_t tTotalSqMeasure) {
   double dEstimatedMeanInside = (tCases ? tMeasure/tCases : 0);
   count_t tCasesOutside = tTotalCases - tCases;
   double dEstimatedMeanOutside = (tCasesOutside ? (tTotalMeasure - tMeasure)/tCasesOutside : 0);
   double dUnbiasedVariance = 1.0/(tTotalCases - 1) *
                             (tSqMeasure - 2.0 * tMeasure * dEstimatedMeanInside + tCases * std::pow(dEstimatedMeanInside , 2) +
                              (tTotalSqMeasure - tSqMeasure) - 2.0 * (tTotalMeasure - tMeasure) * dEstimatedMeanOutside +
                              (tTotalCases - tCases) * std::pow(dEstimatedMeanOutside, 2));
   return (dUnbiasedVariance < DBL_CMP_TOLERANCE ? 0.0 : dUnbiasedVariance);
}

/** Returns indication of whether file exists and is readable/writable. */
bool ValidateFileAccess(const std::string& filename, bool bWriteEnable) {
  FILE        * fp=0;
  bool          bReturn=true;

  bReturn = ((fp = fopen(filename.c_str(), bWriteEnable ? "w" : "r")) != NULL);
  if (fp) fclose(fp);

  return bReturn;
}

/** Trims leading and trailing 't' strings from source, inplace. */
void trimString(std::string &source, const char * t) {
  source.erase(0, source.find_first_not_of(t));
  source.erase(source.find_last_not_of(t)+1);
}

/** assigns formatted strng to destination */
std::string& printString(std::string& destination, const char * format, ...) {
  try {
    std::vector<char> temp(1);
    va_list varArgs;
    va_start (varArgs, format);
    size_t iStringLength = vsnprintf(&temp[0], temp.size(), format, varArgs);
    va_end(varArgs);
    temp.resize(iStringLength + 1);
    va_start (varArgs, format);
    vsnprintf(&temp[0], iStringLength + 1, format, varArgs);
    va_end(varArgs);
    destination = &temp[0];
  }
  catch (...) {}
  return destination;
}

/** assigns formatted strng to destination */
std::string& printStringArgs(std::string& destination, va_list varArgs, const char * format) {
   try {
     if (!format) return destination;

     std::vector<char> temp(1);
     // vsnprintf will calculate the required length, not including the NULL,
     // for the format string when given a NULL pointer and a zero length as
     // the first two parameters.
     size_t iStringLength = vsnprintf(&temp[0], temp.size(), format, varArgs);
     temp.resize(iStringLength + 1);
     vsnprintf(&temp[0], iStringLength + 1, format, varArgs);
     destination = &temp[0];
   }
   catch (...) {}
   return destination;
}
