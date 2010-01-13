//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "PoissonRandomizer.h"
#include "SaTScanData.h"
#include "SSException.h" 

/** Creates randomized under the null hypothesis for Poisson model, assigning data to DataSet objects structures.
    Random number generator seed initialized based upon 'iSimulation' index. */
void PoissonNullHypothesisRandomizer::RandomizeData(const RealDataSet& RealSet, DataSet& SimSet, unsigned int iSimulation) {
  unsigned int          t, tNumTracts = RealSet.getLocationDimension(),
                        i, tNumTimeIntervals = RealSet.getIntervalDimension();
  count_t               c, d, cumcases=0, tTotalCases = RealSet.getTotalCases(), ** ppSimCases = SimSet.getCaseData().GetArray();
  measure_t             cummeasure=0, tTotalMeasure = RealSet.getTotalMeasure(), ** ppMeasure= RealSet.getMeasureData().GetArray();

  SetSeed(iSimulation, SimSet.getSetIndex());
  for (t=0; t < tNumTracts; ++t) {
     if (tTotalMeasure-cummeasure > 0)
       c = gBinomialGenerator.GetBinomialDistributedVariable(tTotalCases - cumcases,
                              ppMeasure[0][t] / (tTotalMeasure-cummeasure), gRandomNumberGenerator);
     else
      c = 0;
     ppSimCases[0][t] = c;
     cumcases += c;
     cummeasure += ppMeasure[0][t];
     for (i=0; i < tNumTimeIntervals-1; ++i) {
        if (ppMeasure[i][t] > 0)
          d = gBinomialGenerator.GetBinomialDistributedVariable(ppSimCases[i][t],
                                                                1 - ppMeasure[i+1][t] / ppMeasure[i][t],
                                                                gRandomNumberGenerator);
        else
          d = 0;
        ppSimCases[i+1][t] = ppSimCases[i][t] - d;
    }
  }
}

//******************************************************************************

/** Creates randomized under the null hypothesis for Poisson model, assigning data to DataSet objects structures.
    Random number generator seed initialized based upon 'iSimulation' index. */
void PoissonPurelyTemporalNullHypothesisRandomizer::RandomizeData(const RealDataSet& RealSet, DataSet& SimSet, unsigned int iSimulation) {
  unsigned int          i, tNumTimeIntervals = RealSet.getIntervalDimension();
  count_t               d, tTotalCases = RealSet.getTotalCases();
  measure_t             tTotalMeasure = RealSet.getTotalMeasure();

  SimSet.allocateCaseData_PT();
  count_t * pSimPTCases = SimSet.getCaseData_PT();
  measure_t * pPTMeasure = RealSet.getMeasureData_PT();
  SetSeed(iSimulation, SimSet.getSetIndex());
  pSimPTCases[0] = gBinomialGenerator.GetBinomialDistributedVariable(tTotalCases, pPTMeasure[0]/tTotalMeasure, gRandomNumberGenerator);
  for (i=0; i < tNumTimeIntervals-1; ++i) {
    if (pPTMeasure[i] > 0)
      d = gBinomialGenerator.GetBinomialDistributedVariable(pSimPTCases[i], 1 - pPTMeasure[i+1]/pPTMeasure[i], gRandomNumberGenerator);
    else
      d = 0;
    pSimPTCases[i+1] = pSimPTCases[i] - d;
  }
}

//******************************************************************************

/** Creates randomized under the null hypothesis for Poisson model, non-parametric (time stratified), and assigning
    data to DataSet objects structures. Random number generator seed initialized based upon 'iSimulation' index. */
