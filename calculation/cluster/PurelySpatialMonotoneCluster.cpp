//*****************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//*****************************************************************************
#include "PurelySpatialMonotoneCluster.h"
#include "stsAreaSpecificData.h"

CPSMonotoneCluster::CPSMonotoneCluster(const AbstractClusterDataFactory * pClusterFactory,
                                       const AbtractDataStreamGateway & DataGateway,
                                       int iRate)
                   :CPurelySpatialCluster(pClusterFactory, DataGateway, iRate) {
  m_nMaxCircles        = 0;
  m_pCasesList         = NULL;
  m_pMeasureList       = NULL;
  m_pFirstNeighborList = NULL;
  m_pLastNeighborList  = NULL;

  Initialize(0);
}
CPSMonotoneCluster::CPSMonotoneCluster(const AbstractClusterDataFactory * pClusterFactory,
                                       const DataStreamInterface & Interface,
                                       int iRate)
                   :CPurelySpatialCluster(pClusterFactory, Interface, iRate) {
  m_nMaxCircles        = 0;
  m_pCasesList         = NULL;
  m_pMeasureList       = NULL;
  m_pFirstNeighborList = NULL;
  m_pLastNeighborList  = NULL;

  Initialize(0);
}
CPSMonotoneCluster::CPSMonotoneCluster(const CPSMonotoneCluster& rhs) : CPurelySpatialCluster(rhs){
  m_nMaxCircles        = 0;
  m_pCasesList         = NULL;
  m_pMeasureList       = NULL;
  m_pFirstNeighborList = NULL;
  m_pLastNeighborList  = NULL;

  *this = rhs;
}

CPSMonotoneCluster::~CPSMonotoneCluster()
{
  if (m_pCasesList != NULL)
    free(m_pCasesList);
  if (m_pMeasureList != NULL)
    free(m_pMeasureList);
  if (m_pFirstNeighborList != NULL)
    free(m_pFirstNeighborList);
  if (m_pLastNeighborList != NULL)
    free(m_pLastNeighborList);
}

/** returns newly cloned CPSMonotoneCluster */
CPSMonotoneCluster * CPSMonotoneCluster::Clone() const {
  //Note: Replace this code with copy constructor...
  CPSMonotoneCluster * pClone = new CPSMonotoneCluster(*this);
  return pClone;
}

AbstractClusterData * CPSMonotoneCluster::GetClusterData() {
 ZdGenerateException("GetClusterData() not implemented.","CPSMonotoneCluster");
 return 0;
}

/** initialize cluster data */
void CPSMonotoneCluster::Initialize(tract_t nCenter=0) {
  CCluster::Initialize(nCenter);
  m_bRatioSet = false;
  m_nLastInterval = 1;
  for (int i=0; i<m_nMaxCircles; i++) {
     m_pCasesList[i]         = 0;
     m_pMeasureList[i]       = 0;
     m_pFirstNeighborList[i] = 0;
     m_pLastNeighborList[i]  = 0;
  }
}

void CPSMonotoneCluster::AllocateForMaxCircles(tract_t nCircles)
{
   try
      {
      m_nMaxCircles        = nCircles;
      m_pCasesList         = (count_t*)  Smalloc(m_nMaxCircles * sizeof(count_t));
      m_pMeasureList       = (measure_t*)Smalloc(m_nMaxCircles * sizeof(measure_t));
      m_pFirstNeighborList = (tract_t*)  Smalloc(m_nMaxCircles * sizeof(tract_t));
      m_pLastNeighborList  = (tract_t*)  Smalloc(m_nMaxCircles * sizeof(tract_t));
      }
   catch (ZdException & x)
      {
      x.AddCallpath("AllocateForMaxCircles()", "CPSMonotoneCluster");
      throw;
      }
}

