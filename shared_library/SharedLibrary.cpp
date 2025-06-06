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
#include "org_satscan_gui_OberservableRegionsFrame.h"
#include "org_satscan_app_AppConstants.h"
#include "org_satscan_utils_FileAccess.h"
#include "org_satscan_importer_ShapefileDataSource.h"
#include "FileName.h"
#include "JNIException.h"
#include "ObservableRegion.h"
#include "PrintCallback.h"
#include "ParameterFileAccess.h"
#include "ShapeFile.h"
#include "DataSource.h"
#include "org_satscan_gui_ApplicationPreferences.h"
#include "org_satscan_gui_BatchAnalysisFrame.h"
#include "MultipleAnalyses.h"

//#pragma argsused

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

/** Run execution function given passed parameter file and base print. */
void _runAnalysis(const CParameters& Parameters, BasePrint& Console) {
  int                   i;
  time_t                RunTime;
  std::string           sMessage;

  AppToolkit::getToolkit().refreshSession();
  Console.Printf(AppToolkit::getToolkit().GetAcknowledgment(sMessage), BasePrint::P_STDOUT);
  time(&RunTime); //get start time
  Console.SetSuppressWarnings(Parameters.GetSuppressingWarnings());
  //validate parameters - print errors to console
  if (! ParametersValidate(Parameters).Validate(Console))
    throw resolvable_error("\nThe parameter file contains incorrect settings that prevent SaTScan from continuing.\n"
                           "Please review above message(s) and modify parameter settings accordingly.");
  //create analysis runner object and execute analysis
  AnalysisRunner(Parameters, RunTime, Console).run();
}

///////////////////////////////// JNI Shared Library Methods ///////////////////////////////////////////

/*
 * Class:     org_satscan_importer_ShapefileDataSource
 * Method:    isSupportedProjection
 * Signature: (Ljava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_org_satscan_importer_ShapefileDataSource_isSupportedProjection(JNIEnv * pEnv, jclass, jstring filename) {
    jboolean iscopy;
    const char *temp = pEnv->GetStringUTFChars(filename, &iscopy);
    std::string file(temp);
    if (iscopy == JNI_TRUE) pEnv->ReleaseStringUTFChars(filename, temp);
    std::pair<bool, std::string> isSupported = ShapeFileDataSource::isSupportedProjection(file);
    return pEnv->NewStringUTF((isSupported.first == false ? isSupported.second.c_str() : ""));
}

/*
 * Class:     org_satscan_importer_ShapefileDataSource
 * Method:    isSupportedShapeType
 * Signature: (Ljava/lang/String;)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_org_satscan_importer_ShapefileDataSource_isSupportedShapeType(JNIEnv * pEnv, jclass, jstring filename) {
    jboolean iscopy;
    const char *temp = pEnv->GetStringUTFChars(filename, &iscopy);
    std::string file(temp);
    if (iscopy == JNI_TRUE) pEnv->ReleaseStringUTFChars(filename, temp);
    std::pair<bool, std::string> isSupported = ShapeFileDataSource::isSupportedShapeType(file);
    return pEnv->NewStringUTF((isSupported.first == false ? isSupported.second.c_str() : ""));
}

/*
 * Class:     org_satscan_importer_ShapefileDataSource
 * Method:    getNumberOfShapes
 * Signature: (Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_org_satscan_importer_ShapefileDataSource_getNumberOfShapes(JNIEnv * pEnv, jobject, jstring filename) {
  try {
    jboolean iscopy;
    const char *temp = pEnv->GetStringUTFChars(filename, &iscopy);
    std::string file(temp);
    if (iscopy == JNI_TRUE) pEnv->ReleaseStringUTFChars(filename, temp);

    ShapeFile shapefile(file.c_str(), "r", false);
    return (jint)shapefile.getEntityCount();
  }
  catch (jni_error & x) {
    return (jint)-1; // let the Java exception to be handled in the caller of JNI function
  }
  catch (std::exception& x) {
	jni_error::_throwByName(*pEnv, jni_error::_javaRuntimeExceptionClassName, x.what());
    return (jint)-1;
  }
  catch (...) {
	jni_error::_throwByName(*pEnv, jni_error::_javaRuntimeExceptionClassName, "Unknown Program Error Encountered.");
    return (jint)-1;
  }
}

/*
 * Class:     org_satscan_importer_ShapefileDataSource
 * Method:    getCoordinates
 * Signature: (J)[D
 */
