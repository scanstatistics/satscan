#ifndef stsAreaSpecificData_h
#define stsAreaSpecificData_h

#include "stsOutputFileData.h"

extern const char *	AREA_SPECIFIC_FILE_EXT;

class CCluster;
class CSaTScanData;

class stsAreaSpecificData : public BaseOutputStorageClass {
   private:
      long	glRunNumber;
      bool	gbPrintPVal, gbIncludeRunHistory;
       
      void	Init();
      void	Setup(const ZdString& sOutputFileName, const long lRunNumber, const bool bPrintPVal);
   protected:
      virtual void   SetupFields();
   public:
      stsAreaSpecificData(BasePrint *pPrintDirection, const ZdString& sOutputFileName, const long lRunNumber, const bool bPrintPVal);
      virtual ~stsAreaSpecificData();

      virtual void   RecordClusterData(const CCluster& theCluster, const CSaTScanData& theData, int iClusterNumber, tract_t tTract);
};

class AreaSpecificRecord : public BaseOutputRecord {
   private:
      long 	glRunNumber;
      ZdString 	gsLocationID;
      int 	giClusterNumber;
      long 	glClusterObserved;
      double 	gdClusterExpected;
      double 	gdRelRisk;
      double 	gdPValue;
      long 	glAreaObserved;
      double 	gdAreaExpected;
      double 	gdAreaRelRisk;

      bool	gbPrintPVal, gbIncludeRunHistory;
      void 	Init();
   public :
      AreaSpecificRecord(const bool bPrintPVal = true, const bool bIncludeRunHistory = true);
      virtual ~AreaSpecificRecord();

      virtual bool GetFieldIsBlank(int iFieldNumber);
      virtual int GetNumFields();
      virtual ZdFieldValue GetValue(int iFieldNumber);

      void      SetFieldIsBlank(int iFieldNumber, bool bBlank = true);

      void	SetAreaExpected(const double dAreaExpected) { gdAreaExpected = dAreaExpected; }
      void	SetAreaObserved(const long lAreaObserved) { glAreaObserved = lAreaObserved; }  
      void	SetAreaRelativeRisk(const double dAreaRelRisk) { gdAreaRelRisk = dAreaRelRisk; }
      void	SetClusterExpected(const double dClusterExpected) { gdClusterExpected = dClusterExpected; }
      void	SetClusterNumber(const int iClusterNumber) { giClusterNumber = iClusterNumber; }
      void	SetClusterObserved(const long lClusterObserved) { glClusterObserved = lClusterObserved; }
      void	SetClusterRelativeRisk(const double dRelRisk) { gdRelRisk = dRelRisk; }
      void	SetLocationID(const ZdString& sLocationID) { gsLocationID = sLocationID; } 
      void	SetPValue(const double dPValue) { gdPValue = dPValue; }
      void  	SetRunNumber(const long lRunNumber) { glRunNumber = lRunNumber; }   
}; 

#endif