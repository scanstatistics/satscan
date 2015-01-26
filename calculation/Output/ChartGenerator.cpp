//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "ChartGenerator.h"
#include "SaTScanData.h"
#include "cluster.h"
#include "SimulationVariables.h"
#include "Toolkit.h"
#include <boost/regex.hpp>

/** ------------------- AbstractChartGenerator --------------------------------*/
const char * AbstractChartGenerator::HTML_FILE_EXT = ".html";

const char * AbstractChartGenerator::TEMPLATE_BODY = "\n \
        <body style=\"margin:0;background-color: #fff;\"> \n \
        <table width=\"100%\" border=\"0\" cellpadding=\"0\" cellspacing=\"0\" bgcolor=\"#F8FAFA\" style=\"border-bottom: 3px double navy;\"> \n \
        <tbody><tr> \n \
        <td width=\"120\" align=\"center\" bgcolor=\"#DBD7DB\"><img src=\"--resource-path--images/swe2.jpg\" alt=\"&Ouml;stersund map\" title=\"Östersund map\" width=\"120\" height=\"115\" hspace=\"1\" border=\"0\"></td> \n \
        <td align=\"right\" bgcolor=\"#D4DCE5\"><img src=\"--resource-path--images/satscan_title2.jpg\" alt=\"SaTScan&#0153; - Software for the spatial, temporal, and space-time scan statistics\" title=\"SaTScan&#0153; - Software for the spatial, temporal, and space-time scan statistics\" width=\"470\" height=\"115\"></td> \n \
        <td width=\"25%\" bgcolor=\"#F8FAFA\" align=\"right\"><img src=\"--resource-path--images/nyc2.jpg\" alt=\"New York City map\" title=\"New York City map\" width=\"112\" height=\"115\" hspace=\"1\" border=\"0\" align=\"middle\"></td> \n \
        </tr></tbody></table> \n \
        --main-content-- \n";

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

const char * TemporalChartGenerator::BASE_TEMPLATE = " \
<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\" \"http://www.w3.org/TR/html4/strict.dtd\"> \n \
<html lang=\"en\"> \n \
    <head> \n \
        <title>--title--</title> \n \
        <meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\"> \n \
        <style type=\"text/css\"> \n \
        body{font:100% Arial,Helvetica;background:#9ea090} \n \
        .chart-options{display:none} \n \
        .show-chart-options,.hide-chart-options{color:#13369f;border:1px solid #d7e9ed;margin-top:3px;-webkit-border-radius:3px;-moz-border-radius:3px} \n \
        .show-chart-options a,.hide-chart-options a{color:#13369f;background-color:#f5f8fa;font-size:11px;text-decoration:none;padding:4px 6px;display:block} \n \
        .show-chart-options a{padding-left:24px;background-image:url('--resource-path--images/down_grip.png');background-position:5px 4px;background-repeat:no-repeat;color:#13369f} \n \
        .hide-chart-options a{padding-left:24px;background-image:url('--resource-path--images/up_grip.png');background-position:5px 4px;background-repeat:no-repeat;color:#13369f} \n \
        .show-chart-options a:hover{background-color:#d9e6ec;color:#13369f;text-decoration:underline;background-image:url('--resource-path--images/down_grip_selected.png')} \n \
        .hide-chart-options a{border-top:1px solid #d7e9ed;background-image:url('--resource-path--images/up_grip.png')} \n \
        .hide-chart-options a:hover{background-color:#d9e6ec;color:#13369f;text-decoration:underline;background-image:url('--resource-path--images/up_grip_selected.png')} \n \
        .chart-options{padding:10px 0 10px 0;background-color:#e6eef2;border:1px solid silver} \n \
        .options-row{margin:0 10px 10px 10px} \n \
        .options-row>label:first-child, .options-row detail{color:#13369f;font-weight:bold} \n \
        .options-row input[type='radio']{margin:5px} \n \
        .options-table h4{text-align:center;color:#13369f;font-weight:bold;margin:0} \n \
        .options-table th{vertical-align:top;text-align:right;color:#13369f} \n \
        .help-block{font-size:11px;color:#666;font-style:oblique;margin:0} \n \
        </style> \n \
        <script type='text/javascript' src='--resource-path--javascript/jquery/jquery-1.9.0/jquery-1.9.0.js'></script> \n \
        <script type='text/javascript' src='--resource-path--javascript/highcharts/highcharts-4.0.4/js/highcharts.js'></script> \n \
        <script type='text/javascript' src='--resource-path--javascript/highcharts/highcharts-4.0.4/js/modules/exporting.js'></script> \n \
        <script type='text/javascript'> \n \
            var charts = {}; \n \
            $(document).ready(function () { \n \
                --charts--   \n\n \
                $('.chart-section').each(function() { $(this).find('.title-setter').val(charts[$(this).find('.highchart-container').first().attr('id')].title.textStr); }); \n \
                $('.title-setter').keyup(function(){ charts[$(this).parents('.chart-section').find('.highchart-container').first().attr('id')].setTitle({text: $( this ).val()}); }); \n \
                $('.show-chart-options a').click(function(event) { event.preventDefault(); $(this).parents('.options').find('.chart-options').show().end().find('.show-chart-options').hide(); }); \n \
                $('.hide-chart-options a').click(function(event) { event.preventDefault(); $(this).parents('.options').find('.chart-options').hide().end().find('.show-chart-options').show(); }); \n \
                $('.options-row input[type=\"radio\"]').click(function(event) { \n \
                    var series_type = $(this).attr('series-type'); \n \
                    var chart = charts[$(this).parents('.chart-section').find('.highchart-container').first().attr('id')]; \n \
                    $.each($(this).attr('series-id').split(','), function(index, value) { chart.get(value).update({type:series_type}, true); }); \n \
                }); \n \
                $('.options-row input[type=\"checkbox\"]').click(function(event) { \n \
                    var chart = charts[$(this).parents('.chart-section').find('.highchart-container').first().attr('id')]; \n \
                    if ($(this).is(':checked')) { \n \
                        chart.xAxis[0].addPlotBand({color: '#FFB3B3', from: $(this).attr('start-idx'), to: $(this).attr('end-idx'), id: 'band', zindex:0}); \n \
                        chart.xAxis[0].addPlotLine({id:'start',color: '#FF0000', width: 1, value: $(this).attr('start-idx'), zIndex: 5 }); \n \
                        chart.xAxis[0].addPlotLine({id:'end',color: '#FF0000', width: 1, value: $(this).attr('end-idx'), zIndex: 5 }); \n \
                    } else { \n \
                        chart.xAxis[0].removePlotBand('band' ); \n \
                        chart.xAxis[0].removePlotLine('start'); \n \
                        chart.xAxis[0].removePlotLine('end'); \n \
                    } \n \
                }); \n \
            }); \n \
        </script> \n \
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

