//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "DataStream.h"
#include "SaTScanData.h"

/** constructor */
DataSet::DataSet(unsigned int iNumTimeIntervals, unsigned int iNumTracts, unsigned int iStreamIndex)
           : giNumTimeIntervals(iNumTimeIntervals), giNumTracts(iNumTracts), giStreamIndex(iStreamIndex)   {
  Init();
}

/** copy constructor */
DataSet::DataSet(const DataSet& thisStream) {
  ZdGenerateException("copy constructor not implemented.","DataSet");
}

/** destructor */
DataSet::~DataSet() {
  try {
    delete[] gpPTCasesArray;
    delete gpCasesHandler;
    delete gpNCCasesHandler;
    delete gpMeasureHandler;
    delete gpNCMeasureHandler;
    delete gpSqMeasureHandler;
    delete[] gpPTMeasureArray;
    delete gpCasesPerIntervalArray;
    delete gpMeasurePerIntervalArray;
    delete gpPTCategoryCasesHandler;
  }
  catch(...){}
}

/** overloaded assignment operator */
DataSet & DataSet::operator=(const DataSet& rhs) {
  ZdGenerateException("operator=() not implemented.","DataSet");
  return *this;
}

/** Creates a two dimensional array for storing case information, stratified
    by time interval index / location index. Initializes data to zero. If array
    already exists, only initialization occurs.
    Note that data in this array will be cumulated with respect to time intervals
    such that each earlier time interval will include later intervals data. */
void DataSet::AllocateCasesArray() {
  try {
    if (!gpCasesHandler)
      gpCasesHandler = new TwoDimensionArrayHandler<count_t>(giNumTimeIntervals+1, giNumTracts);
    gpCasesHandler->Set(0);
  }
  catch (ZdException &x) {
    x.AddCallpath("AllocateCasesArray()","DataSet");
    throw;
  }
}

/** Creates a vector of  two dimensional array for storing case information, stratified
    by time interval index / location index for 'iNumCategories'. Initializes data to zero. 
    Note that data in this array will be cumulated with respect to time intervals
    such that each earlier time interval will include later intervals data. */
void DataSet::AllocateCategoryCasesArray(unsigned int iNumCategories) {
  try {
    gvCasesByCategory.DeleteAllElements();
    for (unsigned int i=0; i < iNumCategories; ++i)
       gvCasesByCategory.push_back(new TwoDimensionArrayHandler<count_t>(giNumTimeIntervals+1, giNumTracts, 0));
  }
  catch (ZdException &x) {
    x.AddCallpath("AllocateCategoryCasesArray()","DataSet");
    throw;
  }
}

/** Creates a two dimensional array for storing expected case information (measure),
    stratified by time interval index / location index. Initializes data to zero.
    If array already exists, only initialization occurs.
    Note that data in this array will be cumulated with respect to time intervals
    such that each earlier time interval will include later intervals data. */
void DataSet::AllocateMeasureArray() {
  try {
    if (!gpMeasureHandler)
      gpMeasureHandler = new TwoDimensionArrayHandler<measure_t>(giNumTimeIntervals+1, giNumTracts);
    gpMeasureHandler->Set(0);
  }
  catch (ZdException &x) {
    x.AddCallpath("AllocateMeasureArray()","DataSet");
    throw;
  }
}

/** Creates a two dimensional array for storing expected case
    information (measure) squared, stratified by time interval index / location index.
    Initializes data to zero. If array already exists, only initialization occurs.
    Note that data in this array will be cumulated with respect to time intervals
    such that each earlier time interval will include later intervals data. */
void DataSet::AllocateSqMeasureArray() {
  try {
    if (!gpSqMeasureHandler)
      gpSqMeasureHandler = new TwoDimensionArrayHandler<measure_t>(giNumTimeIntervals+1, giNumTracts);
    gpSqMeasureHandler->Set(0);
  }
  catch (ZdException &x) {
    x.AddCallpath("AllocateSqMeasureArray()","DataSet");
    throw;
  }
}

/** Creates a one dimensional array for storing simulated expected case
    information (measure) squared, stratified by time interval index. Initializes data
    to zero. If array already exists, only initialization occurs.
    Note that data in this array will be cumulated with respect to time intervals
    such that each earlier time interval will include later intervals data. */
