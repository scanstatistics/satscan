// data.cpp

#include "data.h"
#include "PoissonModel.h"
#include "BernoulliModel.h"
#include "date.h"
#include "display.h"
#include "error.h"
#include "tinfo.h"
#include "ginfo.h"
#include "calcmsr.h"
#include "cats.h"
#include "makeneighbors.h"
#include "getword.h"

CSaTScanData::CSaTScanData(CParameters* pParameters)
{
  m_pParameters = pParameters;

  switch (m_pParameters->m_nModel)
  {
    case POISSON   : m_pModel = new CPoissonModel(pParameters, this);   break;
    case BERNOULLI : m_pModel = new CBernoulliModel(pParameters, this); break;
  }

  m_pCases     = 0;
  m_pControls  = 0;
  m_pMeasure   = 0;
  m_pPTCases   = 0;
  m_pPTMeasure = 0;

  m_nTotalCases    = 0;
  m_nTotalControls = 0;
  m_nTotalMeasure  = 0;

  m_nAnnualRatePop = 100000;
}

CSaTScanData::~CSaTScanData()
{
  delete m_pModel;

  tiCleanup();
  giCleanup();
  catCleanup();

  int i;

  for (i = 0; i < m_nTimeIntervals; i++)
    free(m_pCases[i]);
  free(m_pCases);

  if (m_pControls != NULL)
  {
    for (i = 0; i < m_nTimeIntervals; i++)
      free(m_pControls[i]);
    free(m_pControls);
  }

  for (i = 0; i < m_nTimeIntervals+1; i++)   // Why allocated +1?
    free(m_pMeasure[i]);
  free(m_pMeasure);

  if (m_pPTCases != 0)
    free(m_pPTCases);
  if (m_pPTMeasure != 0)
    free(m_pPTMeasure);

  for (i = 0; i < m_nGridTracts; i++)
    free(m_pSorted[i]);
  free(m_pSorted);

  free(m_NeighborCounts);

  free(m_pIntervalStartTimes);
}

void CSaTScanData::SetStartAndEndDates()
{
  m_nStartDate = CharToJulian(m_pParameters->m_szStartDate);
  m_nEndDate   = CharToJulian(m_pParameters->m_szEndDate);
}

void CSaTScanData::SetNumTimeIntervals()
{
  long nTime = TimeBetween(m_nStartDate,
                           m_nEndDate,
                           m_pParameters->m_nIntervalUnits);

  m_nTimeIntervals = (int)ceil((float) nTime /
                               (float) m_pParameters->m_nIntervalLength);
}

void CSaTScanData::SetIntervalCut()
{
  /* Calculates the number of time intervals to include in potential clusters */
  /* without exceeding the maximum cluster size with respect to time.         */
  /*  printf("Calculate number of time intervals...\n"); /*KR-6/22/97*/

  if (m_nTimeIntervals == 1)
    m_nIntervalCut = 1;
  else if (m_nTimeIntervals > 1)
  {
    m_nIntervalCut = 0;
    while ((m_nIntervalCut+1)*m_pParameters->m_nIntervalLength <=
           (TimeBetween(m_nStartDate, m_nEndDate, m_pParameters->m_nIntervalUnits))*m_pParameters->m_nMaxTemporalClusterSize/100.0)
      m_nIntervalCut++;
  }

  if (m_nIntervalCut==0)
  {
    printf("  Error: The interval length is longer than the maximum cluster size with\n");
    printf("         respect to time. No clusters can be found.\n");
    FatalError("");
  }
}

void CSaTScanData::SetIntervalStartTimes()
{
  // Not neccessary for purely spatial?

  m_pIntervalStartTimes = (Julian*) Smalloc((m_nTimeIntervals+1)*sizeof(Julian));

  m_pIntervalStartTimes[0] = m_nStartDate;
  m_pIntervalStartTimes[m_nTimeIntervals] = m_nEndDate+1;

  for (int i = m_nTimeIntervals-1; i>0; i--)
  {
    m_pIntervalStartTimes[i] = DecrementDate(m_pIntervalStartTimes[i+1],
                                             m_pParameters->m_nIntervalUnits,
                                             m_pParameters->m_nIntervalLength);
  }

}

void CSaTScanData::ReadDataFromFiles()
{
  SetStartAndEndDates();
  SetNumTimeIntervals();
  SetIntervalCut();
  SetIntervalStartTimes();

  if (!m_pModel->ReadData())
  {
//    FatalError("\nProblem encountered reading in data.");
    FatalError(NULL);
  }

  if (tiFindDuplicateCoords(stderr))
    FatalError("Program canceled.\n");

  if (giFindDuplicateCoords(stderr))
    FatalError("Program canceled.\n");
}

bool CSaTScanData::CalculateMeasure()
{
  bool bReturn (m_pModel->CalculateMeasure());

  m_nTotalTractsAtStart   = m_nTracts;
  m_nTotalCasesAtStart    = m_nTotalCases;
  m_nTotalControlsAtStart = m_nTotalControls;
  m_nTotalMeasureAtStart  = m_nTotalMeasure;

  SetMaxCircleSize();

/*  if (!ValidateMeasures(Measure,	TotalMeasure, MaxCircleSize,
								NumTracts, nTimeIntervals, PStruct.nGeographicSize))
	 FatalError("Program Terminated.");
*/

  return bReturn;
}

