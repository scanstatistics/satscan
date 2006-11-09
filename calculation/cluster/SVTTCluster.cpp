//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "SVTTCluster.h"

///////////////////////////// SVTTClusterData //////////////////////////////////

/** constructor */
SVTTClusterData::SVTTClusterData(const AbstractDataSetGateway& DataGateway) : giAllocationSize(DataGateway.GetDataSetInterface().GetNumTimeIntervals()) {
  try {
    Init();
    Setup();
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor()","SVTTClusterData");
    throw;
  }
}

/** constructor */
SVTTClusterData::SVTTClusterData(const DataSetInterface & Interface) : giAllocationSize(Interface.GetNumTimeIntervals()) {
  try {
    Init();
    Setup();
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor()","SVTTClusterData");
    throw;
  }
}

/** copy constructor */
SVTTClusterData::SVTTClusterData(const SVTTClusterData& rhs) {
  try {
    Init();
    *this = rhs;
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor()","SVTTClusterData");
    throw;
  }
}

/** destructor */
SVTTClusterData::~SVTTClusterData() {
  try {
    delete[] gpCasesInsideCluster;
    delete[] gpCasesOutsideCluster;
    delete[] gpMeasureInsideCluster;
    delete[] gpMeasureOutsideCluster;
  }
  catch(...){}
}

/** Overloaded assignment assigment operator. */
SVTTClusterData & SVTTClusterData::operator=(const SVTTClusterData& rhs) {
  giAllocationSize = rhs.giAllocationSize;
  if (rhs.geEvaluationAssistDataStatus == Allocated) {
    if(!gpCasesInsideCluster) gpCasesInsideCluster = new count_t[rhs.giAllocationSize];
    if (!gpMeasureInsideCluster) gpMeasureInsideCluster = new measure_t[rhs.giAllocationSize];
    if(!gpCasesOutsideCluster) gpCasesOutsideCluster = new count_t[rhs.giAllocationSize];
    if (!gpMeasureOutsideCluster) gpMeasureOutsideCluster = new measure_t[rhs.giAllocationSize];    
    memcpy(gpCasesInsideCluster, rhs.gpCasesInsideCluster, giAllocationSize * sizeof(count_t));
    memcpy(gpMeasureInsideCluster, rhs.gpMeasureInsideCluster, giAllocationSize * sizeof(measure_t));
    memcpy(gpCasesOutsideCluster, rhs.gpCasesOutsideCluster, giAllocationSize * sizeof(count_t));
    memcpy(gpMeasureOutsideCluster, rhs.gpMeasureOutsideCluster, giAllocationSize * sizeof(measure_t));
  }
  gtTotalCasesOutsideCluster = rhs.gtTotalCasesOutsideCluster;
  gtTotalCasesInsideCluster = rhs.gtTotalCasesInsideCluster;
  gtTotalMeasureInsideCluster = rhs.gtTotalMeasureInsideCluster;
  gTimeTrendInside = rhs.gTimeTrendInside;
  gTimeTrendOutside = rhs.gTimeTrendOutside;
  return *this;
}

/** Returns newly cloned SVTTClusterData object. */
SVTTClusterData * SVTTClusterData::Clone() const {
  return new SVTTClusterData(*this);
}

/** Adds neighbor data to accumulation - caller is responsible for ensuring that
    'tNeighborIndex' and 'tSetIndex' are valid indexes. */
void SVTTClusterData::AddNeighborData(tract_t tNeighborIndex, const AbstractDataSetGateway& DataGateway, size_t tSetIndex) {
  assert(geEvaluationAssistDataStatus == Allocated);
  measure_t  ** ppMeasureNC(DataGateway.GetDataSetInterface().GetNCMeasureArray());
  count_t    ** ppCasesNC(DataGateway.GetDataSetInterface().GetNCCaseArray());

  for (unsigned int i=0; i < DataGateway.GetDataSetInterface().GetNumTimeIntervals(); ++i) {
    gpCasesInsideCluster[i] += ppCasesNC[i][tNeighborIndex];
    gpMeasureInsideCluster[i] += ppMeasureNC[i][tNeighborIndex];
    gpCasesOutsideCluster[i] -= ppCasesNC[i][tNeighborIndex];
    gpMeasureOutsideCluster[i] -= ppMeasureNC[i][tNeighborIndex];
    gtTotalCasesInsideCluster += ppCasesNC[i][tNeighborIndex];
    gtTotalMeasureInsideCluster += ppMeasureNC[i][tNeighborIndex];
    gtTotalCasesOutsideCluster -= ppCasesNC[i][tNeighborIndex];
  }
}

