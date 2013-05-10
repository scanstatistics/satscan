//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "ChartGenerator.h"
#include "SaTScanData.h"
#include "UtilityFunctions.h"
#include "cluster.h"
#include "SimulationVariables.h"
#include "Toolkit.h"
#include <fstream>
#include <boost/regex.hpp>

/** ------------------- AbstractChartGenerator --------------------------------*/
const char * AbstractChartGenerator::HTML_FILE_EXT = ".html";

const char * AbstractChartGenerator::BASE_TEMPLATE = " \
<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\" \"http://www.w3.org/TR/html4/strict.dtd\"> \n \
<html lang=\"en\"> \n \
    <head> \n \
        <title>--title--</title> \n \
        <meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\"> \n \
        <style type=\"text/css\"> body {font: 100% Arial,Helvetica;background: #9ea090;} button {cursor: pointer;}button {-moz-border-radius:5px;-webkit-border-radius:5px;-moz-box-shadow:0px 0px 2px rgba(0,0,0,0.4);-webkit-box-shadow:0px 0px 2px rgba(0,0,0,0.4);color:rgba(0,0,0,0.9);text-shadow:1px 1px 0px rgba(255,255,255,0.8);border:1px solid rgba(0,0,0,0.5);background:-webkit-gradient(linear,0% 0%,0% 100%,from(rgba(255,255,255,1)),to(rgba(185,185,185,1)));background:-moz-linear-gradient(top,rgba(255,255,255,1),rgba(185,185,185,1));padding:5px 5px 5px 5px;}button:hover {background:rgba(240,240,240,1);}button:active, button:focus {background:-webkit-gradient(linear,0% 100%,0% 0%,from(rgba(255,255,255,1)),to(rgba(185,185,185,1)));background:-moz-linear-gradient(bottom,rgba(255,255,255,1),rgba(185,185,185,1));}button:disabled {color:rgba(0,0,0,0.4);text-shadow:1px 1px 0px rgba(255,255,255,0.5);background:rgba(220,220,220,1);}</style> \n \
        <script type=\"text/javascript\" src=\"--resource-path--javascript/jquery/jquery-1.9.0/jquery-1.9.0.js\"></script> \n \
        <script type=\"text/javascript\" src=\"--resource-path--javascript/highcharts/highcharts-2.3.5/js/highcharts.js\"></script> \n \
        <script type=\"text/javascript\" src=\"--resource-path--javascript/highcharts/highcharts-2.3.5/js/modules/exporting.js\"></script> \
        --header-- \
    </head> \n \
    <body> \n \
        <!--[if lt IE 9]> \n \
        <div id=\"ie\" style=\"z-index:255;border-top:5px solid #fff;border-bottom:5px solid #fff;background-color:#c00; color:#fff;\"><div class=\"iewrap\" style=\"border-top:5px solid #e57373;border-bottom:5px solid #e57373;\"><div class=\"iehead\" style=\"margin: 14px 14px;font-size: 20px;\">Notice to Internet Explorer users!</div><div class=\"iebody\" style=\"font-size: 14px;line-height: 14px;margin: 14px 28px;\">It appears that you are using Internet Explorer, <strong>this page may not display correctly with versions 8 or earlier of this browser</strong>.<br /><br /> \n \
            <i>This page is known to display correctly with the following browsers: Safari 4+, Firefox 3+, Opera 10+ and Google Chrome 5+.</i> \n \
        </div></div></div> \n \
        <![endif]--> \
        --body-- \
    </body> \n \
</html> \n";

/** Replaces 'replaceStub' text in passed stringstream 'templateText' with text of 'replaceWith'. */
std::stringstream & AbstractChartGenerator::templateReplace(std::stringstream& templateText, const std::string& replaceStub, const std::string& replaceWith) {
    boost::regex to_be_replaced(replaceStub);
    std::string changed(boost::regex_replace(templateText.str(), to_be_replaced, replaceWith));
    templateText.str(std::string());
    templateText << changed;
    return templateText;
}

/** ------------------- TemporalChartGenerator --------------------------------*/

const char * TemporalChartGenerator::FILE_SUFFIX_EXT = "_temporal";
const int TemporalChartGenerator::MAX_INTERVALS = 4000;