CPSMonotoneCluster& CPSMonotoneCluster::operator =(const CPSMonotoneCluster& cluster)
{
   try
      {
      m_Center         = cluster.m_Center;
      m_nCases         = cluster.m_nCases ;
      m_nMeasure       = cluster.m_nMeasure;
      m_nTracts        = cluster.m_nTracts;
      m_nRatio         = cluster.m_nRatio;
      m_nLogLikelihood = cluster.m_nLogLikelihood;
      m_nRank          = cluster.m_nRank;
      m_nFirstInterval = cluster.m_nFirstInterval;
      m_nLastInterval  = cluster.m_nLastInterval;
      m_nSteps       = cluster.m_nSteps;
      m_nMaxCircles    = cluster.m_nSteps;
    
      if (m_pCasesList != NULL)
        free(m_pCasesList);
      if (m_pMeasureList != NULL)
        free(m_pMeasureList);
      if (m_pFirstNeighborList != NULL)
        free(m_pFirstNeighborList);
      if (m_pLastNeighborList != NULL)
        free(m_pLastNeighborList);
    
      m_pCasesList         = (count_t*)  Smalloc(m_nSteps * sizeof(count_t));
      m_pMeasureList       = (measure_t*)Smalloc(m_nSteps * sizeof(measure_t));
      m_pFirstNeighborList = (tract_t*)  Smalloc(m_nSteps * sizeof(tract_t));
      m_pLastNeighborList  = (tract_t*)  Smalloc(m_nSteps * sizeof(tract_t));
    
      for (int i=0; i<m_nMaxCircles; i++)
      {
        m_pCasesList[i]         = cluster.m_pCasesList[i];
        m_pMeasureList[i]       = cluster.m_pMeasureList[i];
        m_pFirstNeighborList[i] = cluster.m_pFirstNeighborList[i];
        m_pLastNeighborList[i]  = cluster.m_pLastNeighborList[i];
      }
    
      m_bRatioSet      = cluster.m_bRatioSet;
      }
   catch (ZdException & x)
      {
      x.AddCallpath("Operator =()", "CPSMonotoneCluster");
      throw;
      }
  return *this;
}

void CPSMonotoneCluster::AddNeighbor(int iEllipse, const CSaTScanData& Data, count_t** pCases, tract_t n) {
  tract_t       nNeighbor = Data.GetNeighbor(0, m_Center, n);
  measure_t   * pMeasure(Data.GetDataStreamHandler().GetStream(0/*for now*/).GetMeasureArray()[0]);

  m_nSteps++;

  m_nCases   += pCases[0][nNeighbor];
  m_nMeasure += pMeasure[nNeighbor];

  m_pCasesList[m_nSteps-1]         = pCases[0][nNeighbor];
  m_pMeasureList[m_nSteps-1]       = pMeasure[nNeighbor];
  m_pFirstNeighborList[m_nSteps-1] = n;
  m_pLastNeighborList[m_nSteps-1]  = n;
}

void CPSMonotoneCluster::CheckCircle(tract_t n)
{
  if (n != 0)
    if (!m_pfRateOfInterest(m_pCasesList[n-1],m_pMeasureList[n-1], m_pCasesList[n],  m_pMeasureList[n]))
      {
      ConcatLastCircles();
      CheckCircle(n-1);
      }
}

void CPSMonotoneCluster::ConcatLastCircles()
{
  m_nSteps--;
  m_pCasesList[m_nSteps-1]        += m_pCasesList[m_nSteps];
  m_pMeasureList[m_nSteps-1]      += m_pMeasureList[m_nSteps];
  m_pLastNeighborList[m_nSteps-1]  = m_pLastNeighborList[m_nSteps];

  m_pCasesList[m_nSteps]         = 0;
  m_pMeasureList[m_nSteps]       = 0;
  m_pFirstNeighborList[m_nSteps] = 0;
  m_pLastNeighborList[m_nSteps]  = 0;
}

void CPSMonotoneCluster::RemoveRemainder()
{
  m_nSteps--;
  m_pCasesList[m_nSteps]         = 0;
  m_pMeasureList[m_nSteps]       = 0;
  m_pFirstNeighborList[m_nSteps] = 0;
  m_pLastNeighborList[m_nSteps]  = 0;

  if (m_nSteps==0)
    Initialize(m_Center);
}

void CPSMonotoneCluster::AddRemainder(count_t nTotalCases, measure_t nTotalMeasure)
{
  m_nSteps++;

  m_pCasesList[m_nSteps-1]   = nTotalCases - m_nCases;
  m_pMeasureList[m_nSteps-1] = nTotalMeasure - m_nMeasure;
}

void CPSMonotoneCluster::SetCasesAndMeasures()
{
  m_nCases   = 0;
  m_nMeasure = 0;

  for (int i=0; i<m_nSteps; i++)
     {
     m_nCases   += m_pCasesList[i];
     m_nMeasure += m_pMeasureList[i];
     }
}

double CPSMonotoneCluster::SetRatio(double nLikelihoodForTotal)
{
  m_bRatioSet = true;
  m_nRatio    = m_nLogLikelihood - nLikelihoodForTotal;

  return m_nRatio;
}

double CPSMonotoneCluster::SetLogLikelihood()
{
  m_nLogLikelihood = 0;

  for (int i=0; i<m_nSteps; i++)
  {
    if (m_pCasesList[i] != 0)
      m_nLogLikelihood += m_pCasesList[i]*log(m_pCasesList[i]/m_pMeasureList[i]);
  }

  return m_nLogLikelihood;
}

