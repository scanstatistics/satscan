//---------------------------------------------------------------------------
#include "SaTScan.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#include "DataStream.h"
#include "SaTScanData.h"

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

/** Creates a three dimensional array for storing case information, stratified
    by time interval index / location index / population category index.
    Initializes data to zero. If array already exists, only initialization occurs.
    Note that data in this array will be cumulated with respect to time intervals
    such that each earlier time interval will include later intervals data. */
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

/** Creates a two dimensional array for storing case information, stratified
    by time interval index / location index. Initializes data to zero. If array
    already exists, only initialization occurs.
    Note that data in this array will be cumulated with respect to time intervals
    such that each earlier time interval will include later intervals data. */
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

/** Creates a two dimensional array for storing control information, stratified
    by time interval index / location index. Initializes data to zero. If array
    already exists, only initialization occurs.
    Note that data in this array will be cumulated with respect to time intervals
    such that each earlier time interval will include later intervals data. */
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

/** Creates a two dimensional array for storing expected case information (measure),
    stratified by time interval index / location index. Initializes data to zero.
    If array already exists, only initialization occurs.
    Note that data in this array will be cumulated with respect to time intervals
    such that each earlier time interval will include later intervals data. */
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

/** Creates a two dimensional array for storing simulated expected case
    information (measure), stratified by time interval index / location index.
    Initializes data to zero. If array already exists, only initialization occurs.
    Note that data in this array will be cumulated with respect to time intervals
    such that each earlier time interval will include later intervals data. */
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

/** Creates a two dimensional array for storing expected case
    information (measure) squared, stratified by time interval index / location index.
    Initializes data to zero. If array already exists, only initialization occurs.
    Note that data in this array will be cumulated with respect to time intervals
    such that each earlier time interval will include later intervals data. */
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

/** Creates a two dimensional array for storing simulated expected case
    information (measure) squared, stratified by time interval index / location index.
    Initializes data to zero. If array already exists, only initialization occurs.
    Note that data in this array will be cumulated with respect to time intervals
    such that each earlier time interval will include later intervals data. */
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

/** Creates a one dimensional array for storing simulated expected case
    information (measure), stratified by time interval index. Initializes data
    to zero. If array already exists, only initialization occurs.
    Note that data in this array will be cumulated with respect to time intervals
    such that each earlier time interval will include later intervals data. */
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

/** Creates a one dimensional array for storing simulated expected case
    information (measure) squared, stratified by time interval index. Initializes data
    to zero. If array already exists, only initialization occurs.
    Note that data in this array will be cumulated with respect to time intervals
    such that each earlier time interval will include later intervals data. */
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

/** Creates a one dimensional array for storing expected case information
    (measure), stratified by time interval index. Initializes data to zero. If
    array already exists, only initialization occurs.
    Note that data in this array will be cumulated with respect to time intervals
    such that each earlier time interval will include later intervals data. */
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

/** Creates a two dimensional array for storing expected case information (measure),
    stratified by time interval index / location index. Initializes data to zero.
    If array already exists, only initialization occurs.
    Note that data is not cumulative.*/
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

/** Creates a two dimensional array for storing expected case information (measure),
    stratified by population date index / location index. Initializes data to zero.
    If array already exists, only initialization occurs. */
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

/** Creates a two dimensional array for storing simulated case information,
    stratified by time interval index / location index. Initializes data to zero.
    If array already exists, only initialization occurs.
    Note that data in this array will be cumulated with respect to time intervals
    such that each earlier time interval will include later intervals data. */
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

/** Creates a two dimensional array for storing simulated case information,
    stratified by time interval index / location index. Initializes data to zero.
    If array already exists, only initialization occurs.
    Note that data in this array is not cumulative. */
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

/** Creates a one dimensional array for storing case information, stratified by
    time interval index. Initializes data to zero. If array already exists, only
    initialization occurs.
    Note that data in this array will be cumulated with respect to time intervals
    such that each earlier time interval will include later intervals data. */
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

/** Validates that the population data read from file is correct in that a location
    does not contain case data while having zero population. */
void DataStream::CheckPopulationDataCases(CSaTScanData& Data) {
  try {
    gPopulation.CheckCasesHavePopulations(gpCasesHandler->GetArray()[0], Data);
  }
  catch(ZdException &x) {
    x.AddCallpath("CheckPopulationDataCases()","DataStream");
    throw;
  }
}

/** Frees two dimensional array representing expected case data data stratified by
    population date index / location index. */
void DataStream::FreePopulationMeasureArray() {
  try {
    delete gpPopulationMeasureHandler; gpPopulationMeasureHandler=0;
  }
  catch(...){}
}

