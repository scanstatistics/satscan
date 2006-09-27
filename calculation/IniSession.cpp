//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "IniSession.h"

BSessionException::BSessionException ( const char * sMessage, const char * sSourceModule, ZdException::Level iLevel )
                  :ZdException(sMessage, sSourceModule, iLevel){
}

void  BSessionException::GenerateSessionException ( const char * sMessage, const char * sSourceModule, ZdException::Level iLevel, ... ){
   va_list   vArgs;
   char      sExceptionString[2048];

   va_start ( vArgs, iLevel );
   vsprintf ( sExceptionString, sMessage, vArgs );
   va_end ( vArgs );

   throw BSessionException(sExceptionString, sSourceModule, iLevel);
}

void  BSessionException::GenerateSessionException ( const char * sMessage, const char * sSourceModule ){
   throw BSessionException(sMessage, sSourceModule, ZdException::Normal);
}

//------------------------------------------------------------------------------
//ClassDesc Begin BBaseSession
//To use this class you must define __USE_XBBASESESSION
//A Key in this class has a format different from a typical .ini file
//There is no separate SectionName in a SessionProperty, instead, each Key includes
//both a SectionName and a Key... so a SessionProperty Key takes the form of
//[SectionName].Key
//ClassDesc End BBaseSession

//value searched for in GetBool()
const char* BSessionProperty::BOOLEAN_SEARCH_VALUE = "true";

BSessionProperty::BSessionProperty(){
}

BSessionProperty::BSessionProperty(const char * sKey, const char * sValue){
   try{
      Init();
      SetKey(sKey);
      if (sValue)
         SetValue(sValue);
   }
   catch(ZdException &x){
      x.AddCallpath("Constructor", "BSessionProperty");
      throw;
   }
}

BSessionProperty::BSessionProperty(const BSessionProperty& rhs){
   try{
      Init();
      Copy(rhs);
   }
   catch(ZdException &x){
      x.AddCallpath("Copy Constructor","BSessionProperty");
      throw;
   }
}

BSessionProperty::~BSessionProperty(){}

// operators
BSessionProperty&  BSessionProperty::operator=  (const BSessionProperty& rhs){
   try{
      if (this != &rhs)
         Copy(rhs);
   }
   catch(ZdException &x){
      x.AddCallpath("Assignment Operator","BSessionProperty");
      throw;
   }
   return (*this);
}

bool  BSessionProperty::operator== (const BSessionProperty& rhs) const{
#ifdef INTEL_BASED
   return (!strcmpi(gsKey, rhs.gsKey));
#else
   return (!strcasecmp(gsKey, rhs.gsKey));
#endif
}

bool  BSessionProperty::operator!= (const BSessionProperty& rhs) const{
   return !(*this == rhs);
}

bool  BSessionProperty::operator> (const BSessionProperty& rhs) const{
#ifdef INTEL_BASED
   return (strcmpi(gsKey, rhs.gsKey) > 0);
#else
   return (strcasecmp(gsKey, rhs.gsKey) > 0);
#endif
}

bool  BSessionProperty::operator>= (const BSessionProperty& rhs) const{
#ifdef INTEL_BASED
   return !(strcmpi(gsKey, rhs.gsKey) < 0 );
#else
   return !(strcasecmp(gsKey, rhs.gsKey) < 0 );
#endif
}

bool  BSessionProperty::operator< (const BSessionProperty& rhs) const{
#ifdef INTEL_BASED
   return (strcmpi(gsKey, rhs.gsKey) < 0);
#else
   return (strcasecmp(gsKey, rhs.gsKey) < 0);
#endif
}

bool  BSessionProperty::operator<= (const BSessionProperty& rhs) const{
#ifdef INTEL_BASED
   return !(strcmpi(gsKey, rhs.gsKey) > 0);
#else
   return !(strcasecmp(gsKey, rhs.gsKey) > 0);
#endif
}

int BSessionProperty::CompareLike( const BSessionProperty &rhs) const   {
#ifdef INTEL_BASED
   return strncmpi(this->GetKey(), rhs.GetKey(), strlen(rhs.GetKey()));
#else
   return strncasecmp(this->GetKey(), rhs.GetKey(), strlen(rhs.GetKey()));
#endif
}

void  BSessionProperty::Copy(const BSessionProperty& rhs){
   try{
      SetKey(rhs.GetKey());
      SetValue(rhs.GetValue());
   }
   catch(ZdException &x){
      x.AddCallpath("Copy()", "BSessionProperty");
      throw;
   }
}

//Gets the boolean representation of the value of this property.
//If the value is not a valid boolean, the default value is returned
bool BSessionProperty::GetBool(bool bDefault)
{
   char *sReturn=0;
   bool bReturn = bDefault;

   try{
      ZdCopyString(&sReturn, gsValue);
      if (sReturn[0])
         {
         ZdDeblank(sReturn);
         bReturn = (!stricmp(sReturn, gsBooleanSearchValue.GetCString()));
      }
      delete [] sReturn;
   }
   catch (ZdException & x)
      {
      delete [] sReturn;
      x.AddCallpath("GetBool()", "BSessionProperty");
      throw;
      }

   return bReturn;
}

