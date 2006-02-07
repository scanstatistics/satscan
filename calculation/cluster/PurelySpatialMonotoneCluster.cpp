//*****************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//*****************************************************************************
#include "PurelySpatialMonotoneCluster.h"
#include "ClusterLocationsWriter.h"

/** class constructor - const AbstractDataSetGateway */
CPSMonotoneCluster::CPSMonotoneCluster(const AbstractClusterDataFactory * pClusterFactory,
                                       const AbstractDataSetGateway & DataGateway, int iRate)
                   :CCluster() {
  m_nMaxCircles        = 0;
  m_pCasesList         = 0;
  m_pMeasureList       = 0;
  m_pFirstNeighborList = 0;
  m_pLastNeighborList  = 0;
  Initialize(0);
}

/** class constructor - const DataSetInterface */
CPSMonotoneCluster::CPSMonotoneCluster(const AbstractClusterDataFactory * pClusterFactory,
                                       const DataSetInterface & Interface, int iRate)
                   :CCluster() {
  m_nMaxCircles        = 0;
  m_pCasesList         = 0;
  m_pMeasureList       = 0;
  m_pFirstNeighborList = 0;
  m_pLastNeighborList  = 0;
  Initialize(0);
}

/** class copy constructor */
CPSMonotoneCluster::CPSMonotoneCluster(const CPSMonotoneCluster& rhs) : CCluster(rhs) {
  *this = rhs;
}

/** class destructor */
CPSMonotoneCluster::~CPSMonotoneCluster() {
  try {
    free(m_pCasesList);
    free(m_pMeasureList);
    free(m_pFirstNeighborList);
    free(m_pLastNeighborList);
  }
  catch (...){}
}

/** overloaded assigment operator */
CPSMonotoneCluster& CPSMonotoneCluster::operator=(const CPSMonotoneCluster& rhs) {
  try {
    m_Center         = rhs.m_Center;
    m_MostCentralLocation = rhs.m_MostCentralLocation;
    m_nCases         = rhs.m_nCases;
    m_nMeasure       = rhs.m_nMeasure;
    m_nTracts        = rhs.m_nTracts;
    m_nRatio         = rhs.m_nRatio;
    m_nLogLikelihood = rhs.m_nLogLikelihood;
    m_nRank          = rhs.m_nRank;
    m_nFirstInterval = rhs.m_nFirstInterval;
    m_nLastInterval  = rhs.m_nLastInterval;
    m_nSteps         = rhs.m_nSteps;
    m_bRatioSet      = rhs.m_bRatioSet;
    m_nMaxCircles    = rhs.m_nSteps;
    free(m_pCasesList);
    m_pCasesList = (count_t*)Smalloc(rhs.m_nSteps * sizeof(count_t));
    memcpy(m_pCasesList, rhs.m_pCasesList, rhs.m_nMaxCircles * sizeof(count_t));
    free(m_pMeasureList);
    m_pMeasureList = (measure_t*)Smalloc(rhs.m_nSteps * sizeof(measure_t));
    memcpy(m_pMeasureList, rhs.m_pMeasureList, rhs.m_nMaxCircles * sizeof(measure_t));
    free(m_pFirstNeighborList);
    m_pFirstNeighborList = (tract_t*)Smalloc(rhs.m_nSteps * sizeof(tract_t));
    memcpy(m_pFirstNeighborList, rhs.m_pFirstNeighborList, rhs.m_nMaxCircles * sizeof(tract_t));
    free(m_pLastNeighborList);
    m_pLastNeighborList = (tract_t*)Smalloc(rhs.m_nSteps * sizeof(tract_t));
    memcpy(m_pLastNeighborList, rhs.m_pLastNeighborList, rhs.m_nMaxCircles * sizeof(tract_t));
  }
  catch (ZdException &x) {
    x.AddCallpath("operator=","CPSMonotoneCluster");
    throw;
  }
  return *this;
}

