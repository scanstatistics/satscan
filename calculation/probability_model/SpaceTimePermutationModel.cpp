#include "SaTScan.h"
#pragma hdrstop
#include "SpaceTimePermutationModel.h"

/** Allocates randomization structures used by MakeData() routine. Must be called
prior to called MakeData(). */
void SpaceTimeRandomizer::InitializeStructures(DataStream & thisStream, int iTimeIntervals, int iTracts) {
  int	                i, j, k, c, iNumCases,
                        iNumCategories(thisStream.GetPopulationData().GetNumPopulationCategories());
  std::vector<int>      vCummulatedCases;
  count_t               iMaxCasesPerCategory,
                     ** ppCases(thisStream.GetCaseArray()),
                    *** pppCategoryCases(thisStream.GetCategoryCaseArray());

  try {
    gvCategoryCaseLocationTimes.resize(iNumCategories);
    vCummulatedCases.resize(iTracts);
    for (c=0; c < iNumCategories; ++c) {
       std::vector<CCaseLocationTimes>& thisCategory = gvCategoryCaseLocationTimes[c];
       memset(&vCummulatedCases[0], 0, iTracts*sizeof(int));
       for (i=iTimeIntervals - 1; i >= 0; --i) {
          for (j=0; j < iTracts; ++j) {
             iNumCases = pppCategoryCases[i][j][c] - vCummulatedCases[j];
             for (k=0; k < iNumCases; ++k)
                thisCategory.push_back(CCaseLocationTimes(i,j));
             vCummulatedCases[j] += iNumCases;
          }
       }
    }

    // allocate time randomizer to the maximum number of cases in all categories 
    for (c=0, iMaxCasesPerCategory=0; c < iNumCategories; ++c)
       iMaxCasesPerCategory = std::max(iMaxCasesPerCategory, thisStream.GetPopulationData().GetNumCategoryCases(c));
    gvTimeIntervalRandomizer.resize(iMaxCasesPerCategory, CSimulationTimeRandomizer());
  }
  catch (ZdException & x) {
    x.AddCallpath("InitializeRandomizationStructures()", "SpaceTimeRandomizer");
    gvCategoryCaseLocationTimes.clear();
    gvTimeIntervalRandomizer.clear();
    throw;
  }
}

