// $Revision: 1.18 $
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
#include <FileCtrl.hpp>
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
    ZdString               gsHelpText;

    void                   Init();
    void                   Copy(const SaTScanVariable & rhs);

  public:
    SaTScanVariable() {Init();}
    SaTScanVariable(const SaTScanVariable & rhs);
    SaTScanVariable(const char * sVariableName, short wTargetFieldIndex, bool bRequiredVariable, const char * sHelpText=0);
    ~SaTScanVariable();

    SaTScanVariable      & operator=(const SaTScanVariable & rhs);
    SaTScanVariable      * Clone() const;

    const ZdString       & GetHelpText() const {return gsHelpText;}
    short                  GetInputFileVariableIndex() const {return gwInputFileVariableIndex;}
    bool                   GetIsMappedToInputFileVariable() const {return gwInputFileVariableIndex > 0;}
    bool                   GetIsRequiredField() const {return gbRequiredVariable;}
    short                  GetTargetFieldIndex() const {return gwTargetFieldIndex;}
    void                   GetVariableDisplayName(Variant & Value) const;
    const ZdString       & GetVariableName() const {return gsVariableName;}
    void                   SetHelpText(const char * sHelpText) {gsHelpText = sHelpText;}
    void                   SetIsRequiredField(bool bRequiredVariable) {gbRequiredVariable = bRequiredVariable;}
    void                   SetInputFileVariableIndex(short wInputFileVariableIndex) {gwInputFileVariableIndex = wInputFileVariableIndex;}
    void                   SetTargetFieldIndex(short wTargetFieldIndex) {gwTargetFieldIndex = wTargetFieldIndex;}
    void                   SetVariableName(const char * sVariableName) {gsVariableName = sVariableName;}
};

class TfrmAnalysis;
class TBDlgDataImporter : public TForm {
  __published:	// IDE-managed Components
     TOpenDialog *OpenDialog;
     TPanel *pnlButtons;
     TBitBtn *btnPreviousPanel;
     TBitBtn *btnNextPanel;
     TBitBtn *btnCancel;
     TBitBtn *btnExecuteImport;
     TCheckBox *chkFirstRowIsName;
     TPageControl *pgcImportPages;
     TTabSheet *tabDataMapping;
     TTabSheet *tabFileFormat;
     TPanel *pnlImportData;
     TPanel *pnlMappingPanelTop;
     TPanel *pnlTopPanelClient;
     TtsGrid *tsfieldGrid;
     TButton *btnClearImports;
     TButton *btnAutoAlign;
     TPanel *pnlMappingPanelClient;
     TPanel *pnlBottomClient;
     TtsGrid *tsImportFileGrid;
     TPanel *pnlFileFormat;
     TPanel *pnlCSVDefs;
     TLabel *Label3;
     TLabel *Label4;
     TComboBox *cmbColDelimiter;
     TComboBox *cmbGroupMarker;
     TPanel *pnlFixedColumnDefs;
     TLabel *Label10;
     TLabel *lblFieldLength;
     TLabel *Label9;
     TGroupBox *GroupBox1;
     TLabel *Label6;
     TLabel *lblFldLen;
     TLabel *Label8;
     TEdit *edtStartColumn;
     TEdit *edtFieldLength;
     TEdit *edtFieldName;
     TListBox *lstFixedColFieldDefs;
     TButton *btnAddFldDef;
     TButton *btnDeleteFldDef;
     TButton *btnClearFldDefs;
     TButton *btnUpdateFldDef;
     TRadioGroup *rdoFileType;
     TGroupBox *GroupBox3;
     TLabel *Label11;
     TLabel *Label12;
     TEdit *edtIgnoreFirstRows;
     TMemo *memRawData;
     TTabSheet *tabOutputSettings;
     TLabel *lblOutputDirectoryLabel;
     TEdit *edtOutputDirectory;
     TButton *btnChangeDirectory;
     TPanel *pnlTopPanelTop;
     TComboBox *cmbDisplayVariables;
     TStaticText *StaticText1;
        TSplitter *Splitter1;
     void __fastcall NumericKeyPressMask(TObject *Sender, char &Key);
     void __fastcall OnAddFldDefClick(TObject *Sender);
     void __fastcall OnAutoAlignClick(TObject *Sender);
     void __fastcall OnBackClick(TObject *Sender);
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
     void __fastcall OnStartColumnChange(TObject *Sender);
     void __fastcall OnFieldGridResize(TObject *Sender);
     void __fastcall OnDisplayVariableCoordinatesClick(TObject *Sender);
     void __fastcall OnClearFldDefsClick(TObject *Sender);
     void __fastcall OnUpdateFldDefClick(TObject *Sender);
     void __fastcall OnOutputDirectoryChange(TObject *Sender);
     void __fastcall OnChangeDirectoryClick(TObject *Sender);

