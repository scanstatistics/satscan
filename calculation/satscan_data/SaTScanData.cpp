#include "SaTScan.h"
#pragma hdrstop
#include "SaTScanData.h"

CSaTScanData::CSaTScanData(CParameters* pParameters, BasePrint *pPrintDirection) {
  try {
    Init();
    Setup(pParameters, pPrintDirection);
  }
  catch (ZdException & x) {
    x.AddCallpath("CSaTScanData(CParameters *)", "CSaTScanData");
    throw;
  }
}

CSaTScanData::~CSaTScanData() {
  delete m_pModel;

  int i, j;
  long lTotalNumEllipses = m_pParameters->GetNumTotalEllipses();

  // it looks complicated but it cuts out about 7 loops from the original which were unnecessary - AJV 8-28-2002
  for (i = 0; i < m_nTimeIntervals; ++i) {
    if (m_pCases)
       free(m_pCases[i]);
    if (m_pCases_NC)
      free(m_pCases_NC[i]);
    if (m_pControls)
       free(m_pControls[i]);
    if (m_pMeasure)
       free(m_pMeasure[i]);
    if (m_pMeasure_NC)
       free(m_pMeasure_NC[i]);
    if (m_pSimCases_NC)
       free(m_pSimCases_NC[i]);
  }
  //measure allocated to number time intervals plus one
  if (m_pMeasure)
    free(m_pMeasure[m_nTimeIntervals]);
  if (m_pMeasure_NC)
    free(m_pMeasure_NC[m_nTimeIntervals]);


  if (m_pCases)
     free(m_pCases);
  if (m_pCases_NC)
     free(m_pCases_NC);
  if (m_pControls)
     free(m_pControls);
  if (m_pMeasure)
     free(m_pMeasure);
  if (m_pMeasure_NC)
     free(m_pMeasure_NC);
  if (m_pPTCases)
    free(m_pPTCases);
  if (m_pPTMeasure)
    free(m_pPTMeasure);
  if (m_pSimCases_NC)
    free(m_pSimCases_NC);
  if (m_pSimCases_TotalByTimeInt)
    free(m_pSimCases_TotalByTimeInt);
  if (m_pCases_TotalByTimeInt)
    free(m_pCases_TotalByTimeInt);
  if (m_pMeasure_TotalByTimeInt)
    free(m_pMeasure_TotalByTimeInt);

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

   delete gpTInfo;          // DTG
   delete gpGInfo;
}

/** Sequential analyses will call this function to clear neighbor information and
    re-calculate neighbors. Note that only when the maximum spatial cluster size
    is specified as a percentage of the population that this operation need be
    performed between iterations of a sequential scan. */
