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
void CSpaceTimePermutationModel::CalculateMeasure(RealDataStream & thisStream) {
  int                   i, j, c, iNumCategories(thisStream.GetPopulationData().GetNumPopulationCategories());
  count_t               tTotalCases(0),
                     ** ppCases(thisStream.GetCaseArray()),
                    *** pppCategoryCases(thisStream.GetCategoryCaseArray());
  measure_t          ** ppMeasure, T_C, tTotalMeasure(0);
  PopulationData      & Population = thisStream.GetPopulationData();

  try {
    //calculate total number of cases
    for (j=0; j < gData.m_nTracts; j++) {
       tTotalCases += ppCases[0][j];
       // Check to see if total case or control values have wrapped
       if (tTotalCases < 0)
         SSGenerateException("Error: Total cases greater than maximum allowed of %ld.\n", "CalculateMeasure()", std::numeric_limits<count_t>::max());
    }

    thisStream.AllocateMeasureArray();
    ppMeasure = thisStream.GetMeasureArray();
    // set ppMeasure[i] = S*T/C (expected number of cases in a time/tract)
    // S = number of cases in spacial area irrespective of time
    // T = number of cases in temporal domain irrespective of location
    // C = total number of cases
    for (c=0; c < iNumCategories; ++c) {
       for (i=0; i < gData.m_nTimeIntervals; ++i) {
         T_C = pppCategoryCases[i][0][c];
         //Calculate T/C
         for (j=1; j < gData.m_nTracts; ++j)
            T_C += pppCategoryCases[i][j][c];
         T_C /= Population.GetNumCategoryCases(c);
         //Multiply T/C by S and add to measure
         for (j=0; j < gData.m_nTracts; ++j)
           ppMeasure[i][j] += T_C * pppCategoryCases[0][j][c];
       }
    }

    // calculate total measure
    for (j=0; j< gData.m_nTracts; ++j)
       tTotalMeasure += ppMeasure[0][j];

    // Ensure that TotalCases=TotalMeasure
    if (fabs(tTotalCases - tTotalMeasure)>0.0001)
      SSGenerateException("Error: The total measure is not equal to the total number of cases.\n"
                          "Total Cases = %i, Total Measure = %.2lf\n", "CalculateMeasure()",
                          tTotalCases, tTotalMeasure);

    thisStream.SetTotalCases(tTotalCases);
    thisStream.SetTotalControls(0);
    thisStream.SetTotalMeasure(tTotalMeasure);
    thisStream.SetTotalPopulation(0);
  }
  catch (ZdException &x) {
    x.AddCallpath("CalculateMeasure()","CSpaceTimePermutationModel");
    throw;
  }
}

