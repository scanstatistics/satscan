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

//enum FileFormatType {INI_FORMAT=0, SCAN_FORMAT}; /** file structure types of file supplied parameter settings */

/** Abstract base class for reading/writing parameters from file. */
class AbtractParameterFileAccess {
  private:
    virtual const char        * GetParameterLabel(ParameterType eParameterType) const = 0;
    bool                        ReadBoolean(const ZdString& sValue, ParameterType eParameterType);
    void                        ReadDate(const ZdString& sValue, ParameterType eParameterType);
    double                      ReadDouble(const ZdString& sValue, ParameterType eParameterType);
    void                        ReadEllipseRotations(const ZdString& sParameter);
    void                        ReadEllipseShapes(const ZdString& sParameter);
    void                        ReadEndIntervalRange(const ZdString& sParameter);
    int                         ReadEnumeration(int iValue, ParameterType eParameterType, int iLow, int iHigh);
    float                       ReadFloat(const ZdString& sValue, ParameterType eParameterType);
    int                         ReadInt(const ZdString& sValue, ParameterType eParameterType);
    void                        ReadStartIntervalRange(const ZdString& sParameter);
    int                         ReadUnsignedInt(const ZdString& sValue, ParameterType eParameterType);

  protected:
    CParameters               & gParameters;
    BasePrint                 & gPrintDirection;
    std::vector<int>            gvParametersMissingDefaulted;           /** collection of missing ParameterTypes on read from file */
    bool                        gbReadStatusError;                      /** marker of errors encountered while reading parameters from file */

    ZdString                  & AsString(ZdString& ref, int i) {ref = i; return ref;}
    ZdString                  & AsString(ZdString& ref, unsigned int i) {ref.Clear(); ref << i; return ref;}
    ZdString                  & AsString(ZdString& ref, float f) {ref = f; return ref;}
    ZdString                  & AsString(ZdString& ref, double d) {ref = d; return ref;}
    ZdString                  & AsString(ZdString& ref, bool b) {ref = (b ? "y" : "n"); return ref;}
    ZdString                  & AsString(ZdString& ref, const CParameters::CreationVersion& v) {ref.printf("%d.%d.%d", v.iMajor, v.iMinor, v.iRelease); return ref;}
    void                        MarkAsMissingDefaulted(ParameterType eParameterType, BasePrint& PrintDirection);
    void                        SetParameter(ParameterType eParameterType, const ZdString& sParameter, BasePrint& PrintDirection);

  public:
     AbtractParameterFileAccess(CParameters& Parameters, BasePrint& PrintDirection);
     virtual ~AbtractParameterFileAccess();

     virtual bool               Read(const char* szFilename) = 0;
     virtual void               Write(const char * szFilename) = 0;
};
//***************************************************************************
#endif
