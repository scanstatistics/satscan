//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "SaTScanData.h"
#include "UniformTimeClusterData.h"
#include "SSException.h"


//

count_t * UniformTimeClusterDataInterface::getCasesArray() const {
    throw prg_error("Not implemented.", "UniformTimeClusterDataInterface");
}

measure_t * UniformTimeClusterDataInterface::getMeasureArray() const {
    throw prg_error("Not implemented.", "UniformTimeClusterDataInterface");
}

count_t UniformTimeClusterDataInterface::getCases() const {
    throw prg_error("Not implemented.", "UniformTimeClusterDataInterface");
}

void UniformTimeClusterDataInterface::setCases(count_t c) {
    throw prg_error("Not implemented.", "UniformTimeClusterDataInterface");
}

measure_t UniformTimeClusterDataInterface::getMeasure() const {
    throw prg_error("Not implemented.", "UniformTimeClusterDataInterface");
}

void UniformTimeClusterDataInterface::setMeasure(measure_t m) {
    throw prg_error("Not implemented.", "UniformTimeClusterDataInterface");
}


//************** class UniformTimeTemporalData **************************************

/** class constructor */
UniformTimeTemporalData::UniformTimeTemporalData() : TemporalData() {}

/** class constructor */
UniformTimeTemporalData::UniformTimeTemporalData(const DataSetInterface& Interface) :TemporalData(Interface) {
	gtCasesInPeriod = Interface.GetTotalCasesCount();
	gtMeasureInPeriod = Interface.GetTotalMeasureCount();
}

/** class constructor */
UniformTimeTemporalData::UniformTimeTemporalData(const AbstractDataSetGateway& DataGateway)
                   :TemporalData(DataGateway.GetDataSetInterface()) {
	gtCasesInPeriod = DataGateway.GetDataSetInterface().GetTotalCasesCount();
	gtMeasureInPeriod = DataGateway.GetDataSetInterface().GetTotalMeasureCount();
}

/** Assigns cluster data of passed object to 'this' object. Caller of function
    is responsible for ensuring that passed AbstractTemporalClusterData object
    can be casted to 'UniformTimeTemporalData' object. */
void UniformTimeTemporalData::Assign(const AbstractTemporalClusterData& rhs) {
  *this = (const UniformTimeTemporalData&)rhs;
}

/** Returns newly cloned UniformTimeTemporalData object. Caller responsible for deletion of object. */
UniformTimeTemporalData * UniformTimeTemporalData::Clone() const {
  return new UniformTimeTemporalData(*this);
}

/** Copies class data members that reflect the number of cases, expected, and expected
    squared values, which is the data we are interested in for possiblely reporting. */
void UniformTimeTemporalData::CopyEssentialClassMembers(const AbstractClusterData& rhs) {
    TemporalData::CopyEssentialClassMembers(rhs);
    gtCasesInPeriod = ((const UniformTimeTemporalData&)rhs).gtCasesInPeriod;
    gtMeasureInPeriod = ((const UniformTimeTemporalData&)rhs).gtMeasureInPeriod;
}

/** Reassociates internal data with passed DataSetInterface pointers. Not implemented - throws exception */
void UniformTimeTemporalData::Reassociate(const DataSetInterface& Interface) {
  throw prg_error("Reassociate(const DataSetInterface&) not implemented.","UniformTimeTemporalData");
}

/** Reassociates internal data with passed DataSetInterface pointers of DataGateway. */
void UniformTimeTemporalData::Reassociate(const AbstractDataSetGateway& DataGateway) {
  TemporalData::Reassociate(DataGateway.GetDataSetInterface());
}

//*************** class UniformTimeSpaceTimeData ************************************

/** class constructor */
UniformTimeSpaceTimeData::UniformTimeSpaceTimeData(const DataSetInterface& Interface):SpaceTimeData(Interface) {}

/** constructor */
UniformTimeSpaceTimeData::UniformTimeSpaceTimeData(const AbstractDataSetGateway& DataGateway):SpaceTimeData(DataGateway) {}

/** class copy constructor */
UniformTimeSpaceTimeData::UniformTimeSpaceTimeData(const UniformTimeSpaceTimeData& rhs) :SpaceTimeData(rhs) {
  try {
    *this = (const UniformTimeSpaceTimeData&)rhs;
  }
  catch (prg_exception& x) {
    x.addTrace("constructor(const UniformTimeSpaceTimeData&)","UniformTimeSpaceTimeData");
    throw;
  }
}

/** class destructor */
UniformTimeSpaceTimeData::~UniformTimeSpaceTimeData() {}

/** Assigns cluster data of passed object to 'this' object. Caller of function
    is responsible for ensuring that passed AbstractTemporalClusterData object
    can be casted to 'UniformTimeSpaceTimeData' object. */
void UniformTimeSpaceTimeData::Assign(const AbstractTemporalClusterData& rhs) {
  *this = (const UniformTimeSpaceTimeData&)rhs;
}

/** Returns newly cloned UniformTimeSpaceTimeData object. Caller responsible for deletion of object. */
UniformTimeSpaceTimeData * UniformTimeSpaceTimeData::Clone() const {
   return new UniformTimeSpaceTimeData(*this);
}

/** re-initialize data */
void UniformTimeSpaceTimeData::InitializeData() {
    SpaceTimeData::InitializeData();
    gtCasesInPeriod=0;
    gtMeasureInPeriod=0;
}

/** overloaded assignement operator */
UniformTimeSpaceTimeData & UniformTimeSpaceTimeData::operator=(const UniformTimeSpaceTimeData& rhs) {
    SpaceTimeData::operator=(rhs);
    gtCasesInPeriod = rhs.gtCasesInPeriod;
    gtMeasureInPeriod = rhs.gtMeasureInPeriod;
    return *this;
}

/** Copies class data members that reflect the number of cases, expected, and expected
squared values, which is the data we are interested in for possiblely reporting. */
void UniformTimeSpaceTimeData::CopyEssentialClassMembers(const AbstractClusterData& rhs) {
    SpaceTimeData::CopyEssentialClassMembers (rhs);
    gtCasesInPeriod = ((const UniformTimeSpaceTimeData&)rhs).gtCasesInPeriod;
    gtMeasureInPeriod = ((const UniformTimeSpaceTimeData&)rhs).gtMeasureInPeriod;
}

void UniformTimeSpaceTimeData::AddNeighborDataSupplement(tract_t tNeighborIndex, const AbstractDataSetGateway & DataGateway, size_t tSetIndex) {
	AddNeighborDataSupplement(tNeighborIndex, DataGateway.GetDataSetInterface(tSetIndex));
}

void UniformTimeSpaceTimeData::AddNeighborDataSupplement(tract_t tNeighborIndex, const DataSetInterface & Interface) {
	gtCasesInPeriod += Interface.GetCaseArray()[0][tNeighborIndex];
	gtMeasureInPeriod += Interface.GetMeasureArray()[0][tNeighborIndex];
}

void UniformTimeSpaceTimeData::AddNeighborData(tract_t tNeighborIndex, const AbstractDataSetGateway & DataGateway, size_t tSetIndex) {
	SpaceTimeData::AddNeighborData(tNeighborIndex, DataGateway, tSetIndex);
	AddNeighborDataSupplement(tNeighborIndex, DataGateway, tSetIndex);
}