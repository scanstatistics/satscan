//*****************************************************************************
#ifndef __SPACETIMEPURELYTEMPORALANALYSIS_H
#define __SPACETIMEPURELYTEMPORALANALYSIS_H
//*****************************************************************************
#include "SpaceTimeAnalysis.h"
#include "PurelyTemporalCluster.h"
#include "SpaceTimeCluster.h"
#include "PurelyTemporalData.h"
#include "SpaceTimeData.h"
#include "MeasureList.h"

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
//*****************************************************************************
#endif