void DataSet::AllocatePTSqMeasureArray() {
  try {
    if (!gpPTSqMeasureArray)
      gpPTSqMeasureArray = new measure_t[giNumTimeIntervals+1];
    memset(gpPTSqMeasureArray, 0, (giNumTimeIntervals+1) * sizeof(measure_t));
  }
 catch (ZdException &x) {
   x.AddCallpath("AllocatePTSqMeasureArray()","DataSet");
   throw;
 }
}

void DataSet::AllocatePTCasesArray() {
  try {
    if (!gpPTCasesArray)
      gpPTCasesArray = new count_t[giNumTimeIntervals+1];
    memset(gpPTCasesArray, 0, (giNumTimeIntervals+1) * sizeof(count_t));
  }
  catch (ZdException &x) {
    x.AddCallpath("AllocatePTCasesArray()","DataSet");
    throw;
  }
}

/** Creates a two dimensional array for storing case information, stratified
    by time interval index / population category index.
    Initializes data to zero. If array already exists, only initialization occurs.
    Note that data in this array will be cumulated with respect to time intervals
    such that each earlier time interval will include later intervals data. */
void DataSet::AllocatePTCategoryCasesArray() {
  try {
    if (!gpPTCategoryCasesHandler)
      gpPTCategoryCasesHandler = new TwoDimensionArrayHandler<count_t>(gvCasesByCategory.size(), giNumTimeIntervals+1);
    gpPTCategoryCasesHandler->Set(0);
  }
  catch (ZdException &x) {
    x.AddCallpath("AllocatePTCategoryCasesArray()","DataSet");
    throw;
  }
}

/** Creates a one dimensional array for storing expected case information
    (measure), stratified by time interval index. Initializes data to zero. If
    array already exists, only initialization occurs.
    Note that data in this array will be cumulated with respect to time intervals
    such that each earlier time interval will include later intervals data. */
void DataSet::AllocatePTMeasureArray() {
  try {
    if (!gpPTMeasureArray)
      gpPTMeasureArray = new measure_t[giNumTimeIntervals+1];
    memset(gpPTMeasureArray, 0, (giNumTimeIntervals+1) * sizeof(measure_t));
  }
  catch (ZdException &x) {
    x.AddCallpath("AllocatePTMeasureArray()","DataSet");
    throw;
  }
}

/** Creates a two dimensional array for storing expected case information (measure),
    stratified by time interval index / location index. Initializes data to zero.
    If array already exists, only initialization occurs.
    Note that data is not cumulative.*/
void DataSet::AllocateNCMeasureArray() {
  try {
    if (!gpNCMeasureHandler)
      gpNCMeasureHandler = new TwoDimensionArrayHandler<measure_t>(giNumTimeIntervals+1, giNumTracts);
    gpNCMeasureHandler->Set(0);
  }
  catch(ZdException &x) {
    x.AddCallpath("AllocateNCMeasureArray()","DataSet");
    throw;
  }
}

/** Creates a two dimensional array for storing case information,
    stratified by time interval index / location index. Initializes data to zero.
    If array already exists, only initialization occurs.
    Note that data in this array is not cumulative. */
void DataSet::AllocateNCCasesArray() {
  try {
    if (!gpNCCasesHandler)
      gpNCCasesHandler = new TwoDimensionArrayHandler<count_t>(giNumTimeIntervals, giNumTracts);
    gpNCCasesHandler->Set(0);
  }
  catch(ZdException &x) {
    x.AddCallpath("AllocateNCCasesArray()","DataSet");
    throw;
  }
}

/** Returns pointer to one dimensional array representing case data stratified by
    time interval index. Throws exception of not allocated.
    Note that data in this array is cumulated with respect to time intervals
    such that each earlier time interval includes later intervals data. */
count_t * DataSet::GetPTCasesArray() const {
  try {
    if (!gpPTCasesArray)
      ZdGenerateException("Purely temporal cases array not allocated.","GetPTCasesArray()");
  }
  catch (ZdException &x) {
    x.AddCallpath("GetPTCasesArray()","DataSet");
    throw;
  }
  return gpPTCasesArray;
}

/** Returns pointer to two dimensional array representing case data stratified
    by time interval index / location index. Throws exception of not allocated.
    Note that data in this array is cumulated with respect to time intervals
    such that each earlier time interval includes later intervals data. */
