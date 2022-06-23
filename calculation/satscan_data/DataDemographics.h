//******************************************************************************
#ifndef __DataDemographics_H
#define __DataDemographics_H
//******************************************************************************
#include "SaTScan.h"
#include "UtilityFunctions.h"
#include "DataSource.h"
#include <boost/dynamic_bitset.hpp>
#include <boost/optional.hpp>

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
};

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

class CategoricalDemographicAttribute: public DemographicAttribute {
    protected:
        std::map<std::string, unsigned int> _category_counts;

    public:
        CategoricalDemographicAttribute(const std::string& s): DemographicAttribute(s) {}
        virtual ~CategoricalDemographicAttribute() {}

        virtual void add(const std::string& value, unsigned int times);
        virtual LinelistType gettype() const { return CATEGORICAL_DATA;  }
        virtual void print() const;
};

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

class DemographicAttributeSet {
    protected:
        typedef std::map<std::string, boost::shared_ptr<DemographicAttribute> > AttributesSet_t;
        AttributesSet_t  _attributes_set;

    public:
        DemographicAttributeSet(const LineListFieldMapContainer_t& llmap);

        boost::shared_ptr<DemographicAttribute> get(LinelistTuple_t llt);
        const AttributesSet_t& getAttributes() const { return _attributes_set; }

        bool hasEventAttribute() const;
        bool hasEventCoordinatesAttributes() const;
};

class MostLikelyClustersContainer;
class SimulationVariables;
class DataSetHandler;
class RealDataSet;
class CCluster;
class CParameters;

class DataDemographicsProcessor{
    public:
        typedef std::map<int, std::pair<unsigned int, unsigned int> > ClusterEventCounts_t;

    protected:
        const DataSetHandler& _handler;
        const CParameters& _parameters;
        const MostLikelyClustersContainer * _clusters;
        const SimulationVariables * _sim_vars;

        // acculates demographics by data
        std::vector<DemographicAttributeSet> _demographics_by_dataset;
        std::vector<boost::tuple<bool, bool> > _events_by_dataset;
        // cluster locations <mlc cluster index, locations of cluster bitset>
        std::map<int, boost::dynamic_bitset<> > _cluster_locations;
        // cluster demographics <mlc cluster index, demograpghics set>
        std::map<int, std::deque<DemographicAttributeSet> > _cluster_demographics_by_dataset;
        // event ids from previous analyses
        std::set<std::string> _existing_event_ids;
        boost::shared_ptr<bloom_filter> _events_filter;
        // new event ids
        std::set<std::string> _new_event_ids;
        std::string _temp_events_cache_filename;
        // cluster temporary filenames <mlc cluster index, temporary filename>
        std::map<int, std::string> _cluster_location_files;
        // cluster new event counts <mlc cluster index, <# new events, total events>>
        ClusterEventCounts_t _cluster_new_events;

        void appendLinelistData(int clusterIdx, std::vector<std::string>& data, boost::optional<int> first, unsigned int times);
        void createHeadersFile(std::ofstream& linestream, const DataSource::OrderedLineListField_t& llmap);
        bool processCaseFileLinelist(const RealDataSet& DataSet);

    public:
        DataDemographicsProcessor(const DataSetHandler& handler);
        DataDemographicsProcessor(const DataSetHandler& handler, const MostLikelyClustersContainer& clusters, const SimulationVariables& sim_vars);
        ~DataDemographicsProcessor();

        void finalize();
        const ClusterEventCounts_t & getClusterNewEventsCounts() const { return _cluster_new_events; }
        const DemographicAttributeSet& getDataSetDemographics(unsigned int idx=0) const { return _demographics_by_dataset.at(idx); }
        boost::tuple<bool, bool> getEventStatus(unsigned int idx = 0) const { return _events_by_dataset.at(idx); }
        const std::set<std::string>& getNewEventIds() const { return _new_event_ids; }
        bool isExistingEvent(const std::string& s) const { return _existing_event_ids.find(s) != _existing_event_ids.end(); }
        bool isNewEvent(const std::string& s) const { return _new_event_ids.find(s) != _new_event_ids.end(); }
        void print();
        void process();
};

//*****************************************************************************
#endif
