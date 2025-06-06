//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "stsParametersUtility.h"
#include "ParameterFileAccess.h"
#include "JNIException.h"
#include "DataSource.h"
#include "IniParameterFileAccess.h"

/** Reads parameters from file 'filename' in C++ code and sets class members of Java JParameters class. */
JNIEXPORT jboolean JNICALL Java_org_satscan_app_Parameters_Read(JNIEnv * pEnv, jobject jParameters, jstring filename) {
  CParameters           Parameters;
  jboolean              iscopy;

  try {
     const char *sParameterFilename = pEnv->GetStringUTFChars(filename, &iscopy);
     if (sParameterFilename) {
       PrintNull NoPrint;
       ParameterAccessCoordinator(Parameters).Read(sParameterFilename, NoPrint);
     }
     else {
       //New session - creation version is this version.
       CParameters::CreationVersion vVersion;
       Parameters.SetVersion(vVersion);
     }
     if (iscopy == JNI_TRUE)
     	pEnv->ReleaseStringUTFChars(filename, sParameterFilename);
     ParametersUtility::copyCParametersToJParameters(*pEnv, Parameters, jParameters);
  }
  catch (jni_error & x) {    
    return 1; // let the Java exception to be handled in the caller of JNI function
  }
  catch (prg_exception& x) {
      std::stringstream mssg;
      mssg << x.what() << std::endl << x.trace();
      jni_error::_throwByName(*pEnv, jni_error::_javaRuntimeExceptionClassName, mssg.str().c_str());
      return 1;
  }
  catch (std::exception& x) {
    jni_error::_throwByName(*pEnv, jni_error::_javaRuntimeExceptionClassName, x.what());
    return 1;
  }
  catch (...) {
	  jni_error::_throwByName(*pEnv, jni_error::_javaRuntimeExceptionClassName, "Unknown Program Error Encountered.");
    return 1;
  }
  return true;
}

/** Reads parameters from stringstream in C++ code and sets class members of Java JParameters class. */
JNIEXPORT jboolean JNICALL Java_org_satscan_app_Parameters_ReadFromStringStream(JNIEnv * pEnv, jobject jParameters, jstring parameterstream) {
    CParameters           Parameters;
    jboolean              iscopy;

    try {
        const char * sStringStream = pEnv->GetStringUTFChars(parameterstream, &iscopy);
        if (sStringStream) {
            PrintNull NoPrint;
            std::stringstream stream;
            stream >> std::noskipws; // Make sure stream doesn't skip whitespace.
            stream << sStringStream;
            IniParameterFileAccess(Parameters, NoPrint).Read(stream);
        } else {
            //New session - creation version is this version.
            CParameters::CreationVersion vVersion;
            Parameters.SetVersion(vVersion);
        }
        if (iscopy == JNI_TRUE)
            pEnv->ReleaseStringUTFChars(parameterstream, sStringStream);

        ParametersUtility::copyCParametersToJParameters(*pEnv, Parameters, jParameters);
    }
    catch (jni_error & x) {
        return 1; // let the Java exception to be handled in the caller of JNI function
    }
    catch (prg_exception& x) {
        std::stringstream mssg;
        mssg << x.what() << std::endl << x.trace();
        jni_error::_throwByName(*pEnv, jni_error::_javaRuntimeExceptionClassName, mssg.str().c_str());
        return 1;
    }
    catch (std::exception& x) {
        jni_error::_throwByName(*pEnv, jni_error::_javaRuntimeExceptionClassName, x.what());
        return 1;
    }
    catch (...) {
        jni_error::_throwByName(*pEnv, jni_error::_javaRuntimeExceptionClassName, "Unknown Program Error Encountered.");
        return 1;
    }
    return true;
}

/** Set parameters of C++ object from Java object and writes parameters to file 'filename'. */
JNIEXPORT void JNICALL Java_org_satscan_app_Parameters_Write(JNIEnv * pEnv, jobject jParameters, jstring) {
  CParameters   Parameters;

  try {
    ParametersUtility::copyJParametersToCParameters(*pEnv, jParameters, Parameters);
    PrintNull NoPrint;
    ParameterAccessCoordinator(Parameters).Write(Parameters.GetSourceFileName().c_str(), NoPrint);
  }
  catch (jni_error & x) {    
    return; // let the Java exception to be handled in the caller of JNI function
  }
  catch (prg_exception& x) {
    std::stringstream mssg;
    mssg << x.what() << std::endl << x.trace();
    jni_error::_throwByName(*pEnv, jni_error::_javaRuntimeExceptionClassName, mssg.str().c_str());
    return;
  }
  catch (std::exception& x) {
    jni_error::_throwByName(*pEnv, jni_error::_javaRuntimeExceptionClassName, x.what());
    return;
  }
  catch (...) {
    jni_error::_throwByName(*pEnv, jni_error::_javaRuntimeExceptionClassName, "Unknown Program Error Encountered.");
    return;
  }
}

/** Set parameters of C++ object from Java object and writes parameters to string. */
JNIEXPORT jstring JNICALL Java_org_satscan_app_Parameters_WriteToStringStream(JNIEnv * pEnv, jobject jParameters) {
    CParameters   Parameters;

    try {
        ParametersUtility::copyJParametersToCParameters(*pEnv, jParameters, Parameters);
        PrintNull NoPrint;
        std::stringstream stream;
        IniParameterFileAccess(Parameters, NoPrint).Write(stream);
        return pEnv->NewStringUTF(stream.str().c_str());
    } catch (jni_error & x) {
        return pEnv->NewStringUTF("");
    } catch (prg_exception& x) {
        std::stringstream mssg;
        mssg << x.what() << std::endl << x.trace();
        jni_error::_throwByName(*pEnv, jni_error::_javaRuntimeExceptionClassName, mssg.str().c_str());
        return pEnv->NewStringUTF("");
    } catch (std::exception& x) {
        jni_error::_throwByName(*pEnv, jni_error::_javaRuntimeExceptionClassName, x.what());
        return pEnv->NewStringUTF("");
    } catch (...) {
        jni_error::_throwByName(*pEnv, jni_error::_javaRuntimeExceptionClassName, "Unknown Program Error Encountered.");
        return pEnv->NewStringUTF("");
    }
}

/** Returns ordinal of enumeration gotten from 'sFunctionName' called. */
int ParametersUtility::getEnumTypeOrdinalIndex(JNIEnv& Env, jobject& object, const char * sFunctionName, const char * sEnumClassSignature) {
  jclass clazz = Env.GetObjectClass(object);
  jmethodID mid = _getMethodId_Checked(Env, clazz, sFunctionName, std::string(std::string("()") + sEnumClassSignature).c_str());
  jobject t_object = Env.CallObjectMethod(object, mid);
  jni_error::_detectError(Env);
  jclass t_class = Env.FindClass(sEnumClassSignature);
  jmethodID t_mid = _getMethodId_Checked(Env, t_class, "ordinal", "()I");
  int value = Env.CallIntMethod(t_object, t_mid);
  jni_error::_detectError(Env);
  return value;

}

