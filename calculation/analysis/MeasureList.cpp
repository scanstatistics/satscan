#include "SaTScan.h"
#pragma hdrstop
#include "MeasureList.h"

/** Constructor */
CMeasureList::CMeasureList(const CSaTScanData & SaTScanData, BasePrint & PrintDirection)
             : gSaTScanData(SaTScanData), gPrintDirection(PrintDirection) {
  Setup();
}

/** Destructor */
CMeasureList::~CMeasureList() {}

//store as maximum loglikelihood for current iteration(shape)
void CMeasureList::AddMaximumLogLikelihood(double dMaxLogLikelihood, int iIteration) {
  double dMaxLogLikelihoodRatio, dDuczmalCorrection;

  dMaxLogLikelihoodRatio = dMaxLogLikelihood - gSaTScanData.m_pModel->GetLogLikelihoodForTotal();
  if (iIteration > 0 && gSaTScanData.m_nNumEllipsoids && gSaTScanData.m_pParameters->GetDuczmalCorrectEllipses()) {
    dDuczmalCorrection = GetDuczmalCorrection(gSaTScanData.mdE_Shapes[iIteration - 1]);
    gvMaximumLogLikelihoodRatios.push_back(dMaxLogLikelihoodRatio * dDuczmalCorrection);
  }
  else
    gvMaximumLogLikelihoodRatios.push_back(dMaxLogLikelihoodRatio);
}

/** Returns maximum loglikelihood ratio for all iterations(shapes)   */
double CMeasureList::GetMaximumLogLikelihoodRatio() {
  double dMaximumLogLikelihoodRatio;

  //if a loglikelihood has not been calculated yet, then do it now
  //-- analyses like purely temporal never have cause to call
  //   SetForNextIteration(), so calculate LLR when asked.
  if (! gvMaximumLogLikelihoodRatios.size())
    CalculateMaximumLogLikelihood(0);

  dMaximumLogLikelihoodRatio = gvMaximumLogLikelihoodRatios[0];

  for (size_t t=1; t < gvMaximumLogLikelihoodRatios.size(); t++)
     if (gvMaximumLogLikelihoodRatios[t] > dMaximumLogLikelihoodRatio)
       dMaximumLogLikelihoodRatio = gvMaximumLogLikelihoodRatios[t];

  return dMaximumLogLikelihoodRatio; 
}


/** Sets for next interation.
    If iteration is a boundry, calculates loglikelihood and resets measure array(s). */
void CMeasureList::SetForNextIteration(int iIteration) {
  std::vector<int>::iterator    itr;

  itr = std::find(gvCalculationBoundries.begin(), gvCalculationBoundries.end(), iIteration);
  if (itr != gvCalculationBoundries.end()) {
    //Hit a boundry, so calculate loglikelihood for current measure values.
    CalculateMaximumLogLikelihood(iIteration);
    //If this is the last iteration, don't reinitialize measure arrays.
    if (++itr != gvCalculationBoundries.end())
      SetMeasures();
  }
}

/** Internal setup. */
void CMeasureList::Setup() {
  int   iEllipse, iBoundry=0;

  if (gSaTScanData.m_pParameters->GetDuczmalCorrectEllipses()) {
    //If Duczmal corrected, accumulate best measure for each shape.
    //Set calculation boundries between circle/each ellipse shape.
    gvCalculationBoundries.push_back(iBoundry); //circle
    for (iEllipse=0; iEllipse < gSaTScanData.m_nNumEllipsoids; ++iEllipse) {
       //Get the number of angles this ellipse shape rotates through.
       iBoundry += gSaTScanData.m_pParameters->GetEllipseRotations()[iEllipse];
       gvCalculationBoundries.push_back(iBoundry);
     }
   }
   else
     //No correction - accumulate best measure through all circle/ellipses.
     gvCalculationBoundries.push_back(gSaTScanData.m_pParameters->GetNumTotalEllipses());
}

/** Constructor */
CMinMeasureList::CMinMeasureList(const CSaTScanData & SaTScanData, BasePrint & PrintDirection)
                :CMeasureList(SaTScanData, PrintDirection) {
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
  for (i=0; i < gSaTScanData.m_nTotalCases + 1; i++)
     fprintf(pFile, "m_pMinMeasures[%i] = %f\n", i, gpMinMeasures[i]);
}

void CMinMeasureList::CalculateMaximumLogLikelihood(int iIteration) {
  int           i, iListSize = gSaTScanData.m_nTotalCases + 1;
  double        dLogLikelihood, dMaximumLogLikelihood = gSaTScanData.m_pModel->GetLogLikelihoodForTotal();

  i = 2; //Start case index at two -- don't want to consider simulations
         //with one case as this could indicate a false high loglikelihood.
  for (;i < iListSize; i++) {
     if (gpMinMeasures[i] != 0 && i * gSaTScanData.m_nTotalMeasure > gpMinMeasures[i] * gSaTScanData.m_nTotalCases) {
        dLogLikelihood = gSaTScanData.m_pModel->CalcLogLikelihood(i, gpMinMeasures[i]);
        if (dLogLikelihood > dMaximumLogLikelihood)
          dMaximumLogLikelihood = dLogLikelihood;
     }
  }

  //Now store maximum loglikelihood for comparison against other iterations
  AddMaximumLogLikelihood(dMaximumLogLikelihood, iIteration);
}

/** Internal initialization */
void CMinMeasureList::Init() {
  gpMinMeasures = 0;
}

/** Set/Resets measure arrays. */
void CMinMeasureList::SetMeasures() {
  int   i, iListSize = gSaTScanData.m_nTotalCases + 1;

  for (i=0; i < iListSize; ++i)
     gpMinMeasures[i] = (gSaTScanData.m_nTotalMeasure * i) / gSaTScanData.m_nTotalCases;
}

