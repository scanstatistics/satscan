// MeasureList.cpp

#include "MeasureList.h"
//#include "rate.h"
//#include "loglikelihood.h"
#include "Model.h"
#include "display.h"
#include "error.h"
#include <malloc.h>

CMeasureList::CMeasureList(count_t N)
{
  m_nListSize = N+1;
}

CMeasureList::~CMeasureList()
{
}

CMinMeasureList::CMinMeasureList(count_t N, measure_t U)
                :CMeasureList(N)
{
  try
  {
    m_pMinMeasures = new measure_t [m_nListSize];
  }
  catch (...)
  {
    printf("  Error: Unable to allocate sufficient memory.\n");
    printf("         Please see 'memory requirements' in the help file.\n");
    FatalError(0);
  }

  for (int i=0; i<m_nListSize; i++)
    m_pMinMeasures[i] = (U*i)/N;
}

CMinMeasureList::~CMinMeasureList()
{
  delete [] m_pMinMeasures;
}

void CMinMeasureList::AddMeasure(count_t n, measure_t u)
{
  if (m_pMinMeasures[n] > u)
    m_pMinMeasures[n] = u;
}

//double CMinMeasureList::GetMaxLogLikelihood(count_t N, measure_t U, double& nMaxLogLikelihood)
double CMinMeasureList::GetMaxLogLikelihood(const CSaTScanData& Data)
{
  double nLogLikelihood;
  double nMaxLogLikelihood = Data.m_pModel->GetLogLikelihoodForTotal();

  for (int i = 0; i < m_nListSize; i++)
  {
//    if (HighRate(i, m_pMinMeasures[i], N, U))
    if (m_pMinMeasures[i] != 0 &&
        i*Data.m_nTotalMeasure > m_pMinMeasures[i]*Data.m_nTotalCases)
    {
      nLogLikelihood = Data.m_pModel->CalcLogLikelihood(i, m_pMinMeasures[i]);
      if (nLogLikelihood > nMaxLogLikelihood)
        nMaxLogLikelihood = nLogLikelihood;
    }

  }

  return (nMaxLogLikelihood);
}

void CMinMeasureList::Display(FILE* pFile)
{
  fprintf(pFile, "Min Measure List\n");
  for (int i=0; i<m_nListSize; i++)
    fprintf(pFile, "m_pMinMeasures[%i] = %f\n", i, m_pMinMeasures[i]);
}

CMaxMeasureList::CMaxMeasureList(count_t N, measure_t U)
                :CMeasureList(N)
{
  try
  {
    m_pMaxMeasures = new measure_t [m_nListSize];
  }
  catch (...)
  {
    printf("  Error: Unable to allocate sufficient memory.\n");
    printf("         Please see 'memory requirements' in the help file.\n");
    FatalError(0);
  }

  for (int i=0; i<m_nListSize; i++)
    m_pMaxMeasures[i] = (U*i)/N;
}

CMaxMeasureList::~CMaxMeasureList()
{
  delete [] m_pMaxMeasures;
}

void CMaxMeasureList::AddMeasure(count_t n, measure_t u)
{
  if (m_pMaxMeasures[n] < u)
    m_pMaxMeasures[n] = u;
}

//double CMaxMeasureList::GetMaxLogLikelihood(count_t N, measure_t U, double& nMaxLogLikelihood)
double CMaxMeasureList::GetMaxLogLikelihood(const CSaTScanData& Data)
{
  double nLogLikelihood;
  double nMaxLogLikelihood = Data.m_pModel->GetLogLikelihoodForTotal();

  for (int i = 0; i < m_nListSize; i++)
  {
//    if (LowRate(i, m_pMaxMeasures[i], N, U))
    if (m_pMaxMeasures[i] != 0 &&
        i*Data.m_nTotalMeasure < m_pMaxMeasures[i]*Data.m_nTotalCases)
    {
      nLogLikelihood = Data.m_pModel->CalcLogLikelihood(i, m_pMaxMeasures[i]);
      if (nLogLikelihood > nMaxLogLikelihood)
        nMaxLogLikelihood = nLogLikelihood;
    }

  }

  return (nMaxLogLikelihood);
}

