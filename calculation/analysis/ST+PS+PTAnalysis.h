// ST+PS+PTanalysis.h

#ifndef __ST_PS_PT_ANALYSIS_H
#define __ST_PS_PT_ANALYSIS_H

#include "STAnalysis.h"
#include "ST+PSAnalysis.h"
#include "PTCluster.h"

class C_ST_PS_PT_Analysis : public C_ST_PS_Analysis
{
  public:
    C_ST_PS_PT_Analysis(CParameters* pParameters, CSaTScanData* pData, BasePrint *pPrintDirection);
    virtual ~C_ST_PS_PT_Analysis();

    virtual bool   FindTopClusters();
//    virtual void   MakeData();
    virtual double MonteCarlo();
    virtual double MonteCarloProspective();
    CPurelyTemporalCluster* GetTopPTCluster();
};

#endif
