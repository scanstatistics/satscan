//*****************************************************************************
#ifndef __TIMEINTERVALS_H
#define __TIMEINTERVALS_H
//*****************************************************************************
#include "SaTScan.h"
#include "Parameters.h"
#include "cluster.h"
#include "AbstractClusterData.h"

class CMeasureList; /** forward class declaration */
class CCluster;     /** forward class declaration */
class CSaTScanData; /** forward class declaration */

/** Abstract base class which defines methods of iterating through temporal
    windows, evaluating the strength of a clustering.*/
class CTimeIntervals {
  protected:
    int                         giNumIntervals;         /* number of total time intervals */
    int                         giMaxWindowLength;      /* maximum window length          */
    RATE_FUNCPTRTYPE            fRateOfInterest;

  public:
    CTimeIntervals(int nTotal, int nCut, AreaRateType eType=HIGH) {
                         giNumIntervals=nTotal;
                         giMaxWindowLength=nCut;
                         switch(eType) {
                           case LOW        : fRateOfInterest = LowRate;       break;
                           case HIGHANDLOW : fRateOfInterest = HighOrLowRate; break;
                           default         : fRateOfInterest = HighRate;
                         };
                   }
    CTimeIntervals(const CTimeIntervals& rhs) {giNumIntervals = rhs.giNumIntervals;
                                               giMaxWindowLength = rhs.giMaxWindowLength;}
    virtual ~CTimeIntervals() {};

    virtual CTimeIntervals    * Clone() const = 0;

    virtual void                CompareClusters(CCluster& Running, CCluster& TopShapeCluster) = 0;
    virtual void                CompareMeasures(TemporalData& StreamData, CMeasureList& MeasureList) = 0;
    virtual IncludeClustersType GetType() const = 0;
    virtual void                Initialize() {/*stub - no action */}
};
#endif
