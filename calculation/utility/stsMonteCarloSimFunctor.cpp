//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "stsMonteCarloSimFunctor.h"
#include "Toolkit.h"
#include "SSException.h"
                                  
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
    if (!_simulation_output_filename.empty()) {
      boost::mutex::scoped_lock     lock(gMutex);
      for (size_t t=0; t < gpSimulationDataContainer->size(); ++t)
         gDataWriter->write((*(*gpSimulationDataContainer)[t]), gDataHub.GetParameters(), _simulation_output_filename);
    }
    //perform simulation to get loglikelihood ratio
    macroRunTimeStartSerial(SerialRunTimeComponent::ScanningSimulatedData);
    temp_result.dSuccessfulResult = gpAnalysis->ExecuteSimulation(*gpDataGateway);
    macroRunTimeStopSerial();
    temp_result.bUnExceptional = true;
  }
  catch (memory_exception & e)
  {
    temp_result.eException_type = stsMCSimJobSource::result_type::memory;
    temp_result.Exception = prg_exception(e.what(), "stsMCSimSuccessiveFunctor");
    temp_result.bUnExceptional = false;
  }
  catch (prg_exception & e)
  {
    temp_result.eException_type = stsMCSimJobSource::result_type::prg;
    temp_result.Exception = e;
    temp_result.bUnExceptional = false;
  }
  catch (std::exception & e) {
    temp_result.eException_type = stsMCSimJobSource::result_type::std;
    temp_result.Exception = prg_exception(e.what(), "stsMCSimSuccessiveFunctor");
    temp_result.bUnExceptional = false;
  }
  catch (...) {
    temp_result.eException_type = stsMCSimJobSource::result_type::unknown;
    temp_result.Exception = prg_exception("(...) -- unknown error", "stsMCSimSuccessiveFunctor");
    temp_result.bUnExceptional = false;
  }
  if (!temp_result.bUnExceptional) {
    temp_result.Exception.addTrace("operator()", "stsMCSimSuccessiveFunctor");
  }
  return temp_result;
}

