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
#include "LocationRelevance.h"

/** Debug utility function - prints case counts for all datasets. Assume case arrays are currently cumulative.
    Caller is responsible for ensuring that passed file pointer points to valid open file handle. */
void CSaTScanData::DisplayCases(FILE* pFile, bool nonCumulative) const {
  fprintf(pFile, "%s Case counts (Cases Array)\n\n", nonCumulative ? "Non-Cumulative" : "Cumulative");
  for (size_t j=0; j < gDataSets->GetNumDataSets(); ++j) {
     fprintf(pFile, "Data Set %u:\n", j);
     count_t ** ppCases = gDataSets->GetDataSet(j).getCaseData().GetArray();
     for (int i=0; i < GetNumTimeIntervals(); ++i)
        for (int t=0; t < _num_identifiers; ++t)
           fprintf(pFile, "%i,%i,%li\n", i, t, ppCases[i][t] - (nonCumulative && (i + 1) < GetNumTimeIntervals() ? ppCases[i + 1][t] : 0));
     fprintf(pFile, "\n");
  }
  fprintf(pFile, "\n");
  fflush(pFile);
}

/** Debug utility function - prints control counts for all datasets. Assume case arrays are currently cumulative.
    Caller is responsible for ensuring that passed file pointer points to valid open file handle. */
void CSaTScanData::DisplayControls(FILE* pFile, bool nonCumulative) const {
  fprintf(pFile, "%s Control counts (Controls Array)\n\n", nonCumulative ? "Non-Cumulative" : "Cumulative");
  for (size_t j=0; j < gDataSets->GetNumDataSets(); ++j) {
     fprintf(pFile, "Data Set %u:\n", j);
     count_t ** ppControls = gDataSets->GetDataSet(j).getControlData().GetArray();
     for (int i=0; i < GetNumTimeIntervals(); ++i)
       for (int t=0; t < _num_identifiers; ++t)
         fprintf(pFile, "%i,%i,%li\n", i, t, ppControls[i][t] - (nonCumulative && (i + 1) < GetNumTimeIntervals() ? ppControls[i + 1][t] : 0));
     fprintf(pFile, "\n");
  }
  fprintf(pFile, "\n");
  fflush(pFile);
}

/** Debug utility function - prints simulation case counts for all datasets. Assume case arrays are currently cumulative.
    Caller is responsible for ensuring that passed file pointer points to valid open file handle. */
void CSaTScanData::DisplaySimCases(SimulationDataContainer_t& Container, FILE* pFile, bool nonCumulative) const {
  fprintf(pFile, "%s Simulated Case counts (Simulated Cases Array)\n\n", nonCumulative ? "Non-Cumulative" : "Cumulative");
  for (size_t j=0; j < Container.size(); ++j) {
     fprintf(pFile, "Data Set %u:\n", j);
     count_t ** ppSimCases = Container.at(j)->getCaseData().GetArray();
     for (int i=0; i < GetNumTimeIntervals(); ++i)
       for (int t=0; t < _num_identifiers; ++t)
         fprintf(pFile, "%i,%i,%li\n", i, t, ppSimCases[i][t] - (nonCumulative && (i + 1) < GetNumTimeIntervals() ? ppSimCases[i + 1][t] : 0));
     fprintf(pFile, "\n");
  }
  fprintf(pFile, "\n");
  fflush(pFile);
}

/** Debug utility function - prints expected case counts for all datasets.  Assume case arrays are currently cumulative.
    Caller is responsible for ensuring that passed file pointer points to valid open file handle. */
