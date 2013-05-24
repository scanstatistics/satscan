//******************************************************************************
#ifndef __PermutationDataRandomizer_H
#define __PermutationDataRandomizer_H
//******************************************************************************
#include "Randomizer.h"

/** class representing the stationary attribute in a permuted randomization. */
template <class T>
class StationaryAttribute {
  protected:
    T		        gStationaryVariable;

  public:
    StationaryAttribute(T Variable);
    virtual ~StationaryAttribute();
    virtual StationaryAttribute * Clone() const;
    bool operator==(const StationaryAttribute& rhs) const{return (this->gStationaryVariable == rhs.gStationaryVariable);}
    bool operator!=(const StationaryAttribute& rhs) const{return !(this->gStationaryVariable == rhs.gStationaryVariable);}
    inline const T    & GetStationaryVariable() const {return gStationaryVariable;}
};

/** constructor */
template <class T>
StationaryAttribute<T>::StationaryAttribute(T Variable) : gStationaryVariable(Variable) {}

/** destructor */
template <class T>
StationaryAttribute<T>::~StationaryAttribute() {}

/** returns pointer to newly cloned StationaryAttribute */
template <class T>
StationaryAttribute<T> * StationaryAttribute<T>::Clone() const {
  return new StationaryAttribute(*this);
}

/** class representing the permuted attribute in a permuted randomization. */
template <class T>
class PermutedAttribute {
  protected:
    T		        gPermutedVariable;
    float		gfRandomNumber;

  public:
    PermutedAttribute(T Variable);
    virtual ~PermutedAttribute();
    virtual PermutedAttribute  * Clone() const;

    inline const T    & GetPermutedVariable() const {return gPermutedVariable;}
    inline float	GetRandomNumber() const {return gfRandomNumber;}
    inline T	      & ReferencePermutedVariable() {return gPermutedVariable;}
    inline void		SetRandomNumber(float f) {gfRandomNumber = f;}
};

/** constructor */
template <class T>
PermutedAttribute<T>::PermutedAttribute(T Variable) : gPermutedVariable(Variable), gfRandomNumber(0) {}

/** destructor */
template <class T>
PermutedAttribute<T>::~PermutedAttribute() {}

/** returns pointer to newly cloned PermutatedVariable */
template <class T>
PermutedAttribute<T> * PermutedAttribute<T>::Clone() const {
  return new PermutedAttribute(*this);
}

/** Function object used to compare permuted attributes. */
template <class T>
class ComparePermutedAttribute {
  public:
    inline bool operator() (const T& plhs, const T& prhs);
};

/** compares permuted attribute by assigned random number */
template <class T>
inline bool ComparePermutedAttribute<T>::operator() (const T& plhs, const T& prhs) {
  return (plhs.GetRandomNumber() < prhs.GetRandomNumber());
}

/** Function object used to assign random number to permuted attribute. */
template <class T>
class AssignPermutedAttribute {
  protected:
     RandomNumberGenerator      & gGenerator;

  public:
    AssignPermutedAttribute(RandomNumberGenerator & Generator);
    ~AssignPermutedAttribute();

    inline void operator() (T& pAttribute);
};

/** constructor */
template <class T>
AssignPermutedAttribute<T>::AssignPermutedAttribute(RandomNumberGenerator & Generator) : gGenerator(Generator) {}

/** destructor */
template <class T>
AssignPermutedAttribute<T>::~AssignPermutedAttribute() {}

template <class T>
inline void AssignPermutedAttribute<T>::operator() (T& Attribute) {
  Attribute.SetRandomNumber(gGenerator.GetRandomFloat());
}

// ******************************************************************************************************

class CSaTScanData; // forward class declaration

/** abstract permutation randomizer class */
template <class S, class P>
class AbstractPermutedDataRandomizer : public AbstractRandomizer {
   protected:
        const CSaTScanData& _dataHub;
        bool _dayOfWeekAdjustment;

  public:
    typedef std::vector<S>                      StationaryContainer_t;
    typedef std::vector<StationaryContainer_t>  StationaryContainerCollection_t;
    typedef std::vector<P>                      PermutedContainer_t;
    typedef std::vector<PermutedContainer_t>    PermutedContainerCollection_t;

  protected:
    StationaryContainerCollection_t             gvStationaryAttributeCollections;
    PermutedContainerCollection_t               gvOriginalPermutedAttributeCollections;
    PermutedContainerCollection_t               gvPermutedAttributeCollections;

    virtual void                AssignRandomizedData(const RealDataSet& thisRealSet, DataSet& thisSimSet) = 0;
    virtual void                SortPermutedAttribute();

  public:
    AbstractPermutedDataRandomizer(const CSaTScanData& dataHub, bool dayOfWeekAdjustment, long lInitialSeed=RandomNumberGenerator::glDefaultSeed);
    virtual ~AbstractPermutedDataRandomizer();

    virtual void                RandomizeData(const RealDataSet& thisRealSet, DataSet& thisSimSet, unsigned int iSimulation);
};

/** constructor */
template <class S, class P>
AbstractPermutedDataRandomizer<S, P>::AbstractPermutedDataRandomizer(const CSaTScanData& dataHub, bool dayOfWeekAdjustment, long lInitialSeed) 
                                         :AbstractRandomizer(lInitialSeed), _dataHub(dataHub), _dayOfWeekAdjustment(dayOfWeekAdjustment) {
    if (_dayOfWeekAdjustment) {
        // with day of week adjustment, we will keep each week day separate
        gvStationaryAttributeCollections.resize(7);
        gvOriginalPermutedAttributeCollections.resize(7);
        gvPermutedAttributeCollections.resize(7);
    } else {
        gvStationaryAttributeCollections.resize(1);
        gvOriginalPermutedAttributeCollections.resize(1);
        gvPermutedAttributeCollections.resize(1);
    }
}

/** destructor */
template <class S, class P>
AbstractPermutedDataRandomizer<S, P>::~AbstractPermutedDataRandomizer() {}

/** randomizes data of dataset */
template <class S, class P>
void AbstractPermutedDataRandomizer<S, P>::RandomizeData(const RealDataSet& thisRealSet, DataSet& thisSimSet, unsigned int iSimulation) {
  //set seed for simulation number
  SetSeed(iSimulation, thisSimSet.getSetIndex());
  //assign random numbers to permuted attribute and sort
  SortPermutedAttribute();
  //re-assign dataset's simulation data
  AssignRandomizedData(thisRealSet, thisSimSet);
}

/** re-initializes and  sorts permutated attribute */
template <class S, class P>
void AbstractPermutedDataRandomizer<S, P>::SortPermutedAttribute() {
    // Reset permuted attributes to original order - this is needed to maintain
    // consistancy of output when running in parallel.
    gvPermutedAttributeCollections = gvOriginalPermutedAttributeCollections;

    for (PermutedContainerCollection_t::iterator itr=gvPermutedAttributeCollections.begin(); itr != gvPermutedAttributeCollections.end(); ++itr) {
        std::for_each(itr->begin(), itr->end(), AssignPermutedAttribute<P>(gRandomNumberGenerator));
        std::sort(itr->begin(), itr->end(), ComparePermutedAttribute<P>());
    }
}
//******************************************************************************
#endif