//Gets the integer representation of the value of this property.
//If the value is not a valid integer, the default value is returned
int BSessionProperty::GetInt(int iDefault)
{
   char *sReturn=0;
   int  iReturn = iDefault;

   try
      {
      ZdCopyString(&sReturn, gsValue);
      if (sReturn[0])
         {
         ZdDeblank(sReturn);
         if (strspn(sReturn, "-0123456789") == strlen(sReturn))
            iReturn = atoi(sReturn);
         else if ( sReturn[0] == '0' && sReturn[1] == 'x' )
            sscanf ( sReturn, "%x", &iReturn );
         }
      delete [] sReturn;
      }
   catch (ZdException & x)
      {
      delete [] sReturn;
      x.AddCallpath("GetInt()", "BSessionProperty");
      throw;
      }

   return iReturn;
}

const char*  BSessionProperty::GetKey() const{
   return gsKey;
}

const char*  BSessionProperty::GetValue() const{
   return gsValue;
}

void BSessionProperty::Init(){
   InitializeBooleanSearchValue();
}

void BSessionProperty::InitializeBooleanSearchValue(){
   gsBooleanSearchValue = BOOLEAN_SEARCH_VALUE;
}


bool BSessionProperty::IsEqual ( const BSessionProperty &rhs ) const  {
   return *this == rhs;
}

bool BSessionProperty::IsLess ( const BSessionProperty &rhs ) const   {
   return *this < rhs;
}


bool BSessionProperty::IsLike( const BSessionProperty &rhs, int iLength ) const   {
#ifdef INTEL_BASED
   return (strncmpi(this->GetKey(), rhs.GetKey(), iLength) < 0);
#else
   return (strncasecmp(this->GetKey(), rhs.GetKey(), iLength) < 0);
#endif
}

// Function to read the BSessionProperty from a stream
void BSessionProperty::Read( ZdInputStreamInterface &theStream ){
   try {
      gsKey.Read(theStream);
      gsValue.Read(theStream);
   }
   catch (ZdException &x) {
      x.AddCallpath("Read", "BSessionProperty");
      throw;
   }
}


void  BSessionProperty::Set(const char * sKey, const char * sValue){
   try{
      SetKey(sKey);
      SetValue(sValue);
   }
   catch(ZdException &x){
      x.AddCallpath("Set()","BSessionProperty");
      throw;
   }
}

void BSessionProperty::SetBool(bool bValue)
{
   try{
      if (bValue)
         SetValue("true");
      else
         SetValue("false");
   }
   catch (ZdException & x){
      x.AddCallpath("SetBool()", "BSessionProperty");
      throw;
   }
}

void BSessionProperty::SetBooleanSearchValue(const char *sBooleanSearchValue){
   gsBooleanSearchValue = sBooleanSearchValue;
}

void BSessionProperty::SetInt(int iNumber){
   char sEntry[24];

   try{
      sprintf(sEntry, "%d", iNumber);
      SetValue(sEntry);
   }
   catch (ZdException & x){
      x.AddCallpath("SetInt()", "BSessionProperty");
      throw;
   }
}

void  BSessionProperty::SetKey(const char * sKey){
   try{
      if (!sKey)
         BSessionException::GenerateSessionException("null ptr", "SetKey()");
      gsKey = sKey;
   }
   catch(ZdException &x){
      x.AddCallpath("SetKey()","BSessionProperty");
      throw;
   }
}

void  BSessionProperty::SetValue(const char * sValue){
   try{
      if (!sValue)
         BSessionException::GenerateSessionException("null ptr", "SetValue()");
      gsValue = sValue;
   }
   catch(ZdException &x){
      x.AddCallpath("SetValue()","BSessionProperty");
      throw;
   }
}

// Function to write the BSessionProperty to a stream
void BSessionProperty::Write( ZdOutputStreamInterface &theStream ) const {
   try {
      gsKey.Write(theStream);
      gsValue.Write(theStream);
   }
   catch (ZdException &x) {
      x.AddCallpath("Write", "BSessionProperty");
      throw;
   }
}

//------------------------------------------------------------------------------
//ClassDesc Begin BBaseSession
//Basis Session class.
//This class manages a list of BSessionProperty Objects via a ZdSortedVector
// A Key in this class has a format different from a typical .ini file
//This class saves basis session properties in a file for use at a later time
//BaseSession class contains a sorted array of pointers to properties
//ClassDesc End BBaseSession


// unless MAINTAIN_PROPERTY is set,  AddProperty will overwrite other properties
// in the array with the same key
const unsigned long BBaseSession::MAINTAIN_PROPERTY = 1;
// default behavoir of function when MAINTAIN_LIST is not set then the properties
// array will be deleted when properties are read from a file
const unsigned long BBaseSession::MAINTAIN_LIST = 2;

BBaseSession::BBaseSession()
             : gvProperties(0,
                            0,
                            0,
                            BPredicateMtblPtr<BSessionProperty, bool>(&BSessionProperty::IsLess),
                            BPredicateMtblPtr<BSessionProperty, bool>(&BSessionProperty::IsEqual) ) {
   try{
      Setup();
   }
   catch(ZdException &x){
      x.AddCallpath("BBaseSession()","BBaseSession");
      throw;
   }
}

