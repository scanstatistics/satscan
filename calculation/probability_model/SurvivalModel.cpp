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

double CSurvivalModel::GetPopulation(unsigned int iStream, int m_iEllipseOffset, tract_t nCenter,
                                   tract_t nTracts, int nStartInterval, int nStopInterval) const {
  ZdGenerateException("GetPopulation() not implementated.","CSurvivalModel");
  return 0;
}

