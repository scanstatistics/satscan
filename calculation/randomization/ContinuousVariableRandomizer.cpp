//---------------------------------------------------------------------------
#include "SaTScan.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#include "ContinuousVariableRandomizer.h"

/** constructor */
PermutedVariable::PermutedVariable(double dVariable) : PermutedAttribute(), gdVariable(dVariable) {}

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
ContinuousVariableRandomizer::ContinuousVariableRandomizer() : AbstractPermutedDataRandomizer() {}

/** destructor */
ContinuousVariableRandomizer::~ContinuousVariableRandomizer() {}

/** re-initializes and  sorts permutated attribute */
void ContinuousVariableRandomizer::SortPermutedAttribute() {
  std::for_each(gvPermutedAttribute.begin(), gvPermutedAttribute.end(), AssignPermutedAttribute(gRandomNumberGenerator));
  std::sort(gvPermutedAttribute.begin(), gvPermutedAttribute.end(), ComparePermutedAttribute());
}


/** constructor */
NormalRandomizer::NormalRandomizer() : ContinuousVariableRandomizer() {}

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
  gvPermutedAttribute[gvPermutedAttribute.size() - 1] = new PermutedVariable(tContinuousVariable);
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

/** Assigns randomized data to data stream's simulation measure structures. */
void NormalRandomizer::AssignRandomizedData(const RealDataStream& thisRealStream,
                                            SimulationDataStream& thisSimulationStream) {
  //reset simulation measure arrays to zero
  thisSimulationStream.GetMeasureArrayHandler().Set(0);
  thisSimulationStream.GetSqMeasureArrayHandler().Set(0);
  AssignMeasure(thisSimulationStream.GetMeasureArray(), thisSimulationStream.GetSqMeasureArray(),
                thisRealStream.GetNumTimeIntervals(), thisRealStream.GetNumTracts());
}


/** constructor */
RankRandomizer::RankRandomizer() : ContinuousVariableRandomizer() {}

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
  gvPermutedAttribute[gvPermutedAttribute.size() - 1] = new PermutedVariable(tContinuousVariable);
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

/** Assigns randomized data to data stream's simulation measure structures. */
void RankRandomizer::AssignRandomizedData(const RealDataStream& thisRealStream,
                                          SimulationDataStream& thisSimulationStream) {
  //reset simulation measure arrays to zero
  thisSimulationStream.GetMeasureArrayHandler().Set(0);
  AssignMeasure(thisSimulationStream.GetMeasureArray(), thisRealStream.GetNumTimeIntervals(), thisRealStream.GetNumTracts());
}

/** constructor */
PermutedSurvivalAttributes::PermutedSurvivalAttributes(double dVariable, unsigned short uCensored)
                           :PermutedVariable(dVariable), guCensored(uCensored) {}

/** destructor */
PermutedSurvivalAttributes::~PermutedSurvivalAttributes() {}

/** returns pointer to newly cloned PermutatedVariable */
PermutedSurvivalAttributes * PermutedSurvivalAttributes::Clone() const {
  return new PermutedSurvivalAttributes(*this);
}


/** constructor */
SurvivalRandomizer::SurvivalRandomizer() : AbstractPermutedDataRandomizer() {}

/** destructor */
SurvivalRandomizer::~SurvivalRandomizer() {}

/** returns pointer to newly cloned PermutatedVariable */
SurvivalRandomizer * SurvivalRandomizer::Clone() const {
  return new SurvivalRandomizer(*this);
}

/** Adds new randomization entry with passed values. */
void SurvivalRandomizer::AddCase(int iTimeInterval, tract_t tTractIndex, measure_t tContinuousVariable, count_t tCensored) {
  //add stationary values
  gvStationaryAttribute.push_back(SpaceTimeStationaryAttribute(iTimeInterval, tTractIndex));
  //add permutated value
  gvPermutedAttribute.push_back(0);
  gvPermutedAttribute[gvPermutedAttribute.size() - 1] = new PermutedSurvivalAttributes(tContinuousVariable, tCensored);
}


/** Assigns data in randomizer to case and measure structures.
    NOTE: Correctness of passed iNumTimeIntervals and iNumTracts, in relation
          to passed arrays is responsibility of caller. */
void SurvivalRandomizer::Assign(count_t ** ppCases, measure_t ** ppMeasure, int iNumTimeIntervals, int iNumTracts) {
  std::vector<SpaceTimeStationaryAttribute>::iterator   itr;
  int           i, tTract;

  //assign randomized continuous data to measure
  for (i=0, itr=gvStationaryAttribute.begin(); itr != gvStationaryAttribute.end(); ++i, ++itr) {
     ppMeasure[itr->GetTimeInterval()][itr->GetTractIndex()] += gvPermutedAttribute[i]->GetVariable();
     ppCases[itr->GetTimeInterval()][itr->GetTractIndex()] += gvPermutedAttribute[i]->GetCensored();
  }   

  //now set as cumulative
  for (tTract=0; tTract < iNumTracts; ++tTract)
     for (i=iNumTimeIntervals-2; i >= 0; --i) {
        ppMeasure[i][tTract] = ppMeasure[i+1][tTract] + ppMeasure[i][tTract];
        ppCases[i][tTract] = ppCases[i+1][tTract] + ppCases[i][tTract];
     }
}

/** Assigns randomized data to data stream's simulation measure structures. */
void SurvivalRandomizer::AssignRandomizedData(const RealDataStream& thisRealStream,
                                              SimulationDataStream& thisSimulationStream) {
  //reset simulation case structure to zero
  thisSimulationStream.ResetCumulativeCaseArray();

  //reset simulation measure array to zero
  thisSimulationStream.GetMeasureArrayHandler().Set(0);
  Assign(thisSimulationStream.GetCaseArray(), thisSimulationStream.GetMeasureArray(), thisRealStream.GetNumTimeIntervals(), thisRealStream.GetNumTracts());
}

/** re-initializes and  sorts permutated attribute */
void SurvivalRandomizer::SortPermutedAttribute() {
  std::for_each(gvPermutedAttribute.begin(), gvPermutedAttribute.end(), AssignPermutedAttribute(gRandomNumberGenerator));
  std::sort(gvPermutedAttribute.begin(), gvPermutedAttribute.end(), ComparePermutedAttribute());
}

