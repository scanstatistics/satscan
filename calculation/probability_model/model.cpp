//model.cpp

#include "model.h"
#include "error.h"

CModel::CModel(CParameters* pParameters, CSaTScanData* pData)
{
  m_pParameters = pParameters;
  m_pData       = pData;

  #if DEBUGMODEL
  if ((m_pDebugModelFile = fopen("DebugModel.TXT", "w")) == NULL)
  {
    fprintf(stderr, "  Error: Unable to create makedata debug file.\n");
    FatalError(0);
  }
  #endif
}

CModel::~CModel()
{
  #if DEBUGMODEL
  fclose(m_pDebugModelFile);
  #endif
}



