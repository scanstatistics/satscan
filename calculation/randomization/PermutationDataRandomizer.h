//---------------------------------------------------------------------------
#ifndef PermutationDataRandomizerH
#define PermutationDataRandomizerH
//---------------------------------------------------------------------------
#include "Randomizer.h"

/** abstract permutation randomizer class */
class AbstractPermutedDataRandomizer : public AbstractRandomizer {
  protected:
    BinomialGenerator   gBinomialGenerator;

    virtual void        AssignRandomizedData(DataStream & thisStream) = 0;
    virtual void        SortPermutedAttribute() = 0;

  public:
    AbstractPermutedDataRandomizer();
    virtual ~AbstractPermutedDataRandomizer();

    virtual void	RandomizeData(DataStream & thisStream, unsigned int iSimulation);
};

/** abstract permutation attribute - used to randomize permuted attribute */
class PermutedAttribute {
  protected:
    float		gfRandomNumber;

  public:
    PermutedAttribute();
    virtual ~PermutedAttribute();

    inline float	GetRandomNumber() const {return gfRandomNumber;}
    inline void		SetRandomNumber(float f) {gfRandomNumber = f;}
};

/** Function object used to compare permuted attributes. */
class ComparePermutedAttribute {
  public:
    inline bool operator() (const PermutedAttribute* plhs, const PermutedAttribute* prhs);
};

/** compares permuted attribute by assigned random number */
inline bool ComparePermutedAttribute::operator() (const PermutedAttribute* plhs, const PermutedAttribute* prhs) {
  return (plhs->GetRandomNumber() < prhs->GetRandomNumber());
}

/** Function object used to assign random number to permuted attribute. */
class AssignPermutedAttribute {
  protected:
     RandomNumberGenerator      & gGenerator;
     
  public:
    AssignPermutedAttribute(RandomNumberGenerator & Generator);
    virtual ~AssignPermutedAttribute();
    
    inline void operator() (PermutedAttribute* pAttribute);
};

inline void AssignPermutedAttribute::operator() (PermutedAttribute* pAttribute) {
  pAttribute->SetRandomNumber(gGenerator.GetRandomFloat());
}
//---------------------------------------------------------------------------
#endif
