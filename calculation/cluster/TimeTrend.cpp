//*****************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//*****************************************************************************
#include "TimeTrend.h"

static unsigned int iLessThanOne=0;
static unsigned int iLessThanFive=0;
static unsigned int iLessThanTen=0;
static unsigned int iLessThanFifteen=0;
static unsigned int iLessThanTwenty=0;
static unsigned int iLessThanFifty=0;
static unsigned int iRemainder=0;

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

double CTimeTrend::safe_exp(double dValue) const {
   if (dValue < 1)
     ++iLessThanOne;
   else if (dValue < 5)
     ++iLessThanFive;
   else if (dValue < 10)
     ++iLessThanTen;
   else if (dValue < 15)
     ++iLessThanFifteen;
   else if (dValue < 20)
     ++iLessThanTwenty;
   else
     ++iRemainder;

   double d = exp(dValue);
   if (d == HUGE_VAL)
     ZdGenerateException("exp(%lf) overflow", "safe_exp()", dValue);
   if (d == 0)
     ZdGenerateException("exp(%lf) underflow", "safe_exp()", dValue);
   return d;
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
  double nBetaStart, nBetaOld, nBetaNew, nLeastSquareBeta; 
  double FirstDerivative, SecondDerivative;
  bool bConvergence = false;
  bool bGoodBetaStart = false;
  unsigned int nIterations;

  // Preliminary calculations that only need to be done once.
  // ********************************************************
  for (t=0; t < nTimeIntervals; t++) {
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
    nSumTimeCasesMinusMsr += t * (pCases[t]-pMeasure[t]);
    nSumTime += t;
    nSumTimeSquare += t * t;
  }
  nLeastSquareBeta = (nTimeIntervals*nSumTimeCasesMinusMsr - nSumCasesMinusMsr*nSumTime)
               / (nTimeIntervals*nSumTimeSquare-(nSumTime*nSumTime));
  nBetaStart = nLeastSquareBeta / (nSumCases/nTimeIntervals);

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
        nSumMsr_ExpBeta += pMeasure[t] * safe_exp(nBetaStart * t);
        nSumTime_Msr_ExpBeta += t * pMeasure[t] * safe_exp(nBetaStart * t);
        nSumTimeSquared_Msr_ExpBeta += (t*t) * pMeasure[t] * safe_exp(nBetaStart * t);
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
      nSumMsr_ExpBeta += pMeasure[t] * safe_exp(nBetaOld * t);
      nSumTime_Msr_ExpBeta += t * pMeasure[t] * safe_exp(nBetaOld * t);
      nSumTimeSquared_Msr_ExpBeta += (t*t) * pMeasure[t] * safe_exp(nBetaOld * t);
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
      nSumMsr_ExpBeta += pMeasure[t] * safe_exp(nBetaNew * t);
    gdAlpha = Alpha(nSumCases,nSumMsr_ExpBeta);
    gdBeta = nBetaNew;
    //Set status, a negative beta is not likely, but perform check regardless.
    gStatus = (gdBeta < -1 ? TREND_NEGATIVE : TREND_CONVERGED);
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

/** Calculates annual time trend given specfied time interval precision and length. */
double CTimeTrend::SetAnnualTimeTrend(DatePrecisionType eDatePrecision, double nIntervalLen) {
  double nUnits;

  switch (eDatePrecision) {
    case YEAR  : nUnits = 1; break;
    case MONTH : nUnits = 12; break;
    case DAY   : nUnits = 365.25; break;
    default    : ZdGenerateException("SetAnnualTimeTrend() called with unknown date precision '%d'.",
                                     "SetAnnualTimeTrend()", eDatePrecision);
  }

  //**SVTT::TODO**  In a previous revision of this file, we created a status enumeration
  //                to reflect that 'gdBeta' was negative. When calculating loglikelihood,
  //                we have not yet defined the behavior when this object's status is
  //                anything other than TREND_CONVERGED. As a result, we can eventually
  //                call SetAnnualTimeTrend() with a negative 'gdBeta' or some other state.
  //                Until I can discuss this further with Martin, take that absolute value
  //                when negative.
  if (gStatus == TREND_CONVERGED)
    gdAnnualTimeTrend = (pow(1 + gdBeta, nUnits/nIntervalLen) - 1) * 100;
  else if (gStatus == TREND_NEGATIVE) {
    gdAnnualTimeTrend = (pow(1 + std::fabs(gdBeta), nUnits/nIntervalLen) - 1) * 100;
    printf("Undefined behavior, calling SetAnnualTimeTrend() with negative beta.\n");
  }
  else if (gStatus == TREND_UNDEF)
    printf("Undefined behavior, calling SetAnnualTimeTrend() with undefined trend.\n");
  else if (gStatus == TREND_INF)
    printf("Undefined behavior, calling SetAnnualTimeTrend() with infinite trend.\n");
  else if (gStatus == TREND_NOTCONVERGED)
    printf("Undefined behavior, calling SetAnnualTimeTrend() with not converged trend.\n");
  else if (gStatus == TREND_NOTCONVERGED)
    printf("Undefined behavior, calling SetAnnualTimeTrend() with not converged trend.\n");
  else
    printf("Undefined behavior, calling SetAnnualTimeTrend() with unknown trend.\n");

  return gdAnnualTimeTrend;
}

