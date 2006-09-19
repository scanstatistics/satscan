//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "DataSet.h"
#include "SaTScanData.h"
#include "SSException.h" 

/** constructor */
DataSet::DataSet(unsigned int iNumTimeIntervals, unsigned int iNumTracts, unsigned int iSetIndex)
           : giIntervalsDimensions(iNumTimeIntervals), giLocationDimensions(iNumTracts), giSetIndex(iSetIndex),
             gpCaseData_PT(0), gpCaseData(0), gpCaseData_NC(0), gpMeasureData(0), gpMeasureData_NC(0),
             gpMeasureData_Sq(0), gpMeasureData_PT(0), gpMeasureData_PT_Sq(0), gpCaseData_PT_NC(0),
             gpMeasureData_PT_NC(0), gpCaseData_PT_Cat(0) {}

/** copy constructor */
DataSet::DataSet(const DataSet& thisSet) {
  ZdGenerateException("copy constructor not implemented.","DataSet");
}

/** destructor */
DataSet::~DataSet() {
  try {
    delete gpCaseData;
    delete gpCaseData_NC;
    delete gpCaseData_PT_Cat;
    delete[] gpCaseData_PT;
    delete[] gpCaseData_PT_NC;
    delete gpMeasureData;
    delete gpMeasureData_NC;
    delete gpMeasureData_Sq;
    delete[] gpMeasureData_PT;
    delete[] gpMeasureData_PT_Sq;
    delete[] gpMeasureData_PT_NC;
  }
  catch(...){}
}

DataSet * DataSet::Clone() const {
  ZdGenerateException("Clone() not implemented.","DataSet");
  return 0;
}

/** overloaded assignment operator */
DataSet & DataSet::operator=(const DataSet& rhs) {
  ZdGenerateException("operator=() not implemented.","DataSet");
  return *this;
}

/** Allocates two dimensional array which will represent cumulative case data,
    time by space. Initializes all elements of array to zero. */
TwoDimCountArray_t & DataSet::allocateCaseData() {
  try {
    if (!gpCaseData)
      gpCaseData = new TwoDimensionArrayHandler<count_t>(giIntervalsDimensions, giLocationDimensions);
    gpCaseData->Set(0);
  }
  catch (ZdException &x) {
    x.AddCallpath("allocateCaseData()","DataSet");
    throw;
  }
  return *gpCaseData;
}

/** Allocates 'iNumCategories' two dimensional arrays which will represent
    cumulative case data, category by time by space. Initializes all elements of
    array to zero. */
CasesByCategory_t & DataSet::allocateCaseData_Cat(unsigned int iNumCategories) {
  try {
    gvCaseData_Cat.DeleteAllElements();
    for (unsigned int i=0; i < iNumCategories; ++i)
       gvCaseData_Cat.push_back(new TwoDimensionArrayHandler<count_t>(giIntervalsDimensions, giLocationDimensions, 0));
  }
  catch (ZdException &x) {
    x.AddCallpath("allocateCaseData_Cat()","DataSet");
    throw;
  }
  return gvCaseData_Cat;
}

/** Allocates two dimensional array which will represent not cumulative case data, time by space.
    Initializes all elements of array to zero. */
TwoDimCountArray_t & DataSet::allocateCaseData_NC() {
  try {
    if (!gpCaseData_NC)
      gpCaseData_NC = new TwoDimensionArrayHandler<count_t>(giIntervalsDimensions, giLocationDimensions);
    gpCaseData_NC->Set(0);
  }
  catch(ZdException &x) {
    x.AddCallpath("allocateCaseData_NC()","DataSet");
    throw;
  }
  return *gpCaseData_NC;
}

/** Allocates one dimensional array which will represent cumulative case data,
    time only. Initializes all elements of array to zero. */
count_t * DataSet::allocateCaseData_PT() {
  try {
    if (!gpCaseData_PT)
      //allocate to # time intervals plus one -- a pointer to this array will be
      //passed directly CTimeIntervals object, where it is assumed element at index
      //'giIntervalsDimensions' is accessible and set to zero
      gpCaseData_PT = new count_t[giIntervalsDimensions+1];
    memset(gpCaseData_PT, 0, (giIntervalsDimensions+1) * sizeof(count_t));
  }
  catch (ZdException &x) {
    x.AddCallpath("allocateCaseData_PT()","DataSet");
    throw;
  }
  return gpCaseData_PT;
}

