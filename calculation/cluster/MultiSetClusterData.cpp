//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "MultiSetClusterData.h"
#include "ClusterDataFactory.h"
#include "LoglikelihoodRatioUnifier.h"

/** class constructor */
MultiSetSpatialData::MultiSetSpatialData(const ClusterDataFactory& DataFactory, const AbstractDataSetGateway& DataGateway, int iRate)
                    :AbstractSpatialClusterData(iRate) {
  //Allocate SpatialData object for each data set.
  for (size_t t=0; t < DataGateway.GetNumInterfaces(); ++t)
     gvSetClusterData.push_back(dynamic_cast<SpatialData*>(DataFactory.GetNewSpatialClusterData(DataGateway.GetDataSetInterface(t), iRate)));
}

/** class destructor */
MultiSetSpatialData::~MultiSetSpatialData() {}

/** Returns newly cloned MultiSetSpatialData object. Caller responsible for deletion
    of object. */
MultiSetSpatialData * MultiSetSpatialData::Clone() const {
  return new MultiSetSpatialData(*this);
}

/** Assigns cluster data of passed object to 'this' object. Caller of function
    is responsible for ensuring that passed AbstractSpatialClusterData object
    can be casted to 'MultiSetSpatialData' object. */
void MultiSetSpatialData::Assign(const AbstractSpatialClusterData& rhs) {
  const MultiSetSpatialData& _rhs = (const MultiSetSpatialData&)rhs;
  gvSetClusterData = _rhs.gvSetClusterData;
}

/** Not implemented - throws ZdException. */
void MultiSetSpatialData::AddMeasureList(CMeasureList*, tract_t, const DataSetInterface&) {
  ZdGenerateException("AddMeasureList(CMeasureList*, tract_t, const DataSetInterface&) not implemented.","MultiSetSpatialData");
}

/** Adds neighbor data to accumulation  - caller is responsible for ensuring that
    'tNeighborIndex' is a valid index. */
void MultiSetSpatialData::AddNeighborData(tract_t tNeighborIndex, const AbstractDataSetGateway & DataGateway, size_t) {
  unsigned int i;
  for (i=0, gitr=gvSetClusterData.begin(); gitr != gvSetClusterData.end(); ++i, ++gitr)
     (*gitr)->AddNeighborData(tNeighborIndex, DataGateway, i);
}

/** Calculates loglikelihood ratio given current accumulated cluster data in
    each data set and adds together. */
double MultiSetSpatialData::CalculateLoglikelihoodRatio(AbstractLikelihoodCalculator& Calculator) {
  unsigned int                          i=0;
  AbstractLoglikelihoodRatioUnifier   & Unifier = Calculator.GetUnifier();

  Unifier.Reset();
  for (gitr=gvSetClusterData.begin(); gitr != gvSetClusterData.end(); ++gitr, ++i)
     Unifier.AdjoinRatio(Calculator, (*gitr)->gtCases, (*gitr)->gtMeasure, (*gitr)->gtTotalCases, (*gitr)->gtTotalMeasure);
  return Unifier.GetLoglikelihoodRatio();
}

/** Returns number of cases in accumulated respective data sets' cluster data.
    Caller responsible for ensuring that 'tSetIndex' is a valid index. */
count_t MultiSetSpatialData::GetCaseCount(unsigned int tSetIndex) const {
  return gvSetClusterData[tSetIndex]->GetCaseCount();
}

/** Returns expected number of cases in accumulated respective data sets' cluster data.
    Caller responsible for ensuring that 'tSetIndex' is a valid index. */
measure_t MultiSetSpatialData::GetMeasure(unsigned int tSetIndex) const {
  return gvSetClusterData[tSetIndex]->GetMeasure();
}

/** Initializes cluster data in each data set. */
void MultiSetSpatialData::InitializeData() {
  for (gitr=gvSetClusterData.begin(); gitr != gvSetClusterData.end(); ++gitr)
     (*gitr)->InitializeData();
}

//******************************************************************************

/** class constructor */
MultiSetTemporalData::MultiSetTemporalData(const ClusterDataFactory& DataFactory, const AbstractDataSetGateway& DataGateway)
                     :AbstractMultiSetTemporalData() {
  //Allocate TemporalData object for each data set.
  for (size_t t=0; t < DataGateway.GetNumInterfaces(); ++t)
     gvSetClusterData.push_back(dynamic_cast<TemporalData*>(DataFactory.GetNewTemporalClusterData(DataGateway.GetDataSetInterface(t))));
}

/** class destructor */
MultiSetTemporalData::~MultiSetTemporalData() {}

/** Returns newly cloned MultiSetTemporalData object. Caller responsible for
    deletion of object. */
MultiSetTemporalData * MultiSetTemporalData::Clone() const {
  return new MultiSetTemporalData(*this);
}

/** Assigns cluster data of passed object to 'this' object. Caller of function
    is responsible for ensuring that passed AbstractTemporalClusterData object
    can be casted to 'MultiSetTemporalData' object. */