const char * TemporalChartGenerator::TEMPLATE_HEADER = "\n \
        <script type=\"text/javascript\"> \n \
            $(document).ready(function () { \n \
                var chart = new Highcharts.Chart({ \n \
                    chart: { renderTo: 'container', type: 'line', zoomType:'x', resetZoomButton: {relativeTo: 'chart', position: {x: -80, y: 10}, theme: {fill: 'white',stroke: 'silver',r: 0,states: {hover: {fill: '#41739D', style: { color: 'white' } } } } }, marginBottom: --margin-bottom--, borderColor: '#888888', plotBackgroundColor: '#e6e7e3', borderRadius: 10, borderWidth: 4, marginRight: 20 }, \n \
                    title: { text: 'Detected Cluster w/ Observed and Expected', align: 'center' }, \n \
                    tooltip: { crosshairs: true, shared: true, formatter: function(){var is_cluster = false;var has_observed = false;$.each(this.points, function(i, point) {if (point.series.options.id == 'cluster') {is_cluster = true;}if (point.series.options.id == 'obs') {has_observed = true;}});var s = '<b>'+ this.x +'</b>'; if (is_cluster) {s+= '<br/><b>Cluster Point</b>';}$.each(this.points,function(i, point){if (point.series.options.id == 'cluster'){if (!has_observed) {s += '<br/>Observed: '+ point.y;}} else {s += '<br/>'+ point.series.name +': '+ point.y;}});return s;}, }, \n \
                    legend: { backgroundColor: '#F5F5F5' }, \n \
                    xAxis: { categories: [--categories--], tickmarkPlacement: 'on', labels: { step: --step--, rotation: -45, align: 'right' } }, \n \
                    yAxis: { title: { enabled: true, text: 'Number of Cases', style: { fontWeight: 'normal' } }, min: 0 }, \n \
                    navigation: { buttonOptions: { align: 'right' } }, \n \
                    series: [ { id: 'cluster', zIndex: 3, type: 'line', name: 'Detected Cluster', color: '#AA4643', marker: { enabled: true, symbol: 'circle', radius: 0 }, data: [--cluster_data--]}, \n \
                              { id: 'obs', zIndex: 2, type: 'line', name: 'Observed', color: '#4572A7', marker: { enabled: true, symbol: 'square', radius: 0 }, data: [--observed_data--] }, \n \
                              { id: 'exp', zIndex: 1, type: 'line', name: 'Expected', color: '#89A54E', marker: { enabled: true, symbol: 'triangle', radius: 0 }, data: [--expected_data--]} ] \n \
                }); \n \
                $('#toggle_type').click(function(){var newType=chart.series[0].type=='line'?'column':'line';for(var i=0;i<chart.series.length;i++){serie = chart.series[0];serie.chart.addSeries({type:newType,name:serie.name,zIndex: serie.options.zIndex,color:serie.color,marker:{enabled:true,radius:0},data:serie.options.data},false);serie.remove();}$(\"#toggle_type\").html((chart.series[0].type=='line'?'Switch to Column Chart':'Switch to Line Chart'));return false;}); \n \
            }); \n \
        </script> \n";

const char * TemporalChartGenerator::TEMPLATE_BODY = "\n \
        <div id=\"container\" style=\"margin-top:20px;\"></div> \n \
        <div><button id=\"toggle_type\" style=\"margin-top:10px;\">Switch to Column Chart</button> \n \
        <div style=\"font-style:italic;float:right;font-size:small;\">To zoom, select and drag section of chart.</div></div> \n";

/** constructor */
TemporalChartGenerator::TemporalChartGenerator(const CSaTScanData& dataHub, const CCluster & cluster) :_dataHub(dataHub), _cluster(cluster) {
    try {
        if (_cluster.GetClusterType() != PURELYTEMPORALCLUSTER) {
            throw prg_error("TemporalChartGenerator not implemented for clusters of type '%d'.", "constructor()", _cluster.GetClusterType());
        }
    } catch (prg_exception& x) {
        x.addTrace("constructor()","TemporalChartGenerator");
        throw;
    }
}

/* TODO: Once we start creating Gini graph, we might break TEMPLATE into parts. A good portion of the header
         will certainly be shared between the 2 files.
*/

