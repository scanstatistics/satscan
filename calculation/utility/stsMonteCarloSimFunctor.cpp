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
    gDataHub.RandomizeIsolatedData(*gpRandomizationContainer, *gpSimulationDataContainer, params);
    //perform simulation to get loglikelihood ratio
    temp_result = (gpAnalysis->IsMonteCarlo() ? gpAnalysis->MonteCarlo(gpDataGateway->GetDataStreamInterface(0)) : gpAnalysis->FindTopRatio(*gpDataGateway));
    return temp_result;
  }
  catch (ZdException & e)
  {
    e.AddCallpath("operator()", "stsMonteCarloSimFunctor");
    throw;
  }
}
