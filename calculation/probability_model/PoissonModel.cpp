#include "SaTScan.h"
#pragma hdrstop
#include "PoissonModel.h"

using std::ios;
using std::ifstream;

/** constructor */
CPoissonModel::CPoissonModel(CParameters& Parameters, CSaTScanData& Data, BasePrint& PrintDirection)
              :CModel(Parameters, Data, PrintDirection){               
  Init();
}

/** destructor */
CPoissonModel::~CPoissonModel() {
  try {
    DeallocateAlternateHypothesisStructures();
  }
  catch(...){}
}

/** Adjusts the expected number of cases for a time trend, using the data
    itself. For each adjustment interval, which has to be fewer than the
    number of time intervals, all measures are multiplied with a constant
    so that the total measure in all adjustment intervals are the same in
    proportion to its length in time.
    These operations are done on the raw measure matrix rather than the
    later on constructed cumulative measure matrix, while the cumulative
    case matrix is used.                                                */
void CPoissonModel::AdjustForNonParameteric(measure_t ** pNonCumulativeMeasure) {
  int                   i, j, jj, k, tract, AdjustIntervals;
  double                sumcases,summeasure;
  count_t            ** ppCases(gData.gpCasesHandler->GetArray());

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
        for (tract=0; tract < gData.m_nTracts; tract++)
        {
          sumcases   = sumcases + ppCases[j][tract];
          summeasure = summeasure + (pNonCumulativeMeasure)[j][tract];
        } /* for tract */
      else
        for (tract=0; tract < gData.m_nTracts; tract++)
        {
          sumcases   = sumcases + (ppCases[j][tract]-ppCases[j+1][tract]);
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

/** allocates alternative hypothesis data structures */
void CPoissonModel::AllocateAlternateHypothesisStructures() {
  int   i;

  try {
    gpRelativeRisks = new float[static_cast<long>(gData.m_nTotalTractsAtStart)];
    gpMeasure = new measure_t[static_cast<long>(gData.m_nTotalTractsAtStart)];
    gpAlternativeMeasure = (double**)Smalloc((gData.m_nTimeIntervals)*sizeof(measure_t*), &gPrintDirection);
    memset(gpAlternativeMeasure, 0, (gData.m_nTimeIntervals)*sizeof(measure_t*));
    for (i=0; i < gData.m_nTimeIntervals; i++)
       gpAlternativeMeasure[i] = (double*)Smalloc(static_cast<long>(gData.m_nTotalTractsAtStart)*sizeof(measure_t), &gPrintDirection);
  }
  catch (ZdException &x) {
    //free any allocated memory
    delete gpRelativeRisks; gpRelativeRisks=0;
    delete gpMeasure; gpMeasure=0;
    while (gpAlternativeMeasure && --i >= 0) {delete gpAlternativeMeasure[i]; gpAlternativeMeasure[i]=0;}
    delete gpAlternativeMeasure; gpAlternativeMeasure=0;
    x.AddCallpath("AllocateAlternativeMeasure","CPoissonModel");
    throw;
  }
}

/** Calculates and sets non-cumulative measure array. */
int CPoissonModel::AssignMeasure(measure_t ** ppMeasure) {
  int                                   i, interval,
                                        iNumCategories(gData.GetTInfo()->tiGetNumCategories()),
                                        nPops(gData.GetTInfo()->tiGetNumPopDates());
  double                              * pAlpha=0, * pRisk=0;
  tract_t                               tract, nTracts(gData.GetTInfo()->tiGetNumTracts());
  const char                          * tid;
  Julian                              * IntervalDates=0;

  try {
    TwoDimensionArrayHandler<measure_t>   Calcm_ArrayHandler(nPops, nTracts);
    /* Create & Use array of interval dates where last interval date = EndDate */
    IntervalDates = (unsigned long*)Smalloc((gData.m_nTimeIntervals+1)*sizeof(Julian), &gPrintDirection);
    memcpy(IntervalDates, gData.GetTimeIntervalStartTimes(), (gData.m_nTimeIntervals+1)*sizeof(Julian));
#ifdef DEBUGMEASURE
    if ((pMResult = fopen("MEASURE.TXT", "w")) == NULL)
      SSGenerateException("Error: Cannot open output file.\n","AssignMeasure()");
#endif
    gData.GetTInfo()->tiCalculateAlpha(&pAlpha, gData.m_nStartDate, gData.m_nEndDate);
#ifdef DEBUGMEASURE
    DisplayInitialData(gData.m_nStartDate, gData.m_nEndDate, IntervalDates, gData.m_nTimeIntervals, pAlpha, nPops);
#endif
    CalcRisk(gData.GetTInfo(), &pRisk, pAlpha, iNumCategories, nTracts, nPops, &gData.m_nTotalPop, &gData.m_nTotalCases, &gPrintDirection);
    Calcm(gData.GetTInfo(), Calcm_ArrayHandler.GetArray(), pRisk, iNumCategories, nTracts, nPops, &gPrintDirection);
    CalcMeasure(gData.GetTInfo(), ppMeasure, Calcm_ArrayHandler.GetArray(), IntervalDates, gData.m_nStartDate, gData.m_nEndDate,
                iNumCategories, nTracts, nPops, gData.m_nTimeIntervals, &gData.m_nTotalMeasure, &gPrintDirection);
    free(pAlpha);  pAlpha = 0;
    free(pRisk);   pRisk = 0;
/* Bug report: Check to see that Measure matrix has positive entries. */
#if 1
    for (tract=0; tract < gData.m_nTracts; ++tract) {
       for (interval=0; interval < gData.m_nTimeIntervals; ++interval) {
          if(ppMeasure[interval][tract] < 0) {
             char        sMessage[200];
             std::string sBuffer;
    	     tid = gData.GetTInfo()->tiGetTid(tract, sBuffer);
             sprintf(sMessage,"Error: Negative Measure (%8.4f) in function AssignMeasure(),\n\ttract %d, tractid %s, interval %d.\n",ppMeasure[interval][tract], tract, tid, interval);
    	     SSGenerateException(sMessage, "AssignMeasure()");
          } /* endif Measure */
       } /* endfor interval*/
    } /* endfor tract*/
#endif
#ifdef DEBUGMEASURE
   fprintf(pMResult, "Totals: \n\n");
   fprintf(pMResult, "   Cases      = %li\n   Measure    = %f\n   Population = %f\n", *pTotalCases, *pTotalMeasure, *pTotalPop);
   fclose(pMResult);
#endif
   gPrintDirection.SatScanPrintf("\n");
   free(IntervalDates);
  }
  catch (ZdException & x) {
    free(pAlpha);
    free(pRisk);
    free(IntervalDates);
    x.AddCallpath("AssignMeasure()", "CPoissonModel");
    throw;
  }
  return(1);
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
  bool                  bResult;

  try {
    if (gParameters.GetAnalysisType() == SPATIALVARTEMPTREND) {
      gData.gpMeasureNonCumulativeHandler = new TwoDimensionArrayHandler<measure_t>(gData.m_nTimeIntervals+1/*no sure why + 1*/, gData.m_nTracts);
      bResult = AssignMeasure(gData.gpMeasureNonCumulativeHandler->GetArray());
      //adjust non-cumulative measure
      AdjustMeasure(gData.gpMeasureNonCumulativeHandler->GetArray());
      //create cumulative measure from non-cumulative measure
      gData.SetCumulativeMeasure();
      //either reset or set measure by time intervals with non-cumulative measure
      gData.SetMeasureByTimeIntervalArray(gData.gpMeasureNonCumulativeHandler->GetArray());
    }
    else {
      gData.gpMeasureHandler = new TwoDimensionArrayHandler<measure_t>(gData.m_nTimeIntervals+1/*no sure why + 1*/, gData.m_nTracts);
      bResult = AssignMeasure(gData.gpMeasureHandler->GetArray());
      AdjustMeasure(gData.gpMeasureHandler->GetArray());
      if (gParameters.GetTimeTrendAdjustmentType() == STRATIFIED_RANDOMIZATION ||
          gParameters.GetTimeTrendAdjustmentType() == CALCULATED_LOGLINEAR_PERC)
        //need measure by time intervals for time stratified adjustment in simulations
        gData.SetMeasureByTimeIntervalArray(gData.gpMeasureHandler->GetArray());
      gData.SetMeasureAsCumulative(gData.gpMeasureHandler->GetArray());
    }
  }
  catch (ZdException &x) {
    delete gData.gpMeasureNonCumulativeHandler; gData.gpMeasureNonCumulativeHandler=0;
    delete gData.gpMeasureHandler; gData.gpMeasureHandler=0;
    x.AddCallpath("CalculateMeasure()","CPoissonModel");
    throw;
  }
  return bResult;
}

/** deallocates alternative hypothesis data structures */
void CPoissonModel::DeallocateAlternateHypothesisStructures() {
  try {
    delete gpRelativeRisks; gpRelativeRisks=0;
    delete gpMeasure; gpMeasure=0;
    if (gpAlternativeMeasure) {
      for (int i=0; i < gData.m_nTimeIntervals; i++)
         delete gpAlternativeMeasure[i];
      delete gpAlternativeMeasure;
    }  
  }
  catch (ZdException &x) {
    x.AddCallpath("DeallocateAlternativeMeasure()","CPoissonModel");
    throw;
  }
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
  try {
    //reset seed to simulation number
    m_RandomNumberGenerator.SetSeed(iSimulationNumber + m_RandomNumberGenerator.GetDefaultSeed());
    switch(gParameters.GetSimulationType()) {
      case STANDARD         : if (gParameters.GetTimeTrendAdjustmentType() == STRATIFIED_RANDOMIZATION)
                                MakeDataTimeStratified();
                              else
                                MakeDataUnderNullHypothesis();
                              break;
      case POWER_ESTIMATION : if (iSimulationNumber == 1)
                                AllocateAlternateHypothesisStructures();
                              MakeData_AlternateHypothesis(); break;
      case FILESOURCE       : ReadSimulationDataFromFile(); break;
      default : ZdGenerateException("Unknown simulation type '%d'.","MakeData()",gParameters.GetSimulationType());
    };

    if (gParameters.GetOutputSimulationData())
      PrintSimulationDateToFile(iSimulationNumber);
  }
  catch (ZdException &x) {
    x.AddCallpath("MakeData()", "CPoissonModel");
    throw;
  }
}

/** Generate case counts using time stratified algorithm. */
void CPoissonModel::MakeDataTimeStratified() {
  tract_t               tract;
  count_t               c, cumcases=0, ** ppSimCases(gData.GetSimCasesArray());
  measure_t             cummeasure=0, ** ppMeasure(gData.GetMeasureArray());
  int                   interval;      // current time interval

  interval = gData.m_nTimeIntervals - 1;
  for (tract=0; tract < gData.m_nTotalTractsAtStart; tract++) {
     if (gData.m_pCases_TotalByTimeInt[interval] - cumcases > 0)
       c = gBinomialGenerator.GetBinomialDistributedVariable(gData.m_pCases_TotalByTimeInt[interval] - cumcases,
                                                             ppMeasure[interval][tract]/(gData.m_pMeasure_TotalByTimeInt[interval] - cummeasure),
                                                             m_RandomNumberGenerator);
     else
       c = 0;
     cumcases += c;
     cummeasure += ppMeasure[interval][tract];
     ppSimCases[interval][tract] = c;
  }
  for (interval--; interval >= 0; --interval) { //For each other interval, from 2nd to last until the first:
     cumcases = 0;
     cummeasure = 0;
     for (tract=0; tract < gData.m_nTotalTractsAtStart; tract++) { //For each tract:
        if (gData.m_pCases_TotalByTimeInt[interval] - cumcases > 0)
          c = gBinomialGenerator.GetBinomialDistributedVariable(gData.m_pCases_TotalByTimeInt[interval] - cumcases,
                      (ppMeasure[interval][tract] - ppMeasure[interval + 1][tract])/(gData.m_pMeasure_TotalByTimeInt[interval] - cummeasure),
                      m_RandomNumberGenerator);
        else
          c = 0;
        cumcases += c;
        cummeasure += (ppMeasure[interval][tract] - ppMeasure[interval + 1][tract]);
        ppSimCases[interval][tract] = c + ppSimCases[interval + 1][tract];
     }
  }
}

/** Generate case counts under the null hypothesis (standard) */
void CPoissonModel::MakeDataUnderNullHypothesis() {
  tract_t               t;
  count_t               c, d, cumcases=0, ** ppSimCases(gData.GetSimCasesArray());
  measure_t             cummeasure=0, ** ppMeasure(gData.GetMeasureArray());
  int                   i;

  for (t=0; t < gData.m_nTotalTractsAtStart; ++t) {
     if (gData.m_nTotalMeasure-cummeasure > 0)
       c = gBinomialGenerator.GetBinomialDistributedVariable(gData.m_nTotalCases - cumcases,
                              ppMeasure[0][t] / (gData.m_nTotalMeasure-cummeasure), m_RandomNumberGenerator);
     else
       c = 0;
     ppSimCases[0][t] = c;
     cumcases += c;
     cummeasure += ppMeasure[0][t];
     for (i=0; i < gData.m_nTimeIntervals-1; ++i) {
        if (ppMeasure[i][t] > 0)
          d = gBinomialGenerator.GetBinomialDistributedVariable(ppSimCases[i][t],
                                                                1 - ppMeasure[i+1][t] / ppMeasure[i][t],
                                                                m_RandomNumberGenerator);
        else
          d = 0;
        ppSimCases[i+1][t] = ppSimCases[i][t] - d;
     }
  }
}

/** Generates simulated cases under an alternative hypothesis model.
    NOTE: Reading of the power estimation file data is not validated, potentially
          leading to bad program behavior.                                        */
void CPoissonModel::MakeData_AlternateHypothesis() {
  int                   i, j, iInterval;
  std::ifstream         RelativeRiskFile;
  std::string           sTractId;
  tract_t               t, tractIndex;
  measure_t             TotalMeasure=gData.m_nTotalMeasure, cummeasure=0, ** ppMeasure(gData.GetMeasureArray());
  count_t               c, d, cumcases=0, ** ppSimCases(gData.GetSimCasesArray());  

  //duplicate the  ppMeasure[][] into gpAlternativeMeasure[][], gpAlternativeMeasure[][] will be changed depending upon
  //the gpRelativeRisks[], and ppMeasure[][] remains the same as the expected measure
  for (t=0; t < gData.m_nTotalTractsAtStart; ++t)
     for (i=0; i < gData.m_nTimeIntervals; ++i)
        gpAlternativeMeasure[i][t] = ppMeasure[i][t];

  //initialize the gpRelativeRisks[] to be 1.0
  for (t=0; t < gData.m_nTotalTractsAtStart; ++t)
     gpRelativeRisks[t] = 1.0;

  //read in the RR's for those tracts with higher risks
  RelativeRiskFile.open(gParameters.GetPowerEstimationFilename().c_str());
  while (!RelativeRiskFile.eof()) {
       RelativeRiskFile >> sTractId;
       if ((tractIndex = gData.GetTInfo()->tiGetTractIndex(sTractId.c_str())) == -1)
         SSGenerateException("Unknown location identifier '%s', in power estimation file.",
                             "MakeData_AlternateHypothesis()", sTractId.c_str());
        RelativeRiskFile >> gpRelativeRisks[tractIndex];
  }
  RelativeRiskFile.close();

  //modify the measures
  for (t=0; t < gData.m_nTotalTractsAtStart; ++t) {
     gpMeasure[t] = ppMeasure[0][t];
     for (i=gData.m_nTimeIntervals-1; i >= 30/* ??? */ ; i--) {
        if (i == gData.m_nTimeIntervals-1) {//if the last interval, the cummulative measure is the measure itself
          gpMeasure[t] = gpMeasure[t] + ppMeasure[i][t] * (gpRelativeRisks[t]-1);
          TotalMeasure = TotalMeasure + ppMeasure[i][t] * (gpRelativeRisks[t]-1);
          for (j=0; j <= i; ++j)
             gpAlternativeMeasure[j][t] += ppMeasure[i][t] * (gpRelativeRisks[t]-1);
        }
        else {
          //if not the last interval, the measure belongs to the interval  is the difference between
          //the measure of this interval and the measure for next interval, measure[] and TotalMeasure
          //should change accordingly.
          gpMeasure[t] = gpMeasure[t] + (ppMeasure[i][t] - ppMeasure[i+1][t]) * (gpRelativeRisks[t]-1);
          TotalMeasure = TotalMeasure + (ppMeasure[i][t] - ppMeasure[i+1][t]) * (gpRelativeRisks[t]-1);
          for (j=0; j <= i; ++j)
             gpAlternativeMeasure[j][t] += (ppMeasure[i][t] - ppMeasure[i+1][t]) * (gpRelativeRisks[t]-1);
        }
     }
  }

   //start alternative simulations
  for (t=0; t < gData.m_nTotalTractsAtStart; ++t) {
    if (TotalMeasure-cummeasure > 0)
        c = gBinomialGenerator.GetBinomialDistributedVariable(gData.m_nTotalCases - cumcases,
                                                              gpMeasure[t] / (TotalMeasure-cummeasure),
                                                              m_RandomNumberGenerator);
    else
      c = 0;
    ppSimCases[0][t] = c;
    cumcases += c;
    cummeasure += gpMeasure[t];

    for (i=0; i < gData.m_nTimeIntervals-1; ++i) {
       if (gpAlternativeMeasure[i][t] > 0)
        d = gBinomialGenerator.GetBinomialDistributedVariable(ppSimCases[i][t],
              1 - gpAlternativeMeasure[i+1][t] / gpAlternativeMeasure[i][t],m_RandomNumberGenerator);
      else
        d = 0;

      ppSimCases[i+1][t] = ppSimCases[i][t] - d;
    }
  }
}

/** Prints the simulated data to a file. Format printed to file matches
    format expected for read as simulation data source. Truncates file
    when first opened for each analysis(i.e. first simulation). */
void CPoissonModel::PrintSimulationDateToFile(int iSimulationNumber) {
  std::ofstream         SimulationOutputFile;
  tract_t               tract;
  int                   interval;
  count_t            ** ppSimCases(gData.GetSimCasesArray());

  //NOTE: Since this is a hidden feature
  SimulationOutputFile.open(gParameters.GetSimulationDataOutputFilename().c_str(), (iSimulationNumber == 1 ? ios::trunc : ios::ate));
  if (!SimulationOutputFile)
    SSGenerateException("Error: Could not open file simulation output file '%s'.\n", "PrintSimulationDateToFile()",
                        gParameters.GetSimulationDataOutputFilename().c_str());

  if (gParameters.GetAnalysisType() == PROSPECTIVESPACETIME || gParameters.GetAnalysisType() == SPACETIME || gParameters.GetAnalysisType() == PURELYTEMPORAL) {
    for (tract=0; tract < gData.m_nTotalTractsAtStart; tract++) {
       for (interval=0; interval < gData.m_nTimeIntervals;interval++)
           SimulationOutputFile << ppSimCases[interval][tract] << " ";
       SimulationOutputFile << "\n";
    }
    SimulationOutputFile << "\n";
  }
  else if (gParameters.GetAnalysisType() == PURELYSPATIAL) {
    for (tract = 0; tract < gData.m_nTotalTractsAtStart; tract++)
       SimulationOutputFile << ppSimCases[0][tract] << " ";
    SimulationOutputFile << "\n";
  }
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
    if (!(gParameters.GetAnalysisType() == PURELYTEMPORAL || gParameters.GetAnalysisType() == PROSPECTIVEPURELYTEMPORAL))
      if (gParameters.UseMaxCirclePopulationFile() && !gData.ReadMaxCirclePopulationFile())
        return false;
    if (gParameters.UseSpecialGrid() && !gData.ReadGridFile())
      return false;
  }
  catch (ZdException & x) {
    x.AddCallpath("ReadData()", "CPoissonModel");
    throw;
  }
  return true;
}

/** Reads number of simulated cases from a text file rather than generating them randomly.
    NOTE: Data read from the file is not validated. This means that there is potential
          for the program to behave badly if:
          1) the data read from file does not match dimensions of gData.m_pSimCases
          2) the case counts read from file is inappropriate given real data -- probably access violations
          3) file does not actually contains numerical data
          Use of this feature should be discouraged except from someone who has
          detailed knowledge of how code works.                                                                 */
void CPoissonModel::ReadSimulationDataFromFile() {
  ifstream              SimulationDataFile;
  tract_t               t;
  int                   i;
  count_t               c;
  count_t            ** ppSimCases(gData.GetSimCasesArray());

  SimulationDataFile.open(gParameters.GetSimulationDataSourceFilename().c_str());
  if (!SimulationDataFile)
    SSGenerateException("Error: Could not open file '%s' to read simulated data.\n",
                        "ReadSimulationDataFromFile()", gParameters.GetSimulationDataSourceFilename().c_str());

  SimulationDataFile.seekg(glFilePosition);  // puts the file position pointer to the correct simulation

  if (gParameters.GetAnalysisType() == PROSPECTIVESPACETIME || gParameters.GetAnalysisType() == SPACETIME || gParameters.GetAnalysisType() == PURELYTEMPORAL) {
     for (t=0; t < gData.m_nTotalTractsAtStart; ++t) {
        for (i=0; i < gData.m_nTimeIntervals; ++i)
           SimulationDataFile >> ppSimCases[i][t];
     }
  }
  else if (gParameters.GetAnalysisType() == PURELYSPATIAL) {
     for (t=0; t < gData.m_nTotalTractsAtStart; ++t)
        SimulationDataFile >> ppSimCases[0][t];
  }

  glFilePosition = SimulationDataFile.tellg();  // saves the current file location for the next call
}
