//*****************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//*****************************************************************************
#include "TimeTrend.h"

/** constructor */
CTimeTrend::CTimeTrend() {
  Initialize();
}

/** destructor */
CTimeTrend::~CTimeTrend(){}

/** static variable define - beta variable that defines cutoff for trend being negative. */
const double CTimeTrend::TREND_ZERO = 0.00000001;

/** static variable - maximum number of loop iterations before attempt to estimate start beta quits. */
const unsigned int CTimeTrend::MAX_BETA_TEST_ITERATIONS = 1000;

/** static variable - maximum number of loop iterations before attempt to estimate beta quits. */
const unsigned int CTimeTrend::MAX_CONVERGENCE_ITERATIONS = 1000000;

/** Returns the natural log of (nSc/nSME). */
double CTimeTrend::Alpha(double nSC, double nSME) const {
  return log(nSC/nSME);
}

/* Calculates alpha given a specific beta. Required sums are recalculated */
double CTimeTrend::Alpha(count_t nCases, const measure_t* pMeasure, int nTimeIntervals, double nBeta) const {
  double rval;
  //Set local array with new nSumMeasure_ExpBeta values
  double nNewSME = 0; // using new Beta actually

  if (nCases == 0)
    rval = 0;
  else {
    for (int i=0; i < nTimeIntervals; i++)
      nNewSME += pMeasure[i] * exp(nBeta * i); // How to access pMeasure?
    rval = log(nCases / nNewSME);
  }

  return rval;
}

// ******************************************************************************
// This functions calculates the time trend, as a percent decrease or increase.
// The model is lamda(t) = measure(t) * exp ( alpha + beta*t), where t is the 
// index of time, lamda(t) is the risk at time t, beta is the percent annual 
// increase (beta<0 -> decrease) in risk, and alpha is the intercept.
// If the time intervals are one year, it is an annual increase, if the time
// intervals are one month, it is a monthly increase, etc. 
// When the first time interval is shorter then the other time intervals, that
// is not accounted for in the calculations. 
// The value of nConverge should be very small. 
// ******************************************************************************
CTimeTrend::Status CTimeTrend::CalculateAndSet(const count_t* pCases, const measure_t* pMeasure, int nTimeIntervals, double nConverge) {
  double nSumCases = 0;
  double nSumTime_Cases = 0;
  double nSumMsr_ExpBeta = 0;
  double nSumTime_Msr_ExpBeta = 0;
  double nSumTimeSquared_Msr_ExpBeta = 0;
  double nSumCasesMinusMsr = 0;
  double nSumTimeCasesMinusMsr = 0;
  double nSumTimeSquare = 0; 
  double nSumTime = 0;
  int t;
  double nBetaStart, nBetaOld, nBetaNew; 
  double FirstDerivative, SecondDerivative;
  bool bConvergence = false;
  bool bGoodBetaStart = false;
  unsigned int nIterations;

  // Preliminary calculations that only need to be done once.
  // ********************************************************
  for (t=0; t<(nTimeIntervals); t++) {
    nSumCases += pCases[t];
    nSumTime_Cases += t * pCases[t];
  }

  // Eliminates situations when the time trend is undefined.
  // *******************************************************
  if (nSumCases == 0)
    return  (gStatus = TREND_UNDEF);
  else if ((pCases[0] == nSumCases) || (pCases[nTimeIntervals-1] == nSumCases)) {
    gdBeta = 0; /* What if don't set Beta at all??? */
    return (gStatus = TREND_INF);
  }

  // Calculates the start value of beta for the subsequent Newton-Raphson iterations.
  // The numerator of nBetaStart is the average annual increase in observed minus expected
  // when fitting a linear model. The denominator is the average number of cases per year.
  // This is meant to estimate the percent increase per year with a very simple but
  // explicit formula. / MK, March 23, 2004
  // ********************************************************************************
  for (t=0; t < nTimeIntervals; t++) {
    nSumCasesMinusMsr += pCases[t]-pMeasure[t];
    nSumTimeCasesMinusMsr += t*(pCases[t]-pMeasure[t]);
    nSumTime += t;
    nSumTimeSquare += t*t;
  }
  nBetaStart = (nTimeIntervals*nSumTimeCasesMinusMsr - nSumCasesMinusMsr*nSumTime)
               / (nTimeIntervals*nSumTimeSquare-(nSumTime*nSumTime));

  // Ensures that the start value for beta (nBetaStart) is good, and if not, adds one to it.
  // If the second derivative is positive, the iterations will go in the wrong direction,
  // and never converge. If it is larger than -1, it will generate a very large value
  // for the next beta. Hence, better to simply increase beta step wise by one, until
  // the second derivative is less than -1.
  // Incorporated 991005, based on email from MK, when nBetaStart was zero. 
  // Code modified 040323. / MK
  // ********************************************************************************
  nIterations = 0;
  while (!bGoodBetaStart && nIterations < MAX_BETA_TEST_ITERATIONS) {
     for (t=0; t < nTimeIntervals; t++) {
        nSumMsr_ExpBeta += pMeasure[t] * exp(nBetaStart * t);
        nSumTime_Msr_ExpBeta += t * pMeasure[t] * exp(nBetaStart * t);
        nSumTimeSquared_Msr_ExpBeta += (t*t) * pMeasure[t] * exp(nBetaStart * t);
     }
     if ( S(nSumCases,nSumTime_Cases,nSumTime_Msr_ExpBeta,nSumTimeSquared_Msr_ExpBeta) < -1)
         bGoodBetaStart = true;
     else
       nBetaStart += 1;
     nIterations++;  
  }
  if (bGoodBetaStart == false)
    return (gStatus = TREND_NOTCONVERGED);

  // Estimates the time trend beta usisng Newton-Raphson's iterative method.
  // Maximum number of iterations increased from 100 to 1,000,000 by MK's request 1999.
  // That large a number is probably no longer needed, but neither does it hurt. MK 040323
  // *************************************************************************************
  nIterations = 0;
  nBetaOld = nBetaStart;
  while (!bConvergence && nIterations < MAX_CONVERGENCE_ITERATIONS) {
    // Resets the sums to zero.
    // ***********************
    nSumMsr_ExpBeta = 0;
    nSumTime_Msr_ExpBeta = 0;
    nSumTimeSquared_Msr_ExpBeta = 0;

    // Create sum terms for calculations in derivatives F() and S(), the first and second derivatives of Alpha.
    // ********************************************************************************************************
    for (t=0; t < nTimeIntervals; t++) {
      nSumMsr_ExpBeta += pMeasure[t] * exp(nBetaOld * t);
      nSumTime_Msr_ExpBeta += t * pMeasure[t] * exp(nBetaOld * t);
      nSumTimeSquared_Msr_ExpBeta += (t*t) * pMeasure[t] * exp(nBetaOld * t);
    }

    // Uses Newton-Raphsons method to find the zero of F(Alpha),
    // the first derivative of Alpha in terms of Beta.
    // *********************************************************
    FirstDerivative = F(nSumCases,nSumTime_Cases,nSumMsr_ExpBeta,nSumTime_Msr_ExpBeta);
    SecondDerivative = S(nSumCases,nSumTime_Cases,nSumTime_Msr_ExpBeta,nSumTimeSquared_Msr_ExpBeta);
    if (SecondDerivative != 0)
       nBetaNew = nBetaOld - FirstDerivative / SecondDerivative;
    else
       nBetaNew = nBetaOld +0.1;

    // Checks to see if convergence has been achieved.
    // If not, ensures that we will not get an overflow during the next iteration.
    // ***************************************************************************
    if (fabs(nBetaNew-nBetaOld) < nConverge)
      bConvergence = true;
    else {
      if (nBetaNew*nTimeIntervals < log(std::numeric_limits<double>::max()))
        nBetaOld=nBetaNew;
      else
        nBetaOld += 0.1;
    }    	       
    nIterations++;
  }

  // Calculate alpha based on the last estimate of beta.
  // ****************************************************
  if (bConvergence) {
    nSumMsr_ExpBeta=0.0;
    for (t=0; t < nTimeIntervals; t++)
      nSumMsr_ExpBeta += pMeasure[t] * exp(nBetaNew * t);
    gdAlpha = Alpha(nSumCases,nSumMsr_ExpBeta);
    gdBeta = nBetaNew;
    gStatus = TREND_CONVERGED;
  }
  else
    gStatus = TREND_NOTCONVERGED;

  return gStatus;
}

