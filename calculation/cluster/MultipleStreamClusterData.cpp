//---------------------------------------------------------------------------
#include "SaTScan.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#include "MultipleStreamClusterData.h"
#include "ClusterDataFactory.h"

/** constructor */
MultipleStreamSpatialData::MultipleStreamSpatialData(const ClusterDataFactory& DataFactory, const AbtractDataStreamGateway & DataGateway, int iRate)
                          :AbstractSpatialClusterData(iRate) {
  for (size_t t=0; t < DataGateway.GetNumInterfaces(); ++t)
     gvStreamData.push_back(DataFactory.GetNewSpatialClusterData(DataGateway.GetDataStreamInterface(t), iRate));
}

/** destructor */
MultipleStreamSpatialData::~MultipleStreamSpatialData() {}

/** return newly cloned MultipleStreamSpatialData object */
MultipleStreamSpatialData * MultipleStreamSpatialData::Clone() const {
  return new MultipleStreamSpatialData(*this);
}

/** assigns cluster data of passed object to *this* object
    NOTE: Caller of function is responsible for ensuring that passed object
          is of same class type as *this* object. */
void MultipleStreamSpatialData::Assign(const AbstractSpatialClusterData& rhs) {
  const MultipleStreamSpatialData& _rhs = (const MultipleStreamSpatialData&)rhs;
  gvStreamData = _rhs.gvStreamData;
}

/** not implemented - throws exception */
void MultipleStreamSpatialData::AddMeasureList(CMeasureList * pMeasureList, tract_t tNeighbor, const DataStreamInterface & Interface) {
   ZdGenerateException("AddMeasureList() not implemented.","MultipleStreamSpatialData");
}

/** adds neighbor data to accumulation */
void MultipleStreamSpatialData::AddNeighborData(tract_t tNeighbor, const AbtractDataStreamGateway & DataGateway, size_t) {
  unsigned int i;
  for (i=0, gitr=gvStreamData.begin(); gitr != gvStreamData.end(); ++i, ++gitr)
     (*gitr)->AddNeighborData(tNeighbor, DataGateway, i);
}

/** Calculates loglikelihood ratio given current accumulated cluster data in
    each data stream and adds together. */
double MultipleStreamSpatialData::CalculateLoglikelihoodRatio(AbstractLikelihoodCalculator & Calculator) {
  double        dLogLikelihoodRatio=0;

  for (gitr=gvStreamData.begin(); gitr != gvStreamData.end(); ++gitr)
     dLogLikelihoodRatio += (*gitr)->CalculateLoglikelihoodRatio(Calculator);

  return dLogLikelihoodRatio;
}

/** returns number of cases in accumulated respective data streams's cluster data */
count_t MultipleStreamSpatialData::GetCaseCount(unsigned int iStream) const {
  return gvStreamData[iStream]->GetCaseCount();
}

/** returns expected number of cases in accumulated respective data streams's cluster data */
measure_t MultipleStreamSpatialData::GetMeasure(unsigned int iStream) const {
  return gvStreamData[iStream]->GetMeasure();
}

/** initializes cluster data in each data stream */
void MultipleStreamSpatialData::InitializeData() {
  for (gitr=gvStreamData.begin(); gitr != gvStreamData.end(); ++gitr)
     (*gitr)->InitializeData();
}


/** constructor */
MultipleStreamTemporalData::MultipleStreamTemporalData(const ClusterDataFactory& DataFactory, const AbtractDataStreamGateway & DataGateway)
                           :AbstractMultipleStreamTemporalData() {
  for (size_t t=0; t < DataGateway.GetNumInterfaces(); ++t)
     gvStreamData.push_back(DataFactory.GetNewTemporalClusterDataAsTemporalData(DataGateway.GetDataStreamInterface(t)));
}

/** destructor */
MultipleStreamTemporalData::~MultipleStreamTemporalData() {}

/** return newly cloned MultipleStreamTemporalData object */
MultipleStreamTemporalData * MultipleStreamTemporalData::Clone() const {
  return new MultipleStreamTemporalData(*this);
}

/** assigns cluster data of passed object to *this* object
    NOTE: Caller of function is responsible for ensuring that passed object
          is of same class type as *this* object. */
