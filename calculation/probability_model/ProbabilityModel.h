//*****************************************************************************
#ifndef __PROBABILITYMODEL_H
#define __PROBABILITYMODEL_H
//*****************************************************************************
#include "SaTScan.h"
#include "Parameters.h"

//#define DEBUGMODEL 0

class CPSMonotoneCluster;
class CSaTScanData;
class CSVTTCluster;
class CTimeTrend;

class CModel {
  protected:
    BasePrint                 & gPrintDirection;
    CParameters               & gParameters;
    CSaTScanData              & gData;
#ifdef DEBUGMODEL
    FILE                      * m_pDebugModelFile;
#endif

  public:
    CModel(CParameters & Parameters, CSaTScanData & Data, BasePrint & PrintDirection);
    virtual ~CModel();

    virtual double              CalcLogLikelihood(count_t n, measure_t u) = 0;
    virtual double              CalcMonotoneLogLikelihood(const CPSMonotoneCluster& PSMCluster);
    virtual bool                CalculateMeasure() = 0;
    virtual double              CalcSVTTLogLikelihood(CSVTTCluster* Cluster, CTimeTrend GlobalTimeTrend);
    virtual bool                DoesReadMaxCirclePopulationFile();
    virtual double              GetLogLikelihoodForTotal() const = 0;
    virtual double              GetPopulation(int m_iEllipseOffset, tract_t nCenter, tract_t nTracts,
                                              int nStartInterval, int nStopInterval) = 0;
    virtual void                MakeData(int iSimulationNumber) = 0;
    virtual bool                ReadData() = 0;
};

//*****************************************************************************
#endif

