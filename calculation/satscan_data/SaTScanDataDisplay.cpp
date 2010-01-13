//---------------------------------------------------------------------------
#include "SaTScan.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#include <vector>
#include "SaTScanData.h"
#include "LocationRiskEstimateWriter.h"
#include "AsciiPrintFormat.h"
#include "SSException.h"
#include "WeightedNormalRandomizer.h"
#include "HomogeneousPoissonDataSetHandler.h"

/** Debug utility function - prints case counts for all datasets. Caller is
    responsible for ensuring that passed file pointer points to valid, open file
    handle. */
void CSaTScanData::DisplayCases(FILE* pFile) const {
  fprintf(pFile, "Case counts (Cases Array)\n\n");
  for (size_t j=0; j < gDataSets->GetNumDataSets(); ++j) {
     fprintf(pFile, "Data Set %u:\n", j);
     count_t ** ppCases = gDataSets->GetDataSet(j).getCaseData().GetArray();
     for (int i=0; i < m_nTimeIntervals; ++i)
        for (int t=0; t < m_nTracts; ++t)
           fprintf(pFile, "Case [%i][%i] = %i\n", i, j, ppCases[i][t]);
     fprintf(pFile, "\n");
  }
  fprintf(pFile, "\n");
  fflush(pFile);
}

/** Debug utility function - prints control counts for all datasets. Caller is
    responsible for ensuring that passed file pointer points to valid, open file
    handle. */
void CSaTScanData::DisplayControls(FILE* pFile) const {
  fprintf(pFile, "Control counts (Controls Array)\n\n");
  for (size_t j=0; j < gDataSets->GetNumDataSets(); ++j) {
     fprintf(pFile, "Data Set %u:\n", j);
     count_t ** ppControls = gDataSets->GetDataSet(j).getControlData().GetArray();
     for (int i=0; i < m_nTimeIntervals; ++i)
       for (int t=0; t < m_nTracts; ++t)
         fprintf(pFile, "Controls [%i][%i] = %i\n", i, t, ppControls[i][t]);
     fprintf(pFile, "\n");
  }
  fprintf(pFile, "\n");
  fflush(pFile);
}

/** Debug utility function - prints simulation case counts for all datasets. Caller is
    responsible for ensuring that passed file pointer points to valid, open file
    handle. */
void CSaTScanData::DisplaySimCases(SimulationDataContainer_t& Container, FILE* pFile) const {
  fprintf(pFile, "Simulated Case counts (Simulated Cases Array)\n\n");
  for (size_t j=0; j < Container.size(); ++j) {
     fprintf(pFile, "Data Set %u:\n", j);
     count_t ** ppSimCases = Container.at(j)->getCaseData().GetArray();
     for (int i=0; i < m_nTimeIntervals; ++i)
       for (int t=0; t < m_nTracts; ++t)
         fprintf(pFile, "Cases [%i][%i] = %i\n", i, t, ppSimCases[i][t]);
     fprintf(pFile, "\n");
  }
  fprintf(pFile, "\n");
  fflush(pFile);
}

/** Debug utility function - prints expected case counts for all datasets.
    Caller is responsible for ensuring that passed file pointer points to valid,
    open file handle. */
void CSaTScanData::DisplayMeasure(FILE* pFile) const {
  int           i, j;

  fprintf(pFile, "Measures (Measure Array)\n\n");
  for (size_t j=0; j < gDataSets->GetNumDataSets(); ++j) {
     fprintf(pFile, "Data Set %u:\n", j);
     measure_t ** ppMeasure = gDataSets->GetDataSet(j).getMeasureData().GetArray();
     for (int i=0; i < m_nTimeIntervals; ++i)
        for (int t=0; t < m_nTracts; ++t)
          fprintf(pFile, "Measure [%i][%i] = %12.25f\n", i, t, ppMeasure[i][t]);
     fprintf(pFile, "\n");
  }
  fprintf(pFile, "\n");
  fflush(pFile);
}

/** Debug utility function - prints neighbor information.
    Caller is responsible for ensuring that passed file pointer points to valid,
    open file handle. */
