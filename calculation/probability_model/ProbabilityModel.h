//*****************************************************************************
#ifndef __PROBABILITYMODEL_H
#define __PROBABILITYMODEL_H
//*****************************************************************************
#include "SaTScan.h"
#include "Parameters.h"
#include "DataStream.h"
#include "DataStreamInterface.h"

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
    virtual double              CalcLogLikelihoodRatio(count_t tCases, measure_t tMeasure, count_t tTotalCases, measure_t tTotalMeasure) = 0;
    virtual double              CalcLogLikelihoodRatioEx(count_t tCases, measure_t tMeasure, measure_t tMeasure2, count_t tTotalCases, measure_t tTotalMeasure);
    virtual double              CalcMonotoneLogLikelihood(const CPSMonotoneCluster& PSMCluster);
    virtual bool                CalculateMeasure(DataStream & thisStream) = 0;
    virtual double              CalcSVTTLogLikelihoodRatio(size_t tStream, CSVTTCluster* Cluster, CTimeTrend GlobalTimeTrend);
    virtual double              GetLogLikelihoodForTotal() const = 0;
    virtual double              GetPopulation(int m_iEllipseOffset, tract_t nCenter, tract_t nTracts,
                                              int nStartInterval, int nStopInterval) = 0;
};

//*****************************************************************************
#endif