/** Allocates two dimensional array which will represent cumulative case data,
    category by time. Initializes all elements of array to zero. */
TwoDimCountArray_t & DataSet::allocateCaseData_PT_Cat(unsigned int iNumCategories) {
  try {
    if (!gpCaseData_PT_Cat)
      //allocate to # time intervals plus one -- a pointer to this array will be
      //passed directly CTimeIntervals object, where it is assumed element at index
      //'giIntervalsDimensions' is accessible and set to zero
      gpCaseData_PT_Cat = new TwoDimensionArrayHandler<count_t>(iNumCategories, giIntervalsDimensions+1);
    gpCaseData_PT_Cat->Set(0);
  }
  catch (ZdException &x) {
    x.AddCallpath("allocateCaseData_PT_Cat()","DataSet");
    throw;
  }
  return *gpCaseData_PT_Cat;
}

/** Allocates one dimensional array which will represent not cumulative case data,
    time only. Initializes all elements of array to zero. */
count_t * DataSet::allocateCaseData_PT_NC() {
  int             i, j;
  count_t      ** ppCases(gpCaseData->GetArray());

  try {
    if (!gpCaseData_PT_NC)
      //allocate to # time intervals plus one -- a pointer to this array will be
      //passed directly CTimeIntervals object, where it is assumed element at index
      //'giIntervalsDimensions' is accessible and set to zero
      gpCaseData_PT_NC = new count_t[giIntervalsDimensions];
    memset(gpCaseData_PT_NC, 0, giIntervalsDimensions * sizeof(count_t));
  }
  catch (ZdException &x) {
    x.AddCallpath("allocateCaseData_PT_NC()","RealDataSet");
    throw;
  }
  return gpCaseData_PT_NC;
}

/** Allocates two dimensional array which will represent cumulative measure data,
    time by space. Initializes all elements of array to zero. */
TwoDimMeasureArray_t & DataSet::allocateMeasureData() {
  try {
    if (!gpMeasureData)
      gpMeasureData = new TwoDimensionArrayHandler<measure_t>(giIntervalsDimensions, giLocationDimensions);
    gpMeasureData->Set(0);
  }
  catch (ZdException &x) {
    x.AddCallpath("allocateMeasureData()","DataSet");
    throw;
  }
  return *gpMeasureData;
}

/** Allocates two dimensional array which will represent not cumulative measure
    data, time by space. Initializes all elements of array to zero. */
TwoDimMeasureArray_t & DataSet::allocateMeasureData_NC() {
  try {
    if (!gpMeasureData_NC)
      gpMeasureData_NC = new TwoDimensionArrayHandler<measure_t>(giIntervalsDimensions, giLocationDimensions);
    gpMeasureData_NC->Set(0);
  }
  catch(ZdException &x) {
    x.AddCallpath("allocateMeasureData_NC()","DataSet");
    throw;
  }
  return *gpMeasureData_NC;
}

/** Allocates one dimensional array which will represent cumulative measure data,
    time only. Initializes all elements of array to zero. */
measure_t * DataSet::allocateMeasureData_PT() {
  try {
    if (!gpMeasureData_PT)
      //allocate to # time intervals plus one -- a pointer to this array will be
      //passed directly CTimeIntervals object, where it is assumed element at index
      //'giIntervalsDimensions' is accessible and set to zero
      gpMeasureData_PT = new measure_t[giIntervalsDimensions+1];
    memset(gpMeasureData_PT, 0, (giIntervalsDimensions+1) * sizeof(measure_t));
  }
  catch (ZdException &x) {
    x.AddCallpath("allocateMeasureData_PT()","DataSet");
    throw;
  }
  return gpMeasureData_PT;
}

/** Allocates one dimensional array which will represent not cumulative measure
    data, time only. Initializes all elements of array to zero. */
measure_t * DataSet::allocateMeasureData_PT_NC() {
  try {
    if (!gpMeasureData_PT_NC)
      //allocate to # time intervals plus one -- a pointer to this array will be
      //passed directly CTimeIntervals object, where it is assumed element at index
      //'giIntervalsDimensions' is accessible and set to zero
      gpMeasureData_PT_NC = new measure_t[giIntervalsDimensions+1];
    memset(gpMeasureData_PT_NC, 0, (giIntervalsDimensions+1) * sizeof(measure_t));
  }
  catch (ZdException &x) {
    x.AddCallpath("allocateMeasureData_PT_NC()","DataSet");
    throw;
  }
  return gpMeasureData_PT_NC;
}

