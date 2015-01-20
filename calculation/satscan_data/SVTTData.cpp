//*****************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//*****************************************************************************
#include "SVTTData.h"
#include "PoissonModel.h"
#include "BernoulliModel.h"
#include "SpaceTimePermutationModel.h"
#include "LocationRiskEstimateWriter.h"
#include "SSException.h"

/** class constructor */
CSVTTData::CSVTTData(const CParameters& Parameters, BasePrint& PrintDirection)
          :CSaTScanData(Parameters, PrintDirection) {
  try {
    SetProbabilityModel();
  }
  catch (prg_exception& x) {
    x.addTrace("constructor()","CSVTTData");
    throw;
  }
}

/** class destructor */
CSVTTData::~CSVTTData() {}

/** Debug utility function - prints case counts for all datasets. Caller is
    responsible for ensuring that passed file pointer points to valid, open file
    handle. */
void CSVTTData::DisplayCases(FILE* pFile) const {
  unsigned int i;

  for (i=0; i < gDataSets->GetNumDataSets(); ++i) {
     fprintf(pFile, "Data Set %u:\n", i);
     DisplayCounts(pFile, gDataSets->GetDataSet(i).getCaseData().GetArray(), "Cases Array",
                   gDataSets->GetDataSet(i).getCaseData_NC().GetArray(), "Cases Non-Cumulative Array",
                   gDataSets->GetDataSet(i).getCaseData_PT(), "Cases_TotalByTimeInt");
  }
  fflush(pFile);
}

/** Debug utility function - prints counts for passed arrays. Caller is
    responsible for ensuring that passed file pointer points to valid, open file
    handle and array pointers are valid. Probably should call
    DisplayCases(FILE* pFile) instead of this function directly. */
void CSVTTData::DisplayCounts(FILE* pFile,
                              count_t** pCounts,   const char* szVarName,
                              count_t** pCountsNC, const char* szVarNameNC,
                              count_t*  pCountsTI, const char* szVarNameTI,
                              const char* szTitle) const {
  if (szTitle != NULL)
    fprintf(pFile, "%s", szTitle);

  fprintf(pFile, "Counts                         Counts - Not Accumulated\n\n");

  for (int i = 0; i < GetNumTimeIntervals(); i++)
    for (int j = 0; j < m_nTracts; j++)
    {
      fprintf(pFile, "%s [%i][%i] = %6li     ", szVarName, i,j,pCounts[i][j]);
      fprintf(pFile, "%s [%i][%i] = %6li\n", szVarNameNC, i,j,pCountsNC[i][j]);
    }

  fprintf(pFile, "\nCounts Accumulated by Time Interval\n\n");
  for (int i=0; i < GetNumTimeIntervals(); i++)
    fprintf(pFile, "%s [%i] = %6li\n", szVarNameTI, i, pCountsTI[i]);
  fprintf(pFile, "\n");
  fflush(pFile);
}

/** Debug utility function - prints expected counts for for all datasets.
    Caller is responsible for ensuring that passed file pointer points to valid,
    open file handle. */
void CSVTTData::DisplayMeasures(FILE* pFile) const {
  unsigned int           i, j, k;
  measure_t           ** ppMeasure, ** ppMeasureNC;

  fprintf(pFile, "Measures                        Measures - Not Accumulated\n\n");

  for (k=0; k < gDataSets->GetNumDataSets(); ++k) {
     fprintf(pFile, "Data Set %u:\n", k);
     ppMeasure = gDataSets->GetDataSet(k).getMeasureData().GetArray();
     ppMeasureNC = gDataSets->GetDataSet(k).getMeasureData_NC().GetArray();
     for (i=0; i < (unsigned int)GetNumTimeIntervals(); ++i)
        for (j=0; j < (unsigned int)m_nTracts; ++j) {
           fprintf(pFile, "ppMeasure [%i][%i] = %12.5f     ", i, j, ppMeasure[i][j]);
           fprintf(pFile, "ppMeasure_NC [%i][%i] = %12.5f\n", i, j, ppMeasureNC[i][j]);
        }
  }

  fprintf(pFile, "\nMeasures Accumulated by Time Interval\n\n");
  for (k=0; k < gDataSets->GetNumDataSets(); ++k) {
     fprintf(pFile, "Data Set %u:\n", k);
     for (i=0; i < (unsigned int)GetNumTimeIntervals(); ++i)
       fprintf(pFile, "Measure_TotalByTimeInt [%i] = %12.5f\n", i, gDataSets->GetDataSet(k).getMeasureData_PT()[i]);
     fprintf(pFile, "\n");
  }
  fflush(pFile);
}

