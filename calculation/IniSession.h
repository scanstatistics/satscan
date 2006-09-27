//******************************************************************************
#ifndef IniSessionH
#define IniSessionH
//******************************************************************************
#include <functional>

template <typename TYPE, typename RESULT>
class BPredicateMutable : public std::binary_function<TYPE, TYPE, RESULT>
{
private:
   typedef RESULT (TYPE::*ptrfn_t) ( const TYPE & ) const;

   ptrfn_t  gpFunc;

public:
   inline BPredicateMutable ( ptrfn_t pFunc )
   {
      gpFunc = pFunc;
   }

   inline void SetFunction ( ptrfn_t pFunc )
   {
      gpFunc = pFunc;
   }

   inline RESULT operator() ( const TYPE &left, const TYPE &right ) const
   {
      return (left.*gpFunc)( right );
   }
};

template <typename TYPE, typename RESULT>
class BPredicateMtblPtr : public std::binary_function<TYPE *, TYPE *, RESULT>
{
private:
   typedef RESULT (TYPE::*ptrfn_t) ( const TYPE & ) const;

   ptrfn_t  gpFunc;

public:
   inline BPredicateMtblPtr ( ptrfn_t pFunc )
   {
      gpFunc = pFunc;
   }

   inline void SetFunction ( ptrfn_t pFunc )
   {
      gpFunc = pFunc;
   }

   inline RESULT operator() ( const TYPE *left, const TYPE *right ) const
   {
      return (left->*gpFunc)( *right );
   }
};

class BSessionException : public ZdException {
public:
   BSessionException ( const char * sMessage, const char * sSourceModule, ZdException::Level iLevel = Normal);
   virtual ~BSessionException(){};

   void static GenerateSessionException ( const char * sMessage, const char * sSourceModule, ZdException::Level iLevel, ... );
   void static GenerateSessionException ( const char * sMessage, const char * sSourceModule );
};

//------------------------------------------------------------------------------
//ClassDesc Begin BBaseSession
//To use this class you must define __USE_XBBASESESSION
//A Key in this class has a format different from a typical .ini file
//There is no separate SectionName in a SessionProperty, instead, each Key includes
//both a SectionName and a Key... so a SessionProperty Key takes the form of
//[SectionName].Key
//ClassDesc End BBaseSession

class BSessionProperty{
private:
   ZdString gsKey;
   ZdString gsValue;
   void Init();

protected:
   ZdString            gsBooleanSearchValue;

   virtual void        InitializeBooleanSearchValue();
   virtual void        SetBooleanSearchValue(const char * sBooleanSearchValue);


public:
   BSessionProperty();
   BSessionProperty(const char * sKey, const char * sValue = 0);
   BSessionProperty(const BSessionProperty& rhs);
   virtual     ~BSessionProperty();

   // operators
   BSessionProperty&    operator=  (const BSessionProperty& rhs);
   bool                 operator== (const BSessionProperty& rhs) const;
   bool                 operator!= (const BSessionProperty& rhs) const;
   bool                 operator> (const BSessionProperty& rhs) const;
   bool                 operator>= (const BSessionProperty& rhs) const;
   bool                 operator< (const BSessionProperty& rhs) const;
   bool                 operator<= (const BSessionProperty& rhs) const;

   static const char* BOOLEAN_SEARCH_VALUE;

   void              Copy(const BSessionProperty& rhs);
   bool              GetBool(bool bDefault = false);
   int               GetInt(int iDefault = 0);
   const char*       GetKey() const;
   const char*       GetValue() const;

   bool  IsEqual ( const BSessionProperty &rhs ) const;
   bool  IsLess ( const BSessionProperty &rhs ) const;
   bool   IsLike ( const BSessionProperty &rhs, int iLength) const;
   int   CompareLike ( const BSessionProperty &rhs) const;

   void              Read( ZdInputStreamInterface &theStream );
   void              Set(const char * sKey, const char * sValue);
   void              SetBool(bool bValue);
   void              SetInt(int iNumber);
   void              SetKey(const char * sKey);
   void              SetValue(const char * sValue);
   void              Write( ZdOutputStreamInterface &theStream ) const;
};

//Predicate utilized with the upper_bound and lower_bound STL functions to determine if properties
//exist in the session that match a given pattern.
class BSessionPropertyPredicateLike : public std::binary_function<BSessionProperty *, BSessionProperty *, bool>
{
private:
   int giLength;

public:
   inline BSessionPropertyPredicateLike ( int iAmount ) : giLength ( iAmount ) {}

   inline bool operator() ( const BSessionProperty *left, const BSessionProperty *right ) const
   {
     return left->IsLike ( *right, giLength );
   }
};

//------------------------------------------------------------------------------
//ClassDesc Begin BBaseSession
//Basis Session class.
//This class manages a list of BSessionProperty Objects via a ZdSortedVector
//This class saves basis session properties and if desired dumps them to a file for use at a later time
//BaseSession class contains a sorted array of pointers to properties
//ClassDesc End BBaseSession

typedef ZdSortedVector<BSessionProperty*,
                       ZdVector<BSessionProperty*>,
                       BPredicateMtblPtr<BSessionProperty, bool>,
                       BPredicateMtblPtr<BSessionProperty, bool> >  BaseSessionPropertiesType;



class BBaseSession{
private:

   BaseSessionPropertiesType     gvProperties;


public:
   typedef  BaseSessionPropertiesType::iterator               iterator;


   BBaseSession();
   BBaseSession(BBaseSession &rhs);
   BBaseSession(ZdInputStreamInterface &theStream);
   virtual 	~BBaseSession();

   static const unsigned long MAINTAIN_PROPERTY;
   static const unsigned long MAINTAIN_LIST;

