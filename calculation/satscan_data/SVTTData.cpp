//*****************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//*****************************************************************************
#include "SVTTData.h"
#include "PoissonModel.h"
#include "BernoulliModel.h"
#include "SpaceTimePermutationModel.h"
#include "LocationRiskEstimateWriter.h"

/** class constructor */
CSVTTData::CSVTTData(const CParameters& Parameters, BasePrint& PrintDirection)
          :CSaTScanData(Parameters, PrintDirection) {
  try {
    SetProbabilityModel();
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor()","CSVTTData");
    throw;
  }
}

/** class destructor */
CSVTTData::~CSVTTData() {}

void CSVTTData::CalculateMeasure(RealDataSet& DataSet) {
  CSaTScanData::CalculateMeasure(DataSet);
  //calculate time trend for dataset data set
  //TODO: The status of the time trend needs to be checked after CalculateAndSet() returns.
  //      The correct behavior for anything other than CTimeTrend::TREND_CONVERGED
  //      has not been decided yet.
  DataSet.GetTimeTrend().CalculateAndSet(DataSet.GetCasesPerTimeIntervalArray(), DataSet.GetMeasurePerTimeIntervalArray(),
                                            m_nTimeIntervals, gParameters.GetTimeTrendConvergence());
}

/** Debug utility function - prints case counts for all datasets. Caller is
    responsible for ensuring that passed file pointer points to valid, open file
    handle. */
void CSVTTData::DisplayCases(FILE* pFile) {
  unsigned int i;

  for (i=0; i < gpDataSets->GetNumDataSets(); ++i) {
     fprintf(pFile, "Data Set %u:\n", i);
     DisplayCounts(pFile, gpDataSets->GetDataSet(i).GetCaseArray(), "Cases Array",
                   gpDataSets->GetDataSet(i).GetNCCaseArray(), "Cases Non-Cumulative Array",
                   gpDataSets->GetDataSet(i).GetPTCasesArray(), "Cases_TotalByTimeInt");
  }                 
}

/** Debug utility function - prints counts for passed arrays. Caller is
    responsible for ensuring that passed file pointer points to valid, open file
    handle and array pointers are valid. Probably should call
    DisplayCases(FILE* pFile) instead of this function directly. */
void CSVTTData::DisplayCounts(FILE* pFile,
                              count_t** pCounts,   char* szVarName,
                              count_t** pCountsNC, char* szVarNameNC,
                              count_t*  pCountsTI, char* szVarNameTI,
                              char* szTitle) {
  if (szTitle != NULL)
    fprintf(pFile, "%s", szTitle);

  fprintf(pFile, "Counts                         Counts - Not Accumulated\n\n");

  for (int i = 0; i < m_nTimeIntervals; i++)
    for (int j = 0; j < m_nTracts; j++)
    {
      fprintf(pFile, "%s [%i][%i] = %6i     ", szVarName, i,j,pCounts[i][j]);
      fprintf(pFile, "%s [%i][%i] = %6i\n", szVarNameNC, i,j,pCountsNC[i][j]);
    }

  fprintf(pFile, "\nCounts Accumulated by Time Interval\n\n");
  for (int i=0; i<m_nTimeIntervals; i++)
    fprintf(pFile, "%s [%i] = %6i\n", szVarNameTI, i, pCountsTI[i]);

  fprintf(pFile, "\n");
}

/** Debug utility function - prints expected counts for for all datasets.
    Caller is responsible for ensuring that passed file pointer points to valid,
    open file handle. */
void CSVTTData::DisplayMeasures(FILE* pFile) {
  unsigned int           i, j, k;
  measure_t           ** ppMeasure, ** ppMeasureNC;

  fprintf(pFile, "Measures                        Measures - Not Accumulated\n\n");

  for (k=0; k < gpDataSets->GetNumDataSets(); ++k) {
     fprintf(pFile, "Data Set %u:\n", k);
     ppMeasure = gpDataSets->GetDataSet(k).GetMeasureArray();
     ppMeasureNC = gpDataSets->GetDataSet(k).GetNCMeasureArray();
     for (i=0; i < (unsigned int)m_nTimeIntervals; ++i)
        for (j=0; j < (unsigned int)m_nTracts; ++j) {
           fprintf(pFile, "ppMeasure [%i][%i] = %12.5f     ", i, j, ppMeasure[i][j]);
           fprintf(pFile, "ppMeasure_NC [%i][%i] = %12.5f\n", i, j, ppMeasureNC[i][j]);
        }
  }

  fprintf(pFile, "\nMeasures Accumulated by Time Interval\n\n");
  for (k=0; k < gpDataSets->GetNumDataSets(); ++k) {
     fprintf(pFile, "Data Set %u:\n", k);
     for (i=0; i < (unsigned int)m_nTimeIntervals; ++i)
       fprintf(pFile, "Measure_TotalByTimeInt [%i] = %12.5f\n", i, gpDataSets->GetDataSet(k).GetPTMeasureArray()[i]);
     fprintf(pFile, "\n");
  }
}