/** Calculates log likelihood ratio given accumulated data. */
double SVTTClusterData::CalculateSVTTLoglikelihoodRatio(AbstractLikelihoodCalculator& Calculator, const AbstractDataSetGateway& DataGateway) {
  assert(geEvaluationAssistDataStatus == Allocated);
  double ratio = Calculator.CalcSVTTLogLikelihood(0, *this, *DataGateway.GetDataSetInterface().GetTimeTrend());
  return (ratio ? ratio - Calculator.GetLogLikelihoodForTotal() : 0);
}

/** Copies class data members that reflect the number of cases and expected values,
    which is the data we are interested in for possible reporting. */
void SVTTClusterData::CopyEssentialClassMembers(const AbstractClusterData& rhs) {
  gtTotalCasesOutsideCluster = ((const SVTTClusterData&)rhs).gtTotalCasesOutsideCluster;
  gtTotalCasesInsideCluster = ((const SVTTClusterData&)rhs).gtTotalCasesInsideCluster;
  gtTotalMeasureInsideCluster = ((const SVTTClusterData&)rhs).gtTotalMeasureInsideCluster;
  gTimeTrendInside = ((const SVTTClusterData&)rhs).gTimeTrendInside;
  gTimeTrendOutside = ((const SVTTClusterData&)rhs).gTimeTrendOutside;
}

/** Deallocates data members that assist with evaluation of temporal data.
    Once this function is called various class member functions become invalid
    and an assertion will fail if called. */
void SVTTClusterData::DeallocateEvaluationAssistClassMembers() {
  try {
    delete[] gpCasesInsideCluster; gpCasesInsideCluster=0;
    delete[] gpMeasureInsideCluster; gpMeasureInsideCluster=0;
    delete[] gpCasesOutsideCluster; gpCasesOutsideCluster=0;
    delete[] gpMeasureOutsideCluster; gpMeasureOutsideCluster=0;
    giAllocationSize=0;
    geEvaluationAssistDataStatus = Deallocated;
  }
  catch (...){}
}

/** Returns number of cases accumulated in cluster object. */
count_t SVTTClusterData::GetCaseCount(unsigned int tSetIndex) const {
  return gtTotalCasesInsideCluster;
}

/** Returns expected number of cases accumulated in cluster object. */
measure_t SVTTClusterData::GetMeasure(unsigned int tSetIndex) const {
  return gtTotalMeasureInsideCluster;
}

/** internal initialization function */
void SVTTClusterData::Init() {
  gtTotalCasesInsideCluster=0;
  gtTotalMeasureInsideCluster=0;
  gtTotalCasesOutsideCluster=0;
  gpCasesInsideCluster=0;
  gpCasesOutsideCluster=0;
  gpMeasureInsideCluster=0;
  gpMeasureOutsideCluster=0;
}

/** re-intialize data */
void SVTTClusterData::InitializeSVTTData(const AbstractDataSetGateway& DataGateway) {
  InitializeSVTTData(DataGateway.GetDataSetInterface());
}

/** re-intialize data */
void SVTTClusterData::InitializeSVTTData(const DataSetInterface & Interface) {
  assert(geEvaluationAssistDataStatus == Allocated);
  memset(gpCasesInsideCluster, 0, giAllocationSize * sizeof(count_t));
  memset(gpMeasureInsideCluster, 0, giAllocationSize * sizeof(measure_t));
  gtTotalCasesInsideCluster=0;
  gtTotalMeasureInsideCluster=0;
  memcpy(gpCasesOutsideCluster, Interface.GetPTCaseArray(), giAllocationSize * sizeof(count_t));
  memcpy(gpMeasureOutsideCluster, Interface.GetPTMeasureArray(), giAllocationSize * sizeof(measure_t));
  gtTotalCasesOutsideCluster = Interface.GetTotalCasesCount();

  gTimeTrendInside.Initialize();
  gTimeTrendOutside.Initialize();
}

/** internal setup function */
void SVTTClusterData::Setup() {
  try {
    gpCasesInsideCluster = new count_t[giAllocationSize];
    gpCasesOutsideCluster = new count_t[giAllocationSize];
    gpMeasureInsideCluster = new measure_t[giAllocationSize];
    gpMeasureOutsideCluster = new measure_t[giAllocationSize];
  }
  catch (ZdException &x) {
    delete[] gpCasesInsideCluster;
    delete[] gpCasesOutsideCluster;
    delete[] gpMeasureInsideCluster;
    delete[] gpMeasureOutsideCluster;
    x.AddCallpath("Setup()","SVTTClusterData");
    throw;
  }
}


///////////////////////////// MultiSetSVTTClusterData //////////////////////////

