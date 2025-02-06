package org.satscan.app;

import java.util.ArrayList;
import org.satscan.importer.InputSourceSettings;
import org.satscan.utils.FileAccess;

public class Parameters implements Cloneable {

    public native boolean                     Read(String filename);
    public native boolean                     ReadFromStringStream(String stringstream);
    public native void                        Write(String filename);
    public native String                      WriteToStringStream();

    /** analysis and cluster types */
    public enum AnalysisType                  {PURELYSPATIAL, PURELYTEMPORAL, SPACETIME,  PROSPECTIVESPACETIME,
                                               SPATIALVARTEMPTREND, PROSPECTIVEPURELYTEMPORAL, SEASONALTEMPORAL};
    /** cluster types */
    public enum ClusterType                   {PURELYSPATIALCLUSTER, PURELYTEMPORALCLUSTER, SPACETIMECLUSTER,
                                               SPATIALVARTEMPTRENDCLUSTER, PURELYSPATIALMONOTONECLUSTER,
                                               PURELYSPATIALPROSPECTIVECLUSTER, PURELYSPATIALHOMOGENEOUSCLUSTER};
    /** probability model types */
    public enum ProbabilityModelType          {POISSON, BERNOULLI, SPACETIMEPERMUTATION, ORDINAL, EXPONENTIAL,
                                               NORMAL, HOMOGENEOUSPOISSON, CATEGORICAL, RANK, UNIFORMTIME, BATCHED};
    public enum IncludeClustersType           {ALLCLUSTERS, ALIVECLUSTERS, CLUSTERSINRANGE};
    public enum RiskType                      {STANDARDRISK, MONOTONERISK};
    /** area incidence rate types */
    public enum AreaRateType                  {HIGH, LOW, HIGHANDLOW};
    /** time trend adjustment types */
    public enum TimeTrendAdjustmentType       {TEMPORAL_NOTADJUSTED, TEMPORAL_NONPARAMETRIC, LOGLINEAR_PERC,
                                               CALCULATED_LOGLINEAR_PERC, TEMPORAL_STRATIFIED_RANDOMIZATION, CALCULATED_QUADRATIC};
    /** spatial adjustment types */
    public enum SpatialAdjustmentType         {SPATIAL_NOTADJUSTED, SPATIAL_STRATIFIED_RANDOMIZATION, SPATIAL_NONPARAMETRIC};
    public enum CoordinatesType               {CARTESIAN, LATLON};
    /** criteria for reporting secondary clusters types */
    public enum CriteriaSecondaryClustersType {NOGEOOVERLAP, NOCENTROIDSINOTHER, NOCENTROIDSINMORELIKE,
                                               NOCENTROIDSINLESSLIKE, NOPAIRSINEACHOTHERS, NORESTRICTIONS};
    /** interpretation types for maximum temporal size */
    public enum TemporalSizeType              {PERCENTAGETYPE, TIMETYPE};
    /** interpretation types for maximum spatial size */
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
    public enum ProspectiveFrequency          { SAME_TIMEAGGREGATION, DAILY, WEEKLY, MONTHLY, QUARTERLY, YEARLY};
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
    /** temporal graph reporting type */
    public enum TemporalGraphReportType       {MLC_ONLY, X_MCL_ONLY, SIGNIFICANT_ONLY}
    
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
    private SpatialAdjustmentType           geSpatialAdjustmentType=SpatialAdjustmentType.SPATIAL_NOTADJUSTED; /** type of spatial adjustment*/
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
    /* minimum temporal cluster size */
    private int                             _minimum_temporal_cluster_size=1;    
    /* Time interval variables */
    private int                             glTimeAggregationLength=0; /** length of time aggregation */
    private DatePrecisionType               geTimeAggregationUnitsType=DatePrecisionType.NONE; /** time aggregation units type */
    /* Temporal trend adjusment variables */
    private double                          gdTimeTrendAdjustPercentage=0; /** percentage for log linear adjustment */
    private TimeTrendAdjustmentType         geTimeTrendAdjustType=TimeTrendAdjustmentType.TEMPORAL_NOTADJUSTED; /** Adjust for time trend: no, discrete, % */
    private int                             _nonparametric_adjustment_size=1;
    /* Input precision variables */
    private DatePrecisionType               gePrecisionOfTimesType=DatePrecisionType.YEAR; /** precision of case/control data: none = no, years=months=days = yes */
    private CoordinatesType                 geCoordinatesType=CoordinatesType.LATLON; /** coordinates type for coordinates/special grid */
    /* Ellipse variables */
    private ArrayList<Double>               gvEllipseShapes = new ArrayList<>(); /** shape of each ellipsoid */
    private ArrayList<Integer>              gvEllipseRotations = new ArrayList<>(); /** number of rotations for each ellipsoid */
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
    private TemporalGraphReportType         _temporal_graph_report_type=TemporalGraphReportType.MLC_ONLY; /* which clusters to report in temporal graph */
    private int                             _temporal_graph_report_count=1; /* number of MLC clusters to graph with TemporalGraphReportType.X_MCL_ONLY */
    private double                          _temporal_graph_report_cutoff=0.05; /* P-Value used limit graphed clusters with TemporalGraphReportType.SIGNIFICANT_ONLY */
    private boolean                         _include_locations_kml=true; /** include cluster locations in kml output */
    private boolean                         _compress_kml_output=false; /** compress kml output into kmz format */
    private boolean                         _launch_map_viewer=true; /* whether to launch viewer for requested maps */
    private boolean                         _output_shapefiles=false;
    private boolean                         _output_cartesian_graph=false; /* generate spatial cartesian graph */

