//---------------------------------------------------------------------------
#include "SaTScan.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#include "SaTScanData.h"
#include "LocationRiskEstimateWriter.h"
#include "AsciiPrintFormat.h"
#include "SSException.h"

void CSaTScanData::DisplayCases(FILE* pFile) {
  int                   i, j;
  count_t            ** ppCases(gDataSets->GetDataSet(0/*for now*/).getCaseData().GetArray());

  fprintf(pFile, "Case counts (Cases Array)\n\n");
  for (i = 0; i < m_nTimeIntervals; ++i)
    for (j = 0; j < m_nTracts; ++j)
      fprintf(pFile, "Case [%i][%i] = %i\n", i, j, ppCases[i][j]);

  fprintf(pFile, "\n");
}

void CSaTScanData::DisplayControls(FILE* pFile) {
  int                   i, j;
  count_t            ** ppControls(gDataSets->GetDataSet(0/*for now*/).getControlData().GetArray());

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
  measure_t  ** ppMeasure(gDataSets->GetDataSet(0/*for now*/).getMeasureData().GetArray());

  fprintf(pFile, "Measures (Measure Array)\n\n");

  for (i=0; i < m_nTimeIntervals; ++i)
    for (j=0; j < m_nTracts; ++j)
      fprintf(pFile, "Measure [%i][%i] = %12.25f\n", i, j, ppMeasure[i][j]);

  fprintf(pFile, "\n");
}

void CSaTScanData::DisplayNeighbors(FILE* pFile) {
  int                   i, j;
  tract_t            ** ppNeighborCount(gpNeighborCountHandler->GetArray());
  unsigned short    *** pppSortedUShort((gpSortedUShortHandler ? gpSortedUShortHandler->GetArray() : 0));
  tract_t           *** pppSortedInt((gpSortedIntHandler ? gpSortedIntHandler->GetArray() : 0));

  fprintf(pFile, "Neighbors (m_pSorted)   m_nGridTracts=%i\n\n", m_nGridTracts);

  for (i=0; i < m_nGridTracts; ++i) {
    fprintf(pFile, "Grid Point # %i : ", i);
    if (pppSortedInt)
       for (j=0; j < ppNeighborCount[0][i]; ++j)
         fprintf(pFile, "%s ", gTractHandler->getLocations().at(pppSortedInt[0][i][j])->getIndentifier());
    else
       for (j=0; j < ppNeighborCount[0][i]; ++j)
         fprintf(pFile, "%s ", gTractHandler->getLocations().at(pppSortedInt[0][i][j])->getIndentifier());
    fprintf(pFile, "(# of neighbors=%i)\n", ppNeighborCount[0][i]);
  }

  fprintf(pFile,"\n");
}

