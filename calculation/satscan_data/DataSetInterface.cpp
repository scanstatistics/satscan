//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "DataSetInterface.h"

/** constructor */
DataSetInterface::DataSetInterface(unsigned int iNumTimeIntervals, unsigned int iNumTracts, unsigned int add_data_start_index)
                 :giNumTimeIntervals(iNumTimeIntervals), giNumTracts(iNumTracts), _add_data_start_index(add_data_start_index){
  Init();
}

/** destructor */
DataSetInterface::~DataSetInterface() {}

/** internal initialization */
void DataSetInterface::Init() {
  gTotalCases=0;
  gTotalControls=0;
  gTotalMeasure=0;
  gppCaseArray=0;
  gppNCCaseArray=0;
  gpPTCaseArray=0;
  gpPSCaseArray=0;
  gppMeasureArray=0;
  gpPTMeasureArray=0;
  gppMeasureAuxArray=0;
  gppMeasureAux2Array = 0;
  gppNCMeasureArray=0;
  gpTimeTrend=0;
  gpPSMeasureArray=0;
  gpPSMeasureAuxArray=0;
  gpPSMeasureAux2Array=0;
  gpPTMeasureAuxArray=0;
  gpPTMeasureAux2Array = 0;
  gpppCategoryCaseArray=0;
  gppPTCategoryCaseArray=0;
  giNumOrdinalCategories=0;
  gTotalMeasureAux=0;
  gTotalMeasureAux2 = 0;
  _randomizer=0;

  gppBatchIndexesArray = 0;
  gpPsBatchIndexesArray = 0;
  gpPtBatchIndexesArray = 0;
  gppPositiveBatchIndexesArray = 0;
  gpPsPositiveBatchIndexesArray = 0;
  gpPtPositiveBatchIndexesArray = 0;
}

/** initializes case array */
void DataSetInterface::ResetCaseArray(count_t t) {
  unsigned int  i, j;

  for (i=0; i < giNumTimeIntervals; ++i)                           
     for (j=0; j < giNumTracts; ++j)
        gppCaseArray[i][j] = 0;
}

/** Sets internal data structure to hold pointers to passed category case arrays. */
void DataSetInterface::SetCategoryCaseArrays(const CasesByCategory_t& vCategoryCases) {
  gvCategoryCaseArrays.clear();
  for (size_t t=0; t < vCategoryCases.size(); ++t)
     gvCategoryCaseArrays.push_back(vCategoryCases[t]->GetArray());
}

