//*****************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//*****************************************************************************
#include "Toolkit.h"
#include "TimeTrend.h"
#include "SSException.h"
#include "newmat.h"
#include "UtilityFunctions.h"
#include "SaTScanData.h"
#include <numeric>

//----------------------- AbstractTimeTrend ------------------------------------------

/** constructor */
AbstractTimeTrend::AbstractTimeTrend() {
  Initialize();
}

/** destructor */
AbstractTimeTrend::~AbstractTimeTrend(){}

/** static variable define - beta variable that defines cutoff for trend being negative. */
const double AbstractTimeTrend::TREND_ZERO = 0.00000001;

/** static variable - maximum number of loop iterations before attempt to estimate start beta quits. */
const unsigned int AbstractTimeTrend::MAX_BETA_TEST_ITERATIONS = 1000;

/** static variable - maximum number of loop iterations before attempt to estimate beta quits. */
const unsigned int AbstractTimeTrend::MAX_CONVERGENCE_ITERATIONS = 1000000;

/** static variable - Time trend negative infinity */
const double AbstractTimeTrend::NEGATIVE_INFINITY_INDICATOR = -999999.0;

/** static variable - Time trend positive infinity  */
const double AbstractTimeTrend::POSITIVE_INFINITY_INDICATOR = 999999.0;

/** initializes/re-initializes class data members */
void AbstractTimeTrend::Initialize() {
  gdAlpha           = 0;
  gdBeta            = 0;
  gdAnnualTimeTrend = 0;
  gStatus           = AbstractTimeTrend::NOT_CONVERGED;
  gbGlobalAlpha     = 0;
}

/** Calculates annual time trend given specfied time interval precision and length. */
double AbstractTimeTrend::SetAnnualTimeTrend(DatePrecisionType eAggregationPrecision, double dTimeAggregationLength) {
  double nUnits;

  switch (gStatus) {
   case NOT_CONVERGED     : throw prg_error("Unable to call SetAnnualTimeTrend with non-converged time trend.", "SetAnnualTimeTrend()");
   case UNDEFINED         : gdAnnualTimeTrend = 0; return gdAnnualTimeTrend;
   case NEGATIVE_INFINITY : gdAnnualTimeTrend = NEGATIVE_INFINITY_INDICATOR;  return gdAnnualTimeTrend;
   case POSITIVE_INFINITY : gdAnnualTimeTrend = POSITIVE_INFINITY_INDICATOR;  return gdAnnualTimeTrend;
   case CONVERGED         : break;
   default : throw prg_error("Unknown time trend status '%d'.", "SetAnnualTimeTrend()", gStatus);
  }

  switch (eAggregationPrecision) {
    case GENERIC : /* is this correct? */
    case YEAR  : gdAnnualTimeTrend = 100 * (exp(gdBeta/dTimeAggregationLength) - 1); break;
    case MONTH : gdAnnualTimeTrend = 100 * (exp(gdBeta * 12/dTimeAggregationLength) - 1); break;
    case DAY   : gdAnnualTimeTrend = 100 * (exp(gdBeta * 365.25/dTimeAggregationLength) - 1); break;
    default    : throw prg_error("SetAnnualTimeTrend() called with unknown aggregation precision '%d'.",
                                 "SetAnnualTimeTrend()", eAggregationPrecision);
  }
  //If the time trend is very small, then the value assigned above is more likely the
  //result of round-off. In this case, set trend to zero.
  if (-TREND_ZERO < gdAnnualTimeTrend && gdAnnualTimeTrend < TREND_ZERO)
    gdAnnualTimeTrend = 0;
  return gdAnnualTimeTrend;
}

/** Returns instance of time trend objects get type specified in parameters. */
AbstractTimeTrend * AbstractTimeTrend::getTimeTrend(const CParameters& parameters) {
  switch (parameters.getTimeTrendType()) {
    case LINEAR    : return new LinearTimeTrend();
    case QUADRATIC : return new QuadraticTimeTrend();
    default        : throw prg_error("getTimeTrend() called with unknown time trend '%d'.",
                                     "getTimeTrend()", parameters.getTimeTrendType());
  }
}

//----------------------- LinearTimeTrend -------------------------------------

/** constructor */
LinearTimeTrend::LinearTimeTrend() : AbstractTimeTrend() {}

/** destructor */
LinearTimeTrend::~LinearTimeTrend(){}

/** Returns the natural log of (nSc/nSME). */
double LinearTimeTrend::Alpha(double nSC, double nSME) const {
  return log(nSC/nSME);
}

