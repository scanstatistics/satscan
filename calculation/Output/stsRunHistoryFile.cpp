// class stsRunHistoryFile
// Adam J Vaughn
// 9/4/2002

// This class keeps a TXD file log for each run of the SaTScan program which includes information
// specified by the client. For each run the class will record a new run number in the file and
// record the pertinent data along with that run.

#include "SaTScan.h"
#pragma hdrstop

#include "Analysis.h"
#include "stsRunHistoryFile.h"

// constructor
stsRunHistoryFile::stsRunHistoryFile(const ZdString& sFileName) {
   try {
      Init();
      Setup(sFileName);
   }
   catch (ZdException &x) {
      x.AddCallpath("Constructor", "stsRunHistoryFile");
      throw;
   }
}

// destructor
stsRunHistoryFile::~stsRunHistoryFile() {
   try {
   }
   catch (...) {/* munch munch */}
}

// deletes the memory allocated to the ZdField pointer vector because I do
// not trust ZdPointerVector I prefer this method - AJV 9/23/2002
// pre : an allocated vector of ZdField pointers
// post: deletes the memory allocated to the vector and removes all elements
void stsRunHistoryFile::CleanupFieldVector(ZdVector<ZdField*>& vFields) {
   try {
      while (vFields.GetNumElements()) {
         delete vFields[0]; vFields[0] = 0;
         vFields.RemoveElement(0);
      }
   }
   catch (ZdException &x) {
      x.AddCallpath("CleanupFieldVector()", "stsRunHistoryFile");
      throw;
   }
}

// creates the run history file
// pre: txd file doesn't not already exist
// post: will create the txd file with the appropraite fields
void stsRunHistoryFile::CreateRunHistoryFile() {
   ZdField		                *pField = 0;
   ZdVector<ZdField*>	                vFields;
   TXDFile                              File;
   unsigned short                       uwOffset = 0;
   std::vector<history_field_t>         vFieldDescriptors;

   try {
      SetupFields(vFieldDescriptors);

      for(size_t i = 0; i < vFieldDescriptors.size(); ++i) {
         pField = (File.GetNewField());
         pField->SetName(vFieldDescriptors[i].gsFieldName.c_str());
         pField->SetType(vFieldDescriptors[i].gcFieldType);
         pField->SetLength(vFieldDescriptors[i].gwFieldLength);
         pField->SetOffset(uwOffset);
         uwOffset += vFieldDescriptors[i].gwFieldLength;
         if(!i)                    // first field needs to be indexed in order to use GotoRecordByKeys() = AJV 9/24/2002
            pField->SetIndexCount(1);
         vFields.AddElement(pField);
      }

      File.PackFields(vFields);
      File.Create(gsFilename, vFields, 1);
      File.Close();

      CleanupFieldVector(vFields);
   }
   catch (ZdException &x) {   
      CleanupFieldVector(vFields);
      x.AddCallpath("CreateRunHistoryFile()", "stsRunHistoryFile");
      throw;
   }
}

// global initializations
void stsRunHistoryFile::Init() {
   glRunNumber = 0;
}