   virtual void                 AddProperty(const BSessionProperty* pProperty, unsigned long lFlags = 0);
   virtual void                 AddProperty(const char * sKey, const char * sValue , unsigned long lFlags = 0);
   virtual void                 AddProperty(const char * sKey, long lValue , unsigned long lFlags = 0);
   virtual void                 AddProperty(const char * sKey, bool bValue , unsigned long lFlags = 0);
   virtual void                 DeleteProperty(long lIndex);
   virtual void                 DeleteProperty(const char *sKey);
   virtual void                 DeletePropertiesLike(const char *sKey);
   void                         Dump(const char* sDumpFilename);
   virtual long                 FindProperty(const char *sKey);
   virtual long                 GetNumProperties() const;
   virtual BSessionProperty*    GetProperty(const char* sKey);
   virtual BSessionProperty*    GetProperty(long lPosition);
   virtual void                 GetRangeOfPropertiesLike(const char *sKey, long *plStart, long *plEnd);
   void                         Read( ZdInputStreamInterface &theStream );
   virtual void                 RemoveAllProperties();
   void                         ReSortArray();  //Deprecated
   void                         Setup();
   void                         Write( ZdOutputStreamInterface &theStream );
};

class BZdIniSessionException : public ZdException {

public:
   BZdIniSessionException ( const char * sMessage, const char * sSourceModule, ZdException::Level iLevel = Normal);
   virtual ~BZdIniSessionException(){};

   void static GenerateZdIniSessionException ( const char * sMessage, const char * sSourceModule, ZdException::Level iLevel, ... );
   void static GenerateZdIniSessionException ( const char * sMessage, const char * sSourceModule );
};

//------------------------------------------------------------------------------
//ClassDesc Begin BZdIniSession
// To use this class you must define __USE_xbZdIniSession
// This class enhances the functionality of the BBaseSession class by the addition of
// the ability to Read and Write the BSessionProperty objects in the parent class's ZdSortedVector
// to a ZdIniFile. Because the BZdIniSession utilizes a ZdIniFile to store the BSessionProperty 
// objects to disk and because there is no separate SectionName in a SessionProperty object for 
// the ZdIniFile section name, each BSessionProperty::Key must includes both a SectionName and 
// a Key; therefore, a BSessionProperty::Key utilized by a BZdIniSession object must be constructed 
// in the form of [SectionName].Key.
//
// The BZdIniSession class also provides for the use of implied ZdIniFile section names.  Whenever
// an implied section name is in use, the section name is not explicitly prepended to the key prior
// to utilizing the BZdIniSession interface.

//ClassDesc End BZdIniSession

class BZdIniSession: public BBaseSession {

private:
   bool         gbUseImpliedSection;
   ZdString     gsImpliedSection;
   ZdIniFile*   gpIniFile;

   ZdString     CreateSessionKey(const char* sSection, const char* sZdIniFileKey);
   void         Init();
   void         ParseSessionKey(const char* sSessionKey, ZdString& sSection, ZdString& sZdIniFileKey);
   void         TestKey(const char* sSessionKey);
   void         TestKey(ZdStringTokenizer& theTokenizer);

public:
   BZdIniSession();
   BZdIniSession(ZdInputStreamInterface &theStream);
   virtual   ~BZdIniSession();

   static const char* const XBZDINISESSION_KEY_DELIMITER;
   static const char* const XBZDINISESSION_DEFAULT_IMPLIED_SECTION;

   void           AddProperty(const BSessionProperty* pProperty, unsigned long lFlags = 0);
   void           AddProperty(const char * sKey, const char * sValue , unsigned long lFlags = 0);
   void           AddProperty(const char * sKey, long lValue , unsigned long lFlags = 0);
   void           AddProperty(const char * sKey, bool bValue , unsigned long lFlags = 0);
   void           Clear();
   void           CreateZdIniFile(const char *sFileName , ZdIOFlag iOpenFlags , ZdIOFlag iLockFlags = 0, long lNumTries = 1, unsigned short wDelayInMS = 0);
   void           DeleteProperty(const char *sKey);
   void           DeleteProperty(long lIndex);
   void           DeletePropertiesLike(const char *sKey);
   void           DestroyZdIniFileObject(ZdIOFlag iLockFlags = 0);
   long           FindProperty(const char *sKey);
   const char*    GetKeyDelimiter() const;
   const char*    GetImpliedSection() const;
                  
   BSessionProperty*    GetProperty(const char* sKey);
   BSessionProperty*    GetProperty(long lPosition);

   void           GetRangeOfPropertiesLike(const char *sKey, long *plStart, long *plEnd);
   bool           IsUsingImpliedSection() const;
   bool           IsValidKey(const char* sSessionKey);
   bool           IsValidKey(ZdStringTokenizer& theTokenizer);
   void           Read( ZdInputStreamInterface &theStream );
   void           Read(const char* sFileName, ZdIOFlag iOpenFlag = ZDIO_OPEN_READ|ZDIO_OPEN_WRITE,ZdIOFlag iLockFlags = 0,
                       long lNumTries = 1, unsigned short wDelayInMS = 0, unsigned long lFlags = 0);
   void           SetImpliedSection(const char* sImpliedSection);
   void           UseImpliedSection(bool bUseImpliedSection = true);
   void           Write( ZdOutputStreamInterface &theStream );
   void           Write(const char* sFileName, ZdIOFlag iOpenFlags = ZDIO_OPEN_READ|ZDIO_OPEN_WRITE,
                        ZdIOFlag iLockFlags = 0, long lNumTries = 1, unsigned short wDelayInMS = 0, 
                        bool bReSortArray = false);

};
//******************************************************************************
#endif
