//---------------------------------------------------------------------------

#include "SaTScan.h"
#pragma hdrstop

#include "stsMonteCarloSimFunctor.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)

stsMonteCarloSimFunctor::result_type stsMonteCarloSimFunctor::operator() (stsMonteCarloSimFunctor::param_type const & param)
{
  result_type temp_result;
  try
  {
    //randomize data
    gDataHub.RandomizeData(*gpRandomizationContainer, *gpSimulationDataContainer, param);
    //print simulation data to file, if requested
    if (gDataHub.GetParameters().GetOutputSimulationData()) {
      boost::mutex::scoped_lock     lock(gMutex);
      for (size_t t=0; t < gpSimulationDataContainer->size(); ++t)
         (*gpSimulationDataContainer)[t]->WriteSimulationData(gDataHub.GetParameters(), param);
    }
    //perform simulation to get loglikelihood ratio
    temp_result.second = gpAnalysis->ExecuteSimulation(*gpDataGateway);
  }
  catch (ZdException & e)
  {
    e.AddCallpath("operator()", "stsMonteCarloSimFunctor");
    temp_result.first.SetException(e);
  }
  catch (std::exception & e) { temp_result.first.SetException(e); }
  catch (...) { temp_result.first.SetUnknownException(); }
  return temp_result;
}

