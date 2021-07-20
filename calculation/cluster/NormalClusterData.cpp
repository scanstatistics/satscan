//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "SaTScanData.h"
#include "NormalClusterData.h"
#include "SSException.h"
#include "newmat.h"
#include "WeightedNormalRandomizer.h"

//************** class NormalSpatialData ***************************************

/** class constructor */
NormalSpatialData::NormalSpatialData() : SpatialData(), gtMeasureAux(0) {}

/** class constructor */
NormalSpatialData::NormalSpatialData(const DataSetInterface& Interface)
                  :SpatialData(Interface), gtMeasureAux(0) {}

/** class constructor */
NormalSpatialData::NormalSpatialData(const AbstractDataSetGateway& DataGateway)
                  :SpatialData(DataGateway), gtMeasureAux(0) {}

/** Adds neighbor data to accumulation  - caller is responsible for ensuring that
    'tNeighborIndex' and 'tSetIndex' are valid indexes. */
void NormalSpatialData::AddNeighborData(tract_t tNeighborIndex, const AbstractDataSetGateway& DataGateway, size_t tSetIndex) {
  gtCases += DataGateway.GetDataSetInterface(tSetIndex).GetPSCaseArray()[tNeighborIndex];
  gtMeasure += DataGateway.GetDataSetInterface(tSetIndex).GetPSMeasureArray()[tNeighborIndex];
  gtMeasureAux += DataGateway.GetDataSetInterface(tSetIndex).GetPSMeasureAuxArray()[tNeighborIndex];
}

/** Not implemeneted - throws prg_error. */
void NormalSpatialData::AddMeasureList(const DataSetInterface&, CMeasureList*, const CSaTScanData*) {
   throw prg_error("AddMeasureList(const DataSetInterface&, CMeasureList*, const CSaTScanData*) not implemented.","NormalSpatialData");
}

/** Assigns cluster data of passed object to 'this' object. Caller of function
    is responsible for ensuring that passed AbstractSpatialClusterData object
    can be casted to 'NormalSpatialData' object. */
void NormalSpatialData::Assign(const AbstractSpatialClusterData& rhs) {
  *this = (const NormalSpatialData&)rhs;
}

/** Calculates loglikelihood ratio, given current accumulated cluster data, if
    it is determined that data fits scanning area of interest (high, low, both).
    Returns zero if rate not of interest else returns loglikelihood ratio as
    calculated by probability model. */
double NormalSpatialData::CalculateLoglikelihoodRatio(AbstractLikelihoodCalculator& Calculator) {
  if ((Calculator.*Calculator.gpRateOfInterestNormal)(gtCases, gtMeasure, gtMeasureAux))
    return Calculator.CalcLogLikelihoodRatioNormal(gtCases, gtMeasure, gtMeasureAux);
  return 0;
}

/** Returns newly cloned NormalSpatialData object. Caller is responsible for deletion of object. */
NormalSpatialData * NormalSpatialData::Clone() const {
  return new NormalSpatialData(*this);
}

/** Copies class data members that reflect the number of cases, expected, and expected
    squared values, which is the data we are interested in for possiblely reporting. */
void NormalSpatialData::CopyEssentialClassMembers(const AbstractClusterData& rhs) {
  gtCases = ((const NormalSpatialData&)rhs).gtCases;
  gtMeasure = ((const NormalSpatialData&)rhs).gtMeasure;
  gtMeasureAux = ((const NormalSpatialData&)rhs).gtMeasureAux;
}

/** Calculates and returns maximizing value given accumulated cluster data. If data
    is not significant given scanning rate, negation of maximum double returned. */
double NormalSpatialData::GetMaximizingValue(AbstractLikelihoodCalculator& Calculator) {
  if ((Calculator.*Calculator.gpRateOfInterestNormal)(gtCases, gtMeasure, gtMeasureAux))
    return Calculator.CalculateMaximizingValueNormal(gtCases, gtMeasure, gtMeasureAux);
  return -std::numeric_limits<double>::max();
}

//************** class NormalTemporalData **************************************

/** class constructor */
NormalTemporalData::NormalTemporalData() : TemporalData(), gtMeasureAux(0), gpMeasureAux(0) {}

/** class constructor */
NormalTemporalData::NormalTemporalData(const DataSetInterface& Interface)
                   :TemporalData(Interface), gtMeasureAux(0),
                    gpMeasureAux(Interface.GetPTMeasureAuxArray()) {}

