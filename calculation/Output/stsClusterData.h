#ifndef stsClusterFile_H
#define stsClusterFile_H

#include "stsOutputFileData.h"
#include "Parameters.h"

extern const char *	CLUSTER_FILE_EXT;

class CCluster;
class CSaTScanData;

class stsClusterData : public BaseOutputStorageClass {
   private:
      ProbabiltyModelType       geProbabiltyModelType;
      int       giDimension, giCoordType;
      bool      gbPrintEllipses, gbPrintPVal, gbIncludeRunHistory, gbDuczmalCorrect;
      long	glRunNumber;
      
      void	Init();
      void      SetAreaID(std::string& sId, const CCluster& pCluster, const CSaTScanData& pData);
      void      SetCoordinates(ZdString& sLatitude, ZdString& sLongitude, ZdString& sRadius,
                               std::vector<std::string>& vAdditCoords,
                               const CCluster& pCluster, const CSaTScanData& pData);
      void      SetEllipseString(ZdString& sAngle, ZdString& sShape, const CCluster& pCluster, const CSaTScanData& pData);
      void      SetStartAndEndDates(ZdString& sStartDate, ZdString& sEndDate, const CCluster& pCluster, const CSaTScanData& pData);                         
      void	Setup(const ZdString& sOutputFileName);
      void  	SetupFields();

   public:
      stsClusterData(BasePrint *pPrintDirection, const ZdString& sOutputFileName,
                                const long lRunNumber, const int iCoordType,
                                ProbabiltyModelType eProbabiltyModelType, const int iDimension = 2,
                                const bool bPrintPVal = true, const bool bPrintEllipses = false,
                                const bool bDuczmalCorrect = false);
      virtual    ~stsClusterData();

      void      RecordClusterData(const CCluster& theCluster, const CSaTScanData& theData, int iClusterNumber, int iNumSimulations);
};

class ClusterRecord : public BaseOutputRecord {
   private:
      long 			glRunNumber;
      ZdString 			gsLocationID;
      int 			giClusterNumber;
      ZdString 			gsFirstCoord;
      ZdString 			gsSecondCoord;
      std::vector<std::string> 	gvsAdditCoords;
      ZdString 			gsRadius;
      ZdString 			gsEllipseAngles;
      ZdString 			gsEllipseShapes;
      long 			glNumAreas;
      long 			glObserved;
      double 			gdExpected;
      double 			gdRelRisk;
      double 			gdLogLikelihood;
      double                    gdTestStat;
      double 			gdPValue;
      ZdString 			gsStartDate;
      ZdString 			gsEndDate;

      int                       giNumFields;
      bool                      gbPrintEllipses, gbPrintPVal, gbIncludeRunHistory, gbSpaceTimeModel, gbDuczmalCorrect;
      void		        Init();
   public:
      ClusterRecord(const bool bPrintEllipses = true, const bool bPrintPVal = true, const bool bIncludeRunHistory = true, const bool bSpaceTimeModel = false, const bool bDuczmalCorrect = false);
      virtual ~ClusterRecord();

      virtual bool      GetFieldIsBlank(int iFieldNumber);
      virtual int       GetNumFields() { return giNumFields; }
      virtual ZdFieldValue GetValue(int iFieldNumber);
      void              SetFieldIsBlank(int iFieldNumber, bool bBlank);

      void	SetAdditionalCoordinates(const std::vector<std::string>& vsAdditCoords) { gvsAdditCoords = vsAdditCoords; }
      void	SetClusterNumber(const int iClusterNumber) { giClusterNumber = iClusterNumber; }     
      void 	SetEllipseAngles(const ZdString& sEllipseAngles) { gsEllipseAngles = sEllipseAngles; }
      void	SetEllipseShapes(const ZdString& sEllipseShapes) { gsEllipseShapes = sEllipseShapes; }
      void	SetEndDate(const ZdString& sEndDate) { gsEndDate = sEndDate; }
      void	SetExpected(const double dExpected) { gdExpected = dExpected; }
      void	SetFirstCoordinate(const ZdString& sFirstCoord) { gsFirstCoord = sFirstCoord; }
      void	SetLocationID(const ZdString& sLocationID) { gsLocationID = sLocationID; }
      void	SetLogLikelihoodRatio(const double dLogLikelihood) { gdLogLikelihood = dLogLikelihood; }
      void	SetNumAreas(const long lNumAreas) { glNumAreas = lNumAreas; }
      void	SetObserved(const long lObserved) { glObserved = lObserved; }
      void	SetPValue(const double dPValue) { gdPValue = dPValue; }
      void	SetRadius(const ZdString& sRadius) { gsRadius = sRadius; }
      void	SetRelativeRisk(const double dRelRisk) { gdRelRisk = dRelRisk; }
      void 	SetRunNumber(const long	lRunNumber) { glRunNumber = lRunNumber; }
      void	SetSecondCoordinate(const ZdString& sSecondCoord) { gsSecondCoord = sSecondCoord; }
      void	SetStartDate(const ZdString& sStartDate) { gsStartDate = sStartDate; }
      void      SetTestStat(const double dTestStat) { gdTestStat = dTestStat; }      
};

#endif
