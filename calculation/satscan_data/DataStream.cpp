//---------------------------------------------------------------------------
#include "SaTScan.h"
#pragma hdrstop
#include "DataStream.h"
#include "SaTScanData.h"
//---------------------------------------------------------------------------

/** constructor */
DataStream::DataStream(unsigned int iNumTimeIntervals, unsigned int iNumTracts)
           : giNumTimeIntervals(iNumTimeIntervals), giNumTracts(iNumTracts)   {
  try {
    Init();
    Setup();
  }
  catch(ZdException &x) {
    x.AddCallpath("DataStream()","DataStream");
    throw;
  }
}

/** destructor */
DataStream::~DataStream() {
  try {
    delete[] gpPTCasesArray;
    delete gpCasesHandler;
    delete gpNCCasesHandler;
    delete gpControlsHandler;
    delete gpMeasureHandler;
    delete gpNCMeasureHandler;
    delete gpCategoryMeasureHandler;
    delete gpCategoryCasesHandler;
    delete gpCategoryMeasureHandler;
    delete gpSqMeasureHandler;
    delete gpPopulationMeasureHandler;
    FreeSimulationStructures();
  }
  catch(...){}
}

void DataStream::AllocateCategoryCasesArray() {
  try {
    if (!gpCategoryCasesHandler)
      gpCategoryCasesHandler = new ThreeDimensionArrayHandler<count_t>(giNumTimeIntervals+1, giNumTracts, 1);
    gpCategoryCasesHandler->Set(0);
  }
  catch (ZdException &x) {
    x.AddCallpath("AllocateCategoryCasesArray()","DataStream");
    throw;
  }
}

void DataStream::AllocateCasesArray() {
  try {
    if (!gpCasesHandler)
      gpCasesHandler = new TwoDimensionArrayHandler<count_t>(giNumTimeIntervals+1, giNumTracts);
    gpCasesHandler->Set(0);
  }
  catch (ZdException &x) {
    x.AddCallpath("AllocateCasesArray()","DataStream");
    throw;
  }
}

void DataStream::AllocateControlsArray() {
  try {
    if (!gpControlsHandler)
      gpControlsHandler = new TwoDimensionArrayHandler<count_t>(giNumTimeIntervals+1, giNumTracts);
    gpControlsHandler->Set(0);
  }
  catch (ZdException &x) {
    x.AddCallpath("AllocateControlsArray()","DataStream");
    throw;
  }
}

void DataStream::AllocateMeasureArray() {
  try {
    if (!gpMeasureHandler)
      gpMeasureHandler = new TwoDimensionArrayHandler<measure_t>(giNumTimeIntervals+1, giNumTracts);
    gpMeasureHandler->Set(0);
  }
  catch (ZdException &x) {
    x.AddCallpath("AllocateMeasureArray()","DataStream");
    throw;
  }
}

void DataStream::AllocateSimMeasureArray() {
  try {
    if (!gpSimMeasureHandler)
      gpSimMeasureHandler = new TwoDimensionArrayHandler<measure_t>(giNumTimeIntervals+1, giNumTracts);
    gpSimMeasureHandler->Set(0);
  }
  catch (ZdException &x) {
    x.AddCallpath("AllocateSimMeasureArray()","DataStream");
    throw;
  }
}

void DataStream::AllocateSqMeasureArray() {
  try {
    if (!gpSqMeasureHandler)
      gpSqMeasureHandler = new TwoDimensionArrayHandler<measure_t>(giNumTimeIntervals+1, giNumTracts);
    gpSqMeasureHandler->Set(0);
  }
  catch (ZdException &x) {
    x.AddCallpath("AllocateSqMeasureArray()","DataStream");
    throw;
  }
}

