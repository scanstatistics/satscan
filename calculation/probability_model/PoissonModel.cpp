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

bool CPoissonModel::CalculateMeasure()
{
   bool bResult;

   try
      {
      bResult = AssignMeasure(gData.GetTInfo(),
                       gData.m_pCases,
                       gData.m_pTimes,
                       gData.m_nTracts,
                       gData.m_nStartDate,
                       gData.m_nEndDate,
                       gData.m_pIntervalStartTimes,
       		       false/*m_pParameters->m_bExactTimes*/,
                       gParameters.GetTimeTrendAdjustmentType(),
                       gParameters.GetTimeTrendAdjustmentPercentage(),
                       gData.m_nTimeIntervals,
                       gParameters.GetTimeIntervalUnitsType(),
                       gParameters.GetTimeIntervalLength(),
         	       &gData.m_pMeasure,
                       &gData.m_nTotalCases,
                       &gData.m_nTotalPop,
                       &gData.m_nTotalMeasure,
                       &gPrintDirection);

      //  gData.m_nTotalTractsAtStart   = gData.m_nTracts;
      //  gData.m_nTotalCasesAtStart    = gData.m_nTotalCases;
      //  gData.m_nTotalControlsAtStart = gData.m_nTotalControls;
      //  gData.m_nTotalMeasureAtStart  = gData.m_nTotalMeasure;
      //  gData.SetMaxCircleSize();
      }
   catch (ZdException & x)
      {
      x.AddCallpath("CalculateMeasure()", "CPoissonModel");
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

void CPoissonModel::MakeData(int iSimulationNumber)
{
   count_t   cumcases = 0;                       // simulated cases so far
   measure_t cummeasure = 0;                             // measure so far
   tract_t   tract;                                       // current tract
   int       interval;                            // current time interval
   count_t   c;                                          // cases in tract
   count_t   d;                                       // cases in interval

   try
      {
      //reset seed to simulation number
      m_RandomNumberGenerator.SetSeed(iSimulationNumber + m_RandomNumberGenerator.GetDefaultSeed());
      
// One and only one of these can be set to one:
#define H0 1            // standard, generates simulated case data under H0
#define READSIMS 0      // reads simulated data from a file
#define HA 0            // for power esimation, generates simulated case data under user specified HA

// Can be zero or one irrespectively of the choice above:
#define PRINTSIMS 0     // writes simulated data to a file, one simulation per line

#if H0

  // Generate case counts under the null hypothesis (standard)
  for (tract = 0; tract < gData.m_nTotalTractsAtStart; tract++)
  {
    if (gData.m_nTotalMeasure-cummeasure > 0)
        c = gBinomialGenerator.GetBinomialDistributedVariable(gData.m_nTotalCases - cumcases,
                                                              gData.m_pMeasure[0][tract] / (gData.m_nTotalMeasure-cummeasure),
                                                              m_RandomNumberGenerator);
    else
      c = 0;

    gData.m_pSimCases[0][tract] = c;
    cumcases += c;
    cummeasure += gData.m_pMeasure[0][tract];

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

#endif // H0

// Reads number of simulated cases from a text file named simdata.txt,
// rather than generating them randomly. Currently only valid for purely spatial.
#if READSIMS
  static long int  fileposition=0; // keeps track current location of input file

  ifstream inSimFile("simdata.txt", ios::in);
  if(!inSimFile) {
    //printf(" Error: Could not open file to read simulated data\n");
    //FatalError(0, gpPrintDirection);
    SSGenerateException(" Error: Could not open file to read simulated data\n","MakeData");
    }
  inSimFile.seekg(fileposition);  // puts the file position pointer to the correct simulation

  for (tract = 0; tract < gData.m_nTotalTractsAtStart; tract++) {
    inSimFile >> c;
//    printf("%d ",c);
    gData.m_pSimCases[0][tract] = c;
    cumcases += c;
    cummeasure += gData.m_pMeasure[0][tract];
    } // for tract

  fileposition = inSimFile.tellg();  // saves the current file location for the next call
//  printf("\n");
#endif // READSIMS

// Generates simulated cases under an alternative hypothesis model
#if HA
  measure_t TotalMeasure;
  measure_t Measure[245];       // size should be equal to number of tracts
  float     RR1,RR2;

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
      c = Binomial(gData.m_nTotalCases - cumcases,
          Measure[tract] / (TotalMeasure-cummeasure));
    else
      c = 0;
//    printf("t=%d, m=%f, c=%d, cm=%f, cc=%d\n",tract,Measure[tract],c,cummeasure,cumcases);

    gData.m_pSimCases[0][tract] = c;
    cumcases += c;
    cummeasure += Measure[tract];

    for(interval=0;interval<gData.m_nTimeIntervals-1;interval++)
    {
      if(gData.m_pMeasure[interval][tract]>0)
        d = Binomial(gData.m_pSimCases[interval][tract],
            1 - gData.m_pMeasure[interval+1][tract] / gData.m_pMeasure[interval][tract]);
      else
        d = 0;

      gData.m_pSimCases[interval+1][tract] = gData.m_pSimCases[interval][tract] - d;
    } // for interval

  } // for tract
//    printf("totalmeasure=%f, totalcases=%d\n",cummeasure,cumcases);

#endif  // HA

     // Prints the simulated data to a file
#if PRINTSIMS
     FILE* fpSIM;
     if ((fpSIM = fopen("simdata.txt", "a")) == NULL)
        SSGenerateException(" Error: Could not open file to write simulated data\n","MakeData");
     for (tract = 0; tract < gData.m_nTotalTractsAtStart; tract++)
        fprintf(fpSIM,"%d ",gData.m_pSimCases[0][tract]);
     fprintf(fpSIM,"\n");
     fclose(fpSIM);
#endif // PRINTSIMS
      }
   catch (ZdException & x)
      {
      x.AddCallpath("MakeData()", "CPoissonModel");
      throw;
      }

}

/* Not yet working - KR980325
void MakePurelyTemporalData(measure_t* pMeasure,
                            int        nTimeIntervals,
                            count_t    nTotalCases,
                            measure_t  nTotalMeasure,
                            count_t*   pSimCases)
{
  count_t   nCases   = 0;
  measure_t nMeasure = 0;
  count_t   c;

  for (int i=0; i<nTimeIntervals-1; i++)
  {
    if (nTotalMeasure-nMeasure > 0)
      c = Binomial(nTotalCases - nCases,
                   pMeasure[i] / (nTotalMeasure-nMeasure));
    else
      c = 0;

    pSimCases[i] = c;
    nCases      += c;
    nMeasure    += pMeasure[i];
  }

}
*/

double CPoissonModel::GetPopulation(int m_iEllipseOffset, tract_t nCenter, tract_t nTracts,
                                    int nStartInterval, int nStopInterval)
{
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


