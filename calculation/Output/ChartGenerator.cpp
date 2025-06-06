//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "ChartGenerator.h"
#include "SaTScanData.h"
#include "cluster.h"
#include "SimulationVariables.h"
#include "Toolkit.h"
#include "BatchedLikelihoodCalculation.h"
#include <boost/regex.hpp>

/** ------------------- AbstractChartGenerator --------------------------------*/
const char * AbstractChartGenerator::HTML_FILE_EXT = ".html";

const char * AbstractChartGenerator::TEMPLATE_BODY = "\n \
        <body style=\"margin:0;background-color: #fff;\"> \n \
		<div id=\"load_error\" style=\"color:#101010; text-align: center;font-size: 1.2em; padding: 20px;background-color: #ece1e1; border: 1px solid #e49595; display:none;\"></div> \n \
	    <div style=\"position: relative;\"> \n \
	        <div class=\"search-and-account\" title=\"Choose which graphs to display.\"> \n \
                <a href=\"#\" data-toggle=\"modal\" data-target=\"#graphs-modal-modal\" class=\"offscreen\"> \n \
                    <span class=\"screen-reader-text\">Search</span> \n \
                        <svg width=\"40\" height=\"40\" fill=\"red\"> \n \
                            <image xlink:href=\"--resource-path--images/graph-choose.svg\" src=\"--resource-path--images/graph-choose.png\" width=\"40\" height=\"40\"/> \n \
                        </svg> \n \
                </a> \n \
            </div> \n \
        </div> \n \
        <div class=\"modal fade\" id=\"graphs-modal-modal\" data-backdrop=\"static\" data-keyboard=\"false\"> \n \
            <div class=\"modal-dialog modal-sm\"> \n \
                <div class=\"modal-content\"> \n \
                    <div class=\"modal-body\"> \n \
                        <div style=\"display: table;\"> \n \
                            <div class=\"btn-group\" style=\"display: table-cell;\"> \n \
                                <label style=\"font-size: 16px;\">Choose which graphs to display:</label> \n \
                                <select id=\"graph-checkbox-list\" multiple=\"multiple\"> \n \
                                --graph-list-options-- \n \
                                </select> \n \
                            </div> \n \
                           <div class=\"btn-group-vertical\" style=\"display: table-cell; padding-left: 10px;\"> \n \
                               <button id=\"id_apply_graphs\" class=\"btn btn-primary export-submit btn-sm\">Apply</button> \n \
                               <button id=\"id_cancel_modal\" type=\"button\" class=\"btn btn-warning btn-sm\" data-dismiss=\"modal\">Cancel</button> \n \
                           </div> \n \
                       </div> \n \
                       <div class=\"progress\"> \n \
                           <div class=\"progress-bar\" role=\"progressbar\" aria-valuenow=\"0\" aria-valuemin=\"0\" aria-valuemax=\"100\" style=\"width:0%\"> \n \
                               <span class=\"sr-only\">0 % Complete</span> \n \
                           </div> \n \
                       </div> \n \
                    </div> \n \
                </div> \n \
            </div> \n \
        </div> \n \
        --main-content-- \n";

/** ------------------- TemporalChartGenerator --------------------------------*/

const char * TemporalChartGenerator::FILE_SUFFIX_EXT = ".temporal";
const int TemporalChartGenerator::MAX_INTERVALS = 4000;
const int TemporalChartGenerator::MAX_X_AXIS_TICKS = 500;

