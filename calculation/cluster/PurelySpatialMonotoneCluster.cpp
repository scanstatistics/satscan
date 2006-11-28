//*****************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//*****************************************************************************
#include "PurelySpatialMonotoneCluster.h"
#include "ClusterLocationsWriter.h"
#include "SSException.h"

/** class constructor - const AbstractDataSetGateway */
CPSMonotoneCluster::CPSMonotoneCluster(const AbstractClusterDataFactory * pClusterFactory,
                                       const AbstractDataSetGateway & DataGateway, int iRate)
                   :CCluster(), m_nMaxCircles(0), m_bRatioSet(false), m_nCases(0), m_nMeasure(0), m_nSteps(0), m_nLogLikelihood(0) {
  //The last time interval for a purely spatial cluster equals the number of
  //calculated time intervals. This would be 1 for a purely spatial analysis but
  //for a space-time analysis, the index would be dependent on # of intervals requested.
  m_nLastInterval = DataGateway.GetDataSetInterface().GetNumTimeIntervals();
}

/** class constructor - const DataSetInterface */
CPSMonotoneCluster::CPSMonotoneCluster(const AbstractClusterDataFactory * pClusterFactory,
                                       const DataSetInterface & Interface, int iRate)
                   :CCluster(), m_nMaxCircles(0), m_bRatioSet(false), m_nCases(0), m_nMeasure(0), m_nSteps(0), m_nLogLikelihood(0) {
  //The last time interval for a purely spatial cluster equals the number of
  //calculated time intervals. This would be 1 for a purely spatial analysis but
  //for a space-time analysis, the index would be dependent on # of intervals requested.
  m_nLastInterval = Interface.GetNumTimeIntervals();
  
  Initialize(0);
}

/** class copy constructor */
CPSMonotoneCluster::CPSMonotoneCluster(const CPSMonotoneCluster& rhs)
                   :CCluster(rhs), m_nMaxCircles(0), m_bRatioSet(false), m_nCases(0), m_nMeasure(0), m_nSteps(0), m_nLogLikelihood(0) {
  *this = rhs;
}

/** class destructor */
CPSMonotoneCluster::~CPSMonotoneCluster() {}

/** overloaded assigment operator */
CPSMonotoneCluster& CPSMonotoneCluster::operator=(const CPSMonotoneCluster& rhs) {
  try {
    m_Center              = rhs.m_Center;
    m_MostCentralLocation = rhs.m_MostCentralLocation;
    m_nCases              = rhs.m_nCases;
    m_nMeasure            = rhs.m_nMeasure;
    m_nTracts             = rhs.m_nTracts;
    m_nRatio              = rhs.m_nRatio;
    m_nLogLikelihood      = rhs.m_nLogLikelihood;
    m_nRank               = rhs.m_nRank;
    m_nFirstInterval      = rhs.m_nFirstInterval;
    m_nLastInterval       = rhs.m_nLastInterval;
    m_nSteps              = rhs.m_nSteps;
    m_bRatioSet           = rhs.m_bRatioSet;
    m_nMaxCircles         = rhs.m_nSteps;
    gvCasesList           = rhs.gvCasesList;
    gvMeasureList         = rhs.gvMeasureList;
    gvFirstNeighborList   = rhs.gvFirstNeighborList;
    gvLastNeighborList    = rhs.gvLastNeighborList;
  }
  catch (prg_exception& x) {
    x.addTrace("operator=","CPSMonotoneCluster");
    throw;
  }
  return *this;
}

/** Adds neighbor data to cluster definition. */
void CPSMonotoneCluster::AddNeighbor(int iEllipse, const CSaTScanData& Data, count_t** pCases, tract_t n) {
  tract_t       nNeighbor = Data.GetNeighbor(0, m_Center, n);
  measure_t   * pMeasure(Data.GetDataSetHandler().GetDataSet(0/*for now*/).getMeasureData().GetArray()[0]);

  m_nSteps++;

  m_nCases   += pCases[0][nNeighbor];
  m_nMeasure += pMeasure[nNeighbor];

  gvCasesList.at(m_nSteps-1) = pCases[0][nNeighbor];
  gvMeasureList.at(m_nSteps-1) = pMeasure[nNeighbor];
  gvFirstNeighborList.at(m_nSteps-1) = n;
  gvLastNeighborList.at(m_nSteps-1) = n;
}

