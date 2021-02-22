//*****************************************************************************
#ifndef __MULTIPLEANALYSES_H_
#define __MULTIPLEANALYSES_H_
//*****************************************************************************
#include "SaTScan.h"
#include "BasePrint.h"
#include "UtilityFunctions.h"
#include "SSException.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/foreach.hpp>
#include <sstream>
#include <iostream>

namespace pt = boost::property_tree;
using Tree = boost::property_tree::ptree;

class MultipleAnalyses;
class PrintProxy;

/* Class to contain analysis definition with execution of multiple analyses. */
class AnalysisDefinition {
    friend class MultipleAnalyses;
    friend class PrintProxy;

    public:
        typedef std::pair<DatePrecisionType, unsigned int> Offset_t;

    protected:
        static const std::string _YEAR;
        static const std::string _MONTH;
        static const std::string _DAY;
        static const std::string _GENERIC;

        Tree _drilldown_tree;
        std::string description, parameters, studyperiod, studyperiodunit, lag, lagunit, 
                    lastexecution_date, lastexecutionstatus, lastexecutionwarningserrors, lastresultsfilename;

        static DatePrecisionType parseDateUnit(const std::string& val) {
            if (val == _YEAR) return YEAR;
            if (val == _MONTH) return MONTH;
            if (val == _GENERIC) return GENERIC;
            return DAY;
        }

        void setExecutionDateNow() {
            boost::posix_time::ptime timeLocal = boost::posix_time::second_clock::local_time();
            std::stringstream os;
            os.imbue(std::locale(os.getloc(), new boost::posix_time::time_facet("%Y-%m-%d %H:%M:%S")));
            os << timeLocal;
            lastexecution_date = os.str();
        }

    public:

        void signalSuccess() {
            lastexecutionstatus = "Success";
            setExecutionDateNow();
        }
        void signalFailure() {
            lastexecutionstatus = "Failed";
            setExecutionDateNow();
        }
        Offset_t getGetStudyPeriodOffset() const {
            if (studyperiod.empty() || studyperiodunit.empty()) return Offset_t(NONE, 0);
            unsigned int offset;
            if (!string_to_type(studyperiod.c_str(), offset))
                throw prg_error("Unable to convert study period offset to unsigned integer '%s'.", studyperiod.c_str());
            return Offset_t(parseDateUnit(studyperiodunit), offset);
        }

        Offset_t getLagOffset() const {
            if (lag.empty() || lagunit.empty()) return Offset_t(NONE, 0);
            unsigned int offset;
            if (!string_to_type(lag.c_str(), offset))
                throw prg_error("Unable to convert lag offset to unsigned integer '%s'.", lag.c_str());
            return Offset_t(parseDateUnit(lagunit), offset);
        }

        /* Returns a date as an offset from the passed initial date. */
        boost::gregorian::date getDate(boost::gregorian::date initial, Offset_t offset, bool isStudyPeriod) {
            if (offset.first == NONE) return initial;
            boost::posix_time::ptime timeLocal = boost::posix_time::second_clock::local_time();
            switch (offset.first) {
                case YEAR: return initial - boost::gregorian::years(offset.second);
                case MONTH: return initial - boost::gregorian::months(offset.second);
                case DAY:
                case GENERIC:
                    // special case with study period - since end date is included
                    return initial - boost::gregorian::days(offset.second - (isStudyPeriod ? 1 : 0));
            }
            return initial;
        }

};

/* Class which reads / writes xml file defining analyses to execute in batch, sequentially. */
class MultipleAnalyses {
    public:
        static const std::string _ANALYSES_TAG;
        static const std::string _ANALYSIS_TAG;
        static const std::string _DESCRIPTION_TAG;
        static const std::string _PARAMETERS_TAG;
        static const std::string _STUDYPERIOD_TAG;
        static const std::string _STUDYPERIOD_UNITS_TAG;
        static const std::string _LAG_TAG;
        static const std::string _LAG_UNITS_TAG;
        static const std::string _LAST_EXEC_DATE_TAG;
        static const std::string _LAST_EXEC_STATUS_TAG;
        static const std::string _LAG_EXEC_MSSG_TAG;
        static const std::string _LAG_EXEC_RESULTS_TAG;
        static const std::string _DRILLDOWN_TREE_TAG;
        static const std::string _DRILLDOWN_NODE_TAG;
        static const std::string _DRILLDOWN_RESULTS_TAG;
        static const std::string _DRILLDOWN_RESULTS_SIGNIFANT_TAG;

    private:
        typedef std::vector<AnalysisDefinition> AnalysesContainer_t;
        AnalysesContainer_t _analysis_defs;

        template <typename T, typename Indirect>
        void _read_tree(Indirect const& maybeTree, std::string sub, T& into, decltype(&*maybeTree) = nullptr) {
            if (maybeTree) _read_tree(*maybeTree, sub, into); else into = {};
        }

        template <typename T, typename Indirect>
        void _read_tree(Indirect const& maybeTree, T& into, decltype(&*maybeTree) = nullptr) {
            if (maybeTree) _read_tree(*maybeTree, into); else into = {};
        }