const char * TemporalChartGenerator::BASE_TEMPLATE = " \
<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\" \"http://www.w3.org/TR/html4/strict.dtd\"> \n \
<html lang=\"en\"> \n \
    <head> \n \
        <title>--title--</title> \n \
        <meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\"> \n \
        <link rel=\"stylesheet\" href=\"--resource-path--javascript/highcharts/highcharts-9.1.2/code/css/highcharts.css\" type=\"text/css\">\n \
        <link rel='stylesheet' href='--resource-path--javascript/bootstrap/3.3.7/bootstrap.min.css'> \n \
        <link rel='stylesheet' href='--resource-path--javascript/bootstrap/bootstrap-multiselect/bootstrap-multiselect.css'> \n \
        <link rel='stylesheet' href='--resource-path--javascript/clustercharts/chartgenerator-1.0.css'> \n \
        <script type='text/javascript' src='--resource-path--javascript/jquery/jquery-3.1.1.min.js'></script> \n \
        <script type='text/javascript' src='--resource-path--javascript/highcharts/highcharts-9.1.2/code/highcharts.js'></script> \n \
        <script type='text/javascript' src='--resource-path--javascript/highcharts/highcharts-9.1.2/code/modules/exporting.js'></script> \n \
        <script type='text/javascript' src='--resource-path--javascript/highcharts/highcharts-9.1.2/code/modules/offline-exporting.js'></script> \n \
        <script type='text/javascript' src='--resource-path--javascript/bootstrap/3.3.7/bootstrap.min.js'></script> \n \
        <script type='text/javascript' src='--resource-path--javascript/bootstrap/bootstrap-multiselect/bootstrap-multiselect.js'></script> \n \
        <script type='text/javascript'> \n \
            function get_extended_export_buttons(menu_text, menu_function){ \n \
	            var defaultButtons = Highcharts.getOptions().exporting.buttons; // get default Highchart Export buttons \n \
                var extendedButtons = $.extend(true, {}, defaultButtons); \n \
                extendedButtons.contextButton.menuItems.unshift({ text: menu_text, onclick : menu_function }); \n \
                return extendedButtons; \n \
			} \n \
			function showChartOptions() { \n \
			    var section = $(this.renderTo).parents('div.chart-section'); \n \
			    $(section).find('div.show-chart-options a').trigger('click'); \n \
			    $(section).find('div.options-table')[0].scrollIntoView(); \n \
			} \n \
            var charts = {}; \n \
            $(document).ready(function () { \n \
                try { \n \
                --charts--   \n\n \
                $('.chart-section').each(function() { $(this).find('.title-setter').val(charts[$(this).find('.highchart-container').first().attr('id')].title.textStr); }); \n \
                $('.title-setter').keyup(function(){\n \
                    var chart_id = $(this).parents('.chart-section').find('.highchart-container').first().attr('id'); \n \
                    charts[chart_id].setTitle({text: $( this ).val()}); \n \
                    $('option[value=\"' + chart_id +  '\"]').text($( this ).val()); \n \
                    $('#graph-checkbox-list').multiselect('rebuild'); \n \
                }); \n \
                $('.show-chart-options a').click(function(event) { event.preventDefault(); $(this).parents('.options').find('.chart-options').show().end().find('.show-chart-options').hide(); }); \n \
                $('.hide-chart-options a').click(function(event) { event.preventDefault(); $(this).parents('.options').find('.chart-options').hide().end().find('.show-chart-options').show(); }); \n \
                $('.options-row input[type=\"radio\"]').click(function(event) { \n \
                    var series_type = $(this).attr('series-type'); \n \
                    var chart = charts[$(this).parents('.chart-section').find('.highchart-container').first().attr('id')]; \n \
                    $.each($(this).attr('series-id').split(','), function(index, value) { chart.get(value).update({type:series_type}, true); }); \n \
                }); \n \
                $('.options-row input[type=\"checkbox\"]').click(function(event) { \n \
                    var chart = charts[$(this).parents('.chart-section').find('.highchart-container').first().attr('id')]; \n \
                    if (chart.credits !== undefined) { \n \
                        if ($(this).is(':checked')) { \n \
                            chart.xAxis[0].addPlotBand({color: '#FFB3B3', from: $(this).attr('start-idx'), to: $(this).attr('end-idx'), id: 'band', zindex:0}); \n \
                            chart.xAxis[0].addPlotLine({id:'start',color: '#FF0000', dashStyle: 'longdashdot', width: 1, value: $(this).attr('start-idx'), zIndex: 0 }); \n \
                            chart.xAxis[0].addPlotLine({id:'end',color: '#FF0000', dashStyle: 'longdashdot', width: 1, value: $(this).attr('end-idx'), zIndex: 0 }); \n \
                        } else { \n \
                            chart.xAxis[0].removePlotBand('band' ); \n \
                            chart.xAxis[0].removePlotLine('start'); \n \
                            chart.xAxis[0].removePlotLine('end'); \n \
                        } \n \
                    } \n \
                }); \n \
                $.each($('.options-row input.series-toggle[type=\"checkbox\"]'), function(index, checkbox) { seriesToggle(checkbox); }); \n \
                $('.options-row input.series-toggle[type=\"checkbox\"]').click(function(event) { seriesToggle($(this)); }); \n \
                $('.options-row input.show-cluster-band[type=\"checkbox\"]').trigger('click'); \n \
                $('#graph-checkbox-list').multiselect({ numberDisplayed: 1, enableFiltering : true, includeSelectAllOption : true, maxHeight : 300, buttonWidth : '100%', dropRight : true }); \n \
                    if (Object.keys(charts).length) setTimeout(() => charts[Object.keys(charts)[0]].reflow(), 1000); \n \
                     var renderCharts = []; \n \
                     var reflowCharts = []; \n \
                     var timerID; \n \
                     function renderNextChart() { \n \
                         if (renderCharts.length) { \n \
                             var chart_id = renderCharts.shift(); \n \
                             var chart_section = $('div.highchart-container#' + chart_id).parent('div.chart-section'); \n \
                             if (charts[chart_id].credits === undefined) { \n \
                                 charts[chart_id] = new Highcharts.Chart(charts[chart_id]); \n \
                                 $.each($(chart_section).find('.options-row input.series-toggle[type=\"checkbox\"]'), function(index, checkbox) { seriesToggle(checkbox); }); \n \
                                 $(chart_section).find('.options-row input.show-cluster-band[type=\"checkbox\"]').prop('checked', false).trigger('click'); \n \
                                 $(chart_section).find('.title-setter').val(charts[chart_id].title.textStr); \n \
                             } \n \
                             chart_section.parent().show(); \n \
                             reflowCharts.push(chart_id); \n \
                             var increase = parseInt($('.progress-bar').attr('aria-valuenow')) + 1; \n \
                             var percentage = Math.round((increase / parseInt($('.progress-bar').attr('aria-valuemax'))) * 100); \n \
                             $('.progress-bar').css('width', percentage + '%').attr('aria-valuenow', increase); \n \
                             clearInterval(timerID); \n \
                             if (renderCharts.length) { \n \
                                 timerID = setTimeout(function() { renderNextChart() }, 10); \n \
		                     } else { \n \
                                 setTimeout(() => { \n \
                                     $.each(reflowCharts, function(index, chart_id) { charts[chart_id].reflow(); }); \n \
                                     reflowCharts = []; \n \
                                 }, 1000); \n \
                                 $('#graphs-modal-modal').modal('hide'); \n \
                                 $('.modal-footer button').removeClass('disabled').prop('disabled', false); \n \
                                 $('.progress-bar').attr('aria-valuenow', 0).css(\"width\", \"0%\"); \n \
                                 $('.progress').hide(); \n \
                            } \n \
                         } \n \
                     } \n \
                     $('button#id_apply_graphs').on('click', function(e) { \n \
                         $('.modal-footer button').addClass('disabled').prop('disabled', true); \n \
                         $('.progress').show(); \n \
                         $('.progress-bar').attr('aria-valuenow', 0).css(\"width\", \"0%\").attr('aria-valuemax', $('select#graph-checkbox-list option:selected').length); \n \
                         renderCharts = []; \n \
                         $.each($('select#graph-checkbox-list option'), function(index, checkbox) { \n \
                             var chart_id = $(checkbox).val(); \n \
                             if ($(checkbox).is(':checked')) { \n \
                                 renderCharts.push(chart_id); \n \
                             } else { \n \
                                 $('div.highchart-container#' + chart_id).parent('div.chart-section').parent().hide(); \n \
                             } \n \
                        }); \n \
                        timerID = setTimeout(function() { renderNextChart() }, 10); \n \
                     }); \n \
                     renderCharts = ['chart_1_1']; \n \
                     timerID = setTimeout(function() { renderNextChart() }, 10); \n \
                } catch (error) { \n \
                   $('#load_error').html('There was a problem loading the graph. Please <a href=\"mailto:--tech-support-email--?Subject=Graph%20Error\" target=\"_top\">email</a> technical support and attach the file:<br/>' + window.location.href.replace('file:///', '').replace(/%20/g, ' ') ).show(); \n \
                   throw( error ); \n \
                } \n \
            }); \n \
        </script> \n \
    </head> \n \
    <body> \n \
        <!--[if lt IE 9]> \n \
        <div id=\"ie\" style=\"z-index:255;border-top:5px solid #fff;border-bottom:5px solid #fff;background-color:#c00; color:#fff;\"><div class=\"iewrap\" style=\"border-top:5px solid #e57373;border-bottom:5px solid #e57373;\"><div class=\"iehead\" style=\"margin: 14px 14px;font-size: 20px;\">Notice to Internet Explorer users!</div><div class=\"iebody\" style=\"font-size: 14px;line-height: 14px;margin: 14px 28px;\">It appears that you are using Internet Explorer, <strong>this page may not display correctly with versions 8 or earlier of this browser</strong>.<br /><br /> \n \
            <i>This page is known to display correctly with the following browsers: Safari 4+, Firefox 3+, Opera 10+ and Google Chrome 5+.</i> \n \
        </div></div></div> \n \
        <![endif]--> \
        --body--<div style=\"font-style:italic;margin-left:20px;font-size:14px;\">Generated with --satscan-version--</div> \n \
    </body> \n \
</html> \n";

