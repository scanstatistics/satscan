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
  private:
    CPurelyTemporalCluster      * gpTopPurelyTemporalCluster;
    CPurelyTemporalCluster      * gpPurelyTemporalClusterComparator;
    TemporalData                * gpPTClusterData;

    void                          Init();

  protected:
    virtual void                AllocateSimulationObjects(const AbtractDataStreamGateway & DataGateway);  
    virtual double              FindTopRatio(const AbtractDataStreamGateway & DataGateway);
    virtual double              MonteCarlo(const DataStreamInterface & Interface);
    virtual void                SetMaxNumClusters();

  public:
    C_ST_PT_Analysis(CParameters* pParameters, CSaTScanData* pData, BasePrint *pPrintDirection);
    virtual ~C_ST_PT_Analysis();

    virtual bool                FindTopClusters(const AbtractDataStreamGateway & DataGateway);
};
//*****************************************************************************
#endif
