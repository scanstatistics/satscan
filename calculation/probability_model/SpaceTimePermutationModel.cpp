#include "SaTScan.h"
#pragma hdrstop
#include "SpaceTimePermutationModel.h"

/** constructor */
CSpaceTimePermutationModel::CSpaceTimePermutationModel(CParameters* pParameters, CSaTScanData* pData, BasePrint *pPrintDirection)
                           :CModel(pParameters, pData, pPrintDirection) {
   Init();
}

/** destructor */
CSpaceTimePermutationModel::~CSpaceTimePermutationModel() {}

/** Calculates loglikelihood, this routine is identical to Possion model. */
double CSpaceTimePermutationModel::CalcLogLikelihood(count_t n, measure_t u)
{
   double    nLogLikelihood;
   count_t   N = m_pData->m_nTotalCases;
   measure_t U = m_pData->m_nTotalMeasure;

   try
      {
      if (n != N && n != 0)
        nLogLikelihood = n*log(n/u) + (N-n)*log((N-n)/(U-u));
      else if (n == 0)
        nLogLikelihood = (N-n) * log((N-n)/(U-u));
      else
        nLogLikelihood = n*log(n/u);
      }
   catch (SSException & x)
      {
      x.AddCallpath("CalcLogLikelihood()", "CSpaceTimePermutationModel");
      throw;
      }
   return (nLogLikelihood);
}

/** Same log likelihood as for the Poisson model.
    It is no longer a "likelihood", but it will serve the same purpose.
    Martin Kulldorph derived the true log likelihoods, but they are too complex
    and there is no way to implement them in a time efficient way, so using them
    would  lead to slow execution times. The Poisson model is a very good
    approximation of the likelihood, and that can be shown mathematically. */
double CSpaceTimePermutationModel::GetLogLikelihoodForTotal() const
{
  count_t   N = m_pData->m_nTotalCases;
  measure_t U = m_pData->m_nTotalMeasure;

  return N*log(N/U);
}

/** Throws exception. Defined in parent class as pure virtual. */
double CSpaceTimePermutationModel::CalcMonotoneLogLikelihood(const CPSMonotoneCluster& PSMCluster)
{
  SSGenerateException("Function CalcMonotoneLogLikelihood() not implemented in CSpaceTimePermutationModel",
                      "CalcMonotoneLogLikelihood() of CSpaceTimePermutationModel");
  return 0;
}

/** Calculates measure for the purpose of constructing neighbor array. If the
    maximum spatial cluster size is defined in terms of population, then m_pMeasure
    is constructed using same method as Poisson model. Otherwise m_pMeasure is
    constructed by determing expected number of cases through case and geographical
    information only.*/
bool CSpaceTimePermutationModel::CalculateMeasure()
{
   bool bResult;
   int  i;

   try
      {
      if (m_pData->m_pParameters->m_nMaxSpatialClusterSizeType == PERCENTAGEOFMEASURETYPE)
        {
         bResult = AssignMeasure(m_pData->GetTInfo(), m_pData->GetCats(), m_pData->m_pCases,
                                 m_pData->m_pTimes, m_pData->m_nTracts, m_pData->m_nStartDate,
                                 m_pData->m_nEndDate, m_pData->m_pIntervalStartTimes,
                                 m_pParameters->m_bExactTimes, m_pParameters->m_nTimeAdjustType,
                                 m_pParameters->m_nTimeAdjPercent, m_pData->m_nTimeIntervals,
                                 m_pParameters->m_nIntervalUnits,  m_pParameters->m_nIntervalLength,
                                 &m_pData->m_pMeasure, &m_pData->m_nTotalCases,
                                 &m_pData->m_nTotalPop, &m_pData->m_nTotalMeasure, gpPrintDirection);
         m_eMeasureType = PopulationBased;
        }
      else
        {
        // allocate measure two-dimensional array
        m_pData->m_pMeasure = (double**)Smalloc((m_pData->m_nTimeIntervals+1) * sizeof(measure_t *), gpPrintDirection);
        for (i=0; i < m_pData->m_nTimeIntervals+1; i++)
          m_pData->m_pMeasure[i] = (double*)Smalloc(m_pData->m_nTracts * sizeof(measure_t), gpPrintDirection);

        bResult = ReCalculateMeasure();
        m_eMeasureType = CaseBased;                        
        }
      }
   catch (SSException & x)
      {
      x.AddCallpath("CalculateMeasure()", "CPoissonModel");
      throw;
      }
   return bResult;
}

