//******************************************************************************
#ifndef __OrdinalDataRandomizer_H
#define __OrdinalDataRandomizer_H
//******************************************************************************
#include "DenominatorDataRandomizer.h"

/** Randomizes data of data set for a 'Ordinal' probablility model. */
class OrdinalDenominatorDataRandomizer : public AbstractOrdinalDenominatorDataRandomizer {
  protected:
    void                SetMeasure(const RealDataStream& RealSet, TwoDimensionArrayHandler<measure_t>& Measure);

  public:
    OrdinalDenominatorDataRandomizer();
    virtual ~OrdinalDenominatorDataRandomizer();

    virtual OrdinalDenominatorDataRandomizer * Clone() const;

    virtual void        RandomizeData(const RealDataStream& RealSet, SimulationDataStream& SimSet, unsigned int iSimulation);
};
//******************************************************************************
#endif