BBaseSession::BBaseSession(BBaseSession &rhs)
             : gvProperties(0,
                            0,
                            0,
                            BPredicateMtblPtr<BSessionProperty, bool>(rhs.gvProperties.GetOrderingPredicate()),
                            BPredicateMtblPtr<BSessionProperty, bool>(rhs.gvProperties.GetEqualityPredicate()) ) {
   BSessionProperty theProperty;
   long             i;

   try{
      Setup();
      for (i = 0; i < rhs.GetNumProperties(); i++){
         theProperty = *(rhs.GetProperty(i));
         AddProperty(&theProperty);
      }
   }
   catch(ZdException &x){
      x.AddCallpath("BBaseSession(BBaseSession&)","BBaseSession");
      throw;
   }
}

BBaseSession::BBaseSession(ZdInputStreamInterface &theStream)
             : gvProperties(0,
                            0,
                            0,
                            BPredicateMtblPtr<BSessionProperty, bool>(&BSessionProperty::IsLess),
                            BPredicateMtblPtr<BSessionProperty, bool>(&BSessionProperty::IsEqual) ) {
   try{
      Setup();
      Read(theStream);
   }
   catch(ZdException &x){
      x.AddCallpath("BBaseSession(ZdInputStreamInterface&)","BBaseSession");
      throw;
   }
}

BBaseSession::~BBaseSession(){
   try{
      RemoveAllProperties();
   }
   catch(...){}
}

// unless MAINTAIN_PROPERTY is set,  AddProperty will overwrite other properties
// in the array with the same key
void  BBaseSession::AddProperty(const BSessionProperty* pProperty, unsigned long lFlags ){
   BSessionProperty*   pProp = 0;
   long lPos;

   try{
      if (!pProperty)
         BSessionException::GenerateSessionException("null ptr", "AddProperty");
      lPos = BBaseSession::FindProperty(pProperty->GetKey());
      if (lPos >= 0){
         if (!(lFlags & MAINTAIN_PROPERTY)){
            BBaseSession::DeleteProperty(lPos);    //delete and insert in case session class members change someday
            pProp = new BSessionProperty(*pProperty);
            gvProperties.InsertElement(&pProp);
         }
      }
      else{
         pProp = new BSessionProperty(*pProperty);
         gvProperties.InsertElement(&pProp);
      }
   }
   catch(ZdException &x){
      x.AddCallpath("AddProperty()","BBaseSession");
      throw;
   }
}

void  BBaseSession::AddProperty(const char * sKey, const char * sValue, unsigned long lFlags ){
   BSessionProperty theProperty;

   try{
      theProperty.SetKey(sKey);
      if (sValue)
         theProperty.SetValue(sValue);
      BBaseSession::AddProperty(&theProperty, lFlags);
   }
   catch (ZdException &x){
      x.AddCallpath("AddProperty()","BSessionProperty");
      throw;
   }
}

void  BBaseSession::AddProperty(const char * sKey, long lValue, unsigned long lFlags ){
   BSessionProperty theProperty;

   try{
      theProperty.SetKey(sKey);
      theProperty.SetInt(lValue);
      BBaseSession::AddProperty(&theProperty, lFlags);
   }
   catch (ZdException &x){
      x.AddCallpath("AddProperty()","BSessionProperty");
      throw;
   }
}

void  BBaseSession::AddProperty(const char * sKey, bool bValue, unsigned long lFlags ){
   BSessionProperty theProperty;

   try{
      theProperty.SetKey(sKey);
      theProperty.SetBool(bValue);
      BBaseSession::AddProperty(&theProperty, lFlags);
   }
   catch (ZdException &x){
      x.AddCallpath("AddProperty()","BSessionProperty");
      throw;
   }
}

void  BBaseSession::DeleteProperty(long lIndex){
   BSessionProperty* pProperty;

   try{
      pProperty = gvProperties.at(lIndex);
      gvProperties.RemoveElement(lIndex);
      delete pProperty;
   }
   catch(ZdException &x){
      x.AddCallpath("DeleteProperty()","BBaseSession");
      throw;
   }
}

void  BBaseSession::DeleteProperty(const char *sKey){
   long   lPos;

   try{
      lPos = BBaseSession::FindProperty(sKey);
      if (lPos >= 0)
         BBaseSession::DeleteProperty(lPos);
   }
   catch(ZdException &x){
      x.AddCallpath("DeleteProperty()","BBaseSession");
      throw;
   }
}

void  BBaseSession::DeletePropertiesLike(const char *sKey){
   long   lStart, lEnd;

   try{
      BBaseSession::GetRangeOfPropertiesLike(sKey, &lStart, &lEnd);
      if (lStart != -1){  
         while (lEnd >= lStart)
            BBaseSession::DeleteProperty(lEnd--);
      }
   }
   catch(ZdException &x){
      x.AddCallpath("DeletePropertiesLike()","BBaseSession");
      throw;
   }
}

//Debugging function that will dump the contents of the session to the specified file.
//Please note that if the file already exists, its contents will be destroyed.
void BBaseSession::Dump(const char* sDumpFilename){
   long     i;
   ZdIO     theFile;
   ZdString sLine;

   try{
      theFile.Open(sDumpFilename,ZDIO_OPEN_CREATE|ZDIO_OPEN_WRITE|ZDIO_OPEN_TRUNC);
      theFile.WriteLine("Results of BBaseSession::Dump()");
      sLine << ZdString::reset << "Number of properties: " << GetNumProperties()<< "\r\n"; 
      theFile.WriteLine(sLine);
      for (i=0; i < GetNumProperties(); i++){
        sLine << ZdString::reset << BBaseSession::GetProperty(i)->GetKey() << "="
              << BBaseSession::GetProperty(i)->GetValue();
        theFile.WriteLine(sLine);
      }
      theFile.Close();
   }
   catch(ZdException &x){
      x.AddCallpath("Dump()","BBaseSession");
      throw;
   }
}
 
