// data.h

#include "param.h"
//#include "model.h"
#include "date.h"
#include "spatscan.h" // has declarations of "count_t, measure_t

#ifndef __DATA_H
#define __DATA_H

class CModel;

class CSaTScanData
{
  public:
    CSaTScanData(CParameters* pParameters);
    virtual ~CSaTScanData();

    CParameters* m_pParameters;
    CModel*      m_pModel;

//    tract_t     m_nTractsInput;
    tract_t     m_nTracts;                // tract_t defined as short in spatscan.h  ?Ever more than 32,000 tracts?
    tract_t     m_nGridTracts;

    int       m_nTimeIntervals;
    Julian*   m_pIntervalStartTimes;    // Not nec. for purely spatial?
    Julian    m_nStartDate;
    Julian    m_nEndDate;
    int       m_nIntervalCut; // Maximum time intervals allowed in a cluster (base on TimeSize)

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

    tract_t** m_pSorted;
    tract_t*  m_NeighborCounts;

    Julian**  m_pTimes; // Used for exact times...

    double m_nMaxCircleSize;

    virtual void ReadDataFromFiles();
    virtual bool CalculateMeasure();
    virtual void MakeData();

  protected:
//    double  m_nLogLikelihoodForTotal;
//    double  m_nAnnualRate;
    double  m_nAnnualRatePop;

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

    tract_t GetNeighbor(tract_t t, unsigned int nearness) const;
    double  GetAnnualRate() const;
    double  GetAnnualRateAtStart() const;
    double  GetAnnualRatePop() const {return m_nAnnualRatePop;};
    double  GetMeasureAdjustment() const;

    virtual void AllocSimCases();
    virtual void DeAllocSimCases();

    void AdjustNeighborCounts(); // For sequential analysis, after top cluster removed

    virtual void DisplayCases(FILE* pFile);
    virtual void DisplayControls(FILE* pFile);
    virtual void DisplaySimCases(FILE* pFile);
    virtual void DisplayMeasure(FILE* pFile);
    virtual void DisplayNeighbors(FILE* pFile);
    virtual void DisplayRelativeRisksForEachTract(FILE* pFile);

    void DisplaySummary(FILE* fp);
    void DisplaySummary2(FILE* fp);

  protected:
    void SetStartAndEndDates();

    virtual void SetNumTimeIntervals();
    virtual void SetIntervalCut();
    virtual void SetIntervalStartTimes();

    void SetPurelyTemporalCases();
    void SetPurelyTemporalMeasures();

  public:
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
