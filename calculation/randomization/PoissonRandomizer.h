//---------------------------------------------------------------------------
#ifndef PoissonRandomizerH
#define PoissonRandomizerH
//---------------------------------------------------------------------------
#include "DenominatorDataRandomizer.h"
#include <iostream>
#include <fstream>

/** abstract base class for Poisson randomizers */
class PoissonRandomizer : public AbstractDenominatorDataRandomizer {
  protected:
    const CParameters & gParameters;

  public:
    PoissonRandomizer(long lInitialSeed, const CParameters & Parameters);
    virtual ~PoissonRandomizer();
};

/** Randomizes Poisson data stream under null hypothesis */
class PoissonNullHypothesisRandomizer : public PoissonRandomizer {
  public:
    PoissonNullHypothesisRandomizer(long lInitialSeed, const CParameters & Parameters);
    virtual ~PoissonNullHypothesisRandomizer();

    virtual PoissonNullHypothesisRandomizer * Clone() const;

    virtual void	RandomizeData(const RealDataStream& thisRealStream,
                                      SimulationDataStream& thisSimulationStream,
                                      unsigned int iSimulation);
};

/** Randomizes Poisson data stream in time stratified manner. */
class PoissonTimeStratifiedRandomizer : public PoissonRandomizer {
  public:
    PoissonTimeStratifiedRandomizer(long lInitialSeed, const CParameters & Parameters);
    virtual ~PoissonTimeStratifiedRandomizer();

    virtual PoissonTimeStratifiedRandomizer * Clone() const;
    virtual void	                      RandomizeData(const RealDataStream& thisRealStream,
                                                            SimulationDataStream& thisSimulationStream,
                                                            unsigned int iSimulation);
};

/** Randomizes Poisson data stream in spatial stratified manner. */
class PoissonSpatialStratifiedRandomizer : public PoissonRandomizer {
  public:
    PoissonSpatialStratifiedRandomizer(long lInitialSeed, const CParameters & Parameters);
    virtual ~PoissonSpatialStratifiedRandomizer();

    virtual PoissonSpatialStratifiedRandomizer * Clone() const;
    virtual void	                         RandomizeData(const RealDataStream& thisRealStream,
                                                               SimulationDataStream& thisSimulationStream,
                                                               unsigned int iSimulation);
};

class CSaTScanData; /** forward class declaration */

/** Randomizes Poisson data stream under alternate hypothesis.
    NOTE: This unit has note been thoughly tested, especially with multiple
          data streams. */
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
    AlternateHypothesisRandomizer(long lInitialSeed, CSaTScanData & Data);
    AlternateHypothesisRandomizer(const AlternateHypothesisRandomizer & rhs);
    virtual ~AlternateHypothesisRandomizer();

    virtual AlternateHypothesisRandomizer     * Clone() const;
    virtual void	                        RandomizeData(const RealDataStream& thisRealStream,
                                                              SimulationDataStream& thisSimulationStream,
                                                              unsigned int iSimulation);
};
//---------------------------------------------------------------------------
#endif
