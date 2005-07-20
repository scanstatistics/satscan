//*****************************************************************************
#ifndef __PARAMETERS_H
#define __PARAMETERS_H
//*****************************************************************************
#include "SaTScan.h"
#include "JulianDates.h"
#include "UtilityFunctions.h"

/** parameter types
    - parameters that are read from file with the exception of: DIMENSION, EXACTTIMES, and RUN_HISTORY_FILENAME */
enum ParameterType                 {ANALYSISTYPE=1, SCANAREAS, CASEFILE, POPFILE, COORDFILE, OUTPUTFILE,
                                    PRECISION, DIMENSION, SPECIALGRID, GRIDFILE, GEOSIZE, STARTDATE, ENDDATE,
                                    CLUSTERS, EXACTTIMES, TIME_AGGREGATION_UNITS, TIME_AGGREGATION, PURESPATIAL,
                                    TIMESIZE, REPLICAS, MODEL, RISKFUNCTION, POWERCALC, POWERX, POWERY, TIMETREND,
                                    TIMETRENDPERC, PURETEMPORAL, CONTROLFILE, COORDTYPE, OUTPUT_SIM_LLR_ASCII,
                                    SEQUENTIAL, SEQNUM, SEQPVAL, VALIDATE, OUTPUT_RR_ASCII, ELLIPSES, ESHAPES,
                                    ENUMBERS, START_PROSP_SURV, OUTPUT_AREAS_ASCII, OUTPUT_MLC_ASCII,
                                    CRITERIA_SECOND_CLUSTERS, MAX_TEMPORAL_TYPE, MAX_SPATIAL_TYPE,
                                    RUN_HISTORY_FILENAME, OUTPUT_MLC_DBASE, OUTPUT_AREAS_DBASE, OUTPUT_RR_DBASE,
                                    OUTPUT_SIM_LLR_DBASE, NON_COMPACTNESS_PENALTY, INTERVAL_STARTRANGE, 
                                    INTERVAL_ENDRANGE, TIMETRENDCONVRG, MAXCIRCLEPOPFILE, EARLY_SIM_TERMINATION,
                                    REPORTED_GEOSIZE, USE_REPORTED_GEOSIZE, SIMULATION_TYPE,
                                    SIMULATION_SOURCEFILE, ADJ_BY_RR_FILE, OUTPUT_SIMULATION_DATA,
                                    SIMULATION_DATA_OUTFILE, ADJ_FOR_EALIER_ANALYSES, USE_ADJ_BY_RR_FILE, SPATIAL_ADJ_TYPE,
                                    MULTI_DATASET_PURPOSE_TYPE, CREATION_VERSION, RANDOMIZATION_SEED, REPORT_CRITICAL_VALUES,
                                    EXECUTION_TYPE, NUM_PROCESSES};
/** analysis and cluster types */
enum AnalysisType                  {PURELYSPATIAL=1, PURELYTEMPORAL, SPACETIME,  PROSPECTIVESPACETIME,
                                    SPATIALVARTEMPTREND, PROSPECTIVEPURELYTEMPORAL};
/** cluster types */
enum ClusterType                   {PURELYSPATIALCLUSTER=1, PURELYTEMPORALCLUSTER, SPACETIMECLUSTER,
                                    SPATIALVARTEMPTRENDCLUSTER, PURELYSPATIALMONOTONECLUSTER, PURELYSPATIALPROSPECTIVECLUSTER};
/** probability model types */
enum ProbabilityModelType          {POISSON=0, BERNOULLI, SPACETIMEPERMUTATION, ORDINAL, EXPONENTIAL, NORMAL, RANK};
enum IncludeClustersType           {ALLCLUSTERS=0, ALIVECLUSTERS, CLUSTERSINRANGE};
enum RiskType                      {STANDARDRISK=0, MONOTONERISK};
/** area incidence rate types */
enum AreaRateType                  {HIGH=1, LOW, HIGHANDLOW};
/** time trend adjustment types */
enum TimeTrendAdjustmentType       {NOTADJUSTED=0, NONPARAMETRIC, LOGLINEAR_PERC,
                                    CALCULATED_LOGLINEAR_PERC, STRATIFIED_RANDOMIZATION};
