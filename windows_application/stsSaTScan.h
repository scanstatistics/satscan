//---------------------------------------------------------------------------
#ifndef __stsSatScan_H
#define __stsSatScan_H
//---------------------------------------------------------------------------
#include <vcl.h>
#include "SaTScan.h"

#include "stsFrmDownloadProgress.h" // must be included here, prior to 
				    // zd header defining SOCKET

#ifndef ZDHEADER
   #define ZDHEADER "zd543.h"
#endif
#include ZDHEADER

#include <Basis540.h>
#include "stsDBFFile.h"
#include "stsFrmAbout.h"
#include "SSException.h"
#include "stsFrmAnalysisRun.h"
#include "stsPrintWindow.h"
#include "stsOutputFileRegistry.h"
#include "stsDataImporter.h"
#include "stsDlgDataImporter.h"
#include "stsFrmAnalysisParameters.h"
#include "stsFrmAnalysisRun.h"
#include "stsCalculationThread.h"
#include "stsMain.h"
//---------------------------------------------------------------------------
#endif