/** class constructor */
NormalTemporalData::NormalTemporalData(const AbstractDataSetGateway& DataGateway)
                   :TemporalData(DataGateway.GetDataSetInterface()), gtMeasureAux(0),
                    gpMeasureAux(DataGateway.GetDataSetInterface().GetPTMeasureAuxArray()) {}

/** Assigns cluster data of passed object to 'this' object. Caller of function
    is responsible for ensuring that passed AbstractTemporalClusterData object
    can be casted to 'NormalTemporalData' object. */
void NormalTemporalData::Assign(const AbstractTemporalClusterData& rhs) {
  *this = (const NormalTemporalData&)rhs;
}

/** Returns newly cloned NormalTemporalData object. Caller responsible for deletion of object. */
NormalTemporalData * NormalTemporalData::Clone() const {
  return new NormalTemporalData(*this);
}

/** Copies class data members that reflect the number of cases, expected, and expected
    squared values, which is the data we are interested in for possiblely reporting. */
void NormalTemporalData::CopyEssentialClassMembers(const AbstractClusterData& rhs) {
  gtCases = ((const NormalTemporalData&)rhs).gtCases;
  gtMeasure = ((const NormalTemporalData&)rhs).gtMeasure;
  gtMeasureAux = ((const NormalTemporalData&)rhs).gtMeasureAux;
}

/** Reassociates internal data with passed DataSetInterface pointers.
    Not implemented - throws exception */
void NormalTemporalData::Reassociate(const DataSetInterface& Interface) {
  throw prg_error("Reassociate(const DataSetInterface&) not implemented.","NormalTemporalData");
}

/** Reassociates internal data with passed DataSetInterface pointers of DataGateway. */
void NormalTemporalData::Reassociate(const AbstractDataSetGateway& DataGateway) {
  TemporalData::Reassociate(DataGateway.GetDataSetInterface());
  gpMeasureAux = DataGateway.GetDataSetInterface().GetPTMeasureAuxArray();
}
//**************** class NormalProspectiveSpatialData **************************

/** class constructor */
NormalProspectiveSpatialData::NormalProspectiveSpatialData(const CSaTScanData& Data, const DataSetInterface& Interface)
                             :NormalTemporalData(), geEvaluationAssistDataStatus(Allocated) {
  try {
    Init();
    Setup(Data, Interface);
  }
  catch (prg_exception& x) {
    x.addTrace("constructor()","NormalProspectiveSpatialData");
    throw;
  }
}

/** class constructor */
NormalProspectiveSpatialData::NormalProspectiveSpatialData(const CSaTScanData& Data, const AbstractDataSetGateway& DataGateway)
                             :NormalTemporalData(), geEvaluationAssistDataStatus(Allocated) {
  try {
    Init();
    Setup(Data, DataGateway.GetDataSetInterface());
  }
  catch (prg_exception& x) {
    x.addTrace("constructor()","NormalProspectiveSpatialData");
    throw;
  }
}

/** class copy constructor */
NormalProspectiveSpatialData::NormalProspectiveSpatialData(const NormalProspectiveSpatialData& rhs)
                             :NormalTemporalData() {
  try {
    Init();
    *this = rhs;
  }
  catch (prg_exception& x) {
    x.addTrace("copy constructor()","NormalProspectiveSpatialData");
    throw;
  }
}

/** class destructor */
NormalProspectiveSpatialData::~NormalProspectiveSpatialData() {
  try {
    delete[] gpCases;
    delete[] gpMeasure;
    delete[] gpMeasureAux;
  }
  catch (...){}
}

/** Adds neighbor data to accumulation - caller is responsible for ensuring that
    'tNeighborIndex' and 'tSetIndex' are valid indexes. */
