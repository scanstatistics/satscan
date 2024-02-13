//*****************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//*****************************************************************************
#include "PurelySpatialMonotoneCluster.h"
#include "ClusterLocationsWriter.h"
#include "SSException.h"
#include "ClusterSupplement.h"

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
    _central_identifier   = rhs._central_identifier;
    _num_identifiers      = rhs._num_identifiers;
    m_nRatio              = rhs.m_nRatio;
    _ratio_sets           = rhs._ratio_sets;
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
  CClusterObject& clusterObj = clusterSet.getSet().front();
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
    PrintFormat.PrintSectionLabel(fp, "Steps in risk function", false, false);
    PrintFormat.PrintAlignedMarginsDataString(fp, printString(buffer, "%i", gpClusterData->m_nSteps));
    for (int i=0; i < gpClusterData->m_nSteps; ++i) {
       printString(buffer, "Step %i",i + 1);
       PrintFormat.PrintSectionLabel(fp, buffer.c_str(), false, true);
       DisplayCensusTractsInStep(fp, Data, gpClusterData->gvFirstNeighborList[i], gpClusterData->gvLastNeighborList[i], PrintFormat);
    }
  }
  catch (prg_exception& x) {
    x.addTrace("DisplayCensusTracts()","CPSMonotoneCluster");
    throw;
  }
}

/** Writes clusters location information in format required by result output file. */
void CPSMonotoneCluster::DisplayCensusTractsInStep(FILE* fp, const CSaTScanData& DataHub, tract_t nFirstTract, tract_t nLastTract, const AsciiPrintFormat& PrintFormat) const {

    // TODO: Revise for multiple coordinates - would this option ever be used with that option?

    std::string locations;
    Identifier::CombinedIdentifierNames_t vTractIdentifiers;
    try {
        for (tract_t t=nFirstTract; t <= nLastTract; ++t) {
            //get i'th neighbor tracts index
            tract_t tTract = DataHub.GetNeighbor(m_iEllipseOffset, m_Center, t, m_CartesianRadius);
            // Print location identifiers if location data has not been removed in iterative scan.
            if (!DataHub.isNullifiedIdentifier(tTract)) {
                //get all locations ids for tract at index tTract -- might be more than one if combined
                DataHub.getIdentifierInfo().retrieveAll(tTract, vTractIdentifiers);
                for (unsigned int i=0; i < vTractIdentifiers.size(); ++i) {
                    if (locations.size()) locations += ", ";
                    locations += vTractIdentifiers[i].c_str();
                }
            }
        }
        // There should be at least one location printed, else there is likely a bug in the iterative scan code.
        if (!locations.size()) throw prg_error("Attempting to print cluster with no location identifiers.", "DisplayCensusTractsInStep()");
        PrintFormat.PrintAlignedMarginsDataString(fp, locations);
    } catch (prg_exception& x) {
        x.addTrace("DisplayCensusTractsInStep()", "CPSMonotoneCluster");
        throw;
    }
}


/** Prints cartesian coordinates of cluster to file pointer in ACSII format. */
void CPSMonotoneCluster::DisplayCoordinates(FILE* fp, const CSaTScanData& Data, const AsciiPrintFormat& PrintFormat) const {
    std::vector<double> vCoordinates, vCoodinatesOfStep;
    float nRadius;
    std::string buffer, work, work2;

    try {
        // print coordinates for cluster
        Data.GetGInfo()->retrieveCoordinates(m_Center, vCoordinates);
        for (size_t t=0; t < vCoordinates.size() - 1; ++t) {
            printString(work, "%s%g, ", (t == 0 ? "(" : "" ), vCoordinates[t]);
            buffer += work;
        }
        // get coordinates of outer most step to calculate the cluster's radius
        CentroidNeighborCalculator::getTractCoordinates(Data, *this, Data.GetNeighbor(0, m_Center, gpClusterData->gvLastNeighborList[gpClusterData->m_nSteps - 1]), vCoodinatesOfStep);
        nRadius = (float)Coordinates::distanceBetween(vCoordinates, vCoodinatesOfStep);
        buffer += printString(work, "%g) / %s", vCoordinates.back(), getValueAsString(nRadius, work2).c_str());
        printClusterData(fp, PrintFormat, "Coordinates / radius", buffer, false);
    } catch (prg_exception& x) {
        x.addTrace("DisplayCoordinates()","CPSMonotoneCluster");
        throw;
    }
}

