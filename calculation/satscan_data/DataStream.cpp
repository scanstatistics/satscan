//***************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//***************************************************************************
#include "DataStream.h"
#include "SaTScanData.h"

/** constructor */
DataStream::DataStream(unsigned int iNumTimeIntervals, unsigned int iNumTracts, unsigned int iStreamIndex)
           : giNumTimeIntervals(iNumTimeIntervals), giNumTracts(iNumTracts), giStreamIndex(iStreamIndex)   {
  Init();
}

/** copy constructor */
DataStream::DataStream(const DataStream& thisStream) {
  ZdGenerateException("copy constructor not implemented.","DataStream");
}

/** destructor */
DataStream::~DataStream() {
  try {
    delete[] gpPTCasesArray;
    delete gpCasesHandler;
    delete gpNCCasesHandler;
    delete gpMeasureHandler;
    delete gpNCMeasureHandler;
    delete gpSqMeasureHandler;
    delete[] gpPTMeasureArray;
  }
  catch(...){}
}

/** overloaded assignment operator */
DataStream & DataStream::operator=(const DataStream& rhs) {
  ZdGenerateException("operator=() not implemented.","DataStream");
  return *this;
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

/** Creates a one dimensional array for storing simulated expected case
    information (measure) squared, stratified by time interval index. Initializes data
    to zero. If array already exists, only initialization occurs.
    Note that data in this array will be cumulated with respect to time intervals
    such that each earlier time interval will include later intervals data. */
void DataStream::AllocatePTSqMeasureArray() {
  try {
    if (!gpPTSqMeasureArray)
      gpPTSqMeasureArray = new measure_t[giNumTimeIntervals+1];
    memset(gpPTSqMeasureArray, 0, (giNumTimeIntervals+1) * sizeof(measure_t));
  }
 catch (ZdException &x) {
   x.AddCallpath("AllocatePTSqMeasureArray()","DataStream");
   throw;
 }
}

void DataStream::AllocatePTCasesArray() {
  try {
    if (!gpPTCasesArray)
      gpPTCasesArray = new count_t[giNumTimeIntervals+1];
    memset(gpPTCasesArray, 0, (giNumTimeIntervals+1) * sizeof(count_t));
  }
  catch (ZdException &x) {
    x.AddCallpath("AllocatePTCasesArray()","DataStream");
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

/** Creates a two dimensional array for storing case information,
    stratified by time interval index / location index. Initializes data to zero.
    If array already exists, only initialization occurs.
    Note that data in this array is not cumulative. */
void DataStream::AllocateNCCasesArray() {
  try {
    if (!gpNCCasesHandler)
      gpNCCasesHandler = new TwoDimensionArrayHandler<count_t>(giNumTimeIntervals, giNumTracts);
    gpNCCasesHandler->Set(0);
  }
  catch(ZdException &x) {
    x.AddCallpath("AllocateNCCasesArray()","DataStream");
    throw;
  }
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

/** Returns reference to object to manages the two dimensional array representing
    expected case data stratified by time interval index / location index.
    Throws exception of not allocated.
    Note that data in this array is cumulated with respect to time intervals
    such that each earlier time interval includes later intervals data. */
TwoDimMeasureArray_t & DataStream::GetMeasureArrayHandler() {
  try {
    if (!gpMeasureHandler)
      ZdGenerateException("Measure array not allocated.","GetMeasureArrayHandler()");
  }
  catch (ZdException &x) {
    x.AddCallpath("GetMeasureArrayHandler()","DataStream");
    throw;
  }
  return *gpMeasureHandler;
}

/** Returns reference to object to manages the two dimensional array representing
    expected case data, squared and stratified by
    time interval index / location index. Throws exception of not allocated.
    Note that data in this array is cumulated with respect to time intervals
    such that each earlier time interval includes later intervals data. */
TwoDimMeasureArray_t & DataStream::GetSqMeasureArrayHandler() {
  try {
    if (!gpSqMeasureHandler)
      ZdGenerateException("Simulation square measure array not allocated.","GetSqMeasureArrayHandler()");
  }
  catch (ZdException &x) {
    x.AddCallpath("GetSqMeasureArrayHandler()","DataStream");
    throw;
  }
  return *gpSqMeasureHandler;
}

/** Returns pointer to two dimensional array representing expected case data
    stratified by time interval index / location index. Throws exception of not
    allocated.
    Note that data in this array is cumulated with respect to time intervals
    such that each earlier time interval includes later intervals data. */
measure_t ** DataStream::GetSqMeasureArray() const {
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

/** internal class intialization function */
void DataStream::Init() {
  gpPTCasesArray=0;
  gpCasesHandler=0;
  gpNCCasesHandler=0;
  gpMeasureHandler=0;
  gpNCMeasureHandler=0;
  gpSqMeasureHandler=0;
  gpPTMeasureArray=0;
  gpPTSqMeasureArray=0;
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



/** constructor */
RealDataStream::RealDataStream(unsigned int iNumTimeIntervals, unsigned int iNumTracts, unsigned int iStreamIndex)
               :DataStream(iNumTimeIntervals, iNumTracts, iStreamIndex) {
  Init();
  Setup();
}

/** copy constructor */
RealDataStream::RealDataStream(const RealDataStream& thisStream) : DataStream(thisStream) {}

/** destructor */
RealDataStream::~RealDataStream() {
  try {
    delete gpControlsHandler;
    delete gpCategoryCasesHandler;
    delete gpPopulationMeasureHandler;
  }
  catch(...){}
}

/** Creates a three dimensional array for storing case information, stratified
    by time interval index / location index / population category index.
    Initializes data to zero. If array already exists, only initialization occurs.
    Note that data in this array will be cumulated with respect to time intervals
    such that each earlier time interval will include later intervals data. */
void RealDataStream::AllocateCategoryCasesArray() {
  try {
    if (!gpCategoryCasesHandler)
      gpCategoryCasesHandler = new ThreeDimensionArrayHandler<count_t>(giNumTimeIntervals+1, giNumTracts, 1);
    gpCategoryCasesHandler->Set(0);
  }
  catch (ZdException &x) {
    x.AddCallpath("AllocateCategoryCasesArray()","RealDataStream");
    throw;
  }
}

/** Creates a two dimensional array for storing control information, stratified
    by time interval index / location index. Initializes data to zero. If array
    already exists, only initialization occurs.
    Note that data in this array will be cumulated with respect to time intervals
    such that each earlier time interval will include later intervals data. */
void RealDataStream::AllocateControlsArray() {
  try {
    if (!gpControlsHandler)
      gpControlsHandler = new TwoDimensionArrayHandler<count_t>(giNumTimeIntervals+1, giNumTracts);
    gpControlsHandler->Set(0);
  }
  catch (ZdException &x) {
    x.AddCallpath("AllocateControlsArray()","RealDataStream");
    throw;
  }
}

/** Creates a two dimensional array for storing expected case information (measure),
    stratified by population date index / location index. Initializes data to zero.
    If array already exists, only initialization occurs. */
void RealDataStream::AllocatePopulationMeasureArray() {
  try {
    if (!gpPopulationMeasureHandler)
      gpPopulationMeasureHandler = new TwoDimensionArrayHandler<measure_t>(gPopulation.GetNumPopulationDates(), giNumTracts);
    gpPopulationMeasureHandler->Set(0);
  }
  catch(ZdException &x) {
    x.AddCallpath("AllocatePopulationMeasureArray()","RealDataStream");
    throw;
  }
}

/** Validates that the population data read from file is correct in that a location
    does not contain case data while having zero population. */
void RealDataStream::CheckPopulationDataCases(CSaTScanData& Data) {
  try {
    gPopulation.CheckCasesHavePopulations(gpCasesHandler->GetArray()[0], Data);
  }
  catch(ZdException &x) {
    x.AddCallpath("CheckPopulationDataCases()","RealDataStream");
    throw;
  }
}

/** Frees two dimensional array representing expected case data data stratified by
    population date index / location index. */
void RealDataStream::FreePopulationMeasureArray() {
  try {
    delete gpPopulationMeasureHandler; gpPopulationMeasureHandler=0;
  }
  catch(...){}
}

/** Returns reference to class that contains three dimensional array of cases
    stratified by time interval index / location index / category index. Throws
    exception if structure not allocated.
    Note that data in this array is cumulated with respect to time intervals
    such that each earlier time interval includes later intervals data. */
ThreeDimCountArray_t & RealDataStream::GetCategoryCaseArrayHandler() {
  try {
    if (!gpCategoryCasesHandler)
      ZdGenerateException("Category cases handler not allocated.","GetCategoryCaseArrayHandler()");
  }
  catch (ZdException &x) {
    x.AddCallpath("GetCategoryCaseArrayHandler()","RealDataStream");
    throw;
  }
  return *gpCategoryCasesHandler;
}

/** Returns pointer to three dimensional array representing case data stratified
    by time interval index / location index / population category index. Throws
    exception of not allocated.
    Note that data in this array is cumulated with respect to time intervals
    such that each earlier time interval includes later intervals data. */
count_t *** RealDataStream::GetCategoryCaseArray() const {
  try {
    if (!gpCategoryCasesHandler)
      ZdGenerateException("Category case array not allocated.","GetCategoryCaseArray()");
  }
  catch (ZdException &x) {
    x.AddCallpath("GetCategoryCaseArray()","RealDataStream");
    throw;
  }
  return gpCategoryCasesHandler->GetArray();
}

/** Returns pointer to two dimensional array representing control data stratified
    by time interval index / location index. Throws exception of not allocated.
    Note that data in this array is cumulated with respect to time intervals
    such that each earlier time interval includes later intervals data. */
count_t ** RealDataStream::GetControlArray() const {
  try {
    if (!gpControlsHandler)
      ZdGenerateException("Cumulative control array not allocated.","GetControlArray()");
  }
  catch (ZdException &x) {
    x.AddCallpath("GetControlArray()","RealDataStream");
    throw;
  }
  return gpControlsHandler->GetArray();
}

/** Returns pointer to two dimensional array representing expected case data
    stratified by population data index / location index. Throws exception of not
    allocated.
    Note that data in this array is not cumulated. */
measure_t ** RealDataStream::GetPopulationMeasureArray() const {
  try {
    if (!gpPopulationMeasureHandler)
      ZdGenerateException("Population measure array not allocated.","GetPopulationMeasureArray()");
  }
  catch (ZdException &x) {
    x.AddCallpath("GetPopulationMeasureArray()","RealDataStream");
    throw;
  }
  return gpPopulationMeasureHandler->GetArray();
}

/** internal class intialization function */
void RealDataStream::Init() {
  gtTotalCases=0;
  gtTotalCasesAtStart=0;
  gtTotalControls=0;
  gtTotalControlsAtStart=0;
  gdTotalPop=0;
  gpControlsHandler=0;
  gtTotalMeasure=0;
  gtTotalMeasureAtStart=0;
  gpCategoryCasesHandler=0;
  gpPopulationMeasureHandler=0;
  gdCalculatedTimeTrendPercentage=0;
}

/** Allocates and sets array that stores the total number of cases for each time
    interval as gotten from cumulative two dimensional case array. */
void RealDataStream::SetCasesByTimeInterval() {
  int             i, j;
  count_t       * pPTCases, ** ppCases(gpCasesHandler->GetArray());

  try {
    if (!gpPTCasesArray)
      gpPTCasesArray = new count_t[giNumTimeIntervals+1];
    memset(gpPTCasesArray, 0, (giNumTimeIntervals+1) * sizeof(count_t));

    for (i=0; i < (int)giNumTracts; ++i) {
       gpPTCasesArray[giNumTimeIntervals-1] += ppCases[giNumTimeIntervals-1][i];
       for (j=giNumTimeIntervals-2; j >= 0; --j)
          gpPTCasesArray[j] += ppCases[j][i] - ppCases[j+1][i];
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("SetCasesByTimeInterval()","RealDataStream");
    throw;
  }
}

/** Allocates and sets two-dimensional array to be used as cumulative measure.
    Data assembled using previously defined non-cumulative measure. Cumulative
    array allocated if not already allocated. Throws exception if non-cumulative
    array is not allocated. */
void RealDataStream::SetCumulativeMeasureArrayFromNonCumulative() {
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
    x.AddCallpath("SetCumulativeMeasureArrayFromNonCumulative()","RealDataStream");
    throw;
  }
}

/** Sets measure array that represents expected case data for all time intervals
    from passed non-cumulative measure array. Array is allocated is not already
    allocated. Note that caller of function is responsible for ensuring the passed
    array is correct in terms of dimensions and non-cumulative attribute. */
void RealDataStream::SetMeasureByTimeIntervalsArray(measure_t ** ppNonCumulativeMeasure) {
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
    x.AddCallpath("SetMeasureByTimeIntervalsArray()","RealDataStream");
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
void RealDataStream::SetMeasureArrayAsCumulative() {
  int           i, t;
  measure_t  ** ppMeasure;

  if (!gpMeasureHandler)
    ZdGenerateException("Cumulative measure array not allocated.","SetMeasureArrayAsCumulative()");

  ppMeasure = gpMeasureHandler->GetArray();
  for (t=0; t < (int)giNumTracts; ++t)
     for (i=giNumTimeIntervals-2; i >= 0; --i)
        ppMeasure[i][t]= ppMeasure[i+1][t] + ppMeasure[i][t];
}

/** Sets non-cumulative measure from cumulative measure.
    Throw exception of cumulative measure array not allocated. Allocates
    non-cumulative measure array if not already allocated. */
void RealDataStream::SetNonCumulativeMeasureArrayFromCumulative() {
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
    x.AddCallpath("SetNonCumulativeMeasureArrayFromCumulative()","RealDataStream");
    throw;
  }
}

/** internal class setup function */
void RealDataStream::Setup() {
  try {
    gPopulation.SetNumTracts(giNumTracts);
  }
  catch (ZdException &x) {
    x.AddCallpath("Setup()","RealDataStream");
    throw;
  }
}





/** constructor */
SimulationDataStream::SimulationDataStream(unsigned int iNumTimeIntervals, unsigned int iNumTracts, unsigned int iStreamIndex)
                     :DataStream(iNumTimeIntervals, iNumTracts, iStreamIndex) {}

/** copy constructor */
SimulationDataStream::SimulationDataStream(const SimulationDataStream& thisStream) : DataStream(thisStream) {}

/** destructor */
SimulationDataStream::~SimulationDataStream() {}

/** Resets to all zero, the two dimensional array representing simulated case data,
    stratified by time interval index / location index and cumulative by time
    intervals. Throws exception if array not allocated. */
void SimulationDataStream::ResetCumulativeCaseArray() {
  try {
    if (!gpCasesHandler)
      ZdGenerateException("Cumulative simulation case array is not allocated.","ResetCumulativeSimCaseArray()");

    gpCasesHandler->Set(0);
  }
  catch (ZdException &x) {
    x.AddCallpath("ResetCumulativeSimCaseArray()","SimulationDataStream");
    throw;
  }
}


