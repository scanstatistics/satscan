// data.h

#include "param.h"
//#include "model.h"
#include "cats.h"
#include "tinfo.h"
#include "ginfo.h"
#include "date.h"
#include "spatscan.h" // has declarations of "count_t, measure_t
#include "baseprint.h"

#ifndef __DATA_H
#define __DATA_H

class CModel;

class CSaTScanData
{
  protected:
     BasePrint *gpPrintDirection;
  public:
    CSaTScanData(CParameters* pParameters, BasePrint *pPrintDirection);
    virtual ~CSaTScanData();

    CParameters* m_pParameters;
    CModel*      m_pModel;

    Cats *gpCats;            // DTG
    TInfo *gpTInfo;          // DTG
    GInfo *gpGInfo;          // DTG

//    tract_t     m_nTractsInput;
    tract_t     m_nTracts;             // tract_t defined as short in spatscan.h  ?Ever more than 32,000 tracts?
    tract_t     m_nGridTracts;
    short       m_nNumEllipsoids;
    double     *mdE_Angles;            //temp storage for the angles of each "possible" ellipsoid...
    double     *mdE_Shapes;            //temp storage for the shapes of each "Possible" ellipsoid...

    int       m_nTimeIntervals;
    Julian*   m_pIntervalStartTimes;    // Not nec. for purely spatial?
    Julian    m_nStartDate;
    Julian    m_nEndDate;
    int       m_nIntervalCut; // Maximum time intervals allowed in a cluster (base on TimeSize)
    int       m_nProspectiveIntervalStart; // interval where start of prospective space-time begins

    count_t   m_nTotalCasesAtStart;
    count_t   m_nTotalControlsAtStart;
    measure_t m_nTotalMeasureAtStart;
    measure_t m_nTotalTractsAtStart;

    count_t   m_nTotalCases;
    count_t   m_nTotalControls;
    double    m_nTotalPop;
    measure_t m_nTotalMeasure;

    count_t**    m_pCases;
    count_t**    m_pControls;
    count_t**    m_pSimCases;
    measure_t**  m_pMeasure;

    count_t*   m_pPTCases;
    count_t*   m_pPTSimCases;
    measure_t* m_pPTMeasure;

    tract_t  *(** m_pSortedInt);
    unsigned short  *(** m_pSortedUShort);
    tract_t  **  m_NeighborCounts;

    Julian**  m_pTimes; // Used for exact times...

    double m_nMaxCircleSize;

    virtual void ReadDataFromFiles();
    virtual bool CalculateMeasure();
    virtual void MakeData();

  protected:
//    double  m_nLogLikelihoodForTotal;
//    double  m_nAnnualRate;
    double  m_nAnnualRatePop;

    void Init();
  public:

//    virtual bool    CalculateMeasure();
    virtual bool    FindNeighbors();

    bool ReadCounts(const char* szCountFilename,
                    const char* szDescription,
                    count_t***  pCounts);
    bool ParseCountLine(const char*  szDescription, int nRec, char* szData,
                        tract_t& tid, count_t& nCount, Julian& nDate);
    bool ParseCountLineCategories(const char*   szDescription,
                                  const int     nRec,
                                        char*   szData,
                                  const int     nCats,
                                  const int     nDataElements,
                                  const tract_t tid,
                                  const count_t nCount,
                                  const Julian  nDate);

    void IncrementCount(tract_t nTID, int nCount, Julian nDate,
                        count_t** pCounts);

    void   SetPurelyTemporalSimCases();

    void   SetMaxCircleSize() {m_nMaxCircleSize = (m_pParameters->m_nMaxGeographicClusterSize / 100.0) * m_nTotalMeasure;};
    double GetMaxCircleSize() {return m_nMaxCircleSize;};

    tract_t GetNeighbor(int iEllipse, tract_t t, unsigned int nearness) const;
    double  GetAnnualRate() const;
    double  GetAnnualRateAtStart() const;
    double  GetAnnualRatePop() const {return m_nAnnualRatePop;};
    double  GetMeasureAdjustment() const;

    virtual void AllocSimCases();
    virtual void DeAllocSimCases();

    void AdjustNeighborCounts(); // For sequential analysis, after top cluster removed

    int ComputeNewCutoffInterval(Julian jStartDate, Julian jEndDate);

    virtual void DisplayCases(FILE* pFile);
    virtual void DisplayControls(FILE* pFile);
    virtual void DisplaySimCases(FILE* pFile);
    virtual void DisplayMeasure(FILE* pFile);
    virtual void DisplayNeighbors(FILE* pFile);
    virtual void DisplayRelativeRisksForEachTract(FILE* pFile);

    void DisplaySummary(FILE* fp);
    void DisplaySummary2(FILE* fp);

    void PrintNeighbors();
    
  protected:
    void SetStartAndEndDates();

    virtual void SetNumTimeIntervals();
    virtual void SetIntervalCut();
    virtual void SetIntervalStartTimes();
    void SetProspectiveIntervalStart();
    void SetPurelyTemporalCases();
    void SetPurelyTemporalMeasures();

  public:
    inline Cats *GetCats() { return gpCats; };           // DTG
    inline const TInfo *GetTInfo() const { return gpTInfo;} ;          // DTG
    inline const GInfo *GetGInfo() const { return gpGInfo;} ;
    bool ReadGrid();
    bool ReadGridLatLong();
    bool ReadGridCoords();
    bool ReadGeo();
    bool ReadGeoLatLong();
    bool ReadGeoCoords();
    bool ReadPops();
//    bool ReadCases();
//    bool ReadData();

};

#endif