/** constructor */
MultiSetSVTTClusterData::MultiSetSVTTClusterData(const AbstractDataSetGateway& DataGateway) {
  for (size_t t=0; t < DataGateway.GetNumInterfaces(); ++t)
    gvSetClusterData.push_back(new SVTTClusterData(DataGateway.GetDataSetInterface(t)));
}

/** destructor */
MultiSetSVTTClusterData::~MultiSetSVTTClusterData() {}

/** Adds neighbors data to accumulation. */
void MultiSetSVTTClusterData::AddNeighborData(tract_t tNeighborIndex, const AbstractDataSetGateway& DataGateway, size_t) {
  for (size_t t=0; t < DataGateway.GetNumInterfaces(); ++t)
     gvSetClusterData[t]->AddNeighborData(tNeighborIndex, DataGateway);
}

/** Returns calculated log likelihood ratio given accumulated cluster data. */
double MultiSetSVTTClusterData::CalculateSVTTLoglikelihoodRatio(AbstractLikelihoodCalculator& Calculator, const AbstractDataSetGateway& DataGateway) {
  printf("MultiSetSVTTClusterData::CalculateSVTTLoglikelihoodRatio() not implemented. -- returning value of first data set -- unifer updates needed\n");
  double ratio = Calculator.CalcSVTTLogLikelihood(0, *gvSetClusterData[0], *DataGateway.GetDataSetInterface().GetTimeTrend());
  return (ratio ? ratio - Calculator.GetLogLikelihoodForTotal() : 0);
}

/** Returns newly cloned MultiSetSVTTClusterData object */
MultiSetSVTTClusterData * MultiSetSVTTClusterData::Clone() const {
  return new MultiSetSVTTClusterData(*this);
}

/** Copies class data members that reflect the number of cases and expected values,
    which is the data we are interested in for possible reporting. */
void MultiSetSVTTClusterData::CopyEssentialClassMembers(const AbstractClusterData& rhs) {
  for (size_t t=0; t < gvSetClusterData.size(); ++t)
    gvSetClusterData[t]->CopyEssentialClassMembers(*((const MultiSetSVTTClusterData&)rhs).gvSetClusterData[t]);
}

/** Returns the number of cases in data set as index. */
count_t MultiSetSVTTClusterData::GetCaseCount(unsigned int tSetIndex) const {
  return gvSetClusterData[tSetIndex]->GetCaseCount();
}

/** Returns the data sets indexes that comprise the target ratio value. */
void MultiSetSVTTClusterData::GetDataSetIndexesComprisedInRatio(double dTargetLoglikelihoodRatio,
                                                                AbstractLikelihoodCalculator& Calculator,
                                                                std::vector<unsigned int>& vDataSetIndexes) const {
  printf("MultiSetSVTTClusterData::GetDataSetIndexesComprisedInRatio() not implemented. -- returning value of first data set -- unifer updates needed\n");
  vDataSetIndexes.clear();
  vDataSetIndexes.push_back(0);
}

/** Returns the number of expected cases in data set as index. */
measure_t MultiSetSVTTClusterData::GetMeasure(unsigned int tSetIndex) const {
  return gvSetClusterData[tSetIndex]->GetMeasure();
}

/** Deallocates data members that assist with evaluation of temporal data.
    Once this function is called various class member functions become invalid
    and an assertion will fail if called. */
void MultiSetSVTTClusterData::DeallocateEvaluationAssistClassMembers() {
  for (size_t t=0; t < gvSetClusterData.size(); ++t)
    gvSetClusterData[t]->DeallocateEvaluationAssistClassMembers();
}

/** Returns inside time trend for data at index. */
CTimeTrend  * MultiSetSVTTClusterData::getInsideTrend(size_t tSetIndex) {
  return &(gvSetClusterData[tSetIndex]->gTimeTrendInside);
}

/** Returns outside time trend for data at index. */
CTimeTrend  * MultiSetSVTTClusterData::getOutsideTrend(size_t tSetIndex) {
  return &(gvSetClusterData[tSetIndex]->gTimeTrendOutside);
}

/** Returns inside time trend for data at index. */
const CTimeTrend  * MultiSetSVTTClusterData::getInsideTrend(size_t tSetIndex) const {
  return &(gvSetClusterData[tSetIndex]->gTimeTrendInside);
}

/** Returns outside time trend for data at index. */
const CTimeTrend  * MultiSetSVTTClusterData::getOutsideTrend(size_t tSetIndex) const {
  return &(gvSetClusterData[tSetIndex]->gTimeTrendOutside);
}

