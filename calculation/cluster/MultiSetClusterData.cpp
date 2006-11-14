//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "MultiSetClusterData.h"
#include "ClusterDataFactory.h"
#include "LoglikelihoodRatioUnifier.h"

//******************* class MultiSetSpatialData ***********************

/** class constructor */
MultiSetSpatialData::MultiSetSpatialData(const ClusterDataFactory& DataFactory, const AbstractDataSetGateway& DataGateway)
                    :AbstractSpatialClusterData() {
  //Allocate SpatialData object for each data set.
  for (size_t t=0; t < DataGateway.GetNumInterfaces(); ++t)
     gvSetClusterData.push_back(dynamic_cast<SpatialData*>(DataFactory.GetNewSpatialClusterData(DataGateway.GetDataSetInterface(t))));
}

/** Not implemented - throws ZdException. */
void MultiSetSpatialData::AddMeasureList(CMeasureList*, tract_t, const DataSetInterface&) {
  ZdGenerateException("AddMeasureList(CMeasureList*, tract_t, const DataSetInterface&) not implemented.","MultiSetSpatialData");
}

/** Adds neighbor data to accumulation  - caller is responsible for ensuring that
    'tNeighborIndex' is a valid index. */
void MultiSetSpatialData::AddNeighborData(tract_t tNeighborIndex, const AbstractDataSetGateway & DataGateway, size_t) {
  unsigned int                           i=0;
  ptr_vector<SpatialData>::iterator itr=gvSetClusterData.begin();
  for (; itr != gvSetClusterData.end(); ++i, ++itr) (*itr)->AddNeighborData(tNeighborIndex, DataGateway, i);
}

/** Assigns cluster data of passed object to 'this' object. Caller of function
    is responsible for ensuring that passed AbstractSpatialClusterData object
    can be casted to 'MultiSetSpatialData' object. */
void MultiSetSpatialData::Assign(const AbstractSpatialClusterData& rhs) {
  *this = (const MultiSetSpatialData&)rhs;
}

/** Calculates loglikelihood ratio given current accumulated cluster data in
    each data set and adds together. */
double MultiSetSpatialData::CalculateLoglikelihoodRatio(AbstractLikelihoodCalculator& Calculator) {
  unsigned int                           i=0;
  ptr_vector<SpatialData>::iterator itr=gvSetClusterData.begin();
  AbstractLoglikelihoodRatioUnifier    & Unifier = Calculator.GetUnifier();

  Unifier.Reset();
  for (; itr != gvSetClusterData.end(); ++itr, ++i)
     Unifier.AdjoinRatio(Calculator, (*itr)->gtCases, (*itr)->gtMeasure, i);
  return Unifier.GetLoglikelihoodRatio();
}

/** Returns newly cloned MultiSetSpatialData object. Caller responsible for deletion of object. */
MultiSetSpatialData * MultiSetSpatialData::Clone() const {
  return new MultiSetSpatialData(*this);
}

/** Copies class data members that reflect the number of cases per ordinal category,
    which is the data we are interested in for possiblely reporting. */
void MultiSetSpatialData::CopyEssentialClassMembers(const AbstractClusterData& rhs) {
  for (size_t t=0; t < ((const MultiSetSpatialData&)rhs).gvSetClusterData.size(); ++t)
    gvSetClusterData[t]->CopyEssentialClassMembers(*((const MultiSetSpatialData&)rhs).gvSetClusterData[t]);
}

/** Returns number of cases in accumulated respective data sets' cluster data.
    Caller responsible for ensuring that 'tSetIndex' is a valid index. */
count_t MultiSetSpatialData::GetCaseCount(unsigned int tSetIndex) const {
  return gvSetClusterData.at(tSetIndex)->GetCaseCount();
}

/** Fills passed vector with indexes of data sets that contributed to calculated loglikelihood ratio.
    If specified purpose for multiple data sets is multivariate, recalculates high and low
    LLR values to determine which data sets comprised target ratio; else all data sets
    comprised target ratio. */
