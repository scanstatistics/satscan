//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "stsParametersUtility.h"
#include "ParameterFileAccess.h"
#include "JNIException.h"

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
       CParameters::CreationVersion vVersion = {atoi(VERSION_MAJOR), atoi(VERSION_MINOR), atoi(VERSION_RELEASE)};
       Parameters.SetVersion(vVersion);
     }
     if (iscopy == JNI_TRUE)
     	pEnv->ReleaseStringUTFChars(filename, sParameterFilename);
     ParametersUtility::copyCParametersToJParameters(*pEnv, Parameters, jParameters);
  }
  catch (jni_error & x) {    
    return 1; // let the Java exception to be handled in the caller of JNI function
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
  catch (std::exception& x) {
	  jni_error::_throwByName(*pEnv, jni_error::_javaRuntimeExceptionClassName, x.what());
    return;
  }
  catch (...) {
	  jni_error::_throwByName(*pEnv, jni_error::_javaRuntimeExceptionClassName, "Unknown Program Error Encountered.");
    return;
  }
}

/** Returns ordinal of enumeration gotten from 'sFunctionName' called. */
int ParametersUtility::getEnumTypeOrdinalIndex(JNIEnv& Env, jobject& jParameters, const char * sFunctionName, const char * sEnumClassSignature) {
  jclass clazz = Env.GetObjectClass(jParameters);
  jmethodID mid = _getMethodId_Checked(Env, clazz, sFunctionName, std::string(std::string("()") + sEnumClassSignature).c_str());
  jobject t_object = Env.CallObjectMethod(jParameters, mid);
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

  mid = _getMethodId_Checked(Env, clazz, "SetOutputFileName", "(Ljava/lang/String;)V");
  Env.CallVoidMethod(jParameters, mid, Env.NewStringUTF(Parameters.GetOutputFileName().c_str()));
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

  mid = _getMethodId_Checked(Env, clazz, "GetEllipseShapes", "()Ljava/util/Vector;");
  jobject vectorobject = Env.CallObjectMethod(jParameters, mid);
  jni_error::_detectError(Env);

  jclass vclazz = Env.GetObjectClass(vectorobject);
  mid = _getMethodId_Checked(Env, vclazz, "size", "()I");
  jint vsize = Env.CallIntMethod(vectorobject, mid);
  jni_error::_detectError(Env);
  for (jint i=0; i < vsize; ++i) {
      mid = _getMethodId_Checked(Env, vclazz, "elementAt", "(I)Ljava/lang/Object;");
      jobject double_object = Env.CallObjectMethod(vectorobject, mid, i);
      jni_error::_detectError(Env);
      jclass dclazz = Env.GetObjectClass(double_object);
      mid = _getMethodId_Checked(Env, dclazz, "doubleValue", "()D");
      Parameters.AddEllipsoidShape(Env.CallDoubleMethod(double_object, mid), (i ? false : true));
      jni_error::_detectError(Env);
  }

  mid = _getMethodId_Checked(Env, clazz, "GetEllipseRotations", "()Ljava/util/Vector;");
  vectorobject = Env.CallObjectMethod(jParameters, mid);
  jni_error::_detectError(Env);
  vclazz = Env.GetObjectClass(vectorobject);
  mid = _getMethodId_Checked(Env, vclazz, "size", "()I");
  vsize = Env.CallIntMethod(vectorobject, mid);
  jni_error::_detectError(Env);
  for (jint i=0; i < vsize; ++i) {
      mid = _getMethodId_Checked(Env, vclazz, "elementAt", "(I)Ljava/lang/Object;");
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

  mid = _getMethodId_Checked(Env, clazz, "GetCaseFileNames", "()Ljava/util/Vector;");
  vectorobject = Env.CallObjectMethod(jParameters, mid);
  jni_error::_detectError(Env);
  vclazz = Env.GetObjectClass(vectorobject);
  mid = _getMethodId_Checked(Env, vclazz, "size", "()I");
  jni_error::_detectError(Env);
  vsize = Env.CallIntMethod(vectorobject, mid);
  for (jint i=0; i < vsize; ++i) {
      mid = _getMethodId_Checked(Env, vclazz, "elementAt", "(I)Ljava/lang/Object;");
      jstring str_object = (jstring)Env.CallObjectMethod(vectorobject, mid, i);
      jni_error::_detectError(Env);
      sFilename = Env.GetStringUTFChars(str_object, &iscopy);
      Parameters.SetCaseFileName(sFilename, false, i + 1);
      if (iscopy == JNI_TRUE) Env.ReleaseStringUTFChars(str_object, sFilename);
  }

  mid = _getMethodId_Checked(Env, clazz, "GetControlFileNames", "()Ljava/util/Vector;");
  vectorobject = Env.CallObjectMethod(jParameters, mid);
  jni_error::_detectError(Env);
  vclazz = Env.GetObjectClass(vectorobject);
  mid = _getMethodId_Checked(Env, vclazz, "size", "()I");
  vsize = Env.CallIntMethod(vectorobject, mid);
  jni_error::_detectError(Env);
  for (jint i=0; i < vsize; ++i) {
      mid = _getMethodId_Checked(Env, vclazz, "elementAt", "(I)Ljava/lang/Object;");
      jstring str_object = (jstring)Env.CallObjectMethod(vectorobject, mid, i);
	  jni_error::_detectError(Env);
      sFilename = Env.GetStringUTFChars(str_object, &iscopy);
      Parameters.SetControlFileName(sFilename, false, i + 1);
      if (iscopy == JNI_TRUE) Env.ReleaseStringUTFChars(str_object, sFilename);
  }

  mid = _getMethodId_Checked(Env, clazz, "GetPopulationFileNames", "()Ljava/util/Vector;");
  vectorobject = Env.CallObjectMethod(jParameters, mid);
  jni_error::_detectError(Env);
  vclazz = Env.GetObjectClass(vectorobject);
  mid = _getMethodId_Checked(Env, vclazz, "size", "()I");
  vsize = Env.CallIntMethod(vectorobject, mid);
  jni_error::_detectError(Env);
  for (jint i=0; i < vsize; ++i) {
      mid = _getMethodId_Checked(Env, vclazz, "elementAt", "(I)Ljava/lang/Object;");
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

  mid = _getMethodId_Checked(Env, clazz, "GetOutputFileName", "()Ljava/lang/String;");
  jstr = (jstring)Env.CallObjectMethod(jParameters, mid);
  jni_error::_detectError(Env);
  sFilename = Env.GetStringUTFChars(jstr, &iscopy);
  Parameters.SetOutputFileName(sFilename);
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

  mid = _getMethodId_Checked(Env, clazz, "GetObservableRegions", "()Ljava/util/Vector;");
  vectorobject = Env.CallObjectMethod(jParameters, mid);
  jni_error::_detectError(Env);
  vclazz = Env.GetObjectClass(vectorobject);
  mid = _getMethodId_Checked(Env, vclazz, "size", "()I");
  vsize = Env.CallIntMethod(vectorobject, mid);
  jni_error::_detectError(Env);
  for (jint i=0; i < vsize; ++i) {
      mid = _getMethodId_Checked(Env, vclazz, "elementAt", "(I)Ljava/lang/Object;");
      jstring str_object = (jstring)Env.CallObjectMethod(vectorobject, mid, i);
      jni_error::_detectError(Env);
      sFilename = Env.GetStringUTFChars(str_object, &iscopy);
      Parameters.AddObservableRegion(sFilename, i, i == 0);
      if (iscopy == JNI_TRUE) Env.ReleaseStringUTFChars(str_object, sFilename);
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

  return Parameters;
}