/** Allocates one dimensional array which will represent not cumulative measure
    data squared, time only. Initializes all elements of array to zero. */
measure_t * DataSet::allocateMeasureData_PT_Sq() {
  try {
    if (!gpMeasureData_PT_Sq)
      //allocate to # time intervals plus one -- a pointer to this array will be
      //passed directly CTimeIntervals object, where it is assumed element at index
      //'giIntervalsDimensions' is accessible and set to zero
      gpMeasureData_PT_Sq = new measure_t[giIntervalsDimensions+1];
    memset(gpMeasureData_PT_Sq, 0, (giIntervalsDimensions+1) * sizeof(measure_t));
  }
  catch (ZdException &x) {
    x.AddCallpath("allocateMeasureData_PT_Sq()","DataSet");
    throw;
  }
  return gpMeasureData_PT_Sq;
}

/** Allocates two dimensional array which will represent cumulative measure data
    squared, time by space. Initializes all elements of array to zero. */
TwoDimMeasureArray_t & DataSet::allocateMeasureData_Sq() {
  try {
    if (!gpMeasureData_Sq)
      gpMeasureData_Sq = new TwoDimensionArrayHandler<measure_t>(giIntervalsDimensions, giLocationDimensions);
    gpMeasureData_Sq->Set(0);
  }
  catch (ZdException &x) {
    x.AddCallpath("allocateMeasureData_Sq()","DataSet");
    throw;
  }
  return *gpMeasureData_Sq;
}

/** Returns reference to object which manages a two dimensional array that represents
    cumulative case data, time by space. Throws ZdException if not allocated. */
TwoDimCountArray_t & DataSet::getCaseData() const {
  if (!gpCaseData) ZdGenerateException("gpCaseData not allocated.","getCaseData()");
  return *gpCaseData;
}

/** Returns reference to object which manages a two dimensional array that represents
    not cumulative case data, time by space. Throws ZdException if not allocated. */
TwoDimCountArray_t & DataSet::getCaseData_NC() const {
  if (!gpCaseData_NC) ZdGenerateException("gpCaseData_NC not allocated.","getCaseData_NC()");
  return *gpCaseData_NC;
}

/** Returns pointer to allocated array that represents cumulative case data, time
    only. Throws ZdException if not allocated. */
count_t * DataSet::getCaseData_PT() const {
  if (!gpCaseData_PT) ZdGenerateException("gpCaseData_PT not allocated.","getCaseData_PT()");
  return gpCaseData_PT;
}

/** Returns reference to object which manages a two dimensional array that represents
    cumulative case data, category by time. Throws ZdException if not allocated. */
TwoDimCountArray_t & DataSet::getCaseData_PT_Cat() const {
  if (!gpCaseData_PT_Cat) ZdGenerateException("gpCaseData_PT_Cat not allocated.","getCaseData_PT_Cat()");
  return *gpCaseData_PT_Cat;
}

/** Returns pointer to allocated array that represents not cumulative case data,
    time only. Throws ZdException if not allocated. */
count_t * DataSet::getCaseData_PT_NC() const {
  if (!gpCaseData_PT_NC) ZdGenerateException("gpCaseData_PT_NC not allocated.","getCaseData_PT_NC()");
  return gpCaseData_PT_NC;
}

/** Returns reference to object which manages a two dimensional array that represents
    cumulative measure data, time by space. Throws ZdException if not allocated. */
TwoDimMeasureArray_t & DataSet::getMeasureData() const {
  if (!gpMeasureData) ZdGenerateException("gpMeasureData not allocated.","getMeasureData()");
  return *gpMeasureData;
}

/** Returns reference to object which manages a two dimensional array that represents
    not cumulative measure data, time by space. Throws ZdException if not allocated. */
TwoDimMeasureArray_t & DataSet::getMeasureData_NC() const {
  if (!gpMeasureData_NC) ZdGenerateException("gpMeasureData_NC not allocated.","getMeasureData_NC()");
  return *gpMeasureData_NC;
}

/** Returns pointer to allocated array that represents cumulative measure data,
    time only. Throws ZdException if not allocated. */
