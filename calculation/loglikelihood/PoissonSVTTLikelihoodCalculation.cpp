//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "PoissonSVTTLikelihoodCalculation.h"
#include "SVTTCluster.h"

/** Compares inside trend to outside trend; checking for more increasing or less descreasing trends. */
bool IncreasingOrDecreasingTrend(const AbstractTimeTrend& InsideTrend, const AbstractTimeTrend& GlobalTrend) {
  if (InsideTrend.GetStatus() == AbstractTimeTrend::UNDEFINED) return false;
  return !macro_equal(InsideTrend.GetBeta(), GlobalTrend.GetBeta(), DBL_CMP_TOLERANCE);
}

/** Compares inside trend to outside trend; checking for increasing trends. */
bool IncreasingTrend(const AbstractTimeTrend& InsideTrend, const AbstractTimeTrend& GlobalTrend) {
  switch (InsideTrend.GetStatus()) {
    case AbstractTimeTrend::UNDEFINED         : return false;
    case AbstractTimeTrend::NEGATIVE_INFINITY : return false;
    case AbstractTimeTrend::POSITIVE_INFINITY : return true;
    case AbstractTimeTrend::CONVERGED :
    case AbstractTimeTrend::NOT_CONVERGED :
    case AbstractTimeTrend::SINGULAR_MATRIX :
    default: break;
  };
  return macro_less_than(GlobalTrend.GetBeta(), InsideTrend.GetBeta(), DBL_CMP_TOLERANCE);
}

/** Compares inside trend to outside trend; checking for descreasing trends. */
bool DecreasingTrend(const AbstractTimeTrend& InsideTrend, const AbstractTimeTrend& GlobalTrend) {
  switch (InsideTrend.GetStatus()) {
    case AbstractTimeTrend::UNDEFINED         : return false;
    case AbstractTimeTrend::NEGATIVE_INFINITY : return true;
    case AbstractTimeTrend::POSITIVE_INFINITY : return false;
    case AbstractTimeTrend::CONVERGED :
    case AbstractTimeTrend::NOT_CONVERGED :
    case AbstractTimeTrend::SINGULAR_MATRIX :
    default: break;
  };
  return macro_less_than(InsideTrend.GetBeta(), GlobalTrend.GetBeta(), DBL_CMP_TOLERANCE);
}

/** Compares inside trend to outside trend; checking for more increasing or less decreasing trends. 
    Custom comparison for quadratic trend. */
bool IncreasingOrDecreasingTrendQuadratic(const QuadraticTimeTrend& InsideTrend, const QuadraticTimeTrend& GlobalTrend) {
  if (InsideTrend.GetStatus() == AbstractTimeTrend::UNDEFINED ||
      (macro_equal(InsideTrend.GetBeta(), GlobalTrend.GetBeta(), DBL_CMP_TOLERANCE) && 
       macro_equal(InsideTrend.GetBeta2(), GlobalTrend.GetBeta2(), DBL_CMP_TOLERANCE))) return false;
  return !macro_equal(InsideTrend.GetBeta(), GlobalTrend.GetBeta(), DBL_CMP_TOLERANCE) || 
         !macro_equal(InsideTrend.GetBeta2(), GlobalTrend.GetBeta2(), DBL_CMP_TOLERANCE);
}

//--------------------- AbstractPoissonTrendLikelihoodCalculator ----------------------------------

AbstractPoissonTrendLikelihoodCalculator::AbstractPoissonTrendLikelihoodCalculator(const CSaTScanData& DataHub)
                            :AbstractLikelihoodCalculator(DataHub), gParameters(DataHub.GetParameters()) {
  //store data set loglikelihoods under null
  for (size_t t=0; t < DataHub.GetDataSetHandler().GetNumDataSets(); ++t) {
    count_t   N = DataHub.GetDataSetHandler().GetDataSet(t).getTotalCases();
    measure_t U = DataHub.GetDataSetHandler().GetDataSet(t).getTotalMeasure();
    gvDataSetLogLikelihoodUnderNull.push_back((N*log(N/U)));
  }
}

