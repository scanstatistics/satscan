//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "BernoulliModel.h"                                                

/** Constructor */
CBernoulliModel::CBernoulliModel(const CParameters& Parameters, CSaTScanData& Data, BasePrint& PrintDirection)
                :CModel(Parameters, Data, PrintDirection) {}

/** Destructor */                
CBernoulliModel::~CBernoulliModel() {}

/** calculates expected number of cases */
void CBernoulliModel::CalculateMeasure(RealDataSet& DataSet) {
  int                   i, j, k;
  count_t               tTotalCases(0), tTotalControls(0),
                     ** ppCases(DataSet.GetCaseArray()),
                     ** ppControls(DataSet.GetControlArray());
  measure_t          ** ppMeasure, tTotalMeasure(0);

  try {
    DataSet.AllocateMeasureArray();
    ppMeasure = DataSet.GetMeasureArray();

    for (j=0; j < gDataHub.GetNumTracts(); ++j) {
       tTotalCases    += ppCases[0][j];
       tTotalControls += ppControls[0][j];
       for (i=0; i < gDataHub.m_nTimeIntervals/*+1*/; ++i) {
          ppMeasure[i][j]  = ppCases[i][j] + ppControls[i][j];
       }
       tTotalMeasure += ppMeasure[0][j];
       ppMeasure[i][j] = 0;

       // Check to see if total case or control values have wrapped
        if (tTotalCases < 0)
          GenerateResolvableException("Error: : The total number of cases in dataset %u is greater than the maximum allowed of %ld.\n",
                                      "CBernoulliModel", DataSet.GetSetIndex(), std::numeric_limits<count_t>::max());
        if (tTotalControls < 0)
          GenerateResolvableException("Error: The total number of controls in dataset %u is greater than the maximum allowed of %ld.\n",
                                      "CBernoulliModel", DataSet.GetSetIndex(), std::numeric_limits<count_t>::max());
    }

    if (tTotalControls == 0)
      GenerateResolvableException("Error: No controls found in data set %u.\n", "CBernoulliModel", DataSet.GetSetIndex());

    DataSet.SetTotalCases(tTotalCases);
    DataSet.SetTotalControls(tTotalControls);
    DataSet.SetTotalMeasure(tTotalMeasure);
    DataSet.SetTotalPopulation(tTotalMeasure);
  }
  catch (ZdException &x) {
    x.AddCallpath("CalculateMeasure()","CBernoulliModel");
    throw;
  }
}

/** returns population for a given ellipse offset, grid point and time interval period */
double CBernoulliModel::GetPopulation(size_t tSetIndex, int m_iEllipseOffset, tract_t nCenter,
                                      tract_t nTracts, int nStartInterval, int nStopInterval) const {
  double                nPop=0.0;
  count_t               nNeighbor;
  measure_t          ** ppMeasure(gDataHub.GetDataSetHandler().GetDataSet(tSetIndex).GetMeasureArray());

  for (int i=1; i <= nTracts; ++i) {
     nNeighbor = gDataHub.GetNeighbor(m_iEllipseOffset, nCenter, i);
     nPop += ppMeasure[nStartInterval][nNeighbor] - ppMeasure[nStopInterval][nNeighbor];
  }

  return nPop;
}

