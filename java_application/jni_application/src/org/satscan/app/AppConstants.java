/*
 * Copyright (C) 2007 Information Management Services, Inc.
 * Created on July 2, 2007
 */
package org.satscan.app;

import java.util.Enumeration;
import java.util.HashMap;
import java.util.Properties;

/**
 * 
 */
public final class AppConstants {

    private final static String APP_TITLE = "SaTScan - Software for the Spatial and Space-Time Scan Statistic";
    public static final int MIN_YEAR = 1753;
    public static final int MAX_YEAR = 9999;
    public static final int MIN_GENERIC = -219145;
    public static final int MAX_GENERIC = 2921938;
    public static final int VERSION_MAJOR = 9;
    public static final int VERSION_MINOR = 7;
    public static final int VERSION_RELEASE = 0;
    public static final String VERSION_PHASE = "Alpha 1";
    
    // Input Tab
    public static final String CASEFILE_HELPID = "Case File";
    public static final String CONTROLFILE_HELPID = "Control File";
    public static final String TIMEPRECISION_HELPID = "Time Precision";
    public static final String STUDYPERIOD_HELPID = "Study Period";
    public static final String POPULTIONFILE_HELPID = "Population File";
    public static final String COORDINATESFILE_HELPID = "Coordinates File";
    public static final String GRIDFILE_HELPID = "Grid File";
    public static final String COORDINATES_HELPID = "Coordinates";
    // Advanced Input Tab - Multiple Data Sets
    public static final String MULTIPLEDATASETS_HELPID = "Multiple Data Sets Tab";
    // Advanced Input Tab - Data Checking
    public static final String TEMPORALDATACHECK_HELPID = "Temporal Data Check";
    public static final String GEOGRAPHICALDATACHECK_HELPID = "Geographical Data Check";
    // Advanced Input Tab - Spatial Neighbors
    public static final String NONEUCLIDIANFILE_HELPID = "Non-Euclidian Neighbors File";
    public static final String METALOCATIONSFILE_HELPID = "Meta Location File";
    public static final String MULTIPLECOORDLOC_HELPID = "Multiple Coordinates per Location";
    // Analysis Tab
    public static final String TYPEANALYSIS_HELPID = "Type of Analysis";
    public static final String PROBABILITYMODEL_HELPID = "Probability Model";
    public static final String SCANAREA_HELPID = "Scan for High or Low Rates";
    public static final String TIMEAGGREGATION_HELPID = "Time Aggregation";
    // Advanced Analysis Tab - Spatial Window
    public static final String MAXIMUMSPATIAL_HELPID = "Maximum Spatial Cluster Size";
    public static final String SCANNINGWINDOW_HELPID = "Elliptic Scanning Window";
    public static final String MAXCIRCLEFILE_HELPID = "Max Circle Size File";
    // Advanced Analysis Tab - Temporal Window
    public static final String MAXIMUMTEMPORAL_HELPID = "Maximum Temporal Cluster Size";
    public static final String MINIMUMTEMPORAL_HELPID = "Minimum Temporal Cluster Size";
    public static final String FLEXIBLEWINDOW_HELPID = "Flexible Temporal Window Definition";
    // Advanced Analysis Tab - Space and Time Adjustments
    public static final String TEMPORALTRENDADJ_HELPID = "Temporal Trend Adjustment";
    public static final String SPATIALADJ_HELPID = "Spatial Adjustment";
    public static final String ADJUSTMENT_RELRISKS_HELPID = "Adjustment with Known Relative Risks";
    public static final String ADJUSTMENTSFILE_HELPID = "Adjustments File";
    // Advanced Analysis Tab - Inference
    public static final String PVALUE_HELPID = "P-Value";
    public static final String MONTECARLO_HELPID = "Inference Tab";
    public static final String ITERATIVESCAN_HELPID = "Iterative Scan Statistic";
    // Advanced Analysis Tab - Cluster Restrictions
    public static final String RESTRICTMINCASES_HELPID = "Minimum Number of Cases";
    public static final String RESTRICTRELRISK_HELPID = "Boscoe’s Limit of Clusters by Risk Level";
    // Advanced Analysis Tab - Border Analysis
    public static final String OLIVERIERAF_HELPID = "Oliveira’s F";
    // Advanced Analysis Tab - Power Evaluation
    public static final String POWERESTIMATION_HELPID = "Power Estimation Tab";
    public static final String ALTERNATIVEHYPOTHESIS_HELPID = "Alternative Hypothesis File";
    // Output Tab
    public static final String TEXTOUTPUT_HELPID = "Text Output Format";
    public static final String GEOGRAPHICALOUTPUT_HELPID = "Geographical Output Format";
    public static final String COLUMNOUTPUT_HELPID = "Column Output Format";
    // Advanced Output Tab - Spatial Output
    public static final String KMLGOOGLEEARTH_HELPID = "KML Geographical Output File (*.kml)";
    public static final String SECONDARDY_CLUSTERS_HELPID = "Criteria for Reporting Secondary Clusters";
    public static final String MAXIMIMCLUSTERSREPORTED_HELPID = "Maximum Reported Spatial Cluster Size";
    public static final String HTMLCARTESIANMAP_HELPID = "HTML Cartesian Map Output File (*.html)";
    // Advanced Output Tab - Temporal Output
    public static final String TEMPORALGRAPHS_HELPID = "Temporal Graphs";
    // Advanced Output Tab - Other Output
    public static final String CRITICALVALUES_HELPID = "Critical Values";
    public static final String MONTECARLORANK_HELPID = "Monte Carlo Rank";
    public static final String COLUMNHEADERS_HELPID = "Column Headers";
    public static final String RUNTITLE_HELPID = "Title for Results File";
    
    public static final String FILENAME_ASCII_ERROR = "The filename %s is invalid.\nIt may contain only standard ASCII characters in the path and filename.";
    
    public static final String getSoftwareTitle() {
        return APP_TITLE;
    }

    static public final String getGraphicalVersion() {
        StringBuilder version = new StringBuilder();
        version.append(VERSION_MAJOR).append(".").append(VERSION_MINOR);
        if (VERSION_RELEASE > 0) version.append(".").append(VERSION_RELEASE);
        if (VERSION_PHASE.length() > 0) version.append(" ").append(VERSION_PHASE);
        return version.toString();
    }
    
    static public final HashMap getEnviromentVariables() {
        HashMap variables = new HashMap();
        variables.put("SaTScan Application Version", getGraphicalVersion());
        variables.put("SaTScan Engine Version", getVersion()); 
        variables.put("SaTScan Version Id", getVersionId()); 
        Properties sysprops = System.getProperties();
        for ( Enumeration e = sysprops.propertyNames(); e.hasMoreElements(); ) {
            String key = (String)e.nextElement();
            variables.put(key, sysprops.getProperty( key )); 
        } 
        return variables;
    }

    native static public final String getVersion();

    native static public final String getWebSite();

    native static public final String getSubstantiveSupportEmail();

    native static public final String getTechnicalSupportEmail();

    native static public final String getReleaseDate();

    native static public final String getVersionId();
}
