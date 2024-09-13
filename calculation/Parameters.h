//*****************************************************************************
#ifndef __PARAMETERS_H_
#define __PARAMETERS_H_
//*****************************************************************************
#include "SaTScan.h"
#include "JulianDates.h"
#include "UtilityFunctions.h"
#include "FileName.h"
#include "ParameterTypes.h"

#include <boost/date_time.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>

class CParameters {
  public:
    struct CreationVersion {
        unsigned int iMajor; 
        unsigned int iMinor; 
        unsigned int iRelease;

        CreationVersion() : iMajor(static_cast<unsigned int>(std::atoi(VERSION_MAJOR))),
                            iMinor(static_cast<unsigned int>(std::atoi(VERSION_MINOR))),
                            iRelease(static_cast<unsigned int>(std::atoi(VERSION_RELEASE))) {}
        CreationVersion(unsigned int major, unsigned int minor, unsigned int release) : iMajor(major), iMinor(minor), iRelease(release) {}

        bool operator<(const CreationVersion& other) const {
            if (iMajor == other.iMajor) {
                if (iMinor == other.iMinor) {
                    return iRelease < other.iRelease;
                } else {
                    return iMinor < other.iMinor;
                }
            } else {
                return iMajor < other.iMajor;
            }
        }
        bool operator==(const CreationVersion& other) const {
            return iMajor == other.iMajor && iMinor == other.iMinor && iRelease == other.iRelease;
        }
        bool operator!=(const CreationVersion& other) const {
            return !(*this == other);
        }
    };
    class InputSource {
        public:
            enum ShapeCoordinatesType {LATLONG_DATA=0, UTM_CONVERSION, CARTESIAN_DATA};

        private:
            SourceType _source_type;
            FieldMapContainer_t _fields_map;
            // CSV specific options
            std::string _delimiter;
            std::string _grouper;
            unsigned int _skip;
            bool _first_row_headers;
            LineListFieldMapContainer_t _ll_fields_map;

        public:
            InputSource() : _skip(0), _first_row_headers(false) {}

            InputSource(SourceType type, FieldMapContainer_t map):
                _source_type(type), _fields_map(map), 
                _delimiter(","), _grouper("\""), _skip(0), _first_row_headers(false) {}

            InputSource(SourceType type, std::string delimiter, std::string grouper, unsigned int skip, bool first_row_headers) :
                _source_type(type), _delimiter(delimiter), _grouper(grouper), _skip(skip), _first_row_headers(first_row_headers) {}


            InputSource(SourceType type, FieldMapContainer_t map, std::string delimiter, std::string grouper, unsigned int skip, bool first_row_headers):
                _source_type(type), _fields_map(map), 
                _delimiter(delimiter), _grouper(grouper), _skip(skip), _first_row_headers(first_row_headers) {}

            SourceType getSourceType() const {return _source_type;}
            void setSourceType(SourceType e) {_source_type = e;}
            const FieldMapContainer_t & getFieldsMap() const {return _fields_map;}
            void setFieldsMap(const FieldMapContainer_t& m) {_fields_map = m;}
            void clearFieldsMap() {_fields_map.clear();}
            // CSV specific options
            const std::string & getDelimiter() const {return _delimiter;}
            void setDelimiter(const std::string& s) {_delimiter = s;}
            const std::string & getGroup() const {return _grouper;}
            void setGroup(const std::string& s) {_grouper = s;}
            unsigned int getSkip() const {return _skip;}
            void setSkip(unsigned int u) {_skip = u;}
            bool getFirstRowHeader() const {return _first_row_headers;}
            void setFirstRowHeader(bool b) {_first_row_headers = b;}

            const LineListFieldMapContainer_t & getLinelistFieldsMap() const { return _ll_fields_map; }
            void setLinelistFieldsMap(const LineListFieldMapContainer_t& m) { _ll_fields_map = m; }
            void clearLinelistFieldsMap() { _ll_fields_map.clear(); }
    };

  public:
    typedef std::pair<ParameterType, unsigned int> InputSourceKey_t; // ParameterType , data set index
    typedef std::map<InputSourceKey_t, InputSource> InputSourceContainer_t;

