// class stsRunHistoryFile
// Adam J Vaughn
// 9/4/2002

// This class keeps a ZD file log for each run of the SaTScan program which includes information
// specified by the client. For each instance of the class, a new, unnique run number will be 
// recorded in the file and the pertinent data will be updated once the analysis is complete.

#include "SaTScan.h"
#pragma hdrstop

#include "Analysis.h"
#include "stsRunHistoryFile.h"
#include "DBFFile.h"

const int       OUTPUT_FILE_FIELD_LENGTH        = 254;

const char* RUN_NUMBER_FIELD            = "RUN_NUM";
const char* RUN_TIME_FIELD              = "RUN_TIME";
const char* OUTPUT_FILE_FIELD           = "OUT_FILE";
const char* PROB_MODEL_FIELD            = "PROB_MODEL";
const char* RATES_FIELD                 = "RATES";
const char* COORD_TYPE_FIELD            = "COORD_TYPE";
const char* ANALYSIS_TYPE_FIELD         = "ANALYSIS";
const char* NUM_CASES_FIELD             = "NUM_CASES";
const char* TOTAL_POP_FIELD             = "TOTAL_POP";
const char* NUM_GEO_AREAS_FIELD         = "NUM_AREAS";
const char* PRECISION_TIMES_FIELD       = "TIME_PREC";
const char* MAX_GEO_EXTENT_FIELD        = "GEO_EXTENT";
const char* MAX_TIME_EXTENT_FIELD       = "TEMPOR_EXT";
const char* TIME_TREND_ADJUSTMENT_FIELD = "TREND_ADJ";
const char* COVARIATES_FIELD            = "COVAR";
const char* GRID_FILE_FIELD             = "GRID_FILE";
const char* START_DATE_FIELD            = "START_DATE";
const char* END_DATE_FIELD              = "END_DATE";
const char* ALIVE_ONLY_FIELD            = "ALIVE_ONLY";
const char* INTERVAL_FIELD              = "INTERVAL";
const char* MONTE_CARLO_FIELD           = "MONTECARLO";
const char* CUTOFF_001_FIELD            = "CUTOFF_001";
const char* CUTOFF_005_FIELD            = "CUTOFF_005";
const char* NUM_SIGNIF_005_FIELD        = "SIGNIF_005";
const char* P_VALUE_FIELD               = "P_VALUE";
const char* ADDITIONAL_OUTPUT_FILES_FIELD        = "ADDIT_OUT";

// constructor
stsRunHistoryFile::stsRunHistoryFile(const ZdString& sFileName, BasePrint& PrintDirection, bool bPrintPVal)
                 : gpPrintDirection(&PrintDirection) , gbPrintPVal(bPrintPVal) {
   try {
      Init();
      SetFileName(sFileName);
      SetRunNumber();
   }
   catch (ZdException &x) {
      PrintDirection.SatScanPrintWarning("The following error occured in attempting to log run history to file:\n");
      PrintDirection.SatScanPrintWarning(x.GetErrorMessage());
      PrintDirection.SatScanPrintWarning("\n");
   }
}

// destructor
stsRunHistoryFile::~stsRunHistoryFile() {
   try {
      gvFields.DeleteAllElements();
   }
   catch (...) {/* munch munch */}
}

