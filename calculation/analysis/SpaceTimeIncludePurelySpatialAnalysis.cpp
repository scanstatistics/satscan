#include "SaTScan.h"
#pragma hdrstop
#include "SpaceTimeIncludePurelySpatialAnalysis.h"

/** Constructor */
C_ST_PS_Analysis::C_ST_PS_Analysis(CParameters*  pParameters, CSaTScanData* pData, BasePrint *pPrintDirection)
                 :CSpaceTimeAnalysis(pParameters, pData, pPrintDirection) {
  try {
    Init();
    Setup();
  }
  catch (ZdException & x) {
    x.AddCallpath("constructor()", "C_ST_PS_Analysis");
    throw;
  }
}

/** Destructor */
C_ST_PS_Analysis::~C_ST_PS_Analysis() {
  try {
    delete gp_PS_TopShapeClusters;
    delete gp_ST_TopShapeClusters;
  }
  catch(...){}
}

/** Returns cluster centered at grid point nCenter, with the greatest loglikelihood.
    Caller is responsible for deleting returned cluster. */
CCluster* C_ST_PS_Analysis::GetTopCluster(tract_t nCenter) {
  int                           j;
  tract_t                       i;
  CCluster                    * pTopCluster;
  CPurelySpatialCluster       * C_PS_MAX=0;
  CSpaceTimeCluster           * C_ST_MAX=0;
  IncludeClustersType           eIncludeClustersType;

  try {
    // if Prospective Space-Time then Alive Clusters Only.
    if (m_pParameters->GetAnalysisType() == PROSPECTIVESPACETIME)
      eIncludeClustersType = ALIVECLUSTERS;
    else
      eIncludeClustersType = m_pParameters->GetIncludeClustersType();

    C_PS_MAX = new CPurelySpatialCluster(gpPrintDirection);
    C_PS_MAX->SetLogLikelihood(m_pData->m_pModel->GetLogLikelihoodForTotal());
    gp_PS_TopShapeClusters->SetTopClusters(*C_PS_MAX);
    C_ST_MAX = new CSpaceTimeCluster(eIncludeClustersType, m_pData->m_nTimeIntervals, m_pData->m_nIntervalCut, gpPrintDirection);
    C_ST_MAX->SetLogLikelihood(m_pData->m_pModel->GetLogLikelihoodForTotal());
    gp_ST_TopShapeClusters->SetTopClusters(*C_ST_MAX);
    for (j=0 ;j <= m_pParameters->GetNumTotalEllipses(); j++) {
        //set purely spatial cluster
        CPurelySpatialCluster C_PS(gpPrintDirection);
        C_PS.SetCenter(nCenter);
        C_PS.SetRate(m_pParameters->GetAreaScanRateType());
        C_PS.SetEllipseOffset(j);
        C_PS.SetDuczmalCorrection((j == 0 || !m_pParameters->GetDuczmalCorrectEllipses() ? 1 : m_pData->mdE_Shapes[j - 1]));
        //set space-time cluster
        CSpaceTimeCluster C_ST(eIncludeClustersType, m_pData->m_nTimeIntervals, m_pData->m_nIntervalCut, gpPrintDirection);
        C_ST.SetCenter(nCenter);
        C_ST.SetRate(m_pParameters->GetAreaScanRateType());
        C_ST.SetEllipseOffset(j);
        C_ST.SetDuczmalCorrection((j == 0 || !m_pParameters->GetDuczmalCorrectEllipses() ? 1 : m_pData->mdE_Shapes[j - 1]));
        //get top clusters for iterations
        CPurelySpatialCluster & Top_PS_ShapeCluster = (CPurelySpatialCluster&)(gp_PS_TopShapeClusters->GetTopCluster(j));
        CSpaceTimeCluster & Top_ST_ShapeCluster = (CSpaceTimeCluster&)(gp_ST_TopShapeClusters->GetTopCluster(j));
        for (i=1; i <= m_pData->m_NeighborCounts[j][nCenter]; i++) {
           //First find best purely spatial cluster for iteration
           C_PS.AddNeighbor(j, *m_pData, m_pData->m_pCases, i);
           if (C_PS.RateIsOfInterest(m_pData->m_nTotalCases, m_pData->m_nTotalMeasure)) {
             C_PS.m_nLogLikelihood = m_pData->m_pModel->CalcLogLikelihood(C_PS.m_nCases, C_PS.m_nMeasure);
             if (C_PS.m_nLogLikelihood > Top_PS_ShapeCluster.m_nLogLikelihood)
               Top_PS_ShapeCluster = C_PS;
           }
           //now find best space-time cluster for iteration
           C_ST.AddNeighbor(j, *m_pData, m_pData->m_pCases, i);                            
           C_ST.InitTimeIntervalIndeces();
           while (C_ST.SetNextTimeInterval()) {
                if (C_ST.RateIsOfInterest(m_pData->m_nTotalCases, m_pData->m_nTotalMeasure)) {
                  C_ST.m_nLogLikelihood = m_pData->m_pModel->CalcLogLikelihood(C_ST.m_nCases, C_ST.m_nMeasure);
                  if (C_ST.m_nLogLikelihood > Top_ST_ShapeCluster.m_nLogLikelihood)
                    Top_ST_ShapeCluster  = C_ST;
                }
           }
        }
    }

    //get copy of best purely spatial cluster over all iterations
    *C_PS_MAX = (CPurelySpatialCluster&)(gp_PS_TopShapeClusters->GetTopCluster());
    //get copy of best space-time cluster over all iterations
    *C_ST_MAX = (CSpaceTimeCluster&)(gp_ST_TopShapeClusters->GetTopCluster());
    //determine which has better LLR
    if (!C_PS_MAX->ClusterDefined()) {
      //if one is not defined, than other might be... regardless, one of then is returned
      pTopCluster = C_ST_MAX;
      delete C_PS_MAX; C_PS_MAX=0;
    }
    else {//determine by compare LLR or duczmal corrected LLR 
      if (C_PS_MAX->GetDuczmalCorrectedLogLikelihoodRatio() > C_ST_MAX->GetDuczmalCorrectedLogLikelihoodRatio()) {
        pTopCluster = C_PS_MAX;
        delete C_ST_MAX; C_ST_MAX=0;
      }
      else {
        pTopCluster = C_ST_MAX;
        delete C_PS_MAX; C_PS_MAX=0;
      }
    }
  }
  catch (ZdException & x) {
    delete C_PS_MAX;C_PS_MAX=0;
    delete C_ST_MAX;C_ST_MAX=0;
    x.AddCallpath("GetTopCluster()", "C_ST_PS_Analysis");
    throw;
  }
  return pTopCluster;
}

