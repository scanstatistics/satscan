#include "SaTScan.h"
#pragma hdrstop
#include "SaTScanData.h"
#include "TimeIntervalRange.h"

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
  try {
    delete m_pModel;
    delete gpCasesHandler; gpCasesHandler=0;
    delete gpCasesNonCumulativeHandler; gpCasesNonCumulativeHandler=0;
    delete gpControlsHandler; gpControlsHandler=0;
    delete gpSimCasesHandler; gpSimCasesHandler=0;
    delete gpSimCasesNonCumulativeHandler; gpSimCasesNonCumulativeHandler=0;
    delete gpNeighborCountHandler; gpNeighborCountHandler=0;
    delete gpSortedIntHandler; gpSortedIntHandler=0;
    delete gpSortedUShortHandler; gpSortedUShortHandler=0;
    delete gpCategoryCasesHandler; gpCategoryCasesHandler=0;
    delete gpMeasureHandler; gpMeasureHandler=0;
    delete gpMeasureNonCumulativeHandler; gpMeasureNonCumulativeHandler=0;
    delete gpCasesByTimeByCategoryHandler; gpCasesByTimeByCategoryHandler=0;
    delete gpMeasureByTimeByCategoryHandler; gpMeasureByTimeByCategoryHandler=0;
    delete gpCategoryMeasureHandler; gpCategoryMeasureHandler=0;
    delete gpControlsByTimeByCategoryHandler; gpControlsByTimeByCategoryHandler=0;
    delete gpCategoryControlsHandler; gpCategoryControlsHandler=0;
    free(m_pPTCases);
    free(m_pPTMeasure);
    free(m_pSimCases_TotalByTimeInt);
    free(m_pCases_TotalByTimeInt);
    free(m_pMeasure_TotalByTimeInt);
    delete [] mdE_Angles;
    delete [] mdE_Shapes;
    free(m_pIntervalStartTimes);
    delete gpTInfo;
    delete gpGInfo;
    delete gpMaxWindowLengthIndicator; gpMaxWindowLengthIndicator=0;
  }
  catch (...){}  
}

/** Sequential analyses will call this function to clear neighbor information and
    re-calculate neighbors. Note that only when the maximum spatial cluster size
    is specified as a percentage of the population that this operation need be
    performed between iterations of a sequential scan. */
