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
   for (int i=0; i<m_nRatios; i++)
      m_pRatiosList[i] = 0.0;
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

double CSignificantRatios05::GetAlpha01()
{
   return (m_pRatiosList[(m_nRatios/5)-1]);
}

double CSignificantRatios05::GetAlpha05()
{
   return (m_pRatiosList[m_nRatios-1]);
}




