//***************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//***************************************************************************
#include "PurelySpatialMonotoneAnalysis.h"

/** constructor */
CPSMonotoneAnalysis::CPSMonotoneAnalysis(const CParameters& Parameters, const CSaTScanData& DataHub, BasePrint& PrintDirection)
                    :CPurelySpatialAnalysis(Parameters, DataHub, PrintDirection) {
  Init();
}

/** destructor */
CPSMonotoneAnalysis::~CPSMonotoneAnalysis() {
  try {
    delete gpMaxCluster;
  }
  catch (...){}
}

/** Allocates objects used during simulations, instead of repeated allocations
    for each simulation.
    NOTE: This analysis has not been optimized to 'pre' allocate objects used in
          simulation process. This function is only a shell.                     */
void CPSMonotoneAnalysis::AllocateSimulationObjects(const AbstractDataSetGateway& DataGateway) {}

/** Allocates objects used during calculation of most likely clusters, instead
    of repeated allocations for each simulation.
    NOTE: This analysis has not been optimized to 'pre' allocate objects used in
          process of finding most likely clusters. This function is only a shell. */
void CPSMonotoneAnalysis::AllocateTopClustersObjects(const AbstractDataSetGateway& DataGateway) {}

/** Returns cluster centered at grid point nCenter, with the greatest loglikelihood ratio.
    Caller should not assume that returned reference is persistent, but should either call
    Clone() method or overloaded assignment operator. */
const CCluster & CPSMonotoneAnalysis::CalculateTopCluster(tract_t tCenter, const AbstractDataSetGateway& DataGateway) {
  CPSMonotoneCluster          * C_High = 0;
  CPSMonotoneCluster          * C_Low = 0;
  count_t                    ** ppCases(DataGateway.GetDataSetInterface(0/*for now*/).GetCaseArray());

  try {
    if (gParameters.GetExecuteScanRateType() == HIGHANDLOW) {
      C_High = new CPSMonotoneCluster(gpClusterDataFactory, DataGateway, gParameters.GetExecuteScanRateType());
      C_High->SetCenter(tCenter);
      C_High->AllocateForMaxCircles(gDataHub.GetNeighborCountArray()[0][tCenter]+1);
      C_High->SetRate(HIGH);
      C_High->DefineTopCluster(gDataHub, *gpLikelihoodCalculator, ppCases);

      C_Low = new CPSMonotoneCluster(gpClusterDataFactory, DataGateway, gParameters.GetExecuteScanRateType());
      C_Low->SetCenter(tCenter);
      C_Low->AllocateForMaxCircles(gDataHub.GetNeighborCountArray()[0][tCenter]+1);
      C_Low->SetRate(LOW);
      C_Low->DefineTopCluster(gDataHub, *gpLikelihoodCalculator, ppCases);
      //    if (C_High->m_nLogLikelihood >= C_Low->m_nLogLikelihood)
      if (C_High->m_nRatio >= C_Low->m_nRatio) {
         delete gpMaxCluster;
         gpMaxCluster = C_High;
         delete C_Low; C_Low=0;
       }
       else {
         delete gpMaxCluster;
         gpMaxCluster = C_Low;
         delete C_High; C_High=0;
       }
    }
    else {
      delete gpMaxCluster; gpMaxCluster=0;
      gpMaxCluster = new CPSMonotoneCluster(gpClusterDataFactory, DataGateway, gParameters.GetExecuteScanRateType());
      gpMaxCluster->SetCenter(tCenter);
      gpMaxCluster->AllocateForMaxCircles(gDataHub.GetNeighborCountArray()[0][tCenter]+1);
      gpMaxCluster->SetRate(gParameters.GetExecuteScanRateType());
      gpMaxCluster->DefineTopCluster(gDataHub, *gpLikelihoodCalculator, ppCases);
    }
  }
  catch (ZdException &x) {
    delete C_High; C_High=0;
    delete C_Low; C_Low=0;
    //MaxCluster could be C_High or C_Low or neither...
    //so set high and low to zero after deletion...  just in case.
    delete gpMaxCluster; gpMaxCluster=0;
    x.AddCallpath("CalculateTopCluster()","CPSMonotoneAnalysis");
    throw;
  }
  return *gpMaxCluster;
}

/** Returns loglikelihood ratio for Monte Carlo replication. */
double CPSMonotoneAnalysis::MonteCarlo(const DataSetInterface& Interface) {
   CPSMonotoneCluster           MaxCluster(gpClusterDataFactory, Interface, gParameters.GetExecuteScanRateType());
   CPSMonotoneCluster           C(gpClusterDataFactory, Interface, gParameters.GetExecuteScanRateType());
   CPSMonotoneCluster           C_High(gpClusterDataFactory, Interface, HIGH);
   CPSMonotoneCluster           C_Low(gpClusterDataFactory, Interface, LOW);
   count_t                   ** ppSimCases(Interface.GetCaseArray());

   try
      {
      MaxCluster.Initialize(0);
      MaxCluster.m_nLogLikelihood = gpLikelihoodCalculator->GetLogLikelihoodForTotal();
    
      C.AllocateForMaxCircles(gDataHub.m_nGridTracts+1);
      C_High.AllocateForMaxCircles(gDataHub.m_nGridTracts+1);
      C_Low.AllocateForMaxCircles(gDataHub.m_nGridTracts+1);
    
      for (int i = 0; i<gDataHub.m_nGridTracts; i++)
        {
        if (gParameters.GetExecuteScanRateType() == HIGHANDLOW)
          {
          C_High.Initialize(i);
          C_High.SetRate(HIGH);
          C_High.DefineTopCluster(gDataHub, *gpLikelihoodCalculator,  ppSimCases);
    
          C_Low.Initialize(i);
          C_Low.SetRate(LOW);
          C_Low.DefineTopCluster(gDataHub, *gpLikelihoodCalculator,  ppSimCases);

          //if (C_High.m_nLogLikelihood >= C_Low.m_nLogLikelihood &&
          //   C_High.m_nLogLikelihood > MaxCluster.m_nLogLikelihood)
          if (C_High.m_nRatio >= C_Low.m_nRatio &&
              C_High.m_nRatio > MaxCluster.m_nRatio)
            MaxCluster = C_High;
          //else if (C_Low.m_nLogLikelihood > MaxCluster.m_nLogLikelihood)
          else if (C_Low.m_nRatio > MaxCluster.m_nRatio)
            MaxCluster = C_Low;
    
          }
        else
          {
          C.Initialize(i);
          C.SetRate(gParameters.GetExecuteScanRateType());
          C.DefineTopCluster(gDataHub, *gpLikelihoodCalculator, ppSimCases);
    
          //if (C.m_nLogLikelihood > MaxCluster.m_nLogLikelihood)
          if (C.m_nRatio > MaxCluster.m_nRatio)
            MaxCluster = C;
          }
        }
      }
   catch (ZdException &x)
      {
      x.AddCallpath("MonteCarlo()", "CPSMonotoneAnalysis");
      throw;
      }
   return (MaxCluster.GetRatio());
}

/** Returns calculates log likelihood ratio about centroid. Currently this function calls CalculateTopCluster()
    but will likely be updated in the future. */
double CPSMonotoneAnalysis::MonteCarlo(tract_t tCenter, const AbstractDataSetGateway & DataGateway) {
  return CalculateTopCluster(tCenter, DataGateway).m_nRatio;
}