/** Allocates randomization structures used by MakeData() routine. Must be called
prior to called MakeData(). */
void CSpaceTimePermutationModel::InitializeRandomizationStructures() {
  int	                i, j, k, iNumCases;
  std::vector<int>      vCasesAlreadyRecorded;

  try
     {
     //first calculate total number of cases
     m_pData->m_nTotalCases=0;
     for (j=0; j < m_pData->m_nTracts; j++)
        m_pData->m_nTotalCases += m_pData->m_pCases[0][j];

     //reserve vector space 
     m_vCaseLocationTimes.reserve(m_pData->m_nTotalCases);
     vCasesAlreadyRecorded.reserve(m_pData->m_nTracts);
     for (i=0; i < m_pData->m_nTracts; i++)
        vCasesAlreadyRecorded[i] = 0;

     for (i=m_pData->m_nTimeIntervals - 1; i >= 0;i--)
        for (j=0; j < m_pData->m_nTracts; j++)
           {
           iNumCases = m_pData->m_pCases[i][j] - vCasesAlreadyRecorded[j];
           for (k=0; k < iNumCases; k++)
              m_vCaseLocationTimes.push_back(CCaseLocationTimes(i,j));
           vCasesAlreadyRecorded[j] += iNumCases;
           }

      // allocate time randomizer
      m_vTimeIntervalRandomizer.reserve(m_pData->m_nTotalCases);
      for (i=0; i < m_pData->m_nTotalCases; i++ )
         m_vTimeIntervalRandomizer.push_back(CSimulationTimeRandomizer());
      }
   catch (SSException & x)
      {
      x.AddCallpath("InitializeRandomizationStructures()", "CSpaceTimePermutationModel");
      m_vCaseLocationTimes.clear();
      m_vTimeIntervalRandomizer.clear();
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
void CSpaceTimePermutationModel::MakeData()
{
  int           i, k;
  size_t        t, RandomizerSize;

   try
      {
      // reset simulation cases to zero
      for (i=0; i < m_pData->m_nTimeIntervals; i++)
         for (k=0; k < m_pData->m_nTracts; k++)
            m_pData->m_pSimCases[i][k] = 0;

      RandomizerSize = m_vTimeIntervalRandomizer.size();
      for (t=0; t < RandomizerSize; t++)
         {
         // assign random number
         m_vTimeIntervalRandomizer[t].SetRandomNumber(rngRand());
         // assign tract index to original order
         m_vTimeIntervalRandomizer[t].SetTimeIntervalIndex(m_vCaseLocationTimes[t].GetTimeIntervalIndex());
         }
      // sort based on random number   
      std::sort(m_vTimeIntervalRandomizer.begin(), m_vTimeIntervalRandomizer.end(), CompareSimulationTimeRandomizer());
      // re-assign simulation data
      for (t=0; t < RandomizerSize; t++)
         for (k=m_vTimeIntervalRandomizer[t].GetTimeIntervalIndex(); k >= 0; k--)
             m_pData->m_pSimCases[k][m_vCaseLocationTimes[t].GetTractIndex()]++;
      }
  catch (SSException & x)
      {
      x.AddCallpath("MakeData()", "CSpaceTimePermutationModel");
      throw;
      }
}

/** Reads data from files(geographical, case and special grid) into data structures.
    Initializes randomization structures. */
bool CSpaceTimePermutationModel::ReadData()
{
   try
      {
      if (!m_pData->ReadGeo())
        return false;

      if (m_pData->m_pParameters->m_nMaxSpatialClusterSizeType == PERCENTAGEOFMEASURETYPE)
        if (!m_pData->ReadPops())
          return false;

      if (!m_pData->ReadCounts(m_pParameters->m_szCaseFilename, "case", &m_pData->m_pCases))
        return false;
    
      if (m_pParameters->m_bSpecialGridFile && !m_pData->ReadGrid())
        return false;

      InitializeRandomizationStructures();  
      }
   catch (SSException & x)
      {
      x.AddCallpath("ReadData()", "CSpaceTimePermutationModel");
      throw;
      }
   return true;
}

/** Determines the expected number of cases for each time interval/tract.
    Assigns values to CSatScanData::m_pMeasure array. Calculates total measure
    and validates that total measure equals total number of cases in set. */
bool CSpaceTimePermutationModel::ReCalculateMeasure()
{
  int   i, j;

  try
     {
     // Only recalculate measure if current measure setting isn't case based. 
     if (m_eMeasureType != CaseBased)
       {
       if (! m_pData->m_pMeasure)
         SSGenerateException("m_pMeasure array not allocated.", "ReCalculateMeasure()");
       m_pData->m_nTotalMeasure  = 0;

       // set m_pMeasure[i] = S*T/C (expected number of cases in a time/tract)
       // S = number of cases in spacial area irrespective of time
       // T = number of cases in temporal domain irrespective of location
       // C = total number of cases
       for (i=0; i < m_pData->m_nTimeIntervals; i++)
          {
          m_pData->m_pMeasure[i][0] = 0;
          // Since all tracts in interval i will have the same T/C, just add up
          // cases for all tracts in tract 0 then divide by C. After we'll distribute
          // T/C to rest of tracts in interval i and at the same, multiply by S.
          for(j=0; j< m_pData->m_nTracts; j++)
             m_pData->m_pMeasure[i][0] += m_pData->m_pCases[i][j];
          //divide by total number of cases
          m_pData->m_pMeasure[i][0] /= m_pData->m_nTotalCases;
          // Copy to rest of measure slots for interval i and rest of tracts will
          // multiplying each by S at the same time.
          for (j=1; j< m_pData->m_nTracts; j++)
             m_pData->m_pMeasure[i][j] = m_pData->m_pMeasure[i][0] * m_pData->m_pCases[0][j];
          // don't forget to multiply tract 0 by S
          m_pData->m_pMeasure[i][0] = m_pData->m_pMeasure[i][0] * m_pData->m_pCases[0][0];
          }

       // calculate total measure
       for (j=0; j< m_pData->m_nTracts; j++)
             m_pData->m_nTotalMeasure += m_pData->m_pMeasure[0][j];

       /* Ensure that TotalCases=TotalMeasure */
       if (fabs(m_pData->m_nTotalCases - m_pData->m_nTotalMeasure)>0.0001)
         {
         string sMessage;
         sMessage = "\nError: The total measure is not equal to the total number of cases.";
         sMessage += "\nTotalCases="; sMessage += m_pData->m_nTotalCases;
         sMessage += ", TotalMeasure="; sMessage += m_pData->m_nTotalMeasure; sMessage += "\n";
         SSGenerateException(sMessage.c_str(), "ReCalculateMeasure()");
         }
       }
      }
   catch (SSException & x)
      {
      x.AddCallpath("ReCalculateMeasure()", "CSpaceTimePermutationModel");
      throw;
      }
   return true;
}