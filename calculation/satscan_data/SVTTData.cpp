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

/** constructor */
CSVTTData::CSVTTData(const CParameters* pParameters, BasePrint *pPrintDirection)
          :CSaTScanData(pParameters, pPrintDirection) {
  try {
    SetProbabilityModel();
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor()","CSVTTData");
    throw;
  }
}

/** destructor */
CSVTTData::~CSVTTData() {}

void CSVTTData::CalculateMeasure(RealDataStream & thisStream) {
  CSaTScanData::CalculateMeasure(thisStream);
  //calculate time trend for stream data set
  //TODO: The status of the time trend needs to be checked after CalculateAndSet() returns.
  //      The correct behavior for anything other than CTimeTrend::TREND_CONVERGED
  //      has not been decided yet.
  thisStream.GetTimeTrend().CalculateAndSet(thisStream.GetPTCasesArray(), thisStream.GetPTMeasureArray(),
                                            m_nTimeIntervals, m_pParameters->GetTimeTrendConvergence());
}

void CSVTTData::DisplayCases(FILE* pFile) {
    DisplayCounts(pFile, gpDataStreams->GetStream(0/*for now*/).GetCaseArray(), "Cases Array",
                  gpDataStreams->GetStream(0/*for now*/).GetNCCaseArray(), "Cases Non-Cumulative Array",
                  gpDataStreams->GetStream(0/*for now*/).GetPTCasesArray(), "Cases_TotalByTimeInt");
}

void CSVTTData::DisplayCounts(FILE* pFile,
                              count_t** pCounts,   char* szVarName,
                              count_t** pCountsNC, char* szVarNameNC,
                              count_t*  pCountsTI, char* szVarNameTI,
                              char* szTitle)
{
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

void CSVTTData::DisplayMeasures(FILE* pFile) {
  int           i;
  measure_t  ** ppMeasure, ** ppMeasureNC;

  fprintf(pFile, "Measures                        Measures - Not Accumulated\n\n");
  ppMeasure = gpDataStreams->GetStream(0/*for now*/).GetMeasureArray();
  ppMeasureNC = gpDataStreams->GetStream(0/*for now*/).GetNCMeasureArray();
  for (i = 0; i < m_nTimeIntervals; i++)
    for (int j = 0; j < m_nTracts; j++)
    {
      fprintf(pFile, "ppMeasure [%i][%i] = %12.5f     ", i, j, ppMeasure[i][j]);
      fprintf(pFile, "ppMeasure_NC [%i][%i] = %12.5f\n", i, j, ppMeasureNC[i][j]);
    }

  fprintf(pFile, "\nMeasures Accumulated by Time Interval\n\n");
  for (i=0; i<m_nTimeIntervals; i++)
    fprintf(pFile, "Measure_TotalByTimeInt [%i] = %12.5f\n", i, gpDataStreams->GetStream(0/*for now*/).GetPTMeasureArray()[i]);

  fprintf(pFile, "\n");
}

// formats the information necessary in the relative risk output file and prints to the specified format
void CSVTTData::DisplayRelativeRisksForEachTract() const {
  try {
    RelativeRiskData  RelRiskData(*m_pParameters);
    RelRiskData.RecordRelativeRiskData(*this);
    if (m_pParameters->GetOutputRelativeRisksAscii())
      ASCIIFileWriter(RelRiskData, *gpPrint, *m_pParameters);
    if (m_pParameters->GetOutputRelativeRisksDBase())
      DBaseFileWriter(RelRiskData, *gpPrint, *m_pParameters);
  }
  catch (ZdException &x) {
    x.AddCallpath("DisplayRelativeRisksForEachTract()","CSVTTData");
    throw;
  }
}
void CSVTTData::DisplaySimCases(FILE* pFile) {
//  DisplayCounts(pFile, gpDataStreams->GetStream(0/*for now*/).GetSimCaseArray(), "Simulated Cases Array",
//               gpDataStreams->GetStream(0/*for now*/).GetNCSimCaseArray(), "Simulated Non-Cumulative Cases Array",
//                gpDataStreams->GetStream(0/*for now*/).GetPTSimCasesArray(), "SimCases_TotalByTimeInt");
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
                                                           m_pParameters->GetTimeTrendConvergence());
       //QUESTION: Should the purely temporal case array passed to CalculateAndSet() be from
       //          the simulated data stream? It doesn't seem to make sense otherwise.
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("RandomizeData()","CSVTTData");
    throw;
  }
}

void CSVTTData::SetAdditionalCaseArrays(RealDataStream & thisStream) {
  try {
    thisStream.SetCaseArrays();
  }
  catch (ZdException &x) {
    x.AddCallpath("SetAdditionalCaseArrays()","CSVTTData");
    throw;
  }
}

void CSVTTData::SetNumTimeIntervals() {
  CSaTScanData::SetNumTimeIntervals();
  if (m_nTimeIntervals <= 1)
    GenerateResolvableException("Error: A spatial variation of temporal trends analysis requires\n"
                                "       more than one time interval.\n", "SetNumTimeIntervals()");
}

/** allocates probability model */
void CSVTTData::SetProbabilityModel() {
  try {
    switch (m_pParameters->GetProbabiltyModelType()) {
       case POISSON              : m_pModel = new CPoissonModel(*m_pParameters, *this, *gpPrint);   break;
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
                                       "SetProbabilityModel()", m_pParameters->GetProbabiltyModelType());
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("SetProbabilityModel()","CSVTTData");
    throw;
  }
}