void NormalProspectiveSpatialData::AddNeighborData(tract_t tNeighborIndex, const AbstractDataSetGateway& DataGateway, size_t tSetIndex) {
  assert(geEvaluationAssistDataStatus == Allocated);
  unsigned int           i, j;
  count_t             ** ppCases = DataGateway.GetDataSetInterface(tSetIndex).GetCaseArray();
  measure_t           ** ppMeasure = DataGateway.GetDataSetInterface(tSetIndex).GetMeasureArray();
  measure_t           ** ppMeasureAux = DataGateway.GetDataSetInterface(tSetIndex).GetMeasureAuxArray();

  //set cases for entire period added by this neighbor
  gpCases[0]   += ppCases[0][tNeighborIndex];
  gpMeasure[0] += ppMeasure[0][tNeighborIndex];
  gpMeasureAux[0] += ppMeasureAux[0][tNeighborIndex];

  for (j=1, i=giProspectiveStart; i < giNumTimeIntervals; ++j, ++i) {
     gpCases[j] += ppCases[i][tNeighborIndex];
     gpMeasure[j] += ppMeasure[i][tNeighborIndex];
     gpMeasureAux[j] += ppMeasureAux[i][tNeighborIndex];
  }
}

/** Assigns cluster data of passed object to 'this' object. Caller of function
    is responsible for ensuring that passed AbstractTemporalClusterData object
    can be casted to 'NormalProspectiveSpatialData' object. */
void NormalProspectiveSpatialData::Assign(const AbstractTemporalClusterData& rhs) {
  *this = (const NormalProspectiveSpatialData&)rhs;
}

/** Calculates loglikelihood ratio, given current accumulated cluster data, if
    it is determined that data fits scanning area of interest (high, low, both).
    Returns zero if all windows rates not of interest else returns greatest
    loglikelihood ratio as calculated by probability model. */
double NormalProspectiveSpatialData::CalculateLoglikelihoodRatio(AbstractLikelihoodCalculator& Calculator) {
  assert(geEvaluationAssistDataStatus == Allocated);
  unsigned int  iWindowEnd;
  double        dMaxLoglikelihoodRatio=0;

  gtCases = gpCases[0];
  gtMeasure = gpMeasure[0];
  gtMeasureAux = gpMeasureAux[0];
  if ((Calculator.*Calculator.gpRateOfInterestNormal)(gtCases, gtMeasure, gtMeasureAux))
    dMaxLoglikelihoodRatio = Calculator.CalcLogLikelihoodRatioNormal(gtCases, gtMeasure, gtMeasureAux);

  for (iWindowEnd=1; iWindowEnd < giAllocationSize - 1; ++iWindowEnd) {
    gtCases = gpCases[0] - gpCases[iWindowEnd];
    gtMeasure = gpMeasure[0] - gpMeasure[iWindowEnd];
    gtMeasureAux = gpMeasureAux[0] - gpMeasureAux[iWindowEnd];
    if ((Calculator.*Calculator.gpRateOfInterestNormal)(gtCases, gtMeasure, gtMeasureAux))
      dMaxLoglikelihoodRatio = std::max(dMaxLoglikelihoodRatio, Calculator.CalcLogLikelihoodRatioNormal(gtCases, gtMeasure, gtMeasureAux));
  }
  return dMaxLoglikelihoodRatio;
}

/** Calculates and returns maximizing value given accumulated cluster data. If data
    is not significant given scanning rate, negation of maximum double returned. */
double NormalProspectiveSpatialData::GetMaximizingValue(AbstractLikelihoodCalculator& Calculator) {
  assert(geEvaluationAssistDataStatus == Allocated);
  unsigned int  iWindowEnd;
  double        dMaxValue(-std::numeric_limits<double>::max());

  gtCases = gpCases[0];
  gtMeasure = gpMeasure[0];
  gtMeasureAux = gpMeasureAux[0];
  if ((Calculator.*Calculator.gpRateOfInterestNormal)(gtCases, gtMeasure, gtMeasureAux))
    dMaxValue = Calculator.CalculateMaximizingValueNormal(gtCases, gtMeasure, gtMeasureAux);

  for (iWindowEnd=1; iWindowEnd < giAllocationSize - 1; ++iWindowEnd) {
    gtCases = gpCases[0] - gpCases[iWindowEnd];
    gtMeasure = gpMeasure[0] - gpMeasure[iWindowEnd];
    gtMeasureAux = gpMeasureAux[0] - gpMeasureAux[iWindowEnd];
    if ((Calculator.*Calculator.gpRateOfInterestNormal)(gtCases, gtMeasure, gtMeasureAux))
      dMaxValue = std::max(dMaxValue, Calculator.CalculateMaximizingValueNormal(gtCases, gtMeasure, gtMeasureAux));
  }
  return dMaxValue;
}

/** Returns newly cloned NormalProspectiveSpatialData object. Caller responsible
    for deletion of object. */
