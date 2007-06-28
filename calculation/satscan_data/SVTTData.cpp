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
void CSVTTData::DisplayCases(FILE* pFile) {
  unsigned int i;

  for (i=0; i < gDataSets->GetNumDataSets(); ++i) {
     fprintf(pFile, "Data Set %u:\n", i);
     DisplayCounts(pFile, gDataSets->GetDataSet(i).getCaseData().GetArray(), "Cases Array",
                   gDataSets->GetDataSet(i).getCaseData_NC().GetArray(), "Cases Non-Cumulative Array",
                   gDataSets->GetDataSet(i).getCaseData_PT(), "Cases_TotalByTimeInt");
  }                 
}

/** Debug utility function - prints counts for passed arrays. Caller is
    responsible for ensuring that passed file pointer points to valid, open file
    handle and array pointers are valid. Probably should call
    DisplayCases(FILE* pFile) instead of this function directly. */
void CSVTTData::DisplayCounts(FILE* pFile,
                              count_t** pCounts,   char* szVarName,
                              count_t** pCountsNC, char* szVarNameNC,
                              count_t*  pCountsTI, char* szVarNameTI,
                              char* szTitle) {
  if (szTitle != NULL)
    fprintf(pFile, "%s", szTitle);

  fprintf(pFile, "Counts                         Counts - Not Accumulated\n\n");

  for (int i = 0; i < m_nTimeIntervals; i++)
    for (int j = 0; j < m_nTracts; j++)
    {
      fprintf(pFile, "%s [%i][%i] = %6i     ", szVarName, i,j,pCounts[i][j]);
      fprintf(pFile, "%s [%i][%i] = %6i\n", szVarNameNC, i,j,pCountsNC[i][j]);
    }

  fprintf(pFile, "\nCounts Accumulated by Time Interval\n\n");
  for (int i=0; i<m_nTimeIntervals; i++)
    fprintf(pFile, "%s [%i] = %6i\n", szVarNameTI, i, pCountsTI[i]);

  fprintf(pFile, "\n");
}

/** Debug utility function - prints expected counts for for all datasets.
    Caller is responsible for ensuring that passed file pointer points to valid,
    open file handle. */
void CSVTTData::DisplayMeasures(FILE* pFile) {
  unsigned int           i, j, k;
  measure_t           ** ppMeasure, ** ppMeasureNC;

  fprintf(pFile, "Measures                        Measures - Not Accumulated\n\n");

  for (k=0; k < gDataSets->GetNumDataSets(); ++k) {
     fprintf(pFile, "Data Set %u:\n", k);
     ppMeasure = gDataSets->GetDataSet(k).getMeasureData().GetArray();
     ppMeasureNC = gDataSets->GetDataSet(k).getMeasureData_NC().GetArray();
     for (i=0; i < (unsigned int)m_nTimeIntervals; ++i)
        for (j=0; j < (unsigned int)m_nTracts; ++j) {
           fprintf(pFile, "ppMeasure [%i][%i] = %12.5f     ", i, j, ppMeasure[i][j]);
           fprintf(pFile, "ppMeasure_NC [%i][%i] = %12.5f\n", i, j, ppMeasureNC[i][j]);
        }
  }

  fprintf(pFile, "\nMeasures Accumulated by Time Interval\n\n");
  for (k=0; k < gDataSets->GetNumDataSets(); ++k) {
     fprintf(pFile, "Data Set %u:\n", k);
     for (i=0; i < (unsigned int)m_nTimeIntervals; ++i)
       fprintf(pFile, "Measure_TotalByTimeInt [%i] = %12.5f\n", i, gDataSets->GetDataSet(k).getMeasureData_PT()[i]);
     fprintf(pFile, "\n");
  }
}

// formats the information necessary in the relative risk output file and prints to the specified format
void CSVTTData::DisplayRelativeRisksForEachTract() const {
  try {
    LocationRiskEstimateWriter(*this).Write(*this);
  }
  catch (prg_exception& x) {
    x.addTrace("DisplayRelativeRisksForEachTract()","CSVTTData");
    throw;
  }
}

/** Not implemented - needs to be updated */
void CSVTTData::DisplaySimCases(FILE* pFile) {
//  unsigned int i;                                             
//
//  for (i=0; i < gDataSets->GetNumDataSets(); ++i) {
//     fprintf(pFile, "Data Set %u:\n", i);
//     DisplayCounts(pFile, gDataSets->GetDataSet(i).GetSimCaseArray(), "Simulated Cases Array",
//                   gDataSets->GetDataSet(i).GetNCSimCaseArray(), "Simulated Non-Cumulative Cases Array",
//                   gDataSets->GetDataSet(i).GetPTSimCasesArray(), "SimCases_TotalByTimeInt");
//  }
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
                                                           m_nTimeIntervals,
                                                           gParameters.GetTimeTrendConvergence());
      if (SimDataContainer[t]->getTimeTrend().GetStatus() == CTimeTrend::NOT_CONVERGED)
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
                                             m_nTimeIntervals, gParameters.GetTimeTrendConvergence());
       switch ((*itr)->getTimeTrend().GetStatus()) {
          case CTimeTrend::UNDEFINED         :
            throw resolvable_error("Error: The number of cases in data set %d is less than 2.\n"
                                   "       Time trend can not be calculated.", 
                                   std::distance(gDataSets->getDataSets().begin(), itr) + 1);
          case CTimeTrend::NEGATIVE_INFINITY :
          case CTimeTrend::POSITIVE_INFINITY :
            throw resolvable_error("Error: All cases in data set %d are either in first or last time interval.\n"
                                   "       Time trend is infinite.", 
                                   std::distance(gDataSets->getDataSets().begin(), itr) + 1);
          case CTimeTrend::NOT_CONVERGED :
            throw prg_error("The time trend in real data did not converge.\n","ReadDataFromFiles()");
          case CTimeTrend::CONVERGED          :
          default                             : break; 
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
                                           m_nTimeIntervals, gParameters.GetTimeTrendConvergence());
    if ((*itr)->getTimeTrend().GetStatus() == CTimeTrend::NOT_CONVERGED)
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
       case ORDINAL              : throw prg_error("Spatial Variation in Temporal Trends not implemented for Ordinal model.\n",
                                                   "SetProbabilityModel()");
       case EXPONENTIAL          : throw prg_error("Spatial Variation in Temporal Trends not implemented for Exponential model.\n",
                                                   "SetProbabilityModel()");
       case WEIGHTEDNORMAL       :
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

