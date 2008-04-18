/*
 * Copyright (C) 2007 Information Management Services, Inc.
 * Created on July 2, 2007
 */
package org.satscan.app;

/**
 * 
 */
public final class AppConstants {

    private final static String APP_TITLE = "SaTScan - Software for the Spatial and Space-Time Scan Statistic";
    public static final int MIN_YEAR = 1753;
    public static final int MAX_YEAR = 9999;

    public static final String getSoftwareTitle() {
        return APP_TITLE;
    }

    native static public final String getVersion();

    native static public final String getWebSite();

    native static public final String getSubstantiveSupportEmail();

    native static public final String getTechnicalSupportEmail();

    native static public final String getReleaseDate();

    native static public final String getVersionId();
}