void MultiSetTemporalData::Assign(const AbstractTemporalClusterData& rhs) {
  const MultiSetTemporalData& _rhs = (const MultiSetTemporalData&)rhs;
  gvSetClusterData = _rhs.gvSetClusterData;
}

/** Not implemented - throws ZdException. */
void MultiSetTemporalData::AddNeighborData(tract_t, const AbstractDataSetGateway&, size_t) {
  ZdGenerateException("AddNeighbor(tract_t, const AbstractDataSetGateway&, size_t) not implemeneted.","MultiSetTemporalData");
}

/** Returns number of cases in accumulated respective data sets' cluster data.
    Caller is responsible for ensuring that 'tSetIndex' is a valid index. */
count_t MultiSetTemporalData::GetCaseCount(unsigned int tSetIndex) const {
  return gvSetClusterData[tSetIndex]->gtCases;
}

/** Returns expected number of cases in accumulated respective data sets' cluster data.
    Caller is responsible for ensuring that 'tSetIndex' is a valid index. */
measure_t MultiSetTemporalData::GetMeasure(unsigned int tSetIndex) const {
  return gvSetClusterData[tSetIndex]->gtMeasure;
}

/** Initializes cluster data in each data set. */
void MultiSetTemporalData::InitializeData() {
  for (gitr=gvSetClusterData.begin(); gitr != gvSetClusterData.end(); ++gitr)
     (*gitr)->InitializeData();
}

/** Reassociates internal data with passed DataSetInterface pointers. */
void MultiSetTemporalData::Reassociate(const DataSetInterface& Interface) {
  for (gitr=gvSetClusterData.begin(); gitr != gvSetClusterData.end(); ++gitr)
     (*gitr)->Reassociate(Interface);
}

/** Reassociates internal data with passed DataSetInterface pointers of DataGateway. */
void MultiSetTemporalData::Reassociate(const AbstractDataSetGateway& DataGateway) {
  for (gitr=gvSetClusterData.begin(); gitr != gvSetClusterData.end(); ++gitr)
     (*gitr)->Reassociate(DataGateway);
}
//******************************************************************************

/** class constructor */
MultiSetProspectiveSpatialData::MultiSetProspectiveSpatialData(const ClusterDataFactory& DataFactory, const CSaTScanData& Data, const AbstractDataSetGateway& DataGateway)
                               :AbstractMultiSetTemporalData() {
  //Allocate ProspectiveSpatialData object for each data set.
  for (size_t t=0; t < DataGateway.GetNumInterfaces(); ++t)
     gvSetClusterData.push_back(dynamic_cast<TemporalData*>(DataFactory.GetNewProspectiveSpatialClusterData(Data, DataGateway.GetDataSetInterface(t))));
  switch (Data.GetParameters().GetAreaScanRateType()) {
    case LOW        : gfRateOfInterest = LowRate;       break;
    case HIGHANDLOW : gfRateOfInterest = HighOrLowRate; break;
    case HIGH       :
    default         : gfRateOfInterest = HighRate;
  };
}

/** class destructor */
MultiSetProspectiveSpatialData::~MultiSetProspectiveSpatialData() {}

/** Returns newly cloned MultiSetProspectiveSpatialData object. Caller responsible
    for deletion of object. */
MultiSetProspectiveSpatialData * MultiSetProspectiveSpatialData::Clone() const {
  return new MultiSetProspectiveSpatialData(*this);
}

/** Assigns cluster data of passed object to 'this' object. Caller of function
    is responsible for ensuring that passed AbstractTemporalClusterData object
    can be casted to 'MultiSetProspectiveSpatialData' object. */
void MultiSetProspectiveSpatialData::Assign(const AbstractTemporalClusterData& rhs) {
  const MultiSetProspectiveSpatialData& _rhs = (const MultiSetProspectiveSpatialData&)rhs;
  gvSetClusterData = _rhs.gvSetClusterData;
}

/** Adds neighbor data to accumulated cluster data  - caller is responsible for
    ensuring that 'tNeighborIndex' is a valid index. */
void MultiSetProspectiveSpatialData::AddNeighborData(tract_t tNeighborIndex, const AbstractDataSetGateway& DataGateway, size_t) {
  unsigned int i;
  for (i=0, gitr=gvSetClusterData.begin(); gitr != gvSetClusterData.end(); ++i, ++gitr)
     (*gitr)->AddNeighborData(tNeighborIndex, DataGateway, i);
}

/** Calculates loglikelihood ratio given current accumulated cluster data in
    each data set and adds together.*/
