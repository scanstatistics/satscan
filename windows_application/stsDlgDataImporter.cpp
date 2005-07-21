//---------------------------------------------------------------------------
#include "stsSaTScan.h"
#pragma hdrstop
//---------------------------------------------------------------------------

#pragma package(smart_init)
#pragma link "Grids_ts"
#pragma link "TSGrid"
#pragma resource "*.dfm"

#include <iostream>
#include <fstream>

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
SaTScanVariable::SaTScanVariable(const char * sVariableName, short wTargetFieldIndex, bool bRequiredVariable, const char * sHelpText) {
  try {
    Init();
    SetVariableName(sVariableName);
    SetTargetFieldIndex(wTargetFieldIndex);
    SetIsRequiredField(bRequiredVariable);
    if (!bRequiredVariable)                 /** Currently, a non-required variable trumps any other help text.**/
      SetHelpText("optional");              
    else if (sHelpText)
      SetHelpText(sHelpText);
    //else nothing  
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
    SetHelpText(rhs.gsHelpText);
  }
  catch (ZdException &x) {
    x.AddCallpath("Copy()", "SaTScanVariable");
    throw;
  }
}

/** Returns variable display name. */
void SaTScanVariable::GetVariableDisplayName(Variant & Value) const {
  if (gsHelpText.GetLength()) {
    ZdString      sTemp;

    sTemp << gsVariableName << " (" << gsHelpText << ")";
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
__fastcall TBDlgDataImporter::TBDlgDataImporter(TComponent* Owner, const char * sSourceFilename,
                                                InputFileType eFileType, CoordinatesType eCoordinatesType)
                             :TForm(Owner), geFileType(eFileType), geStartingCoordinatesType(eCoordinatesType) {
  try {
    Init();
    Setup(sSourceFilename);
  }
  catch (ZdException &x) {
    x.AddCallpath("constructor()","TBDlgDataImporter");
    throw;
  }
}

__fastcall TBDlgDataImporter::TBDlgDataImporter(TComponent* Owner, TfrmAnalysis & AnalysisForm) : TForm(Owner) {
  ZdGenerateException("this constructor is deprecated.", "TBDlgDataImporter()");
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
void TBDlgDataImporter::BringPanelToFront(TTabSheet* tabShowTab) {
  try {
    pgcImportPages->ActivePage = tabShowTab;
    btnPreviousPanel->Visible = (tabShowTab == tabDataMapping && gSourceDataFileType != dBase ||  (tabShowTab == tabOutputSettings));
    btnNextPanel->Visible = (pgcImportPages->ActivePage == tabFileFormat || pgcImportPages->ActivePage == tabDataMapping);
    btnExecuteImport->Visible = (pgcImportPages->ActivePage == tabOutputSettings);
    chkFirstRowIsName->Visible = (pgcImportPages->ActivePage == tabDataMapping);
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
      sMessage << "For the " << GetInputFileTypeString();
      sMessage << ", the following SaTScan Variable(s) are required\nand an Input File Variable must";
      sMessage << " be selected for each before import can proceed.\n\nSaTScan Variable(s): ";
      for (t=0; t < vMissingFieldIndex.size(); t++) {
         sMessage << gvSaTScanVariables[vMissingFieldIndex[t]].GetVariableName();
         if (t < vMissingFieldIndex.size() - 1)
           sMessage << ", ";
      }
      BImporterException::GenerateException(sMessage, "CheckForRequiredVariables()", ZdException::Notify);
    }

    //hack - we need to ensure that either both or neither dates are assigned for adjustment file
    //     -- redo this better after 4.0 release
    if (geFileType == AdjustmentsByRR) {
      if ((gvSaTScanVariables[2].GetIsMappedToInputFileVariable() && !gvSaTScanVariables[3].GetIsMappedToInputFileVariable()) ||
          (!gvSaTScanVariables[2].GetIsMappedToInputFileVariable() && gvSaTScanVariables[3].GetIsMappedToInputFileVariable())) {
        sMessage << "For the " << GetInputFileTypeString();
        sMessage << ", the dates are required to be selected or omitted as a pair.\n";
        BImporterException::GenerateException(sMessage, "CheckForRequiredVariables()", ZdException::Notify);
      }
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
  
  if (*gitrCurrentTabSheet == tabFileFormat)
    btnNextPanel->Enabled = (!gbErrorSamplingSourceFile &&
                              (rdoFileType->ItemIndex == 2 ||
                               (rdoFileType->ItemIndex == 0 && cmbColDelimiter->GetTextLen()) ||
                                (rdoFileType->ItemIndex == 1 &&  lstFixedColFieldDefs->Items->Count > 0)));
  else if (*gitrCurrentTabSheet == tabDataMapping) {
    for (size_t i=0; i < gvSaTScanVariables.size() && !btnNextPanel->Enabled; ++i) {
       if (tsfieldGrid->RowVisible[i+1])
          btnNextPanel->Enabled = gvSaTScanVariables[i].GetIsMappedToInputFileVariable();
    }
    if (btnNextPanel->Enabled)
      btnNextPanel->Enabled = gpController->GetGridVisibleRowCount();
  }
  else if (*gitrCurrentTabSheet == tabOutputSettings) {
    btnExecuteImport->Enabled = (!edtOutputDirectory->Text.IsEmpty() && DirectoryExists(edtOutputDirectory->Text));
  }
  else
    ZdGenerateException("Unknown tabsheet.", "ContinueButtonEnable()");
}

/** Sets target filename and creates ZdIniFile used in target file creation. */
void TBDlgDataImporter::CreateDestinationInformation() {
  int                   iOffSet=0;
  ZdString              sFieldSection;
  ZdFileName            sFileName;
  ZdIniSection          Section;

  try {
    sFileName.SetFullPath(gSourceDescriptor.GetImportFileName());
    sFileName.SetLocation(edtOutputDirectory->Text.c_str());
    //File extension to file type.
    switch (geFileType) {
      case Case                : sFileName.SetExtension(".cas");
                                 break;
      case Control             : sFileName.SetExtension(".ctl");
                                 break;
      case Population          : sFileName.SetExtension(".pop");
                                 break;
      case Coordinates         : sFileName.SetExtension(".geo");
                                 break;
      case SpecialGrid         : sFileName.SetExtension(".grd");
                                 break;
      case MaxCirclePopulation : sFileName.SetExtension(".max");
                                 break;
      case AdjustmentsByRR     : sFileName.SetExtension(".adj");
                                 break;
      default : ZdGenerateException("Unknown file type : \"%d\"", "ConvertImportedDataFile()", geFileType);
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
       if (geFileType == Coordinates && (t == 3 || t == 4))
         continue;
       if (geFileType == SpecialGrid && (t == 2 || t == 3))
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

/** returns interface coordinates type */
CoordinatesType TBDlgDataImporter::GetCoorinatesControlType() const {
  CoordinatesType eReturn;

  switch (rdoCoordinates->ItemIndex) {
    case 1  : eReturn = CARTESIAN; break;
    default : eReturn = LATLON;
  }
  return eReturn;
}

bool TBDlgDataImporter::GetDateFieldImported() const {
  bool bReturn = false;

  switch (geFileType) {
    case Case                :
    case Control             : bReturn = gvSaTScanVariables[2].GetIsMappedToInputFileVariable(); break;
    case Population          : bReturn = true; break;
    case Coordinates         :
    case SpecialGrid         :
    case MaxCirclePopulation : bReturn = false; break;
    case AdjustmentsByRR     : bReturn = gvSaTScanVariables[2].GetIsMappedToInputFileVariable(); break;
    default : ZdGenerateException("Unknown file type index: \"%d\"", "LoadResultFileNameIntoAnalysis()", geFileType);
  };
  return bReturn;
}

const char * TBDlgDataImporter::GetDestinationFilename(ZdString& sFilename) const {
  sFilename = gDestDescriptor.GetDestinationFileName();
  return sFilename.GetCString();
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

const char * TBDlgDataImporter::GetInputFileTypeString() const {
  const char * sFilename;

  switch (geFileType) {
    case Case                : sFilename = "Case File"; break;
    case Control             : sFilename = "Control File"; break;
    case Population          : sFilename = "Population File"; break;
    case Coordinates         : sFilename = "Coordinates File"; break;
    case SpecialGrid         : sFilename = "Grid File"; break;
    case MaxCirclePopulation : sFilename = "Max Circle Size File"; break;
    case AdjustmentsByRR     : sFilename = "Adjustments File"; break;
    default : ZdGenerateException("Unknown file type index: \"%d\"", "GetInputFileTypeString()", geFileType);
  };
  return sFilename;
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

/** Returns file type given source file extension. */ 
SourceDataFileType TBDlgDataImporter::GetSourceFileType() const {
  SourceDataFileType eType;

  if (!strcmpi(ZdFileName(gSourceDescriptor.GetImportFileName()).GetExtension(), ZdDBFFileType.GetFileTypeExtension()))
    eType = dBase;
  else
    eType = Delimited;

  return eType;
}

/** Hides rows based on coordinates variables selected to be shown. */
void TBDlgDataImporter::HideRows() {
  try {
    switch (geFileType) {
      case Coordinates : for (int i=1; i <= tsfieldGrid->Rows; i++) {
                            if (i == 2 || i == 3) //Latitude and Longitude
                              tsfieldGrid->RowVisible[i] = GetCoorinatesControlType() == LATLON;
                            else if (i >= 4) //Cartesian variables
                              tsfieldGrid->RowVisible[i] = GetCoorinatesControlType() == CARTESIAN;
                            else
                              tsfieldGrid->RowVisible[i] = true;
                         }
                         break;
      case SpecialGrid : for (int i=1; i <= tsfieldGrid->Rows; i++) {
                            if (i == 1 || i == 2) //Latitude and Longitude
                              tsfieldGrid->RowVisible[i] = GetCoorinatesControlType() == LATLON;
                            else //Cartesian variables
                              tsfieldGrid->RowVisible[i] = GetCoorinatesControlType() == CARTESIAN;
                         }
                         break;
      default          : for (int i=1; i <= tsfieldGrid->Rows; i++)
                            tsfieldGrid->RowVisible[i] = true;
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

  CreateDestinationInformation();
  if (chkFirstRowIsName->Checked)
    gSourceDescriptor.SetNumberOfRowsToIgnore(1);
  else
    gSourceDescriptor.SetNumberOfRowsToIgnore(0);

  BFileImportSourceInterface    FileImportSourceInterface(gpSourceFile, gSourceDescriptor.GetNumberOfRowsToIgnore());
  SaTScanFileImporter           FileImporter(gDestinationFileDefinition, geFileType,
                                             gSourceDataFileType, FileImportSourceInterface, gDestDescriptor);

  try {
    FileImporter.OpenDestination();
    SetMappings(FileImporter);
    FileImporter.Import();
    Modal = mrOk;
  }
  catch (ZdFileOpenFailedException &x) {
    sMessage << "The import wizard encountered an error attempting to create the import file.\n";
    sMessage << "This is most likely occuring because write permissions are not granted for\n";
    sMessage << "specified directory. Please check path or review user permissions for specified directory.\n";
    TBMessageBox(0, "Import cancelled!", sMessage.GetCString(), XBMB_OK|XBMB_EXCLAMATION).ShowModal();
    SetCurrentDir(GetToolkit().GetLastDirectory());
    Modal = mrNone;
  }
  catch (BImportRejectedException &x) {
    sMessage << "The import wizard encountered an error importing";
    if (FileImporter.GetImportErrors().size()) {
      sMessage << " field " << FileImporter.GetImportErrors()[0].iColumnNumber;
      sMessage << " in line " << FileImporter.GetImportErrors()[0].ulImportFileLine;
      sMessage << " of import source file.";
    }
    else  {
      sMessage << " data.\n\n";
      sMessage << x.GetErrorMessage();
    }
    TBMessageBox(0, "Import cancelled!", sMessage.GetCString(), XBMB_OK|XBMB_EXCLAMATION).ShowModal();
    Modal = mrNone;
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
    tsfieldGrid->Rows = gvSaTScanVariables.size();
    ClearSaTScanVariableFieldIndexes();
    AutoAlign();
    if (rdoCoordinates->ItemIndex == -1)
      //If coordinates index not set, set to current settings of analysis. 
      SetCoorinatesControlType(geStartingCoordinatesType);
    else
      HideRows();
    ContinueButtonEnable();
  }
  catch (ZdException &x) {
    x.AddCallpath("LoadMappingPanel()","TBDlgDataImporter");
    throw;
  }
}
                                        
/** Setup for panel to show. */
void TBDlgDataImporter::MakePanelVisible(TTabSheet* tabShowTab) {
  try {
    if (tabShowTab == tabFileFormat)
       OnViewFileFormatPanel();
     else if (tabShowTab == tabDataMapping)
       OnViewMappingPanel();
     else if (tabShowTab == tabOutputSettings)
       OnViewOutputSettingsPanel();
     else
       ImporterException::GenerateException("Unknown tab sheet.","MakePanelVisible()");
     BringPanelToFront(tabShowTab);
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
    DisableButtonsForImport(true);
    ModalResult = ImportFile();
    DisableButtonsForImport(false);
  }
  catch (ZdException &x) {
    x.AddCallpath("OnExecuteImport()", "TDlgSaTScanDataImporter");
    DisableButtonsForImport(false);
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

/** Preparation for viewing output settings panel. */
void TBDlgDataImporter::OnViewOutputSettingsPanel() {
  CheckForRequiredVariables();
  if (edtOutputDirectory->Text.IsEmpty())
     edtOutputDirectory->Text = (getenv("TMP") ? getenv("TMP") : GetCurrentDir().c_str());
}

/** Opens source file as character delimited source file. */
void TBDlgDataImporter::OpenSourceAsCSVFile() {
  try {
    gpSourceFile = new CSVFile(GetColumnDelimiter(), GetGroupMarker());
    ((CSVFile*)gpSourceFile)->SetInitialNondataLineCount(gSourceDescriptor.GetNumberOfRowsToIgnore());
    ((CSVFile*)gpSourceFile)->OpenAllFieldsAlpha(gSourceDescriptor.GetImportFileName(), ZDIO_OPEN_READ);
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
    gpSourceFile = ZdOpenFile(gSourceDescriptor.GetImportFileName(), ZDIO_OPEN_READ|ZDIO_SREAD);
  }
  catch (ZdException &x) {
    x.AddCallpath("OpenSourceAsDBaseFile()","TBDlgDataImporter");
    dBaseFileOpenException exception(x.GetErrorMessage(), "OpenSourceAsDBaseFile()");
    exception.SetCallpath(x.GetCallpath());
    throw exception;
  }
}

/** Opens source file as scanf formatted file. */
void TBDlgDataImporter::OpenSourceAsScanfFile() {
  try {
    gpSourceFile = new ScanfFile();
    ((ScanfFile*)gpSourceFile)->SetInitialNondataLineCount(gSourceDescriptor.GetNumberOfRowsToIgnore());
    ((ScanfFile*)gpSourceFile)->OpenAllFieldsAlpha(gSourceDescriptor.GetImportFileName(), ZDIO_OPEN_READ);
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
    fFileName.SetFullPath(gSourceDescriptor.GetImportFileName());
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
    gpSourceFile->Open(gSourceDescriptor.GetImportFileName(), ZDIO_OPEN_READ|ZDIO_SREAD, 0, 0, &DataFileDefinition);
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
    fImportDataFile.Open(gSourceDescriptor.GetImportFileName(), ZDIO_OPEN_READ|ZDIO_SREAD);

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

/** sets interface coordinates type */
void TBDlgDataImporter::SetCoorinatesControlType(CoordinatesType eCoordinatesType) {
  switch (eCoordinatesType) {
    case CARTESIAN : rdoCoordinates->ItemIndex = 1; break;
    default        : rdoCoordinates->ItemIndex = 0;
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
void TBDlgDataImporter::SetPanelsToShow(SourceDataFileType eType) {
  try {
    //Skip file format panel for dBase since we already know structure.
    if (eType == dBase) {
      gvTabSheets.clear();
      gvTabSheets.push_back(tabDataMapping);
      gvTabSheets.push_back(tabOutputSettings);
      gSourceDataFileType = eType;
      rdoFileType->ItemIndex = -1;
    }
    else {
      gvTabSheets.clear();
      gvTabSheets.push_back(tabFileFormat);
      gvTabSheets.push_back(tabDataMapping);
      gvTabSheets.push_back(tabOutputSettings);
      gSourceDataFileType = eType;
      rdoFileType->ItemIndex = 0;
    }
  }
  catch (ZdException &x) {
    x.AddCallpath("SetPanelsToShow()","TBDlgDataImporter");
    throw;
  }
}

/** Internal setup function. */
void TBDlgDataImporter::Setup(const char * sSourceFilename) {
  unsigned long ulCurrentStyle;
  std::ifstream filestream;

  try {
    //first check read access
    filestream.open(sSourceFilename, ios::binary);
    if (!filestream)
      ZdException::GenerateNotification("File '%s' could not be opened for reading.", "Setup()", sSourceFilename);
    filestream.close();

    gSourceDescriptor.SetImportFile(sSourceFilename);
    gDestDescriptor.SetDestinationType(BFileDestDescriptor::SingleFile);
    gDestDescriptor.SetModifyType(BFileDestDescriptor::OverWriteExistingData);
    gDestDescriptor.SetGenerateReport(false);
    gDestDescriptor.SetErrorOptionsType(BFileDestDescriptor::RejectBatch);
    ulCurrentStyle = GetWindowLong(lstFixedColFieldDefs->Handle, GWL_STYLE);
    SetWindowLong(lstFixedColFieldDefs->Handle, GWL_STYLE, ulCurrentStyle|WS_HSCROLL);
    SetPanelsToShow(GetSourceFileType());
    gitrCurrentTabSheet = gvTabSheets.begin();
    switch (geFileType) {
      case Case                : SetupCaseFileVariableDescriptors();
                                 rdoCoordinates->Enabled = false;
                                 pnlBottomPanelTopAligned->Visible = false;
                                 break;
      case Control             : SetupControlFileVariableDescriptors();
                                 rdoCoordinates->Enabled = false;
                                 pnlBottomPanelTopAligned->Visible = false;
                                 break;
      case Population          : SetupPopFileVariableDescriptors();
                                 rdoCoordinates->Enabled = false;
                                 pnlBottomPanelTopAligned->Visible = false;
                                 break;
      case Coordinates         : SetupGeoFileVariableDescriptors();
                                 rdoCoordinates->Enabled = true;
                                 pnlBottomPanelTopAligned->Visible = true;
                                 break;
      case SpecialGrid         : SetupGridFileVariableDescriptors();
                                 rdoCoordinates->Enabled = true;
                                 pnlBottomPanelTopAligned->Visible = true;
                                 break;
      case MaxCirclePopulation : SetupMaxCirclePopFileVariableDescriptors();
                                 rdoCoordinates->Enabled = false;
                                 pnlBottomPanelTopAligned->Visible = false;
                                 break;
      case AdjustmentsByRR     : SetupRelativeRisksFileVariableDescriptors();
                                 rdoCoordinates->Enabled = false;
                                 pnlBottomPanelTopAligned->Visible = false;
                                 break;
      default : ZdGenerateException("Unknown file type index: \"%d\"","SetUp()", geFileType);
    };
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
    gvSaTScanVariables.push_back(SaTScanVariable("Location ID", 0, true));
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
    gvSaTScanVariables.push_back(SaTScanVariable("Location ID", 0, true));
    gvSaTScanVariables.push_back(SaTScanVariable("Number of Controls", 1, true));
    gvSaTScanVariables.push_back(SaTScanVariable("Date/Time", 2, false));
//    gvSaTScanVariables.push_back(SaTScanVariable("Covariate1", 3, false));
//    gvSaTScanVariables.push_back(SaTScanVariable("Covariate2", 4, false ));
//    gvSaTScanVariables.push_back(SaTScanVariable("Covariate3", 5, false));
//    gvSaTScanVariables.push_back(SaTScanVariable("Covariate4", 6, false));
//    gvSaTScanVariables.push_back(SaTScanVariable("Covariate5", 7, false));
//    gvSaTScanVariables.push_back(SaTScanVariable("Covariate6", 8, false));
//    gvSaTScanVariables.push_back(SaTScanVariable("Covariate7", 9, false));
//    gvSaTScanVariables.push_back(SaTScanVariable("Covariate8", 10, false));
//    gvSaTScanVariables.push_back(SaTScanVariable("Covariate9", 11, false));
//    gvSaTScanVariables.push_back(SaTScanVariable("Covariate10", 12, false));
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
    gvSaTScanVariables.push_back(SaTScanVariable("Location ID", 0, true));
    gvSaTScanVariables.push_back(SaTScanVariable("Latitude", 1, true, "y-axis"));
    gvSaTScanVariables.push_back(SaTScanVariable("Longitude", 2, true, "x-axis"));
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
    gvSaTScanVariables.push_back(SaTScanVariable("Latitude", 0, true, "y-axis"));
    gvSaTScanVariables.push_back(SaTScanVariable("Longitude", 1, true, "x-axis"));
    gvSaTScanVariables.push_back(SaTScanVariable("X", 0, true));
    gvSaTScanVariables.push_back(SaTScanVariable("Y", 1, true));
    gvSaTScanVariables.push_back(SaTScanVariable("Z1", 2, false));
    gvSaTScanVariables.push_back(SaTScanVariable("Z2", 3, false));
    gvSaTScanVariables.push_back(SaTScanVariable("Z3", 4, false ));
    gvSaTScanVariables.push_back(SaTScanVariable("Z4", 5, false));
    gvSaTScanVariables.push_back(SaTScanVariable("Z5", 6, false ));
    gvSaTScanVariables.push_back(SaTScanVariable("Z6", 7, false));
    gvSaTScanVariables.push_back(SaTScanVariable("Z7", 8, false));
    gvSaTScanVariables.push_back(SaTScanVariable("Z8", 9, false));
  }
  catch (ZdException &x) {
    x.AddCallpath("SetupGridFileVariableDescriptors()", "TBDlgDataImporter");
    throw;
  }
}

/** Setup field descriptors for maximum circle population file. */
void TBDlgDataImporter::SetupMaxCirclePopFileVariableDescriptors() {
  try {
    gvSaTScanVariables.clear();
    gvSaTScanVariables.push_back(SaTScanVariable("Location ID", 0, true));
    gvSaTScanVariables.push_back(SaTScanVariable("Population", 1, true));
  }
  catch (ZdException &x) {
    x.AddCallpath("SetupMaxCirclePopFileVariableDescriptors()", "TBDlgDataImporter");
    throw;
  }
}

/** Setup field descriptors for population file. */
void TBDlgDataImporter::SetupPopFileVariableDescriptors() {
  try {
    gvSaTScanVariables.clear();
    gvSaTScanVariables.push_back(SaTScanVariable("Location ID", 0, true));
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

/** Setup field descriptors for relative risks file. */
void TBDlgDataImporter::SetupRelativeRisksFileVariableDescriptors() {
  try {
    gvSaTScanVariables.clear();
    gvSaTScanVariables.push_back(SaTScanVariable("Location ID", 0, true));
    gvSaTScanVariables.push_back(SaTScanVariable("Relative Risk", 1, true));
    gvSaTScanVariables.push_back(SaTScanVariable("Start Date", 2, false));
    gvSaTScanVariables.push_back(SaTScanVariable("End Date", 3, false));
  }
  catch (ZdException &x) {
    x.AddCallpath("SetupRelativeRiskFileVariableDescriptors()", "TBDlgDataImporter");
    throw;
  }
}

/** Causes the first panel of importer to be shown. */
void TBDlgDataImporter::ShowFirstPanel() {
  try {
    gitrCurrentTabSheet = gvTabSheets.begin();
    MakePanelVisible(*gitrCurrentTabSheet);
  }
  catch (ZdException &x) {
    x.AddCallpath("ShowFirstPanel()", "TBDlgDataImporter");
    throw;
  }
}

/** Causes next panel of importer to be shown. */
void TBDlgDataImporter::ShowNextPanel() {
  try {
    if (*gitrCurrentTabSheet != gvTabSheets.back()) {
      ++gitrCurrentTabSheet;
      try {
        MakePanelVisible(*gitrCurrentTabSheet);
      }
      catch (ZdException &x) {
        --gitrCurrentTabSheet;
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
    if (gitrCurrentTabSheet != gvTabSheets.begin()) {
      --gitrCurrentTabSheet;
      try {
        BringPanelToFront(*gitrCurrentTabSheet);
      }
      catch (ZdException &x) {
        ++gitrCurrentTabSheet;
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
  SourceDataFileType eFileType = gSourceDataFileType;

  if (eFileType == dBase) {
    //if source file type has been set to dBase, there is a possibility that
    //file open will fail; maybe the file isn't really dBase?
    try {
      SetPanelsToShow(eFileType);
      ShowFirstPanel();
      return;
    }
    catch (dBaseFileOpenException &x) {
      //opening file as dBase failed - try opening as default delimited ascii file
      eFileType = Delimited;
    }
    catch (ZdException &x) {
      x.AddCallpath("OnFormShow()","TBDlgDataImporter");
      DisplayBasisException(this, x);
      return;
    }
  }
  
  try {
    SetPanelsToShow(eFileType);
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

void __fastcall TBDlgDataImporter::OnOutputDirectoryChange(TObject *Sender) {
   ContinueButtonEnable();
}
//---------------------------------------------------------------------------

void __fastcall TBDlgDataImporter::OnChangeDirectoryClick(TObject *Sender) {
  AnsiString Directory;
  if (SelectDirectory("Select directory to save imported file", "", Directory))
    edtOutputDirectory->Text = Directory;
}
//---------------------------------------------------------------------------

//////////////////////////////////
/////ImporterException Class /////
//////////////////////////////////

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

///////////////////////////////////////
/////dBaseFileOpenException Class /////
///////////////////////////////////////

dBaseFileOpenException::dBaseFileOpenException(const char * sMessage, const char * sSourceModule, ZdException::Level iLevel)
                       :BException(sMessage, sSourceModule, iLevel){
}

void dBaseFileOpenException::GenerateException ( const char * sMessage, const char * sSourceModule, ZdException::Level iLevel, ... ) {
   va_list   vArgs;
   char      sExceptionString[2048];

   va_start ( vArgs, iLevel );
   vsprintf ( sExceptionString, sMessage, vArgs );
   va_end ( vArgs );

   throw dBaseFileOpenException(sExceptionString, sSourceModule, iLevel);
}

void dBaseFileOpenException::GenerateException( const char * sMessage, const char * sSourceModule ) {
   throw dBaseFileOpenException(sMessage, sSourceModule, ZdException::Normal);
}
