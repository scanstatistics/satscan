#ifndef stsAreaSpecificData_h
#define stsAreaSpecificData_h

#include "stsOutputFileData.h"

extern const char *	AREA_SPECIFIC_FILE_EXT;

class CCluster;
class CSaTScanData;

class stsAreaSpecificData : public BaseOutputStorageClass {
   private:
      long	glRunNumber;
      bool	gbPrintPVal;
       
      void	Init();
      void	Setup(const ZdString& sOutputFileName, const long lRunNumber, const bool bPrintPVal);
   protected:
      virtual void              SetupFields();
   public:
      __fastcall stsAreaSpecificData(const ZdString& sOutputFileName, const long lRunNumber, const bool bPrintPVal);
      virtual ~stsAreaSpecificData();

      virtual void              RecordClusterData(const CCluster& pCluster, const CSaTScanData& pData, int iClusterNumber, tract_t tTract);
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
      
      void 	Init();
   public :
      AreaSpecificRecord();
      virtual ~AreaSpecificRecord();
   
      virtual int GetNumFields() { return 10; }
      virtual ZdFieldValue GetValue(int iFieldNumber);
        
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