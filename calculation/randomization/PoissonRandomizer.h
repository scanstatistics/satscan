//******************************************************************************
#ifndef __PoissonRandomizer_H
#define __PoissonRandomizer_H
//******************************************************************************
#include "DenominatorDataRandomizer.h"
#include <iostream>
#include <fstream>

/** Abstraction for Poisson data randomizers */
class PoissonRandomizer : public AbstractDenominatorDataRandomizer {
  protected:
    const CParameters & gParameters;

  public:
    PoissonRandomizer(const CParameters& Parameters, long lInitialSeed=RandomNumberGenerator::glDefaultSeed)
       : AbstractDenominatorDataRandomizer(lInitialSeed), gParameters(Parameters) {}
    virtual ~PoissonRandomizer() {}

    virtual void  randomize(const RealDataSet& RealSet, const TwoDimMeasureArray_t& measure, DataSet& SimSet) = 0;
};

/** Poisson randomizer for null hypothesis. */
class PoissonNullHypothesisRandomizer : public PoissonRandomizer {
  public:
    PoissonNullHypothesisRandomizer(const CParameters & Parameters, long lInitialSeed=RandomNumberGenerator::glDefaultSeed)
       : PoissonRandomizer(Parameters, lInitialSeed) {}
    virtual ~PoissonNullHypothesisRandomizer() {}

    virtual PoissonNullHypothesisRandomizer * Clone() const {return new PoissonNullHypothesisRandomizer(*this);}

    virtual void RandomizeData(const RealDataSet& RealSet, DataSet& SimSet, unsigned int iSimulation);
    virtual void randomize(const RealDataSet& RealSet, const TwoDimMeasureArray_t& measure, DataSet& SimSet);
};

class CSaTScanData; /** forward class declaration */

/** Poisson randomizer for null hypothesis. Optimized for purely temporal analyses. */
class AbstractPoissonPurelyTemporalNullHypothesisRandomizer : public PoissonRandomizer {
    protected:
        void  randomize_data(count_t * pSimCases, const measure_t * pMeasure, unsigned int num_intervals, count_t totalcases, measure_t totalmeasure);

    public:
        AbstractPoissonPurelyTemporalNullHypothesisRandomizer(const CParameters & Parameters, long lInitialSeed=RandomNumberGenerator::glDefaultSeed);
        virtual ~AbstractPoissonPurelyTemporalNullHypothesisRandomizer() {}

        virtual void RandomizeData(const RealDataSet& RealSet, DataSet& SimSet, unsigned int iSimulation);
};

/** Poisson randomizer for null hypothesis. Optimized for purely temporal analyses. */
class PoissonPurelyTemporalNullHypothesisRandomizer : public AbstractPoissonPurelyTemporalNullHypothesisRandomizer {
    public:
        PoissonPurelyTemporalNullHypothesisRandomizer(const CParameters & Parameters, long lInitialSeed=RandomNumberGenerator::glDefaultSeed);
        virtual ~PoissonPurelyTemporalNullHypothesisRandomizer() {}

        virtual PoissonPurelyTemporalNullHypothesisRandomizer * Clone() const {return new PoissonPurelyTemporalNullHypothesisRandomizer(*this);}

        virtual void randomize(const RealDataSet& RealSet, const TwoDimMeasureArray_t& measure, DataSet& SimSet);
};

/** Poisson randomizer for null hypothesis. Optimized for purely temporal analyses and closed loop analysis performed during power estimation replications. */
class ClosedLoopPoissonPurelyTemporalNullHypothesisRandomizer : public AbstractPoissonPurelyTemporalNullHypothesisRandomizer {
    protected:
        const CSaTScanData & _data_hub;

    public:
        ClosedLoopPoissonPurelyTemporalNullHypothesisRandomizer(const CSaTScanData& datahub, long lInitialSeed=RandomNumberGenerator::glDefaultSeed);
        virtual ~ClosedLoopPoissonPurelyTemporalNullHypothesisRandomizer() {}

        virtual ClosedLoopPoissonPurelyTemporalNullHypothesisRandomizer * Clone() const {return new ClosedLoopPoissonPurelyTemporalNullHypothesisRandomizer(*this);}

        virtual void randomize(const RealDataSet& RealSet, const TwoDimMeasureArray_t& measure, DataSet& SimSet);
};

/** Poisson randomizer for null hypothesis applying the time stratified adjustment. */
class PoissonTimeStratifiedRandomizer : public PoissonRandomizer {
  protected:
     void  randomize(const RealDataSet& RealSet, const TwoDimMeasureArray_t& measure, const measure_t * pMeasurePerInterval, DataSet& SimSet);

  public:
    PoissonTimeStratifiedRandomizer(const CParameters & Parameters, long lInitialSeed=RandomNumberGenerator::glDefaultSeed)
        : PoissonRandomizer(Parameters, lInitialSeed) {}
    virtual ~PoissonTimeStratifiedRandomizer() {}
    virtual PoissonTimeStratifiedRandomizer * Clone() const {return new PoissonTimeStratifiedRandomizer(*this);}

    virtual void RandomizeData(const RealDataSet& RealSet, DataSet& SimSet, unsigned int iSimulation);
    virtual void randomize(const RealDataSet& RealSet, const TwoDimMeasureArray_t& measure, DataSet& SimSet);
};

/** Poisson randomizer for null hypothesis applying the spatial stratified adjustment. */
class PoissonSpatialStratifiedRandomizer : public PoissonRandomizer {
  public:
    PoissonSpatialStratifiedRandomizer(const CParameters & Parameters, long lInitialSeed=RandomNumberGenerator::glDefaultSeed)
        : PoissonRandomizer(Parameters, lInitialSeed) {}
    virtual ~PoissonSpatialStratifiedRandomizer() {}
    virtual PoissonSpatialStratifiedRandomizer * Clone() const {return new PoissonSpatialStratifiedRandomizer(*this);}

    virtual void RandomizeData(const RealDataSet& RealSet, DataSet& SimSet, unsigned int iSimulation);
    virtual void randomize(const RealDataSet& RealSet, const TwoDimMeasureArray_t& measure, DataSet& SimSet);
};

class RelativeRiskAdjustmentHandler;

/** Poisson randomizer for known relative risks adjustments. */
class AlternateHypothesisRandomizer : public PoissonRandomizer {
  protected:
    const CSaTScanData & gDataHub;
    boost::shared_ptr<RelativeRiskAdjustmentHandler> _riskAdjustments;
    boost::shared_ptr<PoissonRandomizer> _randomizer;

  public:
    AlternateHypothesisRandomizer(const CSaTScanData& gDataHub, 
                                  long lInitialSeed=RandomNumberGenerator::glDefaultSeed);
    AlternateHypothesisRandomizer(const CSaTScanData& gDataHub, 
                                  boost::shared_ptr<RelativeRiskAdjustmentHandler> & riskAdjustments, 
                                  long lInitialSeed=RandomNumberGenerator::glDefaultSeed);
    AlternateHypothesisRandomizer(const AlternateHypothesisRandomizer& rhs);
    virtual ~AlternateHypothesisRandomizer() {}
    
    virtual AlternateHypothesisRandomizer * Clone() const {return new AlternateHypothesisRandomizer(*this);}

    virtual void RandomizeData(const RealDataSet& RealSet, DataSet& SimSet, unsigned int iSimulation);
    virtual void randomize(const RealDataSet& RealSet, const TwoDimMeasureArray_t& measure, DataSet& SimSet);
};
//******************************************************************************
#endif
