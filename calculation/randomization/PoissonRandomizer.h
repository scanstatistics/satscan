//******************************************************************************
#ifndef __PoissonRandomizer_H
#define __PoissonRandomizer_H
//******************************************************************************
#include "DenominatorDataRandomizer.h"
#include <iostream>
#include <fstream>

/** abstract base class for Poisson randomizers */
class PoissonRandomizer : public AbstractDenominatorDataRandomizer {
  protected:
    const CParameters & gParameters;

  public:
    PoissonRandomizer(const CParameters & Parameters);
    virtual ~PoissonRandomizer();
};

/** Randomizes Poisson dataset under null hypothesis */
class PoissonNullHypothesisRandomizer : public PoissonRandomizer {
  public:
    PoissonNullHypothesisRandomizer(const CParameters & Parameters);
    virtual ~PoissonNullHypothesisRandomizer();
    virtual PoissonNullHypothesisRandomizer * Clone() const;

    virtual void  RandomizeData(const RealDataSet& thisRealSet, SimDataSet& thisSimSet, unsigned int iSimulation);
};

/** Randomizes Poisson dataset in time stratified manner. */
class PoissonTimeStratifiedRandomizer : public PoissonRandomizer {
  public:
    PoissonTimeStratifiedRandomizer(const CParameters & Parameters);
    virtual ~PoissonTimeStratifiedRandomizer();
    virtual PoissonTimeStratifiedRandomizer * Clone() const;

    virtual void  RandomizeData(const RealDataSet& thisRealSet, SimDataSet& thisSimSet, unsigned int iSimulation);
};

/** Randomizes Poisson dataset in spatial stratified manner. */
class PoissonSpatialStratifiedRandomizer : public PoissonRandomizer {
  public:
    PoissonSpatialStratifiedRandomizer(const CParameters & Parameters);
    virtual ~PoissonSpatialStratifiedRandomizer();
    virtual PoissonSpatialStratifiedRandomizer * Clone() const;

    virtual void  RandomizeData(const RealDataSet& thisRealSet, SimDataSet& thisSimSet, unsigned int iSimulation);
};

class CSaTScanData; /** forward class declaration */

/** Randomizes Poisson dataset under alternate hypothesis.
    NOTE: This unit has note been thoughly tested, especially with multiple
          datasets. */
class AlternateHypothesisRandomizer : public PoissonRandomizer {
  private:
    void                                        Init() {gpAlternativeMeasure=0;}
    void                                        Setup();

  protected:
    std::vector<float>                          gvRelativeRisks;
    std::vector<measure_t>                      gvMeasure;
    TwoDimensionArrayHandler<measure_t>       * gpAlternativeMeasure;
    CSaTScanData                              & gData;

  public:
    AlternateHypothesisRandomizer(CSaTScanData & Data);
    AlternateHypothesisRandomizer(const AlternateHypothesisRandomizer & rhs);
    virtual ~AlternateHypothesisRandomizer();
    virtual AlternateHypothesisRandomizer     * Clone() const;

    virtual void	                        RandomizeData(const RealDataSet& thisRealSet, SimDataSet& thisSimSet, unsigned int iSimulation);
};
//******************************************************************************
#endif

