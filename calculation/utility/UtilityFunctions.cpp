//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "UtilityFunctions.h"
#include "SSException.h"
#include "Toolkit.h"
#include "newmat.h"
#include "FileName.h"
#include <cmath>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include "SimulationVariables.h"


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

/* Convert UTM to Latitude/Longitude - algorithm at http://www.sascommunity.org/wiki/Latitude/longitude_to_UTM_conversion_%28and_vice-versa%29 */
void UTM_To_LatitudeLongitude(double& latitude, double& longitude, char hemisphere, unsigned int zone, double northing, double easting) {
    double a=6378137; // equatorial radius;
    double b=6356752.314; // polar radius (values of a and b are for WGS84 datum - for other datums see http://www.uwgb.edu/dutchs/usefuldata/utmformulas.htm );
    double k0=0.9996; // scale along central meridian;

    double e =  std::pow(1.0 - std::pow(b, 2.0)/std::pow(a, 2.0), 0.5); // eccentricity; e=(1-(&b**2/&a**2))**0.5;
    double e1 = (1.0 - std::pow(1.0 - std::pow(e, 2.0), 0.5)) / (1.0 + std::pow(1.0 - std::pow(e, 2.0), 0.5)); // e1=%sysevalf((1-(1-&e**2)**0.5)/(1+(1-&e**2)**0.5));
    double ep2 = std::pow(e, 2.0)/(1.0 - std::pow(e, 2.0)); // e prime squared; // ep2=&e**2/(1-&e**2); *e prime squared;
    double c1 = (3.0 * e1/2.0) - (27.0 * std::pow(e1, 3.0) / 32.0); // c1=(3*&e1/2)-(27*(&e1**3)/32);
    double c2 = (21.0 * std::pow(e1, 2.0)/16.0) - (55.0 * std::pow(e1, 4.0)/32.0); // c2=(21*(&e1**2)/16)-(55*(&e1**4)/32);
    double c3 = 151.0 * std::pow(e1, 3.0)/96.0; // c3=(151*(&e1**3)/96);
    double c4 = 1097.0 * std::pow(e1, 4.0)/512.0; // c4=(1097*(&e1**4)/512);

    // Intermediate variables below follow the use by Steven Dutch. CN = corrected northing, EP = east prime, AL = arc length, 
    // mu and phi are greek letters, others have no specific reference;
    double CN, EP, AL, mu, phi, C1, T1, N1, R1, D, Q1, Q2, Q3, Q4, Q5, Q6, Q7, DL;
    if (hemisphere == 'N')
        CN = northing;
    else CN = 10000000.0 - northing;
    EP = 500000.0 - easting;
    AL = CN / k0;
    mu = AL / (a * ((1.0 - std::pow(e, 2.0)/4.0) - (3.0 * std::pow(e, 4.0)/64.0) - (5.0 * std::pow(e, 6.0)/256.0))); // mu=AL/(&a*((1-&e**2/4)-(3*&e**4/64)-(5*&e**6/256)));
    phi = mu + c1 * std::sin(2.0 * mu) + c2 * std::sin(4.0 * mu) + c3 * std::sin(6.0 * mu) + c4 * std::sin(8.0 * mu); // phi=mu+&c1*sin(2*mu)+&c2*sin(4*mu)+&c3*sin(6*mu)+&c4*sin(8*mu);
    C1 = ep2 * std::pow(std::cos(phi), 2.0); // C1=&ep2*cos(phi)**2;
    T1 = std::pow(std::tan(phi), 2.0); // T1=tan(phi)**2;
    N1 = a/std::pow(1.0 -std::pow(e * std::sin(phi), 2.0), 0.5); // N1=&a/(1-(&e*sin(phi))**2)**0.5;
    R1 = a * (1.0 - e * e)/std::pow(1.0 - std::pow(e * std::sin(phi), 2.0), 1.5); // R1=&a*(1-&e*&e)/(1-(&e*sin(phi))**2)**1.5;
    D = EP / ( N1 * k0); // D=EP/(N1*&k0);
    Q1 = N1 * std::tan(phi) / R1; // Q1=N1*tan(phi)/R1;
    Q2 = std::pow(D, 2.0) / 2.0; // Q2=D**2/2;
    Q3 = (5.0 + (3.0 * T1) + (10.0 * C1) - (4 * std::pow(C1, 2.0)) - (9.0 * ep2)) * std::pow(D, 4.0) / 24.0; // Q3=(5+(3*T1)+(10*C1)-(4*C1**2)-(9*&ep2))*(D**4)/24;
    Q4 = (61.0 + (90.0 * T1) + (298.0 * C1) + (45.0 * std::pow(T1,2.0)) - (252.0 * ep2) - (3.0 * std::pow(C1,2.0))) * std::pow(D, 6.0)/720.0; // Q4=(61+(90*T1)+(298*C1)+(45*T1**2)-(252*&ep2)-(3*C1**2))*(D**6)/720;
    Q5 = D;
    Q6 = (1.0 + (2.0 * T1) + C1) * std::pow(D, 3.0)/6.0; //Q6 = (1+(2*T1)+C1)*(D**3)/6;
    Q7 = (5.0 - (2.0 * C1) + (28.0 * T1) - (3.0 * std::pow(C1,2.0) + (8.0 * ep2) + (24.0 * std::pow(T1,2.0)))) * std::pow(D, 5.0)/120.0; // Q7=(5-(2*C1)+(28*T1)-(3*C1**2)+(8*&ep2)+(24*T1**2))*(D**5)/120;
    DL = (Q5 - Q6 + Q7)/std::cos(phi); // DL=(Q5-Q6+Q7)/cos(phi);
    latitude = 180.0 * (phi - Q1 * (Q2 + Q3 + Q4))/PI;
    if (hemisphere == 'S')
        latitude = latitude * -1.0; 
    longitude = ((6.0 * static_cast<double>(zone)) - 183.0) - DL * 180.0/PI;
}

