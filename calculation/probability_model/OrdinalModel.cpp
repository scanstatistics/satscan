//***************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//***************************************************************************
#include "OrdinalModel.h"

/** constructor */
OrdinalModel::OrdinalModel(const CParameters & Parameters, CSaTScanData & Data, BasePrint & PrintDirection)
             :CModel(Parameters, Data, PrintDirection) {}

/** destructor */
OrdinalModel::~OrdinalModel() {}

double OrdinalModel::GetPopulation(unsigned int iStream, int m_iEllipseOffset, tract_t nCenter,
                                   tract_t nTracts, int nStartInterval, int nStopInterval) const {
  ZdGenerateException("GetPopulation() not implementated YET!","OrdinalModel");
  return 0;
}

