//---------------------------------------------------------------------------
#include "stsSaTScan.h"
#pragma hdrstop
//---------------------------------------------------------------------------

#pragma package(smart_init)
#pragma link "Grids_ts"
#pragma link "TSGrid"
#pragma resource "*.dfm"

/** Constructor */
SourceViewController::SourceViewController(TtsGrid * pTopGrid, BGridZdAbstractFileModel * pGridDataModel)
                     :BZdFileViewController(pTopGrid, pGridDataModel) {}

/** Event triggered when top grid asks for a cell value. */
void SourceViewController::OnCellLoaded(int DataCol, int DataRow, Variant &Value) {
  try {
    BZdFileViewController::OnCellLoaded(DataCol, DataRow, Value);
  }
  catch(ZdException &x) {/*trap to prevent repeating exception*/}
}

/** Constructor */
SaTScanVariable::SaTScanVariable(const char * sVariableName, short wTargetFieldIndex, bool bRequiredVariable) {
  try {
    Init();
    SetVariableName(sVariableName);
    SetTargetFieldIndex(wTargetFieldIndex);
    SetIsRequiredField(bRequiredVariable);
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor()","SaTScanVariable");
    throw;
  }
}

/**  Copy Constructor */
SaTScanVariable::SaTScanVariable(const SaTScanVariable & rhs) {
  try {
    Init();
    Copy(rhs);
  }
  catch (ZdException &x) {
    x.AddCallpath("Copy constructor", "SaTScanVariable");
    throw;
  }
}

/** Destructor */
SaTScanVariable::~SaTScanVariable() {}

/** Overload of assignment operator */
SaTScanVariable & SaTScanVariable::operator=(const SaTScanVariable & rhs) {
  try {
    if (this != &rhs)
      Copy(rhs);
  }
  catch (ZdException &x) {
    x.AddCallpath("Copy()", "SaTScanVariable");
    throw;
  }
  return (*this);
}

/** Returns newly cloned object. */ 
SaTScanVariable * SaTScanVariable::Clone() const {
  return new SaTScanVariable(*this);
}

/** Internal function to copy the class */
void SaTScanVariable::Copy(const SaTScanVariable & rhs) {
  try {
    SetVariableName(rhs.gsVariableName);
    SetTargetFieldIndex(rhs.gwTargetFieldIndex);
    SetIsRequiredField(rhs.gbRequiredVariable);
    SetInputFileVariableIndex(rhs.gwInputFileVariableIndex);
  }
  catch (ZdException &x) {
    x.AddCallpath("Copy()", "SaTScanVariable");
    throw;
  }
}

/** Returns variable display name. */
void SaTScanVariable::GetVariableDisplayName(Variant & Value) const {
  if (! gbRequiredVariable) {
    ZdString      sTemp;

    sTemp << gsVariableName << " (optional)";
    Value = sTemp.GetCString();
  }
  else
    Value = gsVariableName.GetCString();
}

/** Internal initialization function. */
void SaTScanVariable::Init() {
  gwTargetFieldIndex=-1;
  gbRequiredVariable=true;
  gwInputFileVariableIndex=0;
}


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
    delete gpSourceFile;
  }
  catch ( ... ) {}
}

/** Enables Add button for fixed column field definitions. */
void TBDlgDataImporter::AddFixedColDefinitionEnable() {
  btnAddFldDef->Enabled = (!gbErrorSamplingSourceFile && edtFieldName->GetTextLen() > 0 &&
                           edtFieldLength->GetTextLen() > 0 && StrToInt(edtFieldLength->Text) > 0
                           && edtStartColumn->GetTextLen() > 0 && StrToInt(edtStartColumn->Text) > 0);
}

/** Adjust file/ file field attributes for import
    - currently this function only applies to dBase files.
      Sets date filter to YYYY/MM/DD. */
void TBDlgDataImporter::AdjustSourceFileAttributes(ZdFile & File) {
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
    x.AddCallpath("AdjustSourceFileAttributes()", "TBDlgDataImporter");
    throw;
  }
}

