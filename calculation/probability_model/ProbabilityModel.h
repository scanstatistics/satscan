//*****************************************************************************
#ifndef __PROBABILITYMODEL_H
#define __PROBABILITYMODEL_H
//*****************************************************************************
#include "SaTScan.h"
#include "Parameters.h"

//#define DEBUGMODEL 0

class CPSMonotoneCluster;
class CSaTScanData;

class CModel
{
  public:
    CModel(CParameters* pParameters, CSaTScanData* pData, BasePrint *pPrintDirection);
    virtual ~CModel();

    CParameters*  m_pParameters;
    CSaTScanData* m_pData;
    BasePrint *gpPrintDirection;
    FILE*         m_pDebugModelFile;

    virtual bool   ReadData() = 0;
    virtual bool   CalculateMeasure() = 0;
    virtual double GetLogLikelihoodForTotal() const = 0;
    virtual double CalcLogLikelihood(count_t n, measure_t u) = 0;
    virtual double CalcMonotoneLogLikelihood(const CPSMonotoneCluster& PSMCluster) = 0;
    virtual void   MakeData() = 0;
    virtual double GetPopulation(int m_iEllipseOffset, tract_t nCenter, tract_t nTracts,
                                 int nStartInterval, int nStopInterval) = 0;
    virtual bool   ReCalculateMeasure() {/* No action taken by default. */return true;}
};

//*****************************************************************************
#endif

