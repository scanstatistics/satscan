#include "SaTScan.h"
#pragma hdrstop
#include "MeasureList.h"

/** Constructor */
CMeasureList::CMeasureList(count_t N, BasePrint *pPrintDirection) {
  m_nListSize = N+1;
  gpPrintDirection = pPrintDirection;
}

/** Destructor */
CMeasureList::~CMeasureList() {}

/** Constructor */
CMinMeasureList::CMinMeasureList(count_t N, measure_t U, BasePrint *pPrintDirection) : CMeasureList(N, pPrintDirection) {
  try {
    m_pMinMeasures = new measure_t [m_nListSize];
    for (int i=0; i < m_nListSize; i++)
       m_pMinMeasures[i] = (U * i) / N;
  }
  catch (ZdException & x) {
    x.AddCallpath("CMinMeasureList()", "CMinMeasureList");
    throw;
  }
}

/** Destructor */
CMinMeasureList::~CMinMeasureList() {
  try {
    delete[] m_pMinMeasures;
  }
  catch(...){}  
}

/** Returns maximum loglikelihood.
    Does not evaluate loglikelihood for zero and one cases.*/
double CMinMeasureList::GetMaxLogLikelihood(const CSaTScanData& Data) {
  int    i;
  double nLogLikelihood;
  double nMaxLogLikelihood = Data.m_pModel->GetLogLikelihoodForTotal();

  i = 2; //start index at two -- we don't want to consider simulation with one case
         //since this could indicate a false high loglikelihood. 
  for (;i < m_nListSize; i++) {
     if (m_pMinMeasures[i] != 0 && i * Data.m_nTotalMeasure > m_pMinMeasures[i] * Data.m_nTotalCases) {
        nLogLikelihood = Data.m_pModel->CalcLogLikelihood(i, m_pMinMeasures[i]);
        if (nLogLikelihood > nMaxLogLikelihood)
          nMaxLogLikelihood = nLogLikelihood;
     }
  }
  return (nMaxLogLikelihood);
}

void CMinMeasureList::Display(FILE* pFile) {
  fprintf(pFile, "Min Measure List\n");
  for (int i=0; i<m_nListSize; i++)
     fprintf(pFile, "m_pMinMeasures[%i] = %f\n", i, m_pMinMeasures[i]);
}

/** Constructor */
CMaxMeasureList::CMaxMeasureList(count_t N, measure_t U, BasePrint *pPrintDirection) : CMeasureList(N, pPrintDirection) {
  try {
    m_pMaxMeasures = new measure_t [m_nListSize];
    for (int i=0; i < m_nListSize; i++)
       m_pMaxMeasures[i] = (U * i) / N;
  }
  catch (ZdException & x) {
    x.AddCallpath("CMaxMeasureList()", "CMaxMeasureList");
    throw;
  }
}

/** Destructor */
CMaxMeasureList::~CMaxMeasureList() {
  try {
    delete[] m_pMaxMeasures;
  }
  catch(...){}  
}

/** Returns maximum loglikelihood. */
double CMaxMeasureList::GetMaxLogLikelihood(const CSaTScanData& Data) {
  double nLogLikelihood;
  double nMaxLogLikelihood = Data.m_pModel->GetLogLikelihoodForTotal();

  for (int i=0; i < m_nListSize; i++) {
     if (m_pMaxMeasures[i] != 0 && i * Data.m_nTotalMeasure < m_pMaxMeasures[i] * Data.m_nTotalCases) {
       nLogLikelihood = Data.m_pModel->CalcLogLikelihood(i, m_pMaxMeasures[i]);
       if (nLogLikelihood > nMaxLogLikelihood)
         nMaxLogLikelihood = nLogLikelihood;
     }
  }
  return (nMaxLogLikelihood);
}

void CMaxMeasureList::Display(FILE* pFile) {
  fprintf(pFile, "Max Measure List\n");
  for (int i=0; i<m_nListSize; i++)
    fprintf(pFile, "m_pMaxMeasures[%i] = %f\n", i, m_pMaxMeasures[i]);
}

/** Constructor */
CMinMaxMeasureList::CMinMaxMeasureList(count_t N, measure_t U, BasePrint *pPrintDirection)
                   :CMeasureList(N, pPrintDirection) {
  try {
    m_pMinMeasures=0; m_pMaxMeasures=0;
    m_pMinMeasures = new measure_t [m_nListSize];
    m_pMaxMeasures = new measure_t [m_nListSize];
    for (int i=0; i < m_nListSize; i++) {
       m_pMinMeasures[i] = (U * i) / N;
       m_pMaxMeasures[i] = (U * i) / N;
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("CMinMaxMeasureList()", "CMinMaxMeasureList");
    delete m_pMinMeasures;
    delete m_pMaxMeasures;
    throw;
  }
}

/** Destructor */
CMinMaxMeasureList::~CMinMaxMeasureList() {
  try {
    delete[] m_pMinMeasures;
    delete[] m_pMaxMeasures;
  }
  catch(...){}  
}

/** Returns maximum loglikelihood.
    Skips calculation of one case situation like CMinMeasureList. */
double CMinMaxMeasureList::GetMaxLogLikelihood(const CSaTScanData& Data) {
  double nLogLikelihood;
  double nMaxLogLikelihood = Data.m_pModel->GetLogLikelihoodForTotal();

  for (int i=0; i < m_nListSize; i++) {
     if (i > 1 && m_pMinMeasures[i] != 0 && i * Data.m_nTotalMeasure > m_pMinMeasures[i] * Data.m_nTotalCases) {
       nLogLikelihood = Data.m_pModel->CalcLogLikelihood(i, m_pMinMeasures[i]);
       if (nLogLikelihood > nMaxLogLikelihood)
         nMaxLogLikelihood = nLogLikelihood;
     }

     if (m_pMaxMeasures[i] != 0 && i * Data.m_nTotalMeasure < m_pMaxMeasures[i] * Data.m_nTotalCases) {
       nLogLikelihood = Data.m_pModel->CalcLogLikelihood(i, m_pMaxMeasures[i]);
       if (nLogLikelihood > nMaxLogLikelihood)
         nMaxLogLikelihood = nLogLikelihood;
     }
  }
  return (nMaxLogLikelihood);
}

void CMinMaxMeasureList::Display(FILE* pFile) {
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


