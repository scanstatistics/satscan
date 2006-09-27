//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "stsMonteCarloSimFunctor.h"
#include "Toolkit.h"

stsMCSimSuccessiveFunctor::result_type stsMCSimSuccessiveFunctor::operator() (stsMCSimSuccessiveFunctor::param_type const & param)
{
  result_type temp_result;
  try
  {
    //randomize data
    macroRunTimeStartSerial(SerialRunTimeComponent::RandomDataGeneration);
    gDataHub.RandomizeData(*gpRandomizationContainer, *gpSimulationDataContainer, param);
    macroRunTimeStopSerial();
    //print simulation data to file, if requested
    if (gDataHub.GetParameters().GetOutputSimulationData()) {
      boost::mutex::scoped_lock     lock(gMutex);
      for (size_t t=0; t < gpSimulationDataContainer->size(); ++t)
         gDataWriter->write((*(*gpSimulationDataContainer)[t]), gDataHub.GetParameters());
    }
    //perform simulation to get loglikelihood ratio
    macroRunTimeStartSerial(SerialRunTimeComponent::ScanningSimulatedData);
    temp_result.dSuccessfulResult = gpAnalysis->ExecuteSimulation(*gpDataGateway);
    macroRunTimeStopSerial();
    temp_result.bUnExceptional = true;
  }
  catch (ZdMemoryException & e)
  {
    temp_result.eException_type = stsMCSimJobSource::result_type::zdmemory;
    temp_result.Exception = e;
    temp_result.bUnExceptional = false;
  }
  catch (ZdException & e)
  {
    temp_result.eException_type = stsMCSimJobSource::result_type::zd;
    temp_result.Exception = e;
    temp_result.bUnExceptional = false;
  }
  catch (std::exception & e) {
    temp_result.eException_type = stsMCSimJobSource::result_type::std;
    temp_result.Exception = ZdException(e, "", "stsMCSimSuccessiveFunctor");
    temp_result.bUnExceptional = false;
  }
  catch (...) {
    temp_result.eException_type = stsMCSimJobSource::result_type::unknown;
    temp_result.Exception = ZdException("(...) -- unknown error", "stsMCSimSuccessiveFunctor", ZdException::Normal);
    temp_result.bUnExceptional = false;
  }
  if (!temp_result.bUnExceptional) {
    temp_result.Exception.AddCallpath("operator()", "stsMCSimSuccessiveFunctor");
  }
  return temp_result;
}

