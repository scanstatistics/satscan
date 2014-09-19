//*****************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//*****************************************************************************
#include "PurelySpatialMonotoneCluster.h"
#include "ClusterLocationsWriter.h"
#include "SSException.h"

//**************** class CPSMonotoneCluster ************************************

/** class constructor - const AbstractDataSetGateway */
CPSMonotoneCluster::CPSMonotoneCluster(const AbstractClusterDataFactory * pClusterFactory,
                                       const AbstractDataSetGateway & DataGateway, AreaRateType eRate)
                   :CCluster() {
  //The last time interval for a purely spatial cluster equals the number of
  //calculated time intervals. This would be 1 for a purely spatial analysis but
  //for a space-time analysis, the index would be dependent on # of intervals requested.
  m_nLastInterval = DataGateway.GetDataSetInterface().GetNumTimeIntervals();
  gpClusterData.reset(new SpatialMonotoneData(DataGateway));
  gpClusterData->SetRate(eRate);
}

/** class constructor - const DataSetInterface */
CPSMonotoneCluster::CPSMonotoneCluster(const AbstractClusterDataFactory * pClusterFactory,
                                       const DataSetInterface & Interface, AreaRateType eRate)
                   :CCluster() {
  //The last time interval for a purely spatial cluster equals the number of
  //calculated time intervals. This would be 1 for a purely spatial analysis but
  //for a space-time analysis, the index would be dependent on # of intervals requested.
  m_nLastInterval = Interface.GetNumTimeIntervals();
  gpClusterData.reset(new SpatialMonotoneData(Interface));
  gpClusterData->SetRate(eRate);
}

/** class copy constructor */
CPSMonotoneCluster::CPSMonotoneCluster(const CPSMonotoneCluster& rhs)
                   :CCluster(rhs) {
  gpClusterData.reset(rhs.gpClusterData->Clone());                 
  *this = rhs;
}

/** class destructor */
CPSMonotoneCluster::~CPSMonotoneCluster() {}

/** overloaded assigment operator */
CPSMonotoneCluster& CPSMonotoneCluster::operator=(const CPSMonotoneCluster& rhs) {
  try {
    m_Center              = rhs.m_Center;
    m_MostCentralLocation = rhs.m_MostCentralLocation;
    m_nTracts             = rhs.m_nTracts;
    m_nRatio              = rhs.m_nRatio;
    m_nRank               = rhs.m_nRank;
    m_nFirstInterval      = rhs.m_nFirstInterval;
    m_nLastInterval       = rhs.m_nLastInterval;
    gpClusterData->Assign(*(rhs.gpClusterData));
  }
  catch (prg_exception& x) {
    x.addTrace("operator=","CPSMonotoneCluster");
    throw;
  }
  return *this;
}

/** Adds neighbor location data from DataGateway to cluster data accumulation and
    calculates loglikelihood ratio. If ratio is greater than that of TopCluster,
    assigns TopCluster to 'this'. */
void CPSMonotoneCluster::CalculateTopClusterAboutCentroidDefinition(const AbstractDataSetGateway& DataGateway,
                                                                    const CentroidNeighbors& CentroidDef,
                                                                    CClusterSet& clusterSet,
                                                                    AbstractLikelihoodCalculator& Calculator) {
  tract_t               t, tNumNeighbors = CentroidDef.GetNumNeighbors(),
                      * pIntegerArray = CentroidDef.GetRawIntegerArray();
  unsigned short      * pUnsignedShortArray = CentroidDef.GetRawUnsignedShortArray();

  for (t=0; t < tNumNeighbors; ++t) {
    //update cluster data
    gpClusterData->AddNeighborData((pUnsignedShortArray ? (tract_t)pUnsignedShortArray[t] : pIntegerArray[t]), DataGateway);
    gpClusterData->CheckCircle(gpClusterData->GetLastCircleIndex());
  }
  gpClusterData->AddRemainder(Calculator.GetDataHub());
  m_nRatio = gpClusterData->CalculateLoglikelihoodRatio(Calculator);
  gpClusterData->RemoveRemainder(Calculator.GetDataHub());
  if (gpClusterData->m_nSteps == 0) Initialize(m_Center);

  //TODO: This implies/requires no index based cluster reporting. This feature might come at a later date.
  CClusterObject& clusterObj = clusterSet.get(0);
  if (m_nRatio > clusterObj.getCluster().GetRatio()) {
    gpClusterData->SetCasesAndMeasures(); //re-calculate case and measure for outer tracts absorber into remainder
    SetTotalTracts();                     //re-calculate total tracts for outer tracts absorber into remainder
    clusterObj.getCluster().CopyEssentialClassMembers(*this);
  }
}

