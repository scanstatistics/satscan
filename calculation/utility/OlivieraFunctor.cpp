//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "OlivieraFunctor.h"
#include "Toolkit.h"
#include "SSException.h"
                                  
OlivieraFunctor::result_type OlivieraFunctor::operator() (OlivieraFunctor::param_type const & param)
{
  result_type temp_result;
  try
  {
    //randomize data
    macroRunTimeStartSerial(SerialRunTimeComponent::RandomDataGeneration);
    for (size_t t=0; t < _oliviera_sets.size(); ++t)
        _randomization_container->at(t)->RandomizeData(*_oliviera_sets.at(t), *_simulation_data_container->at(t), param);
    // update meta location strutures as necessary
    if (_data_hub.GetParameters().UseMetaLocationsFile() || _data_hub.GetParameters().UsingMultipleCoordinatesMetaLocations())
        for (SimulationDataContainer_t::iterator itr=_simulation_data_container->begin(); itr != _simulation_data_container->end(); ++itr)
            (*itr)->setCaseData_MetaLocations(_data_hub.GetTInfo()->getMetaManagerProxy());
    macroRunTimeStopSerial();

    //calculate most likely clusters
    boost::shared_ptr<MLC_Collections_t> topClustersContainer(new MLC_Collections_t());
    for (std::vector<double>::const_iterator itr=_data_hub.GetParameters().getExecuteSpatialWindowStops().begin(); itr != _data_hub.GetParameters().getExecuteSpatialWindowStops().end(); ++itr)
        topClustersContainer->push_back(MostLikelyClustersContainer(*itr));

    _analysis->FindTopClusters(*_data_gateway, *topClustersContainer);
    temp_result.dSuccessfulResult = topClustersContainer;

    temp_result.bUnExceptional = true;
  }
  catch (memory_exception & e)
  {
    temp_result.eException_type = OlivieraJobSource::result_type::memory;
    temp_result.Exception = prg_exception(e.what(), "OlivieraFunctor");
    temp_result.bUnExceptional = false;
  }
  catch (prg_exception & e)
  {
    temp_result.eException_type = OlivieraJobSource::result_type::prg;
    temp_result.Exception = e;
    temp_result.bUnExceptional = false;
  }
  catch (std::exception & e) {
    temp_result.eException_type = OlivieraJobSource::result_type::std;
    temp_result.Exception = prg_exception(e.what(), "OlivieraFunctor");
    temp_result.bUnExceptional = false;
  }
  catch (...) {
    temp_result.eException_type = OlivieraJobSource::result_type::unknown;
    temp_result.Exception = prg_exception("(...) -- unknown error", "OlivieraFunctor");
    temp_result.bUnExceptional = false;
  }
  if (!temp_result.bUnExceptional) {
    temp_result.Exception.addTrace("operator()", "OlivieraFunctor");
  }
  return temp_result;
}