/** Returns loglikelihood for Monte Carlo replication. */
double C_ST_PS_Analysis::MonteCarlo() {
  CMeasureList                * pMeasureList = 0;
  CPurelySpatialCluster         C_PS(gpPrintDirection);
  double                        dMaxLogLikelihoodRatio;
  int                           k;
  tract_t                       i, j;

  try {
    CSpaceTimeCluster C_ST(m_pParameters->GetIncludeClustersType(), m_pData->m_nTimeIntervals,
                           m_pData->m_nIntervalCut, gpPrintDirection);
    C_PS.SetRate(m_pParameters->GetAreaScanRateType());
    C_ST.SetRate(m_pParameters->GetAreaScanRateType());
    switch (m_pParameters->GetAreaScanRateType()) {
      case HIGH       : pMeasureList = new CMinMeasureList(*m_pData, *gpPrintDirection);
                        break;
      case LOW        : pMeasureList = new CMaxMeasureList(*m_pData, *gpPrintDirection);
                        break;
      case HIGHANDLOW : pMeasureList = new CMinMaxMeasureList(*m_pData, *gpPrintDirection);
                        break;
      default         : ZdGenerateException("Unknown incidence rate specifier \"%d\".", "MonteCarlo()",
                                             m_pParameters->GetAreaScanRateType());
    }

    //Iterate over circle/ellipse(s) - remember that circle is allows zero'th item.
    for (k=0; k <= m_pParameters->GetNumTotalEllipses(); k++) {
       for (i=0; i < m_pData->m_nGridTracts; i++) {
          C_PS.Initialize(i);
          C_ST.Initialize(i);
          for (j=1; j <= m_pData->m_NeighborCounts[k][i]; j++) {
            C_PS.AddNeighbor(k, *m_pData, m_pData->m_pSimCases, j);
            pMeasureList->AddMeasure(C_PS.m_nCases, C_PS.m_nMeasure);
            C_ST.AddNeighbor(k, *m_pData, m_pData->m_pSimCases, j);
            C_ST.InitTimeIntervalIndeces();
            while (C_ST.SetNextTimeInterval())
                 pMeasureList->AddMeasure(C_ST.m_nCases, C_ST.m_nMeasure);
          }
       }
       pMeasureList->SetForNextIteration(k);
    }
    dMaxLogLikelihoodRatio = pMeasureList->GetMaximumLogLikelihoodRatio();
    delete pMeasureList;
  }
  catch (ZdException & x) {
    delete pMeasureList;
    x.AddCallpath("MonteCarlo()", "C_ST_PS_Analysis");
    throw;
  }
  return dMaxLogLikelihoodRatio;
}


