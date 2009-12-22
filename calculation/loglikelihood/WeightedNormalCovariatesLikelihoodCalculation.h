//*****************************************************************************
#ifndef __WeightedNormalCovariatesLikelihoodCalculation_H
#define __WeightedNormalCovariatesLikelihoodCalculation_H
//*****************************************************************************
#include "LikelihoodCalculation.h"
#include "ptr_vector.h"

class Matrix;
class ColumnVector;

/** Normal log likelihood calculator. Note that this class is currently just
    a shell. All defined functions, when called, will through exceptions.
    Waiting for further implementation details...                             */
class WeightedNormalCovariatesLikelihoodCalculator : public AbstractLikelihoodCalculator {
  protected:
      std::vector<ColumnVector*>  _wg;
      std::vector<ColumnVector*>  _deltag;
      std::vector<double>         _dataSetMaximize; // maximizing total for entire data set

  public:
    WeightedNormalCovariatesLikelihoodCalculator(const CSaTScanData& Data);
    virtual ~WeightedNormalCovariatesLikelihoodCalculator();

    virtual double      CalcLogLikelihoodRatioNormal(Matrix& xg, Matrix& tobeinversed, Matrix& xgsigmaw, size_t tDataSetIndex=0) const;
    virtual double      CalculateFullStatistic(double dMaximizingValue, size_t tSetIndex=0) const;
    virtual double      CalculateMaximizingValueNormal(Matrix& xg, Matrix& tobeinversed, Matrix& xgsigmaw, size_t tDataSetIndex=0) const;
};
//*****************************************************************************
#endif