const char * TemporalChartGenerator::TEMPLATE_CHARTHEADER = "\n \
                var --container-id-- = new Highcharts.Chart({ \n \
                    chart: { renderTo: '--container-id--', zoomType:'x', resetZoomButton: {relativeTo: 'chart', position: {x: -80, y: 10}, theme: {fill: 'white',stroke: 'silver',r: 0,states: {hover: {fill: '#41739D', style: { color: 'white' } } } } }, marginBottom: --margin-bottom--, borderColor: '#888888', plotBackgroundColor: '#e6e7e3', borderRadius: 0, borderWidth: 1, marginRight: --margin-right-- }, \n \
                    title: { text: '--chart-title--', align: 'center' }, \n \
                    exporting: {filename: 'cluster_graph'}, \n \
                    plotOptions: { column: { grouping: false }}, \n \
                    tooltip: { crosshairs: true, shared: true, formatter: function(){var is_cluster = false;var has_observed = false;$.each(this.points, function(i, point) {if (point.series.options.id == 'cluster') {is_cluster = true;}if (point.series.options.id == 'obs') {has_observed = true;}});var s = '<b>'+ this.x +'</b>'; if (is_cluster) {s+= '<br/><b>Cluster Point</b>';}$.each(this.points,function(i, point){if (point.series.options.id == 'cluster'){if (!has_observed) {s += '<br/>Observed: '+ point.y;}} else {s += '<br/>'+ point.series.name +': '+ point.y;}});return s;}, }, \n \
                    legend: { backgroundColor: '#F5F5F5', verticalAlign: 'top', y: 40 }, \n \
                    xAxis: [{ categories: [--categories--], tickmarkPlacement: 'on', labels: { step: --step--, rotation: -45, align: 'right' } }], \n \
                    yAxis: [{ title: { enabled: true, text: 'Number of Cases', style: { fontWeight: 'normal' } }, min: 0 }--additional-yaxis--], \n \
                    navigation: { buttonOptions: { align: 'right' } }, \n \
                    series: [--series--]\n \
                }); \n \
                charts['--container-id--'] = --container-id--;";