double CPSMonotoneCluster::GetRatio() const
{
  if (m_bRatioSet)
    return m_nRatio;
  else
    return -1;
}

double CPSMonotoneCluster::GetLogLikelihood() const
{
  return m_nLogLikelihood;
}

void CPSMonotoneCluster::DefineTopCluster(const CSaTScanData& Data, AbstractLikelihoodCalculator & Calculator, count_t** pCases) {
  tract_t            ** ppNeighborCount(Data.GetNeighborCountArray());
  CModel              & ProbModel(Data.GetProbabilityModel());  

   try {
      //  int i = 1;
      //  AddNeighbor(Data, pCases, i);
      //  while (i<=Data.m_NeighborCounts[m_Center] &&
      //         m_nMeasure <= Data.m_nMaxCircleSize)
      //for (int k = 0; k <= m_pParameters->GetNumTotalEllipses(); k++)   //circle is 0 offset... (always there)
      //  {
      for (int i=1; i<= ppNeighborCount[0][m_Center]; i++)
        {
        AddNeighbor(0, Data, pCases, i);
        CheckCircle(GetLastCircleIndex());
        //    i++;
        //    if (i<=Data.m_NeighborCounts[m_Center])
        //      AddNeighbor(Data, pCases, i);
        }
      // }
    
      if (Data.GetTotalCases() != m_nCases)
      {
        AddRemainder(Data.GetTotalCases(), Data.GetTotalMeasure());
        CheckCircle(GetLastCircleIndex());
      }
    
      m_nLogLikelihood = Calculator.CalcMonotoneLogLikelihood(*this);
    
      SetRatio(Calculator.GetLogLikelihoodForTotal());
    
      if (Data.GetTotalCases() != m_nCases)
        RemoveRemainder();
    
      // Recalc Total Cases, Measure, and Tracts to  account for
      // outer tracts absorbed into remainder
      SetCasesAndMeasures();
      SetTotalTracts();
      }
   catch (ZdException & x)
      {
      x.AddCallpath("DefineTopCluster()", "CPSMonotoneCluster");
      throw;
      }
}

void CPSMonotoneCluster::SetTotalTracts()
{
  m_nTracts = 0;
  for (int i=0; i<m_nSteps; i++)
    m_nTracts += m_pLastNeighborList[i]-m_pFirstNeighborList[i]+1;
}

