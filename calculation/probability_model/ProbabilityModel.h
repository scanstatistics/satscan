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
    const CParameters         & gParameters;
    CSaTScanData              & gData;
#ifdef DEBUGMODEL
    FILE                      * m_pDebugModelFile;
#endif

  public:
    CModel(const CParameters & Parameters, CSaTScanData & Data, BasePrint & PrintDirection);
    virtual ~CModel();

    virtual void                CalculateMeasure(RealDataStream & thisStream) = 0;
    virtual double              GetPopulation(unsigned int iStream, int m_iEllipseOffset,
                                              tract_t nCenter, tract_t nTracts,
                                              int nStartInterval, int nStopInterval) const;
};

//*****************************************************************************
#endif
