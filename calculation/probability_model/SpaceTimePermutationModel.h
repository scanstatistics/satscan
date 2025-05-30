//******************************************************************************
#ifndef __SPACETIMEPERMUTATIONMODEL_H
#define __SPACETIMEPERMUTATIONMODEL_H
//******************************************************************************
#include "SaTScan.h"
#include "ProbabilityModel.h"
#include "SaTScanData.h"
#include "RandomDistribution.h"
#include <string>
#include <vector>

/** Space-time permutation model. Requires only case and geographical information.
    Calculates loglikelihood identically to Poisson model. */
class CSpaceTimePermutationModel : public CModel {
  public:
    CSpaceTimePermutationModel();
    virtual ~CSpaceTimePermutationModel();

    virtual void CalculateMeasure(RealDataSet& DataSet, const CSaTScanData& DataHub);
};

//******************************************************************************
#endif
