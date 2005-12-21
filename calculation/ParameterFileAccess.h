//***************************************************************************
#ifndef __ParameterFileAccess_H
#define __ParameterFileAccess_H
//***************************************************************************
#include "Parameters.h"

/** Coordinates the reading/writing of parameters to file. */
class ParameterAccessCoordinator {
  protected:
    CParameters               & gParameters;

  public:
    ParameterAccessCoordinator(CParameters& Parameters);
    virtual ~ParameterAccessCoordinator();

    bool                        Read(const char* sFilename, BasePrint& PrintDirection);
    void                        Write(const char * sFilename, BasePrint& PrintDirection /* ability to specify a version to write as ?*/);
};

/** Abstract base class for reading/writing parameters from file. */
class AbtractParameterFileAccess {
  public:
  typedef std::pair<std::string, std::string> DateRange_t;

  private:
    virtual const char        * GetParameterLabel(ParameterType eParameterType) const = 0;
    bool                        ReadBoolean(const ZdString& sValue, ParameterType eParameterType) const;
    void                        ReadDate(const ZdString& sValue, ParameterType eParameterType) const;
    void                        ReadDateRange(const ZdString& sValue, ParameterType eParameterType, DateRange_t& Range) const;
    double                      ReadDouble(const ZdString& sValue, ParameterType eParameterType) const;
    void                        ReadEllipseRotations(const ZdString& sParameter) const;
    void                        ReadEllipseShapes(const ZdString& sParameter) const;
    int                         ReadEnumeration(int iValue, ParameterType eParameterType, int iLow, int iHigh) const;
    float                       ReadFloat(const ZdString& sValue, ParameterType eParameterType) const;
    int                         ReadInt(const ZdString& sValue, ParameterType eParameterType) const;
    int                         ReadUnsignedInt(const ZdString& sValue, ParameterType eParameterType) const;
    void                        ReadVersion(const ZdString& sValue) const;

  protected:
    CParameters               & gParameters;
    BasePrint                 & gPrintDirection;
    std::vector<int>            gvParametersMissingDefaulted;           /** collection of missing ParameterTypes on read from file */
    bool                        gbReadStatusError;                      /** marker of errors encountered while reading parameters from file */
    bool                        gbWriteBooleanAsDigit;

    ZdString                  & AsString(ZdString& ref, int i) const {ref = i; return ref;}
    ZdString                  & AsString(ZdString& ref, unsigned int i) const {ref.Clear(); ref << i; return ref;}
    ZdString                  & AsString(ZdString& ref, float f) const {ref = f; return ref;}
    ZdString                  & AsString(ZdString& ref, double d) const {ref = d; return ref;}
    ZdString                  & AsString(ZdString& ref, bool b) const {ref = (b ? (gbWriteBooleanAsDigit ? "1" : "y") : (gbWriteBooleanAsDigit ? "0" : "n")); return ref;}
    ZdString                  & AsString(ZdString& ref, const CParameters::CreationVersion& v) const {ref.printf("%d.%d.%d", v.iMajor, v.iMinor, v.iRelease); return ref;}
    const char                * GetParameterComment(ParameterType eParameterType) const;
    ZdString                  & GetParameterString(ParameterType eParameterType, ZdString& s) const;
    void                        MarkAsMissingDefaulted(ParameterType eParameterType, BasePrint& PrintDirection);
    void                        SetParameter(ParameterType eParameterType, const ZdString& sParameter, BasePrint& PrintDirection);

  public:
     AbtractParameterFileAccess(CParameters& Parameters, BasePrint& PrintDirection, bool bWriteBooleanAsDigit=false);
     virtual ~AbtractParameterFileAccess();

     virtual bool               Read(const char* szFilename) = 0;
     virtual void               Write(const char * szFilename) = 0;
};

/** Execption class of invalid parameters */
class InvalidParameterException : public ResolvableException {
  public:
   InvalidParameterException(va_list varArgs, const char *sMessage, const char *sSourceModule, ZdException::Level iLevel);
   virtual ~InvalidParameterException() {}

   static void Generate (const char *sMessage, const char *sSourceModule,  ... );
};
//***************************************************************************
#endif
