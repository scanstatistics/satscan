//---------------------------------------------------------------------------
#include "SaTScan.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#include "SaTScanData.h"
#include "LocationRiskEstimateWriter.h"
#include "AsciiPrintFormat.h"

void CSaTScanData::DisplayCases(FILE* pFile) {
  int                   i, j;
  count_t            ** ppCases(gDataSets->GetDataSet(0/*for now*/).GetCaseArray());

  fprintf(pFile, "Case counts (Cases Array)\n\n");
  for (i = 0; i < m_nTimeIntervals; ++i)
    for (j = 0; j < m_nTracts; ++j)
      fprintf(pFile, "Case [%i][%i] = %i\n", i, j, ppCases[i][j]);

  fprintf(pFile, "\n");
}

void CSaTScanData::DisplayControls(FILE* pFile) {
  int                   i, j;
  count_t            ** ppControls(gDataSets->GetDataSet(0/*for now*/).GetControlArray());

  fprintf(pFile, "Control counts (Controls Array)\n\n");

  for (i=0; i < m_nTimeIntervals; ++i)
    for (j=0; j < m_nTracts; ++j)
      fprintf(pFile, "Controls [%i][%i] = %i\n", i, j, ppControls[i][j]);

  fprintf(pFile, "\n");
}

void CSaTScanData::DisplaySimCases(FILE* pFile) {
//  int                   i, j;
//  count_t            ** ppSimCases(gDataSets->GetDataSet(0/*for now*/).GetSimCaseArray());
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
  measure_t  ** ppMeasure(gDataSets->GetDataSet(0/*for now*/).GetMeasureArray());

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
         fprintf(pFile, "%s ", gTractHandler->tiGetTid(pppSortedInt[0][i][j], sBuffer));
    else
       for (j=0; j < ppNeighborCount[0][i]; ++j)
         fprintf(pFile, "%s ", gTractHandler->tiGetTid(pppSortedUShort[0][i][j], sBuffer));
    fprintf(pFile, "(# of neighbors=%i)\n", ppNeighborCount[0][i]);
  }

  fprintf(pFile,"\n");
}