  private:
    InputSourceContainer_t              _input_sources;                         /** input parameter source */
    MultipleCoordinatesType             geMultipleCoordinatesType;              /** multiple locations type */
    unsigned int                        giNumRequestedParallelProcesses;        /** number of parallel processes to run */
    ExecutionType                       geExecutionType;                        /** execution process type */
    MultipleDataSetPurposeType          geMultipleSetPurposeType;               /** purpose for multiple data sets */
    AnalysisType                        geAnalysisType;                         /** analysis type */
    ProbabilityModelType                geProbabilityModelType;                 /** probability model type */
    AreaRateType                        geAreaScanRate;                         /** areas incidence rate type of interest */
    RiskType                            geRiskFunctionType;                     /**  */
    IncludeClustersType                 geIncludeClustersType;
    unsigned int                        giReplications;                         /** number of MonteCarlo replicas */
    CriteriaSecondaryClustersType       geCriteriaSecondClustersType;           /** Criteria for Reporting Secondary Clusters */
    double                              gdTimeTrendConverge;                    /** time trend convergence value for SVTT */
    SimulationType                      _simulationType;                        /** indicates simulation procedure */
    bool                                gbOutputSimulationData;                 /** indicates whether to output simulation data to file */
    bool                                gbAdjustForEarlierAnalyses;             /** indicates whether to adjust for earlier analyses,
                                                                                    pertinent for prospective analyses */
    SpatialAdjustmentType               geSpatialAdjustmentType;                /** type of spatial adjustment*/
    StudyPeriodDataCheckingType         geStudyPeriodDataCheckingType;          /** study period data checking type */
    CoordinatesDataCheckingType         geCoordinatesDataCheckingType;          /** geographical coordinates data checking type */
    /* Maximum spatial cluster variables */
    double                              gdMaxSpatialSizeInPopulationAtRisk;
    bool                                gbRestrictMaxSpatialSizeThroughMaxCirclePopulationFile;
    double                              gdMaxSpatialSizeInMaxCirclePopulationFile;
    bool                                gbRestrictMaxSpatialSizeThroughDistanceFromCenter;
    double                              gdMaxSpatialSizeInMaxDistanceFromCenter;
    /* Reported Maximum spatial cluster variables */
    bool                                gbRestrictReportedClusters;             /** indicates whether reported clusters are limited to specified maximum size */
    double                              gdMaxSpatialSizeInPopulationAtRisk_Reported;
    bool                                gbRestrictMaxSpatialSizeThroughMaxCirclePopulationFile_Reported;
    double                              gdMaxSpatialSizeInMaxCirclePopulationFile_Reported;
    bool                                gbRestrictMaxSpatialSizeThroughDistanceFromCenter_Reported;
    double                              gdMaxSpatialSizeInMaxDistanceFromCenter_Reported;
    /* Maximum temporal cluster variables */
    double                              gdMaxTemporalClusterSize;               /** maximum value for temporal cluster */
    TemporalSizeType                    geMaxTemporalClusterSizeType;           /** maximum temporal cluster value type */
    /* Time interval variables */
    long                                glTimeAggregationLength;                /** length of time aggregation */
    DatePrecisionType                   geTimeAggregationUnitsType;             /** time aggregation units type */
    /* Temporal trend adjusment variables */
    double                              gdTimeTrendAdjustPercentage;            /** percentage for log linear adjustment */
    TimeTrendAdjustmentType             geTimeTrendAdjustType;                  /** Adjust for time trend: no, discrete, % */
    bool                                _adjustWeeklyTrends;                    /** Adjust for weekly trends. */
    /* Input precision variables */
    DatePrecisionType                   gePrecisionOfTimesType;                 /** precision of case/control data: none = no, years=months=days = yes */
    CoordinatesType                     geCoordinatesType;                      /** coordinates type for coordinates/special grid */
    /* Ellipse variables */
    long                                glTotalNumEllipses;                     /** total number of ellipses (ellipses by number rotations) */
    std::vector<double>                 gvEllipseShapes;                        /** shape of each ellipsoid */
    std::vector<int>                    gvEllipseRotations;                     /** number of rotations for each ellipsoid */
    NonCompactnessPenaltyType           geNonCompactnessPenaltyType;            /** indicates penalty for narrower ellipses */
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
    bool                                gbOutputClusterCaseAscii,               /** indicates whether to output most likely cluster cases for each centroid in ascii format */
                                        gbOutputClusterCaseDBase;               /** indicates whether to output most likely cluster cases for each centroid in dBase format */
    bool                                gbOutputAreaSpecificAscii,              /** indicates whether to output tract/location information of reported(.i.e top ranked) clusters in ascii format */
                                        gbOutputAreaSpecificDBase;              /** indicates whether to output tract/location information of reported(.i.e top ranked) clusters in dBase format */
    bool                                _output_kml;                            /** generate KML output file */
    bool                                _include_locations_kml;                 /** include cluster locations in kml output */
    unsigned int                        _locations_threshold_kml;               /** threshold before cluster locations are put into separate kml files */
    bool                                _compress_kml_output;                   /** compress kml output into kmz format */
    bool                                _launch_map_viewer;                     /** launch map viewer */
    bool                                _calculate_oliveira_f;                  /** whether to calculate Oliveira's F */
    unsigned int                        _num_oliveira_sets;                     /** number of data sets to evaluate for Oliveira's F */
    double                              _oliveira_pvalue_cutoff;                /** P-Value used to limit clusters in Oliveira's F calcuation */

    bool                                _output_google_map;                     /** generate google maps output file */

    /* temporal clusters graph */
    bool                                _output_temporal_graph;                 /** generate temporal graph output file */
    TemporalGraphReportType             _temporal_graph_report_type;            /* which clusters to report in temporal graph */
    int                                 _temporal_graph_report_count;           /* number of MLC clusters to graph with TemporalGraphReportType.X_MCL_ONLY */
    double                              _temporal_graph_report_cutoff;          /* P-Value used limit graphed clusters with TemporalGraphReportType.SIGNIFICANT_ONLY */

    bool                                _output_shapefiles;                     /** generate shapefile output */

    /* Iterative scans variables */
    bool                                gbIterativeRuns;                        /** iterative analysis? */
    unsigned int                        giNumIterativeRuns;                     /** number of iterative scans to attempt */
    double                              gbIterativeCutOffPValue;                /** P-Value used to exit iterative analysis */
    /* Input/Output filenames */
    std::string                         gsParametersSourceFileName;             /** parameters source filename */
    std::vector<std::string>            gvCaseFilenames;                        /** case data source filenames */
    std::string                         _linelist_individuals_cache_name;
    double                              _linelist_csv_cutoff;                   /** cutoff value when restricting clusters added to line list csv */
    std::vector<std::string>            gvControlFilenames;                     /** control data source filenames */
    std::vector<std::string>            gvPopulationFilenames;                  /** population data source filenames */
    bool                                gbUsePopulationFile;                    /** indicates whether population data will be read given other parameter settings */
    std::string                         gsCoordinatesFileName;                  /** coordinates data source filename */
    std::string                         gsSpecialGridFileName;                  /** special grid data source filename */
    bool                                gbUseSpecialGridFile;                   /** indicator of special grid file usage */
    std::string                         gsMaxCirclePopulationFileName;          /** special population file for constructing circles only */
    std::string                         gsOutputFileNameSetting;                /** results output filename - user specified filename to write results - can include subtitution variables */
    mutable std::string                 _results_filename;                      /** results output filename but with any substitutions (getFilenameFormatTime) */
    bool                                gbLogRunHistory;                        /** indicates whether to log history */
    std::string                         gsSimulationDataSourceFileName;         /** simulation data source filename */
    bool                                gbUseAdjustmentsForRRFile;              /** indicates whether to use adjustments for known relative risks file */
    std::string                         gsAdjustmentsByRelativeRisksFileName;   /** adjustments by known relative risks filename */
    std::string                         gsSimulationDataOutputFilename;         /** simulation data output filename */
    std::string                         gsLocationNeighborsFilename;            /** sorted array neighbor file */
    bool                                gbUseLocationNeighborsFile;             /** use sorted array neighbor file? */
    std::string                         gsMetaLocationsFilename;                /** meta locations file -- neighbors file */
    bool                                gbUseMetaLocationsFile;                 /** use meta locations file? */
    std::string                         _multiple_locations_file;               /** file which defines multiple locations for a group */
    /* Analysis dates */
    std::string                         gsProspectiveStartDate;                 /** prospective start date in YYYY/MM/DD, YYYY/MM, or YYYY format */
    std::string                         gsStudyPeriodStartDate;                 /** study period start date in YYYY/MM/DD, YYYY/MM, or YYYY format */
    std::string                         gsStudyPeriodEndDate;                   /** study period end date in YYYY/MM/DD, YYYY/MM, or YYYY format */
    std::string                         gsEndRangeStartDate;
    std::string                         gsEndRangeEndDate;
    std::string                         gsStartRangeStartDate;
    std::string                         gsStartRangeEndDate;
    struct CreationVersion              gCreationVersion;
    long                                glRandomizationSeed;                    /** randomization seed */
    bool                                gbRandomlyGenerateSeed;                 /** indicates whether to randomly generate seed */
    bool                                gbReportCriticalValues;                 /** indicates whether to report critical llr values */
    bool                                gbSuppressWarnings;                     /** indicates whether to suppres warnings printed during execution */
    SpatialWindowType                   geSpatialWindowType;                    /** spatial window shape */
    std::vector<std::string>            gvObservableRegions;                    /** collection of observable regions */
    bool                                gbWeightedNormal;                       /** convenience variable - normal model is weighted*/
    bool                                gbWeightedNormalCovariates;             /** convenience variable - normal model is weighted with covariates */
    unsigned int                        giEarlyTermThreshold;                   /** early termination threshold */
    PValueReportingType                 gePValueReportingType;                  /** p-value reporting type */
    bool                                gbReportGumbelPValue;                   /** report Gumbel p-value */
    TimeTrendType                       geTimeTrendType;                        /** time trend type */
    bool                                gbReportRank;                           /** report cluster rank */
    bool                                gbPrintAsciiHeaders;                    /** print headers in ascii output files */
    std::vector<double>                 gvSpatialWindowStops;                   /** spatial window stops */
    std::string                         gsTitleName;
    double                              _giniIndexPValueCutoff;                 /* P-Value used to limit clusters in gini index calcuation */
    bool                                _reportHierarchicalClusters;
    bool                                _reportGiniOptimizedClusters;
    GiniIndexReportType                 _giniIndexReportType;                   /* type for the gini index cluster reporting */
    bool                                _reportGiniIndexCoefficients;           /* report gini index coefficents to results file */
    mutable std::vector<double>         _executeSpatialWindowStops;
    mutable std::string                 _cluster_moniker_prefix;                // prefix for cluster moniker