/* Extends cluster print to include step wise information. */
void CPSMonotoneCluster::Display(FILE* fp, const CSaTScanData& DataHub, const ClusterSupplementInfo& supplementInfo, const SimulationVariables& simVars) const {
    CCluster::Display(fp, DataHub, supplementInfo, simVars);
    // now print step-wise information
    AsciiPrintFormat PrintFormat = getAsciiPrintFormat();
    PrintFormat.SetMarginsAsClusterSection(supplementInfo.getClusterReportIndex(*this));
    if (1 < gpClusterData->m_nSteps) {
        PrintFormat.PrintSectionLabel(fp, "Isotonic step-wise information", false, true);
        fprintf(fp, "\n");

        std::string buffer, work, work2;
        std::vector<double> ClusterCenter, vCoodinatesOfStep;

        // print radius for each step
        DataHub.GetGInfo()->retrieveCoordinates(m_Center, ClusterCenter);
        switch (DataHub.GetParameters().GetCoordinatesType()) {
            case LATLON: {
                for (int i=0; i < gpClusterData->m_nSteps; ++i) {
                    CentroidNeighborCalculator::getTractCoordinates(DataHub, *this, DataHub.GetNeighbor(0, m_Center, gpClusterData->gvLastNeighborList[i]), vCoodinatesOfStep);
                    double dRadius = 2 * EARTH_RADIUS_km * asin(Coordinates::distanceBetween(ClusterCenter, vCoodinatesOfStep)/(2 * EARTH_RADIUS_km));
                    printString(work, "%s km%s", getValueAsString(dRadius, work2).c_str(), (i < gpClusterData->m_nSteps - 1 ? ", " : "" ));
                    buffer += work;
                } 
            } break;
            case CARTESIAN : {
                for (int i=0; i < gpClusterData->m_nSteps; ++i) {
                    CentroidNeighborCalculator::getTractCoordinates(DataHub, *this, DataHub.GetNeighbor(0, m_Center, gpClusterData->gvLastNeighborList[i]), vCoodinatesOfStep);
                    double nRadius = Coordinates::distanceBetween(ClusterCenter, vCoodinatesOfStep);
                    printString(work, "%s%s", getValueAsString(nRadius, work2).c_str(), (i < gpClusterData->m_nSteps - 1 ? ", " : "" ));
                    buffer += work;
                }
            } break;
            default : throw prg_error("Unknown coordinates type (%d)", "Display()", DataHub.GetParameters().GetCoordinatesType());
        }
        printClusterData(fp, PrintFormat, "Radius for each step", buffer, false);
        //print observed cases in each step
        buffer = "";
        for (int step=0; 1 < gpClusterData->m_nSteps && step < gpClusterData->m_nSteps; ++step) {
            printString(work, "%ld%s", GetObservedCountForStep(step), (step < gpClusterData->m_nSteps - 1 ? ", " : "" ));
            buffer += work;
        }
        printClusterData(fp, PrintFormat, "Number of cases by step", buffer, false);
        //print expected cases in each step
        buffer = "";
        for (int step=0; 1 < gpClusterData->m_nSteps && step < gpClusterData->m_nSteps; ++step) {
            printString(work, "%s%s", getValueAsString(GetExpectedCountForStep(step, DataHub), work2).c_str(), (step < gpClusterData->m_nSteps - 1 ? ", " : "" ));
            buffer += work;
        }
        printClusterData(fp, PrintFormat, "Expected cases by step", buffer, false);
        //print observed divided by expected in each step
        buffer = "";
        for (int step=0; 1 < gpClusterData->m_nSteps && step < gpClusterData->m_nSteps; ++step) {
            printString(work, "%s%s", getValueAsString(GetObservedDivExpectedForStep(step, DataHub), work2).c_str(), (step < gpClusterData->m_nSteps - 1 ? ", " : "" ));
            buffer += work;
        }
        printClusterData(fp, PrintFormat, "Observed/expected by step", buffer, false);
        //print relative risks in each step
        buffer = "";
        for (int step=0; 1 < gpClusterData->m_nSteps && step < gpClusterData->m_nSteps; ++step) {
            printString(work, "%s%s", getValueAsString(GetRelativeRiskForStep(step, DataHub), work2).c_str(), (step < gpClusterData->m_nSteps - 1 ? ", " : "" ));
            buffer += work;
        }
        printClusterData(fp, PrintFormat, "Relative risk by step", buffer, false);
    }
}

