//*****************************************************************************
#ifndef __PURELYTTEMPORALCLUSTER_H
#define __PURELYTTEMPORALCLUSTER_H
//*****************************************************************************
#include "Tracts.h"
#include "Cluster.h"
#include "TimeIntervalAll.h"
#include "TimeIntervalAlive.h"
#include "Parameters.h"
#include "TimeIntervalRange.h"
#include <deque>

class TemporalClusterStreamData : public AbstractTemporalClusterStreamData {
   public:
    TemporalClusterStreamData(count_t * pCases, measure_t * pMeasure, measure_t * pSqMeasure);
    virtual ~TemporalClusterStreamData();

    virtual TemporalClusterStreamData  * Clone() const {return new TemporalClusterStreamData(*this);}
};


/** cluster class for purely temporal analysis and purely temporal cluster
    of space-time analysis */
class CPurelyTemporalCluster : public CCluster {
  private:
    void                                Init() {m_TI=0;}
    void                                Setup(const DataStreamGateway & DataGateway, IncludeClustersType eIncludeClustersType, const CSaTScanData & Data);
    void                                Setup(const DataStreamInterface & Interface, IncludeClustersType eIncludeClustersType, const CSaTScanData & Data);
    void                                Setup(const CPurelyTemporalCluster& rhs);

  protected:
    StreamDataContainer_t               gStreamData;

    int                                 m_nTotalIntervals;
    int                                 m_nIntervalCut;
    IncludeClustersType                 m_nTIType;
    CTimeIntervals                    * m_TI;

  public:
    CPurelyTemporalCluster(const DataStreamGateway & DataGateway, IncludeClustersType eIncludeClustersType,
                           const CSaTScanData & Data, BasePrint & PrintDirection);
    CPurelyTemporalCluster(const DataStreamInterface & Interface, IncludeClustersType eIncludeClustersType,
                           const CSaTScanData & Data, BasePrint & PrintDirection);
    CPurelyTemporalCluster(const CPurelyTemporalCluster& rhs);
    virtual ~CPurelyTemporalCluster();

    CPurelyTemporalCluster& CPurelyTemporalCluster::operator =(const CPurelyTemporalCluster& rhs);

    inline virtual void                 AssignAsType(const CCluster& rhs) {*this = (CPurelyTemporalCluster&)rhs;}
    virtual CPurelyTemporalCluster    * Clone() const;
    inline virtual void                 CompareTopCluster(CPurelyTemporalCluster & TopShapeCluster, const CSaTScanData & Data);
    inline virtual void                 ComputeBestMeasures(const CSaTScanData& Data, CMeasureList & MeasureList);
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
    virtual measure_t                   GetMeasure(unsigned int iStream) const;
    virtual measure_t                   GetMeasureForTract(tract_t tTract, const CSaTScanData& Data) const;
    virtual void                        Initialize(tract_t nCenter=0);
    virtual void                        SetCaseCount(unsigned int iStream, count_t tCount) {gStreamData[iStream]->gCases = tCount;}
    virtual void                        SetMeasure(unsigned int iStream, measure_t tMeasure) {gStreamData[iStream]->gMeasure = tMeasure;}
};
//*****************************************************************************
#endif