/** Prints summary of section to results file - detailing input data. */
void CSaTScanData::DisplaySummary(FILE* fp, ZdString sSummaryText, bool bPrintPeriod) {
  ZdString              sBuffer, sWork, sLabel;
  AsciiPrintFormat      PrintFormat(gDataSets->GetNumDataSets() == 1);
  unsigned int          i;

  PrintFormat.SetMarginsAsSummarySection();
  PrintFormat.PrintSectionSeparatorString(fp, 0, 2);
  fprintf(fp, "%s\n\n", sSummaryText.GetCString());
  //print study period
  if (bPrintPeriod) {
    PrintFormat.PrintSectionLabel(fp, "Study period", false, false);
    fprintf(fp,"%s - %s\n", gParameters.GetStudyPeriodStartDate().c_str(), gParameters.GetStudyPeriodEndDate().c_str());
  }  
  if (gParameters.UseCoordinatesFile() || gParameters.UseLocationNeighborsFile()) {
    //print number locations scanned
    PrintFormat.PrintSectionLabel(fp, "Number of locations", false, false);
    fprintf(fp, "%ld\n", (long)m_nTracts - GetNumNullifiedLocations());
  }  
  //print total population per data set
  switch (gParameters.GetProbabilityModelType()) {
    //label for data is dependent on probability model
    case POISSON     : if (!gParameters.UsePopulationFile()) break;
    case BERNOULLI   : PrintFormat.PrintSectionLabel(fp, "Total population", true, false); break;
    case EXPONENTIAL : PrintFormat.PrintSectionLabel(fp, "Total individuals", true, false); break;
    default          : break;
  }
  switch (gParameters.GetProbabilityModelType()) {
    case POISSON     : if (!gParameters.UsePopulationFile()) break;
    case BERNOULLI   :
    case EXPONENTIAL : sBuffer.printf("%.0f", gDataSets->GetDataSet(0).GetTotalPopulation());
                       for (i=1; i < gDataSets->GetNumDataSets(); ++i) {
                         sWork.printf(", %.0f", gDataSets->GetDataSet(i).GetTotalPopulation());
                         sBuffer << sWork;
                       }
                       PrintFormat.PrintAlignedMarginsDataString(fp, sBuffer);
                       break;
    default          : break;
  }
  // print total cases per data set
  switch (gParameters.GetProbabilityModelType()) {
    case POISSON              :
    case BERNOULLI            :
    case SPACETIMEPERMUTATION :
    case ORDINAL              :
    case NORMAL               :
    case EXPONENTIAL          : PrintFormat.PrintSectionLabel(fp, "Total number of cases", true, false);
                                sBuffer.printf("%ld", gDataSets->GetDataSet(0).GetTotalCases());
                                for (i=1; i < gDataSets->GetNumDataSets(); ++i) {
                                  sWork.printf(", %ld", gDataSets->GetDataSet(i).GetTotalCases());
                                  sBuffer << sWork;
                                }
                                PrintFormat.PrintAlignedMarginsDataString(fp, sBuffer);
                                break;
    default                   : break;
  }
  //for the ordinal probability model, also print category values and total cases per ordinal category
  if (gParameters.GetProbabilityModelType() == ORDINAL) {
    if (gDataSets->GetNumDataSets() == 1) {
      PrintFormat.PrintSectionLabel(fp, "Category values", false, false);
      const PopulationData& Population = gDataSets->GetDataSet().GetPopulationData();
      sBuffer="";
      for (size_t j=0; j < Population.GetNumOrdinalCategories(); ++j) {
         sWork.printf("%s%g", (j ? ", " : ""), Population.GetOrdinalCategoryValue(j));
         sBuffer << sWork;
      }
      PrintFormat.PrintAlignedMarginsDataString(fp, sBuffer);
      PrintFormat.PrintSectionLabel(fp, "Total cases per category", false, false);
      sBuffer="";
      for (size_t j=0; j < Population.GetNumOrdinalCategories(); ++j) {
         sWork.printf("%s%ld", (j ? ", " : ""), Population.GetNumOrdinalCategoryCases(j));
         sBuffer << sWork;
      }
      PrintFormat.PrintAlignedMarginsDataString(fp, sBuffer);
    }
    else {
      for (i=0, sBuffer=""; i < gDataSets->GetNumDataSets(); ++i, sBuffer="") {
        sLabel.printf("Category values, data set #%d", i + 1);
        PrintFormat.PrintSectionLabel(fp, sLabel.GetCString(), false, false);
        const PopulationData& Population = gDataSets->GetDataSet(i).GetPopulationData();
        for (size_t j=0; j < Population.GetNumOrdinalCategories(); ++j) {
           sWork.printf("%s%g", (j ? ", " : ""), Population.GetOrdinalCategoryValue(j));
           sBuffer << sWork;
        }
        PrintFormat.PrintAlignedMarginsDataString(fp, sBuffer);
        sLabel.printf("Total category cases, data set #%d ", i + 1);
        PrintFormat.PrintSectionLabel(fp, sLabel.GetCString(), false, false);
        sBuffer="";
        for (size_t j=0; j < Population.GetNumOrdinalCategories(); ++j) {
           sWork.printf("%s%ld", (j ? ", " : ""), Population.GetNumOrdinalCategoryCases(j));
           sBuffer << sWork;
        }
        PrintFormat.PrintAlignedMarginsDataString(fp, sBuffer);
      }
    }
  }
  //for the exponential probability model, also print total censored cases
  if (gParameters.GetProbabilityModelType() == EXPONENTIAL) {
    PrintFormat.PrintSectionLabel(fp, "Total censored", true, false);
    sBuffer.printf("%.0f", gDataSets->GetDataSet(0).GetTotalPopulation() - gDataSets->GetDataSet(0).GetTotalCases());
    for (i=1; i < gDataSets->GetNumDataSets(); ++i) {
       sWork.printf(", %.0f", gDataSets->GetDataSet(i).GetTotalPopulation() - gDataSets->GetDataSet(i).GetTotalCases());
       sBuffer << sWork;
    }
    PrintFormat.PrintAlignedMarginsDataString(fp, sBuffer);
  }
  //for the Poisson probability model, also print annual cases per population
  if (gParameters.GetProbabilityModelType() == POISSON && gParameters.UsePopulationFile()) {
    sBuffer.printf("Annual cases / %.0f",  GetAnnualRatePop());
    PrintFormat.PrintSectionLabel(fp, sBuffer.GetCString(), true, false);
    sBuffer.printf("%.1f", GetAnnualRate(0));
    for (i=1; i < gDataSets->GetNumDataSets(); ++i) {
       sWork.printf(", %.1f", GetAnnualRate(i));
       sBuffer << sWork;
    }
    PrintFormat.PrintAlignedMarginsDataString(fp, sBuffer);
  }
  if (gParameters.GetProbabilityModelType() == NORMAL) {
    PrintFormat.PrintSectionLabel(fp, "Mean", true, false);
    sBuffer.printf("%.2f", gDataSets->GetDataSet(0).GetTotalMeasure()/gDataSets->GetDataSet(0).GetTotalCases());
    for (i=1; i < gDataSets->GetNumDataSets(); ++i) {
       sWork.printf(", %.2f", gDataSets->GetDataSet(i).GetTotalMeasure()/gDataSets->GetDataSet(i).GetTotalCases());
       sBuffer << sWork;
    }
    PrintFormat.PrintAlignedMarginsDataString(fp, sBuffer);
    PrintFormat.PrintSectionLabel(fp, "Variance", true, false);
    sBuffer.printf("%.2f", GetUnbiasedVariance(gDataSets->GetDataSet(0).GetTotalCases(), gDataSets->GetDataSet(0).GetTotalMeasure(), gDataSets->GetDataSet(0).GetTotalMeasureSq()));
    for (i=1; i < gDataSets->GetNumDataSets(); ++i) {
       sWork.printf(", %.2f", GetUnbiasedVariance(gDataSets->GetDataSet(i).GetTotalCases(), gDataSets->GetDataSet(i).GetTotalMeasure(), gDataSets->GetDataSet(i).GetTotalMeasureSq()));
       sBuffer << sWork;
    }
    PrintFormat.PrintAlignedMarginsDataString(fp, sBuffer);
    PrintFormat.PrintSectionLabel(fp, "Standard deviation", true, false);
    sBuffer.printf("%.2f", std::sqrt(GetUnbiasedVariance(gDataSets->GetDataSet(0).GetTotalCases(), gDataSets->GetDataSet(0).GetTotalMeasure(), gDataSets->GetDataSet(0).GetTotalMeasureSq())));
    for (i=1; i < gDataSets->GetNumDataSets(); ++i) {
       sWork.printf(", %.2f", std::sqrt(GetUnbiasedVariance(gDataSets->GetDataSet(i).GetTotalCases(), gDataSets->GetDataSet(i).GetTotalMeasure(), gDataSets->GetDataSet(i).GetTotalMeasureSq())));
       sBuffer << sWork;
    }
    PrintFormat.PrintAlignedMarginsDataString(fp, sBuffer);
  }
  if (gParameters.GetAnalysisType() == SPATIALVARTEMPTREND) {
    double nAnnualTT = gDataSets->GetDataSet(0/*for now*/).GetTimeTrend().SetAnnualTimeTrend(gParameters.GetTimeAggregationUnitsType(), gParameters.GetTimeAggregationLength());
    if (nAnnualTT < 0)
      sBuffer = "Annual decrease";
    else
      sBuffer = "Annual increase";
    PrintFormat.PrintSectionLabel(fp, sBuffer.GetCString(), false, false);
    fprintf(fp, "%.3lf%%\n", fabs(nAnnualTT));
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
  for (size_t t=0; t < gDataSets->GetNumDataSets(); ++t) {
    fprintf(fp, "Set %i Summary...\n", t + 1);
    fprintf(fp, "TotalCasesAtStart...: %ld\n", gDataSets->GetDataSet(t).GetTotalCasesAtStart());
    fprintf(fp, "TotalCases..........: %ld\n", gDataSets->GetDataSet(t).GetTotalCases());
    fprintf(fp, "TotalPop............: %.0f\n", gDataSets->GetDataSet(t).GetTotalPopulation());
    fprintf(fp, "TotalMeasureAtStart.: %.0f\n", gDataSets->GetDataSet(t).GetTotalMeasureAtStart());
    fprintf(fp, "TotalMeasure........: %.0f\n", gDataSets->GetDataSet(t).GetTotalMeasure());
  }
  AsciiPrintFormat::PrintSectionSeparatorString(fp, 0, 1);
}

// formats the information necessary in the relative risk output file and prints to the specified format
// pre: none
// post: prints the relative risk data to the output file
void CSaTScanData::DisplayRelativeRisksForEachTract() const {
  try {
    LocationRiskEstimateWriter(*this).Write(*this);
  }
  catch (ZdException &x) {
    x.AddCallpath("DisplayRelativeRisksForEachTract()", "CSaTScanData");
    throw;
  }
}

