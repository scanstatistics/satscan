//*****************************************************************************
#ifndef __PURELYTEMPORALANALYSIS_H
#define __PURELYTEMPORALANALYSIS_H
//*****************************************************************************
#include "Analysis.h"
#include "PurelyTemporalCluster.h"
#include "PurelyTemporalData.h"
#include "MeasureList.h"

class CPurelyTemporalAnalysis : public CAnalysis
{
  public:
    CPurelyTemporalAnalysis(CParameters* pParameters, CSaTScanData* pData, BasePrint *pPrintDirection);
    virtual ~CPurelyTemporalAnalysis();

    virtual CCluster* GetTopCluster(tract_t nCenter);

  protected:
    virtual bool FindTopClusters();
//    virtual void MakeData();
    virtual double MonteCarlo();
    virtual double MonteCarloProspective();
    void SetMaxNumClusters() {m_nMaxClusters = 1;};

};
//*****************************************************************************
#endif