/* TODO: It might be better to use a true template/generator library. Some possibilities are reference here: 
          http://stackoverflow.com/questions/355650/c-html-template-framework-templatizing-library-html-generator-library

         We'll see first if this simple implementation is adequate.
*/

/* TODO: I'm not certain whether the javascript libraries should be -- locally or from www.satscan.org.
         locally: 
            pros: user does not need internet access
            cons: how do we know where the libraries are installed on user machine?
         satscan.org:
            pros: easy to maintain
            cons: requires user to have internet access
         library website:
            pros: bug fixes automatically
            cons: re-organization of site could break links, site goes away

        I'm leaning towards hosting from satscan.org. In html, test for each library and display message if jQuery or highcharts does not exist.
*/

/** Creates HighCharts graph for purely temporal cluster. */
void TemporalChartGenerator::generateChart() const {
    std::string buffer;
    FileName fileName;

    try {
        fileName.setFullPath(_dataHub.GetParameters().GetOutputFileName().c_str());
        getFilename(fileName);

        std::ofstream HTMLout;
        //open output file
        HTMLout.open(fileName.getFullPath(buffer).c_str());
        if (!HTMLout) throw resolvable_error("Error: Could not open file '%s'.\n", fileName.getFullPath(buffer).c_str());
        std::stringstream html, categories, observed_data, expected_data, cluster_data;

        // read template into stringstream
        html << BASE_TEMPLATE << std::endl;
        // replace specialized header
        templateReplace(html, "--header--", TEMPLATE_HEADER);
        // site resource link path
        templateReplace(html, "--resource-path--", AppToolkit::getToolkit().GetWebSite());
        // replace page title
        templateReplace(html, "--title--", "Cluster Graph");
        // replace specialized body
        templateReplace(html, "--body--", TEMPLATE_BODY);

        // set margin bottom according to time precision
        int margin_bottom=130;
        switch (_dataHub.GetParameters().GetPrecisionOfTimesType()) {
        case YEAR : margin_bottom = 90; break;
        case MONTH : margin_bottom = 110; break;
        case DAY:
        case GENERIC:
        default: margin_bottom=130;
        }
        templateReplace(html, "--margin-bottom--", printString(buffer, "%d", margin_bottom));        

        // TODO: What about multiple data sets?
        const DataSetHandler& handler = _dataHub.GetDataSetHandler();
        count_t * pcases = handler.GetDataSet(0).getCaseData_PT();
        measure_t * pmeasure = handler.GetDataSet(0).getMeasureData_PT();
        double adjustment = _dataHub.GetMeasureAdjustment(0);
        int intervals = _dataHub.GetNumTimeIntervals();

        intervalGroups groups;
        if (intervals <= MAX_INTERVALS) {
            // number of groups equals the number of intervals
            for (int i=0; i < intervals; ++i) {
                groups.addGroup(i, i+1);
            }
        } else {
            int cluster_length = _cluster.m_nLastInterval - _cluster.m_nFirstInterval;
            if (cluster_length <= MAX_INTERVALS) {
                int extra_intervals = ((MAX_INTERVALS - cluster_length)/2) + 5;
                // we can show entire cluster intervals plus a few before and after
                for (int i=std::max(0, _cluster.m_nFirstInterval - extra_intervals); i < std::min(intervals, _cluster.m_nLastInterval + extra_intervals); ++i) {
                    groups.addGroup(i, i+1);
                }
            } else {
                // we can show entire cluster intervals but compressed -- plus a few before and after
                int compressed_interval_length = static_cast<int>(ceil(static_cast<double>(_cluster.m_nLastInterval - _cluster.m_nFirstInterval)/static_cast<double>(MAX_INTERVALS)));

                // Note: This rough calculation of a compressed interval means that the clusters last interval might not fall cleanly onto a interval boundary.
                int extra_intervals = compressed_interval_length * 5;
                for (int i=std::max(0, _cluster.m_nFirstInterval - extra_intervals); i < std::min(intervals, _cluster.m_nLastInterval + extra_intervals); i=i+compressed_interval_length) {
                    groups.addGroup(i, i + compressed_interval_length);
                }
            }
        }

        // increase x-axis 'step' if there are many intervals, so that labels are not crowded
        //  -- empirically, 50 ticks seems like a good upper limit
        templateReplace(html, "--step--", printString(buffer, "%u", static_cast<int>(std::ceil(static_cast<double>(groups.getGroups().size())/50.0))));

        // define categories and replace in template
        const std::vector<Julian>& startDates = _dataHub.GetTimeIntervalStartTimes();
        DatePrecisionType precision = _dataHub.GetParameters().GetPrecisionOfTimesType();
        // iterate through groups, creating totals for each interval grouping
        for (intervalGroups::intervals_t::const_iterator itrGrp=groups.getGroups().begin(); itrGrp != groups.getGroups().end(); ++itrGrp) {
            // define date categories
            categories << (itrGrp==groups.getGroups().begin() ? "'" : ",'") << JulianToString(buffer, startDates[itrGrp->first], precision).c_str() << "'";
            // calcuate the expected and observed for this interval
            measure_t expected=0;
            count_t observed=0;
            for (int i=itrGrp->first; i < itrGrp->second; ++i) {
                expected += (adjustment * (i == intervals - 1 ? pmeasure[i] : pmeasure[i] - pmeasure[i+1]));
                observed += (i == intervals - 1 ? pcases[i] : pcases[i] - pcases[i+1]);
            }
            // put totals to data streams
            expected_data << (itrGrp==groups.getGroups().begin() ? "" : ",") << expected;
            observed_data <<  (itrGrp==groups.getGroups().begin() ? "" : ",");
            if (_cluster.m_nFirstInterval < itrGrp->first && itrGrp->second < _cluster.m_nLastInterval) {
                observed_data << "null";
            } else {
                observed_data << observed;
            }
            cluster_data <<  (itrGrp==groups.getGroups().begin() ? "" : ",");
            if (_cluster.m_nFirstInterval <= itrGrp->first && itrGrp->first < _cluster.m_nLastInterval) {
                cluster_data << observed;
            } else {
                cluster_data << "null";
            }
        }
        templateReplace(html, "--categories--", categories.str());
        templateReplace(html, "--observed_data--", observed_data.str());
        templateReplace(html, "--expected_data--", expected_data.str());
        templateReplace(html, "--cluster_data--", cluster_data.str());

        HTMLout << html.str() << std::endl;
        HTMLout.close();
    } catch (prg_exception& x) {
        x.addTrace("generate()","TemporalChartGenerator");
        throw;
    }
}

