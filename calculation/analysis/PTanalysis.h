// PTAnalysis.h

#ifndef __PTANALYSIS_H
#define __PTANALYSIS_H

#include "analysis.h"

class CPurelyTemporalAnalysis : public CAnalysis
{
  public:
    CPurelyTemporalAnalysis(CParameters* pParameters, CSaTScanData* pData);
    virtual ~CPurelyTemporalAnalysis();

    virtual CCluster* GetTopCluster(tract_t nCenter);

  protected:
    virtual void FindTopClusters();
//    virtual void MakeData();
    virtual double MonteCarlo();
    void SetMaxNumClusters() {m_nMaxClusters = 1;};

};

#endif