/** Adds neighbor location data from DataGateway to cluster data accumulation and
    calculates loglikelihood ratio. If ratio is greater than that of TopCluster,
    assigns TopCluster to 'this'. */
/*void CPSMonotoneCluster::CalculateTopClusterAboutCentroidDefinition(const AbstractDataSetGateway& DataGateway,
                                                                    const CentroidNeighbors& CentroidDef,
                                                                    CPSMonotoneCluster& TopCluster,
                                                                    AbstractLikelihoodCalculator& Calculator) {
  tract_t               t, tNumNeighbors = CentroidDef.GetNumNeighbors(),
                      * pIntegerArray = CentroidDef.GetRawIntegerArray();
  unsigned short      * pUnsignedShortArray = CentroidDef.GetRawUnsignedShortArray();

  for (t=0; t < tNumNeighbors; ++t) {
    //update cluster data
    gpClusterData->AddNeighborData((pUnsignedShortArray ? (tract_t)pUnsignedShortArray[t] : pIntegerArray[t]), DataGateway);
    gpClusterData->CheckCircle(gpClusterData->GetLastCircleIndex());
  }
  gpClusterData->AddRemainder(Calculator.GetDataHub());
  m_nRatio = gpClusterData->CalculateLoglikelihoodRatio(Calculator);
  gpClusterData->RemoveRemainder(Calculator.GetDataHub());
  if (gpClusterData->m_nSteps == 0) Initialize(m_Center);
  if (m_nRatio > TopCluster.m_nRatio) {
    gpClusterData->SetCasesAndMeasures(); //re-calculate case and measure for outer tracts absorber into remainder
    SetTotalTracts();                     //re-calculate total tracts for outer tracts absorber into remainder
    TopCluster.CopyEssentialClassMembers(*this);
  }
}*/

/** returns newly cloned CPSMonotoneCluster */
CPSMonotoneCluster * CPSMonotoneCluster::Clone() const {
  return new CPSMonotoneCluster(*this);
}

/** Prints locations of cluster, detailed by step, to file pointer in ACSII format. */
void CPSMonotoneCluster::DisplayCensusTracts(FILE* fp, const CSaTScanData& Data, const AsciiPrintFormat& PrintFormat) const {
  std::string buffer;

  try {
    PrintFormat.PrintSectionLabel(fp, "Steps in risk function", false, true);
    fprintf(fp, "%i\n", gpClusterData->m_nSteps);
    for (int i=0; i < gpClusterData->m_nSteps; ++i) {
       printString(buffer, "  Step %i",i + 1);
       PrintFormat.PrintSectionLabel(fp, buffer.c_str(), false, true);
       DisplayCensusTractsInStep(fp, Data, gpClusterData->gvFirstNeighborList[i], gpClusterData->gvLastNeighborList[i], PrintFormat);
    }
  }
  catch (prg_exception& x) {
    x.addTrace("DisplayCensusTracts()","CPSMonotoneCluster");
    throw;
  }
}

/** Prints cartesian coordinates of cluster to file pointer in ACSII format. */
void CPSMonotoneCluster::DisplayCoordinates(FILE* fp, const CSaTScanData& Data, const AsciiPrintFormat& PrintFormat) const {
    std::vector<double> vCoordinates, vCoodinatesOfStep;
    float nRadius;
    std::string buffer, work;

    try {
        // print coordinates for cluster
        Data.GetGInfo()->retrieveCoordinates(m_Center, vCoordinates);
        for (size_t t=0; t < vCoordinates.size() - 1; ++t) {
        printString(work, "%s%g,", (t == 0 ? "(" : "" ), vCoordinates[t]);
        buffer += work;
        }
        printString(work, "%g)", vCoordinates.back());
        buffer += work;
        printClusterData(fp, PrintFormat, "Coordinates", buffer, false);
        // print radius for each step
        buffer = "";
        for (int i=0; i < gpClusterData->m_nSteps; ++i) {
            CentroidNeighborCalculator::getTractCoordinates(Data, *this, Data.GetNeighbor(0, m_Center, gpClusterData->gvLastNeighborList[i]), vCoodinatesOfStep);
            nRadius = (float)sqrt(Data.GetTInfo()->getDistanceSquared(vCoordinates, vCoodinatesOfStep));
        printString(work, "%s%4.2f", (i > 0 ? ", " : ""), nRadius);
        buffer += work;
        }
        printClusterData(fp, PrintFormat, "Radius for each step", buffer, false);
    } catch (prg_exception& x) {
        x.addTrace("DisplayCoordinates()","CPSMonotoneCluster");
        throw;
    }
}

