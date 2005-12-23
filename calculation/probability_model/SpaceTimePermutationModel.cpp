//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "SpaceTimePermutationModel.h"
#include "SSException.h"

/** constructor */
CSpaceTimePermutationModel::CSpaceTimePermutationModel() : CModel() {}

/** destructor */
CSpaceTimePermutationModel::~CSpaceTimePermutationModel() {}

/** Determines the expected number of cases for each time interval/tract.
    Assigns values to CSatScanData::Measure array. Calculates total measure
    and validates that total measure equals total number of cases in set. */
void CSpaceTimePermutationModel::CalculateMeasure(RealDataSet& DataSet) {
  unsigned int          i, j, c, iNumCategories(DataSet.GetPopulationData().GetNumCovariateCategories());
  count_t               tTotalCases(0), ** ppCases=0;
  measure_t          ** ppMeasure, T_C, tTotalMeasure(0);
  PopulationData      & Population = DataSet.GetPopulationData();

  try {
    //calculate total number of cases
    for (size_t t=0; t < DataSet.GetCasesByCategory().size(); ++t) {
       ppCases = DataSet.GetCategoryCaseArray(t);
       for (j=0; j < DataSet.GetNumTracts(); ++j) {
          tTotalCases += ppCases[0][j];
          // Check to see if total case values have wrapped
          if (tTotalCases < 0)
            GenerateResolvableException("Error: The total number of cases in dataset %u is greater than the maximum allowed of %ld.\n",
                                        "CalculateMeasure()", DataSet.GetSetIndex(), std::numeric_limits<count_t>::max());
       }
    }

    DataSet.AllocateMeasureArray();
    ppMeasure = DataSet.GetMeasureArray();
    // set ppMeasure[i] = S*T/C (expected number of cases in a time/tract)
    // S = number of cases in spacial area irrespective of time
    // T = number of cases in temporal domain irrespective of location
    // C = total number of cases
    for (c=0; c < iNumCategories; ++c) {
       ppCases = DataSet.GetCategoryCaseArray(c);
       if (Population.GetNumCovariateCategoryCases(c)) {
         for (i=0; i < DataSet.GetNumTimeIntervals(); ++i) {
            T_C = ppCases[i][0];
            //Calculate T/C
            for (j=1; j < DataSet.GetNumTracts(); ++j)
               T_C += ppCases[i][j];
            T_C /= Population.GetNumCovariateCategoryCases(c);
            //Multiply T/C by S and add to measure
            for (j=0; j < DataSet.GetNumTracts(); ++j)
               ppMeasure[i][j] += T_C * ppCases[0][j];
         }
       }
    }

    // calculate total measure
    for (j=0; j< DataSet.GetNumTracts(); ++j)
       tTotalMeasure += ppMeasure[0][j];

    // Ensure that TotalCases=TotalMeasure
    if (fabs(tTotalCases - tTotalMeasure) > 0.0001)
      ZdGenerateException("Error: The total measure is not equal to the total number of cases.\n"
                          "       Total Cases = %i, Total Measure = %.2lf.\n",
                          "CalculateMeasure()", tTotalCases, tTotalMeasure);

    DataSet.SetTotalCases(tTotalCases);
    DataSet.SetTotalControls(0);
    DataSet.SetTotalMeasure(tTotalMeasure);
    DataSet.SetTotalPopulation(0);
  }
  catch (ZdException &x) {
    x.AddCallpath("CalculateMeasure()","CSpaceTimePermutationModel");
    throw;
  }
}

