//******************************************************************************
//#include "SharedLibrary.h"
#pragma hdrstop
//******************************************************************************
//#ifdef WINDOWS
//#include <windows.h>
//#endif
#include <time.h>
#include "UtilityFunctions.h"
#include "PurelySpatialData.h"
#include "PurelyTemporalData.h"
#include "SpaceTimeData.h"
#include "SVTTData.h"
#include "PrintScreen.h"
#include "AnalysisRun.h"
#include "ParameterFileAccess.h"
#include "ParametersValidate.h"
#include "SSException.h"
#include "Toolkit.h"
#include <jni.h>
#include "org_satscan_app_CalculationThread.h"
#include "stsJNIPrintWindow.h"
#include "stsParametersUtility.h"
#include "org_satscan_gui_ParameterSettingsFrame.h"
#include "org_satscan_app_AppConstants.h"
#include "FileName.h"
#include "JNIException.h"

#pragma argsused

void __SaTScanInit() {
printf("__SaTScanInit called\n"); 
  reserve_memory_cache();
  std::set_new_handler(prg_new_handler);
  std::string dir;
  FileName::getCurDirectory(dir);
  dir += "satscan.exe";
  AppToolkit::ToolKitCreate(dir.c_str());
}

void __SaTScanExit() {
printf("__SaTScanExit called\n"); 
  AppToolkit::ToolKitDestroy();
}

#ifdef __GNUC__
void __attribute__((constructor)) my_init()
{
      __SaTScanInit();
      //printf("initialized\n");
}
void __attribute__((destructor)) my_fini()
{
      //printf("existing\n");
      __SaTScanExit();
}
#elif defined(_MSC_VER)
BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID/*lpReserved*/)
{

    if (dwReason == DLL_PROCESS_ATTACH) {
      //printf("initializing\n");
      __SaTScanInit();
    }
    if (dwReason == DLL_PROCESS_DETACH) {
      //printf("existing\n");
      __SaTScanExit();
    }
   return 1;
}
#else
int WINAPI DllEntryPoint(HINSTANCE hinst, unsigned long reason, void* lpReserved)
{

    if (reason == DLL_PROCESS_ATTACH) {
      printf("initializing\n");
      __SaTScanInit();
    }
    if (reason == DLL_PROCESS_DETACH) {
      printf("existing\n");
      __SaTScanExit();
    }
   return 1;
}
#endif

JNIEXPORT jstring JNICALL Java_org_satscan_app_AppConstants_getVersion(JNIEnv *pEnv, jclass) {
   return pEnv->NewStringUTF(AppToolkit::getToolkit().GetVersion());
}

JNIEXPORT jstring JNICALL Java_org_satscan_app_AppConstants_getWebSite(JNIEnv *pEnv, jclass) {
   return pEnv->NewStringUTF(AppToolkit::getToolkit().GetWebSite());
}

JNIEXPORT jstring JNICALL Java_org_satscan_app_AppConstants_getSubstantiveSupportEmail(JNIEnv *pEnv, jclass) {
   return pEnv->NewStringUTF(AppToolkit::getToolkit().GetSubstantiveSupportEmail());
}

JNIEXPORT jstring JNICALL Java_org_satscan_app_AppConstants_getTechnicalSupportEmail(JNIEnv *pEnv, jclass) {
   return pEnv->NewStringUTF(AppToolkit::getToolkit().GetTechnicalSupportEmail());
}

JNIEXPORT jstring JNICALL Java_org_satscan_app_AppConstants_getReleaseDate(JNIEnv *pEnv, jclass) {
   return pEnv->NewStringUTF(VERSION_DATE);
}

JNIEXPORT jstring JNICALL Java_org_satscan_app_AppConstants_getVersionId(JNIEnv *pEnv, jclass) {
   return pEnv->NewStringUTF(VERSION_ID);
}

