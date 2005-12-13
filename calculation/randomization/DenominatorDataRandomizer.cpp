//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "DenominatorDataRandomizer.h"

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

/** Distributes cases into simulation case array, where individuals are initially dichotomized into cases and
    controls then each randomly assigned to be a case or a control. Caller is responsible for ensuring that
    passed array pointers are allocated and dimensions match that of passed tract and locations variables. */
void AbstractOrdinalDenominatorDataRandomizer::RandomizeOrdinalData(count_t tNumCases, count_t tNumControls, count_t** ppSimCases,
                                                                    measure_t** ppMeasure, int tNumTracts, int tNumTimeIntervals) {
  count_t               nCumCounts, nCumMeasure;
  std::vector<count_t>	RandCounts;

  nCumCounts = tNumCases < tNumControls ? tNumCases : tNumControls;
  MakeDataB(nCumCounts, tNumCases + tNumControls, RandCounts);
  nCumMeasure = tNumCases + tNumControls - 1;
  for (int t=tNumTracts-1; t >= 0; --t) {
     for (int i = tNumTimeIntervals-1; i >= 0; --i) {
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
  //now set as cumulative, in respect to time intervals
  for (int i=tNumTimeIntervals-2; i >= 0; i--)
     for (int t=0; t < tNumTracts; t++)
        ppSimCases[i][t] += ppSimCases[i+1][t];
  //now reverse everything if Controls < Cases
  if (tNumCases >= tNumControls) {
    for (int t=0; t < tNumTracts; ++t)
       for (int i=0; i < tNumTimeIntervals; ++i)
          ppSimCases[i][t] = (count_t)(ppMeasure[i][t]) - ppSimCases[i][t];
  }
}

/** Distributes cases into simulation case array, where individuals are initially dichotomized into cases and
    controls then each randomly assigned to be a case or a control. optimized for purely temporal analyses.
    Caller is responsible for ensuring that passed array pointers are allocated and dimensions match that of
    passed tract and locations variables. */
void AbstractOrdinalDenominatorDataRandomizer::RandomizePurelyTemporalOrdinalData(count_t tNumCases, count_t tNumControls,
                                                                                  count_t* pSimCases, measure_t* pMeasure, int tNumTimeIntervals) {
  count_t               nCumCounts, nCumMeasure;
  std::vector<count_t>	RandCounts;

  nCumCounts = tNumCases < tNumControls ? tNumCases : tNumControls;
  MakeDataB(nCumCounts, tNumCases + tNumControls, RandCounts);
  nCumMeasure = tNumCases + tNumControls - 1;
  for (int i = tNumTimeIntervals-1; i >= 0; --i) {
     nCumMeasure -= (i == tNumTimeIntervals - 1  ? (count_t)(pMeasure[i]) : (count_t)(pMeasure[i] - pMeasure[i+1]));
     while (nCumCounts > 0 && RandCounts[nCumCounts-1] > nCumMeasure) {
          ++pSimCases[i];
          --nCumCounts;
     }
  }
  //now set as cumulative, in respect to time intervals
  for (int i=tNumTimeIntervals-2; i >= 0; i--)
     pSimCases[i] += pSimCases[i+1];
  //now reverse everything if Controls < Cases
  if (tNumCases >= tNumControls)
    for (int i=0; i < tNumTimeIntervals; ++i)
       pSimCases[i] = (count_t)(pMeasure[i]) - pSimCases[i];
}


