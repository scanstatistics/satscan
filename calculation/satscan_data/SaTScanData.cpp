#include "SaTScan.h"
#pragma hdrstop
#include "SaTScanData.h"
#include "PoissonModel.h"
#include "BernoulliModel.h"
#include "SpaceTimePermutationModel.h"

CSaTScanData::CSaTScanData(CParameters* pParameters, BasePrint *pPrintDirection) {
   long lCurrentEllipse = 0;

   try {
      gpPrintDirection = pPrintDirection;
      m_pParameters = pParameters;
      m_nNumEllipsoids = pParameters->m_nNumEllipses;

      Init();

      gpCats = new Cats(pPrintDirection);            // DTG
      gpTInfo = new TInfo(gpCats, pPrintDirection);  // DTG
      gpGInfo = new GInfo(pPrintDirection);          // DTG

      switch (m_pParameters->m_nModel) {
        case POISSON              : m_pModel = new CPoissonModel(pParameters, this, pPrintDirection);   break;
        case BERNOULLI            : m_pModel = new CBernoulliModel(pParameters, this, pPrintDirection); break;
        case SPACETIMEPERMUTATION : m_pModel = new CSpaceTimePermutationModel(pParameters, this, pPrintDirection); break;
      }

      //For now, compute the angle and store the angle and shape
      //for each ellipsoid.  Maybe transfer info to a different location in the
      //application or compute "on the fly" prior to printing.
      if (m_pParameters->m_lTotalNumEllipses > 0) {
         mdE_Angles = new double[m_pParameters->m_lTotalNumEllipses];
         mdE_Shapes = new double[m_pParameters->m_lTotalNumEllipses];
         for (int es = 0; es < m_nNumEllipsoids; ++es) {
            for(int ea = 0; ea < m_pParameters->mp_nENumbers[es]; ++ea) {
               mdE_Angles[lCurrentEllipse]=PI*ea/m_pParameters->mp_nENumbers[es];
               mdE_Shapes[lCurrentEllipse]= m_pParameters->mp_dEShapes[es];
               ++lCurrentEllipse ;
            }
         }
      }
   }
   catch (SSException & x) {
      x.AddCallpath("CSaTScanData(CParameters *)", "CSaTScanData");
      throw;
   }
}

CSaTScanData::~CSaTScanData() {
  delete m_pModel;

  int i, j;
  long lTotalNumEllipses = m_pParameters->m_lTotalNumEllipses;

  // it looks complicated but it cuts out about 7 loops from the original which were unnecessary - AJV 8-28-2002
  for (i = 0; i < m_nTimeIntervals; ++i) {
    if (m_pCases)
       free(m_pCases[i]);
    if (m_pControls)
       free(m_pControls[i]);
    if (m_pMeasure)
       free(m_pMeasure[i]);
  }

  if (m_pCases)
     free(m_pCases);
  if (m_pControls)
     free(m_pControls);
  if (m_pMeasure)
     free(m_pMeasure);
  if (m_pPTCases)
    free(m_pPTCases);
  if (m_pPTMeasure)
    free(m_pPTMeasure);

  for (i = 0; i <= lTotalNumEllipses; ++i) {
     for (j = 0; j < m_nGridTracts; ++j) {
        if(m_pSortedInt)
           free(m_pSortedInt[i][j]);
        if (m_pSortedUShort)
           free(m_pSortedUShort[i][j]);
     }
     if (m_NeighborCounts)
         free(m_NeighborCounts[i]);
     if(m_pSortedInt)
         free(m_pSortedInt[i]);
     if (m_pSortedUShort)
         free(m_pSortedUShort[i]);
  }

  if(m_pSortedInt)
     free(m_pSortedInt);
  if (m_pSortedUShort)
     free(m_pSortedUShort);
  if (m_NeighborCounts)
     free(m_NeighborCounts);

  //delete the ellipsoid angle and shape array
  delete [] mdE_Angles;
  delete [] mdE_Shapes;

  free(m_pIntervalStartTimes);

  if (m_pSimCases)
     DeAllocSimCases();

   delete gpCats;            // DTG
   delete gpTInfo;          // DTG
   delete gpGInfo;
}

