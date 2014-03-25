//******************************************************************************
#ifndef __ChartGenerator_H
#define __ChartGenerator_H
//******************************************************************************
#include "FileName.h"
#include "MostLikelyClustersContainer.h"

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
        static const char * TEMPLATE_CHARTHEADER_PT_SERIES;
        static const char * TEMPLATE_CHARTHEADER_ST_SERIES;
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
        std::pair<int, int> _getSeriesStreams(const CCluster& cluster,
                                              const intervalGroups& groups,
                                              size_t dataSetIdx,
                                              std::stringstream& categories,
                                              std::stringstream& cluster_data,
                                              std::stringstream& expected_data,
                                              std::stringstream& observed_data,
                                              std::stringstream& cluster_observed_data,
                                              std::stringstream& cluster_expected_data) const;

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
