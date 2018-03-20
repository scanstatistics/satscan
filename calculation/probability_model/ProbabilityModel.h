//******************************************************************************
#ifndef __PROBABILITYMODEL_H
#define __PROBABILITYMODEL_H
//******************************************************************************
#include "SaTScan.h"
#include "Parameters.h"
#include "DataSet.h"
#include "DataSetInterface.h"

class CSaTScanData;
class CCluster;

class CModel {
  public:
    CModel();
    virtual ~CModel();

    virtual void                CalculateMeasure(RealDataSet& DataSet, const CSaTScanData& DataHub) = 0;
    virtual double              GetPopulation(size_t tSetIndex, const CCluster& Cluster, const CSaTScanData& DataHub) const;
    virtual double              GetLocationPopulation(size_t tSetIndex, tract_t tractIdx, const CCluster& Cluster, const CSaTScanData& DataHub) const;
};
//******************************************************************************
#endif

