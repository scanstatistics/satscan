//*****************************************************************************
#ifndef __TIMEINTERVALS_H
#define __TIMEINTERVALS_H
//*****************************************************************************
#include "SaTScan.h"
#include "Parameters.h"
#include "cluster.h"
#include "AbstractClusterData.h"
#include "MaxWindowLengthIndicator.h"

class CMeasureList; /** forward class declaration */
class CCluster;     /** forward class declaration */
class CSaTScanData; /** forward class declaration */

/** Abstract base class which defines methods of iterating through temporal
    windows, evaluating the strength of a clustering.*/
class CTimeIntervals {
  private:
//    CTimeIntervals(const CTimeIntervals& rhs) {}

    void                               Setup(IncludeClustersType eIncludeClustersType);

  protected:
    int				       giStartRange_Start;         /** start date index of start range */
    int				       giStartRange_End;           /** end date index of start range */
    int				       giEndRange_Start;           /** start date index of end range */
    int				       giEndRange_End;             /** end date index of end range */
    const CSaTScanData               & gDataHub;                      /** data hub */
    AbstractLikelihoodCalculator     & gLikelihoodCalculator;      /** log likelihood calculator */
    AbstractMaxWindowLengthIndicator * gpMaxWindowLengthIndicator; /** indicates maximum temporal window length */
    int                                giNumIntervals;         /* number of total time intervals */
    int                                giMaxWindowLength;      /* maximum window length          */
    RATE_FUNCPTRTYPE                   fRateOfInterest;

  public:
    CTimeIntervals(const CSaTScanData& DataHub, AbstractLikelihoodCalculator& Calculator, IncludeClustersType eIncludeClustersType);
    virtual ~CTimeIntervals();

    virtual void                CompareClusters(CCluster& Running, CCluster& TopShapeCluster) = 0;
    virtual void                CompareMeasures(AbstractTemporalClusterData& StreamData, CMeasureList& MeasureList) = 0;
    virtual double              ComputeMaximizingValue(AbstractTemporalClusterData& ClusterData) = 0;
};
#endif