void PoissonTimeStratifiedRandomizer::RandomizeData(const RealDataSet& RealSet, DataSet& SimSet, unsigned int iSimulation) {
  unsigned int          tract, tNumTracts = RealSet.getLocationDimension();
  count_t               c, cumcases=0, * pCasesPerInterval = RealSet.getCaseData_PT_NC(), ** ppSimCases = SimSet.getCaseData().GetArray();
  measure_t             cummeasure=0, * pMeasurePerInterval = RealSet.getMeasureData_PT_NC(), ** ppMeasure = RealSet.getMeasureData().GetArray();
  int                   interval;

  SetSeed(iSimulation, SimSet.getSetIndex());
  interval = RealSet.getIntervalDimension() - 1;
  for (tract=0; tract < tNumTracts; ++tract) {
     if (pCasesPerInterval[interval] - cumcases > 0)
       c = gBinomialGenerator.GetBinomialDistributedVariable(pCasesPerInterval[interval] - cumcases,
                                                             ppMeasure[interval][tract]/(pMeasurePerInterval[interval] - cummeasure),
                                                             gRandomNumberGenerator);
     else
       c = 0;
     cumcases += c;
     cummeasure += ppMeasure[interval][tract];
     ppSimCases[interval][tract] = c;
  }
  for (interval--; interval >= 0; --interval) { //For each other interval, from 2nd to last until the first:
     cumcases = 0;
     cummeasure = 0;
     for (tract=0; tract < tNumTracts; ++tract) { //For each tract:
       if (pCasesPerInterval[interval] - cumcases > 0)
          c = gBinomialGenerator.GetBinomialDistributedVariable(pCasesPerInterval[interval] - cumcases,
                      (ppMeasure[interval][tract] - ppMeasure[interval + 1][tract])/(pMeasurePerInterval[interval] - cummeasure),
                      gRandomNumberGenerator);
        else
          c = 0;
        cumcases += c;
        cummeasure += (ppMeasure[interval][tract] - ppMeasure[interval + 1][tract]);
        ppSimCases[interval][tract] = c + ppSimCases[interval + 1][tract];
     }
 }
}

//******************************************************************************

/** Creates randomized under the null hypothesis for Poisson model, non-parametric (location stratified), and assigning
    data to DataSet objects structures. Random number generator seed initialized based upon 'iSimulation' index. */
void PoissonSpatialStratifiedRandomizer::RandomizeData(const RealDataSet& RealSet, DataSet& SimSet, unsigned int iSimulation) {
  int                   interval;
  unsigned int          tract;
  count_t               tCases, tCumCases=0, ** ppCases = RealSet.getCaseData().GetArray(), ** ppSimCases = SimSet.getCaseData().GetArray();
  measure_t             tCumMeasure=0, ** ppMeasure = RealSet.getMeasureData().GetArray();

  //reset seed for simulation number
  SetSeed(iSimulation, SimSet.getSetIndex());
  //create randomized data for each tract's last time interval
  interval = RealSet.getIntervalDimension() - 1;
  for (tract=0; tract < RealSet.getLocationDimension(); ++tract) {
     if (ppCases[0][tract])
       ppSimCases[interval][tract] = gBinomialGenerator.GetBinomialDistributedVariable(ppCases[0][tract],
                                           ppMeasure[interval][tract]/(ppMeasure[0][tract]), gRandomNumberGenerator);
     else
       ppSimCases[interval][tract] = 0;
  }
  //create randomized data for each tract's remaining time intervals
  for (tract=0; tract < RealSet.getLocationDimension(); ++tract) {
     tCumCases = 0;
     tCumMeasure = 0;
     interval = RealSet.getIntervalDimension() - 2;
     for (; interval >= 0; --interval) {
        if (ppCases[0][tract] - tCumCases)
          tCases = gBinomialGenerator.GetBinomialDistributedVariable(ppCases[0][tract] - tCumCases,
                     (ppMeasure[interval][tract] - ppMeasure[interval+1][tract])/(ppMeasure[0][tract] - tCumMeasure),
                     gRandomNumberGenerator);
        else
          tCases = 0;
        tCumCases += tCases;
        tCumMeasure += ppMeasure[interval][tract] - ppMeasure[interval+1][tract];
        ppSimCases[interval][tract] = tCases + ppSimCases[interval+1][tract];
     }
  }
}

//******************************************************************************

/** constructor */
AlternateHypothesisRandomizer::AlternateHypothesisRandomizer(CSaTScanData& DataHub, long lInitialSeed)
                              :PoissonRandomizer(DataHub.GetParameters(), lInitialSeed), gDataHub(DataHub),
                               gvRelativeRisks(DataHub.GetNumTracts(), 0), gvMeasure(DataHub.GetNumTracts(), 0),
                               gAlternativeMeasure(DataHub.GetNumTimeIntervals(), DataHub.GetNumTracts(), 0) {}

/** copy constructor */
AlternateHypothesisRandomizer::AlternateHypothesisRandomizer(const AlternateHypothesisRandomizer& rhs)
                              :PoissonRandomizer(rhs), gDataHub(rhs.gDataHub),
                               gAlternativeMeasure(rhs.gDataHub.GetNumTimeIntervals(), rhs.gDataHub.GetNumTracts(), 0) {
  gvRelativeRisks = rhs.gvRelativeRisks;
  gvMeasure = rhs.gvMeasure;
  gAlternativeMeasure = rhs.gAlternativeMeasure;
}