  private:	// User declarations
     void                            Init();
     virtual void                    Setup(const char * sSourceFilename);

  protected:
     BFileSourceDescriptor           gSourceDescriptor;
     BFileDestDescriptor             gDestDescriptor;
     SourceViewController          * gpController;
     BGridZdSingleFileModel        * gpDataModel;
     ZdFile                        * gpSourceFile;
     std::vector<TTabSheet*>         gvTabSheets;
     std::vector<TTabSheet*>::iterator gitrCurrentTabSheet;
     ZdVector<ZdIniSection>          gvIniSections;
     ZdVector<SaTScanVariable>       gvSaTScanVariables;
     ZdIniFile                       gDestinationFileDefinition;
     ZdString                        gsUnassigned;
     SourceDataFileType              gSourceDataFileType;
     bool                            gbErrorSamplingSourceFile;
     CoordinatesType                 geStartingCoordinatesType;
     ProbabilityModelType            geStartingModelType;
     InputFileType                   geFileType;

     void                            AddFixedColDefinitionEnable();
     void                            AdjustSourceFileAttributes(ZdFile & File);
     void                            AutoAlign();
     void                            BringPanelToFront(TTabSheet* tabShowTab);
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
     const char                    * GetInputFileTypeString() const;     
     const char                    * GetInputFileVariableName(int iFieldIndex) const;
     int                             GetNumInputFileVariables() const;
     SourceDataFileType              GetSourceFileType() const;
     void                            HideRows();
     TModalResult                    ImportFile();
     void                            InitializeInputFileVariableMappings();
     void                            LoadMappingPanel();
     void                            MakePanelVisible(TTabSheet* tabShowTab);
     void                            OnAddFieldDefinitionClick();
     void                            OnAutoAlignClick();
     void                            OnDeleteFieldDefinitionClick();
     void                            OnExecuteImport();
     void                            OnFieldDefinitionChange();
     void                            OnFirstRowIsHeadersClick();
     void                            OpenSourceAsCSVFile();
     void                            OpenSourceAsDBaseFile();
     void                            OpenSourceAsScanfFile();
     void                            OpenSourceAsTXVFile();
     void                            OpenSource();
     void                            OnViewFileFormatPanel();
     void                            OnViewMappingPanel();
     void                            OnViewOutputSettingsPanel();
     void                            ReadDataFileIntoRawDisplayField();
     void                            SelectImportFile();
     void                            SetGridHeaders(bool bFirstRowIsHeader=false);
     void                            SetMappings(BZdFileImporter & FileImporter);
     void                            SetPanelsToShow(SourceDataFileType eType);
     void                            SetupCaseFileVariableDescriptors();
     void                            SetupControlFileVariableDescriptors();
     void                            SetupGeoFileVariableDescriptors();
     void                            SetupGridFileVariableDescriptors();
     void                            SetupMaxCirclePopFileVariableDescriptors();
     void                            SetupPopFileVariableDescriptors();
     void                            SetupRelativeRisksFileVariableDescriptors();
     void                            ShowFileTypeFormatPanel(int iFileType);
     void                            ShowFirstPanel();
     void                            ShowNextPanel();
     void                            ShowPreviousPanel();
     void                            UpdateFieldDefinition();
     void                            UpdateFileFormatOptions();
     void                            UpdateFixedColDefinitionEnable();
     void                            ValidateImportSource();

public:		// User declarations
     virtual __fastcall TBDlgDataImporter(TComponent* Owner, const char * sSourceFilename, InputFileType eFileType,
                                          ProbabilityModelType eModelType, CoordinatesType eCoordinatesType);
     virtual __fastcall TBDlgDataImporter(TComponent* Owner, TfrmAnalysis & AnalysisForm);
     virtual __fastcall ~TBDlgDataImporter();

     CoordinatesType                 GetCoorinatesControlType() const;
     const char *                    GetDestinationFilename(ZdString& sFilename) const;
     bool                            GetDateFieldImported() const;
     ProbabilityModelType            GetModelControlType() const;
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

class dBaseFileOpenException : public BException {
public:
   dBaseFileOpenException ( const char * sMessage, const char * sSourceModule, ZdException::Level iLevel = Normal);
   virtual ~dBaseFileOpenException(){};

   void static GenerateException ( const char * sMessage, const char * sSourceModule, ZdException::Level iLevel, ... );
   void static GenerateException ( const char * sMessage, const char * sSourceModule );
};

#endif