/* Calculates alpha given a specific beta. Required sums are recalculated */
double LinearTimeTrend::Alpha(count_t nCases, const measure_t* pMeasure, int nTimeIntervals, double nBeta) const {
  //Set local array with new nSumMeasure_ExpBeta values
  double nNewSME = 0; // using new Beta actually

  if (nCases == 0)
    gbGlobalAlpha = 0;
  else {
    for (int i=0; i < nTimeIntervals; i++)
      nNewSME += pMeasure[i] * exp(nBeta * i);
    gbGlobalAlpha = log(nCases / nNewSME);
  }

  return gbGlobalAlpha;
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
LinearTimeTrend::Status LinearTimeTrend::CalculateAndSet(const count_t* pCases, const measure_t* pMeasure, int nTimeIntervals, double nConverge) {
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

  Initialize();

  // Preliminary calculations that only need to be done once.
  // ********************************************************
  for (t=0; t < nTimeIntervals; t++) {
    nSumCases += pCases[t];
    nSumTime_Cases += t * pCases[t];
  }

  // Eliminates situations when the time trend is undefined.
  // *******************************************************
  if (nSumCases < 2)
    return  (gStatus = LinearTimeTrend::UNDEFINED);
  else if ((pCases[0] == nSumCases)) {
    gdAlpha = gdBeta = 0; /* What if don't set Beta at all??? */
    return (gStatus = LinearTimeTrend::NEGATIVE_INFINITY);
  }
  else if (pCases[nTimeIntervals-1] == nSumCases) {
    gdAlpha = gdBeta = 0; /* What if don't set Beta at all??? */
    return (gStatus = LinearTimeTrend::POSITIVE_INFINITY);
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
     nSumMsr_ExpBeta = nSumTime_Msr_ExpBeta = nSumTimeSquared_Msr_ExpBeta = 0; //SAH -- just trying something
     for (t=0; t < nTimeIntervals; t++) {
        nSumMsr_ExpBeta += pMeasure[t] * exp(nBetaStart * t);
        nSumTime_Msr_ExpBeta += t * pMeasure[t] * exp(nBetaStart * t);
        nSumTimeSquared_Msr_ExpBeta += (t*t) * pMeasure[t] * exp(nBetaStart * t);
     }
     if ( S(nSumCases,nSumTime_Cases,nSumTime_Msr_ExpBeta,nSumTimeSquared_Msr_ExpBeta) < -1)
         bGoodBetaStart = true;
     else
       nBetaStart += 0.1;
     nIterations++;  
  }
  if (bGoodBetaStart == false)
    return (gStatus = LinearTimeTrend::NOT_CONVERGED);

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
    gStatus = LinearTimeTrend::CONVERGED;
  }
  else
    gStatus = LinearTimeTrend::NOT_CONVERGED;

  return gStatus;
}

/** Calculates first derivative. */
double LinearTimeTrend::F(double nSC, double nSTC, double nSME, double nSTME) const {
  return (nSTC * nSME) - (nSC * nSTME);
}

/** Calculates second derivative. */
double LinearTimeTrend::S(double nSC, double nSTC, double nSTME, double nST2ME) const {
  return (nSTC * nSTME) - (nSC * nST2ME);
}

//----------------------- QuadraticTimeTrend ------------------------------------

/** constructor */
QuadraticTimeTrend::QuadraticTimeTrend() : AbstractTimeTrend() {
  Initialize();
}

/** destructor */
QuadraticTimeTrend::~QuadraticTimeTrend(){}


/* Calculates alpha given a specific beta and beta2. Required sums are recalculated */
double QuadraticTimeTrend::Alpha(count_t nCases, const measure_t* pMeasure, int nTimeIntervals, double nBeta, double nBeta2) const {
  double nNewSME = 0;

  if (nCases == 0)
    gbGlobalAlpha = 0;
  else {
    for (int i=0; i < nTimeIntervals; ++i) {
       nNewSME += pMeasure[i] * exp(nBeta * (i + 1) + nBeta2 * std::pow(i + 1, 2.0));
       gbGlobalAlpha = log (nCases / nNewSME);
    }
  }

  return gbGlobalAlpha;
}