/** Prints summary of section to results file - detailing input data. */
void CSaTScanData::DisplaySummary(FILE* fp, std::string sSummaryText, bool bPrintPeriod) {
  std::string           buffer, work, label;
  AsciiPrintFormat      PrintFormat(gDataSets->GetNumDataSets() == 1);
  unsigned int          i;

  PrintFormat.SetMarginsAsSummarySection();
  PrintFormat.PrintSectionSeparatorString(fp, 0, 2);
  fprintf(fp, "%s\n\n", sSummaryText.c_str());
  //print study period
  if (bPrintPeriod) {
    PrintFormat.PrintSectionLabel(fp, "Study period", false, false);
    fprintf(fp,"%s - %s\n", gParameters.GetStudyPeriodStartDate().c_str(), gParameters.GetStudyPeriodEndDate().c_str());
  }  
  if (gParameters.UseCoordinatesFile() || gParameters.UseLocationNeighborsFile()) {
    //print number locations scanned
    PrintFormat.PrintSectionLabel(fp, "Number of locations", false, false);
    fprintf(fp, "%ld\n", (long)m_nTracts + GetNumMetaTractsReferenced() - GetNumNullifiedLocations());
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
    case EXPONENTIAL : printString(buffer, "%.0f", gDataSets->GetDataSet(0).getTotalPopulation());
                       for (i=1; i < gDataSets->GetNumDataSets(); ++i) {
                         printString(work, ", %.0f", gDataSets->GetDataSet(i).getTotalPopulation());
                         buffer += work;
                       }
                       PrintFormat.PrintAlignedMarginsDataString(fp, buffer);
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
                                printString(buffer, "%ld", gDataSets->GetDataSet(0).getTotalCases());
                                for (i=1; i < gDataSets->GetNumDataSets(); ++i) {
                                  printString(work, ", %ld", gDataSets->GetDataSet(i).getTotalCases());
                                  buffer += work;
                                }
                                PrintFormat.PrintAlignedMarginsDataString(fp, buffer);
                                break;
    default                   : break;
  }
  //for the ordinal probability model, also print category values and total cases per ordinal category
  if (gParameters.GetProbabilityModelType() == ORDINAL) {
    if (gDataSets->GetNumDataSets() == 1) {
      PrintFormat.PrintSectionLabel(fp, "Category values", false, false);
      const PopulationData& Population = gDataSets->GetDataSet().getPopulationData();
      buffer="";
      for (size_t j=0; j < Population.GetNumOrdinalCategories(); ++j) {
         printString(work, "%s%g", (j ? ", " : ""), Population.GetOrdinalCategoryValue(j));
         buffer += work;
      }
      PrintFormat.PrintAlignedMarginsDataString(fp, buffer);
      PrintFormat.PrintSectionLabel(fp, "Total cases per category", false, false);
      buffer="";
      for (size_t j=0; j < Population.GetNumOrdinalCategories(); ++j) {
         printString(work, "%s%ld", (j ? ", " : ""), Population.GetNumOrdinalCategoryCases(j));
         buffer += work;
      }
      PrintFormat.PrintAlignedMarginsDataString(fp, buffer);
    }
    else {
      for (i=0, buffer=""; i < gDataSets->GetNumDataSets(); ++i, buffer="") {
        printString(label, "Category values, data set #%d", i + 1);
        PrintFormat.PrintSectionLabel(fp, label.c_str(), false, false);
        const PopulationData& Population = gDataSets->GetDataSet(i).getPopulationData();
        for (size_t j=0; j < Population.GetNumOrdinalCategories(); ++j) {
           printString(work, "%s%g", (j ? ", " : ""), Population.GetOrdinalCategoryValue(j));
           buffer += work;
        }
        PrintFormat.PrintAlignedMarginsDataString(fp, buffer);
        printString(label, "Total category cases, data set #%d ", i + 1);
        PrintFormat.PrintSectionLabel(fp, label.c_str(), false, false);
        buffer="";
        for (size_t j=0; j < Population.GetNumOrdinalCategories(); ++j) {
           printString(work, "%s%ld", (j ? ", " : ""), Population.GetNumOrdinalCategoryCases(j));
           buffer += work;
        }
        PrintFormat.PrintAlignedMarginsDataString(fp, buffer);
      }
    }
  }
  //for the exponential probability model, also print total censored cases
  if (gParameters.GetProbabilityModelType() == EXPONENTIAL) {
    PrintFormat.PrintSectionLabel(fp, "Total censored", true, false);
    printString(buffer, "%.0f", gDataSets->GetDataSet(0).getTotalPopulation() - gDataSets->GetDataSet(0).getTotalCases());
    for (i=1; i < gDataSets->GetNumDataSets(); ++i) {
       printString(work, ", %.0f", gDataSets->GetDataSet(i).getTotalPopulation() - gDataSets->GetDataSet(i).getTotalCases());
       buffer += work;
    }
    PrintFormat.PrintAlignedMarginsDataString(fp, buffer);
  }
  //for the Poisson probability model, also print annual cases per population
  if (gParameters.GetProbabilityModelType() == POISSON && gParameters.UsePopulationFile()) {
    printString(buffer, "Annual cases / %.0f",  GetAnnualRatePop());
    PrintFormat.PrintSectionLabel(fp, buffer.c_str(), true, false);
    printString(buffer, "%.1f", GetAnnualRate(0));
    for (i=1; i < gDataSets->GetNumDataSets(); ++i) {
       printString(work, ", %.1f", GetAnnualRate(i));
       buffer += work;
    }
    PrintFormat.PrintAlignedMarginsDataString(fp, buffer);
  }
  if (gParameters.GetProbabilityModelType() == NORMAL) {
    PrintFormat.PrintSectionLabel(fp, "Mean", true, false);
    printString(buffer, "%.2f", gDataSets->GetDataSet(0).getTotalMeasure()/gDataSets->GetDataSet(0).getTotalCases());
    for (i=1; i < gDataSets->GetNumDataSets(); ++i) {
       printString(work, ", %.2f", gDataSets->GetDataSet(i).getTotalMeasure()/gDataSets->GetDataSet(i).getTotalCases());
       buffer += work;
    }
    PrintFormat.PrintAlignedMarginsDataString(fp, buffer);
    PrintFormat.PrintSectionLabel(fp, "Variance", true, false);
    printString(buffer, "%.2f", GetUnbiasedVariance(gDataSets->GetDataSet(0).getTotalCases(), gDataSets->GetDataSet(0).getTotalMeasure(), gDataSets->GetDataSet(0).getTotalMeasureSq()));
    for (i=1; i < gDataSets->GetNumDataSets(); ++i) {
       printString(work, ", %.2f", GetUnbiasedVariance(gDataSets->GetDataSet(i).getTotalCases(), gDataSets->GetDataSet(i).getTotalMeasure(), gDataSets->GetDataSet(i).getTotalMeasureSq()));
       buffer += work;
    }
    PrintFormat.PrintAlignedMarginsDataString(fp, buffer);
    PrintFormat.PrintSectionLabel(fp, "Standard deviation", true, false);
    printString(buffer, "%.2f", std::sqrt(GetUnbiasedVariance(gDataSets->GetDataSet(0).getTotalCases(), gDataSets->GetDataSet(0).getTotalMeasure(), gDataSets->GetDataSet(0).getTotalMeasureSq())));
    for (i=1; i < gDataSets->GetNumDataSets(); ++i) {
       printString(work, ", %.2f", std::sqrt(GetUnbiasedVariance(gDataSets->GetDataSet(i).getTotalCases(), gDataSets->GetDataSet(i).getTotalMeasure(), gDataSets->GetDataSet(i).getTotalMeasureSq())));
       buffer += work;
    }
    PrintFormat.PrintAlignedMarginsDataString(fp, buffer);
  }
  if (gParameters.GetAnalysisType() == SPATIALVARTEMPTREND) {
    double nAnnualTT = gDataSets->GetDataSet(0/*for now*/).getTimeTrend().SetAnnualTimeTrend(gParameters.GetTimeAggregationUnitsType(), gParameters.GetTimeAggregationLength());
    if (gDataSets->GetDataSet(0/*for now*/).getTimeTrend().IsNegative())
      buffer = "Annual decrease";
    else
      buffer = "Annual increase";
    PrintFormat.PrintSectionLabel(fp, buffer.c_str(), false, false);
    fprintf(fp, "%.3lf%%\n", nAnnualTT);
  }
  PrintFormat.PrintSectionSeparatorString(fp, 0, 1);
}

// formats the information necessary in the relative risk output file and prints to the specified format
// pre: none
// post: prints the relative risk data to the output file
void CSaTScanData::DisplayRelativeRisksForEachTract() const {
  try {
    LocationRiskEstimateWriter(*this).Write(*this);
  }
  catch (prg_exception& x) {
    x.addTrace("DisplayRelativeRisksForEachTract()", "CSaTScanData");
    throw;
  }
}