/** destructor */
AbstractPoissonTrendLikelihoodCalculator::~AbstractPoissonTrendLikelihoodCalculator() {}

/** calculates the Poisson log likelihood given the number of observed and expected cases
    - the total cases and expected cases used are that of first data set */
double AbstractPoissonTrendLikelihoodCalculator::CalcLogLikelihoodSpatialOnly(count_t n, measure_t u) const {
  count_t   N = gvDataSetTotals[0].first;
  measure_t U = gvDataSetTotals[0].second;

   if (n != N && n != 0)
     return n*log(n/u) + (N-n)*log((N-n)/(U-u));
   else if (n == 0)
     return (N-n) * log((N-n)/(U-u));
   else
     return n*log(n/u);
}

/** returns log likelihood for total data set at index */
double AbstractPoissonTrendLikelihoodCalculator::GetLogLikelihoodForTotal(size_t tSetIndex) const {
  return gvDataSetLogLikelihoodUnderNull[tSetIndex];
}

/** Calculates the full loglikelihood ratio/test statistic given passed maximizing value and
    data set index. For the Poisson calculator, the maximizing value would be the loglikelihood in
    a particular clustering. If maximizing value equals negative double max value, zero is returned
    as this indicates that no significant maximizing value was calculated. */
double AbstractPoissonTrendLikelihoodCalculator::CalculateFullStatistic(double dMaximizingValue, size_t tSetIndex) const {
  if (dMaximizingValue == -std::numeric_limits<double>::max()) return 0.0;
  return dMaximizingValue - (gvDataSetLogLikelihoodUnderNull[tSetIndex]);
}

///** Calculates the maximizing value given observed cases, expected cases and data set index.
//    For the Poisson calculator, the maximizing value is the loglikelihood. */
//double AbstractPoissonSVTTLikelihoodCalculator::CalculateMaximizingValue(count_t n, measure_t u, size_t tDataSetIndex) const {
//  count_t   N = gvDataSetTotals[tDataSetIndex].first;
// measure_t U = gvDataSetTotals[tDataSetIndex].second;
//
//  if (n != N && n != 0)
//    return n*log(n/u) + (N-n)*log((N-n)/(U-u));
//  else if (n == 0)
//    return (N-n) * log((N-n)/(U-u));
//  else
//    return n*log(n/u);
//}
 
//--------------------- PoissonLinearTrendLikelihoodCalculator ------------------------------------------

/** constructor */
PoissonLinearTrendLikelihoodCalculator::PoissonLinearTrendLikelihoodCalculator(const CSaTScanData& DataHub)
                            :AbstractPoissonTrendLikelihoodCalculator(DataHub) {
  switch (gParameters.GetExecuteScanRateType()) {
    case LOW  : gpTrendOfInterest = DecreasingTrend; break;
    case HIGH : gpTrendOfInterest = IncreasingTrend; break;
    default   : gpTrendOfInterest = IncreasingOrDecreasingTrend;
  }
}

/** destructor */
PoissonLinearTrendLikelihoodCalculator::~PoissonLinearTrendLikelihoodCalculator() {}

/** needs documentation */
double PoissonLinearTrendLikelihoodCalculator::CalcLogLikelihood(const count_t* pCases, const measure_t* pMeasure,
                                                                 count_t pTotalCases, double nAlpha,
                                                                 double nBeta, int nStatus) const {
  double nLL;

  /* Check for extremes where all the cases in the first or last interval
     or 0 cases in given tract.  For now there are multiple returns. */
  if (nStatus == AbstractTimeTrend::UNDEFINED) {// No cases in tract
    nLL = 0;
  } else if (nStatus == AbstractTimeTrend::NEGATIVE_INFINITY) {// All cases in first t.i.
    nLL = CalcLogLikelihoodSpatialOnly(pCases[0], pMeasure[0]);
  } else if (nStatus == AbstractTimeTrend::POSITIVE_INFINITY) {// All cases in last t.i.
    nLL = CalcLogLikelihoodSpatialOnly(pCases[gDataHub.GetNumTimeIntervals()-1], pMeasure[gDataHub.GetNumTimeIntervals()-1]);
  } else {
    double nSum1 = 0;
    double nSum2 = 0;

    for (int i=0; i < gDataHub.GetNumTimeIntervals(); ++i) {
		if (pMeasure[i]) {
			nSum1 += pCases[i] * (log(pMeasure[i]) + nAlpha + (nBeta)*i); 
			nSum2 += pMeasure[i] * exp(nAlpha + (nBeta)*i);
		}
    }
    nLL = nSum1 - nSum2;
  }
  #if DEBUGMODEL
  fprintf(m_pDebugModelFile, "Alpha = %f  nBeta = %f  nStatus = %i  LogLikelihood = %f\n", nAlpha, nBeta, nStatus, nLL);
  #endif
  return nLL;
}

