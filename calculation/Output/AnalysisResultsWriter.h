//******************************************************************************
#ifndef __AnalysisResultsWriter_H
#define __AnalysisResultsWriter_H
//******************************************************************************
#include "Parameters.h"
#include "ClusterSupplement.h"

class CSaTScanData;
class CCluster;
//class ClusterSupplementInfo;
//class SimulationVariables;

class AnalysisResultsWriter {
    public:
        typedef std::vector<std::pair<std::string, std::string>> PairsContainer_t;

        struct HtmlColumn {
            std::string _header;
            bool _cluster_level; // true if column is per-cluster (vs per-dataset)
            bool _sortable;
            HtmlColumn(const std::string& hdr, bool cl): _header(hdr), _cluster_level(cl), _sortable(true){}
            HtmlColumn(const std::string& hdr, bool cl, bool sortable) : _header(hdr), _cluster_level(cl), _sortable(sortable) {}
        };
        struct ReportedCluster {
            boost::dynamic_bitset<> _locations;
            int _start_interval;
            int _end_interval;

            ReportedCluster(int start, int end): _start_interval(start), _end_interval(end) {}
        };

    private:
        const CSaTScanData& _dataHub;
        const CParameters& _parameters;
        std::string _textOutputName;
        FILE * _text_out;
        std::string _htmlOutputName;
        std::ofstream _html_out;
        std::stringstream _html_cluster_table;
        std::vector<HtmlColumn> _html_columns;
        std::stringstream _html_sub_rows;
        std::stringstream _html_messages;
        std::vector<ReportedCluster> _cluster_locations;

        std::string& getTotalRunningTime(time_t start, time_t end, std::string& buffer) const;

    public:
        AnalysisResultsWriter(const CSaTScanData& dataHub);
        ~AnalysisResultsWriter();

        FILE * getTextFile() { return _text_out; }
        BasePrint& getPrintDirection() const;
        std::ofstream& getHtmlFile() { return _html_out; }
        std::stringstream& getHtmlMessages() { return _html_messages; }

        void writeClusterToHtmlTable(const CCluster& cluster, const ClusterSupplementInfo& supplementInfo, const SimulationVariables& simVars);
        void writeComputationCompletion(time_t startTime, time_t completedTime);
        void writeHeaderAndSummary(time_t startTime);
        void writeHtmlTableStart();
        void writeHtmlTableEnd();
        void writeMessage(const std::string& message, const std::string& divClass);
        void writeMessage(const std::vector<std::string>& message, const std::string& header, const std::string& divClass);
        void writeMessageHtmlFile(const std::vector<std::string>& message, const std::string& header, const std::string& divClass);
        void writeMessageTextFile(const std::vector<std::string>& message, const std::string& header);
        void writeMessageHtmlToggle(std::stringstream& message, const std::string& header, const std::string& divClass);
        void writeParameters(bool isDrilldown);
        static void writeCardContainer(
            std::ofstream& writef, const std::stringstream& markup,
			const std::string& clazz, const std::string& faclass, const std::string& header, bool closed = true
        );
};
//******************************************************************************
#endif