/* Convert Latitude/Longitude to UTM - algorithm at http://www.sascommunity.org/wiki/Latitude/longitude_to_UTM_conversion_%28and_vice-versa%29 */
void LatitudeLongitude_To_UTM(double lat, double lon, char& hemisphere, unsigned int& zone, double& northing, double& easting) {
    double a = 6378137; //equatorial radius (default is for WGS84 datum);
    double b = 6356752.314; //polar radius (default is for WGS84 datum);
    double k0 = 0.9996; //scale along central meridian;
    double e =  std::pow(1.0 - std::pow(b, 2.0)/std::pow(a, 2.0), 0.5); // eccentricity; e=(1-(&b**2/&a**2))**0.5;
    double ep2 = std::pow(e, 2.0)/(1.0 - std::pow(e, 2.0)); // e prime squared; // ep2=&e**2/(1-&e**2); *e prime squared;
    double n = (a - b) / (a + b);
    double A0 = a * ((1.0 - n) + ((1.0 - n) * (5.0 * std::pow(n,2.0)) / 4.0) + ((1.0 - n) * (81.0/64.0 * std::pow(n,4.0)))); // &a*((1-&n)+((1-&n)*(5*&n**2)/4)+((1-&n)*(81/64*&n**4)))
    double B0 = (3.0/2.0 * a * n) * ((1.0 - n) - ((1.0 - n) * 7.0/8.0 * std::pow(n,2.0)) + (55.0/64.0 * std::pow(n,4.0))); // (3/2*&a*&n)*((1-&n)-((1-&n)*7/8*&n**2)+(55/64*&n**4))
    double C0 = (15.0/16.0 * a * std::pow(n,2.0)) * ((1.0 - n) + ((1.0 - n) * (3.0 / 4.0 * std::pow(n,2.0)))); // (15/16*&a*&n**2)*((1-&n)+((1-&n)*(3/4*&n**2)))
    double D0 = (35.0 /48.0 * a * std::pow(n,3.0)) * ((1.0 - n) + (11.0/16.0 * std::pow(n,2.0))); // (35/48*&a*&n**3)*((1-&n)+(11/16*&n**2))
    double E0 = (315.0 / 51.0 * a * std::pow(n,4.0)) * (1.0 - n); // (315/51*&a*&n**4)*(1-&n)
    // LM = longitude central meridian, MA = meridianal arc, RN = raw northing
    //rho, nu are greek letters;
    //all others are intermediate unnamed constants;;
 
    double LM, deltalon, latr, lonr, rho, nu, MA, Ki, Kii, Kiii, Kiv, Kv, A6, RN;
    if (lat < 0)
        hemisphere = 'S';
    else 
        hemisphere = 'N';
    zone = static_cast<unsigned int>(31.0 + std::floor(lon/6.0));
    LM = 6.0 * static_cast<double>(zone) - 183.0;
    deltalon = (lon - LM) * PI/180.0;
    latr = lat * PI/180.0;
    lonr = lon * PI/180.0;
    rho = a * (1.0 - std::pow(e,2.0)) / std::pow(1.0 - std::pow(e * std::sin(latr), 2.0), 1.5); //&a*(1-&e**2)/((1-(&e*sin(latr))**2)**1.5);
    nu = a / std::pow(std::pow(1.0 - (e * std::sin(latr)), 2.0), 0.5); // nu = &a/((1-(&e*sin(latr))**2)**0.5);
    MA = (A0 * latr) - (B0 * std::sin(2.0 * latr)) + (C0 * std::sin(4.0 * latr)) - (D0 * std::sin(6.0 * latr)) + (E0 * std::sin(8.0 * latr)); // MA = (&A0*latr)-(&B0*sin(2*latr))+(&C0*sin(4*latr))-(&D0*sin(6*latr))+(&E0*sin(8*latr));
    Ki = MA * k0;
    Kii = nu * std::sin(latr) * std::cos(latr)/2.0; // Kii = nu*sin(latr)*cos(latr)/2;
    Kiii = ((nu * std::sin(latr) * std::pow(std::cos(latr), 3.0))/24.0) * (5.0 - std::pow(std::tan(latr), 2.0) + 9.0 * ep2 * std::pow(std::cos(latr), 2.0) + 4.0 * std::pow(ep2, 2.0) * std::pow(std::cos(latr),4.0)) * k0; // Kiii = ((nu*sin(latr)*cos(latr)**3)/24)*(5-tan(latr)**2+9*&ep2*cos(latr)**2+4*&ep2**2*cos(latr)**4)*&k0;
    Kiv = nu * std::cos(latr) * k0; // Kiv = nu*cos(latr)*&k0;
    Kv = std::pow(std::cos(latr), 3.0) * nu / 6.0 * (1.0 - std::pow(tan(latr), 2.0) + ep2 * std::pow(cos(latr), 2.0)) * k0; // Kv = cos(latr)**3*nu/6*(1-tan(latr)**2+&ep2*cos(latr)**2)*&k0;
    A6 = ( std::pow(deltalon,6.0) * nu * std::sin(latr) * std::pow(std::cos(latr),5.0) / 720.0 ) * (61.0 - 58.0 * std::pow(std::tan(latr),2.0) + std::pow(std::tan(latr), 4.0) + 270.0 * ep2 * std::pow(std::cos(latr), 2.0) - 330.0 * ep2 * std::pow(std::sin(latr), 2.0)) * k0; // A6 = ((deltalon)**6*nu*sin(latr)*cos(latr)**5/720)*(61-58*tan(latr)**2+tan(latr)**4+270*&ep2*cos(latr)**2-330*&ep2*sin(latr)**2)*&k0;
    RN = Ki + Kii * std::pow(deltalon,2.0) + Kiii * std::pow(deltalon, 4.0);
    if (RN < 0)
        northing = 10000000.0 + RN;
    else 
        northing = RN;
    easting = 500000.0 + (Kiv * deltalon + Kv * std::pow(deltalon,3.0));
}

