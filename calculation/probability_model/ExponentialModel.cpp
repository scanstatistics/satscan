//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "ExponentialModel.h"
#include "SaTScanData.h"

/** constructor */
ExponentialModel::ExponentialModel(const CParameters & Parameters, CSaTScanData & Data, BasePrint & PrintDirection)
                 :CModel(Parameters, Data, PrintDirection) {}

/** destructor */
ExponentialModel::~ExponentialModel() {}

double ExponentialModel::GetPopulation(unsigned int iStream, int m_iEllipseOffset, tract_t nCenter,
                                   tract_t nTracts, int nStartInterval, int nStopInterval) const {
  ZdGenerateException("GetPopulation() not implementated.","ExponentialModel");
  return 0;
}