void MultiSetSpatialData::GetDataSetIndexesComprisedInRatio(double dTargetLoglikelihoodRatio,
                                                            AbstractLikelihoodCalculator& Calculator,
                                                            std::vector<unsigned int>& vDataSetIndexes) const {
  MultivariateUnifier * pUnifier = dynamic_cast<MultivariateUnifier*>(&Calculator.GetUnifier());

  vDataSetIndexes.clear();
  if (pUnifier) {
    std::vector<std::pair<double, double> >             vHighLowRatios(gvSetClusterData.size());
    double                                              dHighRatios=0, dLowRatios=0;

    //for each data set, calculate llr values - possibly scanning for both high and low rates
    for (size_t t=0; t < gvSetClusterData.size(); ++t) {
       pUnifier->GetHighLowRatio(Calculator, gvSetClusterData[t]->gtCases, gvSetClusterData[t]->gtMeasure, t, vHighLowRatios[t]);
       dHighRatios += vHighLowRatios[t].first;
       dLowRatios += vHighLowRatios[t].second;
    }
    //assess collection of llr values to determine which data sets created target ratios
    // - if scanning for high and low ratios, determine which corresponding rate produced
    //   target ratio - high wins if they are equal
    bool        bHighRatios=false;
    if (dHighRatios == dTargetLoglikelihoodRatio)
      bHighRatios = true;
    else if (dLowRatios == dTargetLoglikelihoodRatio)
      bHighRatios = false;
    else {//target ratio does not equal high or low ratios
     //likely round-off error
      if (std::fabs(dHighRatios - dTargetLoglikelihoodRatio) < 0.00000001)
        bHighRatios = true;
      else if (std::fabs(dLowRatios - dTargetLoglikelihoodRatio) < 0.00000001)
        bHighRatios = false;
      else
        ZdGenerateException("Target ratio %lf not found (low=%lf, high=%lf).","MultiSetSpatialData",
                            dTargetLoglikelihoodRatio, dLowRatios, dHighRatios);
    }

    std::vector<std::pair<double, double> >::iterator   itr_pair;
    for (itr_pair=vHighLowRatios.begin(); itr_pair != vHighLowRatios.end(); ++itr_pair)
       if ((bHighRatios ? itr_pair->first : itr_pair->second))
         vDataSetIndexes.push_back(std::distance(vHighLowRatios.begin(), itr_pair));
  }
  else {
    size_t t=0;
    while (t < gvSetClusterData.size()) {vDataSetIndexes.push_back(t); ++t;}
  }
}

/** Implements interface that calculates maximizing value - for multiple data sets, maximizing
    value is the full test statistic/ratio. */
double MultiSetSpatialData::GetMaximizingValue(AbstractLikelihoodCalculator& Calculator) {
  return CalculateLoglikelihoodRatio(Calculator);
}

/** Returns expected number of cases in accumulated respective data sets' cluster data.
    Caller responsible for ensuring that 'tSetIndex' is a valid index. */
measure_t MultiSetSpatialData::GetMeasure(unsigned int tSetIndex) const {
  return gvSetClusterData.at(tSetIndex)->GetMeasure();
}

/** Initializes cluster data in each data set. */
void MultiSetSpatialData::InitializeData() {
  ptr_vector<SpatialData>::iterator itr=gvSetClusterData.begin();
  for (;itr != gvSetClusterData.end(); ++itr) (*itr)->InitializeData();
}

//******************* class AbstractMultiSetTemporalData ***********************

/** Assigns cluster data of passed object to 'this' object. Caller of function
    is responsible for ensuring that passed AbstractTemporalClusterData object
    can be casted to 'MultiSetTemporalData' object. */
void AbstractMultiSetTemporalData::Assign(const AbstractTemporalClusterData& rhs) {
  *this = (const AbstractMultiSetTemporalData&)rhs;
}

/** Copies class data members that reflect the number of cases per ordinal category,
    which is the data we are interested in for possiblely reporting. */
void AbstractMultiSetTemporalData::CopyEssentialClassMembers(const AbstractClusterData& rhs) {
  for (size_t t=0; t < ((const AbstractMultiSetTemporalData&)rhs).gvSetClusterData.size(); ++t)
    gvSetClusterData[t]->CopyEssentialClassMembers(*((const AbstractMultiSetTemporalData&)rhs).gvSetClusterData[t]);
}