/* test function for conversions between UMT -> latitude/longitiude and reverse.
    see http://www.sascommunity.org/wiki/Latitude/longitude_to_UTM_conversion_%28and_vice-versa%29 */
void UTM_conversion_test() {
    double longitude, latitude, northing, easting;
    unsigned int zone;
    char hemisphere;

    //Eiffel Tower  N 31 5411949 448231
    UTM_To_LatitudeLongitude(latitude, longitude, 'N', 31, 5411949, 448231);
    printf("Eiffel Tower: %g / %g\n", latitude, longitude);
    //Eiffel Tower  48.8583 2.2942
    LatitudeLongitude_To_UTM(48.8583, 2.2942, hemisphere, zone, northing, easting);
    printf("Eiffel Tower: %c %u %g %g\n", hemisphere, zone, northing, easting);

    //Sydney Opera House  S 56 6252309 334897
    UTM_To_LatitudeLongitude(latitude, longitude, 'S', 56, 6252309, 334897);
    printf("Sydney Opera House: %g / %g\n", latitude, longitude);
    //Sydney Opera House  -33.8566 151.2153
    LatitudeLongitude_To_UTM(-33.8566, 151.2153, hemisphere, zone, northing, easting);
    printf("Sydney Opera House: %c %u %g %g\n", hemisphere, zone, northing, easting);

    //Buenos Aires Obelisk  S 21 6170000 373315
    UTM_To_LatitudeLongitude(latitude, longitude, 'S', 21, 6170000, 373315);
    printf("Buenos Aires Obelisk: %g / %g\n", latitude, longitude);
    //Buenos Aires Obelisk  -34.6040 -58.3816
    LatitudeLongitude_To_UTM(-34.6040, -58.3816, hemisphere, zone, northing, easting);
    printf("Buenos Aires Obelisk: %c %u %g %g\n", hemisphere, zone, northing, easting);

    //Statue of Liberty  N 18 4504682 580720
    UTM_To_LatitudeLongitude(latitude, longitude, 'N', 18, 4504682, 580720);
    printf("Statue of Liberty: %g / %g\n", latitude, longitude);
    //Statue of Liberty  40.6890 -74.0447
    LatitudeLongitude_To_UTM(40.6890, -74.0447, hemisphere, zone, northing, easting);
    printf("Statue of Liberty: %c %u %g %g\n", hemisphere, zone, northing, easting);

    //Bozeman, Montana  N 12 495019 5055755
    UTM_To_LatitudeLongitude(latitude, longitude, 'N', 12, 5055755, 495019);
    printf("Bozeman, Montana: %g / %g\n", latitude, longitude);
    //Bozeman, Montana  45.65533, -111.06393
    LatitudeLongitude_To_UTM(45.65533, -111.06393, hemisphere, zone, northing, easting);
    printf("Bozeman, Montana: %c %u %g %g\n", hemisphere, zone, northing, easting);
}

