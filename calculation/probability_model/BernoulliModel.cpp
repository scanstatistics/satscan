#include "SaTScan.h"
#pragma hdrstop
#include "BernoulliModel.h"                                                

#define DEBUG 1

/** Constructor */
CBernoulliModel::CBernoulliModel(CParameters& Parameters, CSaTScanData& Data, BasePrint& PrintDirection)
                :CModel(Parameters, Data, PrintDirection) {}

/** Destructor */                
CBernoulliModel::~CBernoulliModel() {}

/** calls appropriate CSaTScanData methods for reading input files */
bool CBernoulliModel::ReadData() {
  try {
    if (!gData.ReadBernoulliData())
      return false;
  }
  catch (ZdException &x) {
    x.AddCallpath("ReadData()", "CBernoulliModel");
    throw;
  }
  return true;
}

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
          SSGenerateException("Error: Total cases greater than maximum allowed of %ld.\n", "CBernoulliModel", std::numeric_limits<count_t>::max());
        if (tTotalControls < 0)
          SSGenerateException("Error: Total controls greater than maximum allowed of %ld.\n", "CBernoulliModel", std::numeric_limits<count_t>::max());
    }

    if (tTotalControls == 0)
      SSGenerateException("Error: No controls found in input data.\n", "CBernoulliModel");

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

/** calculates loglikelihood for entire population */
double CBernoulliModel::GetLogLikelihoodForTotal() const {
  count_t   N = gData.GetTotalCases();
  measure_t U = gData.GetTotalMeasure();

  return N*log(N/U) + (U-N)*log((U-N)/U);
}

/** calculates loglikelihood given passed count and measure */
double CBernoulliModel::CalcLogLikelihood(count_t n, measure_t u) {
  double    nLogLikelihood;
  count_t   N = gData.GetTotalCases();
  measure_t U = gData.GetTotalMeasure();

  double    nLL_A = 0.0;
  double    nLL_B = 0.0;
  double    nLL_C = 0.0;
  double    nLL_D = 0.0;

  if (n != 0)
    nLL_A = n*log(n/u);
  if (n != u)
    nLL_B = (u-n)*log(1-(n/u));
  if (N-n != 0)
    nLL_C = (N-n)*log((N-n)/(U-u));
   if (N-n != U-u)
    nLL_D = ((U-u)-(N-n))*log(1-((N-n)/(U-u)));

  nLogLikelihood = nLL_A + nLL_B + nLL_C + nLL_D;

  return (nLogLikelihood);
}

/** calculates loglikelihood given passed count and measure */
double CBernoulliModel::CalcLogLikelihoodRatio(count_t tCases, measure_t tMeasure, count_t tTotalCases, measure_t tTotalMeasure, double dCompactnessCorrection) {
  double    dLogLikelihood;
  double    nLL_A = 0.0;
  double    nLL_B = 0.0;
  double    nLL_C = 0.0;
  double    nLL_D = 0.0;

  // calculate the loglikelihood
  if (tCases != 0)
    nLL_A = tCases*log(tCases/tMeasure);
  if (tCases != tMeasure)
    nLL_B = (tMeasure-tCases)*log(1-(tCases/tMeasure));
  if (tTotalCases-tCases != 0)
    nLL_C = (tTotalCases-tCases)*log((tTotalCases-tCases)/(tTotalMeasure-tMeasure));
   if (tTotalCases-tCases != tTotalMeasure-tMeasure)
    nLL_D = ((tTotalMeasure-tMeasure)-(tTotalCases-tCases))*log(1-((tTotalCases-tCases)/(tTotalMeasure-tMeasure)));

  dLogLikelihood = nLL_A + nLL_B + nLL_C + nLL_D;

  // return the logliklihood ratio (loglikelihood - loglikelihood for total) * duczmal compactness correction
  return (dLogLikelihood - (tTotalCases*log(tTotalCases/tTotalMeasure) + (tTotalMeasure-tTotalCases)*log((tTotalMeasure-tTotalCases)/tTotalMeasure))) * dCompactnessCorrection;
}