const char * TemporalChartGenerator::TEMPLATE_CHARTSECTION = "\
         <div style=\"margin:20px;\" class=\"chart-section\"> \n \
            <div id=\"--container-id--\" class=\"highchart-container\" style=\"margin-top:0px;\"></div> \n \
            <div class=\"options\"> \n \
                <div class=\"show-chart-options\"><a href=\"#\">Show Chart Options</a></div> \n \
                <div class=\"chart-options\"> \n \
                    <div class=\"options-table\"> \n \
                      <h4>Chart Options</h4> \n \
                      <div class=\"options-row\"> \n \
                          <label for=\"title_obs\">Title</label> \n \
                          <div><input type=\"text\" style=\"width:95%;\" class=\"title-setter\" id=\"title_obs\"> \n \
                              <p class=\"help-block\">The graph title can be updated simply by editing this text.</p> \n \
                          </div> \n \
                      </div> \n \
                      <div class=\"options-row\"> \n \
                          <label>Observed Chart Type</label> \n \
                          <div> \n \
                            <label> \n \
                              <input type=\"radio\" name=\"--container-id--_obs_series_type\" series-type=\"column\" series-id=\"--chart-switch-ids--\" checked=checked/>Histogram \n \
                            </label> \n \
                            <label> \n \
                              <input type=\"radio\" name=\"--container-id--_obs_series_type\" series-type=\"line\" series-id=\"--chart-switch-ids--\"/>Line \n \
                            </label> \n \
                            <p class=\"help-block\">Switch the series type between line and histogram.</p> \n \
                          </div> \n \
                      </div> \n \
                      <div class=\"options-row\"> \n \
                          <label>Cluster Band</label> \n \
                          <div> \n \
                            <label> \n \
                              <input type=\"checkbox\" name=\"--container-id--_cluster_band\" start-idx=\"--cluster-start-idx--\" end-idx=\"--cluster-end-idx--\"/>Show Cluster Band \n \
                            </label> \n \
                            <p class=\"help-block\">Band stretching across the plot area marking cluster interval.</p> \n \
                          </div> \n \
                      </div> \n \
                      <div class=\"options-row\">To zoom a portion of the chart, select and drag mouse within the chart.</div> \n \
                    </div> \n \
                    <div class=\"hide-chart-options\"><a href=\"#\">Close Chart Options</a></div> \n \
                </div> \n \
            </div> \n \
         </div> \n";