/** Prints population, observed cases, expected cases and relative risk
    to file stream is in format required by result output file. */
void CPSMonotoneCluster::DisplayClusterDataStandard(FILE* fp, const CSaTScanData& DataHub, const AsciiPrintFormat& PrintFormat) const {
    std::string buffer, work, work2;

    DisplayPopulation(fp, DataHub, PrintFormat);
    //print observed cases in entire cluster
    printClusterData(fp, PrintFormat, "Number of cases", printString(buffer, "%ld", GetObservedCount()), true);
    //print expected cases in entire cluster
    printClusterData(fp, PrintFormat, "Expected cases", getValueAsString(GetExpectedCount(DataHub), buffer), true);
    DisplayAnnualCaseInformation(fp, 0, DataHub, PrintFormat);
    //print expected cases in entire cluster
    CCluster::DisplayObservedDivExpected(fp, 0, DataHub, PrintFormat);
    //print relative risks in entire cluster
    CCluster::DisplayRelativeRisk(fp, 0, DataHub, PrintFormat);
    if (DataHub.GetParameters().GetProbabilityModelType() == BERNOULLI) {
        //percent cases in an area
        double percentCases = 100.0 * GetObservedCount() / DataHub.GetProbabilityModel().GetPopulation(0, *this, DataHub);
        printClusterData(fp, PrintFormat, "Percent cases in area", getValueAsString(percentCases, buffer,1), true);
    }
}

/* Returns observed divided by expected for this step. */
double CPSMonotoneCluster::GetObservedDivExpectedForStep(tract_t step, const CSaTScanData& DataHub) const {
    measure_t expected = GetExpectedCountForStep(step, DataHub);
    return (expected ? static_cast<double>(GetObservedCountForStep(step))/expected : 0.0);
}

