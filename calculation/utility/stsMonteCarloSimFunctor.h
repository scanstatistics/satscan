//---------------------------------------------------------------------------

#ifndef stsMonteCarloSimFunctorH
#define stsMonteCarloSimFunctorH

//#include "DataStreamHandler.h"
#include "stsLogLikelihood.h"
//#include "DataStreamGateway.h"
#include "AnalysisRun.h"
#include "boost/shared_ptr.hpp"
//---------------------------------------------------------------------------
struct stsMonteCarloSimFunctorParams
{
  unsigned int giSimulationNumber;
};

class stsMonteCarloSimFunctor
{
public:
  typedef unsigned int params_type;
  typedef double result_type;

private:
  CSaTScanData const &                 gDataHub;
  boost::shared_ptr<AbtractDataStreamGateway> gpDataGateway;
  boost::shared_ptr<CAnalysis>         gpAnalysis;
  boost::shared_ptr<SimulationDataContainer_t> gpSimulationDataContainer;
  boost::shared_ptr<RandomizerContainer_t>     gpRandomizationContainer;

public:
  stsMonteCarloSimFunctor(CSaTScanData const & theDataHub, boost::shared_ptr<CAnalysis> pAnalysis, boost::shared_ptr<SimulationDataContainer_t> pSimulationDataContainer, boost::shared_ptr<RandomizerContainer_t> pRandomizationContainer)
   : gDataHub(theDataHub)
   , gpAnalysis(pAnalysis)
   , gpSimulationDataContainer(pSimulationDataContainer)
   , gpRandomizationContainer(pRandomizationContainer)
  {
    //get container for simulation data - this data will be modified in the randomize process
    gDataHub.GetDataStreamHandler().GetSimulationDataContainer(*gpSimulationDataContainer);
    //get container of data randomizers - these will modify the simulation data
    gDataHub.GetDataStreamHandler().GetRandomizerContainer(*gpRandomizationContainer);
    //get data gateway given data stream handler's real data and simulated data structures
    gpDataGateway.reset(gDataHub.GetDataStreamHandler().GetNewSimulationDataGateway(*gpSimulationDataContainer));
    //allocate appropriate data members for simulation algorithm
    gpAnalysis->AllocateSimulationObjects(*gpDataGateway);
  }

//  ~stsMonteCarloSimFunctor()
//  {
//  }

  result_type operator() (params_type const & params);

};





#endif