// creates the run history file
// pre: txd file doesn't not already exist
// post: will create the txd file with the appropraite fields
void stsRunHistoryFile::CreateRunHistoryFile() {
   unsigned short   uwOffset(0);     // offset is altered by the CreateNewField function

   try {
      ::CreateNewField(gvFields, RUN_NUMBER_FIELD, ZD_NUMBER_FLD, 8, 0, uwOffset, true);
      ::CreateNewField(gvFields, RUN_TIME_FIELD, ZD_ALPHA_FLD, 32, 0, uwOffset);

      ::CreateNewField(gvFields, ANALYSIS_TYPE_FIELD, ZD_ALPHA_FLD, 32, 0, uwOffset);
      ::CreateNewField(gvFields, PROB_MODEL_FIELD, ZD_ALPHA_FLD, 32, 0, uwOffset); 
      ::CreateNewField(gvFields, RATES_FIELD, ZD_ALPHA_FLD, 16, 0, uwOffset);
      ::CreateNewField(gvFields, GRID_FILE_FIELD, ZD_BOOLEAN_FLD, 1, 0, uwOffset);
      ::CreateNewField(gvFields, MAX_GEO_EXTENT_FIELD, ZD_NUMBER_FLD, 16, 3, uwOffset);
      ::CreateNewField(gvFields, MAX_TIME_EXTENT_FIELD, ZD_NUMBER_FLD, 16, 3, uwOffset);
      ::CreateNewField(gvFields, INTERVAL_FIELD, ZD_ALPHA_FLD, 32, 0, uwOffset);
      ::CreateNewField(gvFields, ALIVE_ONLY_FIELD, ZD_BOOLEAN_FLD, 1, 0, uwOffset);
      ::CreateNewField(gvFields, TIME_TREND_ADJUSTMENT_FIELD, ZD_ALPHA_FLD, 20, 3, uwOffset);
      ::CreateNewField(gvFields, COVARIATES_FIELD, ZD_NUMBER_FLD, 3, 0, uwOffset);
      ::CreateNewField(gvFields, MONTE_CARLO_FIELD, ZD_NUMBER_FLD, 8, 0, uwOffset);

      ::CreateNewField(gvFields, NUM_GEO_AREAS_FIELD, ZD_NUMBER_FLD, 8, 0, uwOffset);
      ::CreateNewField(gvFields, COORD_TYPE_FIELD, ZD_ALPHA_FLD, 16, 0, uwOffset);
      ::CreateNewField(gvFields, START_DATE_FIELD, ZD_ALPHA_FLD, 16, 0, uwOffset);
      ::CreateNewField(gvFields, END_DATE_FIELD, ZD_ALPHA_FLD, 16, 0, uwOffset);
      ::CreateNewField(gvFields, PRECISION_TIMES_FIELD, ZD_ALPHA_FLD, 16, 0, uwOffset);
      ::CreateNewField(gvFields, NUM_CASES_FIELD, ZD_NUMBER_FLD, 8, 0, uwOffset);
      ::CreateNewField(gvFields, TOTAL_POP_FIELD, ZD_NUMBER_FLD, 16, 3, uwOffset);

      ::CreateNewField(gvFields, P_VALUE_FIELD, ZD_NUMBER_FLD, 12, 5, uwOffset);
      ::CreateNewField(gvFields, NUM_SIGNIF_005_FIELD, ZD_NUMBER_FLD, 8, 0, uwOffset);
      ::CreateNewField(gvFields, CUTOFF_001_FIELD, ZD_NUMBER_FLD, 8, 3, uwOffset);
      ::CreateNewField(gvFields, CUTOFF_005_FIELD, ZD_NUMBER_FLD, 8, 3, uwOffset);
      ::CreateNewField(gvFields, OUTPUT_FILE_FIELD, ZD_ALPHA_FLD, OUTPUT_FILE_FIELD_LENGTH, 0, uwOffset);
      ::CreateNewField(gvFields, ADDITIONAL_OUTPUT_FILES_FIELD, ZD_ALPHA_FLD, OUTPUT_FILE_FIELD_LENGTH, 0, uwOffset);

      DBFFile File;
      File.PackFields(gvFields);
      File.Create(gsFilename, gvFields, 1);
      File.Close();
   }
   catch (ZdException &x) {
      gpPrintDirection->SatScanPrintWarning("Unable to create run history file - %s\n", gsFilename.GetCString());
      throw;
   }
}

