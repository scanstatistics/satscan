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
    gData.gPopulationCategories.SetAggregateCategories(true); // temporary measure to prevent covariates from being read
    if (!gData.ReadCoordinatesFile())
      return false;
    if (! gData.ReadCaseFile())
      return false;
    if (! gData.ReadControlFile())
      return false;
//    //synchronize case and control structures that breakdown data into categories
//    //so that we access them with same last dimension -- for speed reasons
//    ThreeDimensionArrayHandler<count_t>::SynchronizeThirdDimension(*(gData.gpCategoryCasesHandler), *(gData.gpCategoryControlsHandler), 0);
//    if (gParameters.GetTimeTrendAdjustmentType() == STRATIFIED_RANDOMIZATION)
//      TwoDimensionArrayHandler<count_t>::SynchronizeSecondDimension(*(gData.gpCasesByTimeByCategoryHandler), *(gData.gpControlsByTimeByCategoryHandler), 0);
    if (DoesReadMaxCirclePopulationFile() && !gData.ReadMaxCirclePopulationFile())
        return false;
    if (gParameters.UseSpecialGrid() && !gData.ReadGridFile())
      return false;
  }
  catch (ZdException &x) {
    x.AddCallpath("ReadData()", "CBernoulliModel");
    throw;
  }
  return true;
}

/** calculates expected number of cases */
bool CBernoulliModel::CalculateMeasure() {
  int                   i, j, k;
  count_t            ** ppCases(gData.GetCasesArray())/*,
                    *** pppCategoryCases(gData.gpCategoryCasesHandler->GetArray()),
                     ** ppCasesByTimeByCategory(0)*/;
  count_t            ** ppControls(gData.GetControlsArray())/*,
                    *** pppCategoryControls(gData.gpCategoryControlsHandler->GetArray()),
                     ** ppControlsByTimeByCategory(0)*/;
  measure_t          ** ppMeasure/*, *** pppCategoryMeasure, ** ppMeasureByTimeByCategory=0*/;

  try {
    gData.gpMeasureHandler = new TwoDimensionArrayHandler<measure_t>(gData.m_nTimeIntervals+1, gData.m_nTracts);
    ppMeasure = gData.gpMeasureHandler->GetArray();
//    gData.gpCategoryMeasureHandler = new ThreeDimensionArrayHandler<measure_t>(gData.m_nTimeIntervals+1, gData.m_nTracts, gData.gPopulationCategories.GetNumPopulationCategories());
//    pppCategoryMeasure = gData.gpCategoryMeasureHandler->GetArray();
//    if (gParameters.GetTimeTrendAdjustmentType() == STRATIFIED_RANDOMIZATION) {
//      gData.gpMeasureByTimeByCategoryHandler = new TwoDimensionArrayHandler<measure_t>(gData.m_nTimeIntervals, gData.gPopulationCategories.GetNumPopulationCategories());
//      ppMeasureByTimeByCategory = gData.gpMeasureByTimeByCategoryHandler->GetArray();
//      ppCasesByTimeByCategory = gData.gpCasesByTimeByCategoryHandler->GetArray();
//      ppControlsByTimeByCategory = gData.gpControlsByTimeByCategoryHandler->GetArray();
//    }
    gData.m_nTotalCases    = 0;
    gData.m_nTotalControls = 0;
    gData.m_nTotalMeasure  = 0;

    for (j=0; j < gData.m_nTracts; ++j) {
       gData.m_nTotalCases    += ppCases[0][j];
       gData.m_nTotalControls += ppControls[0][j];
       for (i=0; i < gData.m_nTimeIntervals/*+1*/; ++i) {
          ppMeasure[i][j]  = ppCases[i][j] + ppControls[i][j];
       }
       gData.m_nTotalMeasure += ppMeasure[0][j];
       ppMeasure[i][j] = 0;

       // Check to see if total case or control values have wrapped
        if (gData.m_nTotalCases < 0)
          SSGenerateException("Error: Total cases greater than maximum allowed of %ld.\n", "CBernoulliModel", std::numeric_limits<count_t>::max());
        if (gData.m_nTotalControls < 0)
          SSGenerateException("Error: Total controls greater than maximum allowed of %ld.\n", "CBernoulliModel", std::numeric_limits<count_t>::max());
    }

    if (gData.m_nTotalControls == 0)
      SSGenerateException("Error: No controls found in input data.\n", "CBernoulliModel");

//    for (k=0; k < gData.GetPopulationCategories().GetNumPopulationCategories(); ++k) {
//       for (i=0; i < gData.m_nTimeIntervals; ++i) {
//          for (j=0; j < gData.m_nTracts; ++j)
//             pppCategoryMeasure[i][j][k] = pppCategoryCases[i][j][k] + pppCategoryControls[i][j][k];
//          if (ppMeasureByTimeByCategory)
//            ppMeasureByTimeByCategory[i][k] = ppCasesByTimeByCategory[i][k] + ppControlsByTimeByCategory[i][k];
//       }
//    }

    gData.m_nTotalPop = gData.m_nTotalMeasure;

//    if (gParameters.GetTimeTrendAdjustmentType() == STRATIFIED_RANDOMIZATION ||
//        gParameters.GetTimeTrendAdjustmentType() == CALCULATED_LOGLINEAR_PERC ||
//        gParameters.GetAnalysisType() == SPATIALVARTEMPTREND)
//      //need measure by time intervals for time stratified adjustment
//      gData.SetMeasureByTimeIntervalArray();
//    if (gParameters.GetAnalysisType() == SPATIALVARTEMPTREND)
//      //procedure above produces cumulative measure, for spatial variation analysis
//      //we also need the non-cumulative measure/
//      gData.SetNonCumulativeMeasure();
  }
  catch (ZdException &x) {
    delete gData.gpMeasureHandler; gData.gpMeasureHandler=0;
//    delete gData.gpCategoryMeasureHandler; gData.gpCategoryMeasureHandler=0;
//    delete gData.gpMeasureByTimeByCategoryHandler; gData.gpMeasureByTimeByCategoryHandler=0;
    x.AddCallpath("CalculateMeasure()","CBernoulliModel");
    throw;
  }
  return true;
}