count_t ** DataSet::GetCaseArray() const {
  try {
    if (!gpCasesHandler)
      ZdGenerateException("Cumulative case array not allocated.","GetCaseArray()");
  }
  catch (ZdException &x) {
    x.AddCallpath("GetCaseArray()","DataSet");
    throw;
  }
  return gpCasesHandler->GetArray();
}

/** Returns pointer to one dimensional array representing case data for each
    time interval index. Throws exception of not allocated. */
count_t * DataSet::GetCasesPerTimeIntervalArray() const {
  try {
    if (!gpCasesPerIntervalArray)
      ZdGenerateException("Cases per time interval array not allocated.","GetCasesPerTimeIntervalArray()");
  }
  catch (ZdException &x) {
    x.AddCallpath("GetCasesPerTimeIntervalArray()","DataSet");
    throw;
  }
  return gpCasesPerIntervalArray;
}

/** Returns pointer to two dimensional array representing expected case data stratified
    by time interval index / location index. Throws exception of not allocated.
    Note that data in this array is cumulated with respect to time intervals
    such that each earlier time interval includes later intervals data. */
measure_t ** DataSet::GetMeasureArray() const {
  try {
    if (!gpMeasureHandler)
      ZdGenerateException("Cumulative measure array not allocated.","GetMeasureArray()");
  }
  catch (ZdException &x) {
    x.AddCallpath("GetMeasureArray()","DataSet");
    throw;
  }
  return gpMeasureHandler->GetArray();
}

/** Returns reference to object to manages the two dimensional array representing
    expected case data stratified by time interval index / location index.
    Throws exception of not allocated.
    Note that data in this array is cumulated with respect to time intervals
    such that each earlier time interval includes later intervals data. */
TwoDimMeasureArray_t & DataSet::GetMeasureArrayHandler() {
  try {
    if (!gpMeasureHandler)
      ZdGenerateException("Measure array not allocated.","GetMeasureArrayHandler()");
  }
  catch (ZdException &x) {
    x.AddCallpath("GetMeasureArrayHandler()","DataSet");
    throw;
  }
  return *gpMeasureHandler;
}

/** Returns pointer to one dimensional array representing measure data for each
    time interval index. Throws exception of not allocated. */
measure_t * DataSet::GetMeasurePerTimeIntervalArray() const {
  try {
    if (!gpMeasurePerIntervalArray)
      ZdGenerateException("Measure per time interval array not allocated.","GetMeasureByTimeIntervalArray()");
  }
  catch (ZdException &x) {
    x.AddCallpath("GetMeasurePerTimeIntervalArray()","DataSet");
    throw;
  }
  return gpMeasurePerIntervalArray;
}

/** Returns reference to object to manages the two dimensional array representing
    expected case data stratified by time interval index / location index.
    Throws exception of not allocated.
    Note that data in this array is not cumulated with respect to time intervals. */
TwoDimMeasureArray_t & DataSet::GetNCMeasureArrayHandler() {
  try {
    if (!gpNCMeasureHandler)
      ZdGenerateException("Measure array not allocated.","GetNCMeasureArrayHandler()");
  }
  catch (ZdException &x) {
    x.AddCallpath("GetNCMeasureArrayHandler()","DataSet");
    throw;
  }
  return *gpNCMeasureHandler;
}

/** Returns reference to object to manages the two dimensional array representing
    expected case data, squared and stratified by
    time interval index / location index. Throws exception of not allocated.
    Note that data in this array is cumulated with respect to time intervals
    such that each earlier time interval includes later intervals data. */
TwoDimMeasureArray_t & DataSet::GetSqMeasureArrayHandler() {
  try {
    if (!gpSqMeasureHandler)
      ZdGenerateException("Simulation square measure array not allocated.","GetSqMeasureArrayHandler()");
  }
  catch (ZdException &x) {
    x.AddCallpath("GetSqMeasureArrayHandler()","DataSet");
    throw;
  }
  return *gpSqMeasureHandler;
}

/** Returns pointer to two dimensional array representing expected case data
    stratified by time interval index / location index. Throws exception of not
    allocated.
    Note that data in this array is cumulated with respect to time intervals
    such that each earlier time interval includes later intervals data. */
measure_t ** DataSet::GetSqMeasureArray() const {
  try {
    if (!gpSqMeasureHandler)
      ZdGenerateException("Squared measure array not allocated.","GetSqMeasureArray()");
  }
  catch (ZdException &x) {
    x.AddCallpath("GetSqMeasureArray()","DataSet");
    throw;
  }
  return gpSqMeasureHandler->GetArray();
}

