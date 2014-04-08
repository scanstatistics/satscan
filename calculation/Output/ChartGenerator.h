//******************************************************************************
#ifndef __ChartGenerator_H
#define __ChartGenerator_H
//******************************************************************************
#include "FileName.h"
#include "MostLikelyClustersContainer.h"
#include "UtilityFunctions.h"
#include <fstream>

/* abstract base class for chart generation. */
class AbstractChartGenerator {
    protected:
        static const char * HTML_FILE_EXT;
        static const char * TEMPLATE_BODY;
        static std::stringstream & templateReplace(std::stringstream& templateText, const std::string& replaceStub, const std::string& replaceWith);

    public:
        AbstractChartGenerator() {}
        virtual void generateChart() const = 0;
};

class ChartSeries {
    protected:
        std::string _id;
        unsigned int _zindex;
        std::string _type;
        std::string _name;
        std::string _color;
        std::string _symbol;
        unsigned int _y_axis;
        std::stringstream _data_stream;

    public:
        ChartSeries(const std::string& id, unsigned int zindex, const std::string& type, 
                    const std::string& name, const std::string& color, const std::string& symbol, unsigned int y_axis) 
                    : _id(id), _zindex(zindex), _type(type), _name(name), _color(color), _symbol(symbol), _y_axis(y_axis) {}

        std::stringstream & datastream() {return _data_stream;}
        std::string & toString(std::string& r) const {
            return printString(r, 
                               "{ id: '%s', zIndex: %u, type: '%s', name: '%s', color: '#%s', yAxis: %u, marker: { enabled: true, symbol: '%s', radius: 0 }, data: [%s] }", 
                               _id.c_str(), _zindex, _type.c_str(), _name.c_str(), _color.c_str(), _y_axis, _symbol.c_str(), _data_stream.str().c_str());
        }
};

class CSaTScanData;
class CCluster;
class SimulationVariables;

/* generator for temporal chart */
class TemporalChartGenerator : public AbstractChartGenerator {
    public:
        static const char * FILE_SUFFIX_EXT;
        static const int    MAX_INTERVALS;

    protected:
        static const char * BASE_TEMPLATE;
        static const char * TEMPLATE_CHARTHEADER;
        static const char * TEMPLATE_CHARTSECTION;
        const CSaTScanData & _dataHub;
        const MostLikelyClustersContainer & _clusters;
        const SimulationVariables & _simVars;

        class intervalGroups {
            public:
                typedef std::vector<std::pair<int,int> > intervals_t;
            private:
                intervals_t _interval_grps;
            public:
                void addGroup(int start, int end) {_interval_grps.push_back(std::make_pair(start,end));}
                const intervals_t& getGroups() const {return _interval_grps;}
        };
        intervalGroups getIntervalGroups(const CCluster& cluster) const;
        std::pair<int, int> getSeriesStreams(const CCluster& cluster,
                                              const intervalGroups& groups,
                                              size_t dataSetIdx,
                                              std::stringstream& categories,
                                              ChartSeries& clusterSeries,
                                              ChartSeries& observedSeries,
                                              ChartSeries& expectedSeries,
                                              ChartSeries * cluster_observedSeries,
                                              ChartSeries * cluster_expectedSeries,
                                              ChartSeries * odeSeries,
                                              ChartSeries * cluster_odeSeries) const;

    public:
        TemporalChartGenerator(const CSaTScanData& dataHub, const MostLikelyClustersContainer & clusters, const SimulationVariables& simVars);

        virtual void generateChart() const;
        static FileName& getFilename(FileName& filename);
};

/* generator for Gini chart */
class GiniChartGenerator : public AbstractChartGenerator {
    public:
        static const char * FILE_SUFFIX_EXT;

    protected:
        static const char * BASE_TEMPLATE;
        static const char * TEMPLATE_CHARTHEADER;
        static const char * TEMPLATE_CHARTSECTION;
        const MLC_Collections_t & _mlc;
        const CSaTScanData & _dataHub;
        const SimulationVariables & _simVars;

    public:
        GiniChartGenerator(const MLC_Collections_t& mlc, const CSaTScanData& dataHub, const SimulationVariables& simVars) : _mlc(mlc), _dataHub(dataHub), _simVars(simVars) {}

        virtual void generateChart() const;
        static FileName& getFilename(FileName& filename);
};
//******************************************************************************
#endif
