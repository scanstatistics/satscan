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

const char*      ANALYSIS_HISTORY_FILE  = "AnalysisHistory.txd";

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
// pre: txd file doesn't not already exist
// post: will create the csv file with the appropraite fields
void stsRunHistoryFile::CreateRunHistoryFile() {
   ZdField*		pField = 0;
   ZdVector<ZdField*>	vFields;
   TXDFile              *pFile = 0;
   ZdVector<pair<pair<ZdString, char>, long> >  vFieldDescrip;

   try {
      pFile = new TXDFile();
      SetupFields(vFieldDescrip);
      for(unsigned int i = 0; i < vFieldDescrip.GetNumElements(); ++i) {
         pField = pFile->GetNewField();
         pField->SetName(vFieldDescrip[i].first.first.GetCString());
         pField->SetType(vFieldDescrip[i].first.second);
         pField->SetLength(vFieldDescrip[i].second);
         vFields.AddElement(pField->Clone());
         delete pField;
      }

//      pFile->PackFields(vFields);
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
   TXDFile	        *pFile = 0;
   ZdTransaction*	pTransaction = 0;
   ZdFileRecord         *pRecord = 0, *pLastRecord = 0;
   unsigned long        ulLastRecordNumber;
   unsigned short       uwFieldNumber = 0;

   try {
      // if we don't have one then create it
      if(!ZdIO::Exists(gsFilename.GetCString()))
         CreateRunHistoryFile();

      pFile = (new TXDFile(gsFilename, ZDIO_OPEN_READ | ZDIO_OPEN_WRITE));
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
      pRecord->PutLong(uwFieldNumber, ++glRunNumber);

      // run time and date field
      ZdString sTime(gpAnalysis->GetStartTime());
      sTime.Replace("\r\n", "", true);
      sTime.Replace("\n", "", true);
      pRecord->PutAlpha(++uwFieldNumber, sTime.GetCString());

      // output file name field
      ZdString sOutputFilename(gpAnalysis->GetSatScanData()->m_pParameters->m_szOutputFilename);
      sOutputFilename.Replace("\r\n", "", true);
      sOutputFilename.Replace("\n", "", true);
      pRecord->PutAlpha(++uwFieldNumber, sOutputFilename.GetCString());

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
      pRecord->PutLong(++uwFieldNumber, (long)0);

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
      gsFilename = ANALYSIS_HISTORY_FILE;
      gpAnalysis = const_cast<CAnalysis*>(pAnalysis);
   }
   catch (ZdException &x) {
      x.AddCallpath("Setup()", "stsRunHistoryFile");
      throw;
   }
}

// creates the vector of field names
// pre: pass in an empty vector of pair of pair (triplets - this one's for you Brian S ;op)
// post: passes back through reference the vector of field names
void stsRunHistoryFile::SetupFields(ZdVector<pair<pair<ZdString, char>, long> >&  vFieldDescrip ) {
   std::pair<std::pair<ZdString, char>, long>   field;

   try {
      field.first.first = "Run_Number";
      field.first.second = ZD_NUMBER_FLD;
      field.second = 12;
      vFieldDescrip.AddElement(field);

      field.first.first = "Run_Time";
      field.first.second = ZD_ALPHA_FLD;
      field.second = 32;
      vFieldDescrip.AddElement(field);

      field.first.first = "Output_File";
      field.first.second = ZD_ALPHA_FLD;
      field.second = 128;
      vFieldDescrip.AddElement(field);

      field.first.first = "Prob_Model";
      field.first.second = ZD_ALPHA_FLD;
      field.second = 32;
      vFieldDescrip.AddElement(field);

      field.first.first = "Rates";
      field.first.second = ZD_ALPHA_FLD;
      field.second = 32;
      vFieldDescrip.AddElement(field);

      field.first.first = "Coord_Type";
      field.first.second = ZD_ALPHA_FLD;
      field.second = 32;
      vFieldDescrip.AddElement(field);

      field.first.first = "Analysis_Type";
      field.first.second = ZD_ALPHA_FLD;
      field.second = 32;
      vFieldDescrip.AddElement(field);

      field.first.first = "Number_Cases";
      field.first.second = ZD_LONG_FLD;
      field.second = 12;
      vFieldDescrip.AddElement(field);

      field.first.first = "Total_Pop";
      field.first.second = ZD_LONG_FLD;
      field.second = 12;
      vFieldDescrip.AddElement(field);

      field.first.first = "Num_Geo_Areas";
      field.first.second = ZD_LONG_FLD;
      field.second = 12;
      vFieldDescrip.AddElement(field);

      field.first.first = "Precis_Times";
      field.first.second = ZD_LONG_FLD;
      field.second = 12;
      vFieldDescrip.AddElement(field);

      field.first.first = "Max_Geo_Extent";
      field.first.second = ZD_NUMBER_FLD;
      field.second = 12;
      vFieldDescrip.AddElement(field);

      field.first.first = "Max_Temp_Extent";
      field.first.second = ZD_NUMBER_FLD;
      field.second = 12;
      vFieldDescrip.AddElement(field);

      field.first.first = "Time_Adjust";
      field.first.second = ZD_ALPHA_FLD;
      field.second = 32;
      vFieldDescrip.AddElement(field);

      field.first.first = "Grid_File";
      field.first.second = ZD_BOOLEAN_FLD;
      field.second = 1;
      vFieldDescrip.AddElement(field);

      field.first.first = "Start_Date";
      field.first.second = ZD_ALPHA_FLD;
      field.second = 32;
      vFieldDescrip.AddElement(field);

      field.first.first = "End_Date";
      field.first.second = ZD_ALPHA_FLD;
      field.second = 32;
      vFieldDescrip.AddElement(field);

      field.first.first = "Alive_Only";
      field.first.second = ZD_ALPHA_FLD;
      field.second = 12;
      vFieldDescrip.AddElement(field);

      field.first.first = "Interv_Units";
      field.first.second = ZD_ALPHA_FLD;
      field.second = 32;
      vFieldDescrip.AddElement(field);

      field.first.first = "Interv_Len";
      field.first.second = ZD_ALPHA_FLD;
      field.second = 32;
      vFieldDescrip.AddElement(field);

      field.first.first = "Monte_Carlo";
      field.first.second = ZD_LONG_FLD;
      field.second = 12;
      vFieldDescrip.AddElement(field);

      field.first.first = "001_CutOff";
      field.first.second = ZD_NUMBER_FLD;
      field.second = 12;
      vFieldDescrip.AddElement(field);

      field.first.first = "005_CutOff";
      field.first.second = ZD_NUMBER_FLD;
      field.second = 12;
      vFieldDescrip.AddElement(field);

      field.first.first = "Num_Signif_005";
      field.first.second = ZD_LONG_FLD;
      field.second = 12;
      vFieldDescrip.AddElement(field);
   }
   catch (ZdException &x) {
      x.AddCallpath("SetupFields()", "stsRunHistoryFile");
      throw;
   }
}