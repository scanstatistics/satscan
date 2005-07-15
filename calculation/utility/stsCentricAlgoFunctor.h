//---------------------------------------------------------------------------

#ifndef stsCentricAlgoFunctorH
#define stsCentricAlgoFunctorH
//---------------------------------------------------------------------------
#include <utility>
#include "SaTScan.h"
#include "AbstractCentricAnalysis.h"
#include "contractor.h"
#include "stsCentricAlgoJobSource.h"
#include "AsynchronouslyAccessible.h"




//runs jobs for the "centric" algorithm
class stsCentricAlgoFunctor
{
public:
  typedef unsigned int param_type;
  typedef std::pair<bool, ZdException> result_type;

private:
  AbstractCentricAnalysis & grCentricAnalysis;
  CentroidNeighborCalculator & grCentroidCalculator;
  AbstractDataSetGateway const & grDataSetGateway;
  ZdPointerVector<AbstractDataSetGateway> const & grSimDataGateways;

public:
  stsCentricAlgoFunctor(
    AbstractCentricAnalysis & rCentricAnalysis
   ,CentroidNeighborCalculator & rCentroidCalculator
   ,AbstractDataSetGateway const & rDataSetGateway
   ,ZdPointerVector<AbstractDataSetGateway> const & rSimDataGateways
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
class stsPurelyTemporal_Plus_CentricAlgoThreadFunctor
{
  typedef contractor<stsCentricAlgoJobSource> contractor_type;

  contractor_type & grContractor;
//  AsynchronouslyAccessible<BasePrint> & grPrintDirection;
  AsynchronouslyAccessible<PrintQueue> & grPrintDirection;
  AbstractCentricAnalysis & grCentricAnalysis;
  AbstractDataSetGateway const & grDataSetGateway;
  ZdPointerVector<AbstractDataSetGateway> const & grSimDataGateways;
  subcontractor<contractor_type,stsCentricAlgoFunctor> gRegularSubcontractor;

public:
  stsPurelyTemporal_Plus_CentricAlgoThreadFunctor(
    contractor_type & rContractor
//   ,AsynchronouslyAccessible<BasePrint> & rPrintDirection
   ,AsynchronouslyAccessible<PrintQueue> & rPrintDirection
   ,AbstractCentricAnalysis & rCentricAnalysis
   ,CentroidNeighborCalculator & rCentroidCalculator
   ,AbstractDataSetGateway const & rDataSetGateway
   ,ZdPointerVector<AbstractDataSetGateway> const & rSimDataGateways
  );

  void operator() ();

};




#endif

