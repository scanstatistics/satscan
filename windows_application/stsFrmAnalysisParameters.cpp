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
//---------------------------------------------------------------------------
// case file selector
//---------------------------------------------------------------------------
void __fastcall TfrmAnalysis::btnCaseBrowseClick(TObject *Sender) {
  BFTFImportDescriptor  * pBFTFPointer = 0;
  ZdFileName            sFileName;
  char                  sBuffer[1024];

  try {
    OpenDialog1->FileName = "";
    OpenDialog1->DefaultExt = "*.cas";
    OpenDialog1->Filter = "CAS Files (*.cas)|*.cas|DBase Files (*.dbf)|*.dbf|All files (*.*)|*.*";
    OpenDialog1->Title = "Select Case File";
    if (OpenDialog1->Execute()) {
      sFileName = OpenDialog1->FileName.c_str();
      //Detect dbf file and launch importer if detected
      if ( DetermineIfDbfExtension(OpenDialog1->FileName) ) {
         pBFTFPointer = new BFTFImportDescriptor();
         pBFTFPointer->SetGenerateReport(false);
         SetupImportDescriptor(*pBFTFPointer, OpenDialog1->FileName.c_str());
         GetTempPath(sizeof(sBuffer), sBuffer);
         sFileName.SetLocation(sBuffer);
         sFileName.SetExtension(TXD_EXT);
         pBFTFPointer->SetDestinationFile(sFileName.GetFullPath());
         CreateTXDFile(sFileName, gvCaseFileFieldDescriptors);
         auto_ptr<TBdlgImporter> pImporter = auto_ptr<TBdlgImporter>(new TBdlgImporter(0, 0, pBFTFPointer));
         pImporter->ShowOptionalPanels(false, false, false);
         pImporter->ShowModal();
         delete pBFTFPointer;
      }

      //Why is this here? KMC 8/30/2002
      // sets the global paramater to store the filename and also sets the editbox to display the filename - AJV 9/4/2002
      strcpy(gpParams->m_szCaseFilename, sFileName.GetFullPath());
      edtCaseFileName->Text = sFileName.GetFullPath();
      strcpy(gpParams->m_szCaseFilename, edtCaseFileName->Text.c_str());
    }
  }
  catch (ZdException & x) {
    delete pBFTFPointer; pBFTFPointer = 0;
    x.AddCallpath("btnCaseBrowseClick()", "TfrmAnalysis");
    DisplayBasisException(this, x);
  }
}
//---------------------------------------------------------------------------
// Control file selector -- *.ctl files
//---------------------------------------------------------------------------
void __fastcall TfrmAnalysis::btnControlBrowseClick(TObject *Sender) {
  BFTFImportDescriptor  * pBFTFPointer = 0;
  ZdFileName            sFileName;
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
         pBFTFPointer = new BFTFImportDescriptor();
         pBFTFPointer->SetGenerateReport(false);
         SetupImportDescriptor(*pBFTFPointer, OpenDialog1->FileName.c_str());
         GetTempPath(sizeof(sBuffer), sBuffer);
         sFileName.SetLocation(sBuffer);
         sFileName.SetExtension(TXD_EXT);
         pBFTFPointer->SetDestinationFile(sFileName.GetFullPath());
         CreateTXDFile(sFileName, gvControlFileFieldDescriptors);
         auto_ptr<TBdlgImporter> pImporter = auto_ptr<TBdlgImporter>(new TBdlgImporter(0, 0, pBFTFPointer));
         pImporter->ShowOptionalPanels(false, false, false);
         pImporter->ShowModal();
         delete pBFTFPointer;
      }

      //Why is this here? KMC 8/30/2002
      // sets the global paramater to store the filename and also sets the editbox to display the filename - AJV 9/4/2002
      strcpy(gpParams->m_szControlFilename, sFileName.GetFullPath());
      edtControlFileName->Text = sFileName.GetFullPath();
      strcpy(gpParams->m_szControlFilename, edtControlFileName->Text.c_str());
     }
  }
  catch (ZdException & x) {
    delete pBFTFPointer; pBFTFPointer = 0;
    x.AddCallpath("btnControlBrowseClick()", "TfrmAnalysis");
    DisplayBasisException(this, x);
  }
}
//---------------------------------------------------------------------------
// Geographic file selector -- *.geo files
//---------------------------------------------------------------------------
void __fastcall TfrmAnalysis::btnCoordBrowseClick(TObject *Sender) {
  BFTFImportDescriptor  * pBFTFPointer = 0;
  ZdFileName            sFileName;
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
         pBFTFPointer = new BFTFImportDescriptor();
         pBFTFPointer->SetGenerateReport(false);
         SetupImportDescriptor(*pBFTFPointer, OpenDialog1->FileName.c_str());
         GetTempPath(sizeof(sBuffer), sBuffer);
         sFileName.SetLocation(sBuffer);
         sFileName.SetExtension(TXD_EXT);
         pBFTFPointer->SetDestinationFile(sFileName.GetFullPath());
         CreateTXDFile(sFileName, gvGeoFileFieldDescriptors);
         auto_ptr<TBdlgImporter> pImporter = auto_ptr<TBdlgImporter>(new TBdlgImporter(0, 0, pBFTFPointer));
         pImporter->ShowOptionalPanels(false, false, false);
         pImporter->ShowModal();
         delete pBFTFPointer;
      }

      //Why is this here? KMC 8/30/2002
      // sets the global paramater to store the filename and also sets the editbox to display the filename - AJV 9/4/2002
      strcpy(gpParams->m_szCoordFilename, sFileName.GetFullPath());
      edtCoordinateFileName->Text = sFileName.GetFullPath();
      strcpy(gpParams->m_szCoordFilename, edtCoordinateFileName->Text.c_str());
    }
  }
  catch (ZdException & x) {
    delete pBFTFPointer; pBFTFPointer = 0;
    x.AddCallpath("btnCoordBrowseClick()", "TfrmAnalysis");
    DisplayBasisException(this, x);
  }
}
//---------------------------------------------------------------------------
//  Grid file selector -- *.grd files
//---------------------------------------------------------------------------
void __fastcall TfrmAnalysis::btnGridBrowseClick(TObject *Sender) {
  BFTFImportDescriptor  * pBFTFPointer = 0;
  ZdFileName            sFileName;
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
         pBFTFPointer = new BFTFImportDescriptor();
         pBFTFPointer->SetGenerateReport(false);
         SetupImportDescriptor(*pBFTFPointer, OpenDialog1->FileName.c_str());
         GetTempPath(sizeof(sBuffer), sBuffer);
         sFileName.SetLocation(sBuffer);
         sFileName.SetExtension(TXD_EXT);
         pBFTFPointer->SetDestinationFile(sFileName.GetFullPath());
         CreateTXDFile(sFileName, gvGridFileFieldDescriptors);
         auto_ptr<TBdlgImporter> pImporter = auto_ptr<TBdlgImporter>(new TBdlgImporter(0, 0, pBFTFPointer));
         pImporter->ShowOptionalPanels(false, false, false);
         pImporter->ShowModal();
         delete pBFTFPointer;
      }

      //Why is this here? KMC 8/30/2002
      // sets the global paramater to store the filename and also sets the editbox to display the filename - AJV 9/4/2002
      strcpy(gpParams->m_szGridFilename, sFileName.GetFullPath());
      edtGridFileName->Text = sFileName.GetFullPath();
      strcpy(gpParams->m_szGridFilename, edtGridFileName->Text.c_str());
    }
  }
  catch (ZdException & x) {
    delete pBFTFPointer; pBFTFPointer = 0;
    x.AddCallpath("btnGridBrowseClick()", "TfrmAnalysis");
    DisplayBasisException(this, x);
  }
}
//---------------------------------------------------------------------------
// Population file selector -- *.pop files
//---------------------------------------------------------------------------
void __fastcall TfrmAnalysis::btnPopBrowseClick(TObject *Sender) {
  BFTFImportDescriptor  * pBFTFPointer = 0;
  ZdFileName            sFileName;
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
          pBFTFPointer = new BFTFImportDescriptor();
          pBFTFPointer->SetGenerateReport(false);
          SetupImportDescriptor(*pBFTFPointer, OpenDialog1->FileName.c_str());
          GetTempPath(sizeof(sBuffer), sBuffer);
          sFileName.SetLocation(sBuffer);
          sFileName.SetExtension(TXD_EXT);
          pBFTFPointer->SetDestinationFile(sFileName.GetFullPath());
          CreateTXDFile(sFileName, gvPopFileFieldDescriptors);
          auto_ptr<TBdlgImporter> pImporter = auto_ptr<TBdlgImporter>(new TBdlgImporter(0, 0, pBFTFPointer));
          pImporter->ShowOptionalPanels(false, false, false);
          pImporter->ShowModal();
          delete pBFTFPointer;
       }

      //Why is this here? KMC 8/30/2002
      // sets the global paramater to store the filename and also sets the editbox to display the filename - AJV 9/4/2002
      strcpy(gpParams->m_szPopFilename, sFileName.GetFullPath());
      edtPopFileName->Text = sFileName.GetFullPath();
      strcpy(gpParams->m_szPopFilename, edtPopFileName->Text.c_str());
    }
  }
  catch (ZdException & x) {
    delete pBFTFPointer; pBFTFPointer = 0;
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
      //UpdateData(false);
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
      MessageBox(NULL, sFinalMessage.c_str(), "Parameter Error" , MB_OK);
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
// Verifies the interval length.  If you add interval length to start date,
// the resultant date must not be beyond the end date
//------------------------------------------------------------------------------
bool TfrmAnalysis::Check_IntervalLength(int iStartYear, int iStartMonth, int iStartDay,
            int iEndYear, int iEndMonth, int iEndDay, int iIntervalUnits, int iIntervalLength) {
  char szMessage[100];
  Julian Start, End;
  long   lMin = 1, lMax;
  bool   bIntervalLenOk = true;

  try {
    Start = MDYToJulian(iStartMonth, iStartDay, iStartYear);
    End   = MDYToJulian(iEndMonth, iEndDay, iEndYear);
    lMax = TimeBetween(Start, End, iIntervalUnits);  //use to be iIntervalUnits+1
    if ((iIntervalLength < lMin) || (iIntervalLength > lMax)) {
      sprintf(szMessage, "The interval length must be between %i and %i.", lMin, lMax);
      MessageBox(NULL, szMessage, "Parameter Error" , MB_OK);
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
  bool bMonthOk = true;
  AnsiString sFinalMessage;

  try {
    if ((iMonth < 1) || (iMonth > 12)) {
      sFinalMessage += sDateName;
      sFinalMessage += ":  Please specify an month between 1 and 12.";
      MessageBox(NULL, sFinalMessage.c_str(), "Parameter Error" , MB_OK);
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
  bool bTrendPctOk = true;

  if ( ! (dValue > -100.00)) {
    MessageBox(NULL, "Invalid time trend percentage specified.", "Parameter Error" , MB_OK);
    bTrendPctOk = false;
  }
 return bTrendPctOk;
}
//------------------------------------------------------------------------------
// Generic Year checker -- simulates old interface
//MUST CHECK TO SEE IF THE DAY EDIT BOX IS ENABLED FIRST !!!!
//------------------------------------------------------------------------------
bool TfrmAnalysis::Check_Year(int iYear, char *sDateName) {
  char szMessage[100];
  bool bYearOk = true;
  AnsiString sFinalMessage;

  try {
    if ( ! (iYear >= MIN_YEAR) && (iYear <= MAX_YEAR)) {
      sFinalMessage += sDateName;
      sprintf(szMessage, ":  Please specify a year between %i and %i.", MIN_YEAR, MAX_YEAR);
      sFinalMessage += szMessage;
      MessageBox(NULL, sFinalMessage.c_str(), "Parameter Error" , MB_OK);
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
      if (bParamsOk) Check_Year(atoi(edtEndYear->Text.c_str()),"Study Period End Year");
    }
    //if Months enabled, then check values...
    //if start month enabled, assume end month enabled.
    if (bParamsOk && edtStartMonth->Enabled) {
      bParamsOk = Check_Month(atoi(edtStartMonth->Text.c_str()), "Study Period Start Month");
      if (bParamsOk) Check_Month(atoi(edtEndMonth->Text.c_str()), "Study Period End Month");
    }
    //if Days enabled, then check values...
    //if start days enabled, assume end days enabled.
    if (bParamsOk && edtStartDay->Enabled) {
      bParamsOk = Check_Days(atoi(edtStartYear->Text.c_str()), atoi(edtStartMonth->Text.c_str()), atoi(edtStartDay->Text.c_str()),"Study Period Start Date");
      if (bParamsOk) Check_Days(atoi(edtEndYear->Text.c_str()), atoi(edtEndMonth->Text.c_str()), atoi(edtEndDay->Text.c_str()),"Study Period End Date");
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
// Verifies the relationship between a start and end date
//------------------------------------------------------------------------------
bool TfrmAnalysis::CheckDateRange(int iStartYear, int iStartMonth, int iStartDay,
                                  int iEndYear, int iEndMonth, int iEndDay,
                                  int iIntervalUnits, int iIntervalLength) {
  bool bRangeOk = true;
  char szMessage[100];
  char szUnit[10];
  Julian Start, End;
  long   lTimeBetween;

  try {
    Start = MDYToJulian(iStartMonth, iStartDay, iStartYear);
    End   = MDYToJulian(iEndMonth, iEndDay, iEndYear);
    lTimeBetween = TimeBetween(Start, End, iIntervalUnits);  // use to be iIntervalUnits+1
    switch (iIntervalUnits) { // use to be iIntervalUnits + 1;
      case (YEAR)       : strcpy(szUnit, "year(s)"); break;
      case (MONTH)      : strcpy(szUnit, "month(s)"); break;
      case (DAY)        : strcpy(szUnit, "day(s)"); break;
      default           : break;
    };
    if (iIntervalLength > lTimeBetween) {
      sprintf(szMessage, "Due to interval length specified (Time Parameter tab), the start and end dates must be at least %d %s apart.", iIntervalLength, szUnit);
      MessageBox(NULL, szMessage, "Parameter Error" , MB_OK);
      bRangeOk = false;
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
    if (edtResultFile->Enabled)
      bReturn = ValidateFileCanCreate(edtResultFile->Text, "Output");
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
      MessageBox(NULL, "The Start date of Prospective Space-Time must be between the Study Period start and end dates.", "Parameter Error" , MB_OK);
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
  bool bReplicasOk = true;

  try {
    if (! ((iReplicas == 0) || (iReplicas == 9) || (iReplicas == 19) || (fmod(iReplicas+1, 1000) == 0.0)) ) {
      MessageBox(NULL, "Invalid number of replicas specified.", "Parameter Error" , MB_OK);
      bReplicasOk = false;
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("CheckReplicas()", "TfrmAnalysis");
    throw;
  }
  return bReplicasOk;
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
          MessageBox(NULL, "Please enter a number in time trend percentage.", "Parameter Error" , MB_OK);
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
void __fastcall TfrmAnalysis::chkCensusAreasClick(TObject *Sender) {
   gpParams->m_bOutputCensusAreas = chkCensusAreas->Checked;
}
//------------------------------------------------------------------------------
// Include relative risks in output
//------------------------------------------------------------------------------
void __fastcall TfrmAnalysis::chkInclRelRiskEstClick(TObject *Sender) {
  try {
    gpParams->m_bOutputRelRisks = chkInclRelRiskEst->Checked;
    DataExchange();
  }
  catch (ZdException & x) {
    x.AddCallpath("chkInclRelRiskEstClick()", "TfrmAnalysis");
    DisplayBasisException(this, x);
  }
}
//------------------------------------------------------------------------------
// Include Purely Spacial Clusters selection control
//------------------------------------------------------------------------------
void __fastcall TfrmAnalysis::chkIncludePurSpacClustClick(TObject *Sender) {
  try {
    gpParams->m_bIncludePurelySpatial = chkIncludePurSpacClust->Checked;
    DataExchange();
  }
  catch (ZdException & x) {
    x.AddCallpath("chkIncludePurSpacClustClick()", "TfrmAnalysis");
    DisplayBasisException(this, x);
  }
}
//------------------------------------------------------------------------------
// Include Purely Temporal Clusters selection control
//------------------------------------------------------------------------------
void __fastcall TfrmAnalysis::chkInclPurTempClustClick(TObject *Sender) {
  try {
    gpParams->m_bIncludePurelyTemporal = chkInclPurTempClust->Checked;
    DataExchange();
  }
  catch (ZdException & x) {
    x.AddCallpath("chkInclPurTempClustClick()", "TfrmAnalysis");
    DisplayBasisException(this, x);
  }
}
//------------------------------------------------------------------------------
// Simulated Log likelihood ratio set
//------------------------------------------------------------------------------
void __fastcall TfrmAnalysis::chkInclSimLogLikClick(TObject *Sender) {
  try {
    gpParams->m_bSaveSimLogLikelihoods = chkInclSimLogLik->Checked;
    DataExchange();
  }
  catch (ZdException & x) {
    x.AddCallpath("chkInclSimLogLikClick()", "TfrmAnalysis");
    DisplayBasisException(this, x);
  }
}
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void __fastcall TfrmAnalysis::chkLikelyClustersClick(TObject *Sender) {
   gpParams->m_bMostLikelyClusters = chkLikelyClusters->Checked;
}
//---------------------------------------------------------------------------
// This function mimics the Visual C++ inteface.  It changed the parameters
// of the Purely Spacial run so that the Interval Unit and length are zero.
//---------------------------------------------------------------------------
void TfrmAnalysis::ConvertPurelySpacialIntervals() {
  if (gpParams->m_nAnalysisType == PURELYSPATIAL) {
    gpParams->m_nIntervalUnits = NONE;
    gpParams->m_nIntervalLength = 0;
  }
}

// create the TXD file with the appropriate field names - AJV 8/29/2002
// pre: sFilename has a txd extension and vFieldNames has been filled with the appropraite field names
// post: will create a txd file with padded spaces delimiting the fields
void TfrmAnalysis::CreateTXDFile(const ZdFileName& sFileName, const ZdVector<const char*>& vFieldNames) {
   ZdVector<ZdField*>	        vFields;
   ZdField*		        pField = 0;
   TXDFile*                     pFile = 0;
   unsigned short               uwOffset = 0, uwLength = 128;

   try {
      // create a TXD file with a space delimiter
      pFile = new TXDFile();

      // creates the field vector from the provided field names
      for(unsigned long i = 0; i < vFieldNames.GetNumElements(); ++i) {
      	 pField = pFile->GetNewField();
         pField->SetName(vFieldNames[i]);
         if(!i)     // need a better system here to define which fields are required
            pField->SetRequired(true);
          // field 1 the only alpha field in the input so allow to greater width here, consider
          // other options for this in the future - AJV 9/4/2002
         pField->SetType(ZD_ALPHA_FLD);
         uwLength = (!i ? 200 : 50);      
         pField->SetOffset(uwOffset);
         pField->SetLength(uwLength);
         pField->SetIndexCount(0);
         vFields.AddElement(pField->Clone());
         delete pField; pField=0;
         // NOTE: Our original design would simply make gaps in the fields offsets
         //       to allow created ZdTXD file to work like current SaTScan data files.
         //       But, unfortunetly, those gaps would have value 0x00 causing problems
         //       for scanf. So, let's assume that there will be gaps between fields.
         //       I think this is a pretty good assumption for now considering the
         //       large field lengths we are defining and given sample data observed.
         uwOffset += ( uwLength );
      }

      // don't pack fields or else you'll lose the offset!!! AJV 9/5/2002
      // BUGBUG - we'll temporarily delete the txd file if it already exists - AJV 9/5/2002
      pFile->Delete(sFileName.GetFullPath());
      pFile->SetTitle(sFileName.GetFileName());
      pFile->Create(sFileName.GetFullPath(), vFields, 0);
      pFile->Close();

      for(int i = vFields.GetNumElements() - 1; i > 0; --i) {
         delete vFields[0]; vFields[0] = 0;
         vFields.RemoveElement(0);
      }

      delete pFile;
   }
   catch (ZdException &x) {
      if(pFile)
         pFile->Close();
      delete pFile; pFile = 0;
      delete pField; pField = 0;
      for(int i = vFields.GetNumElements() - 1; i > 0; --i) {
         delete vFields[0]; vFields[0] = 0;
         vFields.RemoveElement(0);
      }	 
      x.AddCallpath("CreateTXDFile()", "TfrmAnalysis");
      throw;
   }
}


//---------------------------------------------------------------------------
// THIS FUNCTIONS IS THE MAIN CONTROLLING FUNCTION FOR CHECKING RELATIONSHIPS
// AND TURNING ON AND OFF CONTROLS.  Each tab has an "Enable" function that
// checks the relationships pertaining to the controls on its particular tab.
// THERE ARE MANY MANY CONTROL RELATIONSHOPS THAT SPAN TABS !!!
//---------------------------------------------------------------------------
void TfrmAnalysis::DataExchange() {
  try {
    EnableScanningWindow();
    EnablePopulationFileInput();
    EnableTimeTrendAdj();
    EnableTimeIntervals();
    EnableProspStartDate();
    EnableAdditionalAsciiFileOptions();
    EnablePrecision();
    EnableScanningWindow();
  }
  catch (ZdException & x) {
    x.AddCallpath("DataExchange()", "TfrmAnalysis");
    throw;
  }
}
//---------------------------------------------------------------------------

bool TfrmAnalysis::DetermineIfDbfExtension(AnsiString sFileName) {
  bool bDbfStatus = false;

  try {
     bDbfStatus = ! std::strcmp(((sFileName.SubString(sFileName.Length() - 3, 4)).LowerCase()).c_str(), ".txd"/*".dbf"*/);
  }
  catch (ZdException & x) {
    x.AddCallpath("DetermineIfDbaseFile(AnsiString & sFileName)", "TfrmAnalysis");
    throw;
  }
   return bDbfStatus;
}
//---------------------------------------------------------------------------

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
    MessageBox(NULL, "The percentage per year value can not be blank.", "Parameter Error" , MB_OK);
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
  gpParams->m_nMaxGeographicClusterSize = atof(edtMaxClusterSize->Text.c_str());
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
  gpParams->m_nMaxTemporalClusterSize = atof(edtMaxTemporalClusterSize->Text.c_str());
}
//---------------------------------------------------------------------------
void __fastcall TfrmAnalysis::edtProspDayExit(TObject *Sender) {
  try {
    if ((atoi(edtProspDay->Text.c_str()) < 1) || (atoi(edtProspDay->Text.c_str()) > 31)) {
      MessageBox(NULL, "Please specify a valid day.", "Parameter Error" , MB_OK);
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
      MessageBox(NULL, "Please specify a valid day.", "Parameter Error" , MB_OK);
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
  if (!bParamsOk) {
    PageControl1->ActivePage = tbTimeParameter;
    edtUnitLength->SetFocus();
  }
}
//------------------------------------------------------------------------------
// Enables 'Additional Ascii File Options' based on current settings.
//---------------------------------------------------------------------------
void TfrmAnalysis::EnableAdditionalAsciiFileOptions() {
  chkInclRelRiskEst->Enabled = gpParams->m_nModel != SPACETIMEPERMUTATION;
  if (! chkInclRelRiskEst->Enabled)
     chkInclRelRiskEst->Checked = false;
}
//------------------------------------------------------------------------------
// Purely spacial runs are always available, but other times the other
// 3 options have to be turned off (or back on).
//---------------------------------------------------------------------------
void TfrmAnalysis::EnableAnalysisType(bool bValue) {
   TControl *ChildControl;
   ChildControl = rgTypeAnalysis->Controls[1];  //Purely Temporal
   ChildControl->Enabled = bValue;
   ChildControl = rgTypeAnalysis->Controls[2];  //Retrospective space-Time
   ChildControl->Enabled = bValue;
   ChildControl = rgTypeAnalysis->Controls[3];  //Prospective Space-Time
   ChildControl->Enabled = bValue;
}
/** Enables controls which permit user to specify population file. */
void TfrmAnalysis::EnablePopulationFileInput()
{
  edtPopFileName->Enabled = ((rgProbability->ItemIndex == SPACETIMEPERMUTATION && rdoSpatialDistance->Checked)/*percentage*/ ||
                             rgProbability->ItemIndex == BERNOULLI ? false : true);;
  btnPopBrowse->Enabled = edtPopFileName->Enabled;
}

//---------------------------------------------------------------------------
// Precision is enabled or disabled depending on the analysis type !!!
//---------------------------------------------------------------------------
void TfrmAnalysis::EnablePrecision() {
  try {
    if (gpParams->m_nAnalysisType == PURELYSPATIAL) {   // use to be 0
      TControl *ChildControl;
      ChildControl = rgPrecisionTimes->Controls[NONE];
      ChildControl->Enabled = true;
      ChildControl = rgPrecisionTimes->Controls[YEAR];
      ChildControl->Enabled = true;
      ChildControl = rgPrecisionTimes->Controls[MONTH];
      ChildControl->Enabled = true;
    }
    else {// Purely Temporal, Retrospective Space-Time, Prospective Space-Time
      TControl *ChildControl;
      ChildControl = rgPrecisionTimes->Controls[NONE];
      ChildControl->Enabled = false;
      ChildControl = rgPrecisionTimes->Controls[YEAR];
      ChildControl->Enabled = (gpParams->m_nIntervalUnits < 2);   // use to be 1
      ChildControl = rgPrecisionTimes->Controls[MONTH];
      ChildControl->Enabled = (gpParams->m_nIntervalUnits < 3);   // use to be 2
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("EnablePrecision()", "TfrmAnalysis");
    throw;
  }
}
//------------------------------------------------------------------------------
// Enable or disables the Prospective Start Date controls
//------------------------------------------------------------------------------
void TfrmAnalysis::EnableProspStartDate() {
  if (gpParams->m_nAnalysisType != PROSPECTIVESPACETIME) {
    edtProspYear->Enabled  = false;
    edtProspYear->Color    = clInactiveBorder;
    edtProspMonth->Enabled = false;
    edtProspMonth->Color   = clInactiveBorder;
    edtProspDay->Enabled   = false;
    edtProspDay->Color     = clInactiveBorder;
  }
  else {
    switch (rgPrecisionTimes->ItemIndex) {
      case NONE         : edtProspYear->Enabled  = true;  edtProspYear->Color = clWindow;
                          edtProspMonth->Enabled = true;  edtProspMonth->Color = clWindow;
                          edtProspDay->Enabled   = true;  edtProspDay->Color = clWindow;
                          break;
      case YEAR         : edtProspYear->Enabled  = true;  edtProspYear->Color = clWindow;
                          edtProspMonth->Enabled = false;  edtProspMonth->Color = clInactiveBorder;
                          edtProspDay->Enabled   = false;  edtProspDay->Color = clInactiveBorder;
                          edtProspMonth->Text = "1";
                          edtProspDay->Text = "1";
                          break;
      case MONTH        : edtProspYear->Enabled  = true;  edtProspYear->Color = clWindow;
                          edtProspMonth->Enabled = true;  edtProspMonth->Color = clWindow;
                          edtProspDay->Enabled   = false;  edtProspDay->Color = clInactiveBorder;
                          edtProspDay->Text = "1";
                          break;
      case DAY          : edtProspYear->Enabled  = true;  edtProspYear->Color = clWindow;
                          edtProspMonth->Enabled = true;  edtProspMonth->Color = clWindow;
                          edtProspDay->Enabled   = true;  edtProspDay->Color = clWindow;
                          break;
    }
  }
}
//------------------------------------------------------------------------------
// Main function to enable and disable Scanning tab controls
//------------------------------------------------------------------------------
void TfrmAnalysis::EnableScanningWindow() {
  // fixed the bSpacial boolean.  Use to be == 1 or == 2.  changed to == 1 or == 3
  bool bSpacial = (gpParams->m_nAnalysisType == PURELYSPATIAL) ||(gpParams->m_nAnalysisType == SPACETIME) || // PS, ST  use to be 0 and 1
                  (gpParams->m_nAnalysisType == PROSPECTIVESPACETIME) || (gpParams->m_nModel == SPACETIMEPERMUTATION);
  bool bTemporal = (gpParams->m_nAnalysisType == PURELYTEMPORAL) ||(gpParams->m_nAnalysisType == SPACETIME) || // ST, PT   use to be 1 and 2
                    (gpParams->m_nAnalysisType == PROSPECTIVESPACETIME) || (gpParams->m_nModel == SPACETIMEPERMUTATION);
  bool bTemporalNotCountingProspST = (gpParams->m_nAnalysisType == PURELYTEMPORAL) || (gpParams->m_nAnalysisType == SPACETIME);

  edtMaxClusterSize->Enabled =    bSpacial;
  rdoSpatialPercentage->Enabled = bSpacial;
  rdoSpatialDistance->Enabled = bSpacial;
  edtMaxClusterSize->Color = (bSpacial ? clWindow : clInactiveBorder);
  chkInclPurTempClust->Enabled =  (bSpacial && gpParams->m_nModel != SPACETIMEPERMUTATION && bTemporal);

  edtMaxTemporalClusterSize->Enabled = bTemporal;
  edtMaxTemporalClusterSize->Color = (bTemporal ? clWindow:clInactiveBorder);
  rdoPercentageTemproal->Enabled = bTemporal;
  rdoTimeTemproal->Enabled = bTemporal;
  if (rbUnitYear->Checked)
    rdoTimeTemproal->Caption = "Time in years";
  else if (rbUnitMonths->Checked)
    rdoTimeTemproal->Caption = "Time in months";
  else
    rdoTimeTemproal->Caption = "Time in days";
  chkIncludePurSpacClust->Enabled = (((gpParams->m_nAnalysisType == PURELYSPATIAL) ||
                                      (gpParams->m_nAnalysisType == SPACETIME)) &&
                                      bTemporal && gpParams->m_nModel != SPACETIMEPERMUTATION);

  rgClustersToInclude->Enabled = bTemporalNotCountingProspST;
}
//------------------------------------------------------------------------------
// Enables or disables time interval controls
//------------------------------------------------------------------------------
void TfrmAnalysis::EnableTimeIntervals() {
  bool bTemporal = (gpParams->m_nAnalysisType != PURELYSPATIAL); // PT, Retro S-T, Prosp S-T  use to be 1 and 2
  bool bEnableMo = (gpParams->m_nPrecision >= 2);
  bool bEnableDy = (gpParams->m_nPrecision >= 3);
  rbUnitYear->Enabled = bTemporal;
  rbUnitMonths->Enabled = (bTemporal && bEnableMo);
  rbUnitDay->Enabled = (bTemporal && bEnableDy);
  edtUnitLength->Enabled = bTemporal;
  edtUnitLength->Color = (bTemporal ? clWindow:clInactiveBorder);
}
//------------------------------------------------------------------------------
// Enables ro disables Time trend controls
//------------------------------------------------------------------------------
void TfrmAnalysis::EnableTimeTrendAdj() {
  bool bTemporal = (gpParams->m_nAnalysisType != PURELYSPATIAL);// PT, Retro ST, Pro ST   --- use to be 1 and 2
  TControl *ChildControl;

  try {
    rgTemporalTrendAdj->Enabled = true;
    if (gpParams->m_nModel == POISSON) {
      ChildControl = rgTemporalTrendAdj->Controls[0];  //NONE
      ChildControl->Enabled = bTemporal;
      //NON-PARAMETRIC
      ChildControl = rgTemporalTrendAdj->Controls[1];  //NON-PARAMETRIC
      ChildControl->Enabled = ((gpParams->m_nAnalysisType != PURELYSPATIAL) && (gpParams->m_nAnalysisType != PURELYTEMPORAL)); //(bTemporal && (gpParams->m_nAnalysisType != PURELYTEMPORAL));  //
      //ChildControl->Enabled = (bTemporal && (gpParams->m_nAnalysisType != SPACETIME) && (gpParams->m_nAnalysisType != PROSPECTIVESPACETIME));  // use to be 2
      //LOG LINEAR
      ChildControl = rgTemporalTrendAdj->Controls[2];  //LOG LINEAR
      ChildControl->Enabled = bTemporal;
      //LOG LINEAR PERCENTAGE
      edtLogPerYear->Enabled = (bTemporal && gpParams->m_nTimeAdjustType == LINEAR);
      edtLogPerYear->Color = ((bTemporal && gpParams->m_nTimeAdjustType == LINEAR) ? clWindow:clInactiveBorder);
    }
    else if (gpParams->m_nModel == BERNOULLI ||gpParams->m_nModel == SPACETIMEPERMUTATION) {
      ChildControl = rgTemporalTrendAdj->Controls[0];  //NONE
      ChildControl->Enabled = false;
      //NON-PARAMETRIC
      ChildControl = rgTemporalTrendAdj->Controls[1];  //NON-PARAMETRIC
      ChildControl->Enabled = false;
      //LOG LINEAR
      ChildControl = rgTemporalTrendAdj->Controls[2];  //LOG LINEAR
      ChildControl->Enabled = false;
      //LOG LINEAR PERCENTAGE
      edtLogPerYear->Enabled = false;
      edtLogPerYear->Color = clInactiveBorder;
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("EnableTimeTrendAdj()", "TfrmAnalysis");
    throw;
  }
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
//------------------------------------------------------------------------------
void __fastcall TfrmAnalysis::NaturalNumberKeyPress(TObject *Sender, char &Key) {
  if (!strchr("0123456789\b",Key))
    Key = 0;
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
        pYear->Text  = thePart.c_str();
        theDate.Delete(1, iLoc);
        iLoc = theDate.Pos("/");
        if (iLoc == 0)
          ZdException::GenerateNotification("Invalid date found in parameter file.", "ParseDate()");
        else {
          thePart = theDate.SubString(1,iLoc-1);
          pMonth->Text = thePart.c_str();
          theDate.Delete(1, iLoc);
          pDay->Text   = theDate.c_str();
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
//------------------------------------------------------------------------------
// Specific Day unit control
//------------------------------------------------------------------------------
void __fastcall TfrmAnalysis::rbUnitDayClick(TObject *Sender) {
  try {
    gpParams->m_nIntervalUnits = 3; // use to be 2
    DataExchange();
  }
  catch (ZdException & x) {
    x.AddCallpath("rbUnitDayClick()", "TfrmAnalysis");
    DisplayBasisException(this, x);
   }
}
//------------------------------------------------------------------------------
// Specific Months unit control
//------------------------------------------------------------------------------
void __fastcall TfrmAnalysis::rbUnitMonthsClick(TObject *Sender) {
  try {
    gpParams->m_nIntervalUnits = 2; // use to be 1
    DataExchange();
  }
  catch (ZdException & x) {
    x.AddCallpath("rbUnitMonthsClick()", "TfrmAnalysis");
    DisplayBasisException(this, x);
  }
}
//------------------------------------------------------------------------------
// Specific Year unit control
//------------------------------------------------------------------------------
void __fastcall TfrmAnalysis::rbUnitYearClick(TObject *Sender) {
  try {
    gpParams->m_nIntervalUnits = 1; // use to be 0
    DataExchange();
  }
  catch (ZdException & x) {
    x.AddCallpath("rbUnitYearClick()", "TfrmAnalysis");
    DisplayBasisException(this, x);
  }
}
//------------------------------------------------------------------------------
//  Control to include "Alive" clusters
//------------------------------------------------------------------------------
void __fastcall TfrmAnalysis::rgClustersToIncludeClick(TObject *Sender) {
  try {
    gpParams->m_bAliveClustersOnly = (rgClustersToInclude->ItemIndex == 0 ? 0:1);
    DataExchange();
  }
  catch (ZdException & x) {
    x.AddCallpath("rgClustersToIncludeClick()", "TfrmAnalysis");
    DisplayBasisException(this, x);
  }
}
//------------------------------------------------------------------------------
// If the types of coordinates are changed, then various interface options
// need to be updated
//------------------------------------------------------------------------------
void __fastcall TfrmAnalysis::rgCoordinatesClick(TObject *Sender) {
  try {
    gpParams->m_nCoordType = rgCoordinates->ItemIndex;
    SetSpatialDistanceCaption();
    DataExchange();
  }
  catch (ZdException & x) {
    x.AddCallpath("rgCoordinatesClick()", "TfrmAnalysis");
    DisplayBasisException(this, x);
  }
}
//---------------------------------------------------------------------------

//  When the time precision control is changed, various interace options are
//  toggled and changed.
//---------------------------------------------------------------------------
void __fastcall TfrmAnalysis::rgPrecisionTimesClick(TObject *Sender) {
  try {
    gpParams->m_nPrecision = rgPrecisionTimes->ItemIndex;
    switch (rgPrecisionTimes->ItemIndex) {
      case NONE                 : edtStartYear->Enabled  = true;  edtStartYear->Color = clWindow;
                                  edtStartMonth->Enabled = true;  edtStartMonth->Color = clWindow;
                                  edtStartDay->Enabled   = true;  edtStartDay->Color = clWindow;
                                  edtEndYear->Enabled    = true;  edtEndYear->Color = clWindow;
                                  edtEndMonth->Enabled   = true;  edtEndMonth->Color = clWindow;
                                  edtEndDay->Enabled     = true;  edtEndDay->Color = clWindow;
                                  //prospective space-time start date...
                                  if (gpParams->m_nAnalysisType == PROSPECTIVESPACETIME) {
                                    edtProspYear->Enabled  = true;  edtProspYear->Color = clWindow;
                                    edtProspMonth->Enabled = true;  edtProspMonth->Color = clWindow;
                                    edtProspDay->Enabled   = true;  edtProspDay->Color = clWindow;
                                  }
                                  EnableAnalysisType(false);
                                  break;
      case YEAR                 : edtStartYear->Enabled  = true;   edtStartYear->Color = clWindow;
                                  edtStartMonth->Enabled = false;  edtStartMonth->Color = clInactiveBorder;
                                  edtStartDay->Enabled   = false;  edtStartDay->Color = clInactiveBorder;
                                  edtEndYear->Enabled    = true;   edtEndYear->Color = clWindow;
                                  edtEndMonth->Enabled   = false;  edtEndMonth->Color = clInactiveBorder;
                                  edtEndDay->Enabled     = false;  edtEndDay->Color = clInactiveBorder;
                                  edtStartMonth->Text="1";
                                  edtStartDay->Text = "1";
                                  edtEndMonth->Text = "12";
                                  edtEndDay->Text = "31";
                                  //prospective space-time start date...
                                  if (gpParams->m_nAnalysisType == PROSPECTIVESPACETIME) {
                                    edtProspYear->Enabled  = true;  edtProspYear->Color = clWindow;
                                    edtProspMonth->Enabled = false;  edtProspMonth->Color = clInactiveBorder;
                                    edtProspDay->Enabled   = false;  edtProspDay->Color = clInactiveBorder;
                                    edtProspMonth->Text = "1";
                                    edtProspDay->Text = "1";
                                  }
                                  EnableAnalysisType(true);
                                  break;
      case MONTH                : edtStartYear->Enabled  = true;   edtStartYear->Color = clWindow;
                                  edtStartMonth->Enabled = true;   edtStartMonth->Color = clWindow;
                                  edtStartDay->Enabled   = false;  edtStartDay->Color = clInactiveBorder;
                                  edtEndYear->Enabled    = true;   edtEndYear->Color = clWindow;
                                  edtEndMonth->Enabled   = true;   edtEndMonth->Color = clWindow;
                                  edtEndDay->Enabled     = false;  edtEndDay->Color = clInactiveBorder;
                                  edtEndDay->Text = DaysThisMonth(atoi(edtEndYear->Text.c_str()), atoi(edtEndMonth->Text.c_str()));
                                  edtStartDay->Text = "1";
                                  //prospective space-time start date...
                                  if (gpParams->m_nAnalysisType == PROSPECTIVESPACETIME) {
                                    edtProspYear->Enabled  = true;  edtProspYear->Color = clWindow;
                                    edtProspMonth->Enabled = true;  edtProspMonth->Color = clWindow;
                                    edtProspDay->Enabled   = false;  edtProspDay->Color = clInactiveBorder;
                                    edtProspDay->Text = "1";
                                  }
                                  EnableAnalysisType(true);
                                  break;
      case DAY                  : edtStartYear->Enabled  = true;  edtStartYear->Color = clWindow;
                                  edtStartMonth->Enabled = true;  edtStartMonth->Color = clWindow;
                                  edtStartDay->Enabled   = true;  edtStartDay->Color = clWindow;
                                  edtEndYear->Enabled    = true;  edtEndYear->Color = clWindow;
                                  edtEndMonth->Enabled   = true;  edtEndMonth->Color = clWindow;
                                  edtEndDay->Enabled     = true;  edtEndDay->Color = clWindow;
                                  //prospective space-time start date...
                                  if (gpParams->m_nAnalysisType == PROSPECTIVESPACETIME) {
                                    edtProspYear->Enabled  = true;  edtProspYear->Color = clWindow;
                                    edtProspMonth->Enabled = true;  edtProspMonth->Color = clWindow;
                                    edtProspDay->Enabled   = true;  edtProspDay->Color = clWindow;
                                  }
                                  EnableAnalysisType(true);
                                  break;

    }
    //*************************************
    // this is the OnPrecisionChange() function
    if (gpParams->m_nAnalysisType == PURELYSPATIAL && gpParams->m_nPrecision != 0) {  // use to be m_nAnalysisType == 0
      if (gpParams->m_nIntervalUnits > gpParams->m_nPrecision )
        gpParams->m_nIntervalUnits = gpParams->m_nPrecision;
      //THIS USE TO BE..
      //if (gpParams->m_nIntervalUnits > gpParams->m_nPrecision -1)
      //   gpParams->m_nIntervalUnits = gpParams->m_nPrecision - 1;
    }
    DataExchange();
  }
  catch (ZdException & x) {
    x.AddCallpath("rgPrecisionTimesClick()", "TfrmAnalysis");
    throw;
  }
}
//------------------------------------------------------------------------------
// Probability is changed.  Need to perform a "dataexchange" to see if any
// other tab controls need to be enabled or disabled
//------------------------------------------------------------------------------
void __fastcall TfrmAnalysis::rgProbabilityClick(TObject *Sender) {
  try {
    gpParams->m_nModel = rgProbability->ItemIndex;
    //enable buttons based on selected model
    rgTypeAnalysis->Controls[0]->Enabled = ( rgProbability->ItemIndex == SPACETIMEPERMUTATION ? false : true );
    rgTypeAnalysis->Controls[1]->Enabled = ( rgProbability->ItemIndex == SPACETIMEPERMUTATION ? false : true );
    EnablePopulationFileInput();
    edtControlFileName->Enabled = ( rgProbability->ItemIndex == SPACETIMEPERMUTATION || rgProbability->ItemIndex == POISSON ? false : true );;
    btnControlBrowse->Enabled = ( rgProbability->ItemIndex == SPACETIMEPERMUTATION || rgProbability->ItemIndex == POISSON ? false : true );
    // indicate that for Space-Time Permutation model, max temporal clusters size is 50.
    if (rgProbability->ItemIndex == SPACETIMEPERMUTATION)
      rdoPercentageTemproal->Caption = "Percentage (<= 50%)";
    else
      rdoPercentageTemproal->Caption = "Percentage (<= 90%)";
    if ( rgProbability->ItemIndex == SPACETIMEPERMUTATION && !( rgTypeAnalysis->ItemIndex + 1  == SPACETIME || rgTypeAnalysis->ItemIndex + 1 == PROSPECTIVESPACETIME ) )
      //Space-Time Permutation model only valid for space-time analysis types.
      //If analysis isn't space-time, then set to PROSPECTIVESPACETIME by default.
      //Note that setting rgTypeAnalysis->ItemIndex triggers event, which in turn calls DataExchange.
      rgTypeAnalysis->ItemIndex = PROSPECTIVESPACETIME;
    else
      DataExchange();
  }
  catch ( ZdException & x ) {
    x.AddCallpath( "rgProbabilityClick()", "TfrmAnalysis" );
    DisplayBasisException( this, x );
  }
}
//------------------------------------------------------------------------------
// Scan Areas is changed.  Need to perform a "dataexchange" to see if any
// other tab controls need to be enabled or disabled
//------------------------------------------------------------------------------
void __fastcall TfrmAnalysis::rgScanAreasClick(TObject *Sender) {
  try {
    gpParams->m_nAreas = rgScanAreas->ItemIndex + 1;
    DataExchange();
  }
  catch (ZdException & x) {
    x.AddCallpath("rgScanAreasClick()", "TfrmAnalysis");
    DisplayBasisException(this, x);
  }
}
//------------------------------------------------------------------------------
// Temporal Time Trend control
//------------------------------------------------------------------------------
void __fastcall TfrmAnalysis::rgTemporalTrendAdjClick(TObject *Sender) {
  gpParams->m_nTimeAdjustType = rgTemporalTrendAdj->ItemIndex;

  try {
    if (rgTemporalTrendAdj->ItemIndex == 0) {      // None
      edtLogPerYear->Enabled = false;
      edtLogPerYear->Color = clInactiveBorder;
    }
    else if (rgTemporalTrendAdj->ItemIndex == 1) { // NonParametric
      edtLogPerYear->Enabled = false;                                              
      edtLogPerYear->Color = clInactiveBorder;
    }
    else if (rgTemporalTrendAdj->ItemIndex == 2) { // Log Linear
      edtLogPerYear->Enabled = true;
      edtLogPerYear->Color = clWindow;
    }
    DataExchange();
  }
  catch (ZdException & x) {
    x.AddCallpath("rgTemporalTrendAdjClick()", "TfrmAnalysis");
    DisplayBasisException(this, x);
   }
}
//------------------------------------------------------------------------------
// Type of Analsyis is changed
//------------------------------------------------------------------------------
void __fastcall TfrmAnalysis::rgTypeAnalysisClick(TObject *Sender) {
  try {
    gpParams->m_nAnalysisType = rgTypeAnalysis->ItemIndex + 1;

    if (gpParams->m_nAnalysisType == PURELYTEMPORAL) {
      if (gpParams->m_nTimeAdjustType == 1) {
        gpParams->m_nTimeAdjustType = 0; //discrete adjustment not valid for P.T. analysis
        //update interface
        rgTemporalTrendAdj->ItemIndex = gpParams->m_nTimeAdjustType;
      }
      TControl *ChildControl;
      ChildControl = rgTemporalTrendAdj->Controls[1];
      ChildControl->Enabled = false;
    }
    else {// enable non-parametric
      TControl *ChildControl;
      ChildControl = rgTemporalTrendAdj->Controls[1];
      ChildControl->Enabled = true;
    }
    if (gpParams->m_nAnalysisType == PROSPECTIVESPACETIME) {
      EnableProspStartDate();
      //DISABLE the Include Purely Spacial Clusters option.
      chkIncludePurSpacClust->Checked = false;
      chkIncludePurSpacClust->Enabled = false;
    }
    else // make sure that the Include Purely Spacial Cluster option in enabled
      chkIncludePurSpacClust->Enabled = false;

    DataExchange();
  }
  catch (ZdException & x) {
    x.AddCallpath("rgTypeAnalysisClick()", "TfrmAnalysis");
    DisplayBasisException(this, x);
  }
}
//---------------------------------------------------------------------------
// main function to save a parameter file AS
//---------------------------------------------------------------------------
void TfrmAnalysis::SaveAs() {
  try {
    if (SaveDialog->Execute()) {
      gsParamFileName = SaveDialog->FileName.c_str();
      frmAnalysis->Caption = SaveDialog->FileName.c_str();
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
    gpParams->m_nMaxTemporalClusterSize    = atof(edtMaxTemporalClusterSize->Text.c_str());
    gpParams->m_nMaxClusterSizeType = (rdoPercentageTemproal->Checked ? PERCENTAGETYPE : TIMETYPE);
    //Time Parameter Tab
    gpParams->m_nIntervalLength  = atoi(edtUnitLength->Text.c_str());
    gpParams->m_nTimeAdjPercent = atof(edtLogPerYear->Text.c_str());
    sprintf(gpParams->m_szProspStartDate, "%i/%i/%i", atoi(edtProspYear->Text.c_str()), atoi(edtProspMonth->Text.c_str()), atoi(edtProspDay->Text.c_str()));
    //Output File Tab
    strcpy(gpParams->m_szOutputFilename, edtResultFile->Text.c_str());

    gpParams->SetOutputClusterLevelDBF(chkDbaseFile1->Checked);
    gpParams->SetOutputAreaSpecificDBF(chkDbaseFile2->Checked);
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
       case 0  : rdoSpatialDistance->Caption = "Distance (Cartesian units)";
                 break;
       case 1  : rdoSpatialDistance->Caption = "Distance (in kilometers)";
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
   try {
      gvCaseFileFieldDescriptors.AddElement("Tract ID");
      gvCaseFileFieldDescriptors.AddElement("Number of Cases");
      gvCaseFileFieldDescriptors.AddElement("Date/Time");
      gvCaseFileFieldDescriptors.AddElement("Number of Controls");
      gvCaseFileFieldDescriptors.AddElement("Covariant1");
      gvCaseFileFieldDescriptors.AddElement("Covariant2");
      gvCaseFileFieldDescriptors.AddElement("Covariant3");
      gvCaseFileFieldDescriptors.AddElement("Covariant4");
      gvCaseFileFieldDescriptors.AddElement("Covariant5");
      gvCaseFileFieldDescriptors.AddElement("Covariant6");
      gvCaseFileFieldDescriptors.AddElement("Covariant7");
      gvCaseFileFieldDescriptors.AddElement("Covariant8");
      gvCaseFileFieldDescriptors.AddElement("Covariant9");
      gvCaseFileFieldDescriptors.AddElement("Covariant10");
   }
   catch (ZdException &x) {
      x.AddCallpath("SetupCaseFileFieldDescriptors()", "TfrmAnalysis");
      throw;
   }
}

// fill the control File field descriptor vector with the appropriate field names for a control file
void TfrmAnalysis::SetupControlFileFieldDescriptors() {
   try {
      gvControlFileFieldDescriptors.AddElement("Tract ID");
      gvControlFileFieldDescriptors.AddElement("Number of Cases");
      gvControlFileFieldDescriptors.AddElement("Date/Time");
      gvControlFileFieldDescriptors.AddElement("Number of Controls");
      gvControlFileFieldDescriptors.AddElement("Covariant1");
      gvControlFileFieldDescriptors.AddElement("Covariant2");
      gvControlFileFieldDescriptors.AddElement("Covariant3");
      gvControlFileFieldDescriptors.AddElement("Covariant4");
      gvControlFileFieldDescriptors.AddElement("Covariant5");
      gvControlFileFieldDescriptors.AddElement("Covariant6");
      gvControlFileFieldDescriptors.AddElement("Covariant7");
      gvControlFileFieldDescriptors.AddElement("Covariant8");
      gvControlFileFieldDescriptors.AddElement("Covariant9");
      gvControlFileFieldDescriptors.AddElement("Covariant10");
   }
   catch (ZdException &x) {
      x.AddCallpath("SetupControlFileFieldDescriptors()", "TfrmAnalysis");
      throw;
   }
}

// fill the Geo File field descriptor vector with the appropriate field names for a geo file
void TfrmAnalysis::SetupGeoFileFieldDescriptors() {
   try {
      gvGeoFileFieldDescriptors.AddElement("Tract ID");
      gvGeoFileFieldDescriptors.AddElement("Longitude");
      gvGeoFileFieldDescriptors.AddElement("Latitude");
      gvGeoFileFieldDescriptors.AddElement("Dimension1");
      gvGeoFileFieldDescriptors.AddElement("Dimension2");
      gvGeoFileFieldDescriptors.AddElement("Dimension3");
      gvGeoFileFieldDescriptors.AddElement("Dimension4");
      gvGeoFileFieldDescriptors.AddElement("Dimension5");
      gvGeoFileFieldDescriptors.AddElement("Dimension6");
      gvGeoFileFieldDescriptors.AddElement("Dimension7");
      gvGeoFileFieldDescriptors.AddElement("Dimension8");
      gvGeoFileFieldDescriptors.AddElement("Dimension9");
      gvGeoFileFieldDescriptors.AddElement("Dimension10");
   }
   catch (ZdException &x) {
      x.AddCallpath("SetupGeoFileFieldDescriptors()", "TfrmAnalysis");
      throw;
   }
}

// fill the Geo File field descriptor vector with the appropriate field names for a geo file
void TfrmAnalysis::SetupGridFileFieldDescriptors() {
   try {
      gvGridFileFieldDescriptors.AddElement("Longitude");
      gvGridFileFieldDescriptors.AddElement("Latitude");
      gvGridFileFieldDescriptors.AddElement("Dimension1");
      gvGridFileFieldDescriptors.AddElement("Dimension2");
      gvGridFileFieldDescriptors.AddElement("Dimension3");
      gvGridFileFieldDescriptors.AddElement("Dimension4");
   }
   catch (ZdException &x) {
      x.AddCallpath("SetupGridFileFieldDescriptors()", "TfrmAnalysis");
      throw;
   }
}

// sets up the appropriate options for the FTFImportDescriptor
// pre: create and pass in a default FTFDescriptor and a .dbf ImportFileName
// post: function will modify the FTFDescriptor to contain the appropraite import options for our purposes
void TfrmAnalysis::SetupImportDescriptor(BFTFImportDescriptor& descrip, const ZdString& sImportFileName) {
   ZdString     sDestFile;

   try {
      descrip.SetDestinationType(BFileDestDescriptor::SingleFile);
      descrip.SetModifyType(BFileDestDescriptor::OverWriteExistingData);
      descrip.SetImportFile(sImportFileName);
//      sDestFile = sImportFileName;
      // these here should probably be defines later on, also there should be some type of checking to make
      // sure that we are getting a dbf extension file - AJV 8/30/2002
//      sDestFile.Replace(".dbf", ".txd");
//      descrip.SetDestinationFile(sDestFile);
   }
   catch (ZdException &x) {
      x.AddCallpath("SetupImportDescriptor()", "TfrmAnalysis");
      throw;
   }
}

// fill the Geo File field descriptor vector with the appropriate field names for a geo file
void TfrmAnalysis::SetupPopFileFieldDescriptors() {
   try {
      gvPopFileFieldDescriptors.AddElement("Tract ID");
      gvPopFileFieldDescriptors.AddElement("Date/Time");
      gvPopFileFieldDescriptors.AddElement("Population");
      gvPopFileFieldDescriptors.AddElement("Covariant1");
      gvPopFileFieldDescriptors.AddElement("Covariant2");
      gvPopFileFieldDescriptors.AddElement("Covariant3");
   }
   catch (ZdException &x) {
      x.AddCallpath("SetupPopFileFieldDescriptors()", "TfrmAnalysis");
      throw;
   }
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
    rdoSpatialPercentage->Checked = gpParams->m_nMaxSpatialClusterSizeType == PERCENTAGEOFMEASURETYPE;
    rdoSpatialDistance->Checked = gpParams->m_nMaxSpatialClusterSizeType == DISTANCETYPE;
    EnablePopulationFileInput();
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
    chkInclRelRiskEst->Checked = gpParams->m_bOutputRelRisks;
    chkInclSimLogLik->Checked  = gpParams->m_bSaveSimLogLikelihoods;
    chkCensusAreas->Checked    = gpParams->m_bOutputCensusAreas;  // Output Census areas in Reported Clusters
    chkLikelyClusters->Checked = gpParams->m_bMostLikelyClusters;  // Output Most Likely Cluster for each Centroid
    cboCriteriaSecClusters->ItemIndex = gpParams->m_iCriteriaSecondClusters;
    //now enable or disable controls appropriately
    DataExchange();
  }
  catch (ZdException & x) {
    x.AddCallpath("SetupInterface()", "TfrmAnalysis");
    throw;
  }
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
//---------------------------------------------------------------------------
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
        MessageBox(NULL, "Please specify a case file.", "Parameter Error" , MB_OK);
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
        MessageBox(NULL, "Please specify a control file.", "Parameter Error" , MB_OK);
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
        MessageBox(NULL, "Please specify a population file.", "Parameter Error" , MB_OK);
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
        MessageBox(NULL, "Please specify a coordinates file.", "Parameter Error" , MB_OK);
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
    DisplayBasisException(this, x);
  }
  return bDataOk;
}

bool TfrmAnalysis::ValidateSpatialClusterSize() {
  double dValue = atof(edtMaxClusterSize->Text.c_str());
  bool   bOkParams=true;

  try {
    if (! edtMaxClusterSize->Text.Length()) {
      PageControl1->ActivePage = tbScanningWindow;
      edtMaxClusterSize->SetFocus();
      ZdException::GenerateNotification("Please specify maximum geographic size.", "ValidateSpatialClusterSize()");
    }

    if (!(dValue > 0.0 && dValue <= 50.0) && rdoSpatialPercentage->Checked) {
      ZdException::GenerateNotification("Please specify valid maximum geographic size between %d - %d.",
                                        "ValidateSpatialClusterSize()", 0, 50);
      PageControl1->ActivePage = tbScanningWindow;
      edtMaxClusterSize->SetFocus();
    }
    else
      gpParams->m_nMaxGeographicClusterSize = atof(edtMaxClusterSize->Text.c_str());
  }
  catch (ZdException & x) {
    x.AddCallpath("ValidateSpatialClusterSize()", "TfrmAnalysis");
    bOkParams = false;
    DisplayBasisException(this, x);
  }
  return bOkParams;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
bool TfrmAnalysis::ValidateTemoralClusterSize() {
  bool          bParamsOk = true;
  float         dValue, dTimeBetween, dMaxValue;
  AnsiString    sMessage;

  try {
    if (edtMaxTemporalClusterSize->Enabled) {
      if (rdoPercentageTemproal->Checked) {
        dMaxValue = (rgProbability->ItemIndex == SPACETIMEPERMUTATION ? 50 : 90);
        dValue = atof(edtMaxTemporalClusterSize->Text.c_str());
        if (!(dValue > 0.0 && dValue <= dMaxValue))
          ZdException::GenerateNotification("Please specify valid maximum time size between %d - %.0f", "ValidateTemoralClusterSize()", 0, dMaxValue);
      }
      else if (rdoTimeTemproal->Checked) {
        dMaxValue = 90; 
        if (atof(edtUnitLength->Text.c_str()) <= atof(edtMaxTemporalClusterSize->Text.c_str())) {
          dTimeBetween = TimeBetween(CharToJulian(gpParams->m_szStartDate),CharToJulian(gpParams->m_szEndDate),gpParams->m_nIntervalUnits);
          dValue = atof(edtMaxTemporalClusterSize->Text.c_str());
          if (dTimeBetween <= 0 || dValue > dTimeBetween*(dMaxValue/100))
            ZdException::GenerateNotification("Maximum temporal cluster size must be less than %d of duration of study period.",
                                              "ValidateTemoralClusterSize()", dMaxValue);
          if (floor(dValue/dTimeBetween*100) < 1) {
            sMessage = "Invalid maximum temoral cluster size specified.\nWith study period spanning from year ";
            sMessage += gpParams->m_szStartDate;
            sMessage += " to year ";
            sMessage += gpParams->m_szEndDate;
            sMessage += " ,\nmaximum cluster size of ";
            sMessage += dValue;
            if (rbUnitYear->Checked)
              sMessage += " year(s) ";
            if (rbUnitMonths->Checked)
              sMessage += " month(s) ";
            if (rbUnitDay->Checked)
              sMessage += " days(s) ";
            sMessage += " is less than 1 percent of study period.";
            ZdException::GenerateNotification(sMessage.c_str(), "ValidateTemoralClusterSize()");
          }
        }
        else
          ZdException::GenerateNotification("Maximum temporal cluster size must be greater than interval length.",
                                            "ValidateTemoralClusterSize()");
      }
      else
        ZdException::GenerateNotification("Type specified as neither percentage nor time.",
                                            "ValidateTemoralClusterSize()");
    }
  }
  catch (ZdException & x) {
    x.AddCallpath("ValidateTemoralClusterSize()", "TfrmAnalysis");
    bParamsOk = false;
    PageControl1->ActivePage = tbScanningWindow;
    edtMaxTemporalClusterSize->SetFocus();
    MessageBox( NULL, x.GetErrorMessage(), "Notification", MB_OK );
  }
  return bParamsOk;
}

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
  try {
    EnablePopulationFileInput();
    gpParams->m_nMaxSpatialClusterSizeType = PERCENTAGEOFMEASURETYPE;
  }
  catch (ZdException & x) {
    x.AddCallpath("rdoSpatialPercentageClick()", "TfrmAnalysis");
    DisplayBasisException(this, x);
  }
}
//---------------------------------------------------------------------------
void __fastcall TfrmAnalysis::rdoSpatialDistanceClick(TObject *Sender){
  try {
    EnablePopulationFileInput();
    gpParams->m_nMaxSpatialClusterSizeType = DISTANCETYPE;
  }
  catch (ZdException & x) {
    x.AddCallpath("rdoSpatialDistanceClick()", "TfrmAnalysis");
    DisplayBasisException(this, x);
  }
}
//---------------------------------------------------------------------------