// formats the information necessary in the relative risk output file and prints to the specified format
void CSVTTData::DisplayRelativeRisksForEachTract(const LocationRelevance& location_relevance) const {
  try {
    LocationRiskEstimateWriter(*this).Write(*this);
  }
  catch (prg_exception& x) {
    x.addTrace("DisplayRelativeRisksForEachTract()","CSVTTData");
    throw;
  }
}

/** Prints simulation data to file stream. */
void CSVTTData::DisplaySimCases(SimulationDataContainer_t& Container, FILE* pFile) const {
  for (unsigned int i=0; i < Container.size(); ++i) {
     fprintf(pFile, "Data Set %u:\n", i);
     DisplayCounts(pFile, Container.at(i)->getCaseData().GetArray(), "Simulated Cases Array",
                   Container.at(i)->getCaseData_NC().GetArray(), "Simulated Non-Cumulative Cases Array",
                   Container.at(i)->getCaseData_PT(), "SimCases_TotalByTimeInt");
  }
  fflush(pFile);
}

/** Randomizes collection of simulation data in concert with passed collection of randomizers. */
void CSVTTData::RandomizeData(RandomizerContainer_t& RandomizerContainer,
                              SimulationDataContainer_t& SimDataContainer,
                              unsigned int iSimulationNumber) const {
  try {
    CSaTScanData::RandomizeData(RandomizerContainer, SimDataContainer, iSimulationNumber);
    std::for_each(SimDataContainer.begin(), SimDataContainer.end(), std::mem_fun(&DataSet::setCaseData_NC));
    std::for_each(SimDataContainer.begin(), SimDataContainer.end(), std::mem_fun(&DataSet::setCaseData_PT_NC));
    for (size_t t=0; t < SimDataContainer.size(); ++t) {
       //calculate time trend for entire randomized data set
       SimDataContainer[t]->getTimeTrend().CalculateAndSet(SimDataContainer[t]->getCaseData_PT_NC(),
                                                           gDataSets->GetDataSet(t).getMeasureData_PT_NC(),
                                                           GetNumTimeIntervals(),
                                                           gParameters.GetTimeTrendConvergence());
      if (SimDataContainer[t]->getTimeTrend().GetStatus() == AbstractTimeTrend::NOT_CONVERGED)
        throw prg_error("Randomized data set time trend does not converge.\n", "RandomizeData()");
    }
  }
  catch (prg_exception& x) {
    x.addTrace("RandomizeData()","CSVTTData");
    throw;
  }
}