JNIEXPORT jdoubleArray JNICALL Java_org_satscan_importer_ShapefileDataSource_getCoordinates(JNIEnv * pEnv, jobject, jstring filename, jlong row) {
  try {
    jboolean iscopy;
    const char *temp = pEnv->GetStringUTFChars(filename, &iscopy);
    std::string file(temp);
    if (iscopy == JNI_TRUE) pEnv->ReleaseStringUTFChars(filename, temp);

    ShapeFile shapefile(file.c_str(), "r", false);
    if (row >= shapefile.getEntityCount())
        return (jdoubleArray)NULL;

    double x, y;
    shapefile.getShapeAsXY(row, x, y);

    jdoubleArray coordinates = pEnv->NewDoubleArray(2);
    if (coordinates) {
        jdouble outCArray[] = {y, x};
        pEnv->SetDoubleArrayRegion(coordinates, 0 , 2, outCArray);
    }
    return coordinates;
  }
  catch (jni_error & x) {
      printf("jni_error: %s",x.what());
    return (jdoubleArray)NULL; // let the Java exception to be handled in the caller of JNI function
  }
  catch (std::exception& x) {
      printf("exception: %s",x.what());
	jni_error::_throwByName(*pEnv, jni_error::_javaRuntimeExceptionClassName, x.what());
    return (jdoubleArray)NULL;
  }
  catch (...) {
      printf("... exception");
	jni_error::_throwByName(*pEnv, jni_error::_javaRuntimeExceptionClassName, "Unknown Program Error Encountered.");
    return (jdoubleArray)NULL;
  }
}

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

/** Returns the date format substituted filename. */
JNIEXPORT jstring JNICALL Java_org_satscan_utils_FileAccess_getFormatSubstitutedFilename(JNIEnv * pEnv, jclass, jstring jfilename) {
    jboolean iscopy;
    try {
        const char * sfilename = pEnv->GetStringUTFChars(jfilename, &iscopy);
        std::string substitutedfilename(getFilenameFormatTime(std::string(sfilename), boost::posix_time::second_clock::local_time()));
        if (iscopy == JNI_TRUE) pEnv->ReleaseStringUTFChars(jfilename, sfilename);
        return pEnv->NewStringUTF(substitutedfilename.c_str());
    }
    catch (jni_error & x) {
        return pEnv->NewStringUTF("");
    }
    catch (std::exception& x) {
        jni_error::_throwByName(*pEnv, jni_error::_javaRuntimeExceptionClassName, x.what());
        return pEnv->NewStringUTF("");
    }
    catch (...) {
        jni_error::_throwByName(*pEnv, jni_error::_javaRuntimeExceptionClassName, "Unknown Program Error Encountered.");
        return pEnv->NewStringUTF("");
    }
}

JNIEXPORT jstring JNICALL Java_org_satscan_gui_OberservableRegionsFrame__1getRegionIsValid(JNIEnv  * pEnv, jobject , jstring regions) {
  try {
    jboolean iscopy;
    const char *temp = pEnv->GetStringUTFChars(regions, &iscopy);
    std::string listRegions(temp);
    if (iscopy == JNI_TRUE) pEnv->ReleaseStringUTFChars(regions, temp);
    //attempt to parse the regions into polygon object   
    ConvexPolygonBuilder::buildConvexPolygon(ConvexPolygonBuilder::parse(listRegions));  
    return pEnv->NewStringUTF("Ok");
  }
  catch (jni_error & x) {
    return pEnv->NewStringUTF("Failed"); // let the Java exception to be handled in the caller of JNI function
  }
  catch (region_exception& e) {
    printf("Polygon Error: %s\n", e.what());
    return pEnv->NewStringUTF(e.what());
  }
  catch (std::exception& x) {
	jni_error::_throwByName(*pEnv, jni_error::_javaRuntimeExceptionClassName, x.what());
    return pEnv->NewStringUTF("Failed");
  }
  catch (...) {
	jni_error::_throwByName(*pEnv, jni_error::_javaRuntimeExceptionClassName, "Unknown Program Error Encountered.");
    return pEnv->NewStringUTF("Failed");
  }
}

