#include "SaTScan.h"
#pragma hdrstop
#include "PoissonModel.h"

using std::ios;
using std::ifstream;

CPoissonModel::CPoissonModel(CParameters* pParameters, CSaTScanData* pData, BasePrint *pPrintDirection)
              :CModel(pParameters, pData, pPrintDirection){}

CPoissonModel::~CPoissonModel(){}

bool CPoissonModel::ReadData() {
  try {
    if (!m_pData->ReadCoordinatesFile())
      return false;
    if (!m_pData->ReadPopulationFile())
      return false;
    if (!(m_pData->GetTInfo())->tiCheckZeroPopulations(stderr))
      return false;
    if (! m_pData->ReadCaseFile())
      return false;
    m_pData->GetTInfo()->tiCheckCasesHavePopulations();
    if (m_pParameters->UseSpecialGrid() && !m_pData->ReadGridFile())
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
      bResult = AssignMeasure(m_pData->GetTInfo(),
                       m_pData->m_pCases,
                       m_pData->m_pTimes,
                       m_pData->m_nTracts,
                       m_pData->m_nStartDate,
                       m_pData->m_nEndDate,
                       m_pData->m_pIntervalStartTimes,
       		       false/*m_pParameters->m_bExactTimes*/,
                       m_pParameters->GetTimeTrendAdjustmentType(),
                       m_pParameters->GetTimeTrendAdjustmentPercentage(),
                       m_pData->m_nTimeIntervals,
                       m_pParameters->GetTimeIntervalUnitsType(),
                       m_pParameters->GetTimeIntervalLength(),
         	       &m_pData->m_pMeasure,
                       &m_pData->m_nTotalCases,
                       &m_pData->m_nTotalPop,
                       &m_pData->m_nTotalMeasure,
                       gpPrintDirection);

      //  m_pData->m_nTotalTractsAtStart   = m_pData->m_nTracts;
      //  m_pData->m_nTotalCasesAtStart    = m_pData->m_nTotalCases;
      //  m_pData->m_nTotalControlsAtStart = m_pData->m_nTotalControls;
      //  m_pData->m_nTotalMeasureAtStart  = m_pData->m_nTotalMeasure;
      //  m_pData->SetMaxCircleSize();
      }
   catch (SSException & x)
      {
      x.AddCallpath("CalculateMeasure()", "CPoissonModel");
      throw;
      }
   return bResult;
}

double CPoissonModel::GetLogLikelihoodForTotal() const
{
  count_t   N = m_pData->m_nTotalCases;
  measure_t U = m_pData->m_nTotalMeasure;

  return N*log(N/U);
}