void MultipleStreamTemporalData::Assign(const AbstractTemporalClusterData& rhs) {
  const MultipleStreamTemporalData& _rhs = (const MultipleStreamTemporalData&)rhs;
  gvStreamData = _rhs.gvStreamData;
}

/** not implemented - throws exception */
void MultipleStreamTemporalData::AddNeighborData(tract_t, const AbtractDataStreamGateway&, size_t) {
  ZdGenerateException("AddNeighbor() not implemeneted.","TemporalData");
}

/** returns number of cases in accumulated respective data streams's cluster data */
count_t MultipleStreamTemporalData::GetCaseCount(unsigned int iStream) const {
  return gvStreamData[iStream]->gtCases;
}

/** returns expected number of cases in accumulated respective data streams's cluster data */
measure_t MultipleStreamTemporalData::GetMeasure(unsigned int iStream) const {
  return gvStreamData[iStream]->gtMeasure;
}

/** initializes cluster data in each data stream */
void MultipleStreamTemporalData::InitializeData() {
  for (gitr=gvStreamData.begin(); gitr != gvStreamData.end(); ++gitr)
     (*gitr)->InitializeData();
}


/** constructor */
MultipleStreamProspectiveSpatialData::MultipleStreamProspectiveSpatialData(const ClusterDataFactory& DataFactory, const CSaTScanData & Data, const AbtractDataStreamGateway & DataGateway)
                                     :AbstractMultipleStreamTemporalData() {
  for (size_t t=0; t < DataGateway.GetNumInterfaces(); ++t)
     gvStreamData.push_back(DataFactory.GetNewProspectiveSpatialClusterDataAsTemporalData(Data, DataGateway.GetDataStreamInterface(t)));
  switch (Data.GetParameters().GetAreaScanRateType()) {
    case LOW        : gfRateOfInterest = LowRate;       break;
    case HIGHANDLOW : gfRateOfInterest = HighOrLowRate; break;
    case HIGH       :
    default         : gfRateOfInterest = HighRate;
  };
}

/** destructor */
MultipleStreamProspectiveSpatialData::~MultipleStreamProspectiveSpatialData() {}

/** return newly cloned MultipleStreamProspectiveSpatialData object */
MultipleStreamProspectiveSpatialData * MultipleStreamProspectiveSpatialData::Clone() const {
  return new MultipleStreamProspectiveSpatialData(*this);
}

/** assigns cluster data of passed object to *this* object
    NOTE: Caller of function is responsible for ensuring that passed object
          is of same class type as *this* object. */
void MultipleStreamProspectiveSpatialData::Assign(const AbstractTemporalClusterData& rhs) {
  const MultipleStreamProspectiveSpatialData& _rhs = (const MultipleStreamProspectiveSpatialData&)rhs;
  gvStreamData = _rhs.gvStreamData;
}

/** adds neighbor data to accumulated cluster data*/
void MultipleStreamProspectiveSpatialData::AddNeighborData(tract_t tNeighbor, const AbtractDataStreamGateway & DataGateway, size_t) {
  unsigned int i;
  for (i=0, gitr=gvStreamData.begin(); gitr != gvStreamData.end(); ++i, ++gitr)
     (*gitr)->AddNeighborData(tNeighbor, DataGateway, i);
}

/** Calculates loglikelihood ratio given current accumulated cluster data in
    each data stream and adds together.*/