/** Calls base class CSaTScanData::ReadDataFromFiles() then sets non-cummulative case data strcutures. */
void CSVTTData::ReadDataFromFiles() {
  try {
    CSaTScanData::ReadDataFromFiles();
    std::for_each(gDataSets->getDataSets().begin(), gDataSets->getDataSets().end(), std::mem_fun(&DataSet::setCaseData_NC));
    std::for_each(gDataSets->getDataSets().begin(), gDataSets->getDataSets().end(), std::mem_fun(&DataSet::setCaseData_PT_NC));
    std::for_each(gDataSets->getDataSets().begin(), gDataSets->getDataSets().end(), std::mem_fun(&DataSet::setMeasureData_NC));
    std::for_each(gDataSets->getDataSets().begin(), gDataSets->getDataSets().end(), std::mem_fun(&DataSet::setMeasureData_PT_NC));
    for (RealDataContainer_t::iterator itr=gDataSets->getDataSets().begin(); itr != gDataSets->getDataSets().end(); ++itr) {
      //calculate time trend for dataset data set
      (*itr)->getTimeTrend().CalculateAndSet((*itr)->getCaseData_PT_NC(), (*itr)->getMeasureData_PT_NC(),
                                             GetNumTimeIntervals(), gParameters.GetTimeTrendConvergence());
       switch ((*itr)->getTimeTrend().GetStatus()) {
          case AbstractTimeTrend::UNDEFINED         :
            if (gParameters.getTimeTrendType() == QUADRATIC) {
              throw resolvable_error("Error: Time trend can not be calculated for data set %d.\n"
                                     "       There must exist at least 3 cases in 3 separate time intervals to calculate quadratic trend.", 
                                     std::distance(gDataSets->getDataSets().begin(), itr) + 1);
            } else {
              throw resolvable_error("Error: The number of cases in data set %d is less than 2.\n"
                                     "       Time trend can not be calculated.", 
                                     std::distance(gDataSets->getDataSets().begin(), itr) + 1);
            }
          case AbstractTimeTrend::NEGATIVE_INFINITY :
          case AbstractTimeTrend::POSITIVE_INFINITY :
            throw resolvable_error("Error: All cases in data set %d are either in first or last time interval.\n"
                                   "       Time trend is infinite.", 
                                   std::distance(gDataSets->getDataSets().begin(), itr) + 1);
          case AbstractTimeTrend::NOT_CONVERGED :
            throw prg_error("The time trend in real data did not converge.\n","ReadDataFromFiles()");
          case AbstractTimeTrend::CONVERGED          :
          default                             : break; 
       }
       const LinearTimeTrend * pLinearTimeTrend = dynamic_cast<const LinearTimeTrend *>(&((*itr)->getTimeTrend()));
       if (pLinearTimeTrend) {
           pLinearTimeTrend->Alpha((*itr)->getTotalCases(), (*itr)->getMeasureData_PT_NC(), GetNumTimeIntervals(), pLinearTimeTrend->GetBeta());
       }
       const QuadraticTimeTrend * pQuadraticTimeTrend = dynamic_cast<const QuadraticTimeTrend *>(&((*itr)->getTimeTrend()));
       if (pQuadraticTimeTrend) {
           pQuadraticTimeTrend->Alpha((*itr)->getTotalCases(), (*itr)->getMeasureData_PT_NC(), GetNumTimeIntervals(), pQuadraticTimeTrend->GetBeta(), pQuadraticTimeTrend->GetBeta2());
       }
    }
  }
  catch (prg_exception& x) {
    x.addTrace("ReadDataFromFiles()","CSVTTData");
    throw;
  }
}

/** Removes all cases/controls/measure from data sets, geographically and temporally, for
    location at tTractIndex in specified interval range. */
void CSVTTData::RemoveClusterSignificance(const CCluster& Cluster) {
  CSaTScanData::RemoveClusterSignificance(Cluster);
  std::for_each(gDataSets->getDataSets().begin(), gDataSets->getDataSets().end(), std::mem_fun(&DataSet::setCaseData_NC));
  std::for_each(gDataSets->getDataSets().begin(), gDataSets->getDataSets().end(), std::mem_fun(&DataSet::setCaseData_PT_NC));
  std::for_each(gDataSets->getDataSets().begin(), gDataSets->getDataSets().end(), std::mem_fun(&DataSet::setMeasureData_NC));
  std::for_each(gDataSets->getDataSets().begin(), gDataSets->getDataSets().end(), std::mem_fun(&DataSet::setMeasureData_PT_NC));
  for (RealDataContainer_t::iterator itr=gDataSets->getDataSets().begin(); itr != gDataSets->getDataSets().end(); ++itr) {
     //calculate time trend for dataset data set
    (*itr)->getTimeTrend().CalculateAndSet((*itr)->getCaseData_PT_NC(), (*itr)->getMeasureData_PT_NC(),
                                           GetNumTimeIntervals(), gParameters.GetTimeTrendConvergence());
    if ((*itr)->getTimeTrend().GetStatus() == AbstractTimeTrend::NOT_CONVERGED)
    throw prg_error("The time trend does not converge after removing cluster data.\n", "RemoveClusterSignificance()");
  }
}

