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
    RandomNumberGenerator                       m_RandomNumberGenerator;
    BinomialGenerator                           gBinomialGenerator;
    float                                     * gpRelativeRisks;
    measure_t                                 * gpMeasure;
    measure_t                                ** gpAlternativeMeasure;
    ifstream                                    gSimulationDataInputFile;

    void                        AdjustForNonParameteric(DataStream & thisStream, measure_t ** pNonCumulativeMeasure);
    void                        AdjustForLLPercentage(DataStream & thisStream, measure_t ** pNonCumulativeMeasure, double nPercentage);
    void                        AdjustForLogLinear(DataStream & thisStream, measure_t ** pNonCumulativeMeasure);
    void                        AdjustMeasure(DataStream & thisStream, measure_t ** ppNonCumulativeMeasure);
    void                        AllocateAlternateHypothesisStructures();
    void                        AssignMeasure(DataStream & thisStream, measure_t ** ppNonCumulativeMeasure);

    virtual double              CalcSVTTLogLikelihood(count_t* pCases, measure_t* pMeasure,
                                                      count_t pTotalCases, double nAlpha,
                                                      double nBeta, int nStatus);
    void                        DeallocateAlternateHypothesisStructures();
    void                        Init() {gpRelativeRisks=0; gpAlternativeMeasure=0;gpMeasure=0;}
    void                        MakeData_AlternateHypothesis(DataStreamInterface & DataInterface);
    void                        MakeDataTimeStratified(DataStreamInterface & DataInterface);
    void                        MakeDataUnderNullHypothesis(DataStreamInterface & DataInterface);
    void                        PrintSimulationDateToFile(int iSimulationNumber, DataStreamInterface & DataInterface);
    void                        ReadSimulationDataFromFile(DataStreamInterface & DataInterface);

  public:
    CPoissonModel(CParameters& Parameters, CSaTScanData& Data, BasePrint& PrintDirection);
    virtual ~CPoissonModel();

    virtual double              CalcLogLikelihood(count_t n, measure_t u);
    virtual double              CalcLogLikelihoodRatio(count_t tCases, measure_t tMeasure, count_t tTotalCases, measure_t tTotalMeasure, double dCompactnessCorrection);
    virtual double              CalcMonotoneLogLikelihood(const CPSMonotoneCluster& PSMCluster);
    virtual double              CalcSVTTLogLikelihood(size_t tStream, CSVTTCluster* Cluster, CTimeTrend GlobalTimeTrend);
    virtual bool                CalculateMeasure(DataStream & thisStream);
    virtual double              GetLogLikelihoodForTotal() const;
    virtual double              GetPopulation(int m_iEllipseOffset, tract_t nCenter,
                                              tract_t nTracts, int nStartInterval, int nStopInterval);
    virtual void                MakeData(int iSimulationNumber, DataStreamInterface & DataInterface, unsigned int tInterface=0);
    virtual bool                ReadData();
};
//*****************************************************************************
#endif

