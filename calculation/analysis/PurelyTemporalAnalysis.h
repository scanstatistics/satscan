//*****************************************************************************
#ifndef __PURELYTEMPORALANALYSIS_H
#define __PURELYTEMPORALANALYSIS_H
//*****************************************************************************
#include "Analysis.h"
#include "PurelyTemporalCluster.h"
#include "PurelyTemporalData.h"
#include "MeasureList.h"

class CPurelyTemporalAnalysis : public CAnalysis {
  private:
    CPurelyTemporalCluster    * gpTopCluster;

    void                        Init() {gpTopCluster=0;}
    virtual double              MonteCarlo(const DataStreamInterface & Interface);
    virtual double              MonteCarloProspective(const DataStreamInterface & Interface);
    void                        SetMaxNumClusters() {m_nMaxClusters = 1;}

  protected:
    virtual void                CalculateTopCluster(tract_t tCenter, const DataStreamGateway & DataGateway, bool bSimulation);
    virtual bool                FindTopClusters();
    virtual CCluster          & GetTopCalculatedCluster() {return *gpTopCluster;}
    virtual void                SetTopClusters(const DataStreamGateway & DataGateway, bool bSimulation) {/*no action*/}
  
  public:
    CPurelyTemporalAnalysis(CParameters* pParameters, CSaTScanData* pData, BasePrint *pPrintDirection);
    virtual ~CPurelyTemporalAnalysis();
};
//*****************************************************************************
#endif

