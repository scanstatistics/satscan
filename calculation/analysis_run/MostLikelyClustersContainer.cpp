//***************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//***************************************************************************
#include "MostLikelyClustersContainer.h"
#include "SaTScanData.h"
#include "SSException.h" 

unsigned long MostLikelyClustersContainer::MAX_RANKED_CLUSTERS  = 500;

/** constructor */
MostLikelyClustersContainer::MostLikelyClustersContainer() {}

/** destructor */
MostLikelyClustersContainer::~MostLikelyClustersContainer() {}

/** Adds clone of passed cluster object to list of top clusters. */
void MostLikelyClustersContainer::Add(const CCluster& Cluster) {
  if (Cluster.ClusterDefined()) {
    gvTopClusterList.push_back(0);
    gvTopClusterList.back() = Cluster.Clone();
  }
}

/** Adds cluster object to list of top clusters, taking ownership. */
void MostLikelyClustersContainer::Add(std::auto_ptr<CCluster>& pCluster) {
  if (pCluster.get() && pCluster->ClusterDefined())
    gvTopClusterList.push_back(pCluster.release());
}

//Does the point at 'theCentroid' lie within the spherical region described by
//'theCircleCentroid' and 'dCircleRadius'?
bool MostLikelyClustersContainer::CentroidLiesWithinSphereRegion(stsClusterCentroidGeometry const & theCentroid, stsClusterCentroidGeometry const & theSphereCentroid, double dSphereRadius) {
  bool bResult = false;
  try {
//----------------------------------float_stuff
//These lines of code exist to force identical behavior to the previous code.
//Comment them out and uncomment the double_stuff when you want higher precision.
    float fDistance = (float)theSphereCentroid.DistanceTo(theCentroid);
    float fSphereRadius = (float)dSphereRadius;
    bResult = fDistance <= fSphereRadius;
//----------------------------------double_stuff
//    bResult = theSphereCentroid.DistanceTo(theCentroid) <= dSphereRadius;
  }
  catch (ZdException &x) {
     x.AddCallpath("CentroidLiesWithinSphereRegion()","MostLikelyClustersContainer");
     throw;
  }
  return bResult;
}

/** Removes all cluster objects from list. */
void MostLikelyClustersContainer::Empty() {
  gvTopClusterList.DeleteAllElements();
}

/** Returns reference to cluster object at passed index. Throws exception if index
    is out of range.*/
const CCluster& MostLikelyClustersContainer::GetCluster(tract_t tClusterIndex) const {
  try {
    if (tClusterIndex < 0 || (unsigned int)tClusterIndex > gvTopClusterList.size() - 1)
      ZdGenerateException("Index '%d' out of range[%d-%d].","GetCluster()", 0, gvTopClusterList.size() - 1);
  }
  catch (ZdException &x) {
     x.AddCallpath("GetCluster()","MostLikelyClustersContainer");
     throw;
  }
  return *gvTopClusterList[tClusterIndex];
}

