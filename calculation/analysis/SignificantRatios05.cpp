//***************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//***************************************************************************
#include "SignificantRatios05.h"

/** constructor */
CSignificantRatios05::CSignificantRatios05(bool bSuppress, unsigned int iReplications)
                     :m_bSuppressed(bSuppress), m_pRatiosList(0), m_nTotalReplications(0), m_nRatios(0) {
   try
      {
         if (!m_bSuppressed) {
           m_nTotalReplications = iReplications;
           m_nRatios     = (unsigned int)(ceil((m_nTotalReplications+1)*0.05));
           m_pRatiosList = (double*) Smalloc((m_nRatios+1) * sizeof(double));
           Initialize();
         }
      }
   catch (ZdException & x)
      {
      x.AddCallpath("CSignificantRatios05()", "CSignificantRatios05");
      throw;
      }
}

CSignificantRatios05::~CSignificantRatios05()
{
   if (!m_bSuppressed) free(m_pRatiosList);
}

void CSignificantRatios05::Initialize()
{
  if (!m_bSuppressed) memset(m_pRatiosList, 0, (m_nRatios+1)*sizeof(double));
}

bool CSignificantRatios05::AddRatio(double r)
{
   int  i;
   bool bAdded = false;

   if (!m_bSuppressed && m_nTotalReplications >= 19)
     {
    if (r > m_pRatiosList[m_nRatios-1])
       {
       bAdded = true;
       for (i=m_nRatios-1; i>0 && r>m_pRatiosList[i-1]; i--)
          m_pRatiosList[i] =  m_pRatiosList[i-1];
       m_pRatiosList[i] = r;
       }
     }

  return bAdded;
}

// this function should not be called for Monte Carlo reps less than 99 due to the fact
// that that seems to cause the ratio list to not have enough elements in it for the integer
// division by five to not return a zero and thus prevents the list from accessing a -1 element - AJV 10/03/2002
double CSignificantRatios05::GetAlpha01()  const
{
  return (m_bSuppressed ? 0 : (m_pRatiosList[(m_nRatios/5)-1]));
}

double CSignificantRatios05::GetAlpha05() const
{
  return (m_bSuppressed ? 0 : (m_pRatiosList[m_nRatios-1]));
}