/** Return non-compactness penalty coefficient for specified elliptic shape and
    compactness penalty power. */
double CalculateNonCompactnessPenalty(double dEllipseShape, double dPower) {
  return pow((4*dEllipseShape/(pow(dEllipseShape + 1, 2))), dPower);
}

//What is the current time? (UTC | Coordinated Universal Time)
#ifdef _WINDOWS_
boost::posix_time::ptime GetCurrentTime_HighResolution()
{
  using namespace boost::posix_time;
  using namespace boost::gregorian;
  SYSTEMTIME stm;
  GetSystemTime(&stm);
  time_duration::fractional_seconds_type frct_secs = static_cast<time_duration::fractional_seconds_type>(stm.wMilliseconds * pow(static_cast<double>(10), time_duration::num_fractional_digits()-3));
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
  time_duration::fractional_seconds_type frct_secs = static_cast<time_duration::fractional_seconds_type>(tmStruct.tv_usec * std::pow(static_cast<double>(10), time_duration::num_fractional_digits()-6));
  return ptime(date(1970,1,1), time_duration(0,0,tmStruct.tv_sec,frct_secs));
}
#endif

/** Returns date precision as string. */
const char * GetDatePrecisionAsString(DatePrecisionType eType, std::string& sString, bool bPlural, bool bCapitalizeFirstLetter) {
  sString.clear();
  switch (eType) {
    case YEAR    : sString = (bCapitalizeFirstLetter ? "Y" : "y"); sString += "ear"; break;
    case MONTH   : sString = (bCapitalizeFirstLetter ? "M" : "m"); sString += "onth"; break;
    case DAY     : sString = (bCapitalizeFirstLetter ? "D" : "d"); sString += "ay"; break;
    case GENERIC : sString = (bCapitalizeFirstLetter ? "G" : "g"); sString += "eneric"; break;
    default      : sString = "none"; break;
  };
  if (bPlural)
    sString += "s";

  return sString.c_str();
}

/** Returns number of processors in the system. */
unsigned int GetNumSystemProcessors() {
  unsigned int iNumProcessors = 1;

#ifdef _WINDOWS_
   SYSTEM_INFO siSysInfo;
   GetSystemInfo(&siSysInfo);
   iNumProcessors = siSysInfo.dwNumberOfProcessors;
#else
  iNumProcessors = sysconf(_SC_NPROCESSORS_ONLN);
#endif

  //return at least one, system calls might have failed
  return (iNumProcessors > 0 ? iNumProcessors : 1);
}