// although the name implies an oxymoron, this function will record a new run into the history file
// tries to open the run history file if one exists, if not creates the file
// pre: none
// post: opens/creates the run history file and records the run history
void stsRunHistoryFile::LogNewHistory(const CAnalysis* pAnalysis, const unsigned short& uwSignificantAt005, BasePrint& PrintDirection) {
   ZdTransaction	*pTransaction = 0;
   unsigned short       uwFieldNumber = 0;
   ZdString             sTempValue;
   auto_ptr<ZdFileRecord> pRecord;
   auto_ptr<TXDFile>    pFile;

   try {
      pFile.reset(new TXDFile(gsFilename, ZDIO_OPEN_READ | ZDIO_OPEN_WRITE));

      pTransaction = (pFile->BeginTransaction());

      // note: I'm going to document the heck out of this section in case they can't the run
      // specs on us at any time and that way I can interpret my assumptions in case any just so
      // happen to be incorrect, so bear with me - AJV 9/3/2002

      pRecord.reset(pFile->GetNewRecord());
      pRecord->PutField(0, glRunNumber);
      if(!pFile->GotoRecordByKeys(&(*pRecord), &(*pRecord)))
         ZdException::GenerateNotification("Error! Run number not found in the run history file.", "LogNewhistory()");

      //  run number field -- increment the run number so that we have a new unique run number - AJV 9/4/2002
      SetDoubleField(*pRecord, double(glRunNumber), uwFieldNumber);

      // run time and date field
      sTempValue << pAnalysis->GetStartTime();    // hack here because txd files don't like embedded \r or \n AJV
      sTempValue.Replace("\n", "", true);
      sTempValue.Replace("\r", "", true);
      SetStringField(*pRecord, sTempValue, (++uwFieldNumber));

      // output file name field
      sTempValue << ZdString::reset << pAnalysis->GetSatScanData()->m_pParameters->m_szOutputFilename;      // hack here because txd files don't like embedded \r or \n AJV
      sTempValue.Replace("\r", "", true);
      sTempValue.Replace("\n", "", true);
      SetStringField(*pRecord, sTempValue, (++uwFieldNumber));

      // probability model field
      switch(pAnalysis->GetSatScanData()->m_pParameters->m_nModel) {
         case POISSON :
            sTempValue << ZdString::reset << "Poisson";
            break;
         case BERNOULLI :
            sTempValue << ZdString::reset << "Bernoulli";
            break;
         case SPACETIMEPERMUTATION :
            sTempValue << ZdString::reset << "Space Time Permutation";
            break;
      }
      SetStringField(*pRecord, sTempValue, (++uwFieldNumber));

      // rates(high, low or both) field
      switch (pAnalysis->GetSatScanData()->m_pParameters->m_nAreas) {
         case HIGH :
            sTempValue << ZdString::reset << "High";
            break;
         case LOW :
            sTempValue << ZdString::reset << "Low";
            break;
         case HIGHANDLOW :
            sTempValue << ZdString::reset << "Both";
            break;
      }
      SetStringField(*pRecord, sTempValue, (++uwFieldNumber));

      // coordinate type field
      sTempValue << ZdString::reset << ((pAnalysis->GetSatScanData()->m_pParameters->m_nCoordType == CARTESIAN) ? "Cartesian" : "LongLat");
      SetStringField(*pRecord, sTempValue, (++uwFieldNumber));

      // analysis type field
      switch(pAnalysis->GetSatScanData()->m_pParameters->m_nAnalysisType) {
         case PURELYSPATIAL :
            sTempValue << ZdString::reset << "Purely Spatial";
            break;
         case PURELYTEMPORAL :
            sTempValue << ZdString::reset << "Purely Temporal";
            break;
         case SPACETIME :
            sTempValue << ZdString::reset << "Space Time";
            break;
         case PROSPECTIVESPACETIME :
            sTempValue << ZdString::reset << "Prospective Space Time";
            break;
         case PURELYSPATIALMONOTONE :
            sTempValue << ZdString::reset << "Purely Spatial Monotone";
            break;
      }
      SetStringField(*pRecord, sTempValue, (++uwFieldNumber));
      SetLongField(*pRecord, pAnalysis->GetSatScanData()->m_nTotalCases, (++uwFieldNumber));   // total number of cases field
      SetLongField(*pRecord, pAnalysis->GetSatScanData()->m_nTotalPop, (++uwFieldNumber));  // total population field
      SetLongField(*pRecord, pAnalysis->GetSatScanData()->m_nTracts, (++uwFieldNumber));     // number of geographic areas field

      // precision of case times field
      switch (pAnalysis->GetSatScanData()->m_pParameters->m_nPrecision) {
         case 0:
            sTempValue << ZdString::reset << "None";
            break;
         case 1:
            sTempValue << ZdString::reset << "Year";
            break;
         case 2:
            sTempValue << ZdString::reset << "Month";
            break;
         case 3:
            sTempValue << ZdString::reset << "Day";
            break;
      }
      SetStringField(*pRecord, sTempValue, (++uwFieldNumber));
      SetLongField(*pRecord, pAnalysis->GetSatScanData()->m_pParameters->m_nMaxGeographicClusterSize, (++uwFieldNumber));   // max geographic extent field
      SetLongField(*pRecord, pAnalysis->GetSatScanData()->m_pParameters->m_nMaxTemporalClusterSize, (++uwFieldNumber));   // max temporal extent field

      // time trend adjustment field
      switch(pAnalysis->GetSatScanData()->m_pParameters->m_nTimeAdjustType) {
         case NOTADJUSTED :
            sTempValue << ZdString::reset << "None";
            break;
         case NONPARAMETRIC :
            sTempValue << ZdString::reset << "Non-parametric";
            break;
         case LINEAR :
            sTempValue << ZdString::reset << "Linear";
            break;
      }
      SetStringField(*pRecord, sTempValue, (++uwFieldNumber));
      SetBoolField(*pRecord, pAnalysis->GetSatScanData()->m_pParameters->m_bSpecialGridFile, (++uwFieldNumber)); // special grid file used field
      SetStringField(*pRecord, pAnalysis->GetSatScanData()->m_pParameters->m_szStartDate, (++uwFieldNumber));  // start date field
      SetStringField(*pRecord, pAnalysis->GetSatScanData()->m_pParameters->m_szEndDate, (++uwFieldNumber)); // end date field
      SetBoolField(*pRecord, pAnalysis->GetSatScanData()->m_pParameters->m_bAliveClustersOnly, (++uwFieldNumber)); // alive clusters only field

      // interval units field
      switch (pAnalysis->GetSatScanData()->m_pParameters->m_nIntervalUnits) {
         case 0:
            sTempValue << ZdString::reset << "None";
            break;
         case 1:
            sTempValue << ZdString::reset << "Year";
            break;
         case 2:
            sTempValue << ZdString::reset << "Month";
            break;
         case 3:
            sTempValue << ZdString::reset << "Day";
            break;
      }
      SetStringField(*pRecord, sTempValue, (++uwFieldNumber));
      SetLongField(*pRecord, pAnalysis->GetSatScanData()->m_pParameters->m_nIntervalLength, (++uwFieldNumber)); // intervals length field
      SetLongField(*pRecord, pAnalysis->GetSatScanData()->m_pParameters->m_nReplicas, (++uwFieldNumber));  // monte carlo  replications field
      SetDoubleField(*pRecord, pAnalysis->GetSimRatio01(), (++uwFieldNumber)); // 0.01 cutoff field
      SetDoubleField(*pRecord, pAnalysis->GetSimRatio05(), (++uwFieldNumber)); // 0.05 cutoff field
      SetLongField(*pRecord, (long)uwSignificantAt005, (++uwFieldNumber));  // number of clusters significant at tthe .05 llr cutoff field

      pFile->UpdateRecord(*pTransaction, *pRecord);
      pFile->EndTransaction(pTransaction); pTransaction = 0;
      pFile->Close();
   }
   catch(ZdException &x) {
      try {
         if(pTransaction)
            pFile->EndTransaction(pTransaction);
         pTransaction = 0;
         pFile->Close();
      }
      catch(...) {}    // Although I really hate to do this here, it is necessary because I don't want
                       // to throw because that would cause the thread to terminate and the calculation
                       // not to be finished printing. Instead we'll just print a message to the user that
                       // the log file was not able to be written to - AJV 9/24/2002
      x.AddCallpath("LogNewHistory()", "stsRunHistoryFile");
      PrintDirection.SatScanPrintWarning("\nUnable to log analysis information:\n");
      PrintDirection.SatScanPrintWarning(x.GetCallpath());
      PrintDirection.SatScanPrintWarning(x.GetErrorMessage());
      PrintDirection.SatScanPrintWarning("\n");
   }
}

