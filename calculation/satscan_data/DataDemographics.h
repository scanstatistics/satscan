//******************************************************************************
#ifndef __DataDemographics_H
#define __DataDemographics_H
//******************************************************************************
#include "SaTScan.h"
#include "UtilityFunctions.h"
#include "DataSource.h"
#include <boost/dynamic_bitset.hpp>
#include <boost/optional.hpp>
#include <numeric> 

/* Abtract base class for a demographic attribute. */
class DemographicAttribute {
    protected:
        std::string _label;

    public:
        DemographicAttribute(const std::string& s) : _label(s) {}
        virtual ~DemographicAttribute() {}

        virtual void add(const std::string& value, unsigned int times) = 0;
        const std::string & label() const { return _label; }
        virtual LinelistType gettype() const = 0;
        virtual void print() const = 0;
        virtual bool reportedInVisualizations() const { return true; }
};

/* Implements DemographicAttribute class to define a general demographic attribute. */
class GeneralDemographicAttribute : public DemographicAttribute {
    private:
        LinelistType _true_type;

    public:
        GeneralDemographicAttribute(const std::string& s) : DemographicAttribute(s), _true_type(GENERAL_DATA) {}
        GeneralDemographicAttribute(const std::string& s, LinelistType subtype) : DemographicAttribute(s), _true_type(subtype) {}
        virtual ~GeneralDemographicAttribute() {}

        virtual void add(const std::string& value, unsigned int times) {}
        virtual LinelistType gettype() const { return _true_type; }
        virtual void print() const {}
};

/* Implements DemographicAttribute class to define a categorical demographic attribute. */
class CategoricalDemographicAttribute: public DemographicAttribute {
    protected:
        // The maximum number of categories before attribute is excluded from map/earth visualizations.
        size_t MAXIMUM_CATEGORIES_FOR_VISUALIZATIONS;
        std::map<std::string, unsigned int> _category_counts;

    public:
        CategoricalDemographicAttribute(const std::string& s) : DemographicAttribute(s) { 
            MAXIMUM_CATEGORIES_FOR_VISUALIZATIONS = 12;
        }
        virtual ~CategoricalDemographicAttribute() {}

        virtual void add(const std::string& value, unsigned int times);
        virtual LinelistType gettype() const { return CATEGORICAL_DATA;  }
        virtual void print() const;
        virtual bool reportedInVisualizations() const { 
            // Returns whether this attribute would be reported in earth/map visualizations.
            // This property is just a way to prevent reporting attributes with many different values
            // (e.g. street address) in Earth/Maps were the user typically wouldn't find so many values useful.
            return _category_counts.size() <= MAXIMUM_CATEGORIES_FOR_VISUALIZATIONS;
        }
};

/* Implements DemographicAttribute class to define a continuous demographic attribute. */
class ContinuousDemographicAttribute : public DemographicAttribute {
    protected:
        std::vector<double> _values;

    public:
        ContinuousDemographicAttribute(const std::string& s) : DemographicAttribute(s) {}
        virtual ~ContinuousDemographicAttribute() {}

        virtual void add(const std::string& value, unsigned int times);
        virtual LinelistType gettype() const { return CONTINUOUS_DATA; }
        virtual void print() const;
};

/* A class to collect demographic attributes of a data set. */
class DemographicAttributeSet {
    public:
        typedef std::map<std::pair<LinelistType, std::string>, boost::shared_ptr<DemographicAttribute> > AttributesSet_t;
        AttributesSet_t _attributes_set;

    public:
        DemographicAttributeSet(const LineListFieldMapContainer_t& llmap);

        boost::shared_ptr<DemographicAttribute> get(LinelistTuple_t llt);
        const AttributesSet_t& getAttributes() const { return _attributes_set; }

        bool hasIndividual() const;
        bool hasIndividualGeographically() const;
        bool hasDescriptiveCoordinates() const;
};

class CSaTScanData;
class MostLikelyClustersContainer;
class SimulationVariables;
class DataSetHandler;
class RealDataSet;
class CCluster;
class CParameters;

/* A class which processes the line list attributes in the case files and facilitates generating csv, kml and maps output files. */
class DataDemographicsProcessor{
    public:
        typedef std::map<int, std::pair<unsigned int, unsigned int> > ClusterEventCounts_t;

    protected:
        const DataSetHandler& _handler;
        const CParameters& _parameters;
        const MostLikelyClustersContainer * _clusters;
        const SimulationVariables * _sim_vars;

        std::vector<DemographicAttributeSet> _demographics_by_dataset; // accumulates demographics by data
        std::map<int, std::pair<boost::dynamic_bitset<>, bool>> _cluster_locations; // cluster locations <mlc cluster index, locations of cluster bitset, reported-in-csv>
        std::map<int, std::deque<DemographicAttributeSet> > _cluster_demographics_by_dataset; // cluster demographics <mlc cluster index, demographics set>
        std::set<std::string> _existing_individuals; // individuals from previous analyses
        boost::shared_ptr<bloom_filter> _individuals_filter;
        std::set<std::string> _new_individuals; // new individuals
        std::string _temp_individuals_cache_filename;
        std::map<int, std::string> _cluster_location_files; // cluster temporary filenames <mlc cluster index, temporary filename>
        ClusterEventCounts_t _cluster_event_totals; // cluster event totals <mlc cluster index, <total new individuals, total events/individuals>>

        void appendLinelistData(int clusterIdx, std::vector<std::string>& data, boost::optional<int> first, unsigned int times, unsigned int analysis_count);
        bool processCaseFileLinelist(const RealDataSet& DataSet, unsigned int analysis_count);
        void removeTempClusterFiles();
        void writeClusterLineListFile(const DataSource::OrderedLineListField_t& llmap, unsigned int idxDataSet, unsigned int analysis_count);

    public:
        DataDemographicsProcessor(const DataSetHandler& handler);
        DataDemographicsProcessor(const DataSetHandler& handler, const MostLikelyClustersContainer& clusters, const SimulationVariables& sim_vars);
        ~DataDemographicsProcessor();

        void                             finalize();
        boost::dynamic_bitset<>        & getApplicableClusters(tract_t tid, Julian nDate, boost::dynamic_bitset<>& applicable_clusters, bool reportedOnly) const;
        const ClusterEventCounts_t     & getClusterEventTotals() const { return _cluster_event_totals; }
        const DemographicAttributeSet  & getDataSetDemographics(unsigned int idx) const { return _demographics_by_dataset.at(idx); }
        bool                             hasIndividualAttribute() const;
        bool                             hasIndividualGeographically() const;
        bool                             inCluster(tract_t tid, Julian nDate) const;
        bool                             isExistingIndividual(const std::string& s) const { return _existing_individuals.find(s) != _existing_individuals.end(); }
        bool                             isNewIndividual(const std::string& s) const { return _new_individuals.find(s) != _new_individuals.end(); }
        static                           bool isReportedInCsv(const CSaTScanData& Data, const CCluster& cluster, unsigned int iReportedCluster, const SimulationVariables& simVars);
        static                           bool meetsMainResultsCutoff(const CCluster& cluster, unsigned int iReportedCluster, const SimulationVariables& simVars);
        void                             print();
        void                             process(unsigned int analysis_count);
};

//*****************************************************************************
#endif
