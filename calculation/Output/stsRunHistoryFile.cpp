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
// post: will create the csv file with the appropraite fields
void stsRunHistoryFile::CreateRunHistoryFile() {
   ZdField		*pField = 0;
   ZdVector<ZdField*>	vFields;
   TXDFile              File;
   ZdVector<pair<pair<ZdString, char>, long> >  vFieldDescrip;
   unsigned short       uwOffset = 0;

   try {
      SetupFields(vFieldDescrip);
      for(unsigned int i = 0; i < vFieldDescrip.GetNumElements(); ++i) {
         pField = (File.GetNewField());
         pField->SetName(vFieldDescrip[i].first.first.GetCString());
         pField->SetType(vFieldDescrip[i].first.second);
         pField->SetLength(vFieldDescrip[i].second);
         pField->SetOffset(uwOffset);
         uwOffset += vFieldDescrip[i].second;
         vFields.AddElement(pField);
      }

      File.PackFields(vFields);
      File.Create(gsFilename, vFields, ZDIO_OPEN_READ | ZDIO_OPEN_WRITE);
      File.Close();

      CleanupFieldVector(vFields);
   }
   catch (ZdException &x) {   
      CleanupFieldVector(vFields);
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
void stsRunHistoryFile::LogNewHistory(const unsigned short& uwSignificantAt005, BasePrint& PrintDirection) {
   try {
      OpenRunHistoryFile(uwSignificantAt005);
   }
   catch (ZdException &x) {
      x.AddCallpath("LogNewHistory()", "stsRunHistoryFile");
      PrintDirection.SatScanPrintWarning("\nUnable to log analysis information:\n");
      PrintDirection.SatScanPrintWarning(x.GetCallpath());
      PrintDirection.SatScanPrintWarning(x.GetErrorMessage());
      PrintDirection.SatScanPrintWarning("\n");
   }
}

// tries to open the run history file if one exists, if not creates the file
// pre: none
// post: opens/creates the run history file and writes to it
void stsRunHistoryFile::OpenRunHistoryFile(const unsigned short& uwSignificantAt005) {
   ZdTransaction	*pTransaction;
   unsigned long        ulLastRecordNumber;
   unsigned short       uwFieldNumber = 0;
   ZdString             sTempValue;
   ZdFieldValue         fv;
   auto_ptr<ZdFileRecord> pLastRecord, pRecord;
   auto_ptr<TXDFile>    pFile;

   try {
      // if we don't have one then create it
      if(!ZdIO::Exists(gsFilename.GetCString()))
         CreateRunHistoryFile();

      pFile.reset(new TXDFile(gsFilename, ZDIO_OPEN_READ | ZDIO_OPEN_WRITE));

      // get a record buffer, input data and append the record
      pLastRecord.reset(pFile->GetNewRecord());
      ulLastRecordNumber = pFile->GotoLastRecord(&(*pLastRecord));
      // if there's records in the file
      if(ulLastRecordNumber)
         pLastRecord->GetField(0, glRunNumber);

      pTransaction = (pFile->BeginTransaction());

      // note: I'm going to document the heck out of this section in case they can't the run
      // specs on us at any time and that way I can interpret my assumptions in case any just so
      // happen to be incorrect, so bear with me - AJV 9/3/2002
      pRecord.reset(pFile->GetNewRecord());
      //  run number field -- increment the run number so that we have a new unique run number - AJV 9/4/2002
      fv.SetType(pRecord->GetFieldType(uwFieldNumber));
      fv.AsDouble() = ++glRunNumber;
      pRecord->PutFieldValue(uwFieldNumber, fv);

      // run time and date field
      sTempValue << gpAnalysis->GetStartTime();    // hack here because txd files don't like embedded \r or \n AJV
      sTempValue.Replace("\n", "", true);
      sTempValue.Replace("\r", "", true);
      fv.SetType(pRecord->GetFieldType(++uwFieldNumber));
      fv.AsZdString() = sTempValue;
      pRecord->PutFieldValue(uwFieldNumber, fv);

      // output file name field
      sTempValue << ZdString::reset << gpAnalysis->GetSatScanData()->m_pParameters->m_szOutputFilename;      // hack here because txd files don't like embedded \r or \n AJV
      sTempValue.Replace("\r", "", true);
      sTempValue.Replace("\n", "", true);
      fv.SetType(pRecord->GetFieldType(++uwFieldNumber));
      fv.AsZdString() = sTempValue;
      pRecord->PutFieldValue(uwFieldNumber, fv);

      // probability model field
      fv.SetType(pRecord->GetFieldType(++uwFieldNumber));
      switch(gpAnalysis->GetSatScanData()->m_pParameters->m_nModel) {
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
      fv.AsZdString() = sTempValue;
      pRecord->PutFieldValue(uwFieldNumber, fv);

      // rates(high, low or both) field
      fv.SetType(pRecord->GetFieldType(++uwFieldNumber));
      switch (gpAnalysis->GetSatScanData()->m_pParameters->m_nAreas) {
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
      fv.AsZdString() = sTempValue;
      pRecord->PutFieldValue(uwFieldNumber, fv);

      // coordinate type field
      fv.SetType(pRecord->GetFieldType(++uwFieldNumber));
      sTempValue << ZdString::reset << ((gpAnalysis->GetSatScanData()->m_pParameters->m_nCoordType == CARTESIAN) ? "Cartesian" : "LongLat");
      fv.AsZdString() = sTempValue;
      pRecord->PutFieldValue(uwFieldNumber, fv);

      // analysis type field
      fv.SetType(pRecord->GetFieldType(++uwFieldNumber));
      switch(gpAnalysis->GetSatScanData()->m_pParameters->m_nAnalysisType) {
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
      fv.AsZdString() = sTempValue;
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
      switch (gpAnalysis->GetSatScanData()->m_pParameters->m_nPrecision) {
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
      fv.AsZdString() = sTempValue;
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
      switch(gpAnalysis->GetSatScanData()->m_pParameters->m_nTimeAdjustType) {
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
      fv.AsZdString() = sTempValue;
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
      switch (gpAnalysis->GetSatScanData()->m_pParameters->m_nIntervalUnits) {
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
      fv.AsZdString() = sTempValue;
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

      pFile->AppendRecord(*pTransaction, *pRecord);
      pFile->EndTransaction(pTransaction); pTransaction = 0;
      pFile->Close();
   }
   catch(ZdException &x) {
      if(pTransaction)
         pFile->EndTransaction(pTransaction);
      pTransaction = 0;
      pFile->Close();
      x.AddCallpath("OpenRunHistoryFile()", "stsRunHistoryFile");
      throw;
   }
}

// internal setup
void stsRunHistoryFile::Setup(const CAnalysis* pAnalysis, const ZdString& sFileName) {
   try {
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
      field.second = 254;
      vFieldDescrip.AddElement(field);

      field.first.first = "Prob_Model";
      field.first.second = ZD_ALPHA_FLD;
      field.second = 32;
      vFieldDescrip.AddElement(field);

      field.first.first = "Rates";
      field.first.second = ZD_ALPHA_FLD;
      field.second = 16;
      vFieldDescrip.AddElement(field);

      field.first.first = "Coord_Type";
      field.first.second = ZD_ALPHA_FLD;
      field.second = 16;
      vFieldDescrip.AddElement(field);

      field.first.first = "Analysis_Type";
      field.first.second = ZD_ALPHA_FLD;
      field.second = 32;
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
      field.first.second = ZD_ALPHA_FLD;
      field.second = 16;
      vFieldDescrip.AddElement(field);

      field.first.first = "Max_Geo_Extent";
      field.first.second = ZD_LONG_FLD;
      field.second = 8;
      vFieldDescrip.AddElement(field);

      field.first.first = "Max_Temp_Extent";
      field.first.second = ZD_LONG_FLD;
      field.second = 8;
      vFieldDescrip.AddElement(field);

      field.first.first = "Time_Trend_Adjust";
      field.first.second = ZD_ALPHA_FLD;
      field.second = 20;
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
      field.first.second = ZD_ALPHA_FLD;
      field.second = 16;
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