/** needs documentation */
double PoissonLinearTrendLikelihoodCalculator::CalcSVTTLogLikelihood(size_t tSetIndex, SVTTClusterData& ClusterData, const AbstractTimeTrend& GlobalTimeTrend) const {
  double nLogLikelihood(0.0), nGlobalAlphaIn(0.0), nGlobalAlphaOut(0.0), nLogLikelihoodInside(0.0), nLogLikelihoodOutside(0.0);

  //log likelihood can only be calculated when the number of cases inside and outside the cluster are less than 2.
  if (GlobalTimeTrend.GetStatus() == AbstractTimeTrend::UNDEFINED || ClusterData.gtTotalCasesInsideCluster <= 1 || ClusterData.gtTotalCasesOutsideCluster <= 1)
    return 0.0;

  //calculate time trend inside cluster
  ClusterData.getInsideTrend().CalculateAndSet(ClusterData.gpCasesInsideCluster, 
                                               ClusterData.gpMeasureInsideCluster,
                                               gDataHub.GetNumTimeIntervals(), 
                                               gDataHub.GetParameters().GetTimeTrendConvergence());
                                               
  if (ClusterData.getInsideTrend().GetStatus() == AbstractTimeTrend::NOT_CONVERGED)
    throw prg_error("The time trend inside of cluster did not converge.\n", "CalcSVTTLogLikelihood()");

  //just return if time trend is not the trend we are looking for
  if (!gpTrendOfInterest(ClusterData.getInsideTrend(), GlobalTimeTrend))
    return 0.0;

  //calculate time trend outside cluster
  ClusterData.getOutsideTrend().CalculateAndSet(ClusterData.gpCasesOutsideCluster, 
                                                ClusterData.gpMeasureOutsideCluster,
                                                gDataHub.GetNumTimeIntervals(), 
                                                gDataHub.GetParameters().GetTimeTrendConvergence());

  if (ClusterData.getOutsideTrend().GetStatus() == AbstractTimeTrend::UNDEFINED)
    return 0.0;
  if (ClusterData.getOutsideTrend().GetStatus() == AbstractTimeTrend::NOT_CONVERGED)
    throw prg_error("The time trend outside of cluster did not converge.\n", "CalcSVTTLogLikelihood()");

  if (ClusterData.getInsideTrend().GetStatus() == AbstractTimeTrend::NEGATIVE_INFINITY || ClusterData.getInsideTrend().GetStatus() == AbstractTimeTrend::POSITIVE_INFINITY) {
    nLogLikelihoodInside = CalcLogLikelihoodSpatialOnly(ClusterData.gtTotalCasesInsideCluster, ClusterData.gtTotalMeasureInsideCluster);
  } else {
    nLogLikelihoodInside = CalcLogLikelihood(ClusterData.gpCasesInsideCluster, 
                                             ClusterData.gpMeasureInsideCluster,
                                             ClusterData.gtTotalCasesInsideCluster, 
                                             ClusterData.getInsideTrend().GetAlpha(),
                                             ClusterData.getInsideTrend().GetBeta(), 
                                             ClusterData.getInsideTrend().GetStatus());

    nGlobalAlphaIn = ((LinearTimeTrend&)ClusterData.getInsideTrend()).Alpha(ClusterData.gtTotalCasesInsideCluster, 
                                                                            ClusterData.gpMeasureInsideCluster,
                                                                            gDataHub.GetNumTimeIntervals(), 
                                                                            GlobalTimeTrend.GetBeta());
    nLogLikelihoodInside -= CalcLogLikelihood(ClusterData.gpCasesInsideCluster, 
                                              ClusterData.gpMeasureInsideCluster,
                                              ClusterData.gtTotalCasesInsideCluster, 
                                              nGlobalAlphaIn,
                                              GlobalTimeTrend.GetBeta(), 
                                              ClusterData.getInsideTrend().GetStatus());
  }

  if (ClusterData.getOutsideTrend().GetStatus() == AbstractTimeTrend::NEGATIVE_INFINITY || ClusterData.getOutsideTrend().GetStatus() == AbstractTimeTrend::POSITIVE_INFINITY) {
    nLogLikelihoodOutside = CalcLogLikelihoodSpatialOnly(ClusterData.gtTotalCasesOutsideCluster, ClusterData.gtTotalMeasureOutsideCluster);
  } else {
    nLogLikelihoodOutside = CalcLogLikelihood(ClusterData.gpCasesOutsideCluster, 
                                              ClusterData.gpMeasureOutsideCluster,
                                              ClusterData.gtTotalCasesOutsideCluster, 
                                              ClusterData.getOutsideTrend().GetAlpha(),
                                              ClusterData.getOutsideTrend().GetBeta(), 
                                              ClusterData.getOutsideTrend().GetStatus());

    nGlobalAlphaOut = ((LinearTimeTrend&)ClusterData.getOutsideTrend()).Alpha(ClusterData.gtTotalCasesOutsideCluster, 
                                                                              ClusterData.gpMeasureOutsideCluster,
                                                                              gDataHub.GetNumTimeIntervals(), 
                                                                              GlobalTimeTrend.GetBeta());
    nLogLikelihoodOutside -= CalcLogLikelihood(ClusterData.gpCasesOutsideCluster, 
                                               ClusterData.gpMeasureOutsideCluster,
                                               ClusterData.gtTotalCasesOutsideCluster, 
                                               nGlobalAlphaOut,
                                               GlobalTimeTrend.GetBeta(), 
                                               ClusterData.getOutsideTrend().GetStatus());
  }
  nLogLikelihood = nLogLikelihoodInside + nLogLikelihoodOutside;
  return nLogLikelihood;
}