/** spatial adjustment types */
enum SpatialAdjustmentType         {NO_SPATIAL_ADJUSTMENT=0, SPATIALLY_STRATIFIED_RANDOMIZATION};
enum CoordinatesType               {CARTESIAN=0, LATLON};
/** criteria for reporting secondary clusters types */
enum CriteriaSecondaryClustersType {NOGEOOVERLAP=0, NOCENTROIDSINOTHER, NOCENTROIDSINMORELIKE,
                                    NOCENTROIDSINLESSLIKE, NOPAIRSINEACHOTHERS, NORESTRICTIONS};
/** interperation types for maximum temporal size */
enum TemporalSizeType              {PERCENTAGETYPE=0, TIMETYPE};
/** interperation types for maximum spatial size */
enum SpatialSizeType               {PERCENTOFPOPULATIONTYPE=0, DISTANCETYPE, PERCENTOFPOPULATIONFILETYPE};
/** defines how simulated data will be created - only pertinent for Poisson */
enum SimulationType                {STANDARD=0, HA_RANDOMIZATION, FILESOURCE};
/** purpose of multiple data sets */
enum MultipleDataSetPurposeType    {MULTIVARIATE=0, ADJUSTMENT};
/** analysis execution type */
enum ExecutionType                 {AUTOMATIC=0, SUCCESSIVELY, CENTRICALLY};

class DataSetHandler; /** forward class declaration */

class CParameters {
  public:
    struct CreationVersion {unsigned int iMajor; unsigned int iMinor; unsigned int iRelease;};

