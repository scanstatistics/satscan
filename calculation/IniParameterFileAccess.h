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
    void                                ReadAnalysisSettings(const ZdIniFile& SourceFile);
    void                                ReadBatchModeFeaturesSettings(const ZdIniFile& SourceFile);
    void                                ReadClustersReportedSettings(const ZdIniFile& SourceFile);
    void                                ReadEllipticScanSettings(const ZdIniFile& SourceFile);
    void                                ReadInferenceSettings(const ZdIniFile& SourceFile);
    void                                ReadIniParameter(const ZdIniFile& SourceFile, ParameterType eParameterType);
    std::vector<ZdString>             & ReadIniParameter(const ZdIniFile& SourceFile, ParameterType eParameterType, std::vector<ZdString>& vParameters) const;
    void                                ReadIsotonicScanSettings(const ZdIniFile& SourceFile);
    void                                ReadInputSettings(const ZdIniFile& SourceFile);
    void                                ReadMultipleDataSetsSettings(const ZdIniFile& SourceFile);
    void                                ReadOutputSettings(const ZdIniFile& SourceFile);
    void                                ReadPowerSimulationsSettings(const ZdIniFile& SourceFile);
    void                                ReadSequentialScanSettings(const ZdIniFile& SourceFile);
    void                                ReadSpaceAndTimeAdjustmentSettings(const ZdIniFile& SourceFile);
    void                                ReadSpatialWindowSettings(const ZdIniFile& SourceFile);
    void                                ReadSystemSettings(const ZdIniFile& SourceFile);
    void                                ReadTemporalWindowSettings(const ZdIniFile& SourceFile);

    void                                WriteAnalysisSettings(ZdIniFile& SaveFile);
    void                                WriteBatchModeFeaturesSettings(ZdIniFile& SaveFile);
    void                                WriteClustersReportedSettings(ZdIniFile& SaveFile);
    void                                WriteEllipticScanSettings(ZdIniFile& SaveFile);
    void                                WriteInferenceSettings(ZdIniFile& SaveFile);
    void                                WriteIniParameter(ZdIniFile& SaveFile, ParameterType eParameterType, const char* sValue, const char* sComment=0);
    void                                WriteIniParameterAsKey(ZdIniFile& SaveFile, const char* sSectionName, const char* sKey, const char* sValue, const char* sComment=0);
    void                                WriteIsotonicScanSettings(ZdIniFile& SaveFile);
    void                                WriteInputSettings(ZdIniFile& SaveFile);
    void                                WriteMultipleDataSetsSettings(ZdIniFile& SaveFile);
    void                                WriteOutputSettings(ZdIniFile& SaveFile);
    void                                WritePowerSimulationsSettings(ZdIniFile& SaveFile);
    void                                WriteSequentialScanSettings(ZdIniFile& SaveFile);
    void                                WriteSpaceAndTimeAdjustmentSettings(ZdIniFile& SaveFile);
    void                                WriteSpatialWindowSettings(ZdIniFile& SaveFile);
    void                                WriteSystemSettings(ZdIniFile& SaveFile);
    void                                WriteTemporalWindowSettings(ZdIniFile& SaveFile);

  public:
     IniParameterFileAccess(CParameters& Parameters, BasePrint& PrintDirection);
     virtual ~IniParameterFileAccess();

     virtual bool                       Read(const char* szFilename);
     virtual void                       Write(const char * szFilename);
};
//***************************************************************************
#endif