// converter function to turn the iType into a legible string for printing
// pre :  iType is contained in (PURELYSPATIAL, PURELYTEMPORAL, SPACETIME, PROSPECTIVESPACETIME, PURELYSPATIALMONOTONE)
// post : string will be assigned a formatted value based on iType
void stsRunHistoryFile::GetAnalysisTypeString(ZdString& sTempValue, int iType) {
   try {
      switch(iType) {
         case PURELYSPATIAL :
            sTempValue = "Purely Spatial"; break;
         case PURELYTEMPORAL :
            sTempValue = "Purely Temporal"; break;
         case SPACETIME :
            sTempValue = "Space Time";  break;
         case PROSPECTIVESPACETIME :
            sTempValue = "Prospective Space Time"; break;
         case PURELYSPATIALMONOTONE :
            sTempValue = "Purely Spatial Monotone"; break;
         default :
            ZdException::GenerateNotification("Invalid analysis type in the run history file.", "stsRunHistoryFile");   
      }
   }
   catch (ZdException &x) {
      x.AddCallpath("GetAnalysisTypeString()", "stsRunHistoryFile");
      throw;
   }
}

// converts the iPrecision into a legible string for printing
// pre : 0 <= iPrecision <= 3
// post : string is assigned a formatted value based on iPrecision
void stsRunHistoryFile::GetCasePrecisionString(ZdString& sTempValue, int iPrecision) {
   try {
      switch (iPrecision) {
         case 0:
            sTempValue = "None"; break;
         case 1:
            sTempValue = "Year"; break;
         case 2:
            sTempValue = "Month"; break;
         case 3:
            sTempValue = "Day";  break;
         default :
            ZdException::GenerateNotification("Invalid case time precision in run history file.", "stsRunHistoryFile");   
      }
   }
   catch (ZdException &x) {
      x.AddCallpath("GetCasePrecisionString()", "stsRunHistoryFile");
      throw;
   }
}

// basically a converter function which converts the Interval units from the way we store
// them as ints to a legible string to be printed in the file
// pre : 0 <= iUnits <= 3, sTempValue has been allocated
// post: will assign the appropraite value to the string so that it can be printed
void stsRunHistoryFile::GetIntervalUnitsString(ZdString& sTempValue, int iUnits) {
   try {
      switch (iUnits) {
         case 0:
            sTempValue = "None"; break;
         case 1:
            sTempValue = "Year"; break;
         case 2:
            sTempValue = "Month"; break;
         case 3:
            sTempValue = "Day"; break;
         default:
            ZdException::GenerateNotification("Invalid interval units in run history file.", "GetIntervalUnitsString()");
      }
   }
   catch (ZdException &x) {
      x.AddCallpath("GetIntervalUnitsString()", "stsRunHistoryFile");
      throw;
   }
}

// a converter function to convert the stored int into a legible string to be printed
// pre: iModel conatined in (POISSON, BERNOULLI, SPACETIMEPERMUTATION) and sTempValue allocated
// post : string will contain the formatted value for printing
void stsRunHistoryFile::GetProbabilityModelString(ZdString& sTempValue, int iModel) {
   try {
      switch(iModel) {
         case POISSON :
            sTempValue = "Poisson";  break;
         case BERNOULLI :
            sTempValue = "Bernoulli";  break;
         case SPACETIMEPERMUTATION :
            sTempValue = "Space Time Permutation"; break;
         default :
            ZdException::GenerateNotification("Invalid probability model in the run history file.", "stsRunHistoryFile");   
      }
   }
   catch (ZdException &x) {
      x.AddCallpath("GetProbabilityModelString()", "stsRunHistoryFile");
      throw;
   }
}

// converter function to make a legible string for printing
// pre : iRate conatined in (HIGH, LOW, HIGHANDLOW) and sTempValue allocated
// post : will assign the appropraite formated value to sTempValue
void stsRunHistoryFile::GetRatesString(ZdString& sTempValue, int iRate) {
   try {
      switch (iRate) {
         case HIGH :
            sTempValue = "High"; break;
         case LOW :
            sTempValue = "Low"; break;
         case HIGHANDLOW :
            sTempValue = "Both"; break;
         default :
            ZdException::GenerateNotification("Invalid rate defined in run history file.", "stsRunHistoryFile");   
      }
   }
   catch (ZdException &x) {
      x.AddCallpath("GetRatesString()", "stsRunHistoryFile");
      throw;
   }
}

