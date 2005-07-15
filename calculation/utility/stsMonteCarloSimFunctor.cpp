//---------------------------------------------------------------------------

#include "SaTScan.h"
#pragma hdrstop

#include "stsMonteCarloSimFunctor.h"

//---------------------------------------------------------------------------
#pragma package(smart_init)

stsMCSimSuccessiveFunctor::result_type stsMCSimSuccessiveFunctor::operator() (stsMCSimSuccessiveFunctor::param_type const & param)
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
    temp_result.second.first = gpAnalysis->ExecuteSimulation(*gpDataGateway);
    temp_result.first = true;
  }
  catch (ZdException & e)
  {
    temp_result.second.second = e;
    temp_result.first = false;
  }
  catch (std::exception & e) {
    temp_result.second.second = ZdException(e, "", "stsMCSimSuccessiveFunctor");
    temp_result.first = false;
  }
  catch (...) {
    temp_result.second.second = ZdException("(...) -- unknown error", "stsMCSimSuccessiveFunctor", ZdException::Normal);
    temp_result.first = false;
  }
  if (!temp_result.first) {
    temp_result.second.second.AddCallpath("operator()", "stsMCSimSuccessiveFunctor");
  }
  return temp_result;
}

