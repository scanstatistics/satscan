//***************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//***************************************************************************
#include "MeasureList.h"
#include "Toolkit.h"
#include "SSException.h"
#include "SaTScanData.h"
#include "LikelihoodCalculation.h"

/** Constructor */
CMeasureList::CMeasureList(const CSaTScanData & SaTScanData, AbstractLikelihoodCalculator & LikelihoodCalculator)
             : gSaTScanData(SaTScanData), gLikelihoodCalculator(LikelihoodCalculator) {
  Setup();
}

/** Destructor */
CMeasureList::~CMeasureList() {}

/** Adds log likelihood to internal array, adjusting the passed log likelihood
    where iteration represents an elliptical shaped cylinder in conjunction with
    requested correction for compactness. */
void CMeasureList::AddMaximumLogLikelihood(double dMaxLogLikelihood, int iIteration) {
  double dMaxLogLikelihoodRatio, dNonCompactnessPenalty;

  dMaxLogLikelihoodRatio = std::max(0.0, gLikelihoodCalculator.CalculateFullStatistic(dMaxLogLikelihood));


  dNonCompactnessPenalty = CalculateNonCompactnessPenalty(gSaTScanData.GetEllipseShape(iIteration), gSaTScanData.GetParameters().GetNonCompactnessPenaltyPower());
  gvMaximumLogLikelihoodRatios.push_back(dMaxLogLikelihoodRatio * dNonCompactnessPenalty);
}

/** Returns maximum loglikelihood ratio for all iterations(shapes) */
double CMeasureList::GetMaximumLogLikelihoodRatio() {
  double dMaximumLogLikelihoodRatio;

  //if a loglikelihood has not been calculated yet, then do it now
  //-- analyses like purely temporal never have cause to call
  //   SetForNextIteration(), so calculate LLR when asked.
  if (! gvMaximumLogLikelihoodRatios.size()) {
    if (gSaTScanData.GetParameters().GetProbabilityModelType() == BERNOULLI)
      CalculateBernoulliMaximumLogLikelihood(0);
    else  
      CalculateMaximumLogLikelihood(0);
  }

  dMaximumLogLikelihoodRatio = gvMaximumLogLikelihoodRatios[0];

  for (size_t t=1; t < gvMaximumLogLikelihoodRatios.size(); t++)
     if (gvMaximumLogLikelihoodRatios[t] > dMaximumLogLikelihoodRatio)
       dMaximumLogLikelihoodRatio = gvMaximumLogLikelihoodRatios[t];

  return dMaximumLogLikelihoodRatio; 
}

/** re-intializes measure list structures for another simulation */
void CMeasureList::Reset() {
  SetMeasures();
  gvMaximumLogLikelihoodRatios.clear();
}

/** Sets for next interation.
    If iteration is a boundry, calculates loglikelihood and resets measure array(s). */
void CMeasureList::SetForNextIteration(int iIteration) {
  std::vector<int>::iterator    itr;

  itr = std::find(gvCalculationBoundries.begin(), gvCalculationBoundries.end(), iIteration);
  if (itr != gvCalculationBoundries.end()) {
    //Hit a boundry, so calculate loglikelihood for current measure values.
    if (gSaTScanData.GetParameters().GetProbabilityModelType() == BERNOULLI)
      CalculateBernoulliMaximumLogLikelihood(iIteration);
    else
      CalculateMaximumLogLikelihood(iIteration);
    //If this is the last iteration, don't reinitialize measure arrays.
    if (++itr != gvCalculationBoundries.end())
      SetMeasures();
  }
}

/** Internal setup. */
void CMeasureList::Setup() {
  int   iEllipse, iBoundry=0, iNumRequestedEllipses=gSaTScanData.GetParameters().GetNumRequestedEllipses();

  if (gSaTScanData.GetParameters().GetNonCompactnessPenaltyType() != NOPENALTY) {
    //If penalizing for compactness, accumulate best measure for each shape.
    //Set calculation boundries between circle/each ellipse shape.
    gvCalculationBoundries.push_back(iBoundry); //circle
    for (iEllipse=0; iEllipse < iNumRequestedEllipses; ++iEllipse) {
       //Get the number of angles this ellipse shape rotates through.
       iBoundry += gSaTScanData.GetParameters().GetEllipseRotations()[iEllipse];
       gvCalculationBoundries.push_back(iBoundry);
     }
   }
   else
     //No correction - accumulate best measure through all circle/ellipses.
     gvCalculationBoundries.push_back(gSaTScanData.GetParameters().GetNumTotalEllipses());
}

