#include "SaTScan.h"
#pragma hdrstop
#include "BernoulliModel.h"                                                

#define DEBUG 1

/** Constructor */
CBernoulliModel::CBernoulliModel(CParameters& Parameters, CSaTScanData& Data, BasePrint& PrintDirection)
                :CModel(Parameters, Data, PrintDirection) {}

/** Destructor */                
CBernoulliModel::~CBernoulliModel() {}

/** calculates expected number of cases */
bool CBernoulliModel::CalculateMeasure(DataStream & thisStream) {
  int                   i, j, k;
  count_t               tTotalCases(0), tTotalControls(0),
                     ** ppCases(thisStream.GetCaseArray()),
                     ** ppControls(thisStream.GetControlArray());
  measure_t          ** ppMeasure, tTotalMeasure(0);

  try {
    thisStream.AllocateMeasureArray();
    ppMeasure = thisStream.GetMeasureArray();

    for (j=0; j < gData.m_nTracts; ++j) {
       tTotalCases    += ppCases[0][j];
       tTotalControls += ppControls[0][j];
       for (i=0; i < gData.m_nTimeIntervals/*+1*/; ++i) {
          ppMeasure[i][j]  = ppCases[i][j] + ppControls[i][j];
       }
       tTotalMeasure += ppMeasure[0][j];
       ppMeasure[i][j] = 0;

       // Check to see if total case or control values have wrapped
        if (tTotalCases < 0)
          SSGenerateException("Error: Total cases in data stream %u greater than maximum allowed of %ld.\n",
                              "CBernoulliModel", thisStream.GetStreamIndex(), std::numeric_limits<count_t>::max());
        if (tTotalControls < 0)
          SSGenerateException("Error: Total controls in data stream %u greater than maximum allowed of %ld.\n",
                              "CBernoulliModel", thisStream.GetStreamIndex(), std::numeric_limits<count_t>::max());
    }

    if (tTotalControls == 0)
      SSGenerateException("Error: No controls found in input stream %u.\n", "CBernoulliModel", thisStream.GetStreamIndex());

    thisStream.SetTotalCases(tTotalCases);
    thisStream.SetTotalControls(tTotalControls);
    thisStream.SetTotalMeasure(tTotalMeasure);
    thisStream.SetTotalPopulation(tTotalMeasure);
  }
  catch (ZdException &x) {
    x.AddCallpath("CalculateMeasure()","CBernoulliModel");
    throw;
  }
  return true;
}

/** returns population for a given ellipse offset, grid point and time interval period */
double CBernoulliModel::GetPopulation(int m_iEllipseOffset, tract_t nCenter, tract_t nTracts, int nStartInterval, int nStopInterval) {
  double                nPop=0.0;
  count_t               nNeighbor;
  measure_t          ** ppMeasure(gData.GetDataStreamHandler().GetStream(0/*for now*/).GetMeasureArray());

  for (int i=1; i <= nTracts; ++i) {
     nNeighbor = gData.GetNeighbor(m_iEllipseOffset, nCenter, i);
     nPop += ppMeasure[nStartInterval][nNeighbor] - ppMeasure[nStopInterval][nNeighbor];
  }

  return nPop;
}

