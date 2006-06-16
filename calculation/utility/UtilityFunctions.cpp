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
    Returns pair of doubles <latitude, longtitude>. Throws ZdException if number of
    coordinates in passed vector is not 3. */
std::pair<double, double> ConvertToLatLong(const std::vector<double>& vCoordinates) {
  std::pair<double, double>     prLatitudeLongitude;
  double RADIUS = 6367; // Constant; radius of earth in km)

  if (vCoordinates.size() != 3)
    ZdGenerateException("Conversion to latitude/longitude requires a vector of 3 elements.\n"
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
const char * GetDatePrecisionAsString(DatePrecisionType eType, ZdString& sString, bool bPlural, bool bCapitalizeFirstLetter) {
  sString.Clear();
  switch (eType) {
    case YEAR  : sString << (bCapitalizeFirstLetter ? "Y" : "y") << "ear"; break;
    case MONTH : sString << (bCapitalizeFirstLetter ? "M" : "m") << "onth"; break;
    case DAY   : sString << (bCapitalizeFirstLetter ? "D" : "d") << "ay"; break;
    default    : sString = "none"; break;
  };
  if (bPlural)
    sString << "s";

  return sString.GetCString();
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

/** Returns estimated unbiased variance. */
double GetUnbiasedVariance(count_t tObservations, measure_t tSumMeasure, measure_t tSumSqMeasure) {
  double dUnbiasedVariance;

  if (tObservations < 1) return -1; //error condition

  dUnbiasedVariance = std::fabs((tObservations == 1 ? 0.0 : (tSumSqMeasure - std::pow(tSumMeasure, 2)/tObservations)/(tObservations - 1)));
  return (dUnbiasedVariance < 0.00000001 ? 0.0 : dUnbiasedVariance);
}

/** Returns indication of whether file exists and is readable/writable. */
bool ValidateFileAccess(const std::string& filename, bool bWriteEnable) {
  FILE        * fp=0;
  bool          bReturn=true;

  bReturn = ((fp = fopen(filename.c_str(), bWriteEnable ? "w" : "r")) != NULL);
  fclose(fp);

  return bReturn;
}

