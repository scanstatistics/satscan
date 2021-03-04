//*****************************************************************************
#ifndef __PARAMETERTYPES_H_
#define __PARAMETERTYPES_H_
//*****************************************************************************
/** parameter types
    - parameters that are read from file with the exception of: DIMENSION, EXACTTIMES, and RUN_HISTORY_FILENAME */
enum ParameterType
{
    ANALYSISTYPE=1,                 /* analysis type (enumeration) */
    SCANAREAS,                      /* scanning areas type (enumeration) */
    CASEFILE,                       /* case filename (vector<string>) */
    POPFILE,                        /* population filename (vector<string>) */
    COORDFILE,                      /* coordinates filename (string) */
    OUTPUTFILE,                     /* text results filename (string) */
    PRECISION,                      /* case/control file date precision (enumeration) */
    DIMENSION,                      /* not used */
    SPECIALGRID,                    /* use grid file (boolean) */
    GRIDFILE,                       /* grid filename (string) */
    GEOSIZE,                        /* maximum spatial cluster size (numeric) */
    STARTDATE,                      /* study period start date (string) */
    ENDDATE,                        /* study period end date (string) */
    CLUSTERS,                       /* temporal clusters evaluated (enumeration) */
    EXACTTIMES,                     /* not used */
    TIME_AGGREGATION_UNITS,         /* time aggregation units (enumeration) */
    TIME_AGGREGATION,               /* time aggregation (integer) */
    PURESPATIAL,                    /* include purely spatial clusters (boolean) */
    TIMESIZE,                       /* maximum temporal cluster size (numeric) */
    REPLICAS,                       /* number simulation replications (integer) */
    MODEL,                          /* probabilty model type (enumeration) */
    RISKFUNCTION,                   /* isotonic scan type (enumeration) */
    POWER_EVALUATION,               /* perform power evaluation (boolean) */
    POWER_05,                       /* power evaluation critical value .05 (numeric) */
    POWER_01,                       /* power evaluation critical value .01 (numeric) */
    TIMETREND,                      /* time trend adjustment type (enumeration) */
    TIMETRENDPERC,                  /* time trend adjustment percentage (numeric) */
    PURETEMPORAL,                   /* include purely temporal clusters (boolean) */
    CONTROLFILE,                    /* control filename (vector<string>) */
    COORDTYPE,                      /* coordinates type (enumeration) */
    OUTPUT_SIM_LLR_ASCII,           /* output simulated log likelihoods ratios in ASCII format (boolean) */
    ITERATIVE,                      /* perform iterative scans (boolean) */
    ITERATIVE_NUM,                  /* maximum iterations for iterative scan (integer) */
    ITERATIVE_PVAL,                 /* max p-value for iterative scan before cutoff (numeric) */
    VALIDATE,                       /* not used */
    OUTPUT_RR_ASCII,                /* output relative risks in ASCII format (boolean) */
    WINDOW_SHAPE,                   /* spatial window shape (enumeration) */
    ESHAPES,                        /* elliptic shapes (csv string) */
    ENUMBERS,                       /* elliptic angles (csv string) */
    START_PROSP_SURV,               /* prospective surveillance start date (string) */
    OUTPUT_AREAS_ASCII,             /* output location information in ASCII format (boolean) */
    OUTPUT_MLC_ASCII,               /* output cluster information in ASCII format (boolean) */
    CRITERIA_SECOND_CLUSTERS,       /* criteria for reporting secondary clusters (enumeration) */
    MAX_TEMPORAL_TYPE,              /* maximum temporal cluster size type (enumeration) */
    MAX_SPATIAL_TYPE,               /* maximum spatial cluster size type (enumeration) */
    RUN_HISTORY_FILENAME,           /* not used */
    OUTPUT_MLC_DBASE,               /* output cluster information in dBase format (boolean) */
    OUTPUT_AREAS_DBASE,             /* output location information in dBase format (boolean) */
    OUTPUT_RR_DBASE,                /* output relative risks in dBase format (boolean) */
    OUTPUT_SIM_LLR_DBASE,           /* output simulated log likelihoods ratios in dBase format (boolean) */
    NON_COMPACTNESS_PENALTY,        /* elliptic non-compactness penalty (enumeration) */
    INTERVAL_STARTRANGE,            /* flexible temporal window start range (string) */
    INTERVAL_ENDRANGE,              /* flexible temporal window end range (string) */
    TIMETRENDCONVRG,                /* time trend convergence (integer) */
    MAXCIRCLEPOPFILE,               /* max circle size filename (string) */
    EARLY_SIM_TERMINATION,          /* terminate simulations early for large p-values (boolean) */
    REPORTED_GEOSIZE,               /* max reported geographic size (numeric) */
    USE_REPORTED_GEOSIZE,           /* restrict reported clusters (boolean) */
    SIMULATION_TYPE,                /* analysis simulation method (enumeration) */
    SIMULATION_SOURCEFILE,          /* analysis simulation data source filename (string) */
    ADJ_BY_RR_FILE,                 /* adjustments by known relative risks filename (string) */
    OUTPUT_SIMULATION_DATA,         /* print analysis simulation data to file (boolean) */
    SIMULATION_DATA_OUTFILE,        /* analysis simulation data output filename (string) */
    ADJ_FOR_EALIER_ANALYSES,        /* adjust for earlier analyses (boolean) */
    USE_ADJ_BY_RR_FILE,             /* use adjustments by known relative risks file (boolean) */
    SPATIAL_ADJ_TYPE,               /* spatial adjustments type (enumeration) */
    MULTI_DATASET_PURPOSE_TYPE,     /* multiple data sets purpose type (enumeration) */
    CREATION_VERSION,               /* parameter file version (string) */
    RANDOMIZATION_SEED,             /* randomization seed (integer) */
    REPORT_CRITICAL_VALUES,         /* report critical values (boolean) */
    EXECUTION_TYPE,                 /* analysis execution method type (enumeration) */
    NUM_PROCESSES,                  /* number of parallel processes to execute (integer) */
    LOG_HISTORY,                    /* log analysis run to history file (boolean) */
    SUPPRESS_WARNINGS,              /* suppressing warnings (boolean) */
    MAX_REPORTED_SPATIAL_TYPE,      /* maximum spatial size type - reported clusters (enumeration) */
    OUTPUT_MLC_CASE_ASCII,          /* output cluster case information in ASCII format (boolean) */
    OUTPUT_MLC_CASE_DBASE,          /* output cluster case information in dBase format (boolean) */
    STUDYPERIOD_DATACHECK,          /* study period data check type (enumeration) */
    COORDINATES_DATACHECK,          /* geographical coordinates data check type (enumeration) */
    MAXGEOPOPATRISK,                /* maximum spatial size in population at risk (numeric) */
    MAXGEOPOPFILE,                  /* maximum spatial size in max circle population file (numeric) */
    MAXGEODISTANCE,                 /* maximum spatial size in distance from center (numeric) */
    USE_MAXGEOPOPFILE,              /* restrict maximum spatial size - max circle file (boolean) */
    USE_MAXGEODISTANCE,             /* restrict maximum spatial size - distance (boolean) */
    MAXGEOPOPATRISK_REPORTED,       /* maximum spatial size in population at risk for reported clusters (numeric) */
    MAXGEOPOPFILE_REPORTED,         /* maximum spatial size in max circle population file for reported clusters (numeric) */
    MAXGEODISTANCE_REPORTED,        /* maximum spatial size in distance from center for reported clusters (numeric) */
    USE_MAXGEOPOPFILE_REPORTED,     /* restrict maximum spatial size of reported clusters - max circle file (boolean) */
    USE_MAXGEODISTANCE_REPORTED,    /* restrict maximum spatial size of reported clusters - distance (boolean) */
    LOCATION_NEIGHBORS_FILE,        /* location neighbors filename (string) */
    USE_LOCATION_NEIGHBORS_FILE,    /* use location neighbors file (boolean) */
    RANDOMLY_GENERATE_SEED,         /* randomly generate simulation seed (boolean) */
    MULTIPLE_COORDINATES_TYPE,      /* multiple coordinates type (enumeration) */
    META_LOCATIONS_FILE,            /* meta locations filename (string) */
    USE_META_LOCATIONS_FILE,        /* use meta locations file (boolean) */
    OBSERVABLE_REGIONS,             /* polygon inequalities (string) */
    EARLY_TERM_THRESHOLD,           /* early termination threshold (integer) */
    PVALUE_REPORT_TYPE,             /* p-value reporting type (enumeration) */
    REPORT_GUMBEL,                  /* report Gumbel p-values (boolean) */
    TIME_TREND_TYPE,                /* spatial variation time trend type (enumeration) */
    REPORT_RANK,                    /* report cluster rank (boolean) */
    PRINT_ASCII_HEADERS,            /* print headers in ascii output files (boolean) */
    REPORT_HIERARCHICAL_CLUSTERS,   /* report hierarchical clusters (boolean) */
    REPORT_GINI_CLUSTERS,           /* report gini clusters (boolean) */
    SPATIAL_MAXIMA,                 /* spatial window maxima stops for gini (csv string) */
    GINI_INDEX_REPORT_TYPE,         /* gini cluster reporting type (enumeration) */
    GINI_INDEX_PVALUE_CUTOFF,       /* max p-value for clusters used in calculation of gini coefficients (numeric) */
    REPORT_GINI_COEFFICENTS,        /* report gini coefficents to results file (boolean) */
    PE_COUNT,                       /* total cases in power evaluation (integer) */
    PE_CRITICAL_TYPE,               /* critical values type (enumeration) */
    PE_ESTIMATION_TYPE,             /* power estimation type (enumeration) */
    PE_ALT_HYPOTHESIS_FILE,         /* altername htpothesis filename (string) */
    PE_POWER_REPLICAS,              /* number of replications in power step (integer) */
    PE_SIMULATION_TYPE,             /* power evaluation simulation method (enumeration) */
    PE_SIMULATION_SOURCEFILE,       /* power evaluation simulation data source filename (string) */
    PE_METHOD_TYPE,                 /* power evaluation method (enumeration) */
    POWER_001,                      /* power evaluation critical value .001 (numeric) */
    PE_OUTPUT_SIMUALTION_DATA,      /* report power evaluation's step 2 randomization data (boolean) */
    PE_SIMUALTION_OUTPUTFILE,       /* power evaluation simulation data output filename (string) */
    OUTPUT_KML,                     /* generate KML output file */
    OUTPUT_TEMPORAL_GRAPH,          /* generate temporal graph output file */
    TEMPORAL_GRAPH_REPORT_TYPE,     /* which clusters to generate temporal graph (enum)*/
    TEMPORAL_GRAPH_MLC_COUNT,       /* number of most likely clusters to generate temporal graph (integer)*/
    TEMPORAL_GRAPH_CUTOFF,          /* p-value cutoff for signicant clusters when generating temporal graph (numeric)*/
    OUTPUT_SHAPEFILES               /* generate shape files */,
    INCLUDE_LOCATIONS_KML,          /* whether to include cluster locations kml output */
    LOCATIONS_THRESHOLD_KML,        /* threshold for generating separate kml files for cluster locations */
    COMPRESS_KML_OUTPUT,            /* compress the kml output into a kmz file */
    LAUNCH_MAP_VIEWER,              /* whether to launch map viewer - gui only */
    ADJUST_WEEKLY_TRENDS,           /* adjust for weekly trends, nonparametric */
    MIN_TEMPORAL_CLUSTER,           /* minimum temporal cluster size */
    USER_DEFINED_TITLE,             /* user defined title */
    CALCULATE_OLIVEIRA,             /* whether to calculate Oliveira's F*/
    NUM_OLIVEIRA_SETS,              /* number of Oliveira sets to calculate */
    OLIVEIRA_CUTOFF,                /* max p-value for clusters used with Oliveira's F (numeric) */
    OUTPUT_CARTESIAN_GRAPH,         /* generate html cartesian graph file */
    RISK_LIMIT_HIGH_CLUSTERS,       /* restrict high clusters by risk level */      
    RISK_THESHOLD_HIGH_CLUSTERS,    /* threshold for high clusters by risk level */
    RISK_LIMIT_LOW_CLUSTERS,        /* restrict low clusters by risk level */
    RISK_THESHOLD_LOW_CLUSTERS,     /* threshold for low clusters by risk level */
    MIN_CASES_LOWRATE_CLUSTERS,     /* minimum number of cases in cluster when scanning low rates */
    MIN_CASES_HIGHRATE_CLUSTERS,    /* minimum number of cases in cluster when scanning high rates */
    LAUNCH_CARTESIAN_MAP,           /* whether to launch browser to view cartesian graph - no longer used */
    OUTPUT_GOOGLE_MAP               /* whether to generate google maps output */,
    PERFORM_STANDARD_DRILLDOWN,     /* whether to perform standard anaylsis drilldown */
    PERFORM_BERNOULLI_DRILLDOWN,    /* whether to perform Bernoulli anaylsis drilldown */
    DRILLDOWN_MIN_LOCATIONS,        /* minimum number of locations in detected cluster to perform drilldown */
    DRILLDOWN_MIN_CASES,            /* minimum number of cases in detected cluster to perform drilldown */
    DRILLDOWN_PVLAUE_CUTOFF,        /* maximum p-value in detected cluster to perform drilldown */
    DRILLDOWN_ADJ_WEEKLY_TRENDS,    /* whether to adjust for weekly trends in Bernoulli drilldown, nonparametric */
    USE_NETWORK_FILE,               /* whether to use locations network file */
    NETWORK_FILE,                   /* locations network filename */
    NETWORK_PURPOSE,                /* purpose of network file */
    PROSPECTIVE_FREQ_TYPE,          /* frequency of prospective analysis type */
    PROSPECTIVE_FREQ                /* frequency of prospective analysis type */
};
/** frequency of prospective analyses */
enum ProspectiveFrequency {
    SAME_TIMEAGGREGATION=0,
    DAILY,
    WEEKLY,
    MONTHLY,
    QUARTERLY,
    YEARLY
};
/** network file purpose */
enum NetworkPurposeType {
    COORDINATES_OVERRIDE=0,
    NETWORK_DEFINITION
};
/** analysis and cluster types */
enum AnalysisType
{
    PURELYSPATIAL=1, 
    PURELYTEMPORAL, 
    SPACETIME,  
    PROSPECTIVESPACETIME,
    SPATIALVARTEMPTREND, 
    PROSPECTIVEPURELYTEMPORAL,
    SEASONALTEMPORAL
};
/** cluster types */
enum ClusterType
{
    PURELYSPATIALCLUSTER=1, 
    PURELYTEMPORALCLUSTER, 
    SPACETIMECLUSTER,
    SPATIALVARTEMPTRENDCLUSTER, 
    PURELYSPATIALMONOTONECLUSTER, 
    PURELYSPATIALPROSPECTIVECLUSTER,
    PURELYSPATIALHOMOGENEOUSCLUSTER
};
/** probability model types */
enum ProbabilityModelType
{
    POISSON=0, 
    BERNOULLI, 
    SPACETIMEPERMUTATION, 
    ORDINAL, 
    EXPONENTIAL, 
    NORMAL, 
    HOMOGENEOUSPOISSON, 
    CATEGORICAL, 
    RANK,
    UNIFORMTIME
};
enum IncludeClustersType
{
    ALLCLUSTERS=0, 
    ALIVECLUSTERS, 
    CLUSTERSINRANGE
};
enum RiskType
{
    STANDARDRISK=0, 
    MONOTONERISK
};
/** area incidence rate types */
enum AreaRateType
{
    HIGH=1, 
    LOW, 
    HIGHANDLOW
};
/** time trend adjustment types */
enum TimeTrendAdjustmentType
{
    TEMPORAL_NOTADJUSTED=0,
    TEMPORAL_NONPARAMETRIC,
    LOGLINEAR_PERC,
    CALCULATED_LOGLINEAR_PERC, 
    TEMPORAL_STRATIFIED_RANDOMIZATION,
    CALCULATED_QUADRATIC
};
/** spatial adjustment types */
enum SpatialAdjustmentType
{
    SPATIAL_NOTADJUSTED=0,
    SPATIAL_STRATIFIED_RANDOMIZATION,
    SPATIAL_NONPARAMETRIC
};
enum CoordinatesType
{
    CARTESIAN=0, 
    LATLON
};
/** criteria for reporting secondary clusters types */
enum CriteriaSecondaryClustersType
{
    NOGEOOVERLAP=0, 
    NOCENTROIDSINOTHER, 
    NOCENTROIDSINMORELIKE,
    NOCENTROIDSINLESSLIKE, 
    NOPAIRSINEACHOTHERS, 
    NORESTRICTIONS
};
/** interperation types for maximum temporal size */
enum TemporalSizeType
{
    PERCENTAGETYPE=0, 
    TIMETYPE
};
/** interperation types for maximum spatial size */
enum SpatialSizeType
{
    PERCENTOFPOPULATION=0, 
    MAXDISTANCE, 
    PERCENTOFMAXCIRCLEFILE
};
/** defines how simulated data will be generated */
enum SimulationType
{
    STANDARD=0, 
    HA_RANDOMIZATION, 
    FILESOURCE
};
/** purpose of multiple data sets */
enum MultipleDataSetPurposeType
{
    MULTIVARIATE=0, 
    ADJUSTMENT
};
/** analysis execution type */
enum ExecutionType
{
    AUTOMATIC=0, 
    SUCCESSIVELY, 
    CENTRICALLY
};
/** spatial window shape */
enum SpatialWindowType
{
    CIRCULAR=0, 
    ELLIPTIC
};
/** non-compactness penalty type */
enum NonCompactnessPenaltyType
{
    NOPENALTY=0, 
    MEDIUMPENALTY, 
    STRONGPENALTY
};
/** study period data checking type */
enum StudyPeriodDataCheckingType
{
    STRICTBOUNDS=0, 
    RELAXEDBOUNDS
};
/** geographical coordinates data checking type */
enum CoordinatesDataCheckingType
{
    STRICTCOORDINATES=0, 
    RELAXEDCOORDINATES
};
/** multiple coordinates type */
enum MultipleCoordinatesType
{
    ONEPERLOCATION=0, 
    ATLEASTONELOCATION, 
    ALLLOCATIONS
};
/** p-values reporting type */
enum PValueReportingType
{
    DEFAULT_PVALUE=0, 
    STANDARD_PVALUE, 
    TERMINATION_PVALUE, 
    GUMBEL_PVALUE
};
/** time trend calculation type */
enum TimeTrendType
{
    LINEAR=0,
    QUADRATIC
};
/** cluster reporting type */
enum ClusterReportType
{
    HIERARCHICAL=0, 
    GINI_INDEX, 
    ALL_CLUSTER_TYPES
};
/** Gini index cluster reporting type*/
enum GiniIndexReportType
{
    OPTIMAL_ONLY=0, 
    ALL_VALUES
};
/** critical values calculation type */
enum CriticalValuesType
{
    CV_MONTECARLO=0,                /* standard monte carlo */
    CV_GUMBEL,                      /* Gumbel approximation */
    CV_POWER_VALUES                 /* user specified values */
};
/** power estimation calculation type */
enum PowerEstimationType
{
    PE_MONTECARLO=0,                /* standard monte carlo */
    PE_GUMBEL                       /* Gumbel approximation */
};
/** power evaluation method */
enum PowerEvaluationMethodType
{
    PE_WITH_ANALYSIS=0,             /* execute standard analysis and power evaluation together */
    PE_ONLY_CASEFILE,               /* execute only power evaluation, using total cases from case file */
    PE_ONLY_SPECIFIED_CASES         /* execute only power evaluation, using user specified total cases */
};
/** temporal graph reporting type */
enum TemporalGraphReportType
{
    MLC_ONLY=0,                    /* generate a temporal graph for the most likley cluster only */
    X_MCL_ONLY,                    /* generate a temporal graph for the first X likley clusters */
    SIGNIFICANT_ONLY               /* generate a temporal graph for significant clusters only */
};
//*****************************************************************************
#endif
