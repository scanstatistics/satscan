//---------------------------------------------------------------------------
#ifndef uprMainH
#define uprMainH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <string>
#include <vector>
#include <dos.h>
#include "zziplibrary.h"
#include "uprException.h"
//---------------------------------------------------------------------------
class TfrmMain : public TForm {
__published:	// IDE-managed Components
        TLabel *lblProgress;

  private:	// User declarations
    AnsiString                  gsArchiveFile,
                                gsBasePath;

    void                CheckAccess(const AnsiString& sFileFullPath);
    void                CheckForDrive(const AnsiString& sArchiveFile);
    void                CorrectPathDelimiter(const AnsiString& sArchiveFile);
    void                ExtendBasePath(AnsiString& sArchiveFile, AnsiString& ZipFileFullPath);
    void                OpenArchive();
    void                OpenTarArchive();
    void                OpenZipArchive();

  public:		// User declarations
    __fastcall TfrmMain(TComponent* Owner);

    void                RunUpdate(const char * sArchiveFile);
    void                SetArchive(const char * sArchiveFile) {gsArchiveFile = sArchiveFile;
                                                               gsBasePath = ExtractFilePath(Application->ExeName); }
};
//---------------------------------------------------------------------------
extern PACKAGE TfrmMain *frmMain;
//---------------------------------------------------------------------------
#endif
