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

/** Prints the simulated data to a file. Format printed to file matches
    format expected for read as simulation data source. Truncates file
    when first opened for each analysis(i.e. first simulation).

    NOTE: The process of writing and reading simulation data to/from file
          is not well tested. It is known that it is not checking the validity
          of the files themselves or in relation to the running analysis.
          Also, not previsions have been made for this code to work for multiple
          data streams at this time.                                             */
void PoissonRandomizer::DumpDateToFile(DataStream & thisStream, int iSimulation) {
  std::ofstream         SimulationOutputFile;
  unsigned int          tract, interval;
  count_t            ** ppSimCases(thisStream.GetSimCaseArray());

  //open output file
  SimulationOutputFile.open(gParameters.GetSimulationDataOutputFilename().c_str(), (iSimulation == 1 ? ios::trunc : ios::ate));
  if (!SimulationOutputFile)
    SSGenerateException("Error: Could not open file simulation output file '%s'.\n", "PrintSimulationDateToFile()",
                        gParameters.GetSimulationDataOutputFilename().c_str());

  //print to file for time based analyses                      
  if (gParameters.GetAnalysisType() == PROSPECTIVESPACETIME || gParameters.GetAnalysisType() == SPACETIME ||
      gParameters.GetAnalysisType() == PURELYTEMPORAL || gParameters.GetAnalysisType() == PROSPECTIVEPURELYTEMPORAL) {
    for (tract=0; tract < thisStream.GetNumTracts(); tract++) {
       for (interval=0; interval < thisStream.GetNumTimeIntervals();interval++)
           SimulationOutputFile << ppSimCases[interval][tract] << " ";
       SimulationOutputFile << "\n";
    }
    SimulationOutputFile << "\n";
  }
  //print to file for spatial only analysis
  else if (gParameters.GetAnalysisType() == PURELYSPATIAL) {
    for (tract = 0; tract < thisStream.GetNumTracts(); tract++)
       SimulationOutputFile << ppSimCases[0][tract] << " ";
    SimulationOutputFile << "\n";
  }
  else
    SSGenerateException("Error: Printing simulation data to file not implemented for %s analysis.\n",
                        "ReadSimulationDataFromFile()", gParameters.GetAnalysisTypeAsString());
}


/** constructor */
PoissonNullHypothesisRandomizer::PoissonNullHypothesisRandomizer(const CParameters & Parameters) : PoissonRandomizer(Parameters) {}

/** destructor */
PoissonNullHypothesisRandomizer::~PoissonNullHypothesisRandomizer() {}

/** returns newly cloned PoissonNullHypothesisRandomizer */
PoissonNullHypothesisRandomizer * PoissonNullHypothesisRandomizer::Clone() const {
   return new PoissonNullHypothesisRandomizer(*this);
}

