package org.satscan.app;
import java.util.*;

public class Parameters implements Cloneable {

    public native boolean                     Read(String filename);
    public native void                        Write(String filename);

    /** analysis and cluster types */
    public enum AnalysisType                  {PURELYSPATIAL, PURELYTEMPORAL, SPACETIME,  PROSPECTIVESPACETIME,
                                               SPATIALVARTEMPTREND, PROSPECTIVEPURELYTEMPORAL};
    /** cluster types */
    public enum ClusterType                   {PURELYSPATIALCLUSTER, PURELYTEMPORALCLUSTER, SPACETIMECLUSTER,
                                               SPATIALVARTEMPTRENDCLUSTER, PURELYSPATIALMONOTONECLUSTER,
                                               PURELYSPATIALPROSPECTIVECLUSTER, PURELYSPATIALHOMOGENEOUSCLUSTER};
    /** probability model types */
    public enum ProbabilityModelType          {POISSON, BERNOULLI, SPACETIMEPERMUTATION, ORDINAL, EXPONENTIAL,
                                               NORMAL, HOMOGENEOUSPOISSON, CATEGORICAL, RANK};
    public enum IncludeClustersType           {ALLCLUSTERS, ALIVECLUSTERS, CLUSTERSINRANGE};
    public enum RiskType                      {STANDARDRISK, MONOTONERISK};
    /** area incidence rate types */
    public enum AreaRateType                  {HIGH, LOW, HIGHANDLOW};
    /** time trend adjustment types */
    public enum TimeTrendAdjustmentType       {NOTADJUSTED, NONPARAMETRIC, LOGLINEAR_PERC,
                                               CALCULATED_LOGLINEAR_PERC, STRATIFIED_RANDOMIZATION, CALCULATED_QUADRATIC_PERC};
    /** spatial adjustment types */
    public enum SpatialAdjustmentType         {NO_SPATIAL_ADJUSTMENT, SPATIALLY_STRATIFIED_RANDOMIZATION};
    public enum CoordinatesType               {CARTESIAN, LATLON};
    /** criteria for reporting secondary clusters types */
    public enum CriteriaSecondaryClustersType {NOGEOOVERLAP, NOCENTROIDSINOTHER, NOCENTROIDSINMORELIKE,
                                               NOCENTROIDSINLESSLIKE, NOPAIRSINEACHOTHERS, NORESTRICTIONS};
    /** interperation types for maximum temporal size */
    public enum TemporalSizeType              {PERCENTAGETYPE, TIMETYPE};
    /** interperation types for maximum spatial size */
    public enum SpatialSizeType               {PERCENTOFPOPULATION, MAXDISTANCE, PERCENTOFMAXCIRCLEFILE};
    /** defines how simulated data will be created - only pertinent for Poisson */
    public enum SimulationType                {STANDARD, HA_RANDOMIZATION, FILESOURCE};
    /** purpose of multiple data sets */
    public enum MultipleDataSetPurposeType    {MULTIVARIATE, ADJUSTMENT};
    /** analysis execution type */
    public enum ExecutionType                 {AUTOMATIC, SUCCESSIVELY, CENTRICALLY};
    /** spatial window shape */
    public enum SpatialWindowType             {CIRCULAR, ELLIPTIC};
    /** non-compactness penalty type */
    public enum NonCompactnessPenaltyType     {NOPENALTY, MEDIUMPENALTY, STRONGPENALTY};
    /** study period data checking type */
    public enum StudyPeriodDataCheckingType   {STRICTBOUNDS, RELAXEDBOUNDS};
    /** geographical coordinates data checking type  */
    public enum CoordinatesDataCheckingType   {STRICTCOORDINATES, RELAXEDCOORDINATES};
    public enum DatePrecisionType             { NONE, YEAR, MONTH, DAY, GENERIC };
    public class CreationVersion {
      public int giMajor;
      public int giMinor;
      public int giRelease;
      public CreationVersion(int iMajor, int iMinor, int iRelease) {
        super();
        giMajor = iMajor;
        giMinor = iMinor;
        giRelease = iRelease;
        }
    }
    /** multiple coordinates type */
    public enum MultipleCoordinatesType       {ONEPERLOCATION, ATLEASTONELOCATION, ALLLOCATIONS};
    /** p-values reporting type */
    public enum PValueReportingType           {DEFAULT_PVALUE, STANDARD_PVALUE, TERMINATION_PVALUE, GUMBEL_PVALUE};
    /** time trend calculation type */
    public enum TimeTrendType                 {LINEAR,QUADRATIC};
    /** cluster reporting type */
    public enum ClusterReportType             {HIERARCHICAL, GINI_INDEX, ALL_CLUSTER_TYPES};
    /** gini index cluster reporting type*/
    public enum GiniIndexReportType           {OPTIMAL_ONLY, ALL_VALUES};
    /** critical values calculation type */
    public enum CriticalValuesType            {CV_MONTECARLO, CV_GUMBEL, CV_POWER_VALUES};
    /** critical values calculation type */
    public enum PowerEstimationType           {PE_MONTECARLO, PE_GUMBEL};
    /** power evaluation method */
    public enum PowerEvaluationMethodType     {PE_WITH_ANALYSIS, PE_ONLY_CASEFILE,PE_ONLY_SPECIFIED_CASES};
    
    private int                             giNumRequestedParallelProcesses=0; /** number of parallel processes to run */
    private ExecutionType                   geExecutionType=ExecutionType.AUTOMATIC; /** execution process type */
    private MultipleDataSetPurposeType      geMultipleSetPurposeType=MultipleDataSetPurposeType.MULTIVARIATE; /** purpose for multiple data sets */
    private AnalysisType                    geAnalysisType=AnalysisType.PURELYSPATIAL; /** analysis type */
    private ProbabilityModelType            geProbabilityModelType=ProbabilityModelType.POISSON; /** probability model type */
    private AreaRateType                    geAreaScanRate=AreaRateType.HIGH; /** areas incidence rate type of interest */
    private RiskType                        geRiskFunctionType=RiskType.STANDARDRISK; /**  */
    private IncludeClustersType             geIncludeClustersType=IncludeClustersType.ALLCLUSTERS;
    private int                             giReplications=999; /** number of MonteCarlo replicas */
    private CriteriaSecondaryClustersType   geCriteriaSecondClustersType=CriteriaSecondaryClustersType.NOGEOOVERLAP; /** Criteria for Reporting Secondary Clusters */
    private double                          gdTimeTrendConverge=0.0000001; /** time trend convergence value for SVTT */
    private SimulationType                  geSimulationType=SimulationType.STANDARD; /** indicates simulation procedure - Poisson only */
    private boolean                         gbOutputSimulationData=false; /** indicates whether to output simulation data to file */
    private boolean                         gbAdjustForEarlierAnalyses=false; /** indicates whether to adjust for earlier analyses,
                                                                                  pertinent for prospective analyses */
    private SpatialAdjustmentType           geSpatialAdjustmentType=SpatialAdjustmentType.NO_SPATIAL_ADJUSTMENT; /** type of spatial adjustment*/
    private StudyPeriodDataCheckingType     geStudyPeriodDataCheckingType=StudyPeriodDataCheckingType.STRICTBOUNDS; /** study period data checking type */
    private CoordinatesDataCheckingType     geCoordinatesDataCheckingType=CoordinatesDataCheckingType.STRICTCOORDINATES; /** geographical coordinates data checking type */
    private MultipleCoordinatesType         geMultipleCoordinatesType=MultipleCoordinatesType.ONEPERLOCATION; /** multiple locations type */
    /* PValue Reporting variables */
    private PValueReportingType             gePValueReportingType=PValueReportingType.DEFAULT_PVALUE; /** PValue reporting type */
    private int                             giEarlyTermThreshold=50; /** early termination threshold */
    private boolean                         gbReportGumbelPValue=false;                   /** report Gumbel p-value */

