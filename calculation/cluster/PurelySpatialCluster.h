//*****************************************************************************
#ifndef __PURELYSPATIALCLUSTER_H
#define __PURELYSPATIALCLUSTER_H
//*****************************************************************************
#include "Cluster.h"
#include "UtilityFunctions.h"
#include "MeasureList.h"

/** cluster class for purely spatial analysis and purely spatial cluster
    of space-time analysis */
class CPurelySpatialCluster : public CCluster {
  typedef std::vector<ClusterStreamData>           StreamDataContainer_t;
  typedef std::vector<ClusterStreamData>::iterator StreamDataContainerIterator_t;
  
  private:
    StreamDataContainer_t               gStreamData;

    void                                Setup(const CSaTScanData & Data);
    void                                Setup(const CPurelySpatialCluster& rhs);

  public:
    CPurelySpatialCluster(const CSaTScanData & Data, BasePrint *pPrintDirection);
    CPurelySpatialCluster(const CPurelySpatialCluster& rhs);
    ~CPurelySpatialCluster();
    
    CPurelySpatialCluster             & operator=(const CPurelySpatialCluster& rhs);


    void                                AddNeighbor(tract_t tNeighbor, const DataStreamGateway & DataGateway);
    void                                AddNeighbor(tract_t tNeighbor, const DataStreamInterface & Interface, size_t tStream=0);
    inline virtual void                 AssignAsType(const CCluster& rhs) {*this = (CPurelySpatialCluster&)rhs;}
    virtual CPurelySpatialCluster     * Clone() const;
    virtual void                        CompareTopCluster(CPurelySpatialCluster & TopShapeCluster, const CSaTScanData * pData);
    virtual void                        ComputeBestMeasures(CMeasureList & MeasureList);
    virtual void                        DisplayTimeFrame(FILE* fp, char* szSpacesOnLeft, int nAnalysisType);
    virtual count_t                     GetCaseCount(unsigned int iStream) const {return gStreamData[iStream].gCases;}
    virtual count_t                     GetCaseCountForTract(tract_t tTract, const CSaTScanData& Data) const;
    virtual measure_t                   GetMeasure(unsigned int iStream) const {return gStreamData[iStream].gMeasure;}
    virtual measure_t                   GetMeasureForTract(tract_t tTract, const CSaTScanData& Data) const;
    virtual void                        Initialize(tract_t nCenter);
    virtual void                        SetCaseCount(unsigned int iStream, count_t tCount) {gStreamData[iStream].gCases = tCount;}
    virtual void                        SetMeasure(unsigned int iStream, measure_t tMeasure) {gStreamData[iStream].gMeasure = tMeasure;}
    virtual void                        SetStartAndEndDates(const Julian* pIntervalStartTimes, int nTimeIntervals);
};
//*****************************************************************************
#endif