//  returns -1 if not found
long  BBaseSession::FindProperty(const char *sKey){
   bool bFound;
   BSessionProperty   theSessionProp, *p;
   long lPos;

   try{
      theSessionProp.SetKey(sKey);
      p = &theSessionProp;
      lPos = gvProperties.FindPosition(&p,&bFound);
   }
   catch(ZdException &x){
      x.AddCallpath("FindProperty()","BBaseSession");
      throw;
   }
   return bFound ? lPos : -1;
}

long  BBaseSession::GetNumProperties() const{
   return gvProperties.GetNumElements();
}

BSessionProperty*  BBaseSession::GetProperty(const char* sKey){
   BSessionProperty*   pProperty;
   long                lPos;

   try{
      lPos = BBaseSession::FindProperty(sKey);
      if (lPos == -1)
         BSessionException::GenerateSessionException("Key:\"%s\" not found", "GetProperty()", ZdException::Notify, sKey);
      pProperty = gvProperties.at(lPos);
   }
   catch(ZdException &x){
      x.AddCallpath("GetProperty()","BBaseSession");
      throw;
   }
   return pProperty;
}

BSessionProperty*  BBaseSession::GetProperty(long lPosition) {
   BSessionProperty*   pProperty;

   try{
      pProperty = gvProperties.at(lPosition);
   }
   catch(ZdException &x){
      x.AddCallpath("GetProperty()","BBaseSession");
      throw;
   }
   return pProperty;
}

//Start and end values are set to -1 if sKey is not found.
void  BBaseSession::GetRangeOfPropertiesLike(const char *sKey, long *plStart, long *plEnd){
   int                iKeyLength;
   BSessionProperty   theSessionProp;
   iterator           itrUpper, itrLower;

   try{
      if (!plStart || !plEnd || !sKey)
         BSessionException::GenerateSessionException("null ptr", "GetRangeOfPropertiesLike");
      iKeyLength = strlen(sKey);
      theSessionProp.SetKey(sKey);
      itrLower  = std::lower_bound ( gvProperties.begin(),
                                     gvProperties.end(),
                                     &theSessionProp,
                                     BSessionPropertyPredicateLike(iKeyLength) );
#ifdef INTEL_BASED
      if (itrLower == gvProperties.end() || strncmpi((*itrLower)->GetKey(), sKey, iKeyLength)){ //not found
         *plStart = -1;
         *plEnd = -1;
      }
#else
      if (itrLower == gvProperties.end() || strncasecmp((*itrLower)->GetKey(), sKey, iKeyLength)){ //not found
         *plStart = -1;
         *plEnd = -1;
      }
#endif
      else{
         itrUpper = std::upper_bound ( gvProperties.begin(),
                                       gvProperties.end(),
                                       &theSessionProp,
                                       BSessionPropertyPredicateLike(iKeyLength) );
         *plStart = itrLower - gvProperties.begin();
         *plEnd = itrUpper - gvProperties.begin() - 1;
      }

   }
   catch(ZdException &x){
      x.AddCallpath("GetRangeOfPropertiesLike()", "BBaseSession");
      throw;
   }
}

// Function to read the BBaseSession from a stream
void BBaseSession::Read( ZdInputStreamInterface &theStream ){
   BSessionProperty   theProperty;
   int                i, iNumToRead;

   try {
      iNumToRead = theStream.ReadInt();
      for (i = 0; i < iNumToRead; i++){
         theProperty.Read(theStream);
         AddProperty(&theProperty);
      }
   }
   catch (ZdException &x) {
      x.AddCallpath("Read", "BBaseSession");
      throw;
   }
}

void  BBaseSession::RemoveAllProperties(){
   BSessionProperty* pProperty;

   try{
      while (GetNumProperties()){
        pProperty = gvProperties.at(0);
        gvProperties.RemoveElement(0);
        delete pProperty;
      }
   }
   catch(ZdException &x){
      x.AddCallpath("RemoveAllProperties()","BBaseSession");
      throw;
   }
}

//Note: Deprecated!!
//Forces the vector to resort.  This should not ever be necessary.
//If you are using this function, it is likely that your vector is
//not being properly maintained.
void BBaseSession::ReSortArray(){
   try{
      gvProperties.SetOrderingPredicate(gvProperties.GetOrderingPredicate());
   }
   catch(ZdException &x){
      x.AddCallpath("ReSortArray()","BBaseSession");
      throw;
   }
}


void  BBaseSession::Setup(){

   try{
      //gvProperties.SetOrderingPredicate ( BPredicateMtblPtr<BSessionProperty, bool> ( &BSessionProperty::IsLess ) );
   }
   catch(ZdException &x){
      x.AddCallpath("Setup()","BBaseSession");
      throw;
   }

}

