//***************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//***************************************************************************
#include "MeasureList.h"

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

  dMaxLogLikelihoodRatio = dMaxLogLikelihood - gLikelihoodCalculator.GetLogLikelihoodForTotal();
  if (iIteration > 0 && gSaTScanData.GetParameters().GetNumRequestedEllipses() && gSaTScanData.GetParameters().GetNonCompactnessPenalty()) {
    dNonCompactnessPenalty = CalculateNonCompactnessPenalty(gSaTScanData.GetEllipseShape(iIteration));
    gvMaximumLogLikelihoodRatios.push_back(dMaxLogLikelihoodRatio * dNonCompactnessPenalty);
  }
  else
    gvMaximumLogLikelihoodRatios.push_back(dMaxLogLikelihoodRatio);
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

  if (gSaTScanData.GetParameters().GetNonCompactnessPenalty()) {
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
  catch (ZdException & x) {
    x.AddCallpath("constructor()", "CMinMeasureList");
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
  for (i=0; i < gSaTScanData.GetDataSetHandler().GetDataSet().GetTotalCases() + 1; i++)
     fprintf(pFile, "m_pMinMeasures[%i] = %f\n", i, gpMinMeasures[i]);
}

/** Calculates log likelihood for accumulated data with consideration that the
    probability model is Bernoulli. Calls AddMaximumLogLikelihood() to add
    result to internal list. */
void CMinMeasureList::CalculateBernoulliMaximumLogLikelihood(int iIteration) {
  macroRunTimeStartFocused(FocusRunTimeComponent::MeasureListCalcLL);

  int           i, iHalfListSize = gSaTScanData.GetDataSetHandler().GetDataSet().GetTotalCases()/2,
                iListSize = gSaTScanData.GetDataSetHandler().GetDataSet().GetTotalCases();
  double        dLogLikelihood, dMaxExcess(0), dTotalMeasure(gSaTScanData.GetDataSetHandler().GetDataSet().GetTotalMeasure()),
                dRisk(gSaTScanData.GetDataSetHandler().GetDataSet().GetTotalCases()/gSaTScanData.GetDataSetHandler().GetDataSet().GetTotalMeasure()),
                dMaximumLogLikelihood(gLikelihoodCalculator.GetLogLikelihoodForTotal());

  i = 2; //Start case index at two -- don't want to consider simulations
         //with one case as this could indicate a false high loglikelihood.

  //Calculating the LLR for less than half the cases can use a trick where the
  //calculation is performed only if the excess exceeds any previous excess.
  for (;i < iHalfListSize; i++) {
     if (i - gpMinMeasures[i] * dRisk > dMaxExcess) {
       dMaxExcess = i - gpMinMeasures[i] * dRisk;
       dLogLikelihood = gLikelihoodCalculator.CalcLogLikelihood(i, gpMinMeasures[i]);
       if (dLogLikelihood > dMaximumLogLikelihood)
         dMaximumLogLikelihood = dLogLikelihood;
     }
  }
  //Calculate LLR for remaining half - trick not valid when number of cases is
  //greater than or equal half.
  for (i=std::max(iHalfListSize, 2); i <= iListSize; i++) {
     if (gpMinMeasures[i] != 0 && i * dTotalMeasure > gpMinMeasures[i] * iListSize) {
       dLogLikelihood = gLikelihoodCalculator.CalcLogLikelihood(i, gpMinMeasures[i]);
       if (dLogLikelihood > dMaximumLogLikelihood)
         dMaximumLogLikelihood = dLogLikelihood;
     }
  }

  //Now store maximum loglikelihood for comparison against other iterations
  AddMaximumLogLikelihood(dMaximumLogLikelihood, iIteration);
  macroRunTimeStopFocused(FocusRunTimeComponent::MeasureListCalcLL);
}

/** Calculates log likelihood for accumulated data. Calls AddMaximumLogLikelihood()
    to add result to internal list. */
void CMinMeasureList::CalculateMaximumLogLikelihood(int iIteration) {
  macroRunTimeStartFocused(FocusRunTimeComponent::MeasureListCalcLL);

  int           i, iHalfListSize = gSaTScanData.GetDataSetHandler().GetDataSet().GetTotalCases()/2,
                iListSize = gSaTScanData.GetDataSetHandler().GetDataSet().GetTotalCases();
  double        dLogLikelihood, dMaxExcess(0),
                dMaximumLogLikelihood(gLikelihoodCalculator.GetLogLikelihoodForTotal());
  i = 2; //Start case index at two -- don't want to consider simulations
         //with one case as this could indicate a false high loglikelihood.

  //Calculating the LLR for less than half the cases can use a trick where the
  //calculation is performed only if the excess exceeds any previous excess.
  for (;i < iHalfListSize; i++) {
     if (i - gpMinMeasures[i] > dMaxExcess) {
       dMaxExcess = i - gpMinMeasures[i];
       dLogLikelihood = gLikelihoodCalculator.CalcLogLikelihood(i, gpMinMeasures[i]);
       if (dLogLikelihood > dMaximumLogLikelihood)
         dMaximumLogLikelihood = dLogLikelihood;
     }
  }
  //Calculate LLR for remaining half - trick not valid when number of cases is
  //greater than or equal half.
  for (i=std::max(iHalfListSize, 2); i <= iListSize; i++) {
     if (gpMinMeasures[i] != 0 && i > gpMinMeasures[i]) {
       dLogLikelihood = gLikelihoodCalculator.CalcLogLikelihood(i, gpMinMeasures[i]);
       if (dLogLikelihood > dMaximumLogLikelihood)
         dMaximumLogLikelihood = dLogLikelihood;
     }
  }

  //Now store maximum loglikelihood for comparison against other iterations
  AddMaximumLogLikelihood(dMaximumLogLikelihood, iIteration);

  macroRunTimeStopFocused(FocusRunTimeComponent::MeasureListCalcLL);  
}

/** Internal initialization */
void CMinMeasureList::Init() {
  gpMinMeasures = 0;
}

/** Set/Resets measure arrays. */
void CMinMeasureList::SetMeasures() {
  int           i, iListSize = gSaTScanData.GetDataSetHandler().GetDataSet().GetTotalCases() + 1;
  count_t       tTotalCases = gSaTScanData.GetDataSetHandler().GetDataSet().GetTotalCases();
  measure_t     tTotalMeasure = gSaTScanData.GetDataSetHandler().GetDataSet().GetTotalMeasure();

  if (gSaTScanData.GetParameters().GetProbabilityModelType() == BERNOULLI)
    //Bernoulli model has cases + controls = total measure
    for (i=0; i < iListSize; ++i)
       gpMinMeasures[i] = (tTotalMeasure *  i) / tTotalCases;
  else
    for (i=0; i < iListSize; ++i)
       gpMinMeasures[i] = i;
}

/** Internal setup */
void CMinMeasureList::Setup() {
  try {
    gpMinMeasures = new measure_t [gSaTScanData.GetDataSetHandler().GetDataSet().GetTotalCases() + 1];
    SetMeasures();
  }
  catch (ZdException & x) {
    x.AddCallpath("CMinMeasureList()", "CMinMeasureList");
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
  catch (ZdException & x) {
    x.AddCallpath("constructor()", "CMaxMeasureList");
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
  for (i=0; i< gSaTScanData.GetDataSetHandler().GetDataSet().GetTotalCases() + 1; i++)
    fprintf(pFile, "m_pMaxMeasures[%i] = %f\n", i, gpMaxMeasures[i]);
}

/** Calculates log likelihood for accumulated data. Calls AddMaximumLogLikelihood()
    to add result to internal list. */
void CMaxMeasureList::CalculateBernoulliMaximumLogLikelihood(int iIteration) {
  macroRunTimeStartFocused(FocusRunTimeComponent::MeasureListCalcLL);

  int           i, iListSize = gSaTScanData.GetDataSetHandler().GetDataSet().GetTotalCases();
  double        dLogLikelihood, dTotalMeasure(gSaTScanData.GetDataSetHandler().GetDataSet().GetTotalMeasure()),
                dMaximumLogLikelihood(gLikelihoodCalculator.GetLogLikelihoodForTotal());

  for (i=0; i <= iListSize; i++) {
     if (gpMaxMeasures[i] != 0 && i * dTotalMeasure < gpMaxMeasures[i] * iListSize) {
       dLogLikelihood = gLikelihoodCalculator.CalcLogLikelihood(i, gpMaxMeasures[i]);
       if (dLogLikelihood > dMaximumLogLikelihood)
           dMaximumLogLikelihood = dLogLikelihood;
     }
  }

  //Now store maximum loglikelihood for comparison against other iterations
  AddMaximumLogLikelihood(dMaximumLogLikelihood, iIteration);

  macroRunTimeStopFocused(FocusRunTimeComponent::MeasureListCalcLL);
}

/** Calculates log likelihood for accumulated data. Calls AddMaximumLogLikelihood()
    to add result to internal list. */
void CMaxMeasureList::CalculateMaximumLogLikelihood(int iIteration) {
  macroRunTimeStartFocused(FocusRunTimeComponent::MeasureListCalcLL);

  int           i, iListSize = gSaTScanData.GetDataSetHandler().GetDataSet().GetTotalCases();
  double        dLogLikelihood, dTotalMeasure(gSaTScanData.GetDataSetHandler().GetDataSet().GetTotalMeasure()),
                dMaximumLogLikelihood(gLikelihoodCalculator.GetLogLikelihoodForTotal());

  for (i=0; i <= iListSize; i++) {
     if (gpMaxMeasures[i] != 0 && i < gpMaxMeasures[i]) {
       dLogLikelihood = gLikelihoodCalculator.CalcLogLikelihood(i, gpMaxMeasures[i]);
       if (dLogLikelihood > dMaximumLogLikelihood)
           dMaximumLogLikelihood = dLogLikelihood;
     }
  }

  //Now store maximum loglikelihood for comparison against other iterations
  AddMaximumLogLikelihood(dMaximumLogLikelihood, iIteration);

  macroRunTimeStopFocused(FocusRunTimeComponent::MeasureListCalcLL);  
}

/** Internal initialization */
void CMaxMeasureList::Init() {
  gpMaxMeasures = 0;
}

/** Set/Resets measure arrays. */
void CMaxMeasureList::SetMeasures() {
  int           i, iListSize = gSaTScanData.GetDataSetHandler().GetDataSet().GetTotalCases() + 1;
  count_t       tTotalCases = gSaTScanData.GetDataSetHandler().GetDataSet().GetTotalCases();
  measure_t     tTotalMeasure = gSaTScanData.GetDataSetHandler().GetDataSet().GetTotalMeasure();

  if (gSaTScanData.GetParameters().GetProbabilityModelType() == BERNOULLI)
    //Bernoulli model has cases + controls = total measure
    for (i=0; i < iListSize; ++i)
       gpMaxMeasures[i] = (tTotalMeasure * i) / tTotalCases;
  else
    for (i=0; i < iListSize; ++i)
       gpMaxMeasures[i] = i;
}

/** Internal initialization */
void CMaxMeasureList::Setup() {
  try {
    gpMaxMeasures = new measure_t [gSaTScanData.GetDataSetHandler().GetDataSet().GetTotalCases() + 1];
    SetMeasures();
  }
  catch (ZdException & x) {
    x.AddCallpath("CMaxMeasureList()", "CMaxMeasureList");
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
  catch (ZdException & x) {
    x.AddCallpath("constructor()", "CMinMaxMeasureList");
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
  for (i=0; i < gSaTScanData.GetDataSetHandler().GetDataSet().GetTotalCases() + 1; i++)
     fprintf(pFile, "m_pMinMeasures[%i] = %f\n", i, gpMinMeasures[i]);
  fprintf(pFile, "\n");

  fprintf(pFile, "Max Measure List\n");
  for (i=0; i < gSaTScanData.GetDataSetHandler().GetDataSet().GetTotalCases() + 1; i++)
     fprintf(pFile, "m_pMaxMeasures[%i] = %f\n", i, gpMaxMeasures[i]);
  fprintf(pFile, "\n");
}

/** Calculates log likelihood for accumulated data. Calls AddMaximumLogLikelihood()
    to add result to internal list. */
void CMinMaxMeasureList::CalculateBernoulliMaximumLogLikelihood(int iIteration) {
  macroRunTimeStartFocused(FocusRunTimeComponent::MeasureListCalcLL);

  int           i, iHalfListSize = gSaTScanData.GetDataSetHandler().GetDataSet().GetTotalCases()/2,
                iListSize = gSaTScanData.GetDataSetHandler().GetDataSet().GetTotalCases();
  double        dLogLikelihood, dMaxExcess(0),
                dTotalMeasure(gSaTScanData.GetDataSetHandler().GetDataSet().GetTotalMeasure()),
                dRisk(gSaTScanData.GetDataSetHandler().GetDataSet().GetTotalCases()/gSaTScanData.GetDataSetHandler().GetDataSet().GetTotalMeasure()),
  		dMaximumLogLikelihood(gLikelihoodCalculator.GetLogLikelihoodForTotal());

  //Calculating the LLR for less than half the cases can use a trick where the
  //calculation is performed only if the excess exceeds any previous excess. But
  //note that this trick is not valid for low rates, which use same process regardless.
  for (i=0; i < iHalfListSize; ++i) {
     if (i > 1 && i - gpMinMeasures[i] * dRisk > dMaxExcess) {
       dMaxExcess = i - gpMinMeasures[i] * dRisk;
       dLogLikelihood = gLikelihoodCalculator.CalcLogLikelihood(i, gpMinMeasures[i]);
       if (dLogLikelihood > dMaximumLogLikelihood)
         dMaximumLogLikelihood = dLogLikelihood;
     }

     if (gpMaxMeasures[i] != 0 && i * dTotalMeasure < gpMaxMeasures[i] * iListSize) {
       dLogLikelihood = gLikelihoodCalculator.CalcLogLikelihood(i, gpMaxMeasures[i]);
       if (dLogLikelihood > dMaximumLogLikelihood)
           dMaximumLogLikelihood = dLogLikelihood;
     }
  }
  //Calculate LLR for remaining half - trick not valid when number of cases is
  //greater than or equal half.
  for (i=iHalfListSize; i <= iListSize; ++i) {
     if (i > 1 && gpMinMeasures[i] != 0 && i * dTotalMeasure > gpMinMeasures[i] * iListSize) {
       dLogLikelihood = gLikelihoodCalculator.CalcLogLikelihood(i, gpMinMeasures[i]);
       if (dLogLikelihood > dMaximumLogLikelihood)
         dMaximumLogLikelihood = dLogLikelihood;
     }

     if (gpMaxMeasures[i] != 0 && i * dTotalMeasure < gpMaxMeasures[i] * iListSize) {
       dLogLikelihood = gLikelihoodCalculator.CalcLogLikelihood(i, gpMaxMeasures[i]);
       if (dLogLikelihood > dMaximumLogLikelihood)
           dMaximumLogLikelihood = dLogLikelihood;
     }
  }

  //Now store maximum loglikelihood for comparison against other iterations
  AddMaximumLogLikelihood(dMaximumLogLikelihood, iIteration);

  macroRunTimeStopFocused(FocusRunTimeComponent::MeasureListCalcLL);
}

/** Calculates log likelihood for accumulated data. Calls AddMaximumLogLikelihood()
    to add result to internal list. */
void CMinMaxMeasureList::CalculateMaximumLogLikelihood(int iIteration) {
  macroRunTimeStartFocused(FocusRunTimeComponent::MeasureListCalcLL);

  int           i, iHalfListSize = gSaTScanData.GetDataSetHandler().GetDataSet().GetTotalCases()/2,
                iListSize = gSaTScanData.GetDataSetHandler().GetDataSet().GetTotalCases();
  double        dLogLikelihood, dMaxExcess(0),
  		dMaximumLogLikelihood(gLikelihoodCalculator.GetLogLikelihoodForTotal());

  //Calculating the LLR for less than half the cases can use a trick where the
  //calculation is performed only if the excess exceeds any previous excess. But
  //note that this trick is not valid for low rates, which use same process regardless.
  for (i=0; i < iHalfListSize; ++i) {
     if (i > 1 && i - gpMinMeasures[i] > dMaxExcess) {
       dMaxExcess = i - gpMinMeasures[i];
       dLogLikelihood = gLikelihoodCalculator.CalcLogLikelihood(i, gpMinMeasures[i]);
       if (dLogLikelihood > dMaximumLogLikelihood)
         dMaximumLogLikelihood = dLogLikelihood;
     }
     if (gpMaxMeasures[i] != 0 && i < gpMaxMeasures[i]) {
       dLogLikelihood = gLikelihoodCalculator.CalcLogLikelihood(i, gpMaxMeasures[i]);
       if (dLogLikelihood > dMaximumLogLikelihood)
           dMaximumLogLikelihood = dLogLikelihood;
     }
  }
  //Calculate LLR for remaining half - trick not valid when number of cases is
  //greater than or equal half.
  for (i=iHalfListSize; i <= iListSize; ++i) {
     if (i > 1 && gpMinMeasures[i] != 0 && i > gpMinMeasures[i]) {
       dLogLikelihood = gLikelihoodCalculator.CalcLogLikelihood(i, gpMinMeasures[i]);
       if (dLogLikelihood > dMaximumLogLikelihood)
         dMaximumLogLikelihood = dLogLikelihood;
     }
     if (gpMaxMeasures[i] != 0 && i < gpMaxMeasures[i]) {
       dLogLikelihood = gLikelihoodCalculator.CalcLogLikelihood(i, gpMaxMeasures[i]);
       if (dLogLikelihood > dMaximumLogLikelihood)
           dMaximumLogLikelihood = dLogLikelihood;
     }
  }

  //Now store maximum loglikelihood for comparison against other iterations
  AddMaximumLogLikelihood(dMaximumLogLikelihood, iIteration);

  macroRunTimeStopFocused(FocusRunTimeComponent::MeasureListCalcLL);  
}

/** Internal initialization */
void CMinMaxMeasureList::Init() {
  gpMinMeasures=0;
  gpMaxMeasures=0;
}

/** Set/Resets measure arrays. */
void CMinMaxMeasureList::SetMeasures() {
  int           i, iListSize = gSaTScanData.GetDataSetHandler().GetDataSet().GetTotalCases() + 1;
  count_t       tTotalCases = gSaTScanData.GetDataSetHandler().GetDataSet().GetTotalCases();
  measure_t     tTotalMeasure = gSaTScanData.GetDataSetHandler().GetDataSet().GetTotalMeasure();

  if (gSaTScanData.GetParameters().GetProbabilityModelType() == BERNOULLI)
    //Bernoulli model has cases + controls = total measure
    for (i=0; i < iListSize; ++i)
       gpMaxMeasures[i] = gpMinMeasures[i] = (tTotalMeasure * i) / tTotalCases;
  else
    for (i=0; i < iListSize; ++i)
       gpMaxMeasures[i] = gpMinMeasures[i] = i;
}

/** Internal setup */
void CMinMaxMeasureList::Setup() {
  try {
    gpMinMeasures = new measure_t [gSaTScanData.GetDataSetHandler().GetDataSet().GetTotalCases() + 1];
    gpMaxMeasures = new measure_t [gSaTScanData.GetDataSetHandler().GetDataSet().GetTotalCases() + 1];
    SetMeasures();
  }
  catch (ZdException & x) {
    x.AddCallpath("Setup()", "CMinMaxMeasureList");
    delete gpMinMeasures;
    delete gpMaxMeasures;
    throw;
  }
}

