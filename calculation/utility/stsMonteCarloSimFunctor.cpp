//---------------------------------------------------------------------------

#include "SaTScan.h"
#pragma hdrstop

#include "stsMonteCarloSimFunctor.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)

stsMonteCarloSimFunctor::result_type stsMonteCarloSimFunctor::operator() (stsMonteCarloSimFunctor::params_type const & params)
{
  try
  {
    result_type temp_result;
    //randomize data
    gDataHub.RandomizeData(*gpRandomizationContainer, *gpSimulationDataContainer, params);
    //print simulation data to file, if requested
    if (gDataHub.GetParameters().GetOutputSimulationData()) {
      boost::mutex::scoped_lock     lock(gMutex);
      for (size_t t=0; t < gpSimulationDataContainer->size(); ++t)
         (*gpSimulationDataContainer)[t]->WriteSimulationData(gDataHub.GetParameters(), params);
    }
    //perform simulation to get loglikelihood ratio
    temp_result = gpAnalysis->ExecuteSimulation(*gpDataGateway);
    return temp_result;
  }
  catch (ZdException & e)
  {
    e.AddCallpath("operator()", "stsMonteCarloSimFunctor");
    throw;
  }
}