void CPSMonotoneCluster::DisplayCensusTracts(FILE* fp, const CSaTScanData& Data,
                                             measure_t nMinMeasure, const AsciiPrintFormat& PrintFormat) const {
  int           i, j;
  ZdString      sBuffer;

  try {
    PrintFormat.PrintSectionLabel(fp, "Location IDs included", false, false);  
    for (i=0; i < m_nSteps; ++i) {
       fprintf(fp, "\n");
       sBuffer.printf("  Step %i",i + 1);
       PrintFormat.PrintSectionLabel(fp, sBuffer.GetCString(), false, true);
       DisplayCensusTractsInStep(fp, Data, m_pFirstNeighborList[i], m_pLastNeighborList[i], nMinMeasure, PrintFormat);
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("DisplayCensusTracts()","CPSMonotoneCluster");
    throw;
  }
}

void CPSMonotoneCluster::DisplayRelativeRisk(FILE* fp, const CSaTScanData& DataHub, const AsciiPrintFormat& PrintFormat) const {
  ZdString      sBuffer, sWork;
  int           i;

  try {
    CCluster::DisplayRelativeRisk(fp, DataHub, PrintFormat);
    if (m_nSteps == 1)
      return;
    PrintFormat.PrintSectionLabel(fp, "Relative risk by step", false, true);
    sBuffer.printf("%.3f", GetRelativeRisk(0, DataHub.GetMeasureAdjustment()));
    for (i=1; i < m_nSteps; ++i) {
       sWork.printf(", %.3f", GetRelativeRisk(i, DataHub.GetMeasureAdjustment()));
       sBuffer << sWork;
    }
    PrintFormat.PrintAlignedMarginsDataString(fp, sBuffer);
  }
  catch (ZdException &x) {
    x.AddCallpath("DisplayRelativeRisk()","CPSMonotoneCluster");
    throw;
  }
}

double CPSMonotoneCluster::GetRelativeRisk(tract_t nStep, double nMeasureAdjustment) const {
  return ((double)(m_pCasesList[nStep]))/(m_pMeasureList[nStep] * nMeasureAdjustment);
}

void CPSMonotoneCluster::DisplaySteps(FILE* fp, const AsciiPrintFormat& PrintFormat) const {
  PrintFormat.PrintSectionLabel(fp, "Steps in risk function", false, true);
  fprintf(fp, "%i\n", m_nSteps);
}

void CPSMonotoneCluster::DisplayCoordinates(FILE* fp, const CSaTScanData& Data, const AsciiPrintFormat& PrintFormat) const {
  double      * pCoords=0, * pCoords2=0;
  float         nRadius;
  int           i;
  ZdString      sBuffer, sWork;

  try {
    Data.GetGInfo()->giGetCoords(m_Center, &pCoords);
    PrintFormat.PrintSectionLabel(fp, "Coordinates", false, true);
    for (i=0; i < Data.GetParameters().GetDimensionsOfData() - 1; ++i) {
       sWork.printf("%s%g,", (i == 0 ? "(" : "" ), pCoords[i]);
       sBuffer << sWork;
    }
    sWork.printf("%g)", pCoords[Data.GetParameters().GetDimensionsOfData() - 1]);
    sBuffer << sWork;
    PrintFormat.PrintAlignedMarginsDataString(fp, sBuffer);
    PrintFormat.PrintSectionLabel(fp, "Radius for each step", false, true);
    sBuffer << ZdString::reset;
    for (i=0; i < m_nSteps; ++i) {
       Data.GetTInfo()->tiGetCoords(Data.GetNeighbor(0, m_Center, m_pLastNeighborList[i]), &pCoords2);
       nRadius = (float)sqrt(Data.GetTInfo()->tiGetDistanceSq(pCoords, pCoords2));
       free(pCoords2);
       sWork.printf("%s%4.2f", (i > 0 ? ", " : ""), nRadius);
       sBuffer << sWork;
    }
    PrintFormat.PrintAlignedMarginsDataString(fp, sBuffer);
    free(pCoords);
  }
  catch (ZdException &x) {
    x.AddCallpath("DisplayCoordinates()","CPSMonotoneCluster");
    free(pCoords);
    free(pCoords2);
    throw;
  }
}

void CPSMonotoneCluster::DisplayLatLongCoords(FILE* fp, const CSaTScanData& Data, const AsciiPrintFormat& PrintFormat) const {
  double      * pCoords=0, * pCoords2=0;
  int           i;
  float         Latitude, Longitude, nRadius;
  char          cNorthSouth, cEastWest;
  ZdString      sBuffer, sWork;

  try {
    Data.GetGInfo()->giGetCoords(m_Center, &pCoords);
    Data.GetTInfo()->tiGetCoords(Data.GetNeighbor(0, m_Center, m_nTracts), &pCoords2);
    nRadius = (float)sqrt((Data.GetTInfo())->tiGetDistanceSq(pCoords, pCoords2));
    ConvertToLatLong(&Latitude, &Longitude, pCoords);
    Latitude >= 0 ? cNorthSouth = 'N' : cNorthSouth = 'S';
    Longitude >= 0 ? cEastWest = 'W' : cEastWest = 'E';
    PrintFormat.PrintSectionLabel(fp, "Coordinates", false, true);
    fprintf(fp, "(%.6f %c, %.6f %c)\n", fabs(Latitude), cNorthSouth, fabs(Longitude), cEastWest, nRadius);
    PrintFormat.PrintSectionLabel(fp, "Radius for each step", false, true);
    for (i=0; i < m_nSteps; ++i) {
      Data.GetTInfo()->tiGetCoords(Data.GetNeighbor(0, m_Center, m_pLastNeighborList[i]), &pCoords2);
      nRadius = (float)sqrt(Data.GetTInfo()->tiGetDistanceSq(pCoords, pCoords2));
      free(pCoords2);
      sWork.printf("%s%5.2f km", (i == 0 ? "(" : "" ), nRadius);
      sBuffer << sWork;
    }
    PrintFormat.PrintAlignedMarginsDataString(fp, sBuffer);
    free(pCoords);
  }
  catch (ZdException &x) {
    x.AddCallpath("DisplayLatLongCoords()", "CPSMonotoneCluster");
    free(pCoords);
    free(pCoords2);
    throw;
  }
}

void CPSMonotoneCluster::Write(stsAreaSpecificData& AreaData, const CSaTScanData& Data,
                     unsigned int iClusterNumber, unsigned int iNumSimsCompleted) const {
  tract_t       tTract;
  int           i;

  try {         
    for (i=m_pFirstNeighborList[i]; i <= m_pLastNeighborList[i]; i++) {
       tTract = Data.GetNeighbor(m_iEllipseOffset, m_Center, i);
       AreaData.RecordClusterData(*this, Data, iClusterNumber, tTract, iNumSimsCompleted);
    }   
  }
  catch (ZdException &x) {
    x.AddCallpath("Write(stsAreaSpecificData*)","CPSMonotoneCluster");
    throw;
  }
}