/** calculates loglikelihood ratio for purely spatial monotone analysis given passed cluster */
double CBernoulliModel::CalcMonotoneLogLikelihood(const CPSMonotoneCluster& PSMCluster) {
  double    nLogLikelihood = 0;
  count_t   n;
  measure_t u;

  for (int i=0; i < PSMCluster.m_nSteps; ++i) {
     n = PSMCluster.m_pCasesList[i];
     u = PSMCluster.m_pMeasureList[i];
     if (n != 0  && n != u)
       nLogLikelihood += n*log(n/u) + (u-n)*log(1-(n/u));
     else if (n == 0)
       nLogLikelihood += (u-n)*log(1-(n/u));
     else if (n == u)
       nLogLikelihood += n*log(n/u);
  }

  return nLogLikelihood;
}

/** Generates simulation data. */
void CBernoulliModel::MakeData(int iSimulationNumber, DataStreamInterface & DataInterface, unsigned int tInterface) {
  //reset seed to simulation number
  m_RandomNumberGenerator.SetSeed(iSimulationNumber + m_RandomNumberGenerator.GetDefaultSeed());
  MakeDataUnderNullHypothesis(DataInterface);
}

/*********************************************************************
 Each of the totalMeasure number of individuals (sum of cases and
 controls), are randomized to either be a case or a control. The
 output is an array with the indices of the TotalCounts number of
 cases. For example, if there are 20 cases and 80 controls, the
 output is an array the the indices between 0 and 99 that correspond
 to the randomized cases. (MK Oct 27, 2003)
 *********************************************************************/
void CBernoulliModel::MakeDataB(count_t tTotalCounts, measure_t tTotalMeasure, std::vector<count_t>& RandCounts) {
  int           i;
  count_t       nCumCounts=0;
  double        x, ratio;

  RandCounts.resize(tTotalCounts);
  for (i=0; i < tTotalMeasure; ++i) {
     x = m_RandomNumberGenerator.GetRandomDouble();
     ratio = (double) (tTotalCounts - nCumCounts) / (tTotalMeasure - i);
     if (x <= ratio) {
       RandCounts[nCumCounts] = i;
       nCumCounts++;
     }
  }
}

/** standard procedure for randomized data */
void CBernoulliModel::MakeDataUnderNullHypothesis(DataStreamInterface & DataInterface) {
  tract_t                       t;
  int                           i, c;
  count_t                    ** ppSimCases(DataInterface.GetCaseArray()),
                                nCumCounts, nCumMeasure, tNumCases, tNumControls;
  measure_t                  ** ppMeasure(DataInterface.GetMeasureArray());
  std::vector<count_t>          RandCounts;

  // reset simulation cases to zero
  DataInterface.ResetCaseArray(0);

  tNumCases = DataInterface.GetTotalCasesCount();
  tNumControls = DataInterface.GetTotalControlsCount();
  //use counts from entire data set to determine whether cases or controls are used in MakeDataB()
  if (DataInterface.GetTotalCasesCount() < DataInterface.GetTotalControlsCount())
    nCumCounts = tNumCases;
  else
    nCumCounts = tNumControls;

  MakeDataB(nCumCounts, tNumCases + tNumControls, RandCounts);
  nCumMeasure = tNumCases + tNumControls - 1;
  for (t=(tract_t)(gData.m_nTotalTractsAtStart-1); t >= 0; --t) {
     for (i = gData.m_nTimeIntervals-1; i >= 0; --i) {
        if (i == gData.m_nTimeIntervals-1)
          nCumMeasure -= (count_t)(ppMeasure[i][t]);
        else
          nCumMeasure -= (count_t)(ppMeasure[i][t] - ppMeasure[i+1][t]);
        while (nCumCounts > 0 && RandCounts[nCumCounts-1] > nCumMeasure) {
             ppSimCases[i][t]++;
             nCumCounts--;
        }
     }
  }

  //set as cumulative
  for (i=gData.m_nTimeIntervals-2; i >= 0; i--)
     for (t=0; t < gData.m_nTotalTractsAtStart; t++)
        ppSimCases[i][t] += ppSimCases[i+1][t];

  // Now reverse everything if Controls < Cases
  if (DataInterface.GetTotalCasesCount() >= DataInterface.GetTotalControlsCount()) {
    for (t=0; t < gData.m_nTotalTractsAtStart; ++t)
       for (i=0; i < gData.m_nTimeIntervals; ++i)
          ppSimCases[i][t] = (count_t)(ppMeasure[i][t]) - ppSimCases[i][t];
  }
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

