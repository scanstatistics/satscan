//******************************************************************************
#ifndef stsCentricAlgoFunctorH
#define stsCentricAlgoFunctorH
//******************************************************************************
#include <utility>
#include "SaTScan.h"
#include "AbstractCentricAnalysis.h"
#include "contractor.h"
#include "stsCentricAlgoJobSource.h"
#include "AsynchronouslyAccessible.h"
#include "ptr_vector.h"

//runs jobs for the "centric" algorithm
class stsCentricAlgoFunctor
{
public:
  typedef unsigned int param_type;
  typedef stsCentricAlgoJobSource::result_type result_type;

private:
  AbstractCentricAnalysis & grCentricAnalysis;
  CentroidNeighborCalculator & grCentroidCalculator;
  AbstractDataSetGateway const & grDataSetGateway;
  ptr_vector<AbstractDataSetGateway> const & grSimDataGateways;

public:
  stsCentricAlgoFunctor(
    AbstractCentricAnalysis & rCentricAnalysis
   ,CentroidNeighborCalculator & rCentroidCalculator
   ,AbstractDataSetGateway const & rDataSetGateway
   ,ptr_vector<AbstractDataSetGateway> const & rSimDataGateways
  )
   : grCentricAnalysis(rCentricAnalysis)
   , grCentroidCalculator(rCentroidCalculator)
   , grDataSetGateway(rDataSetGateway)
   , grSimDataGateways(rSimDataGateways)
  {
  }

//  ~stsMonteCarloSimCentricFunctor()
//  {
//  }

  result_type operator() (param_type const & param);

};




//This is a special functor that runs the execution on the purely temporal
//cluster first, then launches into the regular executions.  Only one thread
//should be assigned this functor.  The rest should be assigned
//subcontractor<contractor<stsCentricAlgoFunctor>,stsCentricAlgoFunctor> objects.
//rPurelyTemporalExecutionFailure is a reference to an object that is populated
//with the failure status of the "purely temporal cluster" execution.
class stsPurelyTemporal_Plus_CentricAlgoThreadFunctor
{
  typedef stsCentricAlgoJobSource job_source_type;
  typedef contractor<job_source_type> contractor_type;

  contractor_type & grContractor;
  job_source_type & grJobSource;
   stsCentricAlgoJobSource::result_type & grPurelyTemporalExecutionResult;
  AsynchronouslyAccessible<PrintQueue> & grPrintDirection;
  AbstractCentricAnalysis & grCentricAnalysis;
  AbstractDataSetGateway const & grDataSetGateway;
  ptr_vector<AbstractDataSetGateway> const & grSimDataGateways;
  subcontractor<contractor_type,stsCentricAlgoFunctor> gRegularSubcontractor;

public:
  stsPurelyTemporal_Plus_CentricAlgoThreadFunctor(
    contractor_type & rContractor
   ,job_source_type & rJobSource
   ,stsCentricAlgoJobSource::result_type & rPurelyTemporalExecutionResult
   ,AsynchronouslyAccessible<PrintQueue> & rPrintDirection
   ,AbstractCentricAnalysis & rCentricAnalysis
   ,CentroidNeighborCalculator & rCentroidCalculator
   ,AbstractDataSetGateway const & rDataSetGateway
   ,ptr_vector<AbstractDataSetGateway> const & rSimDataGateways
  );

  void operator() ();

};
//******************************************************************************
#endif

