//---------------------------------------------------------------------------
#include "stsSaTScan.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TfrmAnalysis *frmAnalysis;

//ClassDesc Begin TfrmAnalysis
// This class contains all the main interface controls and relationships.
// Since it the main session interface is a tab dialog, decided to keep
// everything in one class and one cpp.
//ClassDesc End TfrmAnalysis

//---------------------------------------------------------------------------
// Constructor
// If a parameter file is passed it, it will parse it (read it) and set up the interface.
//---------------------------------------------------------------------------
__fastcall TfrmAnalysis::TfrmAnalysis(TComponent* Owner, char *sParamFileName) : TForm(Owner) {
  try {
    Init();

    PageControl1->ActivePage = tbInputFiles;
    gpParams = new CParameters(false);
    if (sParamFileName) {
      gsParamFileName = sParamFileName;
      if (! ReadSession(sParamFileName))
        Close();
    }
    else
      SetupInterface();
  }
  catch (ZdException & x) {
    x.AddCallpath("constructor()", "TfrmAnalysis");
    throw;
  }
}
//---------------------------------------------------------------------------
// Destructor
//---------------------------------------------------------------------------
__fastcall TfrmAnalysis::~TfrmAnalysis() {
  try {
    delete gpParams;
  }
  catch (...){}
}

/** This a temporary hack function that formats date fields to sFormat. This is needed because satscan
    expects dates in human readable form such as '12/08/2002' as apposed to raw data form of 20021208. 
    This function should be removed once the zdfile interface for satscan is implementated! */
void TfrmAnalysis::AttemptFilterDateFields(const char * sFileName, const char * sFormat, unsigned short uwField) {
  ZdFile              * pFile=0;
  ZdTransaction       * pTransaction=0;
  unsigned long         u;
  ZdFieldValue          Value;
  ZdDateFilter          Filter(sFormat);
  char                  sFiltered[1024];

  try {
    pFile = BasisGetToolkit().OpenZdFile(sFileName , ZDIO_OPEN_READ|ZDIO_OPEN_WRITE);
    pTransaction = pFile->BeginTransaction();
    for (u=1; u <= pFile->GetNumRecords(); u++) {
       pFile->GotoRecord(u);
       if (! pFile->GetSystemRecord()->GetIsBlank (uwField)) {
         pFile->GetSystemRecord()->RetrieveFieldValue(uwField, Value);
         try {
           Filter.FilterValue(sFiltered, sizeof(sFiltered), Value.AsCString());
           Value.AsZdString() = sFiltered;
           pFile->GetSystemRecord()->PutFieldValue(uwField, Value);
           pFile->SaveRecord(pTransaction);
         }
         catch (...){}
       }
    }
    if (pTransaction)
      pFile->EndTransaction(pTransaction);
    pTransaction=0;  
    delete pFile; pFile=0;
  }
  catch (ZdException & x) {
    x.AddCallpath("AttemptFilterDateFields()", "TfrmAnalysis");
    if (pTransaction)
      pFile->EndTransaction(pTransaction);
    delete pFile;
    throw;
  }
}

