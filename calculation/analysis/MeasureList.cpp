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

/** Sets for next interation.
    If iteration is a boundry, calculates loglikelihood and resets measure array(s). */
void CMeasureList::SetForNextIteration(int iIteration, double & dMaxLogLikelihood) {
  std::vector<int>::iterator itr;

  itr = std::find(gvCalculationBoundries.begin(), gvCalculationBoundries.end(), iIteration);
  if (itr != gvCalculationBoundries.end()) {
    //Hit a boundry, so calculate loglikelihood for current measure values.
    dMaxLogLikelihood = GetMaxLogLikelihood(dMaxLogLikelihood, iIteration);
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
    //Set calculation boundries between circle/each ellipse shape(s).
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

/** Returns maximum loglikelihood.
    Does not evaluate loglikelihood for zero and one cases.*/
double CMinMeasureList::GetMaxLogLikelihood(double dMaxLogLikelihood, int iEllipseOffset) {
  int           i, iListSize = gSaTScanData.m_nTotalCases + 1;
  double        dLogLikelihood, dDuczmal=1;

  //Get Duczmal correction if option set and this calculation involves an ellispe.
  if (gSaTScanData.m_pParameters->GetDuczmalCorrectEllipses() && iEllipseOffset > 0)
    dDuczmal = GetDuczmalCorrection(gSaTScanData.mdE_Shapes[iEllipseOffset - 1]);

  i = 2; //Start case index at two -- don't want to consider simulations
         //with one case as this could indicate a false high loglikelihood.
  for (;i < iListSize; i++) {
     if (gpMinMeasures[i] != 0 && i * gSaTScanData.m_nTotalMeasure > gpMinMeasures[i] * gSaTScanData.m_nTotalCases) {
        dLogLikelihood = gSaTScanData.m_pModel->CalcLogLikelihood(i, gpMinMeasures[i]);
        if (dLogLikelihood * dDuczmal > dMaxLogLikelihood)
          dMaxLogLikelihood = dLogLikelihood;
     }
  }
  return dMaxLogLikelihood;
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

/** Returns maximum loglikelihood. */
double CMaxMeasureList::GetMaxLogLikelihood(double dMaxLogLikelihood, int iEllipseOffset) {
  int           i, iListSize = gSaTScanData.m_nTotalCases + 1;
  double        dLogLikelihood, dDuczmal=1;

  //Get Duczmal correction if option set and this calculation involves an ellispe.
  if (gSaTScanData.m_pParameters->GetDuczmalCorrectEllipses() && iEllipseOffset > 0)
    dDuczmal = GetDuczmalCorrection(gSaTScanData.mdE_Shapes[iEllipseOffset - 1]);

  for (i=0; i < iListSize; i++) {
     if (gpMaxMeasures[i] != 0 && i * gSaTScanData.m_nTotalMeasure < gpMaxMeasures[i] * gSaTScanData.m_nTotalCases) {
       dLogLikelihood = gSaTScanData.m_pModel->CalcLogLikelihood(i, gpMaxMeasures[i]);
       if (dLogLikelihood * dDuczmal > dMaxLogLikelihood)
           dMaxLogLikelihood = dLogLikelihood;
     }
  }
  return dMaxLogLikelihood;
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

/** Returns maximum loglikelihood.
    Skips calculation of one case situation like CMinMeasureList. */
double CMinMaxMeasureList::GetMaxLogLikelihood(double dMaxLogLikelihood, int iEllipseOffset) {
  int           i, iListSize = gSaTScanData.m_nTotalCases + 1;
  double        dLogLikelihood, dDuczmal=1;

  //Get Duczmal correction if option set and this calculation involves an ellispe.
  if (gSaTScanData.m_pParameters->GetDuczmalCorrectEllipses() && iEllipseOffset > 0)
    dDuczmal = GetDuczmalCorrection(gSaTScanData.mdE_Shapes[iEllipseOffset - 1]);

  for (i=0; i < iListSize; i++) {
     if (i > 1 && gpMinMeasures[i] != 0 && i * gSaTScanData.m_nTotalMeasure > gpMinMeasures[i] * gSaTScanData.m_nTotalCases) {
       dLogLikelihood = gSaTScanData.m_pModel->CalcLogLikelihood(i, gpMinMeasures[i]);
       if (dLogLikelihood * dDuczmal > dMaxLogLikelihood)
         dMaxLogLikelihood = dLogLikelihood;
     }

     if (gpMaxMeasures[i] != 0 && i * gSaTScanData.m_nTotalMeasure < gpMaxMeasures[i] * gSaTScanData.m_nTotalCases) {
       dLogLikelihood = gSaTScanData.m_pModel->CalcLogLikelihood(i, gpMaxMeasures[i]);
       if (dLogLikelihood * dDuczmal > dMaxLogLikelihood)
         dMaxLogLikelihood = dLogLikelihood;
     }
  }
  return dMaxLogLikelihood;
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