        template <typename T>
        void _read_tree(Tree const& tree, std::string sub, std::vector<T>& into) {
            for (auto& child : tree) {
                if (child.first == sub) {
                    into.emplace_back();
                    read_analysis(child.second, into.back());
                }
            }
        }

        /* Reads analysis definition from property tree. */
        void read_analysis(Tree const& tree, AnalysisDefinition& into) {
            into.description = tree.get(_DESCRIPTION_TAG, "");
            into.parameters = tree.get(_PARAMETERS_TAG, "");
            into.studyperiod = tree.get(_STUDYPERIOD_TAG, "");
            into.studyperiodunit = tree.get(_STUDYPERIOD_UNITS_TAG, "");
            into.lag = tree.get(_LAG_TAG, "");
            into.lagunit = tree.get(_LAG_UNITS_TAG, "");
            into.lastexecution_date = tree.get(_LAST_EXEC_DATE_TAG, "");
            into.lastexecutionstatus = tree.get(_LAST_EXEC_STATUS_TAG, "");
            into.lastexecutionwarningserrors = tree.get(_LAG_EXEC_MSSG_TAG, "");
            into.lastresultsfilename = tree.get(_LAG_EXEC_RESULTS_TAG, "");
        }

        /* Reads analysis property tree from file. */
        bool read_file(const std::string& filename) {
            if (!ValidateFileAccess(filename, false, false)) return false;
            _analysis_defs.clear();
            Tree tree;
            pt::read_xml(filename, tree);
            _read_tree(tree.get_child_optional(_ANALYSES_TAG), _ANALYSIS_TAG, _analysis_defs);
            return true;
        }

        /* Writes analysis property tree to file. */
        void write_file(const std::string& filename) {
            std::stringstream analysis_group;
            analysis_group << _ANALYSES_TAG << "." << _ANALYSIS_TAG;
            pt::ptree tree;
            for (AnalysesContainer_t::const_iterator itr = _analysis_defs.begin(); itr != _analysis_defs.end(); ++itr) {
                pt::ptree subtree;
                subtree.put(_DESCRIPTION_TAG, itr->description);
                subtree.put(_PARAMETERS_TAG, itr->parameters);
                subtree.put(_STUDYPERIOD_TAG, itr->studyperiod);
                subtree.put(_STUDYPERIOD_UNITS_TAG, itr->studyperiodunit);
                subtree.put(_LAG_TAG, itr->lag);
                subtree.put(_LAG_UNITS_TAG, itr->lagunit);
                subtree.put(_LAST_EXEC_DATE_TAG, itr->lastexecution_date);
                subtree.put(_LAST_EXEC_STATUS_TAG, itr->lastexecutionstatus);
                subtree.put(_LAG_EXEC_MSSG_TAG, itr->lastexecutionwarningserrors.empty() ? std::string("No Warnings or Errors.") : itr->lastexecutionwarningserrors);
                subtree.put(_LAG_EXEC_RESULTS_TAG, itr->lastresultsfilename);
                subtree.add_child(_DRILLDOWN_TREE_TAG, itr->_drilldown_tree);
                tree.add_child(analysis_group.str(), subtree);
            }
            write_xml(filename, tree, std::locale(), boost::property_tree::xml_parser::xml_writer_settings<boost::property_tree::ptree::key_type>('\t', 1));
        }

    public:
        MultipleAnalyses() {}

        void execute(BasePrint& print);
        static bool addResults(const std::string& resultsname, const std::string& parentname, unsigned int significant, pt::ptree &pt_parent, pt::ptree &pt_child, bool target = false);
};

/** Print direction class that is a proxy to actual object. */
class PrintProxy : public BasePrint {
protected:
    AnalysisDefinition & _analysisDef;
    BasePrint & _print;
    std::stringstream _warn_error_stream;
    std::map<std::string, pt::ptree> _result_subtree_map;
    pt::ptree _drilldown_tree;

    virtual void PrintError(const char * sMessage) {
        _print.PrintError(sMessage);
        _warn_error_stream << sMessage << std::endl;
    }
    virtual void PrintNotice(const char * sMessage) {
        _print.PrintNotice(sMessage);
        _warn_error_stream << sMessage << std::endl;
    }
    virtual void PrintStandard(const char * sMessage) { _print.PrintStandard(sMessage); }
    virtual void PrintWarning(const char * sMessage) {
        _print.PrintWarning(sMessage);
        _warn_error_stream << sMessage << std::endl;
    }
    virtual void ReportDrilldownResults(const char * drilldown_resultfile, const char * parent_resultfile, unsigned int significantClusters);

public:
    PrintProxy(AnalysisDefinition& analysisDef, BasePrint& print, bool bSuppressWarnings = true) : BasePrint(bSuppressWarnings), _print(print), _analysisDef(analysisDef){
        _warn_error_stream >> std::noskipws; // Make sure stream doesn't skip whitespace.
    }
    virtual ~PrintProxy() {}

    virtual bool GetIsCanceled() const { return _print.GetIsCanceled(); }
    std::stringstream & getWarnErrors() { return _warn_error_stream; }
};
//*****************************************************************************
#endif