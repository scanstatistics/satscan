//---------------------------------------------------------------------------
#include "SaTScan.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#include "WilcoxonLikelihoodCalculation.h"

/** constructor */
WilcoxonLikelihoodCalculator::WilcoxonLikelihoodCalculator(const CSaTScanData& Data)
                              :AbstractLikelihoodCalculator(Data) {}

/** destructor */
WilcoxonLikelihoodCalculator::~WilcoxonLikelihoodCalculator() {}