    /* Iterative scans variables */
    private boolean                         gbIterativeRuns=false; /* Iterative analysis? */
    private int                             giNumIterativeRuns=0; /* number of Iterative scans to attempt */
    private double                          gbIterativeCutOffPValue=0.05; /* P-Value used to exit Iterative analysis */
        /* Input/Output filenames */
    private String                          gsParametersSourceFileName=""; /** parameters source filename */
    private ArrayList<String>               gvCaseFilenames = new ArrayList<>(); /** case data source filenames */
    private ArrayList<String>               gvControlFilenames = new ArrayList<>(); /** control data source filenames */
    private ArrayList<String>               gvPopulationFilenames = new ArrayList<>(); /** population data source filenames */
    private ArrayList<String>               _data_set_names = new ArrayList<>(); /** data set names */
    private String                          gsCoordinatesFileName=""; /** coordinates data source filename */
    private String                          gsSpecialGridFileName=""; /** special grid data source filename */
    private String                          gsMaxCirclePopulationFileName=""; /** special population file for constructing circles only */
    private String                          gsOutputFileName=""; /** results output filename */
    private boolean                         gbLogRunHistory=false; /** indicates whether to log history */
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
    private ArrayList<String>               gvObservableRegions = new ArrayList<>(); /** collection of observable regions */
    private CreationVersion                 gCreationVersion;
    private int                             glRandomizationSeed=12345678; /** randomization seed */
    private boolean                         gbReportCriticalValues=false; /** indicates whether to report critical LLR values */
    private boolean                         gbSuppressWarnings=false; /** indicates whether to suppress warnings printed during execution */
    private SpatialWindowType               geSpatialWindowType=SpatialWindowType.CIRCULAR; /** spatial window shape */
    private TimeTrendType                   geTimeTrendType=TimeTrendType.LINEAR;                        /** time trend type */
    private boolean                         gbReportRank=false;  /** report cluster rank */
    private boolean                         gbPrintAsciiHeaders=false;  /** print ASCII column headers */
    private String                          gsTitleName="";
    private double                          _giniIndexPValueCutoff=0.05; /* P-Value used to limit clusters in gini index coefficients calcuation */

    private boolean                         _reportHierarchicalClusters=true;  /** print ASCII column headers */
    private boolean                         _reportGiniOptimizedClusters=true;  /** print ASCII column headers */
    GiniIndexReportType                     _giniIndexReportType=GiniIndexReportType.OPTIMAL_ONLY; /* type for the gini index cluster reporting */
    boolean                                 _outputGiniIndexCoefficients=false; /* output gini index coefficents */
    private boolean                         _calculate_oliveira_f=false;
    private int                             _num_oliveira_sets=1000;

    /* Power Evaluation variables */
    private boolean                         _performPowerEvaluation=false; /** indicator of whether to perform power calculations */
    CriticalValuesType                      _critical_value_type=CriticalValuesType.CV_MONTECARLO;
    PowerEstimationType                     _power_estimation_type=PowerEstimationType.PE_MONTECARLO;
    PowerEvaluationMethodType               _power_evaluation_method=PowerEvaluationMethodType.PE_WITH_ANALYSIS;
    private String                          _power_alt_hypothesis_filename=""; /** power evaluation alternative filename */
    private int                             _powerEvaluationTotalCases=600; /* number cases in power evaluation, user specified */
    private int                             _power_replica=1000; /* number of replications in power step of power evaluations */
    
    private ArrayList<InputSourceSettings>  _input_sources = new ArrayList<>();
    
    private boolean                         _risk_limit_high_clusters=false;
    private double                          _risk_threshold_high_clusters=1.0;
    private boolean                         _risk_limit_low_clusters=false;
    private double                          _risk_threshold_low_clusters=1.0;
    
