//***************************************************************************
#ifndef __ParameterFileAccess_H
#define __ParameterFileAccess_H
//***************************************************************************
#include "Parameters.h"
#include "SSException.h" 

/** Coordinates the reading/writing of parameters to file. */
class ParameterAccessCoordinator {
  protected:
    CParameters               & gParameters;

  public:
    ParameterAccessCoordinator(CParameters& Parameters);
    virtual ~ParameterAccessCoordinator();

    static CParameters::CreationVersion getIniVersion(const char* szFilename);

    bool                         Read(const char* sFilename, BasePrint& PrintDirection);
    void                         Write(const char * sFilename, BasePrint& PrintDirection /* ability to specify a version to write as ?*/);
};

/** Abstract base class for reading/writing parameters from file. */
class AbtractParameterFileAccess {
  public:
  typedef std::pair<std::string, std::string> DateRange_t;

  private:
    virtual const char        * GetParameterLabel(ParameterType eParameterType) const = 0;
    bool                        ReadBoolean(const std::string& sValue, ParameterType eParameterType) const;
    void                        ReadDate(const std::string& sValue, ParameterType eParameterType) const;
    void                        ReadDateRange(const std::string& sValue, ParameterType eParameterType, DateRange_t& Range) const;
    double                      ReadDouble(const std::string& sValue, ParameterType eParameterType) const;
    void                        ReadEllipseRotations(const std::string& sParameter) const;
    void                        ReadEllipseShapes(const std::string& sParameter) const;
    int                         ReadEnumeration(int iValue, ParameterType eParameterType, int iLow, int iHigh) const;
    int                         ReadInt(const std::string& sValue, ParameterType eParameterType) const;
    void                        ReadSpatialWindowStops(const std::string& sParameter) const;
    int                         ReadUnsignedInt(const std::string& sValue, ParameterType eParameterType) const;
    void                        ReadVersion(const std::string& sValue) const;

  protected:
    CParameters               & gParameters;
    BasePrint                 & gPrintDirection;
    std::vector<int>            gvParametersMissingDefaulted;           /** collection of missing ParameterTypes on read from file */
    bool                        gbReadStatusError;                      /** marker of errors encountered while reading parameters from file */
    bool                        gbWriteBooleanAsDigit;
    double                      gdMaxSpatialClusterSize;
    double                      gdMaxReportedSpatialClusterSize;

    std::string               & AsString(std::string& ref, int i) const {printString(ref, "%d", i); return ref;}
    std::string               & AsString(std::string& ref, unsigned int i) const {printString(ref, "%u", i); return ref;}
    std::string               & AsString(std::string& ref, float f) const {printString(ref, "%g", f); return ref;}
    std::string               & AsString(std::string& ref, double d) const {printString(ref, "%g", d); return ref;}
    std::string               & AsString(std::string& ref, bool b) const {printString(ref, "%s", (b ? (gbWriteBooleanAsDigit ? "1" : "y") : (gbWriteBooleanAsDigit ? "0" : "n"))); return ref;}
    std::string               & AsString(std::string& ref, const CParameters::CreationVersion& v) const {printString(ref, "%d.%d.%d", v.iMajor, v.iMinor, v.iRelease); return ref;}
    const char                * GetParameterComment(ParameterType eParameterType) const;
    std::string               & GetParameterString(ParameterType eParameterType, std::string& s) const;

  public:
     AbtractParameterFileAccess(CParameters& Parameters, BasePrint& PrintDirection, bool bWriteBooleanAsDigit=false);
     virtual ~AbtractParameterFileAccess();

     virtual bool               Read(const char* szFilename) = 0;
     void                       SetParameter(ParameterType eParameterType, const std::string& sParameter, BasePrint& PrintDirection);
     CParameters::InputSource & setInputSource(CParameters::InputSource & source,
                                               const std::string& typeStr,
                                               const std::string& mapStr,
                                               const std::string& delimiterStr,
                                               const std::string& groupStr,
                                               const std::string& skipStr,
                                               const std::string& headerStr,
                                               BasePrint& PrintDirection);
     virtual void               Write(const char * szFilename) = 0;
};

/** Execption class of invalid parameters */
class parameter_error : public resolvable_error {
  public:
   parameter_error(const char * format, ...);
};
//***************************************************************************
#endif