void DataStream::AllocateSqSimMeasureArray() {
  try {
    if (!gpSimSqMeasureHandler)
      gpSimSqMeasureHandler = new TwoDimensionArrayHandler<measure_t>(giNumTimeIntervals+1, giNumTracts);
    gpSimSqMeasureHandler->Set(0);
  }
  catch (ZdException &x) {
    x.AddCallpath("AllocateSimSqMeasureArray()","DataStream");
    throw;
  }
}

void DataStream::AllocatePTSimMeasureArray() {
  try {
    if (!gpPTSimMeasureArray)
      gpPTSimMeasureArray = new measure_t[giNumTimeIntervals+1];
    memset(gpPTSimMeasureArray, 0, (giNumTimeIntervals+1) * sizeof(measure_t));
  }
  catch (ZdException &x) {
    x.AddCallpath("AllocatePTSimMeasureArray()","DataStream");
    throw;
  }
}

void DataStream::AllocatePTSimSqMeasureArray() {
  try {
    if (!gpPTSimSqMeasureArray)
      gpPTSimSqMeasureArray = new measure_t[giNumTimeIntervals+1];
    memset(gpPTSimSqMeasureArray, 0, (giNumTimeIntervals+1) * sizeof(measure_t));
  }
  catch (ZdException &x) {
    x.AddCallpath("AllocatePTSimSqMeasureArray()","DataStream");
    throw;
  }
}

void DataStream::AllocatePTMeasureArray() {
  try {
    if (!gpPTMeasureArray)
      gpPTMeasureArray = new measure_t[giNumTimeIntervals+1];
    memset(gpPTMeasureArray, 0, (giNumTimeIntervals+1) * sizeof(measure_t));
  }
  catch (ZdException &x) {
    x.AddCallpath("AllocatePTMeasureArray()","DataStream");
    throw;
  }
}

void DataStream::AllocateNCMeasureArray() {
  try {
    if (!gpNCMeasureHandler)
      gpNCMeasureHandler = new TwoDimensionArrayHandler<measure_t>(giNumTimeIntervals+1, giNumTracts);
    gpNCMeasureHandler->Set(0);
  }
  catch(ZdException &x) {
    x.AddCallpath("AllocateNCMeasureArray()","DataStream");
    throw;
  }
}

void DataStream::AllocatePopulationMeasureArray() {
  try {
    if (!gpPopulationMeasureHandler)
      gpPopulationMeasureHandler = new TwoDimensionArrayHandler<measure_t>(gPopulation.GetNumPopulationDates(), giNumTracts);
    gpPopulationMeasureHandler->Set(0);
  }
  catch(ZdException &x) {
    x.AddCallpath("AllocatePopulationMeasureArray()","DataStream");
    throw;
  }
}

void DataStream::AllocateSimulationCasesArray() {
  try {
    if (!gpSimCasesHandler)
      gpSimCasesHandler = new TwoDimensionArrayHandler<count_t>(giNumTimeIntervals, giNumTracts);
    gpSimCasesHandler->Set(0);  
  }
  catch(ZdException &x) {
    x.AddCallpath("AllocateSimulationCasesArray()","DataStream");
    throw;
  }
}

void DataStream::AllocateSimulationNCCasesArray() {
  try {
    if (!gpNCSimCasesHandler)
      gpNCSimCasesHandler = new TwoDimensionArrayHandler<count_t>(giNumTimeIntervals, giNumTracts);
    gpNCSimCasesHandler->Set(0);  
  }
  catch(ZdException &x) {
    x.AddCallpath("AllocateSimulationNCCasesArray()","DataStream");
    throw;
  }
}

void DataStream::AllocateSimulationPTCasesArray() {
  try {
    if (!gpPTSimCasesArray)
      gpPTSimCasesArray = new count_t[giNumTimeIntervals+1];
    memset(gpPTSimCasesArray, 0, (giNumTimeIntervals+1) * sizeof(count_t));
  }
  catch(ZdException &x) {
    x.AddCallpath("AllocateSimulationPTCasesArray()","DataStream");
    throw;
  }
}

