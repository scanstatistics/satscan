//*****************************************************************************
#ifndef __POISSONMODEL_H
#define __POISSONMODEL_H
//*****************************************************************************
#include "SaTScan.h"
#include "ProbabilityModel.h"
#include "CalculateMeasure.h"
#include "RandomDistribution.h"
#include "PurelySpatialMonotoneCluster.h"
#include <iostream>
#include <fstream>
#include "SVTTCluster.h"

class CPoissonModel : public CModel {
  private:
    RandomNumberGenerator       m_RandomNumberGenerator;
    BinomialGenerator           gBinomialGenerator;
    float                     * gpRelativeRisks;
    measure_t                 * gpMeasure;  
    measure_t                ** gpAlternativeMeasure;
    long int                    glFilePosition;

    void                        AllocateAlternateHypothesisStructures();
    virtual double              CalcSVTTLogLikelihood(count_t* pCases, measure_t* pMeasure,
                                                      count_t pTotalCases, double nAlpha,
                                                      double nBeta, int nStatus);
    void                        DeallocateAlternateHypothesisStructures();
    void                        Init() {gpRelativeRisks=0; gpAlternativeMeasure=0;gpMeasure=0;glFilePosition=0;}
    void                        MakeData_AlternateHypothesis();
    void                        MakeDataTimeStratified();
    void                        MakeDataUnderNullHypothesis();
    void                        PrintSimulationDateToFile(int iSimulationNumber);
    void                        ReadSimulationDataFromFile();

  public:
    CPoissonModel(CParameters& Parameters, CSaTScanData& Data, BasePrint& PrintDirection);
    virtual ~CPoissonModel();

    virtual void                AdjustForNonParameteric(measure_t ** pNonCumulativeMeasure);
    virtual void                AdjustForLLPercentage(measure_t ** pNonCumulativeMeasure, double nPercentage);
    virtual void                AdjustForLogLinear(measure_t ** pNonCumulativeMeasure);
    void                        AdjustMeasure(measure_t ** pMeasure);
    virtual double              CalcLogLikelihood(count_t n, measure_t u);
    virtual double              CalcMonotoneLogLikelihood(const CPSMonotoneCluster& PSMCluster);
    virtual double              CalcSVTTLogLikelihood(CSVTTCluster* Cluster, CTimeTrend GlobalTimeTrend);
    virtual bool                CalculateMeasure();
    virtual double              GetLogLikelihoodForTotal() const;
    virtual double              GetPopulation(int m_iEllipseOffset, tract_t nCenter,
                                              tract_t nTracts, int nStartInterval, int nStopInterval);
    virtual void                MakeData(int iSimulationNumber);
    virtual bool                ReadData();
};
//*****************************************************************************
#endif

