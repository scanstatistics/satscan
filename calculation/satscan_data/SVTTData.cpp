#include "SaTScan.h"
#pragma hdrstop
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
void CSVTTData::DisplayRelativeRisksForEachTract(const bool bASCIIOutput, const bool bDBaseOutput) {
  std::string                          sBuffer;
  ZdString                             sRisk, sLocation, sTimeTrend;
  std::vector<std::string>             vIdentifiers;
  CTimeTrend                           nTractTT;
  count_t                              nTractCases, * CasesByTI=0,
                                    ** ppCases(gpDataStreams->GetStream(0/*for now*/).GetCaseArray()),
                                    ** ppCaseNC(gpDataStreams->GetStream(0/*for now*/).GetNCCaseArray());
  measure_t                          * MsrByTI=0, ** ppMeasure(gpDataStreams->GetStream(0/*for now*/).GetMeasureArray()),
                                    ** ppMeasureNC(gpDataStreams->GetStream(0/*for now*/).GetNCMeasureArray());
  int                                  i ,j;

   try {
      std::auto_ptr<RelativeRiskData> pData( new RelativeRiskData(gpPrint, *m_pParameters) );
      CasesByTI = (count_t*)   Smalloc((m_nTimeIntervals)*sizeof(count_t), gpPrint);
      MsrByTI   = (measure_t*) Smalloc((m_nTimeIntervals)*sizeof(measure_t), gpPrint);

      for(i=0; i < m_nTracts; ++i) {
         if (GetMeasureAdjustment() && ppMeasure[0][i])
            sRisk.printf("%12.3f", ((double)(ppCases[0][i]))/(GetMeasureAdjustment()*ppMeasure[0][i]));
         else
            sRisk = "n/a";
         gpTInfo->tiGetTractIdentifiers(i, vIdentifiers);
         sLocation = gpTInfo->tiGetTid(i, sBuffer);
         if (vIdentifiers.size() > 1)
            sLocation << " et al";
         // Set Array of cases/measures by time interval for current time tract
         for (j=0; j < m_nTimeIntervals; j++) {
           CasesByTI[j]  = ppCaseNC[j][i];
           MsrByTI[j]    = ppMeasureNC[j][i];
         }
         //calculate time trend for location
         //TODO: The status of the time trend needs to be checked after CalculateAndSet() returns.
         //      The correct behavior for anything other than CTimeTrend::TREND_CONVERGED
         //      has not been decided yet.
         nTractTT.CalculateAndSet(CasesByTI, MsrByTI, m_nTimeIntervals, m_pParameters->GetTimeTrendConvergence());   //KR990921 - Handle Bernoulli if implemented!
         nTractTT.SetAnnualTimeTrend(m_pParameters->GetTimeIntervalUnitsType(), m_pParameters->GetTimeIntervalLength());
         sTimeTrend.printf("%6.3f", (nTractTT.IsNegative() ? -1 : 1) * nTractTT.GetAnnualTimeTrend());
         pData->SetRelativeRiskData(sLocation, ppCases[0][i], GetMeasureAdjustment()*ppMeasure[0][i], sRisk, sTimeTrend);
      }
      free(CasesByTI);
      free(MsrByTI);
      if (bASCIIOutput)
         ASCIIFileWriter(pData.get()).Print();
      if (bDBaseOutput)
         DBaseFileWriter(pData.get()).Print();
   }
   catch (ZdException &x) {
      free(CasesByTI);
      free(MsrByTI);
      x.AddCallpath("DisplayRelativeRisksForEachTract()","CSVTTData");
      throw;
   }
}
void CSVTTData::DisplaySimCases(FILE* pFile) {
//  DisplayCounts(pFile, gpDataStreams->GetStream(0/*for now*/).GetSimCaseArray(), "Simulated Cases Array",
//               gpDataStreams->GetStream(0/*for now*/).GetNCSimCaseArray(), "Simulated Non-Cumulative Cases Array",
//                gpDataStreams->GetStream(0/*for now*/).GetPTSimCasesArray(), "SimCases_TotalByTimeInt");
}

void CSVTTData::RandomizeData(SimulationDataContainer_t& SimDataContainer, unsigned int iSimulationNumber) {
  try {
    CSaTScanData::RandomizeData(SimDataContainer, iSimulationNumber);
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
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("RandomizeData()","CSVTTData");
    throw;
  }
}

/** Randomizes collection of simulation data in concert with passed collection of randomizers. */
void CSVTTData::RandomizeIsolatedData(RandomizerContainer_t& RandomizerContainer,
                                      SimulationDataContainer_t& SimDataContainer,
                                      unsigned int iSimulationNumber) const {
  try {
    CSaTScanData::RandomizeIsolatedData(RandomizerContainer, SimDataContainer, iSimulationNumber);
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
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("RandomizeIsolatedData()","CSVTTData");
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
    SSGenerateException("Error: A spatial variation of temporal trends analysis requires\n"
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