/** Constructor */
CMinMeasureList::CMinMeasureList(const CSaTScanData & SaTScanData, AbstractLikelihoodCalculator & LikelihoodCalculator)
                :CMeasureList(SaTScanData, LikelihoodCalculator) {
  try {
    Init();
    Setup();
  }
  catch (prg_exception& x) {
    x.addTrace("constructor()", "CMinMeasureList");
    throw;
  }
}

/** Destructor */
CMinMeasureList::~CMinMeasureList() {
  try {
    delete[] gpMinMeasures;
  }
  catch(...){}
}

void CMinMeasureList::Display(FILE* pFile) const {
  int   i;

  fprintf(pFile, "Min Measure List\n");
  for (i=0; i < gSaTScanData.GetDataSetHandler().GetDataSet().getTotalCases() + 1; i++)
     fprintf(pFile, "m_pMinMeasures[%i] = %f\n", i, gpMinMeasures[i]);
}

/** Calculates log likelihood for accumulated data with consideration that the
    probability model is Bernoulli. Calls AddMaximumLogLikelihood() to add
    result to internal list. */
void CMinMeasureList::CalculateBernoulliMaximumLogLikelihood(int iIteration) {
  macroRunTimeStartFocused(FocusRunTimeComponent::MeasureListRatioCalculation);

  int           i, iHalfListSize = gSaTScanData.GetDataSetHandler().GetDataSet().getTotalCases()/2,
                iListSize = gSaTScanData.GetDataSetHandler().GetDataSet().getTotalCases();
  double        dLogLikelihood, dMaxExcess(0), dTotalMeasure(gSaTScanData.GetDataSetHandler().GetDataSet().getTotalMeasure()),
                dRisk(gSaTScanData.GetDataSetHandler().GetDataSet().getTotalCases()/gSaTScanData.GetDataSetHandler().GetDataSet().getTotalMeasure()),
                dMaximumLogLikelihood(gLikelihoodCalculator.GetLogLikelihoodForTotal());

  //Start case index at specified minimum number of cases.
  i = static_cast<int>(gSaTScanData.GetParameters().getMinimumCasesHighRateClusters());

  //Calculating the LLR for less than half the cases can use a trick where the
  //calculation is performed only if the excess exceeds any previous excess.
  for (;i < iHalfListSize; i++) {
     if (i - gpMinMeasures[i] * dRisk > dMaxExcess) {
       dMaxExcess = i - gpMinMeasures[i] * dRisk;
       dLogLikelihood = gLikelihoodCalculator.CalculateMaximizingValue(i, gpMinMeasures[i]);
       if (dLogLikelihood > dMaximumLogLikelihood)
         dMaximumLogLikelihood = dLogLikelihood;
     }
  }
  //Calculate LLR for remaining half - trick not valid when number of cases is
  //greater than or equal half.
  for (i=std::max(iHalfListSize, static_cast<int>(gSaTScanData.GetParameters().getMinimumCasesHighRateClusters())); i <= iListSize; i++) {
     if (gpMinMeasures[i] != 0 && i * dTotalMeasure > gpMinMeasures[i] * iListSize) {
       dLogLikelihood = gLikelihoodCalculator.CalculateMaximizingValue(i, gpMinMeasures[i]);
       if (dLogLikelihood > dMaximumLogLikelihood)
         dMaximumLogLikelihood = dLogLikelihood;
     }
  }

  //Now store maximum loglikelihood for comparison against other iterations
  AddMaximumLogLikelihood(dMaximumLogLikelihood, iIteration);
  macroRunTimeStopFocused(FocusRunTimeComponent::MeasureListRatioCalculation);
}

/** Calculates log likelihood for accumulated data. Calls AddMaximumLogLikelihood()
    to add result to internal list. */