/** Prints latitude/longitude coordinates of cluster to file pointer in ACSII format. */
void CPSMonotoneCluster::DisplayLatLongCoords(FILE* fp, const CSaTScanData& Data, const AsciiPrintFormat& PrintFormat) const {
    double dRadius;
    std::vector<double> ClusterCenter, vCoodinatesOfStep;
    std::pair<double, double> prLatitudeLongitude;
    char cNorthSouth, cEastWest;
    std::string buffer, work;

    try {
        // print coordinates for cluster
        Data.GetGInfo()->retrieveCoordinates(m_Center, ClusterCenter);
        prLatitudeLongitude = ConvertToLatLong(ClusterCenter);
        prLatitudeLongitude.first >= 0 ? cNorthSouth = 'N' : cNorthSouth = 'S';
        prLatitudeLongitude.second >= 0 ? cEastWest = 'W' : cEastWest = 'E';
        printString(buffer, "(%.6f %c, %.6f %c)\n", fabs(prLatitudeLongitude.first), cNorthSouth, fabs(prLatitudeLongitude.second), cEastWest);
        printClusterData(fp, PrintFormat, "Coordinates", buffer, false);
        // print radius for each step
        for (int i=0; i < gpClusterData->m_nSteps; ++i) {
            CentroidNeighborCalculator::getTractCoordinates(Data, *this, Data.GetNeighbor(0, m_Center, gpClusterData->gvLastNeighborList[i]), vCoodinatesOfStep);
            dRadius = 2 * EARTH_RADIUS_km * asin(sqrt(Data.GetTInfo()->getDistanceSquared(ClusterCenter, vCoodinatesOfStep))/(2 * EARTH_RADIUS_km));
            printString(work, "%s%5.2lf km", (i == 0 ? "(" : "" ), dRadius);
            buffer += work;
        }
        printClusterData(fp, PrintFormat, "Radius for each step", buffer, false);
    } catch (prg_exception& x) {
        x.addTrace("DisplayLatLongCoords()", "CPSMonotoneCluster");
        throw;
    }
}

/** Prints observed divided by expected and relative risk of cluster to file pointer in ACSII format. */
void CPSMonotoneCluster::DisplayObservedDivExpected(FILE* fp, unsigned int iDataSetIndex, const CSaTScanData& DataHub, const AsciiPrintFormat& PrintFormat) const {
    std::string buffer;

    try {
        CCluster::DisplayObservedDivExpected(fp, iDataSetIndex, DataHub, PrintFormat);
        if (gpClusterData->m_nSteps == 1)
            return;
        printString(buffer, "%.3f", GetRelativeRisk(iDataSetIndex, DataHub));
        printClusterData(fp, PrintFormat, "Relative risk by step", buffer, false);
    } catch (prg_exception& x) {
        x.addTrace("DisplayObservedDivExpected()","CPSMonotoneCluster");
        throw;
    }
}

/** returns end date of defined cluster as formated string */
std::string& CPSMonotoneCluster::GetEndDate(std::string& sDateString, const CSaTScanData& DataHub, const char * sep) const {
  DatePrecisionType eDatePrint = (DataHub.GetParameters().GetPrecisionOfTimesType() == GENERIC ? GENERIC : DAY);  
  return JulianToString(sDateString, DataHub.GetTimeIntervalStartTimes()[DataHub.GetNumTimeIntervals()] - 1, eDatePrint, sep);
}

/** Returns number of expected cases in accumulated data. */
measure_t CPSMonotoneCluster::GetExpectedCount(const CSaTScanData& DataHub, size_t tSetIndex) const {
  return DataHub.GetMeasureAdjustment(tSetIndex) * gpClusterData->GetMeasure(tSetIndex);
}