double CPoissonModel::CalcLogLikelihood(count_t n, measure_t u)
{
   double    nLogLikelihood;
   count_t   N = m_pData->m_nTotalCases;
   measure_t U = m_pData->m_nTotalMeasure;

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
  for (tract = 0; tract < m_pData->m_nTotalTractsAtStart; tract++)
  {
    if (m_pData->m_nTotalMeasure-cummeasure > 0)
        c = gBinomialGenerator.GetBinomialDistributedVariable(m_pData->m_nTotalCases - cumcases,
                                                              m_pData->m_pMeasure[0][tract] / (m_pData->m_nTotalMeasure-cummeasure),
                                                              m_RandomNumberGenerator);
    else
      c = 0;

    m_pData->m_pSimCases[0][tract] = c;
    cumcases += c;
    cummeasure += m_pData->m_pMeasure[0][tract];

    for(interval=0;interval<m_pData->m_nTimeIntervals-1;interval++)
    {
      if(m_pData->m_pMeasure[interval][tract]>0)
        d = gBinomialGenerator.GetBinomialDistributedVariable(m_pData->m_pSimCases[interval][tract],
                                                              1 - m_pData->m_pMeasure[interval+1][tract] / m_pData->m_pMeasure[interval][tract],
                                                              m_RandomNumberGenerator);
      else
        d = 0;

      m_pData->m_pSimCases[interval+1][tract] = m_pData->m_pSimCases[interval][tract] - d;
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

  for (tract = 0; tract < m_pData->m_nTotalTractsAtStart; tract++) {
    inSimFile >> c;
//    printf("%d ",c);
    m_pData->m_pSimCases[0][tract] = c;
    cumcases += c;
    cummeasure += m_pData->m_pMeasure[0][tract];
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
  TotalMeasure=m_pData->m_nTotalMeasure;
  for (tract = 0; tract < m_pData->m_nTotalTractsAtStart; tract++) {
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

        Measure[tract]=m_pData->m_pMeasure[0][tract]*RR1;
        TotalMeasure=TotalMeasure+m_pData->m_pMeasure[0][tract]*(RR1-1);
        }
    else if (tract==160 || tract==221 || tract==162 || tract==223
         || tract==168 || tract==161 || tract==195 || tract==184
        || tract==188 || tract==190 || tract==174 || tract==201
           || tract==214 || tract==219 || tract==169 || tract==191) {

        Measure[tract]=m_pData->m_pMeasure[0][tract]*RR2;
        TotalMeasure=TotalMeasure+m_pData->m_pMeasure[0][tract]*(RR2-1);
        }
    else
        Measure[tract]=m_pData->m_pMeasure[0][tract];
//    printf("tract=%d, pop=%f\n",tract,Measure[tract]);
    }
  //printf("changes before here\n");

  for (tract = 0; tract < m_pData->m_nTotalTractsAtStart; tract++)
  {
    if (TotalMeasure-cummeasure > 0)
      c = Binomial(m_pData->m_nTotalCases - cumcases,
          Measure[tract] / (TotalMeasure-cummeasure));
    else
      c = 0;
//    printf("t=%d, m=%f, c=%d, cm=%f, cc=%d\n",tract,Measure[tract],c,cummeasure,cumcases);

    m_pData->m_pSimCases[0][tract] = c;
    cumcases += c;
    cummeasure += Measure[tract];

    for(interval=0;interval<m_pData->m_nTimeIntervals-1;interval++)
    {
      if(m_pData->m_pMeasure[interval][tract]>0)
        d = Binomial(m_pData->m_pSimCases[interval][tract],
            1 - m_pData->m_pMeasure[interval+1][tract] / m_pData->m_pMeasure[interval][tract]);
      else
        d = 0;

      m_pData->m_pSimCases[interval+1][tract] = m_pData->m_pSimCases[interval][tract] - d;
    } // for interval

  } // for tract
//    printf("totalmeasure=%f, totalcases=%d\n",cummeasure,cumcases);

#endif  // HA

     // Prints the simulated data to a file
#if PRINTSIMS
     FILE* fpSIM;
     if ((fpSIM = fopen("simdata.txt", "a")) == NULL) {
        //printf(" Error: Could not open file to write simulated data\n");
        //FatalError(0, gpPrintDirection);
        SSGenerateException(" Error: Could not open file to write simulated data\n","MakeData");
        }
     for (tract = 0; tract < m_pData->m_nTotalTractsAtStart; tract++)
        fprintf(fpSIM,"%d ",m_pData->m_pSimCases[0][tract]);
     fprintf(fpSIM,"\n");
     fclose(fpSIM);
#endif // PRINTSIMS
      }
   catch (SSException & x)
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
   int     ncats = m_pData->GetPopulationCategories().GetNumPopulationCategories();
   int     nPops = (m_pData->GetTInfo())->tiGetNumPopDates();
   double  nPopulation = 0.0;

   try
      {
      (m_pData->GetTInfo())->tiCalculateAlpha(&pAlpha, m_pData->m_nStartDate, m_pData->m_nEndDate);
   
      for (T = 1; T <= nTracts; T++)
      {
         t = m_pData->GetNeighbor(m_iEllipseOffset, nCenter, T);
         for (c = 0; c < ncats; c++)
            m_pData->GetTInfo()->tiGetAlphaAdjustedPopulation(nPopulation, t, c, 0, nPops, pAlpha);
      }
   
      free(pAlpha); pAlpha = 0;
      }
   catch (SSException & x)
      {
      free(pAlpha);
      x.AddCallpath("GetPopulation()", "CPoissonModel");
      throw;
      }
   return nPopulation;
}


