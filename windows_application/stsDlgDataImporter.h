// $Revision: 1.9 $
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

class SourceViewController : public BZdFileViewController {
  protected:
    virtual void         OnCellLoaded(int DataCol, int DataRow, Variant &Value);

  public:
            SourceViewController(TtsGrid * pTopGrid, BGridZdAbstractFileModel * pGridDataModel);
    virtual ~SourceViewController() {}
};

class SaTScanVariable {
  private:
    ZdString               gsVariableName;
    short                  gwTargetFieldIndex;
    bool                   gbRequiredVariable;
    short                  gwInputFileVariableIndex;

    void                   Init();
    void                   Copy(const SaTScanVariable & rhs);

  public:
    SaTScanVariable() {Init();}
    SaTScanVariable(const SaTScanVariable & rhs);
    SaTScanVariable(const char * sVariableName, short wTargetFieldIndex, bool bRequiredVariable);
    ~SaTScanVariable();

    SaTScanVariable      & operator=(const SaTScanVariable & rhs);
    SaTScanVariable      * Clone() const;

    short                  GetInputFileVariableIndex() const {return gwInputFileVariableIndex;}
    bool                   GetIsMappedToInputFileVariable() {return gwInputFileVariableIndex > 0;}
    bool                   GetIsRequiredField() const {return gbRequiredVariable;}
    short                  GetTargetFieldIndex() const {return gwTargetFieldIndex;}
    void                   GetVariableDisplayName(Variant & Value) const;
    const ZdString &       GetVariableName() const {return gsVariableName;}
    void                   SetIsRequiredField(bool bRequiredVariable) {gbRequiredVariable = bRequiredVariable;}
    void                   SetInputFileVariableIndex(short wInputFileVariableIndex) {gwInputFileVariableIndex = wInputFileVariableIndex;}
    void                   SetTargetFieldIndex(short wTargetFieldIndex) {gwTargetFieldIndex = wTargetFieldIndex;}
    void                   SetVariableName(const char * sVariableName) {gsVariableName = sVariableName;}
};

class TfrmAnalysis;
class TBDlgDataImporter : public TForm {
  __published:	// IDE-managed Components
     TPanel *pnlImportData;
     TPanel *pnlImportWizard;
     TOpenDialog *OpenDialog;
     TPanel *pnlFileFormat;
     TPanel *pnlCSVDefs;
     TLabel *Label3;
     TComboBox *cmbColDelimiter;
     TComboBox *cmbGroupMarker;
     TLabel *Label4;
     TPanel *pnlFixedColumnDefs;
     TGroupBox *GroupBox1;
     TLabel *Label6;
     TLabel *lblFldLen;
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
     TLabel *lblFieldLength;
     TLabel *Label9;
     TGroupBox *grpImportSourceFile;
     TEdit *edtDataFile;
     TBitBtn *btnBrowseForSourceFile;
     TPanel *pnlButtons;
     TBitBtn *btnPreviousPanel;
     TBitBtn *btnNextPanel;
     TBitBtn *btnCancel;
     TBitBtn *btnExecuteImport;
     TCheckBox *chkFirstRowIsName;
     TButton *btnClearFldDefs;
     TRadioGroup *rdgInputFileType;
     TPanel *pnlMappingPanelTop;
     TPanel *pnlMappingPanelClient;
     TPanel *pnlTopPanelClient;
     TtsGrid *tsfieldGrid;
     TButton *btnClearImports;
     TButton *btnAutoAlign;
     TPanel *pnlBottomPanelTopAligned;
     TRadioGroup *rdoCoordinates;
     TPanel *pnlBottomClient;
     TtsGrid *tsImportFileGrid;
     TBevel *Bevel1;
     TButton *btnUpdateFldDef;
     void __fastcall NumericKeyPressMask(TObject *Sender, char &Key);
     void __fastcall OnAddFldDefClick(TObject *Sender);
     void __fastcall OnAutoAlignClick(TObject *Sender);
     void __fastcall OnBackClick(TObject *Sender);
     void __fastcall OnBrowseForImportFile(TObject *Sender);
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
     void __fastcall OnClearFldDefsClick(TObject *Sender);
     void __fastcall OnUpdateFldDefClick(TObject *Sender);