// After each pass through sequential analysis, data on top cluster is
// removed and neighbor counts must be adjusted to include all neighbors
// within the maximum circle size.
void CSaTScanData::AdjustNeighborCounts() {
   int i, j;
   measure_t nCumMeasure;

   try {
      for (i=0; i < m_nGridTracts; ++i) {
         nCumMeasure = 0;
         for (j=1; j<=m_nGridTracts && nCumMeasure+m_pMeasure[0][GetNeighbor(0, i,j)] <= m_nMaxCircleSize; ++j)
            nCumMeasure += m_pMeasure[0][GetNeighbor(0, i,j)];

         m_NeighborCounts[0][i] = j-1;
      }
   }
   catch (SSException & x) {
      x.AddCallpath("AdjustNeighborCounts()", "CSaTScanData");
      throw;
   }
}

void CSaTScanData::AllocSimCases() {
   try {
      m_pSimCases = (count_t**)Smalloc(m_nTimeIntervals * sizeof(count_t *), gpPrintDirection);
      memset(m_pSimCases, 0, m_nTimeIntervals * sizeof(count_t *));
      for(int i = 0; i < m_nTimeIntervals; ++i)
         m_pSimCases[i] = (count_t*)Smalloc(m_nTracts * sizeof(count_t), gpPrintDirection);
   }
   catch (SSException & x) {
      x.AddCallpath("GetNeighbor()", "CSaTScanData");
      throw;
   }
}


bool CSaTScanData::CalculateMeasure() {
   bool bReturn;

   try {
      bReturn = (m_pModel->CalculateMeasure());

      m_nTotalTractsAtStart   = m_nTracts;
      m_nTotalCasesAtStart    = m_nTotalCases;
      m_nTotalControlsAtStart = m_nTotalControls;
      m_nTotalMeasureAtStart  = m_nTotalMeasure;

      SetMaxCircleSize();
   }
   catch (SSException & x) {
      x.AddCallpath("CalculateMeasure()", "CSaTScanData");
      throw;
   }
  return bReturn;
}

int CSaTScanData::ComputeNewCutoffInterval(Julian jStartDate, Julian jEndDate) {
   int iIntervalCut;
   long lTimeBetween;

   try {
      if (m_nTimeIntervals == 1)
         iIntervalCut = 1;
      else if (m_nTimeIntervals > 1) {
         iIntervalCut = 0;
         lTimeBetween = (TimeBetween(jStartDate, jEndDate, m_pParameters->m_nIntervalUnits))*m_pParameters->m_nMaxTemporalClusterSize/100.0;
         iIntervalCut = lTimeBetween / m_pParameters->m_nIntervalLength;
      }
   }
   catch (SSException & x) {
      x.AddCallpath("ComputeNewCutoffInterval()", "CSaTScanData");
      throw;
   }
   return iIntervalCut;
}

void CSaTScanData::DeAllocSimCases() {
  if (m_pSimCases) {
     for (int i = 0; i < m_nTimeIntervals; ++i)
       free(m_pSimCases[i]);
     free(m_pSimCases);
     m_pSimCases = 0;
  }
}