/** Creates randomized under an alternative hypothesis for Poisson model and assigning data to DataSet
    objects structures. Random number generator seed initialized based upon 'iSimulation' index.
    !!! This class is in the experiential stage. !!! */
void AlternateHypothesisRandomizer::RandomizeData(const RealDataSet& RealSet, DataSet& SimSet, unsigned int iSimulation) {
  unsigned int          j, t, i, tNumTracts = RealSet.getLocationDimension(), tNumTimeIntervals = RealSet.getIntervalDimension();
  std::ifstream         RelativeRiskFile;
  std::string           sTractId;
  tract_t               tractIndex;
  measure_t             cummeasure=0, TotalMeasure = RealSet.getTotalMeasure(), ** ppMeasure = RealSet.getMeasureData().GetArray();
  count_t               c, d, cumcases=0, ** ppSimCases = SimSet.getCaseData().GetArray();

  //duplicate the  ppMeasure[][] into gpAlternativeMeasure[][], gpAlternativeMeasure[][] will be changed depending upon
  //the gvRelativeRisks[], and ppMeasure[][] remains the same as the expected measure
  for (t=0; t < tNumTracts; ++t)
     for (i=0; i < tNumTimeIntervals; ++i)
        gAlternativeMeasure.GetArray()[i][t] = ppMeasure[i][t];

  //initialize the gvRelativeRisks[] to be 1.0
  for (t=0; t < tNumTracts; ++t)
     gvRelativeRisks[t] = 1.0;

  //read in the RR's for those tracts with higher risks
  RelativeRiskFile.open(gDataHub.GetParameters().GetAdjustmentsByRelativeRisksFilename().c_str());
  while (!RelativeRiskFile.eof()) {
       RelativeRiskFile >> sTractId;
       if ((tractIndex = gDataHub.GetTInfo()->getLocationIndex(sTractId.c_str())) == -1)
         throw resolvable_error("The Location ID '%s', in power estimation file, is not specified the coordinates file.", sTractId.c_str());
        RelativeRiskFile >> gvRelativeRisks[tractIndex];
  }
  RelativeRiskFile.close();

  //modify the measures
 for (t=0; t < tNumTracts; ++t) {
     gvMeasure[t] = ppMeasure[0][t];
     for (i=tNumTimeIntervals-1; i >= 30/* what is this constant? */ ; i--) {
        if (i == tNumTimeIntervals-1) {//if the last interval, the cummulative measure is the measure itself
          gvMeasure[t] = gvMeasure[t] + ppMeasure[i][t] * (gvRelativeRisks[t]-1);
          TotalMeasure = TotalMeasure + ppMeasure[i][t] * (gvRelativeRisks[t]-1);
          for (j=0; j <= i; ++j)
             gAlternativeMeasure.GetArray()[j][t] += ppMeasure[i][t] * (gvRelativeRisks[t]-1);
        }
        else {
          //if not the last interval, the measure belongs to the interval  is the difference between
          //the measure of this interval and the measure for next interval, measure[] and TotalMeasure
          //should change accordingly.
          gvMeasure[t] = gvMeasure[t] + (ppMeasure[i][t] - ppMeasure[i+1][t]) * (gvRelativeRisks[t]-1);
          TotalMeasure = TotalMeasure + (ppMeasure[i][t] - ppMeasure[i+1][t]) * (gvRelativeRisks[t]-1);
          for (j=0; j <= i; ++j)
             gAlternativeMeasure.GetArray()[j][t] += (ppMeasure[i][t] - ppMeasure[i+1][t]) * (gvRelativeRisks[t]-1);
        }
     }
  }

   //start alternative simulations
  for (t=0; t < tNumTracts; ++t) {
    if (TotalMeasure-cummeasure > 0)
        c = gBinomialGenerator.GetBinomialDistributedVariable(RealSet.getTotalCases() - cumcases,
                                                              gvMeasure[t] / (TotalMeasure-cummeasure),
                                                              gRandomNumberGenerator);
    else
      c = 0;
    ppSimCases[0][t] = c;
    cumcases += c;
    cummeasure += gvMeasure[t];

   for (i=0; i < tNumTimeIntervals-1; ++i) {
       if (gAlternativeMeasure.GetArray()[i][t] > 0)
        d = gBinomialGenerator.GetBinomialDistributedVariable(ppSimCases[i][t],
              1 - gAlternativeMeasure.GetArray()[i+1][t] / gAlternativeMeasure.GetArray()[i][t], gRandomNumberGenerator);
      else
        d = 0;

      ppSimCases[i+1][t] = ppSimCases[i][t] - d;
    }
  }
}