//--------------------- PoissonQuadraticTrendLikelihoodCalculator -------------------------------

/** constructor */
PoissonQuadraticTrendLikelihoodCalculator::PoissonQuadraticTrendLikelihoodCalculator(const CSaTScanData& DataHub)
                            :AbstractPoissonTrendLikelihoodCalculator(DataHub) {
  if (gParameters.GetExecuteScanRateType() != HIGHANDLOW) {
    throw prg_error("PoissonQuadraticTrendLikelihoodCalculator only implemented for simulantienous high and low scanning.","constructor()");
  }
}

/** destructor */
PoissonQuadraticTrendLikelihoodCalculator::~PoissonQuadraticTrendLikelihoodCalculator() {}

/** Calculates the quadratic log likelihood given alpha, beta and beta2. */
double PoissonQuadraticTrendLikelihoodCalculator::CalcLogLikelihood(const count_t* pCases, const measure_t* pMeasure,
                                                                       count_t pTotalCases, double nAlpha,
                                                                       double nBeta, double nBeta2, int nStatus) const {
  double nLL;

  /* Check for extremes where all the cases in the first or last interval
     or 0 cases in given tract.  For now there are multiple returns. */
  if (nStatus == AbstractTimeTrend::UNDEFINED) {// No cases in tract
    nLL = 0;
  } else if (nStatus == AbstractTimeTrend::NEGATIVE_INFINITY) {// All cases in first t.i.
    nLL = CalcLogLikelihoodSpatialOnly(pCases[0], pMeasure[0]);
  } else if (nStatus == AbstractTimeTrend::POSITIVE_INFINITY) {// All cases in last t.i.
    nLL = CalcLogLikelihoodSpatialOnly(pCases[gDataHub.GetNumTimeIntervals()-1], pMeasure[gDataHub.GetNumTimeIntervals()-1]);
  } else {
    double nSum1 = 0;
    double nSum2 = 0;

    for (int i=0; i < gDataHub.GetNumTimeIntervals(); ++i) {
		if (pMeasure[i]) {
			nSum1 += pCases[i] * (log(pMeasure[i]) + nAlpha + nBeta * (i + 1) + nBeta2 * std::pow((double)( i + 1), 2.0));
			nSum2 += pMeasure[i] * exp(nAlpha + nBeta * (i+1) + nBeta2 * std::pow((double)(i + 1), 2.0));
		}
    }

    nLL = nSum1 - nSum2;
  }
  #if DEBUGMODEL
  fprintf(m_pDebugModelFile, "Alpha = %f  nBeta = %f  nStatus = %i  LogLikelihood = %f\n", nAlpha, nBeta, nStatus, nLL);
  #endif
  return nLL;
}