/** calculates loglikelihood for entire population */
double CBernoulliModel::GetLogLikelihoodForTotal() const {
  count_t   N = gData.m_nTotalCases;
  measure_t U = gData.m_nTotalMeasure;

  return N*log(N/U) + (U-N)*log((U-N)/U);
}

/** calculates loglikelihood given passed count and measure */
double CBernoulliModel::CalcLogLikelihood(count_t n, measure_t u) {
  double    nLogLikelihood;
  count_t   N = gData.m_nTotalCases;
  measure_t U = gData.m_nTotalMeasure;

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
void CBernoulliModel::MakeData(int iSimulationNumber) {
  //reset seed to simulation number
  m_RandomNumberGenerator.SetSeed(iSimulationNumber + m_RandomNumberGenerator.GetDefaultSeed());
//  if (gParameters.GetTimeTrendAdjustmentType() == STRATIFIED_RANDOMIZATION)
//    MakeDataTimeStratified();
//  else
    MakeDataUnderNullHypothesis();
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

/** Generates simulation data in a time stratified manner(i.e. data created
    with time intervals evaluated independent of each other. */
void CBernoulliModel::MakeDataTimeStratified() {
  tract_t               t;         // current tract
  int                   i, k;      // current time interval
  count_t               c, cumcases=0,
                     ** ppSimCases(gData.GetSimCasesArray()),
                     ** ppCasesByTimeByCategory(gData.gpCasesByTimeByCategoryHandler->GetArray());
  measure_t             cummeasure=0,
                     ** ppMeasureByTimeByCategory(gData.gpMeasureByTimeByCategoryHandler->GetArray()),
                    *** pppCategoryMeasure(gData.gpCategoryMeasureHandler->GetArray());

  // reset simulation cases to zero
  for (i=0; i < gData.m_nTimeIntervals; ++i)
     for (t=0; t < gData.m_nTotalTractsAtStart; ++t)
        ppSimCases[i][t] = 0;

  i = gData.m_nTimeIntervals - 1;
  for (k=0; k < gData.GetPopulationCategories().GetNumPopulationCategories(); ++k) {
     for (t=0; t < gData.m_nTotalTractsAtStart; ++t) {
        if (ppCasesByTimeByCategory[i][k] - cumcases > 0)
          c = gBinomialGenerator.GetBinomialDistributedVariable(ppCasesByTimeByCategory[i][k] - cumcases,
                                                                pppCategoryMeasure[i][t][k]/(ppMeasureByTimeByCategory[i][k] - cummeasure),
                                                                m_RandomNumberGenerator);
        else
           c = 0;

        cumcases += c;
        cummeasure += pppCategoryMeasure[i][t][k];
        ppSimCases[i][t] += c;
     }
  }

  for (k=0; k < gData.GetPopulationCategories().GetNumPopulationCategories(); ++k) {
     for (i--; i >= 0; --i) { //For each other interval, from 2nd to last until the first:
        cumcases = 0;
        cummeasure = 0;
        for (t=0; t < gData.m_nTotalTractsAtStart; ++t) { //For each tract:
          if (ppCasesByTimeByCategory[i][k] - cumcases > 0)
            c = gBinomialGenerator.GetBinomialDistributedVariable(ppCasesByTimeByCategory[i][k] - cumcases,
                                                                  (pppCategoryMeasure[i][t][k] - pppCategoryMeasure[i + 1][t][k])/(ppMeasureByTimeByCategory[i][k] - cummeasure),
                                                                   m_RandomNumberGenerator);
          else
            c = 0;

          cumcases += c;
          cummeasure += (pppCategoryMeasure[i][t][k] - pppCategoryMeasure[i + 1][t][k]);
          ppSimCases[i][t] += c;
        }
     }
  }
  //set as cumulative
  for (i=gData.m_nTimeIntervals-2; i >= 0; i--)       
     for (t=0; t < gData.m_nTotalTractsAtStart; t++)
        ppSimCases[i][t]= ppSimCases[i+1][t] + ppSimCases[i][t];
}

/** standard procedure for randomized data */
void CBernoulliModel::MakeDataUnderNullHypothesis() {
  tract_t                       t;
  int                           i, c;
  count_t                    ** ppSimCases(gData.GetSimCasesArray()), nCumCounts, nCumMeasure, tNumCases, tNumControls;
  measure_t                  ** ppMeasure(gData.gpMeasureHandler->GetArray())/*,
                            *** pppCategoryMeasure(gData.gpCategoryMeasureHandler->GetArray())*/;
  std::vector<count_t>          RandCounts;

  // reset simulation cases to zero
  gData.gpSimCasesHandler->Set(0);

//  for (c=0; c < gData.gPopulationCategories.GetNumPopulationCategories(); ++c) {
     tNumCases = gData.m_nTotalCases; //gData.gPopulationCategories.GetNumCategoryCases(c);
     tNumControls = gData.m_nTotalControls; //gData.gPopulationCategories.GetNumCategoryControls(c);
     //use counts from entire data set to determine whether cases or controls are used in MakeDataB()
     if (gData.m_nTotalCases < gData.m_nTotalControls)
       nCumCounts = tNumCases;
     else
       nCumCounts = tNumControls;

     MakeDataB(nCumCounts, tNumCases + tNumControls, RandCounts);
     nCumMeasure = tNumCases + tNumControls - 1;
     for (t=(tract_t)(gData.m_nTotalTractsAtStart-1); t >= 0; --t) {
        for (i = gData.m_nTimeIntervals-1; i >= 0; --i) {
           if (i == gData.m_nTimeIntervals-1)
             nCumMeasure -= (count_t)(ppMeasure[i][t]/*pppCategoryMeasure[i][t][c]*/);
           else
             //nCumMeasure -= (count_t)(pppCategoryMeasure[i][t][c] - pppCategoryMeasure[i+1][t][c]);
             nCumMeasure -= (count_t)(ppMeasure[i][t] - ppMeasure[i+1][t]);
           while (nCumCounts > 0 && RandCounts[nCumCounts-1] > nCumMeasure) {
                ppSimCases[i][t]++;
                nCumCounts--;
           }
        }
     }
//  }

  //set as cumulative
  for (i=gData.m_nTimeIntervals-2; i >= 0; i--)
     for (t=0; t < gData.m_nTotalTractsAtStart; t++)
        ppSimCases[i][t] += ppSimCases[i+1][t];

  // Now reverse everything if Controls < Cases
  if (gData.m_nTotalCases >= gData.m_nTotalControls) {
    for (t=0; t < gData.m_nTotalTractsAtStart; ++t)
       for (i=0; i < gData.m_nTimeIntervals; ++i)
          ppSimCases[i][t] = (count_t)(ppMeasure[i][t]) - ppSimCases[i][t];
  }
}

/** returns population for a given ellipse offset, grid point and time interval period */
double CBernoulliModel::GetPopulation(int m_iEllipseOffset, tract_t nCenter, tract_t nTracts, int nStartInterval, int nStopInterval) {
  double                nPop=0.0;
  count_t               nNeighbor;
  measure_t          ** ppMeasure(gData.GetMeasureArray());

  for (int i=1; i <= nTracts; ++i) {
     nNeighbor = gData.GetNeighbor(m_iEllipseOffset, nCenter, i);
     nPop += ppMeasure[nStartInterval][nNeighbor] - ppMeasure[nStopInterval][nNeighbor];
  }

  return nPop;
}