// converts the iType to a legible string for printing
//  pre : iType is conatined in (NOTADJUSTED, NONPARAMETRIC, LINEAR)
// post : string will be assigned a formatted value based upon iType
void stsRunHistoryFile::GetTimeAdjustmentString(ZdString& sTempValue, int iType) {
   try {
      switch(iType) {
         case NOTADJUSTED :
            sTempValue = "None"; break;
         case NONPARAMETRIC :
            sTempValue = "Non-parametric"; break;
         case LINEAR :
            sTempValue = "Linear"; break;
         default :
            ZdException::GenerateNotification("Invalid time trend adjuestment type in run history file.", "stsRunHistoryFile");   
      }
   }
   catch (ZdException &x) {
      x.AddCallpath("GetTimeAdjustmentString()", "stsRunHistoryFile");
      throw;
   }
}

// global initializations
void stsRunHistoryFile::Init() {
   glRunNumber = 0;
}

// although the name implies an oxymoron, this function will record a new run into the history file
// pre: none
// post: records the run history to the file
void stsRunHistoryFile::LogNewHistory(const CAnalysis& pAnalysis, const unsigned short uwSignificantAt005, double dVal) {
   ZdTransaction	*pTransaction = 0;
   ZdString             sTempValue, sInterval;
   std::auto_ptr<DBFFile>    pFile;
   bool                 bFound(false);

   try {
      // NOTE: I'm going to document the heck out of this section for two reasons :
      // 1) in case they change the run specs on us at any time
      // 2) to present my assumptions about the output data in case any happen to be incorrect
      // , so bear with me - AJV 9/3/2002

      pFile.reset(new DBFFile(gsFilename));
      std::auto_ptr<ZdFileRecord> pRecord(pFile->GetNewRecord());

      for(unsigned long i = 1; i <= pFile->GetNumRecords() && !bFound; ++i) {
         pFile->GotoRecord(i, pRecord.get());
         bFound = (pRecord->GetLong(0) == glRunNumber);
      }
      if(!bFound)
         ZdException::GenerateNotification("Error! Run number not found in the run history file.", "LogNewhistory()");

      // NOTE : ordering in which the data is added does not matter here in this function due to the use of the
      // GetFieldNumber function which finds the appropraite field number for the SetField functions and inserts
      // the data in that field - ordering is determined solely in the CreateRunHistoryFile function by the order the
      // fields are added to the vector in that function

      //  run number field
      SetDoubleField(*pRecord, (double)glRunNumber, GetFieldNumber(gvFields, RUN_NUMBER_FIELD));

      // run time and date field
      sTempValue = pAnalysis.GetStartTime();
      StripCRLF(sTempValue);
      SetStringField(*pRecord, sTempValue, GetFieldNumber(gvFields, RUN_TIME_FIELD));

      // output file name field
      sTempValue = pAnalysis.GetSatScanData()->m_pParameters->m_szOutputFilename;
      StripCRLF(sTempValue);
      SetStringField(*pRecord, sTempValue, GetFieldNumber(gvFields, OUTPUT_FILE_FIELD));
      SetAdditionalOutputFileNameString(sTempValue, *(pAnalysis.GetSatScanData()->m_pParameters));
      SetStringField(*pRecord, sTempValue, GetFieldNumber(gvFields, ADDITIONAL_OUTPUT_FILES_FIELD));

      // probability model field
      GetProbabilityModelString(sTempValue, pAnalysis.GetSatScanData()->m_pParameters->m_nModel);
      SetStringField(*pRecord, sTempValue, GetFieldNumber(gvFields, PROB_MODEL_FIELD));

      // rates(high, low or both) field
      GetRatesString(sTempValue, pAnalysis.GetSatScanData()->m_pParameters->m_nAreas);
      SetStringField(*pRecord, sTempValue, GetFieldNumber(gvFields, RATES_FIELD));

      // coordinate type field
      sTempValue = ((pAnalysis.GetSatScanData()->m_pParameters->m_nCoordType == CARTESIAN) ? "Cartesian" : "LatLong");
      SetStringField(*pRecord, sTempValue, GetFieldNumber(gvFields, COORD_TYPE_FIELD));

      // analysis type field
      GetAnalysisTypeString(sTempValue, pAnalysis.GetSatScanData()->m_pParameters->m_nAnalysisType);
      SetStringField(*pRecord, sTempValue, GetFieldNumber(gvFields, ANALYSIS_TYPE_FIELD));

      SetDoubleField(*pRecord, (double)pAnalysis.GetSatScanData()->m_nTotalCases, GetFieldNumber(gvFields, NUM_CASES_FIELD));   // total number of cases field
      SetDoubleField(*pRecord, pAnalysis.GetSatScanData()->m_nTotalPop, GetFieldNumber(gvFields, TOTAL_POP_FIELD));  // total population field
      SetDoubleField(*pRecord, (double)pAnalysis.GetSatScanData()->m_nTracts, GetFieldNumber(gvFields, NUM_GEO_AREAS_FIELD));     // number of geographic areas field

      // precision of case times field
      GetCasePrecisionString(sTempValue, pAnalysis.GetSatScanData()->m_pParameters->m_nPrecision);
      SetStringField(*pRecord, sTempValue, GetFieldNumber(gvFields, PRECISION_TIMES_FIELD));

      SetDoubleField(*pRecord, pAnalysis.GetSatScanData()->m_pParameters->m_nMaxGeographicClusterSize, GetFieldNumber(gvFields, MAX_GEO_EXTENT_FIELD));   // max geographic extent field
      SetDoubleField(*pRecord, pAnalysis.GetSatScanData()->m_pParameters->m_nMaxTemporalClusterSize, GetFieldNumber(gvFields, MAX_TIME_EXTENT_FIELD));   // max temporal extent field

      // time trend adjustment field
      GetTimeAdjustmentString(sTempValue, pAnalysis.GetSatScanData()->m_pParameters->m_nTimeAdjustType);
      SetStringField(*pRecord, sTempValue, GetFieldNumber(gvFields, TIME_TREND_ADJUSTMENT_FIELD));

      // covariates number
      SetDoubleField(*pRecord, (double)pAnalysis.GetSatScanData()->gpCats->catGetNumEls(), GetFieldNumber(gvFields, COVARIATES_FIELD));

      SetBoolField(*pRecord, pAnalysis.GetSatScanData()->m_pParameters->m_bSpecialGridFile, GetFieldNumber(gvFields, GRID_FILE_FIELD)); // special grid file used field
      SetStringField(*pRecord, pAnalysis.GetSatScanData()->m_pParameters->m_szStartDate, GetFieldNumber(gvFields, START_DATE_FIELD));  // start date field
      SetStringField(*pRecord, pAnalysis.GetSatScanData()->m_pParameters->m_szEndDate, GetFieldNumber(gvFields, END_DATE_FIELD)); // end date field
      SetBoolField(*pRecord, pAnalysis.GetSatScanData()->m_pParameters->m_bAliveClustersOnly, GetFieldNumber(gvFields, ALIVE_ONLY_FIELD)); // alive clusters only field

      // interval field
      GetIntervalUnitsString(sTempValue, pAnalysis.GetSatScanData()->m_pParameters->m_nIntervalUnits);
      sInterval << pAnalysis.GetSatScanData()->m_pParameters->m_nIntervalLength << " " << sTempValue;
      SetStringField(*pRecord, sInterval, GetFieldNumber(gvFields, INTERVAL_FIELD));

      // p-value field
      if(gbPrintPVal)
         SetDoubleField(*pRecord, dVal, GetFieldNumber(gvFields, P_VALUE_FIELD));
      else
         pRecord->PutBlank(GetFieldNumber(gvFields, P_VALUE_FIELD));
      SetDoubleField(*pRecord, (double)pAnalysis.GetSatScanData()->m_pParameters->m_nReplicas, GetFieldNumber(gvFields, MONTE_CARLO_FIELD));  // monte carlo  replications field

      if(gbPrintPVal) {    // only print 0.01 and 0.05 cutoffs if pVals are printed, else this would result in access underrun - AJV
         SetDoubleField(*pRecord, pAnalysis.GetSimRatio01(), GetFieldNumber(gvFields, CUTOFF_001_FIELD)); // 0.01 cutoff field
         SetDoubleField(*pRecord, pAnalysis.GetSimRatio05(), GetFieldNumber(gvFields, CUTOFF_005_FIELD)); // 0.05 cutoff field
      }
      else {
         pRecord->PutBlank(GetFieldNumber(gvFields, CUTOFF_001_FIELD));
         pRecord->PutBlank(GetFieldNumber(gvFields, CUTOFF_005_FIELD));
      }

      SetDoubleField(*pRecord, (double)uwSignificantAt005, GetFieldNumber(gvFields, NUM_SIGNIF_005_FIELD));  // number of clusters significant at tthe .05 llr cutoff field

      pTransaction = (pFile->BeginTransaction());
      pFile->SaveRecord(*pTransaction, pFile->GetCurrentRecordNumber(),*pRecord);
      pFile->EndTransaction(pTransaction); pTransaction = 0;
      pFile->Close();
   }
   catch(ZdException &x) {
      gpPrintDirection->SatScanPrintWarning("ERROR - Unable to record analysis information to the log history file:\n");
      gpPrintDirection->SatScanPrintWarning(x.GetErrorMessage());
      gpPrintDirection->SatScanPrintWarning("\n");
      if(pTransaction)
         pFile->EndTransaction(pTransaction);   // if there is a pTransaction then there must be a pFile, so this is valid
      pTransaction = 0;
   }
}