/** Tries to match column headers of data grid to importable fields. */
void TBDlgDataImporter::AutoAlign() {
  size_t              i, j, iNumInputVariables;
  bool                bFound;

  try {
    iNumInputVariables = GetNumInputFileVariables();
    for (i=0; i < gvSaTScanVariables.size(); i++)
       for (j=1, bFound=false; j < iNumInputVariables && !bFound; j++)
          if (! strcmpi(gvSaTScanVariables[i].GetVariableName().GetCString(), GetInputFileVariableName(j))) {
            bFound = true;
            gvSaTScanVariables[i].SetInputFileVariableIndex(j);
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
    chkFirstRowIsName->Visible = (iWhich == DataMapping);
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
  size_t                t;
  ZdString              sMessage;
  std::vector<size_t>   vMissingFieldIndex;

  try {
    for (t=0; t < gvSaTScanVariables.size(); t++)
       if (tsfieldGrid->RowVisible[t+1] && !gvSaTScanVariables[t].GetIsMappedToInputFileVariable() && gvSaTScanVariables[t].GetIsRequiredField())
         vMissingFieldIndex.push_back(t);

    if (vMissingFieldIndex.size()) {
      sMessage << "For the " << rdgInputFileType->Items->Strings[rdgInputFileType->ItemIndex].c_str();
      sMessage << ", the following SaTScan Variable(s) are required\nand an Input File Variable must";
      sMessage << " be selected for each before import can proceed.\n\nSaTScan Variable(s): ";
      for (t=0; t < vMissingFieldIndex.size(); t++) {
         sMessage << gvSaTScanVariables[vMissingFieldIndex[t]].GetVariableName();
         if (t < vMissingFieldIndex.size() - 1)
           sMessage << ", ";
      }
      BImporterException::GenerateException(sMessage, "CheckForRequiredVariables()", ZdException::Notify);
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("CheckForRequiredVariables()","TBDlgDataImporter");
    throw;
  }
}

/** Clears field mapping selections. */
void TBDlgDataImporter::ClearSaTScanVariableFieldIndexes() {
  try {
    for (size_t t=0; t < gvSaTScanVariables.size(); t++)
       gvSaTScanVariables[t].SetInputFileVariableIndex(0);
    tsfieldGrid->Invalidate();
  }
  catch (ZdException &x) {
    x.AddCallpath("ClearSaTScanVariableFieldIndexes()","TBDlgDataImporter");
    throw;
  }
}

/** Clears ZdIniSections for fixed column file. */
void TBDlgDataImporter::ClearFixedColumnDefinitions() {
  ZdString      sFieldName;

  try {
    gvIniSections.clear();
    lstFixedColFieldDefs->Items->Clear();
    edtStartColumn->Text = "1";
    edtFieldLength->Text = "1";
    edtFieldName->Text = GetFixedColumnFieldName(1, sFieldName).GetCString();
    AddFixedColDefinitionEnable();
    DeleteFixedColDefinitionEnable();
    ClearFixedColDefinitionEnable();
    UpdateFixedColDefinitionEnable();
  }
  catch (ZdException &x) {
    x.AddCallpath("ClearFixedColumnDefinitions()", "TBDlgDataImporter");
    throw;
  }
}

/** Enables 'Clear' button for fixed column file defs. */
void TBDlgDataImporter::ClearFixedColDefinitionEnable() {
  btnClearFldDefs->Enabled = lstFixedColFieldDefs->Items->Count;
}

/** Enables continue button based upon current panels settings. */
void TBDlgDataImporter::ContinueButtonEnable() {
  btnNextPanel->Enabled = false;
  btnExecuteImport->Enabled = false;
  switch (*gitrCurrentPanel) {
    case Start       : btnNextPanel->Enabled = FileExists(edtDataFile->Text.c_str()) && rdgInputFileType->ItemIndex > -1;
                       break;
    case FileType    : btnNextPanel->Enabled = (!gbErrorSamplingSourceFile &&
                                                (rdoFileType->ItemIndex == 2 ||
                                                (rdoFileType->ItemIndex == 0 && cmbColDelimiter->GetTextLen()) ||
                                                (rdoFileType->ItemIndex == 1 &&  lstFixedColFieldDefs->Items->Count > 0)));
                       break;
    case DataMapping : for (size_t i=0; i < gvSaTScanVariables.size() && !btnExecuteImport->Enabled; ++i) {
                          if (tsfieldGrid->RowVisible[i+1])
                            btnExecuteImport->Enabled = gvSaTScanVariables[i].GetIsMappedToInputFileVariable();
                       }
                       if (btnExecuteImport->Enabled)
                         btnExecuteImport->Enabled = gpController->GetGridVisibleRowCount();
                       break;
    default : ZdGenerateException("Invalid panel index \"%d\".", "ContinueButtonEnable()", *gitrCurrentPanel);
  };
}

/** Sets target filename and creates ZdIniFile used in target file creation. */
void TBDlgDataImporter::CreateDestinationInformation() {
  char                  sBuffer[1024];
  int                   iOffSet=0;
  ZdString              sFieldSection;
  ZdFileName            sFileName;
  ZdIniSection          Section;

  try {
    //Reset location of target file to users temp directory.
    sFileName.SetFullPath(edtDataFile->Text.c_str());
    GetTempPath(sizeof(sBuffer), sBuffer);
    sFileName.SetLocation(sBuffer);
    //File extension to file type.
    switch (rdgInputFileType->ItemIndex) {
      case Case        : sFileName.SetExtension(".cas");
                         break;
      case Control     : sFileName.SetExtension(".ctl");
                         break;
      case Population  : sFileName.SetExtension(".pop");
                         break;
      case Coordinates : sFileName.SetExtension(".geo");
                         break;
      case SpecialGrid : sFileName.SetExtension(".grd");
                         break;
      default  : ZdGenerateException("Unknown file type : \"%d\"", "ConvertImportedDataFile()", rdgInputFileType->ItemIndex);
    };
    gDestDescriptor.SetDestinationFile(sFileName.GetFullPath());

    //Define ZdIniFile that will be used by import class to create and open target file.
    gDestinationFileDefinition.Clear();
    Section.SetName("[FileInfo]");
    Section.AddLine("FileName", sFileName.GetCompleteFileName());
    Section.AddLine("Title", sFileName.GetFileName());
    Section.AddLine("InputLayoutNumber", "0");
    Section.AddLine("PrimaryKeyFields", "0");
    Section.AddLine("UserFlagCount", "0");
    gDestinationFileDefinition.AddSection(Section.Clone());

    Section.ClearSection();
    for (size_t t=0; t < gvSaTScanVariables.size(); t++) {
       //Skip fields 'X' and 'Y', they are equivalant to 'Latitude' and 'Longitude'
       //in terms of creating a ZdIniFile since they will never exist together
       //and map to the same field index.
       if (rdgInputFileType->ItemIndex == Coordinates && (t == 3 || t == 4))
         continue;
       if (rdgInputFileType->ItemIndex == SpecialGrid && (t == 2 || t == 3))
         continue;

       sFieldSection.printf("[Field%d]", gvSaTScanVariables[t].GetTargetFieldIndex() + 1);
       Section.SetName(sFieldSection.GetCString());
       Section.AddLine("FieldName", gvSaTScanVariables[t].GetVariableName().GetCString());
       Section.AddLine("Type", "A");
       Section.AddLine("Length", "255");
       Section.AddLine("ByteOffset", IntToStr(iOffSet).c_str());
       gDestinationFileDefinition.AddSection(Section.Clone());
       Section.ClearSection();
       iOffSet += 255;
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("CreateDestinationInformation()", "TBDlgDataImporter");
    throw;
  }
}

/** Enables the deleting of fixed column field definitions. */
void TBDlgDataImporter::DeleteFixedColDefinitionEnable() {
  btnDeleteFldDef->Enabled = lstFixedColFieldDefs->ItemIndex != -1;
}

/** Disables buttons during import operation. */
void TBDlgDataImporter::DisableButtonsForImport( bool bEnable ) {
  btnPreviousPanel->Enabled = !bEnable;
  btnExecuteImport->Enabled = !bEnable;
  btnNextPanel->Enabled = !bEnable;
  btnCancel->Enabled = !bEnable;
}

/** Gets column delimter. */
const char TBDlgDataImporter::GetColumnDelimiter() const {
  char  cColDelimiter;

  try {
    switch (cmbColDelimiter->ItemIndex) {
      case 0  : cColDelimiter = COMMA;
                break;
      case 1  : cColDelimiter = ';';
               break;
      case 2  : cColDelimiter = ' ';
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

/** Returns name to call uwFieldIndex'th column of fixed column file type. */
ZdString & TBDlgDataImporter::GetFixedColumnFieldName(unsigned int uwFieldIndex, ZdString & sFieldName) {
  try {
    sFieldName.printf("Field %d", uwFieldIndex);
  }
  catch (ZdException &x) {
    x.AddCallpath("GetFixedColumnFieldName()","TBDlgDataImporter");
    throw;
  }
  return sFieldName;
}

/** Gets group marker. */
const char  TBDlgDataImporter::GetGroupMarker() const {
  char  cGroupMarker;

  try {
    if (cmbGroupMarker->Enabled) {
      switch (cmbGroupMarker->ItemIndex) {
        case 0   : cGroupMarker =  DOUBLEQUOTE;
                   break;
        case 1   : cGroupMarker = SINGLEQUOTE;
                   break;
        default  : ZdString sString( cmbGroupMarker->Text.c_str() );
                   cGroupMarker = (sString.GetLength() ? sString[0] : DOUBLEQUOTE);
      }
    }
    else
     cGroupMarker = '\0'/*no group marker*/;
  }
  catch (ZdException &x) {
    x.AddCallpath("GetGroupMarker()","TBDlgDataImporter");
    throw;
  }
  return cGroupMarker;
}

/** Returns input file variable name at given index. */
const char * TBDlgDataImporter::GetInputFileVariableName(int iFieldIndex) const {
  const char *  sVariableName;

  try {
    if (iFieldIndex < 0 || iFieldIndex > GetNumInputFileVariables() - 1)
      ZdGenerateException("Index \"%d\" out of range(0 - %d)","", iFieldIndex, GetNumInputFileVariables() - 1);

    if (iFieldIndex == 0)
      sVariableName = gsUnassigned.GetCString();
    else
      sVariableName = gpController->GetTopGrid()->Col[iFieldIndex]->Heading.c_str();
  }
  catch (ZdException &x) {
    x.AddCallpath("GetInputFileVariableName()","TBDlgDataImporter");
    throw;
  }
  return sVariableName;
}

/** Returns number of input file variables plus one for 'unassigned' field. */
int TBDlgDataImporter::GetNumInputFileVariables() const {
  int         iNumInputVariables=0;
  
  try {
    if (gpController)
      iNumInputVariables = gpController->GetGridColCount() + 1/*unassigned field*/;
  }     
  catch (ZdException &x) {
    x.AddCallpath("GetNumInputFileVariables()","TBDlgDataImporter");
    throw;
  }
  return iNumInputVariables;
}

/** Hides rows based on coordinates variables selected to be shown. */
void TBDlgDataImporter::HideRows() {
  try {
    switch (rdgInputFileType->ItemIndex) {
      case Case        :
      case Control     :
      case Population  : for (int i=1; i <= tsfieldGrid->Rows; i++)
                            tsfieldGrid->RowVisible[i] = true;
                         break;
      case Coordinates : for (int i=1; i <= tsfieldGrid->Rows; i++) {
                            if (i == 2 || i == 3) //Latitude and Longitude
                              tsfieldGrid->RowVisible[i] = (rdoCoordinates->ItemIndex == 1 ? true : false);
                            else if (i >= 4) //Cartesian variables
                              tsfieldGrid->RowVisible[i] = (rdoCoordinates->ItemIndex == 1 ? false : true);
                            else
                              tsfieldGrid->RowVisible[i] = true;
                         }
                         break;
      case SpecialGrid : for (int i=1; i <= tsfieldGrid->Rows; i++) {
                            if (i == 1 || i == 2) //Latitude and Longitude
                              tsfieldGrid->RowVisible[i] = (rdoCoordinates->ItemIndex == 1 ? true : false);
                            else //Cartesian variables
                              tsfieldGrid->RowVisible[i] = (rdoCoordinates->ItemIndex == 1 ? false : true);
                         }
                         break;
      default          : break;
    };
  }
  catch (ZdException &x) {
    x.AddCallpath("HideRows()","TBDlgDataImporter");
    throw;
  }
}

/** Executes import. */
TModalResult TBDlgDataImporter::ImportFile() {
  TModalResult                  Modal=mrAbort;
  ZdString                      sMessage;

  if (chkFirstRowIsName->Checked)
    gSourceDescriptor.SetNumberOfRowsToIgnore(1);
  else
    gSourceDescriptor.SetNumberOfRowsToIgnore(0);

  BFileImportSourceInterface    FileImportSourceInterface(gpSourceFile, gSourceDescriptor.GetNumberOfRowsToIgnore());
  SaTScanFileImporter           FileImporter(gDestinationFileDefinition, (InputFileType)rdgInputFileType->ItemIndex,
                                             gSourceDataFileType, FileImportSourceInterface, gDestDescriptor);

  try {
    FileImporter.OpenDestination();
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

/** Initialize class. */
void TBDlgDataImporter::Init() {
  gsUnassigned = "unassigned";
  gpController = 0;
  gpDataModel = 0;
  gpSourceFile = 0;
  cmbColDelimiter->ItemIndex = 0;
  cmbGroupMarker->ItemIndex = 0;
  gSourceDataFileType=Delimited;
  gbErrorSamplingSourceFile = false;
}

/** Preps for viewing field mapping panel. */
void TBDlgDataImporter::LoadMappingPanel() {
  try {
    OpenSource();
    CreateDestinationInformation();
    tsfieldGrid->Rows = gvSaTScanVariables.size();
    ClearSaTScanVariableFieldIndexes();
    AutoAlign();
    if (rdoCoordinates->ItemIndex == -1)
      //If coordinates index not set, set to current settings of analysis. 
      rdoCoordinates->ItemIndex = gAnalysisForm.rgCoordinates->ItemIndex;
    else
      HideRows();
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
    switch (rdgInputFileType->ItemIndex) {
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
      default : ZdGenerateException("Unknown file type index: \"%d\"", "LoadResultFileNameIntoAnalysis()", rdgInputFileType->ItemIndex);
    };
  }
  catch (ZdException &x) {
    x.AddCallpath("LoadResultFileNameIntoAnalysis()", "TBDlgDataImporter");
    throw;
  }
}

/** Setup for panel to show. */
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

/** Adds fixed column definition( ZdIniSection ). */
void TBDlgDataImporter::OnAddFieldDefinitionClick() {
  ZdIniSection  addSection;
  AnsiString    sFieldSection;
  ZdString      sListBox, sFieldName;
  char        * sBuffer = "A";
  int           iStart;

  try {
    if (StrToInt(edtFieldLength->Text) > ZD_MAXFIELD_LEN) {
      edtFieldLength->SetFocus();
      ZdException::GenerateNotification("Field length can be no larger than %d.", "OnAddFieldDefinitionClick()", ZD_MAXFIELD_LEN);
    }

    sFieldSection.sprintf("[Field%d]", ( int )gvIniSections.size() + 1);
    addSection.SetName(sFieldSection.c_str());
    addSection.AddLine("FieldName", edtFieldName->Text.c_str());
    addSection.AddLine("Type", sBuffer);                       
    addSection.AddLine("Length", edtFieldLength->Text.c_str());
    //Add one to Byte off set, memo starts at zero.
    iStart = StrToInt(edtStartColumn->Text);
    addSection.AddLine("ByteOffset", IntToStr(iStart).c_str());
    gvIniSections.push_back(addSection);

    //Add to listbox
    sListBox.printf(" %-3d     %-3d   %-15s",
                    StrToInt(edtStartColumn->Text.c_str()),
                    StrToInt(edtFieldLength->Text.c_str()),
                    edtFieldName->Text.c_str());
    lstFixedColFieldDefs->Items->Add(sListBox.GetCString());
    edtFieldName->Text = GetFixedColumnFieldName(gvIniSections.size() + 1, sFieldName).GetCString();
    lstFixedColFieldDefs->ItemIndex = -1;
    ClearFixedColDefinitionEnable();
    ContinueButtonEnable();
    edtFieldName->SetFocus();
    edtFieldName->SelStart = 0;
  }
  catch (ZdException &x) {
    x.AddCallpath("OnAddFieldDefinitionClick()","TBDlgDataImporter");
    throw;
  }
}

/** Response to auto-align button click. */
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

/** Deletes fixed column field definition. */
void TBDlgDataImporter::OnDeleteFieldDefinitionClick() {
  try {
    if (lstFixedColFieldDefs->ItemIndex > -1 && lstFixedColFieldDefs->ItemIndex <= (int)gvIniSections.size()) {
      gvIniSections.RemoveElement(lstFixedColFieldDefs->ItemIndex);
      lstFixedColFieldDefs->Items->Delete(lstFixedColFieldDefs->ItemIndex);
    }
    DeleteFixedColDefinitionEnable();
    ClearFixedColDefinitionEnable();
    UpdateFixedColDefinitionEnable();
    ContinueButtonEnable();
  }
  catch (ZdException &x) {
    x.AddCallpath("OnDeleteFieldDefinitionClick()","TBDlgDataImporter");
    throw;
  }
}

/** Response to import click. */
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
                       edtIgnoreFirstRows->Text = "0";
                       cmbColDelimiter->ItemIndex = 0;
                       cmbGroupMarker->ItemIndex = 0;
                       rdoFileType->ItemIndex = 0;
                       rdgInputFileType->ItemIndex = 0;
                       gSourceDescriptor.SetNumberOfRowsToIgnore(0);
                       ClearFixedColumnDefinitions();
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

/** Sets file descriptors based on file type. */
void TBDlgDataImporter::OnExitStartPanel() {
  try {
    SetPanelsToShow();
    gitrCurrentPanel = gvPanels.begin();
    switch (rdgInputFileType->ItemIndex) {
      case Case        : SetupCaseFileVariableDescriptors();
                         rdoCoordinates->Enabled = false;
                         pnlBottomPanelTopAligned->Visible = false;
                         break;
      case Control     : SetupControlFileVariableDescriptors();
                         rdoCoordinates->Enabled = false;
                         pnlBottomPanelTopAligned->Visible = false;
                         break;
      case Population  : SetupPopFileVariableDescriptors();
                         rdoCoordinates->Enabled = false;
                         pnlBottomPanelTopAligned->Visible = false;
                         break;
      case Coordinates : SetupGeoFileVariableDescriptors();
                         rdoCoordinates->Enabled = true;
                         pnlBottomPanelTopAligned->Visible = true;
                         break;
      case SpecialGrid : SetupGridFileVariableDescriptors();
                         rdoCoordinates->Enabled = true;
                         pnlBottomPanelTopAligned->Visible = true;
                         break;
      default : ZdGenerateException("Unknown file type index: \"%d\"","OnExitStartPanel()", rdgInputFileType->ItemIndex);
    };
  }
  catch (ZdException &x) {
    x.AddCallpath("OnExitStartPanel()","TBDlgDataImporter");
    throw;
  }
}

/** Event response to changing import file type. */
void TBDlgDataImporter::OnFieldDefinitionChange() {
  int   iStartColumn, iSelStart=0, iSelLength=0;

  if (rdoFileType->ItemIndex == Fixed_Column) {
    AddFixedColDefinitionEnable();
    UpdateFixedColDefinitionEnable();
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

/** Resets grid headers in reponse to user action. */
void TBDlgDataImporter::OnFirstRowIsHeadersClick() {
  try {
    SetGridHeaders(chkFirstRowIsName->Checked);
    tsfieldGrid->Invalidate();
    ContinueButtonEnable();
  }
  catch (ZdException &x) {
    x.AddCallpath("OnFirstRowIsHeadersClick()","TBDlgDataImporter");
    throw;
  }
}

/** Preparation for viewing file format panel. */
void TBDlgDataImporter::OnViewFileFormatPanel() {
  ZdString      sFieldName;

  try {
    ReadDataFileIntoRawDisplayField();
    ShowFileTypeFormatPanel(rdoFileType->ItemIndex);
    edtFieldName->Text = GetFixedColumnFieldName(1, sFieldName).GetCString();
    ContinueButtonEnable();
    OnFieldDefinitionChange();
  }
  catch (ZdException &x) {
    x.AddCallpath("OnViewFileFormatPanel()","TBDlgDataImporter");
    throw;
  }
}

/** Preparation for viewing mapping panel. */
void TBDlgDataImporter::OnViewMappingPanel() {
  try {
    Screen->Cursor = crHourGlass;
    UpdateFileFormatOptions();
    LoadMappingPanel();
    Screen->Cursor = crDefault;
  }
  catch (ZdException &x) {
    x.AddCallpath("OnViewMappingPanel()","TBDlgDataImporter");
    Screen->Cursor = crDefault;
    throw;
  }
}

/** Opens source file as character delimited source file. */
void TBDlgDataImporter::OpenSourceAsCSVFile() {
  try {
    gpSourceFile = new CSVFile(GetColumnDelimiter(), GetGroupMarker());
    ((CSVFile*)gpSourceFile)->SetInitialNondataLineCount(gSourceDescriptor.GetNumberOfRowsToIgnore());
    ((CSVFile*)gpSourceFile)->OpenAllFieldsAlpha(edtDataFile->Text.c_str(), ZDIO_OPEN_READ);
  }
  catch (CSVFile::InitialNondataLineCountInvalidForData_Exception &x) {
    ZdString sMessage;
    x.AddCallpath("OpenSourceAsCSVFile()","TBDlgDataImporter");
    x.SetLevel(ZdException::Notify);
    sMessage << "You are attempting to ignore the first " << x.GetSpecifiedCount();
    sMessage << " lines of a file that contains " << x.GetProvidedByDataCount();
    sMessage << " lines.\nPlease specify a lesser number of lines to ignore.";
    x.SetErrorMessage(sMessage);
    edtIgnoreFirstRows->SetFocus();
    throw;
  }
  catch (ZdException &x) {
    x.AddCallpath("OpenSourceAsCSVFile()","TBDlgDataImporter");
    throw;
  }
}

/** Opens source file as dBase file. */
void TBDlgDataImporter::OpenSourceAsDBaseFile() {
  try {
    gpSourceFile = ZdOpenFile(edtDataFile->Text.c_str(), ZDIO_OPEN_READ|ZDIO_SREAD);
  }
  catch (ZdException &x) {
    x.AddCallpath("OpenSourceAsDBaseFile()","TBDlgDataImporter");
    throw;
  }
}

/** Opens source file as scanf formatted file. */
void TBDlgDataImporter::OpenSourceAsScanfFile() {
  try {
    gpSourceFile = new ScanfFile();
    ((ScanfFile*)gpSourceFile)->SetInitialNondataLineCount(gSourceDescriptor.GetNumberOfRowsToIgnore());
    ((ScanfFile*)gpSourceFile)->OpenAllFieldsAlpha(edtDataFile->Text.c_str(), ZDIO_OPEN_READ);
  }
  catch (ScanfFile::InitialNondataLineCountInvalidForData_Exception &x) {
    ZdString sMessage;
    x.AddCallpath("OpenSourceAsScanfFile()","TBDlgDataImporter");
    x.SetLevel(ZdException::Notify);
    sMessage << "You are attempting to ignore the first " << x.GetSpecifiedCount();
    sMessage << " lines of a file that contains " << x.GetProvidedByDataCount();
    sMessage << " lines.\nPlease specify a lesser number of lines to ignore.";
    x.SetErrorMessage(sMessage);
    edtIgnoreFirstRows->SetFocus();
    throw;
  }
  catch (ZdException &x) {
    x.AddCallpath("OpenSourceAsScanfFile()","TBDlgDataImporter");
    throw;
  }
}

/** Opens source file as file column variable record length file. */
void TBDlgDataImporter::OpenSourceAsTXVFile() {
  size_t                t;
  int                   iRecordLength, iMaxLength=0;
  ZdFileName            fFileName;
  ZdString              sFieldSection, sFieldName;
  ZdIniSection          tempSection;
  ZdIniFile             DataFileDefinition;

  try {
    //Define TXV file structure.
    tempSection.SetName("[FileInfo]");
    fFileName.SetFullPath(edtDataFile->Text.c_str());
    tempSection.AddLine("FileName", fFileName.GetCompleteFileName());
    tempSection.AddLine("Title", fFileName.GetFileName());
    tempSection.AddLine("NumberOfFields", IntToStr((int)gvIniSections.size()).c_str());
    for (t=0; t < gvIniSections.size(); ++t) {
      iRecordLength = StrToInt(gvIniSections[t].GetString("Length"));
      iRecordLength += StrToInt(gvIniSections[t].GetString("ByteOffSet"));
      iMaxLength = max(iRecordLength, iMaxLength);
    }
    tempSection.AddLine("RecordLength", IntToStr(iMaxLength).c_str());
    tempSection.AddLine("StartingCol", "1");
    DataFileDefinition.AddSection(tempSection.Clone());
    for (t=0; t < gvIniSections.size(); ++t) {
       sFieldName.printf("[Field%d]", t + 1);
       gvIniSections[t].SetName(sFieldName.GetCString());
       DataFileDefinition.AddSection(gvIniSections[t].Clone());
    }
    gpSourceFile = new TXVFile();
    ((TXVFile*)gpSourceFile)->SetInitialNondataLineCount(gSourceDescriptor.GetNumberOfRowsToIgnore());
    gpSourceFile->Open(edtDataFile->Text.c_str(), ZDIO_OPEN_READ|ZDIO_SREAD, 0, 0, &DataFileDefinition);
  }
  catch (TXVFile::InitialNondataLineCountInvalidForData_Exception &x) {
    ZdString sMessage;
    x.AddCallpath("OpenSourceAsScanfFile()","TBDlgDataImporter");
    x.SetLevel(ZdException::Notify);
    sMessage << "You are attempting to ignore the first " << x.GetSpecifiedCount();
    sMessage << " lines of a file that contains " << x.GetProvidedByDataCount();
    sMessage << " lines.\nPlease specify a lesser number of lines to ignore.";
    x.SetErrorMessage(sMessage);
    edtIgnoreFirstRows->SetFocus();
    throw;
  }
  catch (ZdException &x) {
    x.AddCallpath("OpenSourceAsTXVFile()","TBDlgDataImporter");
    throw;
  }
}

/** Opening source as specified by file type. */
void TBDlgDataImporter::OpenSource() {
  try {
    delete gpController; gpController = 0;
    delete gpDataModel; gpDataModel = 0;
    delete gpSourceFile; gpSourceFile = 0;

    //Open import file source as specified type.
    if (rdoFileType->ItemIndex == 0) {
      if (cmbColDelimiter->ItemIndex == 2/*white space*/)
        OpenSourceAsScanfFile();
      else
        OpenSourceAsCSVFile();
      chkFirstRowIsName->Enabled = true;
    }
    else if (rdoFileType->ItemIndex == 1) {
      OpenSourceAsTXVFile();
      chkFirstRowIsName->Checked = false;
      chkFirstRowIsName->Enabled = false;
    }
    else {
      OpenSourceAsDBaseFile();
      chkFirstRowIsName->Checked = false;
      chkFirstRowIsName->Enabled = false;
    }

    try {
      AdjustSourceFileAttributes(*gpSourceFile);
      gpDataModel = new BGridZdSingleFileModel(gpSourceFile);
      gpController = new SourceViewController(tsImportFileGrid, gpDataModel);
      gpController->SetGridMode(Tsgrid::gmBrowse);
     }
     catch (ZdException &x) {
       ImporterException::GenerateException("The import wizard was unable to read source file.\nPlease review settings.",
                                            "OpenSource()", ZdException::Notify);
     }

     gpController->EmptyContextMenus();
     SetGridHeaders(chkFirstRowIsName->Checked && chkFirstRowIsName->Enabled);
  }
  catch (ZdException &x) {
    x.AddCallpath("OpenSource()","TBDlgDataImporter");
    delete gpSourceFile; gpSourceFile=0;
    delete gpDataModel; gpDataModel=0;
    delete gpController; gpController=0;
    throw;
  }
}

/** Reads in a sample of data file into a memo field to help user
    to determine structure of source file. */
void TBDlgDataImporter::ReadDataFileIntoRawDisplayField() {
  int         i;
  ZdIO        fImportDataFile;
  ZdString    sFileLineBuffer;

  try {
    memRawData->Clear();
    memRawData->HideSelection = false;
    fImportDataFile.Open(edtDataFile->Text.c_str(), ZDIO_OPEN_READ|ZDIO_SREAD);

    for (i=0; i < 200 && !fImportDataFile.GetIsEOF(); ++i) {
       fImportDataFile.ReadLine(sFileLineBuffer);
       memRawData->Lines->Add(sFileLineBuffer.GetCString());
    }
    fImportDataFile.Close();

    if (i==0) {
      memRawData->Lines->Add( "* Source file contains no data. *" );
      gbErrorSamplingSourceFile = true;
    }
    else
    gbErrorSamplingSourceFile = false;
  }
  catch (ZdException &x) {
    memRawData->Clear();
    memRawData->Lines->Add( "* Unable to view source file. *" );
    memRawData->HideSelection = true;
    gbErrorSamplingSourceFile = true;
  }
}

/** Updates which format options panels are shown. */
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
    switch (rdgInputFileType->ItemIndex) {
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
      default : ZdGenerateException("Unknown file type index: \"%d\"","SetImportFields()", rdgInputFileType->ItemIndex);
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

/** Re-assigns grid headers to that of first row of model if not blank. */
void TBDlgDataImporter::SetGridHeaders(bool bFirstRowIsHeader) {
  ZdString    sHeaderValue;
  char        sBuffer[1024];

  try {
     if (gpController && gpController->GetGridRowCount()) {
       gpController->HideRow(1, bFirstRowIsHeader);
       for (int i=1; i <= gpController->GetGridColCount(); ++i) {
          sHeaderValue << ZdString::reset;
          if (rdoFileType->ItemIndex == 0 && bFirstRowIsHeader) //Get header from model row
            sHeaderValue = (char *)gpController->GetValueAt(1, i, (char *)sBuffer, sizeof(sBuffer), true);

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

/** Tell import class what's imported from where. */
void TBDlgDataImporter::SetMappings(BZdFileImporter & FileImporter) {
  size_t                                i;
  ZdFile                              * pFile;
  const ZdField                       * pToField;

  try {
    pFile = FileImporter.GetRemoteFile();
    for (i=0; i < gvSaTScanVariables.size(); i++)
       if (tsfieldGrid->RowVisible[i+1] && gvSaTScanVariables[i].GetIsMappedToInputFileVariable()) {
         pToField =  pFile->GetFieldInfo(gvSaTScanVariables[i].GetTargetFieldIndex());
         FileImporter.AddMapping(gvSaTScanVariables[i].GetInputFileVariableIndex(), pToField);
       }
  }
  catch (ZdException &x) {
    x.AddCallpath("SetMappings()","TBDlgDataImporter");
    throw;
  }
}

/** Sets which panels and the order of showing panels. */
void TBDlgDataImporter::SetPanelsToShow() {
  try {
    //Skip file format panel for dBase since we already know structure.
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
  unsigned long ulCurrentStyle;

  try {
    gDestDescriptor.SetDestinationType(BFileDestDescriptor::SingleFile);
    gDestDescriptor.SetModifyType(BFileDestDescriptor::OverWriteExistingData);
    gDestDescriptor.SetGenerateReport(false);
    gDestDescriptor.SetErrorOptionsType(BFileDestDescriptor::RejectBatch);
    rdgInputFileType->ItemIndex = 0;
    ulCurrentStyle = GetWindowLong(lstFixedColFieldDefs->Handle, GWL_STYLE);
    SetWindowLong(lstFixedColFieldDefs->Handle, GWL_STYLE, ulCurrentStyle|WS_HSCROLL);
    //lblFldLen->Caption.sprintf("Length (max %d)", ZD_MAXFIELD_LEN);
  }
  catch (ZdException &x) {
    x.AddCallpath("SetUp()","TBDlgDataImporter");
    throw;
  }
}

/** Setup field descriptors for case file. */
void TBDlgDataImporter::SetupCaseFileVariableDescriptors() {
  try {
    gvSaTScanVariables.clear();
    gvSaTScanVariables.push_back(SaTScanVariable("Tract ID", 0, true));
    gvSaTScanVariables.push_back(SaTScanVariable("Number of Cases", 1, true));
    gvSaTScanVariables.push_back(SaTScanVariable("Date/Time", 2, false));
    gvSaTScanVariables.push_back(SaTScanVariable("Covariate1", 3, false));
    gvSaTScanVariables.push_back(SaTScanVariable("Covariate2", 4, false ));
    gvSaTScanVariables.push_back(SaTScanVariable("Covariate3", 5, false));
    gvSaTScanVariables.push_back(SaTScanVariable("Covariate4", 6, false));
    gvSaTScanVariables.push_back(SaTScanVariable("Covariate5", 7, false));
    gvSaTScanVariables.push_back(SaTScanVariable("Covariate6", 8, false));
    gvSaTScanVariables.push_back(SaTScanVariable("Covariate7", 9, false));
    gvSaTScanVariables.push_back(SaTScanVariable("Covariate8", 10, false));
    gvSaTScanVariables.push_back(SaTScanVariable("Covariate9", 11, false));
    gvSaTScanVariables.push_back(SaTScanVariable("Covariate10", 12, false));
  }
  catch (ZdException &x) {
    x.AddCallpath("SetupCaseFileVariableDescriptors()", "TBDlgDataImporter");
    throw;
  }
}

/** Setup field descriptors for control file. */
void TBDlgDataImporter::SetupControlFileVariableDescriptors() {
  try {
    gvSaTScanVariables.clear();
    gvSaTScanVariables.push_back(SaTScanVariable("Tract ID", 0, true));
    gvSaTScanVariables.push_back(SaTScanVariable("Number of Controls", 1, true));
    gvSaTScanVariables.push_back(SaTScanVariable("Date/Time", 2, false));
  }
  catch (ZdException &x) {
    x.AddCallpath("SetupControlFileVariableDescriptors()", "TBDlgDataImporter");
    throw;
  }
}

/** Setup field descriptors for coordinates file. */
void TBDlgDataImporter::SetupGeoFileVariableDescriptors() {
  try {
    gvSaTScanVariables.clear();
    gvSaTScanVariables.push_back(SaTScanVariable("Tract ID", 0, true));
    gvSaTScanVariables.push_back(SaTScanVariable("Latitude", 1, true));
    gvSaTScanVariables.push_back(SaTScanVariable("Longitude", 2, true));
    gvSaTScanVariables.push_back(SaTScanVariable("X", 1, true));
    gvSaTScanVariables.push_back(SaTScanVariable("Y", 2, true));
    gvSaTScanVariables.push_back(SaTScanVariable("Z1", 3, false ));
    gvSaTScanVariables.push_back(SaTScanVariable("Z2", 4, false));
    gvSaTScanVariables.push_back(SaTScanVariable("Z3", 5, false ));
    gvSaTScanVariables.push_back(SaTScanVariable("Z4", 6, false));
    gvSaTScanVariables.push_back(SaTScanVariable("Z5", 7, false));
    gvSaTScanVariables.push_back(SaTScanVariable("Z6", 8, false));
    gvSaTScanVariables.push_back(SaTScanVariable("Z7", 9, false));
    gvSaTScanVariables.push_back(SaTScanVariable("Z8", 10, false));
  }
  catch (ZdException &x) {
    x.AddCallpath("SetupGeoFileVariableDescriptors()", "TBDlgDataImporter");
    throw;
  }
}

/** Setup field descriptors for special grid file. */
void TBDlgDataImporter::SetupGridFileVariableDescriptors() {
  try {
    gvSaTScanVariables.clear();
    gvSaTScanVariables.push_back(SaTScanVariable("Latitude", 0, true));
    gvSaTScanVariables.push_back(SaTScanVariable("Longitude", 1, true));
    gvSaTScanVariables.push_back(SaTScanVariable("X", 0, true));
    gvSaTScanVariables.push_back(SaTScanVariable("Y", 1, true));
    gvSaTScanVariables.push_back(SaTScanVariable("Z1", 2, false));
    gvSaTScanVariables.push_back(SaTScanVariable("Z2", 3, false));
  }
  catch (ZdException &x) {
    x.AddCallpath("SetupGridFileVariableDescriptors()", "TBDlgDataImporter");
    throw;
  }
}

/** Setup field descriptors for population file. */
void TBDlgDataImporter::SetupPopFileVariableDescriptors() {
  try {
    gvSaTScanVariables.clear();
    gvSaTScanVariables.push_back(SaTScanVariable("Tract ID", 0, true));
    gvSaTScanVariables.push_back(SaTScanVariable("Date/Time", 1, true));
    gvSaTScanVariables.push_back(SaTScanVariable("Population", 2, true));
    gvSaTScanVariables.push_back(SaTScanVariable("Covariate1", 3, false));
    gvSaTScanVariables.push_back(SaTScanVariable("Covariate2", 4, false));
    gvSaTScanVariables.push_back(SaTScanVariable("Covariate3", 5, false));
    gvSaTScanVariables.push_back(SaTScanVariable("Covariate4", 6, false));
    gvSaTScanVariables.push_back(SaTScanVariable("Covariate5", 7, false));
    gvSaTScanVariables.push_back(SaTScanVariable("Covariate6", 8, false));
    gvSaTScanVariables.push_back(SaTScanVariable("Covariate7", 9, false));
    gvSaTScanVariables.push_back(SaTScanVariable("Covariate8", 10, false));
    gvSaTScanVariables.push_back(SaTScanVariable("Covariate9", 11, false));
    gvSaTScanVariables.push_back(SaTScanVariable("Covariate10", 12, false));
  }
  catch (ZdException &x) {
    x.AddCallpath("SetupPopFileVariableDescriptors()", "TBDlgDataImporter");
    throw;
  }
}

/** Causes the first panel of importer to be shown. */
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

/** Causes next panel of importer to be shown. */
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

/** Causes previous panel of importer to be shown. */
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

/** Updates selected fixed column field definition. */
void TBDlgDataImporter::UpdateFieldDefinition() {
  ZdString      sListBox;
  int           i, iStart, iSelectedIndex=-1;

  try {
    for (i=0; i < lstFixedColFieldDefs->Items->Count && iSelectedIndex == -1; i++)
       if (lstFixedColFieldDefs->Selected[i])
         iSelectedIndex = i;

    if (StrToInt(edtFieldLength->Text) > ZD_MAXFIELD_LEN) {
      edtFieldLength->SetFocus();
      ZdException::GenerateNotification("Field length can be no larger than %d.", "UpdateFieldDefinitionx()", ZD_MAXFIELD_LEN);
    }

    gvIniSections[iSelectedIndex].SetString("FieldName", edtFieldName->Text.c_str());
    gvIniSections[iSelectedIndex].SetString("Length", edtFieldLength->Text.c_str());
    //Add one to Byte off set, memo starts at zero.
    iStart = StrToInt(edtStartColumn->Text);
    gvIniSections[iSelectedIndex].SetString("ByteOffset", IntToStr(iStart).c_str());

    //Update to listbox
    sListBox.printf(" %-3d     %-3d   %-15s",
                    StrToInt(edtStartColumn->Text.c_str()),
                    StrToInt(edtFieldLength->Text.c_str()),
                    edtFieldName->Text.c_str());
    lstFixedColFieldDefs->Items->Strings[iSelectedIndex] = sListBox.GetCString();
    lstFixedColFieldDefs->ItemIndex = -1;
    ClearFixedColDefinitionEnable();
    DeleteFixedColDefinitionEnable();
    UpdateFixedColDefinitionEnable();
    ContinueButtonEnable();
    edtFieldName->SetFocus();
    edtFieldName->SelStart = 0;
  }
  catch (ZdException &x) {
    x.AddCallpath("UpdateFieldDefinition()","TBDlgDataImporter");
    throw;
  }
}

/** Set options from file format panel. */
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

/** Enables update button for fixed column. */
void TBDlgDataImporter::UpdateFixedColDefinitionEnable() {
  int   i, iSelectedIndex=-1;

  for (i=0; i < lstFixedColFieldDefs->Items->Count && iSelectedIndex == -1; i++)
     if (lstFixedColFieldDefs->Selected[i])
        iSelectedIndex = i;

  btnUpdateFldDef->Enabled = (iSelectedIndex != -1 && edtFieldName->GetTextLen() > 0 &&
                              edtFieldLength->GetTextLen() > 0 && StrToInt(edtFieldLength->Text) > 0
                              && edtStartColumn->GetTextLen() > 0 && StrToInt(edtStartColumn->Text) > 0);
}

void TBDlgDataImporter::ValidateImportSource() {
  try {
    for (unsigned long u=1; u <= gpSourceFile->GetNumRecords(); u++)
       gpSourceFile->GotoRecord(u);
  }
  catch (ZdException &x) {
    x.AddCallpath("ValidateImportSource()","TBDlgDataImporter");
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
    ClearSaTScanVariableFieldIndexes();
    ContinueButtonEnable();
  }
  catch (ZdException &x) {
    x.AddCallpath("OnClearImportsClick()","TBDlgDataImporter");
    DisplayBasisException(this, x);
  }
}

void __fastcall TBDlgDataImporter::OnClearFldDefsClick(TObject *Sender) {
  try {
    ClearFixedColumnDefinitions();
    ContinueButtonEnable();
  }
  catch (ZdException &x) {
    x.AddCallpath("OnClearFldDefsClick()","TBDlgDataImporter");
    DisplayBasisException(this, x);
  }
}

void __fastcall TBDlgDataImporter::OnCoordinatesClick(TObject *Sender) {
  try {
    HideRows();
    ContinueButtonEnable();
  }
  catch (ZdException &x) {
    x.AddCallpath("OnCoordinatesClick()","TBDlgDataImporter");
    DisplayBasisException(this, x);
  }
}

void __fastcall TBDlgDataImporter::OnCSVComboChange(TObject *Sender) {
  try {
    cmbGroupMarker->Enabled = (cmbColDelimiter->ItemIndex != 2/*white space*/);
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
      case  1  : gvSaTScanVariables[DataRow - 1].GetVariableDisplayName(Value);
                 break;
      case  2  : Value = GetInputFileVariableName(gvSaTScanVariables[DataRow - 1].GetInputFileVariableIndex());
                 break;
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("OnFieldGridCellLoaded()","TBDlgDataImporter");
    DisplayBasisException(this,x);
  }
}

void __fastcall TBDlgDataImporter::OnFieldGridComboCellLoaded(TObject *Sender, TtsComboGrid *Combo, int DataCol, int DataRow, Variant &Value) {
  try {
    Combo->Grid->Cell[DataCol][DataRow] = GetInputFileVariableName(DataRow - 1);
  }
  catch (ZdException &x) {
    x.AddCallpath("OnFieldGridComboCellLoaded()","TBDlgDataImporter");
    DisplayBasisException(this, x);
  }
}

void __fastcall TBDlgDataImporter::OnFieldGridComboDropDown(TObject *Sender, TtsComboGrid *Combo, int DataCol, int DataRow) {
  try {
    Combo->Grid->Rows = GetNumInputFileVariables();
  }
  catch (ZdException &x) {
    x.AddCallpath("OnFieldGridComboDropDown()","TBDlgDataImporter");
    DisplayBasisException(this,x);
  }
}

void __fastcall TBDlgDataImporter::OnFieldGridComboGetValue(TObject *Sender,TtsComboGrid *Combo, int GridDataCol, int GridDataRow, int ComboDataRow, Variant &Value) {
  try {
    gvSaTScanVariables[GridDataRow - 1].SetInputFileVariableIndex(ComboDataRow - 1);
    tsfieldGrid->Invalidate();
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
    ClearFixedColDefinitionEnable();
    UpdateFixedColDefinitionEnable();
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

void __fastcall TBDlgDataImporter::OnUpdateFldDefClick(TObject *Sender) {
  try {
    UpdateFieldDefinition();
  }
  catch (ZdException &x) {
    x.AddCallpath("OnUpdateFldDefClick()","TBDlgDataImporter");
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

