#include "SaTScan.h"
#pragma hdrstop
#include "SignificantRatios05.h"

CSignificantRatios05::CSignificantRatios05(int nReplications, BasePrint *pPrintDirection)
{
   try
      {
      m_nTotalReplications = nReplications;
      m_nRatios     = (int)(ceil((m_nTotalReplications+1)*0.05));
      m_pRatiosList = (double*) Smalloc((m_nRatios+1) * sizeof(double), pPrintDirection);
      Initialize();
      }
   catch (SSException & x)
      {
      x.AddCallpath("CSignificantRatios05()", "CSignificantRatios05");
      throw;
      }
}

CSignificantRatios05::~CSignificantRatios05()
{
   free(m_pRatiosList);
}

void CSignificantRatios05::Initialize()
{
   memset(m_pRatiosList, 0, (m_nRatios+1)*sizeof(double));
}

bool CSignificantRatios05::AddRatio(double r)
{
   int  i;
   bool bAdded = false;

   try
      {
      if (m_nTotalReplications >= 19)
         {
         if (r > m_pRatiosList[m_nRatios-1])
            {
            bAdded = true;
            for (i=m_nRatios-1; i>0 && r>m_pRatiosList[i-1]; i--)
               m_pRatiosList[i] =  m_pRatiosList[i-1];
            m_pRatiosList[i] = r;
            }
         }
      }
   catch (SSException & x)
      {
      x.AddCallpath("CSignificantRatios05()", "CSignificantRatios05");
      throw;
      }
  return bAdded;
}

const double CSignificantRatios05::GetAlpha01()  const
{
   return (m_pRatiosList[(m_nRatios/5)-1]);
}

const double CSignificantRatios05::GetAlpha05() const
{
   return (m_pRatiosList[m_nRatios-1]);
}