/** constructor */
TemporalChartGenerator::TemporalChartGenerator(const CSaTScanData& dataHub, const MostLikelyClustersContainer & clusters, const SimulationVariables& simVars) 
    :_dataHub(dataHub), _clusters(clusters), _simVars(simVars) {}

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
    std::string buffer, buffer2;
    FileName fileName;

    try {
        fileName.setFullPath(_dataHub.GetParameters().GetOutputFileName().c_str());
        getFilename(fileName);

        std::ofstream HTMLout;
        //open output file
        HTMLout.open(fileName.getFullPath(buffer).c_str());
        if (!HTMLout) throw resolvable_error("Error: Could not open file '%s'.\n", fileName.getFullPath(buffer).c_str());
        if (!_clusters.GetNumClustersRetained()) {
            HTMLout.close();
            return;
        }

        std::stringstream html, charts_javascript, cluster_sections;

        // read template into stringstream
        html << BASE_TEMPLATE << std::endl;
        // replace page title
        templateReplace(html, "--title--", "Cluster Temporal Graph");
        // replace specialized body
        templateReplace(html, "--body--", TEMPLATE_BODY);
        // site resource link path
        templateReplace(html, "--resource-path--", AppToolkit::getToolkit().GetWebSite());

        // set margin bottom according to time precision
        int margin_bottom=130;
        switch (_dataHub.GetParameters().GetPrecisionOfTimesType()) {
            case YEAR : margin_bottom = 90; break;
            case MONTH : margin_bottom = 110; break;
            case DAY:
            case GENERIC:
            default: margin_bottom=130;
        }

        // We might need to calculated purely temporal data structures.
        // TODO: Is there a better way to do this?
        const DataSetHandler& handler = _dataHub.GetDataSetHandler();
        if (_dataHub.GetParameters().GetAnalysisType() != PURELYTEMPORAL) {
            DataSetHandler& temp_handler = const_cast<DataSetHandler&>(handler);
            for (size_t idx=0; idx < temp_handler.GetNumDataSets(); ++idx) {
                temp_handler.GetDataSet(idx).setCaseData_PT();
                temp_handler.GetDataSet(idx).setMeasureData_PT();
            }
        }

        // Determine clusters will have a graph generated based on settings.
        std::vector<const CCluster*> graphClusters;
        switch (_dataHub.GetParameters().getTemporalGraphReportType()) {
        case MLC_ONLY :
            graphClusters.push_back(&_clusters.GetCluster(0)); break;
        case X_MCL_ONLY :
            for (int i=0; i < _dataHub.GetParameters().getTemporalGraphMostLikelyCount() && i < _clusters.GetNumClustersRetained(); ++i)
                graphClusters.push_back(&_clusters.GetCluster(i)); 
            break;
        case SIGNIFICANT_ONLY :
            for (int i=0; i < _clusters.GetNumClustersRetained(); ++i) {
                const CCluster & cluster = _clusters.GetCluster(i);
                if (cluster.getReportingPValue(_dataHub.GetParameters(), _simVars, i == 0) <= _dataHub.GetParameters().getTemporalGraphSignificantCutoff())
                    graphClusters.push_back(&cluster); 
            }
            break;
        }

        for (size_t clusterIdx=0; clusterIdx < graphClusters.size(); ++clusterIdx) {
            const CCluster& cluster = *graphClusters[clusterIdx];
            // This graph is only valid for temporal clusters.
            if (!(cluster.GetClusterType() == PURELYTEMPORALCLUSTER || cluster.GetClusterType() == SPACETIMECLUSTER))
                continue;
            // calculate the graphs interval groups for this cluster
            intervalGroups groups = getIntervalGroups(cluster);
            for (size_t setIdx=0; setIdx < handler.GetNumDataSets(); ++setIdx) {
                std::stringstream chart_js, chart_series, chart_section, categories;
                // set the chart header for this cluster
                chart_js << TEMPLATE_CHARTHEADER;

                bool is_pt(cluster.GetClusterType() == PURELYTEMPORALCLUSTER); // not if cluster is purely temporal
                // define seach series that we'll graph - next three are always printed.
                std::auto_ptr<ChartSeries> observedSeries(new ChartSeries("obs", 1, "column", (is_pt ? "Observed" : "Observed (Outside Cluster)"), "4572A7", "square", 0));
                std::auto_ptr<ChartSeries> expectedSeries(new ChartSeries("exp", is_pt ? 3 : 2, "line", (is_pt ? "Expected" : "Expected (Outside Cluster)"), "89A54E", "triangle", 0));
                std::auto_ptr<ChartSeries> clusterSeries(new ChartSeries("cluster", is_pt ? 2 : 5, "column", "Cluster", "AA4643", "circle", 0));
                // the remaining series are conditionally present in the chart
                std::auto_ptr<ChartSeries> observedClusterSeries, expectedClusterSeries;
                std::auto_ptr<ChartSeries> odeSeries, cluster_odeSeries;

                // space-time clusters also graph series which allow comparison between inside and outside the cluster
                if (cluster.GetClusterType() != PURELYTEMPORALCLUSTER) {
                    observedClusterSeries.reset(new ChartSeries("cluster_obs", 3, "column", "Observed (Inside Cluster)", "003264", "square", 0));
                    expectedClusterSeries.reset(new ChartSeries("cluster_exp", 4, "line", "Expected (Inside Cluster)", "394521", "triangle", 0));
                }

                // the Poisson and Exponential models also graphs observed / expected
                if (_dataHub.GetParameters().GetProbabilityModelType() == POISSON || _dataHub.GetParameters().GetProbabilityModelType() == EXPONENTIAL) {
                    // graphing observed / expected, with y-axis along right side
                    templateReplace(chart_js, "--additional-yaxis--", ", { title: { enabled: true, text: 'Observed / Expected', style: { fontWeight: 'normal' } }, min: 0, opposite: true }");
                    odeSeries.reset(new ChartSeries("obs_exp", 2, "line", (is_pt ? "Observed / Expected" : "Observed / Expected (Outside Cluster)"), "00FF00", "triangle", 1));
                    if (cluster.GetClusterType() != PURELYTEMPORALCLUSTER)
                        // space-time clusters also graph series which allow comparison between inside and outside the cluster
                        cluster_odeSeries.reset(new ChartSeries("cluster_obs_exp", 2, "line", "Observed / Expected (Inside Cluster)", "FF8000", "triangle", 1));
                } else if (_dataHub.GetParameters().GetProbabilityModelType() == BERNOULLI) {
                    // the Bernoulli model also graphs cases / (cases + controls)
                    // graphing cases ratio, with y-axis along right side
                    templateReplace(chart_js, "--additional-yaxis--", ", { title: { enabled: true, text: 'Cases Ratio', style: { fontWeight: 'normal' } }, max: 1, min: 0, opposite: true }");
                    odeSeries.reset(new ChartSeries("case_ratio", 2, "line", (is_pt ? "Cases Ratio" : "Cases Ratio (Outside Cluster)"), "00FF00", "triangle", 1));
                    if (cluster.GetClusterType() != PURELYTEMPORALCLUSTER)
                        // space-time clusters also graph series which allow comparison between inside and outside the cluster
                        cluster_odeSeries.reset(new ChartSeries("cluster_case_ratio", 2, "line", "Cases Ratio (Inside Cluster)", "FF8000", "triangle", 1));
                } else {
                    templateReplace(chart_js, "--additional-yaxis--", "");
                }
                
                // set default chart title 
                if (_dataHub.GetParameters().GetAnalysisType() == PURELYTEMPORAL)
                    buffer = "Detected Cluster";
                else 
                    printString(buffer, "Cluster #%u", clusterIdx + 1);
                // potentially include data set index
                if (handler.GetNumDataSets() > 1)
                    buffer += printString(buffer2, " Data Set #%u", setIdx + 1);
                templateReplace(chart_js, "--chart-title--", buffer);
                templateReplace(chart_js, "--margin-bottom--", printString(buffer, "%d", margin_bottom));
                templateReplace(chart_js, "--margin-right--", printString(buffer, "%d", (odeSeries.get() || cluster_odeSeries.get() ? 80 : 20)));

                // increase x-axis 'step' if there are many intervals, so that labels are not crowded
                //  -- empirically, 50 ticks seems like a good upper limit
                templateReplace(chart_js, "--step--", printString(buffer, "%u", static_cast<int>(std::ceil(static_cast<double>(groups.getGroups().size())/50.0))));

                // get series datastreams plus cluster indexes start and end ticks
                std::pair<int,int> cluster_grp_idx = getSeriesStreams(cluster, groups, setIdx, categories, *clusterSeries, 
                                                                      *observedSeries, *expectedSeries, 
                                                                      observedClusterSeries.get(), expectedClusterSeries.get(),
                                                                      odeSeries.get(), cluster_odeSeries.get());

                // define the identifying attribute of this chart
                printString(buffer, "chart_%d_%u", clusterIdx + 1, setIdx + 1);
                templateReplace(chart_js, "--container-id--", buffer);
                templateReplace(chart_js, "--categories--", categories.str());

                // replace the series
                chart_series << clusterSeries->toString(buffer).c_str();
                chart_series << "," << observedSeries->toString(buffer).c_str();
                chart_series << "," << expectedSeries->toString(buffer).c_str();
                if (observedClusterSeries.get())
                    chart_series << "," << observedClusterSeries->toString(buffer).c_str();
                if (expectedClusterSeries.get())
                    chart_series << "," << expectedClusterSeries->toString(buffer).c_str();
                if (odeSeries.get()) 
                    chart_series << "," << odeSeries->toString(buffer).c_str();
                if (cluster_odeSeries.get())
                    chart_series << "," << cluster_odeSeries->toString(buffer).c_str();
                templateReplace(chart_js, "--series--", chart_series.str());

                // add this charts javascript to collection
                charts_javascript << chart_js.str() << std::endl;

                // create chart html section
                chart_section << TEMPLATE_CHARTSECTION;
                printString(buffer, "chart_%d_%u", clusterIdx + 1, setIdx + 1);
                templateReplace(chart_section, "--container-id--", buffer);
                printString(buffer, "%d", cluster_grp_idx.first);
                templateReplace(chart_section, "--cluster-start-idx--", buffer);
                printString(buffer, "%d", cluster_grp_idx.second);
                templateReplace(chart_section, "--cluster-end-idx--", buffer);
                templateReplace(chart_section, "--chart-switch-ids--", cluster.GetClusterType() == PURELYTEMPORALCLUSTER ? "obs,cluster" : "obs,cluster,cluster_obs");
                // add section to collection of sections
                cluster_sections << chart_section.str() << std::endl << std::endl;
            }
        }

        templateReplace(html, "--charts--", charts_javascript.str());
        if (graphClusters.size()) {
            templateReplace(html, "--main-content--", cluster_sections.str());
        } else {
            printString(buffer2, "<h3 style=\"text-align:center;\">No significant clusters to graph. All clusters had a p-value greater than %lf.</h3>", _dataHub.GetParameters().getTemporalGraphSignificantCutoff());
            templateReplace(html, "--main-content--", buffer2.c_str());
        }
        HTMLout << html.str() << std::endl;
        HTMLout.close();
    } catch (prg_exception& x) {
        x.addTrace("generate()","TemporalChartGenerator");
        throw;
    }
}