// formats the information necessary in the relative risk output file and prints to the specified format
void CSVTTData::DisplayRelativeRisksForEachTract() const {
  try {
    LocationRiskEstimateWriter(gParameters).Write(*this);
  }
  catch (ZdException &x) {
    x.AddCallpath("DisplayRelativeRisksForEachTract()","CSVTTData");
    throw;
  }
}

/** Not implemented - needs to be updated */
void CSVTTData::DisplaySimCases(FILE* pFile) {
//  unsigned int i;                                             
//
//  for (i=0; i < gpDataSets->GetNumDataSets(); ++i) {
//     fprintf(pFile, "Data Set %u:\n", i);
//     DisplayCounts(pFile, gpDataSets->GetDataSet(i).GetSimCaseArray(), "Simulated Cases Array",
//                   gpDataSets->GetDataSet(i).GetNCSimCaseArray(), "Simulated Non-Cumulative Cases Array",
//                   gpDataSets->GetDataSet(i).GetPTSimCasesArray(), "SimCases_TotalByTimeInt");
//  }
}

/** Randomizes collection of simulation data in concert with passed collection of randomizers. */
void CSVTTData::RandomizeData(RandomizerContainer_t& RandomizerContainer,
                              SimulationDataContainer_t& SimDataContainer,
                              unsigned int iSimulationNumber) const {
  try {
    CSaTScanData::RandomizeData(RandomizerContainer, SimDataContainer, iSimulationNumber);
    for (size_t t=0; t < SimDataContainer.size(); ++t) {
       SimDataContainer[t]->SetNonCumulativeCaseArrays();
       //calculate time trend for entire randomized data set
       //TODO: The status of the time trend needs to be checked after CalculateAndSet() returns.
       //      The correct behavior for anything other than CTimeTrend::TREND_CONVERGED
       //      has not been decided yet.
       SimDataContainer[t]->GetTimeTrend().CalculateAndSet(gpDataSets->GetDataSet(t).GetCasesPerTimeIntervalArray(),
                                                           gpDataSets->GetDataSet(t).GetMeasurePerTimeIntervalArray(),
                                                           m_nTimeIntervals,
                                                           gParameters.GetTimeTrendConvergence());
       //QUESTION: Should the purely temporal case array passed to CalculateAndSet() be from
       //          the simulated dataset? It doesn't seem to make sense otherwise.
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("RandomizeData()","CSVTTData");
    throw;
  }
}

/** Redefines base class method to call dataset method
    DataSet::SetNonCumulativeCaseArrays() which allocates and sets cases
    per time interval array and non cumulative case array. */
void CSVTTData::SetAdditionalCaseArrays(RealDataSet& DataSet) {
  try {
    DataSet.SetNonCumulativeCaseArrays();
  }
  catch (ZdException &x) {
    x.AddCallpath("SetAdditionalCaseArrays()","CSVTTData");
    throw;
  }
}

/** Allocates probability model obect. Throws ZdException for all probability
    model type except Poisson. */
void CSVTTData::SetProbabilityModel() {
  try {
    switch (gParameters.GetProbabilityModelType()) {
       case POISSON              : m_pModel = new CPoissonModel(gParameters, *this, gPrint);   break;
       case BERNOULLI            : ZdException::Generate("Spatial Variation of Temporal Trends not implemented for Bernoulli model.\n",
                                                         "SetProbabilityModel()");
       case SPACETIMEPERMUTATION : ZdException::Generate("Spatial Variation of Temporal Trends not implemented for Space-Time Permutation model.\n",
                                                         "SetProbabilityModel()");
       case ORDINAL              : ZdException::Generate("Spatial Variation of Temporal Trends not implemented for Ordinal model.\n",
                                                         "SetProbabilityModel()");
       case EXPONENTIAL          : ZdException::Generate("Spatial Variation of Temporal Trends not implemented for Exponential model.\n",
                                                         "SetProbabilityModel()");
       case NORMAL               : ZdException::Generate("Spatial Variation of Temporal Trends not implemented for Normal model.\n",
                                                         "SetProbabilityModel()");
       case RANK                 : ZdException::Generate("Spatial Variation of Temporal Trends not implemented for Rank model.\n",
                                                         "SetProbabilityModel()");
       default : ZdException::Generate("Unknown probability model type: '%d'.\n",
                                       "SetProbabilityModel()", gParameters.GetProbabilityModelType());
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("SetProbabilityModel()","CSVTTData");
    throw;
  }
}