/** Frees all memory allocated to simulation structures. */
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

/** Returns reference to class that contains three dimensional array of cases
    stratified by time interval index / location index / category index. Throws
    exception if structure not allocated.
    Note that data in this array is cumulated with respect to time intervals
    such that each earlier time interval includes later intervals data. */
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

/** Returns pointer to one dimensional array representing case data stratified by
    time interval index. Throws exception of not allocated.
    Note that data in this array is cumulated with respect to time intervals
    such that each earlier time interval includes later intervals data. */
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

/** Returns pointer to three dimensional array representing case data stratified
    by time interval index / location index / population category index. Throws
    exception of not allocated.
    Note that data in this array is cumulated with respect to time intervals
    such that each earlier time interval includes later intervals data. */
count_t *** DataStream::GetCategoryCaseArray() const {
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

/** Returns pointer to two dimensional array representing case data stratified
    by time interval index / location index. Throws exception of not allocated.
    Note that data in this array is cumulated with respect to time intervals
    such that each earlier time interval includes later intervals data. */
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

/** Returns pointer to two dimensional array representing control data stratified
    by time interval index / location index. Throws exception of not allocated.
    Note that data in this array is cumulated with respect to time intervals
    such that each earlier time interval includes later intervals data. */
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

/** Returns pointer to two dimensional array representing expected case data stratified
    by time interval index / location index. Throws exception of not allocated.
    Note that data in this array is cumulated with respect to time intervals
    such that each earlier time interval includes later intervals data. */
measure_t ** DataStream::GetMeasureArray() const {
  try {
    if (!gpMeasureHandler)
      ZdGenerateException("Cumulative measure array not allocated.","GetMeasureArray()");
  }
  catch (ZdException &x) {
    x.AddCallpath("GetMeasureArray()","DataStream");
    throw;
  }
  return gpMeasureHandler->GetArray();
}

/** Returns pointer to two dimensional array representing simulated case data stratified
    by time interval index / location index. Throws exception of not allocated.
    Note that data in this array is cumulated with respect to time intervals
    such that each earlier time interval includes later intervals data. */
count_t ** DataStream::GetSimCaseArray() const {
  try {
    if (!gpSimCasesHandler)
      ZdGenerateException("Cumulative simulation case array not allocated.","GetSimCaseArray()");
  }
  catch (ZdException &x) {
    x.AddCallpath("GetSimCaseArray()","DataStream");
    throw;
  }
  return gpSimCasesHandler->GetArray();
}

/** Returns pointer to two dimensional array representing simulated expected case
    data stratified by time interval index / location index. Throws exception of
    not allocated.
    Note that data in this array is cumulated with respect to time intervals
    such that each earlier time interval includes later intervals data. */
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

/** Returns reference to object to manages the two dimensional array representing
    simulated expected case data stratified by time interval index / location index.
    Throws exception of not allocated.
    Note that data in this array is cumulated with respect to time intervals
    such that each earlier time interval includes later intervals data. */
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

/** Returns reference to object to manages the two dimensional array representing
    simulated expected case data, squared and stratified by
    time interval index / location index. Throws exception of not allocated.
    Note that data in this array is cumulated with respect to time intervals
    such that each earlier time interval includes later intervals data. */
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

/** Returns pointer to two dimensional array representing simulated expected
    case data, squared and stratified by time interval index / location index.
    Throws exception of not allocated.
    Note that data in this array is cumulated with respect to time intervals
    such that each earlier time interval includes later intervals data. */
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

/** Returns pointer to two dimensional array representing expected case data
    stratified by time interval index / location index. Throws exception of not
    allocated.
    Note that data in this array is cumulated with respect to time intervals
    such that each earlier time interval includes later intervals data. */
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

/** Returns pointer to two dimensional array representing case data stratified
    by time interval index / location index. Throws exception of not allocated.
    Note that data in this array is not cumulated. */
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

/** Returns pointer to two dimensional array representing expected case data
    stratified by time interval index / location index. Throws exception of not
    allocated.
    Note that data in this array is not cumulated. */
measure_t ** DataStream::GetNCMeasureArray() const {
  try {
    if (!gpNCMeasureHandler)
      ZdGenerateException("Non-cumulative measure array not allocated.","GetNCMeasureArray()");
  }
  catch (ZdException &x) {
    x.AddCallpath("GetNCMeasureArray()","DataStream");
    throw;
  }
  return gpNCMeasureHandler->GetArray();
}

/** Returns pointer to two dimensional array representing simulated case data
    stratified by time interval index / location index. Throws exception of not
    allocated.
    Note that data in this array is not cumulated. */
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

/** Returns pointer to two dimensional array representing expected case data
    stratified by population data index / location index. Throws exception of not
    allocated.
    Note that data in this array is not cumulated. */
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

/** Returns pointer to one dimensional array representing expected case data
    stratified by time interval index. Throws exception of not allocated.
    Note that data in this array is cumulated with respect to time intervals
    such that each earlier time interval includes later intervals data. */
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

/** Returns pointer to one dimensional array representing simulated case data
    stratified by time interval index. Throws exception of not allocated.
    Note that data in this array is cumulated with respect to time intervals
    such that each earlier time interval includes later intervals data. */
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

/** internal class intialization function */
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

/** Resets to all zero, the two dimensional array representing simulated case data,
    stratified by time interval index / location index and cumulative by time
    intervals. Throws exception if array not allocated. */
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

/** Ensures that two dimensional non-cumulative case array and purely temporal
    case arrays are allocated and passes to function SetCaseArrays(). */
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
    non-cumulative two dimensional case array and purely temporal case array.
    Caller of function is responsible for ensuring that dimensions of arrays
    are inline with those expected in function. */
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

/** Allocates and sets array that stores the total number of cases for each time
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

/** Allocates and sets two-dimensional array to be used as cumulative measure.
    Data assembled using previously defined non-cumulative measure. Cumulative
    array allocated if not already allocated. Throws exception if non-cumulative
    array is not allocated. */
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

/** Sets cumulative measure array as cumulative 'in-place'. Note that it is assumed
    that the cumulative measure array is currently not cumulative. Throws exception
    if array not already allocated. Repeated calls to this function will produce
    erroneous data.
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

/** Sets measure array that represents expected case data for all time intervals
    from passed non-cumulative measure array. Array is allocated is not already
    allocated. Note that caller of function is responsible for ensuring the passed
    array is correct in terms of dimensions and non-cumulative attribute. */
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
    Throw exception of cumulative measure array not allocated. Allocates
    non-cumulative measure array if not already allocated. */
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

/** Sets one dimensional array representing case data stratified by time intervals
    from cumulative two dimensional case array. Allocates the one dimensional array
    is not allocated. Throws exception if two dimensional array is not allocated. */
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

/** Sets one dimensional array representing expected case data stratified by time
    intervals from cumulative two dimensional expected case array. Allocates the
    one dimensional array if not allocated. Throws exception if two dimensional
    array is not allocated. */
void DataStream::SetPTMeasureArray() {
  int           i, j;
  measure_t  ** ppMeasure;

  try {
    if (!gpMeasureHandler)
      ZdGenerateException("Cumulative measure array not allocated.","SetPTMeasureArray()");

    //delete[] gpPTMeasureArray; gpPTMeasureArray=0;
    if (!gpPTMeasureArray)
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

/** Sets one dimensional array representing expected case data stratified by time
    intervals and squared from cumulative squared two dimensional expected case array.
    Allocates the one dimensional array if not allocated. Throws exception if two
    dimensional array is not allocated. */
void DataStream::SetPTSqMeasureArray() {
  int           i, j;
  measure_t  ** ppMeasure;

  try {
    if (!gpSqMeasureHandler)
      ZdGenerateException("Cumulative square measure array not allocated.","SetPTSqMeasureArray()");

//     delete[] gpPTMeasureArray; gpPTMeasureArray=0;
    if (!gpPTSqMeasureArray)
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

/** Sets one dimensional array representing simulated expected case data stratified
    by time intervals from cumulative two dimensional simulated expected case array.
    Allocates the one dimensional array if not allocated. Throws exception if two
    dimensional array is not allocated. */
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

/** Sets one dimensional array representing simulated expected case data stratified
    by time intervals and squared from cumulative squared two dimensional simulated
    expected case array. Allocates the one dimensional array if not allocated.
    Throws exception if two dimensional array is not allocated. */
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

/** Sets one dimensional array representing simulated case data stratified
    by time intervals from cumulative two dimensional simulated case array.
    Allocates the one dimensional array if not allocated. Throws exception
    if two dimensional array is not allocated. */
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

/** Ensures that two dimensional non-cumulative simulated case array and purely temporal
    simulated case arrays are allocated and passes to function SetCaseArrays(). */
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

/** internal class setup function */
void DataStream::Setup() {
  try {
    gPopulation.SetNumTracts(giNumTracts);
  }
  catch (ZdException &x) {
    x.AddCallpath("Setup()","DataStream");
    throw;
  }
}






