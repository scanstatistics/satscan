//---------------------------------------------------------------------------
#include "stsSaTScan.h"
#pragma hdrstop
//---------------------------------------------------------------------------

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

/** Adjust file/ file field attributes for import
    - currently this function only applies to dBase files.
      Sets date filter to YYYY/MM/DD. */
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
  size_t              i, j;
  bool                bFound;

  try {
    for (i=0; i < gSaTScanVariablesFieldMap.size(); i++)
       for (j=1, bFound=false; j < gvImportFieldChoices.size() && !bFound; j++)
          if (! strcmpi(gSaTScanVariablesFieldMap[i].first.GetCString(), *(gvImportFieldChoices[j]))) {
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
  ZdString              sMessage, ExclusiveError;
  std::vector<size_t>   vMissingFieldIndex;
  bool                  bLatitudeBlank, bLongitudeBlank, bXBlank, bYBlank, bAllOptionalBlank=true;

  try {
    switch (rdgInputFileType->ItemIndex) {
      case Case        : //Case and Control file require first two fields only.
      case Control     : for (t=0; t < 2; t++)
                            if (*gvImportingFields[t] == gsBlank)
                              vMissingFieldIndex.push_back(t);
                         break;
      case Population  : //Population file requires first three fields only.
                         for (t=0; t < 3; t++)
                            if (*gvImportingFields[t] == gsBlank)
                              vMissingFieldIndex.push_back(t);
                         break;
      case Coordinates : //Coordinates file requires first field but also requires
                         //either cartesian variables or lat/long variables, not both.  
                         if (*gvImportingFields[0] == gsBlank)
                           vMissingFieldIndex.push_back(0);
                         bLatitudeBlank = (!tsfieldGrid->RowVisible[2] || *gvImportingFields[1] == gsBlank);
                         bLongitudeBlank = (!tsfieldGrid->RowVisible[3] || *gvImportingFields[2] == gsBlank);
                         bXBlank = (!tsfieldGrid->RowVisible[4] || *gvImportingFields[3] == gsBlank);
                         bYBlank  = (!tsfieldGrid->RowVisible[5] || *gvImportingFields[4] == gsBlank);
                         for (t=5; t < gvImportingFields.size() && bAllOptionalBlank; t++)
                            bAllOptionalBlank = (!tsfieldGrid->RowVisible[t+1] || *gvImportingFields[t] == gsBlank);
                         if (((bLatitudeBlank && !bLongitudeBlank) || (!bLatitudeBlank && bLongitudeBlank)) && (bXBlank && bYBlank && bAllOptionalBlank)) {
                           //User has selected one of the lat/long fields but none of cartesian fields.
                           //Indicate that one of them is missing. 
                           vMissingFieldIndex.push_back((bLatitudeBlank ? 1 : 2));
                         }
                         else if ((bLatitudeBlank && bLongitudeBlank) && ((!bXBlank && bYBlank) || (bXBlank && !bYBlank))) {
                           //User has selected one of the required cartesian fields but none of lat/long fields.
                           //Indicate that one of them is missing.
                           vMissingFieldIndex.push_back((bXBlank ? 3 : 4));
                         }
                         else if (bLatitudeBlank && bLongitudeBlank && bXBlank && bYBlank && !bAllOptionalBlank) {
                           //User has selected one of the required cartesian fields but none of lat/long fields.
                           //Indicate that one of them is missing.
                           vMissingFieldIndex.push_back(3);
                           vMissingFieldIndex.push_back(4);
                         }
                         else if (bLatitudeBlank && bLongitudeBlank && bXBlank && bYBlank && bAllOptionalBlank) {
                           //User has selected neither cartesian and lat/long fields.
                           ExclusiveError << "Either cartesian variables and latitude/longitude variables must be ";
                           ExclusiveError << "selected before import can proceed.\n";
                         }
                         else if (!((!bLatitudeBlank && !bLongitudeBlank && bXBlank && bYBlank && bAllOptionalBlank) ||
                                  (bLatitudeBlank && bLongitudeBlank && !bXBlank && !bYBlank))) {
                           //User has selected a combination of cartesian and lat/long fields.
                           ExclusiveError << "Note that both cartesian variables and latitude/longitude variables can not be ";
                           ExclusiveError << "selected together.\n";
                         }
                         break;
      case SpecialGrid : //Coordinates file requires either cartesian variables
                         //or lat/long variables, not both.  
                         bLatitudeBlank = (!tsfieldGrid->RowVisible[1] || *gvImportingFields[0] == gsBlank);
                         bLongitudeBlank = (!tsfieldGrid->RowVisible[2] || *gvImportingFields[1] == gsBlank);
                         bXBlank = (!tsfieldGrid->RowVisible[3] || *gvImportingFields[2] == gsBlank);
                         bYBlank  = (!tsfieldGrid->RowVisible[4] || *gvImportingFields[3] == gsBlank);
                         for (t=4; t < gvImportingFields.size() && bAllOptionalBlank; t++)
                            bAllOptionalBlank = (!tsfieldGrid->RowVisible[t+1] || *gvImportingFields[t] == gsBlank);
                         if (((bLatitudeBlank && !bLongitudeBlank) || (!bLatitudeBlank && bLongitudeBlank)) && (bXBlank && bYBlank && bAllOptionalBlank)) {
                           //User has selected one of the lat/long fields but none of cartesian fields.
                           //Indicate that one of them is missing. 
                           vMissingFieldIndex.push_back((bLatitudeBlank ? 0 : 1));
                         }
                         else if ((bLatitudeBlank && bLongitudeBlank) && ((!bXBlank && bYBlank) || (bXBlank && !bYBlank))) {
                           //User has selected one of the required cartesian fields but none of lat/long fields.
                           //Indicate that one of them is missing.
                           vMissingFieldIndex.push_back((bXBlank ? 2 : 3));
                         }
                         else if (bLatitudeBlank && bLongitudeBlank && bXBlank && bYBlank && !bAllOptionalBlank) {
                           //User has selected one of the required cartesian fields but none of lat/long fields.
                           //Indicate that one of them is missing.
                           vMissingFieldIndex.push_back(2);
                           vMissingFieldIndex.push_back(3);
                         }
                         else if (bLatitudeBlank && bLongitudeBlank && bXBlank && bYBlank && bAllOptionalBlank) {
                           //User has selected neither cartesian and lat/long fields.
                           ExclusiveError << "Either cartesian variables and latitude/longitude variables must be ";
                           ExclusiveError << "selected before import can proceed.\n";
                         }
                         else if (!((!bLatitudeBlank && !bLongitudeBlank && bXBlank && bYBlank && bAllOptionalBlank) ||
                                  (bLatitudeBlank && bLongitudeBlank && !bXBlank && !bYBlank))) {
                           //User has selected a combination of cartesian and lat/long fields.
                           ExclusiveError << "Note that both cartesian variables and latitude/longitude variables can not be ";
                           ExclusiveError << "selected together.\n";
                         }
                         break;
      default : ZdGenerateException("Unknown file type index: \"%d\"", "CheckForRequiredVariables()", rdgInputFileType->ItemIndex);
    };

    if (vMissingFieldIndex.size()) {
      sMessage << "For the " << rdgInputFileType->Items->Strings[rdgInputFileType->ItemIndex].c_str();
      sMessage << ", the following SaTScan Variable(s) are required\nand an Input File Variable must";
      sMessage << " be selected for each before import can proceed.\n\nSaTScan Variable(s): ";
      sMessage << gSaTScanVariablesFieldMap[vMissingFieldIndex[0]].first << "\n";
      for (t=1; t < vMissingFieldIndex.size(); t++) {
         sMessage << "                                  " << gSaTScanVariablesFieldMap[vMissingFieldIndex[t]].first;
         if (t < vMissingFieldIndex.size() - 1)
           sMessage << "\n";
      }
    }

    if (ExclusiveError.GetLength() || sMessage.GetLength()) {
      if (sMessage.GetLength())
        sMessage << "\n\n";
      sMessage << ExclusiveError;
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
       gvImportingFields[i] = gvImportFieldChoices[0];
    tsfieldGrid->Invalidate();
  }
  catch (ZdException &x) {
    x.AddCallpath("ClearImportFieldSelections()", "TBDlgDataImporter");
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
    case FileType    : btnNextPanel->Enabled = (rdoFileType->ItemIndex == 2 ||
                                               (rdoFileType->ItemIndex == 0 && cmbColDelimiter->GetTextLen()) ||
                                               (rdoFileType->ItemIndex == 1 &&  lstFixedColFieldDefs->Items->Count > 0));
                       break;
    case DataMapping : for (size_t i=0; i < gvImportingFields.size() && !btnExecuteImport->Enabled; ++i) {
                          if (tsfieldGrid->RowVisible[i+1])
                            btnExecuteImport->Enabled = (*gvImportingFields[i] != gsBlank);
                       }

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
    for (size_t t=0; t < gSaTScanVariablesFieldMap.size(); t++) {
       //Skip fields 'X' and 'Y', they are equivalant to 'Latitude' and 'Longitude'
       //in terms of creating a ZdIniFile since they will never exist together
       //and map to the same field index.
       if (rdgInputFileType->ItemIndex == Coordinates && (t == 3 || t == 4))
         continue;
       if (rdgInputFileType->ItemIndex == SpecialGrid && (t == 2 || t == 3))
         continue;

       sFieldSection.printf("[Field%d]", gSaTScanVariablesFieldMap[t].second + 1);
       Section.SetName(sFieldSection.GetCString());
       Section.AddLine("FieldName", gSaTScanVariablesFieldMap[t].first.GetCString());
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

/** Creates ZdIniFile to be used when opening source file for view/import. */
void TBDlgDataImporter::DefineSourceFileStructure() {
  size_t        t;
  ZdFileName    fFileName;
  ZdString      sFieldSection;

  try {
    gDataFileDefinition.Clear();
    if (rdoFileType->ItemIndex == 0) {//CSV File
      if (cmbColDelimiter->ItemIndex == 2/*white spacetab*/) {
        ScanfFile  fScanfFile;
        fScanfFile.OpenAsAlpha(edtDataFile->Text.c_str(), ZDIO_OPEN_READ, false);
        fScanfFile.WriteStructure(&gDataFileDefinition);
      }
      else {
        CSVFile  fCSVFile(GetColumnDelimiter(), GetGroupMarker() );
        fCSVFile.OpenAsAlpha(edtDataFile->Text.c_str(), ZDIO_OPEN_READ, false);
        fCSVFile.WriteStructure(&gDataFileDefinition);
      }  
    }
    else if (rdoFileType->ItemIndex == 1) {//Variable record length file
      ZdIniSection  tempSection("[FileInfo]");
      fFileName.SetFullPath(edtDataFile->Text.c_str());
      tempSection.AddLine("FileName", fFileName.GetCompleteFileName());
      tempSection.AddLine("Title", fFileName.GetFileName());
      tempSection.AddLine("NumberOfFields", IntToStr((int)gvIniSections.size()).c_str());
      int iRecordLength, iMaxLength = 0;
      for (t=0; t < gvIniSections.size(); ++t) {
        iRecordLength = StrToInt(gvIniSections[t].GetString("Length"));
        iRecordLength += StrToInt(gvIniSections[t].GetString("ByteOffSet"));
        iMaxLength = max(iRecordLength, iMaxLength);
      }
      tempSection.AddLine("RecordLength", IntToStr(iMaxLength).c_str());
      tempSection.AddLine("StartingCol", "1");
      gDataFileDefinition.AddSection(tempSection.Clone());
      for (t=0; t < gvIniSections.size(); ++t)
         gDataFileDefinition.AddSection(gvIniSections[t].Clone());
    }
    else { /* Nothing */ }
  }
  catch (ZdException &x) {
    x.AddCallpath("DefineSourceFileStructure()","TBDlgDataImporter");
    throw;
  }
}

/** Enables the deleting of fixed column field definitions. */
void TBDlgDataImporter::DeleteFixedColDefinitionEnable() {
  btnDeleteFldDef->Enabled = ( lstFixedColFieldDefs->ItemIndex != -1 );
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
    gSourceDescriptor.SetNumberOfRowsToIgnore(gSourceDescriptor.GetNumberOfRowsToIgnore() + 1);

  BFileImportSourceInterface    FileImportSourceInterface(gpImportFile, gSourceDescriptor.GetNumberOfRowsToIgnore());
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
  gsBlank << "unassigned";
  gpController = 0;
  gpDataModel = 0;
  gpImportFile = 0;
  cmbColDelimiter->ItemIndex = 0;
  cmbGroupMarker->ItemIndex = 0;
  gSourceDataFileType=Delimited;
}

/** Allocates and initializes vector to contains field mapping choices. */
void TBDlgDataImporter::InitializeImportingFields() {
  size_t     t;

  try {
    gvImportingFields.clear();
    for (t=0; t < gSaTScanVariablesFieldMap.size(); t++)
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
    CreateDestinationInformation();
    tsfieldGrid->Rows = gSaTScanVariablesFieldMap.size();
    InitializeImportingFields();
    SetImportFieldChoices();                   
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
      case Case        : SetupCaseFileFieldDescriptors();
                         rdoCoordinates->Enabled = false;
                         pnlBottomPanelTopAligned->Visible = false;
                         break;
      case Control     : SetupControlFileFieldDescriptors();
                         rdoCoordinates->Enabled = false;
                         pnlBottomPanelTopAligned->Visible = false;
                         break;
      case Population  : SetupPopFileFieldDescriptors();
                         rdoCoordinates->Enabled = false;
                         pnlBottomPanelTopAligned->Visible = false;
                         break;
      case Coordinates : SetupGeoFileFieldDescriptors();
                         rdoCoordinates->Enabled = true;
                         pnlBottomPanelTopAligned->Visible = true;
                         break;
      case SpecialGrid : SetupGridFileFieldDescriptors();
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
  int   i;

  try {
    SetGridHeaders(chkFirstRowIsName->Checked);
    for (i=1; gpController && i <= gpController->GetGridColCount(); i++)
        *(gvImportFieldChoices[i]) = gpController->GetTopGrid()->Col[i]->Heading.c_str();
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

/** Reads in a sample of data file into a memo field to help user
    to determine structure of source file. */
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

/** Makes column headers of data grid combobox items of selection grid. */
void TBDlgDataImporter::SetImportFieldChoices() {
  int   i;
  
  try {
    gvImportFieldChoices.DeleteAllElements();
    gvImportFieldChoices.push_back(gsBlank.Clone());
    for (i=1; gpController && i <= gpController->GetGridColCount(); i++)
        gvImportFieldChoices.push_back(new ZdString(gpController->GetTopGrid()->Col[i]->Heading.c_str()));
    tsfieldGrid->Invalidate();
  }
  catch (ZdException &x) {
    x.AddCallpath("SetImportFieldChoices()","TBdlgBaseImporter");
    throw;
  }
}

/** Tell import class what's imported from where. */
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
       if (tsfieldGrid->RowVisible[i+1] && *gvImportingFields[i] != gsBlank) {
          itr = std::find(gvImportFieldChoices.begin(), gvImportFieldChoices.end(), gvImportingFields[i]);
          if (itr !=  gvImportFieldChoices.end()) {
            pToField =  pFile->GetFieldInfo(gSaTScanVariablesFieldMap[i].second);
            FileImporter.AddMapping(gvImportFieldChoices.GetPosition(itr), pToField);
          }
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
  try {
    gDestDescriptor.SetDestinationType(BFileDestDescriptor::SingleFile);
    gDestDescriptor.SetModifyType(BFileDestDescriptor::OverWriteExistingData);
    gDestDescriptor.SetGenerateReport(false);
    gDestDescriptor.SetErrorOptionsType(BFileDestDescriptor::RejectBatch);
    rdgInputFileType->ItemIndex = 0;
  }
  catch (ZdException &x) {
    x.AddCallpath("SetUp()","TBDlgDataImporter");
    throw;
  }
}

/** Prepares grid for sample viewing of source file. */
void TBDlgDataImporter::OpenSourceFile() {
  try {
    delete gpController; gpController = 0;
    delete gpDataModel; gpDataModel = 0;
    delete gpImportFile; gpImportFile = 0;

    try {
      if (rdoFileType->ItemIndex == 0) {//Special ZdFile open for CSVFile
        if (cmbColDelimiter->ItemIndex == 2/*white space*/) {
          gpImportFile = new ScanfFile();
          gpImportFile->Open(edtDataFile->Text.c_str(), ZDIO_OPEN_READ|ZDIO_SREAD, 0, 0, &gDataFileDefinition);
        }
        else
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
      gpController->SetGridMode(Tsgrid::gmBrowse);
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

/** Setup field descriptors for case file. */
void TBDlgDataImporter::SetupCaseFileFieldDescriptors() {
  try {
    gSaTScanVariablesFieldMap.clear();
    gSaTScanVariablesFieldMap.push_back(std::make_pair("Tract ID",0));
    gSaTScanVariablesFieldMap.push_back(std::make_pair("Number of Cases",1));
    gSaTScanVariablesFieldMap.push_back(std::make_pair("Date/Time",2));
    gSaTScanVariablesFieldMap.push_back(std::make_pair("Covariate1 (optional)",3));
    gSaTScanVariablesFieldMap.push_back(std::make_pair("Covariate2 (optional)",4));
    gSaTScanVariablesFieldMap.push_back(std::make_pair("Covariate3 (optional)",5));
    gSaTScanVariablesFieldMap.push_back(std::make_pair("Covariate4 (optional)",6));
    gSaTScanVariablesFieldMap.push_back(std::make_pair("Covariate5 (optional)",7));
    gSaTScanVariablesFieldMap.push_back(std::make_pair("Covariate6 (optional)",8));
    gSaTScanVariablesFieldMap.push_back(std::make_pair("Covariate7 (optional)",9));
    gSaTScanVariablesFieldMap.push_back(std::make_pair("Covariate8 (optional)",10));
    gSaTScanVariablesFieldMap.push_back(std::make_pair("Covariate9 (optional)",11));
    gSaTScanVariablesFieldMap.push_back(std::make_pair("Covariate10 (optional)",12));
  }
  catch (ZdException &x) {
    x.AddCallpath("SetupCaseFileFieldDescriptors()", "TBDlgDataImporter");
    throw;
  }
}

/** Setup field descriptors for control file. */
void TBDlgDataImporter::SetupControlFileFieldDescriptors() {
  try {
    gSaTScanVariablesFieldMap.clear();
    gSaTScanVariablesFieldMap.push_back(std::make_pair("Tract ID",0));
    gSaTScanVariablesFieldMap.push_back(std::make_pair("Number of Controls",1));
    gSaTScanVariablesFieldMap.push_back(std::make_pair("Date/Time (optional)",2));
  }
  catch (ZdException &x) {
    x.AddCallpath("SetupControlFileFieldDescriptors()", "TBDlgDataImporter");
    throw;
  }
}

/** Setup field descriptors for coordinates file. */
void TBDlgDataImporter::SetupGeoFileFieldDescriptors() {
  try {
    gSaTScanVariablesFieldMap.clear();
    gSaTScanVariablesFieldMap.push_back(std::make_pair("Tract ID",0));
    gSaTScanVariablesFieldMap.push_back(std::make_pair("Latitude",1));
    gSaTScanVariablesFieldMap.push_back(std::make_pair("Longitude",2));
    gSaTScanVariablesFieldMap.push_back(std::make_pair("X",1));
    gSaTScanVariablesFieldMap.push_back(std::make_pair("Y",2));
    gSaTScanVariablesFieldMap.push_back(std::make_pair("Z1 (optional)",3));
    gSaTScanVariablesFieldMap.push_back(std::make_pair("Z2 (optional)",4));
    gSaTScanVariablesFieldMap.push_back(std::make_pair("Z3 (optional)",5));
    gSaTScanVariablesFieldMap.push_back(std::make_pair("Z4 (optional)",6));
    gSaTScanVariablesFieldMap.push_back(std::make_pair("Z5 (optional)",7));
    gSaTScanVariablesFieldMap.push_back(std::make_pair("Z6 (optional)",8));
    gSaTScanVariablesFieldMap.push_back(std::make_pair("Z7 (optional)",9));
    gSaTScanVariablesFieldMap.push_back(std::make_pair("Z8 (optional)",10));
  }
  catch (ZdException &x) {
    x.AddCallpath("SetupGeoFileFieldDescriptors()", "TBDlgDataImporter");
    throw;
  }
}

/** Setup field descriptors for special grid file. */
void TBDlgDataImporter::SetupGridFileFieldDescriptors() {
  try {
    gSaTScanVariablesFieldMap.clear();
    gSaTScanVariablesFieldMap.push_back(std::make_pair("Latitude",0));
    gSaTScanVariablesFieldMap.push_back(std::make_pair("Longitude",1));
    gSaTScanVariablesFieldMap.push_back(std::make_pair("X",0));
    gSaTScanVariablesFieldMap.push_back(std::make_pair("Y",1));
    gSaTScanVariablesFieldMap.push_back(std::make_pair("Z1 (optional)",2));
    gSaTScanVariablesFieldMap.push_back(std::make_pair("Z2 (optional)",3));
  }
  catch (ZdException &x) {
    x.AddCallpath("SetupGridFileFieldDescriptors()", "TBDlgDataImporter");
    throw;
  }
}

/** Setup field descriptors for population file. */
void TBDlgDataImporter::SetupPopFileFieldDescriptors() {
  try {
    gSaTScanVariablesFieldMap.clear();
    gSaTScanVariablesFieldMap.push_back(std::make_pair("Tract ID",0));
    gSaTScanVariablesFieldMap.push_back(std::make_pair("Date/Time",1));
    gSaTScanVariablesFieldMap.push_back(std::make_pair("Population",2));
    gSaTScanVariablesFieldMap.push_back(std::make_pair("Covariate1 (optional)",3));
    gSaTScanVariablesFieldMap.push_back(std::make_pair("Covariate2 (optional)",4));
    gSaTScanVariablesFieldMap.push_back(std::make_pair("Covariate3 (optional)",5));
  }
  catch (ZdException &x) {
    x.AddCallpath("SetupPopFileFieldDescriptors()", "TBDlgDataImporter");
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
      case  1  :     Value = gSaTScanVariablesFieldMap[DataRow - 1].first.GetCString();
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
    ClearFixedColDefinitionEnable();
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


