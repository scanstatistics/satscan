//---------------------------------------------------------------------------
#include <vcl.h>
#pragma hdrstop

#include "uprMain.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TfrmMain *frmMain;
//---------------------------------------------------------------------------
__fastcall TfrmMain::TfrmMain(TComponent* Owner): TForm(Owner) {}

/** deterimnes write accesss of archive file */
void TfrmMain::CheckAccess(const AnsiString& sFileFullPath) {
  FILE                * fpDest;
  AnsiString            s;

  try {
    // if file exists, check write permission
    if (access(sFileFullPath.c_str(), 00) == 0) {
      retry :
      if ((fpDest = fopen(sFileFullPath.c_str(), "ab")) == NULL) {
        s.printf("The updater have determined that file '%s' is open or active and can not be correctly updated.\n\n"
                 "Please shutdown any application(s) accessing this file and select retry.", sFileFullPath.c_str());
        if (Application->MessageBox(s.c_str(), "SaTScan Update", MB_RETRYCANCEL) == IDCANCEL)
          throw file_access_error(sFileFullPath.c_str());
        goto retry;
      }
      fclose(fpDest);
      remove(sFileFullPath.c_str());
    }
  }
  catch (const std::exception& e) {
    throw;
  }
}

/** throws exception if archive file path contains drive/UNC path
    -- this may not be neccesary */
void TfrmMain::CheckForDrive(const AnsiString& sArchiveFile) {
  AnsiString    s;

  try {
    if (ExtractFileDrive(sArchiveFile).Length()) {
      s.printf("Archive file '%s', contains drive/UNC path.", sArchiveFile);
      throw archive_error(s.c_str());
    }  
  }
  catch (const std::exception& e) {
    throw;
  }
}

/** replaces foward slashe with backslash for windows enviroment */
void TfrmMain::CorrectPathDelimiter(const AnsiString& sArchiveFile) {
  char * ptr;

  while ((ptr = strstr(sArchiveFile.c_str(), "/")) != NULL)
       *ptr = '\\';
}

/** attempts to create archive file path */
void TfrmMain::ExtendBasePath(AnsiString& sArchiveFile, AnsiString& ZipFileFullPath) {
  AnsiString    s, sWorkString, sParseString;
  const char  * ptr, * ptr2;
  int           iStart=1, iEnd;

  try {
    //sWorkString.printf("\\\\?\\%s", gsBasePath.c_str()); -- no supported on Windows 95
    sWorkString = gsBasePath.c_str();
    sParseString = sArchiveFile;
    while ((iEnd = sParseString.Pos("\\")) != 0) {
         sWorkString += sParseString.SubString(iStart, iEnd);
         sParseString = sParseString.SubString(iEnd + 1, sParseString.Length() - iEnd );
         if (access(sWorkString.c_str(), 00) && !CreateDirectory(sWorkString.c_str(), 0)) {
           s.printf("Unable to create archive file path, '%s'.", sArchiveFile.c_str());
           throw archive_error(s.c_str());
         }
    }
    ZipFileFullPath.printf("%s%s", gsBasePath.c_str(), sArchiveFile.c_str());
  }
  catch (const std::exception& e) {
    throw;
  }
}

/** open archive by file type extension */
void TfrmMain::OpenArchive() {
  AnsiString    Extension(ExtractFileExt(gsArchiveFile));

  try {
    if (Extension == ".tar" || Extension == ".gz" || Extension == ".tgz")
      OpenTarArchive();
    else if (Extension == ".zip")
      OpenZipArchive();
    else {
      Extension.sprintf("Unknown archive type for file '%s'.", gsArchiveFile.c_str());
      throw archive_error(Extension.c_str());
    }
  }
  catch (const std::exception& e) {
    throw;
  }
}