    /* Power Calcution variables */
    double                              _critical_value_05;                     /* user specified critical value at .05 */
    double                              _critical_value_01;                     /* user specified critical value at .01 */
    double                              _critical_value_001;                    /* user specified critical value at .001 */
    bool                                _performPowerEvaluation;
    count_t                             _powerEvaluationTotalCases; 
    CriticalValuesType                  _critical_value_type;
    PowerEstimationType                 _power_estimation_type;
    std::string                         _power_alt_hypothesis_filename;
    unsigned int                        _power_replica;                          /** number of replicas in power step  of power evaluation */
    SimulationType                      _power_simulation_type;                  /* indicates simulation procedure for step 2 of power evaluation */
    std::string                         _power_simulation_source_filename;       /* source filename for power evaluation's randomization - file import method */
    bool                                _report_power_simulation_data;           /* report power simulation data to file */
    std::string                         _power_simulation_output_filename;       /* output filename for power evaluation's randomization */
    PowerEvaluationMethodType           _power_evaluation_method;                /* power evaluation method */
    unsigned int                        _minimum_temporal_cluster_size;          /* mimimum temporal cluster size in time aggregation units */

    bool                                _output_cartesian_graph;                 /** generate KML output file */
    bool                                _risk_limit_high_clusters;               /** restrict high rate clusters by risk level */
    double                              _risk_threshold_high_clusters;           /** threshold for high rate clusters */
    bool                                _risk_limit_low_clusters;                /** restrict low rate clusters by risk level */
    double                              _risk_threshold_low_clusters;            /** threshold for low rate clusters */

    unsigned int                        _minimum_low_rate_cases;                 /** minimum number of cases in cluster when scanning low rates */
    unsigned int                        _minimum_high_rate_cases;                /** minimum number of cases in cluster when scanning high rates */

    bool                                _perform_standard_drilldown;
    bool                                _perform_bernoulli_drilldown;
    unsigned int                        _drilldown_minimum_locations;
    unsigned int                        _drilldown_minimum_cases;
    double                              _drilldown_cutoff;
    bool                                _drilldown_adjust_weekly_trends;
    std::vector<std::string>            _drilldown_result_filenames;

    std::string                         _locations_network_filename;
    bool                                _use_locations_network_file;

    ProspectiveFrequency                _prospective_frequency_type;
    unsigned int                        _prospective_frequency;

    bool                                _always_email_summary;
    std::string                         _email_always_recipients;
    bool                                _cutoff_email_summary;
    std::string                         _email_cutoff_recipients;
    double                              _cutoff_email_value;
    bool                                _email_attach_results;
    bool                                _email_include_results_directory;
    bool                                _email_custom;
    std::string                         _email_custom_subject;
    std::string                         _email_custom_message_body;
    bool                                _create_email_summary_file;
    double                              _email_summary_cutoff;

    boost::posix_time::ptime            _local_timestamp; // approxiate analysis start time

    void                                AssignMissingPath(std::string & sInputFilename, bool bCheckWritable=false);
    void                                Copy(const CParameters &rhs);
    const char                        * GetRelativeToParameterName(const FileName& fParameterName, const std::string& sFilename, std::string& sValue) const;
    void                                checkEnumeration(int e, int eLow, int eHigh) const;

  public:
    CParameters();
    CParameters(const CParameters &other);

    static const int                    giNumParameters;                        // number enumerated parameters
    static const int                    MAXIMUM_ITERATIVE_ANALYSES;             // maximum number of permitted iterative scans
    static const int                    MAXIMUM_ELLIPSOIDS;                     // maximum number of permitted ellipsoids

    bool                                _cluster_sig_by_ri_; // This class members are used to facilitate a refactor in v10.2.
    DatePrecisionType                   _cluster_sig_ri_type_;
    double                              _cluster_sig_ri_val_;
    bool                                _cluster_sig_by_p_;
    double                              _cluster_sig_p_val_;


    CParameters                       & operator=(const CParameters &rhs);
    bool                                operator==(const CParameters& rhs) const;
    bool                                operator!=(const CParameters& rhs) const;

    double                              getCutoffLineListCSV() const { return _linelist_csv_cutoff; }
    void                                setCutoffLineListCSV(double d) { _linelist_csv_cutoff = d; }

