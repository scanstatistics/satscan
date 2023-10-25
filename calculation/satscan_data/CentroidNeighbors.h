//******************************************************************************
#ifndef CentroidNeighborsH
#define CentroidNeighborsH
//******************************************************************************
#include "Tracts.h"

/** Distance from a centroid to a neighboring location. */
class DistanceToCentroid {
   private:
     tract_t m_tTractNumber;
     double m_dDistance;
     unsigned int m_RelativeCoordinateIndex; /* relative index into tracts respective coordinates container */

   public:
     DistanceToCentroid(tract_t t=0, double d=0, unsigned int i=0) {Set(t, d, i);}
     virtual ~DistanceToCentroid() {}

     double             GetDistance() const {return m_dDistance;}
     unsigned int       GetRelativeCoordinateIndex() const {return m_RelativeCoordinateIndex;}
     tract_t            GetTractNumber() const {return m_tTractNumber;}
     void               Set(tract_t t, double d, unsigned int i) {m_tTractNumber=t;m_dDistance=d;m_RelativeCoordinateIndex=i;}
};

/** Function object used to compare DistanceToCentroid. */
class CompareIdentifierDistance {
private:
	const IdentifiersManager & _identifier_mgr;

public:
	CompareIdentifierDistance(const IdentifiersManager & identifierMgr) : _identifier_mgr(identifierMgr) {}

	bool operator() (const DistanceToCentroid& lhs, const DistanceToCentroid& rhs);
};

class CSaTScanData; /** forward class declaration */

/** Interface which details calculated neighbor indexes about a specific centroid. */
class CentroidNeighbors {
  friend class CentroidNeighborCalculator;
  private:
    tract_t                       gtCentroid;                             /* centroid index */
    tract_t                       gtEllipseOffset;                        /* ellipse index */
    int                           giNeighbors;                            /* neighbor count - variable returned in get method */
    int                           giMaxNeighbors;                         /* neighbor count - variable which defines max for replication process */
    int                           giMaxReportedNeighbors;                 /* neighbor count - variable whch defines maximum for evaluation of real data */
    std::vector<tract_t>          gvMaxReportedNeighbors;                 /* conditionally allocated integer vector of max report neighbor counts */
    std::vector<tract_t>          gvSortedNeighborsIntegerType;           /* conditionally allocated integer vector of neighbor indexes */
    std::vector<unsigned short>   gvSortedNeighborsUnsignedShortType;     /* conditionally allocated unsigned short vector of neighbors indexes */
    tract_t                    ** gppSortedNeighborsIntegerType;          /* pointer to integer array of neighbors indexes */
    unsigned short             ** gppSortedNeighborsUnsignedShortType;    /* pointer to unsigned short array of neighbors indexes */
    tract_t                     * gpNeighborArray;
    tract_t                     * gpSortedNeighborsIntegerType;           /* pointer to integer array of neighbors indexes */
    unsigned short              * gpSortedNeighborsUnsignedShortType;     /* pointer to unsigned short array of neighbors indexes */

    void                          Set(tract_t tEllipseOffset, tract_t tCentroid, int iNumNeighbors, const std::vector<tract_t>& maxReportedNeighbors, const std::vector<DistanceToCentroid>& vOrderedLocations);

  public:
    CentroidNeighbors();
    CentroidNeighbors(tract_t tEllipseOffset, const CSaTScanData& DataHub, tract_t tCentroid=-1);
    ~CentroidNeighbors();

    tract_t                     GetEllipseIndex() const {return gtEllipseOffset;}
    tract_t                     GetCentroidIndex() const {return gtCentroid;}
    inline tract_t              GetNeighborTractIndex(tract_t tNeighborIndex) const;
    inline int                  GetNumNeighbors() const {return giNeighbors;}
    const std::vector<tract_t>& getMaxReportedNeighbors() const {return gvMaxReportedNeighbors;}
    tract_t                   * GetRawIntegerArray() const {return gpSortedNeighborsIntegerType;}
    unsigned short            * GetRawUnsignedShortArray() const {return gpSortedNeighborsUnsignedShortType;}
    inline void                 Set(tract_t tCentroid);
    void                        SetMaximumClusterSize_RealData() {giNeighbors = giMaxReportedNeighbors;}
    void                        SetMaximumClusterSize_SimulatedData() {giNeighbors = giMaxNeighbors;}
};

/** Returns zero based tNeighborIndex'th nearest neighbor's tract index.
    Caller is responsible for ensuring that internal structures have been previously
    set and 'tNeighborIndex' is within zero based range for defined neighbor information. */
inline tract_t CentroidNeighbors::GetNeighborTractIndex(tract_t tNeighborIndex) const {
 //assert(tNeighborIndex + 1 <= (size_t)giNeighbors && (gpSortedNeighborsUnsignedShortType || gpSortedNeighborsIntegerType));
 return (gpSortedNeighborsUnsignedShortType ? (tract_t)gpSortedNeighborsUnsignedShortType[tNeighborIndex] : gpSortedNeighborsIntegerType[tNeighborIndex]);
}

/** Sets class members to define locations about centroid index / ellipse index.
    The neigbor information referenced is that of the 'sorted' array, so caller is
    responsible for ensuring that:
        1) sorted array is allocated and contains calculated neighbors about centroids
        2) tEllipseOffset and tCentroid are valid indexes into sorted array
        3) sorted array will persist until this object is destructed
    Returns reference to self.*/
inline void CentroidNeighbors::Set(tract_t tCentroid) {
  gtCentroid = tCentroid;
  giNeighbors = giMaxNeighbors = giMaxReportedNeighbors = gpNeighborArray[tCentroid];

  if (gppSortedNeighborsIntegerType)
    gpSortedNeighborsIntegerType = gppSortedNeighborsIntegerType[tCentroid];
  else
    gpSortedNeighborsUnsignedShortType = gppSortedNeighborsUnsignedShortType[tCentroid];
}
//******************************************************************************
#endif

