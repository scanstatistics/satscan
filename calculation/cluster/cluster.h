//*****************************************************************************
#ifndef __CLUSTER_H
#define __CLUSTER_H
//*****************************************************************************
#include "SaTScan.h"
#include "SaTScanData.h"
#include "IncidentRate.h"
#include "DataSet.h"
#include "UtilityFunctions.h"
#include "ClusterDataFactory.h"
#include "AsciiPrintFormat.h"
#include "SimulationVariables.h"
#include "boost/tuple/tuple.hpp"
#include "boost/logic/tribool.hpp"
#include "ClusterLocationsWriter.h"

class LocationInformationWriter; // forward class declaration
class ClusterSupplementInfo; // forward class declaration

/** Defines properties of each potential cluster evaluated by analysis. */
class CCluster {
  public:
    typedef std::pair<double,double> RecurrenceInterval_t;
    typedef std::deque<std::pair<std::string, std::pair<std::string, unsigned int> > > ReportCache_t; // <Label, <Value, Set Index> >
    static unsigned int           MIN_RANK_RPT_GUMBEL;

  protected:
    tract_t                       m_Center;                // Center of cluster (index to grid)
    tract_t                       _central_identifier;     // Index of most central identifier in cluster
    tract_t                       _num_identifiers;        // Number of neighboring identifiers in cluster
    double                        m_CartesianRadius;       // radius based upon locations in cluster in Cartesian system
    unsigned int                  m_nRank;                 // Rank based on results of simulations
    double                        m_NonCompactnessPenalty; // non-compactness penalty, for ellipses
    int                           m_iEllipseOffset;        // Link to Circle or Ellipse (top cluster)
    mutable ReportCache_t       * gpCachedReportLines;
	mutable bool                  _gini_cluster;           // indicates that cluster is gini cluster
    mutable bool                  _hierarchical_cluster;   // indicates that cluster is hierarchical cluster
    mutable double                _span_of_locations;      // the distance between furthest locations in cluster

    void                          cacheReportLine(std::string& label, std::string& value, unsigned int setIdx=std::numeric_limits<unsigned int>::max()) const;
    std::string                 & GetPopulationAsString(std::string& sString, double dPopulation) const;
    void                          printClusterData(FILE* fp, const AsciiPrintFormat& PrintFormat, const char * label, std::string& value, bool saveToCache, unsigned int setIdx= std::numeric_limits<unsigned int>::max()) const;

  public:
    CCluster();
    virtual ~CCluster();

    //assignment operations
    virtual CCluster            * Clone() const {throw prg_error("Clone().", "Clone()"); return 0; }//= 0;
    virtual void                  CopyEssentialClassMembers(const CCluster& rhs) {throw prg_error("CopyEssentialClassMembers()", "CopyEssentialClassMembers()");}//= 0;
    CCluster                    & operator=(const CCluster& rhs);
    
    std::string getIdentifyingKey() const {
        // Returns a key that identifies this cluster in terms of ellipse offset, centroid, and window definition.
        std::stringstream key;
        key << m_iEllipseOffset << "-" << m_Center << "-" << m_nFirstInterval << "-" << m_nLastInterval;
        return key.str();
    }

    //pure virtual functions
    virtual AbstractClusterData       * GetClusterData() {throw prg_error("GetClusterData()", "GetClusterData()"); return 0; }//= 0;
    virtual const AbstractClusterData * GetClusterData() const {throw prg_error("GetClusterData().", "GetClusterData()"); return 0; }//= 0;
    virtual ClusterType                 GetClusterType() const {throw prg_error("GetClusterType().", "GetClusterType()"); return PURELYSPATIALCLUSTER; }//= 0;
    //public data members - speed considerations
    double                        m_nRatio;             // Likelihood ratio
    mutable boost::dynamic_bitset<> _ratio_sets;        // With multiple data set and multivariate scans, potentially not all data sets comprised the LLR.
    int                           m_nFirstInterval;     // Index # of first time interval
    int                           m_nLastInterval;      // Index # of last time interval