measure_t * DataSet::getMeasureData_PT() const {
  if (!gpMeasureData_PT) ZdGenerateException("gpMeasureData_PT not allocated.","getMeasureData_PT()");
  return gpMeasureData_PT;
}

/** Returns pointer to allocated array that represents not cumulative measure data,
    time only. Throws ZdException if not allocated. */
measure_t * DataSet::getMeasureData_PT_NC() const {
  if (!gpMeasureData_PT_NC) ZdGenerateException("gpMeasureData_PT_NC not allocated.","getMeasureData_PT_NC()");
  return gpMeasureData_PT_NC;
}

/** Returns pointer to allocated array that represents cumulative measure data
    squared, time only. Throws ZdException if not allocated. */
measure_t * DataSet::getMeasureData_PT_Sq() const {
  if (!gpMeasureData_PT_Sq) ZdGenerateException("gpMeasureData_PT_Sq not allocated.","getMeasureData_PT_Sq()");
  return gpMeasureData_PT_Sq;
}

/** Returns reference to object which manages a two dimensional array that represents
    cumulative measure data squared, time by space. Throws ZdException if not allocated. */
TwoDimMeasureArray_t & DataSet::getMeasureData_Sq() const {
  if (!gpMeasureData_Sq) ZdGenerateException("gpMeasureData_Sq not allocated.","getMeasureData_Sq()");
  return *gpMeasureData_Sq;
}

/** Allocates and sets not cumulative case data (time by space) from cumulative
    case data (time by space). */
