//---------------------------------------------------------------------------
#ifndef BernoulliRandomizerH
#define BernoulliRandomizerH
//---------------------------------------------------------------------------
#include "DenominatorDataRandomizer.h"

/** abstract Bernoulli randomizer */
class BernoulliRandomizer : public AbstractDenominatorDataRandomizer {
  public:
    BernoulliRandomizer(long lInitialSeed);
    virtual ~BernoulliRandomizer();
};

/** Bernoulli randomizer for null hypothesis. */
class BernoulliNullHypothesisRandomizer : public BernoulliRandomizer {
  protected:
    void                        MakeDataB(count_t tTotalCounts, measure_t tTotalMeasure, std::vector<count_t>& RandCounts);

  public:
    BernoulliNullHypothesisRandomizer(long lInitialSeed);
    virtual ~BernoulliNullHypothesisRandomizer();

    virtual BernoulliNullHypothesisRandomizer * Clone() const;

    virtual void                RandomizeData(const RealDataStream& thisRealStream,
                                              SimulationDataStream& thisSimulationStream,
                                              unsigned int iSimulation);
};
//---------------------------------------------------------------------------
#endif