// small helper function for replacing the extension of a filename and appending it to the tempstring
// pre : sSourceFileName is the name of the file to replace the extension on
// post : will replace the result output filename's extension with the replacement extension, if an extension
//        doesn't exist on the original filename, then it will just tack on the extension THEN will append the output filename
//        to the tempstring as long as it still fits within the fieldsize space
void stsRunHistoryFile::ReplaceExtensionAndAppend(ZdString& sOutputFileNames, const ZdFileName& sSourceFileName, const ZdString& sReplacementExtension) {
   try {
      ZdString  sWorkString(sSourceFileName.GetFileName());
      if(strlen(sSourceFileName.GetExtension()) > 0)
         sWorkString.Replace(sSourceFileName.GetExtension(), sReplacementExtension);
      else
         sWorkString << sReplacementExtension;

      // if the temp string plus the work string lengths are less than the field width then append the work string
      // to temp string, else just print ',...'
      if ((sOutputFileNames.GetLength() + sWorkString.GetLength()) < (OUTPUT_FILE_FIELD_LENGTH - 4))
         sOutputFileNames << (sOutputFileNames.GetLength() > 0 ? ", " : "") << sWorkString;
      else
         sOutputFileNames << (sOutputFileNames.GetLength() > 0 ? ", " : "") << "...";
   }
   catch (ZdException &x) {
      x.AddCallpath("ReplaceExtensionAndAppend()", "stsRunHistoryFile");
      throw;
   }
}

