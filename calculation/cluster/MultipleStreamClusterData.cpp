//---------------------------------------------------------------------------
#include "SaTScan.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#include "MultipleStreamClusterData.h"
#include "ClusterDataFactory.h"

/** constructor */
MutlipleStreamSpatialData::MutlipleStreamSpatialData(const ClusterDataFactory& DataFactory, const AbtractDataStreamGateway & DataGateway, int iRate)
                          :AbstractSpatialClusterData(iRate) {
  for (size_t t=0; t < DataGateway.GetNumInterfaces(); ++t)
     gvStreamData.push_back(DataFactory.GetNewSpatialClusterData(DataGateway.GetDataStreamInterface(t), iRate));
}

/** destructor */
MutlipleStreamSpatialData::~MutlipleStreamSpatialData() {}

/** return newly cloned MutlipleStreamSpatialData object */
MutlipleStreamSpatialData * MutlipleStreamSpatialData::Clone() const {
  return new MutlipleStreamSpatialData(*this);
}

/** assigns cluster data of passed object to *this* object
    NOTE: Caller of function is responsible for ensuring that passed object
          is of same class type as *this* object. */
void MutlipleStreamSpatialData::Assign(const AbstractSpatialClusterData& rhs) {
  const MutlipleStreamSpatialData& _rhs = (const MutlipleStreamSpatialData&)rhs;
  gvStreamData = _rhs.gvStreamData;
}

/** not implemented - throws exception */
void MutlipleStreamSpatialData::AddMeasureList(CMeasureList * pMeasureList, tract_t tNeighbor, const DataStreamInterface & Interface) {
   ZdGenerateException("AddMeasureList() not implemented.","MutlipleStreamSpatialData");
}

/** adds neighbor data to accumulation */
void MutlipleStreamSpatialData::AddNeighborData(tract_t tNeighbor, const AbtractDataStreamGateway & DataGateway, size_t) {
  unsigned int i;
  for (i=0, gitr=gvStreamData.begin(); gitr != gvStreamData.end(); ++i, ++gitr)
     (*gitr)->AddNeighborData(tNeighbor, DataGateway, i);
}

/** Calculates loglikelihood ratio given current accumulated cluster data in
    each data stream and adds together. */
double MutlipleStreamSpatialData::CalculateLoglikelihoodRatio(CModel & Model) {
  double        dLogLikelihoodRatio=0;

  for (gitr=gvStreamData.begin(); gitr != gvStreamData.end(); ++gitr)
     dLogLikelihoodRatio += (*gitr)->CalculateLoglikelihoodRatio(Model);

  return dLogLikelihoodRatio;
}

/** returns number of cases in accumulated respective data streams's cluster data */
count_t MutlipleStreamSpatialData::GetCaseCount(unsigned int iStream) const {
  return gvStreamData[iStream]->GetCaseCount();
}

/** returns expected number of cases in accumulated respective data streams's cluster data */
measure_t MutlipleStreamSpatialData::GetMeasure(unsigned int iStream) const {
  return gvStreamData[iStream]->GetMeasure();
}

/** initializes cluster data in each data stream */
void MutlipleStreamSpatialData::InitializeData() {
  for (gitr=gvStreamData.begin(); gitr != gvStreamData.end(); ++gitr)
     (*gitr)->InitializeData();
}


/** constructor */
MutlipleStreamTemporalData::MutlipleStreamTemporalData(const ClusterDataFactory& DataFactory, const AbtractDataStreamGateway & DataGateway)
                           :AbstractMutlipleStreamTemporalData() {
  for (size_t t=0; t < DataGateway.GetNumInterfaces(); ++t)
     gvStreamData.push_back(DataFactory.GetNewTemporalClusterDataAsTemporalData(DataGateway.GetDataStreamInterface(t)));
}

/** destructor */
MutlipleStreamTemporalData::~MutlipleStreamTemporalData() {}

/** return newly cloned MutlipleStreamTemporalData object */
MutlipleStreamTemporalData * MutlipleStreamTemporalData::Clone() const {
  return new MutlipleStreamTemporalData(*this);
}