/** Internal setup */
void CMinMeasureList::Setup() {
  try {
    gpMinMeasures = new measure_t [gSaTScanData.m_nTotalCases + 1];
    SetMeasures();
  }
  catch (ZdException & x) {
    x.AddCallpath("CMinMeasureList()", "CMinMeasureList");
    throw;
  }
}


/** Constructor */
CMaxMeasureList::CMaxMeasureList(const CSaTScanData & SaTScanData, BasePrint & PrintDirection)
                :CMeasureList(SaTScanData, PrintDirection) {
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
  for (i=0; i< gSaTScanData.m_nTotalCases + 1; i++)
    fprintf(pFile, "m_pMaxMeasures[%i] = %f\n", i, gpMaxMeasures[i]);
}

void CMaxMeasureList::CalculateMaximumLogLikelihood(int iIteration) {
  int           i, iListSize = gSaTScanData.m_nTotalCases + 1;
  double        dLogLikelihood, dMaximumLogLikelihood = gSaTScanData.m_pModel->GetLogLikelihoodForTotal();

  for (i=0; i < iListSize; i++) {
     if (gpMaxMeasures[i] != 0 && i * gSaTScanData.m_nTotalMeasure < gpMaxMeasures[i] * gSaTScanData.m_nTotalCases) {
       dLogLikelihood = gSaTScanData.m_pModel->CalcLogLikelihood(i, gpMaxMeasures[i]);
       if (dLogLikelihood > dMaximumLogLikelihood)
           dMaximumLogLikelihood = dLogLikelihood;
     }
  }

  //Now store maximum loglikelihood for comparison against other iterations
  AddMaximumLogLikelihood(dMaximumLogLikelihood, iIteration);
}

/** Internal initialization */
void CMaxMeasureList::Init() {
  gpMaxMeasures = 0;
}

/** Set/Resets measure arrays. */
void CMaxMeasureList::SetMeasures() {
  int   i, iListSize = gSaTScanData.m_nTotalCases + 1;

  for (i=0; i < iListSize; ++i)
     gpMaxMeasures[i] = (gSaTScanData.m_nTotalMeasure * i) / gSaTScanData.m_nTotalCases;
}

/** Internal initialization */
void CMaxMeasureList::Setup() {
  try {
    gpMaxMeasures = new measure_t [gSaTScanData.m_nTotalCases + 1];
    SetMeasures();
  }
  catch (ZdException & x) {
    x.AddCallpath("CMaxMeasureList()", "CMaxMeasureList");
    throw;
  }
}


/** Constructor */
CMinMaxMeasureList::CMinMaxMeasureList(const CSaTScanData & SaTScanData, BasePrint & PrintDirection)
                   :CMeasureList(SaTScanData, PrintDirection) {
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
  for (i=0; i < gSaTScanData.m_nTotalCases + 1; i++)
     fprintf(pFile, "m_pMinMeasures[%i] = %f\n", i, gpMinMeasures[i]);
  fprintf(pFile, "\n");

  fprintf(pFile, "Max Measure List\n");
  for (i=0; i < gSaTScanData.m_nTotalCases + 1; i++)
     fprintf(pFile, "m_pMaxMeasures[%i] = %f\n", i, gpMaxMeasures[i]);
  fprintf(pFile, "\n");
}

void CMinMaxMeasureList::CalculateMaximumLogLikelihood(int iIteration) {
  int           i, iListSize = gSaTScanData.m_nTotalCases + 1;
  double        dLogLikelihood, dMaximumLogLikelihood = gSaTScanData.m_pModel->GetLogLikelihoodForTotal();

  for (i=0; i < iListSize; i++) {
     if (i > 1 && gpMinMeasures[i] != 0 && i * gSaTScanData.m_nTotalMeasure > gpMinMeasures[i] * gSaTScanData.m_nTotalCases) {
       dLogLikelihood = gSaTScanData.m_pModel->CalcLogLikelihood(i, gpMinMeasures[i]);
       if (dLogLikelihood > dMaximumLogLikelihood)
         dMaximumLogLikelihood = dLogLikelihood;
     }

     if (gpMaxMeasures[i] != 0 && i * gSaTScanData.m_nTotalMeasure < gpMaxMeasures[i] * gSaTScanData.m_nTotalCases) {
       dLogLikelihood = gSaTScanData.m_pModel->CalcLogLikelihood(i, gpMaxMeasures[i]);
       if (dLogLikelihood > dMaximumLogLikelihood)
         dMaximumLogLikelihood = dLogLikelihood;
     }
  }

  //Now store maximum loglikelihood for comparison against other iterations
  AddMaximumLogLikelihood(dMaximumLogLikelihood, iIteration);
}

/** Internal initialization */
void CMinMaxMeasureList::Init() {
  gpMinMeasures=0;
  gpMaxMeasures=0;
}

/** Set/Resets measure arrays. */
void CMinMaxMeasureList::SetMeasures() {
  int   i, iListSize = gSaTScanData.m_nTotalCases + 1;

  for (i=0; i < iListSize; ++i)
     gpMaxMeasures[i] = gpMinMeasures[i] = (gSaTScanData.m_nTotalMeasure * i) / gSaTScanData.m_nTotalCases;
}

/** Internal setup */
void CMinMaxMeasureList::Setup() {
  try {
    gpMinMeasures = new measure_t [gSaTScanData.m_nTotalCases + 1];
    gpMaxMeasures = new measure_t [gSaTScanData.m_nTotalCases + 1];
    SetMeasures();
  }
  catch (ZdException & x) {
    x.AddCallpath("Setup()", "CMinMaxMeasureList");
    delete gpMinMeasures;
    delete gpMaxMeasures;
    throw;
  }
}

