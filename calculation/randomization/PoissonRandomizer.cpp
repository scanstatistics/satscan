//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "PoissonRandomizer.h"
#include "SaTScanData.h"
#include "SSException.h" 
#include "AdjustmentHandler.h"
#include "SaTScanDataRead.h"

void PoissonNullHypothesisRandomizer::randomize(const RealDataSet& RealSet, const TwoDimMeasureArray_t& measure, DataSet& SimSet) {
  unsigned int          t, tNumTracts = RealSet.getLocationDimension(),
                        i, tNumTimeIntervals = RealSet.getIntervalDimension();
  count_t               c, d, cumcases=0, tTotalCases = RealSet.getTotalCases(), ** ppSimCases = SimSet.getCaseData().GetArray();
  measure_t             cummeasure=0, tTotalMeasure = RealSet.getTotalMeasure(), ** ppMeasure= measure.GetArray();

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

/** Creates randomized under the null hypothesis for Poisson model, assigning data to DataSet objects structures.
    Random number generator seed initialized based upon 'iSimulation' index. */
void PoissonNullHypothesisRandomizer::RandomizeData(const RealDataSet& RealSet, DataSet& SimSet, unsigned int iSimulation) {
  SetSeed(iSimulation, SimSet.getSetIndex());
  randomize(RealSet, RealSet.getMeasureData(), SimSet);
}

//******************************************************************************

void PoissonPurelyTemporalNullHypothesisRandomizer::randomize(const RealDataSet& RealSet, const measure_t * pPTMeasure, DataSet& SimSet) {
  unsigned int          i, tNumTimeIntervals = RealSet.getIntervalDimension();
  count_t               d, tTotalCases = RealSet.getTotalCases();
  measure_t             tTotalMeasure = RealSet.getTotalMeasure();

  SimSet.allocateCaseData_PT();
  count_t * pSimPTCases = SimSet.getCaseData_PT();
  pSimPTCases[0] = gBinomialGenerator.GetBinomialDistributedVariable(tTotalCases, pPTMeasure[0]/tTotalMeasure, gRandomNumberGenerator);
  for (i=0; i < tNumTimeIntervals-1; ++i) {
    if (pPTMeasure[i] > 0)
      d = gBinomialGenerator.GetBinomialDistributedVariable(pSimPTCases[i], 1 - pPTMeasure[i+1]/pPTMeasure[i], gRandomNumberGenerator);
    else
      d = 0;
    pSimPTCases[i+1] = pSimPTCases[i] - d;
  }
}

void PoissonPurelyTemporalNullHypothesisRandomizer::randomize(const RealDataSet& RealSet, const TwoDimMeasureArray_t& measure, DataSet& SimSet) {
  std::vector<measure_t> vMPT(RealSet.getIntervalDimension(), 0);
  measure_t ** ppMeasure = measure.GetArray();
  for (unsigned int i=0; i < RealSet.getIntervalDimension(); ++i)
      for (unsigned int t=0; t < RealSet.getLocationDimension(); ++t)
        vMPT[i] += ppMeasure[i][t];
  randomize(RealSet, &vMPT[0], SimSet);  
}

/** Creates randomized under the null hypothesis for Poisson model, assigning data to DataSet objects structures.
    Random number generator seed initialized based upon 'iSimulation' index. */
void PoissonPurelyTemporalNullHypothesisRandomizer::RandomizeData(const RealDataSet& RealSet, DataSet& SimSet, unsigned int iSimulation) {
  SetSeed(iSimulation, SimSet.getSetIndex());
  randomize(RealSet, RealSet.getMeasureData_PT(), SimSet);  
}

//******************************************************************************

void PoissonTimeStratifiedRandomizer::randomize(const RealDataSet& RealSet, const TwoDimMeasureArray_t& measure, const measure_t * pMeasurePerInterval, DataSet& SimSet) {
  unsigned int tract, tNumTracts = RealSet.getLocationDimension();
  count_t c, cumcases=0, * pCasesPerInterval = RealSet.getCaseData_PT_NC(), ** ppSimCases = SimSet.getCaseData().GetArray();
  measure_t cummeasure=0, ** ppMeasure = measure.GetArray();
  int interval = RealSet.getIntervalDimension() - 1;

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

void PoissonTimeStratifiedRandomizer::randomize(const RealDataSet& RealSet, const TwoDimMeasureArray_t& measure, DataSet& SimSet) {
  std::vector<measure_t> vMPI(RealSet.getIntervalDimension(), 0);
  measure_t ** ppMeasure = measure.GetArray();
  for (unsigned int t=0; t < RealSet.getIntervalDimension(); ++t) {
    vMPI[RealSet.getIntervalDimension()-1] += ppMeasure[RealSet.getIntervalDimension()-1][t];
    for (unsigned int i=0; i < RealSet.getIntervalDimension() - 1; ++i)
        vMPI[i] += ppMeasure[i][t] - ppMeasure[i+1][t];
  }
  randomize(RealSet, measure, &vMPI[0], SimSet);
}

/** Creates randomized under the null hypothesis for Poisson model, non-parametric (time stratified), and assigning
    data to DataSet objects structures. Random number generator seed initialized based upon 'iSimulation' index. */
void PoissonTimeStratifiedRandomizer::RandomizeData(const RealDataSet& RealSet, DataSet& SimSet, unsigned int iSimulation) {
  SetSeed(iSimulation, SimSet.getSetIndex());
  randomize(RealSet, RealSet.getMeasureData(), SimSet);
}

//******************************************************************************

void PoissonSpatialStratifiedRandomizer::randomize(const RealDataSet& RealSet, const TwoDimMeasureArray_t& measure, DataSet& SimSet) {
  int interval = RealSet.getIntervalDimension() - 1;
  unsigned int tract;
  count_t tCases, tCumCases=0, ** ppCases = RealSet.getCaseData().GetArray(), ** ppSimCases = SimSet.getCaseData().GetArray();
  measure_t tCumMeasure=0, ** ppMeasure = measure.GetArray();

  //create randomized data for each tract's last time interval
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

/** Creates randomized under the null hypothesis for Poisson model, non-parametric (location stratified), and assigning
    data to DataSet objects structures. Random number generator seed initialized based upon 'iSimulation' index. */
void PoissonSpatialStratifiedRandomizer::RandomizeData(const RealDataSet& RealSet, DataSet& SimSet, unsigned int iSimulation) {
  SetSeed(iSimulation, SimSet.getSetIndex());
  randomize(RealSet, RealSet.getMeasureData(), SimSet);
}

//******************************************************************************

/** constructor */
AlternateHypothesisRandomizer::AlternateHypothesisRandomizer(const CSaTScanData& DataHub, long lInitialSeed)
                              :PoissonRandomizer(DataHub.GetParameters(), lInitialSeed), gDataHub(DataHub) {

    // read the adjustments file
    SaTScanDataReader::RiskAdjustmentsContainer_t riskAdjustments;
    SaTScanDataReader reader(const_cast<CSaTScanData&>(gDataHub));
    if (!reader.ReadAdjustmentsByRelativeRisksFile(gParameters.getPowerEvaluationAltHypothesisFilename(), riskAdjustments, true))
        throw resolvable_error("There were problems reading the power evaluation adjustments file.", "AlternateHypothesisRandomizer()");
    if (!riskAdjustments.size())
        throw resolvable_error("Power evaluations can not be performed. No adjustments found in power evaluation file.", "AlternateHypothesisRandomizer()");
    _riskAdjustments = riskAdjustments.front();

    // define the underlying randomization object
    if (gParameters.GetTimeTrendAdjustmentType() == STRATIFIED_RANDOMIZATION)
        _randomizer.reset(new PoissonTimeStratifiedRandomizer(gParameters, lInitialSeed));
    else if (gParameters.GetSpatialAdjustmentType() == SPATIALLY_STRATIFIED_RANDOMIZATION)
        _randomizer.reset(new PoissonSpatialStratifiedRandomizer(gParameters, lInitialSeed));
    else if (gParameters.GetIsPurelyTemporalAnalysis())
        _randomizer.reset(new PoissonPurelyTemporalNullHypothesisRandomizer(gParameters, gParameters.GetRandomizationSeed()));
    else
        _randomizer.reset(new PoissonNullHypothesisRandomizer(gParameters, gParameters.GetRandomizationSeed()));
}

/** constructor */
AlternateHypothesisRandomizer::AlternateHypothesisRandomizer(const CSaTScanData& DataHub, 
                                                             boost::shared_ptr<RelativeRiskAdjustmentHandler> & riskAdjustments,
                                                             long lInitialSeed)
                              :PoissonRandomizer(DataHub.GetParameters(), lInitialSeed), gDataHub(DataHub), _riskAdjustments(riskAdjustments) {
    // define the underlying randomization object
    if (gParameters.GetTimeTrendAdjustmentType() == STRATIFIED_RANDOMIZATION)
        _randomizer.reset(new PoissonTimeStratifiedRandomizer(gParameters, lInitialSeed));
    else if (gParameters.GetSpatialAdjustmentType() == SPATIALLY_STRATIFIED_RANDOMIZATION)
        _randomizer.reset(new PoissonSpatialStratifiedRandomizer(gParameters, lInitialSeed));
    else if (gParameters.GetIsPurelyTemporalAnalysis())
        _randomizer.reset(new PoissonPurelyTemporalNullHypothesisRandomizer(gParameters, gParameters.GetRandomizationSeed()));
    else
        _randomizer.reset(new PoissonNullHypothesisRandomizer(gParameters, gParameters.GetRandomizationSeed()));
}

/** copy constructor */
AlternateHypothesisRandomizer::AlternateHypothesisRandomizer(const AlternateHypothesisRandomizer& rhs)
    :PoissonRandomizer(rhs), gDataHub(rhs.gDataHub), _riskAdjustments(rhs._riskAdjustments), _randomizer(dynamic_cast<PoissonRandomizer*>(rhs._randomizer->Clone())) {
}

void AlternateHypothesisRandomizer::randomize(const RealDataSet& RealSet, const TwoDimMeasureArray_t& measure, DataSet& SimSet) {
    _randomizer->randomize(RealSet, measure, SimSet);
}

/** Creates randomized under an alternative hypothesis for Poisson model and assigning data to DataSet
    objects structures. Random number generator seed initialized based upon 'iSimulation' index. */
void AlternateHypothesisRandomizer::RandomizeData(const RealDataSet& RealSet, DataSet& SimSet, unsigned int iSimulation) {
    _randomizer->SetSeed(iSimulation, SimSet.getSetIndex());
  // make a copy of real measure data for manipulation, getMeasureData_Aux() should be contain non-cummulative measure data with any initial adjustments
  TwoDimensionArrayHandler<measure_t> measure(RealSet.getMeasureData_Aux());
  // adjust the measure for known relative risks
  RealDataSet::PopulationDataPair_t populationPair = RealSet.getPopulationMeasureData();
  _riskAdjustments->apply(*populationPair.first, *populationPair.second, RealSet.getTotalMeasure(), measure);
  //now set the measure as cummulative
  if (measure.Get1stDimension() > 1) {
    measure_t ** ppMeasure = measure.GetArray();
    for (unsigned int t=0; t < measure.Get2ndDimension(); ++t) {
        for (unsigned int i=measure.Get1stDimension()-2; ; --i) {
            ppMeasure[i][t]= ppMeasure[i][t] + ppMeasure[i+1][t];
            if (i == 0) break;
        }
    }
  }
  randomize(RealSet, measure, SimSet);
}