NormalProspectiveSpatialData * NormalProspectiveSpatialData::Clone() const {
   return new NormalProspectiveSpatialData(*this);
}

/** Deallocates data members that assist with evaluation of temporal data.
    Once this function is called various class member functions become invalid
    and an assertion will fail if called. */
void NormalProspectiveSpatialData::DeallocateEvaluationAssistClassMembers() {
  try {
    delete[] gpCases; gpCases=0;
    delete[] gpMeasure; gpMeasure=0;
    delete[] gpMeasureAux; gpMeasureAux=0;
    giAllocationSize=0;
    geEvaluationAssistDataStatus = Deallocated;
  }
  catch (...){}
}

/** re-initialize data*/
void NormalProspectiveSpatialData::InitializeData() {
  assert(geEvaluationAssistDataStatus == Allocated);
  gtCases=0;
  gtMeasure=0;
  gtMeasureAux=0;
  memset(gpCases, 0, sizeof(count_t) * giAllocationSize);
  memset(gpMeasure, 0, sizeof(measure_t) * giAllocationSize);
  memset(gpMeasureAux, 0, sizeof(measure_t) * giAllocationSize);
}

/** overloaded assignement operator */
NormalProspectiveSpatialData & NormalProspectiveSpatialData::operator=(const NormalProspectiveSpatialData& rhs) {
   gtCases = rhs.gtCases;
   gtMeasure = rhs.gtMeasure;
   gtMeasureAux = rhs.gtMeasureAux;
   giAllocationSize = rhs.giAllocationSize;
   giNumTimeIntervals = rhs.giNumTimeIntervals;
   giProspectiveStart = rhs.giProspectiveStart;
   if (rhs.geEvaluationAssistDataStatus == Allocated) {
     if (!gpCases) gpCases = new count_t[rhs.giAllocationSize];
     if (!gpMeasure) gpMeasure = new measure_t[rhs.giAllocationSize];
     if (!gpMeasureAux) gpMeasureAux = new measure_t[rhs.giAllocationSize];
     memcpy(gpCases, rhs.gpCases, giAllocationSize * sizeof(count_t));
     memcpy(gpMeasure, rhs.gpMeasure, giAllocationSize * sizeof(measure_t));
     memcpy(gpMeasureAux, rhs.gpMeasureAux, giAllocationSize * sizeof(measure_t));
   }
   else {
    delete[] gpCases; gpCases=0;
    delete[] gpMeasure; gpMeasure=0;
    delete[] gpMeasureAux; gpMeasureAux=0;
   }
   geEvaluationAssistDataStatus = rhs.geEvaluationAssistDataStatus;
   return *this;
}

/** internal setup function */
void NormalProspectiveSpatialData::Setup(const CSaTScanData& Data, const DataSetInterface& Interface) {
  try {
    giAllocationSize = 1 + Data.GetNumTimeIntervals() - Data.GetProspectiveStartIndex();
    giNumTimeIntervals = Data.GetNumTimeIntervals();
    giProspectiveStart = Data.GetProspectiveStartIndex();
    gpCases = new count_t[giAllocationSize];
    memset(gpCases, 0, sizeof(count_t) * giAllocationSize);
    gpMeasure = new measure_t[giAllocationSize];
    memset(gpMeasure, 0, sizeof(measure_t) * giAllocationSize);
    gpMeasureAux = new measure_t[giAllocationSize];
    memset(gpMeasureAux, 0, sizeof(measure_t) * giAllocationSize);
  }
  catch (prg_exception& x) {
    delete[] gpCases;
    delete[] gpMeasure;
    delete[] gpMeasureAux;
    x.addTrace("Setup(const CSaTScanData&, const DataSetInterface&)","NormalProspectiveSpatialData");
    throw;
  }
}

//*************** class NormalSpaceTimeData ************************************

/** class constructor */
NormalSpaceTimeData::NormalSpaceTimeData(const DataSetInterface& Interface)
                    :NormalTemporalData(), geEvaluationAssistDataStatus(Allocated) {
  try {
    Setup(Interface);
  }
  catch (prg_exception& x) {
    x.addTrace("constructor(const DataSetInterface&)","NormalSpaceTimeData");
    throw;
  }
}

