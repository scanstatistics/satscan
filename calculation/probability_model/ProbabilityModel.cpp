#include "SaTScan.h"
#pragma hdrstop
#include "ProbabilityModel.h"
#include "SaTScanData.h"

/** constructor */
CModel::CModel(const CParameters & Parameters, CSaTScanData & Data, BasePrint & PrintDirection)
       : gParameters(Parameters), gData(Data), gPrintDirection(PrintDirection) {
#ifdef DEBUGMODEL
  try {
    ZdFileName DebugFile(GetToolkit().GetApplicationFullPath());
    DebugFile.SetFileName("Debug_Info_ProbabiltyModel");
    DebugFile.SetExtension(".txt");

    if ((m_pDebugModelFile = fopen(DebugFile.GetFullPath(), "w")) == NULL)
      ZdGenerateException("Unable to create probability model debug file.\n","constructor()");
  }
  catch (ZdException &x) {
    x.AddCallpath("CModel", "CModel");
    throw;
  }
#endif
}

/** destructor */
CModel::~CModel() {
#ifdef DEBUGMODEL
  fclose(m_pDebugModelFile);
#endif
}

/** not implemented - throws exception */
double CModel::GetPopulation(unsigned int iStream, int m_iEllipseOffset, tract_t nCenter,
                             tract_t nTracts, int nStartInterval, int nStopInterval) const {
  ZdGenerateException("GetPopulation() not implementated.","CModel");
  return 0;
}

