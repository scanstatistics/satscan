#include "SaTScan.h"
#pragma hdrstop
#include "SaTScanData.h"
#include "PoissonModel.h"
#include "BernoulliModel.h"
#include "SpaceTimePermutationModel.h"

CSaTScanData::CSaTScanData(CParameters* pParameters, BasePrint *pPrintDirection) {
   long lCurrentEllipse = 0;

   try {
      gpPrint = pPrintDirection;
      m_pParameters = pParameters;
      m_nNumEllipsoids = pParameters->GetNumRequestedEllipses();

      Init();

      gpTInfo = new TractHandler(gPopulationCategories, *pPrintDirection);  // DTG
      gpGInfo = new GInfo(pPrintDirection);          // DTG

      switch (m_pParameters->GetProbabiltyModelType()) {
        case POISSON              : m_pModel = new CPoissonModel(*pParameters, *this, *pPrintDirection);   break;
        case BERNOULLI            : m_pModel = new CBernoulliModel(*pParameters, *this, *pPrintDirection); break;
        case SPACETIMEPERMUTATION : m_pModel = new CSpaceTimePermutationModel(*pParameters, *this, *pPrintDirection); break;
        default : ZdException::Generate("Unknown probability model type: '%d'.\n","CSaTScanData()", m_pParameters->GetProbabiltyModelType());
      }

      //For now, compute the angle and store the angle and shape
      //for each ellipsoid.  Maybe transfer info to a different location in the
      //application or compute "on the fly" prior to printing.
      if (m_pParameters->GetNumTotalEllipses() > 0) {
         mdE_Angles = new double[m_pParameters->GetNumTotalEllipses()];
         mdE_Shapes = new double[m_pParameters->GetNumTotalEllipses()];
         for (int es = 0; es < m_nNumEllipsoids; ++es) {
            for(int ea = 0; ea < m_pParameters->GetEllipseRotations()[es]; ++ea) {
               mdE_Angles[lCurrentEllipse]=PI*ea/m_pParameters->GetEllipseRotations()[es];
               mdE_Shapes[lCurrentEllipse]= m_pParameters->GetEllipseShapes()[es];
               ++lCurrentEllipse;
            }
         }
      }
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
      bReturn = (m_pModel->CalculateMeasure());

      m_nTotalTractsAtStart   = m_nTracts;
      m_nTotalCasesAtStart    = m_nTotalCases;
      m_nTotalControlsAtStart = m_nTotalControls;
      m_nTotalMeasureAtStart  = m_nTotalMeasure;

      SetMaxCircleSize();
   }
   catch (ZdException & x) {
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
//         if (m_pParameters->GetMaximumTemporalClusterSizeType() == PERCENTAGETYPE)
           lTimeBetween = (TimeBetween(jStartDate, jEndDate, m_pParameters->GetTimeIntervalUnitsType()))*m_pParameters->GetMaximumTemporalClusterSize()/100.0;
//         else
//           lTimeBetween = (TimeBetween(jStartDate, jEndDate, m_pParameters->GetTimeIntervalUnitsType()))*
//                          (m_pParameters->GetMaximumTemporalClusterSize()/TimeBetween(m_nStartDate, m_nEndDate, m_pParameters->GetTimeIntervalUnitsType()));
//
         iIntervalCut = lTimeBetween / m_pParameters->GetTimeIntervalLength();
      }
   }
   catch (ZdException & x) {
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
   long lTotalNumEllipses = m_pParameters->GetNumTotalEllipses();

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

      if (m_pParameters->GetIsSequentialScanning())
        MakeNeighbors(gpTInfo, gpGInfo, m_pSortedInt, m_pSortedUShort, m_nTracts, m_nGridTracts,
                      m_pMeasure[0], m_nMaxCircleSize, m_nTotalMeasure, m_NeighborCounts,
                      m_pParameters->GetDimensionsOfData(), m_pParameters->GetNumRequestedEllipses(),
                      m_pParameters->GetEllipseShapes(), m_pParameters->GetEllipseRotations(),
                      m_pParameters->GetMaxGeographicClusterSizeType(), gpPrint);
      else
        MakeNeighbors(gpTInfo, gpGInfo, m_pSortedInt, m_pSortedUShort, m_nTracts, m_nGridTracts,
                      m_pMeasure[0], m_nMaxCircleSize, m_nMaxCircleSize, m_NeighborCounts,
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

    if (m_pParameters->GetAnalysisType() == PROSPECTIVESPACETIME)
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

/* Calculates the number of time intervals to include in potential clusters
   without exceeding the maximum cluster size with respect to time.*/
void CSaTScanData::SetIntervalCut() {
  ZdString      sIntervalCutMessage, sTimeIntervalType;
  long          lMaxTemporalLength, lStudyPeriodLength;

  try {
    if (m_nTimeIntervals == 1)
      m_nIntervalCut = 1;
    else if (m_nTimeIntervals > 1) {
      lStudyPeriodLength = TimeBetween(m_nStartDate, m_nEndDate, m_pParameters->GetTimeIntervalUnitsType());
      lMaxTemporalLength = lStudyPeriodLength * m_pParameters->GetMaximumTemporalClusterSize()/100.0;
      m_nIntervalCut = lMaxTemporalLength / m_pParameters->GetTimeIntervalLength();
    }

    if (m_nIntervalCut==0) {
      switch (m_pParameters->GetTimeIntervalUnitsType()) {
          case YEAR  : sTimeIntervalType = "year"; break;
          case MONTH : sTimeIntervalType = "month"; break;
          case DAY   : sTimeIntervalType = "day"; break;
          // if we get here, there is an error somewhere else
          default: sTimeIntervalType = "none"; break;
        };

        if (m_pParameters->GetInitialMaxTemporalClusterSizeType() == TIMETYPE ) {
          sIntervalCutMessage << "Error: A maximum temporal cluster size of %g %s%s is less than one %d %s time interval.\n";
          sIntervalCutMessage << "       No clusters can be found.\n";
          SSGenerateException(sIntervalCutMessage.GetCString(), "SetIntervalCut()",
                              m_pParameters->GetInitialMaxTemporalClusterSize(), sTimeIntervalType.GetCString(),
                              (m_pParameters->GetInitialMaxTemporalClusterSize() == 1 ? "" : "s"),
                              m_pParameters->GetTimeIntervalLength(), sTimeIntervalType.GetCString());
        }
        else if (m_pParameters->GetInitialMaxTemporalClusterSizeType() == PERCENTAGETYPE) {
          sIntervalCutMessage << "Error: A maximum temporal cluster size that is %g percent of a %d %s study period \n";
          sIntervalCutMessage << "       equates to %d %s%s, which is less than one %d %s time interval.\n";
          sIntervalCutMessage << "       No clusters can be found.\n";
          SSGenerateException(sIntervalCutMessage.GetCString(), "SetIntervalCut()",
                              m_pParameters->GetInitialMaxTemporalClusterSize(),
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

void CSaTScanData::SetNumTimeIntervals() {
  long nTime = TimeBetween(m_nStartDate, m_nEndDate, m_pParameters->GetTimeIntervalUnitsType());
  m_nTimeIntervals = (int)ceil((float)nTime / (float)m_pParameters->GetTimeIntervalLength());
}

/* Calculates which time interval the prospectice space-time start date is in.*/
/* MAKE SURE THIS IS EXECUTED AFTER THE  m_nTimeIntervals VARIABLE HAS BEEN SET */
void CSaTScanData::SetProspectiveIntervalStart() {
  long    lTime;
  Julian  lProspStartDate;

  try {
    lProspStartDate = m_pParameters->GetProspectiveStartDateAsJulian();
    lTime = TimeBetween(m_nStartDate, lProspStartDate, m_pParameters->GetTimeIntervalUnitsType());
    m_nProspectiveIntervalStart = (int)ceil((float)lTime / (float)m_pParameters->GetTimeIntervalLength());

    if (m_nProspectiveIntervalStart < 0)
      SSGenerateException("Error: The prospective start date '%s' is prior to the study period start date '%s'.\n",
                          "SetProspectiveIntervalStart()", m_pParameters->GetProspectiveStartDate().c_str(),
                          m_pParameters->GetStudyPeriodStartDate().c_str());
    if (m_nProspectiveIntervalStart > m_nTimeIntervals)
      SSGenerateException("Error: The prospective start date '%s' is prior to the study period end date '%s'.\n",
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



