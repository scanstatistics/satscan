//---------------------------------------------------------------------------
#include "stsSaTScan.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TfrmAnalysis *frmAnalysis;

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
// Class:: TfrmAnalysis
//
//  This class contains all the main interface controls and relationships.
//  Since it the main session interface is a tab dialog, decided to keep
//  everything in one class and one cpp.
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// Constructor for TfrmAnalysis
//   if a parameter file is passed it, it will parse it (read it)
//   and set up the interface.
//---------------------------------------------------------------------------
__fastcall TfrmAnalysis::TfrmAnalysis(TComponent* Owner, char *sParamFileName)
        : TForm(Owner)
{
   try
      {
      PageControl1->ActivePage = tbInputFiles;
      gpParams = new CParameters(false);
      if (sParamFileName)
         {
         gsParamFileName = sParamFileName;
         if ( ! ReadSession(sParamFileName))
            Close();
         }
      else
         SetupInterface();   
      }
   catch (SSException & x)
      {
      x.AddCallpath("constructor", "TfrmAnalysis");
      DisplayBasisException(x);
      }
}
//---------------------------------------------------------------------------
// Closes the form and frees up all memory associated with the session obj
//---------------------------------------------------------------------------
void __fastcall TfrmAnalysis::FormClose(TObject *Sender,
      TCloseAction &Action)
{
   if (gpParams)
      delete gpParams;
   Action = caFree;
}
//---------------------------------------------------------------------------
// THIS FUNCTIONS IS THE MAIN CONTROLLING FUNCTION FOR CHECKING RELATIONSHIPS
// AND TURNING ON AND OFF CONTROLS.  Each tab has an "Enable" function that
// checks the relationships pertaining to the controls on its particular tab.
// THERE ARE MANY MANY CONTROL RELATIONSHOPS THAT SPAN TABS !!!
//---------------------------------------------------------------------------
void TfrmAnalysis::DataExchange()
{
   try
      {
      //EnableFiles();  commented out in MS C++ SatScan interface
      EnableScanningWindow();

      //time tab stuff
      EnableTimeTrendAdj();
      EnableTimeIntervals();
      EnableProspStartDate();

      EnablePrecision();
      EnableScanningWindow();
      }
   catch (SSException & x)
      {
      x.AddCallpath("DataExchange", "TfrmAnalysis");
      throw;
      }
}
//---------------------------------------------------------------------------
// This function is used right before a job is submitted.  Verifies that
// all the input files exist and can be read.  Also checks each tab to see
// if all settings are in place.
//---------------------------------------------------------------------------
bool TfrmAnalysis::ValidateParams()
{
  bool bDataOk;

  try
     {
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
   catch (SSException & x)
      {
      x.AddCallpath("ValidateParams", "TfrmAnalysis");
      DisplayBasisException(x);
      }
  return bDataOk;
}
//---------------------------------------------------------------------------
// returns the class global gpParams
//---------------------------------------------------------------------------
CParameters * TfrmAnalysis::GetSession()
{
   try
      {
      SaveTextParameters();
      //ConvertPurelySpacialIntervals(); // just to match Visual C++ conversions
      }
   catch (SSException & x)
      {
      x.AddCallpath("GetSession", "TfrmAnalysis");
      DisplayBasisException(x);
      }
   return gpParams;
}
char * TfrmAnalysis::GetFileName()
{
   return gsParamFileName.c_str();
}
//---------------------------------------------------------------------------
// This function mimics the Visual C++ inteface.  It changed the parameters
// of the Purely Spacial run so that the Interval Unit and length are zero.
//---------------------------------------------------------------------------
void TfrmAnalysis::ConvertPurelySpacialIntervals()
{
   if (gpParams->m_nAnalysisType == PURELYSPATIAL)
      {
      gpParams->m_nIntervalUnits = NONE;
      gpParams->m_nIntervalLength = 0;
      }
}
//---------------------------------------------------------------------------
// This is the main function to be called when reading a parameter file.
//  SetParameters(sFileName) will read the parameter file into the session
// parameter object and then SetupInterface sets the controls with the
// session obj information.
//---------------------------------------------------------------------------
bool TfrmAnalysis::ReadSession(char *sFileName)
{
   bool bSessionOk = true;

   try
      {
      //gpParams->Clear();

      /* if ( ! gpParams->SetParameters(sFileName))
         {
         Application->MessageBox("The parameter file you are trying to open is corrupt or is not really a parameter file.", "Error", MB_OK);
         bSessionOk = false;
         }
      else
         SetupInterface();  */

      gpParams->SetParameters(sFileName);
      SetupInterface();
      }
   catch (SSException & x)
      {
      x.AddCallpath("ReadSession", "TfrmAnalysis");
      DisplayBasisException(x);
      }
   return bSessionOk;
}
//---------------------------------------------------------------------------
// Writes the session information to disk
//---------------------------------------------------------------------------
bool TfrmAnalysis::WriteSession()
{
   bool bWriteOk = false;

   try
      {
      if (! gsParamFileName.IsEmpty())
         {
         SaveTextParameters();
         bWriteOk = gpParams->SaveParameters(gsParamFileName.c_str());
         }
      }
   catch (SSException & x)
      {
      x.AddCallpath("WriteSession", "TfrmAnalysis");
      DisplayBasisException(x);
      }
   return bWriteOk;   
}
//---------------------------------------------------------------------------
// main function to save a parameter file AS
//---------------------------------------------------------------------------
bool TfrmAnalysis::SaveAs()
{
   bool          bGotFileName = false;

   try 
      {
      SaveDialog1->DefaultExt = "*.prm";
      SaveDialog1->Filter = "Parameter Files (*.prm)|*.prm";
      SaveDialog1->Options >> ofOverwritePrompt >> ofHideReadOnly;
      SaveDialog1->Title = "Save Parameter File As";

      while (!bGotFileName && SaveDialog1->Execute() == IDOK)
         {
         //FileName = SaveDialog1->FileName.c_str();
         //if (FileName.GetLength() > 0 && !FileName.EndsWith(".se"))
         //  FileName += ".se";

         /*if (File.Exists(FileName.GetCString()))
            {
            Msg = "";
            Msg << FileName.GetCString() << " already exists.  Do you want to replace?";
            bGotFileName = (Application->MessageBox((char *) Msg.GetCString(),SaveDialog1->Title.c_str(), MB_YESNO) == ID_YES);
            SaveDialog1->FileName = FileName.GetCString();
            }
         else */
            bGotFileName = true;
         }

      if (bGotFileName)
         {
         gsParamFileName = SaveDialog1->FileName.c_str();
         frmAnalysis->Caption = SaveDialog1->FileName.c_str();
         //frmAnalysis->Refresh();
         //Application->ProcessMessages();
         WriteSession();
         }
      }
   catch (SSException & x)
      {
      x.AddCallpath("SaveAs", "TfrmAnalysis");
      DisplayBasisException(x);
      }
   return bGotFileName;
}
//-----------------------------------------------------------------------------
//  MAIN FUNCTION TO SET TRANSFER TEXT PARAMETERS INTO SESSION OBJ GLOBALS.
//---------------------------------------------------------------------------
void TfrmAnalysis::SaveTextParameters()
{
   Caption = gsParamFileName;

   try
      {
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
   
      //Time Parameter Tab
      gpParams->m_nIntervalLength  = atoi(edtUnitLength->Text.c_str());
      gpParams->m_nTimeAdjPercent = atof(edtLogPerYear->Text.c_str());
      sprintf(gpParams->m_szProspStartDate, "%i/%i/%i", atoi(edtProspYear->Text.c_str()), atoi(edtProspMonth->Text.c_str()), atoi(edtProspDay->Text.c_str()));

      //Output File Tab
      strcpy(gpParams->m_szOutputFilename, edtResultFile->Text.c_str());
      }
   catch (SSException & x)
      {
      x.AddCallpath("SaveTextParameters", "TfrmAnalysis");
      DisplayBasisException(x);
      }
}
//---------------------------------------------------------------------------
//  Sets all interface controls using the gpParams session object
//---------------------------------------------------------------------------
void TfrmAnalysis::SetupInterface()
{
   Caption = gsParamFileName;

   try
      {
      // Odd SatScan changed these values... WHY ???
      //gpParams->m_nAnalysisType --;
      //gpParams->m_nAreas --;

      // THIS "IF" STATEMENT IS HERE JUST TO MATCH THE CODE FOUND IN THE OLD
      // VISUAL C++ INTERFACE....
      if (gpParams->m_nAnalysisType == PURELYSPATIAL)
         {
         gpParams->m_nIntervalUnits = YEAR;
         gpParams->m_nIntervalLength = 1;
         }

      //Input File Tab
      edtCaseFileName->Text       = gpParams->m_szCaseFilename;
      edtControlFileName->Text    = gpParams->m_szControlFilename;
      rgPrecisionTimes->ItemIndex = gpParams->m_nPrecision;
      edtPopFileName->Text        = gpParams->m_szPopFilename;
      edtCoordinateFileName->Text = gpParams->m_szCoordFilename;
      edtGridFileName->Text       = gpParams->m_szGridFilename;
      rgCoordinates->ItemIndex    = gpParams->m_nCoordType;
    
      //Analysis Tab
      rgTypeAnalysis->ItemIndex = gpParams->m_nAnalysisType - 1;
      rgProbability->ItemIndex  = gpParams->m_nModel;
      rgScanAreas->ItemIndex    = gpParams->m_nAreas - 1;
      ParseDate(gpParams->m_szStartDate, edtStartYear, edtStartMonth, edtStartDay);
      ParseDate(gpParams->m_szEndDate, edtEndYear, edtEndMonth, edtEndDay);
      edtMontCarloReps->Text    = gpParams->m_nReplicas;
     
      //Scanning Window Tab
      edtMaxClusterSize->Text         = gpParams->m_nMaxGeographicClusterSize;
      chkInclPurTempClust->Checked    = gpParams->m_bIncludePurelyTemporal;
      edtMaxTemporalClusterSize->Text = gpParams->m_nMaxTemporalClusterSize;
      chkIncludePurSpacClust->Checked = gpParams->m_bIncludePurelySpatial;
      rgClustersToInclude->ItemIndex  = (gpParams->m_bAliveClustersOnly ? 1:0);  // IS THIS RETURNING THE RIGHT INDEX OR SHOULD I SWITCH IT AROUND ???
      if (gpParams->m_nAnalysisType == PROSPECTIVESPACETIME)
         {
         //DISABLE the Include Purely Spacial Clusters option.
         chkIncludePurSpacClust->Checked = false;
         chkIncludePurSpacClust->Enabled = false;
         }
      else
         chkIncludePurSpacClust->Enabled = true;

      //Time Parameter Tab
      if (gpParams->m_nIntervalUnits == 0) gpParams->m_nIntervalUnits = 1;
      if (gpParams->m_nIntervalLength == 0) gpParams->m_nIntervalLength = 1;
     
      rbUnitYear->Checked           = (gpParams->m_nIntervalUnits == 1);  // use to be 0
      rbUnitMonths->Checked         = (gpParams->m_nIntervalUnits == 2);  // use to be 1
      rbUnitDay->Checked            = (gpParams->m_nIntervalUnits == 3);  // use to be 2
      edtUnitLength->Text           = gpParams->m_nIntervalLength;
      rgTemporalTrendAdj->ItemIndex = gpParams->m_nTimeAdjustType;
      edtLogPerYear->Text           = gpParams->m_nTimeAdjPercent;

      if (strlen(gpParams->m_szProspStartDate) > 0)
         ParseDate(gpParams->m_szProspStartDate, edtProspYear, edtProspMonth, edtProspDay);
     
      //Output File Tab
      edtResultFile->Text        = gpParams->m_szOutputFilename;
      chkInclRelRiskEst->Checked = gpParams->m_bOutputRelRisks;
      chkInclSimLogLik->Checked  = gpParams->m_bSaveSimLogLikelihoods;
      chkCensusAreas->Checked    = gpParams->m_bOutputCensusAreas;  // Output Census areas in Reported Clusters
      chkLikelyClusters->Checked = gpParams->m_bMostLikelyClusters;  // Output Most Likely Cluster for each Centroid
      rgCriteriaSecClusters->ItemIndex = gpParams->m_iCriteriaSecondClusters;
     
      //now enable or disable controls appropriately
      DataExchange();
      }
   catch (SSException & x)
      {
      x.AddCallpath("SetupInterface", "TfrmAnalysis");
      DisplayBasisException(x);
      }      
}
//---------------------------------------------------------------------------
// parses up a date string and places it into the given month, day, year
// interace text control (TEdit *).
//---------------------------------------------------------------------------
void TfrmAnalysis::ParseDate(char * szDate, TEdit *pYear, TEdit *pMonth, TEdit *pDay)
{
   AnsiString theDate, thePart;
   int        iLoc;

   try
      {
      if (szDate != 0)
         {
         theDate = szDate;
         iLoc = theDate.Pos("/");
         if (iLoc == 0)
            MessageBox(NULL, "Invalid date found in parameter file.", "Parameter Error", MB_OK);
         else
            {
            thePart = theDate.SubString(1,iLoc-1);
            pYear->Text  = thePart.c_str();
            theDate.Delete(1, iLoc);
            iLoc = theDate.Pos("/");
            if (iLoc == 0)
               MessageBox(NULL, "Invalid date found in parameter file.", "Parameter Error", MB_OK);
            else
               {
               thePart = theDate.SubString(1,iLoc-1);
               pMonth->Text = thePart.c_str();
               theDate.Delete(1, iLoc);
               pDay->Text   = theDate.c_str();
               }
            }
         }
      }
   catch (SSException & x)
      {
      x.AddCallpath("ParseDate()", "TfrmAnalysis");
      throw;
      }
}
//---------------------------------------------------------------------------
//  When the time precision control is changed, various interace options are
//  toggled and changed.
//---------------------------------------------------------------------------
void __fastcall TfrmAnalysis::rgPrecisionTimesClick(TObject *Sender)
{
   try
      {
      gpParams->m_nPrecision = rgPrecisionTimes->ItemIndex;
    
      switch (rgPrecisionTimes->ItemIndex)
         {
         case NONE :   //None
            edtStartYear->Enabled  = true;  edtStartYear->Color = clWindow;
            edtStartMonth->Enabled = true;  edtStartMonth->Color = clWindow;
            edtStartDay->Enabled   = true;  edtStartDay->Color = clWindow;
            edtEndYear->Enabled    = true;  edtEndYear->Color = clWindow;
            edtEndMonth->Enabled   = true;  edtEndMonth->Color = clWindow;
            edtEndDay->Enabled     = true;  edtEndDay->Color = clWindow;
            //prospective space-time start date...
            if (gpParams->m_nAnalysisType == PROSPECTIVESPACETIME)
               {
               edtProspYear->Enabled  = true;  edtProspYear->Color = clWindow;
               edtProspMonth->Enabled = true;  edtProspMonth->Color = clWindow;
               edtProspDay->Enabled   = true;  edtProspDay->Color = clWindow;
               }
            EnableAnalysisType(false);
            break;
         case YEAR :  //year
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

            //prospective space-time start date...
            if (gpParams->m_nAnalysisType == PROSPECTIVESPACETIME)
               {
               edtProspYear->Enabled  = true;  edtProspYear->Color = clWindow;
               edtProspMonth->Enabled = false;  edtProspMonth->Color = clInactiveBorder;
               edtProspDay->Enabled   = false;  edtProspDay->Color = clInactiveBorder;
               edtProspMonth->Text = "1";
               edtProspDay->Text = "1";
               }

            EnableAnalysisType(true);
            break;
         case MONTH :  //month
            edtStartYear->Enabled  = true;   edtStartYear->Color = clWindow;
            edtStartMonth->Enabled = true;   edtStartMonth->Color = clWindow;
            edtStartDay->Enabled   = false;  edtStartDay->Color = clInactiveBorder;
            edtEndYear->Enabled    = true;   edtEndYear->Color = clWindow;
            edtEndMonth->Enabled   = true;   edtEndMonth->Color = clWindow;
            edtEndDay->Enabled     = false;  edtEndDay->Color = clInactiveBorder;

            edtEndDay->Text = DaysThisMonth(atoi(edtEndYear->Text.c_str()), atoi(edtEndMonth->Text.c_str()));
            edtStartDay->Text = "1";

            //prospective space-time start date...
            if (gpParams->m_nAnalysisType == PROSPECTIVESPACETIME)
               {
               edtProspYear->Enabled  = true;  edtProspYear->Color = clWindow;
               edtProspMonth->Enabled = true;  edtProspMonth->Color = clWindow;
               edtProspDay->Enabled   = false;  edtProspDay->Color = clInactiveBorder;
               edtProspDay->Text = "1";
               }
            EnableAnalysisType(true);
            break;
         case DAY :  //day
            edtStartYear->Enabled  = true;  edtStartYear->Color = clWindow;
            edtStartMonth->Enabled = true;  edtStartMonth->Color = clWindow;
            edtStartDay->Enabled   = true;  edtStartDay->Color = clWindow;
            edtEndYear->Enabled    = true;  edtEndYear->Color = clWindow;
            edtEndMonth->Enabled   = true;  edtEndMonth->Color = clWindow;
            edtEndDay->Enabled     = true;  edtEndDay->Color = clWindow;
            //prospective space-time start date...
            if (gpParams->m_nAnalysisType == PROSPECTIVESPACETIME)
               {
               edtProspYear->Enabled  = true;  edtProspYear->Color = clWindow;
               edtProspMonth->Enabled = true;  edtProspMonth->Color = clWindow;
               edtProspDay->Enabled   = true;  edtProspDay->Color = clWindow;
               }
            EnableAnalysisType(true);
            break;

         }

       //*************************************
       // this is the OnPrecisionChange() function
       if (gpParams->m_nAnalysisType == PURELYSPATIAL && gpParams->m_nPrecision != 0)   // use to be m_nAnalysisType == 0
          {
          if (gpParams->m_nIntervalUnits > gpParams->m_nPrecision )
             gpParams->m_nIntervalUnits = gpParams->m_nPrecision;
    
          //THIS USE TO BE..
          //if (gpParams->m_nIntervalUnits > gpParams->m_nPrecision -1)
          //   gpParams->m_nIntervalUnits = gpParams->m_nPrecision - 1;
          }
    
      DataExchange();
      }
   catch (SSException & x)
      {
      x.AddCallpath("rgPrecisionTimesClick", "TfrmAnalysis");
      throw;
      }   
}
//------------------------------------------------------------------------------
// Purely spacial runs are always available, but other times the other
// 3 options have to be turned off (or back on).
//---------------------------------------------------------------------------
void TfrmAnalysis::EnableAnalysisType(bool bValue)
{
   TControl *ChildControl;
   ChildControl = rgTypeAnalysis->Controls[1];  //Purely Temporal
   ChildControl->Enabled = bValue;
   ChildControl = rgTypeAnalysis->Controls[2];  //Retrospective space-Time
   ChildControl->Enabled = bValue;
   ChildControl = rgTypeAnalysis->Controls[3];  //Prospective Space-Time
   ChildControl->Enabled = bValue;
}
//------------------------------------------------------------------------------
//
// Input Tab functions
//------------------------------------------------------------------------------
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// case file selector
//---------------------------------------------------------------------------
void __fastcall TfrmAnalysis::btnCaseBrowseClick(TObject *Sender)
{
   try
      {
      OpenDialog1->FileName = "";
      OpenDialog1->DefaultExt = "*.cas";
      OpenDialog1->Filter = "CAS Files (*.cas)|*.cas|All files (*.*)|*.*";
      OpenDialog1->Title = "Select Case File";
      OpenDialog1->Execute();
      if (OpenDialog1->Files->Count == 1)
         {
         strcpy(gpParams->m_szCaseFilename, OpenDialog1->FileName.c_str());
         edtCaseFileName->Text = OpenDialog1->FileName.c_str();
         strcpy(gpParams->m_szCaseFilename, edtCaseFileName->Text.c_str());
         }
      }
   catch (SSException & x)
      {
      x.AddCallpath("btnCaseBrowseClick", "TfrmAnalysis");
      DisplayBasisException(x);
      }
}
//---------------------------------------------------------------------------
// Control file selector
//---------------------------------------------------------------------------
void __fastcall TfrmAnalysis::btnControlBrowseClick(TObject *Sender)
{
   //*.ctl files
   try
      {
      OpenDialog1->FileName = "";
      OpenDialog1->DefaultExt = "*.ctl";
      OpenDialog1->Filter = "CTL Files (*.ctl)|*.ctl|All files (*.*)|*.*";
      OpenDialog1->Title = "Select Control File";
      OpenDialog1->Execute();
      if (OpenDialog1->Files->Count == 1)
         {
         strcpy(gpParams->m_szControlFilename, OpenDialog1->FileName.c_str());
         edtControlFileName->Text = OpenDialog1->FileName.c_str();
         strcpy(gpParams->m_szControlFilename, edtControlFileName->Text.c_str());
         }
      }
   catch (SSException & x)
      {
      x.AddCallpath("btnControlBrowseClick", "TfrmAnalysis");
      DisplayBasisException(x);
      }
}
//---------------------------------------------------------------------------
// Population file selector
//---------------------------------------------------------------------------
void __fastcall TfrmAnalysis::btnPopBrowseClick(TObject *Sender)
{
   //*.pop files
   try
      {
      OpenDialog1->FileName = "";
      OpenDialog1->DefaultExt = "*.pop";
      OpenDialog1->Filter = "POP Files (*.pop)|*.pop|All files (*.*)|*.*";
      OpenDialog1->Title = "Select Population File";
      OpenDialog1->Execute();
      if (OpenDialog1->Files->Count == 1)
         {
         strcpy(gpParams->m_szPopFilename, OpenDialog1->FileName.c_str());
         edtPopFileName->Text = OpenDialog1->FileName.c_str();
         strcpy(gpParams->m_szPopFilename, edtPopFileName->Text.c_str());
         }
      }
   catch (SSException & x)
      {
      x.AddCallpath("btnPopBrowseClick", "TfrmAnalysis");
      DisplayBasisException(x);
      }
}
//---------------------------------------------------------------------------
// Geographic file selector
//---------------------------------------------------------------------------
void __fastcall TfrmAnalysis::btnCoordBrowseClick(TObject *Sender)
{
   //*.geo files
   try
      {
      OpenDialog1->FileName = "";
      OpenDialog1->DefaultExt = "*.geo";
      OpenDialog1->Filter = "GEO Files (*.geo)|*.geo|All files (*.*)|*.*";
      OpenDialog1->Title = "Select Coordinates File";
      OpenDialog1->Execute();
      if (OpenDialog1->Files->Count == 1)
         {
         strcpy(gpParams->m_szCoordFilename, OpenDialog1->FileName.c_str());
         edtCoordinateFileName->Text = OpenDialog1->FileName.c_str();
         strcpy(gpParams->m_szCoordFilename, edtCoordinateFileName->Text.c_str());
         }
      }
   catch (SSException & x)
      {
      x.AddCallpath("btnCoordBrowseClick", "TfrmAnalysis");
      DisplayBasisException(x);
      }
}
//---------------------------------------------------------------------------
//  Grid file selector
//---------------------------------------------------------------------------
void __fastcall TfrmAnalysis::btnGridBrowseClick(TObject *Sender)
{
   //*.grd files
   try
      {
      OpenDialog1->FileName = "";
      OpenDialog1->DefaultExt = "*.grd";
      OpenDialog1->Filter = "GRD Files (*.grd)|*.grd|All files (*.*)|*.*";
      OpenDialog1->Title = "Select Special Grid File";
      OpenDialog1->Execute();
      if (OpenDialog1->Files->Count == 1)
         {
         strcpy(gpParams->m_szGridFilename, OpenDialog1->FileName.c_str());
         edtGridFileName->Text = OpenDialog1->FileName.c_str();
         strcpy(gpParams->m_szGridFilename, edtGridFileName->Text.c_str());
         }
      }
   catch (SSException & x)
      {
      x.AddCallpath("btnGridBrowseClick", "TfrmAnalysis");
      DisplayBasisException(x);
      }
}
//---------------------------------------------------------------------------
// Precision is enabled or disabled depending on the analysis type !!!
//---------------------------------------------------------------------------
void TfrmAnalysis::EnablePrecision()
{
   try
      {
      if (gpParams->m_nAnalysisType == PURELYSPATIAL)    // use to be 0
         {
         TControl *ChildControl;
         ChildControl = rgPrecisionTimes->Controls[enNone];
         ChildControl->Enabled = true;
         ChildControl = rgPrecisionTimes->Controls[enYear];
         ChildControl->Enabled = true;
         ChildControl = rgPrecisionTimes->Controls[enMonth];
         ChildControl->Enabled = true;
         }
      else // Purely Temporal, Retrospective Space-Time, Prospective Space-Time
         {
         TControl *ChildControl;
         ChildControl = rgPrecisionTimes->Controls[enNone];
         ChildControl->Enabled = false;
         ChildControl = rgPrecisionTimes->Controls[enYear];
         ChildControl->Enabled = (gpParams->m_nIntervalUnits < 2);   // use to be 1
         ChildControl = rgPrecisionTimes->Controls[enMonth];
         ChildControl->Enabled = (gpParams->m_nIntervalUnits < 3);   // use to be 2
         }
      }
   catch (SSException & x)
      {
      x.AddCallpath("EnablePrecision", "TfrmAnalysis");
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
bool TfrmAnalysis::ValidateInputFiles()
{
   bool bOk = true;

   try
      {
      if (edtCaseFileName->Enabled) // case file edit box enabled
         {
         if (edtCaseFileName->Text.IsEmpty())
            {
            MessageBox(NULL, "Please specify a case file.", "Parameter Error" , MB_OK);
            bOk = false;
            }
         else
            bOk = ValidateFileExists(edtCaseFileName->Text, "Case");
         if (!bOk)
            {
            PageControl1->ActivePage = tbInputFiles;
            edtCaseFileName->SetFocus();
            }
         }
      //Control file for Bernoulli model only
      if (bOk & (rgProbability->ItemIndex==1)) // Control file edit box enabled
         {
         if (edtControlFileName->Text.IsEmpty())
            {
            MessageBox(NULL, "Please specify a control file.", "Parameter Error" , MB_OK);
            bOk = false;
            }
         else
            bOk = ValidateFileExists(edtControlFileName->Text, "Control");
         if (!bOk)
            {
            PageControl1->ActivePage = tbInputFiles;
            edtControlFileName->SetFocus();
            }
         }
      //Pop file for Poisson model only
      if (bOk & (rgProbability->ItemIndex==0)) // Population file edit box enabled
         {
         if (edtPopFileName->Text.IsEmpty())
            {
            MessageBox(NULL, "Please specify a population file.", "Parameter Error" , MB_OK);
            bOk = false;
            }
         else
            bOk = ValidateFileExists(edtPopFileName->Text, "Population");
         if (!bOk)
            {
            PageControl1->ActivePage = tbInputFiles;
            edtPopFileName->SetFocus();
            }
         }
      if (bOk & edtCoordinateFileName->Enabled) // Coordinates file edit box enabled
         {
         if (edtCoordinateFileName->Text.IsEmpty())
            {
            MessageBox(NULL, "Please specify a coordinates file.", "Parameter Error" , MB_OK);
            bOk = false;
            }
         else
            bOk = ValidateFileExists(edtCoordinateFileName->Text, "Coordinate");
         if (!bOk)
            {
            PageControl1->ActivePage = tbInputFiles;
            edtCoordinateFileName->SetFocus();
            }
         }
      if (bOk & edtGridFileName->Enabled & !edtGridFileName->Text.IsEmpty()) // Special grid file edit box enabled
         {
         bOk = ValidateFileExists(edtGridFileName->Text, "Special Grid");
         if (!bOk)
            {
            PageControl1->ActivePage = tbInputFiles;
            edtGridFileName->SetFocus();
            }
         }
      }
   catch (SSException & x)
      {
      x.AddCallpath("ValidateInputFiles", "TfrmAnalysis");
      throw;
      }
   return bOk;
}
//------------------------------------------------------------------------------
// If the types of coordinates are changed, then various interface options
// need to be updated
//------------------------------------------------------------------------------
void __fastcall TfrmAnalysis::rgCoordinatesClick(TObject *Sender)
{
   try
      {
      gpParams->m_nCoordType = rgCoordinates->ItemIndex;
      DataExchange();
      }
   catch (SSException & x)
      {
      x.AddCallpath("rgCoordinatesClick", "TfrmAnalysis");
      DisplayBasisException(x);
      }    
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
// Analysis Tab functions
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Type of Analsyis is changed
//------------------------------------------------------------------------------
void __fastcall TfrmAnalysis::rgTypeAnalysisClick(TObject *Sender)
{
   try
      {
      gpParams->m_nAnalysisType = rgTypeAnalysis->ItemIndex + 1;
   
      if (gpParams->m_nAnalysisType == PURELYTEMPORAL)
         {
         if (gpParams->m_nTimeAdjustType == 1)
            {
            gpParams->m_nTimeAdjustType = 0; //discrete adjustment not valid for P.T. analysis
            //update interface
             rgTemporalTrendAdj->ItemIndex = gpParams->m_nTimeAdjustType;
            }
         TControl *ChildControl;
         ChildControl = rgTemporalTrendAdj->Controls[1];
         ChildControl->Enabled = false;
         }
      else // enable non-parametric
         {
         TControl *ChildControl;
         ChildControl = rgTemporalTrendAdj->Controls[1];
         ChildControl->Enabled = true;
         }
      if (gpParams->m_nAnalysisType == PROSPECTIVESPACETIME)
         {
         EnableProspStartDate();
         //DISABLE the Include Purely Spacial Clusters option.
         chkIncludePurSpacClust->Checked = false;
         chkIncludePurSpacClust->Enabled = false;
         }
      else // make sure that the Include Purely Spacial Cluster option in enabled
         chkIncludePurSpacClust->Enabled = false;
      DataExchange();
      }
   catch (SSException & x)
      {
      x.AddCallpath("rgTypeAnalysisClick", "TfrmAnalysis");
      DisplayBasisException(x);
      }  
  
}
//------------------------------------------------------------------------------
// Probability is changed.  Need to perform a "dataexchange" to see if any
// other tab controls need to be enabled or disabled
//------------------------------------------------------------------------------
void __fastcall TfrmAnalysis::rgProbabilityClick(TObject *Sender)
{
   try
      {
      gpParams->m_nModel = rgProbability->ItemIndex;
      DataExchange();
      }
   catch (SSException & x)
      {
      x.AddCallpath("rgProbabilityClick", "TfrmAnalysis");
      DisplayBasisException(x);
      }        
}
//------------------------------------------------------------------------------
// Scan Areas is changed.  Need to perform a "dataexchange" to see if any
// other tab controls need to be enabled or disabled
//------------------------------------------------------------------------------
void __fastcall TfrmAnalysis::rgScanAreasClick(TObject *Sender)
{
   try
      {
      gpParams->m_nAreas = rgScanAreas->ItemIndex + 1;
      DataExchange();
      }
   catch (SSException & x)
      {
      x.AddCallpath("rgScanAreasClick", "TfrmAnalysis");
      DisplayBasisException(x);
      }
}
//------------------------------------------------------------------------------
// Verifies all the parameters on the Analysis Tab
//------------------------------------------------------------------------------
bool TfrmAnalysis::CheckAnalysisParams()
{
   bool bParamsOk = true;

   try
      {
      //if years enabled, then check values...
      //if start year enabled, assume end year enabled.
      if (edtStartYear->Enabled)
         {
         bParamsOk = Check_Year(atoi(edtStartYear->Text.c_str()),"Study Period Start Year");
         if (bParamsOk) Check_Year(atoi(edtEndYear->Text.c_str()),"Study Period End Year");
         }
      //if Months enabled, then check values...
      //if start month enabled, assume end month enabled.
      if (bParamsOk && edtStartMonth->Enabled)
         {
         bParamsOk = Check_Month(atoi(edtStartMonth->Text.c_str()), "Study Period Start Month");
         if (bParamsOk) Check_Month(atoi(edtEndMonth->Text.c_str()), "Study Period End Month");
         }
      //if Days enabled, then check values...
      //if start days enabled, assume end days enabled.
      if (bParamsOk && edtStartDay->Enabled)
         {
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
   catch (SSException & x)
      {
      x.AddCallpath("CheckAnalysisParams", "TfrmAnalysis");
      throw;
      }        
   return bParamsOk;
}
//------------------------------------------------------------------------------
// Generic Month checker -- simulates old interface
//------------------------------------------------------------------------------
bool TfrmAnalysis::Check_Month(int iMonth, char *sDateName)
{
   bool bMonthOk = true;
   AnsiString sFinalMessage;

   try
      {
      if ((iMonth < 1) || (iMonth > 12))
         {
         sFinalMessage += sDateName;
         sFinalMessage += ":  Please specify an month between 1 and 12.";
         MessageBox(NULL, sFinalMessage.c_str(), "Parameter Error" , MB_OK);
         bMonthOk = false;
         }
      }
   catch (SSException & x)
      {
      x.AddCallpath("Check_Month", "TfrmAnalysis");
      throw;
      } 
   return bMonthOk;
}
//------------------------------------------------------------------------------
// Generic Days checker -- simulates old interface
//MUST CHECK TO SEE IF THE DAY EDIT BOX IS ENABLED FIRST !!!!
//------------------------------------------------------------------------------
bool TfrmAnalysis::Check_Days(int iYear, int iMonth, int iDay, char *sDateName)
{
   bool bDayOk = true;
   char szMessage[100];
   AnsiString sFinalMessage;
   int iMin = 1, iMax;

   try
      {
      iMax = DaysThisMonth(iYear, iMonth);
      if ((iDay < iMin) || (iDay > iMax))
         {
         sFinalMessage += sDateName;
         sprintf(szMessage, ":  Please specify a day between %i and %i.", iMin, iMax);
         sFinalMessage += szMessage;
         MessageBox(NULL, sFinalMessage.c_str(), "Parameter Error" , MB_OK);
         bDayOk = false;
         }
      }
   catch (SSException & x)
      {
      x.AddCallpath("Check_Days", "TfrmAnalysis");
      throw;
      } 
   return bDayOk;
}
//------------------------------------------------------------------------------
// Generic Year checker -- simulates old interface
//MUST CHECK TO SEE IF THE DAY EDIT BOX IS ENABLED FIRST !!!!
//------------------------------------------------------------------------------
bool TfrmAnalysis::Check_Year(int iYear, char *sDateName)
{
   char szMessage[100];
   bool bYearOk = true;
   AnsiString sFinalMessage;

   try
      {
      if ( ! (iYear >= MIN_YEAR) && (iYear <= MAX_YEAR))
         {
         sFinalMessage += sDateName;
         sprintf(szMessage, ":  Please specify a year between %i and %i.", MIN_YEAR, MAX_YEAR);
         sFinalMessage += szMessage;
         MessageBox(NULL, sFinalMessage.c_str(), "Parameter Error" , MB_OK);
         bYearOk = false;
         }
      }
   catch (SSException & x)
      {
      x.AddCallpath("Check_Year", "TfrmAnalysis");
      throw;
      } 
   return bYearOk;
}
//------------------------------------------------------------------------------
// Verifies the relationship between a start and end date
//------------------------------------------------------------------------------
bool TfrmAnalysis::CheckDateRange(int iStartYear, int iStartMonth, int iStartDay,
                                  int iEndYear, int iEndMonth, int iEndDay,
                                  int iIntervalUnits, int iIntervalLength)
{
   bool bRangeOk = true;
   char szMessage[100];
   char szUnit[10];
   Julian Start, End;
   long   lTimeBetween;

   try
      {
      Start = MDYToJulian(iStartMonth, iStartDay, iStartYear);
      End   = MDYToJulian(iEndMonth, iEndDay, iEndYear);
      lTimeBetween = TimeBetween(Start, End, iIntervalUnits);  // use to be iIntervalUnits+1
      switch (iIntervalUnits)  // use to be iIntervalUnits + 1;
         {
         case (YEAR):   strcpy(szUnit, "year(s)"); break;
         case (MONTH):  strcpy(szUnit, "month(s)"); break;
         case (DAY):    strcpy(szUnit, "day(s)"); break;
         default:       break;
         };
      if (iIntervalLength > lTimeBetween)
         {
         sprintf(szMessage, "Due to interval length specified (Time Parameter tab), the start and end dates must be at least %d %s apart.", iIntervalLength, szUnit);
         MessageBox(NULL, szMessage, "Parameter Error" , MB_OK);
         bRangeOk = false;
         }
      }
   catch (SSException & x)
      {
      x.AddCallpath("CheckDateRange", "TfrmAnalysis");
      throw;
      }
   return bRangeOk;
}
//------------------------------------------------------------------------------
// Specific prospective space-time date check
//  Must be between the start and end dates of the analysis
//------------------------------------------------------------------------------
bool TfrmAnalysis::CheckProspDateRange(int iStartYear, int iStartMonth, int iStartDay,
                                  int iEndYear, int iEndMonth, int iEndDay,
                                  int iProspYear, int iProspMonth, int iProspDay)
{
   bool bRangeOk = true;
   Julian Start, End, Prosp;

   try
      {
      Start = MDYToJulian(iStartMonth, iStartDay, iStartYear);
      End   = MDYToJulian(iEndMonth, iEndDay, iEndYear);
      Prosp = MDYToJulian(iProspMonth, iProspDay, iProspYear);
      if (! Check_Days(iProspYear, iProspMonth, iProspDay, "Start date of Prospective Space-Time"))
         {
         PageControl1->ActivePage = tbTimeParameter;
         edtProspDay->SetFocus();
         bRangeOk = false;
         }
      else if ((Prosp < Start) || (Prosp > End))
         {
         MessageBox(NULL, "The Start date of Prospective Space-Time must be between the Study Period start and end dates.", "Parameter Error" , MB_OK);
         PageControl1->ActivePage = tbTimeParameter;
         edtProspYear->SetFocus();
         bRangeOk = false;
         }
      }
   catch (SSException & x)
      {
      x.AddCallpath("CheckProspDateRange", "TfrmAnalysis");
      throw;
      } 
   return bRangeOk;
}
//------------------------------------------------------------------------------
// Monte Carol Replications must follow a specific numeric selection
//------------------------------------------------------------------------------
bool TfrmAnalysis::CheckReplicas(int iReplicas)
{
   bool bReplicasOk = true;

   try
      {
      if (! ((iReplicas == 0) || (iReplicas == 9) || (iReplicas == 19) || (fmod(iReplicas+1, 1000) == 0.0)) )
         {
         MessageBox(NULL, "Invalid number of replicas specified.", "Parameter Error" , MB_OK);
         bReplicasOk = false;
         }
      }
   catch (SSException & x)
      {
      x.AddCallpath("CheckReplicas", "TfrmAnalysis");
      throw;
      } 
   return bReplicasOk;
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
// OUTPUT Tab functions
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Selects a Results file
//------------------------------------------------------------------------------
void __fastcall TfrmAnalysis::btnResultFileBrowseClick(TObject *Sender)
{
   //results files
   try
      {
      OpenDialog1->FileName = "";
      OpenDialog1->DefaultExt = "*.txt";
      OpenDialog1->Filter = "GRD Files (*.txt)|*.txt|All files (*.*)|*.*";
      OpenDialog1->Title = "Select Results File";
      OpenDialog1->Execute();
      if (OpenDialog1->Files->Count == 1)
         {
         strcpy(gpParams->m_szOutputFilename, OpenDialog1->FileName.c_str());
         edtResultFile->Text = OpenDialog1->FileName.c_str();
         //UpdateData(false);
         }
      }
   catch (SSException & x)
      {
      x.AddCallpath("btnResultFileBrowseClick", "TfrmAnalysis");
      DisplayBasisException(x);
      }
}
//------------------------------------------------------------------------------
// Verifies all parameters on the Output tab
//------------------------------------------------------------------------------
bool TfrmAnalysis::CheckOutputParams()
{
   bool bReturn = true;

   try
     {
      if (edtResultFile->Enabled)
         bReturn = ValidateFileCanCreate(edtResultFile->Text, "Output");
      }
   catch (SSException & x)
      {
      x.AddCallpath("CheckOutputParams", "TfrmAnalysis");
      throw;
      }
   return bReturn;
}
//------------------------------------------------------------------------------
// Include relative risks in output
//------------------------------------------------------------------------------
void __fastcall TfrmAnalysis::chkInclRelRiskEstClick(TObject *Sender)
{
   try
      {
      gpParams->m_bOutputRelRisks = chkInclRelRiskEst->Checked;
      DataExchange();
      }
   catch (SSException & x)
      {
      x.AddCallpath("chkInclRelRiskEstClick", "TfrmAnalysis");
      DisplayBasisException(x);
      }        
}
//------------------------------------------------------------------------------
// Simulated Log likelihood ratio set
//------------------------------------------------------------------------------
void __fastcall TfrmAnalysis::chkInclSimLogLikClick(TObject *Sender)
{
   try
      {
      gpParams->m_bSaveSimLogLikelihoods = chkInclSimLogLik->Checked;
      DataExchange();
      }
   catch (SSException & x)
      {
      x.AddCallpath("chkInclSimLogLikClick", "TfrmAnalysis");
      DisplayBasisException(x);
      }        
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
// TIME Tab functions
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Enables ro disables Time trend controls
//------------------------------------------------------------------------------
void TfrmAnalysis::EnableTimeTrendAdj()
{
   bool bTemporal = (gpParams->m_nAnalysisType != PURELYSPATIAL);// PT, Retro ST, Pro ST   --- use to be 1 and 2
   TControl *ChildControl;

   try
      {
      rgTemporalTrendAdj->Enabled = true;
    
      if (gpParams->m_nModel == POISSON)
         {
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
      else if (gpParams->m_nModel == BERNOULLI)
         {
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
   catch (SSException & x)
      {
      x.AddCallpath("EnableTimeTrendAdj", "TfrmAnalysis");
      throw;
      }  
}
//------------------------------------------------------------------------------
// Enables or disables time interval controls
//------------------------------------------------------------------------------
void TfrmAnalysis::EnableTimeIntervals()
{
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
// Enable or disables the Prospective Start Date controls
//------------------------------------------------------------------------------
void TfrmAnalysis::EnableProspStartDate()
{
  if (gpParams->m_nAnalysisType != PROSPECTIVESPACETIME)
     {
     edtProspYear->Enabled  = false;
     edtProspYear->Color    = clInactiveBorder;
     edtProspMonth->Enabled = false;
     edtProspMonth->Color   = clInactiveBorder;
     edtProspDay->Enabled   = false;
     edtProspDay->Color     = clInactiveBorder;
     }
  else
     {
     switch (rgPrecisionTimes->ItemIndex)
        {
        case NONE :   
           edtProspYear->Enabled  = true;  edtProspYear->Color = clWindow;
           edtProspMonth->Enabled = true;  edtProspMonth->Color = clWindow;
           edtProspDay->Enabled   = true;  edtProspDay->Color = clWindow;
           break;
        case YEAR :
           edtProspYear->Enabled  = true;  edtProspYear->Color = clWindow;
           edtProspMonth->Enabled = false;  edtProspMonth->Color = clInactiveBorder;
           edtProspDay->Enabled   = false;  edtProspDay->Color = clInactiveBorder;
           edtProspMonth->Text = "1";
           edtProspDay->Text = "1";
           break;
        case MONTH :
           edtProspYear->Enabled  = true;  edtProspYear->Color = clWindow;
           edtProspMonth->Enabled = true;  edtProspMonth->Color = clWindow;
           edtProspDay->Enabled   = false;  edtProspDay->Color = clInactiveBorder;
           edtProspDay->Text = "1";
           break;
        case DAY :
           edtProspYear->Enabled  = true;  edtProspYear->Color = clWindow;
           edtProspMonth->Enabled = true;  edtProspMonth->Color = clWindow;
           edtProspDay->Enabled   = true;  edtProspDay->Color = clWindow;
           break;
        }
     }
}
//------------------------------------------------------------------------------
// Checks all the time parameters
//------------------------------------------------------------------------------
bool TfrmAnalysis::CheckTimeParams()
{
   bool bParamsOk = true;
   double dValue;

   try
      {
      if (gpParams->m_nAnalysisType != PURELYSPATIAL) //not purely spacial    use to be 0
         {
         if (edtUnitLength->Enabled)
            bParamsOk = Check_IntervalLength(atoi(edtStartYear->Text.c_str()), atoi(edtStartMonth->Text.c_str()), atoi(edtStartDay->Text.c_str()),
                atoi(edtEndYear->Text.c_str()), atoi(edtEndMonth->Text.c_str()), atoi(edtEndDay->Text.c_str()),
                gpParams->m_nIntervalUnits, atoi(edtUnitLength->Text.c_str()));
         if (bParamsOk && edtLogPerYear->Enabled)
            {
            if (edtLogPerYear->Text.IsEmpty())
               {
               bParamsOk = false;
               MessageBox(NULL, "Please enter a number in time trend percentage.", "Parameter Error" , MB_OK);
               PageControl1->ActivePage = tbTimeParameter;
               }
            else
               {
               dValue = atof(edtLogPerYear->Text.c_str());
               bParamsOk = Check_TimeTrendPercentage(dValue);
               }
            }
         //just need to check if the Prospective year is enabled.
         // if year is enabled, then all others are too...
         if (bParamsOk && edtProspYear->Enabled)
            {
            bParamsOk = CheckProspDateRange(atoi(edtStartYear->Text.c_str()), atoi(edtStartMonth->Text.c_str()), atoi(edtStartDay->Text.c_str()),
                atoi(edtEndYear->Text.c_str()), atoi(edtEndMonth->Text.c_str()), atoi(edtEndDay->Text.c_str()),
                atoi(edtProspYear->Text.c_str()), atoi(edtProspMonth->Text.c_str()), atoi(edtProspDay->Text.c_str()));
            }
         }
      }
   catch (SSException & x)
      {
      x.AddCallpath("CheckTimeParams", "TfrmAnalysis");
      throw;
      }        
   return bParamsOk;
}
//------------------------------------------------------------------------------
// Specific Year unit control
//------------------------------------------------------------------------------
void __fastcall TfrmAnalysis::rbUnitYearClick(TObject *Sender)
{
   try
      {
      gpParams->m_nIntervalUnits = 1; // use to be 0
      DataExchange();
      }
   catch (SSException & x)
      {
      x.AddCallpath("rbUnitYearClick", "TfrmAnalysis");
      DisplayBasisException(x);
      }        
}
//------------------------------------------------------------------------------
// Specific Months unit control
//------------------------------------------------------------------------------
void __fastcall TfrmAnalysis::rbUnitMonthsClick(TObject *Sender)
{
   try
      {
      gpParams->m_nIntervalUnits = 2; // use to be 1
      DataExchange();
      }
   catch (SSException & x)
      {
      x.AddCallpath("rbUnitMonthsClick", "TfrmAnalysis");
      DisplayBasisException(x);
      }        
}
//------------------------------------------------------------------------------
// Specific Day unit control
//------------------------------------------------------------------------------
void __fastcall TfrmAnalysis::rbUnitDayClick(TObject *Sender)
{
   try
      {
      gpParams->m_nIntervalUnits = 3; // use to be 2
      DataExchange();
      }
   catch (SSException & x)
      {
      x.AddCallpath("rbUnitDayClick", "TfrmAnalysis");
      DisplayBasisException(x);
      }  
}
//------------------------------------------------------------------------------
// Temporal Time Trend control
//------------------------------------------------------------------------------
void __fastcall TfrmAnalysis::rgTemporalTrendAdjClick(TObject *Sender)
{
  gpParams->m_nTimeAdjustType = rgTemporalTrendAdj->ItemIndex;

   try
      {
     if (rgTemporalTrendAdj->ItemIndex == 0)       // None
        {
        edtLogPerYear->Enabled = false;
        edtLogPerYear->Color = clInactiveBorder;
        }
     else if (rgTemporalTrendAdj->ItemIndex == 1)  // NonParametric
        {
        edtLogPerYear->Enabled = false;
        edtLogPerYear->Color = clInactiveBorder;
        }
     else if (rgTemporalTrendAdj->ItemIndex == 2)  // Log Linear
        {
        edtLogPerYear->Enabled = true;
        edtLogPerYear->Color = clWindow;
        }
     DataExchange();
      }
   catch (SSException & x)
      {
      x.AddCallpath("rgTemporalTrendAdjClick", "TfrmAnalysis");
      DisplayBasisException(x);
      }    
}
//------------------------------------------------------------------------------
// Verifies the interval length.  If you add interval length to start date,
// the resultant date must not be beyond the end date
//------------------------------------------------------------------------------
bool TfrmAnalysis::Check_IntervalLength(int iStartYear, int iStartMonth, int iStartDay,
            int iEndYear, int iEndMonth, int iEndDay, int iIntervalUnits, int iIntervalLength)
{
   char szMessage[100];
   Julian Start, End;
   long   lMin = 1, lMax;
   bool   bIntervalLenOk = true;

   try
      {
      Start = MDYToJulian(iStartMonth, iStartDay, iStartYear);
      End   = MDYToJulian(iEndMonth, iEndDay, iEndYear);
      lMax = TimeBetween(Start, End, iIntervalUnits);  //use to be iIntervalUnits+1
      if ((iIntervalLength < lMin) || (iIntervalLength > lMax))
         {
         sprintf(szMessage, "The interval length must be between %i and %i.", lMin, lMax);
         MessageBox(NULL, szMessage, "Parameter Error" , MB_OK);
         bIntervalLenOk = false;
         }
      }
   catch (SSException & x)
      {
      x.AddCallpath("Check_IntervalLength", "TfrmAnalysis");
      throw;
      }  
   return bIntervalLenOk;
}
//------------------------------------------------------------------------------
// Simple time trend percentage check.  
bool TfrmAnalysis::Check_TimeTrendPercentage(double dValue)
{
   bool bTrendPctOk = true;

   if ( ! (dValue > -100.00))
      {
      MessageBox(NULL, "Invalid time trend percentage specified.", "Parameter Error" , MB_OK);
      bTrendPctOk = false;
      }
   return bTrendPctOk;
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//
// SCANNING Tab functions
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Main function to enable and disable Scanning tab controls
//------------------------------------------------------------------------------
void TfrmAnalysis::EnableScanningWindow()
{
   // fixed the bSpacial boolean.  Use to be == 1 or == 2.  changed to == 1 or == 3
   bool bSpacial = (gpParams->m_nAnalysisType == PURELYSPATIAL) || (gpParams->m_nAnalysisType == SPACETIME) || (gpParams->m_nAnalysisType == PROSPECTIVESPACETIME); // PS, ST  use to be 0 and 1
   bool bTemporal = (gpParams->m_nAnalysisType == PURELYTEMPORAL) || (gpParams->m_nAnalysisType == SPACETIME) || (gpParams->m_nAnalysisType == PROSPECTIVESPACETIME); // ST, PT   use to be 1 and 2
   bool bTemporalNotCountingProspST = (gpParams->m_nAnalysisType == PURELYTEMPORAL) || (gpParams->m_nAnalysisType == SPACETIME);

   edtMaxClusterSize->Enabled =    bSpacial;
   edtMaxClusterSize->Color = (bSpacial ? clWindow:clInactiveBorder);
   chkInclPurTempClust->Enabled =  (bSpacial && bTemporal);

   edtMaxTemporalClusterSize->Enabled = bTemporal;
   edtMaxTemporalClusterSize->Color = (bTemporal ? clWindow:clInactiveBorder);
   chkIncludePurSpacClust->Enabled = (((gpParams->m_nAnalysisType == PURELYSPATIAL) || (gpParams->m_nAnalysisType == SPACETIME)) && bTemporal);

   rgClustersToInclude->Enabled = bTemporalNotCountingProspST;
}
//------------------------------------------------------------------------------
// Checks the validity of the scanning tab controls
//------------------------------------------------------------------------------
bool TfrmAnalysis::CheckScanningWindowParams()
{
   bool bParamsOk = true;
   double dValue;

   try
      {
      if (edtMaxClusterSize->Enabled)
         {
         dValue = atof(edtMaxClusterSize->Text.c_str());
         if ( ! (dValue > 0.0 && dValue <= 50.0))
            {
            MessageBox(NULL, "Invalid maximum geographic size specified.", "Parameter Error" , MB_OK);
            bParamsOk = false;
            }
         }
      if (bParamsOk && edtMaxTemporalClusterSize->Enabled) // check
         {
         dValue = atof(edtMaxTemporalClusterSize->Text.c_str());
         if ( ! (dValue > 0.0 && dValue <= 90.0))
            {
            MessageBox(NULL, "Invalid maximum time size specified.", "Parameter Error" , MB_OK);
            bParamsOk = false;
            }
         }
      }
   catch (SSException & x)
      {
      x.AddCallpath("CheckScanningWindowParams", "TfrmAnalysis");
      throw;
      }
   return bParamsOk;
}
//------------------------------------------------------------------------------
// Include Purely Temporal Clusters selection control
//------------------------------------------------------------------------------
void __fastcall TfrmAnalysis::chkInclPurTempClustClick(TObject *Sender)
{
   try
      {
      gpParams->m_bIncludePurelyTemporal = chkInclPurTempClust->Checked;
      DataExchange();
      }
   catch (SSException & x)
      {
      x.AddCallpath("chkInclPurTempClustClick", "TfrmAnalysis");
      DisplayBasisException(x);
      }  
}
//------------------------------------------------------------------------------
// Include Purely Spacial Clusters selection control
//------------------------------------------------------------------------------
void __fastcall TfrmAnalysis::chkIncludePurSpacClustClick(TObject *Sender)
{
   try
      {
      gpParams->m_bIncludePurelySpatial = chkIncludePurSpacClust->Checked;
      DataExchange();
      }
   catch (SSException & x)
      {
      x.AddCallpath("chkIncludePurSpacClustClick", "TfrmAnalysis");
      DisplayBasisException(x);
      }  
}
//------------------------------------------------------------------------------
//  Control to include "Alive" clusters
//------------------------------------------------------------------------------
void __fastcall TfrmAnalysis::rgClustersToIncludeClick(TObject *Sender)
{
   try
      {
      gpParams->m_bAliveClustersOnly = (rgClustersToInclude->ItemIndex == 0 ? 0:1);
      DataExchange();
      }
   catch (SSException & x)
      {
      x.AddCallpath("rgClustersToIncludeClick", "TfrmAnalysis");
      DisplayBasisException(x);
      }  
}
//------------------------------------------------------------------------------
//  Validates value entered for Maximum Temporal Cluster Size
//------------------------------------------------------------------------------
void __fastcall TfrmAnalysis::edtMaxTemporalClusterSizeExit(
      TObject *Sender)
{
   double dValue = atof(edtMaxTemporalClusterSize->Text.c_str());

   try
      {
      if ( ! (dValue > 0.0 && dValue <= 90.0))
         {
         MessageBox(NULL, "Invalid maximum time size specified.", "Parameter Error" , MB_OK);
         PageControl1->ActivePage = tbScanningWindow;
         edtMaxTemporalClusterSize->SetFocus();
         }
      else
         gpParams->m_nMaxTemporalClusterSize = atof(edtMaxTemporalClusterSize->Text.c_str());
      }
   catch (SSException & x)
      {
      x.AddCallpath("edtMaxTemporalClusterSizeExit", "TfrmAnalysis");
      DisplayBasisException(x);
      }        
}
//------------------------------------------------------------------------------
// Validates value entered for Cluster size
//------------------------------------------------------------------------------
void __fastcall TfrmAnalysis::edtMaxClusterSizeExit(TObject *Sender)
{
   double dValue = atof(edtMaxClusterSize->Text.c_str());

   try
      {
      if ( ! (dValue > 0.0 && dValue <= 50.0))
         {
         MessageBox(NULL, "Invalid maximum geographic size specified.", "Parameter Error" , MB_OK);
         PageControl1->ActivePage = tbScanningWindow;
         edtMaxClusterSize->SetFocus();
         }
      else
         gpParams->m_nMaxGeographicClusterSize = atof(edtMaxClusterSize->Text.c_str());
      }
   catch (SSException & x)
      {
      x.AddCallpath("edtMaxClusterSizeExit", "TfrmAnalysis");
      DisplayBasisException(x);
      }
}
//------------------------------------------------------------------------------
// Validates Number of Monte Carlo reps value
//------------------------------------------------------------------------------
void __fastcall TfrmAnalysis::edtMontCarloRepsExit(TObject *Sender)
{
   try 
      {
      if ( ! CheckReplicas(atoi(edtMontCarloReps->Text.c_str())))
         {
         PageControl1->ActivePage = tbAnalysis;
         edtMontCarloReps->SetFocus();
         }
      }
   catch (SSException & x)
      {
      x.AddCallpath("edtMontCarloRepsExit", "TfrmAnalysis");
      DisplayBasisException(x);
      }  
}
//------------------------------------------------------------------------------
void __fastcall TfrmAnalysis::edtMontCarloRepsKeyPress(TObject *Sender,
      char &Key)
{

   if (!strchr("0123456789\b",Key))
   	{
      Key = 0;
      }
}
//------------------------------------------------------------------------------
void __fastcall TfrmAnalysis::edtEndDayKeyPress(TObject *Sender, char &Key)
{

   if (!strchr("0123456789\b",Key))
   	{
      Key = 0;
      }
}
//------------------------------------------------------------------------------
void __fastcall TfrmAnalysis::edtStartDayKeyPress(TObject *Sender,
      char &Key)
{

   if (!strchr("0123456789\b",Key))
   	{
      Key = 0;
      }        
}
//---------------------------------------------------------------------------
void __fastcall TfrmAnalysis::edtStartMonthKeyPress(TObject *Sender,
      char &Key)
{

   if (!strchr("0123456789\b",Key))
   	{
      Key = 0;
      }
}
//---------------------------------------------------------------------------
void __fastcall TfrmAnalysis::edtEndMonthKeyPress(TObject *Sender,
      char &Key)
{

   if (!strchr("0123456789\b",Key))
   	{
      Key = 0;
      }
}
//---------------------------------------------------------------------------
void __fastcall TfrmAnalysis::edtStartYearKeyPress(TObject *Sender,
      char &Key)
{

   if (!strchr("0123456789\b",Key))
   	{
      Key = 0;
      }
}
//---------------------------------------------------------------------------
void __fastcall TfrmAnalysis::edtEndYearKeyPress(TObject *Sender,
      char &Key)
{

   if (!strchr("0123456789\b",Key))
   	{      
      Key = 0;
      }        
}
//---------------------------------------------------------------------------
void __fastcall TfrmAnalysis::edtMaxClusterSizeKeyPress(TObject *Sender,
      char &Key)
{

   if (!strchr("0123456789.\b",Key))
   	{
      Key = 0;
      }
}
//---------------------------------------------------------------------------
void __fastcall TfrmAnalysis::edtMaxTemporalClusterSizeKeyPress(
      TObject *Sender, char &Key)
{

   if (!strchr("0123456789.\b",Key))
   	{
      Key = 0;
      }
}
//---------------------------------------------------------------------------
void __fastcall TfrmAnalysis::edtUnitLengthKeyPress(TObject *Sender,
      char &Key)
{

   if (!strchr("0123456789\b",Key))
   	{
      Key = 0;
      }        
}
//---------------------------------------------------------------------------
void __fastcall TfrmAnalysis::edtLogPerYearKeyPress(TObject *Sender,
      char &Key)
{
   //NEGATIVE VALUES ALLOWED FOR THIS...
   if (!strchr("-0123456789.\b",Key))
   	{      
      Key = 0;
      }
}
//---------------------------------------------------------------------------
void __fastcall TfrmAnalysis::edtStartMonthExit(TObject *Sender)
{
   try
      {
      if ( ! Check_Month(atoi(edtStartMonth->Text.c_str()), "Study Period Start Month"))
         {
         PageControl1->ActivePage = tbAnalysis;
         edtStartMonth->SetFocus();
         }
      }
   catch (SSException & x)
      {
       x.AddCallpath("edtStartMonthExit", "TfrmAnalysis");
      DisplayBasisException(x);
      }
}
//---------------------------------------------------------------------------
void __fastcall TfrmAnalysis::edtStartYearExit(TObject *Sender)
{
   try
      {
      if ( ! Check_Year(atoi(edtStartYear->Text.c_str()), "Study Period Start Year"))
         {
         PageControl1->ActivePage = tbAnalysis;
         edtStartYear->SetFocus();
         }
      }
   catch (SSException & x)
      {
      x.AddCallpath("edtStartYearExit", "TfrmAnalysis");
      DisplayBasisException(x);
      }  
}
//---------------------------------------------------------------------------

void __fastcall TfrmAnalysis::edtEndMonthExit(TObject *Sender)
{
   try
      {
      if ( ! Check_Month(atoi(edtEndMonth->Text.c_str()), "Study Period End Month"))
         {
         PageControl1->ActivePage = tbAnalysis;
         edtStartMonth->SetFocus();
         }
      }
   catch (SSException & x)
      {
      x.AddCallpath("edtEndMonthExit", "TfrmAnalysis");
      DisplayBasisException(x);
      }  
}
//---------------------------------------------------------------------------

void __fastcall TfrmAnalysis::edtEndYearExit(TObject *Sender)
{
   try
      {
      if ( ! Check_Year(atoi(edtEndYear->Text.c_str()), "Study Period End Year"))
         {
         PageControl1->ActivePage = tbAnalysis;
         edtEndYear->SetFocus();
         }
      }
   catch (SSException & x)
      {
      x.AddCallpath("edtEndYearExit", "TfrmAnalysis");
      DisplayBasisException(x);
      }  
}
//---------------------------------------------------------------------------

void __fastcall TfrmAnalysis::edtStartDayExit(TObject *Sender)
{
   try
      {
      if ((atoi(edtStartDay->Text.c_str()) < 1) || (atoi(edtStartDay->Text.c_str()) > 31))
         {
         MessageBox(NULL, "Please specify a valid day.", "Parameter Error" , MB_OK);
         PageControl1->ActivePage = tbAnalysis;
         edtStartDay->SetFocus();
         }
      }
   catch (SSException & x)
      {
      x.AddCallpath("edtStartDayExit", "TfrmAnalysis");
      DisplayBasisException(x);
      }
}
//---------------------------------------------------------------------------


void __fastcall TfrmAnalysis::edtEndDayExit(TObject *Sender)
{
   try
      {	
      if ((atoi(edtEndDay->Text.c_str()) < 1) || (atoi(edtEndDay->Text.c_str()) > 31))
         {
         PageControl1->ActivePage = tbAnalysis;
         edtEndDay->SetFocus();
         }
      }
   catch (SSException & x)
      {
      x.AddCallpath("edtEndDayExit", "TfrmAnalysis");
      DisplayBasisException(x);
      }  
}
//---------------------------------------------------------------------------



void __fastcall TfrmAnalysis::edtProspYearKeyPress(TObject *Sender,
      char &Key)
{

   if (!strchr("0123456789\b",Key))
   	{
      Key = 0;
      }
}
//---------------------------------------------------------------------------

void __fastcall TfrmAnalysis::edtProspMonthKeyPress(TObject *Sender,
      char &Key)
{

   if (!strchr("0123456789\b",Key))
   	{
      Key = 0;
      }
}
//---------------------------------------------------------------------------

void __fastcall TfrmAnalysis::edtProspDayKeyPress(TObject *Sender,
      char &Key)
{

   if (!strchr("0123456789\b",Key))
   	{
      Key = 0;
      }
}
//---------------------------------------------------------------------------


void __fastcall TfrmAnalysis::edtProspYearExit(TObject *Sender)
{
   try
      {
      if ( ! Check_Year(atoi(edtProspYear->Text.c_str()), "Prospective Space-Time Start Year"))
         {
         PageControl1->ActivePage = tbTimeParameter;
         edtProspYear->SetFocus();
         }
      }
   catch (SSException & x)
      {
      x.AddCallpath("edtProspYearExit", "TfrmAnalysis");
      DisplayBasisException(x);
      }  
}
//---------------------------------------------------------------------------


void __fastcall TfrmAnalysis::edtProspMonthExit(TObject *Sender)
{
   try
      {
      if ( ! Check_Month(atoi(edtProspMonth->Text.c_str()), "Prospective Space-Time Start Month"))
         {
         PageControl1->ActivePage = tbTimeParameter;
         edtProspMonth->SetFocus();
         }
      }
   catch (SSException & x)
      {
      x.AddCallpath("edtProspMonthExit", "TfrmAnalysis");
      DisplayBasisException(x);
      }  
}
//---------------------------------------------------------------------------

void __fastcall TfrmAnalysis::edtProspDayExit(TObject *Sender)
{
   try
      {
      if ((atoi(edtProspDay->Text.c_str()) < 1) || (atoi(edtProspDay->Text.c_str()) > 31))
         {
         MessageBox(NULL, "Please specify a valid day.", "Parameter Error" , MB_OK);
         PageControl1->ActivePage = tbTimeParameter;
         edtProspDay->SetFocus();
         }
      }
   catch (SSException & x)
      {
      x.AddCallpath("edtProspDayExit", "TfrmAnalysis");
      DisplayBasisException(x);
      }  
}
//---------------------------------------------------------------------------

void __fastcall TfrmAnalysis::chkCensusAreasClick(TObject *Sender)
{
   gpParams->m_bOutputCensusAreas = chkCensusAreas->Checked;
}
//---------------------------------------------------------------------------

void __fastcall TfrmAnalysis::chkLikelyClustersClick(TObject *Sender)
{
   gpParams->m_bMostLikelyClusters = chkLikelyClusters->Checked;
}
//---------------------------------------------------------------------------

void __fastcall TfrmAnalysis::rgCriteriaSecClustersClick(TObject *Sender)
{
   gpParams->m_iCriteriaSecondClusters = rgCriteriaSecClusters->ItemIndex;
}
//---------------------------------------------------------------------------

void __fastcall TfrmAnalysis::edtUnitLengthExit(TObject *Sender)
{
   bool bParamsOk;
   bParamsOk = Check_IntervalLength(atoi(edtStartYear->Text.c_str()), atoi(edtStartMonth->Text.c_str()), atoi(edtStartDay->Text.c_str()),
                atoi(edtEndYear->Text.c_str()), atoi(edtEndMonth->Text.c_str()), atoi(edtEndDay->Text.c_str()),
                gpParams->m_nIntervalUnits, atoi(edtUnitLength->Text.c_str()));
   if (!bParamsOk)             
      {
      PageControl1->ActivePage = tbTimeParameter;
      edtUnitLength->SetFocus();
      }
}
//---------------------------------------------------------------------------

void __fastcall TfrmAnalysis::edtLogPerYearExit(TObject *Sender)
{
   double dValue = atof(edtLogPerYear->Text.c_str());
   if (edtLogPerYear->Text.IsEmpty())
      {
      MessageBox(NULL, "The percentage per year value can not be blank.", "Parameter Error" , MB_OK);
      PageControl1->ActivePage = tbTimeParameter;
      edtLogPerYear->SetFocus();
      }
   else if (!Check_TimeTrendPercentage(dValue))
      {
      PageControl1->ActivePage = tbTimeParameter;
      edtLogPerYear->SetFocus();
      }
}
//---------------------------------------------------------------------------