void CMinMeasureList::CalculateMaximumLogLikelihood(int iIteration) {
  macroRunTimeStartFocused(FocusRunTimeComponent::MeasureListRatioCalculation);

  int           i, iHalfListSize = gSaTScanData.GetDataSetHandler().GetDataSet().getTotalCases()/2,
                iListSize = gSaTScanData.GetDataSetHandler().GetDataSet().getTotalCases();
  double        dLogLikelihood, dMaxExcess(0),
                dMaximumLogLikelihood(gLikelihoodCalculator.GetLogLikelihoodForTotal());

  //Start case index at specified minimum number of cases.
  i = static_cast<int>(gSaTScanData.GetParameters().getMinimumCasesHighRateClusters());

  //Calculating the LLR for less than half the cases can use a trick where the
  //calculation is performed only if the excess exceeds any previous excess.
  for (;i < iHalfListSize; i++) {
     if (i - gpMinMeasures[i] > dMaxExcess) {
       dMaxExcess = i - gpMinMeasures[i];
       dLogLikelihood = gLikelihoodCalculator.CalculateMaximizingValue(i, gpMinMeasures[i]);
       if (dLogLikelihood > dMaximumLogLikelihood)
         dMaximumLogLikelihood = dLogLikelihood;
     }
  }
  //Calculate LLR for remaining half - trick not valid when number of cases is
  //greater than or equal half.
  for (i=std::max(iHalfListSize, static_cast<int>(gSaTScanData.GetParameters().getMinimumCasesHighRateClusters())); i <= iListSize; i++) {
     if (gpMinMeasures[i] != 0 && i > gpMinMeasures[i]) {
       dLogLikelihood = gLikelihoodCalculator.CalculateMaximizingValue(i, gpMinMeasures[i]);
       if (dLogLikelihood > dMaximumLogLikelihood)
         dMaximumLogLikelihood = dLogLikelihood;
     }
  }

  //Now store maximum loglikelihood for comparison against other iterations
  AddMaximumLogLikelihood(dMaximumLogLikelihood, iIteration);

  macroRunTimeStopFocused(FocusRunTimeComponent::MeasureListRatioCalculation);
}

/** Internal initialization */
void CMinMeasureList::Init() {
  gpMinMeasures = 0;
}

/** Set/Resets measure arrays. */
void CMinMeasureList::SetMeasures() {
  macroRunTimeStartFocused(FocusRunTimeComponent::MeasureListScanningAdding);

  int           i, iListSize = gSaTScanData.GetDataSetHandler().GetDataSet().getTotalCases() + 1;
  count_t       tTotalCases = gSaTScanData.GetDataSetHandler().GetDataSet().getTotalCases();
  measure_t     tTotalMeasure = gSaTScanData.GetDataSetHandler().GetDataSet().getTotalMeasure();

  if (gSaTScanData.GetParameters().GetProbabilityModelType() == BERNOULLI)
    //Bernoulli model has cases + controls = total measure
    for (i=0; i < iListSize; ++i)
       gpMinMeasures[i] = (tTotalMeasure *  i) / tTotalCases;
  else
    for (i=0; i < iListSize; ++i)
       gpMinMeasures[i] = i;

  macroRunTimeStopFocused(FocusRunTimeComponent::MeasureListScanningAdding);
}

/** Internal setup */
void CMinMeasureList::Setup() {
  try {
    gpMinMeasures = new measure_t [gSaTScanData.GetDataSetHandler().GetDataSet().getTotalCases() + 1];
    SetMeasures();
  }
  catch (prg_exception& x) {
    x.addTrace("CMinMeasureList()", "CMinMeasureList");
    throw;
  }
}

/** Constructor */
CMaxMeasureList::CMaxMeasureList(const CSaTScanData & SaTScanData, AbstractLikelihoodCalculator & LikelihoodCalculator)
                :CMeasureList(SaTScanData, LikelihoodCalculator) {
  try {
    Init();
    Setup();
  }
  catch (prg_exception& x) {
    x.addTrace("constructor()", "CMaxMeasureList");
    throw;
  }
}

/** Destructor */
CMaxMeasureList::~CMaxMeasureList() {
  try {
    delete[] gpMaxMeasures;
  }
  catch(...){}
}