void CSaTScanData::DisplayNeighbors(FILE* pFile) const {
  int                   i, j;
  tract_t            ** ppNeighborCount = gpNeighborCountHandler->GetArray();
  unsigned short    *** pppSortedUShort = (gpSortedUShortHandler ? gpSortedUShortHandler->GetArray() : 0);
  tract_t           *** pppSortedInt = (gpSortedIntHandler ? gpSortedIntHandler->GetArray() : 0);

  fprintf(pFile, "Neighbors (m_pSorted)   m_nGridTracts=%i\n\n", m_nGridTracts);

  for (int e=0; e <= gParameters.GetNumTotalEllipses(); ++e) {
    fprintf(pFile, "\n\nEllipse %i: Ellipse Angle %lf Ellipse Shape %lf\n\n", e, GetEllipseAngle(e), GetEllipseShape(e));
    for (i=0; i < m_nGridTracts; ++i) {
        fprintf(pFile, "Grid Point # %i : ", i);
        if (pppSortedInt)
            for (j=0; j < ppNeighborCount[e][i]; ++j)
                fprintf(pFile, "%s ", gTractHandler->getLocations().at(pppSortedInt[e][i][j])->getIndentifier());
        else
            for (j=0; j < ppNeighborCount[e][i]; ++j)
                fprintf(pFile, "%s ", gTractHandler->getLocations().at(pppSortedUShort[e][i][j])->getIndentifier());
        fprintf(pFile, "(# of neighbors=%i)\n", ppNeighborCount[e][i]);
    }
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
  if (bPrintPeriod && gParameters.GetProbabilityModelType() != HOMOGENEOUSPOISSON) {
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
    case CATEGORICAL          :
    case ORDINAL              :
    case NORMAL               :
    case EXPONENTIAL          : 
    case HOMOGENEOUSPOISSON   : PrintFormat.PrintSectionLabel(fp, "Total number of cases", true, false);
                                printString(buffer, "%ld", gDataSets->GetDataSet(0).getTotalCases());
                                for (i=1; i < gDataSets->GetNumDataSets(); ++i) {
                                  printString(work, ", %ld", gDataSets->GetDataSet(i).getTotalCases());
                                  buffer += work;
                                }
                                PrintFormat.PrintAlignedMarginsDataString(fp, buffer);
                                break;
    default                   : break;
  }
  //print total area per data set for Homogeneous Poisson model
  if (gParameters.GetProbabilityModelType() == HOMOGENEOUSPOISSON) {
      const HomogeneousPoissonDataSetHandler * pHandler = dynamic_cast<const HomogeneousPoissonDataSetHandler*>(gDataSets.get());
    if (!pHandler)
      throw prg_error("Could not cast to HomogeneousPoissonDataSetHandler type.","DisplaySummary()");
    PrintFormat.PrintSectionLabel(fp, "Total Area", true, false);
    printString(buffer, "%.0f", pHandler->getTotalArea());
    PrintFormat.PrintAlignedMarginsDataString(fp, buffer);
  }

  //for the ordinal probability model, also print category values and total cases per ordinal category
  if (gParameters.GetProbabilityModelType() == ORDINAL || gParameters.GetProbabilityModelType() == CATEGORICAL) {
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
  if (gParameters.GetProbabilityModelType() == NORMAL && !gParameters.getIsWeightedNormal()) {
    PrintFormat.PrintSectionLabel(fp, "Mean", true, false);
    printString(buffer, "%.2f", gDataSets->GetDataSet(0).getTotalMeasure()/gDataSets->GetDataSet(0).getTotalCases());
    for (i=1; i < gDataSets->GetNumDataSets(); ++i) {
       printString(work, ", %.2f", gDataSets->GetDataSet(i).getTotalMeasure()/gDataSets->GetDataSet(i).getTotalCases());
       buffer += work;
    }
    PrintFormat.PrintAlignedMarginsDataString(fp, buffer);
    PrintFormat.PrintSectionLabel(fp, "Variance", true, false);
    printString(buffer, "%.2f", GetUnbiasedVariance(gDataSets->GetDataSet(0).getTotalCases(), gDataSets->GetDataSet(0).getTotalMeasure(), gDataSets->GetDataSet(0).getTotalMeasureAux()));
    for (i=1; i < gDataSets->GetNumDataSets(); ++i) {
       printString(work, ", %.2f", GetUnbiasedVariance(gDataSets->GetDataSet(i).getTotalCases(), gDataSets->GetDataSet(i).getTotalMeasure(), gDataSets->GetDataSet(i).getTotalMeasureAux()));
       buffer += work;
    }
    PrintFormat.PrintAlignedMarginsDataString(fp, buffer);
    PrintFormat.PrintSectionLabel(fp, "Standard deviation", true, false);
    printString(buffer, "%.2f", std::sqrt(GetUnbiasedVariance(gDataSets->GetDataSet(0).getTotalCases(), gDataSets->GetDataSet(0).getTotalMeasure(), gDataSets->GetDataSet(0).getTotalMeasureAux())));
    for (i=1; i < gDataSets->GetNumDataSets(); ++i) {
       printString(work, ", %.2f", std::sqrt(GetUnbiasedVariance(gDataSets->GetDataSet(i).getTotalCases(), gDataSets->GetDataSet(i).getTotalMeasure(), gDataSets->GetDataSet(i).getTotalMeasureAux())));
       buffer += work;
    }
    PrintFormat.PrintAlignedMarginsDataString(fp, buffer);
  }
  if (gParameters.GetProbabilityModelType() == NORMAL && gParameters.getIsWeightedNormal()) {
    AbstractWeightedNormalRandomizer *pRandomizer;
    std::vector<AbstractWeightedNormalRandomizer::DataSetStatistics> dataSetStatistics;

    //Check that all randomizers are derived from AbstractWeightedNormalRandomizer class.
    for (i=0; i < gDataSets->GetNumDataSets(); ++i) {
      if ((pRandomizer = dynamic_cast<AbstractWeightedNormalRandomizer*>(gDataSets->GetRandomizer(i))) == 0)
        throw prg_error("Randomizer could not be dynamically casted to AbstractWeightedNormalRandomizer type.\n", "DisplaySummary()");
       dataSetStatistics.push_back(pRandomizer->getDataSetStatistics());
    }

    //Print total weight for all data sets.
    PrintFormat.PrintSectionLabel(fp, "Total weights", true, false);
    printString(buffer, "%.2f", dataSetStatistics.front().gtTotalWeight);
    for (i=1; i < dataSetStatistics.size(); ++i) {
       printString(work, ", %.2f", dataSetStatistics[i].gtTotalWeight);
       buffer += work;
    }
    PrintFormat.PrintAlignedMarginsDataString(fp, buffer);

    //Print mean for all data sets.
    PrintFormat.PrintSectionLabel(fp, "Mean", true, false);
    printString(buffer, "%.2f", dataSetStatistics.front().gtMean);
    for (i=1; i < dataSetStatistics.size(); ++i) {
       printString(work, ", %.2f", dataSetStatistics[i].gtMean);
       buffer += work;
    }
    PrintFormat.PrintAlignedMarginsDataString(fp, buffer);

    //Print variance for all data sets.
    PrintFormat.PrintSectionLabel(fp, "Variance", true, false);
    printString(buffer, "%.2f", dataSetStatistics.front().gtVariance);
    for (i=1; i < dataSetStatistics.size(); ++i) {
       printString(work, ", %.2f", dataSetStatistics[i].gtVariance);
       buffer += work;
    }
    PrintFormat.PrintAlignedMarginsDataString(fp, buffer);

    //Print standard deviation for all data sets.
    PrintFormat.PrintSectionLabel(fp, "Standard deviation", true, false);
    printString(buffer, "%.2f", std::sqrt(dataSetStatistics.front().gtVariance));
    for (i=1; i < dataSetStatistics.size(); ++i) {
       printString(work, ", %.2f", std::sqrt(dataSetStatistics[i].gtVariance));
       buffer += work;
    }
    PrintFormat.PrintAlignedMarginsDataString(fp, buffer);

    //Print weighted mean for all data sets.
    PrintFormat.PrintSectionLabel(fp, "Weighted Mean", true, false);
    printString(buffer, "%.2f", dataSetStatistics.front().gtWeightedMean);
    for (i=1; i < dataSetStatistics.size(); ++i) {
       printString(work, ", %.2f", dataSetStatistics[i].gtWeightedMean);
       buffer += work;
    }
    PrintFormat.PrintAlignedMarginsDataString(fp, buffer);

    //Print weighted variance for all data sets.
    PrintFormat.PrintSectionLabel(fp, "Weighted Variance", true, false);
    printString(buffer, "%.2f", dataSetStatistics.front().gtWeightedVariance);
    for (i=1; i < dataSetStatistics.size(); ++i) {
        printString(work, ", %.2f", dataSetStatistics[i].gtWeightedVariance);
       buffer += work;
    }
    PrintFormat.PrintAlignedMarginsDataString(fp, buffer);

    //Print weighted variance for all data sets.
    PrintFormat.PrintSectionLabel(fp, "Weighted Std deviation", true, false);
    printString(buffer, "%.2f", std::sqrt(dataSetStatistics.front().gtWeightedVariance));
    for (i=1; i < dataSetStatistics.size(); ++i) {
       printString(work, ", %.2f", std::sqrt(dataSetStatistics[i].gtWeightedVariance));
       buffer += work;
    }
    PrintFormat.PrintAlignedMarginsDataString(fp, buffer);
  }
  if (gParameters.GetAnalysisType() == SPATIALVARTEMPTREND) {
    double nAnnualTT = gDataSets->GetDataSet(0/*for now*/).getTimeTrend().SetAnnualTimeTrend(gParameters.GetTimeAggregationUnitsType(), gParameters.GetTimeAggregationLength());
    if (nAnnualTT < 0)
      buffer = "Annual decrease";
    else
      buffer = "Annual increase";
    PrintFormat.PrintSectionLabel(fp, buffer.c_str(), false, false);
    fprintf(fp, "%.3lf%%\n", fabs(nAnnualTT));

    if (gParameters.getTimeTrendType() == QUADRATIC) {
      // TODO: These statements are for testing and will be removed eventually.  
      PrintFormat.PrintSectionLabel(fp, "alpha Global", false, false);
      fprintf(fp, "%g\n", gDataSets->GetDataSet(0/*for now*/).getTimeTrend().GetAlpha());
      PrintFormat.PrintSectionLabel(fp, "beta1 Global", false, false);
      fprintf(fp, "%g\n", gDataSets->GetDataSet(0/*for now*/).getTimeTrend().GetBeta());
      PrintFormat.PrintSectionLabel(fp, "beta2 Global", false, false);
      fprintf(fp, "%g\n", ((const QuadraticTimeTrend&)(gDataSets->GetDataSet(0/*for now*/).getTimeTrend())).GetBeta2());
    }
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

