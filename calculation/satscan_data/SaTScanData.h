//*****************************************************************************
#ifndef __SATSCANDATA_H
#define __SATSCANDATA_H
//*****************************************************************************
#include "SaTScan.h"
#include "Tracts.h"
#include "Parameters.h"
#include "JulianDates.h"
#include "ProbabilityModel.h"
#include "PopulationCategories.h"
#include "GridTractCoordinates.h"
#include "UtilityFunctions.h"
#include "CalculateMeasure.h"
#include "MakeNeighbors.h"

class CModel;

class CSaTScanData
{
  protected:
     BasePrint  *gpPrintDirection;
     double     m_nAnnualRatePop;

     void               Init();
     void               SetStartAndEndDates();
     virtual void       SetNumTimeIntervals();
     virtual void       SetIntervalCut();
     virtual void       SetIntervalStartTimes();
     void               SetProspectiveIntervalStart();
     void               SetPurelyTemporalCases();
     void               SetPurelyTemporalMeasures();

  public:
    CSaTScanData(CParameters* pParameters, BasePrint *pPrintDirection);
    virtual ~CSaTScanData();

    CParameters* m_pParameters;
    CModel*      m_pModel;
    Cats *gpCats;            // DTG
    TInfo *gpTInfo;          // DTG
    GInfo *gpGInfo;          // DTG

    tract_t     m_nGridTracts, m_nTracts;             // tract_t defined as short in SaTScan.h ?Ever more than 32,000 tracts?
    short       m_nNumEllipsoids;
    double     *mdE_Angles, *mdE_Shapes;            //temp storage for the angles, shapes of each "possible" ellipsoid...
    int       m_nTimeIntervals;
    Julian*   m_pIntervalStartTimes;    // Not nec. for purely spatial?
    Julian    m_nStartDate, m_nEndDate;
    int       m_nIntervalCut; // Maximum time intervals allowed in a cluster (base on TimeSize)
    int       m_nProspectiveIntervalStart; // interval where start of prospective space-time begins
    count_t   m_nTotalCasesAtStart, m_nTotalControlsAtStart;
    measure_t m_nTotalMeasureAtStart, m_nTotalTractsAtStart;
    count_t   m_nTotalCases, m_nTotalControls;
    double    m_nTotalPop, m_nMaxCircleSize;
    measure_t m_nTotalMeasure;
    count_t**    m_pCases, **m_pControls, **m_pSimCases;
    measure_t**  m_pMeasure;
    count_t*   m_pPTCases, *m_pPTSimCases;
    measure_t* m_pPTMeasure;
    tract_t  *(** m_pSortedInt);
    unsigned short  *(** m_pSortedUShort);
    tract_t  **  m_NeighborCounts;
    Julian**  m_pTimes; // Used for exact times...

    virtual void AllocSimCases();
    virtual bool CalculateMeasure();
    virtual void DeAllocSimCases();
    virtual bool FindNeighbors();
    virtual void MakeData(int iSimulationNumber);
    virtual void ReadDataFromFiles();

    virtual void DisplayCases(FILE* pFile);
    virtual void DisplayControls(FILE* pFile);
    virtual void DisplayMeasure(FILE* pFile);
    virtual void DisplayNeighbors(FILE* pFile);
    virtual void DisplayRelativeRisksForEachTract(FILE* pFile);
    virtual void DisplaySimCases(FILE* pFile);


    void        AdjustNeighborCounts(); // For sequential analysis, after top cluster removed
    inline Cats *GetCats() { return gpCats; };           // DTG
    int         ComputeNewCutoffInterval(Julian jStartDate, Julian jEndDate);
    void        DisplaySummary(FILE* fp);
    void        DisplaySummary2(FILE* fp);

    double      GetAnnualRate() const;
    double      GetAnnualRateAtStart() const;
    double      GetAnnualRatePop() const {return m_nAnnualRatePop;};
    double      GetMaxCircleSize() {return m_nMaxCircleSize;};
    double      GetMeasureAdjustment() const;
    tract_t     GetNeighbor(int iEllipse, tract_t t, unsigned int nearness) const;
    inline const TInfo *GetTInfo() const { return gpTInfo;} ;          // DTG
    inline const GInfo *GetGInfo() const { return gpGInfo;} ;

    void        IncrementCount(tract_t nTID, int nCount, Julian nDate, count_t** pCounts);
    bool        ParseCountLine(const char*  szDescription, int nRec, char* szData, tract_t& tid, count_t& nCount, Julian& nDate);
    bool        ParseCountLineCategories(const char*   szDescription, const int     nRec, char*   szData, const int     nCats,
                                  const int     nDataElements, const tract_t tid, const count_t nCount, const Julian  nDate);
    void        PrintNeighbors();

    bool        ReadCounts(const char* szCountFilename, const char* szDescription, count_t***  pCounts);
    bool        ReadGrid();
    bool        ReadGridLatLong();
    bool        ReadGridCoords();
    bool        ReadGeo();
    bool        ReadGeoLatLong();
    bool        ReadGeoCoords();
    bool        ReadPops();

    void        SetPurelyTemporalSimCases();
    void        SetMaxCircleSize();
};

//*****************************************************************************
#endif
