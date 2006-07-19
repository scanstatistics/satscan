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

class LocationInformationWriter; /** forward class declaration */

/** Defines properties of each potential cluster evaluated by analysis. Provides
    functionality for printing cluster properties to file stream in predefined
    format. */
class CCluster {
  protected:
    tract_t                       m_Center;                // Center of cluster (index to grid)
    tract_t                       m_MostCentralLocation;   // Index of most central location
    tract_t                       m_nTracts;               // Number of neighboring tracts in cluster
    double                        m_CartesianRadius;       // radius based upon locations in cluster in Cartesian system
    unsigned int                  m_nRank;                 // Rank based on results of simulations
    double                        m_NonCompactnessPenalty; // non-compactness penalty, for ellipses
    int                           m_iEllipseOffset;        // Link to Circle or Ellipse (top cluster)

    ZdString                    & GetPopulationAsString(ZdString& sString, double dPopulation) const;

  public:
    CCluster();
    virtual ~CCluster();

    //assignment operations
    virtual CCluster            * Clone() const = 0;
    virtual void                  CopyEssentialClassMembers(const CCluster& rhs) = 0;
    CCluster                    & operator=(const CCluster& rhs);
    //pure virtual functions
    virtual AbstractClusterData       * GetClusterData() = 0;
    virtual const AbstractClusterData * GetClusterData() const = 0;
    virtual ClusterType                 GetClusterType() const = 0;
    //public data members - speed considerations
    double                        m_nRatio;             // Likelihood ratio
    int                           m_nFirstInterval;     // Index # of first time interval
    int                           m_nLastInterval;      // Index # of last time interval

