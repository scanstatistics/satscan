// PSMCluster.h

#ifndef __PSMCLUSTER_H
#define __PSMCLUSTER_H

#include "PScluster.h"
#include "spatscan.h"
#include "data.h"

class CPSMonotoneCluster : public CPurelySpatialCluster
{
  public:

    CPSMonotoneCluster(BasePrint *pPrintDirection);
    CPSMonotoneCluster(int nRate, tract_t nCircles);
    ~CPSMonotoneCluster();

    friend class CPoissonModel;
    friend class CBernoulliModel;
    
  protected:
    tract_t    m_nMaxCircles;        // Maximum number of circles possible
    count_t*   m_pCasesList;         // Number of cases in each circle
    measure_t* m_pMeasureList;       // Expected count for each circle
    tract_t*   m_pFirstNeighborList; // 1st neighbor in circle
    tract_t*   m_pLastNeighborList;  // Last neighbor in circle

  public:
    CPSMonotoneCluster& operator =(const CPSMonotoneCluster& cluster);

    virtual void Initialize(tract_t nCenter);
    void AllocateForMaxCircles(tract_t nCircles);

    virtual void DefineTopCluster(const CSaTScanData& Data, count_t** pCases);

    void AddNeighbor(int iEllipse, const CSaTScanData& Data, count_t** pCases, tract_t n);
    void CheckCircle(tract_t n);
    void AddRemainder(count_t nTotalCases, measure_t nTotalMeasure);
    void RemoveRemainder();

    double GetRatio();
//    double GetLogLikelihood();

    double GetRelativeRisk(tract_t nStep, double nMeasureAdjustment);

    tract_t GetLastCircleIndex() {return m_nSteps-1;};
    virtual tract_t GetNumTractsInnerCircle() { return m_pLastNeighborList[0]; };

    virtual bool ClusterDefined() {return (m_nSteps > 0);};

    virtual void DisplaySteps(FILE* fp, char* szSpacesOnLeft);
    virtual void DisplayRelativeRisk(FILE* fp, double nMeasureAdjustment,
                                     int nLeftMargin, int nRightMargin,
                                     char cDeliminator, char* szSpacesOnLeft);
    virtual void DisplayCensusTracts(FILE* fp, const CSaTScanData& Data,
                                     int nCluster, measure_t nMinMeasure,
                                     int nReplicas,
                                     bool bIncludeRelRisk, bool bIncludePVal,
                                     int nLeftMargin, int nRightMargin,
                                     char cDeliminator, char* szSpacesOnLeft,
                                     bool bFormat = true);
    virtual void DisplayCoordinates(FILE* fp, const CSaTScanData& Data,
                                    int nLeftMargin, int nRightMargin,
                                    char cDeliminator, char* szSpacesOnLeft);
    virtual void DisplayLatLongCoords(FILE* fp, const CSaTScanData& Data,
                                    int nLeftMargin, int nRightMargin,
                                    char cDeliminator, char* szSpacesOnLeft);

  protected:
    void   SetCasesAndMeasures();
    void   SetTotalTracts();
//    double SetLogLikelihood();
    double SetRatio(double nLogLikelihoodForTotal);

    void   ConcatLastCircles();
};

#endif