/** constructor */
NormalSpaceTimeData::NormalSpaceTimeData(const AbstractDataSetGateway& DataGateway): NormalTemporalData(), geEvaluationAssistDataStatus(Allocated), _start_index(0) {
    try {
        Setup(DataGateway.GetDataSetInterface());
    } catch (prg_exception& x) {
        x.addTrace("constructor(const AbstractDataSetGateway&)","NormalSpaceTimeData");
        throw;
    }
}

/** class copy constructor */
NormalSpaceTimeData::NormalSpaceTimeData(const NormalSpaceTimeData& rhs): NormalTemporalData(), _start_index(0) {
    try {
        *this = rhs;
    } catch (prg_exception& x) {
        x.addTrace("constructor(const NormalSpaceTimeData&)","NormalSpaceTimeData");
        throw;
    }
}

/** class destructor */
NormalSpaceTimeData::~NormalSpaceTimeData() {
    try {
        delete[] gpCases;
        delete[] gpMeasure;
        delete[] gpMeasureAux;
    } catch (...){}
}

/** Adds neighbor data to accumulation - caller is responsible for ensuring that
    'tNeighborIndex' and 'tSetIndex' are valid indexes. */
void NormalSpaceTimeData::AddNeighborData(tract_t tNeighborIndex, const AbstractDataSetGateway& DataGateway, size_t tSetIndex) {
    assert(geEvaluationAssistDataStatus == Allocated);
    count_t    ** ppCases = DataGateway.GetDataSetInterface(tSetIndex).GetCaseArray();
    measure_t  ** ppMeasure = DataGateway.GetDataSetInterface(tSetIndex).GetMeasureArray();
    measure_t  ** ppMeasureAux = DataGateway.GetDataSetInterface(tSetIndex).GetMeasureAuxArray();

    for (unsigned int i= _start_index; i < giAllocationSize - 1; ++i) {
        gpCases[i] += ppCases[i][tNeighborIndex];
        gpMeasure[i] += ppMeasure[i][tNeighborIndex];
        gpMeasureAux[i] += ppMeasureAux[i][tNeighborIndex];
    }
}

/** Assigns cluster data of passed object to 'this' object. Caller of function
    is responsible for ensuring that passed AbstractTemporalClusterData object
    can be casted to 'NormalSpaceTimeData' object. */
void NormalSpaceTimeData::Assign(const AbstractTemporalClusterData& rhs) {
    *this = (const NormalSpaceTimeData&)rhs;
}

/** Returns newly cloned NormalSpaceTimeData object. Caller responsible for deletion
    of object. */
NormalSpaceTimeData * NormalSpaceTimeData::Clone() const {
    return new NormalSpaceTimeData(*this);
}

/** Deallocates data members that assist with evaluation of temporal data.
    Once this function is called various class member functions become invalid
    and an assertion will fail if called. */
void NormalSpaceTimeData::DeallocateEvaluationAssistClassMembers() {
    try {
        delete[] gpCases; gpCases=0;
        delete[] gpMeasure; gpMeasure=0;
        delete[] gpMeasureAux; gpMeasureAux=0;
        giAllocationSize=0;
        geEvaluationAssistDataStatus = Deallocated;
    } catch (...){}
}

/** re-initialize data */
void NormalSpaceTimeData::InitializeData() {
  assert(geEvaluationAssistDataStatus == Allocated);
  gtCases=0;
  gtMeasure=0;
  gtMeasureAux=0;
  memset(gpCases, 0, sizeof(count_t) * giAllocationSize);
  memset(gpMeasure, 0, sizeof(measure_t) * giAllocationSize);
  memset(gpMeasureAux, 0, sizeof(measure_t) * giAllocationSize);
}

/** overloaded assignement operator */
NormalSpaceTimeData & NormalSpaceTimeData::operator=(const NormalSpaceTimeData& rhs) {
  gtCases = rhs.gtCases;
  gtMeasure = rhs.gtMeasure;
  gtMeasureAux = rhs.gtMeasureAux;
  giAllocationSize = rhs.giAllocationSize;
  if (rhs.geEvaluationAssistDataStatus == Allocated) {
    if (!gpCases) gpCases = new count_t[rhs.giAllocationSize];
    if (!gpMeasure) gpMeasure = new measure_t[rhs.giAllocationSize];
    if (!gpMeasureAux) gpMeasureAux = new measure_t[rhs.giAllocationSize];
    memcpy(gpCases, rhs.gpCases, giAllocationSize * sizeof(count_t));
    memcpy(gpMeasure, rhs.gpMeasure, giAllocationSize * sizeof(measure_t));
    memcpy(gpMeasureAux, rhs.gpMeasureAux, giAllocationSize * sizeof(measure_t));
  }
  else {
    delete[] gpCases; gpCases=0;
    delete[] gpMeasure; gpMeasure=0;
    delete[] gpMeasureAux; gpMeasureAux=0;
  }
  geEvaluationAssistDataStatus = rhs.geEvaluationAssistDataStatus;
  return *this;
}

