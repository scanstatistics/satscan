//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "MultiSetNormalClusterData.h"
#include "NormalClusterDataFactory.h"
#include "LoglikelihoodRatioUnifier.h"
#include "SSException.h"

//***************** class MultiSetNormalSpatialData ****************************

/** class constructor */
MultiSetNormalSpatialData::MultiSetNormalSpatialData(const NormalClusterDataFactory& DataFactory, const AbstractDataSetGateway& DataGateway)
                                  :AbstractSpatialClusterData(), AbstractNormalClusterData() {
  for (size_t t=0; t < DataGateway.GetNumInterfaces(); ++t)
     gvSetClusterData.push_back(dynamic_cast<NormalSpatialData*>(DataFactory.GetNewSpatialClusterData(DataGateway.GetDataSetInterface(t))));
}

/** Adds neighbor data to accumulation  - caller is responsible for ensuring that
    'tNeighborIndex' is a valid index. */
void MultiSetNormalSpatialData::AddNeighborData(tract_t tNeighborIndex, const AbstractDataSetGateway& DataGateway, size_t) {
  unsigned int                                        i=0;
  ptr_vector<NormalSpatialData>::iterator   itr=gvSetClusterData.begin();
  for (;itr != gvSetClusterData.end(); ++i, ++itr) (*itr)->AddNeighborData(tNeighborIndex, DataGateway, i);
}

/** Assigns cluster data of passed object to 'this' object. Caller of function
    is responsible for ensuring that passed AbstractSpatialClusterData object
    can be casted to 'MultiSetCategoricalSpatialData' object. */
void MultiSetNormalSpatialData::Assign(const AbstractSpatialClusterData& rhs) {
  *this = (const MultiSetNormalSpatialData&)rhs;
}

const AbstractLoglikelihoodRatioUnifier & MultiSetNormalSpatialData::getRatioUnified(AbstractLikelihoodCalculator& Calculator) const {
    ptr_vector<NormalSpatialData>::const_iterator itr=gvSetClusterData.begin();
    AbstractLoglikelihoodRatioUnifier & Unifier = Calculator.GetUnifier();

    Unifier.Reset();
    for (size_t t=0; itr != gvSetClusterData.end(); ++itr, ++t)
        Unifier.AdjoinRatio(Calculator, (*itr)->gtCases, (*itr)->gtMeasure, (*itr)->gtMeasureAux, t);
    return Unifier;
}

/** Calculates loglikelihood ratio given current accumulated cluster data in
    each data set and adds together. */
double MultiSetNormalSpatialData::CalculateLoglikelihoodRatio(AbstractLikelihoodCalculator& Calculator) {
  return getRatioUnified(Calculator).GetLoglikelihoodRatio();
}

/** Returns newly cloned MultiSetCategoricalSpatialData object. Caller is
    responsible for deletion of object. */
MultiSetNormalSpatialData * MultiSetNormalSpatialData::Clone() const {
  return new MultiSetNormalSpatialData(*this);
}

/** Copies class data members that reflect the number of cases per ordinal category,
    which is the data we are interested in for possiblely reporting. */
void MultiSetNormalSpatialData::CopyEssentialClassMembers(const AbstractClusterData& rhs) {
  for (size_t t=0; t < ((const MultiSetNormalSpatialData&)rhs).gvSetClusterData.size(); ++t)
    gvSetClusterData[t]->CopyEssentialClassMembers(*((const MultiSetNormalSpatialData&)rhs).gvSetClusterData[t]);
}

/** Not implemented - throws prg_error. */
count_t MultiSetNormalSpatialData::GetCaseCount(unsigned int tSetIndex) const {
  return gvSetClusterData.at(tSetIndex)->GetCaseCount();
}

void MultiSetNormalSpatialData::setCaseCount(count_t t, unsigned int tSetIndex) {
  gvSetClusterData.at(tSetIndex)->setCaseCount(t);
}

/** Fills passed vector with indexes of data sets that contributed to calculated loglikelihood ratio.
    If specified purpose for multiple data sets is multivariate, recalculates high and low
    LLR values to determine which data sets comprised target ratio; else all data sets
    comprised target ratio. */