/** Returns loglikelihood for Monte Carlo Prospective replication. */
double C_ST_PS_Analysis::MonteCarloProspective() {
  CMeasureList                * pMeasureList = 0;
  CPurelySpatialCluster         C_PS(gpPrintDirection);
  double                        dMaxLogLikelihoodRatio;
  long                          lTime;
  Julian                        jCurrentDate;
  int                           iThisStartInterval, n, m, k;
  tract_t                       i, j;

  try {
    //for prospective Space-Time, m_bAliveClustersOnly should be false..
    //m_bAliveClustersOnly is the first parameter into the CSpaceTimeCluster class
    CSpaceTimeCluster     C_ST(ALLCLUSTERS, m_pData->m_nTimeIntervals, m_pData->m_nIntervalCut, gpPrintDirection);
    C_PS.SetRate(m_pParameters->GetAreaScanRateType());
    C_ST.SetRate(m_pParameters->GetAreaScanRateType());
    switch (m_pParameters->GetAreaScanRateType()) {
      case HIGH       : pMeasureList = new CMinMeasureList(*m_pData, *gpPrintDirection);
                        break;
      case LOW        : pMeasureList = new CMaxMeasureList(*m_pData, *gpPrintDirection);
                        break;
      case HIGHANDLOW : pMeasureList = new CMinMaxMeasureList(*m_pData, *gpPrintDirection);
                        break;
       default        : ZdGenerateException("Unknown incidence rate specifier \"%d\".", "MonteCarloProspective()",
                                             m_pParameters->GetAreaScanRateType());
    }

    //Iterate over circle/ellipse(s) - remember that circle is allows zero'th item.
    for (k=0; k <= m_pParameters->GetNumTotalEllipses(); k++) {
       for (i=0; i < m_pData->m_nGridTracts; i++) {
          C_PS.Initialize(i);
          C_ST.Initialize(i);
           for (tract_t j=1; j<=m_pData->m_NeighborCounts[k][i]; j++) {
              C_PS.AddNeighbor(k, *m_pData, m_pData->m_pSimCases, j);
              pMeasureList->AddMeasure(C_PS.m_nCases, C_PS.m_nMeasure);
              C_ST.AddNeighbor(k, *m_pData, m_pData->m_pSimCases, j);    // k use to be "0"
              //Need to keep track of the current date as you loop through intervals
              jCurrentDate = m_pData->m_nEndDate;
              // Loop from study end date back to Prospective start date -- loop by interval
              for (n = m_pData->m_nTimeIntervals; n >= m_pData->m_nProspectiveIntervalStart; n--) {
                //Need to re-compute duration due to by using current date (whatever date loop "n" is at)
                //and the Begin Study Date
                iThisStartInterval = std::max(0, n - m_pData->ComputeNewCutoffInterval(m_pData->m_nStartDate,jCurrentDate));
                C_ST.InitTimeIntervalIndeces(iThisStartInterval, n);
                while (C_ST.SetNextProspTimeInterval())
                     pMeasureList->AddMeasure(C_ST.m_nCases, C_ST.m_nMeasure);
              }
           }
       }
       pMeasureList->SetForNextIteration(k);
    }
    dMaxLogLikelihoodRatio = pMeasureList->GetMaximumLogLikelihoodRatio();
    delete pMeasureList;
  }
  catch (ZdException & x) {
    delete pMeasureList;
    x.AddCallpath("MonteCarloProspective()", "C_ST_PS_Analysis");
    throw;
  }
  return dMaxLogLikelihoodRatio;
}

void C_ST_PS_Analysis::Setup() {
  try {
    gp_PS_TopShapeClusters = new TopClustersContainer(*m_pData);
    gp_ST_TopShapeClusters = new TopClustersContainer(*m_pData);
  }
  catch (ZdException &x) {
    x.AddCallpath("Setup()", "C_ST_PS_Analysis");
    delete gp_PS_TopShapeClusters;
    delete gp_ST_TopShapeClusters;
    throw;
  }
}