void DataStream::CheckPopulationDataCases(CSaTScanData& Data) {
  try {
    gPopulation.CheckCasesHavePopulations(gpCasesHandler->GetArray()[0], Data);
  }
  catch(ZdException &x) {
    x.AddCallpath("CheckPopulationDataCases()","DataStream");
    throw;
  }
}

void DataStream::FreePopulationMeasureArray() {
  try {
    delete gpPopulationMeasureHandler; gpPopulationMeasureHandler=0;
  }
  catch(...){}
}

/** frees all memory allocated to simulation structures */
void DataStream::FreeSimulationStructures() {
  try {
    delete[] gpPTSimCasesArray; gpPTSimCasesArray=0;
    delete gpSimCasesHandler; gpSimCasesHandler=0;
    delete gpNCSimCasesHandler; gpNCSimCasesHandler=0;
    delete gpSimMeasureHandler; gpSimMeasureHandler=0;
    delete[] gpPTSimMeasureArray; gpPTSimMeasureArray=0;
    delete[] gpPTSimSqMeasureArray; gpPTSimSqMeasureArray=0;
    delete gpSimSqMeasureHandler; gpSimSqMeasureHandler=0;
  }
  catch(...){}
}

ThreeDimCountArray_t & DataStream::GetCategoryCaseArrayHandler() {
  try {
    if (!gpCategoryCasesHandler)
      ZdGenerateException("Category cases handler not allocated.","GetCategoryCaseArrayHandler()");
  }
  catch (ZdException &x) {
    x.AddCallpath("GetCategoryCaseArrayHandler()","DataStream");
    throw;
  }
  return *gpCategoryCasesHandler;
}

count_t * DataStream::GetPTCasesArray() const {
  try {
    if (!gpPTCasesArray)
      ZdGenerateException("Cases by time interval array not allocated.","GetPTCasesArray()");
  }
  catch (ZdException &x) {
    x.AddCallpath("GetPTCasesArray()","DataStream");
    throw;
  }
  return gpPTCasesArray;
}

count_t *** DataStream::GetCategoryCaseArray() {
  try {
    if (!gpCategoryCasesHandler)
      ZdGenerateException("Category case array not allocated.","GetCategoryCaseArray()");
  }
  catch (ZdException &x) {
    x.AddCallpath("GetCategoryCaseArray()","DataStream");
    throw;
  }
  return gpCategoryCasesHandler->GetArray();
}

/** returns pointer to two-dimensional array representing cases in a cumulative state. */
count_t ** DataStream::GetCaseArray() const {
  try {
    if (!gpCasesHandler)
      ZdGenerateException("Cumulative case array not allocated.","GetCaseArray()");
  }
  catch (ZdException &x) {
    x.AddCallpath("GetCaseArray()","DataStream");
    throw;
  }
  return gpCasesHandler->GetArray();
}

count_t ** DataStream::GetControlArray() const {
  try {
    if (!gpControlsHandler)
      ZdGenerateException("Cumulative control array not allocated.","GetControlArray()");
  }
  catch (ZdException &x) {
    x.AddCallpath("GetControlArray()","DataStream");
    throw;
  }
  return gpControlsHandler->GetArray();
}

/** returns pointer to two-dimensional array representing expected cases in
    a cumulative state. */
measure_t ** DataStream::GetMeasureArray() const {
  try {
    if (!gpMeasureHandler)
      ZdGenerateException("Non-cumulative measure not allocated.","GetMeasureArray()");
  }
  catch (ZdException &x) {
    x.AddCallpath("GetMeasureArray()","DataStream");
    throw;
  }
  return gpMeasureHandler->GetArray();
}

/** returns pointer to two-dimensional array representing cases in a cumulative state. */
count_t ** DataStream::GetSimCaseArray() const {
  try {
    if (!gpSimCasesHandler)
      ZdGenerateException("Cumulative case array not allocated.","GetSimCaseArray()");
  }
  catch (ZdException &x) {
    x.AddCallpath("GetSimCaseArray()","DataStream");
    throw;
  }
  return gpSimCasesHandler->GetArray();
}

