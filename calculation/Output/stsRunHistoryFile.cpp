// class stsRunHistoryFile
// Adam J Vaughn
// 9/4/2002

// This class keeps a TXD file log for each run of the SaTScan program which includes information
// specified by the client. For each instance of the class, a new, unnique run number will be 
// recorded in the file and the pertinent data will be updated once the analysis is complete.

#include "SaTScan.h"
#pragma hdrstop

#include "Analysis.h"
#include "stsOutputDBF.h"
#include "stsRunHistoryFile.h"

const char* RUN_NUMBER_FIELD            = "Run_Number";
const char* RUN_TIME_FIELD              = "Run_Time";
const char* OUTPUT_FILE_FIELD           = "Output_File";
const char* PROB_MODEL_FIELD            = "Prob_Model";
const char* RATES_FIELD                 = "Rates";
const char* COORD_TYPE_FIELD            = "Coord_Type";
const char* ANALYSIS_TYPE_FIELD         = "Analysis_Type";
const char* NUM_CASES_FIELD             = "Number_Cases";
const char* TOTAL_POP_FIELD             = "Total_Pop";
const char* NUM_GEO_AREAS_FIELD         = "Num_Geo_Areas";
const char* PRECISION_TIMES_FIELD       = "Precis_Times";
const char* MAX_GEO_EXTENT_FIELD        = "Max_Geo_Extent";
const char* MAX_TIME_EXTENT_FIELD       = "Max_Temp_Extent";
const char* TIME_TREND_ADJUSTMENT_FIELD = "Time_Trend_Adjust";
const char* GRID_FILE_FIELD             = "Grid_File";
const char* START_DATE_FIELD            = "Start_Date";
const char* END_DATE_FIELD              = "End_Date";
const char* ALIVE_ONLY_FIELD            = "Alive_Only";
const char* INTERVAL_UNITS_FIELD        = "Interv_Units";
const char* INTERVAL_LENGTH_FIELD       = "Interv_Len";
const char* MONTE_CARLO_FIELD           = "Monte_Carlo";
const char* CUTOFF_001_FIELD            = "001_CutOff";
const char* CUTOFF_005_FIELD            = "005_CutOff";
const char* NUM_SIGNIF_005_FIELD        = "Num_Signif_005";

