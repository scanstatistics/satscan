//*****************************************************************************
#ifndef __WilcoxonLikelihoodCalculation_H
#define __WilcoxonLikelihoodCalculation_H
//*****************************************************************************
#include "LikelihoodCalculation.h"

/** Wilcoxon log likelihood calculator. Note that this class is currently just
    a shell. All defined functions, when called, will through exceptions.
    Waiting for further implementation details...                             */
class WilcoxonLikelihoodCalculator : public AbstractLikelihoodCalculator {
  public:
    WilcoxonLikelihoodCalculator(const CSaTScanData& Data);
    virtual ~WilcoxonLikelihoodCalculator();
};
//*****************************************************************************
#endif