    /* Maximum spatial cluster variables */
    private double                          gdMaxSpatialSizeInPopulationAtRisk=50.0;
    private boolean                         gbRestrictMaxSpatialSizeThroughMaxCirclePopulationFile=false;
    private double                          gdMaxSpatialSizeInMaxCirclePopulationFile=50.0;
    private boolean                         gbRestrictMaxSpatialSizeThroughDistanceFromCenter=false;
    private double                          gdMaxSpatialSizeInMaxDistanceFromCenter=1.0;
       /* Reported Maximum spatial cluster variables */
    private boolean                         gbRestrictReportedClusters=false;             /** indicates whether reported clusters are limited to specified maximum size */
    private double                          gdMaxSpatialSizeInPopulationAtRisk_Reported=50.0;
    private boolean                         gbRestrictMaxSpatialSizeThroughMaxCirclePopulationFile_Reported=false;
    private double                          gdMaxSpatialSizeInMaxCirclePopulationFile_Reported=50.0;
    private boolean                         gbRestrictMaxSpatialSizeThroughDistanceFromCenter_Reported=false;
    private double                          gdMaxSpatialSizeInMaxDistanceFromCenter_Reported=1.0;
    /* Maximum temporal cluster variables */
    private double                          gdMaxTemporalClusterSize=50.0; /** maximum value for temporal cluster */
    private TemporalSizeType                geMaxTemporalClusterSizeType=TemporalSizeType.PERCENTAGETYPE; /** maximum temporal cluster value type */
    private boolean                         _adjustWeeklyTrends=false;
        /* Time interval variables */
    private int                             glTimeAggregationLength=0; /** length of time aggregation */
    private DatePrecisionType               geTimeAggregationUnitsType=DatePrecisionType.NONE; /** time aggregation units type */
        /* Temporal trend adjusment variables */
    private double                          gdTimeTrendAdjustPercentage=0; /** percentage for log linear adjustment */
    private TimeTrendAdjustmentType         geTimeTrendAdjustType=TimeTrendAdjustmentType.NOTADJUSTED; /** Adjust for time trend: no, discrete, % */
        /* Input precision variables */
    private DatePrecisionType               gePrecisionOfTimesType=DatePrecisionType.YEAR; /** precision of case/control data: none = no, years=months=days = yes */
    private CoordinatesType                 geCoordinatesType=CoordinatesType.LATLON; /** coordinates type for coordinates/special grid */
        /* Ellipse variables */
    private Vector<Double>                  gvEllipseShapes; /** shape of each ellipsoid */
    private Vector<Integer>                 gvEllipseRotations; /** number of rotations for each ellipsoid */
    private NonCompactnessPenaltyType       geNonCompactnessPenaltyType=NonCompactnessPenaltyType.MEDIUMPENALTY; /** indicates penalty for narrower ellipses */
        /* Pure Clusters variables */
    private boolean                         gbIncludePurelySpatialClusters=false, /** indicates whether to include purely spatial clusters */
                                            gbIncludePurelyTemporalClusters=false; /** indicates whether to include purely temporal clusters */
        /* Additional output variables */
    private boolean                         gbOutputSimLogLikeliRatiosAscii=false, /** indicates whether to output simulated log likelihood ratios in ASCII format */
                                            gbOutputSimLogLikeliRatiosDBase=false; /** indicates whether to output simulated log likelihood ratios in dBase format */
    private boolean                         gbOutputRelativeRisksAscii=false, /** indicates whether to output relative risks for each tract/location in ASCII format */
                                            gbOutputRelativeRisksDBase=false; /** indicates whether to output relative risks for each tract/location in dBase format */
    private boolean                         gbOutputClusterLevelAscii=false, /** indicates whether to output most likely clusters for each centroid in ASCII format */
                                            gbOutputClusterLevelDBase=false; /** indicates whether to output most likely clusters for each centroid in dBase format */
    private boolean                         gbOutputClusterCaseAscii=false, /** indicates whether to output most likely cluster cases for each centroid in ASCII format */
                                            gbOutputClusterCaseDBase=false; /** indicates whether to output most likely cluster cases for each centroid in dBase format */
    private boolean                         gbOutputAreaSpecificAscii=false, /** indicates whether to output tract/location information of reported(.i.e top ranked) clusters in ASCII format */
                                            gbOutputAreaSpecificDBase=false; /** indicates whether to output tract/location information of reported(.i.e top ranked) clusters in dBase format */
    private boolean                         _output_kml=false; /* report google earth kml file */
    private boolean                         _output_temporal_graph=false; /* report temporal graph file */
    private boolean                         _include_locations_kml=false; /** include cluster locations in kml output */
    private boolean                         _compress_kml_output=false; /** compress kml output into kmz format */
    private boolean                         _launch_kml_viewer=true; /* whether to launch kml viewer */
    private boolean                         _output_shapefiles=false;

    /* Iterative scans variables */
    private boolean                         gbIterativeRuns=false; /* Iterative analysis? */
    private int                             giNumIterativeRuns=0; /* number of Iterative scans to attempt */
    private double                          gbIterativeCutOffPValue=0.05; /* P-Value used to exit Iterative analysis */
        /* Input/Output filenames */
    private String                          gsParametersSourceFileName=""; /** parameters source filename */
    private Vector<String>                  gvCaseFilenames; /** case data source filenames */
    private Vector<String>                  gvControlFilenames; /** control data source filenames */
    private Vector<String>                  gvPopulationFilenames; /** population data source filenames */
    private String                          gsCoordinatesFileName=""; /** coordinates data source filename */
    private String                          gsSpecialGridFileName=""; /** special grid data source filename */
    private String                          gsMaxCirclePopulationFileName=""; /** special population file for constructing circles only */
    private String                          gsOutputFileName=""; /** results output filename */
    private boolean                         gbLogRunHistory=true; /** indicates whether to log history */
    private String                          gsSimulationDataSourceFileName=""; /** simulation data source filename */
    private boolean                         gbUseAdjustmentsForRRFile=false; /** indicates whether to use adjustments for known relative risks file */
    private String                          gsAdjustmentsByRelativeRisksFileName=""; /** adjustments by known relative risks filename */
    private String                          gsSimulationDataOutputFilename=""; /** simulation data output filename */
    private String                          gsLocationNeighborsFilename=""; /** sorted array neighbor file */
    boolean                                 gbUseLocationNeighborsFile; /** use sorted array neighbor file? */
    private String                          gsMetaLocationsFilename=""; /** meta locations file */
    boolean                                 gbUseMetaLocationsFile=false;
        /* Analysis dates */
    private String                          gsProspectiveStartDate="2000/12/31"; /** prospective start date in YYYY/MM/DD, YYYY/MM, or YYYY format */
    private String                          gsStudyPeriodStartDate="2000/1/1"; /** study period start date in YYYY/MM/DD, YYYY/MM, or YYYY format */
    private String                          gsStudyPeriodEndDate="2000/12/31"; /** study period end date in YYYY/MM/DD, YYYY/MM, or YYYY format */
    private String                          gsEndRangeStartDate="2000/1/1";
    private String                          gsEndRangeEndDate="2000/12/31";
    private String                          gsStartRangeStartDate="2000/1/1";
    private String                          gsStartRangeEndDate="2000/12/31";
    private Vector<String>                  gvObservableRegions; /** collection of observable regions */
    private CreationVersion                 gCreationVersion;
    private int                             glRandomizationSeed=12345678; /** randomization seed */
    private boolean                         gbReportCriticalValues=false; /** indicates whether to report critical LLR values */
    private boolean                         gbSuppressWarnings=false; /** indicates whether to suppress warnings printed during execution */
    private SpatialWindowType               geSpatialWindowType=SpatialWindowType.CIRCULAR; /** spatial window shape */
    private TimeTrendType                   geTimeTrendType=TimeTrendType.LINEAR;                        /** time trend type */
    private boolean                         gbReportRank=false;  /** report cluster rank */
    private boolean                         gbPrintAsciiHeaders=false;  /** print ASCII column headers */
    private double                          _giniIndexPValueCutoff=0.05; /* P-Value used to limit clusters in gini index coefficients calcuation */

    private boolean                         _reportHierarchicalClusters=true;  /** print ASCII column headers */
    private boolean                         _reportGiniOptimizedClusters=true;  /** print ASCII column headers */
    GiniIndexReportType                     _giniIndexReportType=GiniIndexReportType.OPTIMAL_ONLY; /* type for the gini index cluster reporting */
    boolean                                 _outputGiniIndexCoefficients=false; /* output gini index coefficents */

    /* Power Evaluation variables */
    private boolean                         _performPowerEvaluation=false; /** indicator of whether to perform power calculations */
    CriticalValuesType                      _critical_value_type=CriticalValuesType.CV_MONTECARLO;
    PowerEstimationType                     _power_estimation_type=PowerEstimationType.PE_MONTECARLO;
    PowerEvaluationMethodType               _power_evaluation_method=PowerEvaluationMethodType.PE_WITH_ANALYSIS;
    private String                          _power_alt_hypothesis_filename=""; /** power evaluation alternative filename */
    private int                             _powerEvaluationTotalCases; /* number cases in power evaluation, user specified */
    private int                             _power_replica; /* number of replications in power step of power evaluations */
    
