#include "SaTScan.h"
#pragma hdrstop
#include "SaTScanData.h"
#include "stsRelativeRisk.h"
#include "stsASCIIFileWriter.h"
#include "stsDBaseFileWriter.h"

void CSaTScanData::DisplayCases(FILE* pFile) {
  int                   i, j;
  count_t            ** ppCases(gpDataStreams->GetStream(0/*for now*/).GetCaseArray());

  fprintf(pFile, "Case counts (Cases Array)\n\n");
  for (i = 0; i < m_nTimeIntervals; ++i)
    for (j = 0; j < m_nTracts; ++j)
      fprintf(pFile, "Case [%i][%i] = %i\n", i, j, ppCases[i][j]);

  fprintf(pFile, "\n");
}

void CSaTScanData::DisplayControls(FILE* pFile) {
  int                   i, j;
  count_t            ** ppControls(gpDataStreams->GetStream(0/*for now*/).GetControlArray());

  fprintf(pFile, "Control counts (Controls Array)\n\n");

  for (i=0; i < m_nTimeIntervals; ++i)
    for (j=0; j < m_nTracts; ++j)
      fprintf(pFile, "Controls [%i][%i] = %i\n", i, j, ppControls[i][j]);

  fprintf(pFile, "\n");
}

void CSaTScanData::DisplaySimCases(FILE* pFile) {
//  int                   i, j;
//  count_t            ** ppSimCases(gpDataStreams->GetStream(0/*for now*/).GetSimCaseArray());
//
//  fprintf(pFile, "Simulated Case counts (Simulated Cases Array)\n\n");
//
//  for (i = 0; i < m_nTimeIntervals; ++i)
//    for (j = 0; j < m_nTracts; ++j)
//      fprintf(pFile, "Cases [%i][%i] = %i\n", i, j, ppSimCases[i][j]);
//
// fprintf(pFile, "\n");
}

void CSaTScanData::DisplayMeasure(FILE* pFile) {
  int           i, j;
  measure_t  ** ppMeasure(gpDataStreams->GetStream(0/*for now*/).GetMeasureArray());

  fprintf(pFile, "Measures (Measure Array)\n\n");

  for (i=0; i < m_nTimeIntervals; ++i)
    for (j=0; j < m_nTracts; ++j)
      fprintf(pFile, "Measure [%i][%i] = %12.25f\n", i, j, ppMeasure[i][j]);

  fprintf(pFile, "\n");
}

void CSaTScanData::DisplayNeighbors(FILE* pFile) {
  int                   i, j;
  std::string           sBuffer;
  tract_t            ** ppNeighborCount(gpNeighborCountHandler->GetArray());
  unsigned short    *** pppSortedUShort((gpSortedUShortHandler ? gpSortedUShortHandler->GetArray() : 0));
  tract_t           *** pppSortedInt((gpSortedIntHandler ? gpSortedIntHandler->GetArray() : 0));

  fprintf(pFile, "Neighbors (m_pSorted)   m_nGridTracts=%i\n\n", m_nGridTracts);

  for (i=0; i < m_nGridTracts; ++i) {
    fprintf(pFile, "Grid Point # %i : ", i);
    if (pppSortedInt)
       for (j=0; j < ppNeighborCount[0][i]; ++j)
         fprintf(pFile, "%s ", gpTInfo->tiGetTid(pppSortedInt[0][i][j], sBuffer));
    else
       for (j=0; j < ppNeighborCount[0][i]; ++j)
         fprintf(pFile, "%s ", gpTInfo->tiGetTid(pppSortedUShort[0][i][j], sBuffer));
    fprintf(pFile, "(# of neighbors=%i)\n", ppNeighborCount[0][i]);
  }

  fprintf(pFile,"\n");
}

