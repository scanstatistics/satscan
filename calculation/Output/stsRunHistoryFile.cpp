// class stsRunHistoryFile
// Adam J Vaughn
// 9/4/2002

// This class keeps a CSV file log for each run of the SaTScan program which includes information
// specified by the client. For each run the class will record a new run number in the file and
// record the pertinent data along with that run.

#include "SaTScan.h"
#pragma hdrstop

#include "Analysis.h"
#include "stsRunHistoryFile.h"

const char*      ANALYSIS_HISTORY_FILE  = "AnalysisHistory.csv";

// constructor
stsRunHistoryFile::stsRunHistoryFile(const CAnalysis* pAnalysis) {
   try {
      Init();
      Setup(pAnalysis);
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

// creates the run history file
// pre: csv file doesn't not already exist
// post: will create the csv file with the appropraite fields
void stsRunHistoryFile::CreateRunHistoryFile() {
   ZdVector<char>       vFieldTypes;
   ZdVector<ZdString>	vFieldNames;
   ZdField*		pField = 0;
   ZdVector<ZdField*>	vFields;
   CSVFile              *pFile = 0;

   try {
      pFile = new CSVFile();
      SetupFields(vFieldNames, vFieldTypes);
      for(unsigned int i = 0; i < vFieldNames.GetNumElements(); ++i) {
         pField = pFile->GetNewField();
         pField->SetName(vFieldNames[i].GetCString());
         pField->SetType(vFieldTypes[i]);
         vFields.AddElement(pField->Clone());
         delete pField;
      }

      pFile->PackFields(vFields);
      pFile->Create(gsFilename, vFields, ZDIO_OPEN_READ | ZDIO_OPEN_WRITE);
      pFile->Close();

      for (unsigned int i = vFields.GetNumElements() - 1; i > 0; --i) {
         delete vFields[0]; vFields[0] = 0;
         vFields.RemoveElement(0);
      }
      delete pFile;
   }
   catch (ZdException &x) {     // geesh there's a lot of cleanup here
      try {
         if(pFile)
            pFile->Close();
         delete pFile; pFile = 0;
         delete pField; pField = 0;
         for (unsigned int i = vFields.GetNumElements() - 1; i > 0; --i) {
            delete vFields[0]; vFields[0] = 0;
            vFields.RemoveElement(0);
         }
      }
      catch (...) {/*munch munch here, with all this cleanup going on, I don't want to throw an exception from within
                  an exception, so I'll just suck it up here - AJV 9/3/2002*/}
      x.AddCallpath("CreateRunHistoryFile()", "stsRunHistoryFile");
      throw;
   }
}

// global initializations
void stsRunHistoryFile::Init() {
   gpAnalysis = 0;
   glRunNumber = 0;
}

// although the name implies an oxymoron, this function will record a new run into the history file
// I am also aware that this function is just a facade for the OpenReportHistory function but I prefer
// the name of this function to be used by the user because it is more intuitive and I didn't feel like
// renaming the function OpenReportHistoryFile - AJV 9/4/2002
// pre: none
// post: records the run history
void stsRunHistoryFile::LogNewHistory() {
   try {
      OpenRunHistoryFile();
   }
   catch (ZdException &x) {
      x.AddCallpath("LogNewHistory()", "stsRunHistoryFile");
      throw;
   }
}

// tries to open the run history file if one exists, if not creates the file
// pre: none
// post: opens/creates the run history file and writes to it
void stsRunHistoryFile::OpenRunHistoryFile() {
   CSVFile		*pFile = 0;
   ZdTransaction*	pTransaction = 0;
   CSVRecord            *pRecord = 0, *pLastRecord = 0;
   unsigned long        ulLastRecordNumber;
   unsigned short       uwFieldNumber = 0;

   try {
      // if we don't have one then create it
      if(!ZdIO::Exists(gsFilename.GetCString()))
         CreateRunHistoryFile();

      pFile = new CSVFile(gsFilename, ZDIO_OPEN_READ | ZDIO_OPEN_WRITE);
      pTransaction = pFile->BeginTransaction();

      // get a record buffer, input data and append the record
      ulLastRecordNumber = pFile->GotoLastRecord(pLastRecord);
      // if there's records in the file
      if(ulLastRecordNumber)
         pLastRecord->GetField(0, glRunNumber);
      delete pLastRecord;

      // note: I'm going to document the heck out of this section in case they can't the run
      // specs on us at any time and that way I can interpret my assumptions in case any just so
      // happen to be incorrect, so bear with me - AJV 9/3/2002
      pRecord = pFile->GetNewRecord();
      //  run number field -- increment the run number so that we have a new unique run number - AJV 9/4/2002
      pRecord->PutField(uwFieldNumber, ++glRunNumber);

      // run time and date field
      pRecord->PutField(++uwFieldNumber, gpAnalysis->GetStartTime());

      // output file name field
      pRecord->PutAlpha(++uwFieldNumber, gpAnalysis->GetSatScanData()->m_pParameters->m_szOutputFilename);

      // probability model field
      pRecord->PutLong(++uwFieldNumber, gpAnalysis->GetSatScanData()->m_pParameters->m_nModel);

      // rates(high, low or both) field
      pRecord->PutLong(++uwFieldNumber, gpAnalysis->GetSatScanData()->m_pParameters->m_nAreas);

      // coordinate type field
      pRecord->PutLong(++uwFieldNumber, gpAnalysis->GetSatScanData()->m_pParameters->m_nCoordType);

      // analysis type field
      pRecord->PutLong(++uwFieldNumber, gpAnalysis->GetSatScanData()->m_pParameters->m_nAnalysisType);

      // total number of cases field
      pRecord->PutLong(++uwFieldNumber, gpAnalysis->GetSatScanData()->m_nTotalCases);

      // total population field
      pRecord->PutNumber(++uwFieldNumber, gpAnalysis->GetSatScanData()->m_nTotalPop);

      // number of geographic areas field
      pRecord->PutLong(++uwFieldNumber, gpAnalysis->GetSatScanData()->m_nTracts);

      // precision of case times field
      pRecord->PutLong(++uwFieldNumber, gpAnalysis->GetSatScanData()->m_pParameters->m_nPrecision);

      // max geographic extent field
      pRecord->PutNumber(++uwFieldNumber, gpAnalysis->GetSatScanData()->m_pParameters->m_nMaxGeographicClusterSize);

      // max temporal extent field
      pRecord->PutNumber(++uwFieldNumber, gpAnalysis->GetSatScanData()->m_pParameters->m_nMaxTemporalClusterSize);

      // time trend adjustment field
      pRecord->PutLong(++uwFieldNumber, gpAnalysis->GetSatScanData()->m_pParameters->m_nTimeAdjustType);

      // special grid file used field
      pRecord->PutBoolean(++uwFieldNumber, gpAnalysis->GetSatScanData()->m_pParameters->m_bSpecialGridFile);

      // start date field
      pRecord->PutAlpha(++uwFieldNumber, gpAnalysis->GetSatScanData()->m_pParameters->m_szStartDate);

      // end date field
      pRecord->PutAlpha(++uwFieldNumber, gpAnalysis->GetSatScanData()->m_pParameters->m_szEndDate);

      // alive clusters only field
      pRecord->PutBoolean(++uwFieldNumber, gpAnalysis->GetSatScanData()->m_pParameters->m_bAliveClustersOnly);

      // interval units field
      pRecord->PutLong(++uwFieldNumber, gpAnalysis->GetSatScanData()->m_pParameters->m_nIntervalUnits);

      // intervals length field
      pRecord->PutLong(++uwFieldNumber, gpAnalysis->GetSatScanData()->m_pParameters->m_nIntervalLength);

      // monte carlo (what does poker have to do with this?) replications field
      pRecord->PutLong(++uwFieldNumber, gpAnalysis->GetSatScanData()->m_pParameters->m_nReplicas);

      // 0.01 cutoff field
      pRecord->PutLong(++uwFieldNumber, gpAnalysis->GetSimRatio01());

      // 0.05 cutoff field
      pRecord->PutLong(++uwFieldNumber, gpAnalysis->GetSimRatio05());

      // number of clusters significant at tthe .05 llr cutoff field
//      pRecord->PutLong(++uwFieldNumber, gpAnalysis->m_pData->m_pParameters->);

      pFile->AppendRecord(*pTransaction, *pRecord);
      delete pRecord;

      pFile->EndTransaction(pTransaction);
      pFile->Close();
      delete pTransaction;
      delete pFile;      
   }
   catch(ZdException &x) {
      if(pFile) {
         if(pTransaction)
            pFile->EndTransaction(pTransaction);
         pFile->Close();
      }   
      delete pFile; pFile = 0;
      delete pTransaction; pTransaction = 0;
      delete pRecord; pRecord = 0;
      delete pLastRecord; pLastRecord = 0;
      x.AddCallpath("OpenRunHistoryFile()", "stsRunHistoryFile");
      throw;
   }
}

// internal setup
void stsRunHistoryFile::Setup(const CAnalysis* pAnalysis) {
   try {
      gsFilename = ANALYSIS_HISTORY_FILE;;
      gpAnalysis = const_cast<CAnalysis*>(pAnalysis);
   }
   catch (ZdException &x) {
      x.AddCallpath("Setup()", "stsRunHistoryFile");
      throw;
   }
}

// creates the vector of field names
// pre: pass in an empty vector of strings
// post: passes back through reference the vector of field names
void stsRunHistoryFile::SetupFields(ZdVector<ZdString>& vFields, ZdVector<char>& vFieldTypes ) {
   try {
      vFields.AddElement("Run_Number");
      vFieldTypes.AddElement(ZD_NUMBER_FLD);
      vFields.AddElement("Run_Time");
      vFieldTypes.AddElement(ZD_ALPHA_FLD);
      vFields.AddElement("Output_File");
      vFieldTypes.AddElement(ZD_ALPHA_FLD);
      vFields.AddElement("Prob_Model");
      vFieldTypes.AddElement(ZD_ALPHA_FLD);
      vFields.AddElement("Rates");
      vFieldTypes.AddElement(ZD_ALPHA_FLD);
      vFields.AddElement("Coord_Type");
      vFieldTypes.AddElement(ZD_ALPHA_FLD);
      vFields.AddElement("Analysis_Type");
      vFieldTypes.AddElement(ZD_ALPHA_FLD);
      vFields.AddElement("Number_Cases");
      vFieldTypes.AddElement(ZD_NUMBER_FLD);
      vFields.AddElement("Total_Pop");
      vFieldTypes.AddElement(ZD_NUMBER_FLD);
      vFields.AddElement("Num_Geo_Areas");
      vFieldTypes.AddElement(ZD_NUMBER_FLD);
      vFields.AddElement("Precis_Times");
      vFieldTypes.AddElement(ZD_NUMBER_FLD);
      vFields.AddElement("Max_Geo_Extent");
      vFieldTypes.AddElement(ZD_NUMBER_FLD);
      vFields.AddElement("Max_Temp_Extent");
      vFieldTypes.AddElement(ZD_NUMBER_FLD);
      vFields.AddElement("Time_Adjust");
      vFieldTypes.AddElement(ZD_ALPHA_FLD);
      vFields.AddElement("Grid_File");
      vFieldTypes.AddElement(ZD_ALPHA_FLD);
      vFields.AddElement("Start_Date");
      vFieldTypes.AddElement(ZD_ALPHA_FLD);
      vFields.AddElement("End_Date");
      vFieldTypes.AddElement(ZD_ALPHA_FLD);
      vFields.AddElement("Alive_Only");
      vFieldTypes.AddElement(ZD_ALPHA_FLD);
      vFields.AddElement("Interv_Units");
      vFieldTypes.AddElement(ZD_ALPHA_FLD);
      vFields.AddElement("Interv_Len");
      vFieldTypes.AddElement(ZD_ALPHA_FLD);
      vFields.AddElement("Monte_Carlo");
      vFieldTypes.AddElement(ZD_NUMBER_FLD);
      vFields.AddElement("001_CutOff");
      vFieldTypes.AddElement(ZD_NUMBER_FLD);
      vFields.AddElement("005_CutOff");
      vFieldTypes.AddElement(ZD_NUMBER_FLD);
      vFields.AddElement("Num_Signif_005");
      vFieldTypes.AddElement(ZD_NUMBER_FLD);
   }
   catch (ZdException &x) {
      x.AddCallpath("SetupFields()", "stsRunHistoryFile");
      throw;
   }
}