    private int                             _minimum_high_rate_cases=2; /* minimum number of cases in cluster when scanning high rates */
    
    private boolean                         _output_google_map=false;
    private String                          _google_maps_api_key="";
    
    private boolean                         _perform_standard_drilldown=false;
    private boolean                         _perform_bernoulli_drilldown=false;
    private int                             _drilldown_minimum_locations=2;
    private int                             _drilldown_minimum_cases=10;
    private double                          _drilldown_pvalue_cutoff=0.05;
    private boolean                         _drilldown_adjust_weekly_trends=false;
    
    private String                          _locations_network_filename="";
    private boolean                         _use_locations_network_file=false;
    private ProspectiveFrequency            _prospective_frequency=ProspectiveFrequency.SAME_TIMEAGGREGATION;
    
    private boolean                         _always_email_summary=false;
    private boolean                         _cutoff_email_summary=false;
    private double                          _cutoff_email_value=0.05;
    private boolean                         _email_attach_results=false;  
    private boolean                         _email_include_results_directory=false;
    private String                          _email_always_recipients="";
    private String                          _email_cutoff_recipients="";
    private boolean                         _email_custom=false; 
    private String                          _email_custom_subject=DEFAULT_EMAIL_SUBJECT;
    private String                          _email_custom_message_body=DEFAULT_EMAIL_MESSAGE;
    private boolean                         _create_email_summary_file=false;
    private double                          _email_summary_cutoff=0.05;
    
    private String                          _multiple_locations_file="";
    private String                          _linelist_individuals_cache_name="";
    private double                          _linelist_csv_cutoff=1;
    
    public static final int                 MAXIMUM_ITERATIVE_ANALYSES=32000; /** maximum number of permitted iterative scans */
    public static final int                 MAXIMUM_ELLIPSOIDS=10; /** maximum number of permitted ellipsoids */
    public static final String              DEFAULT_EMAIL_SUBJECT="SaTScan Results - <results-name>";
    public static final String              DEFAULT_EMAIL_MESSAGE="<summary-paragraph><linebreak><linebreak><location-paragraph><linebreak><linebreak><footer-paragraph>";

    public Parameters() {
      super();

      gCreationVersion = new CreationVersion(7,0,0);
      gvCaseFilenames.add("");
      gvControlFilenames.add("");
      gvPopulationFilenames.add("");
      _data_set_names.add("Data Set #1");
      gvEllipseShapes.add(1.5);
      gvEllipseShapes.add(2.0);
      gvEllipseShapes.add(3.0);
      gvEllipseShapes.add(4.0);
      gvEllipseShapes.add(5.0);
      gvEllipseRotations.add(4);
      gvEllipseRotations.add(6);
      gvEllipseRotations.add(9);
      gvEllipseRotations.add(12);
      gvEllipseRotations.add(15);
    }

    @Override
    public Object clone() {
        try {
            Parameters newObject = (Parameters)super.clone();
            newObject.gvCaseFilenames = new ArrayList<>(gvCaseFilenames);
            newObject.gvControlFilenames = new ArrayList<>(gvControlFilenames);
            newObject.gvPopulationFilenames = new ArrayList<>(gvPopulationFilenames);
            newObject._data_set_names = new ArrayList<>(_data_set_names);
            newObject.gvObservableRegions = new ArrayList<>(gvObservableRegions);
            newObject._input_sources = new ArrayList<>();
            for (InputSourceSettings iss : _input_sources)
                newObject._input_sources.add(iss.clone());
            return newObject;
        } catch (CloneNotSupportedException e) {
            throw new InternalError("But we are Cloneable!!!");
        }
    }

