//---------------------------------------------------------------------------
#include "SaTScan.h"
#pragma hdrstop
#include "DataStreamInterface.h"
//---------------------------------------------------------------------------

/** constructor */
DataStreamInterface::DataStreamInterface(unsigned int iNumTimeIntervals, unsigned int iNumTracts)
                    :giNumTimeIntervals(iNumTimeIntervals), giNumTracts(iNumTracts) {
  Init();
}

/** destructor */
DataStreamInterface::~DataStreamInterface() {}

/** internal initialization */
void DataStreamInterface::Init() {
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
}

/** initializes case array */
void DataStreamInterface::ResetCaseArray(count_t t) {
  unsigned int  i, j;

  for (i=0; i < giNumTimeIntervals; ++i)
     for (j=0; j < giNumTracts; ++j)
        gppCaseArray[i][j] = 0;
}