/** no documentation */
void CPSMonotoneCluster::AddRemainder(count_t nTotalCases, measure_t nTotalMeasure) {
  m_nSteps++;

  gvCasesList.at(m_nSteps-1) = nTotalCases - m_nCases;
  gvMeasureList.at(m_nSteps-1) = nTotalMeasure - m_nMeasure;
}

/** Allocates internal structures for number of circles in spatial cluster. */
void CPSMonotoneCluster::AllocateForMaxCircles(tract_t nCircles) {
  try {
    m_nMaxCircles        = nCircles;
    gvCasesList.resize(m_nMaxCircles);
    gvMeasureList.resize(m_nMaxCircles);
    gvFirstNeighborList.resize(m_nMaxCircles);
    gvLastNeighborList.resize(m_nMaxCircles);
  }
  catch (prg_exception& x) {
    x.addTrace("AllocateForMaxCircles()","CPSMonotoneCluster");
    throw;
  }
}

/** no documentation */
void CPSMonotoneCluster::CheckCircle(tract_t n) {
  if (n != 0)
    if (!m_pfRateOfInterest(gvCasesList.at(n-1), gvMeasureList.at(n-1), gvCasesList.at(n), gvMeasureList.at(n))) {
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
  gvCasesList.at(m_nSteps-1) += gvCasesList.at(m_nSteps);
  gvMeasureList.at(m_nSteps-1) += gvMeasureList.at(m_nSteps);
  gvLastNeighborList.at(m_nSteps-1) = gvLastNeighborList.at(m_nSteps);

  gvCasesList.at(m_nSteps) = 0;
  gvMeasureList.at(m_nSteps) = 0;
  gvFirstNeighborList.at(m_nSteps) = 0;
  gvLastNeighborList.at(m_nSteps) = 0;
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
  catch (prg_exception& x) {
    x.addTrace("DefineTopCluster()","CPSMonotoneCluster");
    throw;
  }
}

/** Prints locations of cluster, detailed by step, to file pointer in ACSII format. */
void CPSMonotoneCluster::DisplayCensusTracts(FILE* fp, const CSaTScanData& Data, const AsciiPrintFormat& PrintFormat) const {
  std::string buffer;

  try {
    PrintFormat.PrintSectionLabel(fp, "Steps in risk function", false, true);
    fprintf(fp, "%i\n", m_nSteps);
    for (int i=0; i < m_nSteps; ++i) {
       printString(buffer, "  Step %i",i + 1);
       PrintFormat.PrintSectionLabel(fp, buffer.c_str(), false, true);
       DisplayCensusTractsInStep(fp, Data, gvFirstNeighborList.at(i), gvLastNeighborList.at(i), PrintFormat);
    }
  }
  catch (prg_exception& x) {
    x.addTrace("DisplayCensusTracts()","CPSMonotoneCluster");
    throw;
  }
}

/** Prints cartesian coordinates of cluster to file pointer in ACSII format. */
void CPSMonotoneCluster::DisplayCoordinates(FILE* fp, const CSaTScanData& Data, const AsciiPrintFormat& PrintFormat) const {
  std::vector<double>   vCoordinates, vCoodinatesOfStep;
  float                 nRadius;
  std::string           buffer, work;

  try {
    Data.GetGInfo()->retrieveCoordinates(m_Center, vCoordinates);
    PrintFormat.PrintSectionLabel(fp, "Coordinates", false, true);
    for (size_t t=0; t < vCoordinates.size() - 1; ++t) {
       printString(work, "%s%g,", (t == 0 ? "(" : "" ), vCoordinates[t]);
       buffer += work;
    }
    printString(work, "%g)", vCoordinates.back());
    buffer += work;
    PrintFormat.PrintAlignedMarginsDataString(fp, buffer);
    PrintFormat.PrintSectionLabel(fp, "Radius for each step", false, true);
    buffer = "";
    for (int i=0; i < m_nSteps; ++i) {
       CentroidNeighborCalculator::getTractCoordinates(Data, *this, Data.GetNeighbor(0, m_Center, gvLastNeighborList.at(i)), vCoodinatesOfStep);
       nRadius = (float)sqrt(Data.GetTInfo()->getDistanceSquared(vCoordinates, vCoodinatesOfStep));
       printString(work, "%s%4.2f", (i > 0 ? ", " : ""), nRadius);
       buffer += work;
    }
    PrintFormat.PrintAlignedMarginsDataString(fp, buffer);
  }
  catch (prg_exception& x) {
    x.addTrace("DisplayCoordinates()","CPSMonotoneCluster");
    throw;
  }
}

/** Prints latitude/longitude coordinates of cluster to file pointer in ACSII format. */
void CPSMonotoneCluster::DisplayLatLongCoords(FILE* fp, const CSaTScanData& Data, const AsciiPrintFormat& PrintFormat) const {
  double                        dRadius;
  std::vector<double>           ClusterCenter, vCoodinatesOfStep;
  std::pair<double, double>     prLatitudeLongitude;
  char                          cNorthSouth, cEastWest;
  std::string                   buffer, work;

  try {
    Data.GetGInfo()->retrieveCoordinates(m_Center, ClusterCenter);
    prLatitudeLongitude = ConvertToLatLong(ClusterCenter);
    prLatitudeLongitude.first >= 0 ? cNorthSouth = 'N' : cNorthSouth = 'S';
    prLatitudeLongitude.second >= 0 ? cEastWest = 'W' : cEastWest = 'E';
    PrintFormat.PrintSectionLabel(fp, "Coordinates", false, true);
    fprintf(fp, "(%.6f %c, %.6f %c)\n", fabs(prLatitudeLongitude.first), cNorthSouth, fabs(prLatitudeLongitude.second), cEastWest);
    PrintFormat.PrintSectionLabel(fp, "Radius for each step", false, true);
    for (int i=0; i < m_nSteps; ++i) {
      CentroidNeighborCalculator::getTractCoordinates(Data, *this, Data.GetNeighbor(0, m_Center, gvLastNeighborList.at(i)), vCoodinatesOfStep);
      dRadius = 2 * EARTH_RADIUS_km * asin(sqrt(Data.GetTInfo()->getDistanceSquared(ClusterCenter, vCoodinatesOfStep))/(2 * EARTH_RADIUS_km));
      printString(work, "%s%5.2lf km", (i == 0 ? "(" : "" ), dRadius);
      buffer += work;
    }
    PrintFormat.PrintAlignedMarginsDataString(fp, buffer);
  }
  catch (prg_exception& x) {
    x.addTrace("DisplayLatLongCoords()", "CPSMonotoneCluster");
    throw;
  }
}

/** Prints observed divided by expected and relative risk of cluster to file pointer in ACSII format. */
void CPSMonotoneCluster::DisplayObservedDivExpected(FILE* fp, unsigned int iDataSetIndex, const CSaTScanData& DataHub, const AsciiPrintFormat& PrintFormat) const {
  std::string buffer;

  try {
    CCluster::DisplayObservedDivExpected(fp, iDataSetIndex, DataHub, PrintFormat);
    if (m_nSteps == 1)
      return;
    PrintFormat.PrintSectionLabel(fp, "Relative risk by step", false, true);
    printString(buffer, "%.3f", GetRelativeRisk(iDataSetIndex, DataHub));
    PrintFormat.PrintAlignedMarginsDataString(fp, buffer);
  }
  catch (prg_exception& x) {
    x.addTrace("DisplayObservedDivExpected()","CPSMonotoneCluster");
    throw;
  }
}

/** Returns pointer cluster data object - not implemented, throws exception. */
AbstractClusterData * CPSMonotoneCluster::GetClusterData() {
 throw prg_error("GetClusterData() not implemented.","CPSMonotoneCluster");
}

/** Returns pointer cluster data object - not implemented, throws exception. */
const AbstractClusterData * CPSMonotoneCluster::GetClusterData() const {
 throw prg_error("GetClusterData() not implemented.","CPSMonotoneCluster");
}

/** returns end date of defined cluster as formated string */
std::string& CPSMonotoneCluster::GetEndDate(std::string& sDateString, const CSaTScanData& DataHub) const {
  return JulianToString(sDateString, DataHub.GetTimeIntervalStartTimes()[DataHub.GetNumTimeIntervals()] - 1);
}

/** Returns number of expected cases in accumulated data. */
measure_t CPSMonotoneCluster::GetExpectedCount(const CSaTScanData& DataHub, size_t tSetIndex) const {
  return DataHub.GetMeasureAdjustment(tSetIndex) * m_nMeasure;
}

/** Returns the measure for tract as defined by cluster. */
measure_t CPSMonotoneCluster::GetExpectedCountForTract(tract_t tTractIndex, const CSaTScanData& Data, size_t tSetIndex) const {
  return Data.GetMeasureAdjustment(tSetIndex) * Data.GetDataSetHandler().GetDataSet(tSetIndex).getMeasureData().GetArray()[0][tTractIndex];
}

/** Returns log likelihood of cluster. */
double CPSMonotoneCluster::GetLogLikelihood() const {
  return m_nLogLikelihood;
}

/** returns the number of cases for tract as defined by cluster */
count_t CPSMonotoneCluster::GetObservedCountForTract(tract_t tTractIndex, const CSaTScanData& Data,size_t tSetIndex) const {
  return Data.GetDataSetHandler().GetDataSet(tSetIndex).getCaseData().GetArray()[0][tTractIndex];
}

/** If ratio flag is set, returns log likelihood ratio else returns -1.*/
double CPSMonotoneCluster::GetRatio() const {
  return (m_bRatioSet ? m_nRatio : -1);
}

/** no documentation */
double CPSMonotoneCluster::GetRelativeRisk(tract_t nStep, const CSaTScanData& DataHub) const {
  return ((double)(gvCasesList.at(nStep)))/(gvMeasureList.at(nStep) * DataHub.GetMeasureAdjustment(0));
}

/** returns start date of defined cluster as formated string */
std::string& CPSMonotoneCluster::GetStartDate(std::string& sDateString, const CSaTScanData& DataHub) const {
  return JulianToString(sDateString, DataHub.GetTimeIntervalStartTimes()[0]);
}

/** initialize cluster data and data members */
void CPSMonotoneCluster::Initialize(tract_t nCenter) {
  CCluster::Initialize(nCenter);
  m_bRatioSet = false;
  m_nCases = 0;
  m_nMeasure = 0;
  m_nLogLikelihood = 0;
  m_nSteps = 0;
  std::fill(gvCasesList.begin(), gvCasesList.end(), 0);
  std::fill(gvMeasureList.begin(), gvMeasureList.end(), 0);
  std::fill(gvFirstNeighborList.begin(), gvFirstNeighborList.end(), 0);
  std::fill(gvLastNeighborList.begin(), gvLastNeighborList.end(), 0);
}

/** Prints name and coordinates of locations contained in cluster to ASCII file.
    Note: This is a debug function and can be helpful when used with Excel to get
    visual of cluster using scatter plotting. */
void CPSMonotoneCluster::PrintClusterLocationsToFile(const CSaTScanData& DataHub, const std::string& sFilename) const {
  tract_t                       i, tTract;
  std::ofstream                 outfilestream(sFilename.c_str(), std::ios::ate);

  try {
    if (!outfilestream)
      throw prg_error("Error: Could not open file for write:'%s'.\n", "PrintClusterLocationsToFile()", sFilename.c_str());

    outfilestream.setf(std::ios_base::fixed, std::ios_base::floatfield);

    std::vector<double> vCoords;
    if (DataHub.GetParameters().UseSpecialGrid()) {
      DataHub.GetGInfo()->retrieveCoordinates(GetCentroidIndex(), vCoords);
      outfilestream << "Central_Grid_Point";
      for (size_t t=0; t < vCoords.size(); ++t)
       outfilestream << " " << vCoords.at(t);
      outfilestream << std::endl;
    }

    for (int s=0; s < m_nSteps; ++s) {
      for (i=gvFirstNeighborList.at(i); i <= gvLastNeighborList.at(i); ++i) {
         tTract = DataHub.GetNeighbor(m_iEllipseOffset, m_Center, i, m_CartesianRadius);
         // Print location identifiers if location data has not been removed in iterative scan.
         if (!DataHub.GetIsNullifiedLocation(tTract)) {
           CentroidNeighborCalculator::getTractCoordinates(DataHub, *this, tTract, vCoords);
           outfilestream << DataHub.GetTInfo()->getLocations().at(tTract)->getIndentifier();
           for (size_t t=0; t < vCoords.size(); ++t)
             outfilestream << " " << vCoords.at(t);
           outfilestream << std::endl;
         }
      }
    }
    outfilestream << std::endl;
  }
  catch (prg_exception& x) {
    x.addTrace("PrintClusterLocationsToFile()","CPSMonotoneCluster");
    throw;
  }
}

/** no documentation */
void CPSMonotoneCluster::RemoveRemainder() {
  m_nSteps--;
  gvCasesList.at(m_nSteps) = 0;
  gvMeasureList.at(m_nSteps) = 0;
  gvFirstNeighborList.at(m_nSteps) = 0;
  gvLastNeighborList.at(m_nSteps) = 0;

  if (m_nSteps==0)
    Initialize(m_Center);
}

/** no documentation */
void CPSMonotoneCluster::SetCasesAndMeasures() {
  m_nCases   = 0;
  m_nMeasure = 0;

  for (int i=0; i < m_nSteps; ++i) {
     m_nCases   += gvCasesList.at(i);
     m_nMeasure += gvMeasureList.at(i);
  }
}

/** no documentation - not invoked */
double CPSMonotoneCluster::SetLogLikelihood() {
  m_nLogLikelihood = 0;

  for (int i=0; i<m_nSteps; i++) {
    if (gvCasesList.at(i) != 0)
      m_nLogLikelihood += gvCasesList.at(i) * log(gvCasesList.at(i)/gvMeasureList.at(i));
  }

  return m_nLogLikelihood;
}

/** Sets scanning area rate. */
void CPSMonotoneCluster::SetRate(int nRate) {
  switch (nRate) {
    case HIGH       : m_pfRateOfInterest = HighRate;      break;
    case LOW        : m_pfRateOfInterest = LowRate;       break;
    case HIGHANDLOW : m_pfRateOfInterest = HighOrLowRate; break;
    default         : ;
  }
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
    m_nTracts += gvLastNeighborList.at(i) - gvFirstNeighborList.at(i) + 1;
}

/** Writes cluster data to passed record buffer. */
void CPSMonotoneCluster::Write(LocationInformationWriter& LocationWriter, const CSaTScanData& Data,
                     unsigned int iClusterNumber, unsigned int iNumSimsCompleted) const {
  tract_t       t, tTract;
  int           i;

  try {
    for (i=0; i < m_nSteps; ++i) {
       for (t=gvFirstNeighborList.at(i); t <= gvLastNeighborList.at(i); t++) {
         tTract = Data.GetNeighbor(m_iEllipseOffset, m_Center, t);
         LocationWriter.Write(*this, Data, iClusterNumber, tTract, iNumSimsCompleted);
       }
    }
  }
  catch (prg_exception& x) {
    x.addTrace("Write(stsAreaSpecificData*)","CPSMonotoneCluster");
    throw;
  }
}