JNIEXPORT jstring JNICALL Java_org_satscan_gui_OberservableRegionsFrame__1getRegionIsValidRegionCollection(JNIEnv * pEnv, jobject, jobject vectorobject) {
  jboolean iscopy;

  try {
    std::vector<ConvexPolygonObservableRegion> polygons;

    jclass vclazz = pEnv->GetObjectClass(vectorobject);
    jmethodID mid = _getMethodId_Checked(*pEnv, vclazz, "size", "()I");
    jint vsize = pEnv->CallIntMethod(vectorobject, mid);
    jni_error::_detectError(*pEnv);
    for (jint i=0; i < vsize; ++i) {
        mid = _getMethodId_Checked(*pEnv, vclazz, "elementAt", "(I)Ljava/lang/Object;");
        jstring str_object = (jstring)pEnv->CallObjectMethod(vectorobject, mid, i);
        jni_error::_detectError(*pEnv);
        const char * temp = pEnv->GetStringUTFChars(str_object, &iscopy);
        std::string poly_list(temp);
        polygons.push_back(ConvexPolygonBuilder::buildConvexPolygon(ConvexPolygonBuilder::parse(poly_list))); 
        if (iscopy == JNI_TRUE) pEnv->ReleaseStringUTFChars(str_object, temp);
    }

    //test that polygons do not overlap
    for (size_t i=0; i < polygons.size() - 1; ++i) {
        for (size_t j=i+1; j < polygons.size(); ++j) {
            if (polygons[i].intersectsRegion(polygons[j]))
              throw region_exception("The polygons you have defined are valid but there is overlap.\nPlease check polygons/inequalities and redefine to remove overlap.");
        }
    }
 
    return pEnv->NewStringUTF("Ok");
  }
  catch (jni_error & x) {
    return pEnv->NewStringUTF("Failed"); // let the Java exception to be handled in the caller of JNI function
  }
  catch (region_exception& e) {
    return pEnv->NewStringUTF(e.what());
  }
  catch (std::exception& x) {
	jni_error::_throwByName(*pEnv, jni_error::_javaRuntimeExceptionClassName, x.what());
    return pEnv->NewStringUTF("Failed");
  }
  catch (...) {
	jni_error::_throwByName(*pEnv, jni_error::_javaRuntimeExceptionClassName, "Unknown Program Error Encountered.");
    return pEnv->NewStringUTF("Failed");
  }
}

