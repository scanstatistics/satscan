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

class C_ST_PT_Analysis : public CSpaceTimeAnalysis {
  protected:
    virtual double                      MonteCarlo(const DataStreamInterface & Interface);
    virtual double                      MonteCarloProspective(const DataStreamInterface & Interface);
    virtual void                        SetMaxNumClusters();

  public:
    C_ST_PT_Analysis(CParameters* pParameters, CSaTScanData* pData, BasePrint *pPrintDirection);
    virtual ~C_ST_PT_Analysis();

    virtual bool                        FindTopClusters();
    CPurelyTemporalCluster            * GetTopPTCluster();
};
//*****************************************************************************
#endif
