//---------------------------------------------------------------------------
#include "SaTScan.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#include "BernoulliRandomizer.h"
#include "SaTScanData.h"

/** constructor */
BernoulliRandomizer::BernoulliRandomizer() : AbstractDenominaterDataRandomizer() {}

/** destructor */
BernoulliRandomizer::~BernoulliRandomizer() {}

/** constructor */
BernoulliNullHypothesisRandomizer::BernoulliNullHypothesisRandomizer() : BernoulliRandomizer() {}

/** destructor */
BernoulliNullHypothesisRandomizer::~BernoulliNullHypothesisRandomizer() {}

/** returns newly cloned BernoulliNullHypothesisRandomizer */
BernoulliNullHypothesisRandomizer * BernoulliNullHypothesisRandomizer::Clone() const {
  return new BernoulliNullHypothesisRandomizer(*this);
}

/** Each of the totalMeasure number of individuals (sum of cases and controls),
    are randomized to either be a case or a control. The output is an array with
    the indices of the TotalCounts number of cases. For example, if there are
    20 cases and 80 controls, the output is an array the the indices between
    0 and 99 that correspond to the randomized cases. (MK Oct 27, 2003)         */
void BernoulliNullHypothesisRandomizer::MakeDataB(count_t tTotalCounts, measure_t tTotalMeasure, std::vector<count_t>& RandCounts) {
  int           i;
  count_t       nCumCounts=0;
  double        x, ratio;

  RandCounts.resize(tTotalCounts);
  for (i=0; i < tTotalMeasure; ++i) {
     x = gRandomNumberGenerator.GetRandomDouble();
     ratio = (double) (tTotalCounts - nCumCounts) / (tTotalMeasure - i);
     if (x <= ratio) {
       RandCounts[nCumCounts] = i;
       nCumCounts++;
     }
  }
}

/** Randomizes data stream's data under null hypothesis for Bernoulli model. */
void BernoulliNullHypothesisRandomizer::RandomizeData(DataStream & thisStream, unsigned int iSimulation) {
  int                           t, tNumTracts = thisStream.GetNumTracts(),
                                i, c, tNumTimeIntervals = thisStream.GetNumTimeIntervals();
  count_t                    ** ppSimCases(thisStream.GetSimCaseArray()), nCumCounts, nCumMeasure,
                                tNumCases(thisStream.GetTotalCases()), tNumControls(thisStream.GetTotalControls());
  measure_t                  ** ppMeasure(thisStream.GetMeasureArray());
  std::vector<count_t>          RandCounts;

  // reset seed of random number generator 
  gRandomNumberGenerator.SetSeed(iSimulation + gRandomNumberGenerator.GetDefaultSeed());
  // reset simulation cases to zero
  thisStream.ResetCumulativeSimCaseArray();

  if (tNumCases < tNumControls)
    nCumCounts = tNumCases;
  else
    nCumCounts = tNumControls;

  MakeDataB(nCumCounts, tNumCases + tNumControls, RandCounts);
  nCumMeasure = tNumCases + tNumControls - 1;
  for (t=tNumTracts-1; t >= 0; --t) {
     for (i = tNumTimeIntervals-1; i >= 0; --i) {
        if (i == tNumTimeIntervals-1)
          nCumMeasure -= (count_t)(ppMeasure[i][t]);
        else
          nCumMeasure -= (count_t)(ppMeasure[i][t] - ppMeasure[i+1][t]);
        while (nCumCounts > 0 && RandCounts[nCumCounts-1] > nCumMeasure) {
             ppSimCases[i][t]++;
             nCumCounts--;
        }
     }
  }

  //set as cumulative
  for (i=tNumTimeIntervals-2; i >= 0; i--)
     for (t=0; t < tNumTracts; t++)
        ppSimCases[i][t] += ppSimCases[i+1][t];

  // Now reverse everything if Controls < Cases
  if (tNumCases >= tNumControls) {
    for (t=0; t < tNumTracts; ++t)
       for (i=0; i < tNumTimeIntervals; ++i)
          ppSimCases[i][t] = (count_t)(ppMeasure[i][t]) - ppSimCases[i][t];
  }
}