// Creates the string to be outputed as the additional output filename string in the file
// pre : none
// post : sTempValue will contain the names of the additional output files
void stsRunHistoryFile::SetAdditionalOutputFileNameString(ZdString& sOutputFileNames, const CParameters& params) {
   ZdFileName   sResultFile(ZdFileName(params.m_szOutputFilename).GetFileName());

   try {
      sOutputFileNames.Clear();

      if(params.m_bSaveSimLogLikelihoods)
         ReplaceExtensionAndAppend(sOutputFileNames, sResultFile, ".llr.txt");
      if (params.GetDBaseOutputLogLikeli())
         ReplaceExtensionAndAppend(sOutputFileNames, sResultFile, ".llr.dbf");

      if(params.m_bOutputRelRisks)
         ReplaceExtensionAndAppend(sOutputFileNames, sResultFile, ".rr.txt");
      if(params.GetDBaseOutputRelRisks())
         ReplaceExtensionAndAppend(sOutputFileNames, sResultFile, ".rr.dbf");

      if(params.m_bOutputCensusAreas)
         ReplaceExtensionAndAppend(sOutputFileNames, sResultFile, ".gis.txt");
      if(params.GetOutputAreaSpecificDBF())
         ReplaceExtensionAndAppend(sOutputFileNames, sResultFile, ".gis.dbf");

      if(params.m_bMostLikelyClusters)
         ReplaceExtensionAndAppend(sOutputFileNames, sResultFile, ".col.txt");
      if(params.GetOutputClusterLevelDBF())
         ReplaceExtensionAndAppend(sOutputFileNames, sResultFile, ".col.dbf");

   }
   catch (ZdException &x) {
      x.AddCallpath("SetAdditionalOutputFileNameString()", "stsRunHistoryFile");
      throw;
   }
}