/** According to information contained in 'DataHub', what is the radius of 'theCluster'? */
double MostLikelyClustersContainer::GetClusterRadius(const CSaTScanData& DataHub, CCluster const & theCluster) {
  double dResult;
  std::vector<double> vCoordsOfCluster;
  std::vector<double> vCoordsOfNeighborCluster;
  
  try {
    if (theCluster.GetRadiusDefined())
      return theCluster.GetCartesianRadius(); //return radius already calculated
    else {
      DataHub.GetGInfo()->giRetrieveCoords(theCluster.GetCentroidIndex(), vCoordsOfCluster);
      DataHub.GetTInfo()->tiRetrieveCoords(DataHub.GetNeighbor(theCluster.GetEllipseOffset(),
                                                               theCluster.GetCentroidIndex(),
                                                               theCluster.GetNumTractsInnerCircle()),
                                                               vCoordsOfNeighborCluster);
      dResult = std::sqrt(DataHub.GetTInfo()->tiGetDistanceSq(vCoordsOfCluster, vCoordsOfNeighborCluster));
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("GetClusterRadius()","MostLikelyClustersContainer");
    throw;
  }
  return dResult;
}

/** Returns constant reference to most likely cluster. Throws exception if
    there are no clusters to return. */
const CCluster& MostLikelyClustersContainer::GetTopRankedCluster() const {
  try {
    if (gvTopClusterList.size() == 0)
      ZdGenerateException("No clusters in container.","GetTopRankedCluster()");
  }
  catch (ZdException &x) {
    x.AddCallpath("GetTopRankedCluster()","MostLikelyClustersContainer");
    throw;
  }
  return *gvTopClusterList[0];
}

/** Returns indication of whether geographical overlap exists between passed
    clusters by examining tract locations to comprise the cluster.*/
bool MostLikelyClustersContainer::HasTractsInCommon(const CSaTScanData& DataHub, const CCluster& ClusterOne, const CCluster& ClusterTwo) {
  tract_t       t, v, tTract,
                tTwoNumTracts = ClusterTwo.GetNumTractsInnerCircle(),
                tOneNumTracts = ClusterOne.GetNumTractsInnerCircle(),
                tTwoCentroid = ClusterTwo.GetCentroidIndex(), tOneCentroid = ClusterOne.GetCentroidIndex();
  int           iTwoOffset = ClusterTwo.GetEllipseOffset(), iOneOffset = ClusterOne.GetEllipseOffset();

  if (ClusterOne.GetRadiusDefined() && ClusterTwo.GetRadiusDefined()) {
    //if certain relationships exist between clusters, we don't need to actually
    //compare each location in clusters -- these two 'shortcuts' are meant to allow
    //possible determination of overlap knowing only centroids and previously calculated
    //radii (centric analyses).
    std::vector<double> vClusterOneCoords, vClusterTwoCoords;
    DataHub.GetGInfo()->giRetrieveCoords(ClusterOne.GetCentroidIndex(), vClusterOneCoords);
    DataHub.GetGInfo()->giRetrieveCoords(ClusterTwo.GetCentroidIndex(), vClusterTwoCoords);
    double dDistanceBetween = stsClusterCentroidGeometry(vClusterOneCoords).DistanceTo(stsClusterCentroidGeometry(vClusterTwoCoords));
    //we can say for certain that they don't have tracts in common if their circles don't overlap
    if (dDistanceBetween > ClusterOne.GetCartesianRadius() + ClusterTwo.GetCartesianRadius())
      return false;
    //we can say that they do overlap if the centroid of second cluster is within radius of first cluster
    //or vice versa, centroid of first cluster is within radius of second cluster
    if (dDistanceBetween <= ClusterOne.GetCartesianRadius() || dDistanceBetween <= ClusterTwo.GetCartesianRadius()) {
      //if neighbors for secondard centroid where re-calculated, we can delete this data now
      DataHub.FreeNeighborInfo(tTwoCentroid);
      return true;
    }
  }

  for (t=1; t <= tTwoNumTracts; ++t) {
     tTract = DataHub.GetNeighbor(iTwoOffset, tTwoCentroid, t, ClusterTwo.GetCartesianRadius());
     for (v=1; v <= tOneNumTracts; ++v) {
        if (tTract == DataHub.GetNeighbor(iOneOffset, tOneCentroid, v, ClusterOne.GetCartesianRadius())) {
          //if neighbors for secondard centroid where re-calculated, we can delete this data now
          DataHub.FreeNeighborInfo(tTwoCentroid);
          return true;
        }
     }
  }  
  return false;
}

//Does the point at (dXPoint, dYPoint) lie within the two-dimensional region
//covered by the ellipse described by (dXEllipseCenter, dYEllipseCenter,
//'dEllipseRadius', 'dEllipseAngle', 'dEllipseShape')?
//require
// non_negative_radius: dEllipseRadius >= 0.0
// valid_shape: dEllipseShape >= 1.0
bool MostLikelyClustersContainer::PointLiesWithinEllipseArea(double dXPoint, double dYPoint, double dXEllipseCenter, double dYEllipseCenter, double dEllipseRadius, double dEllipseAngle, double dEllipseShape) {
  bool bResult = false;
  try {
    //assume dEllipseAngle is in radians
    double c = dEllipseRadius * std::sqrt(std::pow(dEllipseShape, 2) - 1);
    double dXFocus1 = dXEllipseCenter + (c * std::cos(dEllipseAngle));
    double dYFocus1 = dYEllipseCenter + (c * std::sin(dEllipseAngle));
    double dXFocus2 = dXEllipseCenter - (c * std::cos(dEllipseAngle));
    double dYFocus2 = dYEllipseCenter - (c * std::sin(dEllipseAngle));
    double dDistance1 = std::sqrt( std::pow(dXPoint - dXFocus1, 2) + std::pow(dYPoint - dYFocus1, 2) );
    double dDistance2 = std::sqrt( std::pow(dXPoint - dXFocus2, 2) + std::pow(dYPoint - dYFocus2, 2) );

//----------------------------------float_stuff
//These lines of code exist to force fewer digits of precision (and even floats
//may preserve too many).  Comment them out and uncomment the double_stuff when
//you want even higher precision.
    float fDistance1 = (float)dDistance1;
    float fDistance2 = (float)dDistance2;
    float fEllipseRadius = (float)dEllipseRadius;
    float fEllipseShape = (float)dEllipseShape;
    bResult = (fDistance1 + fDistance2) <= (2 * fEllipseRadius * fEllipseShape);
//----------------------------------double_stuff
//    bResult = (dDistance1 + dDistance2) <= (2 * dEllipseRadius * dEllipseShape);
  }
  catch (ZdException &x) {
     x.AddCallpath("PointLiesWithinEllipseArea()","MostLikelyClustersContainer");
     throw;
  }
  return bResult;
}

/** Prints properties of cluster objects in top cluster list to ASCII file. */
void MostLikelyClustersContainer::PrintTopClusters(const char * sFilename, const CSaTScanData& DataHub) {
   FILE* pFile;

   try {
      if ((pFile = fopen(sFilename, "w")) == NULL)
        GenerateResolvableException("  Error: Unable to open top clusters file.\n", "PrintTopClusters()");
      else {
        for (size_t i=0; i < gvTopClusterList.size(); ++i) {
          fprintf(pFile, "GridTract:  %i\n", i);
          fprintf(pFile, "  Ellipe Offset:  %i\n", gvTopClusterList[i]->GetEllipseOffset());
          fprintf(pFile, "         Center:  %i\n", gvTopClusterList[i]->GetCentroidIndex());
          fprintf(pFile, "        Measure:  %f\n", gvTopClusterList[i]->GetExpectedCount(DataHub));
          fprintf(pFile, "         Tracts:  %i\n", gvTopClusterList[i]->GetNumTractsInnerCircle());
          fprintf(pFile, "LikelihoodRatio:  %f\n", gvTopClusterList[i]->m_nRatio);
          fprintf(pFile, "           Rank:  %u\n", gvTopClusterList[i]->GetRank());
          fprintf(pFile, "   Cart. Radius:  %lf\n", gvTopClusterList[i]->GetCartesianRadius());
          fprintf(pFile, " \n");
          fprintf(pFile, " \n");
        }
      }
      fclose(pFile);
   }
  catch (ZdException &x) {
    fclose(pFile);
    x.AddCallpath("PrintTopClusters()","MostLikelyClustersContainer");
    throw;
  }
}

/** Reorders list of top cluster objects by sorting by loglikelihood ratio in
    descending order. Clusters that are not 'defined' are removed from top
    clusters list. Retained clusters of list are further restricted by:
    for sequential scan
      - only most likely cluster retained
    reporting secondary clusters, no restrictions
      - limit list size to the number of tract locations
    reporting secondary clusters, all other
      - limit list size to minimum(number of grid points, MAX_RANKED_CLUSTERS)
      - remove clusters based upon specified criteria
    Note that this function should not be called with cluster list containing
    purely temporal clusters. The ranking performed is based soley on geographical
    orientation.   */
void MostLikelyClustersContainer::RankTopClusters(const CParameters& Parameters, const CSaTScanData& DataHub, BasePrint& gPrintDirection) {
   unsigned long                        uClustersToKeepEachPass;
   ZdPointerVector<CCluster>::iterator  itrCurr, itrEnd;
   CriteriaSecondaryClustersType        eClusterInclusionCriterion = Parameters.GetCriteriaSecondClustersType();

   try {
     if (DataHub.GetTInfo()->tiGetDimensions() < 2)
       ZdException::Generate("This function written for at least two (2) dimensions.", "MostLikelyClustersContainer");
     //return from function if no clusters retained
     if (!gvTopClusterList.size()) return;
     //determine maximum number of clusters to retain
     if (Parameters.GetIsSequentialScanning())
       uClustersToKeepEachPass = 1;
     else if (eClusterInclusionCriterion == NORESTRICTIONS)
       uClustersToKeepEachPass = static_cast<unsigned long>(DataHub.GetNumTracts());
     else
       uClustersToKeepEachPass = std::min(static_cast<unsigned long>(DataHub.m_nGridTracts), MAX_RANKED_CLUSTERS);
     //sort by descending m_ratio
     std::sort(gvTopClusterList.begin(), gvTopClusterList.end(), CompareClustersRatios());
     if (eClusterInclusionCriterion != NORESTRICTIONS)
       gPrintDirection.Printf("Checking the Overlapping Nature of Clusters\n", BasePrint::P_STDOUT);
     //remove geographically overlapping clusters
     if (gvTopClusterList.size() > 0) {
       std::vector<CCluster *> vRetainedClusters;
       size_t                  tNumSpatialRetained=0; 
       for (itrCurr=gvTopClusterList.begin(),itrEnd=gvTopClusterList.end(); (tNumSpatialRetained < uClustersToKeepEachPass) && (itrCurr != itrEnd); ++itrCurr) {
          if (ShouldRetainCandidateCluster(vRetainedClusters, **itrCurr, DataHub, eClusterInclusionCriterion)) {
            //transfer ownership of cluster to vRetainedClusters
            vRetainedClusters.push_back(*itrCurr);
            //since previous version had it so that the purely temporal cluster was added after
            //this ranking process, we don't want to consider a purely temporal cluster as
            //part of the retained list when determining whether list is at maximum size
            if ((*itrCurr)->GetClusterType() != PURELYTEMPORALCLUSTER) ++tNumSpatialRetained;
            *itrCurr = 0;
          }
       }
       gvTopClusterList.DeleteAllElements();
       gvTopClusterList.resize(vRetainedClusters.size());
       std::copy(vRetainedClusters.begin(), vRetainedClusters.end(), gvTopClusterList.begin());
     }
   }
   catch (ZdException & x) {
     x.AddCallpath("RankTopClusters()", "MostLikelyClustersContainer");
     throw;
   }
}

//Given an index (into gvTopClusterList) of a candidate cluster and a supporting
//collection of information ('DataHub'), determine whether or not the candidate
//cluster should be retained according to a criterion.
//The clusters in gvTopClusterList before 'uCandidateIndex' are assumed to be
//"more likely" than the candidate.
//If the criterion is NOGEOOVERLAP then none of the clusters may be non-circular.
//require
//
bool MostLikelyClustersContainer::ShouldRetainCandidateCluster(std::vector<CCluster *> const & vRetainedClusters, CCluster const & CandidateCluster, const CSaTScanData& DataHub, CriteriaSecondaryClustersType eCriterion) {
  bool                                          bResult=true;
  double                                        dCandidateRadius, dCurrRadius;
  std::vector<double>                           vCandidateCenterCoords, vCurrCenterCoords;
  std::vector<CCluster*>::const_iterator        itrCurr, itrEnd;

  try {
    //criteria with no restrictions includes all clusters
    if (eCriterion == NORESTRICTIONS)
      return true;
    //this function currently discriminates by geographical orientation only - so candiddate cluster
    //can not be purely temporal 
    if (CandidateCluster.GetClusterType() == PURELYTEMPORALCLUSTER)
      //always keep purely temporal cluster - we don't apply geographical overlap for these clusters
      return true;

    DataHub.GetGInfo()->giRetrieveCoords(CandidateCluster.GetCentroidIndex(), vCandidateCenterCoords);
    stsClusterCentroidGeometry CandidateCenter(vCandidateCenterCoords);
    //validate conditions:
    switch (eCriterion) {
      case NOCENTROIDSINOTHER: //no cluster centroids in any other clusters
      case NOCENTROIDSINMORELIKE: //no cluster centroids in more likely clusters
      case NOCENTROIDSINLESSLIKE: //no cluster centroids in less likely clusters
      case NOPAIRSINEACHOTHERS: //no pairs of centroids in each others clusters
        if ((CandidateCluster.GetEllipseOffset() > 0) && (CandidateCenter.GetDimensionCount() > 2))
          ZdException::Generate("For ellipses, cannot have more than 2 dimensions (got %d).", "MostLikelyClustersContainer", CandidateCenter.GetDimensionCount());
      break;
      case NOGEOOVERLAP: break; //no geographical overlap
      default:  ZdGenerateException("Unknown Criteria for Reporting Secondary Clusters, '%d'.","MostLikelyClustersContainer", eCriterion);
    }

    dCandidateRadius = GetClusterRadius(DataHub, CandidateCluster);
    for (itrCurr=vRetainedClusters.begin(), itrEnd=vRetainedClusters.end(); bResult && (itrCurr != itrEnd); ++itrCurr) {
      if ((*itrCurr)->GetClusterType() == PURELYTEMPORALCLUSTER)
        //skip comparison against retained purely temporal cluster - can't compare
        continue;
      if (eCriterion == NOGEOOVERLAP)
        bResult = !HasTractsInCommon(DataHub, **itrCurr, CandidateCluster);
      else {
        CCluster const & currCluster = **itrCurr;
        DataHub.GetGInfo()->giRetrieveCoords(currCluster.GetCentroidIndex(), vCurrCenterCoords);
        stsClusterCentroidGeometry currCenter(vCurrCenterCoords);
        dCurrRadius = GetClusterRadius(DataHub, currCluster);
        switch (eCriterion) {
          case NOCENTROIDSINOTHER: {//no cluster centroids in any other clusters
            if (CandidateCluster.GetEllipseOffset() > 0) {
              if (currCluster.GetEllipseOffset() > 0) {//both are ellipses
                bResult = !(
                  PointLiesWithinEllipseArea(CandidateCenter.GetCoordinates()[0], CandidateCenter.GetCoordinates()[1], currCenter.GetCoordinates()[0], currCenter.GetCoordinates()[1], dCurrRadius, DataHub.GetEllipseAngle(currCluster.GetEllipseOffset()), DataHub.GetEllipseShape(currCluster.GetEllipseOffset()))
                 || PointLiesWithinEllipseArea(currCenter.GetCoordinates()[0], currCenter.GetCoordinates()[1], CandidateCenter.GetCoordinates()[0], CandidateCenter.GetCoordinates()[1], dCandidateRadius, DataHub.GetEllipseAngle(CandidateCluster.GetEllipseOffset()), DataHub.GetEllipseShape(CandidateCluster.GetEllipseOffset()))
                );
              }
              else {//candidate is ellipse, curr is circle
                bResult = !(
                  CentroidLiesWithinSphereRegion(CandidateCenter, currCenter, dCurrRadius)
                 || PointLiesWithinEllipseArea(currCenter.GetCoordinates()[0], currCenter.GetCoordinates()[1], CandidateCenter.GetCoordinates()[0], CandidateCenter.GetCoordinates()[1], dCandidateRadius, DataHub.GetEllipseAngle(CandidateCluster.GetEllipseOffset()), DataHub.GetEllipseShape(CandidateCluster.GetEllipseOffset()))
                );
              }
            }
            else {
              if (currCluster.GetEllipseOffset() > 0) {//candidate is circle, curr is ellipse
                bResult = !(
                  PointLiesWithinEllipseArea(CandidateCenter.GetCoordinates()[0], CandidateCenter.GetCoordinates()[1], currCenter.GetCoordinates()[0], currCenter.GetCoordinates()[1], dCurrRadius, DataHub.GetEllipseAngle(currCluster.GetEllipseOffset()), DataHub.GetEllipseShape(currCluster.GetEllipseOffset()))
                 || CentroidLiesWithinSphereRegion(currCenter, CandidateCenter, dCandidateRadius)
                );
              }
              else {//both are circles
                bResult = !(CentroidLiesWithinSphereRegion(CandidateCenter, currCenter, dCurrRadius) || CentroidLiesWithinSphereRegion(currCenter, CandidateCenter, dCandidateRadius));
              }
            }
          }
          break;
          case NOCENTROIDSINMORELIKE: {//no cluster centroids in more likely clusters
            if (currCluster.GetEllipseOffset() == 0)
              bResult = !CentroidLiesWithinSphereRegion(CandidateCenter, currCenter, dCurrRadius);
            else
              bResult = !(PointLiesWithinEllipseArea(CandidateCenter.GetCoordinates()[0], CandidateCenter.GetCoordinates()[1], currCenter.GetCoordinates()[0], currCenter.GetCoordinates()[1], dCurrRadius, DataHub.GetEllipseAngle(currCluster.GetEllipseOffset()), DataHub.GetEllipseShape(currCluster.GetEllipseOffset())));
          }
          break;
          case NOCENTROIDSINLESSLIKE: {//no cluster centroids in less likely clusters
            if (CandidateCluster.GetEllipseOffset() == 0)
              bResult = !CentroidLiesWithinSphereRegion(currCenter, CandidateCenter, dCandidateRadius);
            else
              bResult = !PointLiesWithinEllipseArea(currCenter.GetCoordinates()[0], currCenter.GetCoordinates()[1], CandidateCenter.GetCoordinates()[0], CandidateCenter.GetCoordinates()[1], dCandidateRadius, DataHub.GetEllipseAngle(CandidateCluster.GetEllipseOffset()), DataHub.GetEllipseShape(CandidateCluster.GetEllipseOffset()));
          }
          break;
          case NOPAIRSINEACHOTHERS: {//no pairs of centroids in each others clusters
            //(if either center is not in the other cluster then bResult=true)
            if (CandidateCluster.GetEllipseOffset() > 0) {
              if (currCluster.GetEllipseOffset() > 0) {//both are ellipses
                bResult =
                  !PointLiesWithinEllipseArea(CandidateCenter.GetCoordinates()[0], CandidateCenter.GetCoordinates()[1], currCenter.GetCoordinates()[0], currCenter.GetCoordinates()[1], dCurrRadius, DataHub.GetEllipseAngle(currCluster.GetEllipseOffset()), DataHub.GetEllipseShape(currCluster.GetEllipseOffset()))
                 || !PointLiesWithinEllipseArea(currCenter.GetCoordinates()[0], currCenter.GetCoordinates()[1], CandidateCenter.GetCoordinates()[0], CandidateCenter.GetCoordinates()[1], dCandidateRadius, DataHub.GetEllipseAngle(CandidateCluster.GetEllipseOffset()), DataHub.GetEllipseShape(CandidateCluster.GetEllipseOffset()))
                ;
              }
              else {//candidate is ellipse, curr is circle
                bResult =
                  !CentroidLiesWithinSphereRegion(CandidateCenter, currCenter, dCurrRadius)
                 || !PointLiesWithinEllipseArea(currCenter.GetCoordinates()[0], currCenter.GetCoordinates()[1], CandidateCenter.GetCoordinates()[0], CandidateCenter.GetCoordinates()[1], dCandidateRadius, DataHub.GetEllipseAngle(CandidateCluster.GetEllipseOffset()), DataHub.GetEllipseShape(CandidateCluster.GetEllipseOffset()))
                ;
             }
            }
            else {
              if (currCluster.GetEllipseOffset() > 0) {//candidate is circle, curr is ellipse
                bResult =
                  !PointLiesWithinEllipseArea(CandidateCenter.GetCoordinates()[0], CandidateCenter.GetCoordinates()[1], currCenter.GetCoordinates()[0], currCenter.GetCoordinates()[1], dCurrRadius, DataHub.GetEllipseAngle(currCluster.GetEllipseOffset()), DataHub.GetEllipseShape(currCluster.GetEllipseOffset()))
                 || !CentroidLiesWithinSphereRegion(currCenter, CandidateCenter, dCandidateRadius)
                ;
              }
              else {//both are circles
                bResult = !CentroidLiesWithinSphereRegion(CandidateCenter, currCenter, dCurrRadius) || !CentroidLiesWithinSphereRegion(currCenter, CandidateCenter, dCandidateRadius);
              }
            }
          }
          break;
          default:  ZdGenerateException("Unknown Criteria for Reporting Secondary Clusters '%d'.","MostLikelyClustersContainer", eCriterion);
        }
      }
    } // for bResult
  }
  catch (ZdException & e) {
     e.AddCallpath("ShouldRetainCandidateCluster()", "MostLikelyClustersContainer");
     throw;
  }
  return bResult;
}

/** Updates rank of top clusters by comparing simulated loglikelihood ratio(LLR)
    with each remaining clusters LLR.  */
void MostLikelyClustersContainer::UpdateTopClustersRank(double r) {
  ZdPointerVector<CCluster>::reverse_iterator rev(gvTopClusterList.end());
  ZdPointerVector<CCluster>::reverse_iterator rev_end(gvTopClusterList.begin());

  for (; rev != rev_end; rev++) {
     if ((*rev)->GetRatio() > r)
        break;
     (*rev)->IncrementRank();
   }
}