const char * TemporalChartGenerator::TEMPLATE_CHARTHEADER = "\n \
                var --container-id-- = { \n \
                    chart: { height: 400, renderTo: '--container-id--', zoomType:'xy', panning: true, panKey: 'shift', resetZoomButton: {relativeTo: 'chart', position: {x: -80, y: 10}, theme: {fill: 'white',stroke: 'silver',r: 0,states: {hover: {fill: '#41739D', style: { color: 'white' } } } } }, marginBottom: --margin-bottom--, borderColor: '#888888', plotBackgroundColor: '#e6e7e3', borderRadius: 0, borderWidth: 1, marginRight: --margin-right-- }, \n \
                    title: { text: '--chart-title--', align: 'center' }, \n \
                    exporting: {fallbackToExportServer: false, filename: 'cluster_graph', chartOptions: { plotOptions: { series: { showInLegend: true } } }, buttons: get_extended_export_buttons('Chart Options', showChartOptions)}, \n \
                    plotOptions: { column: { grouping: true, stacking: 'normal' }}, \n \
                    responsive: { rules: [{ condition: {  maxWidth: null }, chartOptions: { chart: { height: 400 }, subtitle: { text: null }, navigator: { enabled: false } } }] }, \n \
                    tooltip: { crosshairs: true, shared: true, formatter: function(){var is_cluster = false;var has_observed = false;$.each(this.points, function(i, point) {if (point.series.options.id == 'cluster') {is_cluster = true;}if (point.series.options.id == 'obs') {has_observed = true;}});var s = '<b>'+ this.x +'</b>'; if (is_cluster) {s+= '<br/><b>Cluster Point</b>';}$.each(this.points,function(i, point){if (point.series.options.id == 'cluster'){if (!has_observed) {s += '<br/>Observed: '+ point.y;}} else {s += '<br/>'+ point.series.name +': '+ point.y;}});return s;}, }, \n \
                    legend: { enabled: true, backgroundColor: '#F5F5F5', verticalAlign: 'top', y: 40 }, \n \
                    xAxis: [{ categories: [--categories--], tickmarkPlacement: 'on', labels: { step: --step--, rotation: -45, align: 'right' }, tickInterval: --tickinterval-- }], \n \
                    yAxis: [{ reversedStacks: false, title: { enabled: true, text: 'Number of Cases', style: { fontWeight: 'normal' } }, min: 0, showEmpty: false }--additional-yaxis--], \n \
                    navigation: { buttonOptions: { align: 'right' } }, \n \
                    series: [--series--]\n \
                }; \n \
                charts['--container-id--'] = --container-id--;";