// sets the global filename variable
// pre: none
// post: makes sure the filename has a .dbf extension - will modify the filename if it does not
void stsRunHistoryFile::SetFileName(const ZdString& sFileName) {
   try {
      gsFilename = sFileName;
      ZdString sExt(ZdFileName(sFileName).GetExtension());
      if(sExt.GetIsEmpty())
         gsFilename << ".dbf";
      else if(sExt != ".dbf")
         gsFilename.Replace(sExt, ".dbf");
   }
   catch (ZdException &x) {
      x.AddCallpath("SetFileName()", "stsRunHistoryFile");
      throw;
   }
}

// sets the global variable glRunNumber and secures a unique run number in the file
// by adding a record with that run number to fix the multithreading issue
// if the file doesn't exist then it creates a new one and sets the run number to 1
// pre: none
// post: sets the run number and adds a record to the file with that number
void stsRunHistoryFile::SetRunNumber() {
   ZdTransaction	        *pTransaction = 0;
   std::auto_ptr<DBFFile>       pFile;

   try {
      // if we don't have one then create it
      if(!ZdIO::Exists(gsFilename.GetCString()))
         CreateRunHistoryFile();

      pFile.reset(new DBFFile(gsFilename));
      if(gvFields.empty())
         SetFieldVector(gvFields, *pFile);

      // get the run number field, so that this only has to be found once in the vector
      unsigned short  uwRunNumberField = GetFieldNumber(gvFields, RUN_NUMBER_FIELD);

      // get a record buffer, input data and append the record
      std::auto_ptr<ZdFileRecord> pLastRecord(pFile->GetNewRecord());
      if(pFile->GotoLastRecord(pLastRecord.get()))      // if there's records in the file
         glRunNumber = pLastRecord->GetLong(uwRunNumberField) + 1;
      else
         glRunNumber = 1;

      std::auto_ptr<ZdFileRecord> pRecord(pFile->GetNewRecord());
      pRecord->Clear();
      pRecord->PutField(uwRunNumberField, glRunNumber);       // run number field
      pRecord->PutField(GetFieldNumber(gvFields, OUTPUT_FILE_FIELD), "Run started, but not completed.");   // output filename field, but for now a text field
                                                                 // that I can use to display this message - will let
                                                                 // the user know if an anlysis failed or was cancelled - AJV 9/24/2002
      pTransaction = pFile->BeginTransaction();
      pFile->AppendRecord(*pTransaction, *pRecord);
      pFile->EndTransaction(pTransaction); pTransaction = 0;
      pFile->Close();
   }
   catch (ZdException &x) {
      if(pTransaction)
            pFile->EndTransaction(pTransaction);
         pTransaction = 0;   
      x.AddCallpath("SetRunNumber()", "stsRunHistoryFile");
      throw;
   }
}

// strips the carriage return and line feed off of the string because some ZdFile's don't like them embedded in fields
// pre: none
// post: returns the string by reference without the CR or LF
void stsRunHistoryFile::StripCRLF(ZdString& sStore) {
   try {
      sStore.Replace("\n", "", true);
      sStore.Replace("\r", "", true);
   }
   catch (ZdException &x) {
      x.AddCallpath("StripCRLF()", "stsRunHistoryFile");
      throw;
   }
}

