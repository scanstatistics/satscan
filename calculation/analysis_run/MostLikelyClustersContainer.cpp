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
      ZdGenerateException("Index '%d' out of range[%d-%d].","GetTopRankedCluster()", 0, m_nClustersRetained - 1);
  }
  catch (ZdException &x) {
     x.AddCallpath("GetTopRankedCluster()","MostLikelyClustersContainer");
     throw;
  }
  return *gvTopClusterList[tClusterIndex];
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
    clusterdistance <= newradius) -> all clusters such that their is no
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
   tract_t t, j;
   float newradius;                      /* Radius of new cluster tested */
   float clusterdistance;                /* Distance between the centers */
                                         /* of old and new clusters      */
   float *pRadius = 0;
   double** pCoords = 0;
   double* pCoords1 = 0, *pCoords2 = 0;
   int     i, iNumElements;
   bool bInclude;                 /* 0/1 variable put to zero when a new */
                                  /* cluster should not be incuded.      */
   tract_t m_nClustersToKeepEachPass;                                  

   try {
      //if no restrictions then need array to have m_nNumTracts number of elements
      //else just set it to NUM_RANKED (500) elements.....
      iNumElements = NUM_RANKED;
      if (Parameters.GetCriteriaSecondClustersType() == NORESTRICTIONS)
         iNumElements = DataHub.GetNumTracts();
      pRadius = new float[iNumElements];
      pCoords = (double**)Smalloc(iNumElements * sizeof(double*));
      memset(pCoords, 0, iNumElements * sizeof(double*));

      if (Parameters.GetIsSequentialScanning())
        m_nClustersToKeepEachPass = 1;
      else {
        if (Parameters.GetCriteriaSecondClustersType() == NORESTRICTIONS)
          m_nClustersToKeepEachPass = DataHub.GetNumTracts();
        else
          m_nClustersToKeepEachPass = (DataHub.m_nGridTracts <= NUM_RANKED ? DataHub.m_nGridTracts : NUM_RANKED);
      }
      /* Note: "Old clusters" are clusters already included on the list, while */
      /* a "new cluster" is the present candidate for inclusion.               */

      /* Sort by descending m_ratio, without regard to overlap */
      qsort(&gvTopClusterList[0], m_nClustersRetained, sizeof(CCluster*), CompareClustersByRatio);

      // Remove "Undefined" clusters that have been sorted to bottome of list because ratio=-DBL_MAX
      tract_t nClustersAssigned = m_nClustersRetained;
      for (tract_t k=0; k<nClustersAssigned; k++) {
        if (!gvTopClusterList[k]->ClusterDefined()) {
          m_nClustersRetained--;
          delete gvTopClusterList[k];
          gvTopClusterList[k] = NULL;
        }
      }

      if (m_nClustersRetained != 0) {
        /* Remove certain types of overlapping clusters from later printout */
        (DataHub.GetGInfo())->giGetCoords(gvTopClusterList[0]->GetCentroidIndex(), &pCoords[0]);
        (DataHub.GetTInfo())->tiGetCoords(DataHub.GetNeighbor(gvTopClusterList[0]->GetEllipseOffset(), gvTopClusterList[0]->GetCentroidIndex(), gvTopClusterList[0]->GetNumTractsInnerCircle()/*m_nTracts*/),
                    &pCoords2);

        pRadius[0] = (float)sqrt((DataHub.GetTInfo())->tiGetDistanceSq(pCoords[0],pCoords2));
        free(pCoords2);
        t=1;

        while (t < m_nClustersToKeepEachPass && gvTopClusterList[t] != NULL) {
          (DataHub.GetGInfo())->giGetCoords(gvTopClusterList[t]->GetCentroidIndex(), &pCoords1);
          (DataHub.GetTInfo())->tiGetCoords(DataHub.GetNeighbor(gvTopClusterList[t]->GetEllipseOffset(), gvTopClusterList[t]->GetCentroidIndex(), gvTopClusterList[t]->GetNumTractsInnerCircle()),
                      &pCoords2);
          newradius = (float)sqrt((DataHub.GetTInfo())->tiGetDistanceSq(pCoords1,pCoords2));
          free(pCoords2);
          bInclude=1;

          j=0;
          while (bInclude && j<t)  {
            clusterdistance = (float)sqrt((DataHub.GetTInfo())->tiGetDistanceSq(pCoords1,pCoords[j]));
            //IF ELLIPSOID RUN, THEN SET TO "NO RESTRICTIONS"
            if (Parameters.GetNumRequestedEllipses() > 0)
                bInclude = 1;
            else {
               switch (Parameters.GetCriteriaSecondClustersType()) {
                  case NOGEOOVERLAP: //no geographical overlap
                     if (clusterdistance <= (pRadius[j]+newradius))
                       bInclude=0;
                     break;
                  case NOCENTROIDSINOTHER: //no cluster centroids in other clusters
                     if((clusterdistance <= pRadius[j]) || (clusterdistance <= newradius))
                        bInclude = 0;
                     break;
                  case NOCENTROIDSINMORELIKE: //no cluster centroids in more likely clusters
                     if (clusterdistance <= pRadius[j])
                        bInclude = 0;
                     break;
                  case NOCENTROIDSINLESSLIKE: //no cluster centroids in less likely clusters
                     if(clusterdistance <= newradius)
                        bInclude = 0;
                     break;
                  case NOPAIRSINEACHOTHERS: //no pairs of centroids in each others clusters
                     if((clusterdistance <= pRadius[j]) && (clusterdistance <= newradius))
                        bInclude = 0;
                     break;
                  case NORESTRICTIONS:   //No Restrictions
                     bInclude = 1;
                     break;
                  default:  SSGenerateException("Invalid value found for Criteria for Reporting Secondary Clusters.","RankTopClusters");
               }
            }
            ++j;
          } // while bInclude

          if (bInclude) {
            pRadius[t] = newradius;
            // Now allocate new pCoords[]
            pCoords[t] = (double*)Smalloc(Parameters.GetDimensionsOfData() * sizeof(double));

            // Loop through values of pCoords1
            for (i=0; i < Parameters.GetDimensionsOfData(); i++) {
            	pCoords[t][i] = pCoords1[i];
            }
            // Don't need pCoords1 anymore
            free(pCoords1);
            ++t;
          } // if bInclude
          else {
            m_nClustersRetained--;
            delete gvTopClusterList[t];
            for (j = t; j < m_nClustersRetained; //m_nMaxClusters - 1
                 j++)
              gvTopClusterList[j] = gvTopClusterList[j + 1];
            gvTopClusterList[m_nClustersRetained] = NULL;
            free(pCoords1);
          } // if-else
        }  // while t

        m_nClustersRetained = t;

        // Delete unused clusters
        for (t = m_nClustersToKeepEachPass; t < nClustersAssigned; t++) {
          delete gvTopClusterList[t];
          gvTopClusterList[t] = 0;
        }
      }

      //Clean up pCoords allocation
      for (i=0; i<iNumElements; i++) {
      	if (pCoords[i] != NULL)
           free(pCoords[i]);
      }
      free(pCoords);
      delete [] pRadius;
      gvTopClusterList.DeleteElements(m_nClustersRetained, gvTopClusterList.size() - 1);
   }
   catch (ZdException & x) {
      //Clean up pCoords allocation
      if (pCoords) {
         for (int i=0; i<iNumElements; i++) {
            if (pCoords[i] != NULL)
              free(pCoords[i]);
         }
         free(pCoords);
      }
      delete [] pRadius;
      x.AddCallpath("RankTopClusters()", "MostLikelyClustersContainer");
      throw;
   }
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

