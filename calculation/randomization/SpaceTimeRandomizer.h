//---------------------------------------------------------------------------
#ifndef __SPACETIMERANDOMIZER_H
#define __SPACETIMERANDOMIZER_H
//---------------------------------------------------------------------------
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

class DataStream; /** forward class declaration */

/** Randomizes data of data stream for a 'space-time permutation' probablility model. */
class SpaceTimeRandomizer : public AbstractPermutedDataRandomizer {
  class CategoryGrouping {
    public:
      std::vector<tract_t>	     gvStationaryAttribute;
      ZdPointerVector<PermutedTime>  gvPermutedAttribute;
  };
  protected:
    std::vector<CategoryGrouping>    gCategoryAttributes;

    virtual void                     AssignRandomizedData(DataStream & thisStream);
    virtual void                     SortPermutedAttribute();

  public:
    	    SpaceTimeRandomizer();
    virtual ~SpaceTimeRandomizer();

    virtual SpaceTimeRandomizer    * Clone() const;

    void                             AddCase(unsigned int iCategory, int iTimeInterval, tract_t tTractIndex);
    void                             CreateRandomizationData(const DataStream& thisStream);
};

/** Function object used to compare permuted attributes. */
class ComparePermutedTime {
  public:
    inline bool operator() (const PermutedTime* plhs, const PermutedTime* prhs);
};

/** compares permuted attribute by assigned time interval */
inline bool ComparePermutedTime::operator() (const PermutedTime* plhs, const PermutedTime* prhs) {
  return (plhs->GetTimeInterval() > prhs->GetTimeInterval());
}
//---------------------------------------------------------------------------
#endif