/** Alters pass Filename to include suffix and extension. */
FileName& TemporalChartGenerator::getFilename(FileName& filename) {
    std::string buffer;
    printString(buffer, "%s%s", filename.getFileName().c_str(), FILE_SUFFIX_EXT);
    filename.setFileName(buffer.c_str());
    filename.setExtension(HTML_FILE_EXT);
    return filename;
}

/** ------------------- GiniChartGenerator --------------------------------*/

const char * GiniChartGenerator::FILE_SUFFIX_EXT = "_gini";

const char * GiniChartGenerator::TEMPLATE_HEADER = "\n \
        <script type=\"text/javascript\"> \n \
            $(document).ready(function () { \n \
                var chart = new Highcharts.Chart({ \n \
                    chart: { renderTo: 'container', type: 'line', zoomType:'xy', resetZoomButton: {relativeTo: 'chart', position: {x: -80, y: 10}, theme: {fill: 'white',stroke: 'silver',r: 0,states: {hover: {fill: '#41739D', style: { color: 'white' } } } } }, marginBottom: 80, borderColor: '#888888', plotBackgroundColor: '#e6e7e3', borderRadius: 10, borderWidth: 4, marginRight: 20 }, \n \
                    title: { text: 'Gini coefficient at Spatial Window Stops', align: 'center' }, \n \
                    subtitle: { text: 'Coefficients based on clusters with p<--gini-pvalue--.' }, \n \
                    legend: { backgroundColor: '#F5F5F5' }, \n \
                    tooltip: { crosshairs: true }, \n \
                    xAxis: { categories: [--categories--], tickmarkPlacement: 'on', labels: { formatter: function() { return this.value +'%'} } }, \n \
                    yAxis: { title: { enabled: true, text: 'Gini coefficient', style: { fontWeight: 'normal' } }, min: 0 }, \n \
                    navigation: { buttonOptions: { align: 'right' } }, \n \
                    series: [ { type: 'line', name: 'Gini coefficient', color: '#4572A7', marker: { enabled: true, symbol: 'circle', radius: 6 }, data: [--gini-data--]}, ] \n \
                }); \n \
                $('#toggle_type').click(function(){var newType=chart.series[0].type=='line'?'column':'line';for(var i=0;i<chart.series.length;i++){serie = chart.series[0];serie.chart.addSeries({type:newType,name:serie.name,zIndex: serie.options.zIndex,color:serie.color,marker:{enabled:true,radius:0},data:serie.options.data},false);serie.remove();}return false;}); \n \
            }); \n \
        </script> \n";