void CMaxMeasureList::Display(FILE* pFile) const {
  int   i;

  fprintf(pFile, "Max Measure List\n");
  for (i=0; i< gSaTScanData.GetDataSetHandler().GetDataSet().getTotalCases() + 1; i++)
    fprintf(pFile, "m_pMaxMeasures[%i] = %f\n", i, gpMaxMeasures[i]);
}

/** Calculates log likelihood for accumulated data. Calls AddMaximumLogLikelihood()
    to add result to internal list. */
void CMaxMeasureList::CalculateBernoulliMaximumLogLikelihood(int iIteration) {
  macroRunTimeStartFocused(FocusRunTimeComponent::MeasureListRatioCalculation);

  int           i, iListSize = gSaTScanData.GetDataSetHandler().GetDataSet().getTotalCases();
  double        dLogLikelihood, dTotalMeasure(gSaTScanData.GetDataSetHandler().GetDataSet().getTotalMeasure()),
                dMaximumLogLikelihood(gLikelihoodCalculator.GetLogLikelihoodForTotal());

  //Start case index at specified minimum number of cases.
  i = static_cast<int>(gSaTScanData.GetParameters().getMinimumCasesLowRateClusters());

  for (; i <= iListSize; i++) {
     if (gpMaxMeasures[i] != 0 && i * dTotalMeasure < gpMaxMeasures[i] * iListSize) {
       dLogLikelihood = gLikelihoodCalculator.CalculateMaximizingValue(i, gpMaxMeasures[i]);
       if (dLogLikelihood > dMaximumLogLikelihood)
           dMaximumLogLikelihood = dLogLikelihood;
     }
  }

  //Now store maximum loglikelihood for comparison against other iterations
  AddMaximumLogLikelihood(dMaximumLogLikelihood, iIteration);

  macroRunTimeStopFocused(FocusRunTimeComponent::MeasureListRatioCalculation);
}

/** Calculates log likelihood for accumulated data. Calls AddMaximumLogLikelihood()
    to add result to internal list. */
void CMaxMeasureList::CalculateMaximumLogLikelihood(int iIteration) {
  macroRunTimeStartFocused(FocusRunTimeComponent::MeasureListRatioCalculation);

  int           i, iListSize = gSaTScanData.GetDataSetHandler().GetDataSet().getTotalCases();
  double        dLogLikelihood, dMaximumLogLikelihood(gLikelihoodCalculator.GetLogLikelihoodForTotal());

  //Start case index at specified minimum number of cases.
  i = static_cast<int>(gSaTScanData.GetParameters().getMinimumCasesLowRateClusters());

  for (; i <= iListSize; i++) {
     if (gpMaxMeasures[i] != 0 && i < gpMaxMeasures[i]) {
       dLogLikelihood = gLikelihoodCalculator.CalculateMaximizingValue(i, gpMaxMeasures[i]);
       if (dLogLikelihood > dMaximumLogLikelihood)
           dMaximumLogLikelihood = dLogLikelihood;
     }
  }

  //Now store maximum loglikelihood for comparison against other iterations
  AddMaximumLogLikelihood(dMaximumLogLikelihood, iIteration);

  macroRunTimeStopFocused(FocusRunTimeComponent::MeasureListRatioCalculation);
}

/** Internal initialization */
void CMaxMeasureList::Init() {
  gpMaxMeasures = 0;
}

/** Set/Resets measure arrays. */
void CMaxMeasureList::SetMeasures() {
  macroRunTimeStartFocused(FocusRunTimeComponent::MeasureListScanningAdding);

  int           i, iListSize = gSaTScanData.GetDataSetHandler().GetDataSet().getTotalCases() + 1;
  count_t       tTotalCases = gSaTScanData.GetDataSetHandler().GetDataSet().getTotalCases();
  measure_t     tTotalMeasure = gSaTScanData.GetDataSetHandler().GetDataSet().getTotalMeasure();

  if (gSaTScanData.GetParameters().GetProbabilityModelType() == BERNOULLI)
    //Bernoulli model has cases + controls = total measure
    for (i=0; i < iListSize; ++i)
       gpMaxMeasures[i] = (tTotalMeasure * i) / tTotalCases;
  else
    for (i=0; i < iListSize; ++i)
       gpMaxMeasures[i] = i;

  macroRunTimeStopFocused(FocusRunTimeComponent::MeasureListScanningAdding);
}

