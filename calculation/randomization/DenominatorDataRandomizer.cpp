//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "DenominatorDataRandomizer.h"

/** constructor */
AbstractDenominatorDataRandomizer::AbstractDenominatorDataRandomizer(long lInitialSeed)
                  :AbstractRandomizer(lInitialSeed) {}

/** destructor */
AbstractDenominatorDataRandomizer::~AbstractDenominatorDataRandomizer() {}

//******************************************************************************

/** constructor */
AbstractOrdinalDenominatorDataRandomizer::AbstractOrdinalDenominatorDataRandomizer(long lInitialSeed)
                  :AbstractDenominatorDataRandomizer(lInitialSeed) {}

/** destructor */
AbstractOrdinalDenominatorDataRandomizer::~AbstractOrdinalDenominatorDataRandomizer() {}

/** Each of the totalMeasure number of individuals (sum of cases and controls),
    are randomized to either be a case or a control. The output is an array with
    the indices of the TotalCounts number of cases. For example, if there are
    20 cases and 80 controls, the output is an array the the indices between
    0 and 99 that correspond to the randomized cases. (MK Oct 27, 2003)         */
void AbstractOrdinalDenominatorDataRandomizer::MakeDataB(count_t tTotalCounts, measure_t tTotalMeasure, std::vector<count_t>& RandCounts) {
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

/** TODO: document */
void AbstractOrdinalDenominatorDataRandomizer::RandomizeOrdinalData(count_t tNumCases, count_t tNumControls,
                                                                    count_t** ppSimCases, measure_t** ppMeasure,
                                                                    int tNumTracts, int tNumTimeIntervals) {
                                          
  int                   t, i, c;
  count_t               nCumCounts, nCumMeasure;
  std::vector<count_t>	RandCounts;

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

