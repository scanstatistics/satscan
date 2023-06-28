//*****************************************************************************
#ifndef __PURELYTTEMPORALCLUSTER_H
#define __PURELYTTEMPORALCLUSTER_H
//*****************************************************************************
#include "Tracts.h"
#include "cluster.h"
#include "Parameters.h"
#include "ClusterDataFactory.h"

/** cluster class for purely temporal analysis and purely temporal cluster
    of space-time analysis */
class CPurelyTemporalCluster : public CCluster {
  private:
    void                                Init() {gpClusterData=0;}
    void                                Setup(const AbstractClusterDataFactory * pClusterFactory, const AbstractDataSetGateway & DataGateway, IncludeClustersType eIncludeClustersType, const CSaTScanData & Data);

  protected:
    AbstractTemporalClusterData       * gpClusterData;

  public:
    CPurelyTemporalCluster(const AbstractClusterDataFactory * pClusterFactory, const AbstractDataSetGateway & DataGateway,
                           IncludeClustersType eIncludeClustersType, const CSaTScanData & Data);
    CPurelyTemporalCluster(const CPurelyTemporalCluster& rhs);
    virtual ~CPurelyTemporalCluster();

    virtual CPurelyTemporalCluster    * Clone() const;
    CPurelyTemporalCluster            & operator=(const CPurelyTemporalCluster& rhs);

    virtual void                        CopyEssentialClassMembers(const CCluster& rhs);
    virtual void                        DisplayCensusTracts(FILE* fp, const CSaTScanData& Data, const AsciiPrintFormat& PrintFormat) const;
    virtual void                        DisplayCoordinates(FILE* fp, const CSaTScanData& Data, const AsciiPrintFormat& PrintFormat) const {}
    virtual void                        DisplayLatLongCoords(FILE* fp, const CSaTScanData& Data, const AsciiPrintFormat& PrintFormat) const {}
    virtual AbstractClusterData       * GetClusterData() {return gpClusterData;}
    virtual const AbstractClusterData * GetClusterData() const {return gpClusterData;}
    virtual ClusterType                 GetClusterType() const {return PURELYTEMPORALCLUSTER;}
    virtual std::string               & GetEndDate(std::string& sDateString, const CSaTScanData& DataHub, const char * sep="/") const;
    virtual measure_t                   GetExpectedCountForTract(tract_t tTractIndex, const CSaTScanData& Data, size_t tSetIndex=0, bool adjusted=true) const;
    virtual std::string               & GetStartDate(std::string& sDateString, const CSaTScanData& DataHub, const char * sep="/") const;
    virtual std::vector<tract_t>      & getGroupIndexes(const CSaTScanData& DataHub, std::vector<tract_t>& indexes, bool bAtomize) const;
    tract_t                             mostCentralObservationGroupIdx() const;
    virtual count_t                     GetObservedCountForTract(tract_t tTractIndex, const CSaTScanData& Data, size_t tSetIndex=0) const;
    virtual void                        Initialize(tract_t nCenter=0);
    virtual void                        PrintClusterLocationsToFile(const CSaTScanData& DataHub, const std::string& sFilename) const {/*nop*/}
    virtual void                        SetCartesianRadius(const CSaTScanData& DataHub) {/*nop*/}
    virtual void                        setMostCentralObservationGroup(const CSaTScanData& DataHub) {/*nop*/}
    virtual void                        SetNonPersistantNeighborInfo(const CSaTScanData& DataHub, const CentroidNeighbors& Neighbors) {/*nop*/}
    virtual void                        Write(LocationInformationWriter& LocationWriter, const CSaTScanData& Data, unsigned int iClusterNumber, const SimulationVariables& simVars, const LocationRelevance& location_relevance) const {/*nop*/}
};
//*****************************************************************************
#endif