/** Calculates an estimate for the time remaining to complete X repetition given Y completed. Returns estimation in seconds. */
double ReportTimeEstimate(boost::posix_time::ptime StartTime, int nRepetitions, int nRepsCompleted, BasePrint& printDirection, bool isUpperLimit, bool isUpdate) {
    boost::posix_time::ptime StopTime(GetCurrentTime_HighResolution());
    double dSecondsElapsed;

    //nothing to report if number of repetitions less than 2 or none have been completed
    if (nRepetitions <= 1 || nRepsCompleted <= 0) return 0.0;
    //nothing to report if start time greater than stop time -- error?
    if (StartTime > StopTime) return 0.0;

    boost::posix_time::time_duration ElapsedTime = StopTime - StartTime;
    dSecondsElapsed = ElapsedTime.fractional_seconds() / std::pow(static_cast<double>(10), ElapsedTime.num_fractional_digits());
    dSecondsElapsed += ElapsedTime.seconds();
    dSecondsElapsed += ElapsedTime.minutes() * 60;
    dSecondsElapsed += ElapsedTime.hours() * 60 * 60;
    double dEstimatedSecondsRemaining = dSecondsElapsed/nRepsCompleted * (nRepetitions - nRepsCompleted);

    //print an estimation only if estimated time will be 30 seconds or more
    if (dEstimatedSecondsRemaining >= 30) {
        if (dEstimatedSecondsRemaining < 60.0) {
            const char * message = isUpperLimit ? ".... this step will approximately take at most %.0lf seconds%s.\n" : ".... this step will take approximately %.0lf seconds%s.\n";
            printDirection.Printf(message, BasePrint::P_STDOUT, dEstimatedSecondsRemaining, isUpdate ? " more" : "");
        } else if (dEstimatedSecondsRemaining < 3600.0) {
            double dMinutes = std::ceil(dEstimatedSecondsRemaining/60.0);
            const char * message = isUpperLimit ? ".... this step will approximately take at most %.0lf minute%s%s.\n" : ".... this step will take approximately %.0lf minute%s%s.\n";
            printDirection.Printf(message, BasePrint::P_STDOUT, dMinutes, (dMinutes == 1.0 ? "" : "s"), isUpdate ? " more" : "");
        } else {
            double dHours = std::floor(dEstimatedSecondsRemaining/3600.0);
            double dMinutes = std::ceil((dEstimatedSecondsRemaining - dHours * 3600.0)/60.0);
            const char * message = isUpperLimit ? ".... this step will approximately take at most %.0lf hour%s %.0lf minute%s%s.\n" : ".... this step will take approximately %.0lf hour%s %.0lf minute%s%s.\n";
            printDirection.Printf(message, BasePrint::P_STDOUT, dHours, (dHours == 1.0 ? "" : "s"), dMinutes, (dMinutes == 1.0 ? "" : "s"), isUpdate ? " more" : "");
        }
    }
    return dEstimatedSecondsRemaining;
}

/** Returns estimated unbiased variance for the entire data set. */
double GetUnbiasedVariance(count_t tObservations, measure_t tSumMeasure, measure_t tSumMeasureAux) {
  double dUnbiasedVariance;

  if (tObservations < 1) return -1; //error condition

  dUnbiasedVariance = std::fabs((tObservations == 1 ? 0.0 : (tSumMeasureAux - std::pow(tSumMeasure, 2)/tObservations)/(tObservations - 1)));
  return (dUnbiasedVariance < DBL_CMP_TOLERANCE ? 0.0 : dUnbiasedVariance);
}

/** Returns estimated combined variance for all observations adjusting for the cluster.
    This can be thought of as the variance unexplained by the cluster. */
double GetUnbiasedVariance(count_t tCases, measure_t tMeasure, measure_t tMeasureAux, count_t tTotalCases, measure_t tTotalMeasure, measure_t tTotalMeasureAux) {
   double dEstimatedMeanInside = (tCases ? tMeasure/tCases : 0);
   count_t tCasesOutside = tTotalCases - tCases;
   double dEstimatedMeanOutside = (tCasesOutside ? (tTotalMeasure - tMeasure)/tCasesOutside : 0);
   double dUnbiasedVariance = 1.0/(tTotalCases - 1) *
                             (tMeasureAux - 2.0 * tMeasure * dEstimatedMeanInside + tCases * std::pow(dEstimatedMeanInside , 2) +
                              (tTotalMeasureAux - tMeasureAux) - 2.0 * (tTotalMeasure - tMeasure) * dEstimatedMeanOutside +
                              (tTotalCases - tCases) * std::pow(dEstimatedMeanOutside, 2));
   return (dUnbiasedVariance < DBL_CMP_TOLERANCE ? 0.0 : dUnbiasedVariance);
}

