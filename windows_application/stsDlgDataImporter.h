// $Revision: 1.2 $
//Author Scott Hostovich
#ifndef __stsDlgDataImporter_H
#define __stsDlgDataImporter_H
//------------------------------------------------------------------------------
#include "Grids_ts.hpp"
#include "TSGrid.hpp"
#include <Buttons.hpp>
#include <Classes.hpp>
#include <ComCtrls.hpp>
#include <Controls.hpp>
#include <Dialogs.hpp>
#include <ExtCtrls.hpp>
#include <StdCtrls.hpp>
//------------------------------------------------------------------------------
#include "ScanfFile.h"

class TfrmAnalysis;
class TBDlgDataImporter : public TForm {
  __published:	// IDE-managed Components
     TPanel *pnlImportData;
     TtsGrid   *tsImportFileGrid;
     TBevel    *Bevel1;
     TtsGrid   *tsfieldGrid;
     TPanel *pnlImportWizard;
     TOpenDialog *OpenDialog;
     TButton *btnClearImports;
     TPanel *pnlFileFormat;
     TPanel *pnlCSVDefs;
     TLabel *Label3;
     TComboBox *cmbColDelimiter;
     TComboBox *cmbGroupMarker;
     TLabel *Label4;
     TPanel *pnlFixedColumnDefs;
     TGroupBox *GroupBox1;
     TLabel *Label6;
     TLabel *Label7;
     TLabel *Label8;
     TEdit *edtStartColumn;
     TEdit *edtFieldLength;
     TEdit *edtFieldName;
     TLabel *Label10;
     TListBox *lstFixedColFieldDefs;
     TButton *btnAddFldDef;
     TButton *btnDeleteFldDef;
     TRadioGroup *rdoFileType;
     TGroupBox *GroupBox3;
     TLabel *Label11;
     TLabel *Label12;
     TEdit *edtIgnoreFirstRows;
     TMemo *memRawData;
     TGroupBox *grpInputFileType;
     TComboBox *cmbInputFileType;
     TLabel *Label2;
     TLabel *Label9;
     TGroupBox *grpImportSourceFile;
     TEdit *edtDataFile;
     TBitBtn *btnBrowseForSourceFile;
     TPanel *pnlButtons;
     TBitBtn *btnPreviousPanel;
     TBitBtn *btnNextPanel;
     TBitBtn *btnCancel;
     TBitBtn *btnExecuteImport;
     TButton *btnAutoAlign;
     TRadioGroup *rdoCoordinates;
     TCheckBox *chkFirstRowIsName;
     void __fastcall NumericKeyPressMask(TObject *Sender, char &Key);
     void __fastcall OnAddFldDefClick(TObject *Sender);
     void __fastcall OnAutoAlignClick(TObject *Sender);
     void __fastcall OnBackClick(TObject *Sender);
     void __fastcall OnBrowseForImportFile(TObject *Sender);
     void __fastcall OnCmbDestinationChange(TObject *Sender);
     void __fastcall OnClearImportsClick(TObject *Sender);
     void __fastcall OnCSVComboChange(TObject *Sender);
     void __fastcall OnDeleteFldDefClick(TObject *Sender);
     void __fastcall OnExecuteClick(TObject *Sender);
     void __fastcall OnFieldGridCellLoaded(TObject *Sender, int DataCol,int DataRow, Variant &Value);
     void __fastcall OnFieldGridComboCellLoaded(TObject *Sender,TtsComboGrid *Combo, int DataCol, int DataRow, Variant &Value);
     void __fastcall OnFieldGridComboDropDown(TObject *Sender,TtsComboGrid *Combo, int DataCol, int DataRow);
     void __fastcall OnFieldGridComboGetValue(TObject *Sender,TtsComboGrid *Combo, int GridDataCol, int GridDataRow,int ComboDataRow, Variant &Value);
     void __fastcall OnFieldLengthChange(TObject *Sender);
     void __fastcall OnFieldNameChange(TObject *Sender);
     void __fastcall OnFieldTypeChange(TObject *Sender);
     void __fastcall OnFileTypeClick(TObject *Sender);
     void __fastcall OnFixedColFieldDefsClick(TObject *Sender);
     void __fastcall OnFormShow(TObject *Sender);
     void __fastcall OnFirstRowIsNameClick(TObject *Sender);
     void __fastcall OnNextClick(TObject *Sender);
     void __fastcall OnSourceImportFileChange(TObject *Sender);
     void __fastcall OnStartColumnChange(TObject *Sender);
     void __fastcall tsfieldGridResize(TObject *Sender);
     void __fastcall OnCoordinatesClick(TObject *Sender);