// Function to write the BBaseSession to a stream
void BBaseSession::Write( ZdOutputStreamInterface &theStream ) {
   long   i;

   try {
      theStream.WriteInt(GetNumProperties());
      for (i = 0; i < GetNumProperties(); i++)
         GetProperty(i)->Write(theStream);
   }
   catch (ZdException &x) {
      x.AddCallpath("Write()", "BBaseSession");
      throw;
   }
}

BZdIniSessionException::BZdIniSessionException ( const char * sMessage, const char * sSourceModule, ZdException::Level iLevel )
                  :ZdException(sMessage, sSourceModule, iLevel){
}

void  BZdIniSessionException::GenerateZdIniSessionException ( const char * sMessage, const char * sSourceModule, ZdException::Level iLevel, ... ){
   va_list   vArgs;
   char      sExceptionString[2048];

   va_start ( vArgs, iLevel );
   vsprintf ( sExceptionString, sMessage, vArgs );
   va_end ( vArgs );

   throw BZdIniSessionException(sExceptionString, sSourceModule, iLevel);
}

void  BZdIniSessionException::GenerateZdIniSessionException ( const char * sMessage, const char * sSourceModule ){
   throw BZdIniSessionException(sMessage, sSourceModule, ZdException::Normal);
}


//------------------------------------------------------------------------------
//ClassDesc Begin BZdIniSession
// To use this class you must define __USE_xbZdIniSession
// This class enhances the functionality of the BBaseSession class by the addition of
// the ability to Read and Write the BSessionProperty objects in the parent class's ZdSortedVector
// to a ZdIniFile. Because the BZdIniSession utilizes a ZdIniFile to store the BSessionProperty 
// objects to disk and because there is no separate SectionName in a SessionProperty object for 
// the ZdIniFile section name, each BSessionProperty::Key must includes both a SectionName and 
// a Key; therefore, a BSessionPropert::Key utilized by a BZdIniSession object must be constructed 
// in the form of [SectionName].Key.
// 
// The BZdIniSession class also provides for the use of implied ZdIniFile section names.  Whenever
// an implied section name is in use, the section name is not explicitly prepended to the key prior
// to utilizing the BZdIniSession interface.

//ClassDesc End BZdIniSession

const char* const BZdIniSession::XBZDINISESSION_KEY_DELIMITER = ".";
const char* const BZdIniSession::XBZDINISESSION_DEFAULT_IMPLIED_SECTION = "[__BZDINISESSION_SECTION]";

BZdIniSession::BZdIniSession():BBaseSession(){
   try{
      Init();
   }
   catch(ZdException &x){
      x.AddCallpath("BZdIniSession()","BZdIniSession");
      throw;
   }
}

BZdIniSession::BZdIniSession(ZdInputStreamInterface &theStream):BBaseSession(){
   try{
      Init();
      Read(theStream);
   }
   catch(ZdException &x){
      x.AddCallpath("BZdIniSession(ZdInputStreamInterface)","BZdIniSession");
      throw;
   }
}


BZdIniSession::~BZdIniSession(){}

void BZdIniSession::AddProperty(const BSessionProperty* pProperty, unsigned long lFlags){
   ZdString            sTempKey;
   BSessionProperty   *pNewProperty;
   
   try{ 
     if (!pProperty)
        BZdIniSessionException::GenerateZdIniSessionException("Null pointer." , "AddProperty");
     if (gbUseImpliedSection){ 
        sTempKey << gsImpliedSection << XBZDINISESSION_KEY_DELIMITER << pProperty->GetKey();
        pNewProperty = new BSessionProperty();
        pNewProperty->SetValue(pProperty->GetValue());
        pNewProperty->SetKey(sTempKey);
        BBaseSession::AddProperty(pNewProperty, lFlags);
     }
     else
        BBaseSession::AddProperty(pProperty, lFlags);
   }      
   catch(ZdException &x){
      x.AddCallpath("AddProperty()","BZdIniSession");
      throw;
   }
}

void BZdIniSession::AddProperty(const char * sKey, const char * sValue, unsigned long lFlags){
   ZdString sTempKey;
   
   try{
      if (gbUseImpliedSection){                                      
         sTempKey << gsImpliedSection << XBZDINISESSION_KEY_DELIMITER << sKey;
         BBaseSession::AddProperty(sTempKey, sValue , lFlags);
      }
      else
         BBaseSession::AddProperty(sKey, sValue, lFlags);
   }      
   catch(ZdException &x){
      x.AddCallpath("AddProperty()","BZdIniSession");
      throw;
   }
}


void BZdIniSession::AddProperty(const char * sKey, long lValue, unsigned long lFlags){
   ZdString sTempKey;
   
   try{
      if (gbUseImpliedSection){                                      
         sTempKey << gsImpliedSection << XBZDINISESSION_KEY_DELIMITER << sKey;
         BBaseSession::AddProperty(sTempKey, lValue , lFlags);
      }
      else
         BBaseSession::AddProperty(sKey, lValue, lFlags);
   }      
   catch(ZdException &x){
      x.AddCallpath("AddProperty()","BZdIniSession");
      throw;
   }
}


void BZdIniSession::AddProperty(const char * sKey, bool bValue, unsigned long lFlags){
   ZdString sTempKey;
   
   try{
      if (gbUseImpliedSection){                                      
         sTempKey << gsImpliedSection << XBZDINISESSION_KEY_DELIMITER << sKey;
         BBaseSession::AddProperty(sTempKey, bValue , lFlags);
      }
      else
         BBaseSession::AddProperty(sKey, bValue, lFlags);
   }      
   catch(ZdException &x){
      x.AddCallpath("AddProperty()","BZdIniSession");
      throw;
   }
}