// function to set the value of boolean fields
// pre: record has been allocated
// post: sets the values in the FieldNumber field of the record
void stsRunHistoryFile::SetBoolField(ZdFileRecord& record, const bool& bValue, const unsigned long& uwFieldNumber) {
   ZdFieldValue fv;

   try {
      fv.SetType(record.GetFieldType(uwFieldNumber));
      fv.AsBool() = bValue;
      record.PutFieldValue(uwFieldNumber, fv);
   }
   catch (ZdException &x) {
      x.AddCallpath("SetBoolField()", "stsRunHistoryFile");
      throw;
   }
}

// function to set the value of double fields
// pre: record has been allocated
// post: sets the values in the FieldNumber field of the record
void stsRunHistoryFile::SetDoubleField(ZdFileRecord& record, const double& dValue, const unsigned long& uwFieldNumber) {
   ZdFieldValue fv;

   try {
      fv.SetType(record.GetFieldType(uwFieldNumber));
      fv.AsDouble() = dValue;
      record.PutFieldValue(uwFieldNumber, fv);
   }
   catch (ZdException &x) {
      x.AddCallpath("SetDoubleField()", "stsRunHistoryFile");
      throw;
   }
}

// creates the history_field_t's and adds them to the vector
// pre: pass in an empty vector of history_field_t
// post: passes back through reference the filled vector of history_field_t
void stsRunHistoryFile::SetupFields(std::vector<history_field_t>&  vFieldDescrip ) {
   // why this is a double field I don't know, but need to leave it one for now
   //because else it would change the zds file -AJV 9/24/2002
   vFieldDescrip.push_back(history_field_t("Run_Number", ZD_NUMBER_FLD, 8));
   vFieldDescrip.push_back(history_field_t("Run_Time", ZD_ALPHA_FLD, 32));
   vFieldDescrip.push_back(history_field_t("Output_File", ZD_ALPHA_FLD, 254));
   vFieldDescrip.push_back(history_field_t("Prob_Model", ZD_ALPHA_FLD, 32));
   vFieldDescrip.push_back(history_field_t("Rates", ZD_ALPHA_FLD, 16));
   vFieldDescrip.push_back(history_field_t("Coord_Type", ZD_ALPHA_FLD, 16));
   vFieldDescrip.push_back(history_field_t("Analysis_Type", ZD_ALPHA_FLD, 32));
   vFieldDescrip.push_back(history_field_t("Number_Cases", ZD_LONG_FLD, 8));
   vFieldDescrip.push_back(history_field_t("Total_Pop", ZD_LONG_FLD, 12));
   vFieldDescrip.push_back(history_field_t("Num_Geo_Areas", ZD_LONG_FLD, 8));
   vFieldDescrip.push_back(history_field_t("Precis_Times", ZD_ALPHA_FLD, 16));
   vFieldDescrip.push_back(history_field_t("Max_Geo_Extent", ZD_LONG_FLD, 8));
   vFieldDescrip.push_back(history_field_t("Max_Temp_Extent", ZD_LONG_FLD, 8));
   vFieldDescrip.push_back(history_field_t("Time_Trend_Adjust", ZD_ALPHA_FLD, 20));
   vFieldDescrip.push_back(history_field_t("Grid_File", ZD_BOOLEAN_FLD, 1));
   vFieldDescrip.push_back(history_field_t("Start_Date", ZD_ALPHA_FLD, 16));
   vFieldDescrip.push_back(history_field_t("End_Date", ZD_ALPHA_FLD, 16));
   vFieldDescrip.push_back(history_field_t("Alive_Only", ZD_BOOLEAN_FLD, 1));
   vFieldDescrip.push_back(history_field_t("Interv_Units", ZD_ALPHA_FLD, 16));
   vFieldDescrip.push_back(history_field_t("Interv_Len", ZD_LONG_FLD, 8));
   vFieldDescrip.push_back(history_field_t("Monte_Carlo", ZD_LONG_FLD, 8));
   vFieldDescrip.push_back(history_field_t("001_CutOff", ZD_NUMBER_FLD, 8));
   vFieldDescrip.push_back(history_field_t("005_CutOff", ZD_NUMBER_FLD, 8));
   vFieldDescrip.push_back(history_field_t("Num_Signif_005", ZD_LONG_FLD, 8));
}