/** Internal initialization */
void CMaxMeasureList::Setup() {
  try {
    gpMaxMeasures = new measure_t [gSaTScanData.GetDataSetHandler().GetDataSet().getTotalCases() + 1];
    SetMeasures();
  }
  catch (prg_exception& x) {
    x.addTrace("CMaxMeasureList()", "CMaxMeasureList");
    throw;
  }
}

/** Constructor */
CMinMaxMeasureList::CMinMaxMeasureList(const CSaTScanData & SaTScanData, AbstractLikelihoodCalculator & LikelihoodCalculator)
                   :CMeasureList(SaTScanData, LikelihoodCalculator) {
  try {
    Init();
    Setup();
  }
  catch (prg_exception& x) {
    x.addTrace("constructor()", "CMinMaxMeasureList");
    throw;
  }
}

/** Destructor */
CMinMaxMeasureList::~CMinMaxMeasureList() {
  try {
    delete[] gpMinMeasures;
    delete[] gpMaxMeasures;
  }
  catch(...){}
}

void CMinMaxMeasureList::Display(FILE* pFile) const {
  int i;

  fprintf(pFile, "Min Measure List\n");
  for (i=0; i < gSaTScanData.GetDataSetHandler().GetDataSet().getTotalCases() + 1; i++)
     fprintf(pFile, "m_pMinMeasures[%i] = %f\n", i, gpMinMeasures[i]);
  fprintf(pFile, "\n");

  fprintf(pFile, "Max Measure List\n");
  for (i=0; i < gSaTScanData.GetDataSetHandler().GetDataSet().getTotalCases() + 1; i++)
     fprintf(pFile, "m_pMaxMeasures[%i] = %f\n", i, gpMaxMeasures[i]);
  fprintf(pFile, "\n");
}

/** Calculates log likelihood for accumulated data. Calls AddMaximumLogLikelihood()
    to add result to internal list. */
void CMinMaxMeasureList::CalculateBernoulliMaximumLogLikelihood(int iIteration) {
  macroRunTimeStartFocused(FocusRunTimeComponent::MeasureListRatioCalculation);

  int           i, iHalfListSize = gSaTScanData.GetDataSetHandler().GetDataSet().getTotalCases()/2,
                iListSize = gSaTScanData.GetDataSetHandler().GetDataSet().getTotalCases();
  double        dLogLikelihood, dMaxExcess(0), dMaximumLogLikelihood(gLikelihoodCalculator.GetLogLikelihoodForTotal()),
                dTotalMeasure(gSaTScanData.GetDataSetHandler().GetDataSet().getTotalMeasure()),
                dRisk(gSaTScanData.GetDataSetHandler().GetDataSet().getTotalCases()/gSaTScanData.GetDataSetHandler().GetDataSet().getTotalMeasure());

  //Start case index at specified minimum number of cases.
  int iH = static_cast<int>(gSaTScanData.GetParameters().getMinimumCasesHighRateClusters());
  int iL = static_cast<int>(gSaTScanData.GetParameters().getMinimumCasesLowRateClusters());

  //Calculating the LLR for less than half the cases can use a trick where the
  //calculation is performed only if the excess exceeds any previous excess. But
  //note that this trick is not valid for low rates, which use same process regardless.
  for (i=std::min(iL, iH); i < iHalfListSize; ++i) {
     if (i >= iH && i - gpMinMeasures[i] * dRisk > dMaxExcess) {
       dMaxExcess = i - gpMinMeasures[i] * dRisk;
       dLogLikelihood = gLikelihoodCalculator.CalculateMaximizingValue(i, gpMinMeasures[i]);
       if (dLogLikelihood > dMaximumLogLikelihood)
         dMaximumLogLikelihood = dLogLikelihood;
     }

     if (i >= iL && gpMaxMeasures[i] != 0 && i * dTotalMeasure < gpMaxMeasures[i] * iListSize) {
       dLogLikelihood = gLikelihoodCalculator.CalculateMaximizingValue(i, gpMaxMeasures[i]);
       if (dLogLikelihood > dMaximumLogLikelihood)
           dMaximumLogLikelihood = dLogLikelihood;
     }
  }
  //Calculate LLR for remaining half - trick not valid when number of cases is
  //greater than or equal half.
  i = std::max(std::min(iL, iH), iHalfListSize);
  for (; i <= iListSize; ++i) {
     if (i >= iH && gpMinMeasures[i] != 0 && i * dTotalMeasure > gpMinMeasures[i] * iListSize) {
       dLogLikelihood = gLikelihoodCalculator.CalculateMaximizingValue(i, gpMinMeasures[i]);
       if (dLogLikelihood > dMaximumLogLikelihood)
         dMaximumLogLikelihood = dLogLikelihood;
     }

     if (i >= iL && gpMaxMeasures[i] != 0 && i * dTotalMeasure < gpMaxMeasures[i] * iListSize) {
       dLogLikelihood = gLikelihoodCalculator.CalculateMaximizingValue(i, gpMaxMeasures[i]);
       if (dLogLikelihood > dMaximumLogLikelihood)
           dMaximumLogLikelihood = dLogLikelihood;
     }
  }

  //Now store maximum loglikelihood for comparison against other iterations
  AddMaximumLogLikelihood(dMaximumLogLikelihood, iIteration);

  macroRunTimeStopFocused(FocusRunTimeComponent::MeasureListRatioCalculation);
}