/** Returns indication of whether file exists and is readable/writable. */
bool ValidateFileAccess(const std::string& filename, bool bWriteEnable, bool useTempFile) {
    FILE * fp=0;
    bool bReturn=true;

#ifdef __APPLE__
    // Hack for Mac application - where sample_data is in application bundle. We don't want to write rsult files to the bundle.
    if (bWriteEnable && filename.find("/Contents/app/sample_data/") != std::string::npos)
        return false;
#endif

    if (useTempFile) {
        std::string buffer;
        FileName test(filename.c_str());
        test.setExtension("write-test");
        bReturn = ((fp = fopen(test.getFullPath(buffer).c_str(), bWriteEnable ? "w" : "r")) != NULL);
        if (fp) fclose(fp);
        remove(test.getFullPath(buffer).c_str());
    } else {
        bReturn = ((fp = fopen(filename.c_str(), bWriteEnable ? "w" : "r")) != NULL);
        if (fp) fclose(fp);
    }
    return bReturn;
}

/** Trims leading and trailing 't' strings from source, inplace. */
std::string & trimString(std::string &source, const char * t) {
  source.erase(0, source.find_first_not_of(t));
  source.erase(source.find_last_not_of(t)+1);
  return source;
}

/** Converts string to lower case. */
std::string& lowerString(std::string &source) {
  std::transform(source.begin(), source.end(), source.begin(), (int(*)(int)) tolower);
  return source;
}

/** assigns formatted strng to destination */
std::string& printString(std::string& destination, const char * format, ...) {
  try {
#ifdef _MSC_VER
    std::vector<char> temp(MSC_VSNPRINTF_DEFAULT_BUFFER_SIZE);
    va_list varArgs;
    va_start (varArgs, format);
    vsnprintf(&temp[0], temp.size() - 1, format, varArgs);
    va_end(varArgs);
#else
    std::vector<char> temp(1);    
	va_list varArgs_static;
    va_start (varArgs_static, format);

	std::va_list arglist_test; 
	macro_va_copy(arglist_test, varArgs_static);
    size_t iStringLength = vsnprintf(&temp[0], temp.size(), format, arglist_test);
    temp.resize(iStringLength + 1);

	std::va_list arglist;
	macro_va_copy(arglist, varArgs_static);
    vsnprintf(&temp[0], iStringLength + 1, format, arglist);
    va_end(varArgs_static);
#endif
    destination = &temp[0];
  }
  catch (...) {}
  return destination;
}

/* Get printf precision format specifer given passed value. 
   Returns a minimum of 'iSignificant' significant decimal digits. */
unsigned int getFormatPrecision(double value, unsigned int iSignificant) {
    unsigned int iPrecision = iSignificant;

    if (value == 0.0) return 0;

    if (fabs(value) < 1.0) {
        //If value less than 1.0, we can use log10 to determine what is the 10 power.
        //ex. value = 0.0023:
        //   log10(0.0023) = log10(10^-3) + log10(2.3)
        //   log10(0.0023) = -3 + 0.36172783601759287886777711225119
        //   log10(0.0023) = -2.6382721639824071211322228877488
        //   take ceiling since we really are not interested in log10(2.3) portion
        iPrecision += static_cast<unsigned int>(ceil(fabs(log10(fabs(value))))) - 1;
    }
    return iPrecision;
}

/** Returns value as string with number of 'iSignificant' significant decimals.
    The 'g' format specifier might have sufficed but Martin wanted this format.
*/
std::string& getValueAsString(double value, std::string& s, unsigned int iSignificant) {
    unsigned int iPrecision = getFormatPrecision(value, iSignificant);
    std::string format;
    printString(format, "%%.%dlf", iPrecision);
    printString(s, format.c_str(), value);
    return s;
}

/** Returns double as string with specified decimal precision.
*/
std::string & getRoundAsString(double value, std::string& s, unsigned int precision) {
    std::stringstream buffer;
    buffer << std::setprecision(precision) << std::setiosflags(std::ios_base::fixed) << value;
    s = buffer.str();
    return s;
}