bool CSaTScanData::FindNeighbors() {
   int i, j;
   long lTotalNumEllipses = m_pParameters->m_lTotalNumEllipses;

   try {
      //then use an unsigned short...
      if (m_nTracts < 65536) {
         m_pSortedUShort = (unsigned short ***)Smalloc(sizeof(unsigned short *) * (lTotalNumEllipses+1), gpPrintDirection);
         for (i = 0; i <= lTotalNumEllipses; ++i )
            m_pSortedUShort[i] = (unsigned short **)Smalloc(sizeof(unsigned short *) * m_nGridTracts, gpPrintDirection);
         for (i = 0; i <= lTotalNumEllipses; ++i)                                          //memset here ???
            for (j = 0; j < m_nGridTracts; ++j)
               m_pSortedUShort[i][j] = 0;
      }
      else {
         m_pSortedInt = (tract_t ***)Smalloc(sizeof(tract_t *) * (lTotalNumEllipses+1), gpPrintDirection);
         for (i = 0; i <= lTotalNumEllipses; ++i )
            m_pSortedInt[i] = (tract_t **)Smalloc(sizeof(tract_t *) * m_nGridTracts, gpPrintDirection);
         for (i = 0; i <= lTotalNumEllipses; ++i)                                          //memset here ???
            for (j = 0; j < m_nGridTracts; ++j)
               m_pSortedInt[i][j] = 0;
         }
      //m_NeighborCounts = (tract_t**)Smalloc(m_nNumEllipsoids * m_nGridTracts * sizeof(tract_t));          //DTG --  change this to multiply in the number of ellipsoids
      m_NeighborCounts = (tract_t**)Smalloc((lTotalNumEllipses + 1) * sizeof(tract_t *), gpPrintDirection);
      for(i = 0; i <= lTotalNumEllipses; ++i) {
         m_NeighborCounts[i] = (tract_t*)Smalloc(m_nGridTracts * sizeof(tract_t), gpPrintDirection);
         for (j = 0; j < m_nGridTracts; ++j)
            m_NeighborCounts[i][j] = 0;                // USE MEMSET HERE...
      }

      if (m_pParameters->m_bSequential)
        MakeNeighbors(gpTInfo, gpGInfo, m_pSortedInt, m_pSortedUShort, m_nTracts, m_nGridTracts,
                      m_pMeasure[0], m_nMaxCircleSize, m_nTotalMeasure, m_NeighborCounts,
                      m_pParameters->m_nDimension, m_pParameters->m_nNumEllipses,
                      m_pParameters->mp_dEShapes, m_pParameters->mp_nENumbers, gpPrintDirection);
      else
        MakeNeighbors(gpTInfo, gpGInfo, m_pSortedInt, m_pSortedUShort, m_nTracts, m_nGridTracts,
                      m_pMeasure[0], m_nMaxCircleSize, m_nMaxCircleSize, m_NeighborCounts,
                      m_pParameters->m_nDimension, m_pParameters->m_nNumEllipses,
                      m_pParameters->mp_dEShapes, m_pParameters->mp_nENumbers, gpPrintDirection);

   }
   catch (SSException & x) {
      x.AddCallpath("FindNeighbors()", "CSaTScanData");
      throw;
   }
  return true;
}

double CSaTScanData::GetAnnualRate() const {
// Call to TimeBetween removed so results would match V.1.0.6
// Should TimeBetween be updated to use 365.2425?
// And do we need TimeBetween that returns a double? KR-980325
//  double nYears = TimeBetween(m_nStartDate, m_nEndDate, YEAR);
  double nYears      = (double)(m_nEndDate+1 - m_nStartDate) / 365.2425;
  double nAnnualRate = (m_nAnnualRatePop*(double)m_nTotalCases) / ((double)m_nTotalPop*nYears);

  return nAnnualRate;
}

double CSaTScanData::GetAnnualRateAtStart() const {
  double nYears      = (double)(m_nEndDate+1 - m_nStartDate) / 365.2425;
  double nAnnualRate = (m_nAnnualRatePop*(double)m_nTotalCasesAtStart) / ((double)m_nTotalPop*nYears);

  return nAnnualRate;
}

//Measure Adjustment used when calculating relative risk/expected counts
//to disply in report file.
double CSaTScanData::GetMeasureAdjustment() const {
  if (m_pParameters->m_nModel == POISSON || m_pParameters->m_nModel == SPACETIMEPERMUTATION)
    return 1.0;
  else if (m_pParameters->m_nModel == BERNOULLI)
    return (double)m_nTotalCases/(double)m_nTotalPop;
  else
    return 0.0;
}

/**********************************************************************
 Return "nearness"-th closest neighbor to "t"
 (nearness == 1 returns "t").
 **********************************************************************/
tract_t CSaTScanData::GetNeighbor(int iEllipse, tract_t t, unsigned int nearness) const {
   if (m_pSortedUShort)
      return (tract_t)m_pSortedUShort[iEllipse][t][nearness - 1];
   else
      return m_pSortedInt[iEllipse][t][nearness - 1];
}

void CSaTScanData::Init() {
      gpCats = 0;            // DTG
      gpTInfo = 0;          // DTG
      gpGInfo = 0;
      m_pModel = 0;
      m_pCases     = 0;
      m_pControls  = 0;
      m_pMeasure   = 0;
      m_pPTCases   = 0;
      m_pPTSimCases = 0;
      m_pPTMeasure = 0;
      m_pSimCases  = 0;
      m_pSortedInt = 0;
      m_pSortedUShort = 0;

      m_NeighborCounts = 0;
      m_pTimes = 0;
      m_pIntervalStartTimes = 0;
      m_nTotalCases    = 0;
      m_nTotalControls = 0;
      m_nTotalMeasure  = 0;
      m_nAnnualRatePop = 100000;
      mdE_Angles = 0;
      mdE_Shapes = 0;
}