double MultipleStreamProspectiveSpatialData::CalculateLoglikelihoodRatio(AbstractLikelihoodCalculator & Calculator) {
  unsigned int  iWindowEnd, iAllocationSize;
  double        dLoglikelihood, dMaxLoglikelihood=0;

  iAllocationSize = (*gvStreamData.begin())->GetAllocationSize();
  for (gitr=gvStreamData.begin(); gitr != gvStreamData.end(); ++gitr) {
    (*gitr)->gtCases = (*gitr)->gpCases[0];
    (*gitr)->gtMeasure =  (*gitr)->gpMeasure[0];
    if (gfRateOfInterest((*gitr)->gtCases, (*gitr)->gtMeasure, (*gitr)->gtTotalCases, (*gitr)->gtTotalMeasure))
      dLoglikelihood += Calculator.CalcLogLikelihoodRatio((*gitr)->gtCases, (*gitr)->gtMeasure, (*gitr)->gtTotalCases, (*gitr)->gtTotalMeasure);
  }
  dMaxLoglikelihood = std::max(dMaxLoglikelihood, dLoglikelihood);

  for (iWindowEnd=1; iWindowEnd < iAllocationSize; ++iWindowEnd) {
     dLoglikelihood=0;
     for (gitr=gvStreamData.begin(); gitr != gvStreamData.end(); ++gitr) {
        (*gitr)->gtCases = (*gitr)->gpCases[0] - (*gitr)->gpCases[iWindowEnd];
        (*gitr)->gtMeasure =  (*gitr)->gpMeasure[0] - (*gitr)->gpMeasure[iWindowEnd];
        if (gfRateOfInterest((*gitr)->gtCases, (*gitr)->gtMeasure, (*gitr)->gtTotalCases, (*gitr)->gtTotalMeasure))
         dLoglikelihood += Calculator.CalcLogLikelihoodRatio((*gitr)->gtCases, (*gitr)->gtMeasure, (*gitr)->gtTotalCases, (*gitr)->gtTotalMeasure);
     }
     dMaxLoglikelihood = std::max(dMaxLoglikelihood, dLoglikelihood);
  }

  return dMaxLoglikelihood;
}

/** returns number of cases in accumulated respective data streams's cluster data */
count_t MultipleStreamProspectiveSpatialData::GetCaseCount(unsigned int iStream) const {
  return gvStreamData[iStream]->gtCases;
}

/** returns expected number of cases in accumulated respective data streams's cluster data */
measure_t MultipleStreamProspectiveSpatialData::GetMeasure(unsigned int iStream) const {
  return gvStreamData[iStream]->gtMeasure;
}

/** initializes cluster data in each data stream */
void MultipleStreamProspectiveSpatialData::InitializeData() {
  for (gitr=gvStreamData.begin(); gitr != gvStreamData.end(); ++gitr)
     (*gitr)->InitializeData();
}



/** constructor */
MultipleStreamSpaceTimeData::MultipleStreamSpaceTimeData(const ClusterDataFactory& DataFactory, const AbtractDataStreamGateway & DataGateway)
                            :AbstractMultipleStreamTemporalData() {
  gvStreamData.resize(DataGateway.GetNumInterfaces(), 0);
  for (size_t t=0; t < DataGateway.GetNumInterfaces(); ++t)
     gvStreamData[t] = DataFactory.GetNewSpaceTimeClusterDataAsTemporalData(DataGateway.GetDataStreamInterface(t));
}

/** destructor */
MultipleStreamSpaceTimeData::~MultipleStreamSpaceTimeData() {}

/** return newly cloned MultipleStreamSpaceTimeData object */
MultipleStreamSpaceTimeData * MultipleStreamSpaceTimeData::Clone() const {
  return new MultipleStreamSpaceTimeData(*this);
}

/** assigns cluster data of passed object to *this* object
    NOTE: Caller of function is responsible for ensuring that passed object
          is of same class type as *this* object. */
void MultipleStreamSpaceTimeData::Assign(const AbstractTemporalClusterData& rhs) {
  const MultipleStreamSpaceTimeData& _rhs = (const MultipleStreamSpaceTimeData&)rhs;
  gvStreamData = _rhs.gvStreamData;
}

/** add neighbor data to accumulation */
void MultipleStreamSpaceTimeData::AddNeighborData(tract_t tNeighbor, const AbtractDataStreamGateway & DataGateway, size_t) {
  unsigned int i;
  for (i=0, gitr=gvStreamData.begin(); gitr != gvStreamData.end(); ++i, ++gitr)
     (*gitr)->AddNeighborData(tNeighbor, DataGateway, i);
}
 
/** returns number of cases in accumulated respective data streams's cluster data */
count_t MultipleStreamSpaceTimeData::GetCaseCount(unsigned int iStream) const {
  return gvStreamData[iStream]->gtCases;
}

/** returns expected number of cases in accumulated respective data streams's cluster data */
measure_t MultipleStreamSpaceTimeData::GetMeasure(unsigned int iStream) const {
  return gvStreamData[iStream]->gtMeasure;
}

/** initializes cluster data in each data stream */
void MultipleStreamSpaceTimeData::InitializeData() {
  for (gitr=gvStreamData.begin(); gitr != gvStreamData.end(); ++gitr)
     (*gitr)->InitializeData();
}