    const std::string                 & getMultipleLocationsFile() const { return _multiple_locations_file; }
    void                                setMultipleLocationsFile(const char * filename, bool bCorrectForRelativePath = false);
    bool                                getAlwaysEmailSummary() const { return _always_email_summary; }
    void                                setAlwaysEmailSummary(bool b) { _always_email_summary = b; }
    const std::string                 & getEmailAlwaysRecipients() const { return _email_always_recipients; }
    void                                setEmailAlwaysRecipients(const char* s) { _email_always_recipients = s; }
    std::vector<std::string>            getEmailAlwaysRecipientsList() const {
                                            std::vector<std::string> recipients;
                                            csv_string_to_typelist<std::string>(_email_always_recipients.c_str(), recipients);
                                            return recipients;
                                        }
    bool                                getCutoffEmailSummary() const { return _cutoff_email_summary; }
    void                                setCutoffEmailSummary(bool b) { _cutoff_email_summary = b; }
    const std::string                 & getEmailCutoffRecipients() const { return _email_cutoff_recipients; }
    void                                setEmailCutoffRecipients(const char* s) { _email_cutoff_recipients = s; }
    std::vector<std::string>            getEmailCutoffRecipientsList() const {
                                            std::vector<std::string> recipients;
                                            csv_string_to_typelist<std::string>(_email_cutoff_recipients.c_str(), recipients);
                                            return recipients;
                                        }
    double                              getCutoffEmailValue() const { return _cutoff_email_value; }
    void                                setCutoffEmailValue(double d) { _cutoff_email_value = d; }
    bool                                getEmailAttachResults() const { return _email_attach_results; }
    void                                setEmailAttachResults(bool b) { _email_attach_results = b; }
    bool                                getEmailIncludeResultsDirectory() const { return _email_include_results_directory; }
    void                                setEmailIncludeResultsDirectory(bool b) { _email_include_results_directory = b; }
    bool                                getEmailCustom() const { return _email_custom; }
    void                                setEmailCustom(bool b) { _email_custom = b; }
    const std::string                 & getEmailCustomSubject() const { return _email_custom_subject; }
    void                                setEmailCustomSubject(const char * s) { _email_custom_subject = s; }
    const std::string                 & getEmailCustomMessageBody() const { return _email_custom_message_body; }
    void                                setEmailCustomMessageBody(const char * s) { _email_custom_message_body = s; }
    bool                                getCreateEmailSummaryFile() const { return _create_email_summary_file; }
    void                                setCreateEmailSummaryFile(bool b) { _create_email_summary_file = b; }
    double                              getEmailSummaryValue() const { return _email_summary_cutoff; }
    void                                setEmailSummaryValue(double d) { _email_summary_cutoff = d; }
    ProspectiveFrequency                getProspectiveFrequencyType() const { return _prospective_frequency_type; }
    void                                setProspectiveFrequencyType(ProspectiveFrequency e);
    unsigned int                        getProspectiveFrequency() const { return _prospective_frequency; }
    void                                setProspectiveFrequency(unsigned int i) { _prospective_frequency = i; }
    bool                                getReadingLineDataFromCasefile() const;

    boost::posix_time::ptime            getTimestamp() const { return _local_timestamp; }
    void                                setTimestamp(boost::posix_time::ptime ts) { _local_timestamp = ts; }
    bool                                getUseLocationsNetworkFile() const { return _use_locations_network_file; }
    void                                setUseLocationsNetworkFile(bool b) { _use_locations_network_file = b; }
    const std::string                 & getLocationsNetworkFilename() const { return _locations_network_filename; }
    void                                setLocationsNetworkFilename(const char * filename, bool bCorrectForRelativePath = false);

    const std::string                 & getClusterMonikerPrefix() const { return _cluster_moniker_prefix; }
    void                                setClusterMonikerPrefix(const std::string& s) { _cluster_moniker_prefix = s; }
    bool                                getPerformStandardDrilldown() const { return _perform_standard_drilldown; }
    void                                setPerformStandardDrilldown(bool b) { _perform_standard_drilldown =b; }
    bool                                getPerformBernoulliDrilldown() const { return _perform_bernoulli_drilldown; }
    void                                setPerformBernoulliDrilldown(bool b) { _perform_bernoulli_drilldown = b; }
    unsigned int                        getDrilldownMinimumLocationsCluster() const { return _drilldown_minimum_locations; }
    void                                setDrilldownMinimumLocationsCluster(unsigned int u) { _drilldown_minimum_locations = u; }
    unsigned int                        getDrilldownMinimumCasesCluster() const { return _drilldown_minimum_cases; }
    void                                setDrilldownMinimumCasesCluster(unsigned int u) { _drilldown_minimum_cases = u; }
    double                              getDrilldownCutoff() const { return _drilldown_cutoff; }
    void                                setDrilldownCutoff(double d) { _drilldown_cutoff = d; }
    bool                                getDrilldownAdjustWeeklyTrends() const { return _drilldown_adjust_weekly_trends; }
    void                                setDrilldownAdjustWeeklyTrends(bool b) { _drilldown_adjust_weekly_trends = b; }
    void                                addDrilldownResultFilename(const std::string& s) { _drilldown_result_filenames.push_back(s);  }
    const std::vector<std::string>    & getDrilldownResultFilename() const { return _drilldown_result_filenames; }

    bool                                requestsGeogrphaicalOutput() const {	return _output_google_map || _output_kml || _output_shapefiles || _output_cartesian_graph;	}
    void                                toggleGeogrphaicalOutput(bool b) { _output_google_map = b; _output_kml = b; _output_shapefiles = b; _output_cartesian_graph = b; }

    bool                                getOutputGoogleMapsFile() const { return _output_google_map; }
    void                                setOutputGoogleMapsFile(bool b) { _output_google_map = b; }

    unsigned int                        getMinimumCasesLowRateClusters() const { return _minimum_low_rate_cases; }
    void                                setMinimumCasesLowRateClusters(unsigned int u) { _minimum_low_rate_cases = u; }
    unsigned int                        getMinimumCasesHighRateClusters() const { return _minimum_high_rate_cases; }
    void                                setMinimumCasesHighRateClusters(unsigned int u) { _minimum_high_rate_cases = u; }

    bool                                getRiskLimitHighClusters() const { return _risk_limit_high_clusters; }
    void                                setRiskLimitHighClusters(bool b) { _risk_limit_high_clusters = b; }
    double                              getRiskThresholdHighClusters() const { return _risk_threshold_high_clusters; }
    void                                setRiskThresholdHighClusters(double d) { _risk_threshold_high_clusters = d; }
    bool                                getRiskLimitLowClusters() const { return _risk_limit_low_clusters; }
    void                                setRiskLimitLowClusters(bool b) { _risk_limit_low_clusters = b; }
    double                              getRiskThresholdLowClusters() const { return _risk_threshold_low_clusters; }
    void                                setRiskThresholdLowClusters(double d) { _risk_threshold_low_clusters = d; }

