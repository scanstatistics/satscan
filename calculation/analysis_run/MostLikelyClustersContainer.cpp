//***************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//***************************************************************************
#include "MostLikelyClustersContainer.h"
#include "SaTScanData.h"

MostLikelyClustersContainer::MostLikelyClustersContainer() {
  Init();
}

MostLikelyClustersContainer::~MostLikelyClustersContainer() {}


void MostLikelyClustersContainer::Add(const CCluster& Cluster) {
  gvTopClusterList.push_back(0);
  gvTopClusterList.back() = Cluster.Clone();
  m_nClustersRetained++;
}

/** Comparison function for sorting clusters by descending m_ratio */
int MostLikelyClustersContainer::CompareClustersByRatio(const void *a, const void *b) {
  double rdif = (*(CCluster**)b)->m_nRatio - (*(CCluster**)a)->m_nRatio;
  if (rdif < 0.0)   return -1;
  if (rdif > 0.0)   return 1;
  return 0;
}

/** */
void MostLikelyClustersContainer::Empty() {
  gvTopClusterList.DeleteAllElements();
  m_nClustersRetained=0;
}

/** */
const CCluster& MostLikelyClustersContainer::GetCluster(tract_t tClusterIndex) const {
  try {
    if (tClusterIndex < 0 || tClusterIndex > m_nClustersRetained - 1)
      ZdGenerateException("Index '%d' out of range[%d-%d].","GetCluster()", 0, m_nClustersRetained - 1);
  }
  catch (ZdException &x) {
     x.AddCallpath("GetCluster()","MostLikelyClustersContainer");
     throw;
  }
  return *gvTopClusterList[tClusterIndex];
}

/**
According to information contained in 'DataHub', what is the radius of 'theCluster'?
*/
double MostLikelyClustersContainer::GetClusterRadius(const CSaTScanData& DataHub, CCluster const & theCluster) {
  double dResult;
  double * pCoordsOfCluster = 0;
  double * pCoordsOfNeighborCluster = 0;
  try {
    DataHub.GetGInfo()->giGetCoords(theCluster.GetCentroidIndex(), &pCoordsOfCluster);
    DataHub.GetTInfo()->tiGetCoords(
      DataHub.GetNeighbor(theCluster.GetEllipseOffset() ,theCluster.GetCentroidIndex() ,theCluster.GetNumTractsInnerCircle())
     ,&pCoordsOfNeighborCluster
    );
    dResult = std::sqrt(DataHub.GetTInfo()->tiGetDistanceSq(pCoordsOfCluster,pCoordsOfNeighborCluster));
    free(pCoordsOfCluster);
    free(pCoordsOfNeighborCluster);
  }
  catch (ZdException &x) {
    free(pCoordsOfCluster);
    free(pCoordsOfNeighborCluster);

    x.AddCallpath("GetClusterRadius()","MostLikelyClustersContainer");
    throw;
  }
  return dResult;
}

