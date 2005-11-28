//******************************************************************************
#ifndef __PermutationDataRandomizer_H
#define __PermutationDataRandomizer_H
//******************************************************************************
#include "Randomizer.h"

/** abstract permutation randomizer class */
class AbstractPermutedDataRandomizer : public AbstractRandomizer {
  protected:
    BinomialGenerator   gBinomialGenerator;

    virtual void        AssignRandomizedData(const RealDataSet& thisRealSet, SimDataSet& thisSimSet) = 0;
    virtual void        SortPermutedAttribute() = 0;

  public:
    AbstractPermutedDataRandomizer(long lInitialSeed=RandomNumberGenerator::glDefaultSeed);
    virtual ~AbstractPermutedDataRandomizer();

    virtual void	RandomizeData(const RealDataSet& thisRealSet, SimDataSet& thisSimSet, unsigned int iSimulation);
};

/** Class representing a permuted attribute which is a continuous variable,
    time index or any other attribute that need be permuted. */
template <class T>
class PermutedAttribute {
  protected:
    T		        gPermutedVariable;
    float		gfRandomNumber;

  public:
    PermutedAttribute(T Variable);
    virtual ~PermutedAttribute();
    virtual PermutedAttribute  * Clone() const;

    inline T	        GetPermutedVariable() const {return gPermutedVariable;}
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
    inline bool operator() (const PermutedAttribute<T>& plhs, const PermutedAttribute<T>& prhs);
};

/** compares permuted attribute by assigned random number */
template <class T>
inline bool ComparePermutedAttribute<T>::operator() (const PermutedAttribute<T>& plhs, const PermutedAttribute<T>& prhs) {
  return (plhs.GetRandomNumber() < prhs.GetRandomNumber());
}

/** Function object used to assign random number to permuted attribute. */
template <class T>
class AssignPermutedAttribute {
  protected:
     RandomNumberGenerator      & gGenerator;

  public:
    AssignPermutedAttribute(RandomNumberGenerator & Generator);
    virtual ~AssignPermutedAttribute();

    inline void operator() (PermutedAttribute<T>& pAttribute);
};

/** constructor */
template <class T>
AssignPermutedAttribute<T>::AssignPermutedAttribute(RandomNumberGenerator & Generator) : gGenerator(Generator) {}

/** destructor */
template <class T>
AssignPermutedAttribute<T>::~AssignPermutedAttribute() {}

template <class T>
inline void AssignPermutedAttribute<T>::operator() (PermutedAttribute<T>& Attribute) {
  Attribute.SetRandomNumber(gGenerator.GetRandomFloat());
}
//******************************************************************************
#endif

