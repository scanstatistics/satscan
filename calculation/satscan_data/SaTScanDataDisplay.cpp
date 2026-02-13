//---------------------------------------------------------------------------
#include "SaTScan.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#include <inttypes.h>
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
      
     fprintf(pFile, "Data Set %" PRId64 "\n", j);
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
     fprintf(pFile, "Data Set %" PRId64 "\n", j);
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
     fprintf(pFile, "Data Set %" PRId64 "\n", j);
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
        fprintf(pFile, "Data Set %" PRId64 "\n", j);
        fprintf(pFile, "Location,IntervalIdx,Measure,population\n");
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

/** Collects summary lines for results file - detailing input data. */
CSaTScanData::SummaryPairs_t& CSaTScanData::getSummaryPairs(std::vector<std::pair<std::string, std::string>>& summaryEntries, bool bPrintPeriod) const {
    std::string buffer, work, work2, suffix;
    AsciiPrintFormat PrintFormat(gDataSets->GetNumDataSets() == 1);

    suffix = PrintFormat.getSectionSuffix(buffer, gParameters.getIsBernoulliIterativeDrilldownAsDOW());
    //print study period
    if (bPrintPeriod && gParameters.GetProbabilityModelType() != HOMOGENEOUSPOISSON) {
        summaryEntries.emplace_back("Study period", printString(buffer, "%s to %s",
            gParameters.GetStudyPeriodStartDate().c_str(), gParameters.GetStudyPeriodEndDate().c_str())
        );
    }
    if (!gParameters.GetIsPurelyTemporalAnalysis() && (gParameters.UseCoordinatesFile() || gParameters.UseLocationNeighborsFile() || gParameters.getUseLocationsNetworkFile())) {
        //print number locations scanned
        if (gParameters.GetMultipleCoordinatesType() == ONEPERLOCATION) {
            summaryEntries.emplace_back("Locations",
                std::to_string(static_cast<unsigned int>(_num_identifiers + GetNumMetaIdentifiersReferenced() - GetNumNullifiedIdentifiers()))
            );
        } else {
            summaryEntries.emplace_back("Locations", std::to_string(getLocationsManager().locations().size()));
            summaryEntries.emplace_back("Identifiers", std::to_string(getIdentifierInfo().getIdentifiers().size() + getIdentifierInfo().getAggregated().size()));
        }
    }
    //print total population per data set
    switch (gParameters.GetProbabilityModelType()) {
    case POISSON:
        if (!gParameters.UsePopulationFile()) break;
        summaryEntries.emplace_back("Population, averaged over time", "");
        summaryEntries.back().first += suffix;
        break;
    case BERNOULLI:
        summaryEntries.emplace_back("Total population", "");
        summaryEntries.back().first += suffix;
        break;
    case EXPONENTIAL:
        summaryEntries.emplace_back("Total individuals", "");
        summaryEntries.back().first += suffix;
        break;
    case BATCHED:
        summaryEntries.emplace_back("Batches", "");
        summaryEntries.back().first += suffix;
        break;
    default: break;
    }

    switch (gParameters.GetProbabilityModelType()) {
    case POISSON: if (!gParameters.UsePopulationFile()) break;
    case BERNOULLI:
    case EXPONENTIAL:
    case BATCHED:
        for (unsigned int i = 0; i < gDataSets->GetNumDataSets(); ++i) {
            summaryEntries.back().second += printString(work, "%s%.0f", (i ? ", " : ""), gDataSets->GetDataSet(i).getTotalPopulation());
        } break;
    default: break;
    }
    // print total cases per data set
    switch (gParameters.GetProbabilityModelType()) {
    case BATCHED:
        summaryEntries.emplace_back("Positive batches", "");
        summaryEntries.back().first += suffix;
        break;
    case POISSON:
    case BERNOULLI:
    case SPACETIMEPERMUTATION:
    case CATEGORICAL:
    case ORDINAL:
    case NORMAL:
    case EXPONENTIAL:
    case HOMOGENEOUSPOISSON:
        summaryEntries.emplace_back("Total number of cases", "");
        summaryEntries.back().first += suffix;
        break;
    default: break;
    }
    switch (gParameters.GetProbabilityModelType()) {
    case POISSON:
    case BERNOULLI:
    case SPACETIMEPERMUTATION:
    case CATEGORICAL:
    case ORDINAL:
    case NORMAL:
    case EXPONENTIAL:
    case BATCHED:
    case HOMOGENEOUSPOISSON:
        for (unsigned int i = 0; i < gDataSets->GetNumDataSets(); ++i) {
            summaryEntries.back().second += printString(work, "%s%ld", (i ? ", " : ""), gDataSets->GetDataSet(i).getTotalCases());
        } break;
    default: break;
    }
    if (gParameters.GetProbabilityModelType() == BATCHED) {
        summaryEntries.emplace_back("Individuals", "");
        summaryEntries.back().first += suffix;
        for (unsigned int i = 0; i < gDataSets->GetNumDataSets(); ++i) {
            summaryEntries.back().second += printString(work, "%s%u", (i ? ", " : ""),
                static_cast<unsigned int>(gDataSets->GetDataSet(i).getTotalMeasureAux() + gDataSets->GetDataSet(i).getTotalMeasureAux2())
            );
        }
    }
    if (gParameters.GetProbabilityModelType() == BERNOULLI) {
        summaryEntries.emplace_back("Percent cases", "");
        summaryEntries.back().first += suffix;
        auto getPopulation = [](count_t c, double p) { return 100.0 * (p ? c / p : 0.0); };
        for (unsigned int i = 0; i < gDataSets->GetNumDataSets(); ++i) {
            if (i) summaryEntries.back().second += ", ";
            summaryEntries.back().second += getValueAsString(
                getPopulation(gDataSets->GetDataSet(i).getTotalCases(), gDataSets->GetDataSet(i).getTotalPopulation()), work2, 1
            );
        }
    }
    //print total area per data set for Homogeneous Poisson model
    if (gParameters.GetProbabilityModelType() == HOMOGENEOUSPOISSON) {
        const HomogeneousPoissonDataSetHandler* pHandler = dynamic_cast<const HomogeneousPoissonDataSetHandler*>(gDataSets.get());
        if (!pHandler) throw prg_error("Could not cast to HomogeneousPoissonDataSetHandler type.", "getSummaryPairs()");
        summaryEntries.emplace_back("Total Area", printString(buffer, "%.0f", pHandler->getTotalArea()));
    }
    //for the ordinal probability model, also print category values and total cases per ordinal category
    if (gParameters.GetProbabilityModelType() == ORDINAL || gParameters.GetProbabilityModelType() == CATEGORICAL) {
        if (gDataSets->GetNumDataSets() == 1) {
            summaryEntries.emplace_back("Category values", "");
            const PopulationData& Population = gDataSets->GetDataSet().getPopulationData();
            for (size_t j = 0; j < Population.GetNumOrdinalCategories(); ++j) {
                if (j) summaryEntries.back().second += ", ";
                summaryEntries.back().second += Population.GetCategoryTypeLabel(j);
            }
            summaryEntries.emplace_back("Total cases per category", "");
            for (size_t j = 0; j < Population.GetNumOrdinalCategories(); ++j) {
                summaryEntries.back().second += printString(work, "%s%ld", (j ? ", " : ""), Population.GetNumCategoryTypeCases(j));
            }
            summaryEntries.emplace_back("Percent cases per category", "");
            for (size_t j = 0; j < Population.GetNumOrdinalCategories(); ++j) {
                summaryEntries.back().second += printString(work, "%s%s", (j ? ", " : ""),
                    getValueAsString(100.0 * Population.GetNumCategoryTypeCases(j) / gDataSets->GetDataSet(0).getTotalCases(), work2, 1).c_str()
                );
            }
        } else {
            for (unsigned int i = 0; i < gDataSets->GetNumDataSets(); ++i) {
                summaryEntries.emplace_back(printString(buffer, "Category values, data set #%d", i + 1), "");
                const PopulationData& Population = gDataSets->GetDataSet(i).getPopulationData();
                for (size_t j = 0; j < Population.GetNumOrdinalCategories(); ++j) {
                    if (j) summaryEntries.back().second += ", ";
                    summaryEntries.back().second += Population.GetCategoryTypeLabel(j);
                }
                summaryEntries.emplace_back(printString(buffer, "Total category cases, data set #%d ", i + 1), "");
                for (size_t j = 0; j < Population.GetNumOrdinalCategories(); ++j) {
                    summaryEntries.back().second += printString(work, "%s%ld", (j ? ", " : ""), Population.GetNumCategoryTypeCases(j));
                }
                summaryEntries.emplace_back(printString(buffer, "Percent cases, data set #%d ", i + 1), "");
                for (size_t j = 0; j < Population.GetNumOrdinalCategories(); ++j) {
                    if (j) summaryEntries.back().second += ", ";
                    summaryEntries.back().second += getValueAsString(
                        100.0 * Population.GetNumCategoryTypeCases(j) / gDataSets->GetDataSet(i).getTotalCases(), work2, 1
                    );
                }
            }
        }
    }
    //for the exponential probability model, also print total censored cases
    if (gParameters.GetProbabilityModelType() == EXPONENTIAL) {
        summaryEntries.emplace_back("Total censored", "");
        summaryEntries.back().first += suffix;
        for (unsigned int i = 0; i < gDataSets->GetNumDataSets(); ++i) {
            summaryEntries.back().second += printString(work, "%s%.0f", (i ? ", " : ""),
                gDataSets->GetDataSet(i).getTotalPopulation() - gDataSets->GetDataSet(i).getTotalCases()
            );
        }
    }
    //for the Poisson probability model, also print annual cases per population
    if (gParameters.GetProbabilityModelType() == POISSON && gParameters.UsePopulationFile()) {
        summaryEntries.emplace_back(printString(buffer, "Annual cases / %.0f", GetAnnualRatePop()), "");
        summaryEntries.back().first += suffix;
        for (unsigned int i = 0; i < gDataSets->GetNumDataSets(); ++i) {
            if (i) summaryEntries.back().second += ", ";
            summaryEntries.back().second += getValueAsString(GetAnnualRate(i), work, 1);
        }
    }
    if (gParameters.GetProbabilityModelType() == NORMAL && !gParameters.getIsWeightedNormal()) {
        summaryEntries.emplace_back("Mean", "");
        summaryEntries.back().first += suffix;
        for (unsigned int i = 0; i < gDataSets->GetNumDataSets(); ++i) {
            if (i) summaryEntries.back().second += ", ";
            summaryEntries.back().second += getValueAsString(gDataSets->GetDataSet(i).getTotalMeasure() / gDataSets->GetDataSet(i).getTotalCases(), work);
        }
        summaryEntries.emplace_back("Variance", "");
        summaryEntries.back().first += suffix;
        for (unsigned int i = 0; i < gDataSets->GetNumDataSets(); ++i) {
            if (i) summaryEntries.back().second += ", ";
            summaryEntries.back().second += getValueAsString(GetUnbiasedVariance(
                gDataSets->GetDataSet(i).getTotalCases(), gDataSets->GetDataSet(i).getTotalMeasure(), gDataSets->GetDataSet(i).getTotalMeasureAux()
            ), work);
        }
        summaryEntries.emplace_back("Standard deviation", "");
        summaryEntries.back().first += suffix;
        for (unsigned int i = 0; i < gDataSets->GetNumDataSets(); ++i) {
            if (i) summaryEntries.back().second += ", ";
            summaryEntries.back().second += getValueAsString(std::sqrt(GetUnbiasedVariance(
                gDataSets->GetDataSet(i).getTotalCases(), gDataSets->GetDataSet(i).getTotalMeasure(), gDataSets->GetDataSet(i).getTotalMeasureAux())
            ), work);
        }
    }
    if (gParameters.GetProbabilityModelType() == NORMAL && gParameters.getIsWeightedNormal()) {
        AbstractWeightedNormalRandomizer* pRandomizer;
        std::vector<AbstractWeightedNormalRandomizer::DataSetStatistics> dataSetStatistics;
        //Check that all randomizers are derived from AbstractWeightedNormalRandomizer class.
        for (unsigned int i = 0; i < gDataSets->GetNumDataSets(); ++i) {
            if ((pRandomizer = dynamic_cast<AbstractWeightedNormalRandomizer*>(gDataSets->GetRandomizer(i))) == 0)
                throw prg_error("Randomizer could not be dynamically casted to AbstractWeightedNormalRandomizer type.\n", "getSummaryPairs()");
            dataSetStatistics.push_back(pRandomizer->getDataSetStatistics());
        }
        summaryEntries.emplace_back("Total weights", "");
        summaryEntries.back().first += suffix;
        for (size_t j = 0; j < dataSetStatistics.size(); ++j) {
            if (j) summaryEntries.back().second += ", ";
            summaryEntries.back().second += getValueAsString(dataSetStatistics[j].gtTotalWeight, work);
        }
        summaryEntries.emplace_back("Mean", "");
        summaryEntries.back().first += suffix;
        for (size_t j = 0; j < dataSetStatistics.size(); ++j) {
            if (j) summaryEntries.back().second += ", ";
            summaryEntries.back().second += getValueAsString(dataSetStatistics[j].gtMean, work);
        }
        summaryEntries.emplace_back("Variance", "");
        summaryEntries.back().first += suffix;
        for (size_t j = 0; j < dataSetStatistics.size(); ++j) {
            if (j) summaryEntries.back().second += ", ";
            summaryEntries.back().second += getValueAsString(dataSetStatistics[j].gtVariance, work);
        }
        summaryEntries.emplace_back("Standard deviation", "");
        summaryEntries.back().first += suffix;
        for (size_t j = 0; j < dataSetStatistics.size(); ++j) {
            if (j) summaryEntries.back().second += ", ";
            summaryEntries.back().second += getValueAsString(std::sqrt(dataSetStatistics[j].gtVariance), work);
        }
        summaryEntries.emplace_back("Weighted Mean", "");
        summaryEntries.back().first += suffix;
        for (size_t j = 0; j < dataSetStatistics.size(); ++j) {
            if (j) summaryEntries.back().second += ", ";
            summaryEntries.back().second += getValueAsString(dataSetStatistics[j].gtWeightedMean, work);
        }
        summaryEntries.emplace_back("Weighted Variance", "");
        summaryEntries.back().first += suffix;
        for (size_t j = 0; j < dataSetStatistics.size(); ++j) {
            if (j) summaryEntries.back().second += ", ";
            summaryEntries.back().second += getValueAsString(dataSetStatistics[j].gtWeightedVariance, work);
        }
        summaryEntries.emplace_back("Weighted Std deviation", "");
        summaryEntries.back().first += suffix;
        for (size_t j = 0; j < dataSetStatistics.size(); ++j) {
            if (j) summaryEntries.back().second += ", ";
            summaryEntries.back().second += getValueAsString(std::sqrt(dataSetStatistics[j].gtWeightedVariance), work);
        }
    }
    if (gParameters.GetAnalysisType() == SPATIALVARTEMPTREND) {
        const AbstractTimeTrend& globalTrend = gDataSets->GetDataSet(0/*for now*/).getTimeTrend();
        if (gParameters.getTimeTrendType() == LINEAR) {
            double nAnnualTT = const_cast<AbstractTimeTrend&>(globalTrend).SetAnnualTimeTrend(gParameters.GetTimeAggregationUnitsType(), gParameters.GetTimeAggregationLength());
            summaryEntries.emplace_back("Time trend", "");
            summaryEntries.back().second += getValueAsString(fabs(nAnnualTT), work, 3);
            summaryEntries.back().second += (nAnnualTT < 0 ? "% annual decrease" : "% annual increase");
        }
        //PrintFormat.PrintSectionLabel(fp, "Global Intercept", false, false);
        //fprintf(fp, "%g\n", globalTrend.GetAlpha());
        //PrintFormat.PrintSectionLabel(fp, "Global Linear", false, false);
        //fprintf(fp, "%g\n", globalTrend.GetBeta());
        const QuadraticTimeTrend* pQTrend = dynamic_cast<const QuadraticTimeTrend*>(&globalTrend);
        if (pQTrend) {
            //PrintFormat.PrintSectionLabel(fp, "Global Quadratic", false, false);
            //fprintf(fp, "%g\n", pQTrend->GetBeta2());
            summaryEntries.emplace_back("Global Risk Function", "");
            pQTrend->getRiskFunction(summaryEntries.back().second, *this);
        }
    }
    if (gParameters.GetProbabilityModelType() == RANK) {
        summaryEntries.emplace_back("Average Rank", "");
        summaryEntries.emplace_back("Mean Rank", "");
        summaryEntries.emplace_back("Average Category", "");
        summaryEntries.emplace_back("Variance", "");
        summaryEntries.emplace_back("Standard deviation", "");
    }
    return summaryEntries;
}

// formats the information necessary in the relative risk output file and prints to the specified format
// pre: none
// post: prints the relative risk data to the output file
void CSaTScanData::DisplayRelativeRisksForEachTract(const LocationRelevance& location_relevance, const MostLikelyClustersContainer& mlc) const {
  try {
    LocationRiskEstimateWriter(*this).Write(*this, location_relevance, mlc);
  } catch (prg_exception& x) {
    x.addTrace("DisplayRelativeRisksForEachTract()", "CSaTScanData");
    throw;
  }
}