const char * TemporalChartGenerator::TEMPLATE_CHARTSECTION = "\
	        <div class=\"item\" style=\"display:none;\"><div style=\"margin:20px;\" class=\"chart-section\"> \n \
            <div id=\"--container-id--\" class=\"highchart-container\" style=\"margin-top:0px;\"></div> \n \
            <div class=\"options\"> \n \
                <div class=\"show-chart-options\"><a href=\"#\">Show Chart Options</a></div> \n \
                <div class=\"chart-options\"> \n \
                    <div class=\"options-table\"> \n \
                        <div class=\"row\"> \n \
                        <div class=\"col-md-6\"> \n \
                        <h4 style = \"text-align:left;margin-bottom:5px;\">Chart Options</h4> \n \
                          <div class=\"options-row\"> \n \
                              <label for=\"title_obs\">Title</label> \n \
                              <div><input type=\"text\" style=\"width:95%;\" class=\"title-setter\" id=\"title_obs\"> \n \
                                  <p class=\"help-block\">Title can be changed by editing this text.</p> \n \
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
                                  <input type=\"checkbox\" class=\"show-cluster-band\" name=\"--container-id--_cluster_band\" start-idx=\"--cluster-start-idx--\" end-idx=\"--cluster-end-idx--\" />Show Cluster Band \n \
                                </label> \n \
                                <p class=\"help-block\">Band stretching across the plot area marking cluster interval.</p> \n \
                              </div> \n \
                          </div> \n \
                          <div class=\"options-row\">To zoom a portion of the chart, select and drag mouse within the chart. Hold down shift key to pan zoomed chart.</div> \n \
                        </div> \n \
                        --cluster-details-- \n \
                        </div> \n \
                    </div> \n \
                    <div class=\"hide-chart-options\"><a href=\"#\">Close Chart Options</a></div> \n \
                </div> \n \
            </div> \n \
         </div></div> \n";

const char* TemporalChartGenerator::TEMPLATE_CLUSTERDETAILS = "\n \
            <div class=\"col-md-6\"> \n \
              <h4 style = \"text-align:left; margin-bottom:5px;\">2 x 2 Table</h4> \n \
              <div class=\"row\" style=\"margin-left:10px;\"> \n \
                <table class=\"table table-condensed cluster-details\" cellpadding=\"3\" cellspacing=\"0\"> \n \
                  <thead> \n \
                    <tr> \n \
                      <th style=\"border-bottom: none;\"></th> \n \
                      <th colspan=\"2\" style=\"padding-bottom:0px; text-align:left;\">Cluster Time Period</th> \n \
                    </tr> \n \
                   <tr> \n \
                     <th style=\"text-align:left; border-top:none;\">Geographical Cluster Area</th> \n \
                     <th class=\"cluster-details-sub-header\">Inside</th> \n \
                     <th class=\"cluster-details-sub-header\">Outside</th> \n \
                    </tr> \n \
                  </thead> \n \
                  <tbody> \n \
                    <tr><th class=\"cluster-details-sub-header\">Inside</th><td>--inside-inside--</td><td>--outside-inside--</td></tr> \n \
                    <tr><th class=\"cluster-details-sub-header\">Outside</th><td>--inside-outside--</td><td>--outside-outside--</td></tr> \n \
                    <tr class=\"cluster-details-percentages\"><th></th><td>--inside-percent--%</td><td>--outside-percent--%</td></tr> \n \
                  </tbody> \n \
                </table> \n \
              </div> \n \
            </div> \n";

/** constructor */
TemporalChartGenerator::TemporalChartGenerator(const CSaTScanData& dataHub, const MostLikelyClustersContainer & clusters, const SimulationVariables& simVars) 
    :_dataHub(dataHub), _clusters(clusters), _simVars(simVars){
    if (_dataHub.GetParameters().GetProbabilityModelType() == BATCHED)
        _batched_likelihood_calculator.reset(new BatchedLikelihoodCalculator(dataHub));
}

