//---------------------------------------------------------------------------
#include "SaTScan.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#include "SaTScanData.h"
#include "LocationRiskEstimateWriter.h"

void CSaTScanData::DisplayCases(FILE* pFile) {
  int                   i, j;
  count_t            ** ppCases(gpDataSets->GetDataSet(0/*for now*/).GetCaseArray());

  fprintf(pFile, "Case counts (Cases Array)\n\n");
  for (i = 0; i < m_nTimeIntervals; ++i)
    for (j = 0; j < m_nTracts; ++j)
      fprintf(pFile, "Case [%i][%i] = %i\n", i, j, ppCases[i][j]);

  fprintf(pFile, "\n");
}

void CSaTScanData::DisplayControls(FILE* pFile) {
  int                   i, j;
  count_t            ** ppControls(gpDataSets->GetDataSet(0/*for now*/).GetControlArray());

  fprintf(pFile, "Control counts (Controls Array)\n\n");

  for (i=0; i < m_nTimeIntervals; ++i)
    for (j=0; j < m_nTracts; ++j)
      fprintf(pFile, "Controls [%i][%i] = %i\n", i, j, ppControls[i][j]);

  fprintf(pFile, "\n");
}

void CSaTScanData::DisplaySimCases(FILE* pFile) {
//  int                   i, j;
//  count_t            ** ppSimCases(gpDataSets->GetDataSet(0/*for now*/).GetSimCaseArray());
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
  measure_t  ** ppMeasure(gpDataSets->GetDataSet(0/*for now*/).GetMeasureArray());

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
         fprintf(pFile, "%s ", gTractHandler.tiGetTid(pppSortedInt[0][i][j], sBuffer));
    else
       for (j=0; j < ppNeighborCount[0][i]; ++j)
         fprintf(pFile, "%s ", gTractHandler.tiGetTid(pppSortedUShort[0][i][j], sBuffer));
    fprintf(pFile, "(# of neighbors=%i)\n", ppNeighborCount[0][i]);
  }

  fprintf(pFile,"\n");
}

void CSaTScanData::DisplaySummary(FILE* fp) {
  ZdString              sBuffer, sWork, sLabel;
  AsciiPrintFormat      PrintFormat(gpDataSets->GetNumDataSets() == 1);
  unsigned int          i;

  PrintFormat.SetMarginsAsSummarySection();

  PrintFormat.PrintSectionSeparatorString(fp, 0, 2);
  fprintf(fp, "SUMMARY OF DATA\n\n");

  PrintFormat.PrintSectionLabel(fp, "Study period", false, false);
  fprintf(fp,"%s - %s\n",
          gParameters.GetStudyPeriodStartDate().c_str(),
          gParameters.GetStudyPeriodEndDate().c_str());

  PrintFormat.PrintSectionLabel(fp, "Number of locations", false, false);
  fprintf(fp, "%ld\n", (long) m_nTracts);

  // print total population per data set
  switch (gParameters.GetProbabilityModelType()) {
    case POISSON :
      if (!gParameters.UsePopulationFile())
        break;
    case BERNOULLI :
    case ORDINAL :
      PrintFormat.PrintSectionLabel(fp, "Total population", true, false);
      sBuffer.printf("%.0f", gpDataSets->GetDataSet(0).GetTotalPopulation());
      for (i=1; i < gpDataSets->GetNumDataSets(); ++i) {
        sWork.printf(", %.0f", gpDataSets->GetDataSet(i).GetTotalPopulation());
        sBuffer << sWork;
      }
      PrintFormat.PrintAlignedMarginsDataString(fp, sBuffer);
      break;
    default : break;
  }

  // print total cases per data set
  switch (gParameters.GetProbabilityModelType()) {
    case POISSON :
    case BERNOULLI :
      PrintFormat.PrintSectionLabel(fp, "Total cases", true, false);
      sBuffer.printf("%ld", gpDataSets->GetDataSet(0).GetTotalCasesAtStart());
      for (i=1; i < gpDataSets->GetNumDataSets(); ++i) {
        sWork.printf(", %ld", gpDataSets->GetDataSet(i).GetTotalCasesAtStart());
        sBuffer << sWork;
      }
      PrintFormat.PrintAlignedMarginsDataString(fp, sBuffer);
      break;
    case ORDINAL :
      //total cases per data set, per category
      for (i=0; i < gpDataSets->GetNumDataSets(); ++i, sBuffer="") {
        sLabel.printf("Total cases data set #%d per category", i + 1);
        PrintFormat.PrintSectionLabel(fp, sLabel.GetCString(), false, false);
        const PopulationData& Population = gpDataSets->GetDataSet(i).GetPopulationData();
        for (size_t j=0; j < Population.GetNumOrdinalCategories(); ++j) {
           sWork.printf("%s%ld", (j ? ", " : ""), Population.GetNumOrdinalCategoryCases(j));
           sBuffer << sWork;
        }
        PrintFormat.PrintAlignedMarginsDataString(fp, sBuffer);
      }
      break;
    default : break;
  }


  if (gParameters.GetProbabilityModelType() == POISSON && gParameters.UsePopulationFile()) {
    sBuffer.printf("Annual cases / %.0f",  GetAnnualRatePop());
    PrintFormat.PrintSectionLabel(fp, sBuffer.GetCString(), true, false);
    sBuffer.printf("%.1f", GetAnnualRateAtStart(0));
    for (i=1; i < gpDataSets->GetNumDataSets(); ++i) {
       sWork.printf(", %.1f", GetAnnualRateAtStart(i));
       sBuffer << sWork;
    }
    PrintFormat.PrintAlignedMarginsDataString(fp, sBuffer);
  }

  if (gParameters.GetAnalysisType() == SPATIALVARTEMPTREND) {
    double nAnnualTT = gpDataSets->GetDataSet(0/*for now*/).GetTimeTrend().SetAnnualTimeTrend(gParameters.GetTimeAggregationUnitsType(), gParameters.GetTimeAggregationLength());
    if (gpDataSets->GetDataSet(0/*for now*/).GetTimeTrend().IsNegative())
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
  for (size_t t=0; t < gpDataSets->GetNumDataSets(); ++t) {
    fprintf(fp, "Set %i Summary...\n", t + 1);
    fprintf(fp, "TotalCasesAtStart...: %ld\n", gpDataSets->GetDataSet(t).GetTotalCasesAtStart());
    fprintf(fp, "TotalCases..........: %ld\n", gpDataSets->GetDataSet(t).GetTotalCases());
    fprintf(fp, "TotalPop............: %.0f\n", gpDataSets->GetDataSet(t).GetTotalPopulation());
    fprintf(fp, "TotalMeasureAtStart.: %.0f\n", gpDataSets->GetDataSet(t).GetTotalMeasureAtStart());
    fprintf(fp, "TotalMeasure........: %.0f\n", gpDataSets->GetDataSet(t).GetTotalMeasure());
    fprintf(fp, "MaxCircleSize.......: %.2f\n", m_nMaxCircleSize);
  }
  AsciiPrintFormat::PrintSectionSeparatorString(fp, 0, 1);
}

// formats the information necessary in the relative risk output file and prints to the specified format
// pre: none
// post: prints the relative risk data to the output file
void CSaTScanData::DisplayRelativeRisksForEachTract() const {
  try {
    LocationRiskEstimateWriter(gParameters).Write(*this);
  }
  catch (ZdException &x) {
    x.AddCallpath("DisplayRelativeRisksForEachTract()", "CSaTScanData");
    throw;
  }
}