    @Override
    public boolean equals(Object other) {
        if (this == other) return true; // check self
        if (other == null) return false; // check null
        if (getClass() != other.getClass()) return false; // type check and cast
        
        Parameters rhs = (Parameters)other;

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
        if (_nonparametric_adjustment_size         != rhs._nonparametric_adjustment_size) return false;
        if (gbIncludePurelySpatialClusters         != rhs.gbIncludePurelySpatialClusters) return false;
        if (gbIncludePurelyTemporalClusters        != rhs.gbIncludePurelyTemporalClusters) return false;
        if (!gvCaseFilenames.equals(rhs.gvCaseFilenames)) return false;
        if (!gvControlFilenames.equals(rhs.gvControlFilenames)) return false;
        if (!gvPopulationFilenames.equals(rhs.gvPopulationFilenames)) return false;
        if (!_data_set_names.equals(rhs._data_set_names)) return false;
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
        if (!gsTitleName.equals(rhs.gsTitleName)) return false;
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
        if (_launch_map_viewer != rhs._launch_map_viewer) return false;  
        if (_output_shapefiles != rhs._output_shapefiles) return false;
        if (_minimum_temporal_cluster_size != rhs._minimum_temporal_cluster_size) return false;        
        if (_temporal_graph_report_count != rhs._temporal_graph_report_count) return false;
        if (_temporal_graph_report_cutoff != rhs._temporal_graph_report_cutoff) return false;
        if (_temporal_graph_report_type != rhs._temporal_graph_report_type) return false;
        if (!_input_sources.equals(rhs._input_sources)) return false;
        if (_calculate_oliveira_f != rhs._calculate_oliveira_f) return false;
        if (_num_oliveira_sets != rhs._num_oliveira_sets) return false;
        if (_output_cartesian_graph != rhs._output_cartesian_graph) return false;
        if (_minimum_high_rate_cases != rhs._minimum_high_rate_cases) return false;
        if (_risk_limit_high_clusters != rhs._risk_limit_high_clusters) return false;
        if (_risk_threshold_high_clusters != rhs._risk_threshold_high_clusters) return false;
        if (_risk_limit_low_clusters != rhs._risk_limit_low_clusters) return false;
        if (_risk_threshold_low_clusters != rhs._risk_threshold_low_clusters) return false;
        if (_output_google_map != rhs._output_google_map) return false;
        if (!_google_maps_api_key.equals(rhs._google_maps_api_key)) return false;        
        if (_perform_standard_drilldown != rhs._perform_standard_drilldown) return false;
        if (_perform_bernoulli_drilldown != rhs._perform_bernoulli_drilldown) return false;
        if (_drilldown_minimum_locations != rhs._drilldown_minimum_locations) return false;
        if (_drilldown_minimum_cases != rhs._drilldown_minimum_cases) return false;
        if (_drilldown_pvalue_cutoff != rhs._drilldown_pvalue_cutoff) return false;
        if (_drilldown_adjust_weekly_trends != rhs._drilldown_adjust_weekly_trends) return false;
        if (_use_locations_network_file != rhs._use_locations_network_file) return false;
        if (!_locations_network_filename.equals(rhs._locations_network_filename)) return false;
        if (_prospective_frequency != rhs._prospective_frequency) return false;
        if (_always_email_summary != rhs._always_email_summary) return false;
        if (_cutoff_email_summary != rhs._cutoff_email_summary) return false;
        if (_cutoff_email_value != rhs._cutoff_email_value) return false;
        if (_email_attach_results != rhs._email_attach_results) return false;
        if (_email_include_results_directory != rhs._email_include_results_directory) return false;
        if (!_email_always_recipients.equals(rhs._email_always_recipients)) return false;
        if (!_email_cutoff_recipients.equals(rhs._email_cutoff_recipients)) return false;
        if (_email_custom != rhs._email_custom) return false;
        if (!_email_custom_subject.equals(rhs._email_custom_subject)) return false;
        if (!_email_custom_message_body.equals(rhs._email_custom_message_body)) return false;
        if (!_linelist_individuals_cache_name.equals(rhs._linelist_individuals_cache_name)) return false;
        if (_linelist_csv_cutoff != rhs._linelist_csv_cutoff) return false;
        return _multiple_locations_file.equals(rhs._multiple_locations_file);
    }
    
    public boolean getCreateEmailSummaryFile() { return _create_email_summary_file; }
    public void setCreateEmailSummaryFile(boolean b) { _create_email_summary_file = b; }            
    public double getEmailSummaryValue() { return _email_summary_cutoff; }
    public void setEmailSummaryValue(double d) { _email_summary_cutoff = d; }
    public double getCutoffLineListCSV() { return _linelist_csv_cutoff; }
    public void setCutoffLineListCSV(double d) { _linelist_csv_cutoff = d; }  
    