  private:
    unsigned int                        giNumRequestedParallelProcesses;        /** number of parallel processes to run */
    ExecutionType                       geExecutionType;                        /** execution process type */
    MultipleDataSetPurposeType          geMultipleSetPurposeType;               /** purpose for multiple data sets */
    AnalysisType                        geAnalysisType;                         /** analysis type */
    ProbabilityModelType                geProbabilityModelType;                  /** probability model type */
    AreaRateType                        geAreaScanRate;                         /** areas incidence rate type of interest */
    RiskType                            geRiskFunctionType;                     /**  */
    IncludeClustersType                 geIncludeClustersType;
    unsigned int                        giReplications;                         /** number of MonteCarlo replicas */
    CriteriaSecondaryClustersType       geCriteriaSecondClustersType;           /** Criteria for Reporting Secondary Clusters */
    double                              gdTimeTrendConverge;                    /** time trend convergence value for SVTT */
    bool                                gbEarlyTerminationSimulations;          /** indicates whether to stop simulations if large p-values */
    SimulationType                      geSimulationType;                       /** indicates simulation procedure - Poisson only */
    bool                                gbOutputSimulationData;                 /** indicates whether to output simulation data to file */
    bool                                gbAdjustForEarlierAnalyses;             /** indicates whether to adjust for earlier analyses,
                                                                                    pertinent for prospective analyses */
    SpatialAdjustmentType               geSpatialAdjustmentType;                /** type of spatial adjustment*/
        /* Power Calcution variables */
    bool                                gbPowerCalculation;                     /** indicator of whether to perform power calculations */
    double                              gdPower_X, gdPower_Y;                   /** power calculation variables */
        /* Maximum spatial cluster variables */
    float                               gfMaxGeographicClusterSize;             /** maximum value for spatial cluster */
    SpatialSizeType                     geMaxGeographicClusterSizeType;         /** maximum spatial cluster value type */
    bool                                gbRestrictReportedClusters;             /** indicates whether reported clusters are limited to specified maximum size */
    float                               gfMaxReportedGeographicClusterSize;     /** maximum spatial cluster size reported */
    /* Maximum temporal cluster variables */
    float                               gfMaxTemporalClusterSize;               /** maximum value for temporal cluster */
    TemporalSizeType                    geMaxTemporalClusterSizeType;           /** maximum temporal cluster value type */
        /* Time interval variables */
    long                                glTimeAggregationLength;                /** length of time aggregation */
    DatePrecisionType                   geTimeAggregationUnitsType;             /** time aggregation units type */
        /* Temporal trend adjusment variables */
    double                              gdTimeTrendAdjustPercentage;            /** percentage for log linear adjustment */
    TimeTrendAdjustmentType             geTimeTrendAdjustType;                  /** Adjust for time trend: no, discrete, % */
        /* Input precision variables */
    DatePrecisionType                   gePrecisionOfTimesType;                 /** precision of case/control data: none = no, years=months=days = yes */
    int                                 giDimensionsOfData;                     /** dimensions in geographic data */
    CoordinatesType                     geCoordinatesType;                      /** coordinates type for coordinates/special grid */
        /* Ellipse variables */
    int                                 giNumberEllipses;                       /** number of ellipsoids requested */
    long                                glTotalNumEllipses;                     /** total number of ellipses (ellipses by number rotations) */
    std::vector<double>                 gvEllipseShapes;                        /** shape of each ellipsoid */
    std::vector<int>                    gvEllipseRotations;                     /** number of rotations for each ellipsoid */
    bool                                gbNonCompactnessPenalty;                /** indicates whether narrower ellipses should be penalized */
        /* Pure Clusters variables */
    bool                                gbIncludePurelySpatialClusters,         /** indicates whether to include purely spatial clusters */
                                        gbIncludePurelyTemporalClusters;        /** indicates whether to include purely temporal clusters */
        /* Additional output variables */
    bool                                gbOutputSimLogLikeliRatiosAscii,        /** indicates whether to output simulated loglikelihood ratios in acsii format */
                                        gbOutputSimLogLikeliRatiosDBase;        /** indicates whether to output simulated loglikelihood ratios in dBase format */
    bool                                gbOutputRelativeRisksAscii,             /** indicates whether to output relative risks for each tract/location in ascii format */
                                        gbOutputRelativeRisksDBase;             /** indicates whether to output relative risks for each tract/location in dBase format */
    bool                                gbOutputClusterLevelAscii,              /** indicates whether to output most likely clusters for each centroid in ascii format */
                                        gbOutputClusterLevelDBase;              /** indicates whether to output most likely clusters for each centroid in dBase format */
    bool                                gbOutputAreaSpecificAscii,              /** indicates whether to output tract/location information of reported(.i.e top ranked) clusters in ascii format */
                                        gbOutputAreaSpecificDBase;              /** indicates whether to output tract/location information of reported(.i.e top ranked) clusters in dBase format */
        /* Sequential scans variables */
    bool                                gbSequentialRuns;                       /* sequential analysis? */
    unsigned int                        giNumSequentialRuns;                    /* number of sequential scans to attempt */
    double                              gbSequentialCutOffPValue;               /* P-Value used to exit sequential analysis */
        /* Input/Output filenames */
    std::string                         gsParametersSourceFileName;             /** parameters source filename */
    std::vector<std::string>            gvCaseFilenames;                        /** case data source filenames */
    std::vector<std::string>            gvControlFilenames;                     /** control data source filenames */
    std::vector<std::string>            gvPopulationFilenames;                  /** population data source filenames */
    bool                                gbUsePopulationFile;                    /** indicates whether population data will be read
                                                                                    given other parameter settings */
    std::string                         gsCoordinatesFileName;                  /** coordinates data source filename */
    std::string                         gsSpecialGridFileName;                  /** special grid data source filename */
    bool                                gbUseSpecialGridFile;                   /** indicator of special grid file usage */
    std::string                         gsMaxCirclePopulationFileName;           /** special population file for constructing circles only */
    std::string                         gsOutputFileName;                       /** results output filename */
    ZdString                            gsRunHistoryFilename;                   /** run history filename */
    bool                                gbLogRunHistory;                        /** indicates whether to log history */
    std::string                         gsSimulationDataSourceFileName;         /** simualtion data source filename */
    bool                                gbUseAdjustmentsForRRFile;              /** indicates whether to use adjustments for known relative risks file */
    std::string                         gsAdjustmentsByRelativeRisksFileName;   /** adjustments by known relative risks filename */
    std::string                         gsSimulationDataOutputFilename;         /** simulation data output filename */
        /* Analysis dates */
    std::string                         gsProspectiveStartDate;                 /** prospective start date in YYYY/MM/DD, YYYY/MM, or YYYY format */
    std::string                         gsStudyPeriodStartDate;                 /** study period start date in YYYY/MM/DD, YYYY/MM, or YYYY format */
    std::string                         gsStudyPeriodEndDate;                   /** study period end date in YYYY/MM/DD, YYYY/MM, or YYYY format */
    std::string                         gsEndRangeStartDate;
    std::string                         gsEndRangeEndDate;
    std::string                         gsStartRangeStartDate;
    std::string                         gsStartRangeEndDate;
        /* Parameter validation variables */
    bool                                gbValidatePriorToCalc;                  /** prevents validation prior to execution
                                                                                    The intent of this parameter is to allow an advanced
                                                                                    user to set values to something that would normally
                                                                                    be considered invalid. e.g. setting maximum spatial
                                                                                    cluster size to 60 instead of 50. Settings this parameter
                                                                                    to false has an implied disclaimer, you may get strange
                                                                                    occurances programmatically and statically. */

