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

  public:
    CBernoulliModel(CParameters& Parameters, CSaTScanData& Data, BasePrint& PrintDirection);
    virtual ~CBernoulliModel();

    virtual double              CalcLogLikelihood(count_t n, measure_t u);
    virtual double              CalcMonotoneLogLikelihood(const CPSMonotoneCluster& PSMCluster);
    virtual bool                CalculateMeasure();
    virtual double              GetLogLikelihoodForTotal() const;
    virtual double              GetPopulation(int m_iEllipseOffset, tract_t nCenter,
                                              tract_t nTracts, int nStartInterval, int nStopInterval);
    virtual void                MakeData(int iSimulationNumber);
    void                        MakeDataB(count_t tTotalCounts, measure_t tTotalMeasure, std::vector<count_t>& RandCounts);    
    void                        MakeDataTimeStratified();
    void                        MakeDataUnderNullHypothesis();
    virtual bool                ReadData();
};

//*****************************************************************************
#endif