/** Returns the measure for tract as defined by cluster. */
measure_t CPSMonotoneCluster::GetExpectedCountForTract(tract_t tTractIndex, const CSaTScanData& Data, size_t tSetIndex) const {
  return Data.GetMeasureAdjustment(tSetIndex) * Data.GetDataSetHandler().GetDataSet(tSetIndex).getMeasureData().GetArray()[0][tTractIndex];
}

/** returns the number of cases for tract as defined by cluster */
count_t CPSMonotoneCluster::GetObservedCountForTract(tract_t tTractIndex, const CSaTScanData& Data,size_t tSetIndex) const {
  return Data.GetDataSetHandler().GetDataSet(tSetIndex).getCaseData().GetArray()[0][tTractIndex];
}

/** no documentation */
double CPSMonotoneCluster::GetRelativeRisk(tract_t nStep, const CSaTScanData& DataHub) const {
  return ((double)(gpClusterData->gvCasesList[nStep]))/(gpClusterData->gvMeasureList[nStep] * DataHub.GetMeasureAdjustment(0));
}

/** returns start date of defined cluster as formated string */
std::string& CPSMonotoneCluster::GetStartDate(std::string& sDateString, const CSaTScanData& DataHub, const char * sep) const {
  DatePrecisionType eDatePrint = (DataHub.GetParameters().GetPrecisionOfTimesType() == GENERIC ? GENERIC : DAY);  
  return JulianToString(sDateString, DataHub.GetTimeIntervalStartTimes()[0], eDatePrint, sep);
}

/** initialize cluster data and data members */
void CPSMonotoneCluster::Initialize(tract_t nCenter) {
  m_Center         = nCenter;
  m_MostCentralLocation = -1;
  m_nTracts        = 0;
  m_CartesianRadius= -1;
  m_nRatio         = 0;
  m_nRank          = 1;
  m_NonCompactnessPenalty = 1;
  m_iEllipseOffset = 0;
  gpClusterData->InitializeData();
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
       outfilestream << " " << vCoords[t];
      outfilestream << std::endl;
    }

    for (int s=0; s < gpClusterData->m_nSteps; ++s) {
      for (i=gpClusterData->gvFirstNeighborList[i]; i <= gpClusterData->gvLastNeighborList[i]; ++i) {
         tTract = DataHub.GetNeighbor(m_iEllipseOffset, m_Center, i, m_CartesianRadius);
         // Print location identifiers if location data has not been removed in iterative scan.
         if (!DataHub.GetIsNullifiedLocation(tTract)) {
           CentroidNeighborCalculator::getTractCoordinates(DataHub, *this, tTract, vCoords);
           outfilestream << DataHub.GetTInfo()->getLocations()[tTract]->getIndentifier();
           for (size_t t=0; t < vCoords.size(); ++t)
             outfilestream << " " << vCoords[t];
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

/** Returns the total number of tracts in cluster, across all steps. */
void CPSMonotoneCluster::SetTotalTracts() {
  m_nTracts = 0;
  for (int i=0; i < gpClusterData->m_nSteps; ++i)
    m_nTracts += gpClusterData->gvLastNeighborList[i] - gpClusterData->gvFirstNeighborList[i] + 1;
}

/** Writes cluster data to passed record buffer. */
void CPSMonotoneCluster::Write(LocationInformationWriter& LocationWriter,
                               const CSaTScanData& Data,
                               unsigned int iClusterNumber,
                               const SimulationVariables& simVars,
                               const Relevance_Container_t& location_relevance) const {
  tract_t       t, tTract;
  int           i;

  try {
    for (i=0; i < gpClusterData->m_nSteps; ++i) {
       for (t=gpClusterData->gvFirstNeighborList[i]; t <= gpClusterData->gvLastNeighborList[i]; t++) {
         tTract = Data.GetNeighbor(m_iEllipseOffset, m_Center, t);
         LocationWriter.Write(*this, Data, iClusterNumber, tTract, simVars, location_relevance);
       }
    }
  }
  catch (prg_exception& x) {
    x.addTrace("Write(stsAreaSpecificData*)","CPSMonotoneCluster");
    throw;
  }
}

