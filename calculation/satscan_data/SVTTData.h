//*****************************************************************************
#ifndef __SVTTDATA_H
#define __SVTTDATA_H
//*****************************************************************************
#include "SaTScanData.h"

class CSVTTData : public CSaTScanData {
  private:
    double            * m_pSumLogs; //Added 990915 GG

    virtual void        DisplayCounts(FILE* pFile, count_t** pCounts,
                                      char* szVarName, count_t** pCountsNC,
                                      char* szVarNameNC, count_t*  pCountsTI,
                                      char* szVarNameTI, char* szTitle=NULL);
    void                Init() {m_pSumLogs=0;}
    virtual void        SetProbabilityModel();

  protected:
    virtual bool        CalculateMeasure();
    virtual void        SetAdditionalCaseArrays();
    virtual void        SetNumTimeIntervals();
    void                SetCaseArrays(count_t**  pCases, count_t** pCases_NC, count_t*  pCasesByTimeInt);

  public:
    CSVTTData(CParameters* pParameters, BasePrint *pPrintDirection);
    virtual ~CSVTTData();

    virtual void        DisplayCases(FILE* pFile);
    virtual void        DisplaySimCases(FILE* pFile);
    virtual void        DisplayMeasures(FILE* pFile);
    virtual void        DisplayRelativeRisksForEachTract(const bool bASCIIOutput, const bool bDBaseOutput);
    virtual void        MakeData(int iSimulationNumber);
    void                SetSumLogs();
};

#endif
