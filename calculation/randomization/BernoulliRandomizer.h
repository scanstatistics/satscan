//******************************************************************************
#ifndef __BernoulliRandomizer_H
#define __BernoulliRandomizer_H
//******************************************************************************
#include "DenominatorDataRandomizer.h"

/** Bernoulli randomizer for null hypothesis. */
class BernoulliNullHypothesisRandomizer : public AbstractOrdinalDenominatorDataRandomizer {
  public:
    BernoulliNullHypothesisRandomizer();
    virtual ~BernoulliNullHypothesisRandomizer();

    virtual BernoulliNullHypothesisRandomizer * Clone() const;

    virtual void                RandomizeData(const RealDataStream& thisRealStream,
                                              SimulationDataStream& thisSimulationStream,
                                              unsigned int iSimulation);
};
//******************************************************************************
#endif

