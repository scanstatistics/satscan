//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "DataSetInterface.h"

/** constructor */
DataSetInterface::DataSetInterface(unsigned int iNumTimeIntervals, unsigned int iNumTracts)
                 :giNumTimeIntervals(iNumTimeIntervals), giNumTracts(iNumTracts) {
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
  gppSqMeasureArray=0;
  gppNCMeasureArray=0;
  gpTimeTrend=0;
  gpPSMeasureArray=0;
  gpPSSqMeasureArray=0;
  gpPTSqMeasureArray=0;
  gpppCategoryCaseArray=0;
  gppPTCategoryCaseArray=0;
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
  for (size_t t=0; t < vCategoryCases.size(); ++t)
     gvCategoryCaseArrays.push_back(vCategoryCases[t]->GetArray());
}