void DataSet::setCaseData_NC() {
  try {
    count_t ** ppCases = getCaseData().GetArray(), ** ppCases_NC = allocateCaseData_NC().GetArray();
    for (unsigned int t=0; t < giLocationDimensions; ++t)  {
      ppCases_NC[giIntervalsDimensions-1][t] = ppCases[giIntervalsDimensions-1][t];
      for (unsigned int i=0; i < giIntervalsDimensions - 1; ++i)
        ppCases_NC[i][i] = ppCases[i][t] - ppCases[i+1][t];
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("setCaseData_NC()","DataSet");
    throw;
  }
}

/** Allocates and sets cumulative case data (time only ) from cumulative
    case data (time by space). */
void DataSet::setCaseData_PT() {
  try {
    count_t ** ppCases = getCaseData().GetArray();
    allocateCaseData_PT();
    for (unsigned int i=0, t; i < giIntervalsDimensions; ++i)
       for (t=0, gpCaseData_PT[i]=0; t < giLocationDimensions; ++t)
          gpCaseData_PT[i] += ppCases[i][t];
  }
  catch (ZdException &x) {
    x.AddCallpath("setCaseData_PT()","DataSet");
    throw;
  }
}

/** Allocates and sets cumulative case data (category by time) from cumulative
    case data (category by time by space). */
void DataSet::setCaseData_PT_Cat() {
  try {
    allocateCaseData_PT_Cat(gvCaseData_Cat.size());
    for (unsigned int c=0; c < gvCaseData_Cat.size(); ++c) {
       count_t ** ppCases = getCaseData_Cat()[c]->GetArray();
       count_t * pPTCategoryCases = getCaseData_PT_Cat().GetArray()[c];
       for (unsigned int i=0; i < giIntervalsDimensions; ++i)
          for (unsigned int t=0; t < giLocationDimensions; ++t)
             pPTCategoryCases[i] += ppCases[i][t];
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("setCaseData_PT_Cat()","DataSet");
    throw;
  }
}

/** Allocates and sets not cumulative case data (time only) from cumulative
    case data (time by space). */
void DataSet::setCaseData_PT_NC() {
  try {
    allocateCaseData_PT_NC();
    count_t ** ppCases = getCaseData().GetArray();
    for (unsigned int t=0; t < giLocationDimensions; ++t)  {
      gpCaseData_PT_NC[giIntervalsDimensions-1] += ppCases[giIntervalsDimensions-1][t];
      for (unsigned int i=0; i < giIntervalsDimensions - 1; ++i)
        gpCaseData_PT_NC[i] += ppCases[i][t] - ppCases[i+1][t];
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("setCaseData_PT_NC()","DataSet");
    throw;
  }
}

/** Allocates and sets not cumulative measure data (time by space) from cumulative
    measure data (time by space). */
void DataSet::setMeasureData_NC() {
  try {
    measure_t ** ppMeasure = getMeasureData().GetArray();
    measure_t ** ppMeasureNC = allocateMeasureData_NC().GetArray();
    for (unsigned int t=0; t < giLocationDimensions; ++t) {
      ppMeasureNC[giIntervalsDimensions-1][t] = ppMeasure[giIntervalsDimensions-1][t];
      for (unsigned int i=0; i < giIntervalsDimensions - 1; ++i)
        ppMeasureNC[i][t] = ppMeasure[i][t] - ppMeasure[i+1][t];
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("setMeasureData_NC()","DataSet");
    throw;
  }
}

/** Allocates and sets cumulative measure data (time only) from cumulative
    measure data (time by space). */
void DataSet::setMeasureData_PT() {
  try {
    measure_t  ** ppMeasure = getMeasureData().GetArray();
    allocateMeasureData_PT();
    for (unsigned int i=0; i < giIntervalsDimensions; ++i)
       for (unsigned int t=0; t < giLocationDimensions; ++t)
          gpMeasureData_PT[i] += ppMeasure[i][t];
  }
  catch (ZdException &x) {
    x.AddCallpath("setMeasureData_PT()","DataSet");
    throw;
  }
}

/** Allocates and sets not cumulative measure data (time only) from cumulative
    measure data (time by space). */
void DataSet::setMeasureData_PT_NC() {
  try {
    allocateMeasureData_PT_NC();
    measure_t ** ppMeasure = getMeasureData().GetArray();
    for (unsigned int t=0; t < giLocationDimensions; ++t) {
      gpMeasureData_PT_NC[giIntervalsDimensions-1] += ppMeasure[giIntervalsDimensions-1][t];
      for (unsigned int i=0; i < giIntervalsDimensions - 1; ++i)
        gpMeasureData_PT_NC[i] += ppMeasure[i][t] - ppMeasure[i+1][t];
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("setMeasureData_PT_NC()","DataSet");
    throw;
  }
}

/** Allocates and sets cumulative measure data squared (time only) from cumulative
    measure data squared (time by space). */
void DataSet::setMeasureData_PT_Sq() {
  try {
    allocateMeasureData_PT_Sq();
    measure_t ** ppMeasure = getMeasureData_Sq().GetArray();
    for (unsigned int i=0; i < giIntervalsDimensions; ++i)
       for (unsigned int t=0; t < giLocationDimensions; ++t)
          gpMeasureData_PT_Sq[i] += ppMeasure[i][t];
  }
  catch (ZdException &x) {
    x.AddCallpath("setMeasureData_PT_Sq()","DataSet");
    throw;
  }
}

/** Sets cumulative measure data from self, assuming self is currently not cumulative.
    Repeated calls to this method or calling when data is not in a not cumulative state
    will produce erroneous data. */
void DataSet::setMeasureDataToCumulative() {
  if (giIntervalsDimensions < 2) return;
  measure_t ** ppMeasure = getMeasureData().GetArray();
  for (unsigned int t=0; t < giLocationDimensions; ++t) {
     for (unsigned int i=giIntervalsDimensions-2; ; --i) {
        ppMeasure[i][t]= ppMeasure[i][t] + ppMeasure[i+1][t];
        if (i == 0) break;
     }
  }
}

////////////////////////////////////////////////////////////////////////////////
//////////////////////////// RealDataSet ///////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/** constructor */
RealDataSet::RealDataSet(unsigned int iNumTimeIntervals, unsigned int iNumTracts, unsigned int iSetIndex)
            :DataSet(iNumTimeIntervals, iNumTracts, iSetIndex),
             gtTotalCases(0), gtTotalCasesAtStart(0), gtTotalControls(0), gdTotalPop(0),
             gpControlData(0), gtTotalMeasure(0), gtTotalMeasureAtStart(0),
             gdCalculatedTimeTrendPercentage(0), gpCaseData_Censored(0), gtTotalMeasureSq(0) {
  gPopulation.SetNumTracts(giLocationDimensions);
}

/** copy constructor */
RealDataSet::RealDataSet(const RealDataSet& thisSet) : DataSet(thisSet) {}

/** destructor */
RealDataSet::~RealDataSet() {
  try {
    delete gpControlData;
    delete gpCaseData_Censored;
  }
  catch(...){}
}

/** Allocates two dimensional array which will represent cumulative censored case
    data (time by space). Initializes all elements of array to zero. */
TwoDimCountArray_t & RealDataSet::allocateCaseData_Censored() {
  try {
    if (!gpCaseData_Censored)
      gpCaseData_Censored = new TwoDimensionArrayHandler<count_t>(giIntervalsDimensions, giLocationDimensions);
    gpCaseData_Censored->Set(0);
  }
  catch (ZdException &x) {
    x.AddCallpath("allocateCaseData_Censored()","RealDataSet");
    throw;
  }
  return *gpCaseData_Censored;
}

/** Allocates two dimensional array which will represent cumulative control
    data (time by space). Initializes all elements of array to zero. */
TwoDimCountArray_t & RealDataSet::allocateControlData() {
  try {
    if (!gpControlData)
      gpControlData = new TwoDimensionArrayHandler<count_t>(giIntervalsDimensions, giLocationDimensions);
    gpControlData->Set(0);
  }
  catch (ZdException &x) {
    x.AddCallpath("allocateControlData()","RealDataSet");
    throw;
  }
  return *gpControlData;
}

/** Adds case count to ordinal category, updating PoplationData object. If a two
    dimensional array does not exist for category associated with 'dOrdinalNumber',
    one is allocated. Returns referance to two dimensional array associated with
    'dOrdinalNumber'. */
TwoDimCountArray_t & RealDataSet::addOrdinalCategoryCaseCount(double dOrdinalNumber, count_t Count) {
  size_t        tCategoryIndex;

  tCategoryIndex = gPopulation.AddOrdinalCategoryCaseCount(dOrdinalNumber, Count);
  if (gPopulation.GetNumOrdinalCategories() > gvCaseData_Cat.size())
    gvCaseData_Cat.insert(gvCaseData_Cat.begin() + tCategoryIndex, new TwoDimensionArrayHandler<count_t>(giIntervalsDimensions, giLocationDimensions, 0));

  return *gvCaseData_Cat[tCategoryIndex];
}

/** Validates that the population data is correct in that a location
    does not contain case data while having zero population. */
void RealDataSet::checkPopulationDataCases(CSaTScanData& Data) {
  try {
    gPopulation.CheckCasesHavePopulations(gpCaseData->GetArray()[0], Data);
  }
  catch(ZdException &x) {
    x.AddCallpath("checkPopulationDataCases()","RealDataSet");
    throw;
  }
}

/** Returns reference to object which manages a two dimensional array that represents
    cumulative case data, time by space, for category at 'iCategoryIndex'. */
TwoDimCountArray_t & RealDataSet::getCategoryCaseData(unsigned int iCategoryIndex) const {
  return *gvCaseData_Cat.at(iCategoryIndex);
}

/** Returns reference to object which manages a two dimensional array that represents
    cumulative case data, time by space, for category at 'iCategoryIndex'.
    If category does not exist and 'bCreateable' is true, creates new two dimensional
    array for category and returns referance. Otherwise throws ZdException.*/
TwoDimCountArray_t & RealDataSet::getCategoryCaseData(unsigned int iCategoryIndex, bool bCreateable) {
  if (!gvCaseData_Cat.size() || iCategoryIndex + 1 > gvCaseData_Cat.size()) {
    if (!bCreateable)
      ZdGenerateException("Index %u out of range [size=%u].","GetCategoryCaseArray()", iCategoryIndex, gvCaseData_Cat.size());
    size_t tNumAllocate = iCategoryIndex + 1 - gvCaseData_Cat.size();
    for (size_t t=0; t < tNumAllocate; ++t)
      gvCaseData_Cat.push_back(new TwoDimensionArrayHandler<count_t>(giIntervalsDimensions, giLocationDimensions, 0));
  }

  return *gvCaseData_Cat.at(iCategoryIndex);
}

/** Returns reference to object which manages a two dimensional array that represents
    cumulative censored case data, time by space. */
TwoDimCountArray_t & RealDataSet::getCaseData_Censored() const {
  if (!gpCaseData_Censored) ZdGenerateException("gpCaseData_Censored not allocated.","getCaseData_Censored()");
  return *gpCaseData_Censored;
}

/** Returns reference to object which manages a two dimensional array that represents
    cumulative control data, time by space. */
TwoDimCountArray_t & RealDataSet::getControlData() const {
  if (!gpControlData) ZdGenerateException("gpControlData not allocated.","getControlData()");
  return *gpControlData;
}

