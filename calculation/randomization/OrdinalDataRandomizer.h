//******************************************************************************
#ifndef __OrdinalDataRandomizer_H
#define __OrdinalDataRandomizer_H
//******************************************************************************
#include "DenominatorDataRandomizer.h"

/** Randomizes data of dataset for a 'Ordinal' probablility model. */
class OrdinalDenominatorDataRandomizer : public AbstractOrdinalDenominatorDataRandomizer {
  protected:
    void                SetMeasure(const RealDataSet& thisRealSet, TwoDimensionArrayHandler<measure_t>& Measure);

  public:
    OrdinalDenominatorDataRandomizer(long lInitialSeed=RandomNumberGenerator::glDefaultSeed);
    virtual ~OrdinalDenominatorDataRandomizer();
    virtual OrdinalDenominatorDataRandomizer * Clone() const;

    virtual void        RandomizeData(const RealDataSet& thisRealSet, SimDataSet& thisSimSet, unsigned int iSimulation);
};
//******************************************************************************
#endif
