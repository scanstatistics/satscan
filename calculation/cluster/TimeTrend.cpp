#include "SaTScan.h"
#pragma hdrstop
#include "TimeTrend.h"

CTimeTrend::CTimeTrend()
{
   Initialize();
};

CTimeTrend::~CTimeTrend()
{
};

void CTimeTrend::Initialize()
{
  m_nAlpha    = 0.0;
  m_nBeta     = 0.0;
  m_nAnnualTT = 0.0;
  m_nStatus = TREND_OK;
  m_nSC     = 0.0;
  m_nSTC    = 0.0;
  m_nSME    = 0.0;
  m_nSTME   = 0.0;
  m_nST2ME  = 0.0;
};

double CTimeTrend::SetAnnualTimeTrend(int nIntervalUnits, double nIntervalLen)
{
  double nUnits;

  switch (nIntervalUnits)
  {
    case YEAR  : nUnits = 1; break;
    case MONTH : nUnits = 12; break;
    case DAY   : nUnits = 365.25; break;
  }

  m_nAnnualTT = (pow(1+fabs(m_nBeta), 1/(nIntervalLen/nUnits))-1) * 100;
  return m_nAnnualTT;
}

bool CTimeTrend::IsNegative()
{
  return (m_nBeta < TREND_ZERO);
};

CTimeTrend& CTimeTrend::operator =(const CTimeTrend& TT)
{
  m_nAlpha    = TT.m_nAlpha;
  m_nBeta     = TT.m_nBeta;
  m_nAnnualTT = TT.m_nAnnualTT;
  m_nStatus     = TT.m_nStatus;
  m_nSC     = TT.m_nSC;
  m_nSTC    = TT.m_nSTC;
  m_nSME    = TT.m_nSME;
  m_nSTME   = TT.m_nSTME;
  m_nST2ME  = TT.m_nST2ME;
  return *this;
}