    public static final int                 MAXIMUM_ITERATIVE_ANALYSES=32000; /** maximum number of permitted iterative scans */
    public static final int                 MAXIMUM_ELLIPSOIDS=10; /** maximum number of permitted ellipsoids */

    public Parameters() {
      super();

      gCreationVersion = new CreationVersion(7,0,0);
      gvCaseFilenames = new Vector<String>();
      gvCaseFilenames.addElement("");
      gvControlFilenames = new Vector<String>();
      gvControlFilenames.addElement("");
      gvPopulationFilenames = new Vector<String>();
      gvPopulationFilenames.addElement("");
      gvEllipseShapes = new Vector<Double>();
      gvEllipseShapes.addElement(1.5);
      gvEllipseShapes.addElement(2.0);
      gvEllipseShapes.addElement(3.0);
      gvEllipseShapes.addElement(4.0);
      gvEllipseShapes.addElement(5.0);
      gvEllipseRotations = new Vector<Integer>();
      gvEllipseRotations.addElement(4);
      gvEllipseRotations.addElement(6);
      gvEllipseRotations.addElement(9);
      gvEllipseRotations.addElement(12);
      gvEllipseRotations.addElement(15);
      gvObservableRegions = new Vector<String>();
    }

    @Override
    public Object clone() {
        try {
            Parameters newObject = (Parameters)super.clone();
            newObject.gsStudyPeriodStartDate = new String(gsStudyPeriodStartDate);
            newObject.gsStudyPeriodEndDate = new String(gsStudyPeriodEndDate);
            newObject.gvCaseFilenames = new Vector<String>(gvCaseFilenames);
            newObject.gvControlFilenames = new Vector<String>(gvControlFilenames);
            newObject.gvPopulationFilenames = new Vector<String>(gvPopulationFilenames);
            newObject.gsCoordinatesFileName = new String(gsCoordinatesFileName);
            newObject.gsSpecialGridFileName = new String(gsSpecialGridFileName);
            newObject.gsMaxCirclePopulationFileName = new String(gsMaxCirclePopulationFileName);
            newObject.gsOutputFileName = new String(gsOutputFileName);
            newObject.gsProspectiveStartDate = new String(gsProspectiveStartDate);
            newObject.gsParametersSourceFileName = new String(gsParametersSourceFileName);
            newObject.gsEndRangeStartDate = new String(gsEndRangeStartDate);
            newObject.gsEndRangeEndDate = new String(gsEndRangeEndDate);
            newObject.gsStartRangeStartDate = new String(gsStartRangeStartDate);
            newObject.gsStartRangeEndDate = new String(gsStartRangeEndDate);
            newObject.gsSimulationDataSourceFileName = new String(gsSimulationDataSourceFileName);
            newObject.gsAdjustmentsByRelativeRisksFileName = new String(gsAdjustmentsByRelativeRisksFileName);
            newObject.gsSimulationDataOutputFilename = new String(gsSimulationDataOutputFilename);
            newObject.gsLocationNeighborsFilename = new String(gsLocationNeighborsFilename);
            newObject.gsMetaLocationsFilename = new String(gsMetaLocationsFilename);
            newObject.gvObservableRegions = new Vector<String>(gvObservableRegions);
            return newObject;
        } catch (CloneNotSupportedException e) {
            throw new InternalError("But we are Cloneable!!!");
        }
    }
    
    public boolean getOutputShapeFiles() {return _output_shapefiles;}
    public void setOutputShapeFiles(boolean b) {_output_shapefiles = b;}    
    public boolean getLaunchKMLViewer() {return _launch_kml_viewer;}
    public void setLaunchKMLViewer(boolean b) {_launch_kml_viewer = b;}
    public boolean getIncludeLocationsKML() {return _include_locations_kml;}
    public void setIncludeLocationsKML(boolean b) {_include_locations_kml = b;}
    public boolean getCompressClusterKML() {return _compress_kml_output;}
    public void setCompressClusterKML(boolean b) {_compress_kml_output = b;}
    public boolean getOutputKMLFile() {return _output_kml;}
    public void setOutputKMLFile(boolean b) {_output_kml = b;}
    public boolean getOutputTemporalGraphFile() {return _output_temporal_graph;}
    public void setOutputTemporalGraphFile(boolean b) {_output_temporal_graph = b;}
    public MultipleCoordinatesType GetMultipleCoordinatesType() {return geMultipleCoordinatesType;}
    public PValueReportingType GetPValueReportingType() {return gePValueReportingType;}
    public int GetEarlyTermThreshold() {return giEarlyTermThreshold;}
    public void SetEarlyTermThreshold(int i) {giEarlyTermThreshold = i;}
    public boolean GetReportGumbelPValue() {return gbReportGumbelPValue;}
    public void SetReportGumbelPValue(boolean b) {gbReportGumbelPValue = b;}
    /** Add ellipsoid shape to collection of spatial shapes evaluated. */
    public void  AddEllipsoidShape(double dShape, boolean bEmptyFirst) {
        if (bEmptyFirst) gvEllipseShapes.setSize(0);
        gvEllipseShapes.add(new Double(dShape));
    }
    /** Add ellipsoid rotations to collection of spatial shapes evaluated. */
    public void AddEllipsoidRotations(int iRotations, boolean bEmptyFirst) {
        if (bEmptyFirst) gvEllipseRotations.setSize(0);
        gvEllipseRotations.add(new Integer(iRotations));
    }
    /** Add observable region to definition to collection. */
    public void ClearObservableRegions() {
        gvObservableRegions.setSize(0);
    }
    /** Add observable region to definition to collection. */
    public void AddObservableRegion(final String sRegions, int iIndex, boolean bEmptyFirst) {
        if (bEmptyFirst) gvObservableRegions.setSize(0);
        if (iIndex + 1 > gvObservableRegions.size())
            gvObservableRegions.setSize(iIndex + 1);
        gvObservableRegions.setElementAt(sRegions, iIndex);
    }

