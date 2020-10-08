//***************************************************************************
#ifndef __IniParameterFileAccess_H
#define __IniParameterFileAccess_H
//***************************************************************************
#include "ParameterFileAccess.h"
#include "IniParameterSpecification.h"
#include "Ini.h"

/** Implements class that reading/writing parameter settings of an INI file,
    where each parameter is maintained in a key/value pair of a section. */
class IniParameterFileAccess : public AbtractParameterFileAccess  {
  private:
    const IniParameterSpecification   * gpSpecifications;

    virtual const char                * GetParameterLabel(ParameterType eParameterType) const;
    const IniParameterSpecification   & GetSpecifications() const;
    void                                ReadIniParameter(const IniFile& SourceFile, ParameterType eParameterType);
    std::vector<std::string>          & ReadIniParameter(const IniFile& SourceFile, ParameterType eParameterType, std::vector<std::string>& vParameters, size_t iSuffixIndex) const;
    void                                ReadMultipleDataSetsSettings(const IniFile& SourceFile);
    void                                ReadObservableRegionSettings(const IniFile& SourceFile);
    void                                ReadInputSourceSettings(const IniFile& SourceFile);
    bool                                ReadInputSourceSection(const IniFile& SourceFile, const char* sectionName, const char* keyPrefix, CParameters::InputSource& source);

    void                                WriteInputSource(IniFile& WriteFile, ParameterType eParameterType, const CParameters::InputSource * source);
    void                                WriteInputSource(IniFile& WriteFile, IniSection& section, const std::string& basekey, const CParameters::InputSource * source);
    void                                WriteOtherOutputSettings(IniFile& WriteFile);
    void                                WriteAnalysisSettings(IniFile& WriteFile);
    void                                WriteSpatialOutputSettings(IniFile& WriteFile);
    void                                WriteDataCheckingSettings(IniFile& WriteFile);
	void                                WriteDrilldownSettings(IniFile& WriteFile);
	void                                WriteEllipticScanSettings(IniFile& WriteFile);
    void                                WriteInferenceSettings(IniFile& WriteFile);
    void                                WriteClusterRestrictionsSettings(IniFile& WriteFile);
    void                                WriteBorderAnalysisSettings(IniFile& WriteFile);
    void                                WriteIniParameter(IniFile& WriteFile, ParameterType eParameterType, const char* sValue, const char* sComment=0);
    void                                WriteIniParameterAsKey(IniFile& WriteFile, const char* sSectionName, const char* sKey, const char* sValue, const char* sComment=0);
    void                                WriteInputSettings(IniFile& WriteFile);
    void                                WriteMultipleDataSetsSettings(IniFile& WriteFile);
    void                                WriteObservableRegionSettings(IniFile& WriteFile);
    void                                WriteOutputSettings(IniFile& WriteFile);
    void                                WritePowerEvaluationsSettings(IniFile& WriteFile);
    void                                WritePowerSimulationsSettings(IniFile& WriteFile);
    void                                WriteRunOptionSettings(IniFile& WriteFile);
    void                                WriteSpaceAndTimeAdjustmentSettings(IniFile& WriteFile);
    void                                WriteSpatialNeighborsSettings(IniFile& WriteFile);
    void                                WriteSpatialWindowSettings(IniFile& WriteFile);
    void                                WriteSystemSettings(IniFile& WriteFile);
    void                                WriteTemporalGraphSettings(IniFile& WriteFile);
    void                                WriteTemporalWindowSettings(IniFile& WriteFile);

  public:
     IniParameterFileAccess(CParameters& Parameters, BasePrint& PrintDirection);
     virtual ~IniParameterFileAccess();

     virtual bool                       Read(const char* szFilename);
     virtual void                       Write(const char * szFilename);

     void                               writeSections(IniFile& ini, const IniParameterSpecification& specification);
};
//***************************************************************************
#endif