/** Prints latitude/longitude coordinates of cluster to file pointer in ACSII format. */
void CPSMonotoneCluster::DisplayLatLongCoords(FILE* fp, const CSaTScanData& Data, const AsciiPrintFormat& PrintFormat) const {
    double dRadius;
    std::vector<double> ClusterCenter, vCoodinatesOfStep;
    std::pair<double, double> prLatitudeLongitude;
    char cNorthSouth, cEastWest;
    std::string buffer, work, work2;

    try {
        // print coordinates for cluster
        Data.GetGInfo()->retrieveCoordinates(m_Center, ClusterCenter);
        prLatitudeLongitude = ConvertToLatLong(ClusterCenter);
        prLatitudeLongitude.first >= 0 ? cNorthSouth = 'N' : cNorthSouth = 'S';
        prLatitudeLongitude.second >= 0 ? cEastWest = 'W' : cEastWest = 'E';
        printString(buffer, "(%.6f %c, %.6f %c)", fabs(prLatitudeLongitude.first), cNorthSouth, fabs(prLatitudeLongitude.second), cEastWest);

        // get coordinates of outer most step to calculate the cluster's radius
        CentroidNeighborCalculator::getTractCoordinates(Data, *this, Data.GetNeighbor(0, m_Center, gpClusterData->gvLastNeighborList[gpClusterData->m_nSteps - 1]), vCoodinatesOfStep);
        dRadius = 2 * EARTH_RADIUS_km * asin(Coordinates::distanceBetween(ClusterCenter, vCoodinatesOfStep)/(2 * EARTH_RADIUS_km));
        buffer += printString(work, " / %s km", getValueAsString(dRadius, work2).c_str());
        printClusterData(fp, PrintFormat, "Coordinates / radius", buffer, false);
    } catch (prg_exception& x) {
        x.addTrace("DisplayLatLongCoords()", "CPSMonotoneCluster");
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
measure_t CPSMonotoneCluster::GetExpectedCountForTract(tract_t tTractIndex, const CSaTScanData& Data, size_t tSetIndex, bool adjusted) const {
  return Data.GetMeasureAdjustment(tSetIndex) * Data.GetDataSetHandler().GetDataSet(tSetIndex).getMeasureData().GetArray()[0][tTractIndex];
}

/** returns the number of cases for tract as defined by cluster */
count_t CPSMonotoneCluster::GetObservedCountForTract(tract_t tTractIndex, const CSaTScanData& Data,size_t tSetIndex) const {
  return Data.GetDataSetHandler().GetDataSet(tSetIndex).getCaseData().GetArray()[0][tTractIndex];
}

/** Returns the relative risk for this step. */
double CPSMonotoneCluster::GetRelativeRiskForStep(tract_t nStep, const CSaTScanData& DataHub) const {
    //ODE inside step divided by ODE outside cluster

    double total_cases = static_cast<double>(DataHub.GetDataSetHandler().GetDataSet(0).getTotalCases());
    double observed_in_step = static_cast<double>(gpClusterData->gvCasesList[nStep]);
    double expected_in_step = gpClusterData->gvMeasureList[nStep] * DataHub.GetMeasureAdjustment(0);
    if (total_cases == observed_in_step) return -1;

    double observed_in_cluster = static_cast<double>(GetObservedCount());
    double expected_in_cluster = GetExpectedCount(DataHub, 0);

    if (expected_in_step && total_cases - expected_in_step && ((total_cases - observed_in_step)/(total_cases - expected_in_step)))
        return (observed_in_step/expected_in_step)/((total_cases - observed_in_cluster)/(total_cases - expected_in_cluster));
    return 0;
}

/** returns start date of defined cluster as formated string */
std::string& CPSMonotoneCluster::GetStartDate(std::string& sDateString, const CSaTScanData& DataHub, const char * sep) const {
  DatePrecisionType eDatePrint = (DataHub.GetParameters().GetPrecisionOfTimesType() == GENERIC ? GENERIC : DAY);  
  return JulianToString(sDateString, DataHub.GetTimeIntervalStartTimes()[0], eDatePrint, sep);
}

/** initialize cluster data and data members */
void CPSMonotoneCluster::Initialize(tract_t nCenter) {
  m_Center         = nCenter;
  _central_identifier = -1;
  _num_identifiers        = 0;
  m_CartesianRadius= -1;
  m_nRatio         = 0;
  m_nRank          = 1;
  m_NonCompactnessPenalty = 1;
  m_iEllipseOffset = 0;
  gpClusterData->InitializeData();
}

/** Returns the total number of tracts in cluster, across all steps. */
void CPSMonotoneCluster::SetTotalTracts() {
    _num_identifiers = 0;
    for (int i=0; i < gpClusterData->m_nSteps; ++i)
        _num_identifiers += gpClusterData->gvLastNeighborList[i] - gpClusterData->gvFirstNeighborList[i] + 1;
}

/** Writes cluster data to passed record buffer. */
void CPSMonotoneCluster::Write(
    LocationInformationWriter& LocationWriter, const CSaTScanData& Data, unsigned int iClusterNumber, const SimulationVariables& simVars, const LocationRelevance& location_relevance
) const {
    try {
        // TODO: I'm not sure how to revise this with the multiple locations update and uncertain whether it's worth the time for this little used analysis type.
        throw prg_error("Writing of Location Information is not currently implemented for monotone analyses.", "CPSMonotoneCluster::Write()");
        /*for (int i=0; i < gpClusterData->m_nSteps; ++i) {
            for (tract_t t=gpClusterData->gvFirstNeighborList[i]; t <= gpClusterData->gvLastNeighborList[i]; t++) {
                tract_t tTract = Data.GetNeighbor(m_iEllipseOffset, m_Center, t);
                LocationWriter.Write(*this, Data, iClusterNumber, tTract, simVars, location_relevance);
            }
        }*/
    } catch (prg_exception& x) {
        x.addTrace("Write(stsAreaSpecificData*)","CPSMonotoneCluster");
        throw;
    }
}