const char * GiniChartGenerator::TEMPLATE_BODY = "\n \
        <a href=\"\" id=\"toggle_type\" style=\"margin-top:20px;\">Toggle Types</a> \n \
        <div id=\"container\" style=\"margin-top:20px;\"></div> \n";

/** Creates HighCharts graph for Gini coefficients. */
void GiniChartGenerator::generateChart() const {
    std::string buffer;
    FileName fileName;

    try {
        fileName.setFullPath(_dataHub.GetParameters().GetOutputFileName().c_str());
        getFilename(fileName);

        std::ofstream HTMLout;
        //open output file
        HTMLout.open(fileName.getFullPath(buffer).c_str());
        if (!HTMLout) throw resolvable_error("Error: Could not open file '%s'.\n", fileName.getFullPath(buffer).c_str());
        std::stringstream html, categories, gini_data;

        // read template into stringstream
        html << BASE_TEMPLATE << std::endl;
        // replace specialized header
        templateReplace(html, "--header--", TEMPLATE_HEADER);
        // site resource link path
        templateReplace(html, "--resource-path--", AppToolkit::getToolkit().GetWebSite());
        // replace page title
        templateReplace(html, "--title--", "Gini coefficient");
        // replace specialized body
        templateReplace(html, "--body--", TEMPLATE_BODY);

        // calculate maximized gini collection
        double maxGINI = 0;
        const MostLikelyClustersContainer* maximizedCollection = 0;
        for (MLC_Collections_t::const_iterator itrMLC=_mlc.begin(); itrMLC != _mlc.end(); ++itrMLC) {
            double gini = itrMLC->getGiniCoefficient(_dataHub, _simVars, _dataHub.GetParameters().getGiniIndexPValueCutoff());
            if (gini > maxGINI) {
                maximizedCollection = &(*itrMLC);
                maxGINI = gini;
            }
        }
        // define categories and gini data
        for (MLC_Collections_t::const_iterator itrMLC=_mlc.begin(); itrMLC != _mlc.end(); ++itrMLC) {
            categories << (itrMLC == _mlc.begin() ? "" : ",") << itrMLC->getMaximumWindowSize();
            double gini = itrMLC->getGiniCoefficient(_dataHub, _simVars, _dataHub.GetParameters().getGiniIndexPValueCutoff());
            getValueAsString(gini, buffer, 4).c_str();
            gini_data << (itrMLC == _mlc.begin() ? "" : ",");
            if ((maximizedCollection == &(*itrMLC))) {
                gini_data << "{y: " << buffer.c_str() << ", name: 'Optimal Gini coefficient', marker: {symbol: 'circle', radius: 6, fillColor: 'red', states: {hover: {fillColor: 'red', radius: 8}}} }";
            } else {
                gini_data << buffer.c_str();
            }
        }

        // replace categories in template
        templateReplace(html, "--categories--", categories.str());
        // replace gini data in template
        templateReplace(html, "--gini-data--", gini_data.str());
        // replace gini p-value cutoff
        templateReplace(html, "--gini-pvalue--", getValueAsString(_dataHub.GetParameters().getGiniIndexPValueCutoff(), buffer));

        HTMLout << html.str() << std::endl;
        HTMLout.close();
    } catch (prg_exception& x) {
        x.addTrace("generate()","GiniChartGenerator");
        throw;
    }
}

/** Alters pass Filename to include suffix and extension. */
FileName& GiniChartGenerator::getFilename(FileName& filename) {
    std::string buffer;
    printString(buffer, "%s%s", filename.getFileName().c_str(), FILE_SUFFIX_EXT);
    filename.setFileName(buffer.c_str());
    filename.setExtension(HTML_FILE_EXT);
    return filename;
}