/** internal setup function */
void NormalSpaceTimeData::Setup(const DataSetInterface& Interface) {
  try {
    //Note that giAllocationSize is number of time intervals plus one - this permits
    //us to evaluate last time intervals data with same code as other time intervals
    //in CTimeIntervals object.
    giAllocationSize = Interface.GetNumTimeIntervals() + 1;
    gpCases = new count_t[giAllocationSize];
    memset(gpCases, 0, sizeof(count_t) * giAllocationSize);
    gpMeasure = new measure_t[giAllocationSize];
    memset(gpMeasure, 0, sizeof(measure_t) * giAllocationSize);
    gpMeasureAux = new measure_t[giAllocationSize];
    memset(gpMeasureAux, 0, sizeof(measure_t) * giAllocationSize);
    _start_index = Interface.getDataStartIndex();
  }
  catch (prg_exception& x) {
    delete[] gpCases;
    delete[] gpMeasure;
    delete[] gpMeasureAux;
    x.addTrace("Setup(const DataSetInterface&)","NormalSpaceTimeData");
    throw;
  }
}

//************** class NormalCovariateSpatialData ***************************************

/** class constructor */
NormalCovariateSpatialData::NormalCovariateSpatialData(const DataSetInterface& Interface)
                  :NormalSpatialData(Interface), geEvaluationAssistDataStatus(Allocated),
                   _deltag(0), _w_div_delta(0), _xg(0), _tobeinversed(0), _xgsigmaw(0) {
    try {
        Setup(Interface);
    } catch (prg_exception& x) {
        x.addTrace("NormalCovariateSpatialData(const DataSetInterface&)","NormalCovariateSpatialData");
        throw;
    }
}

/** class constructor */
NormalCovariateSpatialData::NormalCovariateSpatialData(const AbstractDataSetGateway& DataGateway)
                  :NormalSpatialData(DataGateway), geEvaluationAssistDataStatus(Allocated),
                   _deltag(0), _w_div_delta(0), _xg(0), _tobeinversed(0), _xgsigmaw(0){
    try {
        Setup(DataGateway.GetDataSetInterface());
    } catch (prg_exception& x) {
        x.addTrace("NormalCovariateSpatialData(const AbstractDataSetGateway&)","NormalCovariateSpatialData");
        throw;
    }
}

/**  Copy constructor */
NormalCovariateSpatialData::NormalCovariateSpatialData(const NormalCovariateSpatialData& rhs)
                  :_deltag(0), _w_div_delta(0), _xg(0), _tobeinversed(0), _xgsigmaw(0) {
   try {
    *this = rhs;
  }
  catch (prg_exception& x) {
    x.addTrace("constructor(const NormalCovariateSpatialData&)","NormalCovariateSpatialData");
    throw;
  }
}

/** Destructor */
NormalCovariateSpatialData::~NormalCovariateSpatialData() {
    try {
      delete _deltag; _deltag=0;
      delete _w_div_delta; _w_div_delta=0;
      delete _xg; _xg=0;
      delete _tobeinversed; _tobeinversed=0;
      delete _xgsigmaw; _xgsigmaw=0;
    } catch (...) { }
}

