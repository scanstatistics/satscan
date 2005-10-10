//******************************************************************************
#ifndef __SPACETIMERANDOMIZER_H
#define __SPACETIMERANDOMIZER_H
//******************************************************************************
#include "PermutationDataRandomizer.h"

/** class representing a premutated attribute which is a time interval. */
class PermutedTime : public PermutedAttribute {
  protected:
    int			         giTimeIntervalIndex;

  public:
    PermutedTime(int iTimeInterval);
    virtual ~PermutedTime();

    virtual PermutedTime       * Clone() const;

    inline int 		         GetTimeInterval() const {return giTimeIntervalIndex;}
};

class DataSet; /** forward class declaration */

/** Randomizes data of dataset for a 'space-time permutation' probablility model. */
class SpaceTimeRandomizer : public AbstractPermutedDataRandomizer {
  public:
    typedef std::vector<tract_t>                StationaryContainer_t;
    typedef std::vector<PermutedTime>           PermutedContainer_t;

  class CategoryGrouping {
    public:
      StationaryContainer_t	                gvStationaryAttribute;
      PermutedContainer_t                       gvOriginalPermutedAttribute;
      PermutedContainer_t                       gvPermutedAttribute;
  };
  typedef std::vector<CategoryGrouping>         CategoryContainer_t;

  protected:
    CategoryContainer_t              gCategoryAttributes;

    virtual void                     AssignRandomizedData(const RealDataSet& thisRealSet, SimDataSet& thisSimSet);
    virtual void                     SortPermutedAttribute();

  public:
    	    SpaceTimeRandomizer(long lInitialSeed=RandomNumberGenerator::glDefaultSeed);
    virtual ~SpaceTimeRandomizer();

    virtual SpaceTimeRandomizer    * Clone() const;

    void                             CreateRandomizationData(const RealDataSet& thisRealSet);
};
//******************************************************************************
#endif

