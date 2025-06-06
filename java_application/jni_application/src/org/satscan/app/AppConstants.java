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
    public static final int VERSION_MAJOR = 10;
    public static final int VERSION_MINOR = 3;
    public static final int VERSION_RELEASE = 2;
    public static final String VERSION_PHASE = "";
    
    public static String DEFAULT_PVALUE_CUTOFF = "0.05";
    public static String DEFAULT_RECURRENCE_CUTOFF = "365"; // in days
    
    public static String DEFAULT_PVALUE_CUTOFF_CSV_LINELIST = "1";
    public static String DEFAULT_RECURRENCE_CUTOFF_CSV_LINELIST = "1"; // in days    
    
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