/** Returns number of cases in accumulated respective data sets' cluster data.
    Caller is responsible for ensuring that 'tSetIndex' is a valid index. */
count_t AbstractMultiSetTemporalData::GetCaseCount(unsigned int tSetIndex) const {
  return gvSetClusterData.at(tSetIndex)->gtCases;
}

/** Fills passed vector with indexes of data sets that contributed to calculated loglikelihood ratio.
    If specified purpose for multiple data sets is multivariate, recalculates high and low
    LLR values to determine which data sets comprised target ratio; else all data sets
    comprised target ratio. */
void AbstractMultiSetTemporalData::GetDataSetIndexesComprisedInRatio(double dTargetLoglikelihoodRatio,
                                                            AbstractLikelihoodCalculator& Calculator,
                                                            std::vector<unsigned int>& vDataSetIndexes) const {
  MultivariateUnifier * pUnifier = dynamic_cast<MultivariateUnifier*>(&Calculator.GetUnifier());

  vDataSetIndexes.clear();
  if (pUnifier) {
    std::vector<std::pair<double, double> >             vHighLowRatios(gvSetClusterData.size());
    double                                              dHighRatios=0, dLowRatios=0;

    //for each data set, calculate llr values - possibly scanning for both high and low rates
    for (size_t t=0; t < gvSetClusterData.size(); ++t) {
       pUnifier->GetHighLowRatio(Calculator, gvSetClusterData[t]->gtCases, gvSetClusterData[t]->gtMeasure, t, vHighLowRatios[t]);
       dHighRatios += vHighLowRatios[t].first;
       dLowRatios += vHighLowRatios[t].second;
    }
    //assess collection of llr values to determine which data sets created target ratios
    // - if scanning for high and low ratios, determine which corresponding rate produced
    //   target ratio - high wins if they are equal
    bool        bHighRatios=false;
    if (dHighRatios == dTargetLoglikelihoodRatio)
      bHighRatios = true;
    else if (dLowRatios == dTargetLoglikelihoodRatio)
      bHighRatios = false;
    else {//target ratio does not equal high or low ratios
     //likely round-off error
      if (std::fabs(dHighRatios - dTargetLoglikelihoodRatio) < 0.00000001)
        bHighRatios = true;
      else if (std::fabs(dLowRatios - dTargetLoglikelihoodRatio) < 0.00000001)
        bHighRatios = false;
      else
        ZdGenerateException("Target ratio %lf not found (low=%lf, high=%lf).","AbstractMultiSetTemporalData",
                            dTargetLoglikelihoodRatio, dLowRatios, dHighRatios);
    }

    std::vector<std::pair<double, double> >::iterator   itr_pair;
    for (itr_pair=vHighLowRatios.begin(); itr_pair != vHighLowRatios.end(); ++itr_pair)
       if ((bHighRatios ? itr_pair->first : itr_pair->second))
         vDataSetIndexes.push_back(std::distance(vHighLowRatios.begin(), itr_pair));
  }
  else {
    size_t t=0;
    while (t < gvSetClusterData.size()) {vDataSetIndexes.push_back(t); ++t;}
  }
}

/** Returns expected number of cases in accumulated respective data sets' cluster data.
    Caller is responsible for ensuring that 'tSetIndex' is a valid index. */
measure_t AbstractMultiSetTemporalData::GetMeasure(unsigned int tSetIndex) const {
  return gvSetClusterData.at(tSetIndex)->gtMeasure;
}

//********************** class MultiSetTemporalData ****************************

/** class constructor */
MultiSetTemporalData::MultiSetTemporalData(const ClusterDataFactory& DataFactory, const AbstractDataSetGateway& DataGateway)
                     :AbstractMultiSetTemporalData() {
  //Allocate TemporalData object for each data set.
  for (size_t t=0; t < DataGateway.GetNumInterfaces(); ++t)
     gvSetClusterData.push_back(dynamic_cast<TemporalData*>(DataFactory.GetNewTemporalClusterData(DataGateway.GetDataSetInterface(t))));
}

/** Not implemented - throws ZdException. */
void MultiSetTemporalData::AddNeighborData(tract_t, const AbstractDataSetGateway&, size_t) {
  ZdGenerateException("AddNeighbor(tract_t, const AbstractDataSetGateway&, size_t) not implemeneted.","MultiSetTemporalData");
}