/* Calculates the best fit graph groupings for this cluster. */
TemporalChartGenerator::intervalGroups TemporalChartGenerator::getIntervalGroups(const CCluster& cluster) const {
    intervalGroups groups;
    int intervals = _dataHub.GetNumTimeIntervals();

    if (intervals <= MAX_INTERVALS) {
        // number of groups equals the number of intervals
        for (int i=0; i < intervals; ++i) {
            groups.addGroup(i, i+1);
        }
    } else {
        int cluster_length = cluster.m_nLastInterval - cluster.m_nFirstInterval;
        if (cluster_length <= MAX_INTERVALS) {
            int extra_intervals = ((MAX_INTERVALS - cluster_length)/2) + 5;
            // we can show entire cluster intervals plus a few before and after
            for (int i=std::max(0, cluster.m_nFirstInterval - extra_intervals); i < std::min(intervals, cluster.m_nLastInterval + extra_intervals); ++i) {
                groups.addGroup(i, i+1);
            }
        } else {
            // we can show entire cluster intervals but compressed -- plus a few before and after
            int compressed_interval_length = static_cast<int>(ceil(static_cast<double>(cluster.m_nLastInterval - cluster.m_nFirstInterval)/static_cast<double>(MAX_INTERVALS)));
            // Note: This rough calculation of a compressed interval means that the clusters last interval might not fall cleanly onto a interval boundary.
            int extra_intervals = compressed_interval_length * 5;
            for (int i=std::max(0, cluster.m_nFirstInterval - extra_intervals); i < std::min(intervals, cluster.m_nLastInterval + extra_intervals); i=i+compressed_interval_length) {
                groups.addGroup(i, i + compressed_interval_length);
            }
        }
    }
    return groups;
}