//---------------------------------------------------------------------------
// case file selector
//---------------------------------------------------------------------------
void __fastcall TfrmAnalysis::btnCaseBrowseClick(TObject *Sender) {
  BFTFImportDescriptor  ImportDescriptor;
  ZdFileName            sFileName;
  ZdString              sFileNamePrefix("Case_");
  char                  sBuffer[1024];

  try {
    OpenDialog1->FileName =  "";
    OpenDialog1->DefaultExt = "*.cas";
    OpenDialog1->Filter = "CAS Files (*.cas)|*.cas|DBase Files (*.dbf)|*.dbf|All files (*.*)|*.*";
    OpenDialog1->Title = "Select Case File";
    if (OpenDialog1->Execute()) {
      sFileName = OpenDialog1->FileName.c_str();
      //Detect dbf file and launch importer if detected
      if ( DetermineIfDbfExtension(OpenDialog1->FileName) ) {
         ImportDescriptor.SetGenerateReport(false);
         SetupImportDescriptor(ImportDescriptor, OpenDialog1->FileName.c_str());
         // create destination file in user's temp directory
         GetTempPath(sizeof(sBuffer), sBuffer);
         //GetLongPathName(sBuffer, sBuffer, sizeof(sBuffer));
         sFileName.SetLocation(sBuffer);
         sFileName.SetExtension(TXD_EXT);
         // Prefix filename so that we know this sessions created imported files are unique.
         sFileNamePrefix << sFileName.GetFileName();
         sFileName.SetFileName(sFileNamePrefix);
         ImportDescriptor.SetDestinationFile(sFileName.GetFullPath());
         CreateTXDFile(sFileName, gvCaseFileFieldDescriptors);
         auto_ptr<TBdlgImporter> pImporter(new TBdlgImporter(0, 0, &ImportDescriptor));
         pImporter->ShowOptionalPanels(false, false, false);
         if (pImporter->ShowModal() == mrOk) {
           AttemptFilterDateFields(sFileName.GetFullPath(), "%y/%m/%d", 2);
           strcpy(gpParams->m_szCaseFilename, sFileName.GetFullPath());
           edtCaseFileName->Text = sFileName.GetFullPath();
         }
      }
      else {
        strcpy(gpParams->m_szCaseFilename, sFileName.GetFullPath());
        edtCaseFileName->Text = sFileName.GetFullPath();
      }
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("btnCaseBrowseClick()", "TfrmAnalysis");
    DisplayBasisException(this, x);
  }
}
//---------------------------------------------------------------------------
// Control file selector -- *.ctl files
//---------------------------------------------------------------------------
void __fastcall TfrmAnalysis::btnControlBrowseClick(TObject *Sender) {
  BFTFImportDescriptor  ImportDescriptor;
  ZdFileName            sFileName;
  ZdString              sFileNamePrefix("Control_");
  char                  sBuffer[1024];
  
  try {
    OpenDialog1->FileName = "";
    OpenDialog1->DefaultExt = "*.ctl";
    OpenDialog1->Filter = "CTL Files (*.ctl)|*.ctl|DBase Files (*.dbf)|*.dbf|All files (*.*)|*.*";
    OpenDialog1->Title = "Select Control File";
    if (OpenDialog1->Execute()) {
      sFileName = OpenDialog1->FileName.c_str();
      //Detect dbf file and launch importer if detected
      if ( DetermineIfDbfExtension(OpenDialog1->FileName) ) {
         ImportDescriptor.SetGenerateReport(false);
         SetupImportDescriptor(ImportDescriptor, OpenDialog1->FileName.c_str());
         // create destination file in user's temp directory
         GetTempPath(sizeof(sBuffer), sBuffer);
         //GetLongPathName(sBuffer, sBuffer, sizeof(sBuffer));
         sFileName.SetLocation(sBuffer);
         sFileName.SetExtension(TXD_EXT);
         // Prefix filename so that we know this sessions created imported files are unique.
         sFileNamePrefix << sFileName.GetFileName();
         sFileName.SetFileName(sFileNamePrefix);
         ImportDescriptor.SetDestinationFile(sFileName.GetFullPath());
         CreateTXDFile(sFileName, gvControlFileFieldDescriptors);
         auto_ptr<TBdlgImporter> pImporter(new TBdlgImporter(0, 0, &ImportDescriptor));
         pImporter->ShowOptionalPanels(false, false, false);
         if (pImporter->ShowModal() == mrOk) {
           AttemptFilterDateFields(sFileName.GetFullPath(), "%y/%m/%d", 2);
           strcpy(gpParams->m_szControlFilename, sFileName.GetFullPath());
           edtControlFileName->Text = sFileName.GetFullPath();
         }
      }
      else {
        strcpy(gpParams->m_szControlFilename, sFileName.GetFullPath());
        edtControlFileName->Text = sFileName.GetFullPath();
      }
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("btnControlBrowseClick()", "TfrmAnalysis");
    DisplayBasisException(this, x);
  }
}
//---------------------------------------------------------------------------
// Geographic file selector -- *.geo files
//---------------------------------------------------------------------------
void __fastcall TfrmAnalysis::btnCoordBrowseClick(TObject *Sender) {
  BFTFImportDescriptor  ImportDescriptor;
  ZdFileName            sFileName;
  ZdString              sFileNamePrefix("Geographical_");
  char                  sBuffer[1024];

  try {
    OpenDialog1->FileName = "";
    OpenDialog1->DefaultExt = "*.geo";
    OpenDialog1->Filter = "GEO Files (*.geo)|*.geo|DBase Files (*.dbf)|*.dbf|All files (*.*)|*.*";
    OpenDialog1->Title = "Select Coordinates File";
    if (OpenDialog1->Execute()) {
      sFileName = OpenDialog1->FileName.c_str();
      //Detect dbf file and launch importer if detected
      if ( DetermineIfDbfExtension(OpenDialog1->FileName) ) {
         ImportDescriptor.SetGenerateReport(false);
         SetupImportDescriptor(ImportDescriptor, OpenDialog1->FileName.c_str());
         // create destination file in user's temp directory
         GetTempPath(sizeof(sBuffer), sBuffer);
         //GetLongPathName(sBuffer, sBuffer, sizeof(sBuffer));
         sFileName.SetLocation(sBuffer);
         sFileName.SetExtension(TXD_EXT);
         // Prefix filename so that we know this sessions created imported files are unique.
         sFileNamePrefix << sFileName.GetFileName();
         sFileName.SetFileName(sFileNamePrefix);
         ImportDescriptor.SetDestinationFile(sFileName.GetFullPath());
         CreateTXDFile(sFileName, gvGeoFileFieldDescriptors);
         auto_ptr<TBdlgImporter> pImporter(new TBdlgImporter(0, 0, &ImportDescriptor));
         pImporter->ShowOptionalPanels(false, false, false);
         if (pImporter->ShowModal() == mrOk) {
           strcpy(gpParams->m_szCoordFilename, sFileName.GetFullPath());
           edtCoordinateFileName->Text = sFileName.GetFullPath();
         }
      }
      else {
        strcpy(gpParams->m_szCoordFilename, sFileName.GetFullPath());
        edtCoordinateFileName->Text = sFileName.GetFullPath();
      }
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("btnCoordBrowseClick()", "TfrmAnalysis");
    DisplayBasisException(this, x);
  }
}
//---------------------------------------------------------------------------
//  Grid file selector -- *.grd files
//---------------------------------------------------------------------------
void __fastcall TfrmAnalysis::btnGridBrowseClick(TObject *Sender) {
  BFTFImportDescriptor  ImportDescriptor;
  ZdFileName            sFileName;
  ZdString              sFileNamePrefix("SpecialGrid_");
  char                  sBuffer[1024];

  try {
    OpenDialog1->FileName = "";
    OpenDialog1->DefaultExt = "*.grd";
    OpenDialog1->Filter = "GRD Files (*.grd)|*.grd|DBase Files (*.dbf)|*.dbf|All files (*.*)|*.*";
    OpenDialog1->Title = "Select Special Grid File";
    if (OpenDialog1->Execute()) {
      sFileName = OpenDialog1->FileName.c_str();
      //Detect dbf file and launch importer if detected
      if ( DetermineIfDbfExtension(OpenDialog1->FileName) ) {
         ImportDescriptor.SetGenerateReport(false);
         SetupImportDescriptor(ImportDescriptor, OpenDialog1->FileName.c_str());
         // create destination file in user's temp directory
         GetTempPath(sizeof(sBuffer), sBuffer);
         //GetLongPathName(sBuffer, sBuffer, sizeof(sBuffer));
         sFileName.SetLocation(sBuffer);
         sFileName.SetExtension(TXD_EXT);
         // Prefix filename so that we know this sessions created imported files are unique.
         sFileNamePrefix << sFileName.GetFileName();
         sFileName.SetFileName(sFileNamePrefix);
         ImportDescriptor.SetDestinationFile(sFileName.GetFullPath());
         CreateTXDFile(sFileName, gvGridFileFieldDescriptors);
         auto_ptr<TBdlgImporter> pImporter(new TBdlgImporter(0, 0, &ImportDescriptor));
         pImporter->ShowOptionalPanels(false, false, false);
         if (pImporter->ShowModal() ==mrOk) {
           strcpy(gpParams->m_szGridFilename, sFileName.GetFullPath());
           edtGridFileName->Text = sFileName.GetFullPath();
         }
      }
      else {
        strcpy(gpParams->m_szGridFilename, sFileName.GetFullPath());
        edtGridFileName->Text = sFileName.GetFullPath();
      }  
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("btnGridBrowseClick()", "TfrmAnalysis");
    DisplayBasisException(this, x);
  }
}
//---------------------------------------------------------------------------
// Population file selector -- *.pop files
//---------------------------------------------------------------------------
void __fastcall TfrmAnalysis::btnPopBrowseClick(TObject *Sender) {
  BFTFImportDescriptor  ImportDescriptor;
  ZdFileName            sFileName;
  ZdString              sFileNamePrefix("Population_");
  char                  sBuffer[1024];

  try {
    OpenDialog1->FileName = "";
    OpenDialog1->DefaultExt = "*.pop";
    OpenDialog1->Filter = "POP Files (*.pop)|*.pop|DBase Files (*.dbf)|*.dbf|All files (*.*)|*.*";
    OpenDialog1->Title = "Select Population File";
    if (OpenDialog1->Execute()) {
       sFileName = OpenDialog1->FileName.c_str();
       //Detect dbf file and launch importer if detected
       if ( DetermineIfDbfExtension(OpenDialog1->FileName) ) {
          ImportDescriptor.SetGenerateReport(false);
          SetupImportDescriptor(ImportDescriptor, OpenDialog1->FileName.c_str());
          // create destination file in user's temp directory
          GetTempPath(sizeof(sBuffer), sBuffer);
          //GetLongPathName(sBuffer, sBuffer, sizeof(sBuffer));
          sFileName.SetLocation(sBuffer);
          sFileName.SetExtension(TXD_EXT);
          ImportDescriptor.SetDestinationFile(sFileName.GetFullPath());
          // Prefix filename so that we know this sessions created imported files are unique.
          sFileNamePrefix << sFileName.GetFileName();
          sFileName.SetFileName(sFileNamePrefix);
          CreateTXDFile(sFileName, gvPopFileFieldDescriptors);
          auto_ptr<TBdlgImporter> pImporter(new TBdlgImporter(0, 0, &ImportDescriptor));
          pImporter->ShowOptionalPanels(false, false, false);
          if (pImporter->ShowModal() == mrOk) {
            strcpy(gpParams->m_szPopFilename, sFileName.GetFullPath());
            edtPopFileName->Text = sFileName.GetFullPath();
          }
       }
       else {
         strcpy(gpParams->m_szPopFilename, sFileName.GetFullPath());
         edtPopFileName->Text = sFileName.GetFullPath();
       }
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("btnPopBrowseClick()", "TfrmAnalysis");
    DisplayBasisException(this, x);
  }
}
//------------------------------------------------------------------------------
// Selects a Results file
//------------------------------------------------------------------------------
void __fastcall TfrmAnalysis::btnResultFileBrowseClick(TObject *Sender) {
  try {
    OpenDialog1->FileName = "";
    OpenDialog1->DefaultExt = "*.txt";
    OpenDialog1->Filter = "GRD Files (*.txt)|*.txt|All files (*.*)|*.*";
    OpenDialog1->Title = "Select Results File";
    if (OpenDialog1->Execute()) {
      strcpy(gpParams->m_szOutputFilename, OpenDialog1->FileName.c_str());
      edtResultFile->Text = OpenDialog1->FileName.c_str();
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("btnResultFileBrowseClick()", "TfrmAnalysis");
    DisplayBasisException(this, x);
  }
}
//------------------------------------------------------------------------------
// Generic Days checker -- simulates old interface
//MUST CHECK TO SEE IF THE DAY EDIT BOX IS ENABLED FIRST !!!!
//------------------------------------------------------------------------------
bool TfrmAnalysis::Check_Days(int iYear, int iMonth, int iDay, char *sDateName) {
  bool bDayOk = true;
  char szMessage[100];
  AnsiString sFinalMessage;
  int iMin = 1, iMax;

  try {
    iMax = DaysThisMonth(iYear, iMonth);
    if ((iDay < iMin) || (iDay > iMax)) {
      sFinalMessage += sDateName;
      sprintf(szMessage, ":  Please specify a day between %i and %i.", iMin, iMax);
      sFinalMessage += szMessage;
      Application->MessageBox(sFinalMessage.c_str(), "Parameter Error" , MB_OK);
      bDayOk = false;
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("Check_Days()", "TfrmAnalysis");
    throw;
  }
  return bDayOk;
}
//------------------------------------------------------------------------------
//Ensures that interval length is not less than one.
//------------------------------------------------------------------------------
bool TfrmAnalysis::Check_IntervalLength(int iStartYear, int iStartMonth, int iStartDay,
            int iEndYear, int iEndMonth, int iEndDay, int iIntervalUnits, int iIntervalLength) {
  bool   bIntervalLenOk = true;

  try {
    if (atoi(edtUnitLength->Text.c_str()) < 1) {
      Application->MessageBox("Interval length can not be zero.\nPlease specify an interval length.", "Notification" , MB_OK);
      bIntervalLenOk = false;
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("Check_IntervalLength()", "TfrmAnalysis");
    throw;
  }
   return bIntervalLenOk;
}
//------------------------------------------------------------------------------
// Generic Month checker -- simulates old interface
//------------------------------------------------------------------------------
bool TfrmAnalysis::Check_Month(int iMonth, char *sDateName) {
  bool          bMonthOk = true;
  std::string   sFinalMessage;

  try {
    if ((iMonth < 1) || (iMonth > 12)) {
      sFinalMessage += sDateName;
      sFinalMessage += ":  Please specify an month between 1 and 12.";
      Application->MessageBox(sFinalMessage.c_str(), "Parameter Error" , MB_OK);
      bMonthOk = false;
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("Check_Month()", "TfrmAnalysis");
    throw;
  }
  return bMonthOk;
}
//------------------------------------------------------------------------------
// Simple time trend percentage check.  
//------------------------------------------------------------------------------
bool TfrmAnalysis::Check_TimeTrendPercentage(double dValue) {
  if ( ! (dValue > -100.00)) {
    Application->MessageBox("Invalid time trend percentage specified.", "Parameter Error" , MB_OK);
    return false;
  }
  else
     return true;
}
//------------------------------------------------------------------------------
// Generic Year checker -- simulates old interface
//MUST CHECK TO SEE IF THE DAY EDIT BOX IS ENABLED FIRST !!!!
//------------------------------------------------------------------------------
bool TfrmAnalysis::Check_Year(int iYear, char *sDateName) {
  char          szMessage[100];
  bool          bYearOk = true;
  std::string   sFinalMessage;

  try {
    if ( ! (iYear >= MIN_YEAR) && (iYear <= MAX_YEAR)) {
      sFinalMessage += sDateName;
      sprintf(szMessage, ":  Please specify a year between %i and %i.", MIN_YEAR, MAX_YEAR);
      sFinalMessage += szMessage;
      Application->MessageBox(sFinalMessage.c_str(), "Parameter Error" , MB_OK);
      bYearOk = false;
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("Check_Year", "TfrmAnalysis");
    throw;
  }
  return bYearOk;
}
//------------------------------------------------------------------------------
// Verifies all the parameters on the Analysis Tab
//------------------------------------------------------------------------------
bool TfrmAnalysis::CheckAnalysisParams() {
  bool bParamsOk = true;

  try {
    //if years enabled, then check values...
    //if start year enabled, assume end year enabled.
    if (edtStartYear->Enabled) {
      bParamsOk = Check_Year(atoi(edtStartYear->Text.c_str()),"Study Period Start Year");
      if (bParamsOk)
      bParamsOk = Check_Year(atoi(edtEndYear->Text.c_str()),"Study Period End Year");
    }
    //if Months enabled, then check values...
    //if start month enabled, assume end month enabled.
    if (bParamsOk && edtStartMonth->Enabled) {
      bParamsOk = Check_Month(atoi(edtStartMonth->Text.c_str()), "Study Period Start Month");
      if (bParamsOk)
      bParamsOk = Check_Month(atoi(edtEndMonth->Text.c_str()), "Study Period End Month");
    }
    //if Days enabled, then check values...
    //if start days enabled, assume end days enabled.
    if (bParamsOk && edtStartDay->Enabled) {
      bParamsOk = Check_Days(atoi(edtStartYear->Text.c_str()), atoi(edtStartMonth->Text.c_str()), atoi(edtStartDay->Text.c_str()),"Study Period Start Date");
      if (bParamsOk)
      bParamsOk = Check_Days(atoi(edtEndYear->Text.c_str()), atoi(edtEndMonth->Text.c_str()), atoi(edtEndDay->Text.c_str()),"Study Period End Date");
    }


    if (bParamsOk)
      bParamsOk = CheckDateRange(atoi(edtStartYear->Text.c_str()), atoi(edtStartMonth->Text.c_str()), atoi(edtStartDay->Text.c_str()),
                                 atoi(edtEndYear->Text.c_str()), atoi(edtEndMonth->Text.c_str()), atoi(edtEndDay->Text.c_str()),
                                 gpParams->m_nIntervalUnits, atoi(edtUnitLength->Text.c_str()));
    if (bParamsOk)
      bParamsOk = CheckReplicas(atoi(edtMontCarloReps->Text.c_str()));
  }
  catch (ZdException & x) {
    x.AddCallpath("CheckAnalysisParams()", "TfrmAnalysis");
    throw;
  }
  return bParamsOk;
}
//------------------------------------------------------------------------------
// Verifies the relationship between a start date, end date, and interval length.
//------------------------------------------------------------------------------
bool TfrmAnalysis::CheckDateRange(int iStartYear, int iStartMonth, int iStartDay,
                                  int iEndYear, int iEndMonth, int iEndDay,
                                  int iIntervalUnits, int iIntervalLength) {
  bool          bRangeOk = true;
  ZdString      sErrorMessage;
  ZdDate        StartDate, EndDate;
  ZdDateFilter  DateFilter("%4y/%02m/%02d");
  char          FilterBuffer[11];

  try {
    GetStudyPeriodStartDate(StartDate);
    GetStudyPeriodEndDate(EndDate);

    //check that start date is before end date
    if (StartDate >= EndDate) {
      DateFilter.FilterValue(FilterBuffer, sizeof(FilterBuffer), StartDate.GetRawDate());
      sErrorMessage << "The study period start date of " << FilterBuffer;
      DateFilter.FilterValue(FilterBuffer, sizeof(FilterBuffer), EndDate.GetRawDate());
      sErrorMessage << " does not occur before study period end date of " << FilterBuffer;
      sErrorMessage << ".\nPlease review settings.";
      Application->MessageBox(sErrorMessage.GetCString(), "Notification" , MB_OK);
      PageControl1->ActivePage = tbAnalysis;
      bRangeOk = false;
    }

    if (rgTypeAnalysis->ItemIndex > 0) {/* purely spatial does not use interval length */
      //check that interval length is not greater study period
      //(.i.e. can't have study period that is 20 days and intervals of 3 months)
      //to make start and end day inclusive - add 1 to end date
      EndDate.AddDays(1);
      if (bRangeOk) {
        switch (iIntervalUnits) {
          case      (YEAR)      : StartDate.AddYears(static_cast<unsigned short>(iIntervalLength));
                                  strcpy(FilterBuffer,"year(s)");
                                  break;
          case      (MONTH)     : StartDate.AddMonths(static_cast<unsigned short>(iIntervalLength));
                                  strcpy(FilterBuffer,"month(s)");
                                  break;
          case      (DAY)       : StartDate.AddDays(static_cast<unsigned short>(iIntervalLength));
                                  strcpy(FilterBuffer,"day(s)");
                                  break;
          default               : ZdGenerateException("Unknown interval unit \"%d\"","CheckDateRange()",iIntervalUnits);
        };

        if (StartDate > EndDate) {
          sErrorMessage << "Interval length of " << iIntervalLength << " " << FilterBuffer;
          sErrorMessage << " is greater than study period length.\nPlease review settings.";
          Application->MessageBox(sErrorMessage.GetCString(), "Notification" , MB_OK);
          PageControl1->ActivePage = tbTimeParameter;
          bRangeOk = false;
        }
      }
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("CheckDateRange", "TfrmAnalysis");
    throw;
  }
  return bRangeOk;
}
//------------------------------------------------------------------------------
// Verifies all parameters on the Output tab
//------------------------------------------------------------------------------
bool TfrmAnalysis::CheckOutputParams() {
  bool bReturn = true;

  try {
    if (edtResultFile->Text.Length() == 0)
      ZdException::GenerateNotification("Results File not specified.\nNote that file need not currently exist but path must be valid.",
                                        "CheckOutputParams()");

    try {
      ZdIO(edtResultFile->Text.c_str(), ZDIO_OPEN_WRITE|ZDIO_OPEN_READ|ZDIO_OPEN_CREATE);
    }
    catch (ZdException & x) {
      x.SetErrorMessage((const char*)"Results File: \"%s\" could not be opened or created.\nNote that file need not currently exist but path must be valid.",
                         edtResultFile->Text.c_str());
      x.SetLevel(ZdException::Notify);
      throw;
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("CheckOutputParams", "TfrmAnalysis");
    throw;
  }
  return bReturn;
}
//------------------------------------------------------------------------------
// Specific prospective space-time date check
//  Must be between the start and end dates of the analysis
//------------------------------------------------------------------------------
bool TfrmAnalysis::CheckProspDateRange(int iStartYear, int iStartMonth, int iStartDay,
                                  int iEndYear, int iEndMonth, int iEndDay,
                                  int iProspYear, int iProspMonth, int iProspDay) {
  bool bRangeOk = true;
  Julian Start, End, Prosp;

  try {
    Start = MDYToJulian(iStartMonth, iStartDay, iStartYear);
    End   = MDYToJulian(iEndMonth, iEndDay, iEndYear);
    Prosp = MDYToJulian(iProspMonth, iProspDay, iProspYear);
    if (! Check_Days(iProspYear, iProspMonth, iProspDay, "Start date of Prospective Space-Time")) {
      PageControl1->ActivePage = tbTimeParameter;
      edtProspDay->SetFocus();
      bRangeOk = false;
    }
    else if ((Prosp < Start) || (Prosp > End)) {
      Application->MessageBox("The Start date of Prospective Space-Time must be between the Study Period start and end dates.", "Parameter Error" , MB_OK);
      PageControl1->ActivePage = tbTimeParameter;
      edtProspYear->SetFocus();
      bRangeOk = false;
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("CheckProspDateRange()", "TfrmAnalysis");
    throw;
  }
  return bRangeOk;
}
//------------------------------------------------------------------------------
// Monte Carol Replications must follow a specific numeric selection
//------------------------------------------------------------------------------
bool TfrmAnalysis::CheckReplicas(int iReplicas) {
    if (! ((iReplicas == 0) || (iReplicas == 9) || (iReplicas == 19) || (fmod(iReplicas+1, 1000) == 0.0)) ) {
      Application->MessageBox("Invalid number of replicas specified.", "Parameter Error" , MB_OK);
      return false;
    }
    else
       return true;
}
//------------------------------------------------------------------------------
// Checks the validity of the scanning tab controls
//------------------------------------------------------------------------------
bool TfrmAnalysis::CheckScanningWindowParams() {
  bool bParamsOk = true;

  try {
    //validate maximum spatial cluster size
    bParamsOk = ValidateSpatialClusterSize();

    //validate maximum temporal cluster size
    if (bParamsOk)
      bParamsOk = ValidateTemoralClusterSize();
  }
  catch (ZdException & x) {
    x.AddCallpath("CheckScanningWindowParams()", "TfrmAnalysis");
    throw;
  }
  return bParamsOk;
}
//------------------------------------------------------------------------------
// Checks all the time parameters
//------------------------------------------------------------------------------
bool TfrmAnalysis::CheckTimeParams() {
  bool bParamsOk = true;
  double dValue;

  try {
    if (gpParams->m_nAnalysisType != PURELYSPATIAL) { //not purely spacial    use to be 0
      if (edtUnitLength->Enabled)
         bParamsOk = Check_IntervalLength(atoi(edtStartYear->Text.c_str()), atoi(edtStartMonth->Text.c_str()),
                                          atoi(edtStartDay->Text.c_str()),  atoi(edtEndYear->Text.c_str()),
                                          atoi(edtEndMonth->Text.c_str()), atoi(edtEndDay->Text.c_str()),
                                          gpParams->m_nIntervalUnits, atoi(edtUnitLength->Text.c_str()));
      if (bParamsOk && edtLogPerYear->Enabled) {
        if (edtLogPerYear->Text.IsEmpty()) {
          bParamsOk = false;
          Application->MessageBox("Please enter a number in time trend percentage.", "Parameter Error" , MB_OK);
          PageControl1->ActivePage = tbTimeParameter;
        }
        else {
          dValue = atof(edtLogPerYear->Text.c_str());
          bParamsOk = Check_TimeTrendPercentage(dValue);
        }
      }
      //just need to check if the Prospective year is enabled.
      // if year is enabled, then all others are too...
      if (bParamsOk && edtProspYear->Enabled) {
         bParamsOk = CheckProspDateRange(atoi(edtStartYear->Text.c_str()), atoi(edtStartMonth->Text.c_str()),
                                         atoi(edtStartDay->Text.c_str()), atoi(edtEndYear->Text.c_str()),
                                         atoi(edtEndMonth->Text.c_str()), atoi(edtEndDay->Text.c_str()),
                                         atoi(edtProspYear->Text.c_str()), atoi(edtProspMonth->Text.c_str()),
                                         atoi(edtProspDay->Text.c_str()));
      }
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("CheckTimeParams()", "TfrmAnalysis");
    throw;
  }
  return bParamsOk;
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void __fastcall TfrmAnalysis::chkCensusAreasReportedClustersAsciiClick(TObject *Sender) {
   gpParams->m_bOutputCensusAreas = chkCensusAreasReportedClustersAscii->Checked;
}
//------------------------------------------------------------------------------
// Include relative risks in output
//------------------------------------------------------------------------------
void __fastcall TfrmAnalysis::chkRelativeRiskEstimatesAreaAsciiClick(TObject *Sender) {
//    gpParams->m_bOutputRelRisks = chkRelativeRiskEstimatesAreaAscii->Checked;
}
//------------------------------------------------------------------------------
// Include Purely Spacial Clusters selection control
//------------------------------------------------------------------------------
void __fastcall TfrmAnalysis::chkIncludePurSpacClustClick(TObject *Sender) {
    gpParams->m_bIncludePurelySpatial = chkIncludePurSpacClust->Checked;
}
//------------------------------------------------------------------------------
// Include Purely Temporal Clusters selection control
//------------------------------------------------------------------------------
void __fastcall TfrmAnalysis::chkInclPurTempClustClick(TObject *Sender) {
    gpParams->m_bIncludePurelyTemporal = chkInclPurTempClust->Checked;
}
//------------------------------------------------------------------------------
// Simulated Log likelihood ratio set
//------------------------------------------------------------------------------
void __fastcall TfrmAnalysis::chkSimulatedLogLikelihoodRatiosAsciiClick(TObject *Sender) {
    gpParams->m_bSaveSimLogLikelihoods = chkSimulatedLogLikelihoodRatiosAscii->Checked;
}

void __fastcall TfrmAnalysis::chkClustersInColumnFormatAsciiClick(TObject *Sender) {
   gpParams->m_bMostLikelyClusters = chkClustersInColumnFormatAscii->Checked;
}

// create the TXD file with the appropriate field names - AJV 8/29/2002
// pre: sFilename has a txd extension and vFieldNames has been filled with the appropraite field names
// post: will create a txd file with padded spaces delimiting the fields
void TfrmAnalysis::CreateTXDFile(const ZdFileName& sFileName, const std::vector<std::string>& vFieldNames) {
   ZdPointerVector<ZdField>	vFields;
   ZdField		        *pField = 0;
   TXDFile                      File;
   unsigned short               uwOffset = 0, uwLength;

   try {
      // creates the field vector from the provided field names
      for(size_t i = 0; i < vFieldNames.size(); ++i) {
      	 pField = File.GetNewField();
         pField->SetName(vFieldNames[i].c_str());
         if(i == 0)     // need a better system here to define which fields are required
            pField->SetRequired(true);
          // field 1 the only alpha field in the input so allow to greater width here, consider
          // other options for this in the future - AJV 9/4/2002
         pField->SetType(ZD_ALPHA_FLD);
         uwLength = (!i ? 200 : 50);
         pField->SetOffset(uwOffset);
         pField->SetLength(uwLength);
         pField->SetIndexCount(0);
         vFields.AddElement(pField);
         // NOTE: Our original design would simply make gaps in the fields offsets
         //       to allow created ZdTXD file to work like current SaTScan data files.
         //       But, unfortunetly, those gaps would have value 0x00 causing problems
         //       for scanf. So, let's assume that there will be gaps between fields.
         //       I think this is a pretty good assumption for now considering the
         //       large field lengths we are defining and given sample data observed.
         uwOffset += ( uwLength );
      }

      File.Delete(sFileName.GetFullPath());
      File.SetTitle(sFileName.GetFileName());
      File.Create(sFileName.GetFullPath(), vFields, 0);
      File.Close();
   }
   catch (ZdException &x) {
      delete pField; pField = 0;
      x.AddCallpath("CreateTXDFile()", "TfrmAnalysis");
      throw;
   }
}

bool TfrmAnalysis::DetermineIfDbfExtension(const AnsiString& sFileName) {
  bool bDbfStatus = false;

  try {
     bDbfStatus = !strcmp(((sFileName.SubString(sFileName.Length() - 3, 4)).LowerCase()).c_str(), ".dbf");
  }
  catch (ZdException & x) {
    x.AddCallpath("DetermineIfDbaseFile(AnsiString & sFileName)", "TfrmAnalysis");
    throw;
  }
   return bDbfStatus;
}

void __fastcall TfrmAnalysis::edtEndDayExit(TObject *Sender) {
  try {
    if ((atoi(edtEndDay->Text.c_str()) < 1) || (atoi(edtEndDay->Text.c_str()) > 31)) {
      PageControl1->ActivePage = tbAnalysis;
      edtEndDay->SetFocus();
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("edtEndDayExit()", "TfrmAnalysis");
    DisplayBasisException(this, x);
  }
}
//---------------------------------------------------------------------------
void __fastcall TfrmAnalysis::edtEndMonthExit(TObject *Sender) {
  try {
    if ( ! Check_Month(atoi(edtEndMonth->Text.c_str()), "Study Period End Month")) {
      PageControl1->ActivePage = tbAnalysis;
      edtStartMonth->SetFocus();
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("edtEndMonthExit", "TfrmAnalysis");
    DisplayBasisException(this, x);
  }
}
//---------------------------------------------------------------------------
void __fastcall TfrmAnalysis::edtEndYearExit(TObject *Sender) {
  try {
    if ( ! Check_Year(atoi(edtEndYear->Text.c_str()), "Study Period End Year")) {
      PageControl1->ActivePage = tbAnalysis;
      edtEndYear->SetFocus();
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("edtEndYearExit()", "TfrmAnalysis");
    DisplayBasisException(this, x);
  }
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void __fastcall TfrmAnalysis::edtLogPerYearExit(TObject *Sender) {
  double dValue = atof(edtLogPerYear->Text.c_str());
  if (edtLogPerYear->Text.IsEmpty()) {
    Application->MessageBox("The percentage per year value can not be blank.", "Parameter Error" , MB_OK);
    PageControl1->ActivePage = tbTimeParameter;
    edtLogPerYear->SetFocus();
  }
  else if (!Check_TimeTrendPercentage(dValue)) {
    PageControl1->ActivePage = tbTimeParameter;
    edtLogPerYear->SetFocus();
  }
}
//------------------------------------------------------------------------------
// Validates value entered for Cluster size
//------------------------------------------------------------------------------
void __fastcall TfrmAnalysis::edtMaxClusterSizeExit(TObject *Sender) {
  try {
    if (!edtMaxClusterSize->Text.Length() || atof(edtMaxClusterSize->Text.c_str()) == 0)
      ZdException::GenerateNotification("Please specify a maximum spatial cluster size.","edtMaxClusterSizeExit()");
    else
      gpParams->m_nMaxGeographicClusterSize = atof(edtMaxClusterSize->Text.c_str());
  }
  catch (ZdException & x) {
    x.AddCallpath("edtMaxClusterSizeExit()", "TfrmAnalysis");
    edtMaxClusterSize->SetFocus();
    DisplayBasisException(this, x);
  }     
}
//------------------------------------------------------------------------------
// Validates Number of Monte Carlo reps value
//------------------------------------------------------------------------------
void __fastcall TfrmAnalysis::edtMontCarloRepsExit(TObject *Sender) {
  try {
     if (! CheckReplicas(atoi(edtMontCarloReps->Text.c_str()))) {
       PageControl1->ActivePage = tbAnalysis;
       edtMontCarloReps->SetFocus();
     }
  }
  catch (ZdException & x) {
    x.AddCallpath("edtMontCarloRepsExit()", "TfrmAnalysis");
    DisplayBasisException(this, x);
  }
}
//------------------------------------------------------------------------------
//  Validates value entered for Maximum Temporal Cluster Size
//------------------------------------------------------------------------------
void __fastcall TfrmAnalysis::edtMaxTemporalClusterSizeExit(TObject *Sender) {
  try {
    if (!edtMaxTemporalClusterSize->Text.Length() || atof(edtMaxTemporalClusterSize->Text.c_str()) == 0)
      ZdException::GenerateNotification("Please specify a maximum temporal cluster size.","edtMaxTemporalClusterSizeExit()");
    else
      gpParams->m_nMaxTemporalClusterSize = atof(edtMaxTemporalClusterSize->Text.c_str());
  }
  catch (ZdException & x) {
    x.AddCallpath("edtMaxTemporalClusterSizeExit()", "TfrmAnalysis");
    edtMaxTemporalClusterSize->SetFocus();
    DisplayBasisException(this, x);
  }     
}
//---------------------------------------------------------------------------
void __fastcall TfrmAnalysis::edtProspDayExit(TObject *Sender) {
  try {
    if ((atoi(edtProspDay->Text.c_str()) < 1) || (atoi(edtProspDay->Text.c_str()) > 31)) {
      Application->MessageBox("Please specify a valid day.", "Parameter Error" , MB_OK);
      PageControl1->ActivePage = tbTimeParameter;
      edtProspDay->SetFocus();
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("edtProspDayExit()", "TfrmAnalysis");
    DisplayBasisException(this, x);
  }
}
//---------------------------------------------------------------------------
void __fastcall TfrmAnalysis::edtProspYearExit(TObject *Sender) {
  try {
    if ( ! Check_Year(atoi(edtProspYear->Text.c_str()), "Prospective Space-Time Start Year")) {
      PageControl1->ActivePage = tbTimeParameter;
      edtProspYear->SetFocus();
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("edtProspYearExit()", "TfrmAnalysis");
    DisplayBasisException(this, x);
  }
}
//---------------------------------------------------------------------------
void __fastcall TfrmAnalysis::edtProspMonthExit(TObject *Sender) {
  try {
    if ( ! Check_Month(atoi(edtProspMonth->Text.c_str()), "Prospective Space-Time Start Month")) {
      PageControl1->ActivePage = tbTimeParameter;
      edtProspMonth->SetFocus();
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("edtProspMonthExit()", "TfrmAnalysis");
    DisplayBasisException(this, x);
  }
}
//---------------------------------------------------------------------------
void __fastcall TfrmAnalysis::edtStartDayExit(TObject *Sender) {
  try {
    if ((atoi(edtStartDay->Text.c_str()) < 1) || (atoi(edtStartDay->Text.c_str()) > 31)) {
      Application->MessageBox("Please specify a valid day.", "Parameter Error" , MB_OK);
      PageControl1->ActivePage = tbAnalysis;
      edtStartDay->SetFocus();
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("edtStartDayExit()", "TfrmAnalysis");
    DisplayBasisException(this, x);
  }
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void __fastcall TfrmAnalysis::edtStartMonthExit(TObject *Sender) {
  try {
    if ( ! Check_Month(atoi(edtStartMonth->Text.c_str()), "Study Period Start Month")) {
      PageControl1->ActivePage = tbAnalysis;
      edtStartMonth->SetFocus();
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("edtStartMonthExit()", "TfrmAnalysis");
    DisplayBasisException(this, x);
  }
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void __fastcall TfrmAnalysis::edtStartYearExit(TObject *Sender) {
  try {
    if ( ! Check_Year(atoi(edtStartYear->Text.c_str()), "Study Period Start Year")) {
      PageControl1->ActivePage = tbAnalysis;
      edtStartYear->SetFocus();
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("edtStartYearExit()", "TfrmAnalysis");
    DisplayBasisException(this, x);
  }
}
//---------------------------------------------------------------------------
void __fastcall TfrmAnalysis::edtUnitLengthExit(TObject *Sender) {
  bool bParamsOk;
  bParamsOk = Check_IntervalLength(atoi(edtStartYear->Text.c_str()), atoi(edtStartMonth->Text.c_str()),
                                   atoi(edtStartDay->Text.c_str()),  atoi(edtEndYear->Text.c_str()),
                                   atoi(edtEndMonth->Text.c_str()), atoi(edtEndDay->Text.c_str()),
                                   gpParams->m_nIntervalUnits, atoi(edtUnitLength->Text.c_str()));

  if (! bParamsOk) {
    PageControl1->ActivePage = tbTimeParameter;
    edtUnitLength->SetFocus();
  }
}

// enables or disables the PST start date control
void TfrmAnalysis::EnablePSTDate(bool bEnable) {
   GroupBox8->Enabled = bEnable;
   edtProspYear->Enabled = bEnable;
   edtProspMonth->Enabled = bEnable;
   edtProspDay->Enabled = bEnable;
   edtProspYear->Color =  bEnable ? clWindow : clInactiveBorder;
   edtProspMonth->Color = bEnable ? clWindow : clInactiveBorder;
   edtProspDay->Color =  bEnable ? clWindow : clInactiveBorder;
}

// enables or disables the spatial control
void TfrmAnalysis::EnableSpatial(bool bEnable, bool bEnableCheckbox, bool bEnableSpatialPercentage) {
   rdoSpatialPercentage->Enabled = bEnableSpatialPercentage;
   rdoSpatialPercentage->Checked = (gpParams->m_nMaxSpatialClusterSizeType == PERCENTAGEOFMEASURETYPE && bEnableSpatialPercentage) ? true : false;
   rdoSpatialDistance->Enabled = bEnable;
   rdoSpatialDistance->Checked = (gpParams->m_nMaxSpatialClusterSizeType == PERCENTAGEOFMEASURETYPE && bEnableSpatialPercentage) ? false : true;
   chkInclPurTempClust->Enabled = bEnableCheckbox;
   chkInclPurTempClust->Checked = (bEnableCheckbox && gpParams->m_bIncludePurelyTemporal);
   edtMaxClusterSize->Enabled = bEnable;
   edtMaxClusterSize->Color = bEnable ? clWindow : clInactiveBorder;
}

// enables or disables the temporal control
void TfrmAnalysis::EnableTemporal(bool bEnable, bool bEnableCheckbox, bool bEnablePercentage) {
  GroupBox5->Enabled = bEnable;
  rdoPercentageTemproal->Enabled = bEnablePercentage;
  rdoPercentageTemproal->Checked = (gpParams->m_nMaxClusterSizeType == PERCENTAGETYPE && bEnablePercentage) ? true : false;
  rdoTimeTemproal->Enabled = bEnable;
  rdoTimeTemproal->Checked = (gpParams->m_nMaxClusterSizeType == PERCENTAGETYPE && bEnablePercentage) ? false : true;
  chkIncludePurSpacClust->Enabled = bEnableCheckbox;
  chkIncludePurSpacClust->Checked = (bEnableCheckbox && gpParams->m_bIncludePurelySpatial);
  edtMaxTemporalClusterSize->Enabled = bEnable;
  edtMaxTemporalClusterSize->Color = bEnable ? clWindow : clInactiveBorder;
}

// enables or disables the time interval control
void TfrmAnalysis::EnableTimeInterval(bool bEnable) {
   GroupBox6->Enabled = bEnable;
   rbUnitDay->Enabled = bEnable;
   rbUnitMonths->Enabled = bEnable;
   rbUnitYear->Enabled = bEnable;
   edtUnitLength->Enabled = bEnable;
   edtUnitLength->Color = bEnable ? clWindow : clInactiveBorder;
}

//---------------------------------------------------------------------------
void __fastcall TfrmAnalysis::FloatKeyPress(TObject *Sender, char &Key) {
  if (!strchr("-0123456789.\b",Key))
    Key = 0;
}

//---------------------------------------------------------------------------
// The form is closed and all allocated memory for the form is freed.
//---------------------------------------------------------------------------
void __fastcall TfrmAnalysis::FormClose(TObject *Sender, TCloseAction &Action) {
  Action = caFree;
}

//---------------------------------------------------------------------------
// Returns parameter filename fullpath.
//---------------------------------------------------------------------------
char * TfrmAnalysis::GetFileName() {
  return gsParamFileName.c_str();
}
 
//---------------------------------------------------------------------------
// returns the class global gpParams
//---------------------------------------------------------------------------
CParameters * TfrmAnalysis::GetSession() {
  try {
    SaveTextParameters();
  }
  catch (ZdException & x) {
    x.AddCallpath("GetSession()", "TfrmAnalysis");
    throw;
  }
  return gpParams;
}

/** Sets passed ZdDate to study period end date as defined by TEditBoxes.*/
ZdDate & TfrmAnalysis::GetStudyPeriodEndDate(ZdDate & Date) {
  try {
    Date.SetYear(static_cast<unsigned short>(atoi(edtEndYear->Text.c_str())));
    Date.SetMonth(static_cast<unsigned short>(atoi(edtEndMonth->Text.c_str())));
    Date.SetDay(static_cast<unsigned short>(atoi(edtEndDay->Text.c_str())));
  }
  catch (ZdException & x) {
    x.AddCallpath("GetStudyPeriodEndDate()", "TfrmAnalysis");
    throw;
  }
  return Date;
}

/** Sets passed ZdDate to study period start date as defined by TEditBoxes.*/
ZdDate & TfrmAnalysis::GetStudyPeriodStartDate(ZdDate & Date) {
  try {
    Date.SetYear(static_cast<unsigned short>(atoi(edtStartYear->Text.c_str())));
    Date.SetMonth(static_cast<unsigned short>(atoi(edtStartMonth->Text.c_str())));
    Date.SetDay(static_cast<unsigned short>(atoi(edtStartDay->Text.c_str())));
  }
  catch (ZdException & x) {
    x.AddCallpath("GetStudyPeriodStartDate()", "TfrmAnalysis");
    throw;
  }
  return Date;
}

// global initializations
void TfrmAnalysis::Init() {
   gpParams=0;
   cboCriteriaSecClusters->ItemIndex = 0;
   SetupGeoFileFieldDescriptors();
   SetupCaseFileFieldDescriptors();
   SetupControlFileFieldDescriptors();
   SetupGridFileFieldDescriptors();
   SetupPopFileFieldDescriptors();
}

//------------------------------------------------------------------------------
void __fastcall TfrmAnalysis::mitClearSpecialGridEditClick(TObject *Sender) {
  edtGridFileName->Clear();
  strcpy(gpParams->m_szGridFilename, "");
  gpParams->m_bSpecialGridFile = false;
}
//---------------------------------------------------------------------------
void __fastcall TfrmAnalysis::NaturalNumberKeyPress(TObject *Sender, char &Key) {
  if (!strchr("0123456789\b",Key))
    Key = 0;
}

//------------------------------------------------------------------------------
// Type of Analsyis is changed - this is the major affector of all other controls
// this does all enabling and disabling of other controls with the exception of the
// model control which enables and disables this control's options and the precision
// time control which sync's the precision between time controls
//------------------------------------------------------------------------------
void TfrmAnalysis::OnAnalysisTypeClick() {
   try {
      gpParams->m_nAnalysisType = rgTypeAnalysis->ItemIndex + 1;

    switch (rgTypeAnalysis->ItemIndex) {
       case 0:                     // purely spatial
          // enable None case precision
          rgPrecisionTimes->Controls[0]->Enabled = true;
          // disable time trend adjustment
          rgTemporalTrendAdj->Enabled = false;
          edtLogPerYear->Enabled = false;
          // disable clusters to include
          rgClustersToInclude->Enabled = false;
          // enable spatial but not checkbox
          EnableSpatial(true, false, true);
          // disable time intervals
          EnableTimeInterval(false);
          // disable temporal
          EnableTemporal(false, false, false);
          // disable start date PST
          EnablePSTDate(false);
          break;
       case 1:                     // purely temporal
          // disable None option in case precision time
          rgPrecisionTimes->Controls[0]->Enabled = false;
          // Enables Time Trend Adjust without Non-Param
          rgTemporalTrendAdj->Enabled = gpParams->m_nModel == POISSON ? true : false;
          edtLogPerYear->Enabled = (gpParams->m_nModel == POISSON && rgTemporalTrendAdj->ItemIndex == 2) ? true : false;
          rgTemporalTrendAdj->Controls[1]->Enabled = false;
          rgTemporalTrendAdj->ItemIndex = ((rgTemporalTrendAdj->ItemIndex != 1) ? gpParams->m_nTimeAdjustType : 0 );
          // Enables Clusters to include
          rgClustersToInclude->Enabled = true;
          rgClustersToInclude->ItemIndex = (gpParams->m_bAliveClustersOnly ? 1 : 0);
          // Disables Spatial
          EnableSpatial(false, false, false);
          // Enables time intervals
          EnableTimeInterval(true);
          // Enables temporal without checkbox
          EnableTemporal(true, false, true);
          // Disables Start date PST
          EnablePSTDate(false);
          break;
       case 2:                     // retrospective space-time
          // disable None option in case precision time
          rgPrecisionTimes->Controls[0]->Enabled = false;
          //Enables Time Trend Adjust
          rgTemporalTrendAdj->Enabled = gpParams->m_nModel == POISSON ? true : false;
          rgTemporalTrendAdj->Controls[1]->Enabled = gpParams->m_nModel == POISSON ? true : false;
          edtLogPerYear->Enabled = (gpParams->m_nModel == POISSON && rgTemporalTrendAdj->ItemIndex == 2) ? true : false;
          rgTemporalTrendAdj->ItemIndex = gpParams->m_nTimeAdjustType;
          //Enables clusters to include
          rgClustersToInclude->Enabled = true;
          rgClustersToInclude->ItemIndex = (gpParams->m_bAliveClustersOnly ? 1 : 0);
          //Enables spatial
          EnableSpatial(true, !(gpParams->m_nModel == 2), true);
          //Enables time intervals
          EnableTimeInterval(true);
          //Enables temporal
          EnableTemporal(true,!(gpParams->m_nModel == 2), true);
          //Disables Start date PST
          EnablePSTDate(false);
          break;
       case 3:                     // prospective space-time
          rgPrecisionTimes->Controls[0]->Enabled = false;
          //Enables Time Trend Adjust
          rgTemporalTrendAdj->Enabled = gpParams->m_nModel == POISSON ? true : false;
          rgTemporalTrendAdj->Controls[1]->Enabled = gpParams->m_nModel == POISSON ? true : false;
          edtLogPerYear->Enabled = (gpParams->m_nModel == POISSON && rgTemporalTrendAdj->ItemIndex == 2) ? true : false;
          rgTemporalTrendAdj->ItemIndex = gpParams->m_nTimeAdjustType;
          //disables clusters to include
          rgClustersToInclude->Enabled = false;
          //Enables Spatial % box disable
          EnableSpatial(true, !(gpParams->m_nModel == 2), false);
          //Enables time intervals
          EnableTimeInterval(true);
          //Enables temporal with checkbox but disable % option radio button
          EnableTemporal(true, !(gpParams->m_nModel == 2), false);
          //Enables Start Date PST
          EnablePSTDate(true);
          break;
    }
    OnPrecisionTimesClick();

    // if none not enabled and set, then set to case precision Year instead - AJV 10/4/2002
    if(rgPrecisionTimes->ItemIndex == 0 && !(rgPrecisionTimes->Controls[0]->Enabled))
       rgPrecisionTimes->ItemIndex = 1;
   }
   catch (ZdException &x) {
      x.AddCallpath("OnAnalysisTypeClick()", "TfrmAnalysis");
      throw;
   }
}

//---------------------------------------------------------------------------
//  When the time precision control is changed, various interace options are
//  toggled and changed.
void TfrmAnalysis::OnPrecisionTimesClick() {
   try {
      gpParams->m_nPrecision = rgPrecisionTimes->ItemIndex;
    switch (rgPrecisionTimes->ItemIndex) {
      case NONE : edtStartYear->Enabled  = true;  edtStartYear->Color = clWindow;
                  edtStartMonth->Enabled = true;  edtStartMonth->Color = clWindow;
                  edtStartDay->Enabled   = true;  edtStartDay->Color = clWindow;
                  edtEndYear->Enabled    = true;  edtEndYear->Color = clWindow;
                  edtEndMonth->Enabled   = true;  edtEndMonth->Color = clWindow;
                  edtEndDay->Enabled     = true;  edtEndDay->Color = clWindow;
                  // Time Interval disabled
                  GroupBox6->Enabled = false;
                  break;
      case YEAR   : //Study Period same precision
                  edtStartYear->Enabled  = true;   edtStartYear->Color = clWindow;
                  edtStartMonth->Enabled = false;  edtStartMonth->Color = clInactiveBorder;
                  edtStartDay->Enabled   = false;  edtStartDay->Color = clInactiveBorder;
                  edtEndYear->Enabled    = true;   edtEndYear->Color = clWindow;
                  edtEndMonth->Enabled   = false;  edtEndMonth->Color = clInactiveBorder;
                  edtEndDay->Enabled     = false;  edtEndDay->Color = clInactiveBorder;
                  edtStartMonth->Text="1";
                  edtStartDay->Text = "1";
                  edtEndMonth->Text = "12";
                  edtEndDay->Text = "31";
                  break;
      case MONTH  :  //Study Period same precision
                  edtStartYear->Enabled  = true;   edtStartYear->Color = clWindow;
                  edtStartMonth->Enabled = true;   edtStartMonth->Color = clWindow;
                  edtStartDay->Enabled   = false;  edtStartDay->Color = clInactiveBorder;
                  edtEndYear->Enabled    = true;   edtEndYear->Color = clWindow;
                  edtEndMonth->Enabled   = true;   edtEndMonth->Color = clWindow;
                  edtEndDay->Enabled     = false;  edtEndDay->Color = clInactiveBorder;
                  edtEndDay->Text = DaysThisMonth(atoi(edtEndYear->Text.c_str()), atoi(edtEndMonth->Text.c_str()));
                  edtStartDay->Text = "1";
                  break;
      case DAY    : //Study Period same precision
                  edtStartYear->Enabled  = true;  edtStartYear->Color = clWindow;
                  edtStartMonth->Enabled = true;  edtStartMonth->Color = clWindow;
                  edtStartDay->Enabled   = true;  edtStartDay->Color = clWindow;
                  edtEndYear->Enabled    = true;  edtEndYear->Color = clWindow;
                  edtEndMonth->Enabled   = true;  edtEndMonth->Color = clWindow;
                  edtEndDay->Enabled     = true;  edtEndDay->Color = clWindow;
                  break;
    }

    //Time intervals same or greater precision enabled if enabled
    if (GroupBox6->Enabled) {
      rbUnitYear->Enabled = rgPrecisionTimes->ItemIndex == DAY || rgPrecisionTimes->ItemIndex == MONTH || rgPrecisionTimes->ItemIndex == YEAR;
      rbUnitMonths->Enabled = rgPrecisionTimes->ItemIndex == DAY || rgPrecisionTimes->ItemIndex == MONTH;
      rbUnitDay->Enabled = rgPrecisionTimes->ItemIndex == DAY;
      switch (gpParams->m_nPrecision) {
         case 1  : rbUnitYear->Checked = true;
                   rbUnitMonths->Checked = false;
                   rbUnitDay->Checked = false;
                   break;
         case 2  : if (! rbUnitYear->Checked && ! rbUnitMonths->Checked)
                     rbUnitMonths->Checked = true;
                   rbUnitDay->Checked = false;
                   break;
         case 3  : if (!rbUnitYear->Checked && !rbUnitMonths->Checked && !rbUnitDay->Checked)
                     rbUnitDay->Checked = true;
                   break;
         default : if (!rbUnitYear->Checked && !rbUnitMonths->Checked && !rbUnitDay->Checked)
                     rbUnitYear->Checked = true;
      };
    }

    // prospective year group box
    if(gpParams->m_nAnalysisType == PROSPECTIVESPACETIME) {
       edtProspYear->Enabled = (rgPrecisionTimes->ItemIndex == DAY || rgPrecisionTimes->ItemIndex == MONTH || rgPrecisionTimes->ItemIndex == YEAR);
       edtProspMonth->Enabled = (rgPrecisionTimes->ItemIndex == DAY || rgPrecisionTimes->ItemIndex == MONTH);
       edtProspDay->Enabled = rgPrecisionTimes->ItemIndex == DAY;

       edtProspYear->Color = edtProspYear->Enabled ? clWindow : clInactiveBorder;
       edtProspMonth->Color = edtProspMonth->Enabled ? clWindow : clInactiveBorder;
       edtProspDay->Color = edtProspDay->Enabled ? clWindow : clInactiveBorder;
    }

    // this is the OnPrecisionChange() function
    if (gpParams->m_nAnalysisType == PURELYSPATIAL && gpParams->m_nPrecision != 0) {  // use to be m_nAnalysisType == 0
      if (gpParams->m_nIntervalUnits > gpParams->m_nPrecision )
        gpParams->m_nIntervalUnits = gpParams->m_nPrecision;
    }
   }
   catch(ZdException &x) {
      x.AddCallpath("OnPrecisionTimesClick()", "TfrmAnalysis");
      throw;
   }
}

//------------------------------------------------------------------------------
// Probability is changed which should only affect Analysis Type  - AJV 10/04/2002
//------------------------------------------------------------------------------
void TfrmAnalysis::OnProbabilityModelClick() {
   try {
      gpParams->m_nModel = rgProbability->ItemIndex;
     //enable buttons based on selected model
     switch (rgProbability->ItemIndex) {
        case 0:  // drop through, same as 1
        case 1:
           for(int i = 0; i <= 3; ++i)
              rgTypeAnalysis->Controls[i]->Enabled = true;
           chkRelativeRiskEstimatesAreaAscii->Enabled = true;
           rdoPercentageTemproal->Caption = "Percent of Study Period (<= 90%)";
           break;
        case 2:
           rgTypeAnalysis->Controls[0]->Enabled = false;
           rgTypeAnalysis->Controls[1]->Enabled = false;
           rgTypeAnalysis->Controls[2]->Enabled = true;
           rgTypeAnalysis->Controls[3]->Enabled = true;
           if(rgTypeAnalysis->ItemIndex == 0 || rgTypeAnalysis->ItemIndex == 1)
              rgTypeAnalysis->ItemIndex = 2;
           chkRelativeRiskEstimatesAreaAscii->Enabled = false;
           rdoPercentageTemproal->Caption = "Percent of Study Period (<= 50%)";
           break;
     }
     OnAnalysisTypeClick();
   }
   catch (ZdException &x) {
      x.AddCallpath("OnProbablityModelClick()", "TfrmAnalysis");
      throw;
   }
}

//------------------------------------------------------------------------------
// Temporal Time Trend control
//------------------------------------------------------------------------------
void TfrmAnalysis::OnTemporalTrendClick() {
   try {
      gpParams->m_nTimeAdjustType = rgTemporalTrendAdj->ItemIndex;
      switch (rgTemporalTrendAdj->ItemIndex) {
       case 0:                             // none
         edtLogPerYear->Enabled = false;
         edtLogPerYear->Color = clInactiveBorder;
         break;
       case 1:                             // non-parametric
         edtLogPerYear->Enabled = false;
         edtLogPerYear->Color = clInactiveBorder;
         break;
       case 2:                             // log linear
         edtLogPerYear->Enabled = true;
         edtLogPerYear->Color = clWindow;
         break;
      }
   }
   catch (ZdException &x) {
      x.AddCallpath("OnTemporalTrendClick()", "TfrmAnalysis");
      throw;
   }
}

//---------------------------------------------------------------------------
// parses up a date string and places it into the given month, day, year
// interace text control (TEdit *).
//---------------------------------------------------------------------------
void TfrmAnalysis::ParseDate(char * szDate, TEdit *pYear, TEdit *pMonth, TEdit *pDay) {
  AnsiString theDate, thePart;
  int        iLoc;

  try {
    if (szDate != 0) {
      theDate = szDate;
      iLoc = theDate.Pos("/");
      if (iLoc == 0)
        ZdException::GenerateNotification("Invalid date found in parameter file.", "ParseDate()");
      else {
        thePart = theDate.SubString(1,iLoc-1);
        pYear->Text  = thePart;
        theDate.Delete(1, iLoc);
        iLoc = theDate.Pos("/");
        if (iLoc == 0)
          ZdException::GenerateNotification("Invalid date found in parameter file.", "ParseDate()");
        else {  
          thePart = theDate.SubString(1,iLoc-1);
          pMonth->Text = thePart;
          theDate.Delete(1, iLoc);
          pDay->Text   = theDate;
        }
      }
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("ParseDate()", "TfrmAnalysis");
    DisplayBasisException(this, x);//trap exception and display for now since I'm not sure why a
                              //messagebox was being used inplace of exception thrown ...
  }
}
//------------------------------------------------------------------------------
void __fastcall TfrmAnalysis::PositiveFloatKeyPress(TObject *Sender, char &Key) {
  if (!strchr("0123456789.\b",Key))
    Key = 0;
}
//---------------------------------------------------------------------------
// This is the main function to be called when reading a parameter file.
//  SetParameters(sFileName) will read the parameter file into the session
// parameter object and then SetupInterface sets the controls with the
// session obj information.
//---------------------------------------------------------------------------
bool TfrmAnalysis::ReadSession(char *sFileName) {
  bool bSessionOk = true;

  try {
    gpParams->SetParameters(sFileName);
    SetupInterface();
  }
  catch (ZdException & x) {
    x.AddCallpath("ReadSession()", "TfrmAnalysis");
    throw;
  }
  return bSessionOk;
}

//---------------------------------------------------------------------------
// main function to save a parameter file AS
//---------------------------------------------------------------------------
void TfrmAnalysis::SaveAs() {
  try {
    if (SaveDialog->Execute()) {
      gsParamFileName = SaveDialog->FileName;
      frmAnalysis->Caption = SaveDialog->FileName;
      WriteSession();
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("SaveAs()", "TfrmAnalysis");
    throw;
  }
}

//-----------------------------------------------------------------------------
//  MAIN FUNCTION TO SET TRANSFER TEXT PARAMETERS INTO SESSION OBJ GLOBALS.
//---------------------------------------------------------------------------
void TfrmAnalysis::SaveTextParameters() {
  Caption = gsParamFileName;

  try {
    //Input File Tab
    strcpy(gpParams->m_szCaseFilename,edtCaseFileName->Text.c_str());
    strcpy(gpParams->m_szControlFilename,edtControlFileName->Text.c_str());
    strcpy(gpParams->m_szPopFilename,edtPopFileName->Text.c_str());
    strcpy(gpParams->m_szCoordFilename,edtCoordinateFileName->Text.c_str());
    strcpy(gpParams->m_szGridFilename,edtGridFileName->Text.c_str());
    //Analysis Tab
    sprintf(gpParams->m_szStartDate, "%i/%i/%i", atoi(edtStartYear->Text.c_str()), atoi(edtStartMonth->Text.c_str()), atoi(edtStartDay->Text.c_str()));
    sprintf(gpParams->m_szEndDate, "%i/%i/%i", atoi(edtEndYear->Text.c_str()), atoi(edtEndMonth->Text.c_str()), atoi(edtEndDay->Text.c_str()));
    gpParams->m_nReplicas = atoi(edtMontCarloReps->Text.c_str());
    //Scanning Window Tab
    gpParams->m_nMaxGeographicClusterSize  = atof(edtMaxClusterSize->Text.c_str());
    if (rdoPercentageTemproal->Checked)
      gpParams->m_nMaxTemporalClusterSize    = atof(edtMaxTemporalClusterSize->Text.c_str());
    else
      gpParams->m_nMaxTemporalClusterSize    = atoi(edtMaxTemporalClusterSize->Text.c_str());
    gpParams->m_nMaxClusterSizeType = (rdoPercentageTemproal->Checked ? PERCENTAGETYPE : TIMETYPE);
    //Time Parameter Tab
    gpParams->m_nIntervalLength  = atoi(edtUnitLength->Text.c_str());
    gpParams->m_nTimeAdjPercent = atof(edtLogPerYear->Text.c_str());
    //rest time adjustment type if needed - something needs to be worked out so that this isn't needed.
    //Also, this changes settings so that the next time a parameter file is opened, the settings
    //of time trend adjustment are potentially different for model types other than Poisson.
    gpParams->m_nTimeAdjustType = (gpParams->m_nModel == POISSON ? gpParams->m_nTimeAdjustType : NOTADJUSTED);
    sprintf(gpParams->m_szProspStartDate, "%i/%i/%i", atoi(edtProspYear->Text.c_str()), atoi(edtProspMonth->Text.c_str()), atoi(edtProspDay->Text.c_str()));
    //Output File Tab
    strcpy(gpParams->m_szOutputFilename, edtResultFile->Text.c_str());
    gpParams->SetGISFilename();
    gpParams->SetLLRFilename();
    gpParams->SetMLCFilename();
    gpParams->SetRelRiskFilename();

    gpParams->m_bOutputRelRisks = chkRelativeRiskEstimatesAreaAscii->Enabled && chkRelativeRiskEstimatesAreaAscii->Checked;

    gpParams->SetOutputClusterLevelDBF(chkClustersInColumnFormatDBase->Checked);
    gpParams->SetOutputAreaSpecificDBF(chkCensusAreasReportedClustersDBase->Checked);
  }
  catch (ZdException & x) {
    x.AddCallpath("SaveTextParameters()", "TfrmAnalysis");
    throw;
  }
}

// Sets caption of radio button that indicates maximum spatial cluster size is
// in distance units.
void TfrmAnalysis::SetSpatialDistanceCaption() {
  try {
    switch (rgCoordinates->ItemIndex)
       {
       case 0  : rdoSpatialDistance->Caption = "Cartesian Units Radius";
                 break;
       case 1  : rdoSpatialDistance->Caption = "Kilometer Radius";
                 break;
       default : ZdException::Generate("Unknown coordinates radio button index: \"%i\".",
                                       "rgCoordinatesClick()", rgCoordinates->ItemIndex);
       }
  }
  catch (ZdException & x) {
    x.AddCallpath("SetSpatialDistanceCaption()", "TfrmAnalysis");
    throw;
  }
}

// fill the Case File field descriptor vector with the appropriate field names for a case file
void TfrmAnalysis::SetupCaseFileFieldDescriptors() {
   gvCaseFileFieldDescriptors.clear();
   gvCaseFileFieldDescriptors.push_back("Tract ID");
   gvCaseFileFieldDescriptors.push_back("Number of Cases");
   gvCaseFileFieldDescriptors.push_back("Date/Time");
   gvCaseFileFieldDescriptors.push_back("Number of Controls");
   gvCaseFileFieldDescriptors.push_back("Covariant1");
   gvCaseFileFieldDescriptors.push_back("Covariant2");
   gvCaseFileFieldDescriptors.push_back("Covariant3");
   gvCaseFileFieldDescriptors.push_back("Covariant4");
   gvCaseFileFieldDescriptors.push_back("Covariant5");
   gvCaseFileFieldDescriptors.push_back("Covariant6");
   gvCaseFileFieldDescriptors.push_back("Covariant7");
   gvCaseFileFieldDescriptors.push_back("Covariant8");
   gvCaseFileFieldDescriptors.push_back("Covariant9");
   gvCaseFileFieldDescriptors.push_back("Covariant10");
}

// fill the control File field descriptor vector with the appropriate field names for a control file
void TfrmAnalysis::SetupControlFileFieldDescriptors() {
   gvControlFileFieldDescriptors.clear();
   gvControlFileFieldDescriptors.push_back("Tract ID");
   gvControlFileFieldDescriptors.push_back("Number of Cases");
   gvControlFileFieldDescriptors.push_back("Date/Time");
   gvControlFileFieldDescriptors.push_back("Number of Controls");
   gvControlFileFieldDescriptors.push_back("Covariant1");
   gvControlFileFieldDescriptors.push_back("Covariant2");
   gvControlFileFieldDescriptors.push_back("Covariant3");
   gvControlFileFieldDescriptors.push_back("Covariant4");
   gvControlFileFieldDescriptors.push_back("Covariant5");
   gvControlFileFieldDescriptors.push_back("Covariant6");
   gvControlFileFieldDescriptors.push_back("Covariant7");
   gvControlFileFieldDescriptors.push_back("Covariant8");
   gvControlFileFieldDescriptors.push_back("Covariant9");
   gvControlFileFieldDescriptors.push_back("Covariant10");
}

// fill the Geo File field descriptor vector with the appropriate field names for a geo file
void TfrmAnalysis::SetupGeoFileFieldDescriptors() {
   gvGeoFileFieldDescriptors.clear();
   gvGeoFileFieldDescriptors.push_back("Tract ID");
   gvGeoFileFieldDescriptors.push_back("Longitude");
   gvGeoFileFieldDescriptors.push_back("Latitude");
   gvGeoFileFieldDescriptors.push_back("Dimension1");
   gvGeoFileFieldDescriptors.push_back("Dimension2");
   gvGeoFileFieldDescriptors.push_back("Dimension3");
   gvGeoFileFieldDescriptors.push_back("Dimension4");
   gvGeoFileFieldDescriptors.push_back("Dimension5");
   gvGeoFileFieldDescriptors.push_back("Dimension6");
   gvGeoFileFieldDescriptors.push_back("Dimension7");
   gvGeoFileFieldDescriptors.push_back("Dimension8");
   gvGeoFileFieldDescriptors.push_back("Dimension9");
   gvGeoFileFieldDescriptors.push_back("Dimension10");
}

// fill the Geo File field descriptor vector with the appropriate field names for a geo file
void TfrmAnalysis::SetupGridFileFieldDescriptors() {
   gvGridFileFieldDescriptors.clear();
   gvGridFileFieldDescriptors.push_back("Longitude");
   gvGridFileFieldDescriptors.push_back("Latitude");
   gvGridFileFieldDescriptors.push_back("Dimension1");
   gvGridFileFieldDescriptors.push_back("Dimension2");
   gvGridFileFieldDescriptors.push_back("Dimension3");
   gvGridFileFieldDescriptors.push_back("Dimension4");
}

// sets up the appropriate options for the FTFImportDescriptor
// pre: create and pass in a default FTFDescriptor and a .dbf ImportFileName
// post: function will modify the FTFDescriptor to contain the appropraite import options for our purposes
void TfrmAnalysis::SetupImportDescriptor(BFTFImportDescriptor& descrip, const ZdString& sImportFileName) {
   try {
      descrip.SetDestinationType(BFileDestDescriptor::SingleFile);
      descrip.SetModifyType(BFileDestDescriptor::OverWriteExistingData);
      descrip.SetImportFile(sImportFileName);
   }
   catch (ZdException &x) {
      x.AddCallpath("SetupImportDescriptor()", "TfrmAnalysis");
      throw;
   }
}

// fill the Geo File field descriptor vector with the appropriate field names for a geo file
void TfrmAnalysis::SetupPopFileFieldDescriptors() {
   gvPopFileFieldDescriptors.push_back("Tract ID");
   gvPopFileFieldDescriptors.push_back("Date/Time");
   gvPopFileFieldDescriptors.push_back("Population");
   gvPopFileFieldDescriptors.push_back("Covariant1");
   gvPopFileFieldDescriptors.push_back("Covariant2");
   gvPopFileFieldDescriptors.push_back("Covariant3");
}

//---------------------------------------------------------------------------
//  Sets all interface controls using the gpParams session object
//---------------------------------------------------------------------------
void TfrmAnalysis::SetupInterface() {
  Caption = gsParamFileName;

  try {
    // Odd SatScan changed these values... WHY ???
    //gpParams->m_nAnalysisType --;
    //gpParams->m_nAreas --;

    // THIS "IF" STATEMENT IS HERE JUST TO MATCH THE CODE FOUND IN THE OLD VISUAL C++ INTERFACE....
    if (gpParams->m_nAnalysisType == PURELYSPATIAL) {
      gpParams->m_nIntervalUnits = YEAR;
      gpParams->m_nIntervalLength = 1;
    }

    //Input File Tab
    edtCaseFileName->Text = gpParams->m_szCaseFilename;
    edtControlFileName->Text = gpParams->m_szControlFilename;
    rgPrecisionTimes->ItemIndex = gpParams->m_nPrecision;
    edtPopFileName->Text = gpParams->m_szPopFilename;
    edtCoordinateFileName->Text = gpParams->m_szCoordFilename;
    edtGridFileName->Text = gpParams->m_szGridFilename;
    rgCoordinates->ItemIndex = gpParams->m_nCoordType;
    //Analysis Tab
    rgTypeAnalysis->ItemIndex = gpParams->m_nAnalysisType - 1;
    rgProbability->ItemIndex = gpParams->m_nModel;
    rgScanAreas->ItemIndex = gpParams->m_nAreas - 1;
    ParseDate(gpParams->m_szStartDate, edtStartYear, edtStartMonth, edtStartDay);
    ParseDate(gpParams->m_szEndDate, edtEndYear, edtEndMonth, edtEndDay);
    edtMontCarloReps->Text = gpParams->m_nReplicas;
    //Scanning Window Tab
    edtMaxClusterSize->Text = gpParams->m_nMaxGeographicClusterSize;
    chkInclPurTempClust->Checked = gpParams->m_bIncludePurelyTemporal;
    edtMaxTemporalClusterSize->Text = gpParams->m_nMaxTemporalClusterSize;
    rdoPercentageTemproal->Checked = gpParams->m_nMaxClusterSizeType == PERCENTAGETYPE;
    rdoTimeTemproal->Checked = gpParams->m_nMaxClusterSizeType == TIMETYPE;
    chkIncludePurSpacClust->Checked = gpParams->m_bIncludePurelySpatial;
    rdoSpatialPercentage->Checked = gpParams->m_nMaxSpatialClusterSizeType != DISTANCETYPE; // default checked
    rdoSpatialDistance->Checked = gpParams->m_nMaxSpatialClusterSizeType == DISTANCETYPE;
    SetSpatialDistanceCaption();

    //***************** check this code ******************************
    rgClustersToInclude->ItemIndex = (gpParams->m_bAliveClustersOnly ? 1:0);  // IS THIS RETURNING THE RIGHT INDEX OR SHOULD I SWITCH IT AROUND ???
    if (gpParams->m_nAnalysisType == PROSPECTIVESPACETIME) { //DISABLE the Include Purely Spacial Clusters option.
      chkIncludePurSpacClust->Checked = false;
      chkIncludePurSpacClust->Enabled = false;
    }
    else
      chkIncludePurSpacClust->Enabled = true;
    //***************** check this code ******************************

    //Time Parameter Tab
    if (gpParams->m_nIntervalUnits == 0) gpParams->m_nIntervalUnits = 1;
    if (gpParams->m_nIntervalLength == 0) gpParams->m_nIntervalLength = 1;
    rbUnitYear->Checked = (gpParams->m_nIntervalUnits == 1);  // use to be 0
    rbUnitMonths->Checked = (gpParams->m_nIntervalUnits == 2);  // use to be 1
    rbUnitDay->Checked = (gpParams->m_nIntervalUnits == 3);  // use to be 2
    edtUnitLength->Text = gpParams->m_nIntervalLength;
    rgTemporalTrendAdj->ItemIndex = gpParams->m_nTimeAdjustType;
    edtLogPerYear->Text = gpParams->m_nTimeAdjPercent;
    if (strlen(gpParams->m_szProspStartDate) > 0)
      ParseDate(gpParams->m_szProspStartDate, edtProspYear, edtProspMonth, edtProspDay);
    //Output File Tab
    edtResultFile->Text        = gpParams->m_szOutputFilename;
    chkRelativeRiskEstimatesAreaAscii->Checked = gpParams->m_bOutputRelRisks;
    chkSimulatedLogLikelihoodRatiosAscii->Checked  = gpParams->m_bSaveSimLogLikelihoods;
    chkCensusAreasReportedClustersAscii->Checked    = gpParams->m_bOutputCensusAreas;  // Output Census areas in Reported Clusters
    chkClustersInColumnFormatAscii->Checked = gpParams->m_bMostLikelyClusters;  // Output Most Likely Cluster for each Centroid
    cboCriteriaSecClusters->ItemIndex = gpParams->m_iCriteriaSecondClusters;
    chkClustersInColumnFormatDBase->Checked = gpParams->GetOutputClusterLevelDBF();
    chkCensusAreasReportedClustersDBase->Checked = gpParams->GetOutputAreaSpecificDBF();
    //now enable or disable controls appropriately
    OnProbabilityModelClick();
  }
  catch (ZdException & x) {
    x.AddCallpath("SetupInterface()", "TfrmAnalysis");
    throw;
  }
}

//------------------------------------------------------------------------------
// Validates the existence of the input files...  this is more for any
// session file that was read in....  If the Browse buttons are used, then you
// know that the files exist.  But, if a session file was directly read in
// and run, you do not know if the original files still exist etc...
//  THEIR EXISTENCE MUST BE VERIFIED.
//------------------------------------------------------------------------------
bool TfrmAnalysis::ValidateInputFiles() {
  bool bOk = true;

  try {
    if (edtCaseFileName->Enabled) {// case file edit box enabled
      if (edtCaseFileName->Text.IsEmpty()) {
        Application->MessageBox("Please specify a case file.", "Parameter Error" , MB_OK);
        bOk = false;
      }
      else
        bOk = ValidateFileExists(edtCaseFileName->Text, "Case");

      if (!bOk) {
        PageControl1->ActivePage = tbInputFiles;
        edtCaseFileName->SetFocus();
      }
    }
    //Control file for Bernoulli model only
    if (bOk & (rgProbability->ItemIndex==1)) {// Control file edit box enabled
      if (edtControlFileName->Text.IsEmpty()) {
        Application->MessageBox("For the Bernoulli model, please specify a control file.", "Parameter Error" , MB_OK);
        bOk = false;
      }
      else
        bOk = ValidateFileExists(edtControlFileName->Text, "Control");

      if (!bOk) {
        PageControl1->ActivePage = tbInputFiles;
        edtControlFileName->SetFocus();
      }
    }
    //Pop file for Poisson model only
    if (bOk & (rgProbability->ItemIndex==0)) {// Population file edit box enabled
      if (edtPopFileName->Text.IsEmpty()) {
        Application->MessageBox("For the Poisson model, please specify a population file.", "Parameter Error" , MB_OK);
        bOk = false;
      }
      else
        bOk = ValidateFileExists(edtPopFileName->Text, "Population");

      if (!bOk) {
        PageControl1->ActivePage = tbInputFiles;
        edtPopFileName->SetFocus();
      }
    }
    if (bOk & edtCoordinateFileName->Enabled) {// Coordinates file edit box enabled
      if (edtCoordinateFileName->Text.IsEmpty()) {
        Application->MessageBox("Please specify a coordinates file.", "Parameter Error" , MB_OK);
        bOk = false;
      }
      else
        bOk = ValidateFileExists(edtCoordinateFileName->Text, "Coordinate");

      if (!bOk) {
        PageControl1->ActivePage = tbInputFiles;
        edtCoordinateFileName->SetFocus();
      }
    }
    if (bOk & edtGridFileName->Enabled & !edtGridFileName->Text.IsEmpty()) {// Special grid file edit box enabled
      bOk = ValidateFileExists(edtGridFileName->Text, "Special Grid");
      if (!bOk) {
        PageControl1->ActivePage = tbInputFiles;
        edtGridFileName->SetFocus();
      }
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("ValidateInputFiles()", "TfrmAnalysis");
    throw;
  }
  return bOk;
}

//---------------------------------------------------------------------------
// This function is used right before a job is submitted.  Verifies that
// all the input files exist and can be read.  Also checks each tab to see
// if all settings are in place.
//---------------------------------------------------------------------------
bool TfrmAnalysis::ValidateParams() {
  bool bDataOk;

  try {
    SaveTextParameters();
    // check all input tab params
    bDataOk = ValidateInputFiles();
    // check all Analsis and other tab params
    if (bDataOk)
      bDataOk = CheckAnalysisParams();
    if (bDataOk)
      bDataOk = CheckScanningWindowParams();
    if (bDataOk)
      bDataOk = CheckTimeParams();
    if (bDataOk)
      bDataOk = CheckOutputParams();
  }
  catch (ZdException & x) {
    x.AddCallpath("ValidateParams()", "TfrmAnalysis");
    bDataOk = false;
    DisplayBasisException(this, x);
  }
  return bDataOk;
}

bool TfrmAnalysis::ValidateSpatialClusterSize() {
  double dValue;
  bool   bOkParams=true;

  try {
    if (edtMaxClusterSize->Enabled) {
      if (!edtMaxClusterSize->Text.Length() || atof(edtMaxClusterSize->Text.c_str()) == 0)
        ZdException::GenerateNotification("Please specify a maximum spatial cluster size.","ValidateSpatialClusterSize()");

      dValue = atof(edtMaxClusterSize->Text.c_str());
      if (!(dValue > 0.0 && dValue <= 50.0) && rdoSpatialPercentage->Checked)
        ZdException::GenerateNotification("Please specify valid maximum spatial cluster size between %d - %d.",
                                          "ValidateSpatialClusterSize()", 0, 50);

      gpParams->m_nMaxGeographicClusterSize = atof(edtMaxClusterSize->Text.c_str());
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("ValidateSpatialClusterSize()", "TfrmAnalysis");
    PageControl1->ActivePage = tbScanningWindow;
    edtMaxClusterSize->SetFocus();
    bOkParams = false;
    DisplayBasisException(this, x);
  }
  return bOkParams;
}

bool TfrmAnalysis::ValidateTemoralClusterSize() {
  bool          bParamsOk = true;
  float         dValue;
  ZdString      sErrorMessage;
  ZdDate        StartDate, EndDate, EndDatePlusOne,StartPlusIntervalDate;
  ZdDateFilter  DateFilter("%4y/%02m/%02d");
  char          FilterBuffer[11], Buffer[10];
  unsigned long ulMaxClusterDays, ulIntervalLengthInDays;

  try {
    //check whether we are specifiying temporal information
    if (edtMaxTemporalClusterSize->Enabled) {
      if (!edtMaxTemporalClusterSize->Text.Length() || atof(edtMaxTemporalClusterSize->Text.c_str()) == 0)
        ZdException::GenerateNotification("Please specify a maximum temporal cluster size.","ValidateTemoralClusterSize()");

      //check maximum temporal cluster size(as percentage pf population) is less
      //than maximum for given probabilty model
      if (rdoPercentageTemproal->Checked) {
        dValue = atof(edtMaxTemporalClusterSize->Text.c_str());
        if (!(dValue > 0.0 && dValue <= (rgProbability->ItemIndex == SPACETIMEPERMUTATION ? 50 : 90))) {
          sErrorMessage << "For the " << rgProbability->Items->Strings[rgProbability->ItemIndex].c_str();
          sErrorMessage << " model, the maximum temporal cluster size as a percent of study period is ";
          sErrorMessage << (rgProbability->ItemIndex == SPACETIMEPERMUTATION ? 50 : 90);
          sErrorMessage << " percent.\nPlease review settings.";
          ZdException::GenerateNotification(sErrorMessage.GetCString(), "ValidateTemoralClusterSize()");
        }
      }
      //check that maximum temporal cluster size(in time units) is less than
      //maximum for probabilty model. Determine the number of days the maximum
      //temporal cluster can be. Compare that against start date plus interval
      //length units.
      else if (rdoTimeTemproal->Checked) {
        GetStudyPeriodStartDate(StartDate);
        GetStudyPeriodEndDate(EndDate);

        //to make start and end day inclusive - add 1 to EndDate date
        EndDatePlusOne = EndDate;
        EndDatePlusOne.AddDays(1);
        ulMaxClusterDays = EndDatePlusOne.GetJulianDayFromCalendarStart() - StartDate.GetJulianDayFromCalendarStart();
        ulMaxClusterDays = (rgProbability->ItemIndex == SPACETIMEPERMUTATION ? ulMaxClusterDays * 0.5 : ulMaxClusterDays * 0.9);

        StartPlusIntervalDate = StartDate;
        //add time interval length as units to modified start date
        switch (gpParams->m_nIntervalUnits) {
            case      (YEAR)      : StartPlusIntervalDate.AddYears(static_cast<unsigned short>(atoi(edtMaxTemporalClusterSize->Text.c_str())));
                                    strcpy(Buffer,"year(s)");
                                    break;
            case      (MONTH)     : StartPlusIntervalDate.AddMonths(static_cast<unsigned short>(atoi(edtMaxTemporalClusterSize->Text.c_str())));
                                    //to make start and end day inclusive - add one day to interval
                                    StartPlusIntervalDate.AddDays(1);
                                    strcpy(Buffer,"month(s)");
                                    break;
            case      (DAY)       : //to make start and end day inclusive - add interval length minus 1
                                    StartPlusIntervalDate.AddDays(static_cast<unsigned short>(atoi(edtMaxTemporalClusterSize->Text.c_str())));
                                    strcpy(Buffer,"day(s)");
                                    break;
            default               : ZdGenerateException("Unknown interval unit \"%d\"", "ValidateTemoralClusterSize()", gpParams->m_nIntervalUnits);
        };
        ulIntervalLengthInDays = StartPlusIntervalDate.GetJulianDayFromCalendarStart() - StartDate.GetJulianDayFromCalendarStart();
        if (ulIntervalLengthInDays > ulMaxClusterDays) {
          DateFilter.FilterValue(FilterBuffer, sizeof(FilterBuffer), StartDate.GetRawDate());
          sErrorMessage << "For the study period starting on " << FilterBuffer << " and ending on ";
          DateFilter.FilterValue(FilterBuffer, sizeof(FilterBuffer), EndDate.GetRawDate());
          sErrorMessage << FilterBuffer << ",\na maximum temporal cluster size of " << edtMaxTemporalClusterSize->Text.c_str();
          sErrorMessage << " " << Buffer << " is greater than " << (rgProbability->ItemIndex == SPACETIMEPERMUTATION ? 50 : 90);
          sErrorMessage << " percent of study period.\nPlease review settings.";
          ZdGenerateException(sErrorMessage.GetCString(), "ValidateTemoralClusterSize()");
        }
      }
      else
        ZdException::GenerateNotification("Type specified as neither percentage nor time.", "ValidateTemoralClusterSize()");
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("ValidateTemoralClusterSize()", "TfrmAnalysis");
    bParamsOk = false;
    PageControl1->ActivePage = tbScanningWindow;
    edtMaxTemporalClusterSize->SetFocus();
    Application->MessageBox(x.GetErrorMessage(), "Notification", MB_OK );
  }
  return bParamsOk;
}

//---------------------------------------------------------------------------
// Writes the session information to disk
//---------------------------------------------------------------------------
bool TfrmAnalysis::WriteSession() {
  bool bWriteOk = false;

  try {
    if (! gsParamFileName.IsEmpty()) {
      SaveTextParameters();
      bWriteOk = gpParams->SaveParameters(gsParamFileName.c_str());
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("WriteSession()", "TfrmAnalysis");
    throw;
  }
  return bWriteOk;
}


// ==========================================================================
// fastcalls

void __fastcall TfrmAnalysis::edtCaseFileNameChange(TObject *Sender) {
  edtCaseFileName->Hint = edtCaseFileName->Text;
}
//---------------------------------------------------------------------------
void __fastcall TfrmAnalysis::edtControlFileNameChange(TObject *Sender){
  edtControlFileName->Hint = edtControlFileName->Text;
}
//---------------------------------------------------------------------------
void __fastcall TfrmAnalysis::edtPopFileNameChange(TObject *Sender){
  edtPopFileName->Hint = edtPopFileName->Text;
}
//---------------------------------------------------------------------------
void __fastcall TfrmAnalysis::edtCoordinateFileNameChange(TObject *Sender){
  edtCoordinateFileName->Hint = edtCoordinateFileName->Text;
}
//---------------------------------------------------------------------------
void __fastcall TfrmAnalysis::edtGridFileNameChange(TObject *Sender) {
  edtGridFileName->Hint = edtGridFileName->Text;
}
//---------------------------------------------------------------------------
void __fastcall TfrmAnalysis::edtResultFileChange(TObject *Sender){
  edtResultFile->Hint = edtResultFile->Text;
}
//---------------------------------------------------------------------------

void __fastcall TfrmAnalysis::cboCriteriaSecClustersChange(TObject *Sender){
   gpParams->m_iCriteriaSecondClusters = cboCriteriaSecClusters->ItemIndex;
}
//---------------------------------------------------------------------------
void __fastcall TfrmAnalysis::rdoSpatialPercentageClick(TObject *Sender) {
   gpParams->m_nMaxSpatialClusterSizeType = PERCENTAGEOFMEASURETYPE;
}

//------------------------------------------------------------------------------
// Specific Day unit control
//------------------------------------------------------------------------------
void __fastcall TfrmAnalysis::rbUnitDayClick(TObject *Sender) {
    gpParams->m_nIntervalUnits = 3; // use to be 2
    rdoTimeTemproal->Caption = "Days";
}
//------------------------------------------------------------------------------
// Specific Months unit control
//------------------------------------------------------------------------------
void __fastcall TfrmAnalysis::rbUnitMonthsClick(TObject *Sender) {
    gpParams->m_nIntervalUnits = 2; // use to be 1
    rdoTimeTemproal->Caption = "Months";
}
//------------------------------------------------------------------------------
// Specific Year unit control
//------------------------------------------------------------------------------
void __fastcall TfrmAnalysis::rbUnitYearClick(TObject *Sender) {
    gpParams->m_nIntervalUnits = 1; // use to be 0
    rdoTimeTemproal->Caption = "Years";
}
//------------------------------------------------------------------------------
//  Control to include "Alive" clusters
//------------------------------------------------------------------------------
void __fastcall TfrmAnalysis::rgClustersToIncludeClick(TObject *Sender) {
    gpParams->m_bAliveClustersOnly = (rgClustersToInclude->ItemIndex == 0 ? 0:1);
}
//------------------------------------------------------------------------------
// If the types of coordinates are changed, then various interface options
// need to be updated
//------------------------------------------------------------------------------
void __fastcall TfrmAnalysis::rgCoordinatesClick(TObject *Sender) {
  try {
    gpParams->m_nCoordType = rgCoordinates->ItemIndex;
    SetSpatialDistanceCaption();
  }
  catch (ZdException & x) {
    x.AddCallpath("rgCoordinatesClick()", "TfrmAnalysis");
    DisplayBasisException(this, x);
  }
}
//---------------------------------------------------------------------------
void __fastcall TfrmAnalysis::rdoSpatialDistanceClick(TObject *Sender){
   gpParams->m_nMaxSpatialClusterSizeType = DISTANCETYPE;
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
void __fastcall TfrmAnalysis::rgPrecisionTimesClick(TObject *Sender) {
  try {
     OnPrecisionTimesClick();
  }
  catch (ZdException & x) {
    x.AddCallpath("rgPrecisionTimesClick()", "TfrmAnalysis");
    DisplayBasisException(this, x);
  }
}

void __fastcall TfrmAnalysis::rgProbabilityClick(TObject *Sender) {
  try {
     OnProbabilityModelClick();
  }
  catch ( ZdException & x ) {
    x.AddCallpath( "rgProbabilityClick()", "TfrmAnalysis" );
    DisplayBasisException( this, x );
  }
}

void __fastcall TfrmAnalysis::rgTemporalTrendAdjClick(TObject *Sender) {
  try {
     OnTemporalTrendClick();
  }
  catch (ZdException & x) {
    x.AddCallpath("rgTemporalTrendAdjClick()", "TfrmAnalysis");
    DisplayBasisException(this, x);
   }
}

//------------------------------------------------------------------------------
// Scan Areas is changed.  Need to perform a "dataexchange" to see if any
// other tab controls need to be enabled or disabled
//------------------------------------------------------------------------------
void __fastcall TfrmAnalysis::rgScanAreasClick(TObject *Sender) {
  try {
    gpParams->m_nAreas = rgScanAreas->ItemIndex + 1;
  }
  catch (ZdException & x) {
    x.AddCallpath("rgScanAreasClick()", "TfrmAnalysis");
    DisplayBasisException(this, x);
  }
}

void __fastcall TfrmAnalysis::rgTypeAnalysisClick(TObject *Sender) {
  try {
     OnAnalysisTypeClick();
  }
  catch (ZdException & x) {
    x.AddCallpath("rgTypeAnalysisClick()", "TfrmAnalysis");
    DisplayBasisException(this, x);
  }
}