/** Returns pointer to two dimensional array representing case data stratified
    by time interval index / location index. Throws exception of not allocated.
    Note that data in this array is not cumulated. */
count_t ** DataSet::GetNCCaseArray() const {
  try {
    if (!gpNCCasesHandler)
      ZdGenerateException("Non-cumulative case array not allocated.","GetNCCaseArray()");
  }
  catch (ZdException &x) {
    x.AddCallpath("GetNCCaseArray()","DataSet");
    throw;
  }
  return gpNCCasesHandler->GetArray();
}

/** Returns pointer to two dimensional array representing expected case data
    stratified by time interval index / location index. Throws exception of not
    allocated.
    Note that data in this array is not cumulated. */
measure_t ** DataSet::GetNCMeasureArray() const {
  try {
    if (!gpNCMeasureHandler)
      ZdGenerateException("Non-cumulative measure array not allocated.","GetNCMeasureArray()");
  }
  catch (ZdException &x) {
    x.AddCallpath("GetNCMeasureArray()","DataSet");
    throw;
  }
  return gpNCMeasureHandler->GetArray();
}

/** Returns pointer to three dimensional array representing case data stratified
    by time interval index / location index / population category index. Throws
    exception of not allocated.
    Note that data in this array is cumulated with respect to time intervals
    such that each earlier time interval includes later intervals data. */
count_t ** DataSet::GetPTCategoryCasesArray() const {
  try {
    if (!gpPTCategoryCasesHandler)
      ZdGenerateException("Purely temporal category case array not allocated.","GetPTCategoryCasesArray()");
  }
  catch (ZdException &x) {
    x.AddCallpath("GetPTCategoryCasesArray()","DataSet");
    throw;
  }
  return gpPTCategoryCasesHandler->GetArray();
}

/** Returns pointer to one dimensional array representing expected case data
    stratified by time interval index. Throws exception of not allocated.
    Note that data in this array is cumulated with respect to time intervals
    such that each earlier time interval includes later intervals data. */
measure_t * DataSet::GetPTMeasureArray() const {
  try {
    if (!gpPTMeasureArray)
      ZdGenerateException("PT Measure array not allocated.","GetPTMeasureArray()");
  }
  catch (ZdException &x) {
    x.AddCallpath("GetPTMeasureArray()","DataSet");
    throw;
  }
  return gpPTMeasureArray;
}

/** internal class intialization function */
void DataSet::Init() {
  gpPTCasesArray=0;
  gpCasesHandler=0;
  gpNCCasesHandler=0;
  gpMeasureHandler=0;
  gpNCMeasureHandler=0;
  gpSqMeasureHandler=0;
  gpPTMeasureArray=0;
  gpPTSqMeasureArray=0;
  gpCasesPerIntervalArray=0;
  gpMeasurePerIntervalArray=0;
  gpPTCategoryCasesHandler=0;
}

/** Ensures that two dimensional non-cumulative case array and case per time
    interval arrays are allocated and loads data from cumulative case array. */
void DataSet::SetNonCumulativeCaseArrays() {
  int           i, j;
  count_t    ** ppCases = gpCasesHandler->GetArray(), ** ppCases_NC;
  
  try {
    if (!gpNCCasesHandler)
      gpNCCasesHandler = new TwoDimensionArrayHandler<count_t>(giNumTimeIntervals, giNumTracts);
    ppCases_NC = gpNCCasesHandler->GetArray();
    if (!gpCasesPerIntervalArray)
      gpCasesPerIntervalArray = new count_t[giNumTimeIntervals+1];
    memset(gpCasesPerIntervalArray, 0, sizeof(count_t) * (giNumTimeIntervals+1));

    for (i=0; i < (int)giNumTracts; ++i)  {
      ppCases_NC[giNumTimeIntervals-1][i] = ppCases[giNumTimeIntervals-1][i];
      gpCasesPerIntervalArray[giNumTimeIntervals-1] += ppCases_NC[giNumTimeIntervals-1][i];
      for (j=giNumTimeIntervals-2; j >= 0; --j) {
        ppCases_NC[j][i] = ppCases[j][i] - ppCases[j+1][i];
        gpCasesPerIntervalArray[j] += ppCases_NC[j][i];
      }
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("SetNonCumulativeCaseArrays()","DataSet");
    throw;
  }
}

/** Sets one dimensional array representing case data stratified by time intervals
    from cumulative two dimensional case array. Allocates the one dimensional array
    is not allocated. Throws exception if two dimensional array is not allocated. */
void DataSet::SetPTCasesArray() {
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
    x.AddCallpath("SetPTCasesArray()","DataSet");
    throw;
  }
}