/* Calculates the series values in a purely temporal context. */
std::pair<int, int> TemporalChartGenerator::getSeriesStreams(const CCluster& cluster,const intervalGroups& groups, size_t dataSetIdx,
                                                             std::stringstream& categories, ChartSeries& clusterSeries,
                                                             ChartSeries& observedSeries, ChartSeries& expectedSeries,
                                                             ChartSeries * cluster_observedSeries, ChartSeries * cluster_expectedSeries,
                                                             ChartSeries * odeSeries, ChartSeries * cluster_odeSeries) const {

    std::string buffer;
    double adjustment = _dataHub.GetMeasureAdjustment(dataSetIdx);
    int intervals = _dataHub.GetNumTimeIntervals();
    std::pair<int, int> groupClusterIdx(std::numeric_limits<int>::max(), std::numeric_limits<int>::min());
    const DataSetHandler& handler = _dataHub.GetDataSetHandler();
    count_t * pcases = handler.GetDataSet(dataSetIdx).getCaseData_PT();
    measure_t * pmeasure = handler.GetDataSet(dataSetIdx).getMeasureData_PT();
    count_t ** ppcases = handler.GetDataSet(dataSetIdx).getCaseData().GetArray();
    measure_t ** ppmeasure = handler.GetDataSet(dataSetIdx).getMeasureData().GetArray();

    // define categories and replace in template
    const std::vector<Julian>& startDates = _dataHub.GetTimeIntervalStartTimes();
    DatePrecisionType precision = _dataHub.GetParameters().GetPrecisionOfTimesType();
    // iterate through groups, creating totals for each interval grouping
    for (intervalGroups::intervals_t::const_iterator itrGrp=groups.getGroups().begin(); itrGrp != groups.getGroups().end(); ++itrGrp) {
        // define date categories
        categories << (itrGrp==groups.getGroups().begin() ? "'" : ",'") << JulianToString(buffer, startDates[itrGrp->first], precision).c_str() << "'";
        // calcuate the expected and observed for this interval
        measure_t expected=0, cluster_expected=0;
        count_t observed=0, cluster_observed=0;
        for (int i=itrGrp->first; i < itrGrp->second; ++i) {
            expected += (i == intervals - 1 ? pmeasure[i] : pmeasure[i] - pmeasure[i+1]);
            observed += (i == intervals - 1 ? pcases[i] : pcases[i] - pcases[i+1]);
        }
        // if not purely temporal cluster, we're expressing this as outside verse inside cluster
        if (cluster_observedSeries || cluster_expectedSeries) {
            // calculate cluster observed and expected series across entire period, not just cluster window
            for (tract_t t=1; t <= cluster.GetNumTractsInCluster(); ++t) {
                tract_t tTract = _dataHub.GetNeighbor(cluster.GetEllipseOffset(), cluster.GetCentroidIndex(), t, cluster.GetCartesianRadius());
                if (tTract >= _dataHub.GetNumTracts() && _dataHub.GetTInfo()->getMetaManagerProxy().getNumMetaLocations()) {
                    //When the location index exceeds number of tracts and the meta neighbors manager contains
                    //entries, we need to resolve meta location into it's real location indexes.
                    std::vector<tract_t> indexes;
                    _dataHub.GetTInfo()->getMetaManagerProxy().getIndexes(tTract - _dataHub.GetNumTracts(), indexes);
                    for (size_t t=0; t < indexes.size(); ++t) {
                        for (int i=itrGrp->first; i < itrGrp->second; ++i) {
                            cluster_observed += (i == intervals - 1 ? ppcases[i][indexes[t]] : ppcases[i][indexes[t]] - ppcases[i+1][indexes[t]]);
                            cluster_expected += (i == intervals - 1 ? ppmeasure[i][indexes[t]] : ppmeasure[i][indexes[t]] - ppmeasure[i+1][indexes[t]]);
                        }
                    }
                } else {
                    for (int i=itrGrp->first; i < itrGrp->second; ++i) {
                        cluster_observed += (i == intervals - 1 ? ppcases[i][tTract] : ppcases[i][tTract] - ppcases[i+1][tTract]);
                        cluster_expected += (i == intervals - 1 ? ppmeasure[i][tTract] : ppmeasure[i][tTract] - ppmeasure[i+1][tTract]);
                    }
                }
            }
            // removed observed and expected from overall temporal values
            observed -= cluster_observed;
            expected -= cluster_expected;
            if (cluster_odeSeries) {
                // For the Bernoulli model, this represents the ratio of cases / (cases + controls) inside the cluster.
                // For the Poisson/Exponential models, this represents the ratio of observed / expected inside the cluster.
                getValueAsString(cluster_expected ? static_cast<measure_t>(cluster_observed)/cluster_expected : 0, buffer, 2);
                cluster_odeSeries->datastream() <<  (itrGrp==groups.getGroups().begin() ? "" : ",") <<  buffer.c_str();
            }
        }
        if (odeSeries) {
            // For the Bernoulli model, this represents the ratio of cases / (cases + controls) inside the cluster.
            // For the Poisson/Exponential models, this represents the ratio of observed / expected inside the cluster.
            getValueAsString(expected ? static_cast<measure_t>(observed)/expected : 0, buffer, 2);
            odeSeries->datastream() << (itrGrp==groups.getGroups().begin() ? "" : ",") <<  buffer.c_str();
        }
        // apply measure adjustment now
        expected *= adjustment;
        cluster_expected *= adjustment;
        // put totals to other streams
        expectedSeries.datastream() << (itrGrp==groups.getGroups().begin() ? "" : ",") << getValueAsString(expected, buffer, 2).c_str();
        observedSeries.datastream() <<  (itrGrp==groups.getGroups().begin() ? "" : ",") << observed;
        if (cluster_expectedSeries)
            cluster_expectedSeries->datastream() << (itrGrp==groups.getGroups().begin() ? "" : ",") << getValueAsString(cluster_expected, buffer, 2).c_str();
        if (cluster_observedSeries)
            cluster_observedSeries->datastream() << (itrGrp==groups.getGroups().begin() ? "" : ",") << cluster_observed;
        if (cluster.m_nFirstInterval <= itrGrp->first && itrGrp->second <= cluster.m_nLastInterval) {
            groupClusterIdx.first = std::min(groupClusterIdx.first, itrGrp->first);
            groupClusterIdx.second = std::max(groupClusterIdx.second, itrGrp->second) - 1;
            clusterSeries.datastream() <<  (itrGrp==groups.getGroups().begin() ? "" : ",") << (cluster.GetClusterType() == PURELYTEMPORALCLUSTER ? observed : cluster_observed);
        } else {
            clusterSeries.datastream() << (itrGrp==groups.getGroups().begin() ? "" : ",") << "null";
        }
    }

    // if cluster start index is not set, set to start of first group
    if (groupClusterIdx.first == std::numeric_limits<int>::max()) {
        groupClusterIdx.first = groups.getGroups().front().first;
    }
    // if cluster end index is not set, set to end of last group
    if (groupClusterIdx.second == std::numeric_limits<int>::min()) {
        groupClusterIdx.second = groups.getGroups().back().second;
    }
    return groupClusterIdx;
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

const char * GiniChartGenerator::BASE_TEMPLATE = " \
<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\" \"http://www.w3.org/TR/html4/strict.dtd\"> \n \
<html lang=\"en\"> \n \
    <head> \n \
        <title>--title--</title> \n \
        <meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\"> \n \
        <style type=\"text/css\"> \n \
        body{font:100% Arial,Helvetica;background:#9ea090} \n \
        .chart-options{display:none} \n \
        .show-chart-options,.hide-chart-options{color:#13369f;border:1px solid #d7e9ed;margin-top:3px;-webkit-border-radius:3px;-moz-border-radius:3px} \n \
        .show-chart-options a,.hide-chart-options a{color:#13369f;background-color:#f5f8fa;font-size:11px;text-decoration:none;padding:4px 6px;display:block} \n \
        .show-chart-options a{padding-left:24px;background-image:url('--resource-path--images/down_grip.png');background-position:5px 4px;background-repeat:no-repeat;color:#13369f} \n \
        .hide-chart-options a{padding-left:24px;background-image:url('--resource-path--images/up_grip.png');background-position:5px 4px;background-repeat:no-repeat;color:#13369f} \n \
        .show-chart-options a:hover{background-color:#d9e6ec;color:#13369f;text-decoration:underline;background-image:url('--resource-path--images/down_grip_selected.png')} \n \
        .hide-chart-options a{border-top:1px solid #d7e9ed;background-image:url('--resource-path--images/up_grip.png')} \n \
        .hide-chart-options a:hover{background-color:#d9e6ec;color:#13369f;text-decoration:underline;background-image:url('--resource-path--images/up_grip_selected.png')} \n \
        .chart-options{padding:10px 0 10px 0;background-color:#e6eef2;border:1px solid silver} \n \
        .options-row{margin:0 10px 10px 10px} \n \
        .options-row>label:first-child, .options-row detail{color:#13369f;font-weight:bold} \n \
        .options-table h4{text-align:center;color:#13369f;font-weight:bold;margin:0} \n \
        .options-table th{vertical-align:top;text-align:right;color:#13369f} \n \
        .help-block{font-size:11px;color:#666;font-style:oblique;margin:0} \n \
        </style> \n \
        <script type='text/javascript' src='--resource-path--javascript/jquery/jquery-1.9.0/jquery-1.9.0.js'></script> \n \
        <script type='text/javascript' src='--resource-path--javascript/highcharts/highcharts-4.0.4/js/highcharts.js'></script> \n \
        <script type='text/javascript' src='--resource-path--javascript/highcharts/highcharts-4.0.4/js/modules/exporting.js'></script> \n \
        <script type='text/javascript'> \n \
            var charts = {}; \n \
            $(document).ready(function () { \n \
                --charts--   \n\n \
                $('.chart-section').each(function() { $(this).find('.title-setter').val(charts[$(this).find('.highchart-container').first().attr('id')].title.text); }); \n \
                $('.title-setter').keyup(function(){ charts[$(this).parents('.chart-section').find('.highchart-container').first().attr('id')].setTitle({text: $( this ).val()}); }); \n \
                $('.show-chart-options a').click(function(event) { event.preventDefault(); $(this).parents('.options').find('.chart-options').show().end().find('.show-chart-options').hide(); }); \n \
                $('.hide-chart-options a').click(function(event) { event.preventDefault(); $(this).parents('.options').find('.chart-options').hide().end().find('.show-chart-options').show(); }); \n \
            }); \n \
        </script> \n \
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

const char * GiniChartGenerator::TEMPLATE_CHARTHEADER = "\n \
                var chart_0 = new Highcharts.Chart({ \n \
                    chart: { renderTo: 'chart_0', type: 'line', zoomType:'xy', resetZoomButton: {relativeTo: 'chart', position: {x: -80, y: 10}, theme: {fill: 'white',stroke: 'silver',r: 0,states: {hover: {fill: '#41739D', style: { color: 'white' } } } } }, marginBottom: 80, borderColor: '#888888', plotBackgroundColor: '#e6e7e3', borderRadius: 10, borderWidth: 4, marginRight: 20 }, \n \
                    title: { text: 'Gini coefficient at Spatial Window Stops', align: 'center' }, \n \
                    subtitle: { text: 'Coefficients based on clusters with p<--gini-pvalue--.' }, \n \
                    exporting: {filename: 'gini_graph'}, \n \
                    legend: { backgroundColor: '#F5F5F5' }, \n \
                    tooltip: { crosshairs: true }, \n \
                    xAxis: { categories: [--categories--], tickmarkPlacement: 'on', labels: { formatter: function() { return this.value +'%'} } }, \n \
                    yAxis: { title: { enabled: true, text: 'Gini coefficient', style: { fontWeight: 'normal' } }, min: 0 }, \n \
                    navigation: { buttonOptions: { align: 'right' } }, \n \
                    series: [ { type: 'line', name: 'Gini coefficient', color: '#4572A7', marker: { enabled: true, symbol: 'circle', radius: 6 }, data: [--gini-data--]}, ] \n \
                }); \n \
                charts['chart_0'] = chart_0;";

const char * GiniChartGenerator::TEMPLATE_CHARTSECTION = "\
         <div style=\"margin:20px;\" class=\"chart-section\"> \n \
            <div id=\"chart_0\" class=\"highchart-container\" style=\"margin-top:0px;\"></div> \n \
            <div class=\"options\"> \n \
                <div class=\"show-chart-options\"><a href=\"#\">Show Chart Options</a></div> \n \
                <div class=\"chart-options\"> \n \
                    <div class=\"options-table\"> \n \
                      <h4>Chart Options</h4> \n \
                      <div class=\"options-row\"> \n \
                          <label for=\"title_obs\">Title</label> \n \
                          <div><input type=\"text\" style=\"width:95%;\" class=\"title-setter\" id=\"title_obs\"> \n \
                              <p class=\"help-block\">The graph title can be updated simply by editing this text.</p> \n \
                          </div> \n \
                      </div> \n \
                    </div> \n \
                    <div class=\"hide-chart-options\"><a href=\"#\">Close Chart Options</a></div> \n \
                </div> \n \
            </div> \n \
         </div> \n";

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

        //std::stringstream html, categories, gini_data;
        
        std::stringstream html, chart_js, categories, gini_data;

        // read template into stringstream
        html << BASE_TEMPLATE << std::endl;
        // replace page title
        templateReplace(html, "--title--", "Gini coefficient");
        // replace specialized body
        templateReplace(html, "--body--", TEMPLATE_BODY);
        // site resource link path
        templateReplace(html, "--resource-path--", AppToolkit::getToolkit().GetWebSite());

        chart_js << TEMPLATE_CHARTHEADER;

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
            if (maximizedCollection == &(*itrMLC)) {
                gini_data << "{y: " << buffer.c_str() << ", name: 'Optimal Gini coefficient', marker: {symbol: 'circle', radius: 6, fillColor: 'red', states: {hover: {fillColor: 'red', radius: 8}}} }";
            } else {
                gini_data << buffer.c_str();
            }
        }

        // replace categories in template
        templateReplace(chart_js, "--categories--", categories.str());
        // replace gini data in template
        templateReplace(chart_js, "--gini-data--", gini_data.str());
        // replace gini p-value cutoff
        templateReplace(chart_js, "--gini-pvalue--", getValueAsString(_dataHub.GetParameters().getGiniIndexPValueCutoff(), buffer));
        templateReplace(html, "--charts--", chart_js.str());
        templateReplace(html, "--main-content--", TEMPLATE_CHARTSECTION);

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