    virtual bool                  ClusterDefined() const {return m_nTracts;}
    const double                  ConvertAngleToDegrees(double dAngle) const;
    virtual void                  DeallocateEvaluationAssistClassMembers();
    virtual void                  Display(FILE* fp, const CSaTScanData& DataHub, unsigned int iReportedCluster, unsigned int iNumSimsCompleted) const;
    virtual void                  DisplayAnnualCaseInformation(FILE* fp, unsigned int iDataSetIndex,
                                                               const CSaTScanData& DataHub,
                                                               const AsciiPrintFormat& PrintFormat) const;
    virtual void                  DisplayAnnualTimeTrendWithoutTitle(FILE* fp) const {/*stub - no action*/}
    virtual void                  DisplayCensusTracts(FILE* fp, const CSaTScanData& Data, const AsciiPrintFormat& PrintFormat) const;
    void                          DisplayCensusTractsInStep(FILE* fp, const CSaTScanData& Data, tract_t nFirstTract,
                                                            tract_t nLastTract, const AsciiPrintFormat& PrintFormat) const;
    virtual void                  DisplayClusterDataExponential(FILE* fp, const CSaTScanData& DataHub,
                                                                const AsciiPrintFormat& PrintFormat) const;
    virtual void                  DisplayClusterDataNormal(FILE* fp, const CSaTScanData& DataHub,
                                                           const AsciiPrintFormat& PrintFormat) const;
    virtual void                  DisplayClusterDataOrdinal(FILE* fp, const CSaTScanData& DataHub,
                                                            const AsciiPrintFormat& PrintFormat) const;
    virtual void                  DisplayClusterDataStandard(FILE* fp, const CSaTScanData& DataHub,
                                                             const AsciiPrintFormat& PrintFormat) const;
    virtual void                  DisplayCoordinates(FILE* fp, const CSaTScanData& Data,
                                                     const AsciiPrintFormat& PrintFormat) const;
    virtual void                  DisplayLatLongCoords(FILE* fp, const CSaTScanData& Data,
                                                       const AsciiPrintFormat& PrintFormat) const;
    virtual void                  DisplayMonteCarloInformation(FILE* fp, const CSaTScanData& DataHub,
                                                               const AsciiPrintFormat& PrintFormat,
                                                               unsigned int iNumSimsCompleted) const;
    virtual void                  DisplayNullOccurrence(FILE* fp, const CSaTScanData& Data, unsigned int iNumSimulations,
                                                        const AsciiPrintFormat& PrintFormat) const;
    virtual void                  DisplayPopulation(FILE* fp, const CSaTScanData& Data, const AsciiPrintFormat& PrintFormat) const;
    virtual void                  DisplayRatio(FILE* fp, const CSaTScanData& DataHub, const AsciiPrintFormat& PrintFormat) const;
    virtual void                  DisplayRelativeRisk(FILE* fp, unsigned int iDataSetIndex, const CSaTScanData& DataHub, const AsciiPrintFormat& PrintFormat) const;
    virtual void                  DisplayObservedDivExpected(FILE* fp, unsigned int iDataSetIndex, const CSaTScanData& DataHub, const AsciiPrintFormat& PrintFormat) const;
    virtual void                  DisplayTimeFrame(FILE* fp, const CSaTScanData& DataHub, const AsciiPrintFormat& PrintFormat) const;
    virtual void                  DisplayTimeTrend(FILE* fp, const AsciiPrintFormat& PrintFormat) const {/*stub - no action*/}
    virtual tract_t               GetCentroidIndex() const {return m_Center;}
    double                        GetNonCompactnessPenalty() const {return m_NonCompactnessPenalty;}
    int                           GetEllipseOffset() const {return m_iEllipseOffset;}
    virtual ZdString            & GetEndDate(ZdString& sDateString, const CSaTScanData& DataHub) const;
    virtual measure_t             GetExpectedCount(const CSaTScanData& DataHub, size_t tSetIndex=0) const;
    virtual measure_t             GetExpectedCountForTract(tract_t tTractIndex, const CSaTScanData& Data, size_t tSetIndex=0) const = 0;
    virtual measure_t             GetExpectedCountOrdinal(const CSaTScanData& DataHub, size_t tSetIndex, size_t iCategoryIndex) const;
    double                        GetLatLongRadius() const {return 2 * EARTH_RADIUS_km * asin(m_CartesianRadius/(2 * EARTH_RADIUS_km));}
    tract_t                       GetMostCentralLocationIndex() const;
    virtual tract_t               GetNumTractsInCluster() const {return m_nTracts;}
    virtual tract_t               GetNumNonNullifiedTractsInCluster(const CSaTScanData& DataHub) const;
    virtual count_t               GetObservedCount(size_t tSetIndex=0) const;
    virtual count_t               GetObservedCountForTract(tract_t tTractIndex, const CSaTScanData& Data, size_t tSetIndex=0) const = 0;
    virtual count_t               GetObservedCountOrdinal(size_t tSetIndex, size_t iCategoryIndex) const;
    double                        GetObservedDivExpected(const CSaTScanData& DataHub, size_t tSetIndex=0) const;
    virtual double                GetObservedDivExpectedForTract(tract_t tTractIndex, const CSaTScanData& DataHub, size_t tSetIndex=0) const;
    double                        GetObservedDivExpectedOrdinal(const CSaTScanData& DataHub, size_t tSetIndex, size_t iCategoryIndex) const;
    double                        GetPValue(unsigned int uiNumSimulationsCompleted) const;
    double                        GetCartesianRadius() const {return m_CartesianRadius;}
    bool                          GetRadiusDefined() const {return m_CartesianRadius != -1;}
    unsigned int                  GetRank() const {return m_nRank;}
    double                        GetRatio() const {return m_nRatio;}
    double                        GetRelativeRisk(const CSaTScanData& DataHub, size_t tSetIndex=0) const;
    double                        GetRelativeRisk(double dObserved, double dExpected, double dTotalCases) const;
    virtual double                GetRelativeRiskForTract(tract_t tTractIndex, const CSaTScanData& DataHub, size_t tSetIndex=0) const;
    virtual ZdString            & GetStartDate(ZdString& sDateString, const CSaTScanData& DataHub) const;
    void                          IncrementRank() {m_nRank++;}
    virtual void                  Initialize(tract_t nCenter=0);
    virtual void                  PrintClusterLocationsToFile(const CSaTScanData& DataHub, const std::string& sFilename) const;
    virtual void                  SetCartesianRadius(const CSaTScanData& DataHub);
    void                          SetCenter(tract_t nCenter);
    void                          SetEllipseOffset(int iOffset, const CSaTScanData& DataHub);
    virtual void                  SetMostCentralLocationIndex(const CSaTScanData& DataHub);
    void                          SetNonCompactnessPenalty(double dEllipseShape, double dPower);
    virtual void                  SetNonPersistantNeighborInfo(const CSaTScanData& DataHub, const CentroidNeighbors& Neighbors);
    virtual void                  Write(LocationInformationWriter& LocationWriter, const CSaTScanData& DataHub,
                                        unsigned int iReportedCluster, unsigned int iNumSimsCompleted) const;
};

/** Attempts to dynamically cast AbstractClusterData object to class type T.
    Throws ZdException if cast fails, otherwise returns reference to casted type.
    Note that use of this function in the CTimeIntervals object appears to have
    a significant effect on runtime - therefore it is only useful during development
    in the CTimeIntervals classes. */
template <class T>
T & GetClusterDataAsType(AbstractClusterData& DataObject) {
  T * t;

  if ( (t = dynamic_cast<T*>( &DataObject ) ) == 0 )
    ZdGenerateException("Unable to dynamically cast to type.","GetClusterDataAsType()");

  return *t;
}

//*****************************************************************************
#endif
