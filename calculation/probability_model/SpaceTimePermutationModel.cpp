#include "SaTScan.h"
#pragma hdrstop
#include "SpaceTimePermutationModel.h"

/** constructor */
CSpaceTimePermutationModel::CSpaceTimePermutationModel(CParameters& Parameters, CSaTScanData& Data, BasePrint& PrintDirection)
                           :CModel(Parameters, Data, PrintDirection) {}

/** destructor */
CSpaceTimePermutationModel::~CSpaceTimePermutationModel() {}

/** Calculates loglikelihood, this routine is identical to Possion model. */
double CSpaceTimePermutationModel::CalcLogLikelihood(count_t n, measure_t u)
{
   double    nLogLikelihood;
   count_t   N = gData.m_nTotalCases;
   measure_t U = gData.m_nTotalMeasure;

   if (n != N && n != 0)
     nLogLikelihood = n*log(n/u) + (N-n)*log((N-n)/(U-u));
   else if (n == 0)
     nLogLikelihood = (N-n) * log((N-n)/(U-u));
   else
     nLogLikelihood = n*log(n/u);

   return (nLogLikelihood);
}

/** Determines the expected number of cases for each time interval/tract.
    Assigns values to CSatScanData::Measure array. Calculates total measure
    and validates that total measure equals total number of cases in set. */
bool CSpaceTimePermutationModel::CalculateMeasure() {
  int                   i, j, c, iNumCategories(gData.GetTInfo()->tiGetNumCategories());
  count_t            ** ppCases(gData.gpCasesHandler->GetArray()),
                    *** pppCategoryCases(gData.gpCategoryCasesHandler->GetArray());
  measure_t          ** ppMeasure, T_C;

  try {
    //calculate total number of cases
    gData.m_nTotalCases=0;
    for (j=0; j < gData.m_nTracts; j++) {
       gData.m_nTotalCases += ppCases[0][j];
       // Check to see if total case or control values have wrapped
       if (gData.m_nTotalCases < 0)
         SSGenerateException("Error: Total cases greater than maximum allowed of %ld.\n", "CalculateMeasure()", std::numeric_limits<count_t>::max());
    }

    gData.gpMeasureHandler = new TwoDimensionArrayHandler<measure_t>(gData.m_nTimeIntervals+1, gData.m_nTracts, 0);
    ppMeasure = gData.GetMeasureArray();
    gData.m_nTotalMeasure  = 0;
    gData.m_nTotalPop = 0;

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
         T_C /= gData.GetNumCategoryCases(c);
         //Multiply T/C by S and add to measure
         for (j=0; j < gData.m_nTracts; ++j)
           ppMeasure[i][j] += T_C * pppCategoryCases[0][j][c];
       }
    }

    // calculate total measure
    for (j=0; j< gData.m_nTracts; ++j)
       gData.m_nTotalMeasure += ppMeasure[0][j];

    // Ensure that TotalCases=TotalMeasure
    if (fabs(gData.m_nTotalCases - gData.m_nTotalMeasure)>0.0001)
      SSGenerateException("Error: The total measure is not equal to the total number of cases.\n"
                          "Total Cases = %i, Total Measure = %.2lf\n", "CalculateMeasure()",
                          gData.m_nTotalCases, gData.m_nTotalMeasure);
  }
  catch (ZdException &x) {
    delete gData.gpMeasureHandler; gData.gpMeasureHandler=0;
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
  count_t   N = gData.m_nTotalCases;
  measure_t U = gData.m_nTotalMeasure;

  return N*log(N/U);
}

/** Allocates randomization structures used by MakeData() routine. Must be called
prior to called MakeData(). */
void CSpaceTimePermutationModel::InitializeRandomizationStructures() {
  int	                i, j, k, c, iNumCases, iNumCategories(gData.GetTInfo()->tiGetNumCategories());
  std::vector<int>      vCummulatedCases;
  count_t               iMaxCasesPerCategory,
                     ** ppCases(gData.GetCasesArray()),
                    *** pppCategoryCases(gData.gpCategoryCasesHandler->GetArray());

  try {
    gvCategoryCaseLocationTimes.resize(iNumCategories);
    vCummulatedCases.resize(gData.m_nTracts);
    for (c=0; c < iNumCategories; ++c) {
       std::vector<CCaseLocationTimes>& thisCategory = gvCategoryCaseLocationTimes[c];
       memset(&vCummulatedCases[0], 0, gData.m_nTracts*sizeof(int));
       for (i=gData.m_nTimeIntervals - 1; i >= 0; --i) {
          for (j=0; j < gData.m_nTracts; ++j) {
             iNumCases = pppCategoryCases[i][j][c] - vCummulatedCases[j];
             for (k=0; k < iNumCases; ++k)
                thisCategory.push_back(CCaseLocationTimes(i,j));
             vCummulatedCases[j] += iNumCases;
          }
       }
    }

    // allocate time randomizer to the maximum number of cases in all categories 
    for (c=0, iMaxCasesPerCategory=0; c < iNumCategories; ++c)
       iMaxCasesPerCategory = std::max(iMaxCasesPerCategory, gData.GetNumCategoryCases(c));
    gvTimeIntervalRandomizer.resize(iMaxCasesPerCategory, CSimulationTimeRandomizer());
  }
  catch (ZdException & x) {
    x.AddCallpath("InitializeRandomizationStructures()", "CSpaceTimePermutationModel");
    gvCategoryCaseLocationTimes.clear();
    gvTimeIntervalRandomizer.clear();
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
void CSpaceTimePermutationModel::MakeData(int iSimulationNumber) {
  int                   i, k, c;
  size_t                t, tNumCategoryCases;
  count_t            ** ppSimCases(gData.GetSimCasesArray());

  try {
    //reset seed to simulation number
    gRandomNumberGenerator.SetSeed(iSimulationNumber + gRandomNumberGenerator.GetDefaultSeed());
    // reset simulation cases to zero
    gData.gpSimCasesHandler->Set(0);
    
    for (c=0; c < gData.GetTInfo()->tiGetNumCategories(); c++) {
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
    x.AddCallpath("MakeData()", "CSpaceTimePermutationModel");
    throw;
  }
}

/** Reads data from files(geographical, case and special grid) into data structures.
    Initializes randomization structures. */
bool CSpaceTimePermutationModel::ReadData() {
  try {
    if (!gData.ReadCoordinatesFile())
      return false;
    if (DoesReadMaxCirclePopulationFile() && !gData.ReadMaxCirclePopulationFile())
       return false;
    if (! gData.ReadCaseFile())
      return false;
    if (gParameters.UseSpecialGrid() && !gData.ReadGridFile())
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

