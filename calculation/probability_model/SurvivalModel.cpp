//---------------------------------------------------------------------------
#include "SaTScan.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#include "SurvivalModel.h"
#include "SaTScanData.h"

/** constructor */
CSurvivalModel::CSurvivalModel(const CParameters & Parameters, CSaTScanData & Data, BasePrint & PrintDirection)
               :CModel(Parameters, Data, PrintDirection) {}

/** destructor */
CSurvivalModel::~CSurvivalModel() {}

/** not implemented - throws exception */
double CSurvivalModel::GetPopulation(int m_iEllipseOffset, tract_t nCenter, tract_t nTracts, int nStartInterval, int nStopInterval) {
  ZdGenerateException("GetPopulation() not implementated.","CNormalModel");
  return 0;
}