  public:
     enum Import_Panels {Start=0, FileType, DataMapping};

  private:	// User declarations
     void                            Init();
     virtual void                    Setup();

  protected:
     BFileSourceDescriptor           gSourceDescriptor;
     BFileDestDescriptor             gDestDescriptor;
     SourceViewController          * gpController;
     BGridZdSingleFileModel        * gpDataModel;
     ZdFile                        * gpSourceFile;
     ZdVector<int>                   gvPanels;
     ZdVector<int>::const_iterator   gitrCurrentPanel;
     ZdVector<ZdIniSection>          gvIniSections;
     ZdVector<SaTScanVariable>       gvSaTScanVariables;
     ZdIniFile                       gDestinationFileDefinition;
     TfrmAnalysis                  & gAnalysisForm;
     ZdString                        gsUnassigned;
     SourceDataFileType              gSourceDataFileType;
     bool                            gbErrorSamplingSourceFile;

     void                            AddFixedColDefinitionEnable();
     void                            AdjustSourceFileAttributes(ZdFile & File);
     void                            AutoAlign();
     void                            BringPanelToFront(int iWhich);
     void                            CheckForRequiredVariables();
     void                            ClearFixedColumnDefinitions();
     void                            ClearFixedColDefinitionEnable();
     void                            ClearSaTScanVariableFieldIndexes();
     void                            ContinueButtonEnable();
     void                            CreateDestinationInformation();
     void                            DeleteFixedColDefinitionEnable();
     void                            DisableButtonsForImport(bool bEnable);
     const char                      GetColumnDelimiter() const;
     ZdString                      & GetFixedColumnFieldName(unsigned int uwFieldIndex, ZdString & sFieldName);
     const char                      GetGroupMarker() const;
     const char                    * GetInputFileVariableName(int iFieldIndex) const;
     int                             GetNumInputFileVariables() const;   
     void                            HideRows();
     TModalResult                    ImportFile();
     void                            InitializeInputFileVariableMappings();
     void                            LoadMappingPanel();
     void                            LoadResultFileNameIntoAnalysis();
     void                            MakePanelVisible(int iWhich);
     void                            OnAddFieldDefinitionClick();
     void                            OnAutoAlignClick();
     void                            OnDeleteFieldDefinitionClick();
     void                            OnExecuteImport();
     void                            OnExitStartPanel();
     void                            OnFieldDefinitionChange();
     void                            OnFirstRowIsHeadersClick();
     void                            OpenSourceAsCSVFile();
     void                            OpenSourceAsDBaseFile();
     void                            OpenSourceAsScanfFile();
     void                            OpenSourceAsTXVFile();
     void                            OpenSource();
     void                            OnViewFileFormatPanel();
     void                            OnViewMappingPanel();
     void                            ReadDataFileIntoRawDisplayField();
     void                            SelectImportFile();
     void                            SetGridHeaders(bool bFirstRowIsHeader=false);
     void                            SetMappings(BZdFileImporter & FileImporter);
     void                            SetPanelsToShow();
     void                            SetupCaseFileVariableDescriptors();
     void                            SetupControlFileVariableDescriptors();
     void                            SetupGeoFileVariableDescriptors();
     void                            SetupGridFileVariableDescriptors();
     void                            SetupMaxCirclePopFileVariableDescriptors();
     void                            SetupPopFileVariableDescriptors();
     void                            ShowFileTypeFormatPanel(int iFileType);
     void                            ShowFirstPanel();
     void                            ShowNextPanel();
     void                            ShowPreviousPanel();
     void                            UpdateFieldDefinition();
     void                            UpdateFileFormatOptions();
     void                            UpdateFixedColDefinitionEnable();
     void                            ValidateImportSource();

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
