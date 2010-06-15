//---------------------------------------------------------------------------
#include "SaTScan.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#include "WeightedNormalCovariatesLikelihoodCalculation.h"
#include "SaTScanData.h"
#include "WeightedNormalRandomizer.h"
#include "newmat.h"

/** constructor */
WeightedNormalCovariatesLikelihoodCalculator::WeightedNormalCovariatesLikelihoodCalculator(const CSaTScanData& DataHub)
                           :AbstractLikelihoodCalculator(DataHub) {
   try {
     const AbstractWeightedNormalRandomizer *pRandomizer;
     for (size_t t=0; t < DataHub.GetDataSetHandler().GetNumDataSets(); ++t) {
       if ((pRandomizer = dynamic_cast<const AbstractWeightedNormalRandomizer*>(DataHub.GetDataSetHandler().GetRandomizer(t))) == 0)
          throw prg_error("Randomizer could not be dynamically casted to AbstractWeightedNormalRandomizer type.\n", "constructor()");
       std::auto_ptr<ColumnVector> wg, deltag;
       pRandomizer->get_wg_deltag(wg, deltag);
       _wg.push_back(wg.release());
       _deltag.push_back(deltag.release());

       // calculate maximizing value for entire dataset
       std::auto_ptr<Matrix> xg;
       pRandomizer->get_xg(xg,true);
       Matrix tobeinversed(*xg);
       for (int r=0; r < tobeinversed.Nrows(); ++r)
            for (int c=0; c < tobeinversed.Ncols(); ++c) {
                tobeinversed.element(r,c) = tobeinversed.element(r,c)/_deltag.back()->element(r);
       }
       tobeinversed = xg->t() * tobeinversed;
       ColumnVector w_div_delta(_wg.back()->Nrows());
        for (int r=0; r < w_div_delta.Nrows(); ++r)
            w_div_delta.element(r) = _wg.back()->element(r)/_deltag.back()->element(r);
       //initialize xg sigma w matrix
       Matrix xgsigmaw(xg->t() * w_div_delta);
       Matrix temp_matrix = tobeinversed.i() * xgsigmaw;
       Matrix& _wg_ = *(_wg.at(t));
       Matrix& _deltag_ = *(_deltag.at(t));
       temp_matrix = (*xg) * temp_matrix;
       temp_matrix = _wg_ - temp_matrix;
       temp_matrix = SP(temp_matrix, temp_matrix);
       double value = -log((_deltag_.t() * temp_matrix).AsScalar());
       _dataSetMaximize.push_back(value);
     }
  } catch (prg_exception& x) {
    x.addTrace("constructor()","WeightedNormalCovariatesLikelihoodCalculator");
    throw;
  }
}

/** destructor */
WeightedNormalCovariatesLikelihoodCalculator::~WeightedNormalCovariatesLikelihoodCalculator() {
    try {
        for (std::vector<ColumnVector*>::iterator itr=_wg.begin(); itr  != _wg.end(); ++itr) {
            delete (*itr); (*itr)=0;
        }
        for (std::vector<ColumnVector*>::iterator itr=_deltag.begin(); itr  != _deltag.end(); ++itr) {
            delete (*itr); (*itr)=0;
        }
    } catch (...){}
    //TODO: delete objects in vectors of pointers
}

/** Calculates the full loglikelihood ratio/test statistic given passed maximizing value and data set index. 
    The passed matrices are assumed to be in correct relationship such linear algebric operations are valid. */
double WeightedNormalCovariatesLikelihoodCalculator::CalcLogLikelihoodRatioNormal(Matrix& xg, Matrix& tobeinversed, Matrix& xgsigmaw, size_t tDataSetIndex) const {
   double maximizingValue = CalculateMaximizingValueNormal(xg, tobeinversed, xgsigmaw, tDataSetIndex);
   if (maximizingValue != -std::numeric_limits<double>::max())
      return gvDataSetTotals[tDataSetIndex].first/2.0 *(maximizingValue - _dataSetMaximize[tDataSetIndex]);
   return maximizingValue;
}

/** Calculates maximizing value given passed matrices and data set index. The passed matrices are assumed to be
    in correct relationship such linear algebric operations are valid. */
double WeightedNormalCovariatesLikelihoodCalculator::CalculateMaximizingValueNormal(Matrix& xg, Matrix& tobeinversed, Matrix& xgsigmaw, size_t tDataSetIndex) const {
    Matrix temp_matrix = tobeinversed.i() * xgsigmaw;
    switch (gDataHub.GetParameters().GetExecuteScanRateType()) {
        case LOW        : if (temp_matrix.element(0,0) >= 0) 
                              return -std::numeric_limits<double>::max(); 
                          break;
        case HIGHANDLOW : if (temp_matrix.element(0,0) == 0) 
                              return -std::numeric_limits<double>::max(); 
                          break;
        case HIGH       : 
        default         : if (temp_matrix.element(0,0) <= 0) 
                              return -std::numeric_limits<double>::max(); 
                          break;
    };

    Matrix& wg = *(_wg.at(tDataSetIndex));
    Matrix& deltag = *(_deltag.at(tDataSetIndex));
    temp_matrix = xg * temp_matrix;
    temp_matrix = wg - temp_matrix;
    temp_matrix = SP(temp_matrix, temp_matrix);
    //matrix_best.element(i,j) = -log((deltag.t() * temp_matrix).AsScalar());

    return -log((deltag.t() * temp_matrix).AsScalar());
}

/** Calculates the full loglikelihood ratio/test statistic given passed maximizing value and data set index. */
double WeightedNormalCovariatesLikelihoodCalculator::CalculateFullStatistic(double dMaximizingValue, size_t tSetIndex) const {
    if (dMaximizingValue != -std::numeric_limits<double>::max()) 
        return gvDataSetTotals[tSetIndex].first/2.0 *(dMaximizingValue - _dataSetMaximize[tSetIndex]);
    return 0.0;
}