QuadraticTimeTrend::Status QuadraticTimeTrend::CalculateAndSet(const count_t* pCases, const measure_t* pMeasure, int nTimeIntervals, double nConverge) {
  bool bConvergence = false;
  unsigned int        nIterations=0;
  measure_t           tMean=0;

  Initialize();

  //nTimeIntervals = 20;
  //double Cases[20] = {10.0,9.526316,9.052632,8.578947,8.105263,7.631579,7.157895,6.68421,6.210526,5.736842,5.263158,4.789474,4.315789,3.842105,3.368421,2.894737,2.421053,1.947368,1.473684,1.0};
  //double Measure[20] = {1000.0,1000.0,1000.0,1000.0,1000.0,1000.0,1000.0,1000.0,1000.0,1000.0,1000.0,1000.0,1000.0,1000.0,1000.0,1000.0,1000.0,1000.0,1000.0,1000.0};

  // Eliminates situations when the time trend is undefined.
  // *******************************************************
  count_t nSumCases = 0;//std::accumulate(pCases, pCases + nTimeIntervals, 0, plus<count_t>());
  int iIntervalsWithCases=0;
  for (int i=0; i < nTimeIntervals; ++i) {
  nSumCases += pCases[i];
    if (pCases[i]) ++iIntervalsWithCases;
  }

  if (nSumCases < 2 || iIntervalsWithCases < 3) {
    gdAlpha = gdBeta = gdBeta2 = 0;
    return  (gStatus = QuadraticTimeTrend::UNDEFINED);
  } else if ((pCases[0] == nSumCases)) {
    gdAlpha = gdBeta = gdBeta2 = 0;
    return (gStatus = QuadraticTimeTrend::NEGATIVE_INFINITY);
  } else if (pCases[nTimeIntervals-1] == nSumCases) {
    gdAlpha = gdBeta = gdBeta2 = 0;
    return (gStatus = QuadraticTimeTrend::POSITIVE_INFINITY);
  } // else ready to calculate
    
  // copy cases in column vector
  ColumnVector y(nTimeIntervals);
  for (int r=0; r < y.Nrows(); ++r) {
     y.element(r) = pCases[r];
     tMean += pCases[r];
  }
  tMean /= (measure_t)nTimeIntervals;
    
  // copy measure in column vector
  ColumnVector m(nTimeIntervals);
  for (int r=0; r < m.Nrows(); ++r)
     m.element(r) = pMeasure[r];
   
  //column vector: alpha, beta1, beta2
  ColumnVector b(3);
  b.element(0) = log(tMean);
  b.element(1) = 0.0;
  b.element(2) = 0.0;
    
  Matrix X(nTimeIntervals,3);
  for (int r=0; r < X.Nrows(); ++r) {
    for (int c=0; c < X.Ncols(); ++c) {
      if (c == 0)
        X.element(r,c) = 1;
      else if (c == 1)
        X.element(r,c) = r + 1;
      else
        X.element(r,c) = std::pow(r + 1,2.0);
    }
  }

  try {
    while (!bConvergence && nIterations < MAX_CONVERGENCE_ITERATIONS) {
        Matrix mu(X * b);
        for (int r=0; r < mu.Nrows(); ++r) {
            for (int c=0; c < mu.Ncols(); ++c) {
                mu.element(r,c) = exp(mu.element(r,c)) * m.element(r);
            }
        }
        Matrix A = (X.t() * mu.AsDiagonal()) * X;
        Matrix bnew = b - ((A.i() * ((X * -1.0).t())) * (y - mu));
        double dif=0;
        for (int r=0; r < b.Nrows(); ++r) { //Norm of the difference of 2 column vectors
           dif += std::pow(b.element(r) - bnew.element(r,0), 2.0);
        }
        bConvergence = std::sqrt(dif) <= nConverge;
        b = bnew;
        ++nIterations;
    }
    
    if (bConvergence) {
      //printoutMatrix("b", b, stdout);
      gdAlpha = b.element(0);
      gdBeta = b.element(1);
      gdBeta2 = b.element(2);
      gStatus = QuadraticTimeTrend::CONVERGED;
    } else {
      gdAlpha = gdBeta = gdBeta2 = 0;
      gStatus = QuadraticTimeTrend::NOT_CONVERGED;
    }
  } catch (SingularException& x) {
    gdAlpha = gdBeta = gdBeta2 = 0;
	gStatus = QuadraticTimeTrend::SINGULAR_MATRIX;
  }

  //if (gStatus == QuadraticTimeTrend::NOT_CONVERGED || gStatus == QuadraticTimeTrend::SINGULAR_MATRIX) {
  //  fprintf(AppToolkit::getToolkit().openDebugFile(), "\n\nQuadratic trend failed %s\n", 
  //      (gStatus == QuadraticTimeTrend::NOT_CONVERGED ? "to converge." : "due to singular matrix."));
    //printoutMatrix("y", y, AppToolkit::getToolkit().openDebugFile());
    //printoutMatrix("m", m, AppToolkit::getToolkit().openDebugFile());
    //printoutMatrix("b", b, AppToolkit::getToolkit().openDebugFile());
    //printoutMatrix("X", X, AppToolkit::getToolkit().openDebugFile());
  //}

  return gStatus;
}

/** Returns temporal trend expressed as a function of date precision. */
void QuadraticTimeTrend::getRiskFunction(std::string& functionStr, std::string& definitionStr, const CSaTScanData& DataHub) const {
  std::string buffer, buffer2, buffer3("");
  const CParameters& params(DataHub.GetParameters());

  printString(functionStr, "e^(%g + %g t + %g t^2)", GetAlpha(), GetBeta(), GetBeta2());

  switch (params.GetTimeAggregationUnitsType()) {
    case YEAR    : buffer = "year"; break;
    case MONTH   : buffer = "month"; break;
    case DAY     : buffer = "day"; break;
    case GENERIC : buffer = "unit"; break;
    default      : buffer = "none";
  }
  double constant = static_cast<double>(params.GetTimeAggregationLength() - 1)/2.0;
  if (constant != 0.0)
      printString(buffer3, " + %g", constant);

  printString(definitionStr, "where t=(%s - %s)/%u%s",
              buffer.c_str(),
              JulianToString(buffer2, DataHub.GetTimeIntervalStartTimes()[0], params.GetTimeAggregationUnitsType()).c_str(),
              DataHub.GetNumTimeIntervals(),
              buffer3.c_str());
}


/** initializes/re-initializes class data members */
void QuadraticTimeTrend::Initialize() {
  AbstractTimeTrend::Initialize();
  gdBeta2           = 0;
}
