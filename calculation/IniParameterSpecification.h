//***************************************************************************
#ifndef __IniParameterSpecification_H
#define __IniParameterSpecification_H
//***************************************************************************
#include "Parameters.h"
#include "Ini.h"

/** Defines interface for retrieving ini section and key name for parameters. */
class IniParameterSpecification {
  public:
    class SectionInfo {
        public:
            const char *  _label;
            unsigned int  _ordinal;

        public:
            SectionInfo():_label(0), _ordinal(0) {}
            SectionInfo(const char * label, unsigned int ordinal):_label(label), _ordinal(ordinal) {}
    };

    class ParamInfo {
        public:
            ParameterType _type;
            const char *  _label;
            unsigned int  _ordinal;
            const SectionInfo * _section;

        public:
            ParamInfo() : _label(0), _section(0) {}
            ParamInfo(const ParamInfo& other):
              _type(other._type), _label(other._label), _ordinal(other._ordinal), _section(other._section) {}
            ParamInfo(ParameterType type, const char * label, unsigned int ordinal, const SectionInfo& section):
                _type(type), _label(label), _ordinal(ordinal), _section(&section) {}

            bool operator<(const ParamInfo& pinfo) const {
                if (_section->_ordinal == pinfo._section->_ordinal) {
                    return this->_ordinal < pinfo._ordinal;
                } else {
                    return _section->_ordinal < pinfo._section->_ordinal;
                }
            }
    };

    typedef std::map<ParameterType, ParamInfo> ParameterInfoMap_t;
    typedef std::map<ParameterType, ParamInfo > MultipleParameterInfoMap_t;

    typedef std::vector<ParamInfo> ParameterInfoCollection_t;

  public:
    SectionInfo                 _input_section;
    SectionInfo                 _input_files_section;
    SectionInfo                 _analysis_section;
    SectionInfo                 _time_parameters_section;
    SectionInfo                 _scanning_window_section;
    SectionInfo                 _output_section;
    SectionInfo                 _output_files_section;
    SectionInfo                 _polygons_section;
    SectionInfo                 _multiple_data_section;
    SectionInfo                 _data_checking_section;
    SectionInfo                 _non_eucledian_section;
    SectionInfo                 _spatial_neighbors_section;
    SectionInfo                 _locations_network_section;
    SectionInfo                 _linelist_section;
    SectionInfo                 _spatial_window_section;
    SectionInfo                 _temporal_window_section;
    SectionInfo                 _cluster_restrictions_section;
    SectionInfo                 _space_time_adjustments_section;
    SectionInfo                 _inference_section;
    SectionInfo                 _drilldown_section;
    SectionInfo                 _border_analysis_section;
    SectionInfo                 _miscellaneous_analysis_section;
    SectionInfo                 _power_evaluation_section;
    SectionInfo                 _clusters_reported_section;
    SectionInfo                 _spatial_output_section;
    SectionInfo                 _temporal_output_section;
    SectionInfo                 _other_output_section;
    SectionInfo                 _additional_output_section;
    SectionInfo                 _elliptic_section;
    SectionInfo                 _isotonic_scan_section;
    SectionInfo                 _sequential_section;
    SectionInfo                 _power_simulations_section;
    SectionInfo                 _batch_features_section;
    SectionInfo                 _run_options_section;
    SectionInfo                 _advanced_section;
    SectionInfo                 _system_section;
    SectionInfo                 _email_alerts_section;
    SectionInfo                 _notifications_section;

    SectionInfo                 _not_used_section;

    static const char         * NotUsed;
    static const char         * Input;
    static const char         * MultipleDataSets;
    static const char         * DataChecking;
    static const char         * Drilldown;
    static const char         * NeighborsFile;
    static const char         * SpatialNeighbors;
    static const char         * LocationsNetwork;
    static const char         * LineList;
    static const char         * Analysis;
    static const char         * SpatialWindow;
    static const char         * TemporalWindow;
    static const char         * ClusterRestrictions;
    static const char         * Polygons;
    static const char         * SpaceAndTimeAdjustments;
    static const char         * Inference;
    static const char         * BorderAnalysis;
    static const char         * MiscellaneousAnalysis;
    static const char         * Output;
    static const char         * ClustersReported;
    static const char         * AdditionalOutput;
    static const char         * TemporalOutput;
    static const char         * SpatialOutput;
    static const char         * OtherOutput;
    static const char         * EmailAlerts;
    static const char         * Notifications;

    static const char         * EllipticScan;
    static const char         * SequentialScan;
    static const char         * IsotonicScan;
    static const char         * PowerSimulations;
    static const char         * PowerEvaluations;
    static const char         * RunOptions;
    static const char         * BatchModeFeatures;
    static const char         * System;

    static const char         * InputFiles;
    static const char         * ScanningWindow;
    static const char         * TimeParameters;
    static const char         * OutputFiles;
    static const char         * AdvancedFeatures;

    static const char         * DataSetName;

    static const char         * SourceType;
    static const char         * SourceDelimiter;
    static const char         * SourceGrouper;
    static const char         * SourceSkip;
    static const char         * SourceFirstRowHeader;
    static const char         * SourceFieldMap;
    static const char         * SourceLinelistFieldMap;
    static const char         * SourceFieldMapShapeX;
    static const char         * SourceFieldMapShapeY;
    static const char         * SourceFieldMapOneCount;
    static const char         * SourceFieldMapGeneratedId;
    static const char         * SourceFieldMapUnspecifiedPopulationDate;

    void                        setup(CParameters::CreationVersion version);

  protected:
    ParameterInfoMap_t          _parameter_info;
    MultipleParameterInfoMap_t  _multiple_parameter_info;

    void                        Build_3_0_5_ParameterList();
    void                        Build_4_0_x_ParameterList();
    void                        Build_5_0_x_ParameterList();
    void                        Build_5_1_x_ParameterList();
    void                        Build_6_0_x_ParameterList();
    void                        Build_6_1_x_ParameterList();
    void                        Build_7_0_x_ParameterList();
    void                        Build_8_0_x_ParameterList();
    void                        Build_8_2_x_ParameterList();
    void                        Build_9_0_x_ParameterList();
    void                        Build_9_2_x_ParameterList();
    void                        Build_9_3_x_ParameterList();
    void                        Build_9_4_x_ParameterList();
    void                        Build_9_5_x_ParameterList();
    void                        Build_9_6_x_ParameterList();
    void                        Build_9_7_x_ParameterList();
    void                        Build_10_0_x_ParameterList();
    void                        Build_10_1_x_ParameterList();
    void                        Build_10_2_x_ParameterList();

   public:
     IniParameterSpecification();
     IniParameterSpecification(const IniFile& SourceFile, CParameters& Parameters);
     IniParameterSpecification(CParameters::CreationVersion version, CParameters& Parameters);
     IniParameterSpecification(const IniFile& SourceFile, CParameters::CreationVersion version, CParameters& Parameters);
     virtual ~IniParameterSpecification() {}

    static CParameters::CreationVersion getIniVersion(const IniFile& SourceFile);

    bool                GetParameterIniInfo(ParameterType eParameterType, const char ** sSectionName, const char ** sKey) const;
    bool                GetMultipleParameterIniInfo(ParameterType eParameterType, const char ** sSectionName, const char ** sKey) const;

    ParameterInfoCollection_t & getParameterInfoCollection(ParameterInfoCollection_t& collection) const;
};
//***************************************************************************
#endif
