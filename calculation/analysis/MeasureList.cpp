#include "SaTScan.h"
#pragma hdrstop
#include "MeasureList.h"

CMeasureList::CMeasureList(count_t N, BasePrint *pPrintDirection)
{
  m_nListSize = N+1;
  gpPrintDirection = pPrintDirection;
}

CMeasureList::~CMeasureList()
{
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
CMinMeasureList::CMinMeasureList(count_t N, measure_t U, BasePrint *pPrintDirection)
                :CMeasureList(N, pPrintDirection)
{
   try
      {
       m_pMinMeasures = new measure_t [m_nListSize];
       if (! m_pMinMeasures)
          {
          SSGenerateException("  Error: Unable to allocate sufficient memory.\n         Please see 'memory requirements' in the help file.\n", "CMinMeasureList constructor");
          //FatalError(0, gpPrintDirection);
          }

      for (int i=0; i<m_nListSize; i++)
         m_pMinMeasures[i] = (U*i)/N;
      }
   catch (SSException & x)
      {
      x.AddCallpath("CMinMeasureList()", "CMinMeasureList");
      throw;
      }
}

CMinMeasureList::~CMinMeasureList()
{
  delete [] m_pMinMeasures;
}

//double CMinMeasureList::GetMaxLogLikelihood(count_t N, measure_t U, double& nMaxLogLikelihood)
double CMinMeasureList::GetMaxLogLikelihood(const CSaTScanData& Data)
{
   double nLogLikelihood;
   double nMaxLogLikelihood = Data.m_pModel->GetLogLikelihoodForTotal();

   try
      {
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
      }   
   catch (SSException & x)
      {
      x.AddCallpath("GetMaxLogLikelihood()", "CMinMeasureList");
      throw;
      }
   return (nMaxLogLikelihood);
}

void CMinMeasureList::Display(FILE* pFile)
{
  fprintf(pFile, "Min Measure List\n");
  for (int i=0; i<m_nListSize; i++)
    fprintf(pFile, "m_pMinMeasures[%i] = %f\n", i, m_pMinMeasures[i]);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
CMaxMeasureList::CMaxMeasureList(count_t N, measure_t U, BasePrint *pPrintDirection)
                :CMeasureList(N, pPrintDirection)
{
   try
      {
      m_pMaxMeasures = new measure_t [m_nListSize];
      if (! m_pMaxMeasures)
         {
         SSGenerateException("  Error: Unable to allocate sufficient memory.\n         Please see 'memory requirements' in the help file.\n", "CMaxMeauseList constructor");
         //FatalError(0, gpPrintDirection);
         }
       for (int i=0; i<m_nListSize; i++)
          m_pMaxMeasures[i] = (U*i)/N;
       }
   catch (SSException & x)
      {
      x.AddCallpath("CMaxMeasureList()", "CMaxMeasureList");
      throw;
      }
}

CMaxMeasureList::~CMaxMeasureList()
{
  delete [] m_pMaxMeasures;
}

//double CMaxMeasureList::GetMaxLogLikelihood(count_t N, measure_t U, double& nMaxLogLikelihood)
double CMaxMeasureList::GetMaxLogLikelihood(const CSaTScanData& Data)
{
   double nLogLikelihood;
   double nMaxLogLikelihood = Data.m_pModel->GetLogLikelihoodForTotal();

   try
      {
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
      }
   catch (SSException & x)
      {
      x.AddCallpath("GetMaxLogLikelihood()", "CMaxMeasureList");
      throw;
      }

  return (nMaxLogLikelihood);
}

void CMaxMeasureList::Display(FILE* pFile)
{
  fprintf(pFile, "Max Measure List\n");
  for (int i=0; i<m_nListSize; i++)
    fprintf(pFile, "m_pMaxMeasures[%i] = %f\n", i, m_pMaxMeasures[i]);
}

//------------------------------------------------------------------------------
//
//------------------------------------------------------------------------------
CMinMaxMeasureList::CMinMaxMeasureList(count_t N, measure_t U, BasePrint *pPrintDirection)
                   :CMeasureList(N, pPrintDirection)
{
   try
      {
      m_pMinMeasures = new measure_t [m_nListSize];
      m_pMaxMeasures = new measure_t [m_nListSize];
      if ((!m_pMinMeasures) || (!m_pMaxMeasures))
         {
         SSGenerateException("  Error: Unable to allocate sufficient memory.\n         Please see 'memory requirements' in the help file.\n", "CMinMaxMeasureList constructor");
         //FatalError(0, gpPrintDirection);
         }
      for (int i=0; i<m_nListSize; i++)
         {
         m_pMinMeasures[i] = (U*i)/N;
         m_pMaxMeasures[i] = (U*i)/N;
         }
      }
   catch (SSException & x)
      {
      x.AddCallpath("CMinMaxMeasureList()", "CMinMaxMeasureList");
      throw;
      }
}

CMinMaxMeasureList::~CMinMaxMeasureList()
{
  delete [] m_pMinMeasures;
  delete [] m_pMaxMeasures;
}

//double CMinMaxMeasureList::GetMaxLogLikelihood(count_t N, measure_t U, double& nMaxLogLikelihood)
double CMinMaxMeasureList::GetMaxLogLikelihood(const CSaTScanData& Data)
{
   double nLogLikelihood;
   double nMaxLogLikelihood = Data.m_pModel->GetLogLikelihoodForTotal();

   try
      {
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
      }
   catch (SSException & x)
      {
      x.AddCallpath("GetMaxLogLikelihood()", "CMinMaxMeasureList");
      throw;
      }
   return (nMaxLogLikelihood);
}

void CMinMaxMeasureList::Display(FILE* pFile)
{
   int i;

   try
      {
      fprintf(pFile, "Min Measure List\n");
      for (i=0; i<m_nListSize; i++)
        fprintf(pFile, "m_pMinMeasures[%i] = %f\n", i, m_pMinMeasures[i]);
      fprintf(pFile, "\n");

      fprintf(pFile, "Max Measure List\n");
      for (i=0; i<m_nListSize; i++)
        fprintf(pFile, "m_pMaxMeasures[%i] = %f\n", i, m_pMaxMeasures[i]);
      fprintf(pFile, "\n");
      }
   catch (SSException & x)
      {
      x.AddCallpath("Display()", "CMinMaxMeasureList");
      throw;
      }
}