/** Generate case counts under the null hypothesis (standard) */
void PoissonNullHypothesisRandomizer::RandomizeData(DataStream & thisStream, unsigned int iSimulation) {
  unsigned int          t, tNumTracts = thisStream.GetNumTracts(),
                        i, tNumTimeIntervals = thisStream.GetNumTimeIntervals();
  count_t               c, d, cumcases=0, tTotalCases = thisStream.GetTotalCases(),
                     ** ppSimCases = thisStream.GetSimCaseArray();
  measure_t             cummeasure=0, tTotalMeasure = thisStream.GetTotalMeasure(),
                     ** ppMeasure(thisStream.GetMeasureArray());

  gRandomNumberGenerator.SetSeed(iSimulation + gRandomNumberGenerator.GetDefaultSeed());
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
void PoissonTimeStratifiedRandomizer::RandomizeData(DataStream & thisStream, unsigned int iSimulation) {
  unsigned int          tract, tNumTracts = thisStream.GetNumTracts();
  count_t               c, cumcases=0, * pPTCases = thisStream.GetPTCasesArray(),
                     ** ppSimCases = thisStream.GetSimCaseArray();
  measure_t             cummeasure=0, * pPTMeasure = thisStream.GetPTMeasureArray(),
                     ** ppMeasure = thisStream.GetMeasureArray();
  int                   interval;

  gRandomNumberGenerator.SetSeed(iSimulation + gRandomNumberGenerator.GetDefaultSeed());
  interval = thisStream.GetNumTimeIntervals() - 1;
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
void AlternateHypothesisRandomizer::RandomizeData(DataStream & thisStream, unsigned int iSimulation) {
  unsigned int          j, t, i, tNumTracts = thisStream.GetNumTracts(),
                        tNumTimeIntervals = thisStream.GetNumTimeIntervals();
  int                   iInterval;
  std::ifstream         RelativeRiskFile;
  std::string           sTractId;
  tract_t               tractIndex;
  measure_t             cummeasure=0, TotalMeasure = thisStream.GetTotalMeasure(),
                     ** ppMeasure = thisStream.GetMeasureArray();
  count_t               c, d, cumcases=0, ** ppSimCases = thisStream.GetSimCaseArray();

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
         SSGenerateException("Unknown location identifier '%s', in power estimation file.",
                             "MakeData_AlternateHypothesis()", sTractId.c_str());
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
        c = gBinomialGenerator.GetBinomialDistributedVariable(thisStream.GetTotalCases() - cumcases,
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

/** constructor */
FileSourceRandomizer::FileSourceRandomizer(CSaTScanData & Data)
                     :PoissonRandomizer(Data.GetParameters()), gData(Data) {}

/** copy constructor */
FileSourceRandomizer::FileSourceRandomizer(const FileSourceRandomizer & rhs)
                     :PoissonRandomizer(rhs), gData(rhs.gData) {}

/** destructor */
FileSourceRandomizer::~FileSourceRandomizer() {}

/** returns pointer to newly cloned FileSourceRandomizer */
FileSourceRandomizer * FileSourceRandomizer::Clone() const {
  return new FileSourceRandomizer(*this);
}

/** Reads number of simulated cases from a text file rather than generating them randomly.
    NOTE: Data read from the file is not validated. This means that there is potential
          for the program to behave badly if:
          1) the data read from file does not match dimensions of ppSimCases
          2) the case counts read from file is inappropriate given real data -- probably access violations
          3) file does not actually contains numerical data
          Use of this feature should be discouraged except from someone who has
          detailed knowledge of how code works.                                                           */
void FileSourceRandomizer::RandomizeData(DataStream & thisStream, unsigned int iSimulation) {
  unsigned int          i, t, tNumTracts = thisStream.GetNumTracts(),
                        tNumTimeIntervals = thisStream.GetNumTimeIntervals();
  count_t               c;
  count_t            ** ppSimCases = thisStream.GetSimCaseArray();

  if (!gSimulationDataInputFile.is_open())
    gSimulationDataInputFile.open(gData.GetParameters().GetSimulationDataSourceFilename().c_str());
  if (!gSimulationDataInputFile)
    SSGenerateException("Error: Could not open file '%s' to read simulated data.\n",
                        "ReadSimulationDataFromFile()", gData.GetParameters().GetSimulationDataSourceFilename().c_str());

  if (gData.GetParameters().GetAnalysisType() == PROSPECTIVESPACETIME || gData.GetParameters().GetAnalysisType() == SPACETIME ||
      gData.GetParameters().GetAnalysisType() == PURELYTEMPORAL || gData.GetParameters().GetAnalysisType() == PROSPECTIVEPURELYTEMPORAL) {
     for (t=0; t < tNumTracts; ++t) {
        for (i=0; i < tNumTimeIntervals; ++i)
           gSimulationDataInputFile >> ppSimCases[i][t];
     }
  }
  else if (gData.GetParameters().GetAnalysisType() == PURELYSPATIAL) {
     for (t=0; t < tNumTracts; ++t)
        gSimulationDataInputFile >> ppSimCases[0][t];
  }
  else
    SSGenerateException("Error: Reading simulation data from file not implemented for %s analysis.\n",
                        "RandomizeData()", gData.GetParameters().GetAnalysisTypeAsString());
}
