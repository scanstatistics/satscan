// Class DBaseOutput
// Adam J Vaughn
// September 3, 2002
//
// This class is designed to create and fill a DBase file from the data produced
// by the calculations in the SatScan program. This is a base class to be derived off
// of for each necessary dBase output file.

#include "SaTScan.h"
#pragma hdrstop

#include "stsOutputDBF.h"
#include <DBFFile.h>
#include "Cluster.h"

const char *    CLUSTER_LEVEL_EXT       = ".col.dbf";
const char *    AREA_SPECIFIC_EXT       = ".gis.dbf";

const char *    RUN_NUM		        = "RUN_NUM";
const char *    CLUST_NUM 		= "CLUST_NUM";
const char *    LOC_ID   		= "LOC_ID";
const char *    P_VALUE  		= "P_VALUE";
const char *    START_DATE		= "START_DATE";
const char *    END_DATE		= "END_DATE";
const char *    OBSERVED		= "OBSERVED";
const char *    EXPECTED		= "EXPECTED";
const char *    REL_RISK		= "REL_RISK";
const char *    LOG_LIKL		= "LOG_LIKL";
const char *    NUM_AREAS		= "NUM_AREAS";
const char *    COORD_LAT		= "LATITUDE";
const char *    COORD_LONG		= "LONGITUDE";
const char *    COORD_X		        = "X";
const char *    COORD_Y                 = "Y";
const char *    COORD_Z                 = "Z";
const char *    RADIUS		        = "RADIUS";
const char *    AREA_OBS                = "AREA_OBS";
const char *    AREA_EXP                = "AREA_EXP";
const char *    AREA_RSK                = "AREA_RSK";

// constructor
__fastcall DBaseOutput::DBaseOutput(const long lRunNumber, const bool bPrintPVal, const int iCoordType) {
   try {
      Init();
      Setup(lRunNumber, bPrintPVal, iCoordType);
   }
   catch (ZdException &x) {
      x.AddCallpath("Constructor", "DBaseOutput");
      throw;	
   }			
}

// destructor
DBaseOutput::~DBaseOutput() {
   try {
      CleanupFieldVector();
   }
   catch (...) { /* munch munch */ }		
}

// deletes all of the field pointers in the vector and empties the vector
// pre: none
// post: field vector is empty and all of the pointers are deleted
void DBaseOutput::CleanupFieldVector() {
   try {
      gvFields.RemoveAllElements();
   }
   catch(ZdException &x) {
      x.AddCallpath("CleanupFieldVector()", "stsClusterLevelDBF");
      throw;
   }
}

// create the output file
// pre: gvFields have been set up and gsFileName has been set
// post: creates the dbf file with the appropraite fields
void DBaseOutput::CreateDBFFile() {
   try {
      // pack up and create
      DBFFile File;
      File.PackFields(gvFields);

      // BUGBUG
      // for now we'll overwrite files, in the future we may wish to display an exception instead - AJV 9/4/2002
      if(ZdIO::Exists(gsFileName))
        ZdIO::Delete(gsFileName);
      File.Create(gsFileName, gvFields);
      File.Close();
   }
   catch (ZdException &x) {
      x.AddCallpath("CreateDBFFile()", "DBaseOutput");
      throw;
   }
}

// global inits
void DBaseOutput::Init() {
   glRunNumber = 0;
   giCoordType = 0;
}	

// formats the string for the Area ID
// pre: none
// post: sTempvalue will contain the legible area id
void DBaseOutput::SetAreaID(ZdString& sTempValue, const CCluster& pCluster, const CSaTScanData& pData) {
   try {
      if (pCluster.GetClusterType() == PURELYTEMPORAL)
          sTempValue = "n/a";
      else
         sTempValue = pData.gpTInfo->tiGetTid(pCluster.m_Center);
   }
   catch (ZdException &x) {
      x.AddCallpath("SetAreaID", "DBaseOutput");
      throw;
   }
}

// internal setup function
void DBaseOutput::Setup(const long lRunNumber, const bool bPrintPVal, const int iCoordType) {
   giCoordType = iCoordType;
   glRunNumber = lRunNumber;
   gbPrintPVal = bPrintPVal;
}	

