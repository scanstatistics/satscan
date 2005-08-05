//***************************************************************************
#ifndef __IniParameterFileAccess_H
#define __IniParameterFileAccess_H
//***************************************************************************
#include "ParameterFileAccess.h"
#include "IniParameterSpecification.h"

/** Implements class that reading/writing parameter settings of an INI file,
    where each parameter is maintained in a key/value pair of a section. */
class IniParameterFileAccess : public AbtractParameterFileAccess  {
  private:
    const IniParameterSpecification   * gpSpecifications;

    virtual const char                * GetParameterLabel(ParameterType eParameterType) const;
    const IniParameterSpecification   & GetSpecifications() const;
    void                                ReadIniParameter(const ZdIniFile& SourceFile, ParameterType eParameterType);
    std::vector<ZdString>             & ReadIniParameter(const ZdIniFile& SourceFile, ParameterType eParameterType, std::vector<ZdString>& vParameters) const;
    void                                ReadMultipleDataSetsSettings(const ZdIniFile& SourceFile);

    void                                WriteAnalysisSettings(ZdIniFile& WriteFile);
    void                                WriteClustersReportedSettings(ZdIniFile& WriteFile);
    void                                WriteEllipticScanSettings(ZdIniFile& WriteFile);
    void                                WriteInferenceSettings(ZdIniFile& WriteFile);
    void                                WriteIniParameter(ZdIniFile& WriteFile, ParameterType eParameterType, const char* sValue, const char* sComment=0);
    void                                WriteIniParameterAsKey(ZdIniFile& WriteFile, const char* sSectionName, const char* sKey, const char* sValue, const char* sComment=0);
    void                                WriteIsotonicScanSettings(ZdIniFile& WriteFile);
    void                                WriteInputSettings(ZdIniFile& WriteFile);
    void                                WriteMultipleDataSetsSettings(ZdIniFile& WriteFile);
    void                                WriteOutputSettings(ZdIniFile& WriteFile);
    void                                WritePowerSimulationsSettings(ZdIniFile& WriteFile);
    void                                WriteRunOptionSettings(ZdIniFile& WriteFile);
    void                                WriteSequentialScanSettings(ZdIniFile& WriteFile);
    void                                WriteSpaceAndTimeAdjustmentSettings(ZdIniFile& WriteFile);
    void                                WriteSpatialWindowSettings(ZdIniFile& WriteFile);
    void                                WriteSystemSettings(ZdIniFile& WriteFile);
    void                                WriteTemporalWindowSettings(ZdIniFile& WriteFile);

  public:
     IniParameterFileAccess(CParameters& Parameters, BasePrint& PrintDirection);
     virtual ~IniParameterFileAccess();

     virtual bool                       Read(const char* szFilename);
     virtual void                       Write(const char * szFilename);
};
//***************************************************************************
#endif
