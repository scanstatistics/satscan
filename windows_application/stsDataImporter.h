//---------------------------------------------------------------------------
#ifndef stsDataImporterH
#define stsDataImporterH
//---------------------------------------------------------------------------

enum InputFileType      {Case=0, Control, Population, Coordinates, SpecialGrid};
enum SourceDataFileType {Delimited=0, Fixed_Column, dBase};

class SaTScanFileImporter : public BZdFileImporter {
  private:
     void                       Init() {gwDateFilteredField=-1;}
     void                       Setup();

  protected:
    InputFileType               geFileType;
    SourceDataFileType          geSourceDataFileType;
    short                       gwDateFilteredField;
    ZdDateFilter                gDateFilter;
    char                        gsFilterBuffer[1024];

    void                        AquireUniqueFileName(ZdFileName& FileName);
    void                        AttemptFilterDateField(ZdString & sDateToken);
    void                        ConvertImportedDataFile();
    virtual bool                PutTokenToRecord(ZdFileRecord & Record, ZdString & sToken, short wField);

  public:
    SaTScanFileImporter(InputFileType eFileType, SourceDataFileType eSourceDataFileType,
                        BImportSourceInterface & SourceInterface, BFileDestDescriptor & FileDestDescriptor);
    virtual ~SaTScanFileImporter();

    const ZdVector<PutError> &  GetImportErrors() const {return gvErrors;}
    virtual void                Import(ZdProgressInterface * pProgressInterface=0);
};
//---------------------------------------------------------------------------
#endif
 