#ifdef _WINDOWS_
#include "shlobj.h"

std::string & GetUserDirectory(std::string& s, const std::string& defaultPath) {
  TCHAR szPath[MAX_PATH];

  if(SUCCEEDED(SHGetFolderPath(NULL, CSIDL_PROFILE|CSIDL_FLAG_CREATE, NULL, 0, szPath))) {
    //if (!DirectoryExists(szPath)) {
    //  if (!CreateDir(szPath))
    //    throw prg_error("Unable to create My Documents.", "GetMyDocumentsDirectory()");
    //}
    s = szPath;
  } else {
    s = defaultPath; 
  }  
  return s;
}

std::string & GetUserDocumentsDirectory(std::string& s, const std::string& defaultPath) {
    TCHAR szPath[MAX_PATH];

    if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_PERSONAL | CSIDL_FLAG_CREATE, NULL, 0, szPath))) {
        //if (!DirectoryExists(szPath)) {
        //  if (!CreateDir(szPath))
        //    throw prg_error("Unable to create My Documents.", "GetMyDocumentsDirectory()");
        //}
        s = szPath;
    }
    else {
        s = defaultPath;
    }
    return s;
}

#else
#include <sys/types.h>
#include <pwd.h>

std::string & GetUserDirectory(std::string& s, const std::string& defaultPath) {
    uid_t           uid;
    struct passwd * pwd;
    uid = getuid();
    if (!(pwd = getpwuid(uid))) {
        s = defaultPath;
    }
    else {
        s = pwd->pw_dir;
    }
    endpwent();
    return s;
}

std::string & GetUserDocumentsDirectory(std::string& s, const std::string& defaultPath) {
  uid_t           uid;
  struct passwd * pwd;
  uid = getuid();
  if (!(pwd = getpwuid(uid))) {
    s = defaultPath; 
  } else {
    s = pwd->pw_dir;
  }
  endpwent();
  return s;
}    
#endif

std::string & GetUserTemporaryDirectory(std::string& s) {
    s = boost::filesystem::temp_directory_path().string();
    return s;
}

/* Replaces format specifiers with values from todays date. Some format specifiers are derived from:
   https://www.boost.org/doc/libs/1_70_0/doc/html/date_time/date_time_io.html#date_time.format_flags */
std::string getFilenameFormatTime(const std::string& filename, boost::posix_time::ptime timeLocal, bool testUnknown) {

    std::string buffer, mod_filename(filename);
    std::stringstream bufferStream;
    using boost::algorithm::replace_all;
    using boost::algorithm::ireplace_all;
    boost::posix_time::time_facet * facet = new boost::posix_time::time_facet();

    // Four digit year - we're not doing 2 digit years -- notice using ireplace_all, so either {y} or {Y} works
    ireplace_all(mod_filename, "<y>", printString(buffer, "%d", timeLocal.date().year()));
    ireplace_all(mod_filename, "<year>", printString(buffer, "%d", timeLocal.date().year()));
    // Month name as a decimal 1 to 12 -- not padded
    ireplace_all(mod_filename, "<m>", printString(buffer, "%d", timeLocal.date().month()));
    ireplace_all(mod_filename, "<month>", printString(buffer, "%d", timeLocal.date().month()));
    // Month name as a decimal 01 to 12 -- zero padded
    ireplace_all(mod_filename, "<0m>", printString(buffer, "%02d", timeLocal.date().month()));
    ireplace_all(mod_filename, "<0month>", printString(buffer, "%02d", timeLocal.date().month()));
    // Abbreviated month name
    facet->format("%b");
    bufferStream.imbue(std::locale(std::locale::classic(), facet));
    bufferStream.str(""); bufferStream << timeLocal;
    replace_all(mod_filename, "<b>", bufferStream.str());
    // Full month name
    facet->format("%B");
    bufferStream.imbue(std::locale(std::locale::classic(), facet));
    bufferStream.str(""); bufferStream << timeLocal;
    replace_all(mod_filename, "<B>", bufferStream.str());
    // Day of the month as decimal 1 to 31 - not padded
    ireplace_all(mod_filename, "<d>", printString(buffer, "%d", timeLocal.date().day()));
    ireplace_all(mod_filename, "<day>", printString(buffer, "%d", timeLocal.date().day()));
    // Day of the month as decimal 1 to 31 - zero padded
    ireplace_all(mod_filename, "<0d>", printString(buffer, "%02d", timeLocal.date().day()));
    ireplace_all(mod_filename, "<0day>", printString(buffer, "%02d", timeLocal.date().day()));
    // Abbreviated weekday name
    facet->format("%a");
    bufferStream.imbue(std::locale(std::locale::classic(), facet));
    bufferStream.str(""); bufferStream << timeLocal;
    replace_all(mod_filename, "<a>", bufferStream.str());
    // Long weekday name
    facet->format("%A");
    bufferStream.imbue(std::locale(std::locale::classic(), facet));
    bufferStream.str(""); bufferStream << timeLocal;
    replace_all(mod_filename, "<A>", bufferStream.str());

    if (testUnknown && mod_filename.find_first_of("<>") != std::string::npos) {
        throw resolvable_error(
            "Filename '%s' contains substitutions which could not be converted.\nSubstituted filename resulted in: %s.", 
            filename.c_str(), mod_filename.c_str());
    }
    return mod_filename;
}