double MultiSetProspectiveSpatialData::CalculateLoglikelihoodRatio(AbstractLikelihoodCalculator& Calculator) {
  unsigned int                          iWindowEnd, iAllocationSize;
  double                                dMaxLoglikelihoodRatio=0;
  AbstractLoglikelihoodRatioUnifier   & Unifier = Calculator.GetUnifier();

  Unifier.Reset();
  iAllocationSize = (*gvSetClusterData.begin())->GetAllocationSize();
  for (gitr=gvSetClusterData.begin(); gitr != gvSetClusterData.end(); ++gitr)
     Unifier.AdjoinRatio(Calculator, (*gitr)->gpCases[0], (*gitr)->gpMeasure[0], (*gitr)->gtTotalCases, (*gitr)->gtTotalMeasure);
  dMaxLoglikelihoodRatio = Unifier.GetLoglikelihoodRatio();

  for (iWindowEnd=1; iWindowEnd < iAllocationSize; ++iWindowEnd) {
     Unifier.Reset();
     for (gitr=gvSetClusterData.begin(); gitr != gvSetClusterData.end(); ++gitr) {
        (*gitr)->gtCases = (*gitr)->gpCases[0] - (*gitr)->gpCases[iWindowEnd];
        (*gitr)->gtMeasure =  (*gitr)->gpMeasure[0] - (*gitr)->gpMeasure[iWindowEnd];
        Unifier.AdjoinRatio(Calculator, (*gitr)->gtCases, (*gitr)->gtMeasure,
                            (*gitr)->gtTotalCases, (*gitr)->gtTotalMeasure);
     }
     dMaxLoglikelihoodRatio = std::max(dMaxLoglikelihoodRatio, Unifier.GetLoglikelihoodRatio());
  }

  return dMaxLoglikelihoodRatio;
}

/** Returns number of cases in accumulated respective data sets' cluster data.
    Caller is responsible for ensuring that 'tSetIndex' is a valid index. */
count_t MultiSetProspectiveSpatialData::GetCaseCount(unsigned int tSetIndex) const {
  return gvSetClusterData[tSetIndex]->gtCases;
}

/** Returns expected number of cases in accumulated respective data sets' cluster data.
    Caller is responsible for ensuring that 'tSetIndex' is a valid index. */
measure_t MultiSetProspectiveSpatialData::GetMeasure(unsigned int tSetIndex) const {
  return gvSetClusterData[tSetIndex]->gtMeasure;
}

/** Initializes cluster data in each data set. */
void MultiSetProspectiveSpatialData::InitializeData() {
  for (gitr=gvSetClusterData.begin(); gitr != gvSetClusterData.end(); ++gitr)
     (*gitr)->InitializeData();
}

//******************************************************************************

/** class constructor */
MultiSetSpaceTimeData::MultiSetSpaceTimeData(const ClusterDataFactory& DataFactory, const AbstractDataSetGateway& DataGateway)
                      :AbstractMultiSetTemporalData() {
  //Allocate SpaceTimeData object for each data set.
  gvSetClusterData.resize(DataGateway.GetNumInterfaces(), 0);
  for (size_t t=0; t < DataGateway.GetNumInterfaces(); ++t)
     gvSetClusterData[t] = dynamic_cast<TemporalData*>(DataFactory.GetNewSpaceTimeClusterData(DataGateway.GetDataSetInterface(t)));
}

/** class destructor */
MultiSetSpaceTimeData::~MultiSetSpaceTimeData() {}

/** Returns newly cloned MultiSetSpaceTimeData object. Caller responsible for deletion
    of object. */
MultiSetSpaceTimeData * MultiSetSpaceTimeData::Clone() const {
  return new MultiSetSpaceTimeData(*this);
}

/** Assigns cluster data of passed object to 'this' object. Caller of function
    is responsible for ensuring that passed AbstractTemporalClusterData object
    can be casted to 'MultiSetSpaceTimeData' object. */
void MultiSetSpaceTimeData::Assign(const AbstractTemporalClusterData& rhs) {
  const MultiSetSpaceTimeData& _rhs = (const MultiSetSpaceTimeData&)rhs;
  gvSetClusterData = _rhs.gvSetClusterData;
}

/** Add neighbor data to accumulation  - caller is responsible for
    ensuring that 'tNeighborIndex' is a valid index. */
void MultiSetSpaceTimeData::AddNeighborData(tract_t tNeighborIndex, const AbstractDataSetGateway& DataGateway, size_t) {
  unsigned int i;
  for (i=0, gitr=gvSetClusterData.begin(); gitr != gvSetClusterData.end(); ++i, ++gitr)
     (*gitr)->AddNeighborData(tNeighborIndex, DataGateway, i);
}
 
/** Returns number of cases in accumulated respective data sets' cluster data.
    Caller is responsible for ensuring that 'tSetIndex' is a valid index. */
count_t MultiSetSpaceTimeData::GetCaseCount(unsigned int tSetIndex) const {
  return gvSetClusterData[tSetIndex]->gtCases;
}

/** Returns expected number of cases in accumulated respective data sets' cluster data.
    Caller is responsible for ensuring that 'tSetIndex' is a valid index. */
measure_t MultiSetSpaceTimeData::GetMeasure(unsigned int tSetIndex) const {
  return gvSetClusterData[tSetIndex]->gtMeasure;
}

/** Initializes cluster data in each data set. */
void MultiSetSpaceTimeData::InitializeData() {
  for (gitr=gvSetClusterData.begin(); gitr != gvSetClusterData.end(); ++gitr)
     (*gitr)->InitializeData();
}

