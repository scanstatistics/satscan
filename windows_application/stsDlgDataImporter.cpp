// $Revision: 1.1 $
//Programmer: Scott Hostovich
//Basis #define __USE_xbDlgBaseImporter

#include "stsSaTScan.h"
#pragma hdrstop


#pragma package(smart_init)
#pragma link "Grids_ts"
#pragma link "TSGrid"
#pragma resource "*.dfm"

/** Constructor. */
__fastcall TBDlgDataImporter::TBDlgDataImporter(TComponent* Owner, TfrmAnalysis & AnalysisForm)
                             :TForm(Owner), gAnalysisForm(AnalysisForm) {
  try {
    Init();
    Setup();
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor()","TBDlgDataImporter");
    throw;
  }
}

/** Destructor. */
__fastcall TBDlgDataImporter::~TBDlgDataImporter() {
  try {
    delete gpController;
    delete gpDataModel;
    delete gpImportFile;
    TXDFile().Delete(gsActiveZdFileName);
  }
  catch ( ... ) {}
}

/** Enables Add button for fixed column field definitions. */
void TBDlgDataImporter::AddFixedColDefinitionEnable() {
  btnAddFldDef->Enabled = (edtFieldName->GetTextLen() > 0 &&
                           edtFieldLength->GetTextLen() > 0
                           && edtStartColumn->GetTextLen() > 0 &&
                           StrToInt(edtStartColumn->Text) > 0);
}

/** Adjust file/ file field attributes for import - currently that means settings
    a date filter that SaTScan uses. */
void TBDlgDataImporter::AdjustFileSourceFileAttributes(ZdFile & File) {
  unsigned short        w;
  ZdField             * pField;

  try {
    //Set date filter for dbase date fields
    if (gSourceDataFileType == dBase) {
      for (w=0; w < File.GetNumFields(); w++) {
         pField = File.GetFieldInfo(w);
         if (pField->GetType() == ZD_DATE_FLD)
           pField->SetFormat(ZdField::Filtered, "", new ZdDateFilter("%y/%m/%d"));
      }
    }  
  }
  catch (ZdException &x) {
    x.AddCallpath("AdjustFileSourceFileAttributes()", "TBDlgDataImporter");
    throw;
  }
}

/** Tries to match column headers of data grid to importable fields. */
void TBDlgDataImporter::AutoAlign() {
  int                 i, j, iTimes;
  bool                bFound;

  try {
    ClearImportFieldSelections();
    for (i=0; i < ( int )gvSaTScanVariables.size(); i++)
       for (j=1, bFound=false; j < (int)gvImportFieldChoices.size() && !bFound; j++)
          if (! strcmpi(gvSaTScanVariables[i].GetCString(), *(gvImportFieldChoices[j]))) {
            bFound = true;
            gvImportingFields[i] = gvImportFieldChoices[j];
          }
    tsfieldGrid->Invalidate();
  }
  catch (ZdException &x) {
    x.AddCallpath("AutoAlign()","TBDlgDataImporter");
    throw;
  }
}

/** Causes all panels except iWhich to be invisible. */
void TBDlgDataImporter::BringPanelToFront(int iWhich) {
  try {
    if (iWhich < Start || iWhich > DataMapping)
      ImporterException::GenerateException("Index out of range(%d - %d): %d ", "BringPanelToFront()", ZdException::Normal,
                                             Start, DataMapping, iWhich);

    pnlImportWizard->Visible = (iWhich == Start);
    btnPreviousPanel->Visible = (iWhich != Start);
    pnlFileFormat->Visible = (iWhich == FileType);
    pnlImportData->Visible = (iWhich == DataMapping);
    btnNextPanel->Visible = (iWhich == Start || iWhich == FileType);
    btnExecuteImport->Visible = (iWhich == DataMapping);
    ContinueButtonEnable();
    if (btnNextPanel->Visible && btnNextPanel->Enabled) btnNextPanel->SetFocus();
    if (btnExecuteImport->Visible && btnExecuteImport->Enabled) btnExecuteImport->SetFocus();
  }
  catch (ZdException &x) {
    x.AddCallpath("BringPanelToFront()", "TBDlgDataImporter");
    throw;
  }
}

/** Validates that required SaTScan Variables has been specified with an input
    file field to import from. Displays message if variables are missings. */
