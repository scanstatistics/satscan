//******************************************************************************
#ifndef __PROBABILITYMODEL_H
#define __PROBABILITYMODEL_H
//******************************************************************************
#include "SaTScan.h"
#include "Parameters.h"
#include "DataSet.h"
#include "DataSetInterface.h"

//#define DEBUGMODEL 0

class CPSMonotoneCluster;
class CSaTScanData;
class CSVTTCluster;
class CTimeTrend;

class CModel {
  protected:
    BasePrint                 & gPrintDirection;
    const CParameters         & gParameters;
    CSaTScanData              & gDataHub;
#ifdef DEBUGMODEL
    FILE                      * m_pDebugModelFile;
#endif

  public:
    CModel(const CParameters& Parameters, CSaTScanData& DataHub, BasePrint& PrintDirection);
    virtual ~CModel();

    virtual void                CalculateMeasure(RealDataSet& DataSet) = 0;
    virtual double              GetPopulation(size_t iSetIndex, int m_iEllipseOffset,
                                              tract_t nCenter, tract_t nTracts,
                                              int nStartInterval, int nStopInterval) const;
};
//******************************************************************************
#endif