void CSaTScanData::DisplayMeasure(FILE* pFile, bool nonCumulative) const {
    std::string dateString;
    fprintf(pFile, "%s Measures (Measure Array)\n\n", nonCumulative ? "Non-Cumulative" : "Cumulative");
    for (size_t j = 0; j < gDataSets->GetNumDataSets(); ++j) {
        fprintf(pFile, "Data Set %u:\n", j);
        fprintf(pFile, "Location,IntervalIdx,Measure,population\n", j);
        measure_t ** ppMeasure = gDataSets->GetDataSet(j).getMeasureData().GetArray();
        PopulationData & population = gDataSets->GetDataSet(j).getPopulationData();
        for (int t = 0; t < _num_identifiers; ++t) {
   	        const char * locationId = _identifiers_manager->getIdentifiers().at(t)->getLocations()[0]->name().c_str();
            //const char * locationId = this->GetTInfo()->getIdentifier(t);
            for (int i = 0; i < GetNumTimeIntervals(); ++i) {
                Julian date = GetTimeIntervalStartTimes()[i];
                int dateIdx = population.GetPopulationDateIndex(date, true);
                double pop = 0.0; // population.GetPopulation(t, 0, dateIdx);
                JulianToString(dateString, date, gParameters.GetPrecisionOfTimesType());
                fprintf(
                    pFile, "%s (t=%d),%s (i=%d),%g,%g\n", locationId, t,
                    dateString.c_str(), i, ppMeasure[i][t] - (nonCumulative && (i + 1) < GetNumTimeIntervals() ? ppMeasure[i + 1][t] : 0), pop
                );
            }
        }
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
                fprintf(pFile, "%s ", _identifiers_manager->getIdentifiers().at(pppSortedInt[e][i][j])->name().c_str());
        else
            for (j=0; j < ppNeighborCount[e][i]; ++j)
                fprintf(pFile, "%s ", _identifiers_manager->getIdentifiers().at(pppSortedUShort[e][i][j])->name().c_str());
        fprintf(pFile, "(# of neighbors=%i)\n", ppNeighborCount[e][i]);
    }
  }

  fprintf(pFile,"\n");
}