/** Returns newly cloned MultiSetTemporalData object. Caller responsible for
    deletion of object. */
MultiSetTemporalData * MultiSetTemporalData::Clone() const {
  return new MultiSetTemporalData(*this);
}

/** Initializes cluster data in each data set. */
void MultiSetTemporalData::InitializeData() {
  ptr_vector<TemporalData>::iterator itr=gvSetClusterData.begin();
  for (;itr != gvSetClusterData.end(); ++itr) (*itr)->InitializeData();
}

/** Reassociates internal data with passed DataSetInterface pointers. */
void MultiSetTemporalData::Reassociate(const DataSetInterface& Interface) {
  ptr_vector<TemporalData>::iterator itr=gvSetClusterData.begin();
  for (;itr != gvSetClusterData.end(); ++itr) (*itr)->Reassociate(Interface);
}

/** Reassociates internal data with passed DataSetInterface pointers of DataGateway. */
void MultiSetTemporalData::Reassociate(const AbstractDataSetGateway& DataGateway) {
  ptr_vector<TemporalData>::iterator itr=gvSetClusterData.begin();
  for (; itr != gvSetClusterData.end(); ++itr) (*itr)->Reassociate(DataGateway);
}

//*************** class MultiSetProspectiveSpatialData *************************

/** class constructor */
MultiSetProspectiveSpatialData::MultiSetProspectiveSpatialData(const ClusterDataFactory& DataFactory, const CSaTScanData& Data, const AbstractDataSetGateway& DataGateway)
                               :AbstractMultiSetTemporalData() {
  //Allocate ProspectiveSpatialData object for each data set.
  for (size_t t=0; t < DataGateway.GetNumInterfaces(); ++t)
     gvSetClusterData.push_back(dynamic_cast<TemporalData*>(DataFactory.GetNewProspectiveSpatialClusterData(Data, DataGateway.GetDataSetInterface(t))));
}

/** Adds neighbor data to accumulated cluster data  - caller is responsible for
    ensuring that 'tNeighborIndex' is a valid index. */
void MultiSetProspectiveSpatialData::AddNeighborData(tract_t tNeighborIndex, const AbstractDataSetGateway& DataGateway, size_t) {
  assert(geEvaluationAssistDataStatus == Allocated);
  unsigned int                            i=0;
  ptr_vector<TemporalData>::iterator itr=gvSetClusterData.begin();
  for (;itr != gvSetClusterData.end(); ++i, ++itr) (*itr)->AddNeighborData(tNeighborIndex, DataGateway, i);
}

/** Calculates loglikelihood ratio given current accumulated cluster data in
    each data set and adds together.*/
double MultiSetProspectiveSpatialData::CalculateLoglikelihoodRatio(AbstractLikelihoodCalculator& Calculator) {
  assert(geEvaluationAssistDataStatus == Allocated);
  unsigned int                            t, iWindowEnd, iAllocationSize;
  double                                  dMaxLoglikelihoodRatio=0;
  AbstractLoglikelihoodRatioUnifier     & Unifier = Calculator.GetUnifier();
  ptr_vector<TemporalData>::iterator itr=gvSetClusterData.begin();

  Unifier.Reset();
  iAllocationSize = (*gvSetClusterData.begin())->GetAllocationSize();
  for (t=0;itr != gvSetClusterData.end(); ++itr, ++t)
     Unifier.AdjoinRatio(Calculator, (*itr)->gpCases[0], (*itr)->gpMeasure[0], t);
  dMaxLoglikelihoodRatio = Unifier.GetLoglikelihoodRatio();

  for (iWindowEnd=1; iWindowEnd < iAllocationSize; ++iWindowEnd) {
     Unifier.Reset();
     for (t=0, itr=gvSetClusterData.begin(); itr != gvSetClusterData.end(); ++itr, ++t) {
        (*itr)->gtCases = (*itr)->gpCases[0] - (*itr)->gpCases[iWindowEnd];
        (*itr)->gtMeasure =  (*itr)->gpMeasure[0] - (*itr)->gpMeasure[iWindowEnd];
        Unifier.AdjoinRatio(Calculator, (*itr)->gtCases, (*itr)->gtMeasure, t);
     }
     dMaxLoglikelihoodRatio = std::max(dMaxLoglikelihoodRatio, Unifier.GetLoglikelihoodRatio());
  }

  return dMaxLoglikelihoodRatio;
}

