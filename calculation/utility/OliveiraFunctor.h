//******************************************************************************
#ifndef __OliveiraFunctor_H
#define __OliveiraFunctor_H
//******************************************************************************
#include "boost/shared_ptr.hpp"
#include "boost/thread/mutex.hpp"
#include "AnalysisRun.h"
#include "OliveiraJobSource.h"
#include "DataSetWriter.h"
#include "AbstractBruteForceAnalysis.h"

//runs jobs for the "successive" algorithm
class OliveiraFunctor
{
public:
  typedef unsigned int param_type;
  typedef OliveiraJobSource::result_type result_type;

private:
    AnalysisExecution const & _execution;
    const RealDataContainer_t& _oliveira_sets;
    boost::shared_ptr<AbstractDataSetGateway> _data_gateway;
    boost::shared_ptr<CAnalysis> _analysis;
    boost::shared_ptr<SimulationDataContainer_t> _simulation_data_container;
    boost::shared_ptr<RandomizerContainer_t> _randomization_container;

public:
  OliveiraFunctor(const RealDataContainer_t& oliveira_sets, AnalysisExecution const & execution, boost::shared_ptr<CAnalysis> pAnalysis)
      : _execution(execution), _oliveira_sets(oliveira_sets), _analysis(pAnalysis) {

    // create data gateway
    _data_gateway.reset(_execution.getDataHub().GetDataSetHandler().GetNewDataGatewayObject());

    // get container for simulation data - this data will be modified in the randomize process
    _simulation_data_container.reset(new SimulationDataContainer_t());
	execution.getDataHub().GetDataSetHandler().GetSimulationDataContainer(*_simulation_data_container);

    // get container of data randomizers
    _randomization_container.reset(new RandomizerContainer_t());
	execution.getDataHub().GetDataSetHandler().GetRandomizerContainer(*_randomization_container);

    // set data gateway given dataset handler's oliveira data and simulated data structures
	execution.getDataHub().GetDataSetHandler().GetOliveraDataGateway(*_data_gateway, *_simulation_data_container);

    // allocate objects used in 'FindTopClusters()' process
    _analysis->AllocateTopClustersObjects(*_data_gateway);

  }

//  ~stsMonteCarloSimFunctor()
//  {
//  }

  result_type operator() (param_type const & param);

};
//******************************************************************************
#endif