/** Calcuates the number of time intervals given study period start and end dates. */
JNIEXPORT jdouble JNICALL Java_org_satscan_gui_ParameterSettingsFrame_CalculateNumberOfTimeIntervals(JNIEnv * pEnv, jobject,
                                                                                      jstring startYear, jstring startMonth, jstring startDay,
                                                                                      jstring endYear, jstring endMonth, jstring endDay, jint iTimeAggregationType) {
   
  jboolean iscopy;
  UInt     iYear, iMonth, iDay;
  Julian startDate(0), endDate(0);

  try {
     const char *sDateField = pEnv->GetStringUTFChars(startYear, &iscopy);
     iYear = static_cast<UInt>(atoi(sDateField));
	 if (sDateField == NULL) jni_error::_throwException(*pEnv);
     if (iscopy == JNI_TRUE) pEnv->ReleaseStringUTFChars(startYear, sDateField);
     sDateField = pEnv->GetStringUTFChars(startMonth, &iscopy);
	 if (sDateField == NULL) jni_error::_throwException(*pEnv);
     iMonth = static_cast<UInt>(atoi(sDateField));
     if (iscopy == JNI_TRUE) pEnv->ReleaseStringUTFChars(startMonth, sDateField);
     sDateField = pEnv->GetStringUTFChars(startDay, &iscopy);
	 if (sDateField == NULL) jni_error::_throwException(*pEnv);
     iDay = static_cast<UInt>(atoi(sDateField));
     if (iscopy == JNI_TRUE) pEnv->ReleaseStringUTFChars(startDay, sDateField);

     startDate = MDYToJulian(iMonth, iDay, iYear);

     sDateField = pEnv->GetStringUTFChars(endYear, &iscopy);
	 if (sDateField == NULL) jni_error::_throwException(*pEnv);
     iYear = static_cast<UInt>(atoi(sDateField));
     if (iscopy == JNI_TRUE) pEnv->ReleaseStringUTFChars(endYear, sDateField);
     sDateField = pEnv->GetStringUTFChars(endMonth, &iscopy);
	 if (sDateField == NULL) jni_error::_throwException(*pEnv);
     iMonth = static_cast<UInt>(atoi(sDateField));
     if (iscopy == JNI_TRUE) pEnv->ReleaseStringUTFChars(endMonth, sDateField);
     sDateField = pEnv->GetStringUTFChars(endDay, &iscopy);
	 if (sDateField == NULL) jni_error::_throwException(*pEnv);
     iDay = static_cast<UInt>(atoi(sDateField));
     if (iscopy == JNI_TRUE) pEnv->ReleaseStringUTFChars(endDay, sDateField);

     endDate = MDYToJulian(iMonth, iDay, iYear);
     return CalculateNumberOfTimeIntervals(startDate, endDate, (DatePrecisionType)iTimeAggregationType, 1);
  }
  catch (jni_error & x) {
    return 0; // let the Java exception to be handled in the caller of JNI function
  }
  catch (std::exception& x) {
	  jni_error::_throwByName(*pEnv, jni_error::_javaRuntimeExceptionClassName, x.what());
    return 1;
  }
  catch (...) {
	  jni_error::_throwByName(*pEnv, jni_error::_javaRuntimeExceptionClassName, "Unknown Program Error Encountered.");
    return 1;
  }
}

JNIEXPORT jint JNICALL Java_org_satscan_app_CalculationThread_RunAnalysis(JNIEnv *pEnv, jobject JCalculationThread, jobject JParameters) {
  try {
     int                   i;
     time_t                RunTime;
     CParameters           Parameters;
     std::string           sMessage;
     JNIPrintWindow        Console(*pEnv, JCalculationThread, false);
     
     try {
       ParametersUtility::copyJParametersToCParameters(*pEnv, JParameters, Parameters);
       Console.Printf(AppToolkit::getToolkit().GetAcknowledgment(sMessage), BasePrint::P_STDOUT);
       time(&RunTime); //get start time
       Console.SetSuppressWarnings(Parameters.GetSuppressingWarnings());
       Parameters.SetRunHistoryFilename(AppToolkit::getToolkit().GetRunHistoryFileName());
       //validate parameters - print errors to console
       if (! ParametersValidate(Parameters).Validate(Console))
         throw resolvable_error("\nThe parameter file contains incorrect settings that prevent SaTScan from continuing.\n"
                                "Please review above message(s) and modify parameter settings accordingly.");
       //create analysis runner object and execute analysis
       AnalysisRunner(Parameters, RunTime, Console);
     }
     catch (resolvable_error & x) {
       Console.Printf("%s\nEnd of Warnings and Errors", BasePrint::P_ERROR, x.what());
       return 1;
     }
     catch (prg_exception & x) {
       Console.Printf("\nProgram Error Detected:\n%s\nEnd of Warnings and Errors", BasePrint::P_ERROR, x.what());
       Console.RecordCallpath(x.trace());
       return 1;
     }
     catch (std::bad_alloc &x) {
       Console.Printf("\nSaTScan is unable to perform analysis due to insuffient memory.\n"
                      "Please see 'Memory Requirements' in user guide for suggested solutions.\n"
                      "\nEnd of Warnings and Errors", BasePrint::P_ERROR);
       return 1;
     }
     catch (std::exception& x) {
       Console.Printf("\nProgram Error Detected:\n%s\nEnd of Warnings and Errors", BasePrint::P_ERROR, x.what());
       return 1;
     }
     catch (...) {
       Console.Printf("\nUnknown Program Error Encountered\n\nEnd of Warnings and Errors", BasePrint::P_ERROR);
       return 1;
     }
  }
  catch (jni_error & x) {
    // Let the Java exception to be handled in the caller of JNI function.
    // It is preferable to report the error through the JNIPrintWindow
    // object but once a java error exists, our options are limited.
    return 1; 
  }
  
  return 0;
}
//******************************************************************************