/** Calculates the quadratic trend given current cluster data and global trend. */
double PoissonQuadraticTrendLikelihoodCalculator::CalcSVTTLogLikelihood(size_t tSetIndex, SVTTClusterData& ClusterData, const AbstractTimeTrend& GlobalTimeTrend) const {
  double nLogLikelihood(0.0), nGlobalAlphaIn(0.0), nGlobalAlphaOut(0.0), nLogLikelihoodInside(0.0), nLogLikelihoodOutside(0.0);

  //log likelihood can only be calculated when the number of cases inside and outside the cluster are less than 2.
  if (GlobalTimeTrend.GetStatus() == AbstractTimeTrend::UNDEFINED || ClusterData.gtTotalCasesInsideCluster <= 1 || ClusterData.gtTotalCasesOutsideCluster <= 1)
    return 0.0;

  //calculate time trend inside cluster
  ClusterData.getInsideTrend().CalculateAndSet(ClusterData.gpCasesInsideCluster, 
                                               ClusterData.gpMeasureInsideCluster,
                                               gDataHub.GetNumTimeIntervals(), 
                                               gDataHub.GetParameters().GetTimeTrendConvergence());
                                               
  if (ClusterData.getInsideTrend().GetStatus() == AbstractTimeTrend::NOT_CONVERGED)
    throw prg_error("The time trend inside of cluster did not converge.\n", "CalcSVTTLogLikelihood()");

  if (ClusterData.getInsideTrend().GetStatus() == AbstractTimeTrend::SINGULAR_MATRIX)
    throw prg_error("The time trend inside of cluster was not calculated because matrix A is singular.\n", "CalcSVTTLogLikelihood()");

  //just return if time trend is not the trend we are looking for
  if (!IncreasingOrDecreasingTrendQuadratic((const QuadraticTimeTrend&)ClusterData.getInsideTrend(), (const QuadraticTimeTrend&)GlobalTimeTrend))
    return 0.0;

  //calculate time trend outside cluster
  ClusterData.getOutsideTrend().CalculateAndSet(ClusterData.gpCasesOutsideCluster, 
                                                ClusterData.gpMeasureOutsideCluster,
                                                gDataHub.GetNumTimeIntervals(), 
                                                gDataHub.GetParameters().GetTimeTrendConvergence());

  if (ClusterData.getOutsideTrend().GetStatus() == AbstractTimeTrend::UNDEFINED)
    return 0.0;

  if (ClusterData.getOutsideTrend().GetStatus() == AbstractTimeTrend::NOT_CONVERGED)
    throw prg_error("The time trend outside of cluster did not converge.\n", "CalcSVTTLogLikelihood()");

  if (ClusterData.getOutsideTrend().GetStatus() == AbstractTimeTrend::SINGULAR_MATRIX)
    throw prg_error("The time trend outside of cluster was not calculated because matrix A is singular.\n", "CalcSVTTLogLikelihood()");

  if (ClusterData.getInsideTrend().GetStatus() == AbstractTimeTrend::NEGATIVE_INFINITY || ClusterData.getInsideTrend().GetStatus() == AbstractTimeTrend::POSITIVE_INFINITY) {
    nLogLikelihoodInside = CalcLogLikelihoodSpatialOnly(ClusterData.gtTotalCasesInsideCluster, ClusterData.gtTotalMeasureInsideCluster);
  } else {
    nLogLikelihoodInside = CalcLogLikelihood(ClusterData.gpCasesInsideCluster, 
                                             ClusterData.gpMeasureInsideCluster,
                                             ClusterData.gtTotalCasesInsideCluster, 
                                             ClusterData.getInsideTrend().GetAlpha(),
                                             ClusterData.getInsideTrend().GetBeta(), 
                                             ((QuadraticTimeTrend&)ClusterData.getInsideTrend()).GetBeta2(),
                                             ClusterData.getInsideTrend().GetStatus());

    nGlobalAlphaIn = ((QuadraticTimeTrend&)ClusterData.getInsideTrend()).Alpha(ClusterData.gtTotalCasesInsideCluster, 
                                                                               ClusterData.gpMeasureInsideCluster,
                                                                               gDataHub.GetNumTimeIntervals(), 
                                                                               GlobalTimeTrend.GetBeta(),
                                                                               ((QuadraticTimeTrend&)GlobalTimeTrend).GetBeta2());
    nLogLikelihoodInside -= CalcLogLikelihood(ClusterData.gpCasesInsideCluster, 
                                              ClusterData.gpMeasureInsideCluster,
                                              ClusterData.gtTotalCasesInsideCluster, 
                                              nGlobalAlphaIn,
                                              GlobalTimeTrend.GetBeta(), 
                                              ((QuadraticTimeTrend&)GlobalTimeTrend).GetBeta2(),
                                              ClusterData.getInsideTrend().GetStatus());
  }

  if (ClusterData.getOutsideTrend().GetStatus() == AbstractTimeTrend::NEGATIVE_INFINITY || ClusterData.getOutsideTrend().GetStatus() == AbstractTimeTrend::POSITIVE_INFINITY) {
    nLogLikelihoodOutside = CalcLogLikelihoodSpatialOnly(ClusterData.gtTotalCasesOutsideCluster, ClusterData.gtTotalMeasureOutsideCluster);
  } else {
    nLogLikelihoodOutside = CalcLogLikelihood(ClusterData.gpCasesOutsideCluster, 
                                              ClusterData.gpMeasureOutsideCluster,
                                              ClusterData.gtTotalCasesOutsideCluster, 
                                              ClusterData.getOutsideTrend().GetAlpha(),
                                              ClusterData.getOutsideTrend().GetBeta(),
                                              ((QuadraticTimeTrend&)ClusterData.getOutsideTrend()).GetBeta2(),
                                              ClusterData.getOutsideTrend().GetStatus());
    nGlobalAlphaOut = ((QuadraticTimeTrend&)ClusterData.getOutsideTrend()).Alpha(ClusterData.gtTotalCasesOutsideCluster, 
                                                                                 ClusterData.gpMeasureOutsideCluster,
                                                                                 gDataHub.GetNumTimeIntervals(), 
                                                                                 GlobalTimeTrend.GetBeta(),
                                                                                 ((QuadraticTimeTrend&)GlobalTimeTrend).GetBeta2());
    nLogLikelihoodOutside -= CalcLogLikelihood(ClusterData.gpCasesOutsideCluster, 
                                               ClusterData.gpMeasureOutsideCluster,
                                               ClusterData.gtTotalCasesOutsideCluster, 
                                               nGlobalAlphaOut,
                                               GlobalTimeTrend.GetBeta(), 
                                               ((QuadraticTimeTrend&)GlobalTimeTrend).GetBeta2(),
                                               ClusterData.getOutsideTrend().GetStatus());
  }

  nLogLikelihood = nLogLikelihoodInside + nLogLikelihoodOutside;
  return nLogLikelihood;
}
