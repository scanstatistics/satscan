//---------------------------------------------------------------------------

#include "SaTScan.h"
#pragma hdrstop

#include "stsCentricAlgoFunctor.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)


stsCentricAlgoFunctor::result_type stsCentricAlgoFunctor::operator() (param_type const & param)
{
  result_type temp_result;
  temp_result.first = false;
  try
  {
    grCentricAnalysis.ExecuteAboutCentroid(param-1, grCentroidCalculator, grDataSetGateway, grSimDataGateways);
//    if (param == 8)
//      throw ZdException("exception thrown, job id: %i", "stsMCSimSuccessiveFunctor", ZdException::Normal, param);
//    if ((89 <= param) && (param <= 98)) {
//      ZdString sTemp;
//      sTemp.printf("exception thrown, job id: %i", param);
//      throw std::runtime_error(sTemp.GetCString());
//    }
//    if (param == 8)
//      throw param_type(param);
  }
  catch (ZdException & e)
  {
    temp_result.second = e;
    temp_result.first = true;
  }
  catch (std::exception & e) {
    temp_result.second = ZdException(e, "", "stsCentricAlgoFunctor");
    temp_result.first = true;
  }
  catch (...) {
    temp_result.second = ZdException("(...) -- unknown error", "stsCentricAlgoFunctor", ZdException::Normal);
    temp_result.first = true;
  }
  if (temp_result.first) {
    temp_result.second.AddCallpath("operator()", "stsCentricAlgoFunctor");
  }
//  if (param == 8)
//    throw ZdException("exception thrown, job id: %i", "stsCentricAlgoFunctor", ZdException::Normal, param);
  return temp_result;
}



stsPurelyTemporal_Plus_CentricAlgoThreadFunctor::stsPurelyTemporal_Plus_CentricAlgoThreadFunctor(
  contractor_type & rContractor
 ,job_source_type & rJobSource
 ,std::pair<bool,ZdException> & rPurelyTemporalExecutionResult
// ,AsynchronouslyAccessible<BasePrint> & rPrintDirection
 ,AsynchronouslyAccessible<PrintQueue> & rPrintDirection
 ,AbstractCentricAnalysis & rCentricAnalysis
 ,CentroidNeighborCalculator & rCentroidCalculator
 ,AbstractDataSetGateway const & rDataSetGateway
 ,ZdPointerVector<AbstractDataSetGateway> const & rSimDataGateways
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
  rPurelyTemporalExecutionResult.first = false;
}

void stsPurelyTemporal_Plus_CentricAlgoThreadFunctor::operator() ()
{
  try {
    grPrintDirection.Locked().Value().SatScanPrintf("Evaluating purely temporal clusters\n");
    grCentricAnalysis.ExecuteAboutPurelyTemporalCluster(grDataSetGateway, grSimDataGateways);
  }
  catch (ZdException & e) {
    grPurelyTemporalExecutionResult.second = e;
    grPurelyTemporalExecutionResult.first = true;
  }
  catch (std::exception & e) {
    grPurelyTemporalExecutionResult.second = ZdException(e, "", "stsPurelyTemporal_Plus_CentricAlgoThreadFunctor");
    grPurelyTemporalExecutionResult.first = true;
  }
  catch (...) {
    grPurelyTemporalExecutionResult.second = ZdException("(...) -- unknown error", "stsPurelyTemporal_Plus_CentricAlgoThreadFunctor", ZdException::Normal);
    grPurelyTemporalExecutionResult.first = true;
  }
  if (grPurelyTemporalExecutionResult.first) {
    grJobSource.Exhaust();
    grPurelyTemporalExecutionResult.second.AddCallpath("operator()", "stsPurelyTemporal_Plus_CentricAlgoThreadFunctor");
  }
  else {
    gRegularSubcontractor();
  }
}