void CSaTScanData::MakeData() {
   try {
      m_pModel->MakeData();
   }
   catch (SSException & x) {
      x.AddCallpath("MakeData()", "CSaTScanData");
      throw;
   }
}

void CSaTScanData::PrintNeighbors() {
   FILE* pFile;
   int i, j, k;

   try {
      if ((pFile = fopen("c:\\SatScan V.2.1.4\\Borland Calc\\neighbors.txt", "w")) == NULL)
       gpPrintDirection->SatScanPrintf("  Error: Unable to open neighbors file.\n");
      else {
        for (i = 0; i <= m_pParameters->m_lTotalNumEllipses; ++i )
          for (j = 0; j < m_nGridTracts; ++j) {
            k = 0;
            fprintf(pFile, "Ellipse Number %i, Tract  %i - ", i, j);
            while (m_pSortedInt[i][j][k] >= 0)
               fprintf(pFile, "%i, ", m_pSortedInt[i][j][k++]);
            fprintf(pFile, "%i \n", m_pSortedInt[i][j][k]);
          }
         fclose(pFile);
      }
   }
   catch (SSException & x) {
      x.AddCallpath("PrintNeighbors()", "CSaTScanData");
      throw;
   }
}

void CSaTScanData::ReadDataFromFiles() {
   bool bDataOk = true;

   try {
      SetStartAndEndDates();
      SetNumTimeIntervals();
      SetIntervalCut();
      SetIntervalStartTimes();

      if (m_pParameters->m_nAnalysisType == PROSPECTIVESPACETIME)
         SetProspectiveIntervalStart();

      if (!m_pModel->ReadData())
         SSGenerateException("\nProblem encountered reading in data.", "ReadDataFromFiles");

      if (bDataOk && gpTInfo->tiFindDuplicateCoords(stderr))
        SSGenerateException("Program canceled.\n", "ReadDataFromFiles");

      // SINCE giFindDuplicateCoords ONLY RETURNS "FALSE" NOW, I DO NOT KNOW
      // WHY THIS CHECK IS EVEN HERE...   DELETE AFTER REVIEW !!!!
      if (bDataOk && gpGInfo->giFindDuplicateCoords(stderr))
        SSGenerateException("Program canceled.\n", "ReadDataFromFiles");
   }
   catch (SSException & x) {
      x.AddCallpath("ReadDataFromFiles()", "CSaTScanData");
      throw;
   }
}

void CSaTScanData::SetIntervalCut() {
  /* Calculates the number of time intervals to include in potential clusters */
  /* without exceeding the maximum cluster size with respect to time.         */
  /* gpPrintDirection->SatScanPrintf("Calculate number of time intervals...\n"); /*KR-6/22/97*/
   long lTimeBetween;

   try {
      if (m_nTimeIntervals == 1)
         m_nIntervalCut = 1;
      else if (m_nTimeIntervals > 1) {
           lTimeBetween = TimeBetween(m_nStartDate, m_nEndDate, m_pParameters->m_nIntervalUnits)*m_pParameters->m_nMaxTemporalClusterSize/100.0;
           m_nIntervalCut = lTimeBetween / m_pParameters->m_nIntervalLength;
      }

      if (m_nIntervalCut==0) {
         char sMessage[200];
         strcpy(sMessage, "  Error: The interval length is longer than the maximum cluster size with\n");
         strcat(sMessage, "         respect to time. No clusters can be found.\n");
         SSGenerateException(sMessage, "SetIntervalCut");
      }
   }
   catch (SSException & x) {
      x.AddCallpath("SetIntervalCut()", "CSaTScanData");
      throw;
   }
}

