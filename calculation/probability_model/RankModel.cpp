//---------------------------------------------------------------------------
#include "SaTScan.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#include "RankModel.h"
#include "SaTScanData.h"

/** constructor */
CRankModel::CRankModel(const CParameters & Parameters, CSaTScanData & Data, BasePrint & PrintDirection)
           :CModel(Parameters, Data, PrintDirection) {}

/** destructor */
CRankModel::~CRankModel() {}

/** not implemented - throws exception */
double CRankModel::GetPopulation(int m_iEllipseOffset, tract_t nCenter, tract_t nTracts, int nStartInterval, int nStopInterval) {
  //ZdGenerateException("GetPopulation() not implementated.","CRankModel");
  return 0;
}
 
