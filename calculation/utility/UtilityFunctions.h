//******************************************************************************
#ifndef __UTILITYFUNCTIONS_H
#define __UTILITYFUNCTIONS_H
//******************************************************************************
#include "SaTScan.h"
#include "bloom_filter.hpp"
#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>
#include <boost/date_time.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>

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
std::string                   & GetUserTemporaryFilename(std::string& s);
std::string                     getFilenameFormatTime(const std::string& filename, boost::posix_time::ptime timeLocal, bool testUnknown=false);
size_t                          getLineCount(const std::string& filename);
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
template <typename T>           bool csv_string_to_typelist(const char * s, std::vector<T>& list) {
                                    try {
                                        list.clear();
                                        std::string value(s);
                                        boost::escaped_list_separator<char> separator('\\', ',', '\"');
                                        boost::tokenizer<boost::escaped_list_separator<char> > identifiers(value, separator);
                                        for (boost::tokenizer<boost::escaped_list_separator<char> >::const_iterator itr=identifiers.begin(); itr != identifiers.end(); ++itr) {
                                            std::string token(*itr);
                                            T t = boost::lexical_cast<T>(trimString(token).c_str());
                                            list.push_back(t);
                                        }
                                    } catch (boost::bad_lexical_cast&) {
                                        return false;
                                    } 
                                    return true;
                                }
template <typename T>           std::string& typelist_to_csv_string(const std::vector<T>& list, std::string& s) {
                                    std::stringstream buffer;
                                    if (list.empty()) {
                                        s = "";
                                    } else {
                                        std::string grouper = (list.begin()->find(",") == std::string::npos ? "" : "\"");
                                        buffer << grouper << *(list.begin()) << grouper;
                                        for (typename std::vector<T>::const_iterator itr = list.begin() + 1; itr != list.end(); ++itr) {
                                            grouper = (itr->find(",") == std::string::npos ? "" : "\"");
                                            buffer << "," << grouper << *itr << grouper;
                                        }
                                        s = buffer.str();
                                    }
                                    return s;
                                }
std::stringstream             & base64Encode(const std::stringstream & source, std::stringstream & destination);
bool                            sendMail(
                                    const std::string& from, const std::vector<std::string>& to, const std::vector<std::string>& cc, const std::string& reply,
                                    const std::string& subject, const std::stringstream& messagePlain, const std::stringstream& messageHTML,
                                    const std::string& results_fullpath, const std::string& mailserver, BasePrint& printDirection, bool printalways,
                                    const std::string& additionalpass, std::stringstream * output=0
);
bool                            validEmailAdrress(const std::string& emailaddress);
boost::shared_ptr<bloom_filter> getNewBloomFilter(size_t element_count = 5000);
std::stringstream & templateReplace(std::stringstream& templateText, const std::string& replaceStub, const std::string& replaceWith);
//******************************************************************************
#endif
