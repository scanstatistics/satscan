//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "SpaceTimeIncludePurelyTemporalCentricAnalysis.h"

/** constructor */
SpaceTimeIncludePurelyTemporalCentricAnalysis::SpaceTimeIncludePurelyTemporalCentricAnalysis(const CParameters& Parameters,
                                                   const CSaTScanData& Data,
                                                   BasePrint& PrintDirection,
                                                   const AbstractDataSetGateway& RealDataGateway,
                                                   const DataSetGatewayContainer_t& vSimDataGateways)
                         :SpaceTimeCentricAnalysis(Parameters, Data, PrintDirection, RealDataGateway, vSimDataGateways) {}

/** destructor */
SpaceTimeIncludePurelyTemporalCentricAnalysis::~SpaceTimeIncludePurelyTemporalCentricAnalysis() {}