    DataSetIndexes_t              getDataSetIndexesComprisedInRatio(const CSaTScanData& DataHub) const;
	int                           getClusterLength() const { return m_nLastInterval - m_nFirstInterval + 1;	}
    virtual AsciiPrintFormat      getAsciiPrintFormat() const {AsciiPrintFormat printFormat; return printFormat;}
    virtual bool                  ClusterDefined() const {return _num_identifiers > 0;}
    const double                  ConvertAngleToDegrees(double dAngle) const;
    virtual void                  DeallocateEvaluationAssistClassMembers();
    virtual void                  Display(FILE* fp, const CSaTScanData& DataHub, const ClusterSupplementInfo& supplementInfo, const SimulationVariables& simVars) const;
    virtual void                  DisplayAnnualCaseInformation(FILE* fp, unsigned int iDataSetIndex, const CSaTScanData& DataHub, const AsciiPrintFormat& PrintFormat) const;
    virtual void                  DisplayAnnualTimeTrendWithoutTitle(FILE* fp) const {/*stub - no action*/}
    virtual void                  DisplayCensusTracts(FILE* fp, const CSaTScanData& Data, const AsciiPrintFormat& PrintFormat) const;
    virtual void                  DisplayClusterDataBatched(FILE* fp, const CSaTScanData& DataHub, const AsciiPrintFormat& PrintFormat) const;
    virtual void                  DisplayClusterDataExponential(FILE* fp, const CSaTScanData& DataHub, const AsciiPrintFormat& PrintFormat) const;
    virtual void                  DisplayClusterDataNormal(FILE* fp, const CSaTScanData& DataHub, const AsciiPrintFormat& PrintFormat) const;
    virtual void                  DisplayClusterDataOrdinal(FILE* fp, const CSaTScanData& DataHub, const AsciiPrintFormat& PrintFormat) const;
    virtual void                  DisplayClusterDataRank(FILE* fp, const CSaTScanData& DataHub, const AsciiPrintFormat& PrintFormat) const;
    virtual void                  DisplayClusterDataStandard(FILE* fp, const CSaTScanData& DataHub, const AsciiPrintFormat& PrintFormat) const;
    virtual void                  DisplayClusterDataWeightedNormal(FILE* fp, const CSaTScanData& DataHub, const AsciiPrintFormat& PrintFormat) const;
    virtual void                  DisplayCoordinates(FILE* fp, const CSaTScanData& Data, const AsciiPrintFormat& PrintFormat) const;
    virtual void                  DisplayLatLongCoords(FILE* fp, const CSaTScanData& Data, const AsciiPrintFormat& PrintFormat) const;
    virtual void                  DisplayMonteCarloInformation(FILE* fp, const CSaTScanData& DataHub,
                                                               unsigned int iReportedCluster, 
                                                               const AsciiPrintFormat& PrintFormat,
                                                               const SimulationVariables& simVars) const;
    virtual void                  DisplayRecurrenceInterval(FILE* fp, const CSaTScanData& Data, 
                                                            unsigned int iReportedCluster, 
                                                            const SimulationVariables& simVars,
                                                            const AsciiPrintFormat& PrintFormat) const;
    virtual void                  DisplayRatio(FILE* fp, const CSaTScanData& DataHub, const AsciiPrintFormat& PrintFormat) const;
    virtual void                  DisplayRelativeRisk(FILE* fp, unsigned int iDataSetIndex, const CSaTScanData& DataHub, const AsciiPrintFormat& PrintFormat) const;
    virtual void                  DisplayObservedDivExpected(FILE* fp, unsigned int iDataSetIndex, const CSaTScanData& DataHub, const AsciiPrintFormat& PrintFormat) const;
    virtual void                  DisplayTimeFrame(FILE* fp, const CSaTScanData& DataHub, const AsciiPrintFormat& PrintFormat) const;
    virtual void                  DisplayTimeTrend(FILE* fp, const CSaTScanData& DataHub, const AsciiPrintFormat& PrintFormat) const {/*stub - no action*/}
    std::string&                  formatPopulationForDisplay(double population, std::string& buffer) const;
    virtual AreaRateType          getAreaRateForCluster(const CSaTScanData& DataHub) const;
    virtual tract_t               GetCentroidIndex() const {return m_Center;}
    std::string                 & GetClusterLocation(std::string& locationID, const CSaTScanData& DataHub) const;
    double                        GetNonCompactnessPenalty() const {return m_NonCompactnessPenalty;}
    int                           GetEllipseOffset() const {return m_iEllipseOffset;}
    virtual std::string         & GetEndDate(std::string& sDateString, const CSaTScanData& DataHub, const char * sep="/") const;
    virtual measure_t             GetExpectedCount(const CSaTScanData& DataHub, size_t tSetIndex=0) const;
    virtual measure_t             GetExpectedCountForTract(tract_t tTractIndex, const CSaTScanData& Data, size_t tSetIndex=0, bool adjusted=true) const {throw prg_error("GetExpectedCountForTract()", "GetExpectedCountForTract()"); return 0; }
    virtual measure_t             GetExpectedCountOrdinal(const CSaTScanData& DataHub, size_t tSetIndex, size_t iCategoryIndex) const;
    std::pair<double,double>      GetGumbelPValue(const SimulationVariables& simVars) const;
    double                        getLocationsSpan(const CSaTScanData& DataHub) const;
    double                        GetLatLongRadius() const {return 2 * EARTH_RADIUS_km * asin(m_CartesianRadius/(2 * EARTH_RADIUS_km));}
    virtual std::vector<tract_t>& getIdentifierIndexes(const CSaTScanData& DataHub, std::vector<tract_t>& indexes, bool bAtomize) const;
    tract_t                       mostCentralIdentifierIdx() const;
    virtual tract_t               getNumIdentifiers() const {return _num_identifiers;}
    virtual tract_t               numNonNullifiedIdentifiersInCluster(const CSaTScanData& DataHub) const;
    virtual count_t               GetObservedCount(size_t tSetIndex=0) const;
    virtual count_t               GetObservedCountForTract(tract_t tTractIndex, const CSaTScanData& Data, size_t tSetIndex=0) const {throw prg_error("GetObservedCountForTract().", "GetObservedCountForTract()"); return 0; }
	virtual count_t               GetCountForTractOutside(tract_t tTractIndex, const CSaTScanData& Data, size_t tSetIndex = 0) const { throw prg_error("GetCountForTractOutside().", "GetCountForTractOutside()"); return 0; }
	virtual count_t               GetObservedCountOrdinal(size_t tSetIndex, size_t iCategoryIndex) const;
    double                        GetObservedDivExpected(const CSaTScanData& DataHub, size_t tSetIndex=0) const;
    virtual double                GetObservedDivExpectedForTract(tract_t tTractIndex, const CSaTScanData& DataHub, size_t tSetIndex=0) const;
    double                        GetObservedDivExpectedOrdinal(const CSaTScanData& DataHub, size_t tSetIndex, size_t iCategoryIndex) const;
    double                        GetMonteCarloPValue(const CParameters& parameters, const SimulationVariables& simVars, bool bMLC) const;
    double                        getReportingPValue(const CParameters& parameters, const SimulationVariables& simVars, bool bMLC) const;
    double                        GetCartesianRadius() const {return m_CartesianRadius;}
    bool                          GetRadiusDefined() const {return m_CartesianRadius != -1;}
    unsigned int                  GetRank() const {return m_nRank;}
    double                        GetRatio() const {return m_nRatio;}
    double                        GetRelativeRisk(const CSaTScanData& DataHub, size_t tSetIndex=0) const;
    double                        GetRelativeRisk(double dObserved, double dExpected, double dTotalCases, double dTotalMeasure) const;
    virtual double                GetRelativeRiskForTract(tract_t tTractIndex, const CSaTScanData& DataHub, size_t tSetIndex=0) const;
    ReportCache_t               & getReportLinesCache() const;
    RecurrenceInterval_t          GetRecurrenceInterval(const CSaTScanData& Data, unsigned int iReportedCluster, const SimulationVariables& simVars) const;
    virtual std::string         & GetStartDate(std::string& sDateString, const CSaTScanData& DataHub, const char * sep="/") const;
    void                          IncrementRank() {m_nRank++;}
    virtual void                  Initialize(tract_t nCenter=0);
    bool                          isGiniCluster() const {return _gini_cluster;}
    bool                          isHierarchicalCluster() const { return _hierarchical_cluster; }
    boost::logic::tribool         isSignificant(const CSaTScanData& Data, unsigned int iReportedCluster, const SimulationVariables& simVars) const;
    boost::logic::tribool         meetsCutoff(
        const CSaTScanData& Data, unsigned int iReportedCluster, const SimulationVariables& simVars,
        RecurrenceInterval_t ri_cutoff, double pvalue_cutoff
    ) const;
    bool                          reportableGumbelPValue(const CParameters& parameters, const SimulationVariables& simVars) const;
    bool                          reportableMonteCarloPValue(const CParameters& parameters, const SimulationVariables& simVars) const;
    static bool                   reportablePValue(const CParameters& parameters, const SimulationVariables& simVars);
    static bool                   reportableRecurrenceInterval(const CParameters& parameters, const SimulationVariables& simVars);
    virtual void                  SetCartesianRadius(const CSaTScanData& DataHub);
    void                          SetCenter(tract_t nCenter);
    void                          SetEllipseOffset(int iOffset, const CSaTScanData& DataHub);
    void                          setAsGiniCluster(bool b) { _gini_cluster = b;}
    void                          setAsHierarchicalCluster(bool b) { _hierarchical_cluster = b; }
    virtual void                  setMostCentralIdentifier(const CSaTScanData& DataHub);
    void                          SetNonCompactnessPenalty(double dEllipseShape, double dPower);
    virtual void                  SetNonPersistantNeighborInfo(const CSaTScanData& DataHub, const CentroidNeighbors& Neighbors);
};

/** Attempts to dynamically cast AbstractClusterData object to class type T.
    Throws prg_error if cast fails, otherwise returns reference to casted type.
    Note that use of this function in the CTimeIntervals object appears to have
    a significant effect on runtime - therefore it is only useful during development
    in the CTimeIntervals classes. */
template <class T>
T & GetClusterDataAsType(AbstractClusterData& DataObject) {
  T * t;
  if ( (t = dynamic_cast<T*>( &DataObject ) ) == 0 )
    throw prg_error("Unable to dynamically cast to type.","GetClusterDataAsType()");
  return *t;
}
//*****************************************************************************
#endif
