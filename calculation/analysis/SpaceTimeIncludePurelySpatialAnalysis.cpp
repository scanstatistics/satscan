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
  double			dTotalMeasure(m_pData->GetTotalMeasure());
  tract_t                       i, iNumNeighbors;
  CCluster                    * pTopCluster;
  CPurelySpatialCluster       * C_PS_MAX=0;
  CSpaceTimeCluster           * C_ST_MAX=0;
  IncludeClustersType           eIncludeClustersType;
  count_t                    ** ppCases(m_pData->GetCasesArray()), tTotalCases(m_pData->GetNumCases());
  CModel                      & ProbModel(m_pData->GetProbabilityModel());    

  try {
    // if Prospective Space-Time then Alive Clusters Only.
    if (m_pParameters->GetAnalysisType() == PROSPECTIVESPACETIME)
      eIncludeClustersType = ALIVECLUSTERS;
    else
      eIncludeClustersType = m_pParameters->GetIncludeClustersType();

    C_PS_MAX = new CPurelySpatialCluster(gpPrintDirection);
    C_PS_MAX->SetLogLikelihood(ProbModel.GetLogLikelihoodForTotal());
    gp_PS_TopShapeClusters->SetTopClusters(*C_PS_MAX);
    C_ST_MAX = new CSpaceTimeCluster(eIncludeClustersType, *m_pData, *gpPrintDirection);
    C_ST_MAX->SetLogLikelihood(ProbModel.GetLogLikelihoodForTotal());
    gp_ST_TopShapeClusters->SetTopClusters(*C_ST_MAX);
    for (j=0 ;j <= m_pParameters->GetNumTotalEllipses(); j++) {
        //set purely spatial cluster
        CPurelySpatialCluster C_PS(gpPrintDirection);
        C_PS.SetCenter(nCenter);
        C_PS.SetRate(m_pParameters->GetAreaScanRateType());
        C_PS.SetEllipseOffset(j);
        C_PS.SetDuczmalCorrection((j == 0 || !m_pParameters->GetDuczmalCorrectEllipses() ? 1 : m_pData->GetShapesArray()[j - 1]));
        //set space-time cluster
        CSpaceTimeCluster C_ST(eIncludeClustersType, *m_pData, *gpPrintDirection);
        C_ST.SetCenter(nCenter);
        C_ST.SetRate(m_pParameters->GetAreaScanRateType());
        C_ST.SetEllipseOffset(j);
        C_ST.SetDuczmalCorrection((j == 0 || !m_pParameters->GetDuczmalCorrectEllipses() ? 1 : m_pData->GetShapesArray()[j - 1]));
        //get top clusters for iterations
        CPurelySpatialCluster & Top_PS_ShapeCluster = (CPurelySpatialCluster&)(gp_PS_TopShapeClusters->GetTopCluster(j));
        CSpaceTimeCluster & Top_ST_ShapeCluster = (CSpaceTimeCluster&)(gp_ST_TopShapeClusters->GetTopCluster(j));
        iNumNeighbors = m_pData->GetNeighborCountArray()[j][nCenter];
        for (i=1; i <= iNumNeighbors; i++) {
           //First find best purely spatial cluster for iteration
           C_PS.AddNeighbor(j, *m_pData, ppCases, i);
           if (C_PS.RateIsOfInterest(tTotalCases, dTotalMeasure)) {
             C_PS.m_nLogLikelihood = ProbModel.CalcLogLikelihood(C_PS.m_nCases, C_PS.m_nMeasure);
             if (C_PS.m_nLogLikelihood > Top_PS_ShapeCluster.m_nLogLikelihood)
               Top_PS_ShapeCluster = C_PS;
           }
           //now find best space-time cluster for iteration
           C_ST.AddNeighbor(j, *m_pData, ppCases, i);                            
           C_ST.CompareTopCluster(Top_ST_ShapeCluster, *m_pData);
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
  tract_t                       i, j, iNumNeighbors;
  count_t                    ** ppSimCases(m_pData->GetSimCasesArray());

  try {
    CSpaceTimeCluster C_ST(m_pParameters->GetIncludeClustersType(), *m_pData, *gpPrintDirection);
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
          iNumNeighbors = m_pData->GetNeighborCountArray()[k][i];
          for (j=1; j <= iNumNeighbors; j++) {
            C_PS.AddNeighbor(k, *m_pData, ppSimCases, j);
            pMeasureList->AddMeasure(C_PS.m_nCases, C_PS.m_nMeasure);
            C_ST.AddNeighbor(k, *m_pData, ppSimCases, j);
            C_ST.ComputeBestMeasures(*pMeasureList);
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
  double                        dMaxLogLikelihoodRatio;
  long                          lTime;
  Julian                        jCurrentDate;
  int                           iThisStartInterval, n, m, k;
  tract_t                       i, j, iNumNeighbors;
  count_t                    ** ppSimCases(m_pData->GetSimCasesArray());
  
  try {
    //for prospective Space-Time, m_bAliveClustersOnly should be false..
    //m_bAliveClustersOnly is the first parameter into the CSpaceTimeCluster class
    CPurelySpatialProspectiveCluster C_PS(*m_pData, gpPrintDirection);
    CSpaceTimeCluster     C_ST(ALLCLUSTERS, *m_pData, *gpPrintDirection);
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
          iNumNeighbors = m_pData->GetNeighborCountArray()[k][i];
          for (tract_t j=1; j <= iNumNeighbors; j++) {
              C_PS.AddNeighbor(k, *m_pData, ppSimCases, j);
              C_PS.ComputeBestMeasures(*pMeasureList);
              C_ST.AddNeighbor(k, *m_pData, ppSimCases, j);    
              C_ST.ComputeBestMeasures(*pMeasureList);
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

