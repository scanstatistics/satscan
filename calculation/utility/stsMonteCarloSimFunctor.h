//******************************************************************************
#ifndef stsMonteCarloSimFunctorH
#define stsMonteCarloSimFunctorH
//******************************************************************************
#include "boost/shared_ptr.hpp"
#include "boost/thread/mutex.hpp"
#include "AnalysisRun.h"
#include "stsMCSimJobSource.h"
#include "DataSetWriter.h"

//runs jobs for the "successive" algorithm
class stsMCSimSuccessiveFunctor
{
public:
  typedef unsigned int param_type;
  typedef stsMCSimJobSource::result_type result_type;

private:
  boost::mutex                               & gMutex;
  CSaTScanData const                         & gDataHub;
  boost::shared_ptr<AbstractDataSetGateway>    gpDataGateway;
  boost::shared_ptr<CAnalysis>                 gpAnalysis;
  boost::shared_ptr<SimulationDataContainer_t> gpSimulationDataContainer;
  boost::shared_ptr<RandomizerContainer_t>     gpRandomizationContainer;
  boost::shared_ptr<AbstractDataSetWriter>     gDataWriter;

public:
  stsMCSimSuccessiveFunctor(
    boost::mutex& Mutex
   ,CSaTScanData const & theDataHub
   ,boost::shared_ptr<CAnalysis> pAnalysis
   ,boost::shared_ptr<SimulationDataContainer_t> pSimulationDataContainer
   ,boost::shared_ptr<RandomizerContainer_t> pRandomizationContainer
  )
   : gMutex(Mutex)
   , gDataHub(theDataHub)
   , gpAnalysis(pAnalysis)
   , gpSimulationDataContainer(pSimulationDataContainer)
   , gpRandomizationContainer(pRandomizationContainer)
  {
    //get container for simulation data - this data will be modified in the randomize process
    gDataHub.GetDataSetHandler().GetSimulationDataContainer(*gpSimulationDataContainer);
    //get container of data randomizers - these will modify the simulation data
    gDataHub.GetDataSetHandler().GetRandomizerContainer(*gpRandomizationContainer);
    //get data gateway given dataset handler's real data and simulated data structures
    gpDataGateway.reset(gDataHub.GetDataSetHandler().GetNewDataGatewayObject());
    gDataHub.GetDataSetHandler().GetSimulationDataGateway(*gpDataGateway, *gpSimulationDataContainer);
    //allocate appropriate data members for simulation algorithm
    gpAnalysis->AllocateSimulationObjects(*gpDataGateway);
    gDataWriter.reset(AbstractDataSetWriter::getNewDataSetWriter(theDataHub.GetParameters()));
  }

//  ~stsMonteCarloSimFunctor()
//  {
//  }

  result_type operator() (param_type const & param);

};
//******************************************************************************
#endif