/** Attempt to readline for stream giving consideration to DOS, UNIX (or Mac Os X) and Mac 9 (or earlier) line ends. 
    Returns whether data was read or end of file encountered. */
bool getlinePortable(std::istream &readstream, /*std::ifstream& readstream,*/ std::string& line) {
  std::ifstream::char_type nextChar;
  std::stringstream        readStream;

  while (!(readstream.eof() || readstream.fail())) {

	  if (!readstream.get(nextChar)) {//Does reading next char bring us to end of file?
         break;
      }
      if (nextChar == readstream.widen('\r')) {
          // could be either DOS or Mac 9 end of line -- peek at next char
          nextChar = readstream.peek();
          if (nextChar == readstream.widen('\n')) {
		      //DOS end of line characters -- read it.
              readstream.get(nextChar);
          }
          break;
      }
      if (nextChar == readstream.widen('\n')) {
	      //UNIX or Mac OS X end of line character.
          break;
      }
      readStream << nextChar;
  }
  line = readStream.str();
  return line.size() > 0 ? true : (readstream.eof() == false && readstream.fail() == false);
}

void printoutMatrix(const std::string& s, Matrix& m, FILE * fp) {
  fp = fp ? fp : AppToolkit::getToolkit().openDebugFile();
  fprintf(fp, "%s\n", s.c_str());
  for (int r=0; r < m.Nrows(); ++r) {
      fprintf(fp, "row %d\t", r + 1);
      for (int c=0; c < m.Ncols(); ++c)
         fprintf(fp, "%g\t", m.element(r,c));
      fprintf(fp, "\n");
  }
  fprintf(fp, "\n");
  fflush(fp);
}


/** Returns Gumbel p-value. */
std::pair<double,double> calculateGumbelPValue(const SimulationVariables& simVars, double critical_value) {
    double beta = std::sqrt(simVars.get_variance()) * std::sqrt(6.0)/PI;
    double mu = simVars.get_mean() - EULER * beta;
	double p = 1 - std::exp(-std::exp((mu - critical_value)/beta));
    // Determine the alternative minimum p-value. Very strong clusters will cause 
    // the calculated p-value to be computed as zero in above statement.    
    double min = (double)0.1 / std::pow(10.0, std::numeric_limits<double>::digits10 + 1.0);

    return std::make_pair(p,min);
}

std::pair<double,double> calculateGumbelCriticalValue(const SimulationVariables& simVars, double p_value) {
    double beta = std::sqrt(simVars.get_variance()) * std::sqrt(6.0)/PI;
    double mu = simVars.get_mean() - EULER * beta;
	double critical_value = mu - beta * std::log(std::log( 1 /( 1 - p_value )));
    // Determine the alternative minimum p-value. Very strong clusters will cause 
    // the calculated p-value to be computed as zero in above statement.    
    double min = (double)0.1 / std::pow(10.0, std::numeric_limits<double>::digits10 + 1.0);
    return std::make_pair(critical_value,min);
}

const char * ordinal_suffix(unsigned int n) {
	static const char suffixes[][3] = { "th", "st", "nd", "rd" };
	unsigned int ord = n % 100;
	if (ord / 10 == 1) ord = 0; 
	ord = ord % 10;
	if (ord > 3) ord = 0;
	return suffixes[ord];
}