  public:
     enum Import_Panels {Start=0, FileType, DataMapping};

  private:	// User declarations
     void                            Init();
     virtual void                    Setup();

  protected:
     BFileSourceDescriptor                      gSourceDescriptor;
     BFileDestDescriptor                        gDestDescriptor;
     BZdFileViewController                    * gpController;
     BGridZdSingleFileModel                   * gpDataModel;
     ZdFile                                   * gpImportFile;
     ZdVector<int>                              gvPanels;
     ZdVector<int>::const_iterator              gitrCurrentPanel;
     ZdVector<ZdIniSection>                     gvIniSections;
     ZdVector< std::pair<ZdString,short> >      gSaTScanVariablesFieldMap;
     ZdPointerVector<ZdString>                  gvImportFieldChoices;
     ZdVector<ZdString*>                        gvImportingFields;
     ZdIniFile                                  gDataFileDefinition;
     ZdIniFile                                  gDestinationFileDefinition;
     TfrmAnalysis                             & gAnalysisForm;
     ZdString                                   gsBlank;
     SourceDataFileType                         gSourceDataFileType;

     void                            AddFixedColDefinitionEnable();
     void                            AdjustFileSourceFileAttributes(ZdFile & File);
     void                            AutoAlign();
     void                            BringPanelToFront(int iWhich);
     void                            CheckForRequiredVariables();
     void                            ClearImportFieldSelections();
     void                            ContinueButtonEnable();
     void                            CreateDestinationInformation();
     void                            DefineSourceFileStructure();
     void                            DeleteFixedColDefinitionEnable();
     void                            DisableButtonsForImport(bool bEnable);
     const char                      GetColumnDelimiter() const;
     ZdString                      & GetFixedColumnFieldName(unsigned int uwFieldIndex, ZdString & sFieldName);
     const char                      GetGroupMarker() const;
     void                            HideRows();
     TModalResult                    ImportFile();
     void                            InitializeImportingFields();
     void                            LoadMappingPanel();
     void                            LoadResultFileNameIntoAnalysis();
     void                            MakePanelVisible(int iWhich);
     void                            OnAddFieldDefinitionClick();
     void                            OnAutoAlignClick();
     void                            OnCmbDestinationChange();
     void                            OnDeleteFieldDefinitionClick();
     void                            OnExecuteImport();
     void                            OnExitStartPanel();
     void                            OnFieldDefinitionChange();
     void                            OnFirstRowIsHeadersClick();
     void                            OpenSourceFile();
     void                            OnViewFileFormatPanel();
     void                            OnViewMappingPanel();
     void                            ReadDataFileIntoRawDisplayField();
     void                            SelectImportFile();
     void                            SetGridHeaders(bool bFirstRowIsHeader=false);
     void                            SetImportFieldChoices();
     void                            SetMappings(BZdFileImporter & FileImporter);
     void                            SetPanelsToShow();
     void                            SetupCaseFileFieldDescriptors();
     void                            SetupControlFileFieldDescriptors();
     void                            SetupGeoFileFieldDescriptors();
     void                            SetupGridFileFieldDescriptors();
     void                            SetupPopFileFieldDescriptors();
     void                            ShowFileTypeFormatPanel(int iFileType);
     void                            ShowFirstPanel();
     void                            ShowNextPanel();
     void                            ShowPreviousPanel();
     void                            UpdateFileFormatOptions();

public:		// User declarations
     virtual __fastcall TBDlgDataImporter(TComponent* Owner, TfrmAnalysis & AnalysisForm);
     virtual __fastcall ~TBDlgDataImporter();
};
//---------------------------------------------------------------------------
extern PACKAGE TBDlgDataImporter *BDlgDataImporter;
//---------------------------------------------------------------------------

class ImporterException : public BException {
public:
   ImporterException ( const char * sMessage, const char * sSourceModule, ZdException::Level iLevel = Normal);
   virtual ~ImporterException(){};

   void static GenerateException ( const char * sMessage, const char * sSourceModule, ZdException::Level iLevel, ... );
   void static GenerateException ( const char * sMessage, const char * sSourceModule );
};
#endif
