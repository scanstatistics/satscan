#include "SaTScan.h"
#pragma hdrstop
#include "ProbabilityModel.h"

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