void CMaxMeasureList::Display(FILE* pFile)
{
  fprintf(pFile, "Max Measure List\n");
  for (int i=0; i<m_nListSize; i++)
    fprintf(pFile, "m_pMaxMeasures[%i] = %f\n", i, m_pMaxMeasures[i]);
}

CMinMaxMeasureList::CMinMaxMeasureList(count_t N, measure_t U)
                   :CMeasureList(N)
{
  try
  {
    m_pMinMeasures = new measure_t [m_nListSize];
    m_pMaxMeasures = new measure_t [m_nListSize];
  }
  catch (...)
  {
    printf("  Error: Unable to allocate sufficient memory.\n");
    printf("         Please see 'memory requirements' in the help file.\n");
    FatalError(0);
  }

  for (int i=0; i<m_nListSize; i++)
  {
    m_pMinMeasures[i] = (U*i)/N;
    m_pMaxMeasures[i] = (U*i)/N;
  }
}

CMinMaxMeasureList::~CMinMaxMeasureList()
{
  delete [] m_pMinMeasures;
  delete [] m_pMaxMeasures;
}

void CMinMaxMeasureList::AddMeasure(count_t n, measure_t u)
{
  if (m_pMinMeasures[n] > u)
    m_pMinMeasures[n] = u;
  if (m_pMaxMeasures[n] < u)
    m_pMaxMeasures[n] = u;

//  printf("n = %i, u = %f, m_pMaxMeasures[0] = %f\n",
//          n, u, m_pMaxMeasures[0]);
//  HoldForEnter();
}

//double CMinMaxMeasureList::GetMaxLogLikelihood(count_t N, measure_t U, double& nMaxLogLikelihood)
double CMinMaxMeasureList::GetMaxLogLikelihood(const CSaTScanData& Data)
{
  double nLogLikelihood;
  double nMaxLogLikelihood = Data.m_pModel->GetLogLikelihoodForTotal();

  for (int i = 0; i < m_nListSize; i++)
  {
//    if (HighRate(i, m_pMinMeasures[i], N, U))
    if (m_pMinMeasures[i] != 0 &&
        i*Data.m_nTotalMeasure > m_pMinMeasures[i]*Data.m_nTotalCases)
    {
      nLogLikelihood = Data.m_pModel->CalcLogLikelihood(i, m_pMinMeasures[i]);
      if (nLogLikelihood > nMaxLogLikelihood)
        nMaxLogLikelihood = nLogLikelihood;
    }

//    if (LowRate(i, m_pMaxMeasures[i], N, U))
    if (m_pMaxMeasures[i] != 0 &&
        i*Data.m_nTotalMeasure < m_pMaxMeasures[i]*Data.m_nTotalCases)
    {
      nLogLikelihood = Data.m_pModel->CalcLogLikelihood(i, m_pMaxMeasures[i]);
      if (nLogLikelihood > nMaxLogLikelihood)
        nMaxLogLikelihood = nLogLikelihood;
    }

  }

  return (nMaxLogLikelihood);
}

void CMinMaxMeasureList::Display(FILE* pFile)
{
  int i;

  fprintf(pFile, "Min Measure List\n");
  for (i=0; i<m_nListSize; i++)
    fprintf(pFile, "m_pMinMeasures[%i] = %f\n", i, m_pMinMeasures[i]);
  fprintf(pFile, "\n");

  fprintf(pFile, "Max Measure List\n");
  for (i=0; i<m_nListSize; i++)
    fprintf(pFile, "m_pMaxMeasures[%i] = %f\n", i, m_pMaxMeasures[i]);
  fprintf(pFile, "\n");

}