/** re-intialize data */
void MultiSetSVTTClusterData::InitializeSVTTData(const AbstractDataSetGateway& DataGateway) {
  for (size_t t=0; t < DataGateway.GetNumInterfaces(); ++t)
     gvSetClusterData[t]->InitializeSVTTData(DataGateway.GetDataSetInterface());
}

/** re-intialize data */
void MultiSetSVTTClusterData::InitializeSVTTData(const DataSetInterface & Interface) {
  ZdGenerateException("InitializeSVTTData(const DataSetInterface&) not implemented.","MultiSetSVTTClusterData");
}

///////////////////////////// CSVTTCluster /////////////////////////////////////

/** constructor for DataSetGateway - used with calculating most likely clusters */
CSVTTCluster::CSVTTCluster(const AbstractClusterDataFactory * pClusterFactory, const AbstractDataSetGateway & DataGateway)
             :CCluster(){
  try {
    gClusterData.reset(pClusterFactory->GetNewSVTTClusterData(DataGateway));
    InitializeSVTT(0, DataGateway);
    m_nLastInterval = DataGateway.GetDataSetInterface().GetNumTimeIntervals();
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor()","CSVTTCluster");
    throw;
  }
}

/** copy constructor */
CSVTTCluster::CSVTTCluster(const CSVTTCluster & rhs): CCluster() {
  try {
    gClusterData.reset(rhs.gClusterData->CloneSVTT());
    *this = rhs;
  }
  catch (ZdException &x) {
    x.AddCallpath("copy constructor()","CSVTTCluster");
    throw;
  }
}

/** destructor */
CSVTTCluster::~CSVTTCluster() {}

/** overloaded assignment operator */
CSVTTCluster& CSVTTCluster::operator=(const CSVTTCluster& rhs) {
  if (this == &rhs)
    return *this;
  m_Center                      = rhs.m_Center;
  m_MostCentralLocation         = rhs.m_MostCentralLocation;
  m_nTracts                     = rhs.m_nTracts;
  m_CartesianRadius             = rhs.m_CartesianRadius;  
  m_nRatio                      = rhs.m_nRatio;
  m_nRank                       = rhs.m_nRank;
  m_nFirstInterval              = rhs.m_nFirstInterval;
  m_nLastInterval               = rhs.m_nLastInterval;
  m_NonCompactnessPenalty       = rhs.m_NonCompactnessPenalty;
  m_iEllipseOffset              = rhs.m_iEllipseOffset;

  gClusterData.reset(rhs.gClusterData->CloneSVTT()); 
  return *this;
}

/** add neighbor tract data from DataGateway */
void CSVTTCluster::AddNeighbor(tract_t tNeighbor, const AbstractDataSetGateway & DataGateway) {
  ++m_nTracts;
  gClusterData->AddNeighborData(tNeighbor, DataGateway);
}

/** Adds neighbor location data from DataGateway to cluster data accumulation and
    calculates loglikelihood ratio. If ratio is greater than that of TopCluster,
    assigns TopCluster to 'this'. */
void CSVTTCluster::CalculateTopClusterAboutCentroidDefinition(const AbstractDataSetGateway& DataGateway,
                                                              const CentroidNeighbors& CentroidDef,
                                                              CSVTTCluster& TopCluster,
                                                              AbstractLikelihoodCalculator& Calculator) {
  tract_t               t, tNumNeighbors = CentroidDef.GetNumNeighbors(),
                      * pIntegerArray = CentroidDef.GetRawIntegerArray();
  unsigned short      * pUnsignedShortArray = CentroidDef.GetRawUnsignedShortArray();

  for (t=0; t < tNumNeighbors; ++t) {
    ++m_nTracts;
    gClusterData->AddNeighborData((pUnsignedShortArray ? (tract_t)pUnsignedShortArray[t] : pIntegerArray[t]), DataGateway);
    m_nRatio =  gClusterData->CalculateSVTTLoglikelihoodRatio(Calculator, DataGateway);
    if (m_nRatio > TopCluster.m_nRatio)
      TopCluster.CopyEssentialClassMembers(*this);
  }    
}

/** returns newly cloned CSVTTCluster */
CSVTTCluster * CSVTTCluster::Clone() const {
  return new CSVTTCluster(*this);
}

void CSVTTCluster::DisplayAnnualTimeTrendWithoutTitle(FILE* fp) const {
  if (gClusterData->getInsideTrend()->IsNegative())
    fprintf(fp, "     -");
  else
    fprintf(fp, "      ");

  fprintf(fp, "%.3f", gClusterData->getInsideTrend()->GetAnnualTimeTrend());
}