//Empties the session
void BZdIniSession::Clear(){
   try{
      SetImpliedSection("");
      UseImpliedSection(false);
      RemoveAllProperties();
   }
   catch(ZdException &x){
      x.AddCallpath("Clear()","BZdIniSession");
      throw;
   }
}

ZdString  BZdIniSession::CreateSessionKey(const char* sSection, const char* sZdIniFileKey){
   ZdString   sSessionKey;

   try{
      if (!sZdIniFileKey || !sSection)
         BZdIniSessionException::GenerateZdIniSessionException("Null pointer." , "CreateSessionKey");
      sSessionKey << sSection << XBZDINISESSION_KEY_DELIMITER << sZdIniFileKey;
   }
   catch(ZdException &x){
      x.AddCallpath("CreateSessionKey()","BZdIniSession");
      throw;
   }
   return sSessionKey;
}


void BZdIniSession::CreateZdIniFile(const char *sFileName , ZdIOFlag iOpenFlags , ZdIOFlag iLockFlags, long lNumTries, unsigned short wDelayInMS){
   long   i ;
   bool   bLocked = false;
   bool   bCreateFile = true, bLoadSections = true;

   try{
      for (i = 0; i < lNumTries && !bLocked; i++){
         if (!gpIniFile){
            try{
               gpIniFile = new ZdIniFile(sFileName, bLoadSections , bCreateFile, iOpenFlags);
            }
            catch(ZdFileOpenFailedException& x){}
         }
         if (gpIniFile)
            if (iLockFlags != 0)
               bLocked = gpIniFile->TryLock(iLockFlags);
            else
               bLocked = true;
#ifdef INTEL_BASED
         if (!gpIniFile || !bLocked)
            Sleep(wDelayInMS);
#endif
      }
      if (!bLocked)
         BZdIniSessionException::GenerateZdIniSessionException("Failed to open file with specified locks", "OpenIniFile()");
   }
   catch(ZdException &x){
      if (gpIniFile){
         delete gpIniFile;
         gpIniFile = 0;
      }
      x.AddCallpath("CreateZdIniFile()", "BZdIniSession");
      throw;
   }
}

void BZdIniSession::DeleteProperty(const char *sKey){
   ZdString sTempKey;
   
   try{
      if (gbUseImpliedSection){                                      
         sTempKey << gsImpliedSection << XBZDINISESSION_KEY_DELIMITER << sKey;
         BBaseSession::DeleteProperty(sTempKey);
      }
      else
         BBaseSession::DeleteProperty(sKey);
   }      
   catch(ZdException &x){
      x.AddCallpath("DeleteProperty()","BZdIniSession");
      throw;
   }
}

void  BZdIniSession::DeleteProperty(long lIndex){
   try{
      BBaseSession::DeleteProperty(lIndex);
   }      
   catch(ZdException &x){
      x.AddCallpath("DeleteProperty()","BZdIniSession");
      throw;
   }
}

void  BZdIniSession::DeletePropertiesLike(const char *sKey){
   ZdString   sTempKey;

   try{
      if (gbUseImpliedSection){                                      
         sTempKey << gsImpliedSection << XBZDINISESSION_KEY_DELIMITER << sKey;
         BBaseSession::DeletePropertiesLike(sTempKey);
      }
      else
         BBaseSession::DeletePropertiesLike(sKey);
   }
   catch(ZdException &x){
      x.AddCallpath("DeletePropertiesLike()","BZdIniSession");
      throw;
   }
}

void BZdIniSession::DestroyZdIniFileObject(ZdIOFlag iLockFlags){
   try{
      if (gpIniFile)
         gpIniFile->Unlock(iLockFlags);
      delete gpIniFile;
      gpIniFile = 0;
   }
   catch(ZdException &x){
      delete gpIniFile;
      gpIniFile = 0;
      x.AddCallpath("DestroyZdIniFileObject()", "BZdIniSession");
      throw;
   }
}

long BZdIniSession::FindProperty(const char *sKey){
   long     lPosition;
   ZdString sTempKey;
   
   try{
      if (gbUseImpliedSection){                                      
         sTempKey << gsImpliedSection << XBZDINISESSION_KEY_DELIMITER << sKey;
         lPosition = BBaseSession::FindProperty(sTempKey);
      }
      else
         lPosition = BBaseSession::FindProperty(sKey);
   }      
   catch(ZdException &x){
      x.AddCallpath("FindProperty()","BZdIniSession");
      throw;
   }
   return lPosition;
}
   
BSessionProperty* BZdIniSession::GetProperty(const char* sKey){
   BSessionProperty* pProperty;
   ZdString sTempKey;
   
   try{
      if (gbUseImpliedSection){                                      
         sTempKey << gsImpliedSection << XBZDINISESSION_KEY_DELIMITER << sKey;
         pProperty = BBaseSession::GetProperty(sTempKey);
      }
      else
         pProperty = BBaseSession::GetProperty(sKey);
   }      
   catch(ZdException &x){
      x.AddCallpath("GetProperty()","BZdIniSession");
      throw;
   }
   return pProperty;
}

