//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "SaTScanData.h"
#include "SSException.h"
#include "IntermediateClustersContainer.h"
#include "SVTTCluster.h"

/* reset all CCluster objects to center about centroid 'iCenter' -- calls CCluster::Initialize(int) */
void CClusterSetCollections::resetSVTT(int iCenter, const AbstractDataSetGateway& DataGateway) {
    for (size_t shape=0; shape < _clusterSetCollections.size(); ++shape) {
        CClusterSet& clusterSet = _clusterSetCollections[shape];
        for (size_t maxStopIndex=0; maxStopIndex < clusterSet.numInSet(); ++maxStopIndex) {
            CSVTTCluster * pCluster = dynamic_cast<CSVTTCluster*>(&clusterSet.get(maxStopIndex).getCluster());
            if (pCluster) pCluster->InitializeSVTT(iCenter, DataGateway);
        }
    }
}
