// ST+PTanalysis.h

#ifndef __ST_PT_ANALYSIS_H
#define __ST_PT_ANALYSIS_H

#include "STAnalysis.h"
#include "PTCluster.h"

class C_ST_PT_Analysis : public CSpaceTimeAnalysis
{
  public:
    C_ST_PT_Analysis(CParameters* pParameters, CSaTScanData* pData, BasePrint *pPrintDirection);
    virtual ~C_ST_PT_Analysis();

    virtual void      SetMaxNumClusters();
    virtual bool      FindTopClusters();
//    virtual void      MakeData();
    virtual double    MonteCarlo();
    virtual double    MonteCarloProspective();
    
    CPurelyTemporalCluster* GetTopPTCluster();

};

#endif
