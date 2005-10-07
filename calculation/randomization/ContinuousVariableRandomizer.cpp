//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "ContinuousVariableRandomizer.h"

/** constructor */
PermutedVariable::PermutedVariable(double dVariable, unsigned int iOrderIndex)
                 :PermutedAttribute(), gdVariable(dVariable), giOrderIndex(iOrderIndex) {}

/** destructor */
PermutedVariable::~PermutedVariable() {}

/** returns pointer to newly cloned PermutatedVariable */
PermutedVariable * PermutedVariable::Clone() const {
  return new PermutedVariable(*this);
}


/** constructor */
SpaceTimeStationaryAttribute::SpaceTimeStationaryAttribute(int iTimeInterval, count_t tTractIndex)
                             :giTimeIntervalIndex(iTimeInterval), gtTractIndex(tTractIndex) {}

/** destructor */
SpaceTimeStationaryAttribute::~SpaceTimeStationaryAttribute() {}

/** returns pointer to newly cloned PermutatedVariable */
SpaceTimeStationaryAttribute * SpaceTimeStationaryAttribute::Clone() const {
  return new SpaceTimeStationaryAttribute(*this);
}


/** constructor */
ContinuousVariableRandomizer::ContinuousVariableRandomizer(long lInitialSeed) : AbstractPermutedDataRandomizer(lInitialSeed) {}

/** destructor */
ContinuousVariableRandomizer::~ContinuousVariableRandomizer() {}

/** re-initializes and  sorts permutated attribute */
void ContinuousVariableRandomizer::SortPermutedAttribute() {
  // Sort permuted attributes to original order - this is needed to maintain
  // consistancy of output when running in parallel. 
  std::sort(gvPermutedAttribute.begin(), gvPermutedAttribute.end(), ComparePermutedOrderIndex());

  std::for_each(gvPermutedAttribute.begin(), gvPermutedAttribute.end(), AssignPermutedAttribute(gRandomNumberGenerator));
  std::sort(gvPermutedAttribute.begin(), gvPermutedAttribute.end(), ComparePermutedAttribute());
}


/** constructor */
NormalRandomizer::NormalRandomizer(long lInitialSeed) : ContinuousVariableRandomizer(lInitialSeed), giOrderIndex(0) {}

/** destructor */
NormalRandomizer::~NormalRandomizer() {}

/** returns pointer to newly cloned PermutatedVariable */
NormalRandomizer * NormalRandomizer::Clone() const {
  return new NormalRandomizer(*this);
}

/** Adds new randomization entry with passed values. */
void NormalRandomizer::AddCase(int iTimeInterval, tract_t tTractIndex, measure_t tContinuousVariable) {
  //add stationary values
  gvStationaryAttribute.push_back(SpaceTimeStationaryAttribute(iTimeInterval, tTractIndex));
  //add permutated value
  gvPermutedAttribute.push_back(0);
  gvPermutedAttribute[gvPermutedAttribute.size() - 1] = new PermutedVariable(tContinuousVariable, ++giOrderIndex);
}

/** Assigns data in randomizer to measure structures.
    NOTE: Correctness of passed iNumTimeIntervals and iNumTracts, in relation
          to passed arrays is responsibility of caller. */
void NormalRandomizer::AssignMeasure(measure_t ** ppMeasure, measure_t ** ppSqMeasure, int iNumTimeIntervals, int iNumTracts) {
  std::vector<SpaceTimeStationaryAttribute>::iterator   itr;
  int           i, tTract;

  //assign randomized continuous data to measure and measure squared arrays
  for (i=0, itr=gvStationaryAttribute.begin(); itr != gvStationaryAttribute.end(); ++i, ++itr) {
     ppMeasure[itr->GetTimeInterval()][itr->GetTractIndex()] += gvPermutedAttribute[i]->GetVariable();
     ppSqMeasure[itr->GetTimeInterval()][itr->GetTractIndex()] += pow(gvPermutedAttribute[i]->GetVariable(), 2);
  }

  //now set as cumulative
  for (tTract=0; tTract < iNumTracts; ++tTract)
     for (i=iNumTimeIntervals-2; i >= 0; --i) {
        ppMeasure[i][tTract] = ppMeasure[i+1][tTract] + ppMeasure[i][tTract];
        ppSqMeasure[i][tTract] = ppSqMeasure[i+1][tTract] + ppSqMeasure[i][tTract];
     }
}

/** Assigns randomized data to dataset's simulation measure structures. */
void NormalRandomizer::AssignRandomizedData(const RealDataSet& thisRealSet, SimDataSet& thisSimSet) {
  //reset simulation measure arrays to zero
  thisSimSet.GetMeasureArrayHandler().Set(0);
  thisSimSet.GetSqMeasureArrayHandler().Set(0);
  AssignMeasure(thisSimSet.GetMeasureArray(), thisSimSet.GetSqMeasureArray(), thisRealSet.GetNumTimeIntervals(), thisRealSet.GetNumTracts());
}


/** constructor */
RankRandomizer::RankRandomizer(long lInitialSeed) : ContinuousVariableRandomizer(lInitialSeed), giOrderIndex(0) {}

/** destructor */
RankRandomizer::~RankRandomizer() {}

/** returns pointer to newly cloned PermutatedVariable */
RankRandomizer * RankRandomizer::Clone() const {
  return new RankRandomizer(*this);
}

/** Adds new randomization entry with passed values. */
void RankRandomizer::AddCase(int iTimeInterval, tract_t tTractIndex, measure_t tContinuousVariable) {
  //add stationary values
  gvStationaryAttribute.push_back(SpaceTimeStationaryAttribute(iTimeInterval, tTractIndex));
  //add permutated value
  gvPermutedAttribute.push_back(0);
  gvPermutedAttribute[gvPermutedAttribute.size() - 1] = new PermutedVariable(tContinuousVariable, ++giOrderIndex);
}

/** Assigns data in randomizer to measure structures.
    NOTE: Correctness of passed iNumTimeIntervals and iNumTracts, in relation
          to passed arrays is responsibility of caller. */
void RankRandomizer::AssignMeasure(measure_t ** ppMeasure, int iNumTimeIntervals, int iNumTracts) {
  std::vector<SpaceTimeStationaryAttribute>::iterator   itr;
  int           i, tTract;

  //assign randomized continuous data to measure and measure squared arrays
  for (i=0, itr=gvStationaryAttribute.begin(); itr != gvStationaryAttribute.end(); ++i, ++itr)
     ppMeasure[itr->GetTimeInterval()][itr->GetTractIndex()] += gvPermutedAttribute[i]->GetVariable();

  //now set as cumulative
  for (tTract=0; tTract < iNumTracts; ++tTract)
     for (i=iNumTimeIntervals-2; i >= 0; --i)
        ppMeasure[i][tTract] = ppMeasure[i+1][tTract] + ppMeasure[i][tTract];
}

/** Assigns randomized data to data set's simulation measure structures. */
void RankRandomizer::AssignRandomizedData(const RealDataSet& thisRealSet, SimDataSet& thisSimSet) {
  //reset simulation measure arrays to zero
  thisSimSet.GetMeasureArrayHandler().Set(0);
  AssignMeasure(thisSimSet.GetMeasureArray(), thisRealSet.GetNumTimeIntervals(), thisRealSet.GetNumTracts());
}

