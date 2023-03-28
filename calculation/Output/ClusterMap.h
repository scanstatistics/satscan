//******************************************************************************
#ifndef __ClusterMap_H
#define __ClusterMap_H
//******************************************************************************
#include<vector>
#include "SimulationVariables.h"
#include "MostLikelyClustersContainer.h"
#include "GisUtils.h"
#include <fstream>

class CSaTScanData;
class DataDemographicsProcessor;

class EventType {
public:
    typedef boost::tuple<std::string, std::string, std::string> CategoryTuple_t; // type, label, color

private:
    static unsigned int  _counter;
    std::string _class;
    std::string _type;
    std::string _name;
    std::vector<CategoryTuple_t> _categories;
    mutable VisualizationUtils _visual_utilities;

public:
    EventType(const std::string& name) {
        ++_counter;
        _name = name;
        printString(_class, "event_type_%u", _counter);
        std::stringstream text(name);
        _type = templateReplace(text, " ", "_").str();
        lowerString(_type);
    }

    const std::string& name() const { return _name; }
    const std::string& className() const { return _class; }
    const std::string& typeName() const { return _type; }

    std::string toJson(const std::string& resource_path);
    std::string getCategoryColor(unsigned int offset) const;
    const CategoryTuple_t& addCategory(const std::string& label) {
        for (auto itr = _categories.begin(); itr != _categories.end(); ++itr) {
            if (itr->get<1>() == label)
                return *itr;
        }
        std::string color(getCategoryColor(_categories.size()));
        std::stringstream replacer;
        replacer << label;
        std::string ctypename(templateReplace(replacer, " ", "_").str());
        _categories.push_back(CategoryTuple_t(lowerString(ctypename), label, color));
        return _categories.back();
    }
    const std::vector<CategoryTuple_t>& getCategories() const { return _categories; }
};

class ClusterMap {
    typedef std::vector<std::pair<double, double> > points_t;
    static const char * HTML_FILE_EXT;
    static const char * FILE_SUFFIX_EXT;
    static const char * TEMPLATE;

protected:
    const CSaTScanData                & _dataHub;
    std::stringstream                   _cluster_definitions;
    boost::dynamic_bitset<>             _cluster_locations;
    unsigned int                        _clusters_written;
    //std::stringstream                   _event_type_definitions;
    std::stringstream                   _event_definitions;
    std::stringstream                   _cluster_options_significant;
    std::stringstream                   _cluster_options_non_significant;
    std::vector<EventType>              _event_types;

    std::string                       & getClusterLegend(const CCluster& cluster, int iCluster, std::string& legend) const;

public:
    ClusterMap(const CSaTScanData& dataHub);
    ~ClusterMap() {}

    void add(const MostLikelyClustersContainer& clusters, const SimulationVariables& simVars, unsigned int iteration);
    void add(const DataDemographicsProcessor & demographics);
    void finalize();

    static FileName& getFilename(FileName& filename);
};
//******************************************************************************
#endif