void CSaTScanData::DisplaySummary(FILE* fp) {
  ZdString              sBuffer, sWork;
  AsciiPrintFormat      PrintFormat(gpDataStreams->GetNumStreams() == 1);
  unsigned int          i;


  PrintFormat.SetMarginsAsSummarySection();

  PrintFormat.PrintSectionSeparatorString(fp, 0, 2);
  fprintf(fp, "SUMMARY OF DATA\n\n");

  PrintFormat.PrintSectionLabel(fp, "Study period", false, false);
  fprintf(fp,"%s - %s\n",
          m_pParameters->GetStudyPeriodStartDate().c_str(),
          m_pParameters->GetStudyPeriodEndDate().c_str());

  PrintFormat.PrintSectionLabel(fp, "Number of locations", false, false);
  fprintf(fp, "%ld\n", (long) m_nTracts);

  if (m_pParameters->GetProbabiltyModelType() == POISSON || m_pParameters->GetProbabiltyModelType() == BERNOULLI) {
    PrintFormat.PrintSectionLabel(fp, "Total population", true, false);
    sBuffer.printf("%.0f", gpDataStreams->GetStream(0).GetTotalPopulation());
    for (i=1; i < gpDataStreams->GetNumStreams(); ++i) {
      sWork.printf(", %.0f", gpDataStreams->GetStream(i).GetTotalPopulation());
      sBuffer << sWork;
    }
    PrintFormat.PrintAlignedMarginsDataString(fp, sBuffer);
  }

  PrintFormat.PrintSectionLabel(fp, "Total cases", true, false);
  sBuffer.printf("%ld", gpDataStreams->GetStream(0).GetTotalCasesAtStart());
  for (i=1; i < gpDataStreams->GetNumStreams(); ++i) {
     sWork.printf(", %ld", gpDataStreams->GetStream(i).GetTotalCasesAtStart());
     sBuffer << sWork;
  }
  PrintFormat.PrintAlignedMarginsDataString(fp, sBuffer);

  if (m_pParameters->GetProbabiltyModelType() == POISSON) {
    sBuffer.printf("Annual cases / %.0f",  GetAnnualRatePop());
    PrintFormat.PrintSectionLabel(fp, sBuffer.GetCString(), true, false);
    sBuffer.printf("%.1f", GetAnnualRateAtStart(0));
    for (i=1; i < gpDataStreams->GetNumStreams(); ++i) {
       sWork.printf(", %.1f", GetAnnualRateAtStart(i));
       sBuffer << sWork;
    }
    PrintFormat.PrintAlignedMarginsDataString(fp, sBuffer);
  }

  if (m_pParameters->GetAnalysisType() == SPATIALVARTEMPTREND) {
    double nAnnualTT = gpDataStreams->GetStream(0/*for now*/).GetTimeTrend().SetAnnualTimeTrend(m_pParameters->GetTimeIntervalUnitsType(), m_pParameters->GetTimeIntervalLength());
    if (gpDataStreams->GetStream(0/*for now*/).GetTimeTrend().IsNegative())
      sBuffer = "Annual decrease";
    else
      sBuffer = "Annual increase";
    PrintFormat.PrintSectionLabel(fp, sBuffer.GetCString(), false, false);
    fprintf(fp, "%.3lf%%\n", nAnnualTT);
    fprintf(fp, "\n");
  }
  PrintFormat.PrintSectionSeparatorString(fp, 0, 1);
}

void CSaTScanData::DisplaySummary2(FILE* fp) {
  fprintf(fp, "SUMMARY OF DATA\n\n");
  fprintf(fp, "m_nTracts..............: %i\n", m_nTracts);
  fprintf(fp, "m_nGridTracts..........: %i\n", m_nGridTracts);
  fprintf(fp, "m_nTimeIntervals.......: %i\n", m_nTimeIntervals);
  fprintf(fp, "m_nIntervalCut.........: %i\n", m_nIntervalCut);
  for (size_t t=0; t < gpDataStreams->GetNumStreams(); ++t) {
    fprintf(fp, "Stream %i Summary...\n", t + 1);
    fprintf(fp, "TotalCasesAtStart...: %ld\n", gpDataStreams->GetStream(t).GetTotalCasesAtStart());
    fprintf(fp, "TotalCases..........: %ld\n", gpDataStreams->GetStream(t).GetTotalCases());
    fprintf(fp, "TotalPop............: %.0f\n", gpDataStreams->GetStream(t).GetTotalPopulation());
    fprintf(fp, "TotalMeasureAtStart.: %.0f\n", gpDataStreams->GetStream(t).GetTotalMeasureAtStart());
    fprintf(fp, "TotalMeasure........: %.0f\n", gpDataStreams->GetStream(t).GetTotalMeasure());
    fprintf(fp, "MaxCircleSize.......: %.2f\n", m_nMaxCircleSize);
  }
  AsciiPrintFormat::PrintSectionSeparatorString(fp, 0, 1);
}

// formats the information necessary in the relative risk output file and prints to the specified format
// pre: none
// post: prints the relative risk data to the output file
void CSaTScanData::DisplayRelativeRisksForEachTract() const {
  try {
    RelativeRiskData RelRiskData(*m_pParameters);
    RelRiskData.RecordRelativeRiskData(*this);
    if (m_pParameters->GetOutputRelativeRisksAscii())
      ASCIIFileWriter(RelRiskData, *gpPrint, *m_pParameters);
    if (m_pParameters->GetOutputRelativeRisksDBase())
      DBaseFileWriter(RelRiskData, *gpPrint, *m_pParameters);
  }
  catch (ZdException &x) {
    x.AddCallpath("DisplayRelativeRisksForEachTract()", "CSaTScanData");
    throw;
  }
}