/** Adds neighbor data to cluster definition. */
void CPSMonotoneCluster::AddNeighbor(int iEllipse, const CSaTScanData& Data, count_t** pCases, tract_t n) {
  tract_t       nNeighbor = Data.GetNeighbor(0, m_Center, n);
  measure_t   * pMeasure(Data.GetDataSetHandler().GetDataSet(0/*for now*/).GetMeasureArray()[0]);

  m_nSteps++;

  m_nCases   += pCases[0][nNeighbor];
  m_nMeasure += pMeasure[nNeighbor];

  m_pCasesList[m_nSteps-1]         = pCases[0][nNeighbor];
  m_pMeasureList[m_nSteps-1]       = pMeasure[nNeighbor];
  m_pFirstNeighborList[m_nSteps-1] = n;
  m_pLastNeighborList[m_nSteps-1]  = n;
}

/** no documentation */
void CPSMonotoneCluster::AddRemainder(count_t nTotalCases, measure_t nTotalMeasure) {
  m_nSteps++;

  m_pCasesList[m_nSteps-1]   = nTotalCases - m_nCases;
  m_pMeasureList[m_nSteps-1] = nTotalMeasure - m_nMeasure;
}

/** Allocates internal structures for number of circles in spatial cluster. */
void CPSMonotoneCluster::AllocateForMaxCircles(tract_t nCircles) {
  try {
    m_nMaxCircles        = nCircles;
    m_pCasesList         = (count_t*)  Smalloc(m_nMaxCircles * sizeof(count_t));
    m_pMeasureList       = (measure_t*)Smalloc(m_nMaxCircles * sizeof(measure_t));
    m_pFirstNeighborList = (tract_t*)  Smalloc(m_nMaxCircles * sizeof(tract_t));
    m_pLastNeighborList  = (tract_t*)  Smalloc(m_nMaxCircles * sizeof(tract_t));
  }
  catch (ZdException &x) {
    x.AddCallpath("AllocateForMaxCircles()","CPSMonotoneCluster");
    throw;
  }
}

/** no documentation */
void CPSMonotoneCluster::CheckCircle(tract_t n) {
  if (n != 0)
    if (!m_pfRateOfInterest(m_pCasesList[n-1],m_pMeasureList[n-1], m_pCasesList[n],  m_pMeasureList[n])) {
      ConcatLastCircles();
      CheckCircle(n-1);
    }
}

/** returns newly cloned CPSMonotoneCluster */
CPSMonotoneCluster * CPSMonotoneCluster::Clone() const {
  return new CPSMonotoneCluster(*this);
}

/** no documentation */
void CPSMonotoneCluster::ConcatLastCircles() {
  m_nSteps--;
  m_pCasesList[m_nSteps-1]        += m_pCasesList[m_nSteps];
  m_pMeasureList[m_nSteps-1]      += m_pMeasureList[m_nSteps];
  m_pLastNeighborList[m_nSteps-1]  = m_pLastNeighborList[m_nSteps];

  m_pCasesList[m_nSteps]         = 0;
  m_pMeasureList[m_nSteps]       = 0;
  m_pFirstNeighborList[m_nSteps] = 0;
  m_pLastNeighborList[m_nSteps]  = 0;
}

/** no documentation */
void CPSMonotoneCluster::DefineTopCluster(const CSaTScanData& Data, AbstractLikelihoodCalculator & Calculator, count_t** pCases) {
  tract_t            ** ppNeighborCount(Data.GetNeighborCountArray());

  try {
    //  int i = 1;
    //  AddNeighbor(Data, pCases, i);
    //  while (i<=Data.m_NeighborCounts[m_Center] &&
    //         m_nMeasure <= Data.m_nMaxCircleSize)
    //for (int k = 0; k <= m_pParameters->GetNumTotalEllipses(); k++)   //circle is 0 offset... (always there)
    //  {
    for (int i=1; i<= ppNeighborCount[0][m_Center]; i++) {
       AddNeighbor(0, Data, pCases, i);
       CheckCircle(GetLastCircleIndex());
        //    i++;
        //    if (i<=Data.m_NeighborCounts[m_Center])
        //      AddNeighbor(Data, pCases, i);
        }
      // }
      if (Data.GetTotalCases() != m_nCases) {
        AddRemainder(Data.GetTotalCases(), Data.GetTotalMeasure());
        CheckCircle(GetLastCircleIndex());
      }
      m_nLogLikelihood = Calculator.CalcMonotoneLogLikelihood(*this);
      SetRatio(Calculator.GetLogLikelihoodForTotal());
      if (Data.GetTotalCases() != m_nCases)
        RemoveRemainder();
      // Recalc Total Cases, Measure, and Tracts to account for
      // outer tracts absorbed into remainder.
      SetCasesAndMeasures();
      SetTotalTracts();
  }
  catch (ZdException &x) {
    x.AddCallpath("DefineTopCluster()","CPSMonotoneCluster");
    throw;
  }
}