/** assigns cluster data of passed object to *this* object
    NOTE: Caller of function is responsible for ensuring that passed object
          is of same class type as *this* object. */
void MutlipleStreamTemporalData::Assign(const AbstractTemporalClusterData& rhs) {
  const MutlipleStreamTemporalData& _rhs = (const MutlipleStreamTemporalData&)rhs;
  gvStreamData = _rhs.gvStreamData;
}

/** not implemented - throws exception */
void MutlipleStreamTemporalData::AddNeighborData(tract_t, const AbtractDataStreamGateway&, size_t) {
  ZdGenerateException("AddNeighbor() not implemeneted.","TemporalData");
}

/** returns number of cases in accumulated respective data streams's cluster data */
count_t MutlipleStreamTemporalData::GetCaseCount(unsigned int iStream) const {
  return gvStreamData[iStream]->gtCases;
}

/** returns expected number of cases in accumulated respective data streams's cluster data */
measure_t MutlipleStreamTemporalData::GetMeasure(unsigned int iStream) const {
  return gvStreamData[iStream]->gtMeasure;
}

/** initializes cluster data in each data stream */
void MutlipleStreamTemporalData::InitializeData() {
  for (gitr=gvStreamData.begin(); gitr != gvStreamData.end(); ++gitr)
     (*gitr)->InitializeData();
}


