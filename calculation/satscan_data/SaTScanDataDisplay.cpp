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
  fprintf(fp, "________________________________________________________________\n\n");
  fprintf(fp, "SUMMARY OF DATA\n\n");
  fprintf(fp, "Study period .........: %s - %s\n",
              m_pParameters->GetStudyPeriodStartDate().c_str(),
              m_pParameters->GetStudyPeriodEndDate().c_str());
  fprintf(fp, "Number of census areas: %ld\n", (long) m_nTracts);
  if (m_pParameters->GetProbabiltyModelType() == POISSON || m_pParameters->GetProbabiltyModelType() == BERNOULLI)
    fprintf(fp, "Total population .....: %.0f\n", gpDataStreams->GetStream(0/*for now*/).GetTotalPopulation());
  fprintf(fp, "Total cases ..........: %ld\n",  gpDataStreams->GetStream(0/*for now*/).GetTotalCasesAtStart());
  if (m_pParameters->GetProbabiltyModelType() == POISSON)
    fprintf(fp, "Annual cases / %.0f.: %.1f\n",
                GetAnnualRatePop(), GetAnnualRateAtStart());
  if (m_pParameters->GetAnalysisType() == SPATIALVARTEMPTREND) {
    double nAnnualTT = gpDataStreams->GetStream(0/*for now*/).GetTimeTrend().SetAnnualTimeTrend(m_pParameters->GetTimeIntervalUnitsType(), m_pParameters->GetTimeIntervalLength());
    if (gpDataStreams->GetStream(0/*for now*/).GetTimeTrend().IsNegative())
      fprintf(fp, "Annual decrease.......: %.3lf%%\n", nAnnualTT);
    else
      fprintf(fp, "Annual increase.......: %.3lf%%\n", nAnnualTT);
  }
  fprintf(fp, "________________________________________________________________\n");
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
  fprintf(fp, "________________________________________________________________\n");
}

// formats the information necessary in the relative risk output file and prints to the specified format
// pre: none
// post: prints the relative risk data to the output file
void CSaTScanData::DisplayRelativeRisksForEachTract(const bool bASCIIOutput, const bool bDBaseOutput) {
  std::string                          sBuffer;
  ZdString                             sRisk, sLocation;
  std::vector<std::string>             vIdentifiers;
  //NOTE: Displaying of relative risk information is restricted to the first
  //      data stream, at least for the time being. 
  count_t                           ** ppCases(gpDataStreams->GetStream(0).GetCaseArray());
  measure_t                         ** ppMeasure(gpDataStreams->GetStream(0).GetMeasureArray());

   try {
      std::auto_ptr<RelativeRiskData> pData( new RelativeRiskData(gpPrint, *m_pParameters) );
      for(int i = 0; i < m_nTracts; ++i) {
         if (GetMeasureAdjustment() && ppMeasure[0][i])
            sRisk.printf("%12.3f", ((double)(ppCases[0][i]))/(GetMeasureAdjustment()*ppMeasure[0][i]));
         else
            sRisk = "n/a";
         gpTInfo->tiGetTractIdentifiers(i, vIdentifiers);
         sLocation = gpTInfo->tiGetTid(i, sBuffer);
         if (vIdentifiers.size() > 1)
            sLocation << " et al";
         pData->SetRelativeRiskData(sLocation, ppCases[0][i], GetMeasureAdjustment()*ppMeasure[0][i], sRisk);
      }
      if (bASCIIOutput)
         ASCIIFileWriter(pData.get()).Print();
      if (bDBaseOutput)
         DBaseFileWriter(pData.get()).Print();
   }
   catch (ZdException &x) {
      x.AddCallpath("DisplayRelativeRisksForEachTract()", "CSaTScanData");
      throw;
   }
}

