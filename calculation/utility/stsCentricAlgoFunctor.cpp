//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "stsCentricAlgoFunctor.h"

stsCentricAlgoFunctor::result_type stsCentricAlgoFunctor::operator() (param_type const & param)
{
  result_type temp_result;
  temp_result.bExceptional = false;
  try
  {
    grCentricAnalysis.ExecuteAboutCentroid(param-1, grCentroidCalculator, grDataSetGateway, grSimDataGateways);
  }
  catch (memory_exception & e)
  {
    temp_result.eException_type = stsCentricAlgoJobSource::result_type::memory;
    temp_result.Exception = prg_exception(e.what(), "stsCentricAlgoFunctor");
    temp_result.bExceptional = true;
  }
  catch (prg_exception & e)
  {
    temp_result.eException_type = stsCentricAlgoJobSource::result_type::prg;
    temp_result.Exception = e;
    temp_result.bExceptional = true;
  }
  catch (std::exception & e) {
    temp_result.eException_type = stsCentricAlgoJobSource::result_type::std;
    temp_result.Exception = prg_exception(e.what(), "stsCentricAlgoFunctor");
    temp_result.bExceptional = true;
  }
  catch (...) {
    temp_result.eException_type = stsCentricAlgoJobSource::result_type::unknown;
    temp_result.Exception = prg_exception("(...) -- unknown error", "stsCentricAlgoFunctor");
    temp_result.bExceptional = true;
  }
  if (temp_result.bExceptional) {
    temp_result.Exception.addTrace("operator()", "stsCentricAlgoFunctor");
  }
  return temp_result;
}

stsPurelyTemporal_Plus_CentricAlgoThreadFunctor::stsPurelyTemporal_Plus_CentricAlgoThreadFunctor(
  contractor_type & rContractor
 ,job_source_type & rJobSource
   ,stsCentricAlgoJobSource::result_type & rPurelyTemporalExecutionResult
 ,AsynchronouslyAccessible<PrintQueue> & rPrintDirection
 ,AbstractCentricAnalysis & rCentricAnalysis
 ,CentroidNeighborCalculator & rCentroidCalculator
 ,AbstractDataSetGateway const & rDataSetGateway
 ,ptr_vector<AbstractDataSetGateway> const & rSimDataGateways
)
 : grContractor(rContractor)
 , grJobSource(rJobSource)
 , grPurelyTemporalExecutionResult(rPurelyTemporalExecutionResult)
 , grPrintDirection(rPrintDirection)
 , grCentricAnalysis(rCentricAnalysis)
 , grDataSetGateway(rDataSetGateway)
 , grSimDataGateways(rSimDataGateways)
 , gRegularSubcontractor(rContractor, stsCentricAlgoFunctor(rCentricAnalysis, rCentroidCalculator, rDataSetGateway, rSimDataGateways))
{
  rPurelyTemporalExecutionResult.bExceptional = false;
}

void stsPurelyTemporal_Plus_CentricAlgoThreadFunctor::operator() ()
{
  try {
    grPrintDirection.Locked().Value().Printf("Evaluating purely temporal clusters\n", BasePrint::P_STDOUT);
    grCentricAnalysis.ExecuteAboutPurelyTemporalCluster(grDataSetGateway, grSimDataGateways);
  }
  catch (memory_exception & e)
  {
    grPurelyTemporalExecutionResult.eException_type = stsCentricAlgoJobSource::result_type::memory;
    grPurelyTemporalExecutionResult.Exception = prg_exception(e.what(), "stsPurelyTemporal_Plus_CentricAlgoThreadFunctor");
    grPurelyTemporalExecutionResult.bExceptional = true;
  }
  catch (prg_exception & e) {
    grPurelyTemporalExecutionResult.eException_type = stsCentricAlgoJobSource::result_type::prg;
    grPurelyTemporalExecutionResult.Exception = e;
    grPurelyTemporalExecutionResult.bExceptional = true;
  }
  catch (std::exception & e) {
    grPurelyTemporalExecutionResult.eException_type = stsCentricAlgoJobSource::result_type::std;
    grPurelyTemporalExecutionResult.Exception = prg_exception(e.what(), "stsPurelyTemporal_Plus_CentricAlgoThreadFunctor");
    grPurelyTemporalExecutionResult.bExceptional = true;
  }
  catch (...) {
    grPurelyTemporalExecutionResult.eException_type = stsCentricAlgoJobSource::result_type::unknown;
    grPurelyTemporalExecutionResult.Exception = prg_exception("(...) -- unknown error", "stsPurelyTemporal_Plus_CentricAlgoThreadFunctor");
    grPurelyTemporalExecutionResult.bExceptional = true;
  }
  if (grPurelyTemporalExecutionResult.bExceptional) {
    grJobSource.Exhaust();
    grPurelyTemporalExecutionResult.Exception.addTrace("operator()", "stsPurelyTemporal_Plus_CentricAlgoThreadFunctor");
  }
  else {
    gRegularSubcontractor();
  }
}