/** Returns newly cloned MultiSetProspectiveSpatialData object. Caller responsible
    for deletion of object. */
MultiSetProspectiveSpatialData * MultiSetProspectiveSpatialData::Clone() const {
  return new MultiSetProspectiveSpatialData(*this);
}

/** Deallocates data members that assist with evaluation of temporal data.
    Once this function is called various class member functions become invalid
    and an assertion will fail if called. */
void MultiSetProspectiveSpatialData::DeallocateEvaluationAssistClassMembers() {
  unsigned int                            i=0;
  ptr_vector<TemporalData>::iterator itr=gvSetClusterData.begin();
  for (;itr != gvSetClusterData.end(); ++i, ++itr) (*itr)->DeallocateEvaluationAssistClassMembers();
  geEvaluationAssistDataStatus = Deallocated;
}

/** Initializes cluster data in each data set. */
void MultiSetProspectiveSpatialData::InitializeData() {
  assert(geEvaluationAssistDataStatus == Allocated);
  ptr_vector<TemporalData>::iterator itr=gvSetClusterData.begin();
  for (;itr != gvSetClusterData.end(); ++itr) (*itr)->InitializeData();
}

/** Implements interface that calculates maximizing value - for multiple data sets, maximizing
    value is the full test statistic/ratio. */
double MultiSetProspectiveSpatialData::GetMaximizingValue(AbstractLikelihoodCalculator& Calculator) {
  return CalculateLoglikelihoodRatio(Calculator);
}
//********************** class MultiSetSpaceTimeData ***************************

/** class constructor */
MultiSetSpaceTimeData::MultiSetSpaceTimeData(const ClusterDataFactory& DataFactory, const AbstractDataSetGateway& DataGateway)
                      :AbstractMultiSetTemporalData(), geEvaluationAssistDataStatus(Allocated) {
  for (size_t t=0; t < DataGateway.GetNumInterfaces(); ++t)
     gvSetClusterData.push_back(dynamic_cast<TemporalData*>(DataFactory.GetNewSpaceTimeClusterData(DataGateway.GetDataSetInterface(t))));
}

/** Add neighbor data to accumulation  - caller is responsible for
    ensuring that 'tNeighborIndex' is a valid index. */
void MultiSetSpaceTimeData::AddNeighborData(tract_t tNeighborIndex, const AbstractDataSetGateway& DataGateway, size_t) {
  assert(geEvaluationAssistDataStatus == Allocated);
  unsigned int                            i=0;
  ptr_vector<TemporalData>::iterator itr=gvSetClusterData.begin();
  for (;itr != gvSetClusterData.end(); ++i, ++itr) (*itr)->AddNeighborData(tNeighborIndex, DataGateway, i);
}

/** Returns newly cloned MultiSetSpaceTimeData object. Caller responsible for deletion
    of object. */
MultiSetSpaceTimeData * MultiSetSpaceTimeData::Clone() const {
  return new MultiSetSpaceTimeData(*this);
}

/** Deallocates data members that assist with evaluation of temporal data.
    Once this function is called various class member functions become invalid
    and an assertion will fail if called. */
void MultiSetSpaceTimeData::DeallocateEvaluationAssistClassMembers() {
  unsigned int                            i=0;
  ptr_vector<TemporalData>::iterator itr=gvSetClusterData.begin();
  for (;itr != gvSetClusterData.end(); ++i, ++itr)
     (*itr)->DeallocateEvaluationAssistClassMembers();
  geEvaluationAssistDataStatus = Deallocated;
}

/** Initializes cluster data in each data set. */
void MultiSetSpaceTimeData::InitializeData() {
  assert(geEvaluationAssistDataStatus == Allocated);
  ptr_vector<TemporalData>::iterator itr=gvSetClusterData.begin();
  for (;itr != gvSetClusterData.end(); ++itr) (*itr)->InitializeData();
}

