//---------------------------------------------------------------------------
#include "SaTScan.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#include "NormalModel.h"

/** constructor */
CNormalModel::CNormalModel(const CParameters & Parameters, CSaTScanData & Data, BasePrint & PrintDirection)
             :CModel(Parameters, Data, PrintDirection) {}

/** destructor */
CNormalModel::~CNormalModel() {}

double CNormalModel::GetPopulation(int m_iEllipseOffset, tract_t nCenter, tract_t nTracts, int nStartInterval, int nStopInterval) {
  ZdGenerateException("GetPopulation() not implementated.","CNormalModel");
  return 0;
}