void CSVTTCluster::DisplayTimeTrend(FILE* fp, const AsciiPrintFormat& PrintFormat) const {
  std::string buffer;

  PrintFormat.PrintSectionLabel(fp, "Inside Time trend", false, true);
  GetFormattedTimeTrend(buffer, *gClusterData->getInsideTrend());
  PrintFormat.PrintAlignedMarginsDataString(fp, buffer);
  PrintFormat.PrintSectionLabel(fp, "Outside Time trend", false, true);
  GetFormattedTimeTrend(buffer, *gClusterData->getOutsideTrend());
  PrintFormat.PrintAlignedMarginsDataString(fp, buffer);
  PrintFormat.PrintSectionLabel(fp, "Time trend difference", false, true);
  buffer = "?";
  PrintFormat.PrintAlignedMarginsDataString(fp, buffer);
}

AbstractClusterData * CSVTTCluster::GetClusterData() {
 return gClusterData.get();
}

const AbstractClusterData * CSVTTCluster::GetClusterData() const {
 return gClusterData.get();
}

/** returns end date of defined cluster as formated string */
std::string& CSVTTCluster::GetEndDate(std::string& sDateString, const CSaTScanData& DataHub) const {
  return JulianToString(sDateString, DataHub.GetTimeIntervalStartTimes()[DataHub.GetNumTimeIntervals()] - 1);
}

/** Returns number of expcected cases in accumulated data. */
measure_t CSVTTCluster::GetExpectedCount(const CSaTScanData& DataHub, size_t tSetIndex) const {
  return DataHub.GetMeasureAdjustment(tSetIndex) * gClusterData->GetMeasure(tSetIndex);
}

/** Returns the measure for tract as defined by cluster. */
measure_t CSVTTCluster::GetExpectedCountForTract(tract_t tTractIndex, const CSaTScanData& Data, size_t tSetIndex) const {
  return Data.GetMeasureAdjustment(tSetIndex) * Data.GetDataSetHandler().GetDataSet(tSetIndex).getMeasureData().GetArray()[0][tTractIndex];
}

/** Returns formatted time trend string. */
void CSVTTCluster::GetFormattedTimeTrend(std::string& buffer, const CTimeTrend& Trend) const {
  try {
    switch (Trend.GetStatus()) {
     case CTimeTrend::TREND_UNDEF        : buffer = "undefined"; break;
     case CTimeTrend::TREND_INF_BEGIN    : buffer = "-100"; break;
     case CTimeTrend::TREND_INF_END      : buffer = "infinity"; break;
     case CTimeTrend::TREND_NOTCONVERGED : ZdGenerateException("Calling GetFormattedTimeTrend() with time trend that has not converged.", "GetFormattedTimeTrend()");
     case CTimeTrend::TREND_CONVERGED    : printString(buffer, "%f  (%.3f%% %s", Trend.GetBeta(), Trend.GetAnnualTimeTrend(),
                                                       (Trend.IsNegative() ? "annual decrease)" : "annual increase)"));
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("GetFormattedTimeTrend()","CSVTTCluster");
    throw;
  }
}

/** returns the number of cases for tract as defined by cluster */
count_t CSVTTCluster::GetObservedCountForTract(tract_t tTractIndex, const CSaTScanData& Data, size_t tSetIndex) const {
  return Data.GetDataSetHandler().GetDataSet(tSetIndex).getCaseData().GetArray()[0][tTractIndex];
}

/** returns start date of defined cluster as formated string */
std::string& CSVTTCluster::GetStartDate(std::string& sDateString, const CSaTScanData& DataHub) const {
  return JulianToString(sDateString, DataHub.GetTimeIntervalStartTimes()[0]);
}

/** re-initializes cluster data */
void CSVTTCluster::InitializeSVTT(tract_t nCenter, const AbstractDataSetGateway & DataGateway) {
  m_Center         = nCenter;
  m_MostCentralLocation = -1;
  m_nTracts        = 0;
  m_CartesianRadius= -1;
  m_nRatio         = 0;
  m_nRank          = 1;
  m_NonCompactnessPenalty = 1;
  m_iEllipseOffset = 0;
  gClusterData->InitializeSVTTData(DataGateway);
}

/** re-initializes cluster data */
void CSVTTCluster::InitializeSVTT(tract_t nCenter, const DataSetInterface & Interface) {
  CCluster::Initialize(nCenter);
  gClusterData->InitializeSVTTData(Interface);
}

void CSVTTCluster::SetTimeTrend(DatePrecisionType eDatePrecision, double nIntervalLen) {
  gClusterData->getInsideTrend()->SetAnnualTimeTrend(eDatePrecision, nIntervalLen);
}