/** Creates simulation data. Permutates the cases occurance dates through random sorting. */
void SpaceTimeRandomizer::MakeData(int iSimulationNumber, DataStreamInterface & DataInterface) {
  int                   i, k, c;
  size_t                t, tNumCategoryCases;
  count_t            ** ppSimCases(DataInterface.GetCaseArray());

  try {
    //reset seed to simulation number
    gRandomNumberGenerator.SetSeed(iSimulationNumber + gRandomNumberGenerator.GetDefaultSeed());
    // reset simulation cases to zero
    DataInterface.ResetCaseArray(0);
    
    for (c=0; c < (int)gvCategoryCaseLocationTimes.size(); ++c) {
       const std::vector<CCaseLocationTimes>& thisCategory = gvCategoryCaseLocationTimes[c];
       tNumCategoryCases = thisCategory.size();
       for (t=0; t < tNumCategoryCases; ++t) {
         // assign random number
         gvTimeIntervalRandomizer[t].SetRandomNumber(gRandomNumberGenerator.GetRandomFloat());
         // assign tract index to original order
         gvTimeIntervalRandomizer[t].SetTimeIntervalIndex(thisCategory[t].GetTimeIntervalIndex());
       }
       // sort based on random number
       std::sort(gvTimeIntervalRandomizer.begin(), gvTimeIntervalRandomizer.begin() + tNumCategoryCases, CompareSimulationTimeRandomizer());
       // re-assign simulation data
       for (t=0; t < tNumCategoryCases; t++)
         for (k=gvTimeIntervalRandomizer[t].GetTimeIntervalIndex(); k >= 0; k--)
             ppSimCases[k][thisCategory[t].GetTractIndex()]++;
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("MakeData()", "SpaceTimeRandomizer");
    throw;
  }
}

/** constructor */
CSpaceTimePermutationModel::CSpaceTimePermutationModel(CParameters& Parameters, CSaTScanData& Data, BasePrint& PrintDirection)
                           :CModel(Parameters, Data, PrintDirection) {}

/** destructor */
CSpaceTimePermutationModel::~CSpaceTimePermutationModel() {}

/** Calculates loglikelihood, this routine is identical to Possion model. */
double CSpaceTimePermutationModel::CalcLogLikelihood(count_t n, measure_t u) {
   double    nLogLikelihood;
   count_t   N = gData.GetTotalCases();
   measure_t U = gData.GetTotalMeasure();

   if (n != N && n != 0)
     nLogLikelihood = n*log(n/u) + (N-n)*log((N-n)/(U-u));
   else if (n == 0)
     nLogLikelihood = (N-n) * log((N-n)/(U-u));
   else
     nLogLikelihood = n*log(n/u);

   return (nLogLikelihood);
}

/** Calculates loglikelihood, this routine is identical to Possion model. */
double CSpaceTimePermutationModel::CalcLogLikelihoodRatio(count_t tCases, measure_t tMeasure, count_t tTotalCases, measure_t tTotalMeasure, double dCompactnessCorrection) {
  double    dLogLikelihood;

  // calculate the loglikelihood
  if (tCases != tTotalCases && tCases != 0)
    dLogLikelihood = tCases*log(tCases/tMeasure) + (tTotalCases-tCases)*log((tTotalCases-tCases)/(tTotalMeasure-tMeasure));
  else if (tCases == 0)
    dLogLikelihood = (tTotalCases-tCases) * log((tTotalCases-tCases)/(tTotalMeasure-tMeasure));
  else
    dLogLikelihood = tCases*log(tCases/tMeasure);

  // return the logliklihood ratio (loglikelihood - loglikelihood for total) * duczmal compactness correction
  return (dLogLikelihood - (tTotalCases * log(tTotalCases/tTotalMeasure))) * dCompactnessCorrection;
}

/** Determines the expected number of cases for each time interval/tract.
    Assigns values to CSatScanData::Measure array. Calculates total measure
    and validates that total measure equals total number of cases in set. */
bool CSpaceTimePermutationModel::CalculateMeasure(DataStream & thisStream) {
  int                   i, j, c, iNumCategories(thisStream.GetPopulationData().GetNumPopulationCategories());
  count_t               tTotalCases(0),
                     ** ppCases(thisStream.GetCaseArray()),
                    *** pppCategoryCases(thisStream.GetCategoryCaseArray());
  measure_t          ** ppMeasure, T_C, tTotalMeasure(0);
  PopulationData      & Population = thisStream.GetPopulationData();

  try {
    //calculate total number of cases
    for (j=0; j < gData.m_nTracts; j++) {
       tTotalCases += ppCases[0][j];
       // Check to see if total case or control values have wrapped
       if (tTotalCases < 0)
         SSGenerateException("Error: Total cases greater than maximum allowed of %ld.\n", "CalculateMeasure()", std::numeric_limits<count_t>::max());
    }

    thisStream.AllocateMeasureArray();
    ppMeasure = thisStream.GetMeasureArray();
    // set ppMeasure[i] = S*T/C (expected number of cases in a time/tract)
    // S = number of cases in spacial area irrespective of time
    // T = number of cases in temporal domain irrespective of location
    // C = total number of cases
    for (c=0; c < iNumCategories; ++c) {
       for (i=0; i < gData.m_nTimeIntervals; ++i) {
         T_C = pppCategoryCases[i][0][c];
         //Calculate T/C
         for (j=1; j < gData.m_nTracts; ++j)
            T_C += pppCategoryCases[i][j][c];
         T_C /= Population.GetNumCategoryCases(c);
         //Multiply T/C by S and add to measure
         for (j=0; j < gData.m_nTracts; ++j)
           ppMeasure[i][j] += T_C * pppCategoryCases[0][j][c];
       }
    }

    // calculate total measure
    for (j=0; j< gData.m_nTracts; ++j)
       tTotalMeasure += ppMeasure[0][j];

    // Ensure that TotalCases=TotalMeasure
    if (fabs(tTotalCases - tTotalMeasure)>0.0001)
      SSGenerateException("Error: The total measure is not equal to the total number of cases.\n"
                          "Total Cases = %i, Total Measure = %.2lf\n", "CalculateMeasure()",
                          tTotalCases, tTotalMeasure);

    thisStream.SetTotalCases(tTotalCases);
    thisStream.SetTotalControls(0);
    thisStream.SetTotalMeasure(tTotalMeasure);
    thisStream.SetTotalPopulation(0);
  }
  catch (ZdException &x) {
    x.AddCallpath("CalculateMeasure()","CSpaceTimePermutationModel");
    throw;
  }
  return true;
}

/** Same log likelihood as for the Poisson model.
    It is no longer a "likelihood", but it will serve the same purpose.
    Martin Kulldorph derived the true log likelihoods, but they are too complex
    and there is no way to implement them in a time efficient way, so using them
    would  lead to slow execution times. The Poisson model is a very good
    approximation of the likelihood, and that can be shown mathematically. */
double CSpaceTimePermutationModel::GetLogLikelihoodForTotal() const
{
  count_t   N = gData.GetTotalCases();
  measure_t U = gData.GetTotalMeasure();

  return N*log(N/U);
}

/** Allocates randomization structures used by MakeData() routine. Must be called
prior to called MakeData(). */
void CSpaceTimePermutationModel::InitializeRandomizationStructures() {
  try {
    gvRandomizers.resize(gData.GetDataStreamHandler().GetNumStreams());
    for (size_t t=0; t < gvRandomizers.size(); ++t)
       gvRandomizers[t].InitializeStructures(gData.GetDataStreamHandler().GetStream(t), gData.GetNumTimeIntervals(), gData.GetNumTracts());
  }
  catch (ZdException & x) {
    x.AddCallpath("InitializeRandomizationStructures()", "CSpaceTimePermutationModel");
    throw;
  }
}

/** Throws exception. Defined in parent class as pure virtual. */
double CSpaceTimePermutationModel::GetPopulation(int m_iEllipseOffset, tract_t nCenter, tract_t nTracts,
                                                 int nStartInterval, int nStopInterval)
{
  SSGenerateException("Function GetPopulation() not implemented in CSpaceTimePermutationModel",
                      "GetPopulation() of CSpaceTimePermutationModel");
  return 0;
}

/** Creates simulation data. Permutates the cases occurance dates through random sorting. */
void CSpaceTimePermutationModel::MakeData(int iSimulationNumber, DataStreamInterface & DataInterface, unsigned int tInterface) {
  try {
    gvRandomizers[tInterface].MakeData(iSimulationNumber, DataInterface);
  }
  catch (ZdException & x) {
    x.AddCallpath("MakeData()", "CSpaceTimePermutationModel");
    throw;
  }
}

/** Reads data from files(geographical, case and special grid) into data structures.
    Initializes randomization structures. */
bool CSpaceTimePermutationModel::ReadData() {
  try {
    if (!gData.ReadSpaceTimePermutationData())
      return false;

    if (gData.GetParameters().GetNumReplicationsRequested() > 0)
      InitializeRandomizationStructures();
  }
  catch (ZdException & x) {
    x.AddCallpath("ReadData()", "CSpaceTimePermutationModel");
    throw;
  }
  return true;
}

