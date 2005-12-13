//******************************************************************************
#ifndef __BernoulliRandomizer_H
#define __BernoulliRandomizer_H
//******************************************************************************
#include "DenominatorDataRandomizer.h"

/** Bernoulli randomizer for null hypothesis. */
class BernoulliNullHypothesisRandomizer : public AbstractOrdinalDenominatorDataRandomizer {
  public:
    BernoulliNullHypothesisRandomizer(long lInitialSeed=RandomNumberGenerator::glDefaultSeed) : AbstractOrdinalDenominatorDataRandomizer(lInitialSeed) {}
    virtual ~BernoulliNullHypothesisRandomizer() {}

    virtual BernoulliNullHypothesisRandomizer * Clone() const {return new BernoulliNullHypothesisRandomizer(*this);}

    virtual void  RandomizeData(const RealDataSet& RealSet, SimDataSet& SimSet, unsigned int iSimulation);
};

/** Bernoulli randomizer for null hypothesis, optimized for purely temporal analyses. */
class BernoulliPurelyTemporalNullHypothesisRandomizer : public AbstractOrdinalDenominatorDataRandomizer {
  public:
    BernoulliPurelyTemporalNullHypothesisRandomizer(long lInitialSeed=RandomNumberGenerator::glDefaultSeed) : AbstractOrdinalDenominatorDataRandomizer(lInitialSeed) {}
    virtual ~BernoulliPurelyTemporalNullHypothesisRandomizer() {}

    virtual BernoulliPurelyTemporalNullHypothesisRandomizer * Clone() const {return new BernoulliPurelyTemporalNullHypothesisRandomizer(*this);}

    virtual void  RandomizeData(const RealDataSet& RealSet, SimDataSet& SimSet, unsigned int iSimulation);
};
//******************************************************************************
#endif

