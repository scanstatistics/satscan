#include "SaTScan.h"
#pragma hdrstop
#include "SVTTAnalysis.h"
#include "SVTTCluster.h"

/** constructor */
CSpatialVarTempTrendAnalysis::CSpatialVarTempTrendAnalysis(CParameters*  pParameters, CSaTScanData* pData, BasePrint *pPrintDirection)
                             :CAnalysis(pParameters, pData, pPrintDirection) {
  try {
    Init();
    Setup();
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor()","CSpatialVarTempTrendAnalysis");
    throw;
  }
}

/** destructor */
CSpatialVarTempTrendAnalysis::~CSpatialVarTempTrendAnalysis() {
  try {
    delete gpTopShapeClusters;
  }
  catch(...){}
}

/** calculates most likely cluster about central location 'tCenter' */
void CSpatialVarTempTrendAnalysis::CalculateTopCluster(tract_t tCenter, const DataStreamGateway & DataGateway, bool bSimulation) {
  int                   k;
  tract_t               i, iNumNeighbors;
  CModel              & ProbModel(m_pData->GetProbabilityModel());

  try {
    gpTopShapeClusters->Reset(tCenter);
    //Iterate over circle/ellipse(s) - remember that circle is allows zero'th item.
    for (k=0; k <= m_pParameters->GetNumTotalEllipses(); ++k) {
       CSVTTCluster thisCluster(DataGateway, m_pData->GetNumTimeIntervals(), gpPrintDirection);
       thisCluster.SetCenter(tCenter);
       thisCluster.SetRate(m_pParameters->GetAreaScanRateType());
       thisCluster.SetEllipseOffset(k);
       thisCluster.SetDuczmalCorrection((k == 0 || !m_pParameters->GetDuczmalCorrectEllipses() ? 1 : m_pData->GetShapesArray()[k - 1]));
       CSVTTCluster & TopShapeCluster = (CSVTTCluster&)(gpTopShapeClusters->GetTopCluster(k));
       iNumNeighbors = m_pData->GetNeighborCountArray()[k][tCenter];
       for (i=1; i <= iNumNeighbors; ++i) {
          thisCluster.AddNeighbor(m_pData->GetNeighbor(k, tCenter, i), DataGateway);
          thisCluster.m_nLogLikelihood = 0;
          for (size_t t=0; t < DataGateway.GetNumInterfaces(); ++t)
            thisCluster.m_nLogLikelihood += ProbModel.CalcSVTTLogLikelihood(t, &thisCluster, *(DataGateway.GetDataStreamInterface(t).GetTimeTrend()));
          if (thisCluster.m_nLogLikelihood && thisCluster.m_nLogLikelihood > TopShapeCluster.m_nLogLikelihood)
           TopShapeCluster = thisCluster;
       }
    }

    // the ratio needs to be calculated for each circle/cylinder, instead of here !!!!
    CSVTTCluster & Cluster = (CSVTTCluster&)(gpTopShapeClusters->GetTopCluster());
    Cluster.m_nRatio = Cluster.m_nLogLikelihood - m_pData->GetProbabilityModel().GetLogLikelihoodForTotal();
  }
  catch (ZdException &x) {
    x.AddCallpath("CalculateTopCluster()","CSpatialVarTempTrendAnalysis");
    throw;
  }
}

/** returns most likely cluster about a centroid, as calculated from last call to CalculateTopCluster() */
CCluster & CSpatialVarTempTrendAnalysis::GetTopCalculatedCluster() {
  CSVTTCluster & Cluster = (CSVTTCluster&)(gpTopShapeClusters->GetTopCluster());
  Cluster.gTimeTrendInside.SetAnnualTimeTrend(m_pParameters->GetTimeIntervalUnitsType(), m_pParameters->GetTimeIntervalLength());
  return Cluster;
}

/** calculates loglikelihood ratio for simulated data pointed to by DataStreamInterface
    in a retrospective manner */