BSessionProperty* BZdIniSession::GetProperty(long lPosition) {
   BSessionProperty* pProperty;
   ZdString sSection, sKey;
   
   try{
      pProperty = BBaseSession::GetProperty(lPosition);
      if (gbUseImpliedSection){                                      
         ParseSessionKey(pProperty->GetKey(), sSection, sKey);
         if (sSection != gsImpliedSection)
            BZdIniSessionException::
             GenerateZdIniSessionException("Missmatch between implied (%s) and found (%s) section names at position %l.",
                                           "GetProperty()", ZdException::Normal, gsImpliedSection.GetCString(), 
                                           sSection.GetCString(), lPosition);
      }
   }      
   catch(ZdException &x){
      x.AddCallpath("GetProperty()","BZdIniSession");
      throw;
   }
   return pProperty;
}


//Returns the key delimiter.
const char* BZdIniSession::GetKeyDelimiter() const {
   return XBZDINISESSION_KEY_DELIMITER;
}

//Returns the current Implied-section string.
const char* BZdIniSession::GetImpliedSection() const {
   return gsImpliedSection;
}

//Determines the starting and ending positions of the properties in the BBaseSessions sorted array with
//key names beginning with sKey.  Returns -1 in both positions if key is not found.
void  BZdIniSession::GetRangeOfPropertiesLike(const char *sKey, long *plStart, long *plEnd){
   ZdString sTempKey;
   
   try{
      if (gbUseImpliedSection){                                      
         sTempKey << gsImpliedSection << XBZDINISESSION_KEY_DELIMITER << sKey;
         BBaseSession::GetRangeOfPropertiesLike(sTempKey, plStart, plEnd);
      }      
      else
         BBaseSession::GetRangeOfPropertiesLike(sKey, plStart, plEnd);
   }
   catch(ZdException &x){
      x.AddCallpath("GetRangeOfPropertiesLike()", "BZdIniSession");
      throw;
   }
}


void BZdIniSession::Init(){
   gpIniFile = 0;
   gbUseImpliedSection = false;
}


//Returns a boolean value indicating whether or not Implied sections are being utilized.
bool BZdIniSession::IsUsingImpliedSection() const{
   return gbUseImpliedSection;
}

bool BZdIniSession::IsValidKey(const char* sSessionKey){
   bool       bValidKey = false;
   ZdStringTokenizer theTokenizer("", XBZDINISESSION_KEY_DELIMITER);

   try{
      theTokenizer.SetString(sSessionKey);
      bValidKey = IsValidKey(theTokenizer);
   }
   catch(ZdException &x){
      x.AddCallpath("IsValidKey()", "BZdIniSession");
      throw;
   }
   return bValidKey;
}

bool BZdIniSession::IsValidKey(ZdStringTokenizer& theTokenizer){
   bool     bValidKey = true;
   ZdString sSessionKey;

   try{
      sSessionKey = theTokenizer.GetString();
      if (
          (theTokenizer.GetNumTokens() != 2) || (sSessionKey.Find(XBZDINISESSION_KEY_DELIMITER) == 0) || 
          (sSessionKey.Find(XBZDINISESSION_KEY_DELIMITER) == ((signed long)sSessionKey.GetLength() - 1) ) 
         )
         bValidKey = false;
   }
   catch(ZdException &x){
      x.AddCallpath("IsValidKey()", "BZdIniSession");
      throw;
   }
   return bValidKey;
}

void BZdIniSession::ParseSessionKey(const char* sSessionKey, ZdString& sSection, ZdString& sZdIniFileKey){
   ZdStringTokenizer theTokenizer(sSessionKey, XBZDINISESSION_KEY_DELIMITER);

   try{
      TestKey(theTokenizer);
      sSection = theTokenizer.GetNextToken();
      sZdIniFileKey = theTokenizer.GetNextToken();
   }
   catch (ZdException &x){
      x.AddCallpath("ParseSessionKey()","BZdIniSession");
      throw;
   }
}


// Function to read the BZdIniSession from a stream
void BZdIniSession::Read( ZdInputStreamInterface &theStream ){
   try {
      gbUseImpliedSection = theStream.ReadBool();
      gsImpliedSection.Read(theStream);
      BBaseSession::Read(theStream);
   }
   catch (ZdException &x) {
      x.AddCallpath("Read(ZdOutputStreamInterface)", "BBaseSession");
      throw;
   }
}