    struct CreationVersion              gCreationVersion;
    long                                glRandomizationSeed;                    /** randomization seed */
    bool                                gbReportCriticalValues;                 /** indicates whether to report critical llr values */

    void                                ConvertRelativePath(std::string & sInputFilename);
    void                                Copy(const CParameters &rhs);
    const char                        * GetRelativeToParameterName(const ZdFileName& fParameterName, const std::string& sFilename, ZdString& sValue) const;

  public:
    CParameters();
    CParameters(const CParameters &other);
    ~CParameters();

    static const int                    giNumParameters;                        /** number enumerated parameters */
    static const int                    MAXIMUM_SEQUENTIAL_ANALYSES;            /** maximum number of permitted sequential scans */
    static const int                    MAXIMUM_ELLIPSOIDS;                     /** maximum number of permitted ellipsoids */

    CParameters                       & operator=(const CParameters &rhs);
    bool                                operator==(const CParameters& rhs) const;
    bool                                operator!=(const CParameters& rhs) const;

    void                                ClearEllipseSettings();
    bool                                GetAdjustForEarlierAnalyses() const {return gbAdjustForEarlierAnalyses;}
    const std::string                 & GetAdjustmentsByRelativeRisksFilename() const {return gsAdjustmentsByRelativeRisksFileName;}
    AnalysisType                        GetAnalysisType() const {return geAnalysisType;}
    const char                        * GetAnalysisTypeAsString() const;
    AreaRateType                        GetAreaScanRateType() const {return geAreaScanRate;}
    const std::string                 & GetCaseFileName(size_t iSetIndex=1) const;
    const std::vector<std::string>    & GetCaseFileNames() const {return gvCaseFilenames;}
    const std::string                 & GetControlFileName(size_t iSetIndex=1) const;
    const std::vector<std::string>    & GetControlFileNames() const {return gvControlFilenames;}
    const std::string                 & GetCoordinatesFileName() const {return gsCoordinatesFileName;}
    CoordinatesType                     GetCoordinatesType() const {return geCoordinatesType;}
    const CreationVersion            &  GetCreationVersion() const {return gCreationVersion;}
    CriteriaSecondaryClustersType       GetCriteriaSecondClustersType() const {return geCriteriaSecondClustersType;}
    unsigned int                        GetCreationVersionMajor() const {return gCreationVersion.iMajor;}
    int                                 GetDimensionsOfData() const {return giDimensionsOfData;}
    const std::vector<int>            & GetEllipseRotations() const {return gvEllipseRotations;}
    const std::vector<double>         & GetEllipseShapes() const {return gvEllipseShapes;}
    const std::string                 & GetEndRangeEndDate() const {return gsEndRangeEndDate;}
    const std::string                 & GetEndRangeStartDate() const {return gsEndRangeStartDate;}
    ExecutionType                       GetExecutionType() const {return geExecutionType;}
    IncludeClustersType                 GetIncludeClustersType() const {return geIncludeClustersType;}
    bool                                GetIncludePurelySpatialClusters() const {return gbIncludePurelySpatialClusters;}
    bool                                GetIncludePurelyTemporalClusters() const {return gbIncludePurelyTemporalClusters;}
    bool                                GetIsLoggingHistory() const {return gbLogRunHistory;}
    bool                                GetIsPowerCalculated() const {return gbPowerCalculation;}
    bool                                GetIsProspectiveAnalysis() const;
    bool                                GetIsPurelyTemporalAnalysis() const;
    bool                                GetIsSequentialScanning() const {return gbSequentialRuns;}
    bool                                GetIsSpaceTimeAnalysis() const;
    bool                                GetLogLikelihoodRatioIsTestStatistic() const;
    const std::string                 & GetMaxCirclePopulationFileName() const {return gsMaxCirclePopulationFileName;}
    float                               GetMaximumGeographicClusterSize() const {return gfMaxGeographicClusterSize;}
    SpatialSizeType                     GetMaxGeographicClusterSizeType() const {return geMaxGeographicClusterSizeType;}
    bool                                GetMaxGeoClusterSizeTypeIsPopulationBased() const {return geMaxGeographicClusterSizeType == PERCENTOFPOPULATIONTYPE ||
                                                                                                  geMaxGeographicClusterSizeType == PERCENTOFPOPULATIONFILETYPE;}
    float                               GetMaximumTemporalClusterSize() const {return gfMaxTemporalClusterSize;}
    TemporalSizeType                    GetMaximumTemporalClusterSizeType() const {return geMaxTemporalClusterSizeType;}
    float                               GetMaximumReportedGeoClusterSize() const {return gfMaxReportedGeographicClusterSize;}
    MultipleDataSetPurposeType          GetMultipleDataSetPurposeType() const {return geMultipleSetPurposeType;}
    bool                                GetNonCompactnessPenalty() const {return gbNonCompactnessPenalty;}
    unsigned int                        GetNumDataSets() const {return gvCaseFilenames.size();}
    unsigned int                        GetNumRequestedParallelProcesses() const {return giNumRequestedParallelProcesses;}
    unsigned int                        GetNumParallelProcessesToExecute() const;
    int                                 GetNumReadParameters() const {return giNumParameters;}
    unsigned int                        GetNumReplicationsRequested() const {return giReplications;}
    int                                 GetNumRequestedEllipses() const {return giNumberEllipses;}
    unsigned int                        GetNumSequentialScansRequested() const {return giNumSequentialRuns;}
    long                                GetNumTotalEllipses() const {return glTotalNumEllipses;}
    bool                                GetOutputAreaSpecificAscii() const  {return gbOutputAreaSpecificAscii;}
    bool                                GetOutputAreaSpecificDBase() const  {return gbOutputAreaSpecificDBase;}
    bool                                GetOutputAreaSpecificFiles() const;
    bool                                GetOutputClusterLevelAscii() const {return gbOutputClusterLevelAscii;}
    bool                                GetOutputClusterLevelDBase() const {return gbOutputClusterLevelDBase;}
    bool                                GetOutputClusterLevelFiles() const;
    const std::string                 & GetOutputFileName() const {return gsOutputFileName; }
    bool                                GetOutputRelativeRisksAscii() const {return gbOutputRelativeRisksAscii;}
    bool                                GetOutputRelativeRisksDBase() const {return gbOutputRelativeRisksDBase;}
    bool                                GetOutputRelativeRisksFiles() const;
    bool                                GetOutputSimLoglikeliRatiosAscii() const {return gbOutputSimLogLikeliRatiosAscii;}
    bool                                GetOutputSimLoglikeliRatiosDBase() const {return gbOutputSimLogLikeliRatiosDBase;}
    bool                                GetOutputSimLoglikeliRatiosFiles() const;
    bool                                GetOutputSimulationData() const {return gbOutputSimulationData;}
    bool                                GetPermitsPurelySpatialCluster(ProbabilityModelType eModelType) const;
    bool                                GetPermitsPurelySpatialCluster(AnalysisType eAnalysisType) const;
    bool                                GetPermitsPurelyTemporalCluster(ProbabilityModelType eModelType) const;
    bool                                GetPermitsPurelyTemporalCluster(AnalysisType eAnalysisType) const;
    const std::string                 & GetPopulationFileName(size_t iSetIndex=1) const;
    const std::vector<std::string>    & GetPopulationFileNames() const {return gvPopulationFilenames;}
    double                              GetPowerCalculationX() const {return gdPower_X;}
    double                              GetPowerCalculationY() const {return gdPower_Y;}
    DatePrecisionType                   GetPrecisionOfTimesType() const {return gePrecisionOfTimesType;}
    ProbabilityModelType                GetProbabilityModelType() const {return geProbabilityModelType;}
    const char                        * GetProbabilityModelTypeAsString(ProbabilityModelType eProbabilityModelType) const;
    const std::string                 & GetProspectiveStartDate() const {return gsProspectiveStartDate;}
    long                                GetRandomizationSeed() const {return glRandomizationSeed;}
    bool                                GetReportCriticalValues() const {return gbReportCriticalValues;}
    bool                                GetRestrictingMaximumReportedGeoClusterSize() const {return gbRestrictReportedClusters;}
    RiskType                            GetRiskType() const {return geRiskFunctionType;}
    const ZdString                    & GetRunHistoryFilename() const  { return gsRunHistoryFilename; }
    double                              GetSequentialCutOffPValue() const {return gbSequentialCutOffPValue;}
    const std::string                 & GetSimulationDataOutputFilename() const {return gsSimulationDataOutputFilename;}  
    const std::string                 & GetSimulationDataSourceFilename() const {return gsSimulationDataSourceFileName;}
    SimulationType                      GetSimulationType() const {return geSimulationType;}
    const std::string                 & GetSourceFileName() const {return gsParametersSourceFileName;}
    SpatialAdjustmentType               GetSpatialAdjustmentType() const {return geSpatialAdjustmentType;}
    const std::string                 & GetSpecialGridFileName() const {return gsSpecialGridFileName;}
    const std::string                 & GetStartRangeEndDate() const {return gsStartRangeEndDate;}
    const std::string                 & GetStartRangeStartDate() const {return gsStartRangeStartDate;}
    const std::string                 & GetStudyPeriodEndDate() const {return gsStudyPeriodEndDate;}
    const std::string                 & GetStudyPeriodStartDate() const {return gsStudyPeriodStartDate;}
    bool                                GetTerminateSimulationsEarly() const {return gbEarlyTerminationSimulations;}
    long                                GetTimeAggregationLength() const {return glTimeAggregationLength;}
    DatePrecisionType                   GetTimeAggregationUnitsType() const {return geTimeAggregationUnitsType;}
    double                              GetTimeTrendAdjustmentPercentage() const {return gdTimeTrendAdjustPercentage;}
    TimeTrendAdjustmentType             GetTimeTrendAdjustmentType() const {return geTimeTrendAdjustType;}
    double                              GetTimeTrendConvergence() const {return gdTimeTrendConverge;}
    bool                                GetValidatingParameters() const {return gbValidatePriorToCalc;}
    void                                SetAdjustForEarlierAnalyses(bool b) {gbAdjustForEarlierAnalyses = b;}
    void                                SetAdjustmentsByRelativeRisksFilename(const char * sAdjustmentsByRelativeRisksFileName, bool bCorrectForRelativePath=false);  
    void                                SetAnalysisType(AnalysisType eAnalysisType);
    void                                SetAreaRateType(AreaRateType eAreaRateType);
    void                                SetAsDefaulted();
    void                                SetDimensionsOfData(int iDimensions);
    void                                SetEndRangeEndDate(const char * sEndRangeEndDate);
    void                                SetEndRangeStartDate(const char * sEndRangeStartDate);
    void                                SetExecutionType(ExecutionType eExecutionType);
    void                                SetCaseFileName(const char * sCaseFileName, bool bCorrectForRelativePath=false, size_t iSetIndex=1);
    void                                SetControlFileName(const char * sControlFileName, bool bCorrectForRelativePath=false, size_t iSetIndex=1);
    void                                SetCoordinatesFileName(const char * sCoordinatesFileName, bool bCorrectForRelativePath=false);
    void                                SetCoordinatesType(CoordinatesType eCoordinatesType);
    void                                SetCriteriaForReportingSecondaryClusters(CriteriaSecondaryClustersType eCriteriaSecondaryClustersType);
    void                                SetEllipsoidShape(double dShape, int iEllipsoidIndex=-1);
    void                                SetIncludeClustersType(IncludeClustersType eIncludeClustersType);
    void                                SetIncludePurelySpatialClusters(bool b) {gbIncludePurelySpatialClusters = b;}
    void                                SetIncludePurelyTemporalClusters(bool b) {gbIncludePurelyTemporalClusters = b;}
    void                                SetIsLoggingHistory(bool b) {gbLogRunHistory = b;}
    void                                SetMaxCirclePopulationFileName(const char * sMaxCirclePopulationFileName, bool bCorrectForRelativePath=false, bool bSetUsingFlag=false);
    void                                SetMaximumGeographicClusterSize(float fMaxGeographicClusterSize);
    void                                SetMaximumReportedGeographicalClusterSize(float fMaxReportedGeographicClusterSize);
    void                                SetMaximumSpacialClusterSizeType(SpatialSizeType eSpatialSizeType);
    void                                SetMaximumTemporalClusterSize(float fMaxTemporalClusterSize);
    void                                SetMaximumTemporalClusterSizeType(TemporalSizeType eTemporalSizeType);
    void                                SetMultipleDataSetPurposeType(MultipleDataSetPurposeType eType);
    void                                SetNonCompactnessPenalty(bool b) {gbNonCompactnessPenalty = b;}
    void                                SetNumDataSets(size_t iNumDataSets);
    void                                SetNumParallelProcessesToExecute(unsigned int i) {giNumRequestedParallelProcesses = i;}
    void                                SetNumberEllipses(int iNumEllipses);
    void                                SetNumberEllipsoidRotations(int iNumberRotations, int iEllipsoidIndex=-1);
    void                                SetNumberMonteCarloReplications(unsigned int iReplications);
    void                                SetNumSequentialScans(int iNumSequentialScans);
    void                                SetOutputAreaSpecificAscii(bool b) {gbOutputAreaSpecificAscii = b;}
    void                                SetOutputAreaSpecificDBase(bool b) {gbOutputAreaSpecificDBase = b;}
    void                                SetOutputClusterLevelAscii(bool b) {gbOutputClusterLevelAscii = b;}
    void                                SetOutputClusterLevelDBase(bool b) {gbOutputClusterLevelDBase = b;}
    void                                SetOutputFileName(const char * sOutPutFileName, bool bCorrectForRelativePath=false);
    void                                SetOutputRelativeRisksAscii(bool b) {gbOutputRelativeRisksAscii = b;}
    void                                SetOutputRelativeRisksDBase(bool b) {gbOutputRelativeRisksDBase = b;}
    void                                SetOutputSimLogLikeliRatiosAscii(bool b) {gbOutputSimLogLikeliRatiosAscii = b;}
    void                                SetOutputSimLogLikeliRatiosDBase(bool b) {gbOutputSimLogLikeliRatiosDBase = b;}
    void                                SetOutputSimulationData(bool b) {gbOutputSimulationData = b;}
    void                                SetPopulationFileName(const char * sPopulationFileName, bool bCorrectForRelativePath=false, size_t iSetIndex=1);
    void                                SetPowerCalculation(bool b) {gbPowerCalculation = b;}
    void                                SetPowerCalculationX(double dPowerX);
    void                                SetPowerCalculationY(double dPowerY);
    void                                SetPrecisionOfTimesType(DatePrecisionType eDatePrecisionType);
    void                                SetProbabilityModelType(ProbabilityModelType eProbabilityModelType);
    void                                SetProspectiveStartDate(const char * sProspectiveStartDate);
    void                                SetRandomizationSeed(long lSeed);
    void                                SetReportCriticalValues(bool b) {gbReportCriticalValues = b;}
    void                                SetRestrictReportedClusters(bool b) {gbRestrictReportedClusters = b;}
    void                                SetRiskType(RiskType eRiskType);
    void                                SetRunHistoryFilename(const ZdString& sFilename) {gsRunHistoryFilename = sFilename;}
    void                                SetSequentialCutOffPValue(double dPValue);
    void                                SetSequentialScanning(bool b) {gbSequentialRuns = b;}
    void                                SetSimulationDataOutputFileName(const char * sSourceFileName, bool bCorrectForRelativePath=false);
    void                                SetSimulationDataSourceFileName(const char * sSourceFileName, bool bCorrectForRelativePath=false);
    void                                SetSimulationType(SimulationType eSimulationType);
    void                                SetSourceFileName(const char * sParametersSourceFileName);
    void                                SetSpatialAdjustmentType(SpatialAdjustmentType eSpatialAdjustmentType);
    void                                SetSpecialGridFileName(const char * sSpecialGridFileName, bool bCorrectForRelativePath=false, bool bSetUsingFlag=false);
    void                                SetStartRangeEndDate(const char * sStartRangeEndDate);
    void                                SetStartRangeStartDate(const char * sStartRangeStartDate);
    void                                SetStudyPeriodEndDate(const char * sStudyPeriodEndDate);
    void                                SetStudyPeriodStartDate(const char * sStudyPeriodStartDate);
    void                                SetTerminateSimulationsEarly(bool b) {gbEarlyTerminationSimulations = b;}
    void                                SetTimeAggregationLength(long lTimeAggregationLength);
    void                                SetTimeAggregationUnitsType(DatePrecisionType eTimeAggregationUnits);
    void                                SetTimeTrendAdjustmentPercentage(double dPercentage);
    void                                SetTimeTrendAdjustmentType(TimeTrendAdjustmentType eTimeTrendAdjustmentType);
    void                                SetTimeTrendConvergence(double dTimeTrendConvergence);
    void                                SetUseAdjustmentForRelativeRisksFile(bool b) {gbUseAdjustmentsForRRFile = b;}
    void                                SetUseSpecialGrid(bool b) {gbUseSpecialGridFile = b;}
    void                                SetValidatePriorToCalculation(bool b) {gbValidatePriorToCalc = b;}
    void                                SetVersion(const CreationVersion& vVersion);
    bool                                UseAdjustmentForRelativeRisksFile() const {return gbUseAdjustmentsForRRFile;}
    bool                                UseMaxCirclePopulationFile() const;
    bool                                UsePopulationFile() const {return gbUsePopulationFile;}
    void                                SetPopulationFile(bool b) {gbUsePopulationFile = b;}  /******/
    bool                                UseSpecialGrid() const {return gbUseSpecialGridFile;}
};
//*****************************************************************************
#endif
