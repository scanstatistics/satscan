#include "SaTScan.h"
#pragma hdrstop
#include "PoissonModel.h"

using std::ios;
using std::ifstream;

/** constructor */
CPoissonModel::CPoissonModel(CParameters& Parameters, CSaTScanData& Data, BasePrint& PrintDirection)
              :CModel(Parameters, Data, PrintDirection){}

/** destructor */
CPoissonModel::~CPoissonModel(){}

/** Adjusts the expected number of cases for a time trend, using the data
    itself. For each adjustment interval, which has to be fewer than the
    number of time intervals, all measures are multiplied with a constant
    so that the total measure in all adjustment intervals are the same in
    proportion to its length in time.
    These operations are done on the raw measure matrix rather than the
    later on constructed cumulative measure matrix, while the cumulative
    case matrix is used.                                                */
void CPoissonModel::AdjustForNonParameteric(measure_t ** pNonCumulativeMeasure) {
  int     AdjustIntervals;
  int     i,j,jj,k,tract;
  double  sumcases,summeasure;

  AdjustIntervals = gData.m_nTimeIntervals;
  k  = 1;
  j  = 0;
  jj = 0;

  for (i=0; i<AdjustIntervals; i++)
  {
	 sumcases   = 0;
	 summeasure = 0;
    while (j < k*gData.m_nTimeIntervals/AdjustIntervals && j < gData.m_nTimeIntervals)
    {
      if (j == gData.m_nTimeIntervals-1)
        for (tract=0; tract<gData.m_nTracts; tract++)
        {
          sumcases   = sumcases + gData.m_pCases[j][tract];
          summeasure = summeasure + (pNonCumulativeMeasure)[j][tract];
        } /* for tract */
      else
        for (tract=0; tract<gData.m_nTracts; tract++)
        {
          sumcases   = sumcases + (gData.m_pCases[j][tract]-gData.m_pCases[j+1][tract]);
          summeasure = summeasure + (pNonCumulativeMeasure)[j][tract];
        } /* for tract */

        j++;
    }  /* while */

    while (jj < k*gData.m_nTimeIntervals/AdjustIntervals && jj < gData.m_nTimeIntervals)
    {
      for (tract = 0; tract<gData.m_nTracts; tract++)
        (pNonCumulativeMeasure)[jj][tract] =
          (pNonCumulativeMeasure)[jj][tract]*(sumcases/summeasure)/((gData.m_nTotalCases)/(gData.m_nTotalMeasure));

      jj++;
    }  /* while */

    k++;
  } /* for i<AdjustIntervals */

}

/** */
void CPoissonModel::AdjustForLLPercentage(measure_t ** pNonCumulativeMeasure, double nPercentage)
{
  int    i,t;
  double c;
  double k = IntervalInYears(gParameters.GetTimeIntervalUnitsType(), gParameters.GetTimeIntervalLength());
  double p = 1 + (nPercentage/100);
  double nAdjustedMeasure = 0;

  #if DEBUGMODEL
  fprintf(m_pDebugModelFile, "\nAdjust Measure for Time Trend - %f%% per year.\n\n", nPercentage);
  #endif

  /* Adjust the measure assigned to each interval/tract by yearly percentage */
  for (i=0; i<gData.m_nTimeIntervals; i++)
    for (t=0; t<gData.m_nTracts; t++)
    {
      (pNonCumulativeMeasure)[i][t] = (pNonCumulativeMeasure)[i][t]*(pow(p,i*k)) /* * c */ ;
      nAdjustedMeasure += (pNonCumulativeMeasure)[i][t];

      if (nAdjustedMeasure > DBL_MAX)
        SSGenerateException("Error: Data overflow due to time trend adjustment.\n","AdjustForLLPercentage()");
    }

  /* Mutlipy the measure for each interval/tract by constant (c) to obtain */
  /* total adjusted measure (nAdjustedMeasure) equal to previous total     */
  /* measure (gData.m_nTotalMeasure).                                             */
  c = (double)(gData.m_nTotalMeasure)/nAdjustedMeasure;
  nAdjustedMeasure=0;

  for (i=0; i<gData.m_nTimeIntervals; i++)
    for (t=0; t<gData.m_nTracts; t++)
    {
      (pNonCumulativeMeasure)[i][t] = (pNonCumulativeMeasure)[i][t]*c;
      nAdjustedMeasure += (pNonCumulativeMeasure)[i][t];
    }

}