void CSaTScanData::SetIntervalStartTimes() {
   // Not neccessary for purely spatial?
   try {
      m_pIntervalStartTimes = (Julian*) Smalloc((m_nTimeIntervals+1)*sizeof(Julian), gpPrintDirection);

      m_pIntervalStartTimes[0] = m_nStartDate;
      m_pIntervalStartTimes[m_nTimeIntervals] = m_nEndDate+1;

      for (int i = m_nTimeIntervals-1; i > 0; --i)
         m_pIntervalStartTimes[i] = DecrementDate(m_pIntervalStartTimes[i+1], m_pParameters->m_nIntervalUnits, m_pParameters->m_nIntervalLength);
   }
   catch (SSException & x) {
      x.AddCallpath("SetIntervalStartTimes()", "CSaTScanData");
      throw;
   }
}

void CSaTScanData::SetNumTimeIntervals() {
   try {
      long nTime = TimeBetween(m_nStartDate,
                           m_nEndDate,
                           m_pParameters->m_nIntervalUnits);

      m_nTimeIntervals = (int)ceil((float) nTime /
                               (float) m_pParameters->m_nIntervalLength);
   }
   catch (SSException & x) {
      x.AddCallpath("SetNumTimeIntervals()", "CSaTScanData");
      throw;
   }
}

/* Calculates which time interval the prospectice space-time start date is in.*/
/* MAKE SURE THIS IS EXECUTED AFTER THE  m_nTimeIntervals VARIABLE HAS BEEN SET */
void CSaTScanData::SetProspectiveIntervalStart() {
  long    lTime;
  Julian  lProspStartDate;

  try {
      lProspStartDate = CharToJulian(m_pParameters->m_szProspStartDate);
      lTime = TimeBetween(m_nStartDate,
                           lProspStartDate,
                           m_pParameters->m_nIntervalUnits);

      m_nProspectiveIntervalStart = (int)ceil((float) lTime /
                               (float) m_pParameters->m_nIntervalLength);
      if (m_nProspectiveIntervalStart < 0)
         SSGenerateException("  Error: The Start Date of Prospective Space-Time is prior to the Study Begin Date.\n", "SetProspectiveIntervalStart");
      else if (m_nProspectiveIntervalStart > m_nTimeIntervals)
         SSGenerateException("  Error: The Start Date of Prospective Space-Time is after the Study End Date.\n", "SetProspectiveIntervalStart");
  }
   catch (SSException & x) {
      x.AddCallpath("SetProspectiveIntervalStart()", "CSaTScanData");
      throw;
   }
}

void CSaTScanData::SetPurelyTemporalCases() {
   try {
      m_pPTCases = (count_t*) Smalloc((m_nTimeIntervals+1)*sizeof(count_t), gpPrintDirection);

      for (int i=0; i<m_nTimeIntervals; ++i) {
         m_pPTCases[i] = 0;

         for (int t=0; t<m_nTracts; ++t)
            m_pPTCases[i] += m_pCases[i][t];
      }
   }
   catch (SSException & x) {
      x.AddCallpath("SetPurelyTemporalCases()", "CSaTScanData");
      throw;
   }
}

void CSaTScanData::SetPurelyTemporalMeasures() {
   try {
      m_pPTMeasure = (measure_t*)Smalloc((m_nTimeIntervals+1) * sizeof(measure_t), gpPrintDirection);

      for (int i=0; i<m_nTimeIntervals; ++i) {
         m_pPTMeasure[i] = 0;
         for (int t=0; t<m_nTracts; ++t)
            m_pPTMeasure[i] += m_pMeasure[i][t];
      }
   }
   catch (SSException & x) {
      x.AddCallpath("SetPurelyTemporalMeasures()", "CSaTScanData");
      throw;
   }
}

void CSaTScanData::SetPurelyTemporalSimCases() {
   try {
      for (int i = 0; i < m_nTimeIntervals; ++i) {
         m_pPTSimCases[i] = 0;
         for (int t = 0; t < m_nTracts; ++t)
            m_pPTSimCases[i] += m_pSimCases[i][t];
      }
   }
   catch (SSException & x) {
      x.AddCallpath("SetPurelyTemporalSimCases()", "CSaTScanData");
      throw;
   }
}

void CSaTScanData::SetStartAndEndDates() {
   try {
      m_nStartDate = CharToJulian(m_pParameters->m_szStartDate);
      m_nEndDate   = CharToJulian(m_pParameters->m_szEndDate);
   }
   catch (SSException & x) {
      x.AddCallpath("SetStartAndEndDates()", "CSaTScanData");
      throw;
   }
}



