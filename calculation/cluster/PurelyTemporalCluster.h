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
    void                                Setup(const AbstractClusterDataFactory * pClusterFactory, const AbtractDataStreamGateway & DataGateway, IncludeClustersType eIncludeClustersType, const CSaTScanData & Data);

  protected:
    AbstractTemporalClusterData       * gpClusterData;

  public:
    CPurelyTemporalCluster(const AbstractClusterDataFactory * pClusterFactory, const AbtractDataStreamGateway & DataGateway,
                           IncludeClustersType eIncludeClustersType, const CSaTScanData & Data);
    CPurelyTemporalCluster(const CPurelyTemporalCluster& rhs);
    virtual ~CPurelyTemporalCluster();

    inline virtual void                 AssignAsType(const CCluster& rhs) {*this = (CPurelyTemporalCluster&)rhs;m_nTracts=1;}
    virtual CPurelyTemporalCluster    * Clone() const;
    CPurelyTemporalCluster            & CPurelyTemporalCluster::operator=(const CPurelyTemporalCluster& rhs);

    virtual void                        DisplayCensusTracts(FILE* fp, const CSaTScanData& Data,
                                                            int nCluster,  measure_t nMinMeasure,
                                                            int iNumSimulations, long lReportHistoryRunNumber,
                                                            bool bIncludeRelRisk, bool bIncludePVal,
                                                            int nLeftMargin, int nRightMargin,
                                                            char cDeliminator, char* szSpacesOnLeft, bool bFormat = true);
    virtual void                        DisplayCoordinates(FILE* fp, const CSaTScanData& Data, int nLeftMargin,
                                                           int nRightMargin, char cDeliminator, char* szSpacesOnLeft) {}
    virtual void                        DisplayLatLongCoords(FILE* fp, const CSaTScanData& Data, int nLeftMargin,
                                                             int nRightMargin, char cDeliminator, char* szSpacesOnLeft) {}
    virtual void                        DisplayPopulation(FILE* fp, const CSaTScanData& Data, char* szSpacesOnLeft) {};
    virtual count_t                     GetCaseCount(unsigned int iStream) const;
    virtual count_t                     GetCaseCountForTract(tract_t tTract, const CSaTScanData& Data) const;
    virtual AbstractClusterData       * GetClusterData() {return gpClusterData;}
    virtual int                         GetClusterType() const {return PURELYTEMPORAL;}
    virtual measure_t                   GetMeasure(unsigned int iStream) const;
    virtual measure_t                   GetMeasureForTract(tract_t tTract, const CSaTScanData& Data) const;
    virtual void                        Initialize(tract_t nCenter=0);
};
//*****************************************************************************
#endif
