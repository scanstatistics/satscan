#include "SaTScan.h"
#pragma hdrstop
#include "SaTScanData.h"
#include "stsRelativeRisk.h"
#include "stsASCIIFileWriter.h"
#include "stsDBaseFileWriter.h"

void CSaTScanData::DisplayCases(FILE* pFile) {
  fprintf(pFile, "Case counts (m_pCases)\n\n");

  for (int i = 0; i < m_nTimeIntervals; ++i)
    for (int j = 0; j < m_nTracts; ++j)
      fprintf(pFile, "Case [%i][%i] = %i\n", i,j,m_pCases[i][j]);

  fprintf(pFile, "\n");
}

void CSaTScanData::DisplayControls(FILE* pFile) {
  fprintf(pFile, "Control counts (m_pControls)\n\n");

  for (int i = 0; i < m_nTimeIntervals; ++i)
    for (int j = 0; j < m_nTracts; ++j)
      fprintf(pFile, "Controls [%i][%i] = %i\n", i,j,m_pControls[i][j]);

  fprintf(pFile, "\n");
}

void CSaTScanData::DisplaySimCases(FILE* pFile) {
  fprintf(pFile, "Simulated Case counts (m_pSimCases)\n\n");

  for (int i = 0; i < m_nTimeIntervals; ++i)
    for (int j = 0; j < m_nTracts; ++j)
      fprintf(pFile, "Cases [%i][%i] = %i\n", i,j,m_pSimCases[i][j]);

  fprintf(pFile, "\n");
}

void CSaTScanData::DisplayMeasure(FILE* pFile) {
  fprintf(pFile, "Measures (m_pMeasure)\n\n");

  for (int i = 0; i < m_nTimeIntervals; ++i)
    for (int j = 0; j < m_nTracts; ++j)
      fprintf(pFile, "Measure [%i][%i] = %12.25f\n", i,j,m_pMeasure[i][j]);

  fprintf(pFile, "\n");
}

void CSaTScanData::DisplayNeighbors(FILE* pFile) {
  std::string sBuffer;
  fprintf(pFile, "Neighbors (m_pSorted)   m_nGridTracts=%i\n\n", m_nGridTracts);

  for (int i = 0; i < m_nGridTracts; ++i) {
    fprintf(pFile, "Grid Point # %i : ", i);
    if (m_pSortedInt)
       for (int j = 0; j < m_NeighborCounts[0][i]; ++j)
         fprintf(pFile, "%s ", gpTInfo->tiGetTid(m_pSortedInt[0][i][j], sBuffer));
    else
       for (int j = 0; j < m_NeighborCounts[0][i]; ++j)
         fprintf(pFile, "%s ", gpTInfo->tiGetTid(m_pSortedUShort[0][i][j], sBuffer));
    fprintf(pFile, "(# of neighbors=%i)\n", m_NeighborCounts[0][i]);
  }

  fprintf(pFile,"\n");
}

void CSaTScanData::DisplaySummary(FILE* fp) {
  fprintf(fp, "________________________________________________________________\n\n");
  fprintf(fp, "SUMMARY OF DATA\n\n");
  fprintf(fp, "Study period .........: %s - %s\n",
              m_pParameters->m_szStartDate,
              m_pParameters->m_szEndDate);
  fprintf(fp, "Number of census areas: %ld\n", (long) m_nTracts);
  if (m_pParameters->m_nModel != SPACETIMEPERMUTATION)
    fprintf(fp, "Total population .....: %.0f\n", m_nTotalPop);
  fprintf(fp, "Total cases ..........: %ld\n",  m_nTotalCasesAtStart);
  if (m_pParameters->m_nModel == POISSON)
    fprintf(fp, "Annual cases / %.0f.: %.1f\n",
                GetAnnualRatePop(), GetAnnualRateAtStart());
  fprintf(fp, "________________________________________________________________\n");
}

void CSaTScanData::DisplaySummary2(FILE* fp) {
  fprintf(fp, "SUMMARY OF DATA\n\n");
  fprintf(fp, "m_nTracts..............: %i\n", m_nTracts);
  fprintf(fp, "m_nGridTracts..........: %i\n", m_nGridTracts);
  fprintf(fp, "m_nTimeIntervals.......: %i\n", m_nTimeIntervals);
  fprintf(fp, "m_nIntervalCut.........: %i\n", m_nIntervalCut);
  fprintf(fp, "m_nTotalCasesAtStart...: %ld\n", m_nTotalCasesAtStart);
  fprintf(fp, "m_nTotalCases..........: %ld\n", m_nTotalCases);
  fprintf(fp, "m_nTotalPop............: %.0f\n", m_nTotalPop);
  fprintf(fp, "m_nTotalMeasureAtStart.: %.0f\n", m_nTotalMeasureAtStart);
  fprintf(fp, "m_nTotalMeasure........: %.0f\n", m_nTotalMeasure);
  fprintf(fp, "m_nMaxCircleSize.......: %.2f\n", m_nMaxCircleSize);
  fprintf(fp, "________________________________________________________________\n");
}

void CSaTScanData::DisplayRelativeRisksForEachTract(const bool bASCIIOutput, const bool bDBaseOutput) {
   RelativeRiskData     *pData = 0;
   std::string          sBuffer;
   ZdString             sRisk;

   try {
      pData = new RelativeRiskData(gpPrintDirection, m_pParameters->m_szOutputFilename);
      for(int i = 0; i < m_nTracts; ++i) {
         if (GetMeasureAdjustment() && m_pMeasure[0][i])
            sRisk.printf("%12.3f", ((double)(m_pCases[0][i]))/(GetMeasureAdjustment()*m_pMeasure[0][i]));
         else
            sRisk = "n/a";
         pData->SetRelativeRiskData(gpTInfo->tiGetTid(i, sBuffer), m_pCases[0][i],
                                    GetMeasureAdjustment()*m_pMeasure[0][i],
                                    sRisk);
      }
      if (bASCIIOutput) {
         ASCIIFileWriter Awriter(pData);
         Awriter.Print();
      }
      if (bDBaseOutput) {
         DBaseFileWriter DWriter(pData);
         DWriter.Print();
      }

      delete pData; pData = 0;
   }
   catch (ZdException &x) {
      delete pData; pData = 0;
      x.AddCallpath("DisplayRelativeRisksForEachTract()", "CSaTScanData");
      throw;
   }
}