// constructor
stsRunHistoryFile::stsRunHistoryFile(const ZdString& sFileName, BasePrint& PrintDirection)
                 : gsFilename(sFileName) , gpPrintDirection(&PrintDirection){
   try {
      Init();
      SetRunNumber();
   }
   catch (ZdException &x) {
      x.AddCallpath("Constructor", "stsRunHistoryFile");
      PrintDirection.SatScanPrintWarning("\nUnable to log analysis information:\n");
//      PrintDirection.SatScanPrintWarning(x.GetCallpath());
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
      ::CreateNewField(gvFields, RUN_NUMBER_FIELD, ZD_LONG_FLD, 8, 0, uwOffset, true);
      ::CreateNewField(gvFields, RUN_TIME_FIELD, ZD_ALPHA_FLD, 32, 0, uwOffset);

      ::CreateNewField(gvFields, ANALYSIS_TYPE_FIELD, ZD_ALPHA_FLD, 32, 0, uwOffset);
      ::CreateNewField(gvFields, PROB_MODEL_FIELD, ZD_ALPHA_FLD, 32, 0, uwOffset); 
      ::CreateNewField(gvFields, RATES_FIELD, ZD_ALPHA_FLD, 16, 0, uwOffset);
      ::CreateNewField(gvFields, GRID_FILE_FIELD, ZD_BOOLEAN_FLD, 1, 0, uwOffset);
      ::CreateNewField(gvFields, MAX_GEO_EXTENT_FIELD, ZD_NUMBER_FLD, 16, 3, uwOffset);
      ::CreateNewField(gvFields, MAX_TIME_EXTENT_FIELD, ZD_NUMBER_FLD, 16, 3, uwOffset);
      ::CreateNewField(gvFields, INTERVAL_LENGTH_FIELD, ZD_LONG_FLD, 8, 0, uwOffset);
      ::CreateNewField(gvFields, INTERVAL_UNITS_FIELD, ZD_ALPHA_FLD, 16, 0, uwOffset);
      ::CreateNewField(gvFields, ALIVE_ONLY_FIELD, ZD_BOOLEAN_FLD, 1, 0, uwOffset);
      ::CreateNewField(gvFields, TIME_TREND_ADJUSTMENT_FIELD, ZD_ALPHA_FLD, 20, 3, uwOffset);
      // covariates adjusted for
      ::CreateNewField(gvFields, MONTE_CARLO_FIELD, ZD_LONG_FLD, 8, 0, uwOffset);

      ::CreateNewField(gvFields, NUM_GEO_AREAS_FIELD, ZD_LONG_FLD, 8, 0, uwOffset);
      ::CreateNewField(gvFields, COORD_TYPE_FIELD, ZD_ALPHA_FLD, 16, 0, uwOffset);
      ::CreateNewField(gvFields, START_DATE_FIELD, ZD_ALPHA_FLD, 16, 0, uwOffset);
      ::CreateNewField(gvFields, END_DATE_FIELD, ZD_ALPHA_FLD, 16, 0, uwOffset);
      ::CreateNewField(gvFields, PRECISION_TIMES_FIELD, ZD_ALPHA_FLD, 16, 0, uwOffset);
      ::CreateNewField(gvFields, NUM_CASES_FIELD, ZD_LONG_FLD, 8, 0, uwOffset);
      ::CreateNewField(gvFields, TOTAL_POP_FIELD, ZD_NUMBER_FLD, 16, 3, uwOffset);

      // pval most likely cluster
      ::CreateNewField(gvFields, NUM_SIGNIF_005_FIELD, ZD_LONG_FLD, 8, 0, uwOffset);
      ::CreateNewField(gvFields, OUTPUT_FILE_FIELD, ZD_ALPHA_FLD, 254, 0, uwOffset);
      // additional text file names

      ::CreateNewField(gvFields, CUTOFF_001_FIELD, ZD_NUMBER_FLD, 8, 3, uwOffset);
      ::CreateNewField(gvFields, CUTOFF_005_FIELD, ZD_NUMBER_FLD, 8, 3, uwOffset);

      TXDFile File;
      File.PackFields(gvFields);
      File.Create(gsFilename, gvFields, 1);
      File.Close();
   }
   catch (ZdException &x) {
      x.AddCallpath("CreateRunHistoryFile()", "stsRunHistoryFile");
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
   gvFields.DeleteAllElements();
}

// although the name implies an oxymoron, this function will record a new run into the history file
// tries to open the run history file if one exists, if not creates the file
// pre: none
// post: opens/creates the run history file and records the run history
void stsRunHistoryFile::LogNewHistory(const CAnalysis& pAnalysis, const unsigned short uwSignificantAt005) {
   ZdTransaction	*pTransaction = 0;
   ZdString             sTempValue;
   std::auto_ptr<TXDFile>    pFile;

   try {
      pFile.reset(new TXDFile(gsFilename, ZDIO_OPEN_READ | ZDIO_OPEN_WRITE));

      // note: I'm going to document the heck out of this section for two reasons :
      // 1) in case they change the run specs on us at any time
      // 2) to present my assumptions about the output data in case any happen to be incorrect
      // , so bear with me - AJV 9/3/2002

      std::auto_ptr<ZdFileRecord> pRecord(pFile->GetNewRecord());
      pRecord->PutField(0, glRunNumber);
      if(!pFile->GotoRecordByKeys(pRecord.get(), pRecord.get()))
         ZdException::GenerateNotification("Error! Run number not found in the run history file.", "LogNewhistory()");

      //  run number field
      SetLongField(*pRecord, glRunNumber, GetFieldNumber(gvFields, RUN_NUMBER_FIELD));

      // run time and date field
      sTempValue = pAnalysis.GetStartTime();
      StripCRLF(sTempValue);
      SetStringField(*pRecord, sTempValue, GetFieldNumber(gvFields, RUN_TIME_FIELD));

      // output file name field
      sTempValue = pAnalysis.GetSatScanData()->m_pParameters->m_szOutputFilename;
      StripCRLF(sTempValue);
      SetStringField(*pRecord, sTempValue, GetFieldNumber(gvFields, OUTPUT_FILE_FIELD));

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

      SetLongField(*pRecord, pAnalysis.GetSatScanData()->m_nTotalCases, GetFieldNumber(gvFields, NUM_CASES_FIELD));   // total number of cases field
      SetDoubleField(*pRecord, pAnalysis.GetSatScanData()->m_nTotalPop, GetFieldNumber(gvFields, TOTAL_POP_FIELD));  // total population field
      SetLongField(*pRecord, pAnalysis.GetSatScanData()->m_nTracts, GetFieldNumber(gvFields, NUM_GEO_AREAS_FIELD));     // number of geographic areas field

      // precision of case times field
      GetCasePrecisionString(sTempValue, pAnalysis.GetSatScanData()->m_pParameters->m_nPrecision);
      SetStringField(*pRecord, sTempValue, GetFieldNumber(gvFields, PRECISION_TIMES_FIELD));

      SetDoubleField(*pRecord, pAnalysis.GetSatScanData()->m_pParameters->m_nMaxGeographicClusterSize, GetFieldNumber(gvFields, MAX_GEO_EXTENT_FIELD));   // max geographic extent field
      SetDoubleField(*pRecord, pAnalysis.GetSatScanData()->m_pParameters->m_nMaxTemporalClusterSize, GetFieldNumber(gvFields, MAX_TIME_EXTENT_FIELD));   // max temporal extent field

      // time trend adjustment field
      GetTimeAdjustmentString(sTempValue, pAnalysis.GetSatScanData()->m_pParameters->m_nTimeAdjustType);
      SetStringField(*pRecord, sTempValue, GetFieldNumber(gvFields, TIME_TREND_ADJUSTMENT_FIELD));

      SetBoolField(*pRecord, pAnalysis.GetSatScanData()->m_pParameters->m_bSpecialGridFile, GetFieldNumber(gvFields, GRID_FILE_FIELD)); // special grid file used field
      SetStringField(*pRecord, pAnalysis.GetSatScanData()->m_pParameters->m_szStartDate, GetFieldNumber(gvFields, START_DATE_FIELD));  // start date field
      SetStringField(*pRecord, pAnalysis.GetSatScanData()->m_pParameters->m_szEndDate, GetFieldNumber(gvFields, END_DATE_FIELD)); // end date field
      SetBoolField(*pRecord, pAnalysis.GetSatScanData()->m_pParameters->m_bAliveClustersOnly, GetFieldNumber(gvFields, ALIVE_ONLY_FIELD)); // alive clusters only field

      // interval units field
      GetIntervalUnitsString(sTempValue, pAnalysis.GetSatScanData()->m_pParameters->m_nIntervalUnits);
      SetStringField(*pRecord, sTempValue, GetFieldNumber(gvFields, INTERVAL_UNITS_FIELD));

      SetLongField(*pRecord, pAnalysis.GetSatScanData()->m_pParameters->m_nIntervalLength, GetFieldNumber(gvFields, INTERVAL_LENGTH_FIELD)); // intervals length field
      SetLongField(*pRecord, pAnalysis.GetSatScanData()->m_pParameters->m_nReplicas, GetFieldNumber(gvFields, MONTE_CARLO_FIELD));  // monte carlo  replications field
      SetDoubleField(*pRecord, pAnalysis.GetSimRatio01(), GetFieldNumber(gvFields, CUTOFF_001_FIELD)); // 0.01 cutoff field
      SetDoubleField(*pRecord, pAnalysis.GetSimRatio05(), GetFieldNumber(gvFields, CUTOFF_005_FIELD)); // 0.05 cutoff field
      SetLongField(*pRecord, (long)uwSignificantAt005, GetFieldNumber(gvFields, NUM_SIGNIF_005_FIELD));  // number of clusters significant at tthe .05 llr cutoff field

      pTransaction = (pFile->BeginTransaction());
      pFile->UpdateRecord(*pTransaction, *pRecord);
      pFile->EndTransaction(pTransaction); pTransaction = 0;
      pFile->Close();
   }
   catch(ZdException &x) {
      try {
         if(pTransaction)
            pFile->EndTransaction(pTransaction);   // if there is a pTransaction then there must be a pFile, so this is valid
         pTransaction = 0;
         pFile->Close();
      }
      catch(...) {   // Although I really hate to do this here, it is necessary because I don't want
                       // to throw because that would cause the thread to terminate and the calculation
                       // not to be finished printing. Instead we'll just print a message to the user that
                       // the log file was not able to be written to - AJV 9/24/2002
         x.AddCallpath("LogNewHistory()", "stsRunHistoryFile");
         gpPrintDirection->SatScanPrintWarning("\nUnable to log analysis information:\n");
//         gpPrintDirection->SatScanPrintWarning(x.GetCallpath());
         gpPrintDirection->SatScanPrintWarning(x.GetErrorMessage());
         gpPrintDirection->SatScanPrintWarning("\n");
      }
   }
}

// sets the global variable glRunNumber and secures a unique run number in the file
// by adding a record with that run number to fix the multithreading issue
// pre: none
// post: sets the run number and adds a record to the file with that number
void stsRunHistoryFile::SetRunNumber() {
   ZdTransaction	        *pTransaction = 0;
   std::auto_ptr<TXDFile>       pFile;

   try {
      // if we don't have one then create it
      if(!ZdIO::Exists(gsFilename.GetCString()))
         CreateRunHistoryFile();

      pFile.reset(new TXDFile(gsFilename, ZDIO_OPEN_READ | ZDIO_OPEN_WRITE));
      if(gvFields.empty())
         SetFieldVector(gvFields, *pFile);

      // get the run number field, so that this only has to be found once in the vector
      unsigned short  uwRunNumberField = GetFieldNumber(gvFields, RUN_NUMBER_FIELD);

      // get a record buffer, input data and append the record
      std::auto_ptr<TXDRec> pLastRecord(pFile->GetNewRecord());
      if(pFile->GotoLastRecord(pLastRecord.get()))      // if there's records in the file
         glRunNumber = pLastRecord->GetLong(uwRunNumberField) + 1;
      else
         glRunNumber = 1;

      std::auto_ptr<TXDRec> pRecord(pFile->GetNewRecord());
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
      pFile->Close();   
      x.AddCallpath("SetRunNumber()", "stsRunHistoryFile");
      throw;
   }
}

// strips the carriage return and line feed off of the string because TXD doesn't like them embedded in fields
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

