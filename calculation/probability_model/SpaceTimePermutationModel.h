//*****************************************************************************
#ifndef __SPACETIMEPERMUTATIONMODEL_H
#define __SPACETIMEPERMUTATIONMODEL_H
//*****************************************************************************
#include "SaTScan.h"
#include "ProbabilityModel.h"
#include "SaTScanData.h"
#include "RandomDistribution.h"
#include <string>
#include <vector>

/** Couples a time interval index with a random number.
    Used in permutating case dates for simulations. */
class CSimulationTimeRandomizer
{
  private:
   int			m_nTimeIntervalIndex;
   double		m_dRandomNumber;
   
  public:
   CSimulationTimeRandomizer(int nTimeIntervalIndex=0, double dRandomNumber=0) {SetTimeIntervalIndex(nTimeIntervalIndex);
                                                                                SetRandomNumber(dRandomNumber);}
   virtual ~CSimulationTimeRandomizer() {}

   double		GetRandomNumber() const {return m_dRandomNumber;}
   int			GetTimeIntervalIndex() const {return m_nTimeIntervalIndex;}
   void			SetRandomNumber(double d) {m_dRandomNumber=d;}
   void			SetTimeIntervalIndex(int n) {m_nTimeIntervalIndex=n;}
};

/** Function object used to compare CSimulationTimeRandomizer objects. */
class CompareSimulationTimeRandomizer {
  public:
    bool operator() (const CSimulationTimeRandomizer& lhs, const CSimulationTimeRandomizer& rhs)
           {
           return ( lhs.GetRandomNumber() < rhs.GetRandomNumber() );
           }
};

/** Couples tract index with time interval index. 
    Used in permutating case dates for simulations. */
class CCaseLocationTimes
{
  private:
   int		m_nTimeIntervalIndex;
   int		m_nTractIndex;

  public:
                CCaseLocationTimes(){}
                CCaseLocationTimes(int nTimeIntervalIndex, int nTractIndex){SetTimeIntervalIndex(nTimeIntervalIndex);
                                                                            SetTractIndex(nTractIndex);}
   virtual      ~CCaseLocationTimes(){}

   int          GetTimeIntervalIndex() const {return m_nTimeIntervalIndex;}
   int          GetTractIndex() const {return m_nTractIndex;}
   void         SetTimeIntervalIndex(int nIndex) {m_nTimeIntervalIndex = nIndex;}
   void         SetTractIndex(int nIndex) {m_nTractIndex = nIndex;}
};

/** Space-time permutation model. Requires only case and geographical information.
    Calculates loglikelihood identically to Poisson model. */
class CSpaceTimePermutationModel : public CModel
{
  private:
    std::vector<CCaseLocationTimes>             m_vCaseLocationTimes;
    std::vector<CSimulationTimeRandomizer>      m_vTimeIntervalRandomizer;

    void                        InitializeRandomizationStructures();

  public:
    CSpaceTimePermutationModel(CParameters* pParameters, CSaTScanData* pData, BasePrint *pPrintDirection);
    virtual ~CSpaceTimePermutationModel();

    virtual double 	        CalcLogLikelihood(count_t n, measure_t u);
    virtual double              CalcMonotoneLogLikelihood(const CPSMonotoneCluster& PSMCluster);
    virtual bool   	        CalculateMeasure();
    virtual double 	        GetLogLikelihoodForTotal() const;
    virtual double              GetPopulation(int m_iEllipseOffset, tract_t nCenter, tract_t nTracts,
                                              int nStartInterval, int nStopInterval);
    virtual void   	        MakeData();
    virtual bool   	        ReadData();
};

//*****************************************************************************
#endif