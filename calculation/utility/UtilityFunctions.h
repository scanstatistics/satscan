//******************************************************************************
#ifndef __UTILITYFUNCTIONS_H
#define __UTILITYFUNCTIONS_H
//******************************************************************************
#include "SaTScan.h"
#include "boost/lexical_cast.hpp"

class Matrix; // forward declaration
class SimulationVariables;

void 	                        ConvertFromLatLong(double Latitude, double Longitude, std::vector<double>& vCoordinates);
std::pair<double, double>       ConvertToLatLong(const std::vector<double>& vCoordinates);
void                            UTM_To_LatitudeLongitude(double& latitude, double& longitude, char hemisphere, unsigned int zone, double northing, double easting);
void                            LatitudeLongitude_To_UTM(double lat, double lon, char& hemisphere, unsigned int& zone, double& northing, double& easting);
void                            UTM_conversion_test();
double                          CalculateNonCompactnessPenalty(double dEllipseShape, double dPower);
const char                    * GetDatePrecisionAsString(DatePrecisionType eType, std::string& sString, bool bPlural=true, bool bCapitalizeFirstLetter=false);
unsigned int                    GetNumSystemProcessors();
double                          ReportTimeEstimate(boost::posix_time::ptime StartTime, int nRepetitions, int nRepsCompleted, BasePrint& printDirection, bool isUpperLimit = false, bool isUpdate = false);
boost::posix_time::ptime        GetCurrentTime_HighResolution();
double                          GetUnbiasedVariance(count_t tObservations, measure_t tSumMeasure, measure_t tSumMeasureAux);
double                          GetUnbiasedVariance(count_t tCases, measure_t tMeasure, measure_t tMeasureAux, count_t tTotalCases, measure_t tTotalMeasure, measure_t tTotalMeasureAux);
bool                            ValidateFileAccess(const std::string& filename, bool bWriteEnable=false, bool useTempFile=false);
std::string                   & trimString(std::string &source, const char * t=" ");
std::string                   & lowerString(std::string &source); 
std::string                   & printString(std::string& s, const char * format, ...);
unsigned int                    getFormatPrecision(double value, unsigned int iSignificant=2);
std::string                   & getValueAsString(double value, std::string& s, unsigned int iSignificant=2);
std::string                   & getRoundAsString(double value, std::string& s, unsigned int iRound=2);
std::string                   & GetUserDirectory(std::string& s, const std::string& defaultPath);
std::string                   & GetUserDocumentsDirectory(std::string& s, const std::string& defaultPath);
std::string                   & GetUserTemporaryDirectory(std::string& s);
std::string                     getFilenameFormatTime(const std::string& filename);
bool                            getlinePortable(std::istream &readstream, /*std::ifstream& readstream,*/ std::string& line);
void                            printoutMatrix(const std::string& s, Matrix& m, FILE * fp);
template <typename T>           bool string_to_type(const char * s, T& t) {
                                    try {
                                        t = boost::lexical_cast<T>(s);
                                    } catch (boost::bad_lexical_cast& b) {
                                        return false;
                                    } 
                                    return true;
                                }
template <typename T>           bool type_to_string(T& t, std::string& s) {
                                    try {
                                        s = boost::lexical_cast<std::string>(t);
                                    } catch (boost::bad_lexical_cast& b) {
                                        return false;
                                    } 
                                    return true;
                                }
std::pair<double,double>        calculateGumbelPValue(const SimulationVariables& simVars, double critical_value);
std::pair<double,double>        calculateGumbelCriticalValue(const SimulationVariables& simVars, double p_value);
const char                    * ordinal_suffix(unsigned int n);
//******************************************************************************
#endif
