// PTAnalysis.h

#ifndef __PTANALYSIS_H
#define __PTANALYSIS_H

#include "analysis.h"

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

#endif