    @Override
    public boolean equals(Object _rhs) {
        Parameters rhs = (Parameters)_rhs;

        if (geSpatialWindowType                    != rhs.geSpatialWindowType) return false;
        if (gvEllipseShapes                        != rhs.gvEllipseShapes) return false;
        if (gvEllipseRotations                     != rhs.gvEllipseRotations) return false;
        if (geNonCompactnessPenaltyType            != rhs.geNonCompactnessPenaltyType) return false;
        if (geAnalysisType                         != rhs.geAnalysisType) return false;
        if (geAreaScanRate                         != rhs.geAreaScanRate) return false;
        if (geProbabilityModelType                 != rhs.geProbabilityModelType) return false;
        if (geRiskFunctionType                     != rhs.geRiskFunctionType) return false;
        if (giReplications                         != rhs.giReplications) return false;
        if (!gsStudyPeriodStartDate.equals(rhs.gsStudyPeriodStartDate)) return false;
        if (!gsStudyPeriodEndDate.equals(rhs.gsStudyPeriodEndDate)) return false;
        if (gdMaxTemporalClusterSize               != rhs.gdMaxTemporalClusterSize) return false;
        if (geIncludeClustersType                  != rhs.geIncludeClustersType) return false;
        if (geTimeAggregationUnitsType             != rhs.geTimeAggregationUnitsType) return false;
        if (glTimeAggregationLength                != rhs.glTimeAggregationLength) return false;
        if (geTimeTrendAdjustType                  != rhs.geTimeTrendAdjustType) return false;
        if (gdTimeTrendAdjustPercentage            != rhs.gdTimeTrendAdjustPercentage) return false;
        if (gbIncludePurelySpatialClusters         != rhs.gbIncludePurelySpatialClusters) return false;
        if (gbIncludePurelyTemporalClusters        != rhs.gbIncludePurelyTemporalClusters) return false;
        if (!gvCaseFilenames.equals(rhs.gvCaseFilenames)) return false;
        if (!gvControlFilenames.equals(rhs.gvControlFilenames)) return false;
        if (!gvPopulationFilenames.equals(rhs.gvPopulationFilenames)) return false;
        if (!gsCoordinatesFileName.equals(rhs.gsCoordinatesFileName)) return false;
        if (!gsSpecialGridFileName.equals(rhs.gsSpecialGridFileName)) return false;
        if (!gsMaxCirclePopulationFileName.equals(rhs.gsMaxCirclePopulationFileName)) return false;
        if (gePrecisionOfTimesType                 != rhs.gePrecisionOfTimesType) return false;
        if (geCoordinatesType                      != rhs.geCoordinatesType) return false;
        if (!gsOutputFileName.equals(rhs.gsOutputFileName)) return false;
        if (gbOutputSimLogLikeliRatiosAscii        != rhs.gbOutputSimLogLikeliRatiosAscii) return false;
        if (gbOutputRelativeRisksAscii             != rhs.gbOutputRelativeRisksAscii) return false;
        if (gbIterativeRuns                        != rhs.gbIterativeRuns) return false;
        if (giNumIterativeRuns                     != rhs.giNumIterativeRuns) return false;
        if (gbIterativeCutOffPValue                != rhs.gbIterativeCutOffPValue) return false;
        if (!gsProspectiveStartDate.equals(rhs.gsProspectiveStartDate)) return false;
        if (gbOutputAreaSpecificAscii              != rhs.gbOutputAreaSpecificAscii) return false;
        if (gbOutputClusterLevelAscii              != rhs.gbOutputClusterLevelAscii) return false;
        if (geCriteriaSecondClustersType           != rhs.geCriteriaSecondClustersType) return false;
        if (geMaxTemporalClusterSizeType           != rhs.geMaxTemporalClusterSizeType) return false;
        if (gbOutputClusterLevelDBase              != rhs.gbOutputClusterLevelDBase) return false;
        if (gbOutputAreaSpecificDBase              != rhs.gbOutputAreaSpecificDBase) return false;
        if (gbOutputRelativeRisksDBase             != rhs.gbOutputRelativeRisksDBase) return false;
        if (gbOutputSimLogLikeliRatiosDBase        != rhs.gbOutputSimLogLikeliRatiosDBase) return false;
        if (gbLogRunHistory                        != rhs.gbLogRunHistory) return false;
        if (!gsParametersSourceFileName.equals(rhs.gsParametersSourceFileName)) return false;
        if (!gsEndRangeStartDate.equals(rhs.gsEndRangeStartDate)) return false;
        if (!gsEndRangeEndDate.equals(rhs.gsEndRangeEndDate)) return false;
        if (!gsStartRangeStartDate.equals(rhs.gsStartRangeStartDate)) return false;
        if (!gsStartRangeEndDate.equals(rhs.gsStartRangeEndDate)) return false;
        if (gdTimeTrendConverge                != rhs.gdTimeTrendConverge) return false;
        if (gbRestrictReportedClusters             != rhs.gbRestrictReportedClusters) return false;
        if (geSimulationType                       != rhs.geSimulationType) return false;
        if (!gsSimulationDataSourceFileName.equals(rhs.gsSimulationDataSourceFileName)) return false;
        if (!gsAdjustmentsByRelativeRisksFileName.equals(rhs.gsAdjustmentsByRelativeRisksFileName)) return false;
        if (gbOutputSimulationData                 != rhs.gbOutputSimulationData) return false;
        if (!gsSimulationDataOutputFilename.equals(rhs.gsSimulationDataOutputFilename)) return false;
        if (gbAdjustForEarlierAnalyses             != rhs.gbAdjustForEarlierAnalyses) return false;
        if (gbUseAdjustmentsForRRFile              != rhs.gbUseAdjustmentsForRRFile) return false;
        if (geSpatialAdjustmentType                != rhs.geSpatialAdjustmentType) return false;
        if (geMultipleSetPurposeType               != rhs.geMultipleSetPurposeType) return false;
        //if (glRandomizationSeed                    != rhs.glRandomizationSeed) return false;
        if (gbReportCriticalValues                 != rhs.gbReportCriticalValues) return false;
        //if (geExecutionType                        != rhs.geExecutionType) return false;
        if (giNumRequestedParallelProcesses        != rhs.giNumRequestedParallelProcesses) return false;
        if (gbSuppressWarnings                     != rhs.gbSuppressWarnings) return false;
        if (gbOutputClusterCaseAscii               != rhs.gbOutputClusterCaseAscii) return false;
        if (gbOutputClusterCaseDBase               != rhs.gbOutputClusterCaseDBase) return false;
        if (geStudyPeriodDataCheckingType != rhs.geStudyPeriodDataCheckingType) return false;
        if (geCoordinatesDataCheckingType != rhs.geCoordinatesDataCheckingType) return false;
        if (gdMaxSpatialSizeInPopulationAtRisk != rhs.gdMaxSpatialSizeInPopulationAtRisk) return false;
        if (gbRestrictMaxSpatialSizeThroughMaxCirclePopulationFile != rhs.gbRestrictMaxSpatialSizeThroughMaxCirclePopulationFile) return false;
        if (gdMaxSpatialSizeInMaxCirclePopulationFile != rhs.gdMaxSpatialSizeInMaxCirclePopulationFile) return false;
        if (gbRestrictMaxSpatialSizeThroughDistanceFromCenter != rhs.gbRestrictMaxSpatialSizeThroughDistanceFromCenter) return false;
        if (gdMaxSpatialSizeInMaxDistanceFromCenter != rhs.gdMaxSpatialSizeInMaxDistanceFromCenter) return false;
        if (gdMaxSpatialSizeInPopulationAtRisk_Reported != rhs.gdMaxSpatialSizeInPopulationAtRisk_Reported) return false;
        if (gbRestrictMaxSpatialSizeThroughMaxCirclePopulationFile_Reported != rhs.gbRestrictMaxSpatialSizeThroughMaxCirclePopulationFile_Reported) return false;
        if (gdMaxSpatialSizeInMaxCirclePopulationFile_Reported != rhs.gdMaxSpatialSizeInMaxCirclePopulationFile_Reported) return false;
        if (gbRestrictMaxSpatialSizeThroughDistanceFromCenter_Reported != rhs.gbRestrictMaxSpatialSizeThroughDistanceFromCenter_Reported) return false;
        if (gdMaxSpatialSizeInMaxDistanceFromCenter_Reported != rhs.gdMaxSpatialSizeInMaxDistanceFromCenter_Reported) return false;
        if (!gsLocationNeighborsFilename.equals(rhs.gsLocationNeighborsFilename)) return false;
        if (gbUseLocationNeighborsFile != rhs.gbUseLocationNeighborsFile) return false;
        if (geMultipleCoordinatesType != rhs.geMultipleCoordinatesType) return false;
        if (!gsMetaLocationsFilename.equals(rhs.gsMetaLocationsFilename)) return false;
        if (gbUseMetaLocationsFile != rhs.gbUseMetaLocationsFile) return false;
        if (!gvObservableRegions.equals(rhs.gvObservableRegions)) return false;
        if (gePValueReportingType != rhs.gePValueReportingType) return false;
        if (giEarlyTermThreshold != rhs.giEarlyTermThreshold) return false;
        if (gbReportGumbelPValue != rhs.gbReportGumbelPValue) return false;
        if (gbReportGumbelPValue != rhs.gbReportGumbelPValue) return false;
        if (geTimeTrendType != rhs.geTimeTrendType) return false;
        if (gbReportRank != rhs.gbReportRank) return false;
        if (gbPrintAsciiHeaders != rhs.gbPrintAsciiHeaders) return false;
        if (_giniIndexPValueCutoff != rhs._giniIndexPValueCutoff) return false;
        if (_reportHierarchicalClusters != rhs._reportHierarchicalClusters) return false;
        if (_reportGiniOptimizedClusters != rhs._reportGiniOptimizedClusters) return false;
        if (_giniIndexReportType != rhs._giniIndexReportType) return false;
        if (_outputGiniIndexCoefficients != rhs._outputGiniIndexCoefficients) return false;
        if (_performPowerEvaluation != rhs._performPowerEvaluation) return false;
        if (_critical_value_type != rhs._critical_value_type) return false;
        if (_power_estimation_type != rhs._power_estimation_type) return false;
        if (!_power_alt_hypothesis_filename.equals(rhs._power_alt_hypothesis_filename)) return false;
        if (_powerEvaluationTotalCases != rhs._powerEvaluationTotalCases) return false;
        if (_power_replica != rhs._power_replica) return false;
        if (_output_kml != rhs._output_kml) return false;
        if (_output_temporal_graph != rhs._output_temporal_graph) return false;
        if (_adjustWeeklyTrends != rhs._adjustWeeklyTrends) return false;
        if (_include_locations_kml != rhs._include_locations_kml) return false;
        if (_compress_kml_output != rhs._compress_kml_output) return false;
        if (_launch_kml_viewer != rhs._launch_kml_viewer) return false;  
        if (_output_shapefiles != rhs._output_shapefiles) return false;
        
        return true;
    }
    public boolean getAdjustForWeeklyTrends() {return _adjustWeeklyTrends;}
    public void setAdjustForWeeklyTrends(boolean b) {_adjustWeeklyTrends = b;}
    public boolean getPerformPowerEvaluation() {return _performPowerEvaluation;}
    public void setPerformPowerEvaluation(boolean b) {_performPowerEvaluation = b;}
    public CriticalValuesType getPowerEvaluationCriticalValueType() {return _critical_value_type;}
    public void setPowerEvaluationCriticalValueType(int iOrdinal) {
        try { _critical_value_type = CriticalValuesType.values()[iOrdinal];
        } catch (ArrayIndexOutOfBoundsException e) { ThrowOrdinalIndexException(iOrdinal, _critical_value_type.values()); }
    }
    public PowerEstimationType getPowerEstimationType() {return _power_estimation_type;}
    public void setPowerEstimationType(int iOrdinal) {
        try { _power_estimation_type = PowerEstimationType.values()[iOrdinal];
        } catch (ArrayIndexOutOfBoundsException e) { ThrowOrdinalIndexException(iOrdinal, _power_estimation_type.values()); }
    }
    public PowerEvaluationMethodType getPowerEvaluationMethod() {return _power_evaluation_method;}
    public void setPowerEvaluationMethod(int iOrdinal) {
        try { _power_evaluation_method = PowerEvaluationMethodType.values()[iOrdinal];
        } catch (ArrayIndexOutOfBoundsException e) { ThrowOrdinalIndexException(iOrdinal, _power_evaluation_method.values()); }
    }
    public String getPowerEvaluationAltHypothesisFilename() {return _power_alt_hypothesis_filename;}
    public void setPowerEvaluationAltHypothesisFilename(final String s) {_power_alt_hypothesis_filename = s;}
    public int getPowerEvaluationCaseCount() {return _powerEvaluationTotalCases;}
    public void setPowerEvaluationCaseCount(int i) {_powerEvaluationTotalCases = i;}
    public int getNumPowerEvalReplicaPowerStep() {return _power_replica;}
    public void setNumPowerEvalReplicaPowerStep(int i) {_power_replica = i;}