/** Prints summary of section to results file - detailing input data. */
void CSaTScanData::DisplaySummary(FILE* fp, std::string sSummaryText, bool bPrintPeriod) {
  std::string           buffer, work, work2, label;
  AsciiPrintFormat      PrintFormat(gDataSets->GetNumDataSets() == 1);
  unsigned int          i;

  PrintFormat.SetMarginsAsSummarySection();
  PrintFormat.PrintSectionSeparatorString(fp, 0, 2);
  fprintf(fp, "%s\n\n", sSummaryText.c_str());
  //print study period
  if (bPrintPeriod && gParameters.GetProbabilityModelType() != HOMOGENEOUSPOISSON) {
    PrintFormat.PrintSectionLabel(fp, "Study period", false, false);
    fprintf(fp,"%s to %s\n", gParameters.GetStudyPeriodStartDate().c_str(), gParameters.GetStudyPeriodEndDate().c_str());
  }  
  if (!gParameters.GetIsPurelyTemporalAnalysis() && (gParameters.UseCoordinatesFile() || gParameters.UseLocationNeighborsFile())) {
    //print number locations scanned
      if (gParameters.GetMultipleCoordinatesType() == ONEPERLOCATION) {
          PrintFormat.PrintSectionLabel(fp, "Number of locations", false, false);
          fprintf(fp, "%ld\n", (long)_num_identifiers + GetNumMetaIdentifiersReferenced() - GetNumNullifiedIdentifiers());
      } else {
          PrintFormat.PrintSectionLabel(fp, "Number of locations", false, false);
          fprintf(fp, "%u\n", getLocationsManager().locations().size());
          PrintFormat.PrintSectionLabel(fp, "Number of identifiers", false, false);
          fprintf(fp, "%u\n", getIdentifierInfo().getIdentifiers().size() + getIdentifierInfo().getAggregated().size());
      }
  }  
  //print total population per data set
  switch (gParameters.GetProbabilityModelType()) {
    //label for data is dependent on probability model
    case POISSON     : if (!gParameters.UsePopulationFile()) break;
                       PrintFormat.PrintSectionLabel(fp, "Population, averaged over time", true, false); break;
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
  if (gParameters.GetProbabilityModelType() == BERNOULLI) {
    PrintFormat.PrintSectionLabel(fp, "Percent cases in area", true, false);
    getValueAsString(100.0 * gDataSets->GetDataSet(0).getTotalCases() / gDataSets->GetDataSet(0).getTotalPopulation(), buffer, 1);
    for (i=1; i < gDataSets->GetNumDataSets(); ++i) {
        getValueAsString(100.0 * gDataSets->GetDataSet(i).getTotalCases() / gDataSets->GetDataSet(i).getTotalPopulation(), work2, 1);
        printString(work, ", %s", work2.c_str());
        buffer += work;
    }
    PrintFormat.PrintAlignedMarginsDataString(fp, buffer);
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
         printString(work, "%s%s", (j ? ", " : ""), Population.GetCategoryTypeLabel(j).c_str());
         buffer += work;
      }
      PrintFormat.PrintAlignedMarginsDataString(fp, buffer);
      PrintFormat.PrintSectionLabel(fp, "Total cases per category", false, false);
      buffer="";
      for (size_t j=0; j < Population.GetNumOrdinalCategories(); ++j) {
         printString(work, "%s%ld", (j ? ", " : ""), Population.GetNumCategoryTypeCases(j));
         buffer += work;
      }
      PrintFormat.PrintAlignedMarginsDataString(fp, buffer);

      PrintFormat.PrintSectionLabel(fp, "Percent cases per category", false, false);
      buffer="";
      for (size_t j=0; j < Population.GetNumOrdinalCategories(); ++j) {
         getValueAsString(100.0 * Population.GetNumCategoryTypeCases(j) / gDataSets->GetDataSet(0).getTotalCases(), work2, 1);
         printString(work, "%s%s", (j ? ", " : ""), work2.c_str());
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
           printString(work, "%s%s", (j ? ", " : ""), Population.GetCategoryTypeLabel(j).c_str());
           buffer += work;
        }
        PrintFormat.PrintAlignedMarginsDataString(fp, buffer);

        printString(label, "Total category cases, data set #%d ", i + 1);
        PrintFormat.PrintSectionLabel(fp, label.c_str(), false, false);
        buffer="";
        for (size_t j=0; j < Population.GetNumOrdinalCategories(); ++j) {
           printString(work, "%s%ld", (j ? ", " : ""), Population.GetNumCategoryTypeCases(j));
           buffer += work;
        }
        PrintFormat.PrintAlignedMarginsDataString(fp, buffer);

        printString(label, "Percent cases in area, data set #%d ", i + 1);
        PrintFormat.PrintSectionLabel(fp, label.c_str(), false, false);
        buffer="";
        for (size_t j=0; j < Population.GetNumOrdinalCategories(); ++j) {
           getValueAsString(100.0 * Population.GetNumCategoryTypeCases(j) / gDataSets->GetDataSet(i).getTotalCases(), work2, 1);
           printString(work, "%s%s", (j ? ", " : ""), work2.c_str());
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
    getValueAsString(GetAnnualRate(0), buffer, 1);
    for (i=1; i < gDataSets->GetNumDataSets(); ++i) {
       getValueAsString(GetAnnualRate(i), work, 1);
       buffer += ", "; buffer += work;
    }
    PrintFormat.PrintAlignedMarginsDataString(fp, buffer);
  }
  if (gParameters.GetProbabilityModelType() == NORMAL && !gParameters.getIsWeightedNormal()) {
    PrintFormat.PrintSectionLabel(fp, "Mean", true, false);
    getValueAsString(gDataSets->GetDataSet(0).getTotalMeasure()/gDataSets->GetDataSet(0).getTotalCases(), buffer);
    for (i=1; i < gDataSets->GetNumDataSets(); ++i) {
       getValueAsString(gDataSets->GetDataSet(i).getTotalMeasure()/gDataSets->GetDataSet(i).getTotalCases(), work);
       buffer += ", "; buffer += work;
    }
    PrintFormat.PrintAlignedMarginsDataString(fp, buffer);
    PrintFormat.PrintSectionLabel(fp, "Variance", true, false);
    getValueAsString(GetUnbiasedVariance(gDataSets->GetDataSet(0).getTotalCases(), gDataSets->GetDataSet(0).getTotalMeasure(), gDataSets->GetDataSet(0).getTotalMeasureAux()), buffer);
    for (i=1; i < gDataSets->GetNumDataSets(); ++i) {
       getValueAsString(GetUnbiasedVariance(gDataSets->GetDataSet(i).getTotalCases(), gDataSets->GetDataSet(i).getTotalMeasure(), gDataSets->GetDataSet(i).getTotalMeasureAux()), work);
       buffer += ", "; buffer += work;
    }
    PrintFormat.PrintAlignedMarginsDataString(fp, buffer);
    PrintFormat.PrintSectionLabel(fp, "Standard deviation", true, false);
    getValueAsString(std::sqrt(GetUnbiasedVariance(gDataSets->GetDataSet(0).getTotalCases(), gDataSets->GetDataSet(0).getTotalMeasure(), gDataSets->GetDataSet(0).getTotalMeasureAux())), buffer);
    for (i=1; i < gDataSets->GetNumDataSets(); ++i) {
       getValueAsString(std::sqrt(GetUnbiasedVariance(gDataSets->GetDataSet(i).getTotalCases(), gDataSets->GetDataSet(i).getTotalMeasure(), gDataSets->GetDataSet(i).getTotalMeasureAux())), work);
       buffer += ", "; buffer += work;
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
    getValueAsString(dataSetStatistics.front().gtTotalWeight, buffer);
    for (i=1; i < dataSetStatistics.size(); ++i) {
       getValueAsString(dataSetStatistics[i].gtTotalWeight, work);
       buffer += ", "; buffer += work;
    }
    PrintFormat.PrintAlignedMarginsDataString(fp, buffer);

    //Print mean for all data sets.
    PrintFormat.PrintSectionLabel(fp, "Mean", true, false);
    getValueAsString(dataSetStatistics.front().gtMean, buffer);
    for (i=1; i < dataSetStatistics.size(); ++i) {
       getValueAsString(dataSetStatistics[i].gtMean, work);
       buffer += ", "; buffer += work;
    }
    PrintFormat.PrintAlignedMarginsDataString(fp, buffer);

    //Print variance for all data sets.
    PrintFormat.PrintSectionLabel(fp, "Variance", true, false);
    getValueAsString(dataSetStatistics.front().gtVariance, buffer);
    for (i=1; i < dataSetStatistics.size(); ++i) {
       getValueAsString(dataSetStatistics[i].gtVariance, work);
       buffer += ", "; buffer += work;
    }
    PrintFormat.PrintAlignedMarginsDataString(fp, buffer);

    //Print standard deviation for all data sets.
    PrintFormat.PrintSectionLabel(fp, "Standard deviation", true, false);
    getValueAsString(std::sqrt(dataSetStatistics.front().gtVariance), buffer);
    for (i=1; i < dataSetStatistics.size(); ++i) {
       getValueAsString(std::sqrt(dataSetStatistics[i].gtVariance), work);
       buffer += ", "; buffer += work;
    }
    PrintFormat.PrintAlignedMarginsDataString(fp, buffer);

    //Print weighted mean for all data sets.
    PrintFormat.PrintSectionLabel(fp, "Weighted Mean", true, false);
    getValueAsString(dataSetStatistics.front().gtWeightedMean, buffer);
    for (i=1; i < dataSetStatistics.size(); ++i) {
       getValueAsString(dataSetStatistics[i].gtWeightedMean, work);
       buffer += ", "; buffer += work;
    }
    PrintFormat.PrintAlignedMarginsDataString(fp, buffer);

    //Print weighted variance for all data sets.
    PrintFormat.PrintSectionLabel(fp, "Weighted Variance", true, false);
    getValueAsString(dataSetStatistics.front().gtWeightedVariance, buffer);
    for (i=1; i < dataSetStatistics.size(); ++i) {
       getValueAsString(dataSetStatistics[i].gtWeightedVariance, work);
       buffer += ", "; buffer += work;
    }
    PrintFormat.PrintAlignedMarginsDataString(fp, buffer);

    //Print weighted variance for all data sets.
    PrintFormat.PrintSectionLabel(fp, "Weighted Std deviation", true, false);
    getValueAsString(std::sqrt(dataSetStatistics.front().gtWeightedVariance), buffer);
    for (i=1; i < dataSetStatistics.size(); ++i) {
       getValueAsString(std::sqrt(dataSetStatistics[i].gtWeightedVariance), work);
       buffer += ", "; buffer += work;
    }
    PrintFormat.PrintAlignedMarginsDataString(fp, buffer);
  }
  if (gParameters.GetAnalysisType() == SPATIALVARTEMPTREND) {
      const AbstractTimeTrend& globalTrend = gDataSets->GetDataSet(0/*for now*/).getTimeTrend();
      if (gParameters.getTimeTrendType() == LINEAR) {
         double nAnnualTT = const_cast<AbstractTimeTrend&>(globalTrend).SetAnnualTimeTrend(gParameters.GetTimeAggregationUnitsType(), gParameters.GetTimeAggregationLength());
         PrintFormat.PrintSectionLabel(fp, "Time trend", false, false);
         buffer = (nAnnualTT < 0 ? "annual decrease" : "annual increase");
         getValueAsString(fabs(nAnnualTT), work, 3);
         fprintf(fp, "%s%% %s\n", work.c_str(), buffer.c_str());
      }
      //PrintFormat.PrintSectionLabel(fp, "Global Intercept", false, false);
      //fprintf(fp, "%g\n", globalTrend.GetAlpha());
      //PrintFormat.PrintSectionLabel(fp, "Global Linear", false, false);
      //fprintf(fp, "%g\n", globalTrend.GetBeta());
      const QuadraticTimeTrend * pQTrend = dynamic_cast<const QuadraticTimeTrend *>(&globalTrend);
      if (pQTrend) {
        //PrintFormat.PrintSectionLabel(fp, "Global Quadratic", false, false);
        //fprintf(fp, "%g\n", pQTrend->GetBeta2());
        PrintFormat.PrintSectionLabel(fp, "Global Risk Function", false, true);
        pQTrend->getRiskFunction(buffer, work, *this);
        PrintFormat.PrintNonRightMarginedDataString(fp, buffer, false);
        PrintFormat.PrintNonRightMarginedDataString(fp, work, true);
      }
  }
  if (gParameters.GetProbabilityModelType() == RANK) {
      std::string buffer("");
      PrintFormat.PrintSectionLabel(fp, "Average Rank", true, false);
      PrintFormat.PrintAlignedMarginsDataString(fp, buffer);
      PrintFormat.PrintSectionLabel(fp, "Mean Rank", true, false);
      PrintFormat.PrintAlignedMarginsDataString(fp, buffer);
      PrintFormat.PrintSectionLabel(fp, "Average Category", true, false);
      PrintFormat.PrintAlignedMarginsDataString(fp, buffer);
      PrintFormat.PrintSectionLabel(fp, "Variance", true, false);
      PrintFormat.PrintAlignedMarginsDataString(fp, buffer);
      PrintFormat.PrintSectionLabel(fp, "Standard deviation", true, false);
      PrintFormat.PrintAlignedMarginsDataString(fp, buffer);
  }

  PrintFormat.PrintSectionSeparatorString(fp, 0, 1);
}

// formats the information necessary in the relative risk output file and prints to the specified format
// pre: none
// post: prints the relative risk data to the output file
void CSaTScanData::DisplayRelativeRisksForEachTract(const LocationRelevance& location_relevance) const {
  try {
    LocationRiskEstimateWriter(*this).Write(*this, location_relevance);
  }
  catch (prg_exception& x) {
    x.addTrace("DisplayRelativeRisksForEachTract()", "CSaTScanData");
    throw;
  }
}

