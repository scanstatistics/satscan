//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "ProbabilityModel.h"
#include "SaTScanData.h"

/** constructor */
CModel::CModel(const CParameters& Parameters, CSaTScanData& DataHub, BasePrint& PrintDirection)
       : gParameters(Parameters), gDataHub(DataHub), gPrintDirection(PrintDirection) {
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

/** Returns population as defined in CCluster object. Not implemented - throws exception. */
double CModel::GetPopulation(size_t, const CCluster&) const {
  ZdGenerateException("GetPopulation() not implementated.","CModel");
  return 0;
}