double CSpatialVarTempTrendAnalysis::MonteCarlo(const DataStreamInterface & Interface) {
  int           k;
  tract_t       i, j, iNumNeighbors;
  double        dMaximumLogLikelihoodRatio;
  CModel      & ProbModel(m_pData->GetProbabilityModel());  

  try {
    SetTopClusters(Interface, true);

    //Iterate over circle/ellipse(s) - remember that circle is allows zero'th item.
    for (k=0; k <= m_pParameters->GetNumTotalEllipses(); k++) {
       CSVTTCluster thisCluster(Interface, m_pData->GetNumTimeIntervals(), gpPrintDirection);
       thisCluster.SetRate(m_pParameters->GetAreaScanRateType());
       thisCluster.SetEllipseOffset(k);
       thisCluster.SetDuczmalCorrection((k == 0 || !m_pParameters->GetDuczmalCorrectEllipses() ? 1 : m_pData->GetShapesArray()[k - 1]));
       CSVTTCluster & TopShapeCluster = (CSVTTCluster&)(gpTopShapeClusters->GetTopCluster(k));
       for (i=0; i < m_pData->m_nGridTracts; ++i) {
          thisCluster.InitializeSVTT(i, Interface);
          iNumNeighbors = m_pData->GetNeighborCountArray()[k][i];	
          for (j=1; j <= iNumNeighbors; j++) {
             thisCluster.AddNeighbor(m_pData->GetNeighbor(k, i, j), Interface, 0);
             thisCluster.m_nLogLikelihood = ProbModel.CalcSVTTLogLikelihood(0, &thisCluster, *(Interface.GetTimeTrend()));
             if (/*i==1 ||*/ thisCluster.m_nLogLikelihood > TopShapeCluster.m_nLogLikelihood)
               TopShapeCluster = thisCluster;
          }
       }
    }
    //get copy of best cluster over all iterations
    dMaximumLogLikelihoodRatio = gpTopShapeClusters->GetTopCluster().m_nRatio;
  }
  catch (ZdException &x) {
    x.AddCallpath("MonteCarlo()","CSpatialVarTempTrendAnalysis");
    throw;
  }
  return dMaximumLogLikelihoodRatio;
}

/** calculates loglikelihood ratio for simulated data pointed to by DataStreamInterface
    in a prospective manner -- NOT IMPLEMENTED -- */
double CSpatialVarTempTrendAnalysis::MonteCarloProspective(const DataStreamInterface & Interface) {
  ZdGenerateException("MonteCarloProspective() not implemented.","CSpatialVarTempTrendAnalysis");
  return 0;
}

/** sets object that contains to cluster list which represents top clusters for
    the circle and each ellipse shape */
void CSpatialVarTempTrendAnalysis::SetTopClusters(const DataStreamGateway & DataGateway, bool bSimulation) {
  try {
    CSVTTCluster thisCluster(DataGateway, m_pData->GetNumTimeIntervals(), gpPrintDirection);
    thisCluster.InitializeSVTT(0, DataGateway);
    gpTopShapeClusters->SetTopClusters(thisCluster);
  }
  catch (ZdException &x) {
    x.AddCallpath("SetTopClusters()","CSpatialVarTempTrendAnalysis");
    throw;
  }
}

/** sets object that contains to cluster list which represents top clusters for
    the circle and each ellipse shape */
void CSpatialVarTempTrendAnalysis::SetTopClusters(const DataStreamInterface & Interface, bool bSimulation) {
  try {
    CSVTTCluster thisCluster(Interface, m_pData->GetNumTimeIntervals(), gpPrintDirection);
    thisCluster.InitializeSVTT(0, Interface);
    gpTopShapeClusters->SetTopClusters(thisCluster);
  }
  catch (ZdException &x) {
    x.AddCallpath("SetTopClusters()","CSpatialVarTempTrendAnalysis");
    throw;
  }
}

/** internal setup function */
void CSpatialVarTempTrendAnalysis::Setup() {
  try {
    gpTopShapeClusters = new TopClustersContainer(*m_pData);
  }
  catch (ZdException &x) {
    x.AddCallpath("Setup()","CSpatialVarTempTrendAnalysis");
    delete gpTopShapeClusters;
    throw;
  }
}