void CSaTScanData::AdjustNeighborCounts() {
  int           i, j;

  try {
    //Deallocate neighbor information in sorted structures.
    if (m_pParameters->GetMaxGeographicClusterSizeType() == PERCENTAGEOFMEASURETYPE) {
      //Free/clear previous interation's neighbor information.
      if (m_pSortedUShort) {
        for (i=0; i <= m_pParameters->GetNumTotalEllipses(); ++i)
           for (j=0; j < m_nGridTracts; ++j) {
              free(m_pSortedUShort[i][j]); m_pSortedUShort[i][j]=0;
              m_NeighborCounts[i][j]=0;
           }
      }
      else {
        for (i=0; i <= m_pParameters->GetNumTotalEllipses(); ++i)
           for (j=0; j < m_nGridTracts; ++j) {
              free(m_pSortedInt[i][j]); m_pSortedInt[i][j]=0;
              m_NeighborCounts[i][j]=0;
           }
      }
      //Recompute neighbors.
      MakeNeighbors(gpTInfo, gpGInfo, m_pSortedInt, m_pSortedUShort, m_nTotalTractsAtStart/*m_nTracts*/, m_nGridTracts,
                    m_pMeasure[0], m_nMaxCircleSize, m_nMaxCircleSize, m_NeighborCounts,
                    m_pParameters->GetDimensionsOfData(), m_pParameters->GetNumRequestedEllipses(),
                    m_pParameters->GetEllipseShapes(), m_pParameters->GetEllipseRotations(),
                    m_pParameters->GetMaxGeographicClusterSizeType(), gpPrint);
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("AdjustNeighborCounts()", "CSaTScanData");
    throw;
  }
}

void CSaTScanData::AllocSimCases() {
   try {
      m_pSimCases = (count_t**)Smalloc(m_nTimeIntervals * sizeof(count_t *), gpPrint);
      memset(m_pSimCases, 0, m_nTimeIntervals * sizeof(count_t *));
      for(int i = 0; i < m_nTimeIntervals; ++i)
         m_pSimCases[i] = (count_t*)Smalloc(m_nTracts * sizeof(count_t), gpPrint);
   }
   catch (ZdException & x) {
      x.AddCallpath("GetNeighbor()", "CSaTScanData");
      throw;
   }
}

bool CSaTScanData::CalculateMeasure() {
  bool bReturn;

  try {
    SetAdditionalCaseArrays();
    bReturn = (m_pModel->CalculateMeasure());
    m_nTotalTractsAtStart   = m_nTracts;
    m_nTotalCasesAtStart    = m_nTotalCases;
    m_nTotalControlsAtStart = m_nTotalControls;
    m_nTotalMeasureAtStart  = m_nTotalMeasure;
    SetMaxCircleSize();
  }
  catch (ZdException &x) {
    x.AddCallpath("CalculateMeasure()","CSaTScanData");
    throw;
  }
  return bReturn;
}

int CSaTScanData::ComputeNewCutoffInterval(Julian jStartDate, Julian& jEndDate) {
   int  iIntervalCut;
   long lTimeBetween;

   if (m_pParameters->GetMaximumTemporalClusterSizeType() == PERCENTAGETYPE) {
     lTimeBetween = static_cast<long>((TimeBetween(jStartDate, jEndDate, m_pParameters->GetTimeIntervalUnitsType()))*m_pParameters->GetMaximumTemporalClusterSize()/100.0);
     iIntervalCut = lTimeBetween / m_pParameters->GetTimeIntervalLength();
     //now compute a new Current Date by subtracting the interval duration
     jEndDate = DecrementDate(jEndDate, m_pParameters->GetTimeIntervalUnitsType(), m_pParameters->GetTimeIntervalLength());
   }
   else
     iIntervalCut = m_nIntervalCut;

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
   int          i, j;
   double       dTotalPopulation=0;
   long         lTotalNumEllipses = m_pParameters->GetNumTotalEllipses();

   try {
      //then use an unsigned short...
      if (m_nTracts < 65536) {
         m_pSortedUShort = (unsigned short ***)Smalloc(sizeof(unsigned short *) * (lTotalNumEllipses+1), gpPrint);
         for (i = 0; i <= lTotalNumEllipses; ++i )
            m_pSortedUShort[i] = (unsigned short **)Smalloc(sizeof(unsigned short *) * m_nGridTracts, gpPrint);
         for (i = 0; i <= lTotalNumEllipses; ++i)                                          //memset here ???
            for (j = 0; j < m_nGridTracts; ++j)
               m_pSortedUShort[i][j] = 0;
      }
      else {
         m_pSortedInt = (tract_t ***)Smalloc(sizeof(tract_t *) * (lTotalNumEllipses+1), gpPrint);
         for (i = 0; i <= lTotalNumEllipses; ++i )
            m_pSortedInt[i] = (tract_t **)Smalloc(sizeof(tract_t *) * m_nGridTracts, gpPrint);
         for (i = 0; i <= lTotalNumEllipses; ++i)                                          //memset here ???
            for (j = 0; j < m_nGridTracts; ++j)
               m_pSortedInt[i][j] = 0;
         }
      //m_NeighborCounts = (tract_t**)Smalloc(m_nNumEllipsoids * m_nGridTracts * sizeof(tract_t));          //DTG --  change this to multiply in the number of ellipsoids
      m_NeighborCounts = (tract_t**)Smalloc((lTotalNumEllipses + 1) * sizeof(tract_t *), gpPrint);
      for(i = 0; i <= lTotalNumEllipses; ++i) {
         m_NeighborCounts[i] = (tract_t*)Smalloc(m_nGridTracts * sizeof(tract_t), gpPrint);
         for (j = 0; j < m_nGridTracts; ++j)
            m_NeighborCounts[i][j] = 0;                // USE MEMSET HERE...
      }

      //adjust special population file now that we know the total case count
      if (m_pParameters->UseSpecialPopulationFile()) {
        for (i=0; i < (int)gvCircleMeasure.size(); i++)
           dTotalPopulation += gvCircleMeasure[i];
        if (dTotalPopulation ==0)
          SSGenerateException("Error: Total population for special population file can not be zero.","FindNeighbors()");
        for (i=0; i < (int)gvCircleMeasure.size(); i++)
          gvCircleMeasure[i] *= m_nTotalCases/dTotalPopulation;
      }

      if (m_pParameters->GetIsSequentialScanning())
        MakeNeighbors(gpTInfo, gpGInfo, m_pSortedInt, m_pSortedUShort, m_nTracts, m_nGridTracts,
                      (m_pParameters->UseSpecialPopulationFile() ? &gvCircleMeasure[0] : m_pMeasure[0]),
                      m_nMaxCircleSize, m_nTotalMeasure, m_NeighborCounts,
                      m_pParameters->GetDimensionsOfData(), m_pParameters->GetNumRequestedEllipses(),
                      m_pParameters->GetEllipseShapes(), m_pParameters->GetEllipseRotations(),
                      m_pParameters->GetMaxGeographicClusterSizeType(), gpPrint);
      else
        MakeNeighbors(gpTInfo, gpGInfo, m_pSortedInt, m_pSortedUShort, m_nTracts, m_nGridTracts,
                      (m_pParameters->UseSpecialPopulationFile() ? &gvCircleMeasure[0] : m_pMeasure[0]),
                      m_nMaxCircleSize, m_nMaxCircleSize, m_NeighborCounts,
                      m_pParameters->GetDimensionsOfData(), m_pParameters->GetNumRequestedEllipses(),
                      m_pParameters->GetEllipseShapes(), m_pParameters->GetEllipseRotations(),
                      m_pParameters->GetMaxGeographicClusterSizeType(), gpPrint);

   }
   catch (ZdException & x) {
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
  if (m_pParameters->GetProbabiltyModelType() == POISSON || m_pParameters->GetProbabiltyModelType() == SPACETIMEPERMUTATION)
    return 1.0;
  else if (m_pParameters->GetProbabiltyModelType() == BERNOULLI)
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
  gpTInfo = 0;
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
  m_pCases_NC    = 0;
  m_pSimCases_NC = 0;
  m_pMeasure_NC  = 0;
  m_pCases_TotalByTimeInt = 0;
  m_pSimCases_TotalByTimeInt = 0;
  m_pMeasure_TotalByTimeInt = 0;
}
void CSaTScanData::MakeData(int iSimulationNumber) {
   try {
      m_pModel->MakeData(iSimulationNumber);
   }
   catch (ZdException & x) {
      x.AddCallpath("MakeData()", "CSaTScanData");
      throw;
   }
}

void CSaTScanData::PrintNeighbors() {
   FILE* pFile;
   int i, j, k;

   try {
      if ((pFile = fopen("c:\\SatScan V.2.1.4\\Borland Calc\\neighbors.txt", "w")) == NULL)
       gpPrint->SatScanPrintf("  Error: Unable to open neighbors file.\n");
      else {
        for (i = 0; i <= m_pParameters->GetNumTotalEllipses(); ++i )
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
   catch (ZdException & x) {
      x.AddCallpath("PrintNeighbors()", "CSaTScanData");
      throw;
   }
}

void CSaTScanData::ReadDataFromFiles() {
  try {
    SetStartAndEndDates();
    SetNumTimeIntervals();
    SetIntervalCut();
    SetIntervalStartTimes();
    SetTimeIntervalRangeIndexes();

    if (m_pParameters->GetIsProspectiveAnalysis())
      SetProspectiveIntervalStart();

    if (! m_pModel->ReadData())
      SSGenerateException("\nProblem encountered reading in data.", "ReadDataFromFiles");

    gpTInfo->tiConcaticateDuplicateTractIdentifiers();
    gpGInfo->giFindDuplicateCoords(stderr);
  }
  catch (ZdException & x) {
    x.AddCallpath("ReadDataFromFiles()", "CSaTScanData");
    throw;
  }
}

/** Conditionally allocates and sets additional case arrays. */
void CSaTScanData::SetAdditionalCaseArrays() {
  try {
    if (m_pParameters->GetTimeTrendAdjustmentType() == STRATIFIED_RANDOMIZATION ||
        m_pParameters->GetTimeTrendAdjustmentType() == CALCULATED_LOGLINEAR_PERC)
      SetCasesByTimeIntervalArray();
  }
  catch (ZdException &x) {
    x.AddCallpath("SetAdditionalCaseArrays()","CSaTScanData");
    throw;
  }
}

/** Allocates array that stores the total number of cases for each time
    interval as gotten from cumulative two dimensional case array. */
void CSaTScanData::SetCasesByTimeIntervalArray() {
  int   i, j;

  try {
    m_pCases_TotalByTimeInt = (count_t*)Smalloc(m_nTimeIntervals * sizeof(count_t), gpPrint);
    memset(m_pCases_TotalByTimeInt, 0, m_nTimeIntervals * sizeof(count_t));

    for (i=0; i < m_nTracts; i++) {
       m_pCases_TotalByTimeInt[m_nTimeIntervals-1] += m_pCases[m_nTimeIntervals-1][i];
       for (j=m_nTimeIntervals-2; j >= 0; j--)
          m_pCases_TotalByTimeInt[j] += m_pCases[j][i] - m_pCases[j+1][i];
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("SetCasesByTimeIntervalArray()","CSaTScanData");
    throw;
  }
}

/** Allocates two-dimensional array to be used as cumulative measure.
    Data assembled using previously defined non-cumulative measure. */
void CSaTScanData::SetCumulativeMeasure() {
  int   i, tract;

  try {
    if (m_pMeasure)
      ZdGenerateException("Error: Cumulative measure already allocated.\n", "SetCumulativeMeasure()");

    if (m_pMeasure_NC == 0)
      ZdGenerateException("Error: Non-cumulative measure is not allocated.\n", "SetCumulativeMeasure()");

    /*note: measure allocated in CModel::CalculateMeasure() to m_nTimeIntervals + 1,
            not sure why but allocate this array the same way for deallocation,
            m_pMeasure could have been the array that as passed to
            CModel::CalculateMeasure() instead of m_pMeasire_NC. */
    m_pMeasure = (double**)Smalloc((m_nTimeIntervals + 1) * sizeof(measure_t *), gpPrint);
    memset(m_pMeasure, 0, (m_nTimeIntervals + 1) * sizeof(measure_t *));
    for (i=0; i < m_nTimeIntervals; i++)
      m_pMeasure[i] = (double*)Smalloc(m_nTracts * sizeof(measure_t), gpPrint);

    for (tract=0; tract < m_nTracts; tract++) {
       m_pMeasure[m_nTimeIntervals-1][tract]=m_pMeasure_NC[m_nTimeIntervals-1][tract];
       for (i=m_nTimeIntervals-2; i >= 0; i--)
          m_pMeasure[i][tract]=m_pMeasure[i+1][tract] + m_pMeasure_NC[i][tract];
    }

    // Bug check, to ensure that TotalCases=TotalMeasure
    if (fabs(m_nTotalCases-m_nTotalMeasure)>0.0001)
      ZdGenerateException("Error: The total measure '%8.6lf' is not equal to the total number of cases '%ld'.\n",
                          "SetCumulativeMeasure()", m_nTotalMeasure, m_nTotalCases);
  }
  catch (ZdException &x) {
    x.AddCallpath("SetCumulativeMeasure()","CSaTScanData");
    throw;
  }
}

/** Allocates and sets measure array that represents non-cumulative measure
    for all time intervals from cumulative measure array. */
void CSaTScanData::SetMeasureByTimeIntervalArray() {
  int   i, j;

  try {
    if (! m_pMeasure_TotalByTimeInt)
       m_pMeasure_TotalByTimeInt = (measure_t*)Smalloc(m_nTimeIntervals * sizeof(measure_t), gpPrint);

    memset(m_pMeasure_TotalByTimeInt, 0, m_nTimeIntervals * sizeof(measure_t));
    for (i=0; i < m_nTracts; i++) {
       m_pMeasure_TotalByTimeInt[m_nTimeIntervals-1] += m_pMeasure[m_nTimeIntervals-1][i];
       for (j=m_nTimeIntervals-2; j >= 0; j--)
          m_pMeasure_TotalByTimeInt[j] += m_pMeasure[j][i] - m_pMeasure[j+1][i];
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("SetMeasureByTimeIntervalArray()","CSaTScanData");
    throw;
  }
}

/** Allocates and sets measure array that represents non-cumulative measure
    for all time intervals from passed non-cumulative measure array. */
void CSaTScanData::SetMeasureByTimeIntervalArray(measure_t ** pNonCumulativeMeasure) {
  int   i, j;

  try {
    if (! m_pMeasure_TotalByTimeInt)
       m_pMeasure_TotalByTimeInt = (measure_t*)Smalloc(m_nTimeIntervals * sizeof(measure_t), gpPrint);

    memset(m_pMeasure_TotalByTimeInt, 0, m_nTimeIntervals * sizeof(measure_t));
    for (i=0; i < m_nTimeIntervals; i++)
       for (j=0; j < m_nTracts; j++)
          m_pMeasure_TotalByTimeInt[i] += pNonCumulativeMeasure[i][j];
  }
  catch (ZdException &x) {
    x.AddCallpath("SetMeasureByTimeIntervalArray()","CSaTScanData");
    throw;
  }
}

/* Calculates the number of time intervals to include in potential clusters
   without exceeding the maximum cluster size with respect to time.*/
void CSaTScanData::SetIntervalCut() {
  ZdString      sIntervalCutMessage, sTimeIntervalType;
  long          lMaxTemporalLength, lStudyPeriodLength;

  try {
    if (m_nTimeIntervals == 1)
      m_nIntervalCut = 1;
    else if (m_nTimeIntervals > 1) {
      if (m_pParameters->GetMaximumTemporalClusterSizeType() == PERCENTAGETYPE) {
        lStudyPeriodLength = TimeBetween(m_nStartDate, m_nEndDate, m_pParameters->GetTimeIntervalUnitsType());
        lMaxTemporalLength = static_cast<long>(lStudyPeriodLength * m_pParameters->GetMaximumTemporalClusterSize()/100.0);
        m_nIntervalCut = lMaxTemporalLength / m_pParameters->GetTimeIntervalLength();
      }
      else
        m_nIntervalCut = static_cast<int>(m_pParameters->GetMaximumTemporalClusterSize() / m_pParameters->GetTimeIntervalLength());
    }

    if (m_nIntervalCut==0) {
      switch (m_pParameters->GetTimeIntervalUnitsType()) {
          case YEAR  : sTimeIntervalType = "year"; break;
          case MONTH : sTimeIntervalType = "month"; break;
          case DAY   : sTimeIntervalType = "day"; break;
          // if we get here, there is an error somewhere else
          default: sTimeIntervalType = "none"; break;
        };

        if (m_pParameters->GetMaximumTemporalClusterSizeType() == TIMETYPE) {
          sIntervalCutMessage << "Error: A maximum temporal cluster size of %g %s%s is less than one %d %s time interval.\n";
          sIntervalCutMessage << "       No clusters can be found.\n";
          SSGenerateException(sIntervalCutMessage.GetCString(), "SetIntervalCut()",
                              m_pParameters->GetMaximumTemporalClusterSize(), sTimeIntervalType.GetCString(),
                              (m_pParameters->GetMaximumTemporalClusterSize() == 1 ? "" : "s"),
                              m_pParameters->GetTimeIntervalLength(), sTimeIntervalType.GetCString());
        }
        else if (m_pParameters->GetMaximumTemporalClusterSizeType() == PERCENTAGETYPE) {
          sIntervalCutMessage << "Error: A maximum temporal cluster size that is %g percent of a %d %s study period\n";
          sIntervalCutMessage << "       equates to %d %s%s, which is less than one %d %s time interval.\n";
          sIntervalCutMessage << "       No clusters can be found.\n";
          SSGenerateException(sIntervalCutMessage.GetCString(), "SetIntervalCut()",
                              m_pParameters->GetMaximumTemporalClusterSize(),
                              lStudyPeriodLength, sTimeIntervalType.GetCString(),
                              lMaxTemporalLength, sTimeIntervalType.GetCString(), (lMaxTemporalLength == 1 ? "" : "s"),
                              m_pParameters->GetTimeIntervalLength(), sTimeIntervalType.GetCString());
        }
      }
   }
  catch (ZdException &x) {
    x.AddCallpath("SetIntervalCut()","CSaTScanData");
    throw;
  }
}

/** Set array of interval start times. */
void CSaTScanData::SetIntervalStartTimes() {
   // Not neccessary for purely spatial?
   try {
      m_pIntervalStartTimes = (Julian*) Smalloc((m_nTimeIntervals+1)*sizeof(Julian), gpPrint);

      m_pIntervalStartTimes[0] = m_nStartDate;
      m_pIntervalStartTimes[m_nTimeIntervals] = m_nEndDate+1;

      for (int i = m_nTimeIntervals-1; i > 0; --i)
         m_pIntervalStartTimes[i] = DecrementDate(m_pIntervalStartTimes[i+1], m_pParameters->GetTimeIntervalUnitsType(), m_pParameters->GetTimeIntervalLength());
   }
   catch (ZdException & x) {
      x.AddCallpath("SetIntervalStartTimes()", "CSaTScanData");
      throw;
   }
}

/** Causes maximum circle size to be set based on parameters settings. */
void CSaTScanData::SetMaxCircleSize() {
  try {
    switch (m_pParameters->GetMaxGeographicClusterSizeType()) {
      case PERCENTAGEOFMEASURETYPE :
           m_nMaxCircleSize = (m_pParameters->GetMaximumGeographicClusterSize() / 100.0) * m_nTotalMeasure;
           break;
      case DISTANCETYPE :
           m_nMaxCircleSize = m_pParameters->GetMaximumGeographicClusterSize();
           break;
      default : ZdException::Generate("Unknown maximum spatial cluster type: '%i'.", "SetMaxCircleSize()",
                                      m_pParameters->GetMaxGeographicClusterSizeType());
    };
  }
  catch (ZdException &x) {
    x.AddCallpath("SetMaxCircleSize()","CSaTScanData");
    throw;
  }
}

/** Sets measure array as cumulative in-place. The passed array is assumed to be
    set with non-cumulative data. Repeatly calling this function is not suggested.*/
void CSaTScanData::SetMeasureAsCumulative(measure_t ** pMeasure) {
  tract_t       t;
  int           i;

  for (t=0; t < m_nTracts; t++)
     for (i=m_nTimeIntervals-2; i >= 0; i--)
        pMeasure[i][t]= pMeasure[i+1][t] + pMeasure[i][t];
}

/** Sets non-cumulative measure from cumulative measure.
    Non-cumulative measure array should not be already allocated. */
void CSaTScanData::SetNonCumulativeMeasure() {
  if (m_pMeasure_NC)
    ZdException::Generate("Non-cumulative measure array already allocated.\n","SetNonCumulativeMeasure()");

    /*note: measure allocated in CModel::CalculateMeasure() to m_nTimeIntervals + 1,
            not sure why but allocate this array the same way for deallocation,
            m_pMeasure_NC could have been the array that as passed to
            CModel::CalculateMeasure() instead of m_pMeasire. */
  m_pMeasure_NC = (double**)Smalloc((m_nTimeIntervals + 1) * sizeof(measure_t *), gpPrint);
  memset(m_pMeasure_NC, 0, (m_nTimeIntervals + 1) * sizeof(measure_t *));
  for (int i=0; i < m_nTimeIntervals; i++)
     m_pMeasure_NC[i] = (double*)Smalloc(m_nTracts * sizeof(measure_t), gpPrint);

  for (int i=0; i < m_nTracts; i++) {
    m_pMeasure_NC[m_nTimeIntervals-1][i] = m_pMeasure[m_nTimeIntervals-1][i];
    for (int j=m_nTimeIntervals-2; j>=0; j--)
      m_pMeasure_NC[j][i] = m_pMeasure[j][i] - m_pMeasure[j+1][i];
  }
}

void CSaTScanData::SetNumTimeIntervals() {
  long nTime = TimeBetween(m_nStartDate, m_nEndDate, m_pParameters->GetTimeIntervalUnitsType());
  m_nTimeIntervals = (int)ceil((float)nTime / (float)m_pParameters->GetTimeIntervalLength());

  if (m_pParameters->GetTimeTrendAdjustmentType() == STRATIFIED_RANDOMIZATION && m_nTimeIntervals <= 1)
    SSGenerateException("Error: Time stratified randomization adjustment requires more than\n"
                        "       one time interval.\n", "SetNumTimeIntervals()");
}

/** allocates probability model */
/*void CSaTScanData::SetProbabilityModel() {
  try {
    switch (m_pParameters->GetProbabiltyModelType()) {
       case POISSON              : m_pModel = new CPoissonModel(*m_pParameters, *this, *gpPrint);   break;
       case BERNOULLI            : m_pModel = new CBernoulliModel(*m_pParameters, *this, *gpPrint); break;
       case SPACETIMEPERMUTATION : m_pModel = new CSpaceTimePermutationModel(*m_pParameters, *this, *gpPrint); break;
       default : ZdException::Generate("Unknown probability model type: '%d'.\n",
                                       "SetProbabilityModel()", m_pParameters->GetProbabiltyModelType());
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("SetProbabilityModel()","CSaTScanData");
    throw;
  }
} */

/* Calculates which time interval the prospectice space-time start date is in.*/
/* MAKE SURE THIS IS EXECUTED AFTER THE  m_nTimeIntervals VARIABLE HAS BEEN SET */
void CSaTScanData::SetProspectiveIntervalStart() {
  long    lTime;
  Julian  lProspStartDate;

  try {
    lProspStartDate = m_pParameters->GetProspectiveStartDateAsJulian();
    lTime = TimeBetween(lProspStartDate, m_nEndDate, m_pParameters->GetTimeIntervalUnitsType());
    m_nProspectiveIntervalStart = m_nTimeIntervals - (int)ceil((float)lTime/(float)m_pParameters->GetTimeIntervalLength()) + 1;

    if (m_nProspectiveIntervalStart < 0)
      SSGenerateException("Error: The prospective start date '%s' is prior to the study period start date '%s'.\n",
                          "SetProspectiveIntervalStart()", m_pParameters->GetProspectiveStartDate().c_str(),
                          m_pParameters->GetStudyPeriodStartDate().c_str());
    if (m_nProspectiveIntervalStart > m_nTimeIntervals)
      SSGenerateException("Error: The prospective start date '%s' occurs after the study period end date '%s'.\n",
                          "SetProspectiveIntervalStart", m_pParameters->GetProspectiveStartDate().c_str(),
                          m_pParameters->GetStudyPeriodEndDate().c_str());
  }
  catch (ZdException &x) {
    x.AddCallpath("SetProspectiveIntervalStart()","CSaTScanData");
    throw;
  }
}

void CSaTScanData::SetPurelyTemporalCases() {
  int   i, j;

  try {
    m_pPTCases = (count_t*) Smalloc((m_nTimeIntervals+1)*sizeof(count_t), gpPrint);

    for (i=0; i < m_nTimeIntervals; ++i) {
       m_pPTCases[i] = 0;
       for (j=0; j < m_nTracts; ++j)
          m_pPTCases[i] += m_pCases[i][j];
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("SetPurelyTemporalCases()","CSaTScanData");
    throw;
  }
}

void CSaTScanData::SetPurelyTemporalMeasures() {
  int   i, j;

  try {
    m_pPTMeasure = (measure_t*)Smalloc((m_nTimeIntervals+1) * sizeof(measure_t), gpPrint);

    for (i=0; i < m_nTimeIntervals; ++i) {
       m_pPTMeasure[i] = 0;
       for (j=0; j < m_nTracts; ++j)
          m_pPTMeasure[i] += m_pMeasure[i][j];
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("SetPurelyTemporalMeasures()","CSaTScanData");
    throw;
  }
}

void CSaTScanData::SetPurelyTemporalSimCases() {
  int   i, j;

  try {
    for (i=0; i < m_nTimeIntervals; ++i) {
       m_pPTSimCases[i] = 0;
       for (j=0; j < m_nTracts; ++j)
          m_pPTSimCases[i] += m_pSimCases[i][j];
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("SetPurelyTemporalSimCases()","CSaTScanData");
    throw;
  }
}

void CSaTScanData::SetStartAndEndDates() {
  try {
    m_nStartDate = m_pParameters->GetStudyPeriodStartDateAsJulian();
    m_nEndDate   = m_pParameters->GetStudyPeriodEndDateAsJulian();
  }
  catch (ZdException &x) {
    x.AddCallpath("SetStartAndEndDates()","CSaTScanData");
    throw;
  }
}

/** Sets time interval end range index into interval start times array. */
void CSaTScanData::SetScanningWindowEndRangeIndex(Julian EndRangeDate, int & iEndRangeDateIndex) {
  int   i;

  //find index for end range end date, the interval where date fits
  iEndRangeDateIndex = -1;
  //special case for study period end date
  if (EndRangeDate == m_nEndDate)
    iEndRangeDateIndex = m_nTimeIntervals;
  for (i=m_nTimeIntervals; i > 0  && iEndRangeDateIndex == -1; --i)
     if (EndRangeDate < m_pIntervalStartTimes[i] && EndRangeDate >= m_pIntervalStartTimes[i - 1])
        iEndRangeDateIndex = i - 1;
}

/** Sets time interval start range index into interval start times array. */
void CSaTScanData::SetScanningWindowStartRangeIndex(Julian StartRangeDate, int & iStartRangeDateIndex) {
  int   i;

  iStartRangeDateIndex = -1;
  for (i=0; i < m_nTimeIntervals && iStartRangeDateIndex == -1; ++i)
     if (StartRangeDate >= m_pIntervalStartTimes[i] && StartRangeDate < m_pIntervalStartTimes[i + 1])
       iStartRangeDateIndex = (StartRangeDate == m_pIntervalStartTimes[i] ? i : i + 1);
}

/** Sets indexes of time interval ranges into interval start time array. */
void CSaTScanData::SetTimeIntervalRangeIndexes() {
  ZdString      sTimeIntervalType;
  char          sDate[50], sDate2[50];
  int           iMaxEndWindow, iWindowStart;

  if (m_pParameters->GetIncludeClustersType() == CLUSTERSINRANGE) {
    //find start range date indexes
    SetScanningWindowStartRangeIndex(m_pParameters->GetStartRangeDateAsJulian(m_pParameters->GetStartRangeStartDate()), m_nStartRangeStartDateIndex);
    SetScanningWindowStartRangeIndex(m_pParameters->GetStartRangeDateAsJulian(m_pParameters->GetStartRangeEndDate()), m_nStartRangeEndDateIndex);
    //find end range date indexes
    SetScanningWindowEndRangeIndex(m_pParameters->GetEndRangeDateAsJulian(m_pParameters->GetEndRangeStartDate()), m_nEndRangeStartDateIndex);
    SetScanningWindowEndRangeIndex(m_pParameters->GetEndRangeDateAsJulian(m_pParameters->GetEndRangeEndDate()), m_nEndRangeEndDateIndex);
    //validate windows will be evaluated
    //check that there will be clusters evaluated...
    iMaxEndWindow = std::min(m_nEndRangeEndDateIndex, m_nStartRangeEndDateIndex + m_nIntervalCut);
    iWindowStart = std::max(m_nEndRangeStartDateIndex - m_nIntervalCut, m_nStartRangeStartDateIndex);
    if (iWindowStart >= iMaxEndWindow) {
      sTimeIntervalType = m_pParameters->GetDatePrecisionAsString(m_pParameters->GetTimeIntervalUnitsType());
      sTimeIntervalType.ToLowercase();
      SSGenerateException("Error: No clusters will be evaluated.\n"
        "       Although settings indicate a scanning window range of %s-%s to %s-%s,\n"
        "       the incorporation of the maximum temporal cluster size of %i %s causes the maximum window end time\n"
        "       to become %s (%s plus %i %s) and the window start time to become %s\n"
        "       (%s minus %i %s) which results in an invalid scanning window.\n", "Setup()",
        m_pParameters->GetStartRangeStartDate().c_str(),
        m_pParameters->GetStartRangeEndDate().c_str(),
        m_pParameters->GetEndRangeStartDate().c_str(),
        m_pParameters->GetEndRangeEndDate().c_str(),
        m_nIntervalCut, sTimeIntervalType.GetCString(),
        JulianToChar(sDate, m_pIntervalStartTimes[iMaxEndWindow]),
        m_pParameters->GetStartRangeEndDate().c_str(),
        m_nIntervalCut, sTimeIntervalType.GetCString(),
        JulianToChar(sDate2, m_pIntervalStartTimes[iWindowStart]),
        m_pParameters->GetEndRangeStartDate().c_str(),
        m_nIntervalCut, sTimeIntervalType.GetCString());
    }
    //The parameter validation checked already whether the end range dates conflicted,
    //but the maxium temporal cluster size may actually cause the range dates to be
    //different than the user defined.
    if (m_nEndRangeStartDateIndex > iMaxEndWindow) {
      sTimeIntervalType = m_pParameters->GetDatePrecisionAsString(m_pParameters->GetTimeIntervalUnitsType());
      sTimeIntervalType.ToLowercase();
      SSGenerateException("Error: No clusters will be evaluated.\n"
        "       Although settings indicate a scanning window range of %s-%s to %s-%s,\n"
        "       the incorporation of the maximum temporal cluster size of %i %s causes the maximum window end time\n"
        "       to become %s (%s plus %i %s), which does not intersect with requested scanning\n"
        "       window end range.\n","Setup()",
        m_pParameters->GetStartRangeStartDate().c_str(),
        m_pParameters->GetStartRangeEndDate().c_str(),
        m_pParameters->GetEndRangeStartDate().c_str(),
        m_pParameters->GetEndRangeEndDate().c_str(),
        m_nIntervalCut, sTimeIntervalType.GetCString(),
        JulianToChar(sDate, m_pIntervalStartTimes[iMaxEndWindow]),
        m_pParameters->GetStartRangeEndDate().c_str(),
        m_nIntervalCut, sTimeIntervalType.GetCString());
    }
  }
}

/** internal setup function */
void CSaTScanData::Setup(CParameters* pParameters, BasePrint *pPrintDirection) {
  long lCurrentEllipse = 0;

  try {
    gpPrint = pPrintDirection;
    m_pParameters = pParameters;
    m_nNumEllipsoids = pParameters->GetNumRequestedEllipses();

    gpTInfo = new TractHandler(gPopulationCategories, *pPrintDirection); 
    gpGInfo = new GInfo(pPrintDirection);
    //SetProbabilityModel();
    //For now, compute the angle and store the angle and shape
    //for each ellipsoid.  Maybe transfer info to a different location in the
    //application or compute "on the fly" prior to printing.
    if (m_pParameters->GetNumTotalEllipses() > 0) {
      mdE_Angles = new double[m_pParameters->GetNumTotalEllipses()];
      mdE_Shapes = new double[m_pParameters->GetNumTotalEllipses()];
      for (int es = 0; es < m_nNumEllipsoids; ++es) {
         for (int ea = 0; ea < m_pParameters->GetEllipseRotations()[es]; ++ea) {
            mdE_Angles[lCurrentEllipse]=PI*ea/m_pParameters->GetEllipseRotations()[es];
            mdE_Shapes[lCurrentEllipse]= m_pParameters->GetEllipseShapes()[es];
            ++lCurrentEllipse;
         }
      }
    }
  }
  catch (ZdException &x) {
    delete gpTInfo;
    delete gpGInfo;
    delete m_pModel;
    delete mdE_Angles;
    delete mdE_Shapes;
    x.AddCallpath("Setup()","CSaTScanData");
    throw;
  }
}