void CSaTScanData::AdjustNeighborCounts() {
  try {
    //Deallocate neighbor information in sorted structures.
    if (m_pParameters->GetMaxGeographicClusterSizeType() == PERCENTAGEOFMEASURETYPE) {
      //Free/clear previous interation's neighbor information.
      if (gpSortedUShortHandler)
        gpSortedUShortHandler->FreeThirdDimension();
      else
        gpSortedIntHandler->FreeThirdDimension();
      gpNeighborCountHandler->Set(0);
 
      //Recompute neighbors.
      MakeNeighbors(gpTInfo, gpGInfo, (gpSortedIntHandler ? gpSortedIntHandler->GetArray() : 0),
                    (gpSortedUShortHandler ? gpSortedUShortHandler->GetArray() : 0),
                    static_cast<tract_t>(m_nTotalTractsAtStart), m_nGridTracts,
                    gpMeasureHandler->GetArray()[0], m_nMaxCircleSize, m_nMaxCircleSize,
                    gpNeighborCountHandler->GetArray(), m_pParameters->GetDimensionsOfData(),
                    m_pParameters->GetNumRequestedEllipses(), m_pParameters->GetEllipseShapes(),
                    m_pParameters->GetEllipseRotations(), m_pParameters->GetMaxGeographicClusterSizeType(), gpPrint);
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("AdjustNeighborCounts()", "CSaTScanData");
    throw;
  }
}

/** allocates two-dimensional array that will track the number of neighbors
    for each shape/grid point combination. */
void CSaTScanData::AllocateNeighborArray() {
  try {
    gpNeighborCountHandler = new TwoDimensionArrayHandler<tract_t>(m_pParameters->GetNumTotalEllipses() + 1, m_nGridTracts, 0);
  }
  catch (ZdException &x) {
    x.AddCallpath("AllocateNeighborArray()","CSaTScanData");
    throw;
  }
}

/** Allocates multi-dimensional array that stores tract index for each neighbor
    of each ellipse/grid point combination. */
void CSaTScanData::AllocateSortedArray() {
  try {
    if (m_nTracts < std::numeric_limits<unsigned short>::max())
      gpSortedUShortHandler = new ThreeDimensionArrayHandler<unsigned short>(m_pParameters->GetNumTotalEllipses()+1, m_nGridTracts, 0);
    else
      gpSortedIntHandler = new ThreeDimensionArrayHandler<tract_t>(m_pParameters->GetNumTotalEllipses()+1, m_nGridTracts, 0);
  }
  catch (ZdException &x) {
    delete gpSortedUShortHandler; gpSortedUShortHandler=0;
    delete gpSortedIntHandler; gpSortedIntHandler=0;
    x.AddCallpath("AllocateSortedArray()","CSaTScanData");
    throw;
  }
}

void CSaTScanData::AllocSimCases() {
  try {
    gpSimCasesHandler = new TwoDimensionArrayHandler<count_t>(m_nTimeIntervals, m_nTracts, 0);
  }
  catch (ZdException &x) {
    x.AddCallpath("GetNeighbor()", "CSaTScanData");
    throw;
  }
}

bool CSaTScanData::CalculateMeasure() {
  bool bReturn;

  try {
    gpPrint->SatScanPrintf("Calculating expected number of cases\n");  
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

int CSaTScanData::ComputeNewCutoffInterval(Julian jStartDate, Julian jEndDate) {
   int  iIntervalCut;
   long lTimeBetween;

   if (m_pParameters->GetMaximumTemporalClusterSizeType() == PERCENTAGETYPE) {
     lTimeBetween = static_cast<long>(floor((TimeBetween(jStartDate, jEndDate, m_pParameters->GetTimeIntervalUnitsType()))*m_pParameters->GetMaximumTemporalClusterSize()/100.0));
     iIntervalCut = static_cast<int>(floor(lTimeBetween / m_pParameters->GetTimeIntervalLength()));
   }
   else
     iIntervalCut = m_nIntervalCut;

   return iIntervalCut;
}

void CSaTScanData::DeAllocSimCases() {
  delete gpSimCasesHandler; gpSimCasesHandler=0;
}

/** Allocates/deallocates memory to store neighbor information.
    Calls MakeNeighbor() function to calculate neighbors for each centroid. */
bool CSaTScanData::FindNeighbors(bool bSimulations) {
  int           i, j;
  double        dMaxCircleSize, dTotalPopulation=0;

  try {
    //if this iteration of call not simulations
    if (! bSimulations) {
      AllocateSortedArray();
      AllocateNeighborArray();
      //adjust special population file now that we know the total case count
      if (m_pParameters->UseMaxCirclePopulationFile()) {
        for (i=0; i < (int)gvCircleMeasure.size(); i++)
           dTotalPopulation += gvCircleMeasure[i];
        if (dTotalPopulation ==0)
          SSGenerateException("Error: Total population for special population file can not be zero.","FindNeighbors()");
        for (i=0; i < (int)gvCircleMeasure.size(); i++)
          gvCircleMeasure[i] *= m_nTotalCases/dTotalPopulation;
      }
      //for real data, settings my indicate to report only smaller clusters
      dMaxCircleSize = (m_pParameters->GetRestrictingMaximumReportedGeoClusterSize() ? m_nMaxReportedCircleSize : m_nMaxCircleSize);
    }
    else {
      //when this functions is called for simualtions, we need to deallocate memory that
      //will be allocated once again in MakeNeighbors()
      if (gpSortedIntHandler)
        gpSortedIntHandler->FreeThirdDimension();
      else
        gpSortedUShortHandler->FreeThirdDimension();
      gpNeighborCountHandler->Set(0);
      dMaxCircleSize = m_nMaxCircleSize;
    }  

    if (m_pParameters->GetIsSequentialScanning())
        MakeNeighbors(gpTInfo, gpGInfo, (gpSortedIntHandler ? gpSortedIntHandler->GetArray() : 0),
                      (gpSortedUShortHandler ? gpSortedUShortHandler->GetArray() : 0), m_nTracts, m_nGridTracts,
                      (m_pParameters->UseMaxCirclePopulationFile() ? &gvCircleMeasure[0] : gpMeasureHandler->GetArray()[0]),
                      dMaxCircleSize, m_nTotalMeasure, gpNeighborCountHandler->GetArray(),
                      m_pParameters->GetDimensionsOfData(), m_pParameters->GetNumRequestedEllipses(),
                      m_pParameters->GetEllipseShapes(), m_pParameters->GetEllipseRotations(),
                      m_pParameters->GetMaxGeographicClusterSizeType(), gpPrint);
    else
        MakeNeighbors(gpTInfo, gpGInfo, (gpSortedIntHandler ? gpSortedIntHandler->GetArray() : 0),
                      (gpSortedUShortHandler ? gpSortedUShortHandler->GetArray() : 0), m_nTracts, m_nGridTracts,
                      (m_pParameters->UseMaxCirclePopulationFile() ? &gvCircleMeasure[0] : gpMeasureHandler->GetArray()[0]),
                      dMaxCircleSize, dMaxCircleSize, gpNeighborCountHandler->GetArray(),
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
   if (gpSortedUShortHandler)
      return (tract_t)gpSortedUShortHandler->GetArray()[iEllipse][t][nearness - 1];
   else
      return gpSortedIntHandler->GetArray()[iEllipse][t][nearness - 1];
}

/** Returns total population count */
double CSaTScanData::GetTotalPopulationCount() const {
  return m_nTotalPop;
}

void CSaTScanData::Init() {
  gpTInfo = 0;
  gpGInfo = 0;
  m_pModel = 0;
  gpCasesHandler=0;
  gpCasesNonCumulativeHandler=0;
  gpControlsHandler=0;
  gpSimCasesHandler=0;
  gpSimCasesNonCumulativeHandler=0;
  gpNeighborCountHandler=0;
  gpSortedIntHandler=0;
  gpSortedUShortHandler=0;
  gpCategoryCasesHandler=0;
  gpCasesByTimeByCategoryHandler=0;
  gpMeasureHandler=0;
  gpMeasureNonCumulativeHandler=0;
  gpMeasureByTimeByCategoryHandler=0;
  gpCategoryMeasureHandler=0;
  gpControlsByTimeByCategoryHandler=0;
  gpCategoryControlsHandler=0;
  m_pPTCases   = 0;
  m_pPTSimCases = 0;
  m_pPTMeasure = 0;
  m_pIntervalStartTimes = 0;
  m_nTotalCases    = 0;
  m_nTotalControls = 0;
  m_nTotalMeasure  = 0;
  m_nAnnualRatePop = 100000;
  mdE_Angles = 0;
  mdE_Shapes = 0;
  m_pCases_TotalByTimeInt = 0;
  m_pSimCases_TotalByTimeInt = 0;
  m_pMeasure_TotalByTimeInt = 0;
  m_nMaxReportedCircleSize = 0;
  gpMaxWindowLengthIndicator = 0;
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

void CSaTScanData::ReadDataFromFiles() {
  try {
    SetStartAndEndDates();
    SetNumTimeIntervals();
    SetIntervalCut();
    SetIntervalStartTimes();
    SetTimeIntervalRangeIndexes();
    if (m_pParameters->GetIsProspectiveAnalysis())
      SetProspectiveIntervalStart();
    SetMaxTemporalWindowLengthIndicator();
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

void CSaTScanData::RemoveTractSignificance(tract_t tTractIndex) {
  m_nTotalCases -= gpCasesHandler->GetArray()[0][tTractIndex];
  gpCasesHandler->GetArray()[0][tTractIndex] = 0;
  m_nTotalMeasure -= gpMeasureHandler->GetArray()[0][tTractIndex];
  gpMeasureHandler->GetArray()[0][tTractIndex] = 0;
  if (gpControlsHandler) {
     m_nTotalControls -= gpControlsHandler->GetArray()[0][tTractIndex];
     gpControlsHandler->GetArray()[0][tTractIndex] = 0;
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
  int                   i, j;
  count_t            ** ppCases(gpCasesHandler->GetArray());

  try {
    m_pCases_TotalByTimeInt = (count_t*)Smalloc(m_nTimeIntervals * sizeof(count_t), gpPrint);
    memset(m_pCases_TotalByTimeInt, 0, m_nTimeIntervals * sizeof(count_t));

    for (i=0; i < m_nTracts; i++) {
       m_pCases_TotalByTimeInt[m_nTimeIntervals-1] += ppCases[m_nTimeIntervals-1][i];
       for (j=m_nTimeIntervals-2; j >= 0; j--)
          m_pCases_TotalByTimeInt[j] += ppCases[j][i] - ppCases[j+1][i];
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
  int           i, tract;
  measure_t  ** ppMeasure, ** ppMeasureNC(gpMeasureNonCumulativeHandler->GetArray());

  try {
    if (gpMeasureHandler)
      ZdGenerateException("Error: Cumulative measure already allocated.\n", "SetCumulativeMeasure()");

    if (gpMeasureNonCumulativeHandler == 0)
      ZdGenerateException("Error: Non-cumulative measure is not allocated.\n", "SetCumulativeMeasure()");

    /*note: measure allocated in CModel::CalculateMeasure() to m_nTimeIntervals + 1,
            not sure why but allocate this array the same way for deallocation,
            ppMeasure could have been the array that as passed to
            CModel::CalculateMeasure() instead of ppMeasureNC. */
    gpMeasureHandler = new TwoDimensionArrayHandler<measure_t>(m_nTimeIntervals+1, m_nTracts);
    ppMeasure = gpMeasureHandler->GetArray();
    for (tract=0; tract < m_nTracts; tract++) {
       ppMeasure[m_nTimeIntervals-1][tract] = ppMeasureNC[m_nTimeIntervals-1][tract];
       for (i=m_nTimeIntervals-2; i >= 0; i--)
          ppMeasure[i][tract] = ppMeasure[i+1][tract] + ppMeasureNC[i][tract];
    }

    // Bug check, to ensure that TotalCases=TotalMeasure
    if (fabs(m_nTotalCases-m_nTotalMeasure)>0.0001)
      ZdGenerateException("Error: The total measure '%8.6lf' is not equal to the total number of cases '%ld'.\n",
                          "SetCumulativeMeasure()", m_nTotalMeasure, m_nTotalCases);
  }
  catch (ZdException &x) {
    delete gpMeasureHandler; gpMeasureHandler=0;
    x.AddCallpath("SetCumulativeMeasure()","CSaTScanData");
    throw;
  }
}

/** Creates max window length object which dictates maximum temporal window
    length during analysis simluations. */
void CSaTScanData::SetMaxTemporalWindowLengthIndicator() {
  try {
    if (m_pParameters->GetNumReplicationsRequested() > 0) { //no needed if no simulations performed
      if (m_pParameters->GetIsProspectiveAnalysis() && m_pParameters->GetMaximumTemporalClusterSizeType() == PERCENTAGETYPE)
        gpMaxWindowLengthIndicator = new ProspectiveMaxWindowLengthIndicator(*this);
      else
        gpMaxWindowLengthIndicator = new FixedMaxWindowLengthIndicator(*this);
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("SetMaxTemporalWindowLengthIndicator()","CSaTScanData");
    throw;
  }
}

/** Allocates and sets measure array that represents non-cumulative measure
    for all time intervals from cumulative measure array. */
void CSaTScanData::SetMeasureByTimeIntervalArray() {
  int           i, j;
  measure_t  ** ppMeasure(gpMeasureHandler->GetArray());

  try {
    if (! m_pMeasure_TotalByTimeInt)
       m_pMeasure_TotalByTimeInt = (measure_t*)Smalloc(m_nTimeIntervals * sizeof(measure_t), gpPrint);

    memset(m_pMeasure_TotalByTimeInt, 0, m_nTimeIntervals * sizeof(measure_t));
    for (i=0; i < m_nTracts; i++) {
       m_pMeasure_TotalByTimeInt[m_nTimeIntervals-1] += ppMeasure[m_nTimeIntervals-1][i];
       for (j=m_nTimeIntervals-2; j >= 0; j--)
          m_pMeasure_TotalByTimeInt[j] += ppMeasure[j][i] - ppMeasure[j+1][i];
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
        lMaxTemporalLength = static_cast<long>(floor(lStudyPeriodLength * m_pParameters->GetMaximumTemporalClusterSize()/100.0));
        m_nIntervalCut = static_cast<int>(floor(lMaxTemporalLength / m_pParameters->GetTimeIntervalLength()));
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
           if (m_pParameters->GetRestrictingMaximumReportedGeoClusterSize())
             m_nMaxReportedCircleSize = (m_pParameters->GetMaximumReportedGeoClusterSize() / 100.0) * m_nTotalMeasure;
           break;
      case DISTANCETYPE :
           m_nMaxCircleSize = m_pParameters->GetMaximumGeographicClusterSize();
           if (m_pParameters->GetRestrictingMaximumReportedGeoClusterSize())
             m_nMaxReportedCircleSize = m_pParameters->GetMaximumReportedGeoClusterSize();
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
  int           i, j;
  measure_t  ** ppMeasureNC, ** ppMeasure(gpMeasureHandler->GetArray());

  try {
    if (gpMeasureNonCumulativeHandler)
      ZdException::Generate("Non-cumulative measure array already allocated.\n","SetNonCumulativeMeasure()");

    /*note: measure allocated in CModel::CalculateMeasure() to m_nTimeIntervals + 1,
            not sure why but allocate this array the same way for deallocation,
            ppMeasureNC could have been the array that as passed to
            CModel::CalculateMeasure() instead of m_pMeasire. */
    gpMeasureNonCumulativeHandler = new TwoDimensionArrayHandler<measure_t>(m_nTimeIntervals+1, m_nTracts);
    ppMeasureNC = gpMeasureNonCumulativeHandler->GetArray();
    for (i=0; i < m_nTracts; i++) {
      ppMeasureNC[m_nTimeIntervals-1][i] = ppMeasure[m_nTimeIntervals-1][i];
      for (j=m_nTimeIntervals-2; j>=0; j--)
        ppMeasureNC[j][i] = ppMeasure[j][i] - ppMeasure[j+1][i];
    }
  }
  catch (ZdException &x) {
    delete gpMeasureNonCumulativeHandler; gpMeasureNonCumulativeHandler=0;
    x.AddCallpath("SetNonCumulativeMeasure()","CSaTScanData");
    throw;
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
  int                   i, j;
  count_t            ** ppCases(gpCasesHandler->GetArray());

  try {
    m_pPTCases = (count_t*) Smalloc((m_nTimeIntervals+1)*sizeof(count_t), gpPrint);

    for (i=0; i < m_nTimeIntervals; ++i) {
       m_pPTCases[i] = 0;
       for (j=0; j < m_nTracts; ++j)
          m_pPTCases[i] += ppCases[i][j];
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("SetPurelyTemporalCases()","CSaTScanData");
    throw;
  }
}

void CSaTScanData::SetPurelyTemporalMeasures() {
  int           i, j;
  measure_t  ** ppMeasure(gpMeasureHandler->GetArray());

  try {
    m_pPTMeasure = (measure_t*)Smalloc((m_nTimeIntervals+1) * sizeof(measure_t), gpPrint);
    for (i=0; i < m_nTimeIntervals; ++i) {
       m_pPTMeasure[i] = 0;
       for (j=0; j < m_nTracts; ++j)
          m_pPTMeasure[i] += ppMeasure[i][j];
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("SetPurelyTemporalMeasures()","CSaTScanData");
    throw;
  }
}

void CSaTScanData::SetPurelyTemporalSimCases() {
  int                   i, j;
  count_t            ** ppSimCases(gpSimCasesHandler->GetArray());

  try {
    for (i=0; i < m_nTimeIntervals; ++i) {
       m_pPTSimCases[i] = 0;
       for (j=0; j < m_nTracts; ++j)
          m_pPTSimCases[i] += ppSimCases[i][j];
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

  //find index for end range end date, the interval where date fits, rounding to higher index
  iEndRangeDateIndex = -1;
  for (i=m_nTimeIntervals; i > 0  && iEndRangeDateIndex == -1; --i)
     if (EndRangeDate <= m_pIntervalStartTimes[i] - 1 && EndRangeDate > m_pIntervalStartTimes[i - 1] - 1)
        iEndRangeDateIndex = i;
}

/** Sets time interval start range index into interval start times array. */
void CSaTScanData::SetScanningWindowStartRangeIndex(Julian StartRangeDate, int & iStartRangeDateIndex) {
  int   i;

  iStartRangeDateIndex = -1;
  for (i=0; i < m_nTimeIntervals && iStartRangeDateIndex == -1; ++i)
     if (StartRangeDate >= m_pIntervalStartTimes[i] && StartRangeDate < m_pIntervalStartTimes[i + 1])
       iStartRangeDateIndex = i;
}

/** Sets indexes of time interval ranges into interval start time array. */
void CSaTScanData::SetTimeIntervalRangeIndexes() {
  ZdString      sTimeIntervalType, sMessage;
  char          sDateSSR[50], sDateESR[50], sDateSER[50], sDateEER[50], sDateWST[50], sDateMaxWET[50]; 
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
      JulianToChar(sDateSSR, m_pIntervalStartTimes[m_nStartRangeStartDateIndex]);
      JulianToChar(sDateESR, m_pIntervalStartTimes[m_nStartRangeEndDateIndex]);
      JulianToChar(sDateSER, m_pIntervalStartTimes[m_nEndRangeStartDateIndex] - 1);
      JulianToChar(sDateEER, m_pIntervalStartTimes[m_nEndRangeEndDateIndex] - 1);
      JulianToChar(sDateWST, m_pIntervalStartTimes[iWindowStart]);
      JulianToChar(sDateMaxWET, m_pIntervalStartTimes[iMaxEndWindow] - 1);
      SSGenerateException("Error: No clusters will be evaluated.\n"
                          "       Although settings indicate a scanning window range of %s-%s to %s-%s,\n"
                          "       the scanning window range becomes %s-%s to %s-%s when fitted to\n"
                          "       respective calculated intervals.\n\n"
                          "       With the incorporation of a maximum temporal cluster size of %i %s,\n"
                          "       the temporal window scanned has a start time of %s (%s minus %i %s)\n"
                          "       and a maximum window end time of %s (%s plus %i %s), which results\n"
                          "       in no windows scanned."
                          , "Setup()",
        m_pParameters->GetStartRangeStartDate().c_str(), m_pParameters->GetStartRangeEndDate().c_str(),
        m_pParameters->GetEndRangeStartDate().c_str(), m_pParameters->GetEndRangeEndDate().c_str(),
        sDateSSR, sDateESR, sDateSER, sDateEER, m_nIntervalCut * m_pParameters->GetTimeIntervalLength(),
        sTimeIntervalType.GetCString(), sDateWST, sDateSER,
        m_nIntervalCut * m_pParameters->GetTimeIntervalLength(), sTimeIntervalType.GetCString(),
        sDateMaxWET, sDateESR, m_nIntervalCut * m_pParameters->GetTimeIntervalLength(), sTimeIntervalType.GetCString());
    }
    //The parameter validation checked already whether the end range dates conflicted,
    //but the maxium temporal cluster size may actually cause the range dates to be
    //different than the user defined.
    if (m_nEndRangeStartDateIndex > iMaxEndWindow) {
      sTimeIntervalType = m_pParameters->GetDatePrecisionAsString(m_pParameters->GetTimeIntervalUnitsType());
      sTimeIntervalType.ToLowercase();
      JulianToChar(sDateSSR, m_pIntervalStartTimes[m_nStartRangeStartDateIndex]);
      JulianToChar(sDateESR, m_pIntervalStartTimes[m_nStartRangeEndDateIndex]);
      JulianToChar(sDateSER, m_pIntervalStartTimes[m_nEndRangeStartDateIndex] - 1);
      JulianToChar(sDateEER, m_pIntervalStartTimes[m_nEndRangeEndDateIndex] - 1);
      JulianToChar(sDateMaxWET, m_pIntervalStartTimes[iMaxEndWindow] - 1);
      SSGenerateException("Error: No clusters will be evaluated.\n"
                          "       Although settings indicate a scanning window range of %s-%s to %s-%s,\n"
                          "       the scanning window range becomes %s-%s to %s-%s when fitted to\n"
                          "       respective calculated intervals.\n\n"
                          "       With the incorporation of a maximum temporal cluster size of %i %s\n"
                          "       the maximum window end time becomes %s (%s plus %i %s),\n"
                          "       which does not intersect with scanning window end range.\n","Setup()",
        m_pParameters->GetStartRangeStartDate().c_str(), m_pParameters->GetStartRangeEndDate().c_str(),
        m_pParameters->GetEndRangeStartDate().c_str(), m_pParameters->GetEndRangeEndDate().c_str(),
        sDateSSR, sDateESR, sDateSER, sDateEER,
        m_nIntervalCut * m_pParameters->GetTimeIntervalLength(), sTimeIntervalType.GetCString(),
        sDateMaxWET, sDateESR, m_nIntervalCut * m_pParameters->GetTimeIntervalLength(), sTimeIntervalType.GetCString());
    } 
    //reset parameter settings
    JulianToChar(sDateSSR, m_pIntervalStartTimes[m_nStartRangeStartDateIndex]);
    m_pParameters->SetStartRangeStartDate(sDateSSR);
    JulianToChar(sDateESR, m_pIntervalStartTimes[m_nStartRangeEndDateIndex]);
    m_pParameters->SetStartRangeEndDate(sDateESR);
    JulianToChar(sDateSER, m_pIntervalStartTimes[m_nEndRangeStartDateIndex] - 1);
    m_pParameters->SetEndRangeStartDate(sDateSER);
    JulianToChar(sDateEER, m_pIntervalStartTimes[m_nEndRangeEndDateIndex] - 1);
    m_pParameters->SetEndRangeEndDate(sDateEER);
  }
}

/** internal setup function */
void CSaTScanData::Setup(CParameters* pParameters, BasePrint *pPrintDirection) {
  long  lCurrentEllipse=0;

  try {
    gpPrint = pPrintDirection;
    m_pParameters = pParameters;

    gpTInfo = new TractHandler(gPopulationCategories, *pPrintDirection); 
    gpGInfo = new GInfo(pPrintDirection);
    //SetProbabilityModel();
    //For now, compute the angle and store the angle and shape
    //for each ellipsoid.  Maybe transfer info to a different location in the
    //application or compute "on the fly" prior to printing.
    if (m_pParameters->GetNumTotalEllipses() > 0) {
      mdE_Angles = new double[m_pParameters->GetNumTotalEllipses()];
      mdE_Shapes = new double[m_pParameters->GetNumTotalEllipses()];
      for (int es = 0; es < pParameters->GetNumRequestedEllipses(); ++es) {
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
    delete mdE_Angles;
    delete mdE_Shapes;
    x.AddCallpath("Setup()","CSaTScanData");
    throw;
  }
}

