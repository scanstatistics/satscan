#include "SaTScan.h"
#pragma hdrstop
#include "SpaceTimePermutationModel.h"


/** constructor */
CSpaceTimePermutationModel::CSpaceTimePermutationModel(const CParameters& Parameters, CSaTScanData& Data, BasePrint& PrintDirection)
                           :CModel(Parameters, Data, PrintDirection) {}

/** destructor */
CSpaceTimePermutationModel::~CSpaceTimePermutationModel() {}

/** Determines the expected number of cases for each time interval/tract.
    Assigns values to CSatScanData::Measure array. Calculates total measure
    and validates that total measure equals total number of cases in set. */
void CSpaceTimePermutationModel::CalculateMeasure(RealDataStream& RealSet) {
  int                   i, j, c, iNumCategories(RealSet.GetPopulationData().GetNumCovariateCategories());
  count_t               tTotalCases(0), ** ppCases=0;
  measure_t          ** ppMeasure, T_C, tTotalMeasure(0);
  PopulationData      & Population = RealSet.GetPopulationData();

  try {
    //calculate total number of cases
    for (size_t t=0; t < RealSet.GetCasesByCategory().size(); ++t)
       ppCases = RealSet.GetCategoryCaseArray(t);
       for (j=0; j < gData.GetNumTracts(); ++j) {
          tTotalCases += ppCases[0][j];
       // Check to see if total case values have wrapped
       if (tTotalCases < 0)
         GenerateResolvableException("Error: The total number of cases in data set %u is greater than the maximum allowed of %ld.\n",
                                     "CalculateMeasure()", RealSet.GetStreamIndex(), std::numeric_limits<count_t>::max());
    }

    RealSet.AllocateMeasureArray();
    ppMeasure = RealSet.GetMeasureArray();
    // set ppMeasure[i] = S*T/C (expected number of cases in a time/tract)
    // S = number of cases in spacial area irrespective of time
    // T = number of cases in temporal domain irrespective of location
    // C = total number of cases
    for (c=0; c < iNumCategories; ++c) {
       ppCases = RealSet.GetCategoryCaseArray(c);
       if (Population.GetNumCovariateCategoryCases(c)) {
         for (i=0; i < gData.m_nTimeIntervals; ++i) {
            T_C = ppCases[i][0];
            //Calculate T/C
            for (j=1; j < gData.GetNumTracts(); ++j)
               T_C += ppCases[i][j];
            T_C /= Population.GetNumCovariateCategoryCases(c);
            //Multiply T/C by S and add to measure
            for (j=0; j < gData.GetNumTracts(); ++j)
               ppMeasure[i][j] += T_C * ppCases[0][j];
         }
       }
    }

    // calculate total measure
    for (j=0; j< gData.GetNumTracts(); ++j)
       tTotalMeasure += ppMeasure[0][j];

    // Ensure that TotalCases=TotalMeasure
    if (fabs(tTotalCases - tTotalMeasure) > 0.0001)
      ZdGenerateException("Error: The total measure is not equal to the total number of cases.\n"
                          "       Total Cases = %i, Total Measure = %.2lf.\n",
                          "CalculateMeasure()", tTotalCases, tTotalMeasure);

    RealSet.SetTotalCases(tTotalCases);
    RealSet.SetTotalControls(0);
    RealSet.SetTotalMeasure(tTotalMeasure);
    RealSet.SetTotalPopulation(0);
  }
  catch (ZdException &x) {
    x.AddCallpath("CalculateMeasure()","CSpaceTimePermutationModel");
    throw;
  }
}