    public boolean getEmailAttachResults() { return _email_attach_results; }
    public void setEmailAttachResults(boolean b) { _email_attach_results = b; }
    public boolean getEmailIncludeResultsDirectory()  { return _email_include_results_directory; }
    public void setEmailIncludeResultsDirectory(boolean b) { _email_include_results_directory = b; }
    public boolean getAlwaysEmailSummary() { return _always_email_summary; }
    public void setAlwaysEmailSummary(boolean b) { _always_email_summary = b; }
    public boolean getCutoffEmailSummary() { return _cutoff_email_summary; }
    public void setCutoffEmailSummary(boolean b) { _cutoff_email_summary = b; }
    public double getCutoffEmailValue() { return _cutoff_email_value; }
    public void setCutoffEmailValue(double d) { _cutoff_email_value = d; }
    public String getEmailAlwaysRecipients() { return _email_always_recipients; }
    public void setEmailAlwaysRecipients(final String s) { _email_always_recipients = s; }
    public String getEmailCutoffRecipients() { return _email_cutoff_recipients; }
    public void setEmailCutoffRecipients(final String s) { _email_cutoff_recipients = s; }
    public boolean getEmailCustom() { return _email_custom; }
    public void setEmailCustom(boolean b) { _email_custom = b; }    
    public String getEmailCustomSubject() { return _email_custom_subject; }
    public void setEmailCustomSubject(final String s) { _email_custom_subject = s; }
    public String getEmailCustomMessageBody() { return _email_custom_message_body; }
    public void setEmailCustomMessageBody(final String s) { _email_custom_message_body = s; }    
    
    public String getLinelistIndividualsCacheFileName() { return _linelist_individuals_cache_name; }
    public void setLinelistIndividualsCacheFileName(final String s) { _linelist_individuals_cache_name = s; }
    public String getMultipleLocationsFile() { return _multiple_locations_file; }
    public void setMultipleLocationsFile(final String s) { _multiple_locations_file = s; }
    
