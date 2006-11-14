//******************************************************************************
#ifndef __IntermediateClustersContainer_H
#define __IntermediateClustersContainer_H
//******************************************************************************
/** Container class to store top clusters for a spatial analysis during
    method - GetTopCluster(). This class stores the top cluster for a
    shape, circle and ellipses, inorder to postpone determination of top
    cluster by adjusted loglikelihood ratio until all possibilities
    have been calculated and ranked by loglikelihood. For most analyses, the
    shape will not be be a factor, but when penalizing for non-compactness,
    the top cluster for circles and each ellipse shape will have be retained
    until all other calculations have been completed for each iteration of
    function. */

class CSaTScanData; /** forward class declaration */

template <class T>
class IntermediateClustersContainer {
  private:
    const CSaTScanData                & gData;
    std::vector<T>                      gvClusters;

  public:
    IntermediateClustersContainer(const CSaTScanData& Data);
    ~IntermediateClustersContainer() {}

    T                                 & GetTopCluster(int iShapeOffset);
    T                                 & GetTopCluster();
    void                                Reset(int iCenter);
    void                                SetTopClusters(const T& Cluster);
};
#include "IntermediateClustersContainer.hpp"
//******************************************************************************
#endif