void MultiSetNormalSpatialData::GetDataSetIndexesComprisedInRatio(double dTargetLoglikelihoodRatio,
                                                                  AbstractLikelihoodCalculator& Calculator,
                                                                  std::vector<unsigned int>& vDataSetIndexes) const {

    AbstractMultivariateUnifier * pUnifier = dynamic_cast<AbstractMultivariateUnifier*>(&Calculator.GetUnifier());

  vDataSetIndexes.clear();
  if (pUnifier) {
    std::vector<std::pair<double, double> >             vHighLowRatios(gvSetClusterData.size());
    double                                              dHighRatios=0, dLowRatios=0;

    //for each data set, calculate llr values - possibly scanning for both high and low rates
    for (size_t t=0; t < gvSetClusterData.size(); ++t) {
       pUnifier->GetHighLowRatio(Calculator, gvSetClusterData[t]->gtCases, gvSetClusterData[t]->gtMeasure, gvSetClusterData[t]->gtMeasureAux, t, vHighLowRatios[t]);
       dHighRatios += vHighLowRatios[t].first;
       dLowRatios += vHighLowRatios[t].second;
    }
    //assess collected llr values to determine which data sets created target ratios
    // - if scanning for high and low ratios, determine which corresponding rate produced
    //   target ratio - high wins if they are equal
    bool bHighRatios=false;
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
        throw prg_error("Target ratio %lf not found (low=%lf, high=%lf).","MultiSetCategoricalSpatialData",
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

/** Implements interface that calculates maximizing value - for multiple data set, maximizing
    value is the full test statistic/ratio. */
double MultiSetNormalSpatialData::GetMaximizingValue(AbstractLikelihoodCalculator& Calculator) {
  return CalculateLoglikelihoodRatio(Calculator);
}

measure_t MultiSetNormalSpatialData::GetMeasure(unsigned int tSetIndex) const {
  return gvSetClusterData.at(tSetIndex)->GetMeasure();
}

void MultiSetNormalSpatialData::setMeasure(measure_t m, unsigned int tSetIndex) {
  gvSetClusterData.at(tSetIndex)->setMeasure(m);
}

measure_t MultiSetNormalSpatialData::GetMeasureAux(unsigned int tSetIndex) const {
  return gvSetClusterData.at(tSetIndex)->GetMeasureAux();
}

/** Initializes cluster data in each data set. */
void MultiSetNormalSpatialData::InitializeData() {
  ptr_vector<NormalSpatialData>::iterator   itr=gvSetClusterData.begin();
  for (;itr != gvSetClusterData.end(); ++itr) (*itr)->InitializeData();
}

//******************* class AbstractMultiSetNormalTemporalData *****************

/** Assigns cluster data of passed object to 'this' object. Caller of function
    is responsible for ensuring that passed AbstractTemporalClusterData object
    can be casted to 'MultiSetTemporalData' object. */
void AbstractMultiSetNormalTemporalData::Assign(const AbstractTemporalClusterData& rhs) {
  *this = (const AbstractMultiSetNormalTemporalData&)rhs;
}

/** Copies class data members that reflect the number of cases per ordinal category,
    which is the data we are interested in for possiblely reporting. */
void AbstractMultiSetNormalTemporalData::CopyEssentialClassMembers(const AbstractClusterData& rhs) {
  for (size_t t=0; t < ((const AbstractMultiSetNormalTemporalData&)rhs).gvSetClusterData.size(); ++t)
    gvSetClusterData[t]->CopyEssentialClassMembers(*((const AbstractMultiSetNormalTemporalData&)rhs).gvSetClusterData[t]);
}

/** Returns number of cases in accumulated respective data sets' cluster data.
    Caller is responsible for ensuring that 'tSetIndex' is a valid index. */
count_t AbstractMultiSetNormalTemporalData::GetCaseCount(unsigned int tSetIndex) const {
  return gvSetClusterData.at(tSetIndex)->gtCases;
}

void AbstractMultiSetNormalTemporalData::setCaseCount(count_t t, unsigned int tSetIndex) {
  gvSetClusterData.at(tSetIndex)->setCaseCount(t);
}

const AbstractLoglikelihoodRatioUnifier & AbstractMultiSetNormalTemporalData::getRatioUnified(AbstractLikelihoodCalculator& Calculator) const {
    AbstractLoglikelihoodRatioUnifier & Unifier = Calculator.GetUnifier();

    Unifier.Reset();
    for (size_t t=0; t < gvSetClusterData.size(); ++t) {
        Unifier.AdjoinRatio(Calculator, gvSetClusterData[t]->gtCases, gvSetClusterData[t]->gtMeasure, gvSetClusterData[t]->gtMeasureAux, t);
    }
    return Unifier;
}

/** Fills passed vector with indexes of data sets that contributed to calculated loglikelihood ratio.
    If specified purpose for multiple data sets is multivariate, recalculates high and low
    LLR values to determine which data sets comprised target ratio; else all data sets
    comprised target ratio. */
void AbstractMultiSetNormalTemporalData::GetDataSetIndexesComprisedInRatio(double dTargetLoglikelihoodRatio,
                                                                           AbstractLikelihoodCalculator& Calculator,
                                                                           std::vector<unsigned int>& vDataSetIndexes) const {
    AbstractMultivariateUnifier * pUnifier = dynamic_cast<AbstractMultivariateUnifier*>(&Calculator.GetUnifier());

  vDataSetIndexes.clear();
  if (pUnifier) {
    std::vector<std::pair<double, double> >             vHighLowRatios(gvSetClusterData.size());
    double                                              dHighRatios=0, dLowRatios=0;

    //for each data set, calculate llr values - possibly scanning for both high and low rates
    for (size_t t=0; t < gvSetClusterData.size(); ++t) {
       pUnifier->GetHighLowRatio(Calculator, gvSetClusterData[t]->gtCases, gvSetClusterData[t]->gtMeasure, gvSetClusterData[t]->gtMeasureAux, t, vHighLowRatios[t]);
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
        throw prg_error("Target ratio %lf not found (low=%lf, high=%lf).","AbstractMultiSetTemporalData",
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
measure_t AbstractMultiSetNormalTemporalData::GetMeasure(unsigned int tSetIndex) const {
  return gvSetClusterData.at(tSetIndex)->gtMeasure;
}

void AbstractMultiSetNormalTemporalData::setMeasure(measure_t m, unsigned int tSetIndex) {
  gvSetClusterData.at(tSetIndex)->setMeasure(m);
}

/** Returns expected number of cases in accumulated respective data sets' cluster data.
    Caller is responsible for ensuring that 'tSetIndex' is a valid index. */
measure_t AbstractMultiSetNormalTemporalData::GetMeasureAux(unsigned int tSetIndex) const {
  return gvSetClusterData.at(tSetIndex)->gtMeasureAux;
}

//********************** class MultiSetNormalTemporalData ****************************

/** class constructor */
MultiSetNormalTemporalData::MultiSetNormalTemporalData(const NormalClusterDataFactory& DataFactory, const AbstractDataSetGateway& DataGateway)
                           :AbstractMultiSetNormalTemporalData() {
  //Allocate TemporalData object for each data set.
  for (size_t t=0; t < DataGateway.GetNumInterfaces(); ++t)
     gvSetClusterData.push_back(dynamic_cast<NormalTemporalData*>(DataFactory.GetNewTemporalClusterData(DataGateway.GetDataSetInterface(t))));
}

/** Not implemented - throws prg_error. */
void MultiSetNormalTemporalData::AddNeighborData(tract_t, const AbstractDataSetGateway&, size_t) {
  throw prg_error("AddNeighbor(tract_t, const AbstractDataSetGateway&, size_t) not implemeneted.","MultiSetNormalTemporalData");
}

/** Returns newly cloned MultiSetTemporalData object. Caller responsible for
    deletion of object. */
MultiSetNormalTemporalData * MultiSetNormalTemporalData::Clone() const {
  return new MultiSetNormalTemporalData(*this);
}

/** Initializes cluster data in each data set. */
void MultiSetNormalTemporalData::InitializeData() {
  ptr_vector<NormalTemporalData>::iterator itr=gvSetClusterData.begin();
  for (;itr != gvSetClusterData.end(); ++itr) (*itr)->InitializeData();
}

/** Reassociates internal data with passed DataSetInterface pointers. */
void MultiSetNormalTemporalData::Reassociate(const DataSetInterface& Interface) {
  ptr_vector<NormalTemporalData>::iterator itr=gvSetClusterData.begin();
  for (;itr != gvSetClusterData.end(); ++itr) (*itr)->Reassociate(Interface);
}

/** Reassociates internal data with passed DataSetInterface pointers of DataGateway. */
void MultiSetNormalTemporalData::Reassociate(const AbstractDataSetGateway& DataGateway) {
  ptr_vector<NormalTemporalData>::iterator itr=gvSetClusterData.begin();
  for (; itr != gvSetClusterData.end(); ++itr) (*itr)->Reassociate(DataGateway);
}


//*************** class MultiSetNormalProspectiveSpatialData *******************

/** class constructor */
MultiSetNormalProspectiveSpatialData::MultiSetNormalProspectiveSpatialData(const NormalClusterDataFactory& DataFactory, const CSaTScanData& Data, const AbstractDataSetGateway& DataGateway)
                                     :AbstractMultiSetNormalTemporalData() {
  //Allocate ProspectiveSpatialData object for each data set.
  for (size_t t=0; t < DataGateway.GetNumInterfaces(); ++t)
     gvSetClusterData.push_back(dynamic_cast<NormalTemporalData*>(DataFactory.GetNewProspectiveSpatialClusterData(Data, DataGateway.GetDataSetInterface(t))));
}

/** Adds neighbor data to accumulated cluster data  - caller is responsible for
    ensuring that 'tNeighborIndex' is a valid index. */
void MultiSetNormalProspectiveSpatialData::AddNeighborData(tract_t tNeighborIndex, const AbstractDataSetGateway& DataGateway, size_t) {
  assert(geEvaluationAssistDataStatus == Allocated);
  unsigned int                            i=0;
  ptr_vector<NormalTemporalData>::iterator itr=gvSetClusterData.begin();
  for (;itr != gvSetClusterData.end(); ++i, ++itr) (*itr)->AddNeighborData(tNeighborIndex, DataGateway, i);
}

const AbstractLoglikelihoodRatioUnifier & MultiSetNormalProspectiveSpatialData::getRatioUnified(AbstractLikelihoodCalculator& Calculator) const {
    assert(geEvaluationAssistDataStatus == Allocated);
    unsigned int                            t, iWindowEnd, iAllocationSize;
    double                                  dMaxLoglikelihoodRatio=0;
    AbstractLoglikelihoodRatioUnifier     & Unifier = Calculator.GetUnifier();
    ptr_vector<NormalTemporalData>::const_iterator itr=gvSetClusterData.begin();

    Unifier.Reset();
    iAllocationSize = (*gvSetClusterData.begin())->GetAllocationSize();
    for (t=0;itr != gvSetClusterData.end(); ++itr, ++t)
        Unifier.AdjoinRatio(Calculator, (*itr)->gpCases[0], (*itr)->gpMeasure[0], (*itr)->gpMeasureAux[0], t);
    dMaxLoglikelihoodRatio = Unifier.GetLoglikelihoodRatio();

    std::auto_ptr<AbstractLoglikelihoodRatioUnifier> prospectiveUnifier(Unifier.Clone());
    for (iWindowEnd=1; iWindowEnd < iAllocationSize; ++iWindowEnd) {
        prospectiveUnifier->Reset();
        for (t=0, itr=gvSetClusterData.begin(); itr != gvSetClusterData.end(); ++itr, ++t) {
            (*itr)->gtCases = (*itr)->gpCases[0] - (*itr)->gpCases[iWindowEnd];
            (*itr)->gtMeasure =  (*itr)->gpMeasure[0] - (*itr)->gpMeasure[iWindowEnd];
            (*itr)->gtMeasureAux =  (*itr)->gpMeasureAux[0] - (*itr)->gpMeasureAux[iWindowEnd];
            prospectiveUnifier->AdjoinRatio(Calculator, (*itr)->gtCases, (*itr)->gtMeasure, (*itr)->gtMeasureAux, t);
        }
        dMaxLoglikelihoodRatio = std::max(dMaxLoglikelihoodRatio, prospectiveUnifier->GetLoglikelihoodRatio());
    }
    // reset calculators unifier if largest LLR came from prospective process.
    if (dMaxLoglikelihoodRatio != Unifier.GetLoglikelihoodRatio()) {
        Unifier = *prospectiveUnifier;
    }
    return Unifier;
}

/** Calculates loglikelihood ratio given current accumulated cluster data in
    each data set and adds together.*/
double MultiSetNormalProspectiveSpatialData::CalculateLoglikelihoodRatio(AbstractLikelihoodCalculator& Calculator) {
    return getRatioUnified(Calculator).GetLoglikelihoodRatio();
}

/** Returns newly cloned MultiSetProspectiveSpatialData object. Caller responsible
    for deletion of object. */
MultiSetNormalProspectiveSpatialData * MultiSetNormalProspectiveSpatialData::Clone() const {
  return new MultiSetNormalProspectiveSpatialData(*this);
}

/** Deallocates data members that assist with evaluation of temporal data.
    Once this function is called various class member functions become invalid
    and an assertion will fail if called. */
void MultiSetNormalProspectiveSpatialData::DeallocateEvaluationAssistClassMembers() {
  unsigned int                            i=0;
  ptr_vector<NormalTemporalData>::iterator itr=gvSetClusterData.begin();
  for (;itr != gvSetClusterData.end(); ++i, ++itr) (*itr)->DeallocateEvaluationAssistClassMembers();
  geEvaluationAssistDataStatus = Deallocated;
}

/** Implements interface that calculates maximizing value - for multiple data set, maximizing
    value is the full test statistic/ratio. */
double MultiSetNormalProspectiveSpatialData::GetMaximizingValue(AbstractLikelihoodCalculator& Calculator) {
  return CalculateLoglikelihoodRatio(Calculator);
}

/** Initializes cluster data in each data set. */
void MultiSetNormalProspectiveSpatialData::InitializeData() {
  assert(geEvaluationAssistDataStatus == Allocated);
  ptr_vector<NormalTemporalData>::iterator itr=gvSetClusterData.begin();
  for (;itr != gvSetClusterData.end(); ++itr) (*itr)->InitializeData();
}

//********************** class MultiSetNormalSpaceTimeData *********************

/** class constructor */
MultiSetNormalSpaceTimeData::MultiSetNormalSpaceTimeData(const NormalClusterDataFactory& DataFactory, const AbstractDataSetGateway& DataGateway)
                            :AbstractMultiSetNormalTemporalData(), geEvaluationAssistDataStatus(Allocated) {
  for (size_t t=0; t < DataGateway.GetNumInterfaces(); ++t)
     gvSetClusterData.push_back(dynamic_cast<NormalTemporalData*>(DataFactory.GetNewSpaceTimeClusterData(DataGateway.GetDataSetInterface(t))));
}

/** Add neighbor data to accumulation  - caller is responsible for
    ensuring that 'tNeighborIndex' is a valid index. */
void MultiSetNormalSpaceTimeData::AddNeighborData(tract_t tNeighborIndex, const AbstractDataSetGateway& DataGateway, size_t) {
  assert(geEvaluationAssistDataStatus == Allocated);
  unsigned int                            i=0;
  ptr_vector<NormalTemporalData>::iterator itr=gvSetClusterData.begin();
  for (;itr != gvSetClusterData.end(); ++i, ++itr) (*itr)->AddNeighborData(tNeighborIndex, DataGateway, i);
}

/** Returns newly cloned MultiSetSpaceTimeData object. Caller responsible for deletion
    of object. */
MultiSetNormalSpaceTimeData * MultiSetNormalSpaceTimeData::Clone() const {
  return new MultiSetNormalSpaceTimeData(*this);
}

/** Deallocates data members that assist with evaluation of temporal data.
    Once this function is called various class member functions become invalid
    and an assertion will fail if called. */
void MultiSetNormalSpaceTimeData::DeallocateEvaluationAssistClassMembers() {
  unsigned int                            i=0;
  ptr_vector<NormalTemporalData>::iterator itr=gvSetClusterData.begin();
  for (;itr != gvSetClusterData.end(); ++i, ++itr)
     (*itr)->DeallocateEvaluationAssistClassMembers();
  geEvaluationAssistDataStatus = Deallocated;
}

/** Initializes cluster data in each data set. */
void MultiSetNormalSpaceTimeData::InitializeData() {
  assert(geEvaluationAssistDataStatus == Allocated);
  ptr_vector<NormalTemporalData>::iterator itr=gvSetClusterData.begin();
  for (;itr != gvSetClusterData.end(); ++itr) (*itr)->InitializeData();
}

