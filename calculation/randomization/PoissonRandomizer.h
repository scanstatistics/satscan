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

    void                DumpDateToFile(DataStream & thisStream, int iSimulation);

  public:
    PoissonRandomizer(const CParameters & Parameters);
    virtual ~PoissonRandomizer();
};

/** Randomizes Poisson data stream under null hypothesis */
class PoissonNullHypothesisRandomizer : public PoissonRandomizer {
  public:
    PoissonNullHypothesisRandomizer(const CParameters & Parameters);
    virtual ~PoissonNullHypothesisRandomizer();

    virtual PoissonNullHypothesisRandomizer * Clone() const;

    virtual void	RandomizeData(DataStream & thisStream, unsigned int iSimulation);
};

/** Randomizes Poisson data stream in time stratified manner. */
class PoissonTimeStratifiedRandomizer : public PoissonRandomizer {
  public:
    PoissonTimeStratifiedRandomizer(const CParameters & Parameters);
    virtual ~PoissonTimeStratifiedRandomizer();

    virtual PoissonTimeStratifiedRandomizer * Clone() const;
    virtual void	                      RandomizeData(DataStream & thisStream, unsigned int iSimulation);
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
    AlternateHypothesisRandomizer(CSaTScanData & Data);
    AlternateHypothesisRandomizer(const AlternateHypothesisRandomizer & rhs);
    virtual ~AlternateHypothesisRandomizer();

    virtual AlternateHypothesisRandomizer     * Clone() const;
    virtual void	                        RandomizeData(DataStream & thisStream, unsigned int iSimulation);
};

/** Reads simulation data from file.
    NOTE: This unit has note been thoughly tested, especially with multiple
          data streams. */
class FileSourceRandomizer : public PoissonRandomizer {
  protected:
    ifstream                                    gSimulationDataInputFile;
    CSaTScanData                              & gData;  

  public:
    	    FileSourceRandomizer(CSaTScanData & Data);
    	    FileSourceRandomizer(const FileSourceRandomizer & rhs);
    virtual ~FileSourceRandomizer();

    virtual FileSourceRandomizer     * Clone() const;
    virtual void	               RandomizeData(DataStream & thisStream, unsigned int iSimulation);
};
//---------------------------------------------------------------------------
#endif