    public ProspectiveFrequency getProspectiveFrequencyType() { return _prospective_frequency; }
    public void setProspectiveFrequencyType(int iOrdinal) {
        try { _prospective_frequency = ProspectiveFrequency.values()[iOrdinal];
        } catch (ArrayIndexOutOfBoundsException e) { ThrowOrdinalIndexException(iOrdinal, ProspectiveFrequency.values()); }
    }
    public boolean getUseLocationsNetworkFile() { return _use_locations_network_file; }
    public void setUseLocationsNetworkFile(boolean b) { _use_locations_network_file = b; }
    public String getLocationsNetworkFilename() { return _locations_network_filename; }
    public void setLocationsNetworkFilename(final String s) { _locations_network_filename = s; }
    public boolean getPerformStandardDrilldown() { return _perform_standard_drilldown; }
    public void setPerformStandardDrilldown(boolean b) { _perform_standard_drilldown = b; }
    public boolean getPerformBernoulliDrilldown() { return _perform_bernoulli_drilldown; }
    public void setPerformBernoulliDrilldown(boolean b) { _perform_bernoulli_drilldown = b; }
    public int getDrilldownMinimumLocationsCluster() { return _drilldown_minimum_locations; }
    public void setDrilldownMinimumLocationsCluster(int u) { _drilldown_minimum_locations = u; }
    public int getDrilldownMinimumCasesCluster() { return _drilldown_minimum_cases; }
    public void setDrilldownMinimumCasesCluster(int u) { _drilldown_minimum_cases = u; }
    public double getDrilldownCutoff() { return _drilldown_pvalue_cutoff; }
    public void setDrilldownCutoff(double d) { _drilldown_pvalue_cutoff = d; }
    public boolean getDrilldownAdjustWeeklyTrends() { return _drilldown_adjust_weekly_trends; }
    public void setDrilldownAdjustWeeklyTrends(boolean b) { _drilldown_adjust_weekly_trends = b; }    
    public boolean getOutputGoogleMapsFile() { return _output_google_map; }
    public void setOutputGoogleMapsFile(boolean b) { _output_google_map = b; }
    public String getGoogleMapsApiKey() { return _google_maps_api_key; }
    public void setGoogleMapsApiKey(final String api_key) { _google_maps_api_key = api_key; }
    public int getMinimumCasesHighRateClusters() { return _minimum_high_rate_cases; }
    public void setMinimumCasesHighRateClusters(int u) { _minimum_high_rate_cases = u; }
    public boolean getRiskLimitHighClusters() {return _risk_limit_high_clusters;}
    public void setRiskLimitHighClusters(boolean b) {_risk_limit_high_clusters = b;}
    public double getRiskThresholdHighClusters() {return _risk_threshold_high_clusters;}
    public void setRiskThresholdHighClusters(double d) {_risk_threshold_high_clusters = d;}
    public boolean getRiskLimitLowClusters() {return _risk_limit_low_clusters;}
    public void setRiskLimitLowClusters(boolean b) {_risk_limit_low_clusters = b;}
    public double getRiskThresholdLowClusters() {return _risk_threshold_low_clusters;}
    public void setRiskThresholdLowClusters(double d) {_risk_threshold_low_clusters = d;}
    public boolean getOutputCartesianGraph() {return _output_cartesian_graph;}
    public void setOutputCartesianGraph(boolean b) {_output_cartesian_graph = b;}
    public void addInputSourceSettings(InputSourceSettings iss) {_input_sources.add(iss);}
    public void clearInputSourceSettings() {_input_sources.clear();}
    public ArrayList<InputSourceSettings> getInputSourceSettings() {return _input_sources;}
    public double getTemporalGraphSignificantCutoff() {return _temporal_graph_report_cutoff;}
    public void setTemporalGraphSignificantCutoff(double d) {_temporal_graph_report_cutoff = d;}
    public int getTemporalGraphMostLikelyCount() {return _temporal_graph_report_count;}
    public void setTemporalGraphMostLikelyCount(int i) {_temporal_graph_report_count = i;}
    public TemporalGraphReportType getTemporalGraphReportType() {return _temporal_graph_report_type;}
    public void setTemporalGraphReportType(int iOrdinal) {
        try { _temporal_graph_report_type = TemporalGraphReportType.values()[iOrdinal];
        } catch (ArrayIndexOutOfBoundsException e) { ThrowOrdinalIndexException(iOrdinal, CriticalValuesType.values()); }
    }        
    public int getMinimumTemporalClusterSize() {return _minimum_temporal_cluster_size;}
    public void setMinimumTemporalClusterSize(int i) {_minimum_temporal_cluster_size = i;}
    public boolean getOutputShapeFiles() {return _output_shapefiles;}
    public void setOutputShapeFiles(boolean b) {_output_shapefiles = b;}    
    public boolean getLaunchMapViewer() {return _launch_map_viewer;}
    public void setLaunchMapViewer(boolean b) {_launch_map_viewer = b;}
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
    public void AddEllipsoidShape(double dShape, boolean bEmptyFirst) {
        if (bEmptyFirst) gvEllipseShapes.clear();
        gvEllipseShapes.add(dShape);
    }
    /** Add ellipsoid rotations to collection of spatial shapes evaluated. */
    public void AddEllipsoidRotations(int iRotations, boolean bEmptyFirst) {
        if (bEmptyFirst) gvEllipseRotations.clear();
        gvEllipseRotations.add(iRotations);
    }
    /** Add observable region to definition to collection. */
    public void ClearObservableRegions() {
        gvObservableRegions.clear();
    }
    /** Add observable region to definition to collection. */
    public void AddObservableRegion(final String sRegions, int iIndex, boolean bEmptyFirst) {
        if (bEmptyFirst) gvObservableRegions.clear();
        setSize(gvObservableRegions, iIndex + 1);
        gvObservableRegions.set(iIndex, sRegions);
    }
    public boolean getCalculateOliveirasF() {return _calculate_oliveira_f;}
    public void setCalculateOliveirasF(boolean b) {_calculate_oliveira_f = b;}
    public int getNumRequestedOliveiraSets() {return _num_oliveira_sets;}
    public void setNumRequestedOliveiraSets(int i) {_num_oliveira_sets = i;}    
    public boolean getAdjustForWeeklyTrends() {return _adjustWeeklyTrends;}
    public void setAdjustForWeeklyTrends(boolean b) {_adjustWeeklyTrends = b;}
    public boolean getPerformPowerEvaluation() {return _performPowerEvaluation;}
    public void setPerformPowerEvaluation(boolean b) {_performPowerEvaluation = b;}
    public CriticalValuesType getPowerEvaluationCriticalValueType() {return _critical_value_type;}
    public void setPowerEvaluationCriticalValueType(int iOrdinal) {
        try { _critical_value_type = CriticalValuesType.values()[iOrdinal];
        } catch (ArrayIndexOutOfBoundsException e) { ThrowOrdinalIndexException(iOrdinal, CriticalValuesType.values()); }
    }
    public PowerEstimationType getPowerEstimationType() {return _power_estimation_type;}
    public void setPowerEstimationType(int iOrdinal) {
        try { _power_estimation_type = PowerEstimationType.values()[iOrdinal];
        } catch (ArrayIndexOutOfBoundsException e) { ThrowOrdinalIndexException(iOrdinal, PowerEstimationType.values()); }
    }
    public PowerEvaluationMethodType getPowerEvaluationMethod() {return _power_evaluation_method;}
    public void setPowerEvaluationMethod(int iOrdinal) {
        try { _power_evaluation_method = PowerEvaluationMethodType.values()[iOrdinal];
        } catch (ArrayIndexOutOfBoundsException e) { ThrowOrdinalIndexException(iOrdinal, PowerEvaluationMethodType.values()); }
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
        } catch (ArrayIndexOutOfBoundsException e) { ThrowOrdinalIndexException(iOrdinal, GiniIndexReportType.values()); }
    }
    public boolean getReportGiniIndexCoefficents() {return _outputGiniIndexCoefficients;}
    public void setReportGiniIndexCoefficents(boolean b) {_outputGiniIndexCoefficients = b;}
    public boolean getPrintAsciiHeaders() {return gbPrintAsciiHeaders;}
    public void setPrintAsciiHeaders(boolean b) {gbPrintAsciiHeaders = b;}
    public final String GetTitleName() {return gsTitleName;}
    public void SetTitleName(final String sTitleName) {gsTitleName = sTitleName;}
    public boolean GetAdjustForEarlierAnalyses() {return gbAdjustForEarlierAnalyses;}
    public String GetAdjustmentsByRelativeRisksFilename() {return gsAdjustmentsByRelativeRisksFileName;}
    public AnalysisType GetAnalysisType() {return geAnalysisType;}
    public String GetAnalysisTypeAsString(boolean abbr) {
        String sAnalysisType = null;
        switch (geAnalysisType) {
            case PURELYSPATIAL             : sAnalysisType = "Purely Spatial"; break;
            case PURELYTEMPORAL            : sAnalysisType = (abbr ? "Retro. Purely Temporal" : "Retrospective Purely Temporal"); break;
            case SPACETIME                 : sAnalysisType = (abbr ? "Retro. Space-Time" : "Retrospective Space-Time"); break;
            case PROSPECTIVESPACETIME      : sAnalysisType = (abbr ? "Prosp. Space-Time" : "Prospective Space-Time"); break;
            case SPATIALVARTEMPTREND       : sAnalysisType = (abbr ? "SVTT" : "Spatial Variation in Temporal Trends"); break;
            case PROSPECTIVEPURELYTEMPORAL : sAnalysisType = (abbr ? "Prosp. Purely Temporal" : "Prospective Purely Temporal"); break;
          }
        return sAnalysisType;
    }
    public String GetModelTypeAsString(boolean abbr) {
        return GetProbabilityModelTypeAsString(geProbabilityModelType, abbr);
    }    
    public AreaRateType GetAreaScanRateType() {return geAreaScanRate;}
    public final ArrayList<String> GetObservableRegions() {return gvObservableRegions;}

    public final String getDataSourceName(int iSetIndex/*=1*/) {
        return _data_set_names.get(iSetIndex - 1);
    }
    public final ArrayList<String> getDataSourceNames() {return _data_set_names;}


    public final String GetCaseFileName(int iSetIndex/*=1*/) {
        return gvCaseFilenames.get(iSetIndex - 1);
    }
    public final ArrayList<String> GetCaseFileNames() {return gvCaseFilenames;}
    public final String GetControlFileName(int iSetIndex/*=1*/) {
      return gvControlFilenames.get(iSetIndex - 1);
    }
    public final ArrayList<String> GetControlFileNames() {return gvControlFilenames;}
    public final String GetCoordinatesFileName() {return gsCoordinatesFileName;}
    public CoordinatesDataCheckingType GetCoordinatesDataCheckingType() {return geCoordinatesDataCheckingType;}
    public CoordinatesType GetCoordinatesType() {return geCoordinatesType;}
    public final CreationVersion GetCreationVersion() {return gCreationVersion;}
    public CriteriaSecondaryClustersType GetCriteriaSecondClustersType() {return geCriteriaSecondClustersType;}
    public int GetCreationVersionMajor() {return gCreationVersion.giMajor;}
    public final ArrayList<Integer> GetEllipseRotations() {return gvEllipseRotations;}
    public final ArrayList<Double> GetEllipseShapes() {return gvEllipseShapes;}
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
    public final String GetOutputFileNameSetting() {return gsOutputFileName; }
    public final String GetOutputFileName() {return FileAccess.getFormatSubstitutedFilename(gsOutputFileName); }
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
        return gvPopulationFilenames.get(iSetIndex - 1);
    }
    public final ArrayList<String> GetPopulationFileNames() {return gvPopulationFilenames;}
    public DatePrecisionType GetPrecisionOfTimesType() {return gePrecisionOfTimesType;}
    public ProbabilityModelType GetProbabilityModelType() {return geProbabilityModelType;}
    public static final String GetProbabilityModelTypeAsString(ProbabilityModelType eProbabilityModelType, boolean abbr) {
        String sProbabilityModel = null;
        switch (eProbabilityModelType) {
            case POISSON              : sProbabilityModel = "Poisson"; break;
            case BERNOULLI            : sProbabilityModel = "Bernoulli"; break;
            case SPACETIMEPERMUTATION : sProbabilityModel = (abbr ? "STP": "Space-Time Permutation"); break;
            case ORDINAL              : sProbabilityModel = "Ordinal"; break;
            case EXPONENTIAL          : sProbabilityModel = "Exponential"; break;
            case NORMAL               : sProbabilityModel = "Normal"; break;
            case RANK                 : sProbabilityModel = "Rank"; break;
            case HOMOGENEOUSPOISSON   : sProbabilityModel = (abbr ? "H. Poisson" : "Homogeneous Poisson"); break;
            case CATEGORICAL          : sProbabilityModel = "Multinomial"; break;
            case UNIFORMTIME          : sProbabilityModel = "Uniform-Time"; break;
            case BATCHED              : sProbabilityModel = "Batched"; break;
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
    public int GetNonparametricAdjustmentSize() { return _nonparametric_adjustment_size; }
    public TimeTrendAdjustmentType GetTimeTrendAdjustmentType() {return geTimeTrendAdjustType;}
    public double GetTimeTrendConvergence() {return gdTimeTrendConverge;}
    private <T> void setSize(ArrayList<T> list, int size) {
        /* Replicates Vector setSize for ArrayList. */
        while (list.size() > size) list.remove(list.size() - 1);
        while (list.size() < size) list.add(null);
    }
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
        } catch (ArrayIndexOutOfBoundsException e) { ThrowOrdinalIndexException(iOrdinal, PValueReportingType.values()); }
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
    public void setDataSourceName(final String name, int iSetIndex/*=1*/) {
        setSize(_data_set_names, iSetIndex);
        _data_set_names.set(iSetIndex - 1, name);
    }
    public void SetCaseFileName(final String sCaseFileName, int iSetIndex/*=1*/) {
        setSize(gvCaseFilenames, iSetIndex);
        gvCaseFilenames.set(iSetIndex - 1, sCaseFileName);
    }
    public void SetControlFileName(final String sControlFileName, int iSetIndex/*=1*/) {
        setSize(gvControlFilenames, iSetIndex);
        gvControlFilenames.set(iSetIndex - 1, sControlFileName);
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
        setSize(gvCaseFilenames, iNumDataSets);
        setSize(gvControlFilenames, iNumDataSets);
        setSize(gvPopulationFilenames, iNumDataSets);
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
    public void SetOutputFileNameSetting(final String  sOutPutFileName) {gsOutputFileName = sOutPutFileName;}
    public void SetOutputRelativeRisksAscii(boolean b) {gbOutputRelativeRisksAscii = b;}
    public void SetOutputRelativeRisksDBase(boolean b) {gbOutputRelativeRisksDBase = b;}
    public void SetOutputSimLogLikeliRatiosAscii(boolean b) {gbOutputSimLogLikeliRatiosAscii = b;}
    public void SetOutputSimLogLikeliRatiosDBase(boolean b) {gbOutputSimLogLikeliRatiosDBase = b;}
    public void SetOutputSimulationData(boolean b) {gbOutputSimulationData = b;}
    public void SetPopulationFileName(final String  sPopulationFileName, int iSetIndex/*=1*/) {
        setSize(gvPopulationFilenames, iSetIndex);
        gvPopulationFilenames.set(iSetIndex - 1, sPopulationFileName);
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
    public void SetNonparametricAdjustmentSize(int i) { _nonparametric_adjustment_size = i; }
    public void SetTimeTrendAdjustmentType(int iOrdinal) {
        try { geTimeTrendAdjustType = TimeTrendAdjustmentType.values()[iOrdinal];
        } catch (ArrayIndexOutOfBoundsException e) { ThrowOrdinalIndexException(iOrdinal, TimeTrendAdjustmentType.values()); }
    }
    public void SetTimeTrendConvergence(double dTimeTrendConvergence) {gdTimeTrendConverge = dTimeTrendConvergence;}
    public void SetUseAdjustmentForRelativeRisksFile(boolean b) {gbUseAdjustmentsForRRFile = b;}
    public void SetVersion(final CreationVersion vVersion) {gCreationVersion = vVersion;}
    public void ThrowOrdinalIndexException(int iInvalidOrdinal, Enum[] e) {
        throw new RuntimeException("Ordinal index " + iInvalidOrdinal + " out of range [" +  e[0].ordinal() + "," +  e[e.length - 1].ordinal() + "].");
    }
    public boolean UseAdjustmentForRelativeRisksFile() {return gbUseAdjustmentsForRRFile;}
    public void UseLocationNeighborsFile(boolean b) {gbUseLocationNeighborsFile = b;}
    public boolean UseLocationNeighborsFile() {return gbUseLocationNeighborsFile;}
    public void UseMetaLocationsFile(boolean b) {gbUseMetaLocationsFile = b;}
    public boolean UseMetaLocationsFile() {return gbUseMetaLocationsFile;}
}
