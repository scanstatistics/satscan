//******************************************************************************
#ifndef __BernoulliRandomizer_H
#define __BernoulliRandomizer_H
//******************************************************************************
#include "DenominatorDataRandomizer.h"
#include <boost/cast.hpp>

/** Bernoulli randomizer for null hypothesis. */
class BernoulliNullHypothesisRandomizer : public AbstractOrdinalDenominatorDataRandomizer {
  public:
    BernoulliNullHypothesisRandomizer(long lInitialSeed=RandomNumberGenerator::glDefaultSeed) : AbstractOrdinalDenominatorDataRandomizer(lInitialSeed) {}
    virtual ~BernoulliNullHypothesisRandomizer() {}

    virtual BernoulliNullHypothesisRandomizer * Clone() const {return new BernoulliNullHypothesisRandomizer(*this);}

    virtual void  RandomizeData(const RealDataSet& RealSet, DataSet& SimSet, unsigned int iSimulation);
};

/** Bernoulli randomizer for null hypothesis, optimized for purely temporal analyses. */
class BernoulliPurelyTemporalNullHypothesisRandomizer : public AbstractOrdinalDenominatorDataRandomizer {
  public:
    BernoulliPurelyTemporalNullHypothesisRandomizer(long lInitialSeed=RandomNumberGenerator::glDefaultSeed) : AbstractOrdinalDenominatorDataRandomizer(lInitialSeed) {}
    virtual ~BernoulliPurelyTemporalNullHypothesisRandomizer() {}

    virtual BernoulliPurelyTemporalNullHypothesisRandomizer * Clone() const {return new BernoulliPurelyTemporalNullHypothesisRandomizer(*this);}

    virtual void  RandomizeData(const RealDataSet& RealSet, DataSet& SimSet, unsigned int iSimulation);
};

/*
class AbstractBernoulliNullHypothesisDayOfWeekRandomizer : public AbstractOrdinalDenominatorDataRandomizer {
  protected:
    typedef std::deque<boost::gregorian::greg_weekday> WeekDaysContainer_t;
    WeekDaysContainer_t _weekdays;

  public:
    AbstractBernoulliNullHypothesisDayOfWeekRandomizer(boost::gregorian::greg_weekday last_weekday, long lInitialSeed=RandomNumberGenerator::glDefaultSeed);
    virtual ~AbstractBernoulliNullHypothesisDayOfWeekRandomizer() {}
};
*/
 
/** Bernoulli randomizer for null hypothesis with day of week adjustment. */
class BernoulliNullHypothesisDayOfWeekRandomizer : public AbstractOrdinalDenominatorDataRandomizer {
  public:
    BernoulliNullHypothesisDayOfWeekRandomizer(boost::gregorian::greg_weekday last_weekday, long lInitialSeed=RandomNumberGenerator::glDefaultSeed);
    virtual ~BernoulliNullHypothesisDayOfWeekRandomizer() {}

    virtual BernoulliNullHypothesisDayOfWeekRandomizer * Clone() const {return new BernoulliNullHypothesisDayOfWeekRandomizer(*this);}

    virtual void  RandomizeData(const RealDataSet& RealSet, DataSet& SimSet, unsigned int iSimulation);
};

/** Bernoulli randomizer for null hypothesis with day of week adjustment -- optimized for purely temporal analyses. */
class BernoulliPurelyTemporalNullHypothesisDayOfWeekRandomizer : public AbstractOrdinalDenominatorDataRandomizer {
  public:
    BernoulliPurelyTemporalNullHypothesisDayOfWeekRandomizer(boost::gregorian::greg_weekday last_weekday, long lInitialSeed=RandomNumberGenerator::glDefaultSeed);
    virtual ~BernoulliPurelyTemporalNullHypothesisDayOfWeekRandomizer() {}

    virtual BernoulliPurelyTemporalNullHypothesisDayOfWeekRandomizer * Clone() const {return new BernoulliPurelyTemporalNullHypothesisDayOfWeekRandomizer(*this);}

    virtual void  RandomizeData(const RealDataSet& RealSet, DataSet& SimSet, unsigned int iSimulation);
};

//******************************************************************************
#endif

