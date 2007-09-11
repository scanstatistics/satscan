//---------------------------------------------------------------------------

#ifndef QueueItemDefinitionH
#define QueueItemDefinitionH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Dialogs.hpp>
#include <ExtCtrls.hpp>
//---------------------------------------------------------------------------
class TfrmBatchDefinition : public TForm {
__published:	// IDE-managed Components
        TPanel *pnlClient;
        TLabel *lblExecutable;
        TEdit *edtExecutableFileName;
        TButton *Button1;
        TEdit *edtParameterFilename;
        TButton *Button2;
        TLabel *lblParameterFile;
        TOpenDialog *OpenDialog;
        TPanel *pnlBottom;
        TButton *btnOk;
        TButton *btnCancel;
        TLabel *lblCommandLineOptions;
        TEdit *edtCommandLineOptions;
        void __fastcall BrowseExecutableFilenameClick(TObject *Sender);
        void __fastcall BrowseParameterFilenameClick(TObject *Sender);
private:	// User declarations
  AnsiString    gsExecutableFileName,
                gsParameterFileName;

  void          EnableOkButton();
  void          SetExecutableFilename(const char * sExecutableFilename);
  void          SetParameterFilename(const char * sParameterFilename);

public:		// User declarations
  __fastcall TfrmBatchDefinition(TComponent* Owner, const AnsiString & ExecutableFilename, const AnsiString & ParameterFilename);

  const char *  GetExecutableFilename() const {return gsExecutableFileName.c_str();}
  const char *  GetParameterFilename() const {return gsParameterFileName.c_str();}
};
#endif