measure_t ** DataStream::GetSimMeasureArray() const {
  try {
    if (!gpSimMeasureHandler)
      ZdGenerateException("Simulation measure array not allocated.","GetSimMeasureArray()");
  }
  catch (ZdException &x) {
    x.AddCallpath("GetSimMeasureArray()","DataStream");
    throw;
  }
  return gpSimMeasureHandler->GetArray();
}

TwoDimMeasureArray_t & DataStream::GetSimMeasureArrayHandler() {
  try {
    if (!gpSimMeasureHandler)
      ZdGenerateException("Simulation measure array not allocated.","GetSimMeasureArrayHandler()");
  }
  catch (ZdException &x) {
    x.AddCallpath("GetSimMeasureArrayHandler()","DataStream");
    throw;
  }
  return *gpSimMeasureHandler;
}

TwoDimMeasureArray_t & DataStream::GetSimSqMeasureArrayHandler() {
  try {
    if (!gpSimSqMeasureHandler)
      ZdGenerateException("Simulation square measure array not allocated.","GetSimSqMeasureArrayHandler()");
  }
  catch (ZdException &x) {
    x.AddCallpath("GetSimSqMeasureArrayHandler()","DataStream");
    throw;
  }
  return *gpSimSqMeasureHandler;
}

measure_t ** DataStream::GetSimSqMeasureArray() {
  try {
    if (!gpSimSqMeasureHandler)
      ZdGenerateException("Simulation squared measure array not allocated.","GetSimSqMeasureArray()");
  }
  catch (ZdException &x) {
    x.AddCallpath("GetSimSqMeasureArray()","DataStream");
    throw;
  }
  return gpSimSqMeasureHandler->GetArray();
}

measure_t ** DataStream::GetSqMeasureArray() {
  try {
    if (!gpSqMeasureHandler)
      ZdGenerateException("Squared measure array not allocated.","GetSqMeasureArray()");
  }
  catch (ZdException &x) {
    x.AddCallpath("GetSqMeasureArray()","DataStream");
    throw;
  }
  return gpSqMeasureHandler->GetArray();
}

/** returns pointer to two-dimensional array representing cases in a cumulative state. */
count_t ** DataStream::GetNCCaseArray() const {
  try {
    if (!gpNCCasesHandler)
      ZdGenerateException("Non-cumulative case array not allocated.","GetNCCaseArray()");
  }
  catch (ZdException &x) {
    x.AddCallpath("GetNCCaseArray()","DataStream");
    throw;
  }
  return gpNCCasesHandler->GetArray();
}

/** returns pointer to two-dimensional array representing expected cases in
    a non cumulative state. */
measure_t ** DataStream::GetNCMeasureArray() const {
  try {
    if (!gpNCMeasureHandler)
      ZdGenerateException("Non-cumulative measure not allocated.","GetNCMeasureArray()");
  }
  catch (ZdException &x) {
    x.AddCallpath("GetNCMeasureArray()","DataStream");
    throw;
  }
  return gpNCMeasureHandler->GetArray();
}

/** returns pointer to two-dimensional array representing cases in a cumulative state. */
count_t ** DataStream::GetNCSimCaseArray() const {
  try {
    if (!gpNCSimCasesHandler)
      ZdGenerateException("Non-cumulative simulation case array not allocated.","GetNCSimCaseArray()");
  }
  catch (ZdException &x) {
    x.AddCallpath("GetNCSimCaseArray()","DataStream");
    throw;
  }
  return gpNCSimCasesHandler->GetArray();
}

