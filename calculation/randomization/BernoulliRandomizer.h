//******************************************************************************
#ifndef __BernoulliRandomizer_H
#define __BernoulliRandomizer_H
//******************************************************************************
#include "DenominatorDataRandomizer.h"

/** Bernoulli randomizer for null hypothesis. */
class BernoulliNullHypothesisRandomizer : public AbstractOrdinalDenominatorDataRandomizer {
  public:
    BernoulliNullHypothesisRandomizer(long lInitialSeed=RandomNumberGenerator::glDefaultSeed);
    virtual ~BernoulliNullHypothesisRandomizer();
    virtual BernoulliNullHypothesisRandomizer * Clone() const;

    virtual void  RandomizeData(const RealDataSet& thisRealSet, SimDataSet& thisSimSet, unsigned int iSimulation);
};
//******************************************************************************
#endif