// default behavoir of function when MAINTAIN_LIST is not set then the properties
// array will be deleted
void   BZdIniSession::Read(const char* sFileName, ZdIOFlag iOpenFlags, ZdIOFlag iLockFlags, long lNumTries, unsigned short wDelayInMS, unsigned long lFlags){
   bool               bSavedUseImpliedState;
   BSessionProperty   theProperty ;
   ZdIniSection*      pIniSection;
   ZdString           sSection, sZdIniFileKey, sValue, sKey;
   long               i,j;

   try{
      CreateZdIniFile(sFileName, iOpenFlags, iLockFlags, lNumTries, wDelayInMS);
      if (!(lFlags & MAINTAIN_LIST))
         RemoveAllProperties();
      for (i=0; i < gpIniFile->GetNumSections(); i++){
         pIniSection = gpIniFile->GetSection(i);
         sSection = pIniSection->GetName();
         if (gbUseImpliedSection)
            SetImpliedSection(sSection);
         for (j=0; j < pIniSection->GetNumLines(); j++){
            sZdIniFileKey = pIniSection->GetLine(j)->GetKey();
            sValue = pIniSection->GetLine(j)->GetValue();
            theProperty.SetKey(CreateSessionKey(sSection, sZdIniFileKey));
            theProperty.SetValue(sValue);
            //Regardless of how gbUseImpliedSection is set, AddProperty should be called with
            //gbUseImpliedSection equal to false during the read because the section will already
            //be present within the key because of the call to CreateSessionkey.  --Jim
            bSavedUseImpliedState = gbUseImpliedSection;
            gbUseImpliedSection = false;
            AddProperty(&theProperty, lFlags);
            gbUseImpliedSection = bSavedUseImpliedState;
         }
      }
      DestroyZdIniFileObject(iLockFlags);
   }
   catch(ZdException &x){
      try{
         DestroyZdIniFileObject(iLockFlags);
      }
      catch(...){}
      x.AddCallpath("Read()","BZdIniSession");
      throw;
   }
}


//Sets the Implied-section string.
void BZdIniSession::SetImpliedSection(const char* sImpliedSection){
   try{
      if (!sImpliedSection)
         BZdIniSessionException::GenerateZdIniSessionException("Null pointer." , "SetImpliedSection");
      gsImpliedSection = sImpliedSection;
   }
   catch(ZdException &x){
      x.AddCallpath("SetImpliedSection()","BZdIniSession");
      throw;
   }
}

void BZdIniSession::TestKey(const char* sSessionKey){
   ZdStringTokenizer theTokenizer(sSessionKey, XBZDINISESSION_KEY_DELIMITER);

   try{
      TestKey(theTokenizer);
   }
   catch(ZdException &x){
      x.AddCallpath("TestKey", "BZdIniSession");
      throw;
   }
}

void BZdIniSession::TestKey(ZdStringTokenizer& theTokenizer){
   ZdString sTemp;

   try{
      if (!IsValidKey(theTokenizer)){
         sTemp = "Malformed SessionKey:\"%s\" "; 
         if (gbUseImpliedSection)
            sTemp << " (Note: Default section in use)";
         BZdIniSessionException::GenerateZdIniSessionException(sTemp, "TestKey", ZdException::Normal, theTokenizer.GetString());
      }
   }
   catch(ZdException &x){
      x.AddCallpath("TestKey", "BZdIniSession");
      throw;
   }
}

//Causes the class to use either Implied or Explicit sections as directed.
void BZdIniSession::UseImpliedSection(bool bUseImpliedSection){ 
   gbUseImpliedSection = bUseImpliedSection;
}


// Function to write the BZdIniSession to a stream
void BZdIniSession::Write( ZdOutputStreamInterface &theStream ) {
   try {
      theStream.WriteBool(gbUseImpliedSection);
      gsImpliedSection.Write(theStream);
      BBaseSession::Write(theStream);
   }
   catch (ZdException &x) {
      x.AddCallpath("Write(ZdOutputStreamInterface)", "BZdIniSession");
      throw;
   }
}


// the default behavoir is to clear the file before writing to it
// if you want to save the info. in the file then you need to read it and MAINTAIN_LIST
// to add it to your array of properties
void   BZdIniSession::Write(const char* sFileName, ZdIOFlag iOpenFlags, ZdIOFlag iLockFlags, long lNumTries, unsigned short wDelayInMS, bool bReSortArray){
   bool                bSavedUseImpliedState;
   ZdIniSection*       pIniSection = 0;
   BSessionProperty*   pProperty;
   ZdString            sSectName, sZdIniFileKey, sValue, sSessionKey, sWritingSection;
   long                i;

   try{
      bSavedUseImpliedState = gbUseImpliedSection;
      if (bReSortArray)
         ReSortArray();
      CreateZdIniFile(sFileName, iOpenFlags, iLockFlags, lNumTries, wDelayInMS);
      while (gpIniFile->GetNumSections() > 0)
         gpIniFile->DeleteSection(static_cast<long>(0));
      //copy all elements of property array into inifile vector
      for (i = 0; i < GetNumProperties(); i++){
         //Regardless of how gbUseImpliedSection is set, GetProperty and ParseSessionKey should be called with
         //gbUseImpliedSection equal to false during the Write because the section will always
         //be present within the key   --SPB
         gbUseImpliedSection = false;
         pProperty = GetProperty(i);
         ParseSessionKey(pProperty->GetKey(), sSectName, sZdIniFileKey);
         gbUseImpliedSection = bSavedUseImpliedState;
         sValue = pProperty->GetValue();
         if ((sSectName != sWritingSection) || !pIniSection)
            pIniSection = gpIniFile->GetSection(sSectName);
         pIniSection->AddLine(sZdIniFileKey, sValue);
         sWritingSection = sSectName;
      }
      gpIniFile->Write();
      DestroyZdIniFileObject(iLockFlags);
   }
   catch(ZdException &x){
      gbUseImpliedSection = bSavedUseImpliedState;
      try{
         DestroyZdIniFileObject(iLockFlags);
      }
      catch(...){}
      x.AddCallpath("Write()","BZdIniSession");
      throw;
   }
}

