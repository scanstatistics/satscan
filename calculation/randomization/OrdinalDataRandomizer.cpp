//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "OrdinalDataRandomizer.h"

/** constructor */
OrdinalDenominatorDataRandomizer::OrdinalDenominatorDataRandomizer() : AbstractOrdinalDenominatorDataRandomizer() {}

/** destructor */
OrdinalDenominatorDataRandomizer::~OrdinalDenominatorDataRandomizer() {}

/** returns pointer to newly cloned OrdinalDenominatorDataRandomizer */
OrdinalDenominatorDataRandomizer * OrdinalDenominatorDataRandomizer::Clone() const {
  return new OrdinalDenominatorDataRandomizer(*this);
}

/** Randomizes data sets' data for Ordinal model. */
void OrdinalDenominatorDataRandomizer::RandomizeData(const RealDataStream& RealSet, SimulationDataStream& SimSet, unsigned int iSimulation) {
  count_t        ** ppSimCases=0;
   measure_t     ** ppMeasure=0;
   unsigned int     c, i, t;

  // reset seed of random number generator
  SetSeed(iSimulation, SimSet.GetStreamIndex());
  //create measure array
  TwoDimensionArrayHandler<measure_t>  Measure(RealSet.GetNumTimeIntervals(), RealSet.GetNumTracts());
  SetMeasure(RealSet, Measure);
  ppMeasure = Measure.GetArray();
  //randomize data
  count_t tTotalControls = RealSet.GetTotalCases();
  for (size_t c=0; c < SimSet.GetCasesByCategory().size() - 1; ++c) {
     SimSet.GetCasesByCategory()[c]->Set(0);
     ppSimCases = SimSet.GetCasesByCategory()[c]->GetArray();
     tTotalControls -= RealSet.GetPopulationData().GetNumOrdinalCategoryCases(c);
     RandomizeOrdinalData(RealSet.GetPopulationData().GetNumOrdinalCategoryCases(c), tTotalControls,
                          ppSimCases, ppMeasure, RealSet.GetNumTracts(), RealSet.GetNumTimeIntervals());
     //update measure so that assigned cases are removed from measure
     for (i=0; i < RealSet.GetNumTimeIntervals(); ++i)
        for (t=0; t < RealSet.GetNumTracts(); ++t)
           ppMeasure[i][t] -= ppSimCases[i][t];
  }
  //after randomizing all categories except last, remaining cases can just be assigned respective category array
  ppSimCases = SimSet.GetCasesByCategory().back()->GetArray();
  for (i=0; i < RealSet.GetNumTimeIntervals(); ++i)
     for (t=0; t < RealSet.GetNumTracts(); ++t)
        ppSimCases[i][t] = static_cast<count_t>(ppMeasure[i][t]);
}

/** Calculates the total measure for ordinal data set, which is the cases in all categories added together. */
void OrdinalDenominatorDataRandomizer::SetMeasure(const RealDataStream& RealSet, TwoDimensionArrayHandler<measure_t>& Measure) {
  count_t       ** ppCases=0;
  measure_t     ** ppMeasure = Measure.GetArray();
  size_t           c, i, t;

  Measure.Set(0);
  //assign measure - total cases in all categories
  for (c=0; c < RealSet.GetCasesByCategory().size(); ++c) {
     ppCases = RealSet.GetCasesByCategory()[c]->GetArray();
     for (i=0; i < RealSet.GetNumTimeIntervals(); ++i)
        for (t=0; t < RealSet.GetNumTracts(); ++t)
           ppMeasure[i][t] += ppCases[i][t];
  }
}