void CTimeTrend::CalculateAndSet(count_t*   pCases,
                                 measure_t* pMeasure,
                                 int        nTimeIntervals,
                                 double     nConverge)
{

  double nSumCases = 0;
  double nSumTime_Cases = 0;
  double nSumMsr_ExpBeta = 0;
  double nSumTime_Msr_ExpBeta = 0;
  double nSumTimeSquared_Msr_ExpBeta = 0;
  int i;
  double nBetaOld = 0;
  double nBetaNew = 0;  // -1 for now.  Better choice???
  bool bDone = false;
  int nIterations;

  /* Preliminary calculations to compute Time Trend (so-called Beta). */
  /* These are only computed once. */
  for (i=0; i<(nTimeIntervals); i++)
  {
    nSumCases += pCases[i];
    nSumTime_Cases += i * pCases[i];
  }

  if (nSumCases == 0)
  {
    m_nStatus = TREND_UNDEF;
    return;
  }
  else if ((pCases[0] == nSumCases) || (pCases[nTimeIntervals-1] == nSumCases))
  {
    m_nStatus = TREND_INF;
    m_nBeta = 0; /* What if don't set Beta at all??? */
    return;
  }

  nIterations = 0;
  //Maximum number of iterations increased from 100 to 1,000,000 by Martin's request.
  while (nIterations < 1000000 && !bDone)
  {
    /* Create sum terms for calculations in derivatives F() and S(), */
    /* the first and second derivatives of Alpha */

    /* First initialize each time computed: */
    nSumMsr_ExpBeta = 0;
    nSumTime_Msr_ExpBeta = 0;
    nSumTimeSquared_Msr_ExpBeta = 0;

    for (i=0; i<(nTimeIntervals); i++)
    {
      nSumMsr_ExpBeta += pMeasure[i] * exp(nBetaOld * i);
      nSumTime_Msr_ExpBeta += i * pMeasure[i] * exp(nBetaOld * i);
      nSumTimeSquared_Msr_ExpBeta += (i*i) * pMeasure[i] * exp(nBetaOld * i);
    }

    // From e-mail M Kulldorff 991005 a new initialization of nBetaOld:
    // beta(0)=0       (as before)
    // while S(beta(0))>-1 beta(0)=beta(0)+1     (new)
    while (nIterations == 0 &&
      S(nSumCases,nSumTime_Cases,nSumTime_Msr_ExpBeta,nSumTimeSquared_Msr_ExpBeta) > -1)
     {
     nBetaOld += 1;
      nSumMsr_ExpBeta = 0;
      nSumTime_Msr_ExpBeta = 0;
      nSumTimeSquared_Msr_ExpBeta = 0;
      for (i=0; i<(nTimeIntervals); i++)
      {
        nSumMsr_ExpBeta += pMeasure[i] * exp(nBetaOld * i);
        nSumTime_Msr_ExpBeta += i * pMeasure[i] * exp(nBetaOld * i);
        nSumTimeSquared_Msr_ExpBeta += (i*i) * pMeasure[i] * exp(nBetaOld * i);
      }
    } // End of change 991005

    /* Here we are using Newton's method to find the zero of F(Alpha), */
    /* the first derivative of Alpha in terms of Beta. */
    nBetaNew = nBetaOld -
    (F(nSumCases,nSumTime_Cases,nSumMsr_ExpBeta,nSumTime_Msr_ExpBeta) /
    S(nSumCases,nSumTime_Cases,nSumTime_Msr_ExpBeta,nSumTimeSquared_Msr_ExpBeta));

    /* Has convergence been achieved? */
    if (fabs(nBetaNew-nBetaOld) < nConverge)
      bDone = true;
    else
    	nBetaOld=nBetaNew;
      nIterations++;

  }

  /* Done with the loop.  Handle both cases. */
  if (bDone)
  {
    /* Calc Alpha in terms of new Beta: Need new nSumMsr_ExpBeta */
    /* Need to reinitialize nSumMsr_ExpBeta again! */
    nSumMsr_ExpBeta=0.0;
    for (i=0; i<(nTimeIntervals); i++)
    {
      nSumMsr_ExpBeta += pMeasure[i] * exp(nBetaNew * i);
    }
    m_nAlpha = Alpha(nSumCases,nSumMsr_ExpBeta);
    m_nBeta = nBetaNew;
  }
  else
  {
    /* Handle non-convergence: */
    /* Print error message and quit program */
    SSGenerateException("No convergence in Time Trend calculation.", "CalculateAndSet()");
  }
}

double CTimeTrend::Alpha(double nSC, double nSME) const
{
  return(log((nSC / nSME)));
//  return(log((nSumCases / nSumMeasure_ExpBeta)));
}

// Function to calc alpha given a specific beta
// Required sums are recalculated
double CTimeTrend::Alpha(count_t nCases,
                         measure_t* pMeasure,
                         int nTimeIntervals,
                         double nBeta) const
{
  double rval;
  //Set local array with new nSumMeasure_ExpBeta values
  double nNewSME = 0; // using new Beta actually
  if (nCases == 0)
  {
    rval = 0;
  }
  else
  {
    for (int i=0; i<nTimeIntervals; i++)
    {
      nNewSME += pMeasure[i] * exp(nBeta * i); // How to access pMeasure?
    }
    rval = log(nCases / nNewSME);
  }
  return(rval);
}

double CTimeTrend::F(double nSC, double nSTC, double nSME, double nSTME) const
{
  return( (nSTC * nSME) - (nSC * nSTME) );

//  The above abbrevs translate to:
//    (nSumTime_Cases * nSumMeasure_ExpBeta)
//          - (nSumCases * nSumTime_Measure_ExpBeta)
}

double CTimeTrend::S(double nSC, double nSTC, double nSTME, double nST2ME) const
{
  return( (nSTC * nSTME) - (nSC * nST2ME) );

//  The above abbrevs translate to:
//    (nSumTime_Cases * nSumTime_Measure_ExpBeta)
//          - (nSumCases * nSumTimeSquared_Measure_ExpBeta)
}