/** Copies CParameter object data members to JParameters object. */
jobject& ParametersUtility::copyCParametersToJParameters(JNIEnv& Env, CParameters& Parameters, jobject& jParameters) {
  //set jParameters object from data members of CParameters class
  jclass clazz = Env.GetObjectClass(jParameters);

  jmethodID mid = _getMethodId_Checked(Env, clazz, "SetNumParallelProcessesToExecute", "(I)V");
  Env.CallVoidMethod(jParameters, mid, (jint)Parameters.GetNumRequestedParallelProcesses());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "SetExecutionType", "(I)V");
  Env.CallVoidMethod(jParameters, mid, (jint)Parameters.GetExecutionType());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "SetMultipleDataSetPurposeType", "(I)V");
  Env.CallVoidMethod(jParameters, mid, (jint)Parameters.GetMultipleDataSetPurposeType());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "SetAnalysisType", "(I)V");
  Env.CallVoidMethod(jParameters, mid, (jint)Parameters.GetAnalysisType() - 1);
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "SetProbabilityModelType", "(I)V");
  Env.CallVoidMethod(jParameters, mid, (jint)Parameters.GetProbabilityModelType());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "SetAreaRateType", "(I)V");
  Env.CallVoidMethod(jParameters, mid, (jint)Parameters.GetAreaScanRateType() - 1);
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "SetRiskType", "(I)V");
  Env.CallVoidMethod(jParameters, mid, (jint)Parameters.GetRiskType());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "SetIncludeClustersType", "(I)V");
  Env.CallVoidMethod(jParameters, mid, (jint)Parameters.GetIncludeClustersType());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "SetNumberMonteCarloReplications", "(I)V");
  Env.CallVoidMethod(jParameters, mid, (jint)Parameters.GetNumReplicationsRequested());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "SetCriteriaForReportingSecondaryClusters", "(I)V");
  Env.CallVoidMethod(jParameters, mid, (jint)Parameters.GetCriteriaSecondClustersType());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "SetTimeTrendConvergence", "(D)V");
  Env.CallVoidMethod(jParameters, mid, (jdouble)Parameters.GetTimeTrendConvergence());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "SetNonparametricAdjustmentSize", "(I)V");
  Env.CallVoidMethod(jParameters, mid, (jint)Parameters.GetNonparametricAdjustmentSize());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "SetPValueReportingType", "(I)V");
  Env.CallVoidMethod(jParameters, mid, (jint)Parameters.GetPValueReportingType());
  jni_error::_detectError(Env);
  mid = _getMethodId_Checked(Env, clazz, "SetEarlyTermThreshold", "(I)V");
  Env.CallVoidMethod(jParameters, mid, (jint)Parameters.GetEarlyTermThreshold());
  jni_error::_detectError(Env);
  mid = _getMethodId_Checked(Env, clazz, "SetReportGumbelPValue", "(Z)V");
  Env.CallVoidMethod(jParameters, mid, (jboolean)Parameters.GetReportGumbelPValue());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "SetSimulationType", "(I)V");
  Env.CallVoidMethod(jParameters, mid, (jint)Parameters.GetSimulationType());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "SetOutputSimulationData", "(Z)V");
  Env.CallVoidMethod(jParameters, mid, (jboolean)Parameters.GetOutputSimulationData());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "SetAdjustForEarlierAnalyses", "(Z)V");
  Env.CallVoidMethod(jParameters, mid, (jboolean)Parameters.GetAdjustForEarlierAnalyses());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "SetSpatialAdjustmentType", "(I)V");
  Env.CallVoidMethod(jParameters, mid, (jint)Parameters.GetSpatialAdjustmentType());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "SetStudyPeriodDataCheckingType", "(I)V");
  Env.CallVoidMethod(jParameters, mid, (jint)Parameters.GetStudyPeriodDataCheckingType());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "SetCoordinatesDataCheckingType", "(I)V");
  Env.CallVoidMethod(jParameters, mid, (jint)Parameters.GetCoordinatesDataCheckingType());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "SetMaxSpatialSizeForType", "(IDZ)V");
  Env.CallVoidMethod(jParameters, mid, (jint)PERCENTOFPOPULATION, (jdouble)Parameters.GetMaxSpatialSizeForType(PERCENTOFPOPULATION, false), false);
  jni_error::_detectError(Env);
  Env.CallVoidMethod(jParameters, mid, (jint)PERCENTOFMAXCIRCLEFILE, (jdouble)Parameters.GetMaxSpatialSizeForType(PERCENTOFMAXCIRCLEFILE, false), false);
  jni_error::_detectError(Env);
  Env.CallVoidMethod(jParameters, mid, (jint)MAXDISTANCE, (jdouble)Parameters.GetMaxSpatialSizeForType(MAXDISTANCE, false), false);
  jni_error::_detectError(Env);
  Env.CallVoidMethod(jParameters, mid, (jint)PERCENTOFPOPULATION, (jdouble)Parameters.GetMaxSpatialSizeForType(PERCENTOFPOPULATION, true), true);
  jni_error::_detectError(Env);
  Env.CallVoidMethod(jParameters, mid, (jint)PERCENTOFMAXCIRCLEFILE, (jdouble)Parameters.GetMaxSpatialSizeForType(PERCENTOFMAXCIRCLEFILE, true), true);
  jni_error::_detectError(Env);
  Env.CallVoidMethod(jParameters, mid, (jint)MAXDISTANCE, (jdouble)Parameters.GetMaxSpatialSizeForType(MAXDISTANCE, true), true);
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "SetRestrictMaxSpatialSizeForType", "(IZZ)V");
  Env.CallVoidMethod(jParameters, mid, (jint)PERCENTOFPOPULATION, Parameters.GetRestrictMaxSpatialSizeForType(PERCENTOFPOPULATION, false), false);
  jni_error::_detectError(Env);
  Env.CallVoidMethod(jParameters, mid, (jint)PERCENTOFMAXCIRCLEFILE, Parameters.GetRestrictMaxSpatialSizeForType(PERCENTOFMAXCIRCLEFILE, false), false);
  jni_error::_detectError(Env);
  Env.CallVoidMethod(jParameters, mid, (jint)MAXDISTANCE, Parameters.GetRestrictMaxSpatialSizeForType(MAXDISTANCE, false), false);
  jni_error::_detectError(Env);
  Env.CallVoidMethod(jParameters, mid, (jint)PERCENTOFPOPULATION, Parameters.GetRestrictMaxSpatialSizeForType(PERCENTOFPOPULATION, true), true);
  jni_error::_detectError(Env);
  Env.CallVoidMethod(jParameters, mid, (jint)PERCENTOFMAXCIRCLEFILE, Parameters.GetRestrictMaxSpatialSizeForType(PERCENTOFMAXCIRCLEFILE, true), true);
  jni_error::_detectError(Env);
  Env.CallVoidMethod(jParameters, mid, (jint)MAXDISTANCE, Parameters.GetRestrictMaxSpatialSizeForType(MAXDISTANCE, true), true);
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "SetRestrictReportedClusters", "(Z)V");
  Env.CallVoidMethod(jParameters, mid, (jboolean)Parameters.GetRestrictingMaximumReportedGeoClusterSize());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "SetMaximumTemporalClusterSize", "(D)V");
  Env.CallVoidMethod(jParameters, mid, (jdouble)Parameters.GetMaximumTemporalClusterSize());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "SetMaximumTemporalClusterSizeType", "(I)V");
  Env.CallVoidMethod(jParameters, mid, (jint)Parameters.GetMaximumTemporalClusterSizeType());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "SetTimeAggregationLength", "(I)V");
  Env.CallVoidMethod(jParameters, mid, (jint)Parameters.GetTimeAggregationLength());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "SetTimeAggregationUnitsType", "(I)V");
  Env.CallVoidMethod(jParameters, mid, (jint)Parameters.GetTimeAggregationUnitsType());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "SetTimeTrendAdjustmentPercentage", "(D)V");
  Env.CallVoidMethod(jParameters, mid, (jdouble)Parameters.GetTimeTrendAdjustmentPercentage());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "SetTimeTrendAdjustmentType", "(I)V");
  Env.CallVoidMethod(jParameters, mid, (jint)Parameters.GetTimeTrendAdjustmentType());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "SetPrecisionOfTimesType", "(I)V");
  Env.CallVoidMethod(jParameters, mid, (jint)Parameters.GetPrecisionOfTimesType());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "SetCoordinatesType", "(I)V");
  Env.CallVoidMethod(jParameters, mid, (jint)Parameters.GetCoordinatesType());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "AddEllipsoidShape", "(DZ)V");
  for (size_t t=0; t < Parameters.GetEllipseShapes().size(); ++t) {
    Env.CallVoidMethod(jParameters, mid, (jdouble)Parameters.GetEllipseShapes()[t], (t ? JNI_FALSE : JNI_TRUE));
    jni_error::_detectError(Env);
  }

  mid = _getMethodId_Checked(Env, clazz, "AddEllipsoidRotations", "(IZ)V");
  for (size_t t=0; t < Parameters.GetEllipseRotations().size(); ++t) {
    Env.CallVoidMethod(jParameters, mid, (jint)Parameters.GetEllipseRotations()[t], (t ? JNI_FALSE : JNI_TRUE));
    jni_error::_detectError(Env);
  }

  mid = _getMethodId_Checked(Env, clazz, "SetNonCompactnessPenalty", "(I)V");
  Env.CallVoidMethod(jParameters, mid, (jint)Parameters.GetNonCompactnessPenaltyType());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "SetIncludePurelySpatialClusters", "(Z)V");
  Env.CallVoidMethod(jParameters, mid, (jboolean)Parameters.GetIncludePurelySpatialClusters());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "SetIncludePurelyTemporalClusters", "(Z)V");
  Env.CallVoidMethod(jParameters, mid, (jboolean)Parameters.GetIncludePurelyTemporalClusters());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "SetOutputSimLogLikeliRatiosAscii", "(Z)V");
  Env.CallVoidMethod(jParameters, mid, (jboolean)Parameters.GetOutputSimLoglikeliRatiosAscii());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "SetOutputSimLogLikeliRatiosDBase", "(Z)V");
  Env.CallVoidMethod(jParameters, mid, (jboolean)Parameters.GetOutputSimLoglikeliRatiosDBase());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "SetOutputRelativeRisksAscii", "(Z)V");
  Env.CallVoidMethod(jParameters, mid, (jboolean)Parameters.GetOutputRelativeRisksAscii());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "SetOutputRelativeRisksDBase", "(Z)V");
  Env.CallVoidMethod(jParameters, mid, (jboolean)Parameters.GetOutputRelativeRisksDBase());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "SetOutputClusterLevelAscii", "(Z)V");
  Env.CallVoidMethod(jParameters, mid, (jboolean)Parameters.GetOutputClusterLevelAscii());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "SetOutputClusterLevelDBase", "(Z)V");
  Env.CallVoidMethod(jParameters, mid, (jboolean)Parameters.GetOutputClusterLevelDBase());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "SetOutputClusterCaseAscii", "(Z)V");
  Env.CallVoidMethod(jParameters, mid, (jboolean)Parameters.GetOutputClusterCaseAscii());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "SetOutputClusterCaseDBase", "(Z)V");
  Env.CallVoidMethod(jParameters, mid, (jboolean)Parameters.GetOutputClusterCaseDBase());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "SetOutputAreaSpecificAscii", "(Z)V");
  Env.CallVoidMethod(jParameters, mid, (jboolean)Parameters.GetOutputAreaSpecificAscii());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "SetOutputAreaSpecificDBase", "(Z)V");
  Env.CallVoidMethod(jParameters, mid, (jboolean)Parameters.GetOutputAreaSpecificDBase());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "SetIterativeScanning", "(Z)V");
  Env.CallVoidMethod(jParameters, mid, (jboolean)Parameters.GetIsIterativeScanning());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "SetNumIterativeScans", "(I)V");
  Env.CallVoidMethod(jParameters, mid, (jboolean)Parameters.GetNumIterativeScansRequested());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "SetIterativeCutOffPValue", "(D)V");
  Env.CallVoidMethod(jParameters, mid, (jdouble)Parameters.GetIterativeCutOffPValue());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "SetSourceFileName", "(Ljava/lang/String;)V");
  Env.CallVoidMethod(jParameters, mid, Env.NewStringUTF(Parameters.GetSourceFileName().c_str()));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setDataSourceName", "(Ljava/lang/String;I)V");
  for (size_t t = 0; t < Parameters.getDataSourceNames().size(); ++t) {
      Env.CallVoidMethod(jParameters, mid, Env.NewStringUTF(Parameters.getDataSourceNames()[t].c_str()), (jint)(t + 1));
      jni_error::_detectError(Env);
  }

  mid = _getMethodId_Checked(Env, clazz, "SetCaseFileName", "(Ljava/lang/String;I)V");
  for (size_t t=0; t < Parameters.GetCaseFileNames().size(); ++t) {
    Env.CallVoidMethod(jParameters, mid, Env.NewStringUTF(Parameters.GetCaseFileNames()[t].c_str()), (jint)(t + 1));
    jni_error::_detectError(Env);
  }

  mid = _getMethodId_Checked(Env, clazz, "SetControlFileName", "(Ljava/lang/String;I)V");
  for (size_t t=0; t < Parameters.GetControlFileNames().size(); ++t) {
    Env.CallVoidMethod(jParameters, mid, Env.NewStringUTF(Parameters.GetControlFileNames()[t].c_str()), (jint)(t + 1));
    jni_error::_detectError(Env);
  }

  mid = _getMethodId_Checked(Env, clazz, "SetPopulationFileName", "(Ljava/lang/String;I)V");
  for (size_t t=0; t < Parameters.GetPopulationFileNames().size(); ++t) {
    Env.CallVoidMethod(jParameters, mid, Env.NewStringUTF(Parameters.GetPopulationFileNames()[t].c_str()), (jint)(t + 1));
      jni_error::_detectError(Env);
  }
  mid = _getMethodId_Checked(Env, clazz, "SetCoordinatesFileName", "(Ljava/lang/String;)V");
  Env.CallVoidMethod(jParameters, mid, Env.NewStringUTF(Parameters.GetCoordinatesFileName().c_str()));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "SetSpecialGridFileName", "(Ljava/lang/String;)V");
  Env.CallVoidMethod(jParameters, mid, Env.NewStringUTF(Parameters.GetSpecialGridFileName().c_str()));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "SetMaxCirclePopulationFileName", "(Ljava/lang/String;)V");
  Env.CallVoidMethod(jParameters, mid, Env.NewStringUTF(Parameters.GetMaxCirclePopulationFileName().c_str()));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "SetOutputFileNameSetting", "(Ljava/lang/String;)V");
  Env.CallVoidMethod(jParameters, mid, Env.NewStringUTF(Parameters.GetOutputFileNameSetting().c_str()));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "SetIsLoggingHistory", "(Z)V");
  Env.CallVoidMethod(jParameters, mid, Parameters.GetIsLoggingHistory());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "SetSimulationDataSourceFileName", "(Ljava/lang/String;)V");
  Env.CallVoidMethod(jParameters, mid, Env.NewStringUTF(Parameters.GetSimulationDataSourceFilename().c_str()));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "SetUseAdjustmentForRelativeRisksFile", "(Z)V");
  Env.CallVoidMethod(jParameters, mid, Parameters.UseAdjustmentForRelativeRisksFile());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "SetAdjustmentsByRelativeRisksFilename", "(Ljava/lang/String;)V");
  Env.CallVoidMethod(jParameters, mid, Env.NewStringUTF(Parameters.GetAdjustmentsByRelativeRisksFilename().c_str()));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "SetSimulationDataOutputFileName", "(Ljava/lang/String;)V");
  Env.CallVoidMethod(jParameters, mid, Env.NewStringUTF(Parameters.GetSimulationDataOutputFilename().c_str()));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "SetProspectiveStartDate", "(Ljava/lang/String;)V");
  Env.CallVoidMethod(jParameters, mid, Env.NewStringUTF(Parameters.GetProspectiveStartDate().c_str()));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "SetStudyPeriodStartDate", "(Ljava/lang/String;)V");
  Env.CallVoidMethod(jParameters, mid, Env.NewStringUTF(Parameters.GetStudyPeriodStartDate().c_str()));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "SetStudyPeriodEndDate", "(Ljava/lang/String;)V");
  Env.CallVoidMethod(jParameters, mid, Env.NewStringUTF(Parameters.GetStudyPeriodEndDate().c_str()));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "SetEndRangeStartDate", "(Ljava/lang/String;)V");
  Env.CallVoidMethod(jParameters, mid, Env.NewStringUTF(Parameters.GetEndRangeStartDate().c_str()));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "SetEndRangeEndDate", "(Ljava/lang/String;)V");
  Env.CallVoidMethod(jParameters, mid, Env.NewStringUTF(Parameters.GetEndRangeEndDate().c_str()));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "SetStartRangeEndDate", "(Ljava/lang/String;)V");
  Env.CallVoidMethod(jParameters, mid, Env.NewStringUTF(Parameters.GetStartRangeEndDate().c_str()));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "SetStartRangeStartDate", "(Ljava/lang/String;)V");
  Env.CallVoidMethod(jParameters, mid, Env.NewStringUTF(Parameters.GetStartRangeStartDate().c_str()));
  jni_error::_detectError(Env);

  //problems???

  jfieldID vfid = _getFieldId_Checked(Env, clazz, "gCreationVersion", "Lorg/satscan/app/Parameters$CreationVersion;");
  jobject versionobject = Env.GetObjectField(jParameters, vfid);
  jclass vclazz = Env.GetObjectClass(versionobject);
  vfid = _getFieldId_Checked(Env, vclazz, "giMajor", "I");
  Env.SetIntField(versionobject, vfid, (jint)Parameters.GetCreationVersion().iMajor);
  jni_error::_detectError(Env);
  vfid = _getFieldId_Checked(Env, vclazz, "giMinor", "I");
  Env.SetIntField(versionobject, vfid, (jint)Parameters.GetCreationVersion().iMinor);
  jni_error::_detectError(Env);
  vfid = _getFieldId_Checked(Env, vclazz, "giRelease", "I");
  Env.SetIntField(versionobject, vfid, (jint)Parameters.GetCreationVersion().iRelease);
  jni_error::_detectError(Env);

  //jclass c_clazz = Env.FindClass("Lorg/satscan/app/Parameters$CreationVersion;");
  //mid = Env.GetMethodID(c_clazz, "<init>","(III)V"); <---- returned method is null???
  //jobject c = Env.NewObject(c_clazz, mid, (jint)Parameters.GetCreationVersion().iMajor, (jint)Parameters.GetCreationVersion().iMinor, (jint)Parameters.GetCreationVersion().iRelease);
  //mid = Env.GetMethodID(clazz, "SetVersion", "(Lorg/satscan/app/Parameters$CreationVersion;)V");
  //Env.CallVoidMethod(jParameters, mid, c);

  //problems???

  mid = _getMethodId_Checked(Env, clazz, "SetRandomizationSeed", "(I)V");
  Env.CallVoidMethod(jParameters, mid, (jint)Parameters.GetRandomizationSeed());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "SetReportCriticalValues", "(Z)V");
  Env.CallVoidMethod(jParameters, mid, (jboolean)Parameters.GetReportCriticalValues());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "SetSuppressingWarnings", "(Z)V");
  Env.CallVoidMethod(jParameters, mid, (jboolean)Parameters.GetSuppressingWarnings());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "SetSpatialWindowType", "(I)V");
  Env.CallVoidMethod(jParameters, mid, (jint)Parameters.GetSpatialWindowType());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "UseLocationNeighborsFile", "(Z)V");
  Env.CallVoidMethod(jParameters, mid, Parameters.UseLocationNeighborsFile());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "SetLocationNeighborsFileName", "(Ljava/lang/String;)V");
  Env.CallVoidMethod(jParameters, mid, Env.NewStringUTF(Parameters.GetLocationNeighborsFileName().c_str()));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "SetMultipleCoordinatesType", "(I)V");
  Env.CallVoidMethod(jParameters, mid, (jint)Parameters.GetMultipleCoordinatesType());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "SetMetaLocationsFileName", "(Ljava/lang/String;)V");
  Env.CallVoidMethod(jParameters, mid, Env.NewStringUTF(Parameters.getMetaLocationsFilename().c_str()));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "UseMetaLocationsFile", "(Z)V");
  Env.CallVoidMethod(jParameters, mid, Parameters.UseMetaLocationsFile());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "AddObservableRegion", "(Ljava/lang/String;IZ)V");
  for (size_t t=0; t < Parameters.getObservableRegions().size(); ++t) {
    Env.CallVoidMethod(jParameters, mid, Env.NewStringUTF(Parameters.getObservableRegions()[t].c_str()), (jint)(t), (t ? JNI_FALSE : JNI_TRUE));
    jni_error::_detectError(Env);
  }

  mid = _getMethodId_Checked(Env, clazz, "clearInputSourceSettings", "()V");
  Env.CallVoidMethod(jParameters, mid);
  jni_error::_detectError(Env);

  jmethodID mid_add_source = _getMethodId_Checked(Env, clazz, "addInputSourceSettings", "(Lorg/satscan/importer/InputSourceSettings;)V");
  jclass issclazz = Env.FindClass("org/satscan/importer/InputSourceSettings");
  jmethodID mid_constructor = _getMethodId_Checked(Env, issclazz, "<init>", "()V");
  CParameters::InputSourceContainer_t::const_iterator itr=Parameters.getInputSources().begin();
  for (; itr != Parameters.getInputSources().end(); ++itr) {
      const CParameters::InputSourceKey_t& key = itr->first;
      const CParameters::InputSource& iss = itr->second;
      jobject issobject = Env.NewObject(issclazz, mid_constructor);
      Env.CallVoidMethod(jParameters, mid_add_source, issobject);

      // translate ParameterType to Java InputSourceSettings.InputFileType
      mid = _getMethodId_Checked(Env, issclazz, "setInputFileType", "(I)V");
      switch (key.first) {
        case CASEFILE : Env.CallVoidMethod(issobject, mid, (jint)0); break;
        case CONTROLFILE : Env.CallVoidMethod(issobject, mid, (jint)1); break;
        case POPFILE : Env.CallVoidMethod(issobject, mid, (jint)2); break;
        case COORDFILE : Env.CallVoidMethod(issobject, mid, (jint)3); break;
        case GRIDFILE : Env.CallVoidMethod(issobject, mid, (jint)4); break;
        case MAXCIRCLEPOPFILE : Env.CallVoidMethod(issobject, mid, (jint)5); break;
        case ADJ_BY_RR_FILE : Env.CallVoidMethod(issobject, mid, (jint)6); break;
        case NETWORK_FILE : Env.CallVoidMethod(issobject, mid, (jint)7); break;
        case MULTIPLE_LOCATIONS_FILE: Env.CallVoidMethod(issobject, mid, (jint)11); break;
        default : throw prg_error("Unknown parameter type for translation: %d", "copyCParametersToJParameters()", key.first);
      }

      mid = _getMethodId_Checked(Env, issclazz, "setDataSetIndex", "(I)V");
      Env.CallVoidMethod(issobject, mid, (jint)key.second);
      jni_error::_detectError(Env);

      mid = _getMethodId_Checked(Env, issclazz, "setSourceDataFileType", "(I)V");
      Env.CallVoidMethod(issobject, mid, (jint)iss.getSourceType());

      mid = _getMethodId_Checked(Env, issclazz, "setDelimiter", "(Ljava/lang/String;)V");
      Env.CallVoidMethod(issobject, mid, Env.NewStringUTF(iss.getDelimiter().c_str()));
      jni_error::_detectError(Env);

      mid = _getMethodId_Checked(Env, issclazz, "setGroup", "(Ljava/lang/String;)V");
      Env.CallVoidMethod(issobject, mid, Env.NewStringUTF(iss.getGroup().c_str()));
      jni_error::_detectError(Env);

      mid = _getMethodId_Checked(Env, issclazz, "setSkiplines", "(I)V");
      Env.CallVoidMethod(issobject, mid, (jint)iss.getSkip());
      jni_error::_detectError(Env);

      mid = _getMethodId_Checked(Env, issclazz, "setFirstRowHeader", "(Z)V");
      Env.CallVoidMethod(issobject, mid, (jboolean)iss.getFirstRowHeader());
      jni_error::_detectError(Env);

      mid = _getMethodId_Checked(Env, issclazz, "addFieldMapping", "(Ljava/lang/String;)V");
      FieldMapContainer_t::const_iterator itrMap=iss.getFieldsMap().begin();
      for (;itrMap != iss.getFieldsMap().end(); ++itrMap) {
          std::stringstream s;
          if (itrMap->type() == typeid(long)) {
              long c = boost::any_cast<long>(*itrMap);
              if (c == 0) s << c;
              else s << (boost::any_cast<long>(*itrMap) + (iss.getSourceType() == SHAPE ? 4 : 2));
          } else if (itrMap->type() == typeid(DataSource::FieldType)) {
              switch (boost::any_cast<DataSource::FieldType>(*itrMap)) {
                case DataSource::GENERATEDID : s << 1; break;
                case DataSource::ONECOUNT : s << 2; break;
                case DataSource::DEFAULT_DATE : s << 0; break;
                case DataSource::BLANK : s << ""; break;
                default : throw prg_error("Unknown type '%s'.", "WriteInputSource()", boost::any_cast<DataSource::FieldType>(*itr));
              }
          } else if (itrMap->type() == typeid(DataSource::ShapeFieldType)) {
              switch (boost::any_cast<DataSource::ShapeFieldType>(*itrMap)) {
                case DataSource::POINTX   : s << 3; break;
                case DataSource::POINTY   : s << 4; break;
                default : throw prg_error("Unknown type '%s'.", "WriteInputSource()", boost::any_cast<ShapeFileDataSource::ShapeFieldType>(*itr));
              }
          } else {
            throw prg_error("Unknown type '%s'.", "WriteInputSource()", itrMap->type().name());
          }
          Env.CallVoidMethod(issobject, mid, Env.NewStringUTF(s.str().c_str()));
          jni_error::_detectError(Env);
      }
      // If file type is Case file, read any line list mappings.
      if (key.first == CASEFILE) {
        mid = _getMethodId_Checked(Env, issclazz, "addLinelistFieldMapping", "(IILjava/lang/String;)V");
        for (auto llmap: iss.getLinelistFieldsMap()) {
            Env.CallVoidMethod(issobject, mid, llmap.get<0>(), llmap.get<1>(), Env.NewStringUTF(llmap.get<2>().c_str()));
            jni_error::_detectError(Env);
        }
      }
  }

  mid = _getMethodId_Checked(Env, clazz, "SetTimeTrendType", "(I)V");
  Env.CallVoidMethod(jParameters, mid, (jint)Parameters.getTimeTrendType());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setReportClusterRank", "(Z)V");
  Env.CallVoidMethod(jParameters, mid, (jboolean)Parameters.getReportClusterRank());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setPrintAsciiHeaders", "(Z)V");
  Env.CallVoidMethod(jParameters, mid, (jboolean)Parameters.getPrintAsciiHeaders());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setReportHierarchicalClusters", "(Z)V");
  Env.CallVoidMethod(jParameters, mid, (jint)Parameters.getReportHierarchicalClusters());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setReportGiniOptimizedClusters", "(Z)V");
  Env.CallVoidMethod(jParameters, mid, (jint)Parameters.getReportGiniOptimizedClusters());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setGiniIndexReportType", "(I)V");
  Env.CallVoidMethod(jParameters, mid, (jint)Parameters.getGiniIndexReportType());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setReportGiniIndexCoefficents", "(Z)V");
  Env.CallVoidMethod(jParameters, mid, (jboolean)Parameters.getReportGiniIndexCoefficents());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setPerformPowerEvaluation", "(Z)V");
  Env.CallVoidMethod(jParameters, mid, (jboolean)Parameters.getPerformPowerEvaluation());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setPowerEvaluationCriticalValueType", "(I)V");
  Env.CallVoidMethod(jParameters, mid, (jint)Parameters.getPowerEvaluationCriticalValueType());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setPowerEstimationType", "(I)V");
  Env.CallVoidMethod(jParameters, mid, (jint)Parameters.getPowerEstimationType());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setPowerEvaluationMethod", "(I)V");
  Env.CallVoidMethod(jParameters, mid, (jint)Parameters.getPowerEvaluationMethod());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setPowerEvaluationAltHypothesisFilename", "(Ljava/lang/String;)V");
  Env.CallVoidMethod(jParameters, mid, Env.NewStringUTF(Parameters.getPowerEvaluationAltHypothesisFilename().c_str()));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setPowerEvaluationCaseCount", "(I)V");
  Env.CallVoidMethod(jParameters, mid, (jint)Parameters.getPowerEvaluationCaseCount());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setNumPowerEvalReplicaPowerStep", "(I)V");
  Env.CallVoidMethod(jParameters, mid, (jint)Parameters.getNumPowerEvalReplicaPowerStep());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setOutputKMLFile", "(Z)V");
  Env.CallVoidMethod(jParameters, mid, (jboolean)Parameters.getOutputKMLFile());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setOutputTemporalGraphFile", "(Z)V");
  Env.CallVoidMethod(jParameters, mid, (jboolean)Parameters.getOutputTemporalGraphFile());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setAdjustForWeeklyTrends", "(Z)V");
  Env.CallVoidMethod(jParameters, mid, (jboolean)Parameters.getAdjustForWeeklyTrends());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setIncludeLocationsKML", "(Z)V");
  Env.CallVoidMethod(jParameters, mid, (jboolean)Parameters.getIncludeLocationsKML());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setCompressClusterKML", "(Z)V");
  Env.CallVoidMethod(jParameters, mid, (jboolean)Parameters.getCompressClusterKML());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setLaunchMapViewer", "(Z)V");
  Env.CallVoidMethod(jParameters, mid, (jboolean)Parameters.getLaunchMapViewer());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setOutputShapeFiles", "(Z)V");
  Env.CallVoidMethod(jParameters, mid, (jboolean)Parameters.getOutputShapeFiles());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setMinimumTemporalClusterSize", "(I)V");
  Env.CallVoidMethod(jParameters, mid, (jint)Parameters.getMinimumTemporalClusterSize());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setTemporalGraphReportType", "(I)V");
  Env.CallVoidMethod(jParameters, mid, (jint)Parameters.getTemporalGraphReportType());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setTemporalGraphSignificantCutoff", "(D)V");
  Env.CallVoidMethod(jParameters, mid, (jdouble)Parameters.getTemporalGraphSignificantCutoff());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setTemporalGraphMostLikelyCount", "(I)V");
  Env.CallVoidMethod(jParameters, mid, (jint)Parameters.getTemporalGraphMostLikelyCount());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "SetTitleName", "(Ljava/lang/String;)V");
  Env.CallVoidMethod(jParameters, mid, Env.NewStringUTF(Parameters.GetTitleName().c_str()));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setCalculateOliveirasF", "(Z)V");
  Env.CallVoidMethod(jParameters, mid, (jboolean)Parameters.getCalculateOliveirasF());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setNumRequestedOliveiraSets", "(I)V");
  Env.CallVoidMethod(jParameters, mid, (jint)Parameters.getNumRequestedOliveiraSets());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setOutputCartesianGraph", "(Z)V");
  Env.CallVoidMethod(jParameters, mid, (jboolean)Parameters.getOutputCartesianGraph());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setRiskLimitHighClusters", "(Z)V");
  Env.CallVoidMethod(jParameters, mid, (jboolean)Parameters.getRiskLimitHighClusters());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setRiskThresholdHighClusters", "(D)V");
  Env.CallVoidMethod(jParameters, mid, (jdouble)Parameters.getRiskThresholdHighClusters());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setRiskLimitLowClusters", "(Z)V");
  Env.CallVoidMethod(jParameters, mid, (jboolean)Parameters.getRiskLimitLowClusters());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setRiskThresholdLowClusters", "(D)V");
  Env.CallVoidMethod(jParameters, mid, (jdouble)Parameters.getRiskThresholdLowClusters());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setMinimumCasesHighRateClusters", "(I)V");
  Env.CallVoidMethod(jParameters, mid, (jint)Parameters.getMinimumCasesHighRateClusters());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setOutputGoogleMapsFile", "(Z)V");
  Env.CallVoidMethod(jParameters, mid, (jboolean)Parameters.getOutputGoogleMapsFile());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setPerformStandardDrilldown", "(Z)V");
  Env.CallVoidMethod(jParameters, mid, (jboolean)Parameters.getPerformStandardDrilldown());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setPerformBernoulliDrilldown", "(Z)V");
  Env.CallVoidMethod(jParameters, mid, (jboolean)Parameters.getPerformBernoulliDrilldown());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setDrilldownMinimumLocationsCluster", "(I)V");
  Env.CallVoidMethod(jParameters, mid, (jint)Parameters.getDrilldownMinimumLocationsCluster());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setDrilldownMinimumCasesCluster", "(I)V");
  Env.CallVoidMethod(jParameters, mid, (jint)Parameters.getDrilldownMinimumCasesCluster());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setDrilldownCutoff", "(D)V");
  Env.CallVoidMethod(jParameters, mid, (jdouble)Parameters.getDrilldownCutoff());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setUseLocationsNetworkFile", "(Z)V");
  Env.CallVoidMethod(jParameters, mid, (jboolean)Parameters.getUseLocationsNetworkFile());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setLocationsNetworkFilename", "(Ljava/lang/String;)V");
  Env.CallVoidMethod(jParameters, mid, Env.NewStringUTF(Parameters.getLocationsNetworkFilename().c_str()));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setProspectiveFrequencyType", "(I)V");
  Env.CallVoidMethod(jParameters, mid, (jint)Parameters.getProspectiveFrequencyType());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setLinelistIndividualsCacheFileName", "(Ljava/lang/String;)V");
  Env.CallVoidMethod(jParameters, mid, Env.NewStringUTF(Parameters.getLinelistIndividualsCacheFileName().c_str()));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setCutoffLineListCSV", "(D)V");
  Env.CallVoidMethod(jParameters, mid, (jdouble)Parameters.getCutoffLineListCSV());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setAlwaysEmailSummary", "(Z)V");
  Env.CallVoidMethod(jParameters, mid, (jboolean)Parameters.getAlwaysEmailSummary());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setCutoffEmailSummary", "(Z)V");
  Env.CallVoidMethod(jParameters, mid, (jboolean)Parameters.getCutoffEmailSummary());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setCutoffEmailValue", "(D)V");
  Env.CallVoidMethod(jParameters, mid, (jdouble)Parameters.getCutoffEmailValue());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setEmailAlwaysRecipients", "(Ljava/lang/String;)V");
  Env.CallVoidMethod(jParameters, mid, Env.NewStringUTF(Parameters.getEmailAlwaysRecipients().c_str()));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setEmailCutoffRecipients", "(Ljava/lang/String;)V");
  Env.CallVoidMethod(jParameters, mid, Env.NewStringUTF(Parameters.getEmailCutoffRecipients().c_str()));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setEmailCustomSubject", "(Ljava/lang/String;)V");
  Env.CallVoidMethod(jParameters, mid, Env.NewStringUTF(Parameters.getEmailCustomSubject().c_str()));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setEmailAttachResults", "(Z)V");
  Env.CallVoidMethod(jParameters, mid, (jboolean)Parameters.getEmailAttachResults());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setEmailIncludeResultsDirectory", "(Z)V");
  Env.CallVoidMethod(jParameters, mid, (jboolean)Parameters.getEmailIncludeResultsDirectory());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setEmailCustom", "(Z)V");
  Env.CallVoidMethod(jParameters, mid, (jboolean)Parameters.getEmailCustom());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setEmailCustomMessageBody", "(Ljava/lang/String;)V");
  Env.CallVoidMethod(jParameters, mid, Env.NewStringUTF(Parameters.getEmailCustomMessageBody().c_str()));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setEmailAttachResults", "(Z)V");
  Env.CallVoidMethod(jParameters, mid, (jboolean)Parameters.getEmailAttachResults());
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "setMultipleLocationsFile", "(Ljava/lang/String;)V");
  Env.CallVoidMethod(jParameters, mid, Env.NewStringUTF(Parameters.getMultipleLocationsFile().c_str()));
  jni_error::_detectError(Env);

  return jParameters;
}

