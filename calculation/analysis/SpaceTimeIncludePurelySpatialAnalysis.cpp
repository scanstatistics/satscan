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
void C_ST_PS_Analysis::CalculateTopCluster(tract_t tCenter, const DataStreamGateway & DataGateway, bool bSimulation) {
  int                           j;
  tract_t                       i, iNumNeighbors;
  IncludeClustersType           eIncludeClustersType;

  try {
    // if Prospective Space-Time then Alive Clusters Only.
    if (m_pParameters->GetAnalysisType() == PROSPECTIVESPACETIME)
      eIncludeClustersType = (bSimulation ? ALLCLUSTERS : ALIVECLUSTERS);
    else
      eIncludeClustersType = m_pParameters->GetIncludeClustersType();

    gp_PS_TopShapeClusters->Reset(tCenter);
    gp_ST_TopShapeClusters->Reset(tCenter);
    for (j=0 ;j <= m_pParameters->GetNumTotalEllipses(); ++j) {
       //set purely spatial cluster
       CPurelySpatialCluster C_PS(*m_pData, gpPrintDirection);
       C_PS.SetCenter(tCenter);
       C_PS.SetRate(m_pParameters->GetAreaScanRateType());
       C_PS.SetEllipseOffset(j);
       C_PS.SetDuczmalCorrection((j == 0 || !m_pParameters->GetDuczmalCorrectEllipses() ? 1 : m_pData->GetShapesArray()[j - 1]));
       //set space-time cluster
       CSpaceTimeCluster C_ST(eIncludeClustersType, *m_pData, *gpPrintDirection);
       C_ST.SetCenter(tCenter);
       C_ST.SetRate(m_pParameters->GetAreaScanRateType());
       C_ST.SetEllipseOffset(j);
       C_ST.SetDuczmalCorrection((j == 0 || !m_pParameters->GetDuczmalCorrectEllipses() ? 1 : m_pData->GetShapesArray()[j - 1]));
       //get top clusters for iterations
       CPurelySpatialCluster & Top_PS_ShapeCluster = (CPurelySpatialCluster&)(gp_PS_TopShapeClusters->GetTopCluster(j));
       CSpaceTimeCluster & Top_ST_ShapeCluster = (CSpaceTimeCluster&)(gp_ST_TopShapeClusters->GetTopCluster(j));
       iNumNeighbors = m_pData->GetNeighborCountArray()[j][tCenter];
       for (i=1; i <= iNumNeighbors; ++i) {
         //First find best purely spatial cluster for iteration
         C_PS.AddNeighbor(m_pData->GetNeighbor(j, tCenter, i), DataGateway);
         C_PS.CompareTopCluster(Top_PS_ShapeCluster, m_pData);
         //now find best space-time cluster for iteration
         C_ST.AddNeighbor(m_pData->GetNeighbor(j, tCenter, i), DataGateway);
         C_ST.CompareTopCluster(Top_ST_ShapeCluster, *m_pData);
      }
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("CalculateTopCluster()","C_ST_PS_Analysis");
    throw;
  }
}

CCluster & C_ST_PS_Analysis::GetTopCalculatedCluster() {
  try {
    CPurelySpatialCluster& PSCluster = (CPurelySpatialCluster&)(gp_PS_TopShapeClusters->GetTopCluster());
    CSpaceTimeCluster& STCluster = (CSpaceTimeCluster&)(gp_ST_TopShapeClusters->GetTopCluster());
    if (!PSCluster.ClusterDefined())
      return STCluster;
    else if (PSCluster.GetDuczmalCorrectedLogLikelihoodRatio() > STCluster.GetDuczmalCorrectedLogLikelihoodRatio())
      return PSCluster;
    else
      return STCluster;
  }
  catch (ZdException &x) {
    x.AddCallpath("GetTopCalculatedCluster()","C_ST_PS_Analysis");
    throw;
  }
}

/** Returns loglikelihood for Monte Carlo replication. */
double C_ST_PS_Analysis::MonteCarlo(const DataStreamInterface & Interface) {
  CMeasureList                * pMeasureList = 0;
  CPurelySpatialCluster         C_PS(*m_pData, gpPrintDirection);
  double                        dMaxLogLikelihoodRatio;
  int                           k;
  tract_t                       i, j, iNumNeighbors;

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
            C_PS.AddNeighborData(m_pData->GetNeighbor(k, i, j), Interface);
            C_PS.ComputeBestMeasures(*pMeasureList);
            C_ST.AddNeighborData(m_pData->GetNeighbor(k, i, j), Interface);
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
double C_ST_PS_Analysis::MonteCarloProspective(const DataStreamInterface & Interface) {
  CMeasureList                * pMeasureList = 0;
  double                        dMaxLogLikelihoodRatio;
  long                          lTime;
  Julian                        jCurrentDate;
  int                           iThisStartInterval, n, m, k;
  tract_t                       i, j, iNumNeighbors;

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
              C_PS.AddNeighborData(m_pData->GetNeighbor(k, i, j), Interface);
              C_PS.ComputeBestMeasures(*pMeasureList);
              C_ST.AddNeighborData(m_pData->GetNeighbor(k, i, j), Interface);    
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

void C_ST_PS_Analysis::SetTopClusters(const DataStreamGateway & DataGateway, bool bSimulation) {
  IncludeClustersType   eIncludeClustersType;

  try {
    // if Prospective Space-Time then Alive Clusters Only.
    if (m_pParameters->GetAnalysisType() == PROSPECTIVESPACETIME)
      eIncludeClustersType = (bSimulation ? ALLCLUSTERS : ALIVECLUSTERS);
    else
      eIncludeClustersType = m_pParameters->GetIncludeClustersType();

    gp_PS_TopShapeClusters->SetTopClusters(CPurelySpatialCluster(*m_pData, gpPrintDirection));
    gp_ST_TopShapeClusters->SetTopClusters(CSpaceTimeCluster(eIncludeClustersType, *m_pData, *gpPrintDirection));
  }
  catch (ZdException &x) {
    x.AddCallpath("SetTopClusters()","C_ST_PS_Analysis");
    throw;
  }
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

