//*****************************************************************************
#ifndef __SVTTDATA_H
#define __SVTTDATA_H
//*****************************************************************************
#include "SaTScanData.h"

/** central data hub for spatial variation and temporal trends analysis */
class CSVTTData : public CSaTScanData {
  private:
    virtual void        DisplayCounts(FILE* pFile, count_t** pCounts,
                                      char* szVarName, count_t** pCountsNC,
                                      char* szVarNameNC, count_t*  pCountsTI,
                                      char* szVarNameTI, char* szTitle=NULL);
    virtual void        SetProbabilityModel();

  protected:
    virtual bool        CalculateMeasure(DataStream & thisStream);
    virtual void        SetAdditionalCaseArrays(DataStream & thisStream);
    virtual void        SetNumTimeIntervals();

  public:
    CSVTTData(CParameters* pParameters, BasePrint *pPrintDirection);
    virtual ~CSVTTData();

    virtual void        AllocateSimulationStructures();
    virtual void        DisplayCases(FILE* pFile);
    virtual void        DisplaySimCases(FILE* pFile);
    virtual void        DisplayMeasures(FILE* pFile);
    virtual void        DisplayRelativeRisksForEachTract(const bool bASCIIOutput, const bool bDBaseOutput);
    virtual void        MakeData(int iSimulationNumber, DataStreamGateway & DataGateway);
};

#endif
