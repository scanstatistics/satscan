#include "SaTScan.h"
#pragma hdrstop
#include "ProbabilityModel.h"

CModel::CModel(CParameters* pParameters, CSaTScanData* pData, BasePrint *pPrintDirection)
{
   try
      {
      m_pParameters = pParameters;
      m_pData       = pData;
      gpPrintDirection = pPrintDirection;

#ifdef DEBUGMODEL
      if ((m_pDebugModelFile = fopen("DebugModel.TXT", "w")) == NULL)
         {
         fprintf(stderr, "  Error: Unable to create makedata debug file.\n");
         //FatalError(0, gpPrintDirection);
         SSGenerateException("  Error: Unable to create makedata debug file.\n","CModel constructor");
         }
#endif
      }
   catch (SSException & x)
      {
      x.AddCallpath("CModel", "CModel");
      throw;
      }
}

CModel::~CModel()
{
#ifdef DEBUGMODEL
  fclose(m_pDebugModelFile);
#endif
}



