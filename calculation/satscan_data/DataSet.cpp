//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "DataSet.h"
#include "SaTScanData.h"
#include "SSException.h"
#include "MetaTractManager.h"


void printCountArray(const TwoDimCountArray_t& arrayClass, FILE * stream) {
    count_t ** pp = arrayClass.GetArray();
    for (unsigned int i=0; i < arrayClass.Get1stDimension(); ++i)
        for (unsigned int t=0; t < arrayClass.Get2ndDimension(); ++t)
            fprintf(stream, "Count [%i][%i] = %li\n", i, t, pp[i][t]);
    fprintf(stream, "\n");
    fflush(stream);
}

void printMeasureArray(const TwoDimMeasureArray_t& arrayClass, FILE * stream) {
    measure_t ** pp = arrayClass.GetArray();
    for (unsigned int i=0; i < arrayClass.Get1stDimension(); ++i)
        for (unsigned int t=0; t < arrayClass.Get2ndDimension(); ++t)
            fprintf(stream, "Measure [%i][%i] = %lf\n", i, t, pp[i][t]);
    fprintf(stream, "\n");
    fflush(stream);
}

/** constructor */
DataSet::DataSet(unsigned int iNumTimeIntervals, unsigned int iNumTracts, unsigned int iMetaLocations, const CParameters& parameters, unsigned int iSetIndex)
           : giIntervalsDimensions(iNumTimeIntervals), giLocationDimensions(iNumTracts), giMetaLocations(iMetaLocations), giSetIndex(iSetIndex),
             gpCaseData_PT(0), gpCaseData(0), gpCaseData_NC(0), gpMeasureData(0), gpMeasureData_NC(0),
             gpMeasureData_Aux(0), gpMeasureData_Aux2(0), gpMeasureData_PT(0), gpMeasureData_PT_Aux(0), gpCaseData_PT_NC(0),
             gpMeasureData_PT_NC(0), gpCaseData_PT_Cat(0), gpPositiveBatchIndexes(0), gpMeasureData_PT_Aux2(0), gpPositiveBatchIndexes_PT(0),
			 gpTimeTrend(AbstractTimeTrend::getTimeTrend(parameters)), gtTotalMeasureAux(0), gtTotalMeasureAux2(0){}

/** copy constructor */
DataSet::DataSet(const DataSet& thisSet) {
  throw prg_error("copy constructor not implemented.","DataSet");
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
    delete gpMeasureData_Aux;
    delete gpMeasureData_Aux2;
    delete[] gpMeasureData_PT;
    delete[] gpMeasureData_PT_Aux;
    delete[] gpMeasureData_PT_Aux2;
    delete[] gpMeasureData_PT_NC;
	delete gpTimeTrend;
    delete gpPositiveBatchIndexes;
    delete[] gpPositiveBatchIndexes_PT;
  }
  catch(...){}
}

DataSet * DataSet::Clone() const {
  throw prg_error("Clone() not implemented.","DataSet");
}

/** overloaded assignment operator */
DataSet & DataSet::operator=(const DataSet& rhs) {
  throw prg_error("operator=() not implemented.","DataSet");
}

/** Allocates two dimensional array which will represent cumulative case data,
    time by space. Initializes all elements of array to zero. */
TwoDimCountArray_t & DataSet::allocateCaseData() {
  try {
    if (!gpCaseData)
      gpCaseData = new TwoDimensionArrayHandler<count_t>(giIntervalsDimensions, giLocationDimensions + giMetaLocations);
    gpCaseData->Set(0);
  }
  catch (prg_exception& x) {
    x.addTrace("allocateCaseData()","DataSet");
    throw;
  }
  return *gpCaseData;
}

/** Allocates 'iNumCategories' two dimensional arrays which will represent
    cumulative case data, category by time by space. Initializes all elements of
    array to zero. */
CasesByCategory_t & DataSet::allocateCaseData_Cat(unsigned int iNumCategories) {
  try {
    gvCaseData_Cat.killAll();
    for (unsigned int i=0; i < iNumCategories; ++i)
       gvCaseData_Cat.push_back(new TwoDimensionArrayHandler<count_t>(giIntervalsDimensions, giLocationDimensions + giMetaLocations, 0));
  }
  catch (prg_exception& x) {
    x.addTrace("allocateCaseData_Cat()","DataSet");
    throw;
  }
  return gvCaseData_Cat;
}

/** Allocates two dimensional array which will represent not cumulative case data, time by space.
    Initializes all elements of array to zero. */
