/** constructor */
template <class T>
IntermediateClustersContainer<T>::IntermediateClustersContainer(const CSaTScanData& Data)
                                 :gData(Data){}

/** Returns reference to top cluster for shape offset. This function is tightly
    coupled to the presumption that CAnalysis::GetTopCluster() iterates over
    circles and ellispes in such a way that an offset of zero is a circle, and
    anything greater offset corresponds to an particular rotation of an ellipse shape.
    NOTE: Caller should never assume that reference will remain after return from
          get CAnalysis::GetTopCluster(), as the vector is reset for each iteration
          of GetTopClusters(). Instead, a copy should be made, using operator= method
          and appropriate casting, to store cluster information. */
template <class T>           
T & IntermediateClustersContainer<T>::GetTopCluster(int iShapeOffset) {
  int   iEllipse, iBoundry=0;

  if (iShapeOffset && gData.GetParameters().GetNumRequestedEllipses() && gData.GetParameters().GetNonCompactnessPenaltyType() != NOPENALTY)
    for (iEllipse=0; iEllipse < gData.GetParameters().GetNumRequestedEllipses(); ++iEllipse) {
       //Get the number of angles this ellipse shape rotates through.
       iBoundry += gData.GetParameters().GetEllipseRotations()[iEllipse];
       if (iShapeOffset <= iBoundry)
         return gvClusters[iEllipse + 1];
    }

  return gvClusters[0];
}

/** Returns the top cluster for all shapes, taking into account the option of
    non-compactness penalizing for ellispes.
    NOTE: Caller should never assume that reference will remain after return from
          get CAnalysis::GetTopCluster(), as the vector is reset for each iteration
          of GetTopClusters(). Instead a copy should be made, using operator= method
          and appropriate casting, to store cluster information. 
    NOTE: This function should only be called after all iterations in
          CAnalysis::GetTopCluster() are completed. */
template <class T>           
T & IntermediateClustersContainer<T>::GetTopCluster() {
  //set the maximum cluster to the circle shape initially
  T& TopCluster = gvClusters[0];
  //apply compactness correction
  TopCluster.m_nRatio *= TopCluster.GetNonCompactnessPenalty();
  //if the there are ellipses, compare current top cluster against them
  //note: we don't have to be concerned with whether we are comparing circles and ellipses,
  //     the adjusted loglikelihood ratio for a circle is just the loglikelihood ratio
  for (size_t t=1; t < gvClusters.size(); t++) {
     if (gvClusters[t].ClusterDefined()) {
       //apply compactness correction
       gvClusters[t].m_nRatio *= gvClusters[t].GetNonCompactnessPenalty();
       //compare against current top cluster
       if (gvClusters[t].m_nRatio > TopCluster.m_nRatio)
         TopCluster = gvClusters[t];
     }
  }
  return TopCluster;
}

template <class T>
void IntermediateClustersContainer<T>::Reset(int iCenter) {
  for (size_t t=0; t < gvClusters.size(); ++t)
     gvClusters[t].Initialize(iCenter);
}

/** Initialzies the vector of top clusters to cloned copies of cluster,
    taking into account whether spatial shape will be a factor in analysis. */
template <class T>
void IntermediateClustersContainer<T>::SetTopClusters(const T& Cluster) {
  int   i, iNumTopClusters;

  try {
    gvClusters.clear();
    //if there are ellipses and a non-compactness penalty, then we need a top cluster for the circle and each ellipse shape
    if (gData.GetParameters().GetNumRequestedEllipses() && gData.GetParameters().GetNonCompactnessPenaltyType() != NOPENALTY)
      iNumTopClusters = gData.GetParameters().GetNumRequestedEllipses() + 1;
    else
    //else there is only one top cluster - regardless of whether there are ellipses
      iNumTopClusters = 1;

    for (i=0; i < iNumTopClusters; i++)
       gvClusters.push_back(Cluster);
  }
  catch (ZdException &x) {
    x.AddCallpath("SetTopClusters()","IntermediateClustersContainer");
    throw;
  }
}