    public double getGiniIndexPValueCutoff() {return _giniIndexPValueCutoff;}
    public void setGiniIndexPValueCutoff(double d) {_giniIndexPValueCutoff = d;}
    public boolean getReportHierarchicalClusters() {return _reportHierarchicalClusters;}
    public void setReportHierarchicalClusters(boolean b) {_reportHierarchicalClusters = b;}
    public boolean getReportGiniOptimizedClusters() {return _reportGiniOptimizedClusters;}
    public void setReportGiniOptimizedClusters(boolean b) {_reportGiniOptimizedClusters = b;}
    public GiniIndexReportType getGiniIndexReportType() {return _giniIndexReportType;}
    public void setGiniIndexReportType(int iOrdinal) {
        try { _giniIndexReportType = GiniIndexReportType.values()[iOrdinal];
        } catch (ArrayIndexOutOfBoundsException e) { ThrowOrdinalIndexException(iOrdinal, _giniIndexReportType.values()); }
    }
    public boolean getReportGiniIndexCoefficents() {return _outputGiniIndexCoefficients;}
    public void setReportGiniIndexCoefficents(boolean b) {_outputGiniIndexCoefficients = b;}
    public boolean getPrintAsciiHeaders() {return gbPrintAsciiHeaders;}
    public void setPrintAsciiHeaders(boolean b) {gbPrintAsciiHeaders = b;}
    public boolean GetAdjustForEarlierAnalyses() {return gbAdjustForEarlierAnalyses;}
    public String GetAdjustmentsByRelativeRisksFilename() {return gsAdjustmentsByRelativeRisksFileName;}
    public AnalysisType GetAnalysisType() {return geAnalysisType;}
    public String GetAnalysisTypeAsString() {
        String sAnalysisType = null;
        switch (geAnalysisType) {
            case PURELYSPATIAL             : sAnalysisType = "Purely Spatial"; break;
            case PURELYTEMPORAL            : sAnalysisType = "Retrospective Purely Temporal"; break;
            case SPACETIME                 : sAnalysisType = "Retrospective Space-Time"; break;
            case PROSPECTIVESPACETIME      : sAnalysisType = "Prospective Space-Time"; break;
            case SPATIALVARTEMPTREND       : sAnalysisType = "Spatial Variation in Temporal Trends"; break;
            case PROSPECTIVEPURELYTEMPORAL : sAnalysisType = "Prospective Purely Temporal"; break;
          }
        return sAnalysisType;
    }
    public AreaRateType GetAreaScanRateType() {return geAreaScanRate;}
    public final Vector<String> GetObservableRegions() {return gvObservableRegions;}
    public final String GetCaseFileName(int iSetIndex/*=1*/) {
        return gvCaseFilenames.elementAt(iSetIndex - 1);
    }
    public final Vector<String> GetCaseFileNames() {return gvCaseFilenames;}
    public final String GetControlFileName(int iSetIndex/*=1*/) {
      return gvControlFilenames.elementAt(iSetIndex - 1);
    }
    public final Vector<String> GetControlFileNames() {return gvControlFilenames;}
    public final String GetCoordinatesFileName() {return gsCoordinatesFileName;}
    public CoordinatesDataCheckingType GetCoordinatesDataCheckingType() {return geCoordinatesDataCheckingType;}
    public CoordinatesType GetCoordinatesType() {return geCoordinatesType;}
    public final CreationVersion GetCreationVersion() {return gCreationVersion;}
    public CriteriaSecondaryClustersType GetCriteriaSecondClustersType() {return geCriteriaSecondClustersType;}
    public int GetCreationVersionMajor() {return gCreationVersion.giMajor;}
    public final Vector<Integer> GetEllipseRotations() {return gvEllipseRotations;}
    public final Vector<Double> GetEllipseShapes() {return gvEllipseShapes;}
    public final String GetEndRangeEndDate() {return gsEndRangeEndDate;}
    public final String GetEndRangeStartDate() {return gsEndRangeStartDate;}
    public ExecutionType GetExecutionType() {return geExecutionType;}
    public IncludeClustersType GetIncludeClustersType() {return geIncludeClustersType;}
    public boolean GetIncludePurelySpatialClusters() {return gbIncludePurelySpatialClusters;}
    public boolean GetIncludePurelyTemporalClusters() {return gbIncludePurelyTemporalClusters;}
    public boolean GetIsLoggingHistory() {return gbLogRunHistory;}
    /** Returns whether analysis is a prospective analysis. */
    public boolean GetIsProspectiveAnalysis() {
        return (geAnalysisType == AnalysisType.PROSPECTIVESPACETIME || geAnalysisType == AnalysisType.PROSPECTIVEPURELYTEMPORAL);
    }
    /** Returns whether analysis is purely temporal. */
    public boolean GetIsPurelyTemporalAnalysis() {
        return (geAnalysisType == AnalysisType.PURELYTEMPORAL || geAnalysisType == AnalysisType.PROSPECTIVEPURELYTEMPORAL);
    }
    public boolean GetIsIterativeScanning() {return gbIterativeRuns;}
    /** Returns whether analysis is space-time. */
    public boolean GetIsSpaceTimeAnalysis() {
        return (geAnalysisType == AnalysisType.SPACETIME || geAnalysisType == AnalysisType.PROSPECTIVESPACETIME);
    }
    public String GetLocationNeighborsFileName() {return gsLocationNeighborsFilename;}
    public String GetMetaLocationsFileName() {return gsMetaLocationsFilename;}
    public final String GetMaxCirclePopulationFileName() {return gsMaxCirclePopulationFileName;}
    public double GetMaxSpatialSizeForType(int iOrdinal, boolean bReported) {
        SpatialSizeType eSpatialSizeType=SpatialSizeType.PERCENTOFPOPULATION;
        try { eSpatialSizeType=SpatialSizeType.values()[iOrdinal];
        } catch (ArrayIndexOutOfBoundsException e) {
            ThrowOrdinalIndexException(iOrdinal, SpatialSizeType.values());
        }
        switch (eSpatialSizeType) {
            case PERCENTOFPOPULATION    : return bReported ? gdMaxSpatialSizeInPopulationAtRisk_Reported : gdMaxSpatialSizeInPopulationAtRisk;
            case MAXDISTANCE            : return bReported ? gdMaxSpatialSizeInMaxDistanceFromCenter_Reported : gdMaxSpatialSizeInMaxDistanceFromCenter;
            case PERCENTOFMAXCIRCLEFILE : return bReported ? gdMaxSpatialSizeInMaxCirclePopulationFile_Reported : gdMaxSpatialSizeInMaxCirclePopulationFile;
            default : throw new UnknownEnumException(eSpatialSizeType);
        }
    }
    public double GetMaximumTemporalClusterSize() {return gdMaxTemporalClusterSize;}
    public TemporalSizeType GetMaximumTemporalClusterSizeType() {return geMaxTemporalClusterSizeType;}
    public MultipleDataSetPurposeType GetMultipleDataSetPurposeType() {return geMultipleSetPurposeType;}
    public NonCompactnessPenaltyType GetNonCompactnessPenaltyType() {return geNonCompactnessPenaltyType;}
    public int GetNumDataSets() {return gvCaseFilenames.size();}
    public int GetNumRequestedParallelProcesses() {return giNumRequestedParallelProcesses;}
    public int GetNumReplicationsRequested() {return giReplications;}
    public int GetNumRequestedEllipses() {return 5;/*gvEllipseShapes.size();*/}
    public int GetNumIterativeScansRequested() {return giNumIterativeRuns;}
    public boolean GetOutputAreaSpecificAscii() {return gbOutputAreaSpecificAscii;}
    public boolean GetOutputAreaSpecificDBase() {return gbOutputAreaSpecificDBase;}
    public boolean GetOutputClusterLevelAscii() {return gbOutputClusterLevelAscii;}
    public boolean GetOutputClusterLevelDBase() {return gbOutputClusterLevelDBase;}
    public boolean GetOutputClusterCaseAscii() {return gbOutputClusterCaseAscii;}
    public boolean GetOutputClusterCaseDBase() {return gbOutputClusterCaseDBase;}
    public final String GetOutputFileName() {return gsOutputFileName; }
    public boolean GetOutputRelativeRisksAscii() {return gbOutputRelativeRisksAscii;}
    public boolean GetOutputRelativeRisksDBase() {return gbOutputRelativeRisksDBase;}
    public boolean GetOutputSimLoglikeliRatiosAscii() {return gbOutputSimLogLikeliRatiosAscii;}
    public boolean GetOutputSimLoglikeliRatiosDBase() {return gbOutputSimLogLikeliRatiosDBase;}
    public boolean GetOutputSimulationData() {return gbOutputSimulationData;}
    public boolean GetPermitsPurelySpatialCluster(ProbabilityModelType eModelType) {
        return geAnalysisType == AnalysisType.PURELYSPATIAL || geAnalysisType == AnalysisType.SPACETIME || GetIsProspectiveAnalysis();
    }
    public boolean GetPermitsPurelySpatialCluster() {
        return geAnalysisType == AnalysisType.PURELYSPATIAL || geAnalysisType == AnalysisType.SPACETIME || GetIsProspectiveAnalysis();
    }
    public boolean GetPermitsPurelyTemporalCluster(ProbabilityModelType eModelType) {
        return geAnalysisType == AnalysisType.PURELYTEMPORAL || geAnalysisType == AnalysisType.SPACETIME || GetIsProspectiveAnalysis();
    }
    public boolean GetPermitsPurelyTemporalCluster() {
        return geAnalysisType == AnalysisType.PURELYTEMPORAL || geAnalysisType == AnalysisType.SPACETIME || GetIsProspectiveAnalysis();
    }
    public final String GetPopulationFileName(int iSetIndex/*=1*/) {
        return gvPopulationFilenames.elementAt(iSetIndex - 1);
    }
    public final Vector<String> GetPopulationFileNames() {return gvPopulationFilenames;}
    public DatePrecisionType GetPrecisionOfTimesType() {return gePrecisionOfTimesType;}
    public ProbabilityModelType GetProbabilityModelType() {return geProbabilityModelType;}
    public static final String GetProbabilityModelTypeAsString(ProbabilityModelType eProbabilityModelType) {
        String sProbabilityModel = null;
        switch (eProbabilityModelType) {
            case POISSON              : sProbabilityModel = "Poisson"; break;
            case BERNOULLI            : sProbabilityModel = "Bernoulli"; break;
            case SPACETIMEPERMUTATION : sProbabilityModel = "Space-Time Permutation"; break;
            case ORDINAL              : sProbabilityModel = "Ordinal"; break;
            case EXPONENTIAL          : sProbabilityModel = "Exponential"; break;
            case NORMAL               : sProbabilityModel = "Normal"; break;
            case RANK                 : sProbabilityModel = "Rank"; break;
            case HOMOGENEOUSPOISSON   : sProbabilityModel = "Homogeneous Poisson"; break;
            case CATEGORICAL          : sProbabilityModel = "Multinomial"; break;
        }
        return sProbabilityModel;
    }
    public final String GetProspectiveStartDate() {return gsProspectiveStartDate;}
    public int GetRandomizationSeed() {return glRandomizationSeed;}
    public boolean GetReportCriticalValues() {return gbReportCriticalValues;}
    public boolean getReportClusterRank() {return gbReportRank;}
    public boolean GetRestrictingMaximumReportedGeoClusterSize() {return gbRestrictReportedClusters;}
    public boolean GetRestrictMaxSpatialSizeForType(int iOrdinal, boolean bReported) {
        SpatialSizeType eSpatialSizeType=SpatialSizeType.PERCENTOFPOPULATION;
        try { eSpatialSizeType=SpatialSizeType.values()[iOrdinal];
        } catch (ArrayIndexOutOfBoundsException e) { ThrowOrdinalIndexException(iOrdinal, SpatialSizeType.values()); }
        switch (eSpatialSizeType) {
            case PERCENTOFPOPULATION    : return true;
            case MAXDISTANCE            : return bReported ? gbRestrictMaxSpatialSizeThroughDistanceFromCenter_Reported : gbRestrictMaxSpatialSizeThroughDistanceFromCenter;
            case PERCENTOFMAXCIRCLEFILE : return bReported ? gbRestrictMaxSpatialSizeThroughMaxCirclePopulationFile_Reported : gbRestrictMaxSpatialSizeThroughMaxCirclePopulationFile;
            default : throw new UnknownEnumException(eSpatialSizeType);
        }
    }
    public RiskType GetRiskType() {return geRiskFunctionType;}
    public double GetIterativeCutOffPValue() {return gbIterativeCutOffPValue;}
    public final String GetSimulationDataOutputFilename() {return gsSimulationDataOutputFilename;}
    public final String GetSimulationDataSourceFilename() {return gsSimulationDataSourceFileName;}
    public SimulationType GetSimulationType() {return geSimulationType;}
    public final String GetSourceFileName() {return gsParametersSourceFileName;}
    public SpatialAdjustmentType GetSpatialAdjustmentType() {return geSpatialAdjustmentType;}
    public SpatialWindowType GetSpatialWindowType() {return geSpatialWindowType;}
    public final String GetSpecialGridFileName() {return gsSpecialGridFileName;}
    public final String GetStartRangeEndDate() {return gsStartRangeEndDate;}
    public final String GetStartRangeStartDate() {return gsStartRangeStartDate;}
    public StudyPeriodDataCheckingType GetStudyPeriodDataCheckingType() {return geStudyPeriodDataCheckingType;}
    public final String GetStudyPeriodEndDate() {return gsStudyPeriodEndDate;}
    public final String GetStudyPeriodStartDate() {return gsStudyPeriodStartDate;}
    public boolean GetSuppressingWarnings() {return gbSuppressWarnings;}
    public int GetTimeAggregationLength() {return glTimeAggregationLength;}
    public DatePrecisionType GetTimeAggregationUnitsType() {return geTimeAggregationUnitsType;}
    public double GetTimeTrendAdjustmentPercentage() {return gdTimeTrendAdjustPercentage;}
    public TimeTrendAdjustmentType GetTimeTrendAdjustmentType() {return geTimeTrendAdjustType;}
    public double GetTimeTrendConvergence() {return gdTimeTrendConverge;}
    public void SetAdjustForEarlierAnalyses(boolean b) {gbAdjustForEarlierAnalyses = b;}
    public void SetAdjustmentsByRelativeRisksFilename(String sAdjustmentsByRelativeRisksFileName) {gsAdjustmentsByRelativeRisksFileName = sAdjustmentsByRelativeRisksFileName;}
    public void SetAnalysisType(int iOrdinal) {
        try { geAnalysisType = AnalysisType.values()[iOrdinal];
        } catch (ArrayIndexOutOfBoundsException e) { ThrowOrdinalIndexException(iOrdinal, AnalysisType.values()); }
    }
    public void SetMultipleCoordinatesType(int iOrdinal) {
        try { geMultipleCoordinatesType = MultipleCoordinatesType.values()[iOrdinal];
        } catch (ArrayIndexOutOfBoundsException e) { ThrowOrdinalIndexException(iOrdinal, MultipleCoordinatesType.values()); }
    }
    public void SetPValueReportingType(int iOrdinal) {
        try { gePValueReportingType = PValueReportingType.values()[iOrdinal];
        } catch (ArrayIndexOutOfBoundsException e) { ThrowOrdinalIndexException(iOrdinal, gePValueReportingType.values()); }
    }
    public void SetAreaRateType(int iOrdinal) {
        try { geAreaScanRate = AreaRateType.values()[iOrdinal];
        } catch (ArrayIndexOutOfBoundsException e) { ThrowOrdinalIndexException(iOrdinal, AreaRateType.values()); }
    }
    public void SetEndRangeEndDate(final String sEndRangeEndDate) {gsEndRangeEndDate = sEndRangeEndDate;}
    public void SetEndRangeStartDate(final String sEndRangeStartDate) {gsEndRangeStartDate = sEndRangeStartDate;}
    public void SetExecutionType(int iOrdinal) {
        try { geExecutionType = ExecutionType.values()[iOrdinal];
        } catch (ArrayIndexOutOfBoundsException e) { ThrowOrdinalIndexException(iOrdinal, ExecutionType.values()); }
    }
    public void SetCaseFileName(final String sCaseFileName, int iSetIndex/*=1*/) {
        if (iSetIndex > gvCaseFilenames.size())
            gvCaseFilenames.setSize(iSetIndex);
        gvCaseFilenames.setElementAt(sCaseFileName, iSetIndex - 1);
    }
    public void SetControlFileName(final String sControlFileName, int iSetIndex/*=1*/) {
        if (iSetIndex > gvControlFilenames.size())
            gvControlFilenames.setSize(iSetIndex);
        gvControlFilenames.setElementAt(sControlFileName, iSetIndex - 1);
    }
    public void SetCoordinatesFileName(final String sCoordinatesFileName) {gsCoordinatesFileName = sCoordinatesFileName;}
    public void SetCoordinatesDataCheckingType(int iOrdinal) {
        try { geCoordinatesDataCheckingType = CoordinatesDataCheckingType.values()[iOrdinal];
        } catch (ArrayIndexOutOfBoundsException e) { ThrowOrdinalIndexException(iOrdinal, CoordinatesDataCheckingType.values()); }
    }
    public void SetCoordinatesType(int iOrdinal) {
        try { geCoordinatesType = CoordinatesType.values()[iOrdinal];
        } catch (ArrayIndexOutOfBoundsException e) { ThrowOrdinalIndexException(iOrdinal, CoordinatesType.values()); }
    }
    public void SetCriteriaForReportingSecondaryClusters(int iOrdinal) {
        try { geCriteriaSecondClustersType = CriteriaSecondaryClustersType.values()[iOrdinal];
        } catch (ArrayIndexOutOfBoundsException e) { ThrowOrdinalIndexException(iOrdinal, CriteriaSecondaryClustersType.values()); }
    }
    public void SetIncludeClustersType(int iOrdinal) {
        try { geIncludeClustersType = IncludeClustersType.values()[iOrdinal];
        } catch (ArrayIndexOutOfBoundsException e) { ThrowOrdinalIndexException(iOrdinal, IncludeClustersType.values()); }
    }
    public void SetIncludePurelySpatialClusters(boolean b) {gbIncludePurelySpatialClusters = b;}
    public void SetIncludePurelyTemporalClusters(boolean b) {gbIncludePurelyTemporalClusters = b;}
    public void SetIsLoggingHistory(boolean b) {gbLogRunHistory = b;}
    public void SetLocationNeighborsFileName(final String sLocationNeighborsFilename) {gsLocationNeighborsFilename = sLocationNeighborsFilename;}
    public void SetMetaLocationsFileName(final String sMetaLocationsFilename) {gsMetaLocationsFilename = sMetaLocationsFilename;}
    public void SetMaxCirclePopulationFileName(final String sMaxCirclePopulationFileName) {gsMaxCirclePopulationFileName = sMaxCirclePopulationFileName;}
    public void SetMaxSpatialSizeForType(int iOrdinal, double d, boolean bReported) {
        SpatialSizeType eSpatialSizeType=SpatialSizeType.PERCENTOFPOPULATION;
        try { eSpatialSizeType=SpatialSizeType.values()[iOrdinal];
        } catch (ArrayIndexOutOfBoundsException e) { ThrowOrdinalIndexException(iOrdinal, SpatialSizeType.values()); }
        switch (eSpatialSizeType) {
            case PERCENTOFPOPULATION    : if (bReported) gdMaxSpatialSizeInPopulationAtRisk_Reported = d;
                                          else gdMaxSpatialSizeInPopulationAtRisk = d; break;
            case MAXDISTANCE            : if (bReported)gdMaxSpatialSizeInMaxDistanceFromCenter_Reported = d;
                                          else gdMaxSpatialSizeInMaxDistanceFromCenter = d; break;
            case PERCENTOFMAXCIRCLEFILE : if (bReported) gdMaxSpatialSizeInMaxCirclePopulationFile_Reported = d;
                                          else gdMaxSpatialSizeInMaxCirclePopulationFile = d; break;
        }
    }
    public void SetMaximumTemporalClusterSize(double dMaxTemporalClusterSize) {gdMaxTemporalClusterSize = dMaxTemporalClusterSize;}
    public void SetMaximumTemporalClusterSizeType(int iOrdinal) {
        try { geMaxTemporalClusterSizeType = TemporalSizeType.values()[iOrdinal];
        } catch (ArrayIndexOutOfBoundsException e) { ThrowOrdinalIndexException(iOrdinal, TemporalSizeType.values()); }
    }
    public void SetMultipleDataSetPurposeType(int iOrdinal) {
        try { geMultipleSetPurposeType = MultipleDataSetPurposeType.values()[iOrdinal];
        } catch (ArrayIndexOutOfBoundsException e) { ThrowOrdinalIndexException(iOrdinal, MultipleDataSetPurposeType.values()); }
    }
    public void SetNonCompactnessPenalty(int iOrdinal) {
        try { geNonCompactnessPenaltyType = NonCompactnessPenaltyType.values()[iOrdinal];
        } catch (ArrayIndexOutOfBoundsException e) { ThrowOrdinalIndexException(iOrdinal, NonCompactnessPenaltyType.values()); }
    }
    public void SetNumDataSets(int iNumDataSets) {
        //adjust the number of filenames for case, control, and population
        gvCaseFilenames.setSize(iNumDataSets);
        gvControlFilenames.setSize(iNumDataSets);
        gvPopulationFilenames.setSize(iNumDataSets);
    }
    public void SetNumParallelProcessesToExecute(int i) {giNumRequestedParallelProcesses = i;}
    public void SetNumberMonteCarloReplications(int iReplications) {giReplications = iReplications;}
    public void SetNumIterativeScans(int iNumIterativeScans) {giNumIterativeRuns = iNumIterativeScans;}
    public void SetOutputAreaSpecificAscii(boolean b) {gbOutputAreaSpecificAscii = b;}
    public void SetOutputAreaSpecificDBase(boolean b) {gbOutputAreaSpecificDBase = b;}
    public void SetOutputClusterLevelAscii(boolean b) {gbOutputClusterLevelAscii = b;}
    public void SetOutputClusterLevelDBase(boolean b) {gbOutputClusterLevelDBase = b;}
    public void SetOutputClusterCaseAscii(boolean b) {gbOutputClusterCaseAscii = b;}
    public void SetOutputClusterCaseDBase(boolean b) {gbOutputClusterCaseDBase = b;}
    public void SetOutputFileName(final String  sOutPutFileName) {gsOutputFileName = sOutPutFileName;}
    public void SetOutputRelativeRisksAscii(boolean b) {gbOutputRelativeRisksAscii = b;}
    public void SetOutputRelativeRisksDBase(boolean b) {gbOutputRelativeRisksDBase = b;}
    public void SetOutputSimLogLikeliRatiosAscii(boolean b) {gbOutputSimLogLikeliRatiosAscii = b;}
    public void SetOutputSimLogLikeliRatiosDBase(boolean b) {gbOutputSimLogLikeliRatiosDBase = b;}
    public void SetOutputSimulationData(boolean b) {gbOutputSimulationData = b;}
    public void SetPopulationFileName(final String  sPopulationFileName, int iSetIndex/*=1*/) {
        if (iSetIndex > gvPopulationFilenames.size())
        gvPopulationFilenames.setSize(iSetIndex);
        gvPopulationFilenames.setElementAt(sPopulationFileName, iSetIndex - 1);
    }
    public void SetPrecisionOfTimesType(int iOrdinal) {
        try { gePrecisionOfTimesType = DatePrecisionType.values()[iOrdinal];
        } catch (ArrayIndexOutOfBoundsException e) { ThrowOrdinalIndexException(iOrdinal, DatePrecisionType.values()); }
    }
    public void SetProbabilityModelType(int iOrdinal) {
        try { geProbabilityModelType = ProbabilityModelType.values()[iOrdinal];
        } catch (ArrayIndexOutOfBoundsException e) { ThrowOrdinalIndexException(iOrdinal, ProbabilityModelType.values()); }
    }
    public void SetProspectiveStartDate(final String  sProspectiveStartDate) {gsProspectiveStartDate = sProspectiveStartDate;}
    public void SetRandomizationSeed(int lSeed) {glRandomizationSeed = lSeed;}
    public void SetReportCriticalValues(boolean b) {gbReportCriticalValues = b;}
    public void setReportClusterRank(boolean b) {gbReportRank = b;}
    public void SetRestrictReportedClusters(boolean b) {gbRestrictReportedClusters = b;}
    public void SetRestrictMaxSpatialSizeForType(int iOrdinal, boolean b, boolean bReported) {
        SpatialSizeType eSpatialSizeType=SpatialSizeType.PERCENTOFPOPULATION;
        try { eSpatialSizeType=SpatialSizeType.values()[iOrdinal];
        } catch (ArrayIndexOutOfBoundsException e) { ThrowOrdinalIndexException(iOrdinal, SpatialSizeType.values()); }
        switch (eSpatialSizeType) {
            case PERCENTOFPOPULATION    : break;
            case MAXDISTANCE            : if (bReported) gbRestrictMaxSpatialSizeThroughDistanceFromCenter_Reported = b;
                                          else gbRestrictMaxSpatialSizeThroughDistanceFromCenter = b; break;
            case PERCENTOFMAXCIRCLEFILE : if (bReported) gbRestrictMaxSpatialSizeThroughMaxCirclePopulationFile_Reported = b;
                                          else gbRestrictMaxSpatialSizeThroughMaxCirclePopulationFile = b; break;
            default : throw new UnknownEnumException(eSpatialSizeType);
        }
    }
    public void SetRiskType(int iOrdinal) {
        try { geRiskFunctionType = RiskType.values()[iOrdinal];
        } catch (ArrayIndexOutOfBoundsException e) { ThrowOrdinalIndexException(iOrdinal, RiskType.values()); }
    }
    public void SetIterativeCutOffPValue(double dPValue) {gbIterativeCutOffPValue = dPValue;}
    public void SetIterativeScanning(boolean b) {gbIterativeRuns = b;}
    public void SetSimulationDataOutputFileName(final String sSourceFileName) {gsSimulationDataOutputFilename = sSourceFileName;}
    public void SetSimulationDataSourceFileName(final String sSourceFileName) {gsSimulationDataSourceFileName = sSourceFileName;}
    public void SetSimulationType(int iOrdinal) {
        try { geSimulationType = SimulationType.values()[iOrdinal];
        } catch (ArrayIndexOutOfBoundsException e) { ThrowOrdinalIndexException(iOrdinal, SimulationType.values()); }
    }
    public void SetSourceFileName(final String sParametersSourceFileName) {gsParametersSourceFileName = sParametersSourceFileName;}
    public void SetSpatialAdjustmentType(int iOrdinal) {
        try { geSpatialAdjustmentType = SpatialAdjustmentType.values()[iOrdinal];
        } catch (ArrayIndexOutOfBoundsException e) { ThrowOrdinalIndexException(iOrdinal, SpatialAdjustmentType.values()); }
    }
    public void SetSpatialWindowType(int iOrdinal) {
        try { geSpatialWindowType = SpatialWindowType.values()[iOrdinal];
        } catch (ArrayIndexOutOfBoundsException e) { ThrowOrdinalIndexException(iOrdinal, SpatialWindowType.values()); }
    }
    public TimeTrendType getTimeTrendType() {return geTimeTrendType;}
    public void SetTimeTrendType(int iOrdinal) {
        try { geTimeTrendType = TimeTrendType.values()[iOrdinal];
        } catch (ArrayIndexOutOfBoundsException e) { ThrowOrdinalIndexException(iOrdinal, SpatialWindowType.values()); }
    }
    public void SetSpecialGridFileName(final String sSpecialGridFileName/*, boolean bSetUsingFlag*//*=false*/) {
        gsSpecialGridFileName = sSpecialGridFileName;
    }
    public void SetStartRangeEndDate(final String sStartRangeEndDate) {gsStartRangeEndDate = sStartRangeEndDate;}
    public void SetStartRangeStartDate(final String sStartRangeStartDate) {gsStartRangeStartDate = sStartRangeStartDate;}
    public void SetStudyPeriodDataCheckingType(int iOrdinal) {
        try { geStudyPeriodDataCheckingType = StudyPeriodDataCheckingType.values()[iOrdinal];
        } catch (ArrayIndexOutOfBoundsException e) { ThrowOrdinalIndexException(iOrdinal, StudyPeriodDataCheckingType.values()); }
    }
    public void SetStudyPeriodEndDate(final String sStudyPeriodEndDate) {gsStudyPeriodEndDate = sStudyPeriodEndDate;}
    public void SetStudyPeriodStartDate(final String sStudyPeriodStartDate) {gsStudyPeriodStartDate = sStudyPeriodStartDate;}
    public void SetSuppressingWarnings(boolean b) {gbSuppressWarnings=b;}
    public void SetTimeAggregationLength(int lTimeAggregationLength) {glTimeAggregationLength = lTimeAggregationLength;}
    public void SetTimeAggregationUnitsType(int iOrdinal) {
        try { geTimeAggregationUnitsType = DatePrecisionType.values()[iOrdinal];
        } catch (ArrayIndexOutOfBoundsException e) { ThrowOrdinalIndexException(iOrdinal, DatePrecisionType.values()); }
    }
    public void SetTimeTrendAdjustmentPercentage(double dPercentage) {gdTimeTrendAdjustPercentage = dPercentage;}
    public void SetTimeTrendAdjustmentType(int iOrdinal) {
        try { geTimeTrendAdjustType = TimeTrendAdjustmentType.values()[iOrdinal];
        } catch (ArrayIndexOutOfBoundsException e) { ThrowOrdinalIndexException(iOrdinal, TimeTrendAdjustmentType.values()); }
    }
    public void SetTimeTrendConvergence(double dTimeTrendConvergence) {gdTimeTrendConverge = dTimeTrendConvergence;}
    public void SetUseAdjustmentForRelativeRisksFile(boolean b) {gbUseAdjustmentsForRRFile = b;}
    public void SetVersion(final CreationVersion vVersion) {gCreationVersion = vVersion;}
    /**
     * @param iInvalidOrdinal -- index of ordinal position that is out of range
     * @param e               -- array of Enum that ordinal failed with
     */
    public void ThrowOrdinalIndexException(int iInvalidOrdinal, Enum[] e) {
        throw new RuntimeException("Ordinal index " + iInvalidOrdinal + " out of range [" +  e[0].ordinal() + "," +  e[e.length - 1].ordinal() + "].");
    }
    public boolean UseAdjustmentForRelativeRisksFile() {return gbUseAdjustmentsForRRFile;}
    public void UseLocationNeighborsFile(boolean b) {gbUseLocationNeighborsFile = b;}
    public boolean UseLocationNeighborsFile() {return gbUseLocationNeighborsFile;}
    public void UseMetaLocationsFile(boolean b) {gbUseMetaLocationsFile = b;}
    public boolean UseMetaLocationsFile() {return gbUseMetaLocationsFile;}
}
