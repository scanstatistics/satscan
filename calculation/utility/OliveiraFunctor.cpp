//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "OliveiraFunctor.h"
#include "Toolkit.h"
#include "SSException.h"
                                  
OliveiraFunctor::result_type OliveiraFunctor::operator() (OliveiraFunctor::param_type const & param)
{
  result_type temp_result;
  try
  {
    //randomize data
    macroRunTimeStartSerial(SerialRunTimeComponent::RandomDataGeneration);
    for (size_t t=0; t < _oliveira_sets.size(); ++t)
        _randomization_container->at(t)->RandomizeData(*_oliveira_sets.at(t), *_simulation_data_container->at(t), param);
    // update meta location structures as necessary
    if (_runner.GetDataHub().GetParameters().UseMetaLocationsFile() || _runner.GetDataHub().GetParameters().UsingMultipleCoordinatesMetaLocations())
        for (SimulationDataContainer_t::iterator itr=_simulation_data_container->begin(); itr != _simulation_data_container->end(); ++itr)
            (*itr)->setCaseData_MetaLocations(_runner.GetDataHub().GetTInfo()->getMetaManagerProxy());
    macroRunTimeStopSerial();

    //calculate most likely clusters
    boost::shared_ptr<MLC_Collections_t> topClustersContainer(new MLC_Collections_t());
    for (std::vector<double>::const_iterator itr=_runner.GetDataHub().GetParameters().getExecuteSpatialWindowStops().begin(); itr != _runner.GetDataHub().GetParameters().getExecuteSpatialWindowStops().end(); ++itr)
        topClustersContainer->push_back(MostLikelyClustersContainer(*itr));

    _analysis->FindTopClusters(*_data_gateway, *topClustersContainer);
    boost::shared_ptr<MostLikelyClustersContainer> reportClusters(new MostLikelyClustersContainer(0));
    _runner.rankClusterCollections(*topClustersContainer, *reportClusters, 0, PrintNull());

    temp_result.dSuccessfulResult = std::make_pair(reportClusters, topClustersContainer);
    temp_result.bUnExceptional = true;
  }
  catch (memory_exception & e)
  {
    temp_result.eException_type = OliveiraJobSource::result_type::memory;
    temp_result.Exception = prg_exception(e.what(), "OliveiraFunctor");
    temp_result.bUnExceptional = false;
  }
  catch (prg_exception & e)
  {
    temp_result.eException_type = OliveiraJobSource::result_type::prg;
    temp_result.Exception = e;
    temp_result.bUnExceptional = false;
  }
  catch (std::exception & e) {
    temp_result.eException_type = OliveiraJobSource::result_type::std;
    temp_result.Exception = prg_exception(e.what(), "OliveiraFunctor");
    temp_result.bUnExceptional = false;
  }
  catch (...) {
    temp_result.eException_type = OliveiraJobSource::result_type::unknown;
    temp_result.Exception = prg_exception("(...) -- unknown error", "OliveiraFunctor");
    temp_result.bUnExceptional = false;
  }
  if (!temp_result.bUnExceptional) {
    temp_result.Exception.addTrace("operator()", "OliveiraFunctor");
  }
  return temp_result;
}