/** Sets two dimensional array representing case data stratified by time intervals
    from cumulative two dimensional case array of each category.  Throws exception
    if no two dimensional arrays are allocated. */
void DataSet::SetPTCategoryCasesArray() {
  unsigned int          i, j, c;
  count_t           **  ppCases, * pPTCategoryCases;

  try {
    if (!gvCasesByCategory.size())
      ZdGenerateException("Cumulative category cases array not allocated.","SetPTCategoryCasesArray()");

    AllocatePTCategoryCasesArray();
    for (c=0; c < gvCasesByCategory.size(); ++c) {
       ppCases = gvCasesByCategory[c]->GetArray();
       pPTCategoryCases = gpPTCategoryCasesHandler->GetArray()[c];
       for (i=0; i < giNumTimeIntervals; ++i)
          for (j=0; j < giNumTracts; ++j)
             pPTCategoryCases[i] += ppCases[i][j];
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("SetPTCategoryCasesArray()","DataSet");
    throw;
  }
}

/** Sets one dimensional array representing expected case data stratified by time
    intervals from cumulative two dimensional expected case array. Allocates the
    one dimensional array if not allocated. Throws exception if two dimensional
    array is not allocated. */
void DataSet::SetPTMeasureArray() {
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
    x.AddCallpath("SetPTMeasureArray()","DataSet");
    throw;
  }
}

/** Sets one dimensional array representing expected case data stratified by time
    intervals and squared from cumulative squared two dimensional expected case array.
    Allocates the one dimensional array if not allocated. Throws exception if two
    dimensional array is not allocated. */
void DataSet::SetPTSqMeasureArray() {
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
    x.AddCallpath("SetPTSqMeasureArray()","DataSet");
    throw;
  }
}



/** constructor */
RealDataStream::RealDataStream(unsigned int iNumTimeIntervals, unsigned int iNumTracts, unsigned int iStreamIndex)
               :DataSet(iNumTimeIntervals, iNumTracts, iStreamIndex) {
  Init();
  Setup();
}

/** copy constructor */
RealDataStream::RealDataStream(const RealDataStream& thisStream) : DataSet(thisStream) {}