/** Calcuates the number of time intervals given study period start and end dates. */
JNIEXPORT jdouble JNICALL Java_org_satscan_gui_ParameterSettingsFrame_CalculateNumberOfTimeIntervals(JNIEnv * pEnv, jobject,
                                                                                      jstring startYear, jstring startMonth, jstring startDay,
                                                                                      jstring endYear, jstring endMonth, jstring endDay, jint iTimeAggregationType) {
   
  jboolean     iscopy;
  UInt         iYear, iMonth, iDay;
  Julian       startDate(0), endDate(0);
  std::string  yearBuffer;

  try {
     const char *sDateField = pEnv->GetStringUTFChars(startYear, &iscopy);
     yearBuffer = sDateField;
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

     if (iTimeAggregationType == GENERIC) {
       startDate = relativeDateToJulian(yearBuffer.c_str());
     } else {
        startDate = MDYToJulian(iMonth, iDay, iYear);
     }

     sDateField = pEnv->GetStringUTFChars(endYear, &iscopy);
     yearBuffer = sDateField;
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

     if (iTimeAggregationType == GENERIC) {
       endDate = relativeDateToJulian(yearBuffer.c_str());
     } else {
        endDate = MDYToJulian(iMonth, iDay, iYear);
     }

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

/* Java JNI call to email summary of most recent MultipleAnalyses execution. */
JNIEXPORT jint JNICALL Java_org_satscan_gui_BatchAnalysisFrame_SendSummaryEmail(JNIEnv*, jobject) {
    PrintNull null;
    try {
        MultipleAnalyses().emailSummary(null, false);
    } catch (std::exception& x) {
        return 1;
    }
    return 0;
}

JNIEXPORT jint JNICALL Java_org_satscan_app_CalculationThread_RunAnalysis(JNIEnv *pEnv, jobject JCalculationThread, jobject JParameters) {
  try {
    CParameters           Parameters;
    JNIPrintWindow        Console(*pEnv, JCalculationThread, false);
     
    try {
      ParametersUtility::copyJParametersToCParameters(*pEnv, JParameters, Parameters);
      _runAnalysis(Parameters, Console);
    } catch (resolvable_error & x) {
      Console.Printf("%s\nEnd of Warnings and Errors", BasePrint::P_ERROR, x.what());
      return 1;
    } catch (jni_error & x) {
      // Let the Java exception to be handled in the caller of JNI function.
      // It is preferable to report the error through the JNIPrintWindow
      // object but once a java error exists, our options are limited.
      return 1; 
    } catch (prg_exception & x) {
      Console.Printf("\nProgram Error Detected:\n%s\nEnd of Warnings and Errors", BasePrint::P_ERROR, x.what());
      Console.RecordCallpath(x.trace());
      return 1;
    } catch (std::bad_alloc &x) {
      Console.Printf("\nSaTScan is unable to perform analysis due to insuffient memory.\n"
                     "Please see 'Memory Requirements' in user guide for suggested solutions.\n"
                     "\nEnd of Warnings and Errors", BasePrint::P_ERROR);
      return 1;
    } catch (std::exception& x) {
      Console.Printf("\nProgram Error Detected:\n%s\nEnd of Warnings and Errors", BasePrint::P_ERROR, x.what());
      return 1;
    }  
  } catch (jni_error & x) {
    // Let the Java exception to be handled in the caller of JNI function.
    // It is preferable to report the error through the JNIPrintWindow
    // object but once a java error exists, our options are limited.
    return 1; 
  } catch (...) {
    jni_error::_throwByName(*pEnv, jni_error::_javaRuntimeExceptionClassName, "Unknown Program Error Encountered.");
    return 1;
  } 

  return 0;
}

/*
* Class:     org_satscan_gui_ApplicationPreferences
* Method:    SendTestMail
* Signature: (Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;
*/
JNIEXPORT jstring JNICALL Java_org_satscan_gui_ApplicationPreferences_SendTestMail(JNIEnv * pEnv, jobject apppref, jstring jservername, jstring jadditional, jstring jfrom, jstring jreply) {
    PrintNull print;
    try {
        std::string servername, additional, from, reply;
        jboolean iscopy;
        const char *temp = pEnv->GetStringUTFChars(jservername, &iscopy);
        servername = temp;
        if (iscopy == JNI_TRUE) pEnv->ReleaseStringUTFChars(jservername, temp);

        temp = pEnv->GetStringUTFChars(jadditional, &iscopy);
        additional = temp;
        if (iscopy == JNI_TRUE) pEnv->ReleaseStringUTFChars(jadditional, temp);

        temp = pEnv->GetStringUTFChars(jfrom, &iscopy);
        from = temp;
        if (iscopy == JNI_TRUE) pEnv->ReleaseStringUTFChars(jfrom, temp);

        temp = pEnv->GetStringUTFChars(jreply, &iscopy);
        reply = temp;
        if (iscopy == JNI_TRUE) pEnv->ReleaseStringUTFChars(jreply, temp);

        std::stringstream messagePlain, messageHTML, outputText, responseText;
        messagePlain << "SaTScan test email using curl." << std::endl;
        messageHTML << "<html><body><div><p>" << messagePlain.str() << "</p></div></body></html>";
        bool success = sendMail(from, { from }, {}, reply, "SaTScan Test Email", messagePlain, messageHTML, "", servername, print, true, additional, &outputText);
        responseText << "Send mail " << (success ? "succeeded!" : "errored!") << std::endl << std::endl << outputText.str();
        return pEnv->NewStringUTF(responseText.str().c_str());
    } catch (std::exception& x) {
        print.Printf("\nProgram Error Detected:\n%s\nEnd of Warnings and Errors", BasePrint::P_ERROR, x.what());
        return pEnv->NewStringUTF("Failed");
    } catch (jni_error & x) {
        // Let the Java exception to be handled in the caller of JNI function.
        // It is preferable to report the error through the JNIPrintWindow
        // object but once a java error exists, our options are limited.
        return pEnv->NewStringUTF("Failed");
    } catch (...) {
        jni_error::_throwByName(*pEnv, jni_error::_javaRuntimeExceptionClassName, "Unknown Program Error Encountered.");
        return pEnv->NewStringUTF("Failed");
    }
}

///////////////////////////////// C Shared Library Methods ///////////////////////////////////////////

/* Alternative ways of doing this.
1) The good old functionpointers.
2) Functionobjects (these are classes that define the operator () ()
3) The template way:
      template<typename Callable>
      void registerCallback(Callable const &call_back);
4) The Functors of the Lokilibrary.
*/

int DLL_EXP C_RunAnalysis(const char * filename, C_Callback* call_back) {
  CParameters           Parameters;
  C_PrintCallback       Console(call_back, false);
     
  try {
    ParameterAccessCoordinator reader(Parameters);
    reader.Read(filename, Console);
    _runAnalysis(Parameters, Console);
  } catch (resolvable_error & x) {
    Console.Printf("%s\nEnd of Warnings and Errors", BasePrint::P_ERROR, x.what());
    return 1;
  } catch (prg_exception & x) {
    Console.Printf("\nProgram Error Detected:\n%s\nEnd of Warnings and Errors", BasePrint::P_ERROR, x.what());
    return 1;
  } catch (std::bad_alloc &x) {
    Console.Printf("\nSaTScan is unable to perform analysis due to insuffient memory.\n"
                   "Please see 'Memory Requirements' in user guide for suggested solutions.\n"
                   "\nEnd of Warnings and Errors", BasePrint::P_ERROR);
    return 1;
  } catch (std::exception& x) {
    Console.Printf("\nProgram Error Detected:\n%s\nEnd of Warnings and Errors", BasePrint::P_ERROR, x.what());
    return 1;
  } catch (...) {
    Console.Printf("\nUnknown Program Error Encountered\n\nEnd of Warnings and Errors", BasePrint::P_ERROR);
    return 1;
  }
  
  return 0;
}

///////////////////////////////// Python Shared Library Methods ///////////////////////////////////////////

#ifdef _PYTHON_CALLBACK_
int DLL_EXP PY_RunAnalysis(const char * filename, PY_Callback* call_back) {

  //(C_Callback(call_back))("hello");
  //return 978;
  CParameters           Parameters;
  PY_PrintCallback       Console(call_back, false);
     
  try {
    ParameterAccessCoordinator reader(Parameters);
    reader.Read(filename, Console);
    _runAnalysis(Parameters, Console);
  } catch (resolvable_error & x) {
    Console.Printf("%s\nEnd of Warnings and Errors", BasePrint::P_ERROR, x.what());
    return 1;
  } catch (prg_exception & x) {
    Console.Printf("\nProgram Error Detected:\n%s\nEnd of Warnings and Errors", BasePrint::P_ERROR, x.what());
    return 1;
  } catch (std::bad_alloc &x) {
    Console.Printf("\nSaTScan is unable to perform analysis due to insuffient memory.\n"
                   "Please see 'Memory Requirements' in user guide for suggested solutions.\n"
                   "\nEnd of Warnings and Errors", BasePrint::P_ERROR);
    return 1;
  } catch (std::exception& x) {
    Console.Printf("\nProgram Error Detected:\n%s\nEnd of Warnings and Errors", BasePrint::P_ERROR, x.what());
    return 1;
  } catch (...) {
    Console.Printf("\nUnknown Program Error Encountered\n\nEnd of Warnings and Errors", BasePrint::P_ERROR);
    return 1;
  }
  
  return 0;
}
#endif

///////////////////////////////// VB Shared Library Methods ///////////////////////////////////////////

#ifdef _WINDOWS_
int DLL_EXP VB_RunAnalysis(const char * filename, long cbAddress) {
  CParameters       Parameters;
  VB_PrintCallback  Console(cbAddress, false);
     
  try {
    ParameterAccessCoordinator reader(Parameters);
    reader.Read(filename, Console);
    _runAnalysis(Parameters, Console);
  } catch (resolvable_error & x) {
    Console.Printf("%s\nEnd of Warnings and Errors", BasePrint::P_ERROR, x.what());
    return 1;
  } catch (prg_exception & x) {
    Console.Printf("\nProgram Error Detected:\n%s\nEnd of Warnings and Errors", BasePrint::P_ERROR, x.what());
    return 1;
  } catch (std::bad_alloc &x) {
    Console.Printf("\nSaTScan is unable to perform analysis due to insuffient memory.\n"
                   "Please see 'Memory Requirements' in user guide for suggested solutions.\n"
                   "\nEnd of Warnings and Errors", BasePrint::P_ERROR);
    return 1;
  } catch (std::exception& x) {
    Console.Printf("\nProgram Error Detected:\n%s\nEnd of Warnings and Errors", BasePrint::P_ERROR, x.what());
    return 1;
  } catch (...) {
    Console.Printf("\nUnknown Program Error Encountered\n\nEnd of Warnings and Errors", BasePrint::P_ERROR);
    return 1;
  }  
  return 0;
}
#endif
//******************************************************************************