/** Prints locations of cluster, detailed by step, to file pointer in ACSII format. */
void CPSMonotoneCluster::DisplayCensusTracts(FILE* fp, const CSaTScanData& Data, const AsciiPrintFormat& PrintFormat) const {
  int           i;
  ZdString      sBuffer;

  try {
    PrintFormat.PrintSectionLabel(fp, "Location IDs included", false, false);
    for (i=0; i < m_nSteps; ++i) {
       fprintf(fp, "\n");                                                                        
       sBuffer.printf("  Step %i",i + 1);
       PrintFormat.PrintSectionLabel(fp, sBuffer.GetCString(), false, true);
       DisplayCensusTractsInStep(fp, Data, m_pFirstNeighborList[i], m_pLastNeighborList[i], PrintFormat);
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("DisplayCensusTracts()","CPSMonotoneCluster");
    throw;
  }
}

/** Prints cartesian coordinates of cluster to file pointer in ACSII format. */
void CPSMonotoneCluster::DisplayCoordinates(FILE* fp, const CSaTScanData& Data, const AsciiPrintFormat& PrintFormat) const {
  std::vector<double>   vCoordinates, vCoodinatesOfStep;
  float                 nRadius;
  ZdString              sBuffer, sWork;

  try {
    Data.GetGInfo()->giRetrieveCoords(m_Center, vCoordinates);
    PrintFormat.PrintSectionLabel(fp, "Coordinates", false, true);
    for (size_t t=0; t < vCoordinates.size() - 1; ++t) {
       sWork.printf("%s%g,", (t == 0 ? "(" : "" ), vCoordinates[t]);
       sBuffer << sWork;
    }
    sWork.printf("%g)", vCoordinates.back());
    sBuffer << sWork;
    PrintFormat.PrintAlignedMarginsDataString(fp, sBuffer);
    PrintFormat.PrintSectionLabel(fp, "Radius for each step", false, true);
    sBuffer << ZdString::reset;
    for (int i=0; i < m_nSteps; ++i) {
       Data.GetTInfo()->tiRetrieveCoords(Data.GetNeighbor(0, m_Center, m_pLastNeighborList[i]), vCoodinatesOfStep);
       nRadius = (float)sqrt(Data.GetTInfo()->tiGetDistanceSq(vCoordinates, vCoodinatesOfStep));
       sWork.printf("%s%4.2f", (i > 0 ? ", " : ""), nRadius);
       sBuffer << sWork;
    }
    PrintFormat.PrintAlignedMarginsDataString(fp, sBuffer);
  }
  catch (ZdException &x) {
    x.AddCallpath("DisplayCoordinates()","CPSMonotoneCluster");
    throw;
  }
}

/** Prints latitude/longitude coordinates of cluster to file pointer in ACSII format. */
void CPSMonotoneCluster::DisplayLatLongCoords(FILE* fp, const CSaTScanData& Data, const AsciiPrintFormat& PrintFormat) const {
  double                        dRadius, * pCoords2=0;
  std::vector<double>           vCoordinates, vCoodinatesOfStep;
  std::pair<double, double>     prLatitudeLongitude;
  char                          cNorthSouth, cEastWest;
  ZdString                      sBuffer, sWork;

  try {
    Data.GetGInfo()->giRetrieveCoords(m_Center, vCoordinates);
    prLatitudeLongitude = ConvertToLatLong(vCoordinates);
    prLatitudeLongitude.first >= 0 ? cNorthSouth = 'N' : cNorthSouth = 'S';
    prLatitudeLongitude.second >= 0 ? cEastWest = 'W' : cEastWest = 'E';
    PrintFormat.PrintSectionLabel(fp, "Coordinates", false, true);
    fprintf(fp, "(%.6f %c, %.6f %c)\n", fabs(prLatitudeLongitude.first), cNorthSouth, fabs(prLatitudeLongitude.second), cEastWest);
    PrintFormat.PrintSectionLabel(fp, "Radius for each step", false, true);
    for (int i=0; i < m_nSteps; ++i) {
      Data.GetTInfo()->tiRetrieveCoords(Data.GetNeighbor(0, m_Center, m_pLastNeighborList[i]), vCoodinatesOfStep);
      dRadius = 2 * EARTH_RADIUS_km * asin(sqrt(Data.GetTInfo()->tiGetDistanceSq(vCoordinates, vCoodinatesOfStep))/(2 * EARTH_RADIUS_km));
      sWork.printf("%s%5.2lf km", (i == 0 ? "(" : "" ), dRadius);
      sBuffer << sWork;
    }
    PrintFormat.PrintAlignedMarginsDataString(fp, sBuffer);
  }
  catch (ZdException &x) {
    x.AddCallpath("DisplayLatLongCoords()", "CPSMonotoneCluster");
    throw;
  }
}

/** Prints observed divided by expected and relative risk of cluster to file pointer in ACSII format. */
void CPSMonotoneCluster::DisplayObservedDivExpected(FILE* fp, unsigned int iDataSetIndex, const CSaTScanData& DataHub, const AsciiPrintFormat& PrintFormat) const {
  ZdString      sBuffer;

  try {
    CCluster::DisplayObservedDivExpected(fp, iDataSetIndex, DataHub, PrintFormat);
    if (m_nSteps == 1)
      return;
    PrintFormat.PrintSectionLabel(fp, "Relative risk by step", false, true);
    sBuffer.printf("%.3f", GetRelativeRisk(iDataSetIndex, DataHub));
    PrintFormat.PrintAlignedMarginsDataString(fp, sBuffer);
  }
  catch (ZdException &x) {
    x.AddCallpath("DisplayObservedDivExpected()","CPSMonotoneCluster");
    throw;
  }
}

/** Prints number of steps to file pointer in ACSII format. */
void CPSMonotoneCluster::DisplaySteps(FILE* fp, const AsciiPrintFormat& PrintFormat) const {
  PrintFormat.PrintSectionLabel(fp, "Steps in risk function", false, true);
  fprintf(fp, "%i\n", m_nSteps);
}

/** Returns pointer cluster data object - not implemented, throws exception. */
AbstractClusterData * CPSMonotoneCluster::GetClusterData() {
 ZdGenerateException("GetClusterData() not implemented.","CPSMonotoneCluster");
 return 0;
}

/** Returns pointer cluster data object - not implemented, throws exception. */
const AbstractClusterData * CPSMonotoneCluster::GetClusterData() const {
 ZdGenerateException("GetClusterData() not implemented.","CPSMonotoneCluster");
 return 0;
}

/** returns end date of defined cluster as formated string */
ZdString& CPSMonotoneCluster::GetEndDate(ZdString& sDateString, const CSaTScanData& DataHub) const {
  return JulianToString(sDateString, DataHub.GetTimeIntervalStartTimes()[DataHub.GetNumTimeIntervals()] - 1);
}

/** Returns number of expected cases in accumulated data. */
measure_t CPSMonotoneCluster::GetExpectedCount(const CSaTScanData& DataHub, size_t tSetIndex) const {
  return DataHub.GetMeasureAdjustment(tSetIndex) * m_nMeasure;
}

/** Returns the measure for tract as defined by cluster. */
measure_t CPSMonotoneCluster::GetExpectedCountForTract(tract_t tTractIndex, const CSaTScanData& Data, size_t tSetIndex) const {
  return Data.GetMeasureAdjustment(tSetIndex) * Data.GetDataSetHandler().GetDataSet(tSetIndex).GetMeasureArray()[0][tTractIndex];
}

/** Returns log likelihood of cluster. */
double CPSMonotoneCluster::GetLogLikelihood() const {
  return m_nLogLikelihood;
}

/** returns the number of cases for tract as defined by cluster */
count_t CPSMonotoneCluster::GetObservedCountForTract(tract_t tTractIndex, const CSaTScanData& Data,size_t tSetIndex) const {
  return Data.GetDataSetHandler().GetDataSet(tSetIndex).GetCaseArray()[0][tTractIndex];
}

/** If ratio flag is set, returns log likelihood ratio else returns -1.*/
double CPSMonotoneCluster::GetRatio() const {
  return (m_bRatioSet ? m_nRatio : -1);
}

/** no documentation */
double CPSMonotoneCluster::GetRelativeRisk(tract_t nStep, const CSaTScanData& DataHub) const {
  return ((double)(m_pCasesList[nStep]))/(m_pMeasureList[nStep] * DataHub.GetMeasureAdjustment(0));
}

/** returns start date of defined cluster as formated string */
ZdString& CPSMonotoneCluster::GetStartDate(ZdString& sDateString, const CSaTScanData& DataHub) const {
  return JulianToString(sDateString, DataHub.GetTimeIntervalStartTimes()[0]);
}

/** initialize cluster data and data members */
void CPSMonotoneCluster::Initialize(tract_t nCenter) {
  CCluster::Initialize(nCenter);
  m_bRatioSet = false;
  m_nLastInterval = 1;
  m_nCases = 0;
  m_nMeasure = 0;
  m_nLogLikelihood = 0;
  m_nSteps = 0;
  memset(m_pCasesList, 0, m_nMaxCircles * sizeof(count_t));
  memset(m_pMeasureList, 0, m_nMaxCircles * sizeof(measure_t));
  memset(m_pFirstNeighborList, 0, m_nMaxCircles * sizeof(tract_t));
  memset(m_pLastNeighborList, 0, m_nMaxCircles * sizeof(tract_t));
}

/** no documentation */
void CPSMonotoneCluster::RemoveRemainder() {
  m_nSteps--;
  m_pCasesList[m_nSteps]         = 0;
  m_pMeasureList[m_nSteps]       = 0;
  m_pFirstNeighborList[m_nSteps] = 0;
  m_pLastNeighborList[m_nSteps]  = 0;

  if (m_nSteps==0)
    Initialize(m_Center);
}

/** no documentation */
void CPSMonotoneCluster::SetCasesAndMeasures() {
  m_nCases   = 0;
  m_nMeasure = 0;

  for (int i=0; i < m_nSteps; ++i) {
     m_nCases   += m_pCasesList[i];
     m_nMeasure += m_pMeasureList[i];
  }
}

/** no documentation - not invoked */
double CPSMonotoneCluster::SetLogLikelihood() {
  m_nLogLikelihood = 0;

  for (int i=0; i<m_nSteps; i++) {
    if (m_pCasesList[i] != 0)
      m_nLogLikelihood += m_pCasesList[i] * log(m_pCasesList[i]/m_pMeasureList[i]);
  }

  return m_nLogLikelihood;
}

/** Calculates log likelihood ratio given log likelihood for total and marks
    internal flag that ratio is set. */
double CPSMonotoneCluster::SetRatio(double nLikelihoodForTotal) {
  m_bRatioSet = true;
  m_nRatio    = m_nLogLikelihood - nLikelihoodForTotal;

  return m_nRatio;
}

/** Returns the total number of tracts in cluster, across all steps. */
void CPSMonotoneCluster::SetTotalTracts() {
  m_nTracts = 0;
  for (int i=0; i < m_nSteps; i++)
    m_nTracts += m_pLastNeighborList[i] - m_pFirstNeighborList[i] + 1;
}

/** Writes cluster data to passed record buffer. */
void CPSMonotoneCluster::Write(LocationInformationWriter& LocationWriter, const CSaTScanData& Data,
                     unsigned int iClusterNumber, unsigned int iNumSimsCompleted) const {
  tract_t       t, tTract;
  int           i;

  try {
    for (i=0; i < m_nSteps; ++i) {
       for (t=m_pFirstNeighborList[i]; t <= m_pLastNeighborList[i]; t++) {
         tTract = Data.GetNeighbor(m_iEllipseOffset, m_Center, t);
         LocationWriter.Write(*this, Data, iClusterNumber, tTract, iNumSimsCompleted);
       }
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("Write(stsAreaSpecificData*)","CPSMonotoneCluster");
    throw;
  }
}