/** Adds neighbor data to accumulation  - caller is responsible for ensuring that 'tNeighborIndex' and 'tSetIndex' are valid indexes. */
void NormalCovariateSpatialData::AddNeighborData(tract_t tNeighborIndex, const AbstractDataSetGateway& DataGateway, size_t tSetIndex) {
  assert(geEvaluationAssistDataStatus == Allocated);
  //update case and measure totals
  gtCases += DataGateway.GetDataSetInterface(tSetIndex).GetPSCaseArray()[tNeighborIndex];
  gtMeasure += DataGateway.GetDataSetInterface(tSetIndex).GetPSMeasureArray()[tNeighborIndex];
  gtMeasureAux += DataGateway.GetDataSetInterface(tSetIndex).GetPSMeasureAuxArray()[tNeighborIndex];

  //NOTE: There is a major assumption that is expected to be enforced in other parts of the program.
  //      When adding each neighbor's data, the number of cases should increase by exactly one. That
  //      assumption is carried forward with the operations to the matrices below.
  //      This iss how the algorithm was designed -- each location must have exactly one case record,
  //      not allowing missing data.

  //update matrices for addition of location
  _xg->element(tNeighborIndex,0) = 1;
  _tobeinversed->element(0,0) = _tobeinversed->element(0,0) + (1/_deltag->element(tNeighborIndex));
  for (int k=1; k < _tobeinversed->Ncols(); ++k) {
    _tobeinversed->element(0,k) = _tobeinversed->element(0,k) + _xg->element(tNeighborIndex,k)/_deltag->element(tNeighborIndex);
    _tobeinversed->element(k,0) = _tobeinversed->element(0,k);
  }
  _xgsigmaw->element(0,0) = _xgsigmaw->element(0,0) + _w_div_delta->element(tNeighborIndex);
}

/** overloaded assignement operator */
NormalCovariateSpatialData & NormalCovariateSpatialData::operator=(const NormalCovariateSpatialData& rhs) {
  gtCases = rhs.gtCases;
  gtMeasure = rhs.gtMeasure;
  gtMeasureAux = rhs.gtMeasureAux;
  if (rhs.geEvaluationAssistDataStatus == Allocated) {
      if (_deltag)
          *_deltag = *rhs._deltag;
      else
          _deltag = new ColumnVector(*rhs._deltag);
      if (_w_div_delta)
          *_w_div_delta = *rhs._w_div_delta;
      else
          _w_div_delta = new ColumnVector(*rhs._w_div_delta);
      if (_xg)
          *_xg = *rhs._xg;
      else
          _xg = new Matrix(*rhs._xg);
      if (_tobeinversed)
          *_tobeinversed = *rhs._tobeinversed;
      else
          _tobeinversed = new Matrix(*rhs._tobeinversed);
      if (_xgsigmaw)
          *_xgsigmaw = *rhs._xgsigmaw;
      else
          _xgsigmaw = new Matrix(*rhs._xgsigmaw);
  } else {
    delete _xg; _xg=0;
    delete _tobeinversed; _tobeinversed=0;
    delete _xgsigmaw; _xgsigmaw=0;
    delete _deltag; _deltag=0;
    delete _w_div_delta; _w_div_delta=0;
  }
  geEvaluationAssistDataStatus = rhs.geEvaluationAssistDataStatus;
  return *this;
}

/** Assigns cluster data of passed object to 'this' object. Caller of function
    is responsible for ensuring that passed AbstractSpatialClusterData object
    can be casted to 'NormalSpatialData' object. */
void NormalCovariateSpatialData::Assign(const AbstractSpatialClusterData& rhs) {
  *this = (const NormalCovariateSpatialData&)rhs;
}

/** Calculates loglikelihood ratio, given current accumulated cluster data, if
    it is determined that data fits scanning area of interest (high, low, both).
    Returns zero if rate not of interest else returns loglikelihood ratio as
    calculated by probability model. */
double NormalCovariateSpatialData::CalculateLoglikelihoodRatio(AbstractLikelihoodCalculator& Calculator) {
   assert(geEvaluationAssistDataStatus == Allocated);
   if ((Calculator.*Calculator.gpRateOfInterestNormal)(gtCases, gtMeasure, gtMeasureAux)) {
      double ratio = Calculator.CalcLogLikelihoodRatioNormal(*_xg, *_tobeinversed,*_xgsigmaw);
      return ratio;
   }
   return 0;
}

/** Returns newly cloned NormalSpatialData object. Caller is responsible for deletion of object. */
NormalCovariateSpatialData * NormalCovariateSpatialData::Clone() const {
  return new NormalCovariateSpatialData(*this);
}

/** Copies class data members that reflect the number of cases, expected, and expected
    squared values, which is the data we are interested in for possiblely reporting. */