/** destructor */
RealDataStream::~RealDataStream() {
  try {
    delete gpControlsHandler;
    delete gpPopulationMeasureHandler;
  }
  catch(...){}
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
measure_t** RealDataStream::AllocatePopulationMeasureArray() {
  try {
    if (!gpPopulationMeasureHandler)
      gpPopulationMeasureHandler = new TwoDimensionArrayHandler<measure_t>(gPopulation.GetNumPopulationDates(), giNumTracts);
    gpPopulationMeasureHandler->Set(0);
  }
  catch(ZdException &x) {
    x.AddCallpath("AllocatePopulationMeasureArray()","RealDataStream");
    throw;
  }
  return gpPopulationMeasureHandler->GetArray();
}

/** Adds case count to ordinal category, update PoplationData object and dataset
    data structures. Returns array which maintains counts. */
count_t ** RealDataStream::AddOrdinalCategoryCaseCount(double dOrdinalNumber, count_t Count) {
  size_t        tCategoryIndex;

  tCategoryIndex = gPopulation.AddOrdinalCategoryCaseCount(dOrdinalNumber, Count);
  if (gPopulation.GetNumOrdinalCategories() > gvCasesByCategory.size())
    gvCasesByCategory.insert(gvCasesByCategory.begin() + tCategoryIndex, new TwoDimensionArrayHandler<count_t>(giNumTimeIntervals+1, giNumTracts, 0));

  return gvCasesByCategory[tCategoryIndex]->GetArray();
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

/** Returns two dimensional array of intervals by locations for 'iCategoryIndex'. */
count_t ** RealDataStream::GetCategoryCaseArray(unsigned int iCategoryIndex) const {
  if (!gvCasesByCategory.size() || iCategoryIndex > gvCasesByCategory.size() - 1)
    ZdGenerateException("Index '%d' out of range.","GetCategoryCaseArray()");

  return gvCasesByCategory[iCategoryIndex]->GetArray();
}

/** Returns two dimensional array of intervals by locations for 'iCategoryIndex'. If
    'bCreateable' is true, allocates necessary arrays to make accessing requested
    category valid. */
count_t ** RealDataStream::GetCategoryCaseArray(unsigned int iCategoryIndex, bool bCreateable) {
  if (!gvCasesByCategory.size() || iCategoryIndex + 1 > gvCasesByCategory.size()) {
    if (!bCreateable)
      ZdGenerateException("Index out of range.","GetCategoryCaseArray()");
    size_t tNumAllocate = iCategoryIndex + 1 - gvCasesByCategory.size();
    for (size_t t=0; t < tNumAllocate; ++t)
      gvCasesByCategory.push_back(new TwoDimensionArrayHandler<count_t>(giNumTimeIntervals+1, giNumTracts, 0));
  }

  return gvCasesByCategory[iCategoryIndex]->GetArray();
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
  gpPopulationMeasureHandler=0;
  gdCalculatedTimeTrendPercentage=0;
}

/** Allocates and sets array that stores the total number of cases for each time
    interval as gotten from cumulative two dimensional case array. */
void RealDataStream::SetCasesPerTimeIntervalArray() {
  int             i, j;
  count_t      ** ppCases(gpCasesHandler->GetArray());

  try {
    if (!gpCasesPerIntervalArray)
      gpCasesPerIntervalArray = new count_t[giNumTimeIntervals+1];
    memset(gpCasesPerIntervalArray, 0, (giNumTimeIntervals+1) * sizeof(count_t));

    for (i=0; i < (int)giNumTracts; ++i) {
       gpCasesPerIntervalArray[giNumTimeIntervals-1] += ppCases[giNumTimeIntervals-1][i];
       for (j=giNumTimeIntervals-2; j >= 0; --j)
          gpCasesPerIntervalArray[j] += ppCases[j][i] - ppCases[j+1][i];
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("SetCasesPerTimeIntervalArray()","RealDataStream");
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
void RealDataStream::SetMeasurePerTimeIntervalsArray(measure_t ** ppNonCumulativeMeasure) {
  unsigned int   i, j;

  try {
    if (!gpMeasurePerIntervalArray)
      gpMeasurePerIntervalArray = new measure_t[giNumTimeIntervals+1];
      
    memset(gpMeasurePerIntervalArray, 0, (giNumTimeIntervals+1) * sizeof(measure_t));
    for (i=0; i < giNumTimeIntervals; ++i)
       for (j=0; j < giNumTracts; ++j)
          gpMeasurePerIntervalArray[i] += ppNonCumulativeMeasure[i][j];
  }
  catch (ZdException &x) {
    x.AddCallpath("SetMeasurePerTimeIntervalsArray()","RealDataStream");
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
                     :DataSet(iNumTimeIntervals, iNumTracts, iStreamIndex) {}

/** copy constructor */
SimulationDataStream::SimulationDataStream(const SimulationDataStream& thisStream) : DataSet(thisStream) {}

/** destructor */
SimulationDataStream::~SimulationDataStream() {}

/** Reads number of simulated cases from a text file rather than generating them randomly.
    NOTE: Data read from the file is not validated. This means that there is potential
          for the program to behave badly if:
          1) the data read from file does not match dimensions of ppSimCases
          2) the case counts read from file is inappropriate given real data -- probably access violations
          3) file does not actually contains numerical data
          Use of this feature should be discouraged except from someone who has
          detailed knowledge of how code works.                                                           */
void SimulationDataStream::ReadSimulationData(const CParameters& Parameters, unsigned int iSimulation) {
  unsigned int          i, t, tNumTracts = GetNumTracts();
  std::ifstream         filestream;

  //open file stream
  if (!filestream.is_open())
    filestream.open(Parameters.GetSimulationDataSourceFilename().c_str());
  if (!filestream)
    GenerateResolvableException("Error: Could not open file '%s' to read the simulated data.\n",
                                "ReadSimulationDataFromFile()", Parameters.GetSimulationDataSourceFilename().c_str());

  if (Parameters.GetProbabilityModelType() == ORDINAL)
    ReadSimulationDataOrdinal(filestream, iSimulation);
  else
    ReadSimulationDataStandard(filestream, iSimulation);
}

/** Reads simulation data from file stream into those data structures which are
    normally set in randomization process of OrdinalDenominatorDataRandomizer. */
void SimulationDataStream::ReadSimulationDataOrdinal(std::ifstream& filestream, unsigned int iSimulation) {
  unsigned int                  i, t, tNumTracts = GetNumTracts(),
                                tNumTimeIntervals = GetNumTimeIntervals();
  count_t                    ** ppSimCases = 0;
  CasesByCategory_t::iterator   itr=gvCasesByCategory.begin();

  //seek line offset for reading iSimulation'th simulation data
  t = (tNumTracts + 1) * gvCasesByCategory.size() * (iSimulation - 1);
  for (i=0; i < t; ++i)
    filestream.ignore(std::numeric_limits<int>::max(), '\n'/*InputFile.widen('\n')*/);

  for (; itr != gvCasesByCategory.end(); ++itr) {
     ppSimCases = (*itr)->GetArray();
     for (t=0; t < tNumTracts; ++t)
        for (i=0; i < tNumTimeIntervals; ++i)
           filestream >> ppSimCases[i][t];
  }
}

/** Reads simulation data from file stream into those data structures which are
    normally set in randomization process. */
void SimulationDataStream::ReadSimulationDataStandard(std::ifstream& filestream, unsigned int iSimulation) {
  unsigned int          i, t, tNumTracts = GetNumTracts(),
                        tNumTimeIntervals = GetNumTimeIntervals();
  count_t            ** ppSimCases = GetCaseArray();

  //seek line offset for reading iSimulation'th simulation data
  t = (tNumTracts + 1) * (iSimulation - 1);
  for (i=0; i < t; ++i)
    filestream.ignore(std::numeric_limits<int>::max(), '\n'/*InputFile.widen('\n')*/);

  for (t=0; t < tNumTracts; ++t)
     for (i=0; i < tNumTimeIntervals; ++i)
        filestream >> ppSimCases[i][t];
}

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

/** Prints the simulated data to a file. Format printed to file matches
    format expected for read as simulation data source. Truncates file
    when first opened for each analysis(i.e. first simulation).
    NOTE: The process of writing and reading simulation data to/from file
          is not well tested. It is known that it is not checking the validity
          of the files themselves or in relation to the running analysis.
          Also, not previsions have been made for this code to work for multiple
          data streams at this time.                                             */
void SimulationDataStream::WriteSimulationData(const CParameters& Parameters, int iSimulation) const {
  std::ofstream                 filestream;

  //open output file
  filestream.open(Parameters.GetSimulationDataOutputFilename().c_str(), (iSimulation == 1 ? ios::trunc : ios::ate));
  if (!filestream)
    GenerateResolvableException("Error: Could not open the simulated data output file '%s'.\n", "WriteSimulationData()",
                                Parameters.GetSimulationDataOutputFilename().c_str());

  if (Parameters.GetProbabilityModelType() == ORDINAL)
    WriteSimulationDataOrdinal(filestream);
  else
    WriteSimulationDataStandard(filestream);

  filestream.close();
}

/** Writes simulation data to file stream from those data structures which are
    normally set in randomization process of OrdinalDenominatorDataRandomizer. */
void SimulationDataStream::WriteSimulationDataOrdinal(std::ofstream& filestream) const {
  unsigned int                        i, t;
  count_t                          ** ppSimCases = 0;
  CasesByCategory_t::const_iterator   itr=gvCasesByCategory.begin();

  for (; itr != gvCasesByCategory.end(); ++itr) {
     ppSimCases = (*itr)->GetArray();
     for (t=0; t < GetNumTracts(); ++t) {
        for (i=0; i < GetNumTimeIntervals(); ++i)
           filestream << ppSimCases[i][t] << " ";
        filestream << std::endl;
     }
     filestream << std::endl;
  }   
}

/** Writes simulation data to file stream from those data structures which are
    normally set in randomization process. */
void SimulationDataStream::WriteSimulationDataStandard(std::ofstream& filestream) const {
  unsigned int                  t, i;
  count_t                    ** ppSimCases(GetCaseArray());

  for (t=0; t < GetNumTracts(); ++t) {
     for (i=0; i < GetNumTimeIntervals(); ++i)
        filestream << ppSimCases[i][t] << " ";
     filestream << std::endl;
  }
  filestream << std::endl;
}