void CPoissonModel::AdjustForLogLinear(measure_t ** pNonCumulativeMeasure)
{
/*  #if DEBUGMODEL
  fprintf(m_pDebugModelFile, "\nCalculate Time Trend and Adjust for Log Linear\n\n");
  #endif
*/
  // Calculate time trend for whole dataset
  gData.m_nTimeTrend.CalculateAndSet(gData.m_pCases_TotalByTimeInt,
                                        gData.m_pMeasure_TotalByTimeInt,
                                        gData.m_nTimeIntervals,
                                        gParameters.GetTimeTrendConvergence());

  // Global time trend will be recalculated after the measure is adjusted.
  // Therefore this value will be lost unless retain in parameters for
  // display in the report.
  gParameters.SetTimeTrendAdjustmentPercentage(gData.m_nTimeTrend.m_nBeta);
  AdjustForLLPercentage(pNonCumulativeMeasure, gData.m_nTimeTrend.m_nBeta);  // Adjust Measure             */
}

void CPoissonModel::AdjustMeasure(measure_t ** pMeasure) {
  try {
    if (gData.m_nTimeIntervals > 1) {
      switch (gParameters.GetTimeTrendAdjustmentType()) {
        case NOTADJUSTED               : break;
        case NONPARAMETRIC             : AdjustForNonParameteric(pMeasure); break;
        case LOGLINEAR_PERC            : AdjustForLLPercentage(pMeasure, gParameters.GetTimeTrendAdjustmentPercentage()); break;
        case CALCULATED_LOGLINEAR_PERC : gData.SetMeasureByTimeIntervalArray(pMeasure);
                                         AdjustForLogLinear(pMeasure); break;
        case STRATIFIED_RANDOMIZATION  : break;//this adjustment occurs during randomization
        default : ZdGenerateException("Unknown time trend adjustment type: '%d'.",
                                      "AdjustMeasure()", gParameters.GetTimeTrendAdjustmentType());
      }
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("AdjustMeasure","CPoissonModel");
    throw;
  }
}

double CPoissonModel::CalcLogLikelihood(count_t n, measure_t u)
{
   double    nLogLikelihood;
   count_t   N = gData.m_nTotalCases;
   measure_t U = gData.m_nTotalMeasure;

   if (n != N && n != 0)
     nLogLikelihood = n*log(n/u) + (N-n)*log((N-n)/(U-u));
   else if (n == 0)
     nLogLikelihood = (N-n) * log((N-n)/(U-u));
   else
     nLogLikelihood = n*log(n/u);

   return (nLogLikelihood);
}

double CPoissonModel::CalcMonotoneLogLikelihood(const CPSMonotoneCluster& PSMCluster)
{
  double nLogLikelihood = 0;

  for (int i=0; i<PSMCluster.m_nSteps; i++)
     {
     if (PSMCluster.m_pCasesList[i] != 0)
       nLogLikelihood += PSMCluster.m_pCasesList[i] * log(PSMCluster.m_pCasesList[i]/PSMCluster.m_pMeasureList[i]);
     }

   return nLogLikelihood;
}

/** NEEDS DOCUMENTATION */
double CPoissonModel::CalcSVTTLogLikelihood(count_t*   pCases,
                                            measure_t* pMeasure,
                                            count_t    pTotalCases,
                                            double     nAlpha,
                                            double     nBeta,
                                            int        nStatus)
{
  double nLL;

  /* Check for extremes where all the cases in the first or last interval
     or 0 cases in given tract.  For now there are multiple returns. */
  if (nStatus == TREND_UNDEF) // No cases in tract
  {
    nLL = 0;
  }
  else if (nStatus == TREND_INF) // All cases in first/last t.i.
  {
    nLL = (pTotalCases*log(pTotalCases) - pTotalCases )/**-
          ((CSVTTData&)gData).m_pSumLogs[pTotalCases])**/;
    // -x+x*log(x)-log(x!) from M Kulldorff
  }
  else
  {
    double nSum1 = 0;
    double nSum2 = 0;
    double nSum3 = 0;
    int i;

    for (i=0; i<(gData.m_nTimeIntervals); i++)
    {
      nSum1 += pCases[i] * (log(pMeasure[i]) + nAlpha + (nBeta)*i);
      nSum2 += pMeasure[i] * exp(nAlpha + (nBeta)*i);
      /**nSum3 += ((CSVTTData&)gData).m_pSumLogs[pCases[i]]; // Modified 990916 GG**/
    }
    nLL = nSum1-nSum2-nSum3;
  }

  #if DEBUGMODEL
  fprintf(m_pDebugModelFile, "Alpha = %f  nBeta = %f  nStatus = %i  LogLikelihood = %f\n", nAlpha, nBeta, nStatus, nLL);
  #endif

  return(nLL);
}

/** NEEDS DOCUMENTATION */
double CPoissonModel::CalcSVTTLogLikelihood(CSVTTCluster* Cluster, CTimeTrend GlobalTimeTrend)
{
  double nLogLikelihood   = 0.0;
  double nGlobalAlphaIn   = 0.0;
  double nGlobalAlphaOut = 0.0;

  Cluster->m_nTimeTrend.CalculateAndSet(Cluster->m_pCumCases,         // Inside Cluster
                                        Cluster->m_pCumMeasure,
                                        gData.m_nTimeIntervals,
                                        gParameters.GetTimeTrendConvergence());

  nGlobalAlphaIn = Cluster->m_nTimeTrend.Alpha(Cluster->m_nCases,
                                               Cluster->m_pCumMeasure,
                                               gData.m_nTimeIntervals,
                                               GlobalTimeTrend.m_nBeta);

  Cluster->m_nRemTimeTrend.CalculateAndSet(Cluster->m_pRemCases,         // Outside Cluster
                                           Cluster->m_pRemMeasure,
                                           gData.m_nTimeIntervals,
                                           gParameters.GetTimeTrendConvergence());

  nGlobalAlphaOut = Cluster->m_nRemTimeTrend.Alpha(Cluster->m_nRemCases,
                                                   Cluster->m_pRemMeasure,
                                                   gData.m_nTimeIntervals,
                                                   GlobalTimeTrend.m_nBeta);
  #if DEBUGMODEL
  fprintf(m_pDebugModelFile, "Inside                Outside\n");
  fprintf(m_pDebugModelFile, "Cases    Msr          Cases    Msr\n");
  fprintf(m_pDebugModelFile, "==========================================\n");
  for (int i=0; i<m_pData->m_nTimeIntervals; i++)
    fprintf (m_pDebugModelFile, "%i       %.2f        %i       %.2f\n",
                                Cluster->m_pCumCases[i], Cluster->m_pCumMeasure[i],
                                Cluster->m_pRemCases[i], Cluster->m_pRemMeasure[i]);
  fprintf(m_pDebugModelFile, "------------------------------------------\n");
  fprintf (m_pDebugModelFile, "%i                   %i\n\n",
                              Cluster->m_nCases, Cluster->m_nRemCases);

//  fprintf(m_pDebugModelFile, "\nGlobal Time Trend: Alpha = %f, Beta = %f\n\n",
//          GlobalTimeTrend.m_nAlpha, GlobalTimeTrend.m_nBeta);
  #endif

  nLogLikelihood = (CalcSVTTLogLikelihood(Cluster->m_pCumCases,
                                          Cluster->m_pCumMeasure,
                                          Cluster->m_nCases,
                                          Cluster->m_nTimeTrend.m_nAlpha,
                                          Cluster->m_nTimeTrend.m_nBeta,
                                          Cluster->m_nTimeTrend.m_nStatus)
                    +
                    CalcSVTTLogLikelihood(Cluster->m_pRemCases,
                                          Cluster->m_pRemMeasure,
                                          Cluster->m_nRemCases,
                                          Cluster->m_nRemTimeTrend.m_nAlpha,
                                          Cluster->m_nRemTimeTrend.m_nBeta,
                                          Cluster->m_nRemTimeTrend.m_nStatus))
                    -
                   (CalcSVTTLogLikelihood(Cluster->m_pCumCases,
                                          Cluster->m_pCumMeasure,
                                          Cluster->m_nCases,
                                          nGlobalAlphaIn,
                                          GlobalTimeTrend.m_nBeta,
                                          Cluster->m_nTimeTrend.m_nStatus)
                    +
                    CalcSVTTLogLikelihood(Cluster->m_pRemCases,
                                          Cluster->m_pRemMeasure,
                                          Cluster->m_nRemCases,
                                          nGlobalAlphaOut,
                                          GlobalTimeTrend.m_nBeta,
                                          Cluster->m_nRemTimeTrend.m_nStatus));


  #if DEBUGMODEL
  fprintf(m_pDebugModelFile, "\nTotal LogLikelihood = %f\n\n\n", nLogLikelihood);
  #endif

  return nLogLikelihood;
}

bool CPoissonModel::CalculateMeasure() {
  bool bResult;

  try {
    bResult = AssignMeasure(gData.GetTInfo(), gData.m_pCases, gData.m_pTimes, gData.m_nTracts,
                            gData.m_nStartDate, gData.m_nEndDate, gData.m_pIntervalStartTimes,
                            gData.m_nTimeIntervals, gParameters.GetTimeIntervalUnitsType(),
                            gParameters.GetTimeIntervalLength(),
                            (gParameters.GetAnalysisType() == SPATIALVARTEMPTREND ? &gData.m_pMeasure_NC : &gData.m_pMeasure),
                            &gData.m_nTotalCases, &gData.m_nTotalPop, &gData.m_nTotalMeasure, &gPrintDirection);

    if (gParameters.GetAnalysisType() == SPATIALVARTEMPTREND) {
      //adjust non-cumulative measure
      AdjustMeasure(gData.m_pMeasure_NC);
      //create cumulative measure from non-cumulative measure
      gData.SetCumulativeMeasure();
      //either reset or set measure by time intervals with non-cumulative measure
      gData.SetMeasureByTimeIntervalArray(gData.m_pMeasure_NC);
    }
    else {
      AdjustMeasure(gData.m_pMeasure);
      if (gParameters.GetTimeTrendAdjustmentType() == STRATIFIED_RANDOMIZATION ||
          gParameters.GetTimeTrendAdjustmentType() == CALCULATED_LOGLINEAR_PERC)
        //need measure by time intervals for time stratified adjustment in simulations
        gData.SetMeasureByTimeIntervalArray(gData.m_pMeasure);
      gData.SetMeasureAsCumulative(gData.m_pMeasure);
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("CalculateMeasure()","CPoissonModel");
    throw;
  }
  return bResult;
}

double CPoissonModel::GetLogLikelihoodForTotal() const
{
  count_t   N = gData.m_nTotalCases;
  measure_t U = gData.m_nTotalMeasure;

  return N*log(N/U);
}

double CPoissonModel::GetPopulation(int m_iEllipseOffset, tract_t nCenter, tract_t nTracts,
                                    int nStartInterval, int nStopInterval) {
   tract_t T, t;
   int     c, n;
   double* pAlpha = 0;
   int     ncats = gData.GetPopulationCategories().GetNumPopulationCategories();
   int     nPops = (gData.GetTInfo())->tiGetNumPopDates();
   double  nPopulation = 0.0;

   try
      {
      (gData.GetTInfo())->tiCalculateAlpha(&pAlpha, gData.m_nStartDate, gData.m_nEndDate);

      for (T = 1; T <= nTracts; T++)
      {
         t = gData.GetNeighbor(m_iEllipseOffset, nCenter, T);
         for (c = 0; c < ncats; c++)
            gData.GetTInfo()->tiGetAlphaAdjustedPopulation(nPopulation, t, c, 0, nPops, pAlpha);
      }

      free(pAlpha); pAlpha = 0;
      }
   catch (ZdException & x)
      {
      free(pAlpha);
      x.AddCallpath("GetPopulation()", "CPoissonModel");
      throw;
      }
   return nPopulation;
}

/** Sets simulated data. */
void CPoissonModel::MakeData(int iSimulationNumber) {
#define PRINTSIMS 0 /* writes simulated data to a file, one simulation per line - irrespective of above */
/* One and only one of these can be set to one: */
#define H0 1        /* standard, generates simulated case data under H0 */
#define READSIMS 0  /* reads simulated data from a file */
#define HA 0        /* for power esimation, generates simulated case data under user specified HA */

  try {
    //reset seed to simulation number
    m_RandomNumberGenerator.SetSeed(iSimulationNumber + m_RandomNumberGenerator.GetDefaultSeed());
#if H0
  if (gParameters.GetTimeTrendAdjustmentType() == STRATIFIED_RANDOMIZATION)
    MakeDataTimeStratified();
  else
    MakeDataUnderNullHypothesis();
#endif
#if READSIMS
   ReadSimulationDataFromFile()
#endif
#if HA
   MakeData_AlternateHypothesis();
#endif
#if PRINTSIMS
   PrintSimulationDateToFile()
#endif
  }
  catch (ZdException &x) {
    x.AddCallpath("MakeData()", "CPoissonModel");
    throw;
  }
}

/** Generate case counts using time stratified algorithm. */
void CPoissonModel::MakeDataTimeStratified() {
  tract_t       tract;
  count_t       c, cumcases=0; // simulated cases so far
  measure_t     cummeasure=0;  // measure so far
  int           interval;      // current time interval

  interval = gData.m_nTimeIntervals - 1;
  for (tract=0; tract < gData.m_nTotalTractsAtStart; tract++) {
     if (gData.m_pCases_TotalByTimeInt[interval] - cumcases > 0)
       c = gBinomialGenerator.GetBinomialDistributedVariable(gData.m_pCases_TotalByTimeInt[interval] - cumcases,
                                                             gData.m_pMeasure[interval][tract]/(gData.m_pMeasure_TotalByTimeInt[interval] - cummeasure),
                                                             m_RandomNumberGenerator);
     else
       c = 0;
     cumcases += c;
     cummeasure += gData.m_pMeasure[interval][tract];
     gData.m_pSimCases[interval][tract] = c;
  }
  for (interval--; interval >= 0; --interval) { //For each other interval, from 2nd to last until the first:
     cumcases = 0;
     cummeasure = 0;
     for (tract=0; tract < gData.m_nTotalTractsAtStart; tract++) { //For each tract:
        if (gData.m_pCases_TotalByTimeInt[interval] - cumcases > 0)
          c = gBinomialGenerator.GetBinomialDistributedVariable(gData.m_pCases_TotalByTimeInt[interval] - cumcases,
                      (gData.m_pMeasure[interval][tract] - gData.m_pMeasure[interval + 1][tract])/(gData.m_pMeasure_TotalByTimeInt[interval] - cummeasure),
                      m_RandomNumberGenerator);
        else
          c = 0;
        cumcases += c;
        cummeasure += (gData.m_pMeasure[interval][tract] - gData.m_pMeasure[interval + 1][tract]);
        gData.m_pSimCases[interval][tract] = c + gData.m_pSimCases[interval + 1][tract];
     }
  }
}

/** Generate case counts under the null hypothesis (standard) */
void CPoissonModel::MakeDataUnderNullHypothesis() {
  tract_t       tract;
  count_t       c, d, cumcases=0; // simulated cases so far
  measure_t     cummeasure=0;  // measure so far
  int           interval;      // current time interval

  for (tract = 0; tract < gData.m_nTotalTractsAtStart; tract++) {
     if (gData.m_nTotalMeasure-cummeasure > 0)
       c = gBinomialGenerator.GetBinomialDistributedVariable(gData.m_nTotalCases - cumcases,
                              gData.m_pMeasure[0][tract] / (gData.m_nTotalMeasure-cummeasure), m_RandomNumberGenerator);
     else
       c = 0;
     gData.m_pSimCases[0][tract] = c;
     cumcases += c;
     cummeasure += gData.m_pMeasure[0][tract];
     for (interval=0;interval<gData.m_nTimeIntervals-1;interval++) {
        if (gData.m_pMeasure[interval][tract]>0)
          d = gBinomialGenerator.GetBinomialDistributedVariable(gData.m_pSimCases[interval][tract],
                                 1 - gData.m_pMeasure[interval+1][tract] / gData.m_pMeasure[interval][tract],
                                 m_RandomNumberGenerator);
        else
          d = 0;
        gData.m_pSimCases[interval+1][tract] = gData.m_pSimCases[interval][tract] - d;
     } // for interval
  } // for tract
}

/** Generates simulated cases under an alternative hypothesis model */
void CPoissonModel::MakeData_AlternateHypothesis() {
  measure_t     TotalMeasure, Measure[245];       // size should be equal to number of tracts
  float         RR1,    RR2;
  tract_t       tract;
  count_t       c, d, cumcases=0;                       // simulated cases so far
  measure_t     cummeasure=0;                             // measure so far
  int           interval;                            // current time interval

  RR1=2.20; RR2=1.0;
  TotalMeasure=gData.m_nTotalMeasure;
  for (tract = 0; tract < gData.m_nTotalTractsAtStart; tract++) {
    if (
    //tract==109 || tract==135|| tract==149 ||tract==143
      //  ||tract==105||tract==222||tract==152||tract==116
       // ||tract==100 ||tract==216 ||tract==125||tract==97
       // || tract==210 ||tract==123 ||tract==193 ||tract==142){

        tract==127 || tract==99) {
        // || tract==99 || tract==139 ) {
         //|| tract==139 || tract==137 || tract==95 || tract==87
         //|| tract==82 || tract==77  || tract==156 || tract==0
         //|| tract==126 || tract==86 || tract==93 || tract==88){

    // tract==237  || tract==236 || tract==106 || tract==112
    // || tract==234 || tract==117 || tract==113 || tract==118
    // || tract==238 || tract==240 || tract==231 || tract==129
    // || tract==121 || tract==153 || tract==123 || tract==242 ) {

      //tract==160  || tract==221
       //||tract==161 ||tract==191
       //|| tract==168 || tract==174|| tract==182|| tract==188
      //|| tract==162|| tract==190 || tract==223 || tract==185
      //|| tract==200 || tract==171|| tract==175|| tract==211) {

        Measure[tract]=gData.m_pMeasure[0][tract]*RR1;
        TotalMeasure=TotalMeasure+gData.m_pMeasure[0][tract]*(RR1-1);
        }
    else if (tract==160 || tract==221 || tract==162 || tract==223
         || tract==168 || tract==161 || tract==195 || tract==184
        || tract==188 || tract==190 || tract==174 || tract==201
           || tract==214 || tract==219 || tract==169 || tract==191) {

        Measure[tract]=gData.m_pMeasure[0][tract]*RR2;
        TotalMeasure=TotalMeasure+gData.m_pMeasure[0][tract]*(RR2-1);
        }
    else
        Measure[tract]=gData.m_pMeasure[0][tract];
//    printf("tract=%d, pop=%f\n",tract,Measure[tract]);
    }
  //printf("changes before here\n");

  for (tract = 0; tract < gData.m_nTotalTractsAtStart; tract++)
  {
    if (TotalMeasure-cummeasure > 0)
      c = gBinomialGenerator.GetBinomialDistributedVariable(gData.m_nTotalCases - cumcases,
                             Measure[tract] / (TotalMeasure-cummeasure), m_RandomNumberGenerator);
    else
      c = 0;
//    printf("t=%d, m=%f, c=%d, cm=%f, cc=%d\n",tract,Measure[tract],c,cummeasure,cumcases);

    gData.m_pSimCases[0][tract] = c;
    cumcases += c;
    cummeasure += Measure[tract];

    for(interval=0;interval<gData.m_nTimeIntervals-1;interval++)
    {
      if(gData.m_pMeasure[interval][tract]>0)
        d = gBinomialGenerator.GetBinomialDistributedVariable(gData.m_pSimCases[interval][tract],
                               1 - gData.m_pMeasure[interval+1][tract] / gData.m_pMeasure[interval][tract],
                               m_RandomNumberGenerator);
      else
        d = 0;

      gData.m_pSimCases[interval+1][tract] = gData.m_pSimCases[interval][tract] - d;
    } // for interval

  } // for tract
//    printf("totalmeasure=%f, totalcases=%d\n",cummeasure,cumcases);
}

/** Prints the simulated data to a file */
void CPoissonModel::PrintSimulationDateToFile() {
  FILE        * fpSIM;

  if ((fpSIM = fopen("simdata.txt", "a")) == NULL)
    SSGenerateException("Error: Could not open file to write simulated data\n","MakeData");

  for (tract_t tract=0; tract < gData.m_nTotalTractsAtStart; tract++)
      fprintf(fpSIM,"%d ",gData.m_pSimCases[0][tract]);
  fprintf(fpSIM,"\n");
  fclose(fpSIM);
}

bool CPoissonModel::ReadData() {
  try {
    if (! gData.ReadCoordinatesFile())
      return false;
    if (! gData.ReadPopulationFile())
      return false;
    if (! gData.GetTInfo()->tiCheckZeroPopulations(stderr))
      return false;
    if (! gData.ReadCaseFile())
      return false;
    gData.GetTInfo()->tiCheckCasesHavePopulations();
    if (gParameters.UseSpecialGrid() && !gData.ReadGridFile())
      return false;
  }
  catch (ZdException & x) {
    x.AddCallpath("ReadData()", "CPoissonModel");
    throw;
  }
  return true;
}

/** Reads number of simulated cases from a text file named simdata.txt,
    rather than generating them randomly. Currently only valid for purely spatial. */
void CPoissonModel::ReadSimulationDataFromFile() {
  static long int       fileposition=0;
  tract_t               tract;
  count_t               c, cumcases=0;
  measure_t             cummeasure=0;

  ifstream inSimFile("simdata.txt", ios::in);
  if(!inSimFile)
    SSGenerateException("Error: Could not open file to read simulated data.\n","MakeData()");

  inSimFile.seekg(fileposition);  // puts the file position pointer to the correct simulation
  for (tract=0; tract < gData.m_nTotalTractsAtStart; tract++) {
    inSimFile >> c;
    gData.m_pSimCases[0][tract] = c;
    cumcases += c;
    cummeasure += gData.m_pMeasure[0][tract];
  } // for tract
  fileposition = inSimFile.tellg();  // saves the current file location for the next call
}