/** Calculates log likelihood for accumulated data. Calls AddMaximumLogLikelihood()
    to add result to internal list. */
void CMinMaxMeasureList::CalculateMaximumLogLikelihood(int iIteration) {
  macroRunTimeStartFocused(FocusRunTimeComponent::MeasureListRatioCalculation);

  int           i, iHalfListSize = gSaTScanData.GetDataSetHandler().GetDataSet().getTotalCases()/2,
                iListSize = gSaTScanData.GetDataSetHandler().GetDataSet().getTotalCases();
  double        dLogLikelihood, dMaxExcess(0), dMaximumLogLikelihood(gLikelihoodCalculator.GetLogLikelihoodForTotal());

  //Start case index at specified minimum number of cases.
  int iH = static_cast<int>(gSaTScanData.GetParameters().getMinimumCasesHighRateClusters());
  int iL = static_cast<int>(gSaTScanData.GetParameters().getMinimumCasesLowRateClusters());

  //Calculating the LLR for less than half the cases can use a trick where the
  //calculation is performed only if the excess exceeds any previous excess. But
  //note that this trick is not valid for low rates, which use same process regardless.
  for (i=std::min(iL, iH); i < iHalfListSize; ++i) {
     if (i >= iH && i - gpMinMeasures[i] > dMaxExcess) {
       dMaxExcess = i - gpMinMeasures[i];
       dLogLikelihood = gLikelihoodCalculator.CalculateMaximizingValue(i, gpMinMeasures[i]);
       if (dLogLikelihood > dMaximumLogLikelihood)
         dMaximumLogLikelihood = dLogLikelihood;
     }
     if (i >= iL && gpMaxMeasures[i] != 0 && i < gpMaxMeasures[i]) {
       dLogLikelihood = gLikelihoodCalculator.CalculateMaximizingValue(i, gpMaxMeasures[i]);
       if (dLogLikelihood > dMaximumLogLikelihood)
           dMaximumLogLikelihood = dLogLikelihood;
     }
  }
  //Calculate LLR for remaining half - trick not valid when number of cases is
  //greater than or equal half.
  i = std::max(std::min(iL, iH), iHalfListSize);
  for (; i <= iListSize; ++i) {
     if (i >= iH && gpMinMeasures[i] != 0 && i > gpMinMeasures[i]) {
       dLogLikelihood = gLikelihoodCalculator.CalculateMaximizingValue(i, gpMinMeasures[i]);
       if (dLogLikelihood > dMaximumLogLikelihood)
         dMaximumLogLikelihood = dLogLikelihood;
     }
     if (i >= iL && gpMaxMeasures[i] != 0 && i < gpMaxMeasures[i]) {
       dLogLikelihood = gLikelihoodCalculator.CalculateMaximizingValue(i, gpMaxMeasures[i]);
       if (dLogLikelihood > dMaximumLogLikelihood)
           dMaximumLogLikelihood = dLogLikelihood;
     }
  }

  //Now store maximum loglikelihood for comparison against other iterations
  AddMaximumLogLikelihood(dMaximumLogLikelihood, iIteration);

  macroRunTimeStopFocused(FocusRunTimeComponent::MeasureListRatioCalculation);
}

