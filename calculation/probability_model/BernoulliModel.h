//*****************************************************************************
#ifndef __BERNOULLIMODEL_H
#define __BERNOULLIMODEL_H
//*****************************************************************************
#include "ProbabilityModel.h"
#include "PurelySpatialMonotoneCluster.h"
#include "RandomDistribution.h"
#include "RandomNumberGenerator.h"

class CBernoulliModel : public CModel {
  private:
    BinomialGenerator           gBinomialGenerator;
    RandomNumberGenerator       m_RandomNumberGenerator;

    void                        MakeDataB(count_t tTotalCounts, measure_t tTotalMeasure, std::vector<count_t>& RandCounts);    
    void                        MakeDataUnderNullHypothesis(DataStreamInterface & DataInterface);

  public:
    CBernoulliModel(CParameters& Parameters, CSaTScanData& Data, BasePrint& PrintDirection);
    virtual ~CBernoulliModel();

    virtual double              CalcLogLikelihood(count_t n, measure_t u);
    virtual double              CalcLogLikelihoodRatio(count_t tCases, measure_t tMeasure, count_t tTotalCases, measure_t tTotalMeasure, double dCompactnessCorrection);
    virtual double              CalcMonotoneLogLikelihood(const CPSMonotoneCluster& PSMCluster);
    virtual bool                CalculateMeasure(DataStream & thisStream);
    virtual double              GetLogLikelihoodForTotal() const;
    virtual double              GetPopulation(int m_iEllipseOffset, tract_t nCenter,
                                              tract_t nTracts, int nStartInterval, int nStopInterval);
    virtual void                MakeData(int iSimulationNumber, DataStreamInterface & DataInterface, unsigned int tInterface=0);
    virtual bool                ReadData();
};

//*****************************************************************************
#endif

