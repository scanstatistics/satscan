#ifndef stsClusterFile_H
#define stsClusterFile_H

#include "stsOutputFileData.h"

extern const char *	CLUSTER_FILE_EXT;

class CCluster;
class CSaTScanData;

class stsClusterData : public BaseOutputStorageClass {
   private:
      int       giDimension, giModelType, giCoordType;
      bool      gbPrintEllipses, gbPrintPVal, gbIncludeRunHistory;
      long	glRunNumber;
      
      void	Init();
      void      SetAreaID(ZdString& sTempValue, const CCluster& pCluster, const CSaTScanData& pData);
      void      SetCoordinates(ZdString& sLatitude, ZdString& sLongitude, ZdString& sRadius,
                               ZdVector<ZdString>& vAdditCoords,
                               const CCluster& pCluster, const CSaTScanData& pData);
      void      SetEllipseString(ZdString& sAngle, ZdString& sShape, const CCluster& pCluster, const CSaTScanData& pData);
      void      SetStartAndEndDates(ZdString& sStartDate, ZdString& sEndDate, const CCluster& pCluster, const CSaTScanData& pData);                         
      void	Setup(const ZdString& sOutputFileName, const int iModelType,
                      const int iDimension,  const int iCoordType,
                      const long lRunNumber, const bool bPrintEllipses, const bool bPrintPVal);
      void  	SetupFields();

   public:
      __fastcall stsClusterData(BasePrint *pPrintDirection, const ZdString& sOutputFileName, const long lRunNumber, const int iCoordType, const int iModelType, const int iDimension = 2, const bool bPrintPVal = true, const bool bPrintEllipses = false);
      virtual    ~stsClusterData();

      void      RecordClusterData(const CCluster& pCluster, const CSaTScanData& pData, int iClusterNumber);
};

class ClusterRecord : public BaseOutputRecord {
   private:
      long 			glRunNumber;
      ZdString 			gsLocationID;
      int 			giClusterNumber;
      ZdString 			gsFirstCoord;
      ZdString 			gsSecondCoord;
      ZdVector<ZdString> 	gvsAdditCoords;
      ZdString 			gsRadius;
      ZdString 			gsEllipseAngles;
      ZdString 			gsEllipseShapes;
      long 			glNumAreas;
      long 			glObserved;
      double 			gdExpected;
      double 			gdRelRisk;
      double 			gdLogLikelihood;
      double 			gdPValue;
      ZdString 			gsStartDate;
      ZdString 			gsEndDate;

      bool                      gbPrintEllipses, gbPrintPVal, gbIncludeRunHistory;
      void		        Init();
   public:
      ClusterRecord(const bool bPrintEllipses = true, const bool bPrintPVal = true, const bool bIncludeRunHistory = true);
      virtual ~ClusterRecord();
      
      virtual int GetNumFields();
      virtual ZdFieldValue GetValue(int iFieldNumber);
    
      void	SetAdditionalCoordinates(const ZdVector<ZdString>& vsAdditCoords) { gvsAdditCoords = vsAdditCoords; }
      void	SetClusterNumber(const int iClusterNumber) { giClusterNumber = iClusterNumber; }     
      void 	SetEllipseAngles(const ZdString& sEllipseAngles) { gsEllipseAngles = sEllipseAngles; }
      void	SetEllipseShapes(const ZdString& sEllipseShapes) { gsEllipseShapes = sEllipseShapes; }
      void	SetEndDate(const ZdString& sEndDate) { gsEndDate = sEndDate; }
      void	SetExpected(const double dExpected) { gdExpected = dExpected; }
      void	SetFirstCoordinate(const ZdString& sFirstCoord) { gsFirstCoord = sFirstCoord; }
      void	SetLocationID(const ZdString& sLocationID) { gsLocationID = sLocationID; }
      void	SetLogLikelihood(const double dLogLikelihood) { gdLogLikelihood = dLogLikelihood; }
      void	SetNumAreas(const long lNumAreas) { glNumAreas = lNumAreas; }
      void	SetObserved(const long lObserved) { glObserved = lObserved; }
      void	SetPValue(const double dPValue) { gdPValue = dPValue; }
      void	SetRadius(const ZdString& sRadius) { gsRadius = sRadius; }
      void	SetRelativeRisk(const double dRelRisk) { gdRelRisk = dRelRisk; }
      void 	SetRunNumber(const long	lRunNumber) { glRunNumber = lRunNumber; }
      void	SetSecondCoordinate(const ZdString& sSecondCoord) { gsSecondCoord = sSecondCoord; }
      void	SetStartDate(const ZdString& sStartDate) { gsStartDate = sStartDate; }      
};

#endif