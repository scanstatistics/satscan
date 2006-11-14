//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "SpaceTimeIncludePureCentricAnalysis.h"

/** constructor */
SpaceTimeIncludePureCentricAnalysis::SpaceTimeIncludePureCentricAnalysis(const CParameters& Parameters,
                                                                         const CSaTScanData& Data,
                                                                         BasePrint& PrintDirection,
                                                                         const AbstractDataSetGateway& RealDataGateway,
                                                                         const ptr_vector<AbstractDataSetGateway>& vSimDataGateways)
                                     :SpaceTimeIncludePurelySpatialCentricAnalysis(Parameters, Data, PrintDirection, RealDataGateway, vSimDataGateways) {}

/** destructor */
SpaceTimeIncludePureCentricAnalysis::~SpaceTimeIncludePureCentricAnalysis() {}