void NormalCovariateSpatialData::CopyEssentialClassMembers(const AbstractClusterData& rhs) {
  gtCases = ((const NormalCovariateSpatialData&)rhs).gtCases;
  gtMeasure = ((const NormalCovariateSpatialData&)rhs).gtMeasure;
  gtMeasureAux = ((const NormalCovariateSpatialData&)rhs).gtMeasureAux;
}

/** Deallocates data members that assist with evaluation of temporal data.
    Once this function is called various class member functions become invalid
    and an assertion will fail if called. */
void NormalCovariateSpatialData::DeallocateEvaluationAssistClassMembers() {
  try {
    delete _deltag; _deltag=0;
    delete _xg; _xg=0;
    delete _w_div_delta; _w_div_delta=0;
    geEvaluationAssistDataStatus = Deallocated;
  }
  catch (...){}
}

/** Calculates and returns maximizing value given accumulated cluster data. If data
    is not significant given scanning rate, negation of maximum double returned. */
double NormalCovariateSpatialData::GetMaximizingValue(AbstractLikelihoodCalculator& Calculator) {
  assert(geEvaluationAssistDataStatus == Allocated);
  if ((Calculator.*Calculator.gpRateOfInterestNormal)(gtCases, gtMeasure, gtMeasureAux))
    return Calculator.CalculateMaximizingValueNormal(*_xg, *_tobeinversed,*_xgsigmaw);   
  return -std::numeric_limits<double>::max();
}

/** Initializes data given passed data gateway. Accesses randomizer of data interface to
    retrieve initial data structures. */
void NormalCovariateSpatialData::InitializeData(const AbstractDataSetGateway& DataGateway) {
   try {
     delete _deltag; _deltag=0;
     delete _xg; _xg=0;
     delete _w_div_delta; _w_div_delta=0;

     const DataSetInterface& Interface = DataGateway.GetDataSetInterface();
     const AbstractWeightedNormalRandomizer* randomizer = dynamic_cast<const AbstractWeightedNormalRandomizer*>(Interface.GetRandomizer());
     if (randomizer == 0)
        throw prg_error("Unable to cast pointer to AbstractWeightedNormalRandomizer object.","InitializeData(const AbstractDataSetGateway&)");

     std::auto_ptr<ColumnVector> wg, deltag;
     randomizer->get_wg_deltag(wg, deltag);
     _deltag = deltag.release();

     _w_div_delta = new ColumnVector(_deltag->Nrows());
     for (int r=0; r < _w_div_delta->Nrows(); ++r)
        _w_div_delta->element(r) = wg->element(r)/_deltag->element(r);

     std::auto_ptr<Matrix> xg;
     randomizer->get_xg(xg,false);
     _xg = xg.release();
     geEvaluationAssistDataStatus = Allocated;
     InitializeData();
   } catch (prg_exception& x) {
       x.addTrace("InitializeData(const AbstractDataSetGateway&)","NormalCovariateSpatialData");
       throw;
   }
}

/** Initializes data such that no observations are in data. */
void NormalCovariateSpatialData::InitializeData() {
    assert(geEvaluationAssistDataStatus == Allocated);
    gtCases=0;gtMeasure=0;gtMeasureAux=0;
    //set all elements of first column to zero -- meaing no tract in data
    for (int r=0; r < _xg->Nrows(); ++r) {
      _xg->element(r,0) = 0;
    }
    //initialize the the matrix that will be inversed
    Matrix temp(*_xg);
    for (int r=0; r < temp.Nrows(); ++r)
       for (int c=0; c < temp.Ncols(); ++c) {
           temp.element(r,c) = temp.element(r,c)/_deltag->element(r);
       }
    if (_tobeinversed)
        *_tobeinversed = _xg->t() * temp;
    else
        _tobeinversed = new Matrix(_xg->t() * temp);
    //initialize xg sigma w matrix
    if (_xgsigmaw)
        *_xgsigmaw = _xg->t() * (*_w_div_delta);
    else 
        _xgsigmaw = new Matrix(_xg->t() * (*_w_div_delta));
}

/** internal setup function */
void NormalCovariateSpatialData::Setup(const DataSetInterface& Interface) {
    try {
        DataSetGateway gateway;
        gateway.AddDataSetInterface(const_cast<DataSetInterface&>(Interface));
        InitializeData(gateway);
    } catch (prg_exception& x) {
        x.addTrace("NormalCovariateSpatialData(const AbstractDataSetGateway&)","Setup()");
        throw;
    }
}