/** Creates HighCharts graph for purely temporal cluster. */
void TemporalChartGenerator::generateChart() const {
    std::string clusterName, buffer, buffer2;
    FileName fileName;
    const CParameters& parameters = _dataHub.GetParameters();

    try {
        fileName.setFullPath(parameters.GetOutputFileName().c_str());
        getFilename(fileName);

        std::ofstream HTMLout;
        //open output file
        HTMLout.open(fileName.getFullPath(buffer).c_str());
        if (!HTMLout) throw resolvable_error("Error: Could not open file '%s'.\n", fileName.getFullPath(buffer).c_str());
        if (!_clusters.GetNumClustersRetained()) {
            HTMLout.close();
            return;
        }

        std::stringstream html, charts_javascript, cluster_sections, chart_select_options;

        // read template into stringstream
        html << BASE_TEMPLATE << std::endl;
        // replace page title
        templateReplace(html, "--title--", "Cluster Temporal Graph");
        // replace specialized body
        templateReplace(html, "--body--", TEMPLATE_BODY);
        // site resource link path
        templateReplace(html, "--resource-path--", AppToolkit::getToolkit().GetWebSite());
        // site resource link path
        templateReplace(html, "--tech-support-email--", AppToolkit::getToolkit().GetTechnicalSupportEmail());

        // set margin bottom according to time precision
        int margin_bottom=130;
        switch (parameters.GetPrecisionOfTimesType()) {
            case YEAR : margin_bottom = 90; break;
            case MONTH : margin_bottom = 110; break;
            case DAY:
            case GENERIC:
            default: margin_bottom=130;
        }

        // We might need to calculate purely temporal data structures.
        // TODO: Is there a better way to do this?
        const DataSetHandler& handler = _dataHub.GetDataSetHandler();
        if (parameters.GetAnalysisType() != PURELYTEMPORAL) {
            DataSetHandler& temp_handler = const_cast<DataSetHandler&>(handler);
            for (size_t idx=0; idx < temp_handler.GetNumDataSets(); ++idx) {
                temp_handler.GetDataSet(idx).setCaseData_PT();
                temp_handler.GetDataSet(idx).setMeasureData_PT();
            }
        }

        // Determine clusters will have a graph generated based on settings.
        std::vector<const CCluster*> graphClusters;
        switch (parameters.getTemporalGraphReportType()) {
            case MLC_ONLY :
                graphClusters.push_back(&_clusters.GetCluster(0)); break;
            case X_MCL_ONLY :
                for (int i = 0; i < parameters.getTemporalGraphMostLikelyCount() && i < _clusters.GetNumClustersRetained(); ++i) {
                    if (i == 0 || (_clusters.GetCluster(i).m_nRatio >= MIN_CLUSTER_LLR_REPORT && _clusters.GetCluster(i).GetRank() <= _simVars.get_sim_count()))
                        graphClusters.push_back(&_clusters.GetCluster(i));
                }
                break;
            case SIGNIFICANT_ONLY :
                for (int i=0; i < _clusters.GetNumClustersRetained(); ++i) {
                    const CCluster & cluster = _clusters.GetCluster(i);
                    if (cluster.m_nRatio < MIN_CLUSTER_LLR_REPORT || cluster.GetRank() > _simVars.get_sim_count()) continue;
                    if (parameters.GetIsProspectiveAnalysis() && !_dataHub.isDrilldown()) {
                        if (cluster.reportableRecurrenceInterval(parameters, _simVars) && // round RI to whole days
                            std::round(cluster.GetRecurrenceInterval(_dataHub, i + 1, _simVars).second) >= _dataHub.GetParameters().getTemporalGraphSignificantCutoff())
                            graphClusters.push_back(&cluster);
                    } else if (cluster.getReportingPValue(_dataHub.GetParameters(), _simVars, i == 0) <= _dataHub.GetParameters().getTemporalGraphSignificantCutoff()) {
                        graphClusters.push_back(&cluster);
                    }
                }
                break;
        }

        for (size_t clusterIdx=0; clusterIdx < graphClusters.size(); ++clusterIdx) {
            const CCluster& cluster = *graphClusters[clusterIdx];
            // This graph is only valid for temporal clusters.
            if (!(cluster.GetClusterType() == PURELYTEMPORALCLUSTER || cluster.GetClusterType() == SPACETIMECLUSTER || cluster.GetClusterType() == SPATIALVARTEMPTRENDCLUSTER))
                continue;
            // calculate the graphs interval groups for this cluster
            intervalGroups groups = getIntervalGroups(cluster);
            for (size_t setIdx=0; setIdx < handler.GetNumDataSets(); ++setIdx) {
                std::stringstream chart_js, chart_series, chart_section, categories, cluster_details;
                // set the chart header for this cluster
                chart_js << TEMPLATE_CHARTHEADER;

                bool is_pt(cluster.GetClusterType() == PURELYTEMPORALCLUSTER); // not if cluster is purely temporal
                // define seach series that we'll graph - next three are always printed.
                std::auto_ptr<ChartSeries> observedSeries(new ChartSeries("obs", 1, "column", (is_pt ? "Observed" : "Observed outside cluster area"), "8BB8EB", "square", 0, "observed"));
                // the remaining series are conditionally present in the chart
                std::auto_ptr<ChartSeries> observedClusterSeries, expectedClusterSeries, odeSeries, cluster_odeSeries, clusterSeries, expectedSeries;

				// expectedSeries.reset(new ChartSeries("exp", is_pt ? 3 : 2, "line", (is_pt ? "Expected" : "Expected (Outside Cluster Area)"), "89A54E", "triangle", 0, ""));
				// clusterSeries.reset(new ChartSeries("cluster", is_pt ? 2 : 5, "column", "Cluster", "AA4643", "circle", 0, ""));

                // space-time clusters also graph series which allow comparison between inside and outside the cluster
                if (!is_pt) {
                    observedClusterSeries.reset(new ChartSeries("cluster_obs", 3, "column", "Observed in cluster area (red=cluster)", "003264", "square", 0, "observed"));
                    expectedClusterSeries.reset(new ChartSeries("cluster_exp", 4, "line", "Expected in cluster area", "394521", "triangle", 0, ""));
                }

                // Poisson, Exponential, Space Time Permutation and batch models also graph observed / expected
                if (parameters.GetProbabilityModelType() == POISSON || parameters.GetProbabilityModelType() == EXPONENTIAL ||
                    parameters.GetProbabilityModelType() == SPACETIMEPERMUTATION || parameters.GetProbabilityModelType() == BATCHED) {
                    // graphing observed / expected, with y-axis along right side
                    templateReplace(chart_js, "--additional-yaxis--", ", { title: { enabled: true, text: 'Observed / expected', style: { fontWeight: 'normal' } }, min: 0, opposite: true, showEmpty: false }");
					if (is_pt)
						odeSeries.reset(new ChartSeries("obs_exp", 2, "line", (is_pt ? "Observed / expected" : "Observed / expected outside cluster area"), "00FF00", "triangle", 1, ""));
					else // space-time clusters also graph series which allow comparison between inside and outside the cluster
                        cluster_odeSeries.reset(new ChartSeries("cluster_obs_exp", 2, "line", "Observed / expected in cluster area", "FF8000", "triangle", 1, ""));
                } else if (parameters.GetProbabilityModelType() == BERNOULLI) {
                    // the Bernoulli model also graphs cases / (cases + controls)
                    // graphing cases ratio, with y-axis along right side
                    templateReplace(chart_js, "--additional-yaxis--", ", { title: { enabled: true, text: 'Cases ratio', style: { fontWeight: 'normal' } }, max: 1, min: 0, opposite: true, showEmpty: false }");
					if (is_pt)
						odeSeries.reset(new ChartSeries("case_ratio", 2, "line", (is_pt ? "Cases ratio" : "Cases ratio outside cluster area"), "00FF00", "triangle", 1, ""));
					else // space-time clusters also graph series which allow comparison between inside and outside the cluster
                        cluster_odeSeries.reset(new ChartSeries("cluster_case_ratio", 2, "line", "Cases ratio in cluster area", "FF8000", "triangle", 1, ""));
                } else {
                    templateReplace(chart_js, "--additional-yaxis--", "");
                }
                
                // set default chart title 
                if (parameters.GetAnalysisType() == PURELYTEMPORAL)
					clusterName = "Detected Cluster";
                else 
                    printString(clusterName, "Cluster #%u", clusterIdx + 1);
                if (handler.GetNumDataSets() > 1) { // include data set name with multiple sets
                    clusterName += printString(buffer2, ": %s",
                        parameters.getDataSourceNames()[_dataHub.GetDataSetHandler().getDataSetRelativeIndex(setIdx)].c_str()
                    );
                }

                templateReplace(chart_js, "--chart-title--", clusterName);
                templateReplace(chart_js, "--margin-bottom--", printString(buffer, "%d", margin_bottom));
                templateReplace(chart_js, "--margin-right--", printString(buffer, "%d", (odeSeries.get() || cluster_odeSeries.get() ? 80 : 20)));

                // increase x-axis 'step' if there are many intervals, so that labels are not crowded
                //  -- empirically, 50 ticks seems like a good upper limit
                templateReplace(chart_js, "--step--", printString(buffer, "%u", static_cast<int>(std::ceil(static_cast<double>(groups.getGroups().size())/50.0))));

                // get series datastreams plus cluster indexes start and end ticks
                std::pair<int,int> cluster_grp_idx = getSeriesStreams(
                    cluster, groups, setIdx, categories, clusterSeries.get(), *observedSeries, expectedSeries.get(), 
                    observedClusterSeries.get(), expectedClusterSeries.get(), odeSeries.get(), cluster_odeSeries.get()
                );

                // define the identifying attribute of this chart
                printString(buffer, "chart_%d_%u", clusterIdx + 1, setIdx + 1);
				// add select option for this chart
				chart_select_options << "<option value=\"" << buffer.c_str() << "\" " << (clusterIdx == 0 ? "selected=selected" : "") << ">" << clusterName.c_str() << "</option>" << std::endl;
                templateReplace(chart_js, "--container-id--", buffer);
				printString(buffer, "%u", static_cast<unsigned int>(std::ceil( static_cast<double>(groups.getGroups().size()) / static_cast<double>(MAX_X_AXIS_TICKS) )));
				templateReplace(chart_js, "--tickinterval--", buffer);
                templateReplace(chart_js, "--categories--", categories.str());

                // replace the series
				// if (clusterSeries.get()) chart_series << (chart_series.rdbuf()->in_avail() ? "," : "") << clusterSeries->toString(buffer).c_str();
                if (observedClusterSeries.get())
                    chart_series << (chart_series.rdbuf()->in_avail() ? "," : "") << observedClusterSeries->toString(buffer).c_str();
                if (expectedClusterSeries.get())
                    chart_series << (chart_series.rdbuf()->in_avail() ? "," : "") << expectedClusterSeries->toString(buffer).c_str();
                //if (expectedSeries.get()) chart_series << (chart_series.rdbuf()->in_avail() ? "," : "") << expectedSeries->toString(buffer).c_str();
                if (cluster_odeSeries.get())
                    chart_series << (chart_series.rdbuf()->in_avail() ? "," : "") << cluster_odeSeries->toString(buffer).c_str();
                chart_series << (chart_series.rdbuf()->in_avail() ? "," : "") << observedSeries->toString(buffer).c_str();
                if (odeSeries.get())
                    chart_series << (chart_series.rdbuf()->in_avail() ? "," : "") << odeSeries->toString(buffer).c_str();
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

                // create cluster details table for space time permutation analysis
                cluster_details << TEMPLATE_CLUSTERDETAILS;
                if (_dataHub.GetParameters().GetProbabilityModelType() == SPACETIMEPERMUTATION) {
                    ClusterCaseTotals_t caseTotals = getClusterCaseTotals(cluster, setIdx);
                    printString(buffer, "%d", caseTotals.get<0>());
                    templateReplace(cluster_details, "--inside-inside--", buffer);
                    printString(buffer, "%d", caseTotals.get<1>());
                    templateReplace(cluster_details, "--outside-inside--", buffer);
                    printString(buffer, "%d", caseTotals.get<2>());
                    templateReplace(cluster_details, "--inside-outside--", buffer);
                    printString(buffer, "%d", caseTotals.get<3>());
                    templateReplace(cluster_details, "--outside-outside--", buffer);
                    printString(buffer, "%.1f", static_cast<double>(caseTotals.get<0>()) / static_cast<double>(caseTotals.get<0>() + caseTotals.get<2>()) * 100.0);
                    templateReplace(cluster_details, "--inside-percent--", buffer);
                    printString(buffer, "%.1f", static_cast<double>(caseTotals.get<1>()) / static_cast<double>(caseTotals.get<1>() + caseTotals.get<3>()) * 100.0);
                    templateReplace(cluster_details, "--outside-percent--", buffer);
                    templateReplace(chart_section, "--cluster-details--", cluster_details.str());
                } else {
                    templateReplace(chart_section, "--cluster-details--", "");
                }
                               
                // add section to collection of sections
                cluster_sections << chart_section.str() << std::endl << std::endl;
            }
        }

        templateReplace(html, "--charts--", charts_javascript.str());
		templateReplace(html, "--graph-list-options--", chart_select_options.str());
        if (graphClusters.size()) {
            templateReplace(html, "--main-content--", cluster_sections.str());
        } else {
            if (_dataHub.GetParameters().GetIsProspectiveAnalysis() && !_dataHub.isDrilldown())
                printString(buffer2, "<h3 style=\"text-align:center;\">No clusters to graph. All clusters had a recurrence interval less than %.0lf.</h3>", _dataHub.GetParameters().getTemporalGraphSignificantCutoff());
            else
                printString(buffer2, "<h3 style=\"text-align:center;\">No clusters to graph. All clusters had a p-value greater than %g.</h3>", _dataHub.GetParameters().getTemporalGraphSignificantCutoff());
            templateReplace(html, "--main-content--", buffer2.c_str());
        }
		printString(buffer,
			"SaTScan v%s.%s%s%s%s%s",
			VERSION_MAJOR,
			VERSION_MINOR,
			(!strcmp(VERSION_RELEASE, "0") ? "" : "."),
			(!strcmp(VERSION_RELEASE, "0") ? "" : VERSION_RELEASE),
			(strlen(VERSION_PHASE) ? " " : ""),
			VERSION_PHASE
		);
		templateReplace(html, "--satscan-version--", buffer.c_str());
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
                                                             std::stringstream& categories, ChartSeries * clusterSeries,
                                                             ChartSeries& observedSeries, ChartSeries * expectedSeries,
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
        // calculate the expected and observed for this interval
        measure_t expected=0, cluster_expected=0;
        count_t observed=0, cluster_observed=0;
        if (_batched_likelihood_calculator.get())
            expected += _batched_likelihood_calculator->getExpectedInWindow(itrGrp->first, itrGrp->second, dataSetIdx);
        for (int i=itrGrp->first; i < itrGrp->second; ++i) {
            if (!_batched_likelihood_calculator.get())
                expected += (i == intervals - 1 ? pmeasure[i] : pmeasure[i] - pmeasure[i + 1]);
            observed += (i == intervals - 1 ? pcases[i] : pcases[i] - pcases[i+1]);
        }
        // if not purely temporal cluster, we're expressing this as outside verse inside cluster
        if (cluster_observedSeries || cluster_expectedSeries) {
            // calculate cluster observed and expected series across entire period, not just cluster window
            std::vector<tract_t> indexes;
            cluster.getIdentifierIndexes(_dataHub, indexes, true);
            if (_batched_likelihood_calculator.get())
                cluster_expected += _batched_likelihood_calculator->getExpectedInWindow(itrGrp->first, itrGrp->second, indexes, dataSetIdx);
            for (auto t : indexes) {
                for (int i = itrGrp->first; i < itrGrp->second; ++i) {
                    cluster_observed += (i == intervals - 1 ? ppcases[i][t] : ppcases[i][t] - ppcases[i + 1][t]);
                    if (!_batched_likelihood_calculator.get())
                        cluster_expected += (i == intervals - 1 ? ppmeasure[i][t] : ppmeasure[i][t] - ppmeasure[i + 1][t]);
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
            // For the Poisson/Exponential/Batched models, this represents the ratio of observed / expected inside the cluster.
            getValueAsString(expected ? static_cast<measure_t>(observed)/expected : 0, buffer, 2);
            odeSeries->datastream() << (itrGrp==groups.getGroups().begin() ? "" : ",") <<  buffer.c_str();
        }
        // apply measure adjustment now
        expected *= adjustment;
        cluster_expected *= adjustment;
        // put totals to other streams
		if (expectedSeries)
			expectedSeries->datastream() << (itrGrp==groups.getGroups().begin() ? "" : ",") << getValueAsString(expected, buffer, 2).c_str();
        observedSeries.datastream() <<  (itrGrp==groups.getGroups().begin() ? "" : ",") << observed;
        if (cluster_expectedSeries)
            cluster_expectedSeries->datastream() << (itrGrp==groups.getGroups().begin() ? "" : ",") << getValueAsString(cluster_expected, buffer, 2).c_str();
		if (cluster_observedSeries) {
			cluster_observedSeries->datastream() << (itrGrp == groups.getGroups().begin() ? "" : ",");
			if (cluster.m_nFirstInterval <= itrGrp->first && itrGrp->second <= cluster.m_nLastInterval) {
				cluster_observedSeries->datastream() << "{y:" << cluster_observed << "," << "color:'#BF0B23'}";
			} else
				cluster_observedSeries->datastream() << cluster_observed;
		}
        if (cluster.m_nFirstInterval <= itrGrp->first && itrGrp->second <= cluster.m_nLastInterval) {
            groupClusterIdx.first = std::min(groupClusterIdx.first, itrGrp->first);
            groupClusterIdx.second = std::max(groupClusterIdx.second, itrGrp->second) - 1;
			if (clusterSeries)
				clusterSeries->datastream() <<  (itrGrp==groups.getGroups().begin() ? "" : ",") << (cluster.GetClusterType() == PURELYTEMPORALCLUSTER ? observed : cluster_observed);
        } else {
			if (clusterSeries)
				clusterSeries->datastream() << (itrGrp==groups.getGroups().begin() ? "" : ",") << "null";
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

/** Reports number of cases in respect to cluster window and geographical area:
      Inside Cluster Window, Inside Cluster Area
      Outside Cluster Window, Inside Cluster Area
      Inside Cluster Window, Outside Cluster Area
      Outside Cluster Window, Outside Cluster Area */
TemporalChartGenerator::ClusterCaseTotals_t TemporalChartGenerator::getClusterCaseTotals(const CCluster& cluster, size_t dataSetIdx) const {
    count_t* pcases = _dataHub.GetDataSetHandler().GetDataSet(dataSetIdx).getCaseData_PT();
    count_t** ppcases = _dataHub.GetDataSetHandler().GetDataSet(dataSetIdx).getCaseData().GetArray();
    ClusterCaseTotals_t caseTotals(0, 0, 0, 0);

    std::vector<tract_t> indexes;
    cluster.getIdentifierIndexes(_dataHub, indexes, true);

    // count_t totalCases = _dataHub.GetTotalDataSetCases(dataSetIdx);
    int intervals = _dataHub.GetNumTimeIntervals();

    for (int i = 0; i < intervals; ++i) {
        count_t intervalTotalCases = (i == intervals - 1 ? pcases[i] : pcases[i] - pcases[i + 1]);
        count_t intervalInClusterArea = 0;
        for (auto t : indexes)
            intervalInClusterArea += (i == intervals - 1 ? ppcases[i][t] : ppcases[i][t] - ppcases[i + 1][t]);
        if (cluster.m_nFirstInterval <= i && i < cluster.m_nLastInterval) {
            caseTotals.get<0>() += intervalInClusterArea;
            caseTotals.get<2>() += (intervalTotalCases - intervalInClusterArea);
        } else {
            caseTotals.get<1>() += intervalInClusterArea;
            caseTotals.get<3>() += (intervalTotalCases - intervalInClusterArea);
        }
            
    }

    return caseTotals;
}

/** ------------------- GiniChartGenerator --------------------------------*/

const char * GiniChartGenerator::FILE_SUFFIX_EXT = "_gini";

const char * GiniChartGenerator::BASE_TEMPLATE = " \
<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01//EN\" \"http://www.w3.org/TR/html4/strict.dtd\"> \n \
<html lang=\"en\"> \n \
    <head> \n \
        <title>--title--</title> \n \
        <meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\"> \n \
        <link rel=\"stylesheet\" href=\"--resource-path--javascript/highcharts/highcharts-6.0.3/code/css/highcharts.css\" type=\"text/css\">\n \
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
        <script type='text/javascript' src='--resource-path--javascript/highcharts/highcharts-6.0.3/code/js/highcharts.js'></script> \n \
        <script type='text/javascript' src='--resource-path--javascript/highcharts/highcharts-6.0.3/code/js/modules/exporting.js'></script> \n \
        <script type='text/javascript'> \n \
            var charts = {}; \n \
            $(document).ready(function () { \n \
                --charts--   \n\n \
                $('.chart-section').each(function() { $(this).find('.title-setter').val(charts[$(this).find('.highchart-container').first().attr('id')].title.textStr); }); \n \
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
                              <p class=\"help-block\">Title can be changed by editing this text.</p> \n \
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
            getValueAsString(gini, buffer, 4);
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
