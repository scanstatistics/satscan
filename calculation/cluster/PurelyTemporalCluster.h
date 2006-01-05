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

    inline virtual void                 AssignAsType(const CCluster& rhs) {*this = (CPurelyTemporalCluster&)rhs;m_nTracts=1;}
    virtual CPurelyTemporalCluster    * Clone() const;
    CPurelyTemporalCluster            & CPurelyTemporalCluster::operator=(const CPurelyTemporalCluster& rhs);

    virtual void                        DisplayCensusTracts(FILE* fp, const CSaTScanData& Data, const AsciiPrintFormat& PrintFormat) const;
    virtual void                        DisplayCoordinates(FILE* fp, const CSaTScanData& Data, const AsciiPrintFormat& PrintFormat) const {}
    virtual void                        DisplayLatLongCoords(FILE* fp, const CSaTScanData& Data, const AsciiPrintFormat& PrintFormat) const {}
    virtual AbstractClusterData       * GetClusterData() {return gpClusterData;}
    virtual const AbstractClusterData * GetClusterData() const {return gpClusterData;}
    virtual ClusterType                 GetClusterType() const {return PURELYTEMPORALCLUSTER;}
    virtual measure_t                   GetExpectedCountForTract(tract_t tTractIndex, const CSaTScanData& Data, size_t tSetIndex=0) const;
    tract_t                             GetMostCentralLocationIndex() const;
    virtual count_t                     GetObservedCountForTract(tract_t tTractIndex, const CSaTScanData& Data, size_t tSetIndex=0) const;
    virtual void                        Initialize(tract_t nCenter=0);
    virtual void                        SetCartesianRadius(const CSaTScanData& DataHub) {/*nop*/}
    virtual void                        SetMostCentralLocationIndex(const CSaTScanData& DataHub) {/*nop*/}
    virtual void                        SetNonPersistantNeighborInfo(const CSaTScanData& DataHub, const CentroidNeighbors& Neighbors) {/*nop*/}
    virtual void                        Write(LocationInformationWriter& LocationWriter, const CSaTScanData& Data,
                                              unsigned int iClusterNumber, unsigned int iNumSimsCompleted) const {/*nop*/}
};
//*****************************************************************************
#endif
