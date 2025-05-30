//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "PoissonRandomizer.h"
#include "SaTScanData.h"
#include "SSException.h" 
#include "AdjustmentHandler.h"
#include "SaTScanDataRead.h"

//********** PoissonNullHypothesisRandomizer ***********

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

//********** AbstractPoissonPurelyTemporalNullHypothesisRandomizer **********

AbstractPoissonPurelyTemporalNullHypothesisRandomizer::AbstractPoissonPurelyTemporalNullHypothesisRandomizer(const CParameters & Parameters, long lInitialSeed)
    : PoissonRandomizer(Parameters, lInitialSeed) {}

void AbstractPoissonPurelyTemporalNullHypothesisRandomizer::randomize_data(count_t * pSimCases, const measure_t * pMeasure, unsigned int num_intervals, count_t totalcases, measure_t totalmeasure) {
    count_t d;
    pSimCases[0] = gBinomialGenerator.GetBinomialDistributedVariable(totalcases, pMeasure[0]/totalmeasure, gRandomNumberGenerator);
    for (int i=0; i < num_intervals - 1; ++i) {
        if (pMeasure[i] > 0)
            pSimCases[i+1] = pSimCases[i] - gBinomialGenerator.GetBinomialDistributedVariable(pSimCases[i], 1 - pMeasure[i+1]/pMeasure[i], gRandomNumberGenerator);
        else
            pSimCases[i+1] = pSimCases[i];
    }
}

/** Creates randomized under the null hypothesis for Poisson model, assigning data to DataSet objects structures.
    Random number generator seed initialized based upon 'iSimulation' index. */
void AbstractPoissonPurelyTemporalNullHypothesisRandomizer::RandomizeData(const RealDataSet& RealSet, DataSet& SimSet, unsigned int iSimulation) {
    SetSeed(iSimulation, SimSet.getSetIndex());
    SimSet.allocateCaseData_PT();
    randomize_data(SimSet.getCaseData_PT(), RealSet.getMeasureData_PT(), RealSet.getIntervalDimension(), RealSet.getTotalCases(), RealSet.getTotalMeasure());
}

//********** PoissonPurelyTemporalNullHypothesisRandomizer ***********

PoissonPurelyTemporalNullHypothesisRandomizer::PoissonPurelyTemporalNullHypothesisRandomizer(const CParameters & Parameters, long lInitialSeed)
    : AbstractPoissonPurelyTemporalNullHypothesisRandomizer(Parameters, lInitialSeed) {}

void PoissonPurelyTemporalNullHypothesisRandomizer::randomize(const RealDataSet& RealSet, const TwoDimMeasureArray_t& measure, DataSet& SimSet) {
    std::vector<measure_t> vMPT(measure.Get1stDimension(), 0);
    measure_t ** ppMeasure = measure.GetArray();
    for (unsigned int i=0; i < measure.Get1stDimension(); ++i)
        for (unsigned int t=0; t < measure.Get2ndDimension(); ++t)
            vMPT[i] += ppMeasure[i][t];
    SimSet.allocateCaseData_PT();
    randomize_data(SimSet.getCaseData_PT(), &vMPT[0], measure.Get1stDimension(), RealSet.getTotalCases(), RealSet.getTotalMeasure());
}

//********** ClosedLoopPoissonPurelyTemporalNullHypothesisRandomizer ***********

ClosedLoopPoissonPurelyTemporalNullHypothesisRandomizer::ClosedLoopPoissonPurelyTemporalNullHypothesisRandomizer(const CSaTScanData& datahub, long lInitialSeed)
    : AbstractPoissonPurelyTemporalNullHypothesisRandomizer(datahub.GetParameters(), lInitialSeed), _data_hub(datahub) {}

void ClosedLoopPoissonPurelyTemporalNullHypothesisRandomizer::randomize(const RealDataSet& RealSet, const TwoDimMeasureArray_t& measure, DataSet& SimSet) {
    std::vector<measure_t> vMPT(measure.Get1stDimension(), 0);
    measure_t ** ppMeasure = measure.GetArray();
    for (unsigned int i=0; i < measure.Get1stDimension(); ++i)
        for (unsigned int t=0; t < measure.Get2ndDimension(); ++t)
            vMPT[i] += ppMeasure[i][t];
    SimSet.allocateCaseData_PT();
    std::vector<count_t> non_seasonal_cases(measure.Get1stDimension(), 0);
    randomize_data(&non_seasonal_cases[0], &vMPT[0], measure.Get1stDimension(), RealSet.getTotalCases(), RealSet.getTotalMeasure());
    // now we need to compress case data into seasonal time intervals
    count_t * pSimCases = SimSet.getCaseData_PT();
    for (unsigned int i=0; i < measure.Get1stDimension() - 1; ++i) {
        count_t ncases = non_seasonal_cases[i] - non_seasonal_cases[i + 1];
        if (ncases) {
            pSimCases[_data_hub.GetTimeIntervalOfDate(_data_hub.convertToSeasonalDate(_data_hub.CSaTScanData::GetTimeIntervalStartTimes()[i]))] += ncases;
        }
    }
    // now set data as cumulative
    for (unsigned int i=RealSet.getIntervalDimension() - 2; ; --i) {
        pSimCases[i]= pSimCases[i] + pSimCases[i+1];
        if (i == 0) break;
    }
}

//********** PoissonTimeStratifiedRandomizer ***********

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
  for (unsigned int t=0; t < RealSet.getLocationDimension(); ++t) {
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

//********** PoissonSpatialStratifiedRandomizer ***********

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

//********** AlternateHypothesisRandomizer **********

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
    if (gParameters.GetTimeTrendAdjustmentType() == TEMPORAL_STRATIFIED_RANDOMIZATION)
        _randomizer.reset(new PoissonTimeStratifiedRandomizer(gParameters, lInitialSeed));
    else if (gParameters.GetSpatialAdjustmentType() == SPATIAL_STRATIFIED_RANDOMIZATION)
        _randomizer.reset(new PoissonSpatialStratifiedRandomizer(gParameters, lInitialSeed));
    else if (gParameters.GetAnalysisType() == SEASONALTEMPORAL)
        _randomizer.reset(new ClosedLoopPoissonPurelyTemporalNullHypothesisRandomizer(DataHub, gParameters.GetRandomizationSeed()));
    else if (gParameters.GetIsPurelyTemporalAnalysis())
        _randomizer.reset(new PoissonPurelyTemporalNullHypothesisRandomizer(gParameters, gParameters.GetRandomizationSeed()));
    else _randomizer.reset(new PoissonNullHypothesisRandomizer(gParameters, gParameters.GetRandomizationSeed()));
}

/** constructor */
AlternateHypothesisRandomizer::AlternateHypothesisRandomizer(const CSaTScanData& DataHub, 
                                                             boost::shared_ptr<RelativeRiskAdjustmentHandler> & riskAdjustments,
                                                             long lInitialSeed)
                              :PoissonRandomizer(DataHub.GetParameters(), lInitialSeed), gDataHub(DataHub), _riskAdjustments(riskAdjustments) {
    // define the underlying randomization object
    if (gParameters.GetTimeTrendAdjustmentType() == TEMPORAL_STRATIFIED_RANDOMIZATION)
        _randomizer.reset(new PoissonTimeStratifiedRandomizer(gParameters, lInitialSeed));
    else if (gParameters.GetSpatialAdjustmentType() == SPATIAL_STRATIFIED_RANDOMIZATION)
        _randomizer.reset(new PoissonSpatialStratifiedRandomizer(gParameters, lInitialSeed));
    else if (gParameters.GetAnalysisType() == SEASONALTEMPORAL)
        _randomizer.reset(new ClosedLoopPoissonPurelyTemporalNullHypothesisRandomizer(DataHub, gParameters.GetRandomizationSeed()));
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