void TBDlgDataImporter::CheckForRequiredVariables() {
  size_t                t, tNumRequired;
  ZdString              sMessage;
  std::vector<size_t>   vMissingFieldIndex;

  try {
    switch (cmbInputFileType->ItemIndex) {
      case Case        : tNumRequired = 2;
                         break;
      case Control     : tNumRequired = 2;
                         break;
      case Population  : tNumRequired = 3;
                         break;
      case Coordinates : tNumRequired = 2;
                         break;
      case SpecialGrid : tNumRequired = 2;
                         break;
      default : ZdGenerateException("Unknown file type index: \"%d\"", "CheckForRequiredVariables()", cmbInputFileType->ItemIndex);
    };

    for (t=0; t < gvImportingFields.size() && t < tNumRequired; t++)
       if (*gvImportingFields[t] == gsBlank)
         vMissingFieldIndex.push_back(t);

    if (vMissingFieldIndex.size()) {
      sMessage << "For the " << cmbInputFileType->Items->Strings[cmbInputFileType->ItemIndex].c_str();
      sMessage << ", the following SaTScan Variable(s) are required\nand an Input File Variable must";
      sMessage << " be selected for each before import can proceed.\n\nSaTScan Variable(s): ";
      sMessage << gvSaTScanVariables[vMissingFieldIndex[0]] << "\n";
      for (t=1; t < vMissingFieldIndex.size(); t++) {
         sMessage << "                                  " << gvSaTScanVariables[vMissingFieldIndex[t]];
         if (t < vMissingFieldIndex.size() - 1)
           sMessage << "\n";
      }
      BImporterException::GenerateException(sMessage, "CheckForRequiredVariables()", ZdException::Notify);
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("CheckForRequiredVariables()","TBDlgDataImporter");
    throw;
  }
}

/** Clears all the elements of the import fields array. */
void TBDlgDataImporter::ClearImportFieldSelections() {
  try {
    for (size_t i=0; i < gvImportingFields.size(); ++i)
       gvImportingFields[i] = &gsBlank;
    tsfieldGrid->Invalidate();
  }
  catch (ZdException &x) {
    x.AddCallpath("ClearImportFieldSelections()", "TBDlgDataImporter");
    throw;
  }
}

/** */
void TBDlgDataImporter::ContinueButtonEnable() {
  btnNextPanel->Enabled = false;
  btnExecuteImport->Enabled = false;
  switch (*gitrCurrentPanel) {
    case Start       : btnNextPanel->Enabled = FileExists(edtDataFile->Text.c_str()) && cmbInputFileType->ItemIndex > -1;
                       break;
    case FileType    : btnNextPanel->Enabled = (rdoFileType->ItemIndex == 2 ||
                                               (rdoFileType->ItemIndex == 0 && cmbColDelimiter->GetTextLen()) ||
                                               (rdoFileType->ItemIndex == 1 &&  lstFixedColFieldDefs->Items->Count > 0));
                       break;
    case DataMapping : for (size_t i=0; i < gvImportingFields.size() && !btnExecuteImport->Enabled; ++i)
                          btnExecuteImport->Enabled = (*gvImportingFields[i] != gsBlank);

                       break;
    default : ZdGenerateException("Invalid panel index \"%d\".", "ContinueButtonEnable()", *gitrCurrentPanel);
  };
}

/** Creates import destination ZdFile in user's temporary directory based on
    passed file specifications . ZdFileName upon return will contain full path
    of created ZdFile's data file. */
void TBDlgDataImporter::CreateDestinationFile(ZdFileName& sFileName, ZdVector<ZdField*>& vFields) {
  unsigned short        uwOffset(0);
  TXDFile               File;
  char                  sBuffer[1024];
  size_t                i;

  try {
    //Delete any previous ZdFile that importer created but did not import to
    //in this instance of dialog.
    File.Delete(gsActiveZdFileName);

    //Create the temporary ZdFile to be the destination of the import in the users temp directory.
    GetTempPath(sizeof(sBuffer), sBuffer);
    sFileName.SetLocation(sBuffer);
    sFileName.SetExtension(TXD_EXT);

    // creates the field vector from the provided field names
    for (i=0; i < vFields.size(); ++i) {
       vFields[i]->SetIndexCount(0);
       vFields[i]->SetOffset(uwOffset);
       uwOffset += vFields[i]->GetLength();
    }

    File.Delete(sFileName.GetFullPath());
    File.SetTitle(sFileName.GetFileName());
    File.PackFields(vFields);
    File.Create(sFileName.GetFullPath(), vFields, 0, false);
    gsActiveZdFileName = sFileName.GetFullPath();
    File.Open(sFileName.GetFullPath(), ZDIO_OPEN_READ|ZDIO_OPEN_WRITE);
    File.Empty();
    File.Close();
  }
  catch (ZdException &x) {
    x.AddCallpath("CreateDestinationFile()", "TBDlgDataImporter");
    throw;
  }
}

/** Creates ZdIniFile to be used when opening CSV and fixed column file types. */
void TBDlgDataImporter::DefineSourceFileStructure() {
  ZdFileName    fFileName;

  try {
    gDataFileDefinition.Clear();
    if ( rdoFileType->ItemIndex == 0  ) {//CSV File
      CSVFile  fCSVFile( GetColumnDelimiter(), GetGroupMarker() );
      fCSVFile.OpenAsAlpha ( edtDataFile->Text.c_str(), ZDIO_OPEN_READ, false );
      fCSVFile.WriteStructure( &gDataFileDefinition );
    }
    else if ( rdoFileType->ItemIndex == 1 ) {//Variable record length file
      ZdIniSection  tempSection( "[FileInfo]" );
      fFileName.SetFullPath( edtDataFile->Text.c_str() );
      tempSection.AddLine( "FileName", fFileName.GetCompleteFileName() );
      tempSection.AddLine( "Title", fFileName.GetFileName() );
      tempSection.AddLine( "NumberOfFields", IntToStr( ( int )gvIniSections.size() ).c_str() );
      int iRecordLength, iMaxLength = 0;
      for ( unsigned int i = 0; i < gvIniSections.size(); ++i ) {
        iRecordLength = StrToInt( gvIniSections.GetElement( i ).GetString( "Length" ) );
        iRecordLength += StrToInt( gvIniSections.GetElement( i ).GetString( "ByteOffSet" ) );
        iMaxLength = max( iRecordLength, iMaxLength );
      }
      tempSection.AddLine( "RecordLength", IntToStr( iMaxLength ).c_str() );
      tempSection.AddLine( "StartingCol", "1" );
      gDataFileDefinition.AddSection( tempSection.Clone() );
      for ( unsigned int j = 0; j < gvIniSections.size(); ++j )
         gDataFileDefinition.AddSection( new ZdIniSection( gvIniSections.GetElement( j ) ) );
    }
    //else { /* Nothing */ }
  }
  catch (ZdException &x) {
    x.AddCallpath("DefineSourceFileStructure()","TBDlgDataImporter");
    throw;
  }
}

//Enables the deleting of fixed column field definitions.
void TBDlgDataImporter::DeleteFixedColDefinitionEnable() {
  btnDeleteFldDef->Enabled = ( lstFixedColFieldDefs->ItemIndex != -1 );
}

void TBDlgDataImporter::DisableButtonsForImport( bool bEnable ) {
  btnPreviousPanel->Enabled = !bEnable;
  btnExecuteImport->Enabled = !bEnable;
  btnNextPanel->Enabled = !bEnable;
  btnCancel->Enabled = !bEnable;
}

/** Gets column delimter for CSV files. */
const char TBDlgDataImporter::GetColumnDelimiter() const {
  char  cColDelimiter;

  try {
    switch (cmbColDelimiter->ItemIndex) {
      case 0  : cColDelimiter = COMMA;
                break;
      case 1  : cColDelimiter = ';';
               break;
      case 2  : cColDelimiter = '\t';
                break;
      default : ZdString sString( cmbColDelimiter->Text.c_str() );
                cColDelimiter = (sString.GetLength() ? sString[0] : COMMA);
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("GetColumnDelimiter()","TBDlgDataImporter");
    throw;
  }
  return cColDelimiter;
}

ZdString & TBDlgDataImporter::GetFixedColumnFieldName(unsigned int uwFieldIndex, ZdString & sFieldName) {
  try {
    sFieldName.Clear();
    if (uwFieldIndex + 1 > gvFieldDescriptors.size())
      sFieldName.printf("Field %d", uwFieldIndex);
    else
      sFieldName = gvFieldDescriptors[uwFieldIndex]->GetName();
  }
  catch (ZdException &x) {
    x.AddCallpath("GetFixedColumnFieldName()","TBDlgDataImporter");
    throw;
  }
  return sFieldName;
}

/** Gets group marker for CSV files. */
const char  TBDlgDataImporter::GetGroupMarker() const {
  char  cGroupMarker;

  try {
    switch (cmbGroupMarker->ItemIndex) {
      case 0   : cGroupMarker =  DOUBLEQUOTE;
                 break;
      case 1   : cGroupMarker = SINGLEQUOTE;
                 break;
      default  : ZdString sString( cmbGroupMarker->Text.c_str() );
                 cGroupMarker = (sString.GetLength() ? sString[0] : DOUBLEQUOTE);
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("GetGroupMarker()","TBDlgDataImporter");
    throw;
  }
  return cGroupMarker;
}

/** Executes import. */
TModalResult TBDlgDataImporter::ImportFile() {
  TModalResult                  Modal=mrAbort;
  ZdString                      sMessage;

  if (chkFirstRowIsName->Checked)
    gSourceDescriptor.SetNumberOfRowsToIgnore(gSourceDescriptor.GetNumberOfRowsToIgnore() + 1);

  BFileImportSourceInterface    FileImportSourceInterface(gpImportFile, gSourceDescriptor.GetNumberOfRowsToIgnore());
  SaTScanFileImporter           FileImporter((InputFileType)cmbInputFileType->ItemIndex, gSourceDataFileType,
                                             FileImportSourceInterface, gDestDescriptor);

  try {
    SetMappings(FileImporter);
    FileImporter.Import();
    LoadResultFileNameIntoAnalysis();
    Modal = mrOk;
  }
  catch (BImportRejectedException &x) {
    sMessage << "The import wizard encountered an error writing";
    if (FileImporter.GetImportErrors().size()) {
      sMessage << " field " << FileImporter.GetImportErrors()[0].iColumnNumber;
      sMessage << " in line " << FileImporter.GetImportErrors()[0].ulImportFileLine;
      sMessage << " of import source file.";
    }
    else
      sMessage << " data.";
    TBMessageBox(0, "Import cancelled!", sMessage.GetCString(), XBMB_OK|XBMB_EXCLAMATION).ShowModal();
  }
  catch (ZdException &x) {
    x.AddCallpath("ImportFile()", "TBDlgDataImporter");
    throw;
  }
  return Modal; 
}

//Initialize class.
void TBDlgDataImporter::Init() {
  gsBlank << "unassigned";
  gpController = 0;
  gpDataModel = 0;
  gpImportFile = 0;
  cmbColDelimiter->ItemIndex = 0;
  cmbGroupMarker->ItemIndex = 0;
  gSourceDataFileType=Delimited;
}

/** */
void TBDlgDataImporter::InitializeImportingFields() {
  size_t     t;

  try {
    gvImportingFields.clear();
    for (t=0; t < gvSaTScanVariables.size(); t++)
       gvImportingFields.push_back(&gsBlank);
  }
  catch (ZdException &x) {
    x.AddCallpath("InitializeImportingFields()","TBDlgDataImporter");
    throw;
  }
}

/** Preps for viewing field mapping panel. */
void TBDlgDataImporter::LoadMappingPanel() {
  try {
    OpenSourceFile();                          //Display the data in grid
    SetImportFields();                         //Show the fields that can be imported to
    InitializeImportingFields();               //Initialize array that will indicate what fields model that are imported
    SetImportFieldChoices();                   //Initialize array that will populate grid combo dropdown
    AutoAlign();
    ContinueButtonEnable();
  }
  catch (ZdException &x) {
    x.AddCallpath("LoadMappingPanel()","TBDlgDataImporter");
    throw;
  }
}

/** Sets TfrmAnalysis data members to reflect import. */
void TBDlgDataImporter::LoadResultFileNameIntoAnalysis() {
  try {
    switch (cmbInputFileType->ItemIndex) {
      case Case        : gAnalysisForm.SetCaseFile(gDestDescriptor.GetDestinationFileName());
                         break;
      case Control     : gAnalysisForm.SetControlFile(gDestDescriptor.GetDestinationFileName());
                         break;
      case Population  : gAnalysisForm.SetPopulationFile(gDestDescriptor.GetDestinationFileName());
                         break;
      case Coordinates : gAnalysisForm.SetCoordinateFile(gDestDescriptor.GetDestinationFileName());
                         break;
      case SpecialGrid : gAnalysisForm.SetSpecialGridFile(gDestDescriptor.GetDestinationFileName());
                         break;
      default : ZdGenerateException("Unknown file type index: \"%d\"", "LoadResultFileNameIntoAnalysis()", cmbInputFileType->ItemIndex);
    };
  }
  catch (ZdException &x) {
    x.AddCallpath("LoadResultFileNameIntoAnalysis()", "TBDlgDataImporter");
    throw;
  }
}

//Setup for panel to show.
void TBDlgDataImporter::MakePanelVisible(int iWhich) {
  try {
     switch ( iWhich ) {
       case Start       : ContinueButtonEnable();
                          break;
       case FileType    : OnViewFileFormatPanel();
                          break;
       case DataMapping : OnViewMappingPanel();
                          break;
       default : ImporterException::GenerateException("Unknown panel trying to be shown", "MakePanelVisible()");
     }
     BringPanelToFront(iWhich);
  }
  catch (ZdException &x) {
    x.AddCallpath("MakePanelVisible()", "TBDlgDataImporter");
    throw;
  }
}

//Adds fixed column definition( ZdIniSection ).
void TBDlgDataImporter::OnAddFieldDefinitionClick() {
  ZdIniSection  addSection;
  AnsiString    sFieldSection;
  ZdString      sListBox, sFieldName;
  char        * sBuffer = "A";

  try {
    sFieldSection.sprintf("[Field%d]", ( int )gvIniSections.size() + 1);
    addSection.SetName(sFieldSection.c_str());
    addSection.AddLine("FieldName", edtFieldName->Text.c_str());
    addSection.AddLine("Type", sBuffer);
    addSection.AddLine("Length", edtFieldLength->Text.c_str());
    //Add one to Byte off set, memo starts at zero.
    int  iStart = StrToInt(edtStartColumn->Text);
    //iStart++;
    addSection.AddLine("ByteOffset", IntToStr(iStart).c_str());
    gvIniSections.push_back(addSection);

    //Add to listbox
    sListBox.printf(" %-3d     %-3d   %-15s",
                    StrToInt(edtStartColumn->Text.c_str()),
                    StrToInt(edtFieldLength->Text.c_str()),
                    edtFieldName->Text.c_str());
    lstFixedColFieldDefs->Items->Add(sListBox.GetCString());
    edtFieldName->Text = GetFixedColumnFieldName(gvIniSections.size(), sFieldName).GetCString();
    lstFixedColFieldDefs->ItemIndex = -1;
    ContinueButtonEnable();
    edtFieldName->SetFocus();
    edtFieldName->SelStart = 0;
  }
  catch (ZdException &x) {
    x.AddCallpath("OnAddFieldDefinitionClick()","TBDlgDataImporter");
    throw;
  }
}

void TBDlgDataImporter::OnAutoAlignClick() {
  try {
    AutoAlign();
    ContinueButtonEnable();
  }
  catch (ZdException &x) {
    x.AddCallpath("OnAutoAlignClick()","TBDlgDataImporter");
    throw;
  }
}

//Deletes fixed column field definition
void TBDlgDataImporter::OnDeleteFieldDefinitionClick() {
  try {
     if ( lstFixedColFieldDefs->ItemIndex > -1 && lstFixedColFieldDefs->ItemIndex <= ( int )gvIniSections.size() ) {
       gvIniSections.RemoveElement( lstFixedColFieldDefs->ItemIndex );
       lstFixedColFieldDefs->Items->Delete( lstFixedColFieldDefs->ItemIndex );
     }
     DeleteFixedColDefinitionEnable();
     ContinueButtonEnable();
  }
  catch ( ZdException & x ) {
      x.AddCallpath( "OnDeleteFieldDefinitionClick()", "TBDlgDataImporter" );
      throw;
  }
}

//Called when the index of destination combo box changes
void TBDlgDataImporter::OnCmbDestinationChange() {
  try {
    cmbInputFileType->Hint = cmbInputFileType->Text;
    ContinueButtonEnable();
  }
  catch ( ZdException & x ) {
      x.AddCallpath( "OnDestinationChange()", "TBDlgDataImporter" );
      throw;
  }
}

//Response to import click.
void TBDlgDataImporter::OnExecuteImport() {
  TModalResult          Modal;

  try {
    CheckForRequiredVariables();
    DisableButtonsForImport(true);
    Modal = ImportFile();
    DisableButtonsForImport(false);
    //Prompt to import another file?
    switch (Modal) {
      case mrOk    : if (TBMessageBox::Response(this, "Continue?", "Would you like to import another file into this session?", MB_YESNO) == IDYES) {
                       gDestDescriptor.SetDestinationFile("");
                       edtDataFile->Text = "";
                       ShowFirstPanel();
                     }
                     else
                       ModalResult = Modal;
                     break;
      case mrAbort : break;
      default      : ModalResult = Modal;
    };
  }
  catch (ZdException &x) {
    x.AddCallpath("OnExecuteImport()", "TDlgSaTScanDataImporter");
    DisableButtonsForImport(false);
    throw;
  }
}

/** */
void TBDlgDataImporter::OnExitStartPanel() {
  try {
    SetPanelsToShow();
    gitrCurrentPanel = gvPanels.begin();
    switch (cmbInputFileType->ItemIndex) {
      case Case        : SetupCaseFileFieldDescriptors(gvFieldDescriptors);
                         break;
      case Control     : SetupControlFileFieldDescriptors(gvFieldDescriptors);
                         break;
      case Population  : SetupPopFileFieldDescriptors(gvFieldDescriptors);
                         break;
      case Coordinates : SetupGeoFileFieldDescriptors(gvFieldDescriptors);
                         break;
      case SpecialGrid : SetupGridFileFieldDescriptors(gvFieldDescriptors);
                         break;
      default : ZdGenerateException("Unknown file type index: \"%d\"","OnExitStartPanel()", cmbInputFileType->ItemIndex);
    };
  }
  catch (ZdException &x) {
    x.AddCallpath("OnExitStartPanel()","TBDlgDataImporter");
    throw;
  }
}

//Event response to changing import file type.
void TBDlgDataImporter::OnFieldDefinitionChange() {
  int   iStartColumn, iSelStart=0, iSelLength=0;

  if (rdoFileType->ItemIndex == Fixed_Column) {
    AddFixedColDefinitionEnable();
    if (edtStartColumn->GetTextLen()) {
      iStartColumn = StrToInt(edtStartColumn->Text);
      if (iStartColumn > 0) {
        iSelStart = StrToInt(iStartColumn - 1);
        if (edtFieldLength->GetTextLen())
          iSelLength = StrToInt(edtFieldLength->Text);
      }
    }
  }

  memRawData->SelStart = iSelStart;
  memRawData->SelLength = iSelLength;
}

void TBDlgDataImporter::OnFirstRowIsHeadersClick() {
  try {
    SetGridHeaders(chkFirstRowIsName->Checked);
    ClearImportFieldSelections();
    SetImportFieldChoices();
    AutoAlign();
    ContinueButtonEnable();
  }
  catch (ZdException &x) {
    x.AddCallpath("OnFirstRowIsHeadersClick()","TBDlgDataImporter");
    throw;
  }
}

void TBDlgDataImporter::OnViewFileFormatPanel() {
  ZdString      sFieldName;

  try {
    ReadDataFileIntoRawDisplayField();
    ShowFileTypeFormatPanel(rdoFileType->ItemIndex);
    edtFieldName->Text = GetFixedColumnFieldName(0, sFieldName).GetCString();
    ContinueButtonEnable();
    OnFieldDefinitionChange();
  }
  catch (ZdException &x) {
    x.AddCallpath("OnViewFileFormatPanel()","TBDlgDataImporter");
    throw;
  }
}

void TBDlgDataImporter::OnViewMappingPanel() {
  try {
    Screen->Cursor = crHourGlass;
    UpdateFileFormatOptions();
    DefineSourceFileStructure();
    LoadMappingPanel();
    Screen->Cursor = crDefault;
  }
  catch (ZdException &x) {
    x.AddCallpath("OnViewMappingPanel()","TBDlgDataImporter");
    Screen->Cursor = crDefault;
    throw;
  }
}

//Reads in a sample of data file into a memo field to help user to determine
//how to import file.
void TBDlgDataImporter::ReadDataFileIntoRawDisplayField() {
  ZdIO        fImportDataFile;
  char        sFileLineBuffer[1024];

  try {
    memRawData->Clear();
    memRawData->HideSelection = false;
    fImportDataFile.Open(edtDataFile->Text.c_str(), ZDIO_OPEN_READ|ZDIO_SREAD);

    for (int i=0; i < 50 && !fImportDataFile.GetIsEOF(); ++i) {
       fImportDataFile.ReadLine(sFileLineBuffer, sizeof(sFileLineBuffer));
       memRawData->Lines->Add(sFileLineBuffer);
    }
    fImportDataFile.Close();
  }
  catch (ZdException &x) {
    memRawData->Clear();
    memRawData->Lines->Add( "* Unable to view source data file." );
    memRawData->HideSelection = true;
  }
}

//Updates which format options panels are shown.
void TBDlgDataImporter::ShowFileTypeFormatPanel(int iFileType) {
  switch (iFileType) {
    case 0 : pnlCSVDefs->Visible = true;
             pnlFixedColumnDefs->Visible = false;
             memRawData->Visible = true;
             gSourceDataFileType = Delimited;
             break;
    case 1 : pnlCSVDefs->Visible = false;
             pnlFixedColumnDefs->Visible = true;
             memRawData->Visible = true;
             gSourceDataFileType = Fixed_Column;
             break;
    default: pnlCSVDefs->Visible = false;
             pnlFixedColumnDefs->Visible = false;
             memRawData->Visible = false;
  };
}

/** Shows OpenDialog for user to select import source file. Sets open dialog's
    filters based upon SaTScan data file type. */
void TBDlgDataImporter::SelectImportFile() {
  try {
    switch (cmbInputFileType->ItemIndex) {
      case Case        : OpenDialog->Filter = "dBase files (*.dbf)|*.dbf|Delimited files (*.csv)|*.csv|Case files (*.cas)|*.cas|Text files (*.txt)|*.txt|All files (*.*)|*.*";
                         OpenDialog->Title = "Select Source Case File";
                         break;
      case Control     : OpenDialog->Filter = "dBase files (*.dbf)|*.dbf|Delimited files (*.csv)|*.csv|Text files (*.txt)|*.txt|Control files (*.ctl)|*.ctl|Text files (*.txt)|*.txt|All files (*.*)|*.*";
                         OpenDialog->Title = "Select Source Control File";
                         break;
      case Population  : OpenDialog->Filter = "dBase files (*.dbf)|*.dbf|Delimited files (*.csv)|*.csv|Population files (*.pop)|*.pop|Text files (*.txt)|*.txt|All files (*.*)|*.*";
                         OpenDialog->Title = "Select Source Population File";
                         break;
      case Coordinates : OpenDialog->Filter = "dBase files (*.dbf)|*.dbf|Delimited files (*.csv)|*.csv|Coordinates files (*.geo)|*.geo|Text files (*.txt)|*.txt|All files (*.*)|*.*";
                         OpenDialog->Title = "Select Source Coordinates File";
                         break;
      case SpecialGrid : OpenDialog->Filter = "dBase files (*.dbf)|*.dbf|Delimited files (*.csv)|*.csv|Special Grid files (*.grd)|*.grd|Text files (*.txt)|*.txt|All files (*.*)|*.*";
                         OpenDialog->Title = "Select Source Special Grid File";
                         break;
      default : ZdGenerateException("Unknown file type index: \"%d\"","SetImportFields()", cmbInputFileType->ItemIndex);
    };

    OpenDialog->FileName =  "";
    OpenDialog->DefaultExt = "*.dbf";
    OpenDialog->FilterIndex = 0;
    if (OpenDialog->Execute()) {
      edtDataFile->Text = OpenDialog->FileName;
      gSourceDescriptor.SetImportFile(edtDataFile->Text.c_str());
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("SelectImportFile()", "TDlgSaTScanDataImporter");
    throw;
  }
}

//Re-assigns grid headers to that of first row of model if not blank.
void TBDlgDataImporter::SetGridHeaders(bool bFirstRowIsHeader) {
  ZdString    sHeaderValue;
  char        sBuffer[1024];

  try {
     if (gpController) {
       gpController->HideRow(gSourceDescriptor.GetNumberOfRowsToIgnore() + 1, bFirstRowIsHeader);
       for (int i=1; i <= gpController->GetGridColCount(); ++i) {
          sHeaderValue << ZdString::reset;
          if (rdoFileType->ItemIndex == 0 && bFirstRowIsHeader) //Get header from model row
            sHeaderValue = (char *)gpController->GetValueAt(gSourceDescriptor.GetNumberOfRowsToIgnore() + 1, i, (char *)sBuffer, sizeof(sBuffer), true);

          if (sHeaderValue.GetLength())
            gpController->GetTopGrid()->Col[i]->Heading = sHeaderValue.GetCString();
          else
             gpController->UpdateColumnHeading(i);
       }
       gpController->GetTopGrid()->TopRow = 1; //Cause first row to be visible.
     }
  }
  catch (ZdException &x) {
    x.AddCallpath("SetGridHeaders()","TBDlgDataImporter");
    throw;
  }
}

/** Creates import destination file  based on SaTScan input file and loads
    mapping interface based on created file. */
void TBDlgDataImporter::SetImportFields() {
  ZdFileName                    sFileName;
  unsigned short                uwPrimaryKeys, i;
  ZdFile                      * pFile = 0;

  try {
    sFileName = edtDataFile->Text.c_str();
    CreateDestinationFile(sFileName, gvFieldDescriptors);
    gDestDescriptor.SetDestinationFile(sFileName.GetFullPath());
    //Set SaTScan variables
    pFile = BasisGetToolkit().OpenZdFile(gDestDescriptor.GetDestinationFileName().GetCString(), ZDIO_OPEN_READ|ZDIO_OPEN_WRITE, ZDIO_SREAD, 60, 2000);
    gvSaTScanVariables.clear();
    for (i=0; i < pFile->GetNumFields(); i++)
      gvSaTScanVariables.push_back(ZdString(pFile->GetFieldInfo(i)->GetName()));
    tsfieldGrid->Rows = pFile->GetNumFields();
    BasisGetToolkit().CloseZdFile(pFile, ZDIO_SREAD);
  }
  catch (ZdException &x) {
    x.AddCallpath("SetImportFields()","TBdlgImporter");
    if (pFile)
      BasisGetToolkit().CloseZdFile(pFile, ZDIO_SREAD);
    throw;
  }
}

/** Makes column headers of data grid combobox items of selection grid. */
void TBDlgDataImporter::SetImportFieldChoices() {
  try {
    gvImportFieldChoices.DeleteAllElements();
    gvImportFieldChoices.push_back(gsBlank.Clone());
    for (int i=1; gpController && i <= gpController->GetGridColCount(); i++)
        gvImportFieldChoices.push_back(new ZdString(gpController->GetTopGrid()->Col[i]->Heading.c_str()));
    tsfieldGrid->Invalidate();
  }
  catch (ZdException &x) {
    x.AddCallpath("SetImportFieldChoices()","TBdlgBaseImporter");
    throw;
  }
}

//Tell import class what's imported from where.
void TBDlgDataImporter::SetMappings(BZdFileImporter & FileImporter) {
  size_t                                i;
  ZdPointerVector<ZdString>::iterator   itr;
  ZdFile                              * pFile;
  const ZdField                       * pToField;

  try {
    pFile = FileImporter.GetRemoteFile();

    //Find the string of gvImportingFields in gvImportFieldChoices
    //Element index in gvImportFieldChoices is column number of import file.
    for (i=0; i < gvImportingFields.size(); i++)
         if (*gvImportingFields[i] != gsBlank) {
           itr = std::find(gvImportFieldChoices.begin(), gvImportFieldChoices.end(), gvImportingFields[i]);
           if (itr !=  gvImportFieldChoices.end()) {
             pToField =  pFile->GetFieldInfo(gvSaTScanVariables[i].GetCString());
             FileImporter.AddMapping(gvImportFieldChoices.GetPosition(itr), pToField);
           }
         }
  }
  catch ( ZdException & x ) {
      x.AddCallpath( "SetMappings()", "TBDlgDataImporter" );
      throw;
  }
}

//Sets which panels and the order of showing panels.
void TBDlgDataImporter::SetPanelsToShow() {
  try {
    if (! strcmpi(ZdFileName(edtDataFile->Text.c_str()).GetExtension(), ZdDBFFileType.GetFileTypeExtension())) {
      gvPanels.clear();
      gvPanels.push_back(Start);
      gvPanels.push_back(DataMapping);
      gSourceDataFileType = dBase;
      rdoFileType->ItemIndex = -1;
    }
    else {
      gvPanels.clear();
      gvPanels.push_back(Start);
      gvPanels.push_back(FileType);
      gvPanels.push_back(DataMapping);
      gSourceDataFileType = Delimited;
      rdoFileType->ItemIndex = 0;
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("SetPanelsToShow()","TBDlgDataImporter");
    throw;
  }
}

/** Internal setup function. */
void TBDlgDataImporter::Setup() {
  try {
    gDestDescriptor.SetDestinationType(BFileDestDescriptor::SingleFile);
    gDestDescriptor.SetModifyType(BFileDestDescriptor::OverWriteExistingData);
    gDestDescriptor.SetGenerateReport(false);
    gDestDescriptor.SetErrorOptionsType(BFileDestDescriptor::RejectBatch);
    cmbInputFileType->ItemIndex = 0;
  }
  catch (ZdException &x) {
    x.AddCallpath("SetUp()","TBDlgDataImporter");
    throw;
  }
}

//Prepares grid for sample viewing of import file.
//At this point, we have a zds file
void TBDlgDataImporter::OpenSourceFile() {
  try {
    delete gpController; gpController = 0;
    delete gpDataModel; gpDataModel = 0;
    delete gpImportFile; gpImportFile = 0;

    try {
      if (rdoFileType->ItemIndex == 0) {//Special ZdFile open for CSVFile
        gpImportFile = new CSVFile(edtDataFile->Text.c_str(), ZDIO_OPEN_READ, 0, 0, &gDataFileDefinition, GetColumnDelimiter(), GetGroupMarker());
        chkFirstRowIsName->Enabled = true;
      }
      else if (rdoFileType->ItemIndex == 1) {//Special ZdFile open for variable txd
        gpImportFile = new TXVFile();
        gpImportFile->Open(edtDataFile->Text.c_str(), ZDIO_OPEN_READ|ZDIO_SREAD, 0, 0, &gDataFileDefinition);
        chkFirstRowIsName->Checked = false;
        chkFirstRowIsName->Enabled = false;
      }
      else {//Open as a ZdFileType
         gpImportFile = ZdOpenFile(edtDataFile->Text.c_str(), ZDIO_OPEN_READ|ZDIO_SREAD);
         chkFirstRowIsName->Checked = false;
         chkFirstRowIsName->Enabled = false;
      }
      AdjustFileSourceFileAttributes(*gpImportFile);
      gpDataModel = new BGridZdSingleFileModel(gpImportFile);
      gpController = new BZdFileViewController(tsImportFileGrid, gpDataModel);
     }
     catch (ZdException &x) {
       ImporterException::GenerateException("Import Information\nThe import wizard was unable to read file \"%s\".\nPlease confirm the file format settings for this file.",
                                            "OpenSourceFile()", ZdException::Notify, edtDataFile->Text.c_str());
     }

     gpController->EmptyContextMenus();
     if ((unsigned long)gSourceDescriptor.GetNumberOfRowsToIgnore() >= gpImportFile->GetNumRecords())
       ImporterException::GenerateException("Import Information\nSettings indicate to ignore %d rows but import file contains %d rows.\nPlease review settings.",
                                            "OpenSourceFile()", ZdException::Notify,
                                            gSourceDescriptor.GetNumberOfRowsToIgnore(), gpImportFile->GetNumRecords());
     for (int i=1; i <= gSourceDescriptor.GetNumberOfRowsToIgnore(); ++i)
        gpController->HideRow(i, true);
     SetGridHeaders(chkFirstRowIsName->Checked && chkFirstRowIsName->Enabled);
  }
  catch (ZdException &x) {
    x.AddCallpath("OpenSourceFile()","TBDlgDataImporter");
    delete gpImportFile; gpImportFile=0;
    delete gpDataModel; gpDataModel=0;
    delete gpController; gpController=0;
    throw;
  }
}

// fill the Case File field descriptor vector with the appropriate field names for a case file
void TBDlgDataImporter::SetupCaseFileFieldDescriptors(ZdPointerVector<ZdField>& vFields) {
  try {
    vFields.DeleteAllElements();
    vFields.push_back(new ZdField("Tract ID", 1, ZD_ALPHA_FLD, 200));
    vFields.push_back(new ZdField("Number of Cases", 2, ZD_ALPHA_FLD, 50));
    vFields.push_back(new ZdField("Date/Time (optional)", 3, ZD_ALPHA_FLD, 50));
    vFields.push_back(new ZdField("Covariate1 (optional)", 4, ZD_ALPHA_FLD, 50));
    vFields.push_back(new ZdField("Covariate2 (optional)", 5, ZD_ALPHA_FLD, 50));
    vFields.push_back(new ZdField("Covariate3 (optional)", 6, ZD_ALPHA_FLD, 50));
    vFields.push_back(new ZdField("Covariate4 (optional)", 7, ZD_ALPHA_FLD, 50));
    vFields.push_back(new ZdField("Covariate5 (optional)", 8, ZD_ALPHA_FLD, 50));
    vFields.push_back(new ZdField("Covariate6 (optional)", 9, ZD_ALPHA_FLD, 50));
    vFields.push_back(new ZdField("Covariate7 (optional)", 10, ZD_ALPHA_FLD, 50));
    vFields.push_back(new ZdField("Covariate8 (optional)", 11, ZD_ALPHA_FLD, 50));
    vFields.push_back(new ZdField("Covariate9 (optional)", 12, ZD_ALPHA_FLD, 50));
    vFields.push_back(new ZdField("Covariate10 (optional)", 13, ZD_ALPHA_FLD, 50));
  }
  catch (ZdException &x) {
    x.AddCallpath("SetupCaseFileFieldDescriptors()", "TBDlgDataImporter");
    throw;
  }
}

// fill the control File field descriptor vector with the appropriate field names for a control file
void TBDlgDataImporter::SetupControlFileFieldDescriptors(ZdPointerVector<ZdField>& vFields) {
  try {
    vFields.DeleteAllElements();
    vFields.push_back(new ZdField("Tract ID", 1, ZD_ALPHA_FLD, 200));
    vFields.push_back(new ZdField("Number of Controls", 2, ZD_ALPHA_FLD, 50));
    vFields.push_back(new ZdField("Date/Time (optional)", 3, ZD_ALPHA_FLD, 50));
  }
  catch (ZdException &x) {
    x.AddCallpath("SetupControlFileFieldDescriptors()", "TBDlgDataImporter");
    throw;
  }
}

// fill the Geo File field descriptor vector with the appropriate field names for a geo file
void TBDlgDataImporter::SetupGeoFileFieldDescriptors(ZdPointerVector<ZdField>& vFields) {
  try {
    vFields.DeleteAllElements();
    vFields.push_back(new ZdField("Tract ID", 1, ZD_ALPHA_FLD, 200));
    vFields.push_back(new ZdField("Latitude", 2, ZD_ALPHA_FLD, 50));
    vFields.push_back(new ZdField("Longitude", 3, ZD_ALPHA_FLD, 50));
    vFields.push_back(new ZdField("X", 4, ZD_ALPHA_FLD, 50));
    vFields.push_back(new ZdField("Y", 5, ZD_ALPHA_FLD, 50));
    vFields.push_back(new ZdField("Z1 (optional)", 6, ZD_ALPHA_FLD, 50));
    vFields.push_back(new ZdField("Z2 (optional)", 7, ZD_ALPHA_FLD, 50));
    vFields.push_back(new ZdField("Z3 (optional)", 8, ZD_ALPHA_FLD, 50));
    vFields.push_back(new ZdField("Z4 (optional)", 9, ZD_ALPHA_FLD, 50));
    vFields.push_back(new ZdField("Z5 (optional)", 10, ZD_ALPHA_FLD, 50));
    vFields.push_back(new ZdField("Z6 (optional)", 11, ZD_ALPHA_FLD, 50));
    vFields.push_back(new ZdField("Z7 (optional)", 12, ZD_ALPHA_FLD, 50));
    vFields.push_back(new ZdField("Z8 (optional)", 13, ZD_ALPHA_FLD, 50));
  }
  catch (ZdException &x) {
    x.AddCallpath("SetupGeoFileFieldDescriptors()", "TBDlgDataImporter");
    throw;
  }
}

// fill the Geo File field descriptor vector with the appropriate field names for a geo file
void TBDlgDataImporter::SetupGridFileFieldDescriptors(ZdPointerVector<ZdField>& vFields) {
  try {
    vFields.DeleteAllElements();
    vFields.push_back(new ZdField("Latitude", 1, ZD_ALPHA_FLD, 50));
    vFields.push_back(new ZdField("Longitude", 2, ZD_ALPHA_FLD, 50));
    vFields.push_back(new ZdField("X", 3, ZD_ALPHA_FLD, 50));
    vFields.push_back(new ZdField("Y", 4, ZD_ALPHA_FLD, 50));
    vFields.push_back(new ZdField("Z1 (optional)", 5, ZD_ALPHA_FLD, 50));
    vFields.push_back(new ZdField("Z2 (optional)", 6, ZD_ALPHA_FLD, 50));
  }
  catch (ZdException &x) {
    x.AddCallpath("SetupGridFileFieldDescriptors()", "TBDlgDataImporter");
    throw;
  }
}

// fill the Geo File field descriptor vector with the appropriate field names for a geo file
void TBDlgDataImporter::SetupPopFileFieldDescriptors(ZdPointerVector<ZdField>& vFields) {
  try {
    vFields.DeleteAllElements();
    vFields.push_back(new ZdField("Tract ID", 1, ZD_ALPHA_FLD, 200));
    vFields.push_back(new ZdField("Date/Time", 2, ZD_ALPHA_FLD, 50));
    vFields.push_back(new ZdField("Population", 3, ZD_ALPHA_FLD, 50));
    vFields.push_back(new ZdField("Covariate1 (optional)", 4, ZD_ALPHA_FLD, 50));
    vFields.push_back(new ZdField("Covariate2 (optional)", 5, ZD_ALPHA_FLD, 50));
    vFields.push_back(new ZdField("Covariate3 (optional)", 6, ZD_ALPHA_FLD, 50));
  }
  catch (ZdException &x) {
    x.AddCallpath("SetupPopFileFieldDescriptors()", "TBDlgDataImporter");
    throw;
  }
}

//Causes the first panel of importer to be shown.
void TBDlgDataImporter::ShowFirstPanel() {
  try {
    gitrCurrentPanel = gvPanels.begin();
    MakePanelVisible(*gitrCurrentPanel);
  }
  catch (ZdException &x) {
    x.AddCallpath("ShowFirstPanel()", "TBDlgDataImporter");
    throw;
  }
}

//Causes next panel of importer to be shown.
void TBDlgDataImporter::ShowNextPanel() {
  try {
    //Note attributes of current panel when needed.
    if (*gitrCurrentPanel == Start)
      OnExitStartPanel();

    if (*gitrCurrentPanel != gvPanels.back()) {
      ++gitrCurrentPanel;
      try {
        MakePanelVisible(*gitrCurrentPanel);
      }
      catch (ZdException &x) {
        --gitrCurrentPanel;
        throw;
      }
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("ShowNextPanel()", "TBDlgDataImporter");
    throw;
  }
}

//Causes previous panel of importer to be shown.
void TBDlgDataImporter::ShowPreviousPanel() {
  try {
    if (gitrCurrentPanel != gvPanels.begin()) {
      --gitrCurrentPanel;
      try {
        BringPanelToFront(*gitrCurrentPanel);
      }
      catch (ZdException &x) {
        ++gitrCurrentPanel;
        throw;
      }
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("ShowPreviousPanel()", "TBDlgDataImporter");
    throw;
  }
}

void TBDlgDataImporter::UpdateFileFormatOptions() {
  int  iIgnoreFirstRows = 0;

  try {
    if (edtIgnoreFirstRows->Text.Length())
      iIgnoreFirstRows = Sysutils::StrToInt(edtIgnoreFirstRows->Text.Trim());
    gSourceDescriptor.SetNumberOfRowsToIgnore(iIgnoreFirstRows);
  }
  catch (ZdException &x) {
    x.AddCallpath("UpdateFileFormatOptions()","TBDlgDataImporter");
    throw;
  }
}

//////////////////////////////
/////// fastcalls  ///////////
//////////////////////////////

void __fastcall TBDlgDataImporter::NumericKeyPressMask(TObject *Sender, char &Key) {
    if ( !( Key == '0' || Key == '1' || Key == '2' || Key == '3' || Key == '4' ||
            Key == '5' || Key == '6' || Key == '7' || Key == '8' || Key == '9' || Key == '\b' ) )
      Key = NULL;
}

void __fastcall TBDlgDataImporter::OnAddFldDefClick(TObject *Sender) {
  try {
    OnAddFieldDefinitionClick();
  }
  catch (ZdException &x) {
    x.AddCallpath("OnAddFldDefClick()","TBDlgDataImporter");
    DisplayBasisException(this,x);
  }

}
void __fastcall TBDlgDataImporter::OnAutoAlignClick(TObject *Sender) {
  try {
    OnAutoAlignClick();
  }
  catch (ZdException &x) {
    x.AddCallpath("OnAutoAlignClick()","TBDlgDataImporter");
    DisplayBasisException(this,x);
  }
}

void __fastcall TBDlgDataImporter::OnBackClick(TObject *Sender) {
  try {
    ShowPreviousPanel();
  }
  catch (ZdException &x) {
    x.AddCallpath("OnBackClick()","TBDlgDataImporter");
    DisplayBasisException(this,x);
  }
}

void __fastcall TBDlgDataImporter::OnBrowseForImportFile(TObject *Sender) {
  try {
    SelectImportFile();
    ContinueButtonEnable();
  }
  catch (ZdException &x) {
    x.AddCallpath("OnBrowseForImportFile()","TBDlgDataImporter");
    DisplayBasisException(this,x);
  }
}
void __fastcall TBDlgDataImporter::OnClearImportsClick(TObject *Sender) {
  try {
    ClearImportFieldSelections();
    ContinueButtonEnable();
  }
  catch (ZdException &x) {
    x.AddCallpath("OnClearImportsClick()","TBDlgDataImporter");
    DisplayBasisException(this, x);
  }
}

void __fastcall TBDlgDataImporter::OnCmbDestinationChange(TObject *Sender) {
  try {
    OnCmbDestinationChange();
  }
  catch (ZdException &x) {
    x.AddCallpath("OnCmbDestinationChange()","TBDlgDataImporter");
    DisplayBasisException(this,x);
  }
}

void __fastcall TBDlgDataImporter::OnCSVComboChange(TObject *Sender) {
  try {
    ContinueButtonEnable();
  }
  catch (ZdException &x) {
    x.AddCallpath("OnCSVComboChange()", "TBDlgDataImporter");
    DisplayBasisException( this, x );
  }
}

void __fastcall TBDlgDataImporter::OnDeleteFldDefClick(TObject *Sender) {
  try {
    OnDeleteFieldDefinitionClick();
  }
  catch (ZdException &x) {
    x.AddCallpath("OnDeleteFldDefClick()","TBDlgDataImporter");
    DisplayBasisException(this,x);
  }
}

void __fastcall TBDlgDataImporter::OnExecuteClick(TObject *Sender) {
  try {
    OnExecuteImport();
  }
  catch (ZdException &x) {
    x.AddCallpath("OnExecuteClick()","TBDlgDataImporter");
    DisplayBasisException(this,x);
  }
}

void __fastcall TBDlgDataImporter::OnFieldGridCellLoaded(TObject *Sender, int DataCol, int DataRow, Variant &Value) {
  try {
    switch ( DataCol ) {
      case  1  :     Value = gvSaTScanVariables[DataRow - 1].GetCString();
                     break;
      case  2  :     Value = gvImportingFields[DataRow - 1]->GetCString();
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("OnFieldGridCellLoaded()","TBDlgDataImporter");
    DisplayBasisException(this,x);
  }
}

void __fastcall TBDlgDataImporter::OnFieldGridComboCellLoaded(TObject *Sender, TtsComboGrid *Combo, int DataCol, int DataRow, Variant &Value) {
  try {
    Combo->Grid->Cell[DataCol][DataRow] = gvImportFieldChoices[DataRow - 1]->GetCString();
  }
  catch (ZdException &x) {
    x.AddCallpath("OnFieldGridComboCellLoaded()","TBDlgDataImporter");
    DisplayBasisException(this, x);
  }
}

void __fastcall TBDlgDataImporter::OnFieldGridComboDropDown(TObject *Sender, TtsComboGrid *Combo, int DataCol, int DataRow) {
  try {
    Combo->Grid->Rows = (int)gvImportFieldChoices.size();
  }
  catch (ZdException &x) {
    x.AddCallpath("OnFieldGridComboDropDown()","TBDlgDataImporter");
    DisplayBasisException(this,x);
  }
}

void __fastcall TBDlgDataImporter::OnFieldGridComboGetValue(TObject *Sender,TtsComboGrid *Combo, int GridDataCol, int GridDataRow, int ComboDataRow, Variant &Value) {
  try {
    gvImportingFields[GridDataRow - 1] = gvImportFieldChoices[ComboDataRow - 1];
    ContinueButtonEnable();
  }
  catch (ZdException &x) {
    x.AddCallpath("OnFieldGridComboGetValue()","TBDlgDataImporter");
    DisplayBasisException(this,x);
  }
}

void __fastcall TBDlgDataImporter::OnFieldLengthChange(TObject *Sender) {
  try {
    OnFieldDefinitionChange();
  }
  catch (ZdException &x) {
    x.AddCallpath("OnFieldLengthChange()","TBDlgDataImporter");
    DisplayBasisException(this,x);
  }
}

void __fastcall TBDlgDataImporter::OnFieldNameChange(TObject *Sender) {
  try {
    AddFixedColDefinitionEnable();
  }
  catch (ZdException &x) {
    x.AddCallpath("OnFieldNameChange()","TBDlgDataImporter");
    DisplayBasisException(this,x);
  }
}

void __fastcall TBDlgDataImporter::OnFieldTypeChange(TObject *Sender) {
  try {
    AddFixedColDefinitionEnable();
  }
  catch (ZdException &x) {
    x.AddCallpath("OnFieldTypeChange()", "TBDlgDataImporter");
    DisplayBasisException(this, x);
  }
}

void __fastcall TBDlgDataImporter::OnFileTypeClick(TObject *Sender) {
  try {
    ShowFileTypeFormatPanel(rdoFileType->ItemIndex);
    ContinueButtonEnable();
    OnFieldDefinitionChange();
  }
  catch (ZdException &x) {
    x.AddCallpath("OnFileTypeClick()", "TBDlgDataImporter");
    DisplayBasisException(this, x);
  }
}

void __fastcall TBDlgDataImporter::OnFirstRowIsNameClick(TObject *Sender) {
  try {
    OnFirstRowIsHeadersClick();
  }
  catch (ZdException &x) {
    x.AddCallpath("OnFirstRowIsNameClick()","TBDlgDataImporter");
    DisplayBasisException(this,x);
  }
}

/** Puts clicked fixed column definition back into appropriate field definition boxes. */
void __fastcall TBDlgDataImporter::OnFixedColFieldDefsClick(TObject *Sender) {
  ZdIniSection    tempSection;
  int             iStart;
  
  try {
    if (lstFixedColFieldDefs->ItemIndex > -1 && lstFixedColFieldDefs->ItemIndex <= (int)gvIniSections.size()) {
      tempSection = gvIniSections.GetElement(lstFixedColFieldDefs->ItemIndex);
      iStart = StrToInt(tempSection.GetLine(3)->GetValue());
      edtStartColumn->Text = IntToStr(iStart );
      edtFieldLength->Text = tempSection.GetLine(2)->GetValue();
      edtFieldName->Text = tempSection.GetLine(0)->GetValue();
    }
    DeleteFixedColDefinitionEnable();
  }
  catch (ZdException &x) {
    x.AddCallpath("OnFixedColFieldDefsClick()","TBDlgDataImporter");
    DisplayBasisException(this, x);
  }
}

void __fastcall TBDlgDataImporter::OnFormShow(TObject *Sender) {
  try {
    SetPanelsToShow();
    ShowFirstPanel();
  }
  catch (ZdException &x) {
    x.AddCallpath("OnFormShow()","TBDlgDataImporter");
    DisplayBasisException(this, x);
  }
}

void __fastcall TBDlgDataImporter::OnNextClick(TObject *Sender) {
  try {
    ShowNextPanel();
  }
  catch (ZdException &x) {
    x.AddCallpath("OnNextClick()","TBDlgDataImporter");
    DisplayBasisException(this, x);
  }
}

/** Actions taken when source data file editboxt is changed. */
void __fastcall TBDlgDataImporter::OnSourceImportFileChange(TObject *Sender) {
  try {
    edtDataFile->Hint = edtDataFile->Text;
    ContinueButtonEnable();
  }
  catch (ZdException &x) {
    x.AddCallpath("OnSourceImportFileChange()","TBDlgDataImporter");
    DisplayBasisException(this, x);
  }
}

/** Event response to changing import file type. */
void __fastcall TBDlgDataImporter::OnStartColumnChange(TObject *Sender) {
  try {
    OnFieldDefinitionChange();
  }
  catch (ZdException &x) {
    x.AddCallpath("OnStartColumnChange()","TBDlgDataImporter");
    DisplayBasisException(this, x);
  }
}
//////////////////////////
/////Exception Class /////
//////////////////////////

ImporterException::ImporterException(const char * sMessage, const char * sSourceModule, ZdException::Level iLevel)
                :BException(sMessage, sSourceModule, iLevel){
}

void ImporterException::GenerateException ( const char * sMessage, const char * sSourceModule, ZdException::Level iLevel, ... ) {
   va_list   vArgs;
   char      sExceptionString[2048];

   va_start ( vArgs, iLevel );
   vsprintf ( sExceptionString, sMessage, vArgs );
   va_end ( vArgs );

   throw ImporterException(sExceptionString, sSourceModule, iLevel);
}

void ImporterException::GenerateException( const char * sMessage, const char * sSourceModule ) {
   throw ImporterException(sMessage, sSourceModule, ZdException::Normal);
}


/** Event triggered when mapping grid is resized. Intended to cause second
    columns combobox buttons to be visible without user needing to resize
    column widths.*/

void __fastcall TBDlgDataImporter::tsfieldGridResize(TObject *Sender) {
  //Adjust width of 'input file variable' column to fit.
  if (!tsfieldGrid->VertScrollBarVisible)
    tsfieldGrid->Col[2]->Width = tsfieldGrid->Width - ((tsfieldGrid->RowBarOn ? tsfieldGrid->RowBarWidth + 5/*buffer*/ : 5/*buffer*/) + tsfieldGrid->Col[1]->Width);
  else
    tsfieldGrid->Col[2]->Width = tsfieldGrid->Width - ((tsfieldGrid->RowBarOn ? tsfieldGrid->RowBarWidth + 5/*buffer*/ : 5/*buffer*/) + tsfieldGrid->Col[1]->Width + 15);
}
//---------------------------------------------------------------------------