TwoDimCountArray_t & DataSet::allocateCaseData_NC() {
  try {
    if (!gpCaseData_NC)
      gpCaseData_NC = new TwoDimensionArrayHandler<count_t>(giIntervalsDimensions, giLocationDimensions + giMetaLocations);
    gpCaseData_NC->Set(0);
  }
  catch(prg_exception& x) {
    x.addTrace("allocateCaseData_NC()","DataSet");
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
  catch (prg_exception& x) {
    x.addTrace("allocateCaseData_PT()","DataSet");
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
  catch (prg_exception& x) {
    x.addTrace("allocateCaseData_PT_Cat()","DataSet");
    throw;
  }
  return *gpCaseData_PT_Cat;
}

/** Allocates one dimensional array which will represent not cumulative case data,
    time only. Initializes all elements of array to zero. */
count_t * DataSet::allocateCaseData_PT_NC() {
  try {
    if (!gpCaseData_PT_NC)
      //allocate to # time intervals plus one -- a pointer to this array will be
      //passed directly CTimeIntervals object, where it is assumed element at index
      //'giIntervalsDimensions' is accessible and set to zero
      gpCaseData_PT_NC = new count_t[giIntervalsDimensions];
    memset(gpCaseData_PT_NC, 0, giIntervalsDimensions * sizeof(count_t));
  }
  catch (prg_exception& x) {
    x.addTrace("allocateCaseData_PT_NC()","RealDataSet");
    throw;
  }
  return gpCaseData_PT_NC;
}

/** Allocates two dimensional array which will represent cumulative measure data,
    time by space. Initializes all elements of array to zero. */
TwoDimMeasureArray_t & DataSet::allocateMeasureData() {
  try {
    if (!gpMeasureData)
      gpMeasureData = new TwoDimensionArrayHandler<measure_t>(giIntervalsDimensions, giLocationDimensions + giMetaLocations);
    gpMeasureData->Set(0);
  }
  catch (prg_exception& x) {
    x.addTrace("allocateMeasureData()","DataSet");
    throw;
  }
  return *gpMeasureData;
}

/** Allocates two dimensional array which will represent not cumulative measure
    data, time by space. Initializes all elements of array to zero. */
TwoDimMeasureArray_t & DataSet::allocateMeasureData_NC() {
  try {
    if (!gpMeasureData_NC)
      gpMeasureData_NC = new TwoDimensionArrayHandler<measure_t>(giIntervalsDimensions, giLocationDimensions + giMetaLocations);
    gpMeasureData_NC->Set(0);
  }
  catch(prg_exception& x) {
    x.addTrace("allocateMeasureData_NC()","DataSet");
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
  catch (prg_exception& x) {
    x.addTrace("allocateMeasureData_PT()","DataSet");
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
  catch (prg_exception& x) {
    x.addTrace("allocateMeasureData_PT_NC()","DataSet");
    throw;
  }
  return gpMeasureData_PT_NC;
}

/** Allocates one dimensional array which will represent not cumulative auxillary measure
    data, time only. Initializes all elements of array to zero. */
measure_t * DataSet::allocateMeasureData_PT_Aux() {
  try {
    if (!gpMeasureData_PT_Aux)
      //allocate to # time intervals plus one -- a pointer to this array will be
      //passed directly CTimeIntervals object, where it is assumed element at index
      //'giIntervalsDimensions' is accessible and set to zero
      gpMeasureData_PT_Aux = new measure_t[giIntervalsDimensions+1];
    memset(gpMeasureData_PT_Aux, 0, (giIntervalsDimensions+1) * sizeof(measure_t));
  }
  catch (prg_exception& x) {
    x.addTrace("allocateMeasureData_PT_Aux()","DataSet");
    throw;
  }
  return gpMeasureData_PT_Aux;
}

/** Allocates one dimensional array which will represent not cumulative auxillary measure
    data, time only. Initializes all elements of array to zero. */
BatchIndexes_t * DataSet::allocatePositiveBatchData_PT(unsigned int setSize) {
    try {
        if (!gpPositiveBatchIndexes_PT)
            //allocate to # time intervals plus one -- a pointer to this array will be
            //passed directly CTimeIntervals object, where it is assumed element at index
            //'giIntervalsDimensions' is accessible and set to zero
            gpPositiveBatchIndexes_PT = new BatchIndexes_t[giIntervalsDimensions + 1];
        boost::dynamic_bitset<> theset(setSize);
        for (unsigned int i = 0; i < giIntervalsDimensions + 1; ++i)
            gpPositiveBatchIndexes_PT[i] = theset;
    } catch (prg_exception& x) {
        x.addTrace("allocatePositiveBatchData_PT()", "DataSet");
        throw;
    }
    return gpPositiveBatchIndexes_PT;
}

/** Allocates one dimensional array which will represent not cumulative auxillary measure
    data, time only. Initializes all elements of array to zero. */
measure_t* DataSet::allocateMeasureData_PT_Aux2() {
    try {
        if (!gpMeasureData_PT_Aux2)
            //allocate to # time intervals plus one -- a pointer to this array will be
            //passed directly CTimeIntervals object, where it is assumed element at index
            //'giIntervalsDimensions' is accessible and set to zero
            gpMeasureData_PT_Aux2 = new measure_t[giIntervalsDimensions + 1];
        memset(gpMeasureData_PT_Aux2, 0, (giIntervalsDimensions + 1) * sizeof(measure_t));
    } catch (prg_exception& x) {
        x.addTrace("allocateMeasureData_PT_Aux2()", "DataSet");
        throw;
    }
    return gpMeasureData_PT_Aux2;
}

/** Allocates two dimensional array which will represent cumulative auxillary measure data,
    time by space. Initializes all elements of array to zero. */
TwoDimMeasureArray_t & DataSet::allocateMeasureData_Aux() {
  try {
    if (!gpMeasureData_Aux)
      gpMeasureData_Aux = new TwoDimensionArrayHandler<measure_t>(giIntervalsDimensions, giLocationDimensions + giMetaLocations);
    gpMeasureData_Aux->Set(0);
  }
  catch (prg_exception& x) {
    x.addTrace("allocateMeasureData_Aux()","DataSet");
    throw;
  }
  return *gpMeasureData_Aux;
}

/** Allocates two dimensional array which will represent cumulative auxillary measure data,
    time by space. Initializes all elements of array to zero. */
TwoDimMeasureArray_t& DataSet::allocateMeasureData_Aux2() {
    try {
        if (!gpMeasureData_Aux2)
            gpMeasureData_Aux2 = new TwoDimensionArrayHandler<measure_t>(giIntervalsDimensions, giLocationDimensions + giMetaLocations);
        gpMeasureData_Aux2->Set(0);
    } catch (prg_exception& x) {
        x.addTrace("allocateMeasureData_Aux2()", "DataSet");
        throw;
    }
    return *gpMeasureData_Aux2;
}

TwoDimBitsetArray_t& DataSet::allocatePositiveBatchData(unsigned int setSize) {
    try {
        if (!gpPositiveBatchIndexes)
            gpPositiveBatchIndexes = new TwoDimensionArrayHandler<BatchIndexes_t>(giIntervalsDimensions, giLocationDimensions + giMetaLocations);
        boost::dynamic_bitset<> theset(setSize);
        gpPositiveBatchIndexes->Set(theset);
    } catch (prg_exception& x) {
        x.addTrace("allocatePositiveBatchData()", "DataSet");
        throw;
    }
    return *gpPositiveBatchIndexes;
}

/** Returns reference to object which manages a two dimensional array that represents
    cumulative case data, time by space. Throws prg_error if not allocated. */
TwoDimCountArray_t & DataSet::getCaseData() const {
  if (!gpCaseData) throw prg_error("gpCaseData not allocated.","getCaseData()");
  return *gpCaseData;
}

/** Returns reference to object which manages a two dimensional array that represents
    not cumulative case data, time by space. Throws prg_error if not allocated. */
TwoDimCountArray_t & DataSet::getCaseData_NC() const {
  if (!gpCaseData_NC) throw prg_error("gpCaseData_NC not allocated.","getCaseData_NC()");
  return *gpCaseData_NC;
}

/** Returns pointer to allocated array that represents cumulative case data, time
    only. Throws prg_error if not allocated. */
count_t * DataSet::getCaseData_PT() const {
  if (!gpCaseData_PT) throw prg_error("gpCaseData_PT not allocated.","getCaseData_PT()");
  return gpCaseData_PT;
}

/** Returns reference to object which manages a two dimensional array that represents
    cumulative case data, category by time. Throws prg_error if not allocated. */
TwoDimCountArray_t & DataSet::getCaseData_PT_Cat() const {
  if (!gpCaseData_PT_Cat) throw prg_error("gpCaseData_PT_Cat not allocated.","getCaseData_PT_Cat()");
  return *gpCaseData_PT_Cat;
}

/** Returns pointer to allocated array that represents not cumulative case data,
    time only. Throws prg_error if not allocated. */
count_t * DataSet::getCaseData_PT_NC() const {
  if (!gpCaseData_PT_NC) throw prg_error("gpCaseData_PT_NC not allocated.","getCaseData_PT_NC()");
  return gpCaseData_PT_NC;
}

/** Returns reference to object which manages a two dimensional array that represents
    cumulative measure data, time by space. Throws prg_error if not allocated. */
TwoDimMeasureArray_t & DataSet::getMeasureData() const {
  if (!gpMeasureData) throw prg_error("gpMeasureData not allocated.","getMeasureData()");
  return *gpMeasureData;
}

/** Returns reference to object which manages a two dimensional array that represents
    not cumulative measure data, time by space. Throws prg_error if not allocated. */
TwoDimMeasureArray_t & DataSet::getMeasureData_NC() const {
  if (!gpMeasureData_NC) throw prg_error("gpMeasureData_NC not allocated.","getMeasureData_NC()");
  return *gpMeasureData_NC;
}

/** Returns pointer to allocated array that represents cumulative measure data,
    time only. Throws prg_error if not allocated. */
measure_t * DataSet::getMeasureData_PT(bool check) const {
  if (!gpMeasureData_PT && check) throw prg_error("gpMeasureData_PT not allocated.","getMeasureData_PT()");
  return gpMeasureData_PT;
}

/** Returns pointer to allocated array that represents not cumulative measure data,
    time only. Throws prg_error if not allocated. */
measure_t * DataSet::getMeasureData_PT_NC(bool check) const {
  if (!gpMeasureData_PT_NC && check) throw prg_error("gpMeasureData_PT_NC not allocated.","getMeasureData_PT_NC()");
  return gpMeasureData_PT_NC;
}

/** Returns pointer to allocated array that represents cumulative auxillary measure data
    time only. Throws prg_error if not allocated. */
measure_t * DataSet::getMeasureData_PT_Aux() const {
  if (!gpMeasureData_PT_Aux) throw prg_error("gpMeasureData_PT_Aux not allocated.","getMeasureData_PT_Aux()");
  return gpMeasureData_PT_Aux;
}

/** Returns pointer to allocated array that represents cumulative auxillary measure data
    time only. Throws prg_error if not allocated. */
measure_t* DataSet::getMeasureData_PT_Aux2() const {
    if (!gpMeasureData_PT_Aux2) throw prg_error("gpMeasureData_PT_Aux2 not allocated.", "getMeasureData_PT_Aux2()");
    return gpMeasureData_PT_Aux2;
}

/** Returns pointer to allocated array that represents cumulative auxillary measure data
    time only. Throws prg_error if not allocated. */
BatchIndexes_t * DataSet::getPositiveBatchIndexes_PT() const {
    if (!gpPositiveBatchIndexes_PT) throw prg_error("gpPositiveBatchIndexes_PT not allocated.", "getPositiveBatchIndexes_PT()");
    return gpPositiveBatchIndexes_PT;
}

/** Returns reference to object which manages a two dimensional array that represents
    cumulative auxillary measure data, time by space. Throws prg_error if not allocated. */
TwoDimMeasureArray_t & DataSet::getMeasureData_Aux() const {
  if (!gpMeasureData_Aux) throw prg_error("gpMeasureData_Aux not allocated.","getMeasureData_Aux()");
  return *gpMeasureData_Aux;
}

TwoDimMeasureArray_t& DataSet::getMeasureData_Aux2() const {
    if (!gpMeasureData_Aux2) throw prg_error("gpMeasureData_Aux2 not allocated.", "getMeasureData_Aux2()");
    return *gpMeasureData_Aux2;
}

TwoDimBitsetArray_t& DataSet::getPositiveBatchData() const {
    if (!gpPositiveBatchIndexes) throw prg_error("gpPositiveBatchIndexes not allocated.", "getPositiveBatchData()");
    return *gpPositiveBatchIndexes;
}

/** For each allocated data structure which could contain meta data, reallocates and
    assigns meta data. */
void DataSet::reassignMetaData(const MetaManagerProxy& MetaProxy) {
  giMetaLocations = MetaProxy.getNumMeta();
  if (gpCaseData) {
    gpCaseData->ResizeSecondDimension(giLocationDimensions + giMetaLocations, 0);
    setCaseDataMeta(MetaProxy);
  }
  if (gpCaseData_NC) {
    gpCaseData_NC->ResizeSecondDimension(giLocationDimensions + giMetaLocations, 0);
    setCaseData_NC();
  }
  if (gvCaseData_Cat.size()) {
    for (unsigned int c=0; c < gvCaseData_Cat.size(); ++c)
       gvCaseData_Cat[c]->ResizeSecondDimension(giLocationDimensions + giMetaLocations, 0);
    setCaseDataCatMeta(MetaProxy);
  }
  if (gpMeasureData) {
    gpMeasureData->ResizeSecondDimension(giLocationDimensions + giMetaLocations, 0);
    setMeasureDataMeta(MetaProxy);
  }
  if (gpMeasureData_NC) {
    gpMeasureData_NC->ResizeSecondDimension(giLocationDimensions + giMetaLocations, 0);
    setMeasureData_NC();
  }
  if (gpMeasureData_Aux) {
    gpMeasureData_Aux->ResizeSecondDimension(giLocationDimensions + giMetaLocations, 0);
    setMeasureDataAuxMeta(MetaProxy);
  }
  if (gpMeasureData_Aux2) {
      gpMeasureData_Aux2->ResizeSecondDimension(giLocationDimensions + giMetaLocations, 0);
      // TODO - refactor setMeasureDataAuxMeta(...) to work on TwoDimMeasureArray_t?
      setMeasureDataAuxMeta(MetaProxy);
  }
}

/** Sets case data at meta location indexes. */
void DataSet::setCaseDataMeta(const MetaManagerProxy& MetaProxy) {
  std::vector<tract_t>  AtomicIndexes;
  count_t ** ppCases = getCaseData().GetArray();

  for (unsigned int m=0; m < giMetaLocations; ++m) {
     MetaProxy.getIndexes(m, AtomicIndexes);
     tract_t MetaIndex = m + giLocationDimensions;
     for (unsigned int i=0; i < giIntervalsDimensions; ++i)
        ppCases[i][MetaIndex] = 0;
     for (size_t t=0; t < AtomicIndexes.size(); ++t) {
        tract_t Atomic = AtomicIndexes[t];
        for (unsigned int i=0; i < giIntervalsDimensions; ++i)
           ppCases[i][MetaIndex] += ppCases[i][Atomic];
     }
  }
}

/** Sets case category data at meta location indexes. */
void DataSet::setCaseDataCatMeta(const MetaManagerProxy& MetaProxy) {
  std::vector<tract_t>  AtomicIndexes;

  for (unsigned int m=0; m < giMetaLocations; ++m) {
     MetaProxy.getIndexes(m, AtomicIndexes);
     tract_t MetaIndex = m + giLocationDimensions;
     for (unsigned int c=0; c < getCaseData_Cat().size(); ++c) {
       count_t ** ppCases = getCaseData_Cat()[c]->GetArray();
       for (unsigned int i=0; i < giIntervalsDimensions; ++i)
          ppCases[i][MetaIndex] = 0;
       for (size_t t=0; t < AtomicIndexes.size(); ++t) {
          tract_t Atomic = AtomicIndexes[t];
          for (unsigned int i=0; i < giIntervalsDimensions; ++i)
             ppCases[i][MetaIndex] += ppCases[i][Atomic];
       }
     }
  }
}

/** Sets measure data at meta location indexes. */
void DataSet::setMeasureDataMeta(const MetaManagerProxy& MetaLocations) {
  std::vector<tract_t>  AtomicIndexes;
  measure_t ** ppMeasure = getMeasureData().GetArray();

  for (unsigned int m=0; m < giMetaLocations; ++m) {
     MetaLocations.getIndexes(m, AtomicIndexes);
     tract_t MetaIndex = m + giLocationDimensions;
     for (unsigned int i=0; i < giIntervalsDimensions; ++i)
        ppMeasure[i][MetaIndex] = 0;
     for (size_t t=0; t < AtomicIndexes.size(); ++t) {
        tract_t Atomic = AtomicIndexes[t];
        for (unsigned int i=0; i < giIntervalsDimensions; ++i)
           ppMeasure[i][MetaIndex] += ppMeasure[i][Atomic];
     }
  }
}

/** Sets measure auxillary data at meta location indexes. */
void DataSet::setMeasureDataAuxMeta(const MetaManagerProxy& MetaLocations) {
  std::vector<tract_t>  AtomicIndexes;
  measure_t ** ppMeasure = getMeasureData_Aux().GetArray();

  for (unsigned int m=0; m < giMetaLocations; ++m) {
     MetaLocations.getIndexes(m, AtomicIndexes);
     tract_t MetaIndex = m + giLocationDimensions;
     for (unsigned int i=0; i < giIntervalsDimensions; ++i)
        ppMeasure[i][MetaIndex] = 0;
     for (size_t t=0; t < AtomicIndexes.size(); ++t) {
        tract_t Atomic = AtomicIndexes[t];
        for (unsigned int i=0; i < giIntervalsDimensions; ++i)
           ppMeasure[i][MetaIndex] += ppMeasure[i][Atomic];
     }
  }
}

/** Sets measure auxillary data at meta location indexes. */
void DataSet::setMeasureDataAux2Meta(const MetaManagerProxy& MetaLocations) {
    std::vector<tract_t>  AtomicIndexes;
    measure_t** ppMeasure = getMeasureData_Aux2().GetArray();

    for (unsigned int m = 0; m < giMetaLocations; ++m) {
        MetaLocations.getIndexes(m, AtomicIndexes);
        tract_t MetaIndex = m + giLocationDimensions;
        for (unsigned int i = 0; i < giIntervalsDimensions; ++i)
            ppMeasure[i][MetaIndex] = 0;
        for (size_t t = 0; t < AtomicIndexes.size(); ++t) {
            tract_t Atomic = AtomicIndexes[t];
            for (unsigned int i = 0; i < giIntervalsDimensions; ++i)
                ppMeasure[i][MetaIndex] += ppMeasure[i][Atomic];
        }
    }
}

/** Sets batch bit set data at meta location indexes. */
void DataSet::setPositiveBitsetDataMeta(const MetaManagerProxy& MetaLocations) {
    std::vector<tract_t>  AtomicIndexes;
    BatchIndexes_t ** ppBitset = getPositiveBatchData().GetArray();

    for (unsigned int m = 0; m < giMetaLocations; ++m) {
        MetaLocations.getIndexes(m, AtomicIndexes);
        tract_t MetaIndex = m + giLocationDimensions;
        for (unsigned int i = 0; i < giIntervalsDimensions; ++i)
            ppBitset[i][MetaIndex].reset();
        for (size_t t = 0; t < AtomicIndexes.size(); ++t) {
            tract_t Atomic = AtomicIndexes[t];
            for (unsigned int i = 0; i < giIntervalsDimensions; ++i)
                ppBitset[i][MetaIndex] |= ppBitset[i][Atomic];
        }
    }
}

/** Allocates and sets not cumulative case data (time by space) from cumulative
    case data (time by space). */
void DataSet::setCaseData_NC() {
  try {
    count_t ** ppCases = getCaseData().GetArray(), ** ppCases_NC = allocateCaseData_NC().GetArray();
    for (unsigned int t=0; t < giLocationDimensions + giMetaLocations; ++t)  {
      ppCases_NC[giIntervalsDimensions-1][t] = ppCases[giIntervalsDimensions-1][t];
      for (unsigned int i=0; i < giIntervalsDimensions - 1; ++i)
        ppCases_NC[i][t] = ppCases[i][t] - ppCases[i+1][t];
    }
  }
  catch (prg_exception& x) {
    x.addTrace("setCaseData_NC()","DataSet");
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
  catch (prg_exception& x) {
    x.addTrace("setCaseData_PT()","DataSet");
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
  catch (prg_exception& x) {
    x.addTrace("setCaseData_PT_Cat()","DataSet");
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
  catch (prg_exception& x) {
    x.addTrace("setCaseData_PT_NC()","DataSet");
    throw;
  }
}

/** Allocates and sets not cumulative measure data (time by space) from cumulative
    measure data (time by space). */
void DataSet::setMeasureData_NC() {
  try {
    measure_t ** ppMeasure = getMeasureData().GetArray();
    measure_t ** ppMeasureNC = allocateMeasureData_NC().GetArray();
    for (unsigned int t=0; t < giLocationDimensions + giMetaLocations; ++t) {
      ppMeasureNC[giIntervalsDimensions-1][t] = ppMeasure[giIntervalsDimensions-1][t];
      for (unsigned int i=0; i < giIntervalsDimensions - 1; ++i)
        ppMeasureNC[i][t] = ppMeasure[i][t] - ppMeasure[i+1][t];
    }
  }
  catch (prg_exception& x) {
    x.addTrace("setMeasureData_NC()","DataSet");
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
  catch (prg_exception& x) {
    x.addTrace("setMeasureData_PT()","DataSet");
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
  catch (prg_exception& x) {
    x.addTrace("setMeasureData_PT_NC()","DataSet");
    throw;
  }
}

/** Allocates and sets cumulative auxillary measure data (time only) from cumulative
    auxillry measure data (time by space). */
void DataSet::setMeasureData_PT_Aux() {
  try {
    allocateMeasureData_PT_Aux();
    measure_t ** ppMeasure = getMeasureData_Aux().GetArray();
    for (unsigned int i=0; i < giIntervalsDimensions; ++i)
       for (unsigned int t=0; t < giLocationDimensions; ++t)
          gpMeasureData_PT_Aux[i] += ppMeasure[i][t];
  }
  catch (prg_exception& x) {
    x.addTrace("setMeasureData_PT_Aux()","DataSet");
    throw;
  }
}

/** Allocates and sets cumulative auxillary measure data (time only) from cumulative
    auxillry measure data (time by space). */
void DataSet::setMeasureData_PT_Aux2() {
    try {
        allocateMeasureData_PT_Aux2();
        measure_t** ppMeasure = getMeasureData_Aux2().GetArray();
        for (unsigned int i = 0; i < giIntervalsDimensions; ++i)
            for (unsigned int t = 0; t < giLocationDimensions; ++t)
                gpMeasureData_PT_Aux2[i] += ppMeasure[i][t];
    }
    catch (prg_exception& x) {
        x.addTrace("setMeasureData_PT_Aux2()", "DataSet");
        throw;
    }
}

/** Allocates and sets cumulative auxillary measure data (time only) from cumulative
    auxillry measure data (time by space). */
void DataSet::setPositiveBatchIndexes_PT(unsigned int setSize) {
    try {
        allocatePositiveBatchData_PT(setSize);
        BatchIndexes_t ** ppBitset = getPositiveBatchData().GetArray();
        for (unsigned int i = 0; i < giIntervalsDimensions; ++i) {
            for (unsigned int t = 0; t < giLocationDimensions; ++t)
                gpPositiveBatchIndexes_PT[i] |= ppBitset[i][t];
        }
    } catch (prg_exception& x) {
        x.addTrace("setPositiveBatchIndexes_PT()", "DataSet");
        throw;
    }
}

/** Sets cumulative case data from self, assuming self is currently not cumulative.
    Repeated calls to this method or calling when data is not in a not cumulative state
    will produce erroneous data. */
void DataSet::setCaseDataToCumulative() {
  if (giIntervalsDimensions < 2) return;
  count_t ** ppCases = getCaseData().GetArray();
  for (unsigned int t=0; t < giLocationDimensions + giMetaLocations; ++t) {
     for (unsigned int i=giIntervalsDimensions-2; ; --i) {
        ppCases[i][t]= ppCases[i][t] + ppCases[i+1][t];
        if (i == 0) break;
     }
  }
}

/** Sets cumulative measure data from self, assuming self is currently not cumulative.
    Repeated calls to this method or calling when data is not in a not cumulative state
    will produce erroneous data. */
void DataSet::setMeasureDataToCumulative() {
  if (giIntervalsDimensions < 2) return;
  measure_t ** ppMeasure = getMeasureData().GetArray();
  for (unsigned int t=0; t < giLocationDimensions + giMetaLocations; ++t) {
     for (unsigned int i=giIntervalsDimensions-2; ; --i) {
        ppMeasure[i][t]= ppMeasure[i][t] + ppMeasure[i+1][t];
        if (i == 0) break;
     }
  }
}

void DataSet::setMeasureData_Aux(TwoDimMeasureArray_t& other) {
    if (!gpMeasureData_Aux)
        gpMeasureData_Aux = new TwoDimensionArrayHandler<measure_t>(other);
    else
        *gpMeasureData_Aux = other;
}

void DataSet::setMeasureData_Aux2(TwoDimMeasureArray_t& other) {
    if (!gpMeasureData_Aux2)
        gpMeasureData_Aux2 = new TwoDimensionArrayHandler<measure_t>(other);
    else
        *gpMeasureData_Aux2 = other;
}

////////////////////////////////////////////////////////////////////////////////
//////////////////////////// RealDataSet ///////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

/** constructor */
RealDataSet::RealDataSet(unsigned int iNumTimeIntervals, unsigned int iNumTracts, unsigned int iMetaLocations, const CParameters& parameters, unsigned int iSetIndex)
            :DataSet(iNumTimeIntervals, iNumTracts, iMetaLocations, parameters, iSetIndex),
             gtTotalCases(0), gtTotalCasesAtStart(0), gtTotalControls(0), gdTotalPop(0), gpControlData(0), gtTotalMeasure(0),
             gtTotalMeasureAtStart(0), gdCalculatedTimeTrendPercentage(0), gpCaseData_Censored(0), gpBatchIndexes(0), gpBatchIndexes_PT(0) {
    _population.reset(new PopulationData());
    _population->SetNumTracts(giLocationDimensions);
    boost::gregorian::greg_weekday week_day = boost::date_time::Sunday;
    for (; ; ) {
        _totalCasesByWeekDay[week_day] = 0;
        _totalControlsByWeekDay[week_day] = 0;
        if (week_day == boost::date_time::Saturday) break;
        week_day = boost::gregorian::greg_weekday(week_day.as_number() + 1);
    }
}

/** copy constructor */
RealDataSet::RealDataSet(const RealDataSet& thisSet) : DataSet(thisSet) {}

/** destructor */
RealDataSet::~RealDataSet() {
    try {
        delete gpControlData;
        delete gpCaseData_Censored;
        delete gpBatchIndexes;
        delete[] gpBatchIndexes_PT;
    } catch (...){}
}

/** Resets population data structure, clearing all data and settings. */
void RealDataSet::resetPopulationData() {
    _population.reset(new PopulationData());
    _population->SetNumTracts(giLocationDimensions);
}

TwoDimBitsetArray_t& RealDataSet::allocateBatchData(unsigned int setSize) {
    try {
        if (!gpBatchIndexes)
            gpBatchIndexes = new TwoDimensionArrayHandler<BatchIndexes_t>(giIntervalsDimensions, giLocationDimensions + giMetaLocations);
        boost::dynamic_bitset<> theset(setSize);
        gpBatchIndexes->Set(theset);
    } catch (prg_exception& x) {
        x.addTrace("allocateBatchData()", "DataSet");
        throw;
    }
    return *gpBatchIndexes;
}

/** Allocates one dimensional array which will represent not cumulative auxillary measure
    data, time only. Initializes all elements of array to zero. */
BatchIndexes_t* RealDataSet::allocateBatchData_PT(unsigned int setSize) {
    try {
        if (!gpBatchIndexes_PT)
            //allocate to # time intervals plus one -- a pointer to this array will be
            //passed directly CTimeIntervals object, where it is assumed element at index
            //'giIntervalsDimensions' is accessible and set to zero
            gpBatchIndexes_PT = new BatchIndexes_t[giIntervalsDimensions + 1];
        boost::dynamic_bitset<> theset(setSize);
        for (unsigned int i = 0; i < giIntervalsDimensions + 1; ++i)
            gpBatchIndexes_PT[i] = theset;
    } catch (prg_exception& x) {
        x.addTrace("allocateBatchData_PT()", "DataSet");
        throw;
    }
    return gpBatchIndexes_PT;
}

/** Allocates and sets cumulative array bitsets by time. */
void RealDataSet::setBatchData_PT(unsigned int setSize) {
    try {
        allocateBatchData_PT(setSize);
        BatchIndexes_t** ppBitset = getBatchData().GetArray();
        for (unsigned int i = 0; i < giIntervalsDimensions; ++i) {
            for (unsigned int t = 0; t < giLocationDimensions; ++t)
                gpBatchIndexes_PT[i] |= ppBitset[i][t];
        }
    } catch (prg_exception& x) {
        x.addTrace("setBatchData_PT()", "DataSet");
        throw;
    }
}

/** Allocates two dimensional array which will represent cumulative censored case
    data (time by space). Initializes all elements of array to zero. */
TwoDimCountArray_t & RealDataSet::allocateCaseData_Censored() {
  try {
    if (!gpCaseData_Censored)
      gpCaseData_Censored = new TwoDimensionArrayHandler<count_t>(giIntervalsDimensions, giLocationDimensions + giMetaLocations);
    gpCaseData_Censored->Set(0);
  }
  catch (prg_exception& x) {
    x.addTrace("allocateCaseData_Censored()","RealDataSet");
    throw;
  }
  return *gpCaseData_Censored;
}

/** Allocates two dimensional array which will represent cumulative control
    data (time by space). Initializes all elements of array to zero. */
TwoDimCountArray_t & RealDataSet::allocateControlData() {
  try {
    if (!gpControlData)
      gpControlData = new TwoDimensionArrayHandler<count_t>(giIntervalsDimensions, giLocationDimensions + giMetaLocations);
    gpControlData->Set(0);
  }
  catch (prg_exception& x) {
    x.addTrace("allocateControlData()","RealDataSet");
    throw;
  }
  return *gpControlData;
}

/** Adds case count to ordinal category, updating PoplationData object. If a two
    dimensional array does not exist for category associated with 'dOrdinalNumber',
    one is allocated. Returns referance to two dimensional array associated with
    'dOrdinalNumber'. */
TwoDimCountArray_t & RealDataSet::addCategoryTypeCaseCount(const std::string& categoryTypeLabel, count_t Count, Julian date, bool asOrdinal) {
  size_t        tCategoryIndex;

  tCategoryIndex = _population->addCategoryTypeCaseCount(categoryTypeLabel, Count, asOrdinal);
  if (_population->GetNumOrdinalCategories() > gvCaseData_Cat.size())
    gvCaseData_Cat.insert(gvCaseData_Cat.begin() + tCategoryIndex, new TwoDimensionArrayHandler<count_t>(giIntervalsDimensions, giLocationDimensions + giMetaLocations, 0));

  // update counts by week day by category
  _totalCategoryCasesByWeekDay[getWeekDay(date)][tCategoryIndex] += Count;

  return *gvCaseData_Cat[tCategoryIndex];
}

/** Validates that the population data is correct in that a location
    does not contain case data while having zero population. */
void RealDataSet::checkPopulationDataCases(CSaTScanData& Data) {
  try {
    _population->CheckCasesHavePopulations(gpCaseData->GetArray()[0], Data);
  }
  catch(prg_exception& x) {
    x.addTrace("checkPopulationDataCases()","RealDataSet");
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
    array for category and returns referance. Otherwise throws prg_error.*/
TwoDimCountArray_t & RealDataSet::getCategoryCaseData(unsigned int iCategoryIndex, bool bCreateable) {
  if (!gvCaseData_Cat.size() || iCategoryIndex + 1 > gvCaseData_Cat.size()) {
    if (!bCreateable)
      throw prg_error("Index %u out of range [size=%u].","GetCategoryCaseArray()", iCategoryIndex, gvCaseData_Cat.size());
    size_t tNumAllocate = iCategoryIndex + 1 - gvCaseData_Cat.size();
    for (size_t t=0; t < tNumAllocate; ++t)
      gvCaseData_Cat.push_back(new TwoDimensionArrayHandler<count_t>(giIntervalsDimensions, giLocationDimensions + giMetaLocations, 0));
  }

  return *gvCaseData_Cat.at(iCategoryIndex);
}

TwoDimBitsetArray_t& RealDataSet::getBatchData() const {
    if (!gpBatchIndexes) throw prg_error("gpBatchIndexes not allocated.", "getBatchData()");
    return *gpBatchIndexes;
}

/** Returns pointer to allocated array that represents cumulative auxillary measure data
    time only. Throws prg_error if not allocated. */
BatchIndexes_t* RealDataSet::getBatchIndexes_PT() const {
    if (!gpBatchIndexes_PT) throw prg_error("gpBatchIndexes_PT not allocated.", "getBatchIndexes_PT()");
    return gpBatchIndexes_PT;
}

/** Returns reference to object which manages a two dimensional array that represents
    cumulative censored case data, time by space. */
TwoDimCountArray_t & RealDataSet::getCaseData_Censored() const {
  if (!gpCaseData_Censored) throw prg_error("gpCaseData_Censored not allocated.","getCaseData_Censored()");
  return *gpCaseData_Censored;
}

/** Returns reference to object which manages a two dimensional array that represents
    cumulative control data, time by space. */
TwoDimCountArray_t & RealDataSet::getControlData() const {
  if (!gpControlData) throw prg_error("gpControlData not allocated.","getControlData()");
  return *gpControlData;
}

/** For each allocated data structure which could contain meta data, reallocates and
    assigns meta data. */
void RealDataSet::reassignMetaData(const MetaManagerProxy& MetaProxy) {
  DataSet::reassignMetaData(MetaProxy);
  if (gpCaseData_Censored) {
    gpCaseData_Censored->ResizeSecondDimension(giLocationDimensions + giMetaLocations, 0);
    setCaseData_Censored_MetaLocations(MetaProxy);
  }
  if (gpControlData) {
    gpControlData->ResizeSecondDimension(giLocationDimensions + giMetaLocations, 0);
    setControlData_MetaLocations(MetaProxy);
  }
}

/** Sets batch bit set data at meta location indexes. */
void RealDataSet::setBitsetDataMeta(const MetaManagerProxy& MetaLocations) {
    std::vector<tract_t>  AtomicIndexes;
    BatchIndexes_t** ppBitset = getBatchData().GetArray();

    for (unsigned int m = 0; m < giMetaLocations; ++m) {
        MetaLocations.getIndexes(m, AtomicIndexes);
        tract_t MetaIndex = m + giLocationDimensions;
        for (unsigned int i = 0; i < giIntervalsDimensions; ++i)
            ppBitset[i][MetaIndex].reset();
        for (size_t t = 0; t < AtomicIndexes.size(); ++t) {
            tract_t Atomic = AtomicIndexes[t];
            for (unsigned int i = 0; i < giIntervalsDimensions; ++i)
                ppBitset[i][MetaIndex] |= ppBitset[i][Atomic];
        }
    }
}

/* Sets the quadratic trend used for data set during measure adjustment. */
void RealDataSet::setCalculatedQuadraticTimeTrend(std::string& functionStr, std::string& definitionStr) { 
    std::stringstream s;
    s << functionStr.c_str() << " " << definitionStr.c_str();
    _calculatedQuadraticTrend = s.str(); 
}

/** Sets case data at meta location indexes. */
void RealDataSet::setCaseData_Censored_MetaLocations(const MetaManagerProxy& MetaProxy) {
  std::vector<tract_t>  AtomicIndexes;
  count_t ** ppCases = getCaseData_Censored().GetArray();

  for (unsigned int m=0; m < giMetaLocations; ++m) {
     MetaProxy.getIndexes(m, AtomicIndexes);
     tract_t MetaIndex = m + giLocationDimensions;
     for (unsigned int i=0; i < giIntervalsDimensions; ++i)
        ppCases[i][MetaIndex] = 0;
     for (size_t t=0; t < AtomicIndexes.size(); ++t) {
        tract_t Atomic = AtomicIndexes[t];
        for (unsigned int i=0; i < giIntervalsDimensions; ++i)
           ppCases[i][MetaIndex] += ppCases[i][Atomic];
     }
  }
}

/** Sets control data at meta location indexes. */
void RealDataSet::setControlData_MetaLocations(const MetaManagerProxy& MetaProxy) {
  std::vector<tract_t>  AtomicIndexes;
  count_t ** ppControls = getControlData().GetArray();

  for (unsigned int m=0; m < giMetaLocations; ++m) {
     MetaProxy.getIndexes(m, AtomicIndexes);
     tract_t MetaIndex = m + giLocationDimensions;
     for (unsigned int i=0; i < giIntervalsDimensions; ++i)
        ppControls[i][MetaIndex] = 0;
     for (size_t t=0; t < AtomicIndexes.size(); ++t) {
        tract_t Atomic = AtomicIndexes[t];
        for (unsigned int i=0; i < giIntervalsDimensions; ++i)
           ppControls[i][MetaIndex] += ppControls[i][Atomic];
     }
  }
}

void RealDataSet::setPopulationMeasureData(TwoDimMeasureArray_t& otherMeasure, boost::shared_ptr<PopulationData> * otherPopulation) {
    if (!_populationData.first.get())
        _populationData.first.reset(new TwoDimensionArrayHandler<measure_t>(otherMeasure));
    else
        *(_populationData.first.get()) = otherMeasure;
    if (otherPopulation)  {
        _populationData.second = *otherPopulation;
    }
}

RealDataSet::PopulationDataPair_t RealDataSet::getPopulationMeasureData() const {
  PopulationDataPair_t populationPair;
  if (!_populationData.first.get()) throw prg_error("gpPopulationMeasureData not allocated.","getPopulationMeasureData()");
  populationPair.first = _populationData.first;
  if (!_populationData.second.get()) {
      // If PopulationData in pair is not defined, try returning '_population' -- the primary PopulationData object.
      if (!_population.get()) throw prg_error("gpPopulationMeasureData not allocated.","getPopulationMeasureData()");
      populationPair.second = _population;
  } else {
      populationPair.second = _populationData.second;
  }
  return populationPair;
}

void RealDataSet::reassign(TwoDimCountArray_t& cases, TwoDimMeasureArray_t& measure) {
    if (giMetaLocations != 0)
        throw prg_error("reassign() cannot be called with meta locations.","reassign()");
    if (cases.Get1stDimension() != measure.Get1stDimension())
        throw prg_error("1st dimension does not match.","reassign()");
    if (cases.Get2ndDimension() != measure.Get2ndDimension())
        throw prg_error("2nd dimension does not match.","reassign()");
    // TODO: check other class arrays?
    giIntervalsDimensions = cases.Get1stDimension();
    giLocationDimensions = cases.Get2ndDimension();

    delete gpCaseData; gpCaseData = 0;
    gpCaseData = new TwoDimensionArrayHandler<count_t>(cases);
    delete gpMeasureData; gpMeasureData = 0;
    gpMeasureData = new TwoDimensionArrayHandler<measure_t>(measure);
}
