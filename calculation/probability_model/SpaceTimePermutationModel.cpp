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
    Assigns values to CSatScanData::m_pMeasure array. Calculates total measure
    and validates that total measure equals total number of cases in set. */
bool CSpaceTimePermutationModel::CalculateMeasure() {
  int   i, j;

  try {
    // allocate measure two-dimensional array
    gData.m_pMeasure = (double**)Smalloc((gData.m_nTimeIntervals+1) * sizeof(measure_t *), &gPrintDirection);
    memset(gData.m_pMeasure, 0, (gData.m_nTimeIntervals+1) * sizeof(measure_t *));
    for (i=0; i < gData.m_nTimeIntervals; i++)
       gData.m_pMeasure[i] = (double*)Smalloc(gData.m_nTracts * sizeof(measure_t), &gPrintDirection);

    gData.m_nTotalMeasure  = 0;
    gData.m_nTotalPop = 0;

    // set m_pMeasure[i] = S*T/C (expected number of cases in a time/tract)
    // S = number of cases in spacial area irrespective of time
    // T = number of cases in temporal domain irrespective of location
    // C = total number of cases
    for (i=0; i < gData.m_nTimeIntervals; i++) {
       gData.m_pMeasure[i][0] = 0;
       // Since all tracts in interval i will have the same T/C, just add up
       // cases for all tracts in tract 0 then divide by C. After we'll distribute
       // T/C to rest of tracts in interval i and at the same, multiply by S.
       for (j=0; j< gData.m_nTracts; j++)
          gData.m_pMeasure[i][0] += gData.m_pCases[i][j];
       //divide by total number of cases
       gData.m_pMeasure[i][0] /= gData.m_nTotalCases;
      // Copy to rest of measure slots for interval i and rest of tracts while
      // multiplying each by S at the same time.
      for (j=1; j< gData.m_nTracts; j++)
         gData.m_pMeasure[i][j] = gData.m_pMeasure[i][0] * gData.m_pCases[0][j];
      // don't forget to multiply tract 0 by S
      gData.m_pMeasure[i][0] = gData.m_pMeasure[i][0] * gData.m_pCases[0][0];
    }

    // calculate total measure
    for (j=0; j< gData.m_nTracts; j++)
       gData.m_nTotalMeasure += gData.m_pMeasure[0][j];

    /* Ensure that TotalCases=TotalMeasure */
    if (fabs(gData.m_nTotalCases - gData.m_nTotalMeasure)>0.0001)
      SSGenerateException("Error: The total measure is not equal to the total number of cases.\n"
                          "Total Cases = %i, Total Measure = %.2lf\n", "CalculateMeasure()",
                          gData.m_nTotalCases, gData.m_nTotalMeasure);
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
  count_t   N = gData.m_nTotalCases;
  measure_t U = gData.m_nTotalMeasure;

  return N*log(N/U);
}

/** Allocates randomization structures used by MakeData() routine. Must be called
prior to called MakeData(). */
void CSpaceTimePermutationModel::InitializeRandomizationStructures() {
  int	                i, j, k, iNumCases;
  std::vector<int>      vCasesAlreadyRecorded;

  try
     {
     //first calculate total number of cases
     gData.m_nTotalCases=0;
     for (j=0; j < gData.m_nTracts; j++)
        gData.m_nTotalCases += gData.m_pCases[0][j];

     //reserve vector space 
     gvCaseLocationTimes.reserve(gData.m_nTotalCases);
     vCasesAlreadyRecorded.reserve(gData.m_nTracts);
     for (i=0; i < gData.m_nTracts; i++)
        vCasesAlreadyRecorded[i] = 0;

     for (i=gData.m_nTimeIntervals - 1; i >= 0;i--)
        for (j=0; j < gData.m_nTracts; j++)
           {
           iNumCases = gData.m_pCases[i][j] - vCasesAlreadyRecorded[j];
           for (k=0; k < iNumCases; k++)
              gvCaseLocationTimes.push_back(CCaseLocationTimes(i,j));
           vCasesAlreadyRecorded[j] += iNumCases;
           }

      // allocate time randomizer
      gvTimeIntervalRandomizer.reserve(gData.m_nTotalCases);
      for (i=0; i < gData.m_nTotalCases; i++ )
         gvTimeIntervalRandomizer.push_back(CSimulationTimeRandomizer());
      }
   catch (ZdException & x)
      {
      x.AddCallpath("InitializeRandomizationStructures()", "CSpaceTimePermutationModel");
      gvCaseLocationTimes.clear();
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
void CSpaceTimePermutationModel::MakeData(int iSimulationNumber)
{
  int           i, k;
  size_t        t, RandomizerSize;

   try
      {
      //reset seed to simulation number
      gRandomNumberGenerator.SetSeed(iSimulationNumber + gRandomNumberGenerator.GetDefaultSeed());

      // reset simulation cases to zero
      for (i=0; i < gData.m_nTimeIntervals; i++)
         for (k=0; k < gData.m_nTotalTractsAtStart; k++)
            gData.m_pSimCases[i][k] = 0;

      RandomizerSize = gvTimeIntervalRandomizer.size();
      for (t=0; t < RandomizerSize; t++)
         {
         // assign random number
         gvTimeIntervalRandomizer[t].SetRandomNumber(gRandomNumberGenerator.GetRandomFloat());
         // assign tract index to original order
         gvTimeIntervalRandomizer[t].SetTimeIntervalIndex(gvCaseLocationTimes[t].GetTimeIntervalIndex());
         }
      // sort based on random number   
      std::sort(gvTimeIntervalRandomizer.begin(), gvTimeIntervalRandomizer.end(), CompareSimulationTimeRandomizer());
      // re-assign simulation data
      for (t=0; t < RandomizerSize; t++)
         for (k=gvTimeIntervalRandomizer[t].GetTimeIntervalIndex(); k >= 0; k--)
             gData.m_pSimCases[k][gvCaseLocationTimes[t].GetTractIndex()]++;
      }
  catch (ZdException & x)
      {
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
    if ((gParameters.GetMaxGeographicClusterSizeType() == PERCENTAGEOFMEASURETYPE || gParameters.UseMaxCirclePopulationFile())
        && !gData.ReadMaxCirclePopulationFile())
       return false;
    if (! gData.ReadCaseFile())
      return false;
    if (gParameters.UseSpecialGrid() && !gData.ReadGridFile())
      return false;
    InitializeRandomizationStructures();
  }
  catch (ZdException & x) {
    x.AddCallpath("ReadData()", "CSpaceTimePermutationModel");
    throw;
  }
  return true;
}

