//*****************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//*****************************************************************************
#include "SVTTData.h"
#include "PoissonModel.h"
#include "BernoulliModel.h"
#include "SpaceTimePermutationModel.h"
#include "stsRelativeRisk.h"
#include "stsASCIIFileWriter.h"
#include "stsDBaseFileWriter.h"

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

void CSVTTData::CalculateMeasure(RealDataStream & thisStream) {
  CSaTScanData::CalculateMeasure(thisStream);
  //calculate time trend for stream data set
  //TODO: The status of the time trend needs to be checked after CalculateAndSet() returns.
  //      The correct behavior for anything other than CTimeTrend::TREND_CONVERGED
  //      has not been decided yet.
  thisStream.GetTimeTrend().CalculateAndSet(thisStream.GetPTCasesArray(), thisStream.GetPTMeasureArray(),
                                            m_nTimeIntervals, gParameters.GetTimeTrendConvergence());
}

/** Debug utility function - prints case counts for all data streams. Caller is
    responsible for ensuring that passed file pointer points to valid, open file
    handle. */
void CSVTTData::DisplayCases(FILE* pFile) {
  unsigned int i;

  for (i=0; i < gpDataStreams->GetNumStreams(); ++i) {
     fprintf(pFile, "Data Stream %u:\n", i);
     DisplayCounts(pFile, gpDataStreams->GetStream(i).GetCaseArray(), "Cases Array",
                   gpDataStreams->GetStream(i).GetNCCaseArray(), "Cases Non-Cumulative Array",
                   gpDataStreams->GetStream(i).GetPTCasesArray(), "Cases_TotalByTimeInt");
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

/** Debug utility function - prints expected counts for for all data streams.
    Caller is responsible for ensuring that passed file pointer points to valid,
    open file handle. */
void CSVTTData::DisplayMeasures(FILE* pFile) {
  unsigned int           i, j, k;
  measure_t           ** ppMeasure, ** ppMeasureNC;

  fprintf(pFile, "Measures                        Measures - Not Accumulated\n\n");

  for (k=0; k < gpDataStreams->GetNumStreams(); ++k) {
     fprintf(pFile, "Data Stream %u:\n", k);
     ppMeasure = gpDataStreams->GetStream(k).GetMeasureArray();
     ppMeasureNC = gpDataStreams->GetStream(k).GetNCMeasureArray();
     for (i=0; i < (unsigned int)m_nTimeIntervals; ++i)
        for (j=0; j < (unsigned int)m_nTracts; ++j) {
           fprintf(pFile, "ppMeasure [%i][%i] = %12.5f     ", i, j, ppMeasure[i][j]);
           fprintf(pFile, "ppMeasure_NC [%i][%i] = %12.5f\n", i, j, ppMeasureNC[i][j]);
        }
  }

  fprintf(pFile, "\nMeasures Accumulated by Time Interval\n\n");
  for (k=0; k < gpDataStreams->GetNumStreams(); ++k) {
     fprintf(pFile, "Data Stream %u:\n", k);
     for (i=0; i < (unsigned int)m_nTimeIntervals; ++i)
       fprintf(pFile, "Measure_TotalByTimeInt [%i] = %12.5f\n", i, gpDataStreams->GetStream(k).GetPTMeasureArray()[i]);
     fprintf(pFile, "\n");
  }
}

// formats the information necessary in the relative risk output file and prints to the specified format
void CSVTTData::DisplayRelativeRisksForEachTract() const {
  try {
    RelativeRiskData  RelRiskData(gParameters);
    RelRiskData.RecordRelativeRiskData(*this);
    if (gParameters.GetOutputRelativeRisksAscii())
      ASCIIFileWriter(RelRiskData, gPrint, gParameters);
    if (gParameters.GetOutputRelativeRisksDBase())
      DBaseFileWriter(RelRiskData, gPrint, gParameters);
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
//  for (i=0; i < gpDataStreams->GetNumStreams(); ++i) {
//     fprintf(pFile, "Data Stream %u:\n", i);
//     DisplayCounts(pFile, gpDataStreams->GetStream(i).GetSimCaseArray(), "Simulated Cases Array",
//                   gpDataStreams->GetStream(i).GetNCSimCaseArray(), "Simulated Non-Cumulative Cases Array",
//                   gpDataStreams->GetStream(i).GetPTSimCasesArray(), "SimCases_TotalByTimeInt");
//  }
}

/** Randomizes collection of simulation data in concert with passed collection of randomizers. */
void CSVTTData::RandomizeData(RandomizerContainer_t& RandomizerContainer,
                              SimulationDataContainer_t& SimDataContainer,
                              unsigned int iSimulationNumber) const {
  try {
    CSaTScanData::RandomizeData(RandomizerContainer, SimDataContainer, iSimulationNumber);
    for (size_t t=0; t < SimDataContainer.size(); ++t) {
       SimDataContainer[t]->SetCaseArrays();
       //calculate time trend for entire randomized data set
       //TODO: The status of the time trend needs to be checked after CalculateAndSet() returns.
       //      The correct behavior for anything other than CTimeTrend::TREND_CONVERGED
       //      has not been decided yet.
       SimDataContainer[t]->GetTimeTrend().CalculateAndSet(gpDataStreams->GetStream(t).GetPTCasesArray(),
                                                           gpDataStreams->GetStream(t).GetPTMeasureArray(),
                                                           m_nTimeIntervals,
                                                           gParameters.GetTimeTrendConvergence());
       //QUESTION: Should the purely temporal case array passed to CalculateAndSet() be from
       //          the simulated data stream? It doesn't seem to make sense otherwise.
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("RandomizeData()","CSVTTData");
    throw;
  }
}

/** Redefines base class method to call data stream method
    DataStream::SetCaseArrays() which allocates and set temporal case array and
    non cumulative case array. */
void CSVTTData::SetAdditionalCaseArrays(RealDataStream& thisStream) {
  try {
    thisStream.SetCaseArrays();
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
    switch (gParameters.GetProbabiltyModelType()) {
       case POISSON              : m_pModel = new CPoissonModel(gParameters, *this, gPrint);   break;
       case BERNOULLI            : ZdException::Generate("Spatial Variation of Temporal Trends not implemented for Bernoulli model.\n",
                                                         "SetProbabilityModel()");
       case SPACETIMEPERMUTATION : ZdException::Generate("Spatial Variation of Temporal Trends not implemented for Space-Time Permutation model.\n",
                                                         "SetProbabilityModel()");
       case NORMAL               : ZdException::Generate("Spatial Variation of Temporal Trends not implemented for Normal model.\n",
                                                         "SetProbabilityModel()");
       case SURVIVAL             : ZdException::Generate("Spatial Variation of Temporal Trends not implemented for Survival model.\n",
                                                         "SetProbabilityModel()");
       case RANK                 : ZdException::Generate("Spatial Variation of Temporal Trends not implemented for Rank model.\n",
                                                         "SetProbabilityModel()");
       default : ZdException::Generate("Unknown probability model type: '%d'.\n",
                                       "SetProbabilityModel()", gParameters.GetProbabiltyModelType());
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("SetProbabilityModel()","CSVTTData");
    throw;
  }
}