/** constructor */
MutlipleStreamProspectiveSpatialData::MutlipleStreamProspectiveSpatialData(const ClusterDataFactory& DataFactory, const CSaTScanData & Data, const AbtractDataStreamGateway & DataGateway)
                                     :AbstractMutlipleStreamTemporalData() {
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
MutlipleStreamProspectiveSpatialData::~MutlipleStreamProspectiveSpatialData() {}

/** return newly cloned MutlipleStreamProspectiveSpatialData object */
MutlipleStreamProspectiveSpatialData * MutlipleStreamProspectiveSpatialData::Clone() const {
  return new MutlipleStreamProspectiveSpatialData(*this);
}

/** assigns cluster data of passed object to *this* object
    NOTE: Caller of function is responsible for ensuring that passed object
          is of same class type as *this* object. */
void MutlipleStreamProspectiveSpatialData::Assign(const AbstractTemporalClusterData& rhs) {
  const MutlipleStreamProspectiveSpatialData& _rhs = (const MutlipleStreamProspectiveSpatialData&)rhs;
  gvStreamData = _rhs.gvStreamData;
}

/** adds neighbor data to accumulated cluster data*/
void MutlipleStreamProspectiveSpatialData::AddNeighborData(tract_t tNeighbor, const AbtractDataStreamGateway & DataGateway, size_t) {
  unsigned int i;
  for (i=0, gitr=gvStreamData.begin(); gitr != gvStreamData.end(); ++i, ++gitr)
     (*gitr)->AddNeighborData(tNeighbor, DataGateway, i);
}

/** Calculates loglikelihood ratio given current accumulated cluster data in
    each data stream and adds together.*/
double MutlipleStreamProspectiveSpatialData::CalculateLoglikelihoodRatio(CModel & Model) {
  unsigned int  iWindowEnd, iAllocationSize;
  double        dLoglikelihood, dMaxLoglikelihood=0;

  iAllocationSize = (*gvStreamData.begin())->GetAllocationSize();
  for (gitr=gvStreamData.begin(); gitr != gvStreamData.end(); ++gitr) {
    (*gitr)->gtCases = (*gitr)->gpCases[0];
    (*gitr)->gtMeasure =  (*gitr)->gpMeasure[0];
    if (gfRateOfInterest((*gitr)->gtCases, (*gitr)->gtMeasure, (*gitr)->gtTotalCases, (*gitr)->gtTotalMeasure))
      dLoglikelihood += Model.CalcLogLikelihoodRatio((*gitr)->gtCases, (*gitr)->gtMeasure, (*gitr)->gtTotalCases, (*gitr)->gtTotalMeasure);
  }
  dMaxLoglikelihood = std::max(dMaxLoglikelihood, dLoglikelihood);

  for (iWindowEnd=1; iWindowEnd < iAllocationSize; ++iWindowEnd) {
     dLoglikelihood=0;
     for (gitr=gvStreamData.begin(); gitr != gvStreamData.end(); ++gitr) {
        (*gitr)->gtCases = (*gitr)->gpCases[0] - (*gitr)->gpCases[iWindowEnd];
        (*gitr)->gtMeasure =  (*gitr)->gpMeasure[0] - (*gitr)->gpMeasure[iWindowEnd];
        if (gfRateOfInterest((*gitr)->gtCases, (*gitr)->gtMeasure, (*gitr)->gtTotalCases, (*gitr)->gtTotalMeasure))
         dLoglikelihood += Model.CalcLogLikelihoodRatio((*gitr)->gtCases, (*gitr)->gtMeasure, (*gitr)->gtTotalCases, (*gitr)->gtTotalMeasure);
     }
     dMaxLoglikelihood = std::max(dMaxLoglikelihood, dLoglikelihood);
  }

  return dMaxLoglikelihood;
}

/** returns number of cases in accumulated respective data streams's cluster data */
count_t MutlipleStreamProspectiveSpatialData::GetCaseCount(unsigned int iStream) const {
  return gvStreamData[iStream]->gtCases;
}

/** returns expected number of cases in accumulated respective data streams's cluster data */
measure_t MutlipleStreamProspectiveSpatialData::GetMeasure(unsigned int iStream) const {
  return gvStreamData[iStream]->gtMeasure;
}

/** initializes cluster data in each data stream */
void MutlipleStreamProspectiveSpatialData::InitializeData() {
  for (gitr=gvStreamData.begin(); gitr != gvStreamData.end(); ++gitr)
     (*gitr)->InitializeData();
}



/** constructor */
MutlipleStreamSpaceTimeData::MutlipleStreamSpaceTimeData(const ClusterDataFactory& DataFactory, const AbtractDataStreamGateway & DataGateway)
                            :AbstractMutlipleStreamTemporalData() {
  gvStreamData.resize(DataGateway.GetNumInterfaces(), 0);
  for (size_t t=0; t < DataGateway.GetNumInterfaces(); ++t)
     gvStreamData[t] = DataFactory.GetNewSpaceTimeClusterDataAsTemporalData(DataGateway.GetDataStreamInterface(t));
}

/** destructor */
MutlipleStreamSpaceTimeData::~MutlipleStreamSpaceTimeData() {}

/** return newly cloned MutlipleStreamSpaceTimeData object */
MutlipleStreamSpaceTimeData * MutlipleStreamSpaceTimeData::Clone() const {
  return new MutlipleStreamSpaceTimeData(*this);
}

/** assigns cluster data of passed object to *this* object
    NOTE: Caller of function is responsible for ensuring that passed object
          is of same class type as *this* object. */
void MutlipleStreamSpaceTimeData::Assign(const AbstractTemporalClusterData& rhs) {
  const MutlipleStreamSpaceTimeData& _rhs = (const MutlipleStreamSpaceTimeData&)rhs;
  gvStreamData = _rhs.gvStreamData;
}

/** add neighbor data to accumulation */
void MutlipleStreamSpaceTimeData::AddNeighborData(tract_t tNeighbor, const AbtractDataStreamGateway & DataGateway, size_t) {
  unsigned int i;
  for (i=0, gitr=gvStreamData.begin(); gitr != gvStreamData.end(); ++i, ++gitr)
     (*gitr)->AddNeighborData(tNeighbor, DataGateway, i);
}
 
/** returns number of cases in accumulated respective data streams's cluster data */
count_t MutlipleStreamSpaceTimeData::GetCaseCount(unsigned int iStream) const {
  return gvStreamData[iStream]->gtCases;
}

/** returns expected number of cases in accumulated respective data streams's cluster data */
measure_t MutlipleStreamSpaceTimeData::GetMeasure(unsigned int iStream) const {
  return gvStreamData[iStream]->gtMeasure;
}

/** initializes cluster data in each data stream */
void MutlipleStreamSpaceTimeData::InitializeData() {
  for (gitr=gvStreamData.begin(); gitr != gvStreamData.end(); ++gitr)
     (*gitr)->InitializeData();
}