/** Allocates probability model obect. Throws prg_error for all probability
    model type except Poisson. */
void CSVTTData::SetProbabilityModel() {
  try {
    switch (gParameters.GetProbabilityModelType()) {
       case POISSON              : m_pModel = new CPoissonModel(*this);   break;
       case BERNOULLI            : throw prg_error("Spatial Variation in Temporal Trends not implemented for Bernoulli model.\n",
                                                   "SetProbabilityModel()");
       case SPACETIMEPERMUTATION : throw prg_error("Spatial Variation in Temporal Trends not implemented for Space-Time Permutation model.\n",
                                                   "SetProbabilityModel()");
       case CATEGORICAL          : throw prg_error("Spatial Variation in Temporal Trends not implemented for Categorical model.\n",
                                                   "SetProbabilityModel()");
       case ORDINAL              : throw prg_error("Spatial Variation in Temporal Trends not implemented for Ordinal model.\n",
                                                   "SetProbabilityModel()");
       case EXPONENTIAL          : throw prg_error("Spatial Variation in Temporal Trends not implemented for Exponential model.\n",
                                                   "SetProbabilityModel()");
       case NORMAL               : throw prg_error("Spatial Variation in Temporal Trends not implemented for Normal model.\n",
                                                   "SetProbabilityModel()");
       case RANK                 : throw prg_error("Spatial Variation in Temporal Trends not implemented for Rank model.\n",
                                                   "SetProbabilityModel()");
       default : throw prg_error("Unknown probability model type: '%d'.\n",
                                       "SetProbabilityModel()", gParameters.GetProbabilityModelType());
    }
  }
  catch (prg_exception& x) {
    x.addTrace("SetProbabilityModel()","CSVTTData");
    throw;
  }
}

/** First calls base class SetIntervalStartTimes() then verifies that first time interval
    length is equal to time aggregation length. This is a requirement for SVTT, all time 
    intervals must be of same length. */
void CSVTTData::SetIntervalStartTimes() {
    CSaTScanData::SetIntervalStartTimes();

    //It is possible that the initial time interval is shorter than other time intervals. For SVTT, it is 
    //necessary for all time intervals to be of equal length. We will verfiy that user has specified a valid
    //study period given time aggregation settings and abort analysis if we find start interval is short.

    //To verfiy, we'll decrement from earliest known complete time interval to determine expected start time.
    DecrementableEndDate secondIntervalDate(*(gvTimeIntervalStartTimes.begin() + 1) - 1, gParameters.GetTimeAggregationUnitsType());
    Julian expectedStartingDate = secondIntervalDate.Decrement(gParameters.GetTimeAggregationLength());

    if (*(gvTimeIntervalStartTimes.begin()) != expectedStartingDate) {
        std::string buffer, buffer2;
        throw resolvable_error("Error: For a spatial variation in temporal trends analysis, the length of the time\n"
                               "       intervals must be the same. Please use a different length of the time interval\n"
                               "       that is evenly divisible by the total length of the study period.\n\n"
                               "       Alternatively, shorten the study period and select the advanced input data checking\n"
                               "       option to ignore cases that falls outside the study period. You should not increase\n"
                               "       the study period to include time periods without data.\n"
                               "       Example: Given time aggregation settings, a valid study period is: %s to %s\n",
                               JulianToString(buffer, *(gvTimeIntervalStartTimes.begin() + 1), gParameters.GetPrecisionOfTimesType()).c_str(), 
                               JulianToString(buffer2, m_nEndDate, gParameters.GetPrecisionOfTimesType()).c_str());
    }
}