measure_t ** DataStream::GetPopulationMeasureArray() const {
  try {
    if (!gpPopulationMeasureHandler)
      ZdGenerateException("Population measure array not allocated.","GetPopulationMeasureArray()");
  }
  catch (ZdException &x) {
    x.AddCallpath("GetPopulationMeasureArray()","DataStream");
    throw;
  }
  return gpPopulationMeasureHandler->GetArray();
}

measure_t * DataStream::GetPTMeasureArray() const {
  try {
    if (!gpPTMeasureArray)
      ZdGenerateException("PT Measure array not allocated.","GetPTMeasureArray()");
  }
  catch (ZdException &x) {
    x.AddCallpath("GetPTMeasureArray()","DataStream");
    throw;
  }
  return gpPTMeasureArray;
}

count_t * DataStream::GetPTSimCasesArray() const {
  try {
    if (!gpPTSimCasesArray)
      ZdGenerateException("Simulation cases by time interval array not allocated.","GetPTSimCasesArray()");
  }
  catch (ZdException &x) {
    x.AddCallpath("GetPTSimCasesArray()","DataStream");
    throw;
  }
  return gpPTSimCasesArray;
}

/** internal intialization function */
void DataStream::Init() {
  gtTotalCases=0;
  gtTotalCasesAtStart=0;
  gtTotalControls=0;
  gtTotalControlsAtStart=0;
  gdTotalPop=0;
  gpPTCasesArray=0;
  gpPTSimCasesArray=0;
  gpCasesHandler=0;
  gpNCCasesHandler=0;
  gpControlsHandler=0;
  gpSimCasesHandler=0;
  gpNCSimCasesHandler=0;
  gtTotalMeasure=0;
  gtTotalMeasureAtStart=0;
  gpMeasureHandler=0;
  gpNCMeasureHandler=0;
  gpCategoryMeasureHandler=0;
  gpCategoryCasesHandler=0;
  gpCategoryMeasureHandler=0;
  gpSimMeasureHandler=0;
  gpPTSimMeasureArray=0;
  gpPTSimSqMeasureArray=0;
  gpSqMeasureHandler=0;
  gpSimSqMeasureHandler=0;
  gpPTMeasureArray=0;
  gpPopulationMeasureHandler=0;
  gpPTSqMeasureArray=0;
}

void DataStream::ResetCumulativeSimCaseArray() {
  try {
    if (!gpSimCasesHandler)
      ZdGenerateException("Cumulative simulation case array is not allocated.","ResetCumulativeSimCaseArray()");

    gpSimCasesHandler->Set(0);
  }
  catch (ZdException &x) {
    x.AddCallpath("ResetCumulativeSimCaseArray()","DataStream");
    throw;
  }
}

/** Allocates and sets two dimensional non-cumulative case array and purely
    temporal case array. */
void DataStream::SetCaseArrays() {
  try {
    if (!gpNCCasesHandler)
      gpNCCasesHandler = new TwoDimensionArrayHandler<count_t>(giNumTimeIntervals, giNumTracts);
    if (!gpPTCasesArray)
      gpPTCasesArray = new count_t[giNumTimeIntervals+1];
    SetCaseArrays(gpCasesHandler->GetArray(), gpNCCasesHandler->GetArray(), gpPTCasesArray);
  }
  catch (ZdException &x) {
    x.AddCallpath("SetCaseArrays()","DataStreamHandler");
    throw;
  }
}

/** Given two dimensional cumulative case array 'ppCases' - sets elements of
    non-cumulative two dimensional case array and purely temporal case array. */
void DataStream::SetCaseArrays(count_t** ppCases, count_t** pCases_NC, count_t* pCasesByTimeInt) {
  int   i, j;

  memset(pCasesByTimeInt, 0, sizeof(count_t) * (giNumTimeIntervals+1));
  for (i=0; i < (int)giNumTracts; ++i)  {
    pCases_NC[giNumTimeIntervals-1][i] = ppCases[giNumTimeIntervals-1][i];
    pCasesByTimeInt[giNumTimeIntervals-1] += pCases_NC[giNumTimeIntervals-1][i];
    for (j=giNumTimeIntervals-2; j >= 0; --j) {
      pCases_NC[j][i] = ppCases[j][i] - ppCases[j+1][i];
      pCasesByTimeInt[j] += pCases_NC[j][i];
    }
  }
}

