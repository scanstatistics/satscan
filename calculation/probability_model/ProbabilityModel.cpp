#include "SaTScan.h"
#pragma hdrstop
#include "ProbabilityModel.h"
#include "SaTScanData.h"

/** constructor */
CModel::CModel(CParameters & Parameters, CSaTScanData & Data, BasePrint & PrintDirection)
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

/** Throws exception. Defined in parent class as pure virtual. */
double CModel::CalcMonotoneLogLikelihood(const CPSMonotoneCluster& PSMCluster) {
  ZdGenerateException("CalcMonotoneLogLikelihood() not implementated.","CModel");
  return 0;
}

/** NEEDS DOCUMENTATION */
double CModel::CalcSVTTLogLikelihood(CSVTTCluster* Cluster, CTimeTrend GlobalTimeTrend) {
  ZdGenerateException("CalcSVTTLogLikelihood() not implementated.","CModel");
  return 0;
}

/** returns whether special maximum circle population file should be read */
bool CModel::DoesReadMaxCirclePopulationFile() {
  return gParameters.UseMaxCirclePopulationFile();
}