// function to set the value of long fields
// pre: record has been allocated
// post: sets the values in the FieldNumber field of the record
void stsRunHistoryFile::SetLongField(ZdFileRecord& record, const long& lValue, const unsigned long& uwFieldNumber) {
   ZdFieldValue fv;

   try {
      fv.SetType(record.GetFieldType(uwFieldNumber));
      fv.AsLong() = lValue;
      record.PutFieldValue(uwFieldNumber, fv);
   }
   catch (ZdException &x) {
      x.AddCallpath("SetLongField()", "stsRunHistoryFile");
      throw;
   }
}

// sets the global variable glRunNumber and secures a unique run number in the file
// by adding a record with that run number to fix the multithreading issue
// pre: glRunNumber = 0
// post: sets the run number and adds a record to the file with that number
void stsRunHistoryFile::SetRunNumber() {
   ZdTransaction	*pTransaction = 0;
   ZdString             sTempValue;
   auto_ptr<TXDRec>     pLastRecord, pRecord;
   auto_ptr<TXDFile>    pFile;

   try {
      // if we don't have one then create it
      if(!ZdIO::Exists(gsFilename.GetCString()))
         CreateRunHistoryFile();

      pFile.reset(new TXDFile(gsFilename, ZDIO_OPEN_READ | ZDIO_OPEN_WRITE));

      // get a record buffer, input data and append the record
      pLastRecord.reset(pFile->GetNewRecord());
      // if there's records in the file
      if(pFile->GotoLastRecord(&(*pLastRecord)))
         pLastRecord->GetField(0, glRunNumber);
      ++glRunNumber;

      pTransaction = pFile->BeginTransaction();
      pRecord.reset(pFile->GetNewRecord());
      pRecord->Clear();
      pRecord->PutField(0, glRunNumber);       // run number field
      pRecord->PutField(2, "Run started, but not completed.");   // output filename field, but for now a text field
                                                                 // that I can use to display this message - will let
                                                                 // the user know if an anlysis failed or was cancelled - AJV 9/24/2002

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

// function to set the value of string fields
// pre: record has been allocated
// post: sets the values in the FieldNumber field of the record
void stsRunHistoryFile::SetStringField(ZdFileRecord& record, const ZdString& sValue, const unsigned long& uwFieldNumber) {
   ZdFieldValue fv;

   try {
      fv.SetType(record.GetFieldType(uwFieldNumber));
      fv.AsZdString() = sValue;
      record.PutFieldValue(uwFieldNumber, fv);
   }
   catch (ZdException &x) {
      x.AddCallpath("SetStringField()", "stsRunHistoryFile");
      throw;
   }
}

// internal setup
void stsRunHistoryFile::Setup(const ZdString& sFileName) {
   try {
      gsFilename = sFileName;
      SetRunNumber();
   }
   catch (ZdException &x) {
      x.AddCallpath("Setup()", "stsRunHistoryFile");
      throw;
   }
}


