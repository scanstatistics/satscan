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
CSVTTData::CSVTTData(CParameters* pParameters, BasePrint *pPrintDirection)
          :CSaTScanData(pParameters, pPrintDirection) {
  try {
    Init();
    SetProbabilityModel();
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor()","CSVTTData");
    throw;
  }
}

/** destructor */
CSVTTData::~CSVTTData() {
  try {
    free (m_pSumLogs);
  }
  catch (...){}
}

bool CSVTTData::CalculateMeasure() {
  bool bReturn = CSaTScanData::CalculateMeasure();
  SetSumLogs();
  m_nTimeTrend.CalculateAndSet(m_pCases_TotalByTimeInt, m_pMeasure_TotalByTimeInt,
                               m_nTimeIntervals, m_pParameters->GetTimeTrendConvergence());
  return bReturn;
}

void CSVTTData::DisplayCases(FILE* pFile) {
    DisplayCounts(pFile, gpCasesHandler->GetArray(), "Cases Array",
                  gpCasesNonCumulativeHandler->GetArray(), "Cases Non-Cumulative Array",
                  m_pCases_TotalByTimeInt, "m_pCases_TotalByTimeInt");
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
  if (!gpMeasureHandler || !gpMeasureNonCumulativeHandler || m_pMeasure_TotalByTimeInt==0) {
    fprintf(pFile, "Measures are not defined.\n");
    return;
  }

  ppMeasure = gpMeasureHandler->GetArray();
  ppMeasureNC = gpMeasureNonCumulativeHandler->GetArray();
  for (i = 0; i < m_nTimeIntervals; i++)
    for (int j = 0; j < m_nTracts; j++)
    {
      fprintf(pFile, "ppMeasure [%i][%i] = %12.5f     ", i, j, ppMeasure[i][j]);
      fprintf(pFile, "ppMeasure_NC [%i][%i] = %12.5f\n", i, j, ppMeasureNC[i][j]);
    }

  fprintf(pFile, "\nMeasures Accumulated by Time Interval\n\n");
  for (i=0; i<m_nTimeIntervals; i++)
    fprintf(pFile, "m_pMeasure_TotalByTimeInt [%i] = %12.5f\n", i, m_pMeasure_TotalByTimeInt[i]);

  fprintf(pFile, "\n");
}

// formats the information necessary in the relative risk output file and prints to the specified format
void CSVTTData::DisplayRelativeRisksForEachTract(const bool bASCIIOutput, const bool bDBaseOutput) {
  std::string                          sBuffer;
  ZdString                             sRisk, sLocation, sTimeTrend;
  std::vector<std::string>             vIdentifiers;
  CTimeTrend                           nTractTT;
  count_t                              nTractCases, * CasesByTI=0,
                                    ** ppCases(gpCasesHandler->GetArray()),
                                    ** ppCaseNC(gpCasesNonCumulativeHandler->GetArray());
  measure_t                          * MsrByTI=0, ** ppMeasure(gpMeasureHandler->GetArray()),
                                    ** ppMeasureNC(gpMeasureNonCumulativeHandler->GetArray());
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
  DisplayCounts(pFile, gpSimCasesHandler->GetArray(), "Simulated Cases Array",
                gpSimCasesNonCumulativeHandler->GetArray(), "Simulated Non-Cumulative Cases Array",
                m_pSimCases_TotalByTimeInt, "m_pSimCases_TotalByTimeInt");
}

void CSVTTData::MakeData(int iSimulationNumber) {
  try {
    CSaTScanData::MakeData(iSimulationNumber);
    //allocate/set non-cumulative and by time interval simulation case arrays
    if (!gpSimCasesNonCumulativeHandler)
      gpSimCasesNonCumulativeHandler = new TwoDimensionArrayHandler<count_t>(m_nTimeIntervals, m_nTracts, 0);
    if (!m_pSimCases_TotalByTimeInt)
      m_pSimCases_TotalByTimeInt = new count_t[m_nTimeIntervals];
    SetCaseArrays(gpSimCasesHandler->GetArray(), gpSimCasesNonCumulativeHandler->GetArray(), m_pSimCases_TotalByTimeInt);
    m_nTimeTrend_Sim.CalculateAndSet(m_pCases_TotalByTimeInt, m_pMeasure_TotalByTimeInt,
                                     m_nTimeIntervals, m_pParameters->GetTimeTrendConvergence());
  }
  catch (ZdException &x) {
    delete gpSimCasesNonCumulativeHandler; gpSimCasesNonCumulativeHandler=0;
    delete m_pSimCases_TotalByTimeInt; m_pSimCases_TotalByTimeInt=0;
    x.AddCallpath("MakeData()","CSVTTData");
    throw;
  }
}

void CSVTTData::SetAdditionalCaseArrays() {
  try {
    //allocate/set non-cumulative and by time interval case arrays
    if (!gpCasesNonCumulativeHandler)
      gpCasesNonCumulativeHandler = new TwoDimensionArrayHandler<count_t>(m_nTimeIntervals, m_nTracts);
    if (!m_pCases_TotalByTimeInt)
      m_pCases_TotalByTimeInt = new count_t[m_nTimeIntervals];
    SetCaseArrays(gpCasesHandler->GetArray(), gpCasesNonCumulativeHandler->GetArray(), m_pCases_TotalByTimeInt);
  }
  catch (ZdException &x) {
    delete gpCasesNonCumulativeHandler; gpCasesNonCumulativeHandler=0;
    delete m_pCases_TotalByTimeInt; m_pCases_TotalByTimeInt=0;
    x.AddCallpath("SetAdditionalCaseArrays()","CSVTTData");
    throw;
  }
}

void CSVTTData::SetCaseArrays(count_t** pCases, count_t** pCases_NC, count_t* pCasesByTimeInt) {
  int   i, j;

  for (i=0; i < m_nTimeIntervals; i++)
    pCasesByTimeInt[i] = 0;

  for (i=0; i < m_nTracts; i++)  {
    pCases_NC[m_nTimeIntervals-1][i] = pCases[m_nTimeIntervals-1][i];
    pCasesByTimeInt[m_nTimeIntervals-1] += pCases_NC[m_nTimeIntervals-1][i];
    for (j=m_nTimeIntervals-2; j >= 0; j--) {
      pCases_NC[j][i] = pCases[j][i] - pCases[j+1][i];
      pCasesByTimeInt[j] += pCases_NC[j][i];
    }
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
       default : ZdException::Generate("Unknown probability model type: '%d'.\n",
                                       "SetProbabilityModel()", m_pParameters->GetProbabiltyModelType());
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("SetProbabilityModel()","CSVTTData");
    throw;
  }
}

// Added 990915 GG
void CSVTTData::SetSumLogs()
{
  if ((m_pSumLogs) == NULL && m_nTotalCases != 0)
  {
    m_pSumLogs = (double *) Smalloc((m_nTotalCases+1) * sizeof(double), gpPrint);
    m_pSumLogs[0] = 0;
    m_pSumLogs[1] = 0;

    for (int i=2; i<=m_nTotalCases; i++)
    {
      m_pSumLogs[i] = m_pSumLogs[i-1] + log(i);
    }
  }
}