/** Allocates array that stores the total number of cases for each time
    interval as gotten from cumulative two dimensional case array. */
void DataStream::SetCasesByTimeInterval() {
  int             i, j;
  count_t       * pPTCases, ** ppCases(gpCasesHandler->GetArray());

  try {
    gpPTCasesArray = new count_t[giNumTimeIntervals+1];
    memset(gpPTCasesArray, 0, (giNumTimeIntervals+1) * sizeof(count_t));

    for (i=0; i < (int)giNumTracts; ++i) {
       gpPTCasesArray[giNumTimeIntervals-1] += ppCases[giNumTimeIntervals-1][i];
       for (j=giNumTimeIntervals-2; j >= 0; --j)
          gpPTCasesArray[j] += ppCases[j][i] - ppCases[j+1][i];
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("SetCasesByTimeInterval()","DataStream");
    throw;
  }
}

/** Allocates two-dimensional array to be used as cumulative measure.
    Data assembled using previously defined non-cumulative measure. */
void DataStream::SetCumulativeMeasureArrayFromNonCumulative() {
  int                   i, t;
  measure_t          ** ppMeasure, ** ppMeasureNC;

  try {
    if (!gpNCMeasureHandler)
      ZdGenerateException("Non-cumulative measure is not allocated.","SetCumulativeMeasureArrayFromNonCumulative()");

    if (gpMeasureHandler) {
      delete gpMeasureHandler; gpMeasureHandler=0;
    }
    gpMeasureHandler = new TwoDimensionArrayHandler<measure_t>(giNumTimeIntervals+1, giNumTracts);

    ppMeasure = gpMeasureHandler->GetArray();
    ppMeasureNC = gpNCMeasureHandler->GetArray();
    for (t=0; t < (int)giNumTracts; ++t) {
       ppMeasure[giNumTimeIntervals-1][t] = ppMeasureNC[giNumTimeIntervals-1][t];
       for (i=giNumTimeIntervals-2; i >= 0; i--)
          ppMeasure[i][t] = ppMeasure[i+1][t] + ppMeasureNC[i][t];
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("SetCumulativeMeasureArrayFromNonCumulative()","DataStream");
    throw;
  }
}

/** sets cumulative measure array as cumulative 'in-place'
    - Currently the Poisson model adjusts the measure only when the it is non-cumulative.
      CPoissonModel::CalculateMeasure() and SVTT need to be looked at further
      to reduce the oddness here. */
void DataStream::SetMeasureArrayAsCumulative() {
  int           i, t;
  measure_t  ** ppMeasure;

  if (!gpMeasureHandler)
    ZdGenerateException("Cumulative measure array not allocated.","SetMeasureArrayAsCumulative()");

  ppMeasure = gpMeasureHandler->GetArray();
  for (t=0; t < (int)giNumTracts; ++t)
     for (i=giNumTimeIntervals-2; i >= 0; --i)
        ppMeasure[i][t]= ppMeasure[i+1][t] + ppMeasure[i][t];
}

/** Allocates and sets measure array that represents non-cumulative measure
    for all time intervals from passed non-cumulative measure array. */
void DataStream::SetMeasureByTimeIntervalsArray(measure_t ** ppNonCumulativeMeasure) {
  unsigned int   i, j;

  try {
    if (!gpPTMeasureArray)
      AllocatePTMeasureArray();

    memset(gpPTMeasureArray, 0, (giNumTimeIntervals+1) * sizeof(measure_t));
    for (i=0; i < giNumTimeIntervals; ++i)
       for (j=0; j < giNumTracts; ++j)
          gpPTMeasureArray[i] += ppNonCumulativeMeasure[i][j];
  }
  catch (ZdException &x) {
    x.AddCallpath("SetMeasureByTimeIntervalsArray()","DataStream");
    throw;
  }
}

/** Sets non-cumulative measure from cumulative measure.
    Non-cumulative measure array should not be already allocated. */
void DataStream::SetNonCumulativeMeasureArrayFromCumulative() {
  int           i, j;
  measure_t  ** ppMeasureNC, ** ppMeasure;

  try {
    if (!gpMeasureHandler)
      ZdGenerateException("Cumulative measure array not allocated.","SetNonCumulativeMeasureArrayFromCumulative()");

    if (gpNCMeasureHandler)
      gpNCMeasureHandler->Set(0);
    else
      gpNCMeasureHandler = new TwoDimensionArrayHandler<measure_t>(giNumTimeIntervals+1, giNumTracts);

    ppMeasure = gpMeasureHandler->GetArray();  
    ppMeasureNC = gpNCMeasureHandler->GetArray();
    for (i=0; i < (int)giNumTracts; ++i) {
      ppMeasureNC[giNumTimeIntervals-1][i] = ppMeasure[giNumTimeIntervals-1][i];
      for (j=giNumTimeIntervals-2; j>=0; --j)
        ppMeasureNC[j][i] = ppMeasure[j][i] - ppMeasure[j+1][i];
    }
  }
  catch (ZdException &x) {
    delete gpNCMeasureHandler; gpNCMeasureHandler=0;
    x.AddCallpath("SetNonCumulativeMeasureArrayFromCumulative()","DataStream");
    throw;
  }
}

void DataStream::SetPTCasesArray() {
  int                   i, j;
  count_t            ** ppCases;

  try {
    if (!gpCasesHandler)
      ZdGenerateException("Cumulative measure array not allocated.","SetPTMeasureArray()");

    if (!gpPTCasesArray) {
      gpPTCasesArray = new count_t[giNumTimeIntervals+1];
      memset(gpPTCasesArray, 0, (giNumTimeIntervals+1) * sizeof(count_t));
    }

    ppCases = gpCasesHandler->GetArray();
    for (i=0; i < (int)giNumTimeIntervals; ++i)
       for (gpPTCasesArray[i]=0, j=0; j < (int)giNumTracts; ++j)
          gpPTCasesArray[i] += ppCases[i][j];
  }
  catch (ZdException &x) {
    x.AddCallpath("SetPTCasesArray()","DataStream");
    throw;
  }
}

void DataStream::SetPTMeasureArray() {
  int           i, j;
  measure_t  ** ppMeasure;

  try {
    if (!gpMeasureHandler)
      ZdGenerateException("Cumulative measure array not allocated.","SetPTMeasureArray()");

//     delete[] gpPTMeasureArray; gpPTMeasureArray=0;
    gpPTMeasureArray = new measure_t[giNumTimeIntervals+1];
    memset(gpPTMeasureArray, 0, (giNumTimeIntervals+1)*sizeof(measure_t));

    ppMeasure = gpMeasureHandler->GetArray();
    for (i=0; i < (int)giNumTimeIntervals; ++i)
       for (j=0; j < (int)giNumTracts; ++j)
          gpPTMeasureArray[i] += ppMeasure[i][j];
  }
  catch (ZdException &x) {
    x.AddCallpath("SetPTMeasureArray()","DataStream");
    throw;
  }
}

void DataStream::SetPTSqMeasureArray() {
  int           i, j;
  measure_t  ** ppMeasure;

  try {
    if (!gpSqMeasureHandler)
      ZdGenerateException("Cumulative square measure array not allocated.","SetPTSqMeasureArray()");

//     delete[] gpPTMeasureArray; gpPTMeasureArray=0;
    gpPTSqMeasureArray = new measure_t[giNumTimeIntervals+1];
    memset(gpPTSqMeasureArray, 0, (giNumTimeIntervals+1)*sizeof(measure_t));

    ppMeasure = gpSqMeasureHandler->GetArray();
    for (i=0; i < (int)giNumTimeIntervals; ++i)
       for (j=0; j < (int)giNumTracts; ++j)
          gpPTSqMeasureArray[i] += ppMeasure[i][j];
  }
  catch (ZdException &x) {
    x.AddCallpath("SetPTSqMeasureArray()","DataStream");
    throw;
  }
}

void DataStream::SetPTSimMeasureArray() {
  int           i, j;
  measure_t  ** ppMeasure;

  try {
    if (!gpSimMeasureHandler)
      ZdGenerateException("Cumulative simulation measure array not allocated.","SetPTSimMeasureArray()");
    if (!gpPTSimMeasureArray)
      AllocatePTSimMeasureArray();

    ppMeasure = gpMeasureHandler->GetArray();
    for (i=0; i < (int)giNumTimeIntervals; ++i)
       for (gpPTMeasureArray[i]=0, j=0; j < (int)giNumTracts; ++j)
          gpPTMeasureArray[i] += ppMeasure[i][j];
  }
  catch (ZdException &x) {
    x.AddCallpath("SetPTSimMeasureArray()","DataStream");
    throw;
  }
}

void DataStream::SetPTSqSimMeasureArray() {
  int           i, j;
  measure_t  ** ppMeasure;

  try {
    if (!gpSimSqMeasureHandler)
      ZdGenerateException("Cumulative simulation square measure array not allocated.","SetPTSqSimMeasureArray()");
    if (!gpPTSimSqMeasureArray)
      AllocatePTSimSqMeasureArray();

    ppMeasure = gpSimSqMeasureHandler->GetArray();
    for (i=0; i < (int)giNumTimeIntervals; ++i)
       for (gpPTSimSqMeasureArray[i]=0, j=0; j < (int)giNumTracts; ++j)
          gpPTSimSqMeasureArray[i] += ppMeasure[i][j];
  }
  catch (ZdException &x) {
    x.AddCallpath("SetPTSqSimMeasureArray()","DataStream");
    throw;
  }
}

void DataStream::SetPTSimCasesArray() {
  int           i, j;
  count_t    ** ppSimCases;

  try {
    if (!gpSimCasesHandler)
      ZdGenerateException("Cumulative simulation case array not allocated.","SetPTSimCasesArray()");

    if (!gpPTSimCasesArray)
      AllocateSimulationPTCasesArray();

    ppSimCases = gpSimCasesHandler->GetArray();
    for (i=0; i < (int)giNumTimeIntervals; ++i)
       for (gpPTSimCasesArray[i]=0, j=0; j < (int)giNumTracts; ++j)
          gpPTSimCasesArray[i] += ppSimCases[i][j];
  }
  catch (ZdException &x) {
    x.AddCallpath("SetPTSimCasesArray()","DataStream");
    throw;
  }
}

void DataStream::SetSimCaseArrays() {
  try {

    if (!gpNCSimCasesHandler)
      gpNCSimCasesHandler = new TwoDimensionArrayHandler<count_t>(giNumTimeIntervals, giNumTracts);
    if (!gpPTSimCasesArray)
      gpPTSimCasesArray = new count_t[giNumTimeIntervals+1];
    SetCaseArrays(gpSimCasesHandler->GetArray(), gpNCSimCasesHandler->GetArray(), gpPTSimCasesArray);
  }
  catch (ZdException &x) {
    x.AddCallpath("SetSimCaseArrays()","DataStream");
    throw;
  }
}

void DataStream::Setup() {
  try {
    gPopulation.SetNumTracts(giNumTracts);
  }
  catch (ZdException &x) {
    x.AddCallpath("Setup()","DataStream");
    throw;
  }
}