    double                              getTemporalGraphSignificantCutoff() const {return _temporal_graph_report_cutoff;}
    void                                setTemporalGraphSignificantCutoff(double d) {_temporal_graph_report_cutoff = d;}
    int                                 getTemporalGraphMostLikelyCount() const {return _temporal_graph_report_count;}
    void                                setTemporalGraphMostLikelyCount(int i) {_temporal_graph_report_count = i;}
    TemporalGraphReportType             getTemporalGraphReportType() const {return _temporal_graph_report_type;}
    void                                setTemporalGraphReportType(TemporalGraphReportType e);
    void                                AddEllipsoidShape(double dShape, bool bEmptyFirst);
    void                                AddEllipsoidRotations(int iRotations, bool bEmptyFirst);
    void                                AddObservableRegion(const char * sRegions, size_t iIndex, bool bEmptyFirst);
    void                                AddSpatialWindowStop(double windowStop, bool bEmptyFirst);
    void                                defineInputSource(ParameterType e, InputSource source, unsigned int dataset=1) {_input_sources[std::make_pair(e,dataset)] = source;}
    bool                                GetAdjustForEarlierAnalyses() const {return gbAdjustForEarlierAnalyses;}
    bool                                getAdjustForWeeklyTrends() const {return _adjustWeeklyTrends;}
    const std::string                 & GetAdjustmentsByRelativeRisksFilename() const {return gsAdjustmentsByRelativeRisksFileName;}
    AnalysisType                        GetAnalysisType() const {return geAnalysisType;}
    AreaRateType                        GetAreaScanRateType() const {return geAreaScanRate;}
    bool                                getCalculateOliveirasF() const {return _calculate_oliveira_f;}
    AreaRateType                        GetExecuteScanRateType() const;
    unsigned int                        GetExecuteEarlyTermThreshold() const;
    const std::string                 & GetCaseFileName(size_t iSetIndex=1) const;
    const std::vector<std::string>    & GetCaseFileNames() const {return gvCaseFilenames;}
    const InputSourceContainer_t      & getInputSources() const {return _input_sources;}
    const InputSource                 * getInputSource(ParameterType e, unsigned int dataset=1) const {
                                            InputSourceContainer_t::const_iterator itr = _input_sources.find(std::make_pair(e,dataset));
                                            return itr == _input_sources.end() ? (const InputSource*)0 : &(itr->second);
                                        }
    bool                                getReportHierarchicalClusters() const {return _reportHierarchicalClusters;}
    bool                                getReportGiniOptimizedClusters() const {return _reportGiniOptimizedClusters;}
    const std::string                 & GetControlFileName(size_t iSetIndex=1) const;
    const std::vector<std::string>    & GetControlFileNames() const {return gvControlFilenames;}
    CoordinatesDataCheckingType         GetCoordinatesDataCheckingType() const {return geCoordinatesDataCheckingType;}
    const std::string                 & GetCoordinatesFileName() const {return gsCoordinatesFileName;}
    CoordinatesType                     GetCoordinatesType() const {return geCoordinatesType;}
    const CreationVersion            &  GetCreationVersion() const {return gCreationVersion;}
    CriteriaSecondaryClustersType       GetCriteriaSecondClustersType() const {return geCriteriaSecondClustersType;}
    unsigned int                        GetCreationVersionMajor() const {return gCreationVersion.iMajor;}
    const std::vector<int>            & GetEllipseRotations() const {return gvEllipseRotations;}
    const std::vector<double>         & GetEllipseShapes() const {return gvEllipseShapes;}
    const std::string                 & GetEndRangeEndDate() const {return gsEndRangeEndDate;}
    const std::string                 & GetEndRangeStartDate() const {return gsEndRangeStartDate;}
    unsigned int                        GetEarlyTermThreshold() const {return giEarlyTermThreshold;}
    ExecutionType                       GetExecutionType() const {return geExecutionType;}
    double                              getGiniIndexPValueCutoff() const {return _giniIndexPValueCutoff;}
    bool                                getIncludeLocationsKML() const {return _include_locations_kml;}
    bool                                getCompressClusterKML() const {return _compress_kml_output;}
    unsigned int                        getLocationsThresholdKML() const {return _locations_threshold_kml;}
    IncludeClustersType                 GetIncludeClustersType() const {return geIncludeClustersType;}
    bool                                GetIncludePurelySpatialClusters() const {return gbIncludePurelySpatialClusters;}
    bool                                GetIncludePurelyTemporalClusters() const {return gbIncludePurelyTemporalClusters;}
    GiniIndexReportType                 getGiniIndexReportType() const {return _giniIndexReportType;}
    bool                                GetIsLoggingHistory() const {return gbLogRunHistory;}
    bool                                GetIsProspectiveAnalysis() const;
    bool                                GetIsPurelySpatialAnalysis() const;
    bool                                GetIsPurelyTemporalAnalysis() const;
    bool                                GetIsIterativeScanning() const {return gbIterativeRuns;}
    bool                                GetIsRandomlyGeneratingSeed() const {return gbRandomlyGenerateSeed;}
    bool                                GetIsSpaceTimeAnalysis() const;
    bool                                getLaunchMapViewer() const {return _launch_map_viewer;}
    void                                setLaunchMapViewer(bool b) {_launch_map_viewer = b;}
    const std::string                 & getLinelistIndividualsCacheFileName() const { return _linelist_individuals_cache_name; }
    const std::string                 & GetLocationNeighborsFileName() const {return gsLocationNeighborsFilename;}
    bool                                GetLogLikelihoodRatioIsTestStatistic() const;
    const std::string                 & GetMaxCirclePopulationFileName() const {return gsMaxCirclePopulationFileName;}
    double                              GetMaxSpatialSizeForType(SpatialSizeType eSpatialSizeType, bool bReported) const;
    double                              GetMaximumTemporalClusterSize() const {return gdMaxTemporalClusterSize;}
    unsigned int                        getMinimumTemporalClusterSize() const {return _minimum_temporal_cluster_size;}
    TemporalSizeType                    GetMaximumTemporalClusterSizeType() const {return geMaxTemporalClusterSizeType;}
    const std::string                 & getMetaLocationsFilename() const {return gsMetaLocationsFilename;}  
    MultipleDataSetPurposeType          GetMultipleDataSetPurposeType() const {return geMultipleSetPurposeType;}
    MultipleCoordinatesType             GetMultipleCoordinatesType() const {return geMultipleCoordinatesType;}
    double                              GetNonCompactnessPenaltyPower() const {return (geNonCompactnessPenaltyType == NOPENALTY ? 0.0 : (geNonCompactnessPenaltyType == MEDIUMPENALTY ? .5 : 1.0));}
    NonCompactnessPenaltyType           GetNonCompactnessPenaltyType() const {return geNonCompactnessPenaltyType;}
    unsigned int                        getNumFileSets() const {return gvCaseFilenames.size();}
    unsigned int                        getNumRequestedOliveiraSets() const {return _num_oliveira_sets;}
    unsigned int                        GetNumRequestedParallelProcesses() const {return giNumRequestedParallelProcesses;}
    unsigned int                        GetNumParallelProcessesToExecute() const;
    int                                 GetNumReadParameters() const {return giNumParameters;}
    unsigned int                        GetNumReplicationsRequested() const {return giReplications;}
    unsigned int                        getNumPowerEvalReplicaPowerStep() const {return _power_replica;}
    int                                 GetNumRequestedEllipses() const {return (int)gvEllipseShapes.size();}
    unsigned int                        GetNumIterativeScansRequested() const {return giNumIterativeRuns;}
    long                                GetNumTotalEllipses() const {return (gbUseLocationNeighborsFile || geSpatialWindowType == CIRCULAR ? 0 : glTotalNumEllipses);}
    const std::vector<std::string>    & getObservableRegions() const {return gvObservableRegions;}
    double                              getOliveiraPvalueCutoff() const {return _oliveira_pvalue_cutoff;}
    bool                                GetOutputAreaSpecificAscii() const  {return gbOutputAreaSpecificAscii;}
    bool                                GetOutputAreaSpecificDBase() const  {return gbOutputAreaSpecificDBase;}
    bool                                GetOutputAreaSpecificFiles() const;
    bool                                GetOutputClusterCaseAscii() const {return gbOutputClusterCaseAscii;}
    bool                                GetOutputClusterCaseDBase() const {return gbOutputClusterCaseDBase;}
    bool                                GetOutputClusterCaseFiles() const;
    bool                                GetOutputClusterLevelAscii() const {return gbOutputClusterLevelAscii;}
    bool                                GetOutputClusterLevelDBase() const {return gbOutputClusterLevelDBase;}
    bool                                GetOutputClusterLevelFiles() const;
    const std::string                 & GetOutputFileNameSetting() const {return gsOutputFileNameSetting; }
    const std::string                 & GetOutputFileName() const { return _results_filename; }
    bool                                getReportGiniIndexCoefficents() const {return _reportGiniIndexCoefficients;}
    bool                                getOutputCartesianGraph() const {return _output_cartesian_graph;}
    bool                                getOutputKMLFile() const { return _output_kml; }
    bool                                getOutputTemporalGraphFile() const {return _output_temporal_graph;}
    bool                                getOutputShapeFiles() const {return _output_shapefiles;}
    bool                                GetOutputRelativeRisksAscii() const {return gbOutputRelativeRisksAscii;}
    bool                                GetOutputRelativeRisksDBase() const {return gbOutputRelativeRisksDBase;}
    bool                                GetOutputRelativeRisksFiles() const;
    bool                                GetOutputSimLoglikeliRatiosAscii() const {return gbOutputSimLogLikeliRatiosAscii;}
    bool                                GetOutputSimLoglikeliRatiosDBase() const {return gbOutputSimLogLikeliRatiosDBase;}
    bool                                GetOutputSimLoglikeliRatiosFiles() const;
    bool                                GetOutputSimulationData() const {return gbOutputSimulationData;}
    bool                                getOutputPowerEvaluationSimulationData() const {return _report_power_simulation_data;}
    bool                                getPerformPowerEvaluation() const {return _performPowerEvaluation;}
    tract_t                             getPowerEvaluationCaseCount() const {return _powerEvaluationTotalCases;}
    CriticalValuesType                  getPowerEvaluationCriticalValueType() const {return _critical_value_type;}
    const std::string                 & getPowerEvaluationAltHypothesisFilename() const {return _power_alt_hypothesis_filename;}
    PowerEvaluationMethodType           getPowerEvaluationMethod() const {return _power_evaluation_method;}
    bool                                GetPermitsCentricExecution(bool excludePValue=false) const;
    bool                                GetPermitsPurelySpatialCluster(ProbabilityModelType eModelType) const;
    bool                                GetPermitsPurelySpatialCluster() const;
    bool                                GetPermitsPurelyTemporalCluster(ProbabilityModelType eModelType) const;
    bool                                GetPermitsPurelyTemporalCluster() const;
    const std::string                 & GetPopulationFileName(size_t iSetIndex=1) const;
    const std::vector<std::string>    & GetPopulationFileNames() const {return gvPopulationFilenames;}
    double                              getPowerEvaluationCriticalValue05() const {return _critical_value_05;}
    double                              getPowerEvaluationCriticalValue01() const {return _critical_value_01;}
    double                              getPowerEvaluationCriticalValue001() const {return _critical_value_001;}
    PowerEstimationType                 getPowerEstimationType() const {return _power_estimation_type;}
    DatePrecisionType                   GetPrecisionOfTimesType() const {return gePrecisionOfTimesType;}
    bool                                getPrintAsciiHeaders() const {return gbPrintAsciiHeaders;}
    ProbabilityModelType                GetProbabilityModelType() const {return geProbabilityModelType;}
    const std::string                 & GetProspectiveStartDate() const {return gsProspectiveStartDate;}
    PValueReportingType                 GetPValueReportingType() const {return gePValueReportingType;}
    long                                GetRandomizationSeed() const {return glRandomizationSeed;}
    bool                                getReportClusterRank() const {return gbReportRank;}
    bool                                GetReportCriticalValues() const {return gbReportCriticalValues;}
    bool                                GetReportGumbelPValue() const {return gbReportGumbelPValue;}
    bool                                GetRestrictingMaximumReportedGeoClusterSize() const {return gbRestrictReportedClusters;}
    bool                                GetRestrictMaxSpatialSizeForType(SpatialSizeType eSpatialSizeType, bool bReported) const;
    RiskType                            GetRiskType() const {return geRiskFunctionType;}
    double                              GetIterativeCutOffPValue() const {return gbIterativeCutOffPValue;}
    const std::string                 & GetSimulationDataOutputFilename() const {return gsSimulationDataOutputFilename;}
    const std::string                 & getPowerEvaluationSimulationDataOutputFilename() const {return _power_simulation_output_filename;}
    const std::string                 & GetSimulationDataSourceFilename() const {return gsSimulationDataSourceFileName;}
    const std::string                 & getPowerEvaluationSimulationDataSourceFilename() const {return _power_simulation_source_filename;}
    SimulationType                      GetSimulationType() const {return _simulationType;}
    SimulationType                      GetPowerEvaluationSimulationType() const {return _power_simulation_type;}
    const std::string                 & GetSourceFileName() const {return gsParametersSourceFileName;}
    SpatialAdjustmentType               GetSpatialAdjustmentType() const {return geSpatialAdjustmentType;}
    const std::vector<double>         & getSpatialWindowStops() const {return gvSpatialWindowStops;}
    const std::vector<double>         & getExecuteSpatialWindowStops() const;
    SpatialWindowType                   GetSpatialWindowType() const {return geSpatialWindowType;}
    const std::string                 & GetSpecialGridFileName() const {return gsSpecialGridFileName;}
    const std::string                 & GetStartRangeEndDate() const {return gsStartRangeEndDate;}
    const std::string                 & GetStartRangeStartDate() const {return gsStartRangeStartDate;}
    StudyPeriodDataCheckingType         GetStudyPeriodDataCheckingType() const {return geStudyPeriodDataCheckingType;}
    const std::string                 & GetStudyPeriodEndDate() const {return gsStudyPeriodEndDate;}
    const std::string                 & GetStudyPeriodStartDate() const {return gsStudyPeriodStartDate;}
    bool                                GetSuppressingWarnings() const {return gbSuppressWarnings;}
    bool                                GetTerminateSimulationsEarly() const;
    long                                GetTimeAggregationLength() const {return glTimeAggregationLength;}
    DatePrecisionType                   GetTimeAggregationUnitsType() const {return geTimeAggregationUnitsType;}
    double                              GetTimeTrendAdjustmentPercentage() const {return gdTimeTrendAdjustPercentage;}
    TimeTrendAdjustmentType             GetTimeTrendAdjustmentType() const {return geTimeTrendAdjustType;}
    double                              GetTimeTrendConvergence() const {return gdTimeTrendConverge;}
    TimeTrendType                       getTimeTrendType() const {return geTimeTrendType;}
    bool                                getIsWeightedNormal() const {return gbWeightedNormal;}
    bool                                getIsWeightedNormalCovariates() const {return gbWeightedNormalCovariates;}    
    bool                                getIsReportingIndexBasedCoefficents() const;
    bool                                getIsReportingGumbelPValue() const;
    bool                                getIsReportingStandardPValue() const;
    bool                                getIsReportingGumbelAsAddon() const;
    bool                                getCanReportGumbelInDefaultCombination() const;
    const std::string                 & GetTitleName() const {return gsTitleName;}
    void                                requestAllAdditionalOutputFiles();
    void                                SetAdjustForEarlierAnalyses(bool b) {gbAdjustForEarlierAnalyses = b;}
    void                                setAdjustForWeeklyTrends(bool b) {_adjustWeeklyTrends = b;}
    void                                SetAdjustmentsByRelativeRisksFilename(const char * sAdjustmentsByRelativeRisksFileName, bool bCorrectForRelativePath=false);  
    void                                SetAnalysisType(AnalysisType eAnalysisType);
    void                                SetAreaRateType(AreaRateType eAreaRateType);
    void                                SetAsDefaulted();
    void                                setCalculateOliveirasF(bool b) {_calculate_oliveira_f = b;}
    void                                SetEndRangeEndDate(const char * sEndRangeEndDate);
    void                                SetEndRangeStartDate(const char * sEndRangeStartDate);
    void                                SetEarlyTermThreshold(unsigned int i) {giEarlyTermThreshold = i;}
    void                                SetExecutionType(ExecutionType eExecutionType);
    void                                SetCaseFileName(const char * sCaseFileName, bool bCorrectForRelativePath=false, size_t iSetIndex=1);
    void                                setReportHierarchicalClusters(bool b) {_reportHierarchicalClusters = b;}
    void                                setReportGiniOptimizedClusters(bool b) {_reportGiniOptimizedClusters = b;}
    void                                SetControlFileName(const char * sControlFileName, bool bCorrectForRelativePath=false, size_t iSetIndex=1);
    void                                SetCoordinatesDataCheckingType(CoordinatesDataCheckingType eCoordinatesDataCheckingType);
    void                                SetCoordinatesFileName(const char * sCoordinatesFileName, bool bCorrectForRelativePath=false);
    void                                SetCoordinatesType(CoordinatesType eCoordinatesType);
    void                                SetCriteriaForReportingSecondaryClusters(CriteriaSecondaryClustersType eCriteriaSecondaryClustersType);
    void                                setGiniIndexPValueCutoff(double dPValue) {_giniIndexPValueCutoff = dPValue;}
    void                                setGiniIndexReportType(GiniIndexReportType e);
    void                                setIncludeLocationsKML(bool b) {_include_locations_kml = b;}
    void                                setCompressClusterKML(bool b) {_compress_kml_output = b;}
    void                                setLocationsThresholdKML(unsigned int i) {_locations_threshold_kml = i;}
    void                                SetIncludeClustersType(IncludeClustersType eIncludeClustersType);
    void                                SetIncludePurelySpatialClusters(bool b) {gbIncludePurelySpatialClusters = b;}
    void                                SetIncludePurelyTemporalClusters(bool b) {gbIncludePurelyTemporalClusters = b;}
    void                                SetIsLoggingHistory(bool b) {gbLogRunHistory = b;}
    void                                SetIsRandomlyGeneratingSeed(bool b) {gbRandomlyGenerateSeed = b;}
    void                                SetIterativeCutOffPValue(double dPValue) {gbIterativeCutOffPValue = dPValue;}
    void                                SetLocationNeighborsFileName(const char * sLocationNeighborsFileName, bool bCorrectForRelativePath=false);
    void                                SetMaxCirclePopulationFileName(const char * sMaxCirclePopulationFileName, bool bCorrectForRelativePath=false);
    void                                setMinimumTemporalClusterSize(unsigned int i) {_minimum_temporal_cluster_size = i;}
    void                                SetMaximumTemporalClusterSize(double dMaxTemporalClusterSize);
    void                                SetMaximumTemporalClusterSizeType(TemporalSizeType eTemporalSizeType);
    void                                SetMaxSpatialSizeForType(SpatialSizeType eSpatialSizeType, double d, bool bReported);
    void                                setMetaLocationsFilename(const char * sLocationNeighborsFileName, bool bCorrectForRelativePath=false);
    void                                SetMultipleDataSetPurposeType(MultipleDataSetPurposeType eType);
    void                                SetMultipleCoordinatesType(MultipleCoordinatesType eMultipleCoordinatesType);
    void                                SetNonCompactnessPenalty(NonCompactnessPenaltyType e);
    void                                setNumFileSets(size_t numSets);
    void                                setNumRequestedOliveiraSets(unsigned int i) {_num_oliveira_sets = i;}
    void                                SetNumParallelProcessesToExecute(unsigned int i) {giNumRequestedParallelProcesses = i;}
    void                                SetNumberMonteCarloReplications(unsigned int iReplications);
    void                                setNumPowerEvalReplicaPowerStep(unsigned int r) {_power_replica = r;}
    void                                SetNumIterativeScans(int iNumIterativeScans);
    void                                setOliveiraPvalueCutoff(double d) {_oliveira_pvalue_cutoff = d;}
    void                                setOutputCartesianGraph(bool b) { _output_cartesian_graph = b; }
    void                                setOutputKMLFile(bool b) {_output_kml = b;}
    void                                setOutputTemporalGraphFile(bool b) {_output_temporal_graph = b;}
    void                                setOutputShapeFiles(bool b) {_output_shapefiles = b;}
    void                                SetOutputAreaSpecificAscii(bool b) {gbOutputAreaSpecificAscii = b;}
    void                                SetOutputAreaSpecificDBase(bool b) {gbOutputAreaSpecificDBase = b;}
    void                                SetOutputClusterCaseAscii(bool b) {gbOutputClusterCaseAscii = b;}
    void                                SetOutputClusterCaseDBase(bool b) {gbOutputClusterCaseDBase = b;}
    void                                SetOutputClusterLevelAscii(bool b) {gbOutputClusterLevelAscii = b;}
    void                                SetOutputClusterLevelDBase(bool b) {gbOutputClusterLevelDBase = b;}
    void                                SetOutputFileNameSetting(const char * sOutPutFileName, bool bCorrectForRelativePath=false);
    void                                SetOutputRelativeRisksAscii(bool b) {gbOutputRelativeRisksAscii = b;}
    void                                SetOutputRelativeRisksDBase(bool b) {gbOutputRelativeRisksDBase = b;}
    void                                SetOutputSimLogLikeliRatiosAscii(bool b) {gbOutputSimLogLikeliRatiosAscii = b;}
    void                                SetOutputSimLogLikeliRatiosDBase(bool b) {gbOutputSimLogLikeliRatiosDBase = b;}
    void                                SetOutputSimulationData(bool b) {gbOutputSimulationData = b;}
    void                                setOutputPowerEvaluationSimulationData(bool b) {_report_power_simulation_data = b;}
    void                                setPerformPowerEvaluation(bool b) {_performPowerEvaluation = b;}
    void                                setPowerEvaluationCaseCount(tract_t t) {_powerEvaluationTotalCases = t;}
    void                                setPowerEvaluationAltHypothesisFilename(const char * f, bool bCorrectForRelativePath=false);
    void                                SetPopulationFileName(const char * sPopulationFileName, bool bCorrectForRelativePath=false, size_t iSetIndex=1);
    void                                SetPowerEvaluationCriticalValue05(double d) {_critical_value_05 = d;}
    void                                SetPowerEvaluationCriticalValue01(double d) {_critical_value_01 = d;}
    void                                SetPowerEvaluationCriticalValue001(double d) {_critical_value_001 = d;}
    void                                setPowerEvaluationCriticalValueType(CriticalValuesType e);
    void                                setPowerEstimationType(PowerEstimationType e) {_power_estimation_type = e;}
    void                                setPowerEvaluationMethod(PowerEvaluationMethodType e);
    void                                SetPrecisionOfTimesType(DatePrecisionType eDatePrecisionType);
    void                                setPrintAsciiHeaders(bool b) {gbPrintAsciiHeaders = b;}
    void                                SetProbabilityModelType(ProbabilityModelType eProbabilityModelType);
    void                                SetProspectiveStartDate(const char * sProspectiveStartDate);
    void                                SetPValueReportingType(PValueReportingType eType);
    void                                SetRandomizationSeed(long lSeed);
    void                                setReportClusterRank(bool b) {gbReportRank = b;}
    void                                SetReportCriticalValues(bool b) {gbReportCriticalValues = b;}
    void                                setReportGiniIndexCoefficents(bool b) {_reportGiniIndexCoefficients = b;}
    void                                SetReportGumbelPValue(bool b) {gbReportGumbelPValue = b;}    
    void                                SetRestrictMaxSpatialSizeForType(SpatialSizeType eSpatialSizeType, bool b, bool bReported);
    void                                SetRestrictReportedClusters(bool b) {gbRestrictReportedClusters = b;}
    void                                SetRiskType(RiskType eRiskType);
    void                                SetIterativeScanning(bool b) {gbIterativeRuns = b;}
    void                                SetSimulationDataOutputFileName(const char * sSourceFileName, bool bCorrectForRelativePath=false);
    void                                setPowerEvaluationSimulationDataOutputFilename(const char * sSourceFileName, bool bCorrectForRelativePath=false);
    void                                SetSimulationDataSourceFileName(const char * sSourceFileName, bool bCorrectForRelativePath=false);
    void                                setPowerEvaluationSimulationDataSourceFilename(const char * sSourceFileName, bool bCorrectForRelativePath=false);
    void                                SetSimulationType(SimulationType eSimulationType);
    void                                setLinelistIndividualsCacheFileName(const char * s, bool bCorrectForRelativePath = false);
    void                                setPowerEvaluationSimulationType(SimulationType eSimulationType);
    void                                SetSourceFileName(const char * sParametersSourceFileName);
    void                                SetSpatialAdjustmentType(SpatialAdjustmentType eSpatialAdjustmentType);
    void                                SetSpatialWindowType(SpatialWindowType eSpatialWindowType);
    void                                SetSpecialGridFileName(const char * sSpecialGridFileName, bool bCorrectForRelativePath=false, bool bSetUsingFlag=false);
    void                                SetStartRangeEndDate(const char * sStartRangeEndDate);
    void                                SetStartRangeStartDate(const char * sStartRangeStartDate);
    void                                SetStudyPeriodDataCheckingType(StudyPeriodDataCheckingType eStudyPeriodDataCheckingType);
    void                                SetStudyPeriodEndDate(const char * sStudyPeriodEndDate);
    void                                SetStudyPeriodStartDate(const char * sStudyPeriodStartDate);
    void                                SetSuppressingWarnings(bool b) {gbSuppressWarnings=b;}
    void                                SetTimeAggregationLength(long lTimeAggregationLength);
    void                                SetTimeAggregationUnitsType(DatePrecisionType eTimeAggregationUnits);
    void                                SetTimeTrendAdjustmentPercentage(double dPercentage);
    void                                SetTimeTrendAdjustmentType(TimeTrendAdjustmentType eTimeTrendAdjustmentType);
    void                                SetTimeTrendConvergence(double dTimeTrendConvergence);
    void                                setTimeTrendType(TimeTrendType eTimeTrendType);
    void                                SetUseAdjustmentForRelativeRisksFile(bool b) {gbUseAdjustmentsForRRFile = b;}
    void                                SetIsWeightedNormal(bool b) {gbWeightedNormal = b;}
    void                                SetIsWeightedNormalCovariates(bool b) {gbWeightedNormalCovariates = b;}    
    void                                UseMetaLocationsFile(bool b) {gbUseMetaLocationsFile = b;}
    void                                UseLocationNeighborsFile(bool b) {gbUseLocationNeighborsFile = b;}
    void                                SetUseSpecialGrid(bool b) {gbUseSpecialGridFile = b;}
    void                                SetVersion(const CreationVersion& vVersion);
    bool                                UseAdjustmentForRelativeRisksFile() const {return gbUseAdjustmentsForRRFile;}
    bool                                UseMaxCirclePopulationFile() const;
    bool                                UseCoordinatesFile() const;
    bool                                UsingMultipleCoordinatesMetaLocations() const;
    bool                                UseMetaLocationsFile() const;
    bool                                UseLocationNeighborsFile() const;
    bool                                UsePopulationFile() const {return gbUsePopulationFile;}
    void                                SetPopulationFile(bool b) {gbUsePopulationFile = b;}  /******/
    bool                                UseSpecialGrid() const {return gbUseSpecialGridFile;}
    void                                SetTitleName(const char * sTitleName);
};
//*****************************************************************************
#endif