/** */
const CCluster& MostLikelyClustersContainer::GetTopRankedCluster() const {
  try {
    if (m_nClustersRetained == 0)
      ZdGenerateException("No clusters in container.","GetTopRankedCluster()");
  }
  catch (ZdException &x) {
     x.AddCallpath("GetTopRankedCluster()","MostLikelyClustersContainer");
     throw;
  }
  return *gvTopClusterList[0];
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

void MostLikelyClustersContainer::PrintTopClusters(const char * sFilename, int nHowMany) {
   FILE* pFile;

   try {
      if ((pFile = fopen(sFilename, "w")) == NULL)
        SSGenerateException("  Error: Unable to open top clusters file.\n", "PrintTopClusters()");
      else {
         nHowMany = std::min(m_nClustersRetained, nHowMany);
         for (int i = 0; i < nHowMany; ++i) {
           fprintf(pFile, "GridTract:  %i\n", i);
           fprintf(pFile, "  Ellipe Offset:  %i\n", gvTopClusterList[i]->GetEllipseOffset());
           fprintf(pFile, "         Center:  %i\n", gvTopClusterList[i]->GetCentroidIndex());
           fprintf(pFile, "        Measure:  %f\n", gvTopClusterList[i]->GetMeasure(0));      //measure_t
           fprintf(pFile, "         Tracts:  %i\n", gvTopClusterList[i]->GetNumTractsInnerCircle());
           fprintf(pFile, "LikelihoodRatio:  %f\n", gvTopClusterList[i]->m_nRatio);
           fprintf(pFile, "           Rank:  %u\n", gvTopClusterList[i]->GetRank());
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

/**********************************************************************
 Ranks the top "NumKept" clusters, in descending cd_ratio, such that
 no ranked cluster overlaps a higher-ranked cluster.
 Ratio for TopClusters[NumKept .. NumClusters-1] is set to -DBL_MAX.
 Note: It is easy to modify the selection of clusters to be included
 by making changes in the "if(...) INCLUDE=0" line. For example:
 if(clusterdistance <= radius[j]) -> all clusters with centers not included
                                     in a higher cluster on the list.
 if(clusterdistance <= newradius) -> all clusters not containing the center
                                     of a higher cluster on the list.
 if(clusterdistance <= radius[j] &&
    clusterdistance <= newradius) -> all clusters such that there is no
                                     cluster pair both of which contain
                                     the center point of the other.
 if(clusterdistance <= radius[j] &&
    clusterdistance <= newradius) -> all clusters such that no center point
                                     is contained in any other cluster with
                                     lower or higher rank.
 if(clusterdistance <= radius[j]+
                       newradius) -> all non-overlapping clusters
 Parameters:
   TopClusters[]  - clusters to rank
   NumClusters    - length of TopClusters
   NumKept        - number to rank (<= NumClusters)
//
 **********************************************************************/
void MostLikelyClustersContainer::RankTopClusters(const CParameters& Parameters, const CSaTScanData& DataHub) {
   tract_t nClustersToKeepEachPass;

   try {
     if (DataHub.GetTInfo()->tiGetDimensions() < 2)
       ZdException::Generate("This function written for at least two (2) dimensions.", "MostLikelyClustersContainer");

      if (Parameters.GetIsSequentialScanning())
        nClustersToKeepEachPass = 1;
      else {
        if (Parameters.GetCriteriaSecondClustersType() == NORESTRICTIONS)
          nClustersToKeepEachPass = DataHub.GetNumTracts();
        else
          nClustersToKeepEachPass = (DataHub.m_nGridTracts <= NUM_RANKED ? DataHub.m_nGridTracts : NUM_RANKED);
      }
      /* Note: "Old clusters" are clusters already included on the list, while */
      /* a "new cluster" is the present candidate for inclusion.               */

      /* Sort by descending m_ratio, without regard to overlap */
      qsort(&gvTopClusterList[0], m_nClustersRetained, sizeof(CCluster*), CompareClustersByRatio);

      // Remove "Undefined" clusters that have been sorted to bottom of list because ratio=-DBL_MAX
      for (unsigned u=0; u < gvTopClusterList.size(); ) {
        if (gvTopClusterList.at(u)->ClusterDefined()) {
          ++u;//skip this cluster
        }
        else {
          gvTopClusterList.DeleteElement(u);
        }
      }

      if (gvTopClusterList.size() > 0) {
        CriteriaSecondaryClustersType eClusterInclusionCriterion = Parameters.GetCriteriaSecondClustersType();
        std::vector<CCluster *> vRetainedClusters;

        /* Remove certain types of overlapping clusters from later printout */
        for (ZdPointerVector<CCluster>::iterator itrCurr=gvTopClusterList.begin(), itrEnd = gvTopClusterList.end(); (vRetainedClusters.size() < (unsigned)nClustersToKeepEachPass) && (itrCurr != itrEnd); ++itrCurr) {
          if (ShouldRetainCandidateCluster(vRetainedClusters, **itrCurr, DataHub, eClusterInclusionCriterion)) {
            //transfer ownership of cluster to vRetainedClusters:
            vRetainedClusters.push_back(*itrCurr);
            *itrCurr = 0;
          }
        }
        gvTopClusterList.DeleteAllElements();
        gvTopClusterList.resize(vRetainedClusters.size());
        std::copy(vRetainedClusters.begin(), vRetainedClusters.end(), gvTopClusterList.begin());
        m_nClustersRetained = gvTopClusterList.size();
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
bool MostLikelyClustersContainer::ShouldRetainCandidateCluster(std::vector<CCluster *> const & vRetainedClusters, CCluster const & CandidateCluster, const CSaTScanData& DataHub, CriteriaSecondaryClustersType eCriterion)
{
  bool bResult;
//  std::deque<stsEllipseAreaDescriptor>::const_iterator itrCurrDescriptor(theDescriptorList.begin());
//  std::deque<stsEllipseAreaDescriptor>::const_iterator itrEnd(theDescriptorList.end());

  try {
    stsClusterCentroidGeometry CandidateCenter(DataHub.GetGInfo()->giGetCoords(CandidateCluster.GetCentroidIndex()));
    //validate conditions:
    switch (eCriterion) {
      case NOGEOOVERLAP: {//no geographical overlap
        if (CandidateCluster.GetEllipseOffset() > 0)
          ZdException::Generate("criterion \"NOGEOOVERLAP\" is not supported for non-circular clusters", "MostLikelyClustersContainer");
      }
      break;
      case NOCENTROIDSINOTHER: //no cluster centroids in any other clusters
      case NOCENTROIDSINMORELIKE: //no cluster centroids in more likely clusters
      case NOCENTROIDSINLESSLIKE: //no cluster centroids in less likely clusters
      case NOPAIRSINEACHOTHERS: //no pairs of centroids in each others clusters
        if ((CandidateCluster.GetEllipseOffset() > 0) && (CandidateCenter.GetDimensionCount() > 2))
          ZdException::Generate("For ellipses, cannot have more than 2 dimensions (got %d).", "MostLikelyClustersContainer", CandidateCenter.GetDimensionCount());
      break;
      case NORESTRICTIONS:   //No Restrictions
      break;
      default:  SSGenerateException("Invalid value found for Criteria for Reporting Secondary Clusters.","MostLikelyClustersContainer");
    }

    double dCandidateRadius = GetClusterRadius(DataHub, CandidateCluster);
    bResult = true;
    for (std::vector<CCluster*>::const_iterator itrCurr = vRetainedClusters.begin(), itrEnd = vRetainedClusters.end(); bResult && (itrCurr != itrEnd); ++itrCurr) {
      CCluster const & currCluster = **itrCurr;
      stsClusterCentroidGeometry currCenter(DataHub.GetGInfo()->giGetCoords(currCluster.GetCentroidIndex()));
      double dCurrRadius = GetClusterRadius(DataHub, currCluster);

      switch (eCriterion) {
        case NOGEOOVERLAP: {//no geographical overlap
//----------------------------------float_stuff
//These lines of code exist to force identical behavior to the previous code.
//Comment them out and uncomment the double_stuff when you want higher precision.
          float fDistance = (float)CandidateCenter.DistanceTo(currCenter);
          float fRadiiSum = (float)dCurrRadius + (float)dCandidateRadius;
          bResult = !(fDistance <= fRadiiSum);
//----------------------------------double_stuff
//          bResult = CandidateCenter.DistanceTo(currCenter) > (dCurrRadius + dCandidateRadius);
        }
        break;
        case NOCENTROIDSINOTHER: {//no cluster centroids in any other clusters
          if ((CandidateCenter.GetDimensionCount() > 2) || (CandidateCluster.GetEllipseOffset() == 0))
            bResult = !(CentroidLiesWithinSphereRegion(CandidateCenter, currCenter, dCurrRadius) || CentroidLiesWithinSphereRegion(currCenter, CandidateCenter, dCandidateRadius));
          else
            bResult = !(
              PointLiesWithinEllipseArea(CandidateCenter.GetCoordinates().at(0), CandidateCenter.GetCoordinates().at(1), currCenter.GetCoordinates().at(0), currCenter.GetCoordinates().at(1), dCurrRadius, DataHub.GetAnglesArray()[currCluster.GetEllipseOffset() - 1], DataHub.GetShapesArray()[currCluster.GetEllipseOffset() - 1])
             || PointLiesWithinEllipseArea(currCenter.GetCoordinates().at(0), currCenter.GetCoordinates().at(1), CandidateCenter.GetCoordinates().at(0), CandidateCenter.GetCoordinates().at(1), dCandidateRadius, DataHub.GetAnglesArray()[CandidateCluster.GetEllipseOffset() - 1], DataHub.GetShapesArray()[CandidateCluster.GetEllipseOffset() - 1])
            );
        }
        break;
        case NOCENTROIDSINMORELIKE: {//no cluster centroids in more likely clusters
          if ((CandidateCenter.GetDimensionCount() > 2) || (CandidateCluster.GetEllipseOffset() == 0))
            bResult = !(CentroidLiesWithinSphereRegion(CandidateCenter, currCenter, dCurrRadius));
          else
            bResult = !(PointLiesWithinEllipseArea(CandidateCenter.GetCoordinates().at(0), CandidateCenter.GetCoordinates().at(1), currCenter.GetCoordinates().at(0), currCenter.GetCoordinates().at(1), dCurrRadius, DataHub.GetAnglesArray()[currCluster.GetEllipseOffset() - 1], DataHub.GetShapesArray()[currCluster.GetEllipseOffset() - 1]));
        }
        break;
        case NOCENTROIDSINLESSLIKE: {//no cluster centroids in less likely clusters
          if ((CandidateCenter.GetDimensionCount() > 2) || (CandidateCluster.GetEllipseOffset() == 0))
            bResult = !(CentroidLiesWithinSphereRegion(currCenter, CandidateCenter, dCandidateRadius));
          else
            bResult = !(PointLiesWithinEllipseArea(currCenter.GetCoordinates().at(0), currCenter.GetCoordinates().at(1), CandidateCenter.GetCoordinates().at(0), CandidateCenter.GetCoordinates().at(1), dCandidateRadius, DataHub.GetAnglesArray()[CandidateCluster.GetEllipseOffset() - 1], DataHub.GetShapesArray()[CandidateCluster.GetEllipseOffset() - 1]));
        }
        break;
        case NOPAIRSINEACHOTHERS: {//no pairs of centroids in each others clusters
          //(if either center is not in the other cluster then bResult=true)
          if ((CandidateCenter.GetDimensionCount() > 2) || (CandidateCluster.GetEllipseOffset() == 0))
            bResult = !CentroidLiesWithinSphereRegion(CandidateCenter, currCenter, dCurrRadius) || !CentroidLiesWithinSphereRegion(currCenter, CandidateCenter, dCandidateRadius);
          else
            bResult =
              !PointLiesWithinEllipseArea(CandidateCenter.GetCoordinates().at(0), CandidateCenter.GetCoordinates().at(1), currCenter.GetCoordinates().at(0), currCenter.GetCoordinates().at(1), dCurrRadius, DataHub.GetAnglesArray()[currCluster.GetEllipseOffset() - 1], DataHub.GetShapesArray()[currCluster.GetEllipseOffset() - 1])
             || !PointLiesWithinEllipseArea(currCenter.GetCoordinates().at(0), currCenter.GetCoordinates().at(1), CandidateCenter.GetCoordinates().at(0), CandidateCenter.GetCoordinates().at(1), dCandidateRadius, DataHub.GetAnglesArray()[CandidateCluster.GetEllipseOffset() - 1], DataHub.GetShapesArray()[CandidateCluster.GetEllipseOffset() - 1])
            ;
        }
        break;
        case NORESTRICTIONS:   //No Restrictions
          bResult = true;
        break;
        default:  SSGenerateException("Invalid value found for Criteria for Reporting Secondary Clusters.","MostLikelyClustersContainer");
      }
    } // while bResult
  }
  catch (ZdException & e) {
     e.AddCallpath("ShouldRetainCandidateCluster()", "MostLikelyClustersContainer");
     throw;
  }
  return bResult;
}

void MostLikelyClustersContainer::SortTopClusters() {
  qsort(&gvTopClusterList[0], m_nClustersRetained, sizeof(CCluster*), CompareClustersByRatio);
}

/** Updates rank of top clusters by comparing simulated loglikelihood ratio(LLR)
    with each remaining clusters LLR. If the analysis contains ellipses and is
    duczmal correcting their LLRs, then the duczmal corrected LLR is is used
    to rank the clusters instead of LLR. */
void MostLikelyClustersContainer::UpdateTopClustersRank(double r) {
  int   i;

  for (i=m_nClustersRetained-1; i >= 0; i--) {
     if (gvTopClusterList[i]->m_nRatio > r)
       break;
     gvTopClusterList[i]->IncrementRank();
  }
}

