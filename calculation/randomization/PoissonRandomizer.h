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
};

/** Poisson randomizer for null hypothesis. */
class PoissonNullHypothesisRandomizer : public PoissonRandomizer {
  public:
    PoissonNullHypothesisRandomizer(const CParameters & Parameters, long lInitialSeed=RandomNumberGenerator::glDefaultSeed)
       : PoissonRandomizer(Parameters, lInitialSeed) {}
    virtual ~PoissonNullHypothesisRandomizer() {}

    virtual PoissonNullHypothesisRandomizer * Clone() const {return new PoissonNullHypothesisRandomizer(*this);}

    virtual void  RandomizeData(const RealDataSet& RealSet, DataSet& SimSet, unsigned int iSimulation);
};

/** Poisson randomizer for null hypothesis. Optimized for purely temporal analyses. */
class PoissonPurelyTemporalNullHypothesisRandomizer : public PoissonRandomizer {
  public:
    PoissonPurelyTemporalNullHypothesisRandomizer(const CParameters & Parameters, long lInitialSeed=RandomNumberGenerator::glDefaultSeed)
       : PoissonRandomizer(Parameters, lInitialSeed) {}
    virtual ~PoissonPurelyTemporalNullHypothesisRandomizer() {}

    virtual PoissonPurelyTemporalNullHypothesisRandomizer * Clone() const {return new PoissonPurelyTemporalNullHypothesisRandomizer(*this);}

    virtual void  RandomizeData(const RealDataSet& RealSet, DataSet& SimSet, unsigned int iSimulation);
};

/** Poisson randomizer for null hypothesis applying the time stratified adjustment. */
class PoissonTimeStratifiedRandomizer : public PoissonRandomizer {
  public:
    PoissonTimeStratifiedRandomizer(const CParameters & Parameters, long lInitialSeed=RandomNumberGenerator::glDefaultSeed)
        : PoissonRandomizer(Parameters, lInitialSeed) {}
    virtual ~PoissonTimeStratifiedRandomizer() {}
    virtual PoissonTimeStratifiedRandomizer * Clone() const {return new PoissonTimeStratifiedRandomizer(*this);}

    virtual void  RandomizeData(const RealDataSet& RealSet, DataSet& SimSet, unsigned int iSimulation);
};

/** Poisson randomizer for null hypothesis applying the spatial stratified adjustment. */
class PoissonSpatialStratifiedRandomizer : public PoissonRandomizer {
  public:
    PoissonSpatialStratifiedRandomizer(const CParameters & Parameters, long lInitialSeed=RandomNumberGenerator::glDefaultSeed)
        : PoissonRandomizer(Parameters, lInitialSeed) {}
    virtual ~PoissonSpatialStratifiedRandomizer() {}
    virtual PoissonSpatialStratifiedRandomizer * Clone() const {return new PoissonSpatialStratifiedRandomizer(*this);}

    virtual void  RandomizeData(const RealDataSet& RealSet, DataSet& SimSet, unsigned int iSimulation);
};

class CSaTScanData; /** forward class declaration */

/** Poisson randomizer for alternate hypothesis.
    !!! This unit has note been thoughly tested, especially with multiple datasets. !!! */
class AlternateHypothesisRandomizer : public PoissonRandomizer {
  protected:
    std::vector<double>                         gvRelativeRisks;
    std::vector<measure_t>                      gvMeasure;
    TwoDimensionArrayHandler<measure_t>         gAlternativeMeasure;
    CSaTScanData                              & gDataHub;

  public:
    AlternateHypothesisRandomizer(CSaTScanData& gDataHub, long lInitialSeed=RandomNumberGenerator::glDefaultSeed);
    AlternateHypothesisRandomizer(const AlternateHypothesisRandomizer& rhs);
    virtual ~AlternateHypothesisRandomizer() {}
    
    virtual AlternateHypothesisRandomizer     * Clone() const {return new AlternateHypothesisRandomizer(*this);}

    virtual void	                        RandomizeData(const RealDataSet& RealSet, DataSet& SimSet, unsigned int iSimulation);
};
//******************************************************************************
#endif