bool CSaTScanData::FindNeighbors()
{
  printf("Constructing the circles\n\n");

  m_pSorted        = (tract_t**)Smalloc(m_nGridTracts * sizeof(tract_t *));
  m_NeighborCounts = (tract_t*)Smalloc(m_nGridTracts * sizeof(tract_t));

  for (int i=0; i<m_nGridTracts; i++)
    m_NeighborCounts[i] = 0;

  if (m_pParameters->m_bSequential)
    MakeNeighbors(m_pSorted,
                  m_nTracts,
                  m_nGridTracts,
                  m_pMeasure[0],
                  m_nMaxCircleSize,
                  m_nTotalMeasure,
                  m_NeighborCounts,
                  m_pParameters->m_nDimension);
  else
    MakeNeighbors(m_pSorted,
                  m_nTracts,
                  m_nGridTracts,
                  m_pMeasure[0],
                  m_nMaxCircleSize,
                  m_nMaxCircleSize,
                  m_NeighborCounts,
                  m_pParameters->m_nDimension);

  return true;
}

/**********************************************************************
 Return "nearness"-th closest neighbor to "t"
 (nearness == 1 returns "t").
 **********************************************************************/
tract_t CSaTScanData::GetNeighbor(tract_t t, unsigned int nearness) const
{
   return m_pSorted[t][nearness - 1];
}

void CSaTScanData::AllocSimCases()
{
  m_pSimCases = (count_t**)Smalloc(m_nTimeIntervals * sizeof(count_t *));
  for(int i=0;i<m_nTimeIntervals;i++)
    m_pSimCases[i] = (count_t*)Smalloc(m_nTracts * sizeof(count_t));
}

void CSaTScanData::DeAllocSimCases()
{
  for (int i = 0; i < m_nTimeIntervals; i++)
    free(m_pSimCases[i]);
  free(m_pSimCases);
}

/*void CSaTScanData::MakeData()
{
  printf("Make Data.\n");

  m_pSimCases[0][0] = 2;
  m_pSimCases[0][1] = 7;
  m_pSimCases[0][2] = 4;
  m_pSimCases[0][3] = 3;
  m_pSimCases[0][4] = 1;
  m_pSimCases[0][5] = 3;
}
*/

void CSaTScanData::SetPurelyTemporalCases()
{
  m_pPTCases = (count_t*) Smalloc((m_nTimeIntervals+1)*sizeof(count_t));

  for (int i=0; i<m_nTimeIntervals; i++)
  {
    m_pPTCases[i] = 0;

    for (int t=0; t<m_nTracts; t++)
      m_pPTCases[i] += m_pCases[i][t];
  }
}

void CSaTScanData::SetPurelyTemporalMeasures()
{
  m_pPTMeasure = (measure_t*)Smalloc((m_nTimeIntervals+1) * sizeof(measure_t));

  for (int i=0; i<m_nTimeIntervals; i++)
  {
    m_pPTMeasure[i] = 0;

    for (int t=0; t<m_nTracts; t++)
      m_pPTMeasure[i] += m_pMeasure[i][t];
  }
}

double CSaTScanData::GetAnnualRate() const
{
// Call to TimeBetween removed so results would match V.1.0.6
// Should TimeBetween be updated to use 365.2425?
// And do we need TimeBetween that returns a double? KR-980325
//  double nYears = TimeBetween(m_nStartDate, m_nEndDate, YEAR);
  double nYears      = (double)(m_nEndDate+1 - m_nStartDate) / 365.2425;

  double nAnnualRate = (m_nAnnualRatePop*(double)m_nTotalCases) /
                       ((double)m_nTotalPop*nYears);

  return nAnnualRate;
}

double CSaTScanData::GetAnnualRateAtStart() const
{
  double nYears      = (double)(m_nEndDate+1 - m_nStartDate) / 365.2425;

  double nAnnualRate = (m_nAnnualRatePop*(double)m_nTotalCasesAtStart) /
                       ((double)m_nTotalPop*nYears);

  return nAnnualRate;
}

void CSaTScanData::SetPurelyTemporalSimCases()
{
  for (int i=0; i<m_nTimeIntervals; i++)
  {
    m_pPTSimCases[i] = 0;

    for (int t=0; t<m_nTracts; t++)
      m_pPTSimCases[i] += m_pSimCases[i][t];
  }
}

// After each pass through sequential analysis, data on top cluster is
// removed and neighbor counts must be adjusted to include all neighbors
// within the maximum circle size.
void CSaTScanData::AdjustNeighborCounts()
{
  int i, j;
  measure_t nCumMeasure;

  for (i=0; i<m_nGridTracts; i++)
  {
    nCumMeasure = 0;
    for (j=1;
         j<=m_nGridTracts && nCumMeasure+m_pMeasure[0][GetNeighbor(i,j)] <= m_nMaxCircleSize;
         j++)
      nCumMeasure += m_pMeasure[0][GetNeighbor(i,j)];

    m_NeighborCounts[i] = j-1;
  }

}

void CSaTScanData::MakeData()
{
  m_pModel->MakeData();
}

//Measure Adjustment used when calculating relative risk/expected counts
//to disply in report file.
double CSaTScanData::GetMeasureAdjustment() const
{
  if (m_pParameters->m_nModel == POISSON)
    return 1.0;
  else if (m_pParameters->m_nModel == BERNOULLI)
    return (double)m_nTotalCases/(double)m_nTotalPop;
  else
    return 0.0;
}

