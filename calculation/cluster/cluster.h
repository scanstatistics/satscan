//*****************************************************************************
#ifndef __CLUSTER_H
#define __CLUSTER_H
//*****************************************************************************
#include "SaTScan.h"
#include "SaTScanData.h"
#include "IncidentRate.h"
#include "DataStream.h"
#include "UtilityFunctions.h"
#include "ClusterDataFactory.h"
#include "AsciiPrintFormat.h"

class stsAreaSpecificData;

/** Defines properties of each potential cluster evaluated by analysis. Provides
    functionality for printing cluster properties to file stream in predefined
    format. */
class CCluster {
  protected:
    tract_t                       m_Center;             // Center of cluster (index to grid)
    RATE_FUNCPTRTYPE              m_pfRateOfInterest;
    tract_t                       m_nTracts;            // Number of neighboring tracts in cluster
    unsigned int                  m_nRank;              // Rank based on results of simulations
    double                        m_NonCompactnessPenalty;  // non-compactness penalty, for ellipses
    int                           m_iEllipseOffset;     // Link to Circle or Ellipse (top cluster)

  public:
    CCluster();
    virtual ~CCluster();

    //assignment operations
    inline virtual void           AssignAsType(const CCluster& rhs) {*this = rhs;}
    virtual CCluster            * Clone() const = 0;
    CCluster                    & operator=(const CCluster& rhs);
    //pure virtual functions
    virtual AbstractClusterData * GetClusterData() = 0;
    virtual ClusterType           GetClusterType() const = 0;
    //public data members - speed considerations
    double                        m_nRatio;             // Likelihood ratio
    int                           m_nFirstInterval;     // Index # of first time interval
    int                           m_nLastInterval;      // Index # of last time interval

    virtual bool                  ClusterDefined() const {return m_nTracts;}
    const double                  ConvertAngleToDegrees(double dAngle) const;
    virtual void                  Display(FILE* fp, const CSaTScanData& DataHub, unsigned int iReportedCluster,
                                          measure_t nMinMeasure, unsigned int iNumSimsCompleted) const;
    virtual void                  DisplayAnnualCaseInformation(FILE* fp, const CSaTScanData& DataHub,
                                                               const AsciiPrintFormat& PrintFormat) const;
    virtual void                  DisplayAnnualTimeTrendWithoutTitle(FILE* fp) const {/*stub - no action*/}
    virtual void                  DisplayCaseInformation(FILE* fp, const CSaTScanData& DataHub,
                                                         const AsciiPrintFormat& PrintFormat) const;
    virtual void                  DisplayCensusTracts(FILE* fp, const CSaTScanData& Data, measure_t nMinMeasure,
                                                      const AsciiPrintFormat& PrintFormat) const;
    void                          DisplayCensusTractsInStep(FILE* fp, const CSaTScanData& Data, tract_t nFirstTract,
                                                            tract_t nLastTract, measure_t nMinMeasure,
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
    virtual void                  DisplayPopulation(FILE* fp, const CSaTScanData& Data, const AsciiPrintFormat& PrintFormat) const ;
    virtual void                  DisplayRatio(FILE* fp, const CSaTScanData& DataHub, const AsciiPrintFormat& PrintFormat) const;
    virtual void                  DisplayRelativeRisk(FILE* fp, const CSaTScanData& DataHub,
                                                      const AsciiPrintFormat& PrintFormat) const;
    virtual void                  DisplaySteps(FILE* fp, const AsciiPrintFormat& PrintFormat) const {/*stub - no action*/}
    virtual void                  DisplayTimeFrame(FILE* fp, const CSaTScanData& DataHub, const AsciiPrintFormat& PrintFormat) const;
    virtual void                  DisplayTimeTrend(FILE* fp, const AsciiPrintFormat& PrintFormat) const {/*stub - no action*/}
    virtual count_t               GetCaseCount(unsigned int iStream) const = 0;
    virtual count_t               GetCaseCountForTract(tract_t tTract, const CSaTScanData& Data, unsigned int iStream=0) const = 0;
    virtual tract_t               GetCentroidIndex() const {return m_Center;}
    double                        GetNonCompactnessPenalty() const {return m_NonCompactnessPenalty;}
    int                           GetEllipseOffset() const {return m_iEllipseOffset;}
    virtual ZdString            & GetEndDate(ZdString& sDateString, const CSaTScanData& DataHub) const;
    virtual measure_t             GetMeasure(unsigned int iStream) const = 0;
    virtual measure_t             GetMeasureForTract(tract_t tTract, const CSaTScanData& Data, unsigned int iStream=0) const = 0;
    virtual tract_t               GetNumTractsInnerCircle() const {return m_nTracts;}
    const double                  GetPValue(unsigned int uiNumSimulationsCompleted) const;
    unsigned int                  GetRank() const {return m_nRank;}
    double                        GetRatio() const {return m_nRatio;}
    const double                  GetRelativeRisk(double nMeasureAdjustment, unsigned int iStream=0) const;
    virtual double                GetRelativeRiskForTract(tract_t tTract, const CSaTScanData& DataHub, unsigned int iStream=0) const;
    virtual ZdString            & GetStartDate(ZdString& sDateString, const CSaTScanData& DataHub) const;
    void                          IncrementRank() {m_nRank++;}
    virtual void                  Initialize(tract_t nCenter=0);
    void                          SetCenter(tract_t nCenter);
    void                          SetEllipseOffset(int iOffset);
    void                          SetNonCompactnessPenalty(double dEllipseShape);
    void                          SetRate(int nRate);
    virtual void                  Write(stsAreaSpecificData& AreaData, const CSaTScanData& DataHub,
                                        unsigned int iReportedCluster, unsigned int iNumSimsCompleted) const;
};
//*****************************************************************************
#endif
