//*****************************************************************************
#ifndef __PURELYSPATIALMONOTONECLUSTER_H
#define __PURELYSPATIALMONOTONECLUSTER_H
//*****************************************************************************
#include "cluster.h"
#include "ClusterData.h"
#include "IntermediateClustersContainer.h"

/** Purely spatial monotone cluster */
class CPSMonotoneCluster : public CCluster {
    friend class PoissonLikelihoodCalculator;
    friend class BernoulliLikelihoodCalculator;

  private:
    std::auto_ptr<SpatialMonotoneData>        gpClusterData;

  public:
    CPSMonotoneCluster(const AbstractClusterDataFactory * pClusterFactory, const AbstractDataSetGateway & DataGateway, AreaRateType eRate);
    CPSMonotoneCluster(const AbstractClusterDataFactory * pClusterFactory, const DataSetInterface & Interface, AreaRateType eRate);
    CPSMonotoneCluster(const CPSMonotoneCluster& rhs);
    virtual ~CPSMonotoneCluster();

    CPSMonotoneCluster         & operator=(const CPSMonotoneCluster& rhs);
    virtual CPSMonotoneCluster * Clone() const;

    void                        AllocateForMaxCircles(tract_t nCircles) {gpClusterData->AllocateForMaxCircles(nCircles);}
    void                        CalculateTopClusterAboutCentroidDefinition(const AbstractDataSetGateway& DataGateway,
                                                                           const CentroidNeighbors& CentroidDef,
                                                                           CClusterSet& clusterSet,
                                                                           AbstractLikelihoodCalculator& Calculator);
    virtual bool                ClusterDefined() const {return gpClusterData->m_nSteps > 0;}
    virtual void                CopyEssentialClassMembers(const CCluster& rhs) {*this = (CPSMonotoneCluster&)rhs;}
    virtual void                DisplayCensusTracts(FILE* fp, const CSaTScanData& Data, const AsciiPrintFormat& PrintFormat) const;
    virtual void                DisplayCoordinates(FILE* fp, const CSaTScanData& Data, const AsciiPrintFormat& PrintFormat) const;
    virtual void                DisplayLatLongCoords(FILE* fp, const CSaTScanData& Data, const AsciiPrintFormat& PrintFormat) const;
    virtual void                DisplayObservedDivExpected(FILE* fp, unsigned int iDataSetIndex, const CSaTScanData& DataHub, const AsciiPrintFormat& PrintFormat) const;
    virtual void                DisplayTimeFrame(FILE* fp, const CSaTScanData& DataHub, const AsciiPrintFormat& PrintFormat) const {}
    virtual void                Initialize(tract_t nCenter=0);
    virtual AbstractClusterData       * GetClusterData() {return gpClusterData.get();}
    virtual const AbstractClusterData * GetClusterData() const {return gpClusterData.get();}
    virtual ClusterType         GetClusterType() const {return PURELYSPATIALMONOTONECLUSTER;}
    virtual std::string       & GetEndDate(std::string& sDateString, const CSaTScanData& DataHub) const;
    virtual measure_t           GetExpectedCount(const CSaTScanData& DataHub, size_t tSetIndex=0) const;
    virtual measure_t           GetExpectedCountForTract(tract_t tTractIndex, const CSaTScanData& Data, size_t tSetIndex=0) const;
    virtual count_t             GetObservedCount(size_t tSetIndex=0) const {return gpClusterData->GetCaseCount(tSetIndex);}
    virtual count_t             GetObservedCountForTract(tract_t tTractIndex, const CSaTScanData& Data, size_t tSetIndex=0) const;
    double                      GetRelativeRisk(tract_t nStep, const CSaTScanData& DataHub) const;
    virtual std::string       & GetStartDate(std::string& sDateString, const CSaTScanData& DataHub) const;
    virtual void                PrintClusterLocationsToFile(const CSaTScanData& DataHub, const std::string& sFilename) const;
    void                        SetTotalTracts();
    virtual void                Write(LocationInformationWriter& LocationWriter, const CSaTScanData& Data,
                                      unsigned int iClusterNumber, const SimulationVariables& simVars) const;
};
//*****************************************************************************
#endif
