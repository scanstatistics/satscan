//*****************************************************************************
#ifndef __LikelihoodCalculation_H
#define __LikelihoodCalculation_H
//*****************************************************************************
#include "SaTScan.h"

class CPSMonotoneCluster; /** forward class declaration */
class CSaTScanData;       /** forward class declaration */
class CSVTTCluster;       /** forward class declaration */
class CTimeTrend;         /** forward class declaration */

class AbstractLikelihoodCalculator {
  protected:
    //double              gdLikelihoodForTotal;
    count_t             gtTotalCasesInDataSet;
    measure_t           gtTotalMeasureInDataSet;
    
    void                Init() {/*gdLikelihoodForTotal=0;*/}

  public:
    AbstractLikelihoodCalculator(count_t tTotalCases, measure_t tTotalMeasure);
    virtual ~AbstractLikelihoodCalculator();

    virtual double      CalcLogLikelihood(count_t n, measure_t u) = 0;
    virtual double      CalcLogLikelihoodRatio(count_t tCases, measure_t tMeasure, count_t tTotalCases, measure_t tTotalMeasure) = 0;
    virtual double      CalcLogLikelihoodRatioEx(count_t tCases, measure_t tMeasure, measure_t tMeasure2, count_t tTotalCases, measure_t tTotalMeasure);
    virtual double      CalcMonotoneLogLikelihood(const CPSMonotoneCluster& PSMCluster);
    virtual double      CalcSVTTLogLikelihoodRatio(size_t tStream, CSVTTCluster* Cluster, CTimeTrend& GlobalTimeTrend);
    virtual double      GetLogLikelihoodForTotal() const = 0;
};
//*****************************************************************************
#endif
