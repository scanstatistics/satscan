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

// constructor
stsRunHistoryFile::stsRunHistoryFile(const CAnalysis* pAnalysis, const ZdString& sFileName) {
   try {
      Init();
      Setup(pAnalysis, sFileName);
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
   ZdField		Field;
   ZdVector<ZdField*>	vFields;
   TXDFile              File;
   ZdVector<pair<pair<ZdString, char>, long> >  vFieldDescrip;
   unsigned short       uwOffset = 0;

   try {
      SetupFields(vFieldDescrip);
      for(unsigned int i = 0; i < vFieldDescrip.GetNumElements(); ++i) {
         Field = *(File.GetNewField());
         Field.SetName(vFieldDescrip[i].first.first.GetCString());
         Field.SetType(vFieldDescrip[i].first.second);
         Field.SetLength(vFieldDescrip[i].second);
         Field.SetOffset(uwOffset);
         uwOffset += (2 + vFieldDescrip[i].second);
         vFields.AddElement(Field.Clone());
      }

      File.PackFields(vFields);
      File.Create(gsFilename, vFields, ZDIO_OPEN_READ | ZDIO_OPEN_WRITE);
      File.Close();

      for (int i = vFields.GetNumElements() - 1; i > 0; --i) {
         delete vFields[0]; vFields[0] = 0;
         vFields.RemoveElement(0);
      }
   }
   catch (ZdException &x) {   
      for (int i = vFields.GetNumElements() - 1; i > 0; --i) {
         delete vFields[0]; vFields[0] = 0;
         vFields.RemoveElement(0);
      }
      x.AddCallpath("CreateRunHistoryFile()", "stsRunHistoryFile");
      throw;
   }
}

// returns the filename of the run history file
const ZdString& stsRunHistoryFile::GetRunHistoryFileName() const {
   return gsFilename;
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
void stsRunHistoryFile::LogNewHistory(const unsigned short& uwSignificantAt005) {
   try {
      OpenRunHistoryFile(uwSignificantAt005);
   }
   catch (ZdException &x) {
      x.AddCallpath("LogNewHistory()", "stsRunHistoryFile");
      throw;
   }
}

// tries to open the run history file if one exists, if not creates the file
// pre: none
// post: opens/creates the run history file and writes to it
void stsRunHistoryFile::OpenRunHistoryFile(const unsigned short& uwSignificantAt005) {
   ZdTransaction	*pTransaction = 0;
   ZdFileRecord         *pRecord = 0, *pLastRecord = 0;
   unsigned long        ulLastRecordNumber;
   unsigned short       uwFieldNumber = 0;
   ZdString             sTempTime;
   ZdFieldValue         fv;

   try {
      // if we don't have one then create it
      if(!ZdIO::Exists(gsFilename.GetCString()))
         CreateRunHistoryFile();

      TXDFile File(gsFilename, ZDIO_OPEN_READ | ZDIO_OPEN_WRITE);

      // get a record buffer, input data and append the record
      pLastRecord = File.GetNewRecord();
      ulLastRecordNumber = File.GotoLastRecord(pLastRecord);
      // if there's records in the file
      if(ulLastRecordNumber)
         pLastRecord->GetField(0, glRunNumber);
      delete pLastRecord; pLastRecord = 0;

      pTransaction = (File.BeginTransaction());

      // note: I'm going to document the heck out of this section in case they can't the run
      // specs on us at any time and that way I can interpret my assumptions in case any just so
      // happen to be incorrect, so bear with me - AJV 9/3/2002
      pRecord = File.GetNewRecord();
      //  run number field -- increment the run number so that we have a new unique run number - AJV 9/4/2002
      fv.SetType(pRecord->GetFieldType(uwFieldNumber));
      fv.AsDouble() = ++glRunNumber;
      pRecord->PutFieldValue(uwFieldNumber, fv);

      // run time and date field
      sTempTime << gpAnalysis->GetStartTime();    // hack here because txd files don't like embedded \r or \n AJV
      sTempTime.Replace("\n", "", true);
      sTempTime.Replace("\r", "", true);
      fv.SetType(pRecord->GetFieldType(++uwFieldNumber));
      fv.AsZdString() = sTempTime;
      pRecord->PutFieldValue(uwFieldNumber, fv);

      // output file name field
      sTempTime << ZdString::reset << gpAnalysis->GetSatScanData()->m_pParameters->m_szOutputFilename;      // hack here because txd files don't like embedded \r or \n AJV
      sTempTime.Replace("\r", "", true);
      sTempTime.Replace("\n", "", true);
      fv.SetType(pRecord->GetFieldType(++uwFieldNumber));
      fv.AsZdString() = sTempTime.GetCString();
      pRecord->PutFieldValue(uwFieldNumber, fv);

      // probability model field
      fv.SetType(pRecord->GetFieldType(++uwFieldNumber));
      fv.AsLong() = gpAnalysis->GetSatScanData()->m_pParameters->m_nModel;
      pRecord->PutFieldValue(uwFieldNumber, fv);

      // rates(high, low or both) field
      fv.SetType(pRecord->GetFieldType(++uwFieldNumber));
      fv.AsLong() =gpAnalysis->GetSatScanData()->m_pParameters->m_nAreas;
      pRecord->PutFieldValue(uwFieldNumber, fv);

      // coordinate type field
      fv.SetType(pRecord->GetFieldType(++uwFieldNumber));
      fv.AsLong() = gpAnalysis->GetSatScanData()->m_pParameters->m_nCoordType;
      pRecord->PutFieldValue(uwFieldNumber, fv);

      // analysis type field
      fv.SetType(pRecord->GetFieldType(++uwFieldNumber));
      fv.AsLong() = gpAnalysis->GetSatScanData()->m_pParameters->m_nAnalysisType;
      pRecord->PutFieldValue(uwFieldNumber, fv);

      // total number of cases field
      fv.SetType(pRecord->GetFieldType(++uwFieldNumber));
      fv.AsLong() = gpAnalysis->GetSatScanData()->m_nTotalCases;
      pRecord->PutFieldValue(uwFieldNumber, fv);

      // total population field
      fv.SetType(pRecord->GetFieldType(++uwFieldNumber));
      fv.AsLong() = gpAnalysis->GetSatScanData()->m_nTotalPop;
      pRecord->PutFieldValue(uwFieldNumber, fv);

      // number of geographic areas field
      fv.SetType(pRecord->GetFieldType(++uwFieldNumber));
      fv.AsLong() = gpAnalysis->GetSatScanData()->m_nTracts;
      pRecord->PutFieldValue(uwFieldNumber, fv);

      // precision of case times field
      fv.SetType(pRecord->GetFieldType(++uwFieldNumber));
      fv.AsLong() = gpAnalysis->GetSatScanData()->m_pParameters->m_nPrecision;
      pRecord->PutFieldValue(uwFieldNumber, fv);

      // max geographic extent field
      fv.SetType(pRecord->GetFieldType(++uwFieldNumber));
      fv.AsLong() = gpAnalysis->GetSatScanData()->m_pParameters->m_nMaxGeographicClusterSize;
      pRecord->PutFieldValue(uwFieldNumber, fv);

      // max temporal extent field
      fv.SetType(pRecord->GetFieldType(++uwFieldNumber));
      fv.AsLong() = gpAnalysis->GetSatScanData()->m_pParameters->m_nMaxTemporalClusterSize;
      pRecord->PutFieldValue(uwFieldNumber, fv);

      // time trend adjustment field
      fv.SetType(pRecord->GetFieldType(++uwFieldNumber));
      fv.AsLong() = gpAnalysis->GetSatScanData()->m_pParameters->m_nTimeAdjustType;
      pRecord->PutFieldValue(uwFieldNumber, fv);

      // special grid file used field
      fv.SetType(pRecord->GetFieldType(++uwFieldNumber));
      fv.AsBool() = gpAnalysis->GetSatScanData()->m_pParameters->m_bSpecialGridFile;
      pRecord->PutFieldValue(uwFieldNumber, fv);

      // start date field
      fv.SetType(pRecord->GetFieldType(++uwFieldNumber));
      fv.AsZdString() = gpAnalysis->GetSatScanData()->m_pParameters->m_szStartDate;
      pRecord->PutFieldValue(uwFieldNumber, fv);

      // end date field
      fv.SetType(pRecord->GetFieldType(++uwFieldNumber));
      fv.AsZdString() = gpAnalysis->GetSatScanData()->m_pParameters->m_szEndDate;
      pRecord->PutFieldValue(uwFieldNumber, fv);

      // alive clusters only field
      fv.SetType(pRecord->GetFieldType(++uwFieldNumber));
      fv.AsBool() = gpAnalysis->GetSatScanData()->m_pParameters->m_bAliveClustersOnly;
      pRecord->PutFieldValue(uwFieldNumber, fv);

      // interval units field
      fv.SetType(pRecord->GetFieldType(++uwFieldNumber));
      fv.AsLong() = gpAnalysis->GetSatScanData()->m_pParameters->m_nIntervalUnits;
      pRecord->PutFieldValue(uwFieldNumber, fv);

      // intervals length field
      fv.SetType(pRecord->GetFieldType(++uwFieldNumber));
      fv.AsLong() = gpAnalysis->GetSatScanData()->m_pParameters->m_nIntervalLength;
      pRecord->PutFieldValue(uwFieldNumber, fv);

      // monte carlo  replications field
      fv.SetType(pRecord->GetFieldType(++uwFieldNumber));
      fv.AsLong() = gpAnalysis->GetSatScanData()->m_pParameters->m_nReplicas;
      pRecord->PutFieldValue(uwFieldNumber, fv);

      // 0.01 cutoff field
      fv.SetType(pRecord->GetFieldType(++uwFieldNumber));
      fv.AsDouble() = gpAnalysis->GetSimRatio01();
      pRecord->PutFieldValue(uwFieldNumber, fv);

      // 0.05 cutoff field
      fv.SetType(pRecord->GetFieldType(++uwFieldNumber));
      fv.AsDouble() = gpAnalysis->GetSimRatio05();
      pRecord->PutFieldValue(uwFieldNumber, fv);

      // number of clusters significant at tthe .05 llr cutoff field
      fv.SetType(pRecord->GetFieldType(++uwFieldNumber));
      fv.AsLong() = (long)uwSignificantAt005;
      pRecord->PutFieldValue(uwFieldNumber, fv);

      File.AppendRecord(*pTransaction, *pRecord);
      delete pRecord; pRecord = 0;

      File.EndTransaction(pTransaction); pTransaction = 0;
      File.Close();
   }
   catch(ZdException &x) {
      pTransaction = 0;
      delete pRecord; pRecord = 0;
      delete pLastRecord; pLastRecord = 0;
      x.AddCallpath("OpenRunHistoryFile()", "stsRunHistoryFile");
      throw;
   }
}

// internal setup
void stsRunHistoryFile::Setup(const CAnalysis* pAnalysis, const ZdString& sFileName) {
   try {
//      gsFilename = ANALYSIS_HISTORY_FILE;
      gsFilename = sFileName;
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
      field.second = 8;
      vFieldDescrip.AddElement(field);

      field.first.first = "Run_Time";
      field.first.second = ZD_ALPHA_FLD;
      field.second = 32;
      vFieldDescrip.AddElement(field);

      field.first.first = "Output_File";
      field.first.second = ZD_ALPHA_FLD;
      field.second = 256;
      vFieldDescrip.AddElement(field);

      field.first.first = "Prob_Model";
      field.first.second = ZD_LONG_FLD;
      field.second = 8;
      vFieldDescrip.AddElement(field);

      field.first.first = "Rates";
      field.first.second = ZD_LONG_FLD;
      field.second = 8;
      vFieldDescrip.AddElement(field);

      field.first.first = "Coord_Type";
      field.first.second = ZD_LONG_FLD;
      field.second = 8;
      vFieldDescrip.AddElement(field);

      field.first.first = "Analysis_Type";
      field.first.second = ZD_LONG_FLD;
      field.second = 8;
      vFieldDescrip.AddElement(field);

      field.first.first = "Number_Cases";
      field.first.second = ZD_LONG_FLD;
      field.second = 8;
      vFieldDescrip.AddElement(field);

      field.first.first = "Total_Pop";
      field.first.second = ZD_LONG_FLD;
      field.second = 12;
      vFieldDescrip.AddElement(field);

      field.first.first = "Num_Geo_Areas";
      field.first.second = ZD_LONG_FLD;
      field.second = 8;
      vFieldDescrip.AddElement(field);

      field.first.first = "Precis_Times";
      field.first.second = ZD_LONG_FLD;
      field.second = 8;
      vFieldDescrip.AddElement(field);

      field.first.first = "Max_Geo_Extent";
      field.first.second = ZD_LONG_FLD;
      field.second = 8;
      vFieldDescrip.AddElement(field);

      field.first.first = "Max_Temp_Extent";
      field.first.second = ZD_LONG_FLD;
      field.second = 8;
      vFieldDescrip.AddElement(field);

      field.first.first = "Time_Adjust";
      field.first.second = ZD_LONG_FLD;
      field.second = 8;
      vFieldDescrip.AddElement(field);

      field.first.first = "Grid_File";
      field.first.second = ZD_BOOLEAN_FLD;
      field.second = 1;
      vFieldDescrip.AddElement(field);

      field.first.first = "Start_Date";
      field.first.second = ZD_ALPHA_FLD;
      field.second = 16;
      vFieldDescrip.AddElement(field);

      field.first.first = "End_Date";
      field.first.second = ZD_ALPHA_FLD;
      field.second = 16;
      vFieldDescrip.AddElement(field);

      field.first.first = "Alive_Only";
      field.first.second = ZD_BOOLEAN_FLD;
      field.second = 1;
      vFieldDescrip.AddElement(field);

      field.first.first = "Interv_Units";
      field.first.second = ZD_LONG_FLD;
      field.second = 8;
      vFieldDescrip.AddElement(field);

      field.first.first = "Interv_Len";
      field.first.second = ZD_LONG_FLD;
      field.second = 8;
      vFieldDescrip.AddElement(field);

      field.first.first = "Monte_Carlo";
      field.first.second = ZD_LONG_FLD;
      field.second = 8;
      vFieldDescrip.AddElement(field);

      field.first.first = "001_CutOff";
      field.first.second = ZD_NUMBER_FLD;
      field.second = 8;
      vFieldDescrip.AddElement(field);

      field.first.first = "005_CutOff";
      field.first.second = ZD_NUMBER_FLD;
      field.second = 8;
      vFieldDescrip.AddElement(field);

      field.first.first = "Num_Signif_005";
      field.first.second = ZD_LONG_FLD;
      field.second = 8;
      vFieldDescrip.AddElement(field);
   }
   catch (ZdException &x) {
      x.AddCallpath("SetupFields()", "stsRunHistoryFile");
      throw;
   }
}