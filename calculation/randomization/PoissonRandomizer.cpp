//---------------------------------------------------------------------------
#include "SaTScan.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#include "PoissonRandomizer.h"
#include "SaTScanData.h"

/** constructor */
PoissonRandomizer::PoissonRandomizer(const CParameters & Parameters)
                  :AbstractDenominatorDataRandomizer(), gParameters(Parameters) {}

/** destructor */
PoissonRandomizer::~PoissonRandomizer() {}

/** constructor */
PoissonNullHypothesisRandomizer::PoissonNullHypothesisRandomizer(const CParameters & Parameters) : PoissonRandomizer(Parameters) {}

/** destructor */
PoissonNullHypothesisRandomizer::~PoissonNullHypothesisRandomizer() {}

/** returns newly cloned PoissonNullHypothesisRandomizer */
PoissonNullHypothesisRandomizer * PoissonNullHypothesisRandomizer::Clone() const {
   return new PoissonNullHypothesisRandomizer(*this);
}

/** Generate case counts under the null hypothesis (standard) */
void PoissonNullHypothesisRandomizer::RandomizeData(const RealDataStream& thisRealStream,
                                                    SimulationDataStream& thisSimulationStream,
                                                    unsigned int iSimulation) {
  unsigned int          t, tNumTracts = thisRealStream.GetNumTracts(),
                        i, tNumTimeIntervals = thisRealStream.GetNumTimeIntervals();
  count_t               c, d, cumcases=0, tTotalCases = thisRealStream.GetTotalCases(),
                     ** ppSimCases = thisSimulationStream.GetCaseArray();
  measure_t             cummeasure=0, tTotalMeasure = thisRealStream.GetTotalMeasure(),
                     ** ppMeasure(thisRealStream.GetMeasureArray());
                     
  SetSeed(iSimulation, thisSimulationStream.GetStreamIndex());
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


/** constructor */
PoissonTimeStratifiedRandomizer::PoissonTimeStratifiedRandomizer(const CParameters & Parameters)
                                :PoissonRandomizer(Parameters) {}

/** destructor */
PoissonTimeStratifiedRandomizer::~PoissonTimeStratifiedRandomizer() {}

/** returns newly cloned PoissonTimeStratifiedRandomizer */
PoissonTimeStratifiedRandomizer * PoissonTimeStratifiedRandomizer::Clone() const {
  return new PoissonTimeStratifiedRandomizer(*this);
}

/** Randomizes data of data stream, stratified by time */
void PoissonTimeStratifiedRandomizer::RandomizeData(const RealDataStream& thisRealStream,
                                                    SimulationDataStream& thisSimulationStream,
                                                    unsigned int iSimulation) {
  unsigned int          tract, tNumTracts = thisRealStream.GetNumTracts();
  count_t               c, cumcases=0, * pPTCases = thisRealStream.GetPTCasesArray(),
                     ** ppSimCases = thisSimulationStream.GetCaseArray();
  measure_t             cummeasure=0, * pPTMeasure = thisRealStream.GetPTMeasureArray(),
                     ** ppMeasure = thisRealStream.GetMeasureArray();
  int                   interval;

  SetSeed(iSimulation, thisSimulationStream.GetStreamIndex());
  interval = thisRealStream.GetNumTimeIntervals() - 1;
  for (tract=0; tract < tNumTracts; ++tract) {
     if (pPTCases[interval] - cumcases > 0)
       c = gBinomialGenerator.GetBinomialDistributedVariable(pPTCases[interval] - cumcases,
                                                             ppMeasure[interval][tract]/(pPTMeasure[interval] - cummeasure),
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
       if (pPTCases[interval] - cumcases > 0)
          c = gBinomialGenerator.GetBinomialDistributedVariable(pPTCases[interval] - cumcases,
                      (ppMeasure[interval][tract] - ppMeasure[interval + 1][tract])/(pPTMeasure[interval] - cummeasure),
                      gRandomNumberGenerator);
        else
          c = 0;
        cumcases += c;
        cummeasure += (ppMeasure[interval][tract] - ppMeasure[interval + 1][tract]);
        ppSimCases[interval][tract] = c + ppSimCases[interval + 1][tract];
     }
 }
}

/** constructor */
PoissonSpatialStratifiedRandomizer::PoissonSpatialStratifiedRandomizer(const CParameters & Parameters)
                                :PoissonRandomizer(Parameters) {}

/** destructor */
PoissonSpatialStratifiedRandomizer::~PoissonSpatialStratifiedRandomizer() {}

/** returns newly cloned PoissonTimeStratifiedRandomizer */
PoissonSpatialStratifiedRandomizer * PoissonSpatialStratifiedRandomizer::Clone() const {
  return new PoissonSpatialStratifiedRandomizer(*this);
}

/** Randomizes data of data stream, stratified by space */
void PoissonSpatialStratifiedRandomizer::RandomizeData(const RealDataStream& thisRealStream,
                                                       SimulationDataStream& thisSimulationStream,
                                                       unsigned int iSimulation) {

  int                   interval;
  unsigned int          tract;
  count_t               tCases, tCumCases=0,
                     ** ppCases = thisRealStream.GetCaseArray(),
                     ** ppSimCases = thisSimulationStream.GetCaseArray();
  measure_t             tCumMeasure=0, ** ppMeasure = thisRealStream.GetMeasureArray();

  //reset seed for simulation number
  SetSeed(iSimulation, thisSimulationStream.GetStreamIndex());
  //create randomized data for each tract's last time interval
  interval = thisRealStream.GetNumTimeIntervals() - 1;
  for (tract=0; tract < thisRealStream.GetNumTracts(); ++tract) {
     if (ppCases[0][tract])
       ppSimCases[interval][tract] = gBinomialGenerator.GetBinomialDistributedVariable(ppCases[0][tract],
                                           ppMeasure[interval][tract]/(ppMeasure[0][tract]), gRandomNumberGenerator);
     else
       ppSimCases[interval][tract] = 0;
  }
  //create randomized data for each tract's remaining time intervals
  for (tract=0; tract < thisRealStream.GetNumTracts(); ++tract) {
     tCumCases = 0;
     tCumMeasure = 0;
     interval = thisRealStream.GetNumTimeIntervals() - 2;
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

/** constructor */
AlternateHypothesisRandomizer::AlternateHypothesisRandomizer(CSaTScanData & Data)
                              :PoissonRandomizer(Data.GetParameters()), gData(Data) {
  try {
    Init();
    Setup();
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor()","AlternateHypothesisRandomizer");
    throw;
  }
}

/** copy constructor */
AlternateHypothesisRandomizer::AlternateHypothesisRandomizer(const AlternateHypothesisRandomizer & rhs)
                              :PoissonRandomizer(rhs), gData(rhs.gData) {
  try {
    Init();
    gvRelativeRisks = rhs.gvRelativeRisks;
    gvMeasure = rhs.gvMeasure;
    gpAlternativeMeasure = new TwoDimensionArrayHandler<measure_t>(gData.GetNumTimeIntervals(), gData.GetNumTracts(), 0);
    (*gpAlternativeMeasure) = (*rhs.gpAlternativeMeasure);
  }
  catch (ZdException &x) {
    delete gpAlternativeMeasure;
    x.AddCallpath("copy constructor()","AlternateHypothesisRandomizer");
    throw;
  }
}

/** desctructor */
AlternateHypothesisRandomizer::~AlternateHypothesisRandomizer() {
  try {
    delete gpAlternativeMeasure;
  }
  catch (...){}
}

/** returns pointer to newly cloned AlternateHypothesisRandomizer */
AlternateHypothesisRandomizer * AlternateHypothesisRandomizer::Clone() const {
  return new AlternateHypothesisRandomizer(*this);
}

/** Randomizes data of data stream under alternate hypothesis.
    NOTE: This procedure is in an experiential stage. */
void AlternateHypothesisRandomizer::RandomizeData(const RealDataStream& thisRealStream,
                                                  SimulationDataStream& thisSimulationStream,
                                                  unsigned int iSimulation) {
  unsigned int          j, t, i, tNumTracts = thisRealStream.GetNumTracts(),
                        tNumTimeIntervals = thisRealStream.GetNumTimeIntervals();
  int                   iInterval;
  std::ifstream         RelativeRiskFile;
  std::string           sTractId;
  tract_t               tractIndex;
  measure_t             cummeasure=0, TotalMeasure = thisRealStream.GetTotalMeasure(),
                     ** ppMeasure = thisRealStream.GetMeasureArray();
  count_t               c, d, cumcases=0, ** ppSimCases = thisSimulationStream.GetCaseArray();

  //duplicate the  ppMeasure[][] into gpAlternativeMeasure[][], gpAlternativeMeasure[][] will be changed depending upon
  //the gvRelativeRisks[], and ppMeasure[][] remains the same as the expected measure
  for (t=0; t < tNumTracts; ++t)
     for (i=0; i < tNumTimeIntervals; ++i)
        gpAlternativeMeasure->GetArray()[i][t] = ppMeasure[i][t];

  //initialize the gvRelativeRisks[] to be 1.0
  for (t=0; t < tNumTracts; ++t)
     gvRelativeRisks[t] = 1.0;

  //read in the RR's for those tracts with higher risks
  RelativeRiskFile.open(gData.GetParameters().GetAdjustmentsByRelativeRisksFilename().c_str());
  while (!RelativeRiskFile.eof()) {
       RelativeRiskFile >> sTractId;
       if ((tractIndex = gData.GetTInfo()->tiGetTractIndex(sTractId.c_str())) == -1)
         GenerateResolvableException("The Location ID '%s', in power estimation file, is not specified the coordinates file.",
                                     "RandomizeData()", sTractId.c_str());
        RelativeRiskFile >> gvRelativeRisks[tractIndex];
  }
  RelativeRiskFile.close();

  //modify the measures
 for (t=0; t < tNumTracts; ++t) {
     gvMeasure[t] = ppMeasure[0][t];
     for (i=tNumTimeIntervals; i >= 30/* ??? */ ; i--) {
        if (i == tNumTimeIntervals-1) {//if the last interval, the cummulative measure is the measure itself
          gvMeasure[t] = gvMeasure[t] + ppMeasure[i][t] * (gvRelativeRisks[t]-1);
          TotalMeasure = TotalMeasure + ppMeasure[i][t] * (gvRelativeRisks[t]-1);
          for (j=0; j <= i; ++j)
             gpAlternativeMeasure->GetArray()[j][t] += ppMeasure[i][t] * (gvRelativeRisks[t]-1);
        }
        else {
          //if not the last interval, the measure belongs to the interval  is the difference between
          //the measure of this interval and the measure for next interval, measure[] and TotalMeasure
          //should change accordingly.
          gvMeasure[t] = gvMeasure[t] + (ppMeasure[i][t] - ppMeasure[i+1][t]) * (gvRelativeRisks[t]-1);
          TotalMeasure = TotalMeasure + (ppMeasure[i][t] - ppMeasure[i+1][t]) * (gvRelativeRisks[t]-1);
          for (j=0; j <= i; ++j)
             gpAlternativeMeasure->GetArray()[j][t] += (ppMeasure[i][t] - ppMeasure[i+1][t]) * (gvRelativeRisks[t]-1);
        }
     }
  }

   //start alternative simulations
  for (t=0; t < tNumTracts; ++t) {
    if (TotalMeasure-cummeasure > 0)
        c = gBinomialGenerator.GetBinomialDistributedVariable(thisRealStream.GetTotalCases() - cumcases,
                                                              gvMeasure[t] / (TotalMeasure-cummeasure),
                                                              gRandomNumberGenerator);
    else
      c = 0;
    ppSimCases[0][t] = c;
    cumcases += c;
    cummeasure += gvMeasure[t];

   for (i=0; i < tNumTimeIntervals-1; ++i) {
       if (gpAlternativeMeasure->GetArray()[i][t] > 0)
        d = gBinomialGenerator.GetBinomialDistributedVariable(ppSimCases[i][t],
              1 - gpAlternativeMeasure->GetArray()[i+1][t] / gpAlternativeMeasure->GetArray()[i][t], gRandomNumberGenerator);
      else
        d = 0;

      ppSimCases[i+1][t] = ppSimCases[i][t] - d;
    }
  }
}

/** Internal setup function */
void AlternateHypothesisRandomizer::Setup() {
  try {
    gvRelativeRisks.resize(gData.GetNumTracts(), 0);
    gvMeasure.resize(gData.GetNumTracts(), 0);
    gpAlternativeMeasure = new TwoDimensionArrayHandler<measure_t>(gData.GetNumTimeIntervals(), gData.GetNumTracts(), 0);
  }
  catch (ZdException &x) {
    x.AddCallpath("Setup()","AlternateHypothesisRandomizer");
    throw;
  }
}