/** open tar archive */
void TfrmMain::OpenTarArchive() {
  size_t        t;
  AnsiString    ZipFileFullPath;

  try {
    force = 1; // allow existing files to be overwritten
    quiet = 1; // suppress the normal chatter
    listing = 1; // only print file names
    SetExtractionDirectory(gsBasePath.c_str());
    doarchive(gsArchiveFile.c_str());
    std::vector<std::string>& ZipFiles = GetZipFileNames();
    for (t=0; t < ZipFiles.size(); ++t) {
       ZipFileFullPath.printf("%s%s", gsBasePath.c_str(), ZipFiles[t].c_str());
       CheckAccess(ZipFileFullPath);
    }
    listing = 0; // extract files 
    doarchive(gsArchiveFile.c_str());
  }
  catch (const std::exception& e) {
    throw;
  }
}

/** open zip archive */
void TfrmMain::OpenZipArchive() {
  ZZIP_DIR            * dir=0;
  ZZIP_DIRENT         * d;
  ZZIP_FILE           * fp=0;
  zzip_error_t          rv;
  FILE                * fpDest=0;
  int                   i, fd;
  AnsiString            s, sArchiveName, ZipFileFullPath;
  char                  buf[1024];

  try {
    // open archive file
    if ((fd = open (gsArchiveFile.c_str(), O_RDONLY|O_BINARY)) == -1) {
      s.printf("Could not open archive file '%s'.", gsArchiveFile.c_str());
      throw archive_error(s.c_str());
    }

    // search and parse the archives central directory
    if ((dir = zzip_dir_fdopen(fd, &rv)) == 0) {
      s.printf("Error %d.", rv);
      throw archive_error(s.c_str());
    }

    // check archived file access in local enviroment
    for (i=0; i < 2; i++)
       while ((d=zzip_readdir(dir)) != 0) {
            sArchiveName = d->d_name;
            CorrectPathDelimiter(sArchiveName);
            CheckForDrive(sArchiveName);
            ZipFileFullPath.printf("%s%s", gsBasePath.c_str(), sArchiveName.c_str());
            CheckAccess(ZipFileFullPath);
       }

    // rewind dir pointer to extract archived files   
    zzip_rewinddir(dir);
    for (i = 0; i < 2; i++)  {
       while ((d=zzip_readdir(dir)) != 0) {
            sArchiveName = d->d_name;
            CorrectPathDelimiter(sArchiveName);
            ExtendBasePath(sArchiveName, ZipFileFullPath);
            if ((fpDest = fopen(ZipFileFullPath.c_str(), "wb")) == NULL) {
              s.printf("Could not open file '%s'.", ZipFileFullPath.c_str());
              throw archive_error(s.c_str());
            }
            fp = zzip_file_open(dir, const_cast<char*>(d->d_name), ZZIP_CASEINSENSITIVE);
            if (! fp) {
              s.printf("error %d: %s.", zzip_error(dir), zzip_strerror_of(dir));
              throw archive_error(s.c_str());
            }
            else {
              while (0 < (i = zzip_file_read(fp, buf, 1023)))
                fwrite(buf, i, 1, fpDest);
              if (i < 0) {
                s.printf("error %d\n", zzip_error(dir));
                throw archive_error(s.c_str());
              }
            }
            zzip_close(fp);
            fclose(fpDest);
       }
    }
    zzip_closedir(dir);
    close(fd);
  }
  catch (const std::exception& e) {
    if (fd) close(fd);
    if (dir) zzip_closedir(dir);
    if (fpDest) fclose(fpDest);
    if (fp) zzip_close(fp);
    throw;
  }
}

/** starts update */
void TfrmMain::RunUpdate(const char * sArchiveFile) {
  TCursor t(Screen->Cursor);

  try {
    Show();
    Screen->Cursor = crHourGlass;
    lblProgress->Caption = "Installing updates ...";
    Application->ProcessMessages();
    _sleep(2);
    SetArchive(sArchiveFile);
    OpenArchive();
    //rename user guide to name "SaTScan_Users_Guide.pdf", as linked to in short cut
    //this is a work around until WinZip code is corrected
    _sleep(1);
    lblProgress->Caption = "Update completed ...";
    Application->ProcessMessages();
    Screen->Cursor = crHourGlass;
    remove(_argv[1]); // delete the archive file
    _sleep(1);
  }
  catch (const std::exception& e) {
    Screen->Cursor = t;
    remove(_argv[1]); // delete the archive file
    throw;
  }
  Close();
}


