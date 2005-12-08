//******************************************************************************
#ifndef __SPACETIMERANDOMIZER_H
#define __SPACETIMERANDOMIZER_H
//******************************************************************************
#include "PermutationDataRandomizer.h"

class DataSet; /** forward class declaration */

/** Derived randomizer that generates simulation data through isolating data by
    previously defined categories, keeping a location index fixed and permuting
    associated time interval index.  */
class SpaceTimeRandomizer : public AbstractRandomizer {
  public:
    typedef std::vector<tract_t>                 StationaryContainer_t;
    typedef std::vector<PermutedAttribute<int> > PermutedContainer_t;

  private:
    class CategoryGrouping {
      public:
        StationaryContainer_t     gvStationaryAttribute;
        PermutedContainer_t       gvOriginalPermutedAttribute;
        PermutedContainer_t       gvPermutedAttribute;
    };

    typedef std::vector<CategoryGrouping>  CategoryContainer_t;

  protected:
    CategoryContainer_t              gCategoryAttributes;

    virtual void                     AssignRandomizedData(const RealDataSet& thisRealSet, SimDataSet& thisSimSet);
    virtual void                     SortPermutedAttribute();

  public:
    	    SpaceTimeRandomizer(long lInitialSeed=RandomNumberGenerator::glDefaultSeed);
    virtual ~SpaceTimeRandomizer();

    virtual SpaceTimeRandomizer    * Clone() const;

    void                             CreateRandomizationData(const RealDataSet& thisRealSet);
    virtual void	             RandomizeData(const RealDataSet& thisRealSet, SimDataSet& thisSimSet, unsigned int iSimulation);
};
//******************************************************************************
#endif