/** Copies JParameter object data members to CParameters object. */

CParameters& ParametersUtility::copyJParametersToCParameters(JNIEnv& Env, jobject& jParameters, CParameters& Parameters) {
  jboolean              iscopy;
  const char          * sFilename;

  //set CParameter class from jParameters object
  jclass clazz = Env.GetObjectClass(jParameters);
  jmethodID mid = _getMethodId_Checked(Env, clazz, "GetNumRequestedParallelProcesses", "()I");
  Parameters.SetNumParallelProcessesToExecute(static_cast<unsigned int>(Env.CallIntMethod(jParameters, mid)));
  jni_error::_detectError(Env);

  Parameters.SetExecutionType((ExecutionType)getEnumTypeOrdinalIndex(Env, jParameters, "GetExecutionType", "Lorg/satscan/app/Parameters$ExecutionType;"));
  Parameters.SetMultipleDataSetPurposeType((MultipleDataSetPurposeType)getEnumTypeOrdinalIndex(Env, jParameters, "GetMultipleDataSetPurposeType", "Lorg/satscan/app/Parameters$MultipleDataSetPurposeType;"));
  Parameters.SetAnalysisType((AnalysisType)(getEnumTypeOrdinalIndex(Env, jParameters, "GetAnalysisType", "Lorg/satscan/app/Parameters$AnalysisType;") + 1));
  Parameters.SetProbabilityModelType((ProbabilityModelType)getEnumTypeOrdinalIndex(Env, jParameters, "GetProbabilityModelType", "Lorg/satscan/app/Parameters$ProbabilityModelType;"));
  Parameters.SetAreaRateType((AreaRateType)(getEnumTypeOrdinalIndex(Env, jParameters, "GetAreaScanRateType", "Lorg/satscan/app/Parameters$AreaRateType;") + 1));
  Parameters.SetRiskType((RiskType)getEnumTypeOrdinalIndex(Env, jParameters, "GetRiskType", "Lorg/satscan/app/Parameters$RiskType;"));
  Parameters.SetIncludeClustersType((IncludeClustersType)getEnumTypeOrdinalIndex(Env, jParameters, "GetIncludeClustersType", "Lorg/satscan/app/Parameters$IncludeClustersType;"));

  mid = _getMethodId_Checked(Env, clazz, "GetNumReplicationsRequested", "()I");
  Parameters.SetNumberMonteCarloReplications(static_cast<unsigned int>(Env.CallIntMethod(jParameters, mid)));
  jni_error::_detectError(Env);

  Parameters.SetCriteriaForReportingSecondaryClusters((CriteriaSecondaryClustersType)getEnumTypeOrdinalIndex(Env, jParameters, "GetCriteriaSecondClustersType", "Lorg/satscan/app/Parameters$CriteriaSecondaryClustersType;"));

  mid = _getMethodId_Checked(Env, clazz, "GetTimeTrendConvergence", "()D");
  Parameters.SetTimeTrendConvergence(Env.CallDoubleMethod(jParameters, mid));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "GetNonparametricAdjustmentSize", "()I");
  Parameters.SetNonparametricAdjustmentSize(static_cast<unsigned int>(Env.CallIntMethod(jParameters, mid)));
  jni_error::_detectError(Env);

  Parameters.SetPValueReportingType((PValueReportingType)getEnumTypeOrdinalIndex(Env, jParameters, "GetPValueReportingType", "Lorg/satscan/app/Parameters$PValueReportingType;"));
  mid = _getMethodId_Checked(Env, clazz, "GetEarlyTermThreshold", "()I");
  Parameters.SetEarlyTermThreshold(static_cast<unsigned int>(Env.CallIntMethod(jParameters, mid)));
  jni_error::_detectError(Env);
  mid = _getMethodId_Checked(Env, clazz, "GetReportGumbelPValue", "()Z");
  Parameters.SetReportGumbelPValue(Env.CallBooleanMethod(jParameters, mid));
  jni_error::_detectError(Env);

  Parameters.SetSimulationType((SimulationType)getEnumTypeOrdinalIndex(Env, jParameters, "GetSimulationType", "Lorg/satscan/app/Parameters$SimulationType;"));

  mid = _getMethodId_Checked(Env, clazz, "GetOutputSimulationData", "()Z");
  Parameters.SetOutputSimulationData(Env.CallBooleanMethod(jParameters, mid));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "GetAdjustForEarlierAnalyses", "()Z");
  Parameters.SetAdjustForEarlierAnalyses(Env.CallBooleanMethod(jParameters, mid));
  jni_error::_detectError(Env);

  Parameters.SetSpatialAdjustmentType((SpatialAdjustmentType)getEnumTypeOrdinalIndex(Env, jParameters, "GetSpatialAdjustmentType", "Lorg/satscan/app/Parameters$SpatialAdjustmentType;"));
  Parameters.SetStudyPeriodDataCheckingType((StudyPeriodDataCheckingType)getEnumTypeOrdinalIndex(Env, jParameters, "GetStudyPeriodDataCheckingType", "Lorg/satscan/app/Parameters$StudyPeriodDataCheckingType;"));
  Parameters.SetCoordinatesDataCheckingType((CoordinatesDataCheckingType)getEnumTypeOrdinalIndex(Env, jParameters, "GetCoordinatesDataCheckingType", "Lorg/satscan/app/Parameters$CoordinatesDataCheckingType;"));

  mid = _getMethodId_Checked(Env, clazz, "GetMaxSpatialSizeForType", "(IZ)D");
  Parameters.SetMaxSpatialSizeForType(PERCENTOFPOPULATION, Env.CallDoubleMethod(jParameters, mid, (jint)PERCENTOFPOPULATION, false), false);
  jni_error::_detectError(Env);
  Parameters.SetMaxSpatialSizeForType(PERCENTOFMAXCIRCLEFILE, Env.CallDoubleMethod(jParameters, mid, (jint)PERCENTOFMAXCIRCLEFILE, false), false);
  jni_error::_detectError(Env);
  Parameters.SetMaxSpatialSizeForType(MAXDISTANCE, Env.CallDoubleMethod(jParameters, mid, (jint)MAXDISTANCE, false), false);
  jni_error::_detectError(Env);
  Parameters.SetMaxSpatialSizeForType(PERCENTOFPOPULATION, Env.CallDoubleMethod(jParameters, mid, (jint)PERCENTOFPOPULATION, true), true);
  jni_error::_detectError(Env);
  Parameters.SetMaxSpatialSizeForType(PERCENTOFMAXCIRCLEFILE, Env.CallDoubleMethod(jParameters, mid, (jint)PERCENTOFMAXCIRCLEFILE, true), true);
  jni_error::_detectError(Env);
  Parameters.SetMaxSpatialSizeForType(MAXDISTANCE, Env.CallDoubleMethod(jParameters, mid, (jint)MAXDISTANCE, true), true);
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "GetRestrictMaxSpatialSizeForType", "(IZ)Z");
  Parameters.SetRestrictMaxSpatialSizeForType(PERCENTOFPOPULATION, Env.CallBooleanMethod(jParameters, mid, (jint)PERCENTOFPOPULATION, false), false);
  jni_error::_detectError(Env);
  Parameters.SetRestrictMaxSpatialSizeForType(PERCENTOFMAXCIRCLEFILE, Env.CallBooleanMethod(jParameters, mid, (jint)PERCENTOFMAXCIRCLEFILE, false), false);
  jni_error::_detectError(Env);
  Parameters.SetRestrictMaxSpatialSizeForType(MAXDISTANCE, Env.CallBooleanMethod(jParameters, mid, (jint)MAXDISTANCE, false), false);
  jni_error::_detectError(Env);
  Parameters.SetRestrictMaxSpatialSizeForType(PERCENTOFPOPULATION, Env.CallBooleanMethod(jParameters, mid, (jint)PERCENTOFPOPULATION, true), true);
  jni_error::_detectError(Env);
  Parameters.SetRestrictMaxSpatialSizeForType(PERCENTOFMAXCIRCLEFILE, Env.CallBooleanMethod(jParameters, mid, (jint)PERCENTOFMAXCIRCLEFILE, true), true);
  jni_error::_detectError(Env);
  Parameters.SetRestrictMaxSpatialSizeForType(MAXDISTANCE, Env.CallBooleanMethod(jParameters, mid, (jint)MAXDISTANCE, true), true);
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "GetRestrictingMaximumReportedGeoClusterSize", "()Z");
  Parameters.SetRestrictReportedClusters(Env.CallBooleanMethod(jParameters, mid));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "GetMaximumTemporalClusterSize", "()D");
  Parameters.SetMaximumTemporalClusterSize(Env.CallDoubleMethod(jParameters, mid));
  jni_error::_detectError(Env);

  Parameters.SetMaximumTemporalClusterSizeType((TemporalSizeType)getEnumTypeOrdinalIndex(Env, jParameters, "GetMaximumTemporalClusterSizeType", "Lorg/satscan/app/Parameters$TemporalSizeType;"));

  mid = _getMethodId_Checked(Env, clazz, "GetTimeAggregationLength", "()I");
  Parameters.SetTimeAggregationLength(static_cast<long>(Env.CallIntMethod(jParameters, mid)));
  jni_error::_detectError(Env);

  Parameters.SetTimeAggregationUnitsType((DatePrecisionType)getEnumTypeOrdinalIndex(Env, jParameters, "GetTimeAggregationUnitsType", "Lorg/satscan/app/Parameters$DatePrecisionType;"));

  mid = _getMethodId_Checked(Env, clazz, "GetTimeTrendAdjustmentPercentage", "()D");
  Parameters.SetTimeTrendAdjustmentPercentage(Env.CallDoubleMethod(jParameters, mid));
  jni_error::_detectError(Env);

  Parameters.SetTimeTrendAdjustmentType((TimeTrendAdjustmentType)getEnumTypeOrdinalIndex(Env, jParameters, "GetTimeTrendAdjustmentType", "Lorg/satscan/app/Parameters$TimeTrendAdjustmentType;"));
  Parameters.SetPrecisionOfTimesType((DatePrecisionType)getEnumTypeOrdinalIndex(Env, jParameters, "GetPrecisionOfTimesType", "Lorg/satscan/app/Parameters$DatePrecisionType;"));
  Parameters.SetCoordinatesType((CoordinatesType)getEnumTypeOrdinalIndex(Env, jParameters, "GetCoordinatesType", "Lorg/satscan/app/Parameters$CoordinatesType;"));

  mid = _getMethodId_Checked(Env, clazz, "GetEllipseShapes", "()Ljava/util/ArrayList;");
  jobject vectorobject = Env.CallObjectMethod(jParameters, mid);
  jni_error::_detectError(Env);

  jclass vclazz = Env.GetObjectClass(vectorobject);
  mid = _getMethodId_Checked(Env, vclazz, "size", "()I");
  jint vsize = Env.CallIntMethod(vectorobject, mid);
  jni_error::_detectError(Env);
  for (jint i=0; i < vsize; ++i) {
      mid = _getMethodId_Checked(Env, vclazz, "get", "(I)Ljava/lang/Object;");
      jobject double_object = Env.CallObjectMethod(vectorobject, mid, i);
      jni_error::_detectError(Env);
      jclass dclazz = Env.GetObjectClass(double_object);
      mid = _getMethodId_Checked(Env, dclazz, "doubleValue", "()D");
      Parameters.AddEllipsoidShape(Env.CallDoubleMethod(double_object, mid), (i ? false : true));
      jni_error::_detectError(Env);
  }

  mid = _getMethodId_Checked(Env, clazz, "GetEllipseRotations", "()Ljava/util/ArrayList;");
  vectorobject = Env.CallObjectMethod(jParameters, mid);
  jni_error::_detectError(Env);
  vclazz = Env.GetObjectClass(vectorobject);
  mid = _getMethodId_Checked(Env, vclazz, "size", "()I");
  vsize = Env.CallIntMethod(vectorobject, mid);
  jni_error::_detectError(Env);
  for (jint i=0; i < vsize; ++i) {
      mid = _getMethodId_Checked(Env, vclazz, "get", "(I)Ljava/lang/Object;");
      jobject int_object = Env.CallObjectMethod(vectorobject, mid, i);
      jni_error::_detectError(Env);
      jclass iclazz = Env.GetObjectClass(int_object);
      mid = _getMethodId_Checked(Env, iclazz, "intValue", "()I");
      Parameters.AddEllipsoidRotations(Env.CallIntMethod(int_object, mid), (i ? false : true));
      jni_error::_detectError(Env);
  }

  Parameters.SetNonCompactnessPenalty((NonCompactnessPenaltyType)getEnumTypeOrdinalIndex(Env, jParameters, "GetNonCompactnessPenaltyType", "Lorg/satscan/app/Parameters$NonCompactnessPenaltyType;"));

  mid = _getMethodId_Checked(Env, clazz, "GetIncludePurelySpatialClusters", "()Z");
  Parameters.SetIncludePurelySpatialClusters(Env.CallBooleanMethod(jParameters, mid));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "GetIncludePurelyTemporalClusters", "()Z");
  Parameters.SetIncludePurelyTemporalClusters(Env.CallBooleanMethod(jParameters, mid));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "GetOutputSimLoglikeliRatiosAscii", "()Z");
  Parameters.SetOutputSimLogLikeliRatiosAscii(Env.CallBooleanMethod(jParameters, mid));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "GetOutputSimLoglikeliRatiosDBase", "()Z");
  Parameters.SetOutputSimLogLikeliRatiosDBase(Env.CallBooleanMethod(jParameters, mid));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "GetOutputRelativeRisksAscii", "()Z");
  Parameters.SetOutputRelativeRisksAscii(Env.CallBooleanMethod(jParameters, mid));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "GetOutputRelativeRisksDBase", "()Z");
  Parameters.SetOutputRelativeRisksDBase(Env.CallBooleanMethod(jParameters, mid));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "GetOutputClusterLevelAscii", "()Z");
  Parameters.SetOutputClusterLevelAscii(Env.CallBooleanMethod(jParameters, mid));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "GetOutputClusterLevelDBase", "()Z");
  Parameters.SetOutputClusterLevelDBase(Env.CallBooleanMethod(jParameters, mid));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "GetOutputClusterCaseAscii", "()Z");
  Parameters.SetOutputClusterCaseAscii(Env.CallBooleanMethod(jParameters, mid));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "GetOutputClusterCaseDBase", "()Z");
  Parameters.SetOutputClusterCaseDBase(Env.CallBooleanMethod(jParameters, mid));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "GetOutputAreaSpecificAscii", "()Z");
  Parameters.SetOutputAreaSpecificAscii(Env.CallBooleanMethod(jParameters, mid));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "GetOutputAreaSpecificDBase", "()Z");
  Parameters.SetOutputAreaSpecificDBase(Env.CallBooleanMethod(jParameters, mid));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "GetIsIterativeScanning", "()Z");
  Parameters.SetIterativeScanning(Env.CallBooleanMethod(jParameters, mid));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "GetNumIterativeScansRequested", "()I");
  Parameters.SetNumIterativeScans(static_cast<long>(Env.CallIntMethod(jParameters, mid)));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "GetIterativeCutOffPValue", "()D");
  Parameters.SetIterativeCutOffPValue(Env.CallDoubleMethod(jParameters, mid));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "GetSourceFileName", "()Ljava/lang/String;");
  jstring jstr = (jstring)Env.CallObjectMethod(jParameters, mid);
  jni_error::_detectError(Env);
  sFilename = Env.GetStringUTFChars(jstr, &iscopy);
  Parameters.SetSourceFileName(sFilename);
  if (iscopy == JNI_TRUE) Env.ReleaseStringUTFChars(jstr, sFilename);

  std::vector<std::string> dataSourceNames;
  mid = _getMethodId_Checked(Env, clazz, "getDataSourceNames", "()Ljava/util/ArrayList;");
  vectorobject = Env.CallObjectMethod(jParameters, mid);
  jni_error::_detectError(Env);
  vclazz = Env.GetObjectClass(vectorobject);
  mid = _getMethodId_Checked(Env, vclazz, "size", "()I");
  jni_error::_detectError(Env);
  vsize = Env.CallIntMethod(vectorobject, mid);
  for (jint i = 0; i < vsize; ++i) {
      mid = _getMethodId_Checked(Env, vclazz, "get", "(I)Ljava/lang/Object;");
      jstring str_object = (jstring)Env.CallObjectMethod(vectorobject, mid, i);
      jni_error::_detectError(Env);
      sFilename = Env.GetStringUTFChars(str_object, &iscopy);
      dataSourceNames.push_back(sFilename);
      if (iscopy == JNI_TRUE) Env.ReleaseStringUTFChars(str_object, sFilename);
  }
  Parameters.setDataSourceNames(dataSourceNames);

  mid = _getMethodId_Checked(Env, clazz, "GetCaseFileNames", "()Ljava/util/ArrayList;");
  vectorobject = Env.CallObjectMethod(jParameters, mid);
  jni_error::_detectError(Env);
  vclazz = Env.GetObjectClass(vectorobject);
  mid = _getMethodId_Checked(Env, vclazz, "size", "()I");
  jni_error::_detectError(Env);
  vsize = Env.CallIntMethod(vectorobject, mid);
  for (jint i=0; i < vsize; ++i) {
      mid = _getMethodId_Checked(Env, vclazz, "get", "(I)Ljava/lang/Object;");
      jstring str_object = (jstring)Env.CallObjectMethod(vectorobject, mid, i);
      jni_error::_detectError(Env);
      sFilename = Env.GetStringUTFChars(str_object, &iscopy);
      Parameters.SetCaseFileName(sFilename, false, i + 1);
      if (iscopy == JNI_TRUE) Env.ReleaseStringUTFChars(str_object, sFilename);
  }

  mid = _getMethodId_Checked(Env, clazz, "GetControlFileNames", "()Ljava/util/ArrayList;");
  vectorobject = Env.CallObjectMethod(jParameters, mid);
  jni_error::_detectError(Env);
  vclazz = Env.GetObjectClass(vectorobject);
  mid = _getMethodId_Checked(Env, vclazz, "size", "()I");
  vsize = Env.CallIntMethod(vectorobject, mid);
  jni_error::_detectError(Env);
  for (jint i=0; i < vsize; ++i) {
      mid = _getMethodId_Checked(Env, vclazz, "get", "(I)Ljava/lang/Object;");
      jstring str_object = (jstring)Env.CallObjectMethod(vectorobject, mid, i);
	  jni_error::_detectError(Env);
      sFilename = Env.GetStringUTFChars(str_object, &iscopy);
      Parameters.SetControlFileName(sFilename, false, i + 1);
      if (iscopy == JNI_TRUE) Env.ReleaseStringUTFChars(str_object, sFilename);
  }

  mid = _getMethodId_Checked(Env, clazz, "GetPopulationFileNames", "()Ljava/util/ArrayList;");
  vectorobject = Env.CallObjectMethod(jParameters, mid);
  jni_error::_detectError(Env);
  vclazz = Env.GetObjectClass(vectorobject);
  mid = _getMethodId_Checked(Env, vclazz, "size", "()I");
  vsize = Env.CallIntMethod(vectorobject, mid);
  jni_error::_detectError(Env);
  for (jint i=0; i < vsize; ++i) {
      mid = _getMethodId_Checked(Env, vclazz, "get", "(I)Ljava/lang/Object;");
      jstring str_object = (jstring)Env.CallObjectMethod(vectorobject, mid, i);
      jni_error::_detectError(Env);
      sFilename = Env.GetStringUTFChars(str_object, &iscopy);
      Parameters.SetPopulationFileName(sFilename, false, i + 1);
      if (iscopy == JNI_TRUE) Env.ReleaseStringUTFChars(str_object, sFilename);
  }

  mid = _getMethodId_Checked(Env, clazz, "GetCoordinatesFileName", "()Ljava/lang/String;");
  jstr = (jstring)Env.CallObjectMethod(jParameters, mid);
  jni_error::_detectError(Env);
  sFilename = Env.GetStringUTFChars(jstr, &iscopy);
  Parameters.SetCoordinatesFileName(sFilename);
  if (iscopy == JNI_TRUE) Env.ReleaseStringUTFChars(jstr, sFilename);

  mid = _getMethodId_Checked(Env, clazz, "GetSpecialGridFileName", "()Ljava/lang/String;");
  jstr = (jstring)Env.CallObjectMethod(jParameters, mid);
  jni_error::_detectError(Env);
  sFilename = Env.GetStringUTFChars(jstr, &iscopy);
  Parameters.SetSpecialGridFileName(sFilename, false, true);
  if (iscopy == JNI_TRUE) Env.ReleaseStringUTFChars(jstr, sFilename);

  mid = _getMethodId_Checked(Env, clazz, "GetMaxCirclePopulationFileName", "()Ljava/lang/String;");
  jstr = (jstring)Env.CallObjectMethod(jParameters, mid);
  jni_error::_detectError(Env);
  sFilename = Env.GetStringUTFChars(jstr, &iscopy);
  Parameters.SetMaxCirclePopulationFileName(sFilename);
  if (iscopy == JNI_TRUE) Env.ReleaseStringUTFChars(jstr, sFilename);

  mid = _getMethodId_Checked(Env, clazz, "GetOutputFileNameSetting", "()Ljava/lang/String;");
  jstr = (jstring)Env.CallObjectMethod(jParameters, mid);
  jni_error::_detectError(Env);
  sFilename = Env.GetStringUTFChars(jstr, &iscopy);
  Parameters.SetOutputFileNameSetting(sFilename);
  if (iscopy == JNI_TRUE) Env.ReleaseStringUTFChars(jstr, sFilename);

  mid = _getMethodId_Checked(Env, clazz, "GetIsLoggingHistory", "()Z");
  Parameters.SetIsLoggingHistory(Env.CallBooleanMethod(jParameters, mid));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "GetSimulationDataSourceFilename", "()Ljava/lang/String;");
  jstr = (jstring)Env.CallObjectMethod(jParameters, mid);
  jni_error::_detectError(Env);
  sFilename = Env.GetStringUTFChars(jstr, &iscopy);
  Parameters.SetSimulationDataSourceFileName(sFilename);
  if (iscopy == JNI_TRUE) Env.ReleaseStringUTFChars(jstr, sFilename);

  mid = _getMethodId_Checked(Env, clazz, "UseAdjustmentForRelativeRisksFile", "()Z");
  Parameters.SetUseAdjustmentForRelativeRisksFile(Env.CallBooleanMethod(jParameters, mid));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "GetAdjustmentsByRelativeRisksFilename", "()Ljava/lang/String;");
  jstr = (jstring)Env.CallObjectMethod(jParameters, mid);
  jni_error::_detectError(Env);
  sFilename = Env.GetStringUTFChars(jstr, &iscopy);
  Parameters.SetAdjustmentsByRelativeRisksFilename(sFilename);
  if (iscopy == JNI_TRUE) Env.ReleaseStringUTFChars(jstr, sFilename);

  mid = _getMethodId_Checked(Env, clazz, "GetSimulationDataOutputFilename", "()Ljava/lang/String;");
  jstr = (jstring)Env.CallObjectMethod(jParameters, mid);
  jni_error::_detectError(Env);
  sFilename = Env.GetStringUTFChars(jstr, &iscopy);
  Parameters.SetSimulationDataOutputFileName(sFilename);
  if (iscopy == JNI_TRUE) Env.ReleaseStringUTFChars(jstr, sFilename);

  mid = _getMethodId_Checked(Env, clazz, "GetProspectiveStartDate", "()Ljava/lang/String;");
  jstr = (jstring)Env.CallObjectMethod(jParameters, mid);
  jni_error::_detectError(Env);
  sFilename = Env.GetStringUTFChars(jstr, &iscopy);
  Parameters.SetProspectiveStartDate(sFilename);
  if (iscopy == JNI_TRUE) Env.ReleaseStringUTFChars(jstr, sFilename);

  mid = _getMethodId_Checked(Env, clazz, "GetStudyPeriodStartDate", "()Ljava/lang/String;");
  jstr = (jstring)Env.CallObjectMethod(jParameters, mid);
  jni_error::_detectError(Env);
  sFilename = Env.GetStringUTFChars(jstr, &iscopy);
  Parameters.SetStudyPeriodStartDate(sFilename);
  if (iscopy == JNI_TRUE) Env.ReleaseStringUTFChars(jstr, sFilename);

  mid = _getMethodId_Checked(Env, clazz, "GetStudyPeriodEndDate", "()Ljava/lang/String;");
  jstr = (jstring)Env.CallObjectMethod(jParameters, mid);
  jni_error::_detectError(Env);
  sFilename = Env.GetStringUTFChars(jstr, &iscopy);
  Parameters.SetStudyPeriodEndDate(sFilename);
  if (iscopy == JNI_TRUE) Env.ReleaseStringUTFChars(jstr, sFilename);

  mid = _getMethodId_Checked(Env, clazz, "GetEndRangeStartDate", "()Ljava/lang/String;");
  jstr = (jstring)Env.CallObjectMethod(jParameters, mid);
  jni_error::_detectError(Env);
  sFilename = Env.GetStringUTFChars(jstr, &iscopy);
  Parameters.SetEndRangeStartDate(sFilename);
  if (iscopy == JNI_TRUE) Env.ReleaseStringUTFChars(jstr, sFilename);

  mid = _getMethodId_Checked(Env, clazz, "GetEndRangeEndDate", "()Ljava/lang/String;");
  jstr = (jstring)Env.CallObjectMethod(jParameters, mid);
  jni_error::_detectError(Env);
  sFilename = Env.GetStringUTFChars(jstr, &iscopy);
  Parameters.SetEndRangeEndDate(sFilename);
  if (iscopy == JNI_TRUE) Env.ReleaseStringUTFChars(jstr, sFilename);

  mid = _getMethodId_Checked(Env, clazz, "GetStartRangeEndDate", "()Ljava/lang/String;");
  jstr = (jstring)Env.CallObjectMethod(jParameters, mid);
  jni_error::_detectError(Env);
  sFilename = Env.GetStringUTFChars(jstr, &iscopy);
  Parameters.SetStartRangeEndDate(sFilename);
  if (iscopy == JNI_TRUE) Env.ReleaseStringUTFChars(jstr, sFilename);

  mid = _getMethodId_Checked(Env, clazz, "GetStartRangeStartDate", "()Ljava/lang/String;");
  jstr = (jstring)Env.CallObjectMethod(jParameters, mid);
  jni_error::_detectError(Env);
  sFilename = Env.GetStringUTFChars(jstr, &iscopy);
  Parameters.SetStartRangeStartDate(sFilename);
  if (iscopy == JNI_TRUE) Env.ReleaseStringUTFChars(jstr, sFilename);

  //CParameters::CreationVersion Version;
  //jfieldID vfid = Env.GetFieldID(clazz, "gCreationVersion", "Lorg/satscan/app/Parameters$CreationVersion;");
  //jobject versionobject = Env.GetObjectField(jParameters, vfid);
  //jclass versionclazz = Env.GetObjectClass(versionobject);
  //vfid = Env.GetFieldID(versionclazz, "giMajor", "I");
  //Version.iMajor = Env.GetIntField(versionobject, vfid);
  //vfid = Env.GetFieldID(versionclazz, "giMinor", "I");
  //Version.iMinor = Env.GetIntField(versionobject, vfid);
  //vfid = Env.GetFieldID(versionclazz, "giRelease", "I");
  //Version.iRelease = Env.GetIntField(versionobject, vfid);
  //Parameters.SetVersion(Version);

  mid = _getMethodId_Checked(Env, clazz, "GetRandomizationSeed", "()I");
  Parameters.SetRandomizationSeed(static_cast<long>(Env.CallIntMethod(jParameters, mid)));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "GetReportCriticalValues", "()Z");
  Parameters.SetReportCriticalValues(Env.CallBooleanMethod(jParameters, mid));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "GetSuppressingWarnings", "()Z");
  Parameters.SetSuppressingWarnings(Env.CallBooleanMethod(jParameters, mid));
  jni_error::_detectError(Env);

  Parameters.SetSpatialWindowType((SpatialWindowType)getEnumTypeOrdinalIndex(Env, jParameters, "GetSpatialWindowType", "Lorg/satscan/app/Parameters$SpatialWindowType;"));

  mid = _getMethodId_Checked(Env, clazz, "UseLocationNeighborsFile", "()Z");
  Parameters.UseLocationNeighborsFile(Env.CallBooleanMethod(jParameters, mid));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "GetLocationNeighborsFileName", "()Ljava/lang/String;");
  jstr = (jstring)Env.CallObjectMethod(jParameters, mid);
  jni_error::_detectError(Env);
  sFilename = Env.GetStringUTFChars(jstr, &iscopy);
  Parameters.SetLocationNeighborsFileName(sFilename);
  if (iscopy == JNI_TRUE) Env.ReleaseStringUTFChars(jstr, sFilename);

  Parameters.SetMultipleCoordinatesType((MultipleCoordinatesType)(getEnumTypeOrdinalIndex(Env, jParameters, "GetMultipleCoordinatesType", "Lorg/satscan/app/Parameters$MultipleCoordinatesType;")));

  mid = _getMethodId_Checked(Env, clazz, "GetMetaLocationsFileName", "()Ljava/lang/String;");
  jstr = (jstring)Env.CallObjectMethod(jParameters, mid);
  jni_error::_detectError(Env);
  sFilename = Env.GetStringUTFChars(jstr, &iscopy);
  Parameters.setMetaLocationsFilename(sFilename);
  if (iscopy == JNI_TRUE) Env.ReleaseStringUTFChars(jstr, sFilename);

  mid = _getMethodId_Checked(Env, clazz, "UseMetaLocationsFile", "()Z");
  Parameters.UseMetaLocationsFile(Env.CallBooleanMethod(jParameters, mid));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "GetObservableRegions", "()Ljava/util/ArrayList;");
  vectorobject = Env.CallObjectMethod(jParameters, mid);
  jni_error::_detectError(Env);
  vclazz = Env.GetObjectClass(vectorobject);
  mid = _getMethodId_Checked(Env, vclazz, "size", "()I");
  vsize = Env.CallIntMethod(vectorobject, mid);
  jni_error::_detectError(Env);
  for (jint i=0; i < vsize; ++i) {
      mid = _getMethodId_Checked(Env, vclazz, "get", "(I)Ljava/lang/Object;");
      jstring str_object = (jstring)Env.CallObjectMethod(vectorobject, mid, i);
      jni_error::_detectError(Env);
      sFilename = Env.GetStringUTFChars(str_object, &iscopy);
      Parameters.AddObservableRegion(sFilename, i, i == 0);
      if (iscopy == JNI_TRUE) Env.ReleaseStringUTFChars(str_object, sFilename);
  }

  mid = _getMethodId_Checked(Env, clazz, "getInputSourceSettings", "()Ljava/util/ArrayList;");
  vectorobject = Env.CallObjectMethod(jParameters, mid);
  jni_error::_detectError(Env);
  vclazz = Env.GetObjectClass(vectorobject);
  mid = _getMethodId_Checked(Env, vclazz, "size", "()I");
  vsize = Env.CallIntMethod(vectorobject, mid);
  for (jint i=0; i < vsize; ++i) {
      mid = _getMethodId_Checked(Env, vclazz, "get", "(I)Ljava/lang/Object;");
      jobject iss_object = (jobject)Env.CallObjectMethod(vectorobject, mid, i);
      jclass issclazz = Env.GetObjectClass(iss_object);

      CParameters::InputSource inputsource;
      inputsource.setSourceType((SourceType)getEnumTypeOrdinalIndex(Env, iss_object, "getSourceDataFileType", "Lorg/satscan/importer/InputSourceSettings$SourceDataFileType;"));

      mid = _getMethodId_Checked(Env, issclazz, "getFieldMaps", "()Ljava/util/ArrayList;");
      jobject vectorobject_mappings = Env.CallObjectMethod(iss_object, mid);
      jni_error::_detectError(Env);
      jclass vclazz_mappings = Env.GetObjectClass(vectorobject_mappings);
      mid = _getMethodId_Checked(Env, vclazz_mappings, "size", "()I");
      std::vector<boost::any> map;
      jint vsize_mappings = Env.CallIntMethod(vectorobject_mappings, mid);
      for (jint j=0; j < vsize_mappings; ++j) {
        mid = _getMethodId_Checked(Env, vclazz_mappings, "get", "(I)Ljava/lang/Object;");
        jstring str_object = (jstring)Env.CallObjectMethod(vectorobject_mappings, mid, j);
        jni_error::_detectError(Env);
        sFilename = Env.GetStringUTFChars(str_object, &iscopy);
        std::string buffer(sFilename);
        if (iscopy == JNI_TRUE) Env.ReleaseStringUTFChars(str_object, sFilename);
        if (buffer.size() == 0) {
            map.push_back(DataSource::BLANK);
        } else {
            int column;
            if (!string_to_type<int>(buffer.c_str(), column))
                throw prg_error("Unable to read parameter value '%s' as mapping item.", buffer.c_str());
            // The field mappings will be a collection of integers. The position of element is relative to the input fields order.
            if (column == 0) {
                // Columns less than zero represent 'special' data that are not actually present in the data source.
                // In this case, the value at column zero represents the default population date.
                map.push_back(DataSource::DEFAULT_DATE);
            } else if (column == 1) {
                map.push_back(DataSource::GENERATEDID);
            } else if (column == 2) {
                map.push_back(DataSource::ONECOUNT);
            } else if (inputsource.getSourceType() == SHAPE) {
                if (column == 3) {
                    map.push_back(ShapeFileDataSource::POINTX);
                } else if (column == 4) {
                    map.push_back(ShapeFileDataSource::POINTY);
                } else {
                    map.push_back((long)column - 4);
                }
            } else {
                map.push_back((long)column - 2);
            }
        }
      }
      inputsource.setFieldsMap(map);

      mid = _getMethodId_Checked(Env, issclazz, "getDelimiter", "()Ljava/lang/String;");
      jstr = (jstring)Env.CallObjectMethod(iss_object, mid);
      jni_error::_detectError(Env);
      sFilename = Env.GetStringUTFChars(jstr, &iscopy);
      inputsource.setDelimiter(std::string(sFilename));
      if (iscopy == JNI_TRUE) Env.ReleaseStringUTFChars(jstr, sFilename);

      mid = _getMethodId_Checked(Env, issclazz, "getGroup", "()Ljava/lang/String;");
      jstr = (jstring)Env.CallObjectMethod(iss_object, mid);
      jni_error::_detectError(Env);
      sFilename = Env.GetStringUTFChars(jstr, &iscopy);
      inputsource.setGroup(std::string(sFilename));
      if (iscopy == JNI_TRUE) Env.ReleaseStringUTFChars(jstr, sFilename);

      mid = _getMethodId_Checked(Env, issclazz, "getSkiplines", "()I");
      inputsource.setSkip(Env.CallIntMethod(iss_object, mid));
      jni_error::_detectError(Env);

      mid = _getMethodId_Checked(Env, issclazz, "getFirstRowHeader", "()Z");
      inputsource.setFirstRowHeader(Env.CallBooleanMethod(iss_object, mid));
      jni_error::_detectError(Env);

      unsigned int datasetIdx;
      mid = _getMethodId_Checked(Env, issclazz, "getDataSetIndex", "()I");
      datasetIdx = static_cast<unsigned int>(Env.CallIntMethod(iss_object, mid));
      jni_error::_detectError(Env);

      /* Translate Java class InputSourceSettings.InputFileType into ParameterType.
        {Case=0, Control, Population, Coordinates, SpecialGrid, MaxCirclePopulation, AdjustmentsByRR}
      */
      ParameterType type=CASEFILE;
      int filetype = getEnumTypeOrdinalIndex(Env, iss_object, "getInputFileType", "Lorg/satscan/importer/InputSourceSettings$InputFileType;");
      switch (filetype) {
        case 0/*Case*/                 : type = CASEFILE; break;
        case 1/*Control*/              : type = CONTROLFILE; break;
        case 2/*Population*/           : type = POPFILE; break;
        case 3/*Coordinates*/          : type = COORDFILE; break;
        case 4/*SpecialGrid*/          : type = GRIDFILE; break;
        case 5/*MaxCirclePopulation*/  : type = MAXCIRCLEPOPFILE; break;
        case 6/*AdjustmentsByRR*/      : type = ADJ_BY_RR_FILE; break;
        case 7/*NETWORK_FILE*/         : type = NETWORK_FILE; break;
        case 11/*Multiple Locations*/  : type = MULTIPLE_LOCATIONS_FILE; break;
        default : throw prg_error("Unknown filetype for translation: %d", "copyJParametersToCParameters()", filetype);
      }
      // If file type is Case file, read any line list mappings.
      if (type == CASEFILE) {
          LineListFieldMapContainer_t fields_map;
          mid = _getMethodId_Checked(Env, issclazz, "getLinelistFieldMapsStr", "()Ljava/lang/String;");
          jstr = (jstring)Env.CallObjectMethod(iss_object, mid);
          jni_error::_detectError(Env);
          sFilename = Env.GetStringUTFChars(jstr, &iscopy);
          AbtractParameterFileAccess::parseLinelistStr(std::string(sFilename), fields_map);
          inputsource.setLinelistFieldsMap(fields_map);
          if (iscopy == JNI_TRUE) Env.ReleaseStringUTFChars(jstr, sFilename);
      }
      Parameters.defineInputSource(type, inputsource, datasetIdx);
  }

  Parameters.setTimeTrendType((TimeTrendType)getEnumTypeOrdinalIndex(Env, jParameters, "getTimeTrendType", "Lorg/satscan/app/Parameters$TimeTrendType;"));

  mid = _getMethodId_Checked(Env, clazz, "getReportClusterRank", "()Z");
  Parameters.setReportClusterRank(Env.CallBooleanMethod(jParameters, mid));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "getPrintAsciiHeaders", "()Z");
  Parameters.setPrintAsciiHeaders(Env.CallBooleanMethod(jParameters, mid));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "getReportHierarchicalClusters", "()Z");
  Parameters.setReportHierarchicalClusters(Env.CallBooleanMethod(jParameters, mid));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "getReportGiniOptimizedClusters", "()Z");
  Parameters.setReportGiniOptimizedClusters(Env.CallBooleanMethod(jParameters, mid));
  jni_error::_detectError(Env);

  Parameters.setGiniIndexReportType((GiniIndexReportType)getEnumTypeOrdinalIndex(Env, jParameters, "getGiniIndexReportType", "Lorg/satscan/app/Parameters$GiniIndexReportType;"));

  mid = _getMethodId_Checked(Env, clazz, "getReportGiniIndexCoefficents", "()Z");
  Parameters.setReportGiniIndexCoefficents(Env.CallBooleanMethod(jParameters, mid));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "getPerformPowerEvaluation", "()Z");
  Parameters.setPerformPowerEvaluation(Env.CallBooleanMethod(jParameters, mid));
  jni_error::_detectError(Env);

  Parameters.setPowerEvaluationCriticalValueType((CriticalValuesType)getEnumTypeOrdinalIndex(Env, jParameters, "getPowerEvaluationCriticalValueType", "Lorg/satscan/app/Parameters$CriticalValuesType;"));

  Parameters.setPowerEstimationType((PowerEstimationType)getEnumTypeOrdinalIndex(Env, jParameters, "getPowerEstimationType", "Lorg/satscan/app/Parameters$PowerEstimationType;"));

  Parameters.setPowerEvaluationMethod((PowerEvaluationMethodType)getEnumTypeOrdinalIndex(Env, jParameters, "getPowerEvaluationMethod", "Lorg/satscan/app/Parameters$PowerEvaluationMethodType;"));

  mid = _getMethodId_Checked(Env, clazz, "getPowerEvaluationAltHypothesisFilename", "()Ljava/lang/String;");
  jstr = (jstring)Env.CallObjectMethod(jParameters, mid);
  jni_error::_detectError(Env);
  sFilename = Env.GetStringUTFChars(jstr, &iscopy);
  Parameters.setPowerEvaluationAltHypothesisFilename(sFilename);
  if (iscopy == JNI_TRUE) Env.ReleaseStringUTFChars(jstr, sFilename);

  mid = _getMethodId_Checked(Env, clazz, "getPowerEvaluationCaseCount", "()I");
  Parameters.setPowerEvaluationCaseCount(static_cast<tract_t>(Env.CallIntMethod(jParameters, mid)));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "getNumPowerEvalReplicaPowerStep", "()I");
  Parameters.setNumPowerEvalReplicaPowerStep(static_cast<unsigned int>(Env.CallIntMethod(jParameters, mid)));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "getOutputKMLFile", "()Z");
  Parameters.setOutputKMLFile(Env.CallBooleanMethod(jParameters, mid));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "getOutputTemporalGraphFile", "()Z");
  Parameters.setOutputTemporalGraphFile(Env.CallBooleanMethod(jParameters, mid));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "getAdjustForWeeklyTrends", "()Z");
  Parameters.setAdjustForWeeklyTrends(Env.CallBooleanMethod(jParameters, mid));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "getIncludeLocationsKML", "()Z");
  Parameters.setIncludeLocationsKML(Env.CallBooleanMethod(jParameters, mid));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "getCompressClusterKML", "()Z");
  Parameters.setCompressClusterKML(Env.CallBooleanMethod(jParameters, mid));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "getLaunchMapViewer", "()Z");
  Parameters.setLaunchMapViewer(Env.CallBooleanMethod(jParameters, mid));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "getOutputShapeFiles", "()Z");
  Parameters.setOutputShapeFiles(Env.CallBooleanMethod(jParameters, mid));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "getMinimumTemporalClusterSize", "()I");
  Parameters.setMinimumTemporalClusterSize(Env.CallIntMethod(jParameters, mid));
  jni_error::_detectError(Env);

  Parameters.setTemporalGraphReportType((TemporalGraphReportType)getEnumTypeOrdinalIndex(Env, jParameters, "getTemporalGraphReportType", "Lorg/satscan/app/Parameters$TemporalGraphReportType;"));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "getTemporalGraphSignificantCutoff", "()D");
  Parameters.setTemporalGraphSignificantCutoff(Env.CallDoubleMethod(jParameters, mid));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "getTemporalGraphMostLikelyCount", "()I");
  Parameters.setTemporalGraphMostLikelyCount(Env.CallIntMethod(jParameters, mid));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "GetTitleName", "()Ljava/lang/String;");
  jstr = (jstring)Env.CallObjectMethod(jParameters, mid);
  jni_error::_detectError(Env);
  sFilename = Env.GetStringUTFChars(jstr, &iscopy);
  Parameters.SetTitleName(sFilename);
  if (iscopy == JNI_TRUE) Env.ReleaseStringUTFChars(jstr, sFilename);

  mid = _getMethodId_Checked(Env, clazz, "getCalculateOliveirasF", "()Z");
  Parameters.setCalculateOliveirasF(Env.CallBooleanMethod(jParameters, mid));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "getNumRequestedOliveiraSets", "()I");
  Parameters.setNumRequestedOliveiraSets(Env.CallIntMethod(jParameters, mid));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "getOutputCartesianGraph", "()Z");
  Parameters.setOutputCartesianGraph(Env.CallBooleanMethod(jParameters, mid));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "getRiskLimitHighClusters", "()Z");
  Parameters.setRiskLimitHighClusters(Env.CallBooleanMethod(jParameters, mid));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "getRiskThresholdHighClusters", "()D");
  Parameters.setRiskThresholdHighClusters(Env.CallDoubleMethod(jParameters, mid));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "getRiskLimitLowClusters", "()Z");
  Parameters.setRiskLimitLowClusters(Env.CallBooleanMethod(jParameters, mid));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "getRiskThresholdLowClusters", "()D");
  Parameters.setRiskThresholdLowClusters(Env.CallDoubleMethod(jParameters, mid));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "getMinimumCasesHighRateClusters", "()I");
  Parameters.setMinimumCasesHighRateClusters(static_cast<unsigned int>(Env.CallIntMethod(jParameters, mid)));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "getOutputGoogleMapsFile", "()Z");
  Parameters.setOutputGoogleMapsFile(Env.CallBooleanMethod(jParameters, mid));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "getPerformStandardDrilldown", "()Z");
  Parameters.setPerformStandardDrilldown(Env.CallBooleanMethod(jParameters, mid));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "getPerformBernoulliDrilldown", "()Z");
  Parameters.setPerformBernoulliDrilldown(Env.CallBooleanMethod(jParameters, mid));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "getDrilldownMinimumLocationsCluster", "()I");
  Parameters.setDrilldownMinimumLocationsCluster(static_cast<unsigned int>(Env.CallIntMethod(jParameters, mid)));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "getDrilldownMinimumCasesCluster", "()I");
  Parameters.setDrilldownMinimumCasesCluster(static_cast<unsigned int>(Env.CallIntMethod(jParameters, mid)));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "getDrilldownCutoff", "()D");
  Parameters.setDrilldownCutoff(Env.CallDoubleMethod(jParameters, mid));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "getUseLocationsNetworkFile", "()Z");
  Parameters.setUseLocationsNetworkFile(Env.CallBooleanMethod(jParameters, mid));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "getLocationsNetworkFilename", "()Ljava/lang/String;");
  jstr = (jstring)Env.CallObjectMethod(jParameters, mid);
  jni_error::_detectError(Env);
  sFilename = Env.GetStringUTFChars(jstr, &iscopy);
  Parameters.setLocationsNetworkFilename(sFilename);
  if (iscopy == JNI_TRUE) Env.ReleaseStringUTFChars(jstr, sFilename);

  Parameters.setProspectiveFrequencyType((ProspectiveFrequency)getEnumTypeOrdinalIndex(Env, jParameters, "getProspectiveFrequencyType", "Lorg/satscan/app/Parameters$ProspectiveFrequency;"));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "getLinelistIndividualsCacheFileName", "()Ljava/lang/String;");
  jstr = (jstring)Env.CallObjectMethod(jParameters, mid);
  jni_error::_detectError(Env);
  sFilename = Env.GetStringUTFChars(jstr, &iscopy);
  Parameters.setLinelistIndividualsCacheFileName(sFilename);
  if (iscopy == JNI_TRUE) Env.ReleaseStringUTFChars(jstr, sFilename);

  mid = _getMethodId_Checked(Env, clazz, "getCutoffLineListCSV", "()D");
  Parameters.setCutoffLineListCSV(Env.CallDoubleMethod(jParameters, mid));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "getAlwaysEmailSummary", "()Z");
  Parameters.setAlwaysEmailSummary(Env.CallBooleanMethod(jParameters, mid));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "getCutoffEmailSummary", "()Z");
  Parameters.setCutoffEmailSummary(Env.CallBooleanMethod(jParameters, mid));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "getCutoffEmailValue", "()D");
  Parameters.setCutoffEmailValue(Env.CallDoubleMethod(jParameters, mid));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "getEmailAlwaysRecipients", "()Ljava/lang/String;");
  jstr = (jstring)Env.CallObjectMethod(jParameters, mid);
  jni_error::_detectError(Env);
  sFilename = Env.GetStringUTFChars(jstr, &iscopy);
  Parameters.setEmailAlwaysRecipients(sFilename);
  if (iscopy == JNI_TRUE) Env.ReleaseStringUTFChars(jstr, sFilename);

  mid = _getMethodId_Checked(Env, clazz, "getEmailCutoffRecipients", "()Ljava/lang/String;");
  jstr = (jstring)Env.CallObjectMethod(jParameters, mid);
  jni_error::_detectError(Env);
  sFilename = Env.GetStringUTFChars(jstr, &iscopy);
  Parameters.setEmailCutoffRecipients(sFilename);
  if (iscopy == JNI_TRUE) Env.ReleaseStringUTFChars(jstr, sFilename);

  mid = _getMethodId_Checked(Env, clazz, "getEmailAttachResults", "()Z");
  Parameters.setEmailAttachResults(Env.CallBooleanMethod(jParameters, mid));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "getEmailIncludeResultsDirectory", "()Z");
  Parameters.setEmailIncludeResultsDirectory(Env.CallBooleanMethod(jParameters, mid));
  jni_error::_detectError(Env);
 
  mid = _getMethodId_Checked(Env, clazz, "getEmailCustom", "()Z");
  Parameters.setEmailCustom(Env.CallBooleanMethod(jParameters, mid));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "getEmailCustomSubject", "()Ljava/lang/String;");
  jstr = (jstring)Env.CallObjectMethod(jParameters, mid);
  jni_error::_detectError(Env);
  sFilename = Env.GetStringUTFChars(jstr, &iscopy);
  Parameters.setEmailCustomSubject(sFilename);
  if (iscopy == JNI_TRUE) Env.ReleaseStringUTFChars(jstr, sFilename);

  mid = _getMethodId_Checked(Env, clazz, "getEmailCustomMessageBody", "()Ljava/lang/String;");
  jstr = (jstring)Env.CallObjectMethod(jParameters, mid);
  jni_error::_detectError(Env);
  sFilename = Env.GetStringUTFChars(jstr, &iscopy);
  Parameters.setEmailCustomMessageBody(sFilename);
  if (iscopy == JNI_TRUE) Env.ReleaseStringUTFChars(jstr, sFilename);

  mid = _getMethodId_Checked(Env, clazz, "getMultipleLocationsFile", "()Ljava/lang/String;");
  jstr = (jstring)Env.CallObjectMethod(jParameters, mid);
  jni_error::_detectError(Env);
  sFilename = Env.GetStringUTFChars(jstr, &iscopy);
  Parameters.setMultipleLocationsFile(sFilename);
  if (iscopy == JNI_TRUE) Env.ReleaseStringUTFChars(jstr, sFilename);

  mid = _getMethodId_Checked(Env, clazz, "getCreateEmailSummaryFile", "()Z");
  Parameters.setCreateEmailSummaryFile(Env.CallBooleanMethod(jParameters, mid));
  jni_error::_detectError(Env);

  mid = _getMethodId_Checked(Env, clazz, "getEmailSummaryValue", "()D");
  Parameters.setEmailSummaryValue(Env.CallDoubleMethod(jParameters, mid));
  jni_error::_detectError(Env);

  return Parameters;
}