/** Calculates first derivative. */
double CTimeTrend::F(double nSC, double nSTC, double nSME, double nSTME) const {
  return (nSTC * nSME) - (nSC * nSTME);
}

/** initializes/re-initializes class data members */
void CTimeTrend::Initialize() {
  gdAlpha           = 0;
  gdBeta            = 0;
  gdAnnualTimeTrend = 0;
  gStatus           = TREND_UNDEF;
}

/** Returns true if m_nBeta is less than TREND_ZERO else returns false. */
bool CTimeTrend::IsNegative() const {
  return gdBeta < TREND_ZERO;
}

/** Calculates second derivative. */
double CTimeTrend::S(double nSC, double nSTC, double nSTME, double nST2ME) const {
  return (nSTC * nSTME) - (nSC * nST2ME);
}

/** Calculates annual time trend. */
double CTimeTrend::SetAnnualTimeTrend(DatePrecisionType eDatePrecision, double nIntervalLen) {
  double nUnits;

  switch (eDatePrecision) {
    case YEAR  : nUnits = 1; break;
    case MONTH : nUnits = 12; break;
    case DAY   : nUnits = 365.25; break;
    default    : ZdGenerateException("SetAnnualTimeTrend() called with unknown date precision '%d'.",
                                     "SetAnnualTimeTrend()", eDatePrecision);
  }

  gdAnnualTimeTrend = (pow(1+fabs(gdBeta), 1/(nIntervalLen/nUnits))-1) * 100;
  return gdAnnualTimeTrend;
}

