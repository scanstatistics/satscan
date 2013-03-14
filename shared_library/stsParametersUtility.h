//******************************************************************************
#ifndef stsParametersUtilityH
#define stsParametersUtilityH
//******************************************************************************
#include <jni.h>
#include "org_satscan_app_Parameters.h"

#include "Parameters.h"

class ParametersUtility {
  public:
    static jobject     & copyCParametersToJParameters(JNIEnv& Env, CParameters& Parameters, jobject& jParameters);
    static CParameters & copyJParametersToCParameters(JNIEnv& Env, jobject& jParameters, CParameters& Parameters);
    static int           getEnumTypeOrdinalIndex(JNIEnv& Env, jobject& jParameters, const char * sFunctionName, const char * sEnumClassSignature);
};

//******************************************************************************
#endif
