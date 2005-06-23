//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "SpaceTimeIncludePureCentricAnalysis.h"

/** constructor */
SpaceTimeIncludePureCentricAnalysis::SpaceTimeIncludePureCentricAnalysis(const CParameters& Parameters,
                                                                         const CSaTScanData& Data,
                                                                         BasePrint& PrintDirection,
                                                                         const AbtractDataSetGateway& RealDataGateway,
                                                                         const ZdPointerVector<AbtractDataSetGateway>& vSimDataGateways)
                                     :SpaceTimeIncludePurelySpatialCentricAnalysis(Parameters, Data, PrintDirection, RealDataGateway, vSimDataGateways) {}

/** destructor */
SpaceTimeIncludePureCentricAnalysis::~SpaceTimeIncludePureCentricAnalysis() {}