/** Internal initialization */
void CMinMaxMeasureList::Init() {
  gpMinMeasures=0;
  gpMaxMeasures=0;
}

/** Set/Resets measure arrays. */
void CMinMaxMeasureList::SetMeasures() {
  macroRunTimeStartFocused(FocusRunTimeComponent::MeasureListScanningAdding);

  int           i, iListSize = gSaTScanData.GetDataSetHandler().GetDataSet().getTotalCases() + 1;
  count_t       tTotalCases = gSaTScanData.GetDataSetHandler().GetDataSet().getTotalCases();
  measure_t     tTotalMeasure = gSaTScanData.GetDataSetHandler().GetDataSet().getTotalMeasure();

  if (gSaTScanData.GetParameters().GetProbabilityModelType() == BERNOULLI)
    //Bernoulli model has cases + controls = total measure
    for (i=0; i < iListSize; ++i)
       gpMaxMeasures[i] = gpMinMeasures[i] = (tTotalMeasure * i) / tTotalCases;
  else
    for (i=0; i < iListSize; ++i)
       gpMaxMeasures[i] = gpMinMeasures[i] = i;

  macroRunTimeStopFocused(FocusRunTimeComponent::MeasureListScanningAdding);
}

/** Internal setup */
void CMinMaxMeasureList::Setup() {
  try {
    gpMinMeasures = new measure_t [gSaTScanData.GetDataSetHandler().GetDataSet().getTotalCases() + 1];
    gpMaxMeasures = new measure_t [gSaTScanData.GetDataSetHandler().GetDataSet().getTotalCases() + 1];
    SetMeasures();
  }
  catch (prg_exception& x) {
    x.addTrace("Setup()", "CMinMaxMeasureList");
    delete gpMinMeasures;
    delete gpMaxMeasures;
    throw;
  }
}

RiskMinMeasureList::RiskMinMeasureList(const CSaTScanData& hub, AbstractLikelihoodCalculator& calculator, double risk_threshold)
    : CMinMeasureList(hub, calculator), _risk_threshold(risk_threshold) {
    if (hub.GetParameters().GetProbabilityModelType() == SPACETIMEPERMUTATION || hub.GetParameters().GetProbabilityModelType() == EXPONENTIAL)
        _risk_calc.reset(new ObservedDividedExpectedCalc(hub.GetMeasureAdjustment(0)));
    else
        _risk_calc.reset(new RelativeRiskCalc(hub.GetDataSetHandler().GetDataSet(0).getTotalCases(), hub.GetMeasureAdjustment(0)));
}

RiskMaxMeasureList::RiskMaxMeasureList(const CSaTScanData& hub, AbstractLikelihoodCalculator& calculator, double risk_threshold)
    : CMaxMeasureList(hub, calculator), _risk_threshold(risk_threshold) {
    if (hub.GetParameters().GetProbabilityModelType() == SPACETIMEPERMUTATION || hub.GetParameters().GetProbabilityModelType() == EXPONENTIAL)
        _risk_calc.reset(new ObservedDividedExpectedCalc(hub.GetMeasureAdjustment(0)));
    else
        _risk_calc.reset(new RelativeRiskCalc(hub.GetDataSetHandler().GetDataSet(0).getTotalCases(), hub.GetMeasureAdjustment(0)));
}

RiskMinMaxMeasureList::RiskMinMaxMeasureList(const CSaTScanData& hub, AbstractLikelihoodCalculator& calculator, double low_risk_threshold, double high_risk_threshold)
    : CMinMaxMeasureList(hub, calculator), _low_risk_threshold(low_risk_threshold), _high_risk_threshold(high_risk_threshold) {
    if (hub.GetParameters().GetProbabilityModelType() == SPACETIMEPERMUTATION || hub.GetParameters().GetProbabilityModelType() == EXPONENTIAL)
        _risk_calc.reset(new ObservedDividedExpectedCalc(hub.GetMeasureAdjustment(0)));
    else
        _risk_calc.reset(new RelativeRiskCalc(hub.GetDataSetHandler().GetDataSet(0).getTotalCases(), hub.GetMeasureAdjustment(0)));
}
