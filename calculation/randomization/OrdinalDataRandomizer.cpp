//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "OrdinalDataRandomizer.h"

/** constructor */
OrdinalDenominatorDataRandomizer::OrdinalDenominatorDataRandomizer(long lInitialSeed)
                                 :AbstractOrdinalDenominatorDataRandomizer(lInitialSeed) {}

/** destructor */
OrdinalDenominatorDataRandomizer::~OrdinalDenominatorDataRandomizer() {}

/** returns pointer to newly cloned OrdinalDenominatorDataRandomizer */
OrdinalDenominatorDataRandomizer * OrdinalDenominatorDataRandomizer::Clone() const {
  return new OrdinalDenominatorDataRandomizer(*this);
}

/** Randomizes data sets' data for Ordinal model. */
void OrdinalDenominatorDataRandomizer::RandomizeData(const RealDataSet& thisRealSet, SimDataSet& thisSimSet, unsigned int iSimulation) {
  count_t        ** ppSimCases=0;
   measure_t     ** ppMeasure=0;
   unsigned int     c, i, t;

  // reset seed of random number generator
  SetSeed(iSimulation, thisSimSet.GetSetIndex());
  //create measure array
  TwoDimensionArrayHandler<measure_t>  Measure(thisRealSet.GetNumTimeIntervals(), thisRealSet.GetNumTracts());
  SetMeasure(thisRealSet, Measure);
  ppMeasure = Measure.GetArray();
  //randomize data
  count_t tTotalControls = thisRealSet.GetTotalCases();
  for (size_t c=0; c < thisSimSet.GetCasesByCategory().size() - 1; ++c) {
     thisSimSet.GetCasesByCategory()[c]->Set(0);
     ppSimCases = thisSimSet.GetCasesByCategory()[c]->GetArray();
     tTotalControls -= thisRealSet.GetPopulationData().GetNumOrdinalCategoryCases(c);
     RandomizeOrdinalData(thisRealSet.GetPopulationData().GetNumOrdinalCategoryCases(c), tTotalControls,
                          ppSimCases, ppMeasure, thisRealSet.GetNumTracts(), thisRealSet.GetNumTimeIntervals());
     //update measure so that assigned cases are removed from measure
     for (i=0; i < thisRealSet.GetNumTimeIntervals(); ++i)
        for (t=0; t < thisRealSet.GetNumTracts(); ++t)
           ppMeasure[i][t] -= ppSimCases[i][t];
  }
  //after randomizing all categories except last, remaining cases can just be assigned respective category array
  ppSimCases = thisSimSet.GetCasesByCategory().back()->GetArray();
  for (i=0; i < thisRealSet.GetNumTimeIntervals(); ++i)
     for (t=0; t < thisRealSet.GetNumTracts(); ++t)
        ppSimCases[i][t] = static_cast<count_t>(ppMeasure[i][t]);
}

/** Calculates the total measure for ordinal data set, which is the cases in all categories added together. */
void OrdinalDenominatorDataRandomizer::SetMeasure(const RealDataSet& thisRealSet, TwoDimensionArrayHandler<measure_t>& Measure) {
  count_t       ** ppCases=0;
  measure_t     ** ppMeasure = Measure.GetArray();
  size_t           c, i, t;

  Measure.Set(0);
  //assign measure - total cases in all categories
  for (c=0; c < thisRealSet.GetCasesByCategory().size(); ++c) {
     ppCases = thisRealSet.GetCasesByCategory()[c]->GetArray();
     for (i=0; i < thisRealSet.GetNumTimeIntervals(); ++i)
        for (t=0; t < thisRealSet.GetNumTracts(); ++t)
           ppMeasure[i][t] += ppCases[i][t];
  }
}

