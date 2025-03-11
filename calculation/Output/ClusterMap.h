//******************************************************************************
#ifndef __ClusterMap_H
#define __ClusterMap_H
//******************************************************************************
#include<vector>
#include "SimulationVariables.h"
#include "MostLikelyClustersContainer.h"
#include <fstream>

class CSaTScanData;
class DataDemographicsProcessor;

class EventType {
    public:
        typedef boost::tuple<std::string, std::string, unsigned int> CategoryTuple_t; // key_name, label, count
        typedef std::vector<CategoryTuple_t> CategoriesContainer_t;

    private:
        static unsigned int _counter;
        std::string _class;
        std::string _type;
        std::string _name;
        std::vector<CategoryTuple_t> _categories;

        std::string                           formatTypeString(const std::string& str, unsigned int offset);

    public:
        EventType(const std::string& name);

        const CategoryTuple_t               & addCategory(const std::string& category_label);
        const std::string                   & className() const { return _class; }
        const CategoriesContainer_t         & getCategories() const { return _categories; }
        const std::string                   & name() const { return _name; }
        const CategoriesContainer_t         & sortCategories();
        std::string                         & toJson(std::string& json_str);
        const std::string                   & typeName() const { return _type; }
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
    std::stringstream                   _event_definitions;
    std::stringstream                   _cluster_options;
    std::vector<EventType>              _event_types;
    Julian                              _recent_startdate;
    double                              _mlc_ode;

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
