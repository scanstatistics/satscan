//---------------------------------------------------------------------------
#ifndef ContinuousVariableRandomizerH
#define ContinuousVariableRandomizerH
//---------------------------------------------------------------------------
#include "PermutationDataRandomizer.h"

/** class representing a premuted attribute which is a continuous variable,
    or any other numberic attribute. */
class PermutedVariable : public PermutedAttribute {
  protected:
    double		      gdVariable;

  public:
    PermutedVariable(double dVariable);
    virtual ~PermutedVariable();

    virtual PermutedVariable* Clone() const;
    inline double	      GetVariable() const {return gdVariable;}
};

/** class representing the stationary space-time attributes in a permutated randomization. */
class SpaceTimeStationaryAttribute {
  protected:
    int		        giTimeIntervalIndex;
    count_t             gtTractIndex;

  public:
    SpaceTimeStationaryAttribute(int iTimeInterval, count_t tTractIndex);
    virtual ~SpaceTimeStationaryAttribute();
    virtual SpaceTimeStationaryAttribute * Clone() const;

    inline int		GetTimeInterval() const {return giTimeIntervalIndex;}
    inline int		GetTractIndex() const {return gtTractIndex;}
};

/** Randomizer which has a stationary space-time attribute
    and a randomized continuous variable. */
class ContinuousVariableRandomizer : public AbstractPermutedDataRandomizer {
  protected:
    std::vector<SpaceTimeStationaryAttribute>	gvStationaryAttribute;
    ZdPointerVector<PermutedVariable>           gvPermutedAttribute;

    virtual void                                SortPermutedAttribute();
    
  public:
    ContinuousVariableRandomizer();
    virtual ~ContinuousVariableRandomizer();
};

/** Randomizes data of data stream for a 'normal' probablility model.
    Instead of assigning data to simulation case structures, assigns
    randomized data to simulation meaure structures. */
class NormalRandomizer : public ContinuousVariableRandomizer {
  protected:
    virtual void                                AssignRandomizedData(DataStream & thisStream);

  public:
    NormalRandomizer();
    virtual ~NormalRandomizer();

    virtual NormalRandomizer          * Clone() const;

    void                                AddCase(int iTimeInterval, tract_t tTractIndex, measure_t tContinuosVariable);
    void                                AssignMeasure(measure_t ** ppMeasure, measure_t ** ppSqMeasure, int iNumTimeIntervals, int iNumTracts);
};

/** Randomizes data of data stream for a 'normal' probablility model.
    Instead of assigning data to simulation case structures, assigns
    randomized data to simulation meaure structures. */
class RankRandomizer : public ContinuousVariableRandomizer {
  protected:
    virtual void                                AssignRandomizedData(DataStream & thisStream);

  public:
    RankRandomizer();
    virtual ~RankRandomizer();

    virtual RankRandomizer            * Clone() const;

    void                                AddCase(int iTimeInterval, tract_t tTractIndex, measure_t tContinuosVariable);
    void                                AssignMeasure(measure_t ** ppMeasure, int iNumTimeIntervals, int iNumTracts);
};

/** class representing a permuted attribute which is a continuous variable
    and censored atribute. */
class PermutedSurvivalAttributes : public PermutedVariable {
  protected:
    unsigned short            guCensored;

  public:
    PermutedSurvivalAttributes(double dVariable, unsigned short uCensored);
    virtual ~PermutedSurvivalAttributes();

    virtual PermutedSurvivalAttributes* Clone() const;
    inline unsigned short               GetCensored() const {return guCensored;}
};

/** Randomizes data of data stream for a 'survival' probablility model.
    Instead of assigning data to simulation case structures, assigns
    randomized data to simulation meaure structures. */
class SurvivalRandomizer : public AbstractPermutedDataRandomizer {
  protected:
    std::vector<SpaceTimeStationaryAttribute>	gvStationaryAttribute;
    ZdPointerVector<PermutedSurvivalAttributes> gvPermutedAttribute;

    virtual void                                AssignRandomizedData(DataStream & thisStream);
    virtual void                                SortPermutedAttribute();

  public:
    SurvivalRandomizer();
    virtual ~SurvivalRandomizer();

    virtual SurvivalRandomizer        * Clone() const;

    void                                AddCase(int iTimeInterval, tract_t tTractIndex, measure_t tContinuosVariable, count_t tCensored);
    void                                Assign(count_t ** ppCases, measure_t ** ppMeasure, int iNumTimeIntervals, int iNumTracts);
};
//---------------------------------------------------------------------------
#endif
