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
class CSimulationTimeRandomizer {
  private:
   int			giTimeIntervalIndex;
   float		gfRandomNumber;
   
  public:
   CSimulationTimeRandomizer(int iTimeIntervalIndex=0, float fRandomNumber=0) {SetTimeIntervalIndex(iTimeIntervalIndex);
                                                                                SetRandomNumber(fRandomNumber);}
   virtual ~CSimulationTimeRandomizer() {}

   float		GetRandomNumber() const {return gfRandomNumber;}
   int			GetTimeIntervalIndex() const {return giTimeIntervalIndex;}
   void			SetRandomNumber(float f) {gfRandomNumber=f;}
   void			SetTimeIntervalIndex(int n) {giTimeIntervalIndex=n;}
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
class CCaseLocationTimes {
  private:
   int		giTimeIntervalIndex;
   int		giTractIndex;

  public:
                CCaseLocationTimes(){}
                CCaseLocationTimes(int iTimeIntervalIndex, int iTractIndex){SetTimeIntervalIndex(iTimeIntervalIndex);
                                                                            SetTractIndex(iTractIndex);}
   virtual      ~CCaseLocationTimes(){}

   int          GetTimeIntervalIndex() const {return giTimeIntervalIndex;}
   int          GetTractIndex() const {return giTractIndex;}
   void         SetTimeIntervalIndex(int iIndex) {giTimeIntervalIndex = iIndex;}
   void         SetTractIndex(int iIndex) {giTractIndex = iIndex;}
};

class SpaceTimeRandomizer {
  private:
    std::vector<std::vector<CCaseLocationTimes> >       gvCategoryCaseLocationTimes;
    std::vector<CSimulationTimeRandomizer>              gvTimeIntervalRandomizer;
    RandomNumberGenerator                               gRandomNumberGenerator;
  
  public:
    SpaceTimeRandomizer() {}
    virtual ~SpaceTimeRandomizer() {}

    void        InitializeStructures(DataStream & thisStream, int iTimeIntervals, int iTracts);
    void        MakeData(int iSimulationNumber, DataStreamInterface & DataInterface);    
};

/** Space-time permutation model. Requires only case and geographical information.
    Calculates loglikelihood identically to Poisson model. */
class CSpaceTimePermutationModel : public CModel {
  private:
    std::vector<SpaceTimeRandomizer>                    gvRandomizers;

    std::vector<std::vector<CCaseLocationTimes> >       gvCategoryCaseLocationTimes;
    std::vector<CSimulationTimeRandomizer>              gvTimeIntervalRandomizer;
    RandomNumberGenerator                               gRandomNumberGenerator;

    void                        InitializeRandomizationStructures();

  public:
    CSpaceTimePermutationModel(CParameters& Parameters, CSaTScanData& Data, BasePrint& PrintDirection);
    virtual ~CSpaceTimePermutationModel();

    virtual double 	        CalcLogLikelihood(count_t n, measure_t u);
    virtual double              CalcLogLikelihoodRatio(count_t tCases, measure_t tMeasure, count_t tTotalCases, measure_t tTotalMeasure, double dCompactnessCorrection);
    virtual bool   	        CalculateMeasure(DataStream & thisStream);
    virtual double 	        GetLogLikelihoodForTotal() const;
    virtual double              GetPopulation(int m_iEllipseOffset, tract_t nCenter, tract_t nTracts,
                                              int nStartInterval, int nStopInterval);
    virtual void   	        MakeData(int iSimulationNumber, DataStreamInterface & DataInterface, unsigned int tInterface=0);
    virtual bool   	        ReadData();
};

//*****************************************************************************
#endif