package org.satscan.gui;

import java.awt.CardLayout;
import java.awt.Color;
import java.awt.Component;
import java.awt.Container;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.beans.PropertyVetoException;
import java.io.File;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Calendar;
import java.util.GregorianCalendar;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import javax.swing.ImageIcon;
import javax.swing.JOptionPane;
import javax.swing.JRootPane;
import javax.swing.event.InternalFrameEvent;
import javax.swing.event.InternalFrameListener;
import javax.swing.event.UndoableEditEvent;
import javax.swing.event.UndoableEditListener;
import javax.swing.undo.UndoManager;
import java.util.concurrent.TimeUnit;
import javax.swing.event.DocumentEvent;
import javax.swing.event.DocumentListener;
import org.satscan.app.AdvFeaturesExpection;
import org.satscan.app.AppConstants;
import org.satscan.utils.FileAccess;
import org.satscan.app.ParameterHistory;
import org.satscan.app.Parameters;
import org.satscan.app.RegionFeaturesException;
import org.satscan.app.UnknownEnumException;
import org.satscan.gui.utils.DateComponentsGroup;
import org.satscan.gui.utils.FileSelectionDialog;
import org.satscan.gui.utils.InputFileFilter;
import org.satscan.gui.utils.JHyperLink;
import org.satscan.gui.utils.Utils;
import org.satscan.importer.CSVImportDataSource;
import org.satscan.importer.DBaseImportDataSource;
import org.satscan.importer.ImportDataSource;
import org.satscan.importer.InputSourceSettings;
import org.satscan.importer.ShapefileDataSource;
import org.satscan.importer.XLSImportDataSource;

/**
 * Parameter settings window.
 * @author  Hostovic
 */
public class ParameterSettingsFrame extends javax.swing.JInternalFrame implements InternalFrameListener {

    private Parameters _parameters = new Parameters();
    private Parameters _initialParameters = new Parameters();
    private AdvancedParameterSettingsFrame _advancedParametersSetting = null;
    private OberservableRegionsFrame _oberservableRegionsFrame = null;
    private boolean gbPromptOnExist = true;
    private final UndoManager undo = new UndoManager();
    private final JRootPane _rootPane;
    final static String STUDY_COMPLETE = "study_complete";
    final static String STUDY_GENERIC = "study_generic";
    private DateComponentsGroup _startDateComponentsGroup;
    private DateComponentsGroup _endDateComponentsGroup;
    Map _input_source_map = new HashMap();
    private BatchAnalysisFrame _multipleAnaylsisFrame = null;

    /**
     * Creates new form ParameterSettingsFrame
     */
    public ParameterSettingsFrame(final JRootPane rootPane, final String sParameterFilename) {
        initComponents();
        _startDateComponentsGroup = new DateComponentsGroup(new UndoManager(),_studyPeriodStartDateYearTextField,_studyPeriodStartDateMonthTextField,_studyPeriodStartDateDayTextField, 2000, 1, 1, false);
        _endDateComponentsGroup = new DateComponentsGroup(new UndoManager(),_studyPeriodEndDateYearTextField,_studyPeriodEndDateMonthTextField,_studyPeriodEndDateDayTextField, 2000, 1, 1, true);
        setFrameIcon(new ImageIcon(getClass().getResource("/SaTScan.png")));
        _rootPane = rootPane;
        addInternalFrameListener(this);
        setUp(sParameterFilename);
        pack();
    }

    /**
     * Creates new form ParameterSettingsFrame
     */
    public ParameterSettingsFrame(final JRootPane rootPane, BatchAnalysisFrame batchFrame, final Parameters source_parameters) {
        initComponents();
        _startDateComponentsGroup = new DateComponentsGroup(new UndoManager(),_studyPeriodStartDateYearTextField,_studyPeriodStartDateMonthTextField,_studyPeriodStartDateDayTextField, 2000, 1, 1, false);
        _endDateComponentsGroup = new DateComponentsGroup(new UndoManager(),_studyPeriodEndDateYearTextField,_studyPeriodEndDateMonthTextField,_studyPeriodEndDateDayTextField, 2000, 1, 1, true);
        setFrameIcon(new ImageIcon(getClass().getResource("/SaTScan.png")));
        _rootPane = rootPane;
        _multipleAnaylsisFrame = batchFrame;
        addInternalFrameListener(this);
        defaultHiddenParameters();
        // Clone parameters -- we'll copy any updates when closing window.
        _parameters = (Parameters)source_parameters.clone();
        setupInterface(_parameters);
        saveParameterSettings(_parameters);
        _initialParameters = (Parameters) _parameters.clone();
        setTitle("Batch Analysis Settings");
        pack();
    }    
    
     /** Returns final reference to Parameters object. */
    public final Parameters getParameters() {
        return _parameters;
    }
    
    /**
     * Returns whether the associated AdvancedParameterSettingsFrame object is visible.
     */
    public boolean isAdvancedParametersWindowVisible() {
        return getAdvancedParameterInternalFrame().isVisible();
    }

    /**
     * Set the associated AdvancedParameterSettingsFrame object to not visible.
     */
    public void setAdvancedParametersWindowVisible(boolean b) {
        getAdvancedParameterInternalFrame().setVisible(b, null);
    }

    /* Returns whther this parameter settings window is associated with multiple analysis window. */
    public boolean isMultipleAnalysisSettings() {
        return _multipleAnaylsisFrame != null;
    }
    
    /**
     * Returns whether the associated AdvancedParameterSettingsFrame object is visible.
     */
    public boolean isObservableRegionsParametersWindowVisible() {
        return getAdvancedParameterInternalFrame().isVisible();
    }

    /**
     * Set the associated AdvancedParameterSettingsFrame object to not visible.
     */
    public void setObservableRegionsParametersWindowVisible(boolean b) {
        getAdvancedParameterInternalFrame().setVisible(b, null);
    }

    /**
     * Returns reference to associated observable regions parameters frame.
     */
    private OberservableRegionsFrame geObservableRegionsParameterInternalFrame() {
        return _oberservableRegionsFrame;
    }

    /* Launches 'save as' dialog to permit saving current settings to parameter file. */
    public boolean saveAs() {
        boolean bSaved = true;
        List<InputFileFilter> filters = new ArrayList<InputFileFilter>();
        filters.add(new InputFileFilter("prm", "Settings Files (*.prm)"));
        FileSelectionDialog select = new FileSelectionDialog(org.satscan.gui.SaTScanApplication.getInstance(), "Save Parameters to File", filters, org.satscan.gui.SaTScanApplication.getInstance().lastBrowseDirectory);
        File file = select.browse_saveas();
        if (file != null) {
            org.satscan.gui.SaTScanApplication.getInstance().lastBrowseDirectory = select.getDirectory();
            String filename = file.getAbsolutePath();
            if (new File(filename).getName().lastIndexOf('.') == -1){
                filename = filename + ".prm";
            }
            // Test write access to ensure that file exists and user has write permission.
            if (!FileAccess.ValidateFileAccess(filename, true, false)) {
                JOptionPane.showInternalMessageDialog(this, "Unable to save session parameters.\n" + "Please confirm that the path and/or file name\n" + "are valid and that you have permissions to write\nto this directory and file.");
                return false;
            }
            if (isMultipleAnalysisSettings()) {
                // If this action is being done froma multiple analsis settings window, just write the parameters to specified file.
                Parameters _writeParameters = (Parameters) _parameters.clone();
                _writeParameters.SetSourceFileName(filename);
                _writeParameters.Write(filename);
            } else {
                writeSession(filename);
                setTitle(filename);                
            }
        } else {
            bSaved = false;
        }
        return bSaved;
    }

    /**
     * Writes the session information to disk
     */
    public boolean writeSession(String sParameterFilename) {
        String sFilename = sParameterFilename;
        boolean bSaved = true;

        if (sFilename.equals("")) {
            sFilename = _parameters.GetSourceFileName();
        }
        if (sFilename == null || sFilename.equals("")) {
            bSaved = saveAs();
        } else {
            if (!FileAccess.ValidateFileAccess(sFilename, true, false)) {
                JOptionPane.showInternalMessageDialog(this, "Unable to save session parameters.\n" + "Please confirm that the path and/or file name\n" + "are valid and that you have permissions to write\nto this directory and file.");
            } else {
                saveParameterSettings(_parameters);
                _parameters.SetSourceFileName(sFilename);
                _parameters.Write(sFilename);
                _initialParameters = (Parameters) _parameters.clone();
            }
        }
        return bSaved;
    }

    /**
     * Writes session settings to parameter files */
    public void writeSettingsToParameters() {
        saveParameterSettings(_parameters);
        _initialParameters = (Parameters) _parameters.clone();
    }    
    
    /**
     * Returns reference to associated advanced parameters frame.
     */
    public AdvancedParameterSettingsFrame getAdvancedParameterInternalFrame() {
        return _advancedParametersSetting;
    }

    /* If necessary, removes from from iconized state and brings to front. */
    public void focusWindow() {
        if (isIcon()) {
            try {
                setIcon(false);
            } catch (PropertyVetoException e) { return; }
        }
        toFront();
    }
    
    /** Determines whether window can be closed by comparing parameter settings contained in window verse initial parameter settings. */
    public boolean queryWindowCanClose() {
        boolean bReturn = true;

        saveParameterSettings(_parameters);
        if (!_parameters.equals(_initialParameters)) {
            if (getAdvancedParameterInternalFrame().isVisible()) {
                getAdvancedParameterInternalFrame().setVisible(false, null);
            //return false;
            }
            focusWindow();
            switch (JOptionPane.showInternalConfirmDialog(this, "Parameter settings have changed. Do you want to save?", "Save?", JOptionPane.YES_NO_CANCEL_OPTION)) {
                case JOptionPane.YES_OPTION:
                    if (_multipleAnaylsisFrame != null) {
                        writeSettingsToParameters();
                        gbPromptOnExist = false;
                    } else if (writeSession("")) {
                        gbPromptOnExist = false;
                    } else {
                        bReturn = false;
                    }
                    break;
                case JOptionPane.NO_OPTION:
                    // If this is a settings window opened from batch frame, reset parameters to match initial settings.
                    if (_multipleAnaylsisFrame != null)
                        _parameters = (Parameters)_initialParameters.clone();
                    break;
                case JOptionPane.CANCEL_OPTION:
                    bReturn = false;
                    break;
                default:
                    gbPromptOnExist = false;
            }
        }
        return bReturn;
    }

    public boolean requestsGeographicalOutput() {
        return getReportingGoogleEarthKML() || getReportingGoogleMap() || getReportingCartesianGraph() || getReportingShapefile();
    }
    
    public boolean anyLaunchableSelections() {
        return getReportingGoogleEarthKML() || getReportingGoogleMap() || getReportingCartesianGraph();
    }
    
    public boolean getReportingGoogleEarthKML() {
        return _reportGoogleEarthKML.isEnabled() && _reportGoogleEarthKML.isSelected();
    }

    public boolean getReportingGoogleMap() {
        return _reportGoogleMap.isEnabled() && _reportGoogleMap.isSelected();
    }    
    
    public boolean getReportingCartesianGraph() {
        return _reportCartesianGraph.isEnabled() && _reportCartesianGraph.isSelected();
    }    

    public boolean getReportingShapefile() {
        return _reportShapefile.isEnabled() && _reportShapefile.isSelected();
    }        
    
    /**
     * sets precision of times type control for DatePrecisionType
     */
    public void setPrecisionOfTimesControl(Parameters.DatePrecisionType eDatePrecisionType) {
        switch (eDatePrecisionType) {
            case YEAR:
                _timePrecisionYear.setSelected(true);
                break;
            case MONTH:
                _timePrecisionMonth.setSelected(true);
                break;
            case DAY:
                _timePrecisionDay.setSelected(true);
                break;
            case GENERIC : 
                _timePrecisionGeneric.setSelected(true);    
                break;
            case NONE:                
            default:
                _timePrecisionNone.setSelected(true);
        }
    }

    /**
     * sets precision of times type control for DatePrecisionType
     */
    public void setCoordinatesType(Parameters.CoordinatesType eCoordinatesType) {
        switch (eCoordinatesType) {
            case CARTESIAN:
                _cartesianRadioButton.setSelected(true);
                break;
            case LATLON:
            default:
                _latLongRadioButton.setSelected(true);
        }
    }

    /**
     * sets probaiity model type control for ProbabilityModelType
     */
    public void setModelControl(Parameters.ProbabilityModelType eProbabilityModelType) {
        switch (eProbabilityModelType) {
            case BERNOULLI:
                if (_bernoulliModelRadioButton.isEnabled()) {
                    _bernoulliModelRadioButton.setSelected(true);
                } else {
                    _poissonModelRadioButton.setSelected(true);
                }
                break;
            case SPACETIMEPERMUTATION:
                if (_spaceTimePermutationModelRadioButton.isEnabled()) {
                    _spaceTimePermutationModelRadioButton.setSelected(true);
                } else {
                    _poissonModelRadioButton.setSelected(true);
                }
                break;
            case ORDINAL:
                if (_ordinalModelRadioButton.isEnabled()) {
                    _ordinalModelRadioButton.setSelected(true);
                } else {
                    _poissonModelRadioButton.setSelected(true);
                }
                break;
            case EXPONENTIAL:
                if (_exponentialModelRadioButton.isEnabled()) {
                    _exponentialModelRadioButton.setSelected(true);
                } else {
                    _poissonModelRadioButton.setSelected(true);
                }
                break;
            case NORMAL:
                if (_normalModelRadioButton.isEnabled()) {
                    _normalModelRadioButton.setSelected(true);
                } else {
                    _poissonModelRadioButton.setSelected(true);
                }
                break;
            case CATEGORICAL:
                if (_categoricallModelRadioButton.isEnabled()) {
                    _categoricallModelRadioButton.setSelected(true);
                } else {
                    _poissonModelRadioButton.setSelected(true);
                }
                break;
            case HOMOGENEOUSPOISSON:
                if (_homogeneouspoissonModelRadioButton.isEnabled()) {
                    _homogeneouspoissonModelRadioButton.setSelected(true);
                } else {
                    _poissonModelRadioButton.setSelected(true);
                }
                break;
            case UNIFORMTIME:
                if (_uniformtimeModelRadioButton.isEnabled()) {
                    _uniformtimeModelRadioButton.setSelected(true);
                } else {
                    _poissonModelRadioButton.setSelected(true);
                }
                break;
            case BATCHED:
                if (_batchedModelRadioButton.isEnabled()) {
                    _batchedModelRadioButton.setSelected(true);
                } else {
                    _poissonModelRadioButton.setSelected(true);
                }
                break;
            case POISSON:
            default:
                _poissonModelRadioButton.setSelected(true);
        }
        enableAnalysisControlForModelType();
    }

    /** 
     * Enables/disables area scan rate controls.
     */
    private void setEnableAreaScanRateControl() {
        Boolean bEnable = getModelControlType() != Parameters.ProbabilityModelType.CATEGORICAL;
        Boolean bEnableLowRates = getModelControlType() != Parameters.ProbabilityModelType.HOMOGENEOUSPOISSON;
        Boolean bEnableBothRates = getModelControlType() != Parameters.ProbabilityModelType.HOMOGENEOUSPOISSON;

        _scanAreasGroup.setEnabled(bEnable);
        _lowRatesRadioButton.setEnabled(bEnable && bEnableLowRates);
        _highRatesRadioButton.setEnabled(bEnable);
        _highOrLowRatesRadioButton.setEnabled(bEnable && bEnableBothRates);
        if (bEnable && ((bEnableLowRates == false && _lowRatesRadioButton.isSelected()) ||
                        (bEnableBothRates == false && _highOrLowRatesRadioButton.isSelected()))) {
            _highRatesRadioButton.setSelected(true);
        }    
    }
    
    /**
     * sets area scan rate type control
     */
    private void setAreaScanRateControl(Parameters.AreaRateType eAreaRateType) {
        
        switch (eAreaRateType) {
            case LOW:
                _lowRatesRadioButton.setSelected(true);
                break;
            case HIGHANDLOW:
                _highOrLowRatesRadioButton.setSelected(true);
                break;
            case HIGH:
            default:
                _highRatesRadioButton.setSelected(true);
        }
    }

    /**
     * enables correct advanced settings button on Analysis and Output tabs
     */
    public void enableAdvancedButtons() {
        // Input tab Advanced button
        if (!getAdvancedParameterInternalFrame().getDefaultsSetForInputOptions()) {
            _advancedInputButton.setFont(new java.awt.Font("Dialog", java.awt.Font.BOLD, 12));
        } else {
            _advancedInputButton.setFont(new java.awt.Font("Dialog", java.awt.Font.PLAIN, 12));
        }
        // Analysis tab Advanced button
        if (!getAdvancedParameterInternalFrame().getDefaultsSetForAnalysisOptions()) {
            _advancedAnalysisButton.setFont(new java.awt.Font("Dialog", java.awt.Font.BOLD, 12));
        } else {
            _advancedAnalysisButton.setFont(new java.awt.Font("Dialog", java.awt.Font.PLAIN, 12));
        }
        // Output tab Advanced button
        if (!getAdvancedParameterInternalFrame().getDefaultsSetForOutputOptions()) {
            _advancedFeaturesOutputButton.setFont(new java.awt.Font("Dialog", java.awt.Font.BOLD, 12));
        } else {
            _advancedFeaturesOutputButton.setFont(new java.awt.Font("Dialog", java.awt.Font.PLAIN, 12));
        }
    }

    /** Resets parameters that are not present in interface to default value.
     * Hidden features are to be used solely in command line version at this time. */
    private void defaultHiddenParameters() {
        //non-parametric removed from interface, replaced with time stratified
        if (_parameters.GetTimeTrendAdjustmentType() == Parameters.TimeTrendAdjustmentType.TEMPORAL_NONPARAMETRIC) {
            _parameters.SetTimeTrendAdjustmentType(Parameters.TimeTrendAdjustmentType.TEMPORAL_STRATIFIED_RANDOMIZATION.ordinal());
        }
        _parameters.SetTimeTrendConvergence(.0000001); //default value in CParameters
        _parameters.SetSimulationType(Parameters.SimulationType.STANDARD.ordinal());
        _parameters.SetOutputSimulationData(false);
        //since 'clusters to include' feature no longer in interface, adjust settings so that analysis is equivalent
        if (_parameters.GetAnalysisType() == Parameters.AnalysisType.PURELYTEMPORAL && _parameters.GetIncludeClustersType() == Parameters.IncludeClustersType.ALIVECLUSTERS) {
            _parameters.SetAnalysisType(Parameters.AnalysisType.PROSPECTIVEPURELYTEMPORAL.ordinal());
            _parameters.SetAdjustForEarlierAnalyses(false);
        }
        if (_parameters.GetAnalysisType() == Parameters.AnalysisType.SPACETIME && _parameters.GetIncludeClustersType() == Parameters.IncludeClustersType.ALIVECLUSTERS) {
            _parameters.SetAnalysisType(Parameters.AnalysisType.PROSPECTIVESPACETIME.ordinal());
            _parameters.SetAdjustForEarlierAnalyses(false);
        }
        //if still ALIVECLUSTERS, default to ALLCLUSTERS
        if (_parameters.GetIncludeClustersType() == Parameters.IncludeClustersType.ALIVECLUSTERS) {
            _parameters.SetIncludeClustersType(Parameters.IncludeClustersType.ALLCLUSTERS.ordinal());
        }
        //prevent stratified temporal and spatial adjustments from being set as same time
        if (_parameters.GetTimeTrendAdjustmentType() == Parameters.TimeTrendAdjustmentType.TEMPORAL_STRATIFIED_RANDOMIZATION &&
                _parameters.GetSpatialAdjustmentType() == Parameters.SpatialAdjustmentType.SPATIAL_STRATIFIED_RANDOMIZATION) {
            _parameters.SetSpatialAdjustmentType(Parameters.SpatialAdjustmentType.SPATIAL_NONPARAMETRIC.ordinal());
        }
        _parameters.SetExecutionType(Parameters.ExecutionType.AUTOMATIC.ordinal());
        //before version 6, critical values were always reported
        if (_parameters.GetCreationVersion().giMajor < 6) {
            _parameters.SetReportCriticalValues(true);
        }
        //If parameter file was created with version 4 of SaTScan, use time interval
        //units as specifier for date precision. This was the behavior in v4 but it
        //was decided to revert to time precision units. Note that for a purely spatial
        //analysis, we have no way of knowing what the time precision should be; settings
        //to YEAR is safe since it is permittable to have more precise dates.
        if (_parameters.GetCreationVersion().giMajor == 4) {
            _parameters.SetPrecisionOfTimesType(_parameters.GetAnalysisType() == Parameters.AnalysisType.PURELYSPATIAL ? Parameters.DatePrecisionType.YEAR.ordinal() : _parameters.GetTimeAggregationUnitsType().ordinal());
        }
        //Default elliptic shapes and rotations.
        _parameters.AddEllipsoidShape(1.5, true);
        _parameters.AddEllipsoidRotations(4, true);
        _parameters.AddEllipsoidShape(2, false);
        _parameters.AddEllipsoidRotations(6, false);
        _parameters.AddEllipsoidShape(3, false);
        _parameters.AddEllipsoidRotations(9, false);
        _parameters.AddEllipsoidShape(4, false);
        _parameters.AddEllipsoidRotations(12, false);
        _parameters.AddEllipsoidShape(5, false);
        _parameters.AddEllipsoidRotations(15, false);        
        //quadratic time trend for svtt not exposed in GUI
        if (_parameters.getTimeTrendType() == Parameters.TimeTrendType.QUADRATIC) {
            _parameters.SetTimeTrendType(Parameters.TimeTrendType.LINEAR.ordinal());
        }
        // specifying the critical values for power evaluations is not implemented in gui
        if (_parameters.getPowerEvaluationCriticalValueType() == Parameters.CriticalValuesType.CV_POWER_VALUES) {
            _parameters.setPowerEvaluationCriticalValueType(Parameters.CriticalValuesType.CV_MONTECARLO.ordinal());
        }        
    }

    /**
     * Reads parameter settings from file and loads frames controls.
     */
    private void setUp(final String sParameterFileName) {
        if (sParameterFileName.length() > 0) {
            _parameters.Read(sParameterFileName);
            
            if (_parameters.GetAdjustForEarlierAnalyses() && 
                (_parameters.GetAnalysisType() == Parameters.AnalysisType.PROSPECTIVESPACETIME ||
                 _parameters.GetAnalysisType() == Parameters.AnalysisType.PROSPECTIVEPURELYTEMPORAL)) {
                JOptionPane.showMessageDialog(null, "The option to adjust for earlier analyses with a prospective analysis\n" +
                                                    "is no longer available and has been disabled. This feature can still\n" +
                                                    "be used but only through the command line execution.");
          }
          _parameters.SetAdjustForEarlierAnalyses(false);            
            
        }
        defaultHiddenParameters();
        setupInterface(_parameters);

        //Save orginal parameter settings to compare against when window closes but
        //first save what the interface has produced for the settings read from file.
        saveParameterSettings(_parameters);
        _initialParameters = (Parameters) _parameters.clone();
    }

    /**
     * Returns the study period start date as GregorianCalendar.
     */
    public GregorianCalendar getStudyPeriodStartDateAsCalender() {
        return getStudyPeriodEndDateAsCalender(getPrecisionOfTimesControlType(),
                                               _studyPeriodStartDateYearTextField.getText(),
                                               _studyPeriodStartDateMonthTextField.getText(),
                                               _studyPeriodStartDateDayTextField.getText(),
                                               _studyPeriodStartDateGenericTextField.getText());
    }

    /**
     * Creates GregorianCalendar from date pieces given date precision.
     */
    public static GregorianCalendar getStudyPeriodEndDateAsCalender(final Parameters.DatePrecisionType eDatePrecision, final String year, final String month, final String day, final String generic) {
        GregorianCalendar theDate = new GregorianCalendar();

        if (eDatePrecision.equals(Parameters.DatePrecisionType.GENERIC)) {
          theDate.add(GregorianCalendar.DAY_OF_YEAR, Integer.parseInt(generic,10));
        } else {
          theDate.set(Calendar.YEAR, Integer.parseInt(year));
          theDate.set(Calendar.MONTH, Integer.parseInt(month) - 1);
          theDate.set(Calendar.DAY_OF_MONTH, Integer.parseInt(day));
        }
        return theDate;
    }

    /**
     * Returns the study period end date as GregorianCalendar.
     */
    public GregorianCalendar getStudyPeriodEndDateAsCalender() {
        return getStudyPeriodEndDateAsCalender(getPrecisionOfTimesControlType(),
                                               _studyPeriodEndDateYearTextField.getText(),
                                               _studyPeriodEndDateMonthTextField.getText(),
                                               _studyPeriodEndDateDayTextField.getText(),
                                               _studyPeriodEndDateGenericTextField.getText());
    }

    /** Return the ImportDataSource object -- based upon the source file type. */
    public int getNumImportSourceColumns(InputSourceSettings iss, String filename) {
        try {
            ImportDataSource source=null;
            switch (iss.getSourceDataFileType()) {
                case Shapefile : source = new ShapefileDataSource(new File(filename), true); break;
                case dBase : source = new DBaseImportDataSource(new File(filename), false); break;
                case Excel97_2003 : 
                case Excel : source = new XLSImportDataSource(new File(filename), false); break;
                case CSV :
                default : source = new CSVImportDataSource(new File(filename), iss.getFirstRowHeader(), '\n', iss.getDelimiter().charAt(0), iss.getGroup().charAt(0), iss.getSkiplines());
            }
            int num_columns = source.getColumnNames().length;
            source.close();
            return num_columns;
        } catch (Exception e) {}
        return 0;
    }    
    
    /* Validates the source data file against restrictions on source and InputSourceSettings settings. */
    public String validateInputSourceDataFile(final String filepath, String mapKey, String verbosename) {
        String actual_filepath = FileAccess.getFormatSubstitutedFilename(filepath);
        // First exclude file types that are not readable - namely, Excel97_2003;
        String extension = FileAccess.getExtension(new File(actual_filepath));
        extension = extension == null ? "" : extension.toLowerCase();
        if (extension.equals("xls") || extension.equals("xlsx")) {
            return "Excel files (.xls and  xlsx extensions) can only be read directly by SaTScan.\nYou must import this " + verbosename + " file.";
        }        
        boolean iss_exists = _input_source_map.containsKey(mapKey);
        // If file type is dBase or shapefile, then require an input source setting which defines how to read the file.
        if ((extension.equals("dbf") || extension.equals("shp")) && !iss_exists) {
            return "dBase files (.dbf) and shapefiles (.shp) require you to define how the file is read.\nPlease use the import feature for this " + verbosename + " file.";
        }
        if (iss_exists) {
            InputSourceSettings inputSourceSettings = (InputSourceSettings)_input_source_map.get(mapKey);
            // Verify that the input source settings's source data file type matches extension.
            boolean correct_filetype=true;
            InputSourceSettings.SourceDataFileType extensionType=FileSourceWizard.getSourceFileType(actual_filepath);
            switch (inputSourceSettings.getSourceDataFileType()) {
                case CSV : correct_filetype = !(extension.equals("dbf") || extension.equals("shp") || extension.equals("xls") || extension.equals("xlsx")); break;
                case dBase : correct_filetype = extension.equals("dbf"); break;
                case Shapefile : correct_filetype = extension.equals("shp"); break;
                case Excel97_2003 :
                case Excel :  correct_filetype = extension.equals("xls") || extension.equals("xlsx"); break;
                default:    throw new UnknownEnumException(inputSourceSettings.getSourceDataFileType());
            }
            if (!correct_filetype) {
                return "The import feature must be performed again on the " + verbosename + " file.\nThe current import settings indicate a " + inputSourceSettings.getSourceDataFileType().toString() + " file but the specified file is a " + extensionType.toString() + " file.";
            }
            // Verify that the mappings align with the data source available options.
            // Safely get the number of columns in datasource, if mapping references column index greater than # columns, then display error.
            if (inputSourceSettings.isSet()) {
                int num_cols = getNumImportSourceColumns(inputSourceSettings, actual_filepath);
                int max = 0;
                for (String stdIdx : inputSourceSettings.getFieldMaps()) {
                    if (!stdIdx.isEmpty()) {
                        max = Math.max(Integer.parseInt(stdIdx), max);
                    }
                }
                if (max > num_cols) {
                    return "The import feature must be performed again on the " + verbosename + " file.\nThe current import settings conflict with the file structure.";
                }                    
            }
        }
        return null;
    }
    
    /* Validates 'Input Files' tab */
    private void validateInputFiles() {        
        /* Check that the study period start date comes before the end date. */
        if (getModelControlType() != Parameters.ProbabilityModelType.HOMOGENEOUSPOISSON && 
            getStudyPeriodStartDateAsCalender().after(getStudyPeriodEndDateAsCalender())) {
            throw new SettingsException("The study period start time can not be greater than the end time.", (Component) _studyPeriodStartDateYearTextField);
        }
        /* For the Seasonal analysis with dates that are not generic, the specified study period must be at least 1 year. */
        if (getAnalysisControlType() == Parameters.AnalysisType.SEASONALTEMPORAL && 
            (getPrecisionOfTimesControlType() == Parameters.DatePrecisionType.MONTH || getPrecisionOfTimesControlType() == Parameters.DatePrecisionType.DAY)) {
            long mseconds = getStudyPeriodEndDateAsCalender().getTime().getTime() - getStudyPeriodStartDateAsCalender().getTime().getTime() + 1;
            long days = TimeUnit.DAYS.convert(mseconds, TimeUnit.MILLISECONDS) + 1;
            if (days < 365) {
                throw new SettingsException("The study period must be at least 1 year long for Seasonal analysis.", (Component) _studyPeriodStartDateYearTextField);
            }
        }
        
        //validate the case file
        if (getModelControlType() != Parameters.ProbabilityModelType.HOMOGENEOUSPOISSON && !_advancedParametersSetting.isPowerEvaluationNoCaseFile()) {
            if (_caseFileTextField.getText().length() == 0) {
                throw new SettingsException("Please specify a case file.", (Component) _caseFileTextField);
            }
            if (!FileAccess.ValidateFileAccess(_caseFileTextField.getText(), false, false)) {
                throw new SettingsException("The case file could not be opened for reading.\n" + "Please confirm that the path and/or file name\n" + "are valid and that you have permissions to read\nfrom this directory and file.",
                        (Component) _caseFileTextField);
            }
            if (!FileAccess.isValidFilename(_caseFileTextField.getText())) {                
                throw new SettingsException(String.format(AppConstants.FILENAME_ASCII_ERROR, _caseFileTextField.getText()), (Component) _caseFileTextField);
            }
            String validationString = validateInputSourceDataFile(_caseFileTextField.getText(), InputSourceSettings.InputFileType.Case.toString() + "1", "case");
            if (validationString != null) throw new SettingsException(validationString, (Component) _caseFileTextField);
        }
        //validate the control file - Bernoulli model only
        if (getModelControlType() == Parameters.ProbabilityModelType.BERNOULLI) {
            if (_controlFileTextField.getText().length() == 0) {
                throw new SettingsException("For the Bernoulli model, please specify a control file.", (Component) _controlFileTextField);
            }
            if (!FileAccess.ValidateFileAccess(_controlFileTextField.getText(), false, false)) {
                throw new SettingsException("The control file could not be opened for reading.\n" + "Please confirm that the path and/or file name are\n" + "valid and that you have permissions to read from\nthis directory and file.",
                        (Component) _controlFileTextField);
            }
            if (!FileAccess.isValidFilename(_controlFileTextField.getText())) {                
                throw new SettingsException(String.format(AppConstants.FILENAME_ASCII_ERROR, _controlFileTextField.getText()), (Component) _controlFileTextField);
            }
            String validationString = validateInputSourceDataFile(_controlFileTextField.getText(), InputSourceSettings.InputFileType.Control.toString() + "1", "control");
            if (validationString != null) throw new SettingsException(validationString, (Component) _controlFileTextField);
        }
        //validate the population file -  Poisson model only
        if (getModelControlType() == Parameters.ProbabilityModelType.POISSON) {
            if (_populationFileTextField.getText().length() == 0) {
                //for purely temporal analyses, the population file is optional
                if (!(getAnalysisControlType() == Parameters.AnalysisType.PURELYTEMPORAL || 
                      getAnalysisControlType() == Parameters.AnalysisType.PROSPECTIVEPURELYTEMPORAL ||
                      getAnalysisControlType() == Parameters.AnalysisType.SEASONALTEMPORAL)) {
                    throw new SettingsException("For the Poisson model, please specify a population file.\n" + "Note that for purely temporal analyses, if the risk does\n" + "not change over time, the population file is optional.",
                            (Component) _populationFileTextField);
                }
            } else if (!FileAccess.ValidateFileAccess(_populationFileTextField.getText(), false, false)) {
                throw new SettingsException("The population file could not be opened for reading.\n" + "Please confirm that the path and/or file name are\n" + "valid and that you have permissions to read from this\ndirectory and file.",
                        (Component) _populationFileTextField);
            }
            if (!FileAccess.isValidFilename(_populationFileTextField.getText())) {                
                throw new SettingsException(String.format(AppConstants.FILENAME_ASCII_ERROR, _populationFileTextField.getText()), (Component) _populationFileTextField);
            }            
            String validationString = validateInputSourceDataFile(_populationFileTextField.getText(), InputSourceSettings.InputFileType.Population.toString() + "1", "population");
            if (validationString != null) throw new SettingsException(validationString, (Component) _populationFileTextField);
        }
        Parameters.AnalysisType eAnalysisType = getAnalysisControlType();
        boolean bCheckCoordinatesFile = true;
        if (eAnalysisType == Parameters.AnalysisType.PURELYTEMPORAL || 
            eAnalysisType == Parameters.AnalysisType.PROSPECTIVEPURELYTEMPORAL ||
            eAnalysisType == Parameters.AnalysisType.SEASONALTEMPORAL) {
            bCheckCoordinatesFile = getAdvancedParameterInternalFrame().isAdjustedRelativeRisksSelected() &&
                                    _coordiantesFileTextField.getText().length() != 0;
        } else if (getAdvancedParameterInternalFrame().isNonEucledianNeighborsSelected()) {
            bCheckCoordinatesFile = false;
        } else if (getAdvancedParameterInternalFrame().isNetworkFileSelected() && _coordiantesFileTextField.getText().length() == 0) {
            // If user specified a network file and no geographical output files and the coordinates input is blank, don't check coordinates input.
            // The coordinates file is conditionally required based on 2 things:
            //  - user specified a network file where at least one record does not detail the distance between a connection (checked in data read)
            //  - the user requested a geographical output file, so we need the coordinates file obtain coordinates of each location
            if (requestsGeographicalOutput())
                throw new SettingsException(
                    "Geographical output file(s) have been requested in conjuction\nwith a network file yet a coordinates file has not been provided.\n" + 
                    "A coordinates file is required to place locations geograghically.\nAlternatively you can unselect the geographical output options.",
                    (Component) _coordiantesFileTextField
                );
           bCheckCoordinatesFile = false; 
        }
        //validate coordinates and grid file -- ignore validation if using neighbors file or purely temporal analysis
        if (bCheckCoordinatesFile) {
            if (_coordiantesFileTextField.getText().length() == 0) {
                throw new SettingsException("Please specify a coordinates file.", (Component) _coordiantesFileTextField);
            } else if (!FileAccess.ValidateFileAccess(_coordiantesFileTextField.getText(), false, false)) {
                throw new SettingsException("The coordinates file could not be opened for reading.\n" + "Please confirm that the path and/or file name are\n" + "valid and that you have permissions to read from this\ndirectory and file.",
                        (Component) _coordiantesFileTextField);
            }
            if (!FileAccess.isValidFilename(_coordiantesFileTextField.getText())) {                
                throw new SettingsException(String.format(AppConstants.FILENAME_ASCII_ERROR, _coordiantesFileTextField.getText()), (Component) _coordiantesFileTextField);
            }                        
            String validationString = validateInputSourceDataFile(_coordiantesFileTextField.getText(), InputSourceSettings.InputFileType.Coordinates.toString() + "1", "coordinates");
            if (validationString != null) throw new SettingsException(validationString, (Component) _coordiantesFileTextField);            
            //validate special grid file -- optional
            if (_gridFileTextField.getText().length() > 0 && !getAdvancedParameterInternalFrame().isNetworkFileSelected()) {
                if (!FileAccess.ValidateFileAccess(_gridFileTextField.getText(), false, false))
                    throw new SettingsException("The grid file could not be opened for reading.\n" + "Please confirm that the path and/or file name are\n" + "valid and that you have permissions to read from this\ndirectory and file.", (Component) _gridFileTextField);
                if (!FileAccess.isValidFilename(_gridFileTextField.getText())) {                
                    throw new SettingsException(String.format(AppConstants.FILENAME_ASCII_ERROR, _gridFileTextField.getText()), (Component) _gridFileTextField);
                }                                        
                validationString = validateInputSourceDataFile(_gridFileTextField.getText(), InputSourceSettings.InputFileType.SpecialGrid.toString() + "1", "grid");
                if (validationString != null) throw new SettingsException(validationString, (Component) _coordiantesFileTextField);            
            }
        }
    }

    /** Returns date precision as string. */
    public String getDatePrecisionAsString(Parameters.DatePrecisionType eType, boolean bPlural, boolean bCapitalizeFirstLetter) {
        String sString;
        switch (eType) {
            case YEAR:
                sString = (bCapitalizeFirstLetter ? "Y" : "y") + "ear";
                break;
            case MONTH:
                sString = (bCapitalizeFirstLetter ? "M" : "m") + "onth";
                break;
            case DAY:
                sString = (bCapitalizeFirstLetter ? "D" : "d") + "ay";
                break;
            case GENERIC:
                sString = (bCapitalizeFirstLetter ? "U" : "u") + "nit";
                break;
            default:
                sString = "none";
                break;
        }
        if (bPlural) {
            sString += "s";
        }

        return sString;
    }

    /**
     * Native method which calculates the number of time intervals given the study period and time aggregation.
     */
    native public double CalculateNumberOfTimeIntervals(final String startDateYear, final String startDateMonth, final String startDateDay,
            final String endDateYear, final String endDateMonth, final String endDateDay, int iTimeAggregationType);

    /**
     * Calculates number of time aggregation units in study period.
     */
    public double CalculateTimeAggregationUnitsInStudyPeriod() {
        if (getTimeAggregationControlType() == Parameters.DatePrecisionType.GENERIC) {
            return Math.ceil(CalculateNumberOfTimeIntervals(_studyPeriodStartDateGenericTextField.getText(), "", "",
                                                            _studyPeriodEndDateGenericTextField.getText(), "", "",
                                                            getTimeAggregationControlType().ordinal()));

        } else {
            GregorianCalendar startDate = getStudyPeriodStartDateAsCalender();
            GregorianCalendar endDate = getStudyPeriodEndDateAsCalender();

            return Math.ceil(CalculateNumberOfTimeIntervals(Integer.toString(startDate.get(GregorianCalendar.YEAR)),
                                                            Integer.toString(startDate.get(GregorianCalendar.MONTH) + 1),
                                                            Integer.toString(startDate.get(GregorianCalendar.DAY_OF_MONTH)),
                                                            Integer.toString(endDate.get(GregorianCalendar.YEAR)),
                                                            Integer.toString(endDate.get(GregorianCalendar.MONTH) + 1),
                                                            Integer.toString(endDate.get(GregorianCalendar.DAY_OF_MONTH)),
                                                            getTimeAggregationControlType().ordinal()));
        }
    }

    /**
     * Validates time interval length is not less than zero.
     */
    private void checkTimeAggregationLength() {
        String sPrecisionString;
        double dStudyPeriodLengthInUnits, dMaxTemporalLengthInUnits = 0;

        //validate that the time aggregation length agrees with study period and maximum temporal cluster size
        if (getAnalysisControlType() != Parameters.AnalysisType.PURELYSPATIAL) {
            sPrecisionString = getDatePrecisionAsString(getTimeAggregationControlType(), false, false);
            //report error if control is empty or specified time aggregation length is less than one.
            if (_timeAggregationLengthTextField.getText().length() == 0 || Integer.parseInt(_timeAggregationLengthTextField.getText()) < 1) {
                throw new SettingsException("Please specify a time aggregation length greater than zero.", (Component) _timeAggregationLengthTextField);
            }
            // check time aggregation length if using day of week adjustment, this adjustment can only be used if aggregation is 1 day
            if (_advancedParametersSetting.isAdjustingForDayOfWeek()) {
                int time_length = Integer.parseInt(_timeAggregationLengthTextField.getText());            
                if (!(time_length == 1 && getTimeAggregationControlType() == Parameters.DatePrecisionType.DAY))
                    throw new SettingsException("The adjustment for weekly trends can only be performed with a time aggregation length of 1 day.", (Component) _timeAggregationLengthTextField);            
            }
            
            dStudyPeriodLengthInUnits = CalculateTimeAggregationUnitsInStudyPeriod();
            if (dStudyPeriodLengthInUnits < Double.parseDouble(_timeAggregationLengthTextField.getText())) {
                throw new SettingsException("A time aggregation of " + _timeAggregationLengthTextField.getText() + " " + sPrecisionString + (Integer.parseInt(_timeAggregationLengthTextField.getText()) == 1 ? "" : "s") + " is greater than the " + Math.floor(dStudyPeriodLengthInUnits) + " " + sPrecisionString + " study period.\n", (Component) _timeAggregationLengthTextField);
            }
            if (Math.ceil(dStudyPeriodLengthInUnits / Double.parseDouble(_timeAggregationLengthTextField.getText())) <= 1) {
                throw new SettingsException("A time aggregation of " + _timeAggregationLengthTextField.getText() + " " + sPrecisionString + (Integer.parseInt(_timeAggregationLengthTextField.getText()) == 1 ? "" : "s") + " with a " + Math.floor(dStudyPeriodLengthInUnits) + " " + sPrecisionString + " study period results in only\n one time period to analyze. Temporal and space-time analyses can" + " not be performed\non less than two time periods.\n", (Component) _timeAggregationLengthTextField);
            }
            if (getAdvancedParameterInternalFrame().getMaxTemporalClusterSizeControlType() == Parameters.TemporalSizeType.PERCENTAGETYPE) {
                dMaxTemporalLengthInUnits = Math.floor(dStudyPeriodLengthInUnits * getAdvancedParameterInternalFrame().getMaxTemporalClusterSizeFromControl() / 100.0);
            }
            if (getAdvancedParameterInternalFrame().getMaxTemporalClusterSizeControlType() == Parameters.TemporalSizeType.TIMETYPE) {
                dMaxTemporalLengthInUnits = getAdvancedParameterInternalFrame().getMaxTemporalClusterSizeFromControl();
            }

            //validate the time aggregation agrees with maximum temporal cluster size
            if (Math.floor(dMaxTemporalLengthInUnits / Double.parseDouble(_timeAggregationLengthTextField.getText())) == 0) {
                if (getAdvancedParameterInternalFrame().getMaxTemporalClusterSizeControlType() == Parameters.TemporalSizeType.TIMETYPE) {
                    throw new SettingsException("The time aggregation of " + _timeAggregationLengthTextField.getText() + " " + sPrecisionString + (Integer.parseInt(_timeAggregationLengthTextField.getText()) == 1 ? "" : "s") + " is greater than the maximum temporal cluster size of " + getAdvancedParameterInternalFrame().getMaxTemporalClusterSizeFromControl() + " " + sPrecisionString + (getAdvancedParameterInternalFrame().getMaxTemporalClusterSizeFromControl() == 1 ? "" : "s") + ".\nPlease review settings.", (Component) _timeAggregationLengthTextField);
                }
                if (getAdvancedParameterInternalFrame().getMaxTemporalClusterSizeControlType() == Parameters.TemporalSizeType.PERCENTAGETYPE) {
                    throw new SettingsException("With the maximum temporal cluster size as " + getAdvancedParameterInternalFrame().getMaxTemporalClusterSizeFromControl() + "% of a " + Math.floor(dStudyPeriodLengthInUnits) + " " + sPrecisionString + " study period,\n" + "the time aggregation as " + _timeAggregationLengthTextField.getText() + " " + sPrecisionString + (Integer.parseInt(_timeAggregationLengthTextField.getText()) == 1 ? "" : "s") + " is greater than the resulting maximum\ntemporal cluster size of " + dMaxTemporalLengthInUnits + " " + sPrecisionString + " " + (dMaxTemporalLengthInUnits == 1 ? "" : "s") + ".\nPlease review settings.", (Component) _timeAggregationLengthTextField);
                }
            }
            /* Check that time aggregration length is less than defined maximum for unit type. */
            if (getAnalysisControlType() == Parameters.AnalysisType.SEASONALTEMPORAL) {
                switch (getTimeAggregationControlType()) {
                    case DAY:
                        if (Integer.parseInt(_timeAggregationLengthTextField.getText()) > 90) {
                            throw new SettingsException("The time aggregation may not exceed 90 days for the Seasonal analysis.\nPlease review settings.", (Component) _timeAggregationLengthTextField);
                        } break;                        
                    case MONTH:
                        if (Integer.parseInt(_timeAggregationLengthTextField.getText()) > 3) {
                            throw new SettingsException("The time aggregation may not exceed 3 months for the Seasonal analysis.\nPlease review settings.", (Component) _timeAggregationLengthTextField);
                        } break;                        
                    case GENERIC:
                        double maximum = dStudyPeriodLengthInUnits * 0.25;
                        if (Double.parseDouble(_timeAggregationLengthTextField.getText()) > maximum) {
                            throw new SettingsException("The time aggregation may not exceed " + maximum + " units for the Seasonal analysis.\nPlease review settings.", (Component) _timeAggregationLengthTextField);
                        } break;                        
                    case YEAR:
                    default: throw new UnknownEnumException(getTimeAggregationControlType());
                    
                }
            }
        }
    }

    /* Verifies all parameters on the 'Output Files' tab. Returns whether tab is valid. */
    private void validateOutputParams() {
        if (_resultsFileTextField.getText().length() == 0) {
            throw new SettingsException("Please specify a results file.", (Component) _resultsFileTextField);
        }
        if (!FileAccess.ValidateFileAccess(_resultsFileTextField.getText(), true, true)) {
            throw new SettingsException("Results file could not be opened for writing.\n" + "Please confirm that the path and/or file name\n" + "are valid and that you have permissions to write\nto this directory and file.",
                    (Component) _resultsFileTextField);
        }
        if (!FileAccess.isValidFilename(_resultsFileTextField.getText())) {                
            throw new SettingsException(String.format(AppConstants.FILENAME_ASCII_ERROR, _resultsFileTextField.getText()), (Component) _resultsFileTextField);
        }           
    }

    /**
     * Checks all the parameters on the 'Analysis' tab. Returns whether tab is valid.
     */
    private void validateAnalysisParams() {
        checkTimeAggregationLength();
    }

    /* Validate parameters settings and their inter-relationships. */
    public boolean validateParameters() {
        try {
            validateInputFiles();
            validateAnalysisParams();
            validateOutputParams();
        } catch (SettingsException e) {
            focusWindow();
            JOptionPane.showInternalMessageDialog(this, e.getMessage());
            e.setControlFocus();
            return false;
        }
        try {
            getAdvancedParameterInternalFrame().validateParameters();
        } catch (AdvFeaturesExpection e) {
            focusWindow();
            JOptionPane.showInternalMessageDialog(this, e.getMessage());
            getAdvancedParameterInternalFrame().setVisible(e.focusTab, e.focusComponent);
            enableAdvancedButtons();
            return false;
        }
        try {
            if (getModelControlType() == Parameters.ProbabilityModelType.HOMOGENEOUSPOISSON)
                geObservableRegionsParameterInternalFrame().Validate();
        } catch (RegionFeaturesException e) {
            focusWindow();
            JOptionPane.showInternalMessageDialog(this, e.getMessage());
            geObservableRegionsParameterInternalFrame().selectRegionAtIndex(e._regionIndex);
            geObservableRegionsParameterInternalFrame().setVisible(true);
            return false;
        }
        return true;
    }

    /**
     * Setup the dialog controls from the parameter settings.
     */
    private void setupInterface(final Parameters parameters) {
        _advancedParametersSetting = new AdvancedParameterSettingsFrame(_rootPane, this, parameters);
        _oberservableRegionsFrame = new OberservableRegionsFrame(_rootPane, this, parameters);

        title = parameters.GetSourceFileName();
        if (title == null || title.length() == 0) {
            title = "New Session";
        }

        //Input tab
        _caseFileTextField.setText(parameters.GetCaseFileName(1));
        _caseFileTextField.setCaretPosition(0);
        _controlFileTextField.setText(parameters.GetControlFileName(1));
        _controlFileTextField.setCaretPosition(0);
        setPrecisionOfTimesControl(parameters.GetPrecisionOfTimesType());
        _populationFileTextField.setText(parameters.GetPopulationFileName(1));
        _populationFileTextField.setCaretPosition(0);
        _coordiantesFileTextField.setText(parameters.GetCoordinatesFileName());
        _coordiantesFileTextField.setCaretPosition(0);
        _gridFileTextField.setText(parameters.GetSpecialGridFileName());
        _gridFileTextField.setCaretPosition(0);
        setCoordinatesType(parameters.GetCoordinatesType());
        //Analysis Tab
        setAnalysisTypeControl(parameters.GetAnalysisType());
        setModelControl(parameters.GetProbabilityModelType());
        setAreaScanRateControl(parameters.GetAreaScanRateType());
        if (parameters.GetPrecisionOfTimesType().equals(Parameters.DatePrecisionType.GENERIC)) {
            Utils.parseDateStringToControl(parameters.GetStudyPeriodStartDate(), _studyPeriodStartDateGenericTextField);
            Utils.parseDateStringToControl(parameters.GetStudyPeriodEndDate(), _studyPeriodEndDateGenericTextField);
        } else {
            Utils.parseDateStringToControls(parameters.GetStudyPeriodStartDate(), _studyPeriodStartDateYearTextField, _studyPeriodStartDateMonthTextField, _studyPeriodStartDateDayTextField, false);
            Utils.parseDateStringToControls(parameters.GetStudyPeriodEndDate(), _studyPeriodEndDateYearTextField, _studyPeriodEndDateMonthTextField, _studyPeriodEndDateDayTextField, true);
        }
        if (parameters.GetTimeAggregationUnitsType().equals(Parameters.DatePrecisionType.NONE)) {
            parameters.SetTimeAggregationUnitsType(Parameters.DatePrecisionType.YEAR.ordinal());
        }
        if (parameters.GetTimeAggregationLength() <= 0) {
            parameters.SetTimeAggregationLength(1);
        }
        _timeAggregationYearRadioButton.setSelected(parameters.GetTimeAggregationUnitsType().equals(Parameters.DatePrecisionType.YEAR));
        _timeAggregationMonthRadioButton.setSelected(parameters.GetTimeAggregationUnitsType().equals(Parameters.DatePrecisionType.MONTH));
        _timeAggregationDayRadioButton.setSelected(parameters.GetTimeAggregationUnitsType().equals(Parameters.DatePrecisionType.DAY));
        _timeAggregationLengthTextField.setText(Integer.toString(parameters.GetTimeAggregationLength()));
        //Output File Tab
        _resultsFileTextField.setText(parameters.GetOutputFileNameSetting());
        _resultsFileTextField.setCaretPosition(0);
        _relativeRiskEstimatesAreaAsciiCheckBox.setSelected(parameters.GetOutputRelativeRisksAscii());
        _relativeRiskEstimatesAreaDBaseCheckBox.setSelected(parameters.GetOutputRelativeRisksDBase());
        _simulatedLogLikelihoodRatiosAsciiCheckBox.setSelected(parameters.GetOutputSimLoglikeliRatiosAscii());
        _simulatedLogLikelihoodRatiosDBaseCheckBox.setSelected(parameters.GetOutputSimLoglikeliRatiosDBase());
        _censusAreasReportedClustersAsciiCheckBox.setSelected(parameters.GetOutputAreaSpecificAscii());  // Output Census areas in Reported Clusters
        _censusAreasReportedClustersDBaseCheckBox.setSelected(parameters.GetOutputAreaSpecificDBase());
        _clustersInColumnFormatAsciiCheckBox.setSelected(parameters.GetOutputClusterLevelAscii());  // Output Most Likely Cluster for each Centroid
        _clustersInColumnFormatDBaseCheckBox.setSelected(parameters.GetOutputClusterLevelDBase());
        _clusterCaseInColumnFormatAsciiCheckBox.setSelected(parameters.GetOutputClusterCaseAscii());  // Output Most Likely Cluster for each Centroid
        _clusterCaseInColumnFormatDBaseCheckBox.setSelected(parameters.GetOutputClusterCaseDBase());
        _reportGoogleEarthKML.setSelected(parameters.getOutputKMLFile());
        _reportShapefile.setSelected(parameters.getOutputShapeFiles());
        _reportCartesianGraph.setSelected(parameters.getOutputCartesianGraph());
        _reportGoogleMap.setSelected(parameters.getOutputGoogleMapsFile());
        
        _input_source_map.clear();
        for (int i=0; i < parameters.getInputSourceSettings().size(); ++i) {
            InputSourceSettings iss = parameters.getInputSourceSettings().get(i);
            _input_source_map.put(iss.getInputFileType().toString() + iss.getDataSetIndex(), iss);
        }
        _advancedParametersSetting.setupInterfaceFinalize(parameters); // Finish setting up advanced window
        onCountTimePrecisionChange();
        enableSettingsForAnalysisModelCombination();
    }

    /**
     * returns precision of time type for precision control index
     */
    public Parameters.CoordinatesType getCoordinatesType() {
        Parameters.CoordinatesType eReturn = null;

        if (_cartesianRadioButton.isSelected()) {
            eReturn = Parameters.CoordinatesType.CARTESIAN;
        } else if (_latLongRadioButton.isSelected()) {
            eReturn = Parameters.CoordinatesType.LATLON;
        } else {
            throw new RuntimeException("Unable to determine coordinates type.");
        }
        return eReturn;
    }

    /**
     * returns area scan rate type for control group
     */
    public Parameters.AreaRateType getAreaScanRateControlType() {
        Parameters.AreaRateType eReturn = null;

        if (_highRatesRadioButton.isSelected()) {
            eReturn = Parameters.AreaRateType.HIGH;
        } else if (_lowRatesRadioButton.isSelected()) {
            eReturn = Parameters.AreaRateType.LOW;
        } else if (_highOrLowRatesRadioButton.isSelected()) {
            eReturn = Parameters.AreaRateType.HIGHANDLOW;
        } else {
            throw new RuntimeException("Unable to determine scanning area type.");
        }
        return eReturn;
    }

    /** return precision type for time aggregation type */
    public Parameters.DatePrecisionType getTimeAggregationControlType() {
        Parameters.DatePrecisionType eReturn = null;

        if (_timePrecisionGeneric.isSelected()) {
           eReturn = Parameters.DatePrecisionType.GENERIC;
        } else if (_timeAggregationYearRadioButton.isSelected()) {
            eReturn = Parameters.DatePrecisionType.YEAR;
        } else if (_timeAggregationMonthRadioButton.isSelected()) {
            eReturn = Parameters.DatePrecisionType.MONTH;
        } else if (_timeAggregationDayRadioButton.isSelected()) {
            eReturn = Parameters.DatePrecisionType.DAY;
        } else {
            throw new RuntimeException("Unable to determine time aggregation type.");
        }
        return eReturn;
    }

    /**
     * sets CParameters class with settings in form
     */
    private void saveParameterSettings(Parameters parameters) {
        String sString;

        setTitle(parameters.GetSourceFileName());

        //Input File Tab
        parameters.SetCaseFileName(_caseFileTextField.getText(), 1);
        parameters.SetControlFileName(_controlFileTextField.getText(), 1);
        parameters.SetPrecisionOfTimesType(getPrecisionOfTimesControlType().ordinal());
        parameters.SetPopulationFileName(_populationFileTextField.getText(), 1);
        parameters.SetCoordinatesFileName(_coordiantesFileTextField.getText());
        parameters.SetSpecialGridFileName(_gridFileTextField.getText());
        parameters.SetCoordinatesType(getCoordinatesType().ordinal());

        //Analysis Tab
        parameters.SetAnalysisType(getAnalysisControlType().ordinal());
        parameters.SetProbabilityModelType(getModelControlType().ordinal());
        parameters.SetAreaRateType(getAreaScanRateControlType().ordinal());

        if (parameters.GetPrecisionOfTimesType().equals(Parameters.DatePrecisionType.GENERIC)) {
          parameters.SetStudyPeriodStartDate(_studyPeriodStartDateGenericTextField.getText());
          parameters.SetStudyPeriodEndDate(_studyPeriodEndDateGenericTextField.getText());
        } else {
          sString = _studyPeriodStartDateYearTextField.getText() + "/" + _studyPeriodStartDateMonthTextField.getText() + "/" + _studyPeriodStartDateDayTextField.getText();
          parameters.SetStudyPeriodStartDate(sString);
          sString = _studyPeriodEndDateYearTextField.getText() + "/" + _studyPeriodEndDateMonthTextField.getText() + "/" + _studyPeriodEndDateDayTextField.getText();
          parameters.SetStudyPeriodEndDate(sString);
        }
        parameters.SetTimeAggregationUnitsType(getTimeAggregationControlType().ordinal());
        parameters.SetTimeAggregationLength(Integer.parseInt(_timeAggregationLengthTextField.getText()));

        //Output File Tab
        parameters.SetOutputFileNameSetting(_resultsFileTextField.getText());
        parameters.SetOutputClusterLevelAscii(_clustersInColumnFormatAsciiCheckBox.isSelected());
        parameters.SetOutputClusterLevelDBase(_clustersInColumnFormatDBaseCheckBox.isSelected());
        parameters.SetOutputClusterCaseAscii(_clusterCaseInColumnFormatAsciiCheckBox.isSelected());
        parameters.SetOutputClusterCaseDBase(_clusterCaseInColumnFormatDBaseCheckBox.isSelected());
        parameters.SetOutputAreaSpecificAscii(_censusAreasReportedClustersAsciiCheckBox.isSelected());
        parameters.SetOutputAreaSpecificDBase(_censusAreasReportedClustersDBaseCheckBox.isSelected());
        parameters.SetOutputRelativeRisksAscii(Utils.selected(_relativeRiskEstimatesAreaAsciiCheckBox));
        parameters.SetOutputRelativeRisksDBase(Utils.selected(_relativeRiskEstimatesAreaDBaseCheckBox));
        parameters.SetOutputSimLogLikeliRatiosAscii(_simulatedLogLikelihoodRatiosAsciiCheckBox.isSelected());
        parameters.SetOutputSimLogLikeliRatiosDBase(_simulatedLogLikelihoodRatiosDBaseCheckBox.isSelected());
        parameters.setOutputKMLFile(Utils.selected(_reportGoogleEarthKML));
        parameters.setOutputShapeFiles(Utils.selected(_reportShapefile));
        parameters.setOutputCartesianGraph(Utils.selected(_reportCartesianGraph));
        parameters.setOutputGoogleMapsFile(Utils.selected(_reportGoogleMap));
        getAdvancedParameterInternalFrame().saveParameterSettings(parameters);
        geObservableRegionsParameterInternalFrame().saveParameterSettings(parameters);
        
        parameters.clearInputSourceSettings();
        Iterator it = _input_source_map.entrySet().iterator();
        while (it.hasNext()) {
            Map.Entry pairs = (Map.Entry)it.next();
            InputSourceSettings iss = (InputSourceSettings)pairs.getValue();
            if (iss.isSet()) {
                parameters.addInputSourceSettings((InputSourceSettings)pairs.getValue());
            }
        }
    }

    /**
     * returns precision of time type for precision control index
     */
    public Parameters.DatePrecisionType getPrecisionOfTimesControlType() {
        Parameters.DatePrecisionType eReturn = null;
        if (_timePrecisionNone.isSelected() == true) {
            eReturn = Parameters.DatePrecisionType.NONE;
        } else if (_timePrecisionYear.isSelected() == true) {
            eReturn = Parameters.DatePrecisionType.YEAR;
        } else if (_timePrecisionMonth.isSelected() == true) {
            eReturn = Parameters.DatePrecisionType.MONTH;
        } else if (_timePrecisionDay.isSelected() == true) {
            eReturn = Parameters.DatePrecisionType.DAY;
        } else if (_timePrecisionGeneric.isSelected() == true) {
            eReturn = Parameters.DatePrecisionType.GENERIC;
        } else {
            throw new RuntimeException("Unable to determine precision of times type.");
        }
        return eReturn;
    }

    /**
     *
     */
    public Parameters.AnalysisType getAnalysisControlType() {
        Parameters.AnalysisType eReturn = null;
        if (_retrospectivePurelySpatialRadioButton.isSelected()) {
            eReturn = Parameters.AnalysisType.PURELYSPATIAL;
        } else if (_retrospectivePurelyTemporalRadioButton.isSelected()) {
            eReturn = Parameters.AnalysisType.PURELYTEMPORAL;
        } else if (_retrospectiveSpaceTimeRadioButton.isSelected()) {
            eReturn = Parameters.AnalysisType.SPACETIME;
        } else if (_prospectivePurelyTemporalRadioButton.isSelected()) {
            eReturn = Parameters.AnalysisType.PROSPECTIVEPURELYTEMPORAL;
        } else if (_prospectiveSpaceTimeRadioButton.isSelected()) {
            eReturn = Parameters.AnalysisType.PROSPECTIVESPACETIME;
        } else if (_spatialVariationRadioButton.isSelected()) {
            eReturn = Parameters.AnalysisType.SPATIALVARTEMPTREND;
        } else if (_retrospectiveSeasonalRadioButton.isSelected()) {
            eReturn = Parameters.AnalysisType.SEASONALTEMPORAL;            
        } else {
            throw new RuntimeException("Unable to determine analysis type.");
        }
        return eReturn;
    }

    /* Returns whether controls indicate selected analysis is prospective. */
    public boolean isProspectiveScan() {
        Parameters.AnalysisType e = getAnalysisControlType();
        return e == Parameters.AnalysisType.PROSPECTIVEPURELYTEMPORAL || e == Parameters.AnalysisType.PROSPECTIVESPACETIME;
    }
    
    /**
     *
     */
    public Parameters.ProbabilityModelType getModelControlType() {
        Parameters.ProbabilityModelType eReturn = null;
        if (_poissonModelRadioButton.isSelected()) {
            eReturn = Parameters.ProbabilityModelType.POISSON;
        } else if (_bernoulliModelRadioButton.isSelected()) {
            eReturn = Parameters.ProbabilityModelType.BERNOULLI;
        } else if (_spaceTimePermutationModelRadioButton.isSelected()) {
            eReturn = Parameters.ProbabilityModelType.SPACETIMEPERMUTATION;
        } else if (_ordinalModelRadioButton.isSelected()) {
            eReturn = Parameters.ProbabilityModelType.ORDINAL;
        } else if (_exponentialModelRadioButton.isSelected()) {
            eReturn = Parameters.ProbabilityModelType.EXPONENTIAL;
        } else if (_normalModelRadioButton.isSelected()) {
            eReturn = Parameters.ProbabilityModelType.NORMAL;
        } else if (_categoricallModelRadioButton.isSelected()) {
            eReturn = Parameters.ProbabilityModelType.CATEGORICAL;
        } else if (_homogeneouspoissonModelRadioButton.isSelected()) {
            eReturn = Parameters.ProbabilityModelType.HOMOGENEOUSPOISSON;
        } else if (_uniformtimeModelRadioButton.isSelected()) {
            eReturn = Parameters.ProbabilityModelType.UNIFORMTIME;
        } else if (_batchedModelRadioButton.isSelected()) {
            eReturn = Parameters.ProbabilityModelType.BATCHED;
        } else {
            throw new RuntimeException("Unable to determine probability model type.");
        }
        return eReturn;
    }

    /** enables or disables the study period group controls */
    private void enableStudyPeriodDates(boolean enableYear, boolean enableMonth, boolean enableDay) {
        //enable study period year controls
        _studyPeriodStartDateYearTextField.setEditable(enableYear);
        _studyPeriodEndDateYearTextField.setEnabled(enableYear);
        
        // Start date month and day values.
        if (_studyPeriodStartDateMonthTextField.isEnabled() && !enableMonth) {
            // Start date month is going from enabled to disabled. Save month as January.
            _startDateComponentsGroup.setMonth(1);
        } else if (!_studyPeriodStartDateMonthTextField.isEnabled() && enableMonth) {
            // Start date month is going from disabled to enabled. Restore from sticky value.
            _startDateComponentsGroup.restoreMonth();
        }
        if (_studyPeriodStartDateDayTextField.isEnabled() && !enableDay) {
            // Start date day is going from enabled to disabled. Save as first day of month.
            _startDateComponentsGroup.setDay(1);
        } else if (!_studyPeriodStartDateDayTextField.isEnabled() && enableDay) {
            // Start date day is going from disabled to enabled. Restore from sticky value.
            _startDateComponentsGroup.restoreDay();
        }

        // End date month and day values.
        if (_studyPeriodEndDateMonthTextField.isEnabled() && !enableMonth) {
            // End date month is going from enabled to disabled. Save to month as December.
            _endDateComponentsGroup.setMonth(12);
        } else if (!_studyPeriodEndDateMonthTextField.isEnabled() && enableMonth) {
            // Start date month is going from disabled to enabled. Restore from sticky value.
            _endDateComponentsGroup.restoreMonth();
        }
        if (_studyPeriodEndDateDayTextField.isEnabled() && !enableDay) {
            // End date day is going from enabled to disabled. Save as last day of month.
            _endDateComponentsGroup.setDay(31);
        } else if (!_studyPeriodEndDateDayTextField.isEnabled() && enableDay) {
            // End date day is going from disabled to enabled. Restore from sticky value.
            _endDateComponentsGroup.restoreDay();
        } else if (!enableDay) {
            // End date is continuing to be disabled, make sure that day is last day of month.
            _endDateComponentsGroup.setDay(31);
        }
        
        //enable study period month controls
        _studyPeriodStartDateMonthTextField.setEnabled(enableMonth);
        _studyPeriodEndDateMonthTextField.setEnabled(enableMonth);
        //enable study period day controls
        _studyPeriodStartDateDayTextField.setEnabled(enableDay);
        _studyPeriodEndDateDayTextField.setEnabled(enableDay);
    }

    /**
     * enabled study period and prospective date precision based on time interval unit
     */
    private void enableDatesByTimePrecisionUnits() {
        CardLayout cl = (CardLayout)(_studyPeriodGroup.getLayout());
        switch (getPrecisionOfTimesControlType()) {
            case NONE:
            case DAY:
                enableStudyPeriodDates(true, true, true);
                cl.show(_studyPeriodGroup, STUDY_COMPLETE);
                break;
            case YEAR:
                enableStudyPeriodDates(true, false, false);
                cl.show(_studyPeriodGroup, STUDY_COMPLETE);
                break;
            case MONTH:
                enableStudyPeriodDates(true, true, false);
                cl.show(_studyPeriodGroup, STUDY_COMPLETE);
                break;
            case GENERIC:
                cl.show(_studyPeriodGroup, STUDY_GENERIC);
                break;
            default:
                throw new UnknownEnumException(getPrecisionOfTimesControlType());
        }
        getAdvancedParameterInternalFrame().enableDatesByTimePrecisionUnits();
    }

    /**
     * enables or disables the time aggregation group control
     */
    private void enableTimeAggregationGroup(boolean bEnable) {
        Parameters.DatePrecisionType eDatePrecisionType = getPrecisionOfTimesControlType();
        boolean bGenericType = eDatePrecisionType == Parameters.DatePrecisionType.GENERIC;
        boolean seasonal = getAnalysisControlType() == Parameters.AnalysisType.SEASONALTEMPORAL;

        _timeAggregationGroup.setEnabled(bEnable && eDatePrecisionType != Parameters.DatePrecisionType.NONE);
        _timeAggregationUnitsLabel.setEnabled(bEnable && !eDatePrecisionType.equals(Parameters.DatePrecisionType.NONE) && !eDatePrecisionType.equals(Parameters.DatePrecisionType.GENERIC));
        _timeAggregationLengthTextField.setEnabled(bEnable && !eDatePrecisionType.equals(Parameters.DatePrecisionType.NONE));
        _timeAggregationLengthLabel.setEnabled(bEnable && eDatePrecisionType != Parameters.DatePrecisionType.NONE);                                               
        _aggregrationUnitsLabel.setEnabled(bEnable && eDatePrecisionType != Parameters.DatePrecisionType.NONE);                
        _timeAggregationYearRadioButton.setEnabled(bEnable && !bGenericType && !seasonal && eDatePrecisionType != Parameters.DatePrecisionType.NONE);                
        _timeAggregationMonthRadioButton.setEnabled(bEnable && !bGenericType &&
                                                    eDatePrecisionType != Parameters.DatePrecisionType.NONE && 
                                                    eDatePrecisionType != Parameters.DatePrecisionType.YEAR);        
        if (_timeAggregationGroup.isEnabled() && !bGenericType && _timeAggregationMonthRadioButton.isSelected() && !_timeAggregationMonthRadioButton.isEnabled()) {
            _timeAggregationYearRadioButton.setSelected(true);
        }
        _timeAggregationDayRadioButton.setEnabled(bEnable && eDatePrecisionType == Parameters.DatePrecisionType.DAY);
        if (_timeAggregationGroup.isEnabled() && !bGenericType && _timeAggregationDayRadioButton.isSelected() && !_timeAggregationDayRadioButton.isEnabled()) {
            if (_timeAggregationMonthRadioButton.isEnabled()) {
                _timeAggregationMonthRadioButton.setSelected(true);
            } else {
                _timeAggregationYearRadioButton.setSelected(true);
            }
        }
        if (_timeAggregationGroup.isEnabled() && bGenericType) {
            getAdvancedParameterInternalFrame().updateMaximumTemporalSizeTextCaptions();
            _aggregrationUnitsLabel.setText("Units");            
        }
        
       if (seasonal && _timeAggregationYearRadioButton.isSelected()) {
           _timeAggregationMonthRadioButton.setSelected(true);
       }
    }

    /**
     * enables controls of 'additional optional output file' radio group
     */
    private void enableAdditionalOutFilesOptionsGroup(boolean bRelativeRisks) {
        _relativeRiskEstimatesAreaAsciiCheckBox.setEnabled(bRelativeRisks);
        _relativeRiskEstimatesAreaDBaseCheckBox.setEnabled(bRelativeRisks);
        _relativeRiskEstimatesAreaLabel.setEnabled(bRelativeRisks);
        boolean isPurelyTemporalAnalysis = (
            getAnalysisControlType() == Parameters.AnalysisType.PURELYTEMPORAL ||
            getAnalysisControlType() == Parameters.AnalysisType.PROSPECTIVEPURELYTEMPORAL ||
            getAnalysisControlType() == Parameters.AnalysisType.SEASONALTEMPORAL
        );
        _reportGoogleEarthKML.setEnabled(getCoordinatesType() == Parameters.CoordinatesType.LATLON && !isPurelyTemporalAnalysis);
        _reportShapefile.setEnabled(getCoordinatesType() == Parameters.CoordinatesType.LATLON && !isPurelyTemporalAnalysis);
        _reportCartesianGraph.setEnabled(!isPurelyTemporalAnalysis);
        _reportGoogleMap.setEnabled(getCoordinatesType() == Parameters.CoordinatesType.LATLON && !isPurelyTemporalAnalysis);
    }

    /**
     * Enables window controls given the selection of analysis type and probability model.
     */
    private void enableSettingsForAnalysisModelCombination() {
        Parameters.AnalysisType eAnalysisType = getAnalysisControlType();
        Parameters.ProbabilityModelType eModelType = getModelControlType();

        enableDatesByTimePrecisionUnits();
        enableTimeAggregationGroup(eAnalysisType != Parameters.AnalysisType.PURELYSPATIAL);        
        enableAdditionalOutFilesOptionsGroup(
                eModelType != Parameters.ProbabilityModelType.HOMOGENEOUSPOISSON &&
                eModelType != Parameters.ProbabilityModelType.ORDINAL &&
                eModelType != Parameters.ProbabilityModelType.UNIFORMTIME &&
                eModelType != Parameters.ProbabilityModelType.BATCHED &&
                eModelType != Parameters.ProbabilityModelType.CATEGORICAL &&
                eAnalysisType != Parameters.AnalysisType.SPATIALVARTEMPTREND &&
                eAnalysisType != Parameters.AnalysisType.PURELYTEMPORAL &&
                eAnalysisType != Parameters.AnalysisType.PROSPECTIVEPURELYTEMPORAL &&
                eAnalysisType != Parameters.AnalysisType.SEASONALTEMPORAL);
        _observableRegionsButton.setEnabled(eModelType == Parameters.ProbabilityModelType.HOMOGENEOUSPOISSON);
        setEnableAreaScanRateControl();
        getAdvancedParameterInternalFrame().enableSettingsForAnalysisModelCombination();
        enableAdvancedButtons();
    }

    /**
     * enables probability model control based upon the setting in analysis control
     */
    private void enableModelControlForAnalysisType() {
        Parameters.AnalysisType eAnalysisType = getAnalysisControlType();

        _bernoulliModelRadioButton.setEnabled(eAnalysisType != Parameters.AnalysisType.SPATIALVARTEMPTREND);
        _spaceTimePermutationModelRadioButton.setEnabled(eAnalysisType == Parameters.AnalysisType.SPACETIME || eAnalysisType == Parameters.AnalysisType.PROSPECTIVESPACETIME);
        if (!_spaceTimePermutationModelRadioButton.isEnabled() && _spaceTimePermutationModelRadioButton.isSelected()) {
          _poissonModelRadioButton.setSelected(true);  
        }
        _uniformtimeModelRadioButton.setEnabled(
                eAnalysisType == Parameters.AnalysisType.PURELYTEMPORAL || 
                eAnalysisType == Parameters.AnalysisType.PROSPECTIVEPURELYTEMPORAL ||
                eAnalysisType == Parameters.AnalysisType.SPACETIME || 
                eAnalysisType == Parameters.AnalysisType.PROSPECTIVESPACETIME
        );
        if (!_uniformtimeModelRadioButton.isEnabled() && _uniformtimeModelRadioButton.isSelected()) {
          _poissonModelRadioButton.setSelected(true);  
        }        
        _batchedModelRadioButton.setEnabled(eAnalysisType != Parameters.AnalysisType.SPATIALVARTEMPTREND);
        if (!_batchedModelRadioButton.isEnabled() && _batchedModelRadioButton.isSelected()) {
          _poissonModelRadioButton.setSelected(true);  
        }        
        _categoricallModelRadioButton.setEnabled(eAnalysisType != Parameters.AnalysisType.SPATIALVARTEMPTREND);
        _ordinalModelRadioButton.setEnabled(eAnalysisType != Parameters.AnalysisType.SPATIALVARTEMPTREND);
        _exponentialModelRadioButton.setEnabled(eAnalysisType != Parameters.AnalysisType.SPATIALVARTEMPTREND);
        _normalModelRadioButton.setEnabled(eAnalysisType != Parameters.AnalysisType.SPATIALVARTEMPTREND);
        _homogeneouspoissonModelRadioButton.setEnabled(eAnalysisType == Parameters.AnalysisType.PURELYSPATIAL && getCoordinatesType() == Parameters.CoordinatesType.CARTESIAN);
        if (!_homogeneouspoissonModelRadioButton.isEnabled() && _homogeneouspoissonModelRadioButton.isSelected()) {
          _poissonModelRadioButton.setSelected(true);
        }
        enableSettingsForAnalysisModelCombination();
        setAreaScanRateControlText(getModelControlType());
    }

    private void setAnalysisTypeControl(Parameters.AnalysisType eAnalysisType) {
        Parameters.DatePrecisionType eDatePrecisionType = getPrecisionOfTimesControlType();

        switch (eAnalysisType) {
            case PURELYTEMPORAL:
                if (_retrospectivePurelyTemporalRadioButton.isEnabled() && eDatePrecisionType != Parameters.DatePrecisionType.NONE) {
                    _retrospectivePurelyTemporalRadioButton.setSelected(true);
                    break;
                }
            case SPACETIME:
                if (_retrospectiveSpaceTimeRadioButton.isEnabled() && eDatePrecisionType != Parameters.DatePrecisionType.NONE) {
                    _retrospectiveSpaceTimeRadioButton.setSelected(true);
                    break;
                }
            case PROSPECTIVEPURELYTEMPORAL:
                if (_prospectivePurelyTemporalRadioButton.isEnabled() && eDatePrecisionType != Parameters.DatePrecisionType.NONE) {
                    _prospectivePurelyTemporalRadioButton.setSelected(true);
                    break;
                }
            case PROSPECTIVESPACETIME:
                if (_prospectiveSpaceTimeRadioButton.isEnabled() && eDatePrecisionType != Parameters.DatePrecisionType.NONE) {
                    _prospectiveSpaceTimeRadioButton.setSelected(true);
                    break;
                }
            case SPATIALVARTEMPTREND:
                _spatialVariationRadioButton.setSelected(true);
                break;
            case SEASONALTEMPORAL:
                _retrospectiveSeasonalRadioButton.setSelected(true);
                break;
            case PURELYSPATIAL:
            default:
                _retrospectivePurelySpatialRadioButton.setSelected(true);
        }
        enableModelControlForAnalysisType();
    }

    private void onCountTimePrecisionChange() {
        Parameters.DatePrecisionType eDatePrecisionType = getPrecisionOfTimesControlType();
        Parameters.AnalysisType eAnalysis = getAnalysisControlType();
        Parameters.ProbabilityModelType eProbModel = getModelControlType();

        //disable analyses that don't match precision
        _retrospectivePurelySpatialRadioButton.setEnabled(true);
        _retrospectivePurelyTemporalRadioButton.setEnabled(eDatePrecisionType != Parameters.DatePrecisionType.NONE);
        _retrospectiveSpaceTimeRadioButton.setEnabled(eDatePrecisionType != Parameters.DatePrecisionType.NONE);
        _prospectivePurelyTemporalRadioButton.setEnabled(eDatePrecisionType != Parameters.DatePrecisionType.NONE);
        _prospectiveSpaceTimeRadioButton.setEnabled(eDatePrecisionType != Parameters.DatePrecisionType.NONE);
        _spatialVariationRadioButton.setEnabled(eProbModel == Parameters.ProbabilityModelType.POISSON && eDatePrecisionType != Parameters.DatePrecisionType.NONE);
        _spatialVariationRadioLabel.setEnabled(_spatialVariationRadioButton.isEnabled());
        boolean enableseasonal = !(eDatePrecisionType == Parameters.DatePrecisionType.NONE || eDatePrecisionType == Parameters.DatePrecisionType.YEAR);
        enableseasonal &= !(eProbModel == Parameters.ProbabilityModelType.SPACETIMEPERMUTATION || eProbModel == Parameters.ProbabilityModelType.HOMOGENEOUSPOISSON);
        _retrospectiveSeasonalRadioButton.setEnabled(enableseasonal);

        // switch analysis type to purely spatial if no dates in input data
        if (eDatePrecisionType == Parameters.DatePrecisionType.NONE && eAnalysis != Parameters.AnalysisType.PURELYSPATIAL) {
            setAnalysisTypeControl(Parameters.AnalysisType.PURELYSPATIAL);
        }
        enableTimeAggregationGroup(eAnalysis != Parameters.AnalysisType.PURELYSPATIAL);
        enableDatesByTimePrecisionUnits();
    }

    public final Parameters getParameterSettings() {
        saveParameterSettings(_parameters);
        return _parameters;
    }

    public void showExecOptionsDialog(java.awt.Frame parent) {
        new ExecutionOptionsDialog(parent, _parameters).setVisible(true);
    }

    /**
     * enables analysis control based upon the setting in probability model control
     */
    private void enableAnalysisControlForModelType() {
        Parameters.DatePrecisionType eDatePrecisionType = getPrecisionOfTimesControlType();

        _spatialVariationRadioButton.setEnabled(getModelControlType() == Parameters.ProbabilityModelType.POISSON &&
                                                eDatePrecisionType != Parameters.DatePrecisionType.NONE);
        _spatialVariationRadioLabel.setEnabled(_spatialVariationRadioButton.isEnabled());
        if (!_spatialVariationRadioButton.isEnabled() && _spatialVariationRadioButton.isSelected()) {
          _retrospectivePurelySpatialRadioButton.setSelected(true);
        }
        if (!_spatialVariationRadioButton.isEnabled() && _spatialVariationRadioButton.isSelected()) {
          _retrospectivePurelySpatialRadioButton.setSelected(true);
        }        
        switch (getModelControlType()) {
            case POISSON:                                 
            case BERNOULLI:
            case ORDINAL:
            case NORMAL:
            case CATEGORICAL:
            case BATCHED:                
            case EXPONENTIAL:
                _retrospectivePurelySpatialRadioButton.setEnabled(true);
                _retrospectivePurelyTemporalRadioButton.setEnabled(eDatePrecisionType != Parameters.DatePrecisionType.NONE);
                _retrospectiveSpaceTimeRadioButton.setEnabled(eDatePrecisionType != Parameters.DatePrecisionType.NONE);
                _prospectivePurelyTemporalRadioButton.setEnabled(eDatePrecisionType != Parameters.DatePrecisionType.NONE);
                _prospectiveSpaceTimeRadioButton.setEnabled(eDatePrecisionType != Parameters.DatePrecisionType.NONE);
                _retrospectiveSeasonalRadioButton.setEnabled(eDatePrecisionType != Parameters.DatePrecisionType.NONE || eDatePrecisionType != Parameters.DatePrecisionType.YEAR);
                break;
            case HOMOGENEOUSPOISSON:
                _retrospectivePurelySpatialRadioButton.setEnabled(true);
                _retrospectivePurelyTemporalRadioButton.setEnabled(false);
                _retrospectiveSpaceTimeRadioButton.setEnabled(false);
                _prospectivePurelyTemporalRadioButton.setEnabled(false);
                _prospectiveSpaceTimeRadioButton.setEnabled(false);
                if (!_retrospectivePurelySpatialRadioButton.isSelected()) {
                    _retrospectivePurelySpatialRadioButton.setSelected(true);
                }
                _retrospectiveSeasonalRadioButton.setEnabled(false);
                break;
            case SPACETIMEPERMUTATION:
                _retrospectivePurelySpatialRadioButton.setEnabled(false);
                _retrospectivePurelyTemporalRadioButton.setEnabled(false);
                _retrospectiveSpaceTimeRadioButton.setEnabled(true);
                _prospectivePurelyTemporalRadioButton.setEnabled(false);
                _prospectiveSpaceTimeRadioButton.setEnabled(true);
                if (!_retrospectiveSpaceTimeRadioButton.isSelected() && !_prospectiveSpaceTimeRadioButton.isSelected()) {
                    _retrospectiveSpaceTimeRadioButton.setSelected(true);
                }
                _retrospectiveSeasonalRadioButton.setEnabled(false);
                break;
            case UNIFORMTIME:
                _retrospectivePurelySpatialRadioButton.setEnabled(false);
                _retrospectivePurelyTemporalRadioButton.setEnabled(true);
                _retrospectiveSpaceTimeRadioButton.setEnabled(true);
                _prospectivePurelyTemporalRadioButton.setEnabled(true);
                _prospectiveSpaceTimeRadioButton.setEnabled(true);
                _retrospectiveSeasonalRadioButton.setEnabled(false);
                break;
        }
        enableSettingsForAnalysisModelCombination();
    }

    /**
     * Sets captions of TRadioButton controls of 'Scan for Areas with:' group based upon selected probability model.
     */
    private void setAreaScanRateControlText(Parameters.ProbabilityModelType eProbabilityModelType) {
        switch (eProbabilityModelType) {
            case ORDINAL:
            case NORMAL:
                _lowRatesRadioButton.setText("Low Values");
                _highRatesRadioButton.setText("High Values");
                _highOrLowRatesRadioButton.setText("High or Low Values");
                break;
            case EXPONENTIAL:
                _lowRatesRadioButton.setText("Long Survival");
                _highRatesRadioButton.setText("Short Survival");
                _highOrLowRatesRadioButton.setText("Short or Long Survival");
                break;
            case POISSON:
                if (getAnalysisControlType() == Parameters.AnalysisType.SPATIALVARTEMPTREND) {
                    _lowRatesRadioButton.setText("Lower Trend");
                    _highRatesRadioButton.setText("Higher Trend");
                    _highOrLowRatesRadioButton.setText("Higher or Lower Trend");
                    break;
                }
            case HOMOGENEOUSPOISSON:    
            case BERNOULLI:
            case SPACETIMEPERMUTATION:
            default:    
                _lowRatesRadioButton.setText("Low Rates");
                _highRatesRadioButton.setText("High Rates");
                _highOrLowRatesRadioButton.setText("High or Low Rates");
                break;
        }
        setEnableAreaScanRateControl();
    }

    /**
     * method called in response to 'probability model' radio group click event
     */
    private void onProbabilityModelTypeClick() {
        enableAnalysisControlForModelType();
        if (getModelControlType() == Parameters.ProbabilityModelType.SPACETIMEPERMUTATION ||
            getModelControlType() == Parameters.ProbabilityModelType.UNIFORMTIME) {
            _simulatedLogLikelihoodRatiosLabel.setText("Simulated Test Statistics");
        } else {
            _simulatedLogLikelihoodRatiosLabel.setText("Simulated Log Likelihood Ratios");  
        }
        setAreaScanRateControlText(getModelControlType());
        enableSettingsForAnalysisModelCombination();
    }

    /**
     *
     */
    public final String getEdtStudyPeriodEndDateYearText() {
        return _studyPeriodEndDateYearTextField.getText();
    }

    /**
     *
     */
    public final String getEdtStudyPeriodEndDateMonthText() {
        return _studyPeriodEndDateMonthTextField.getText();
    }

    /**
     *
     */
    public final String getEdtStudyPeriodEndDateDayText() {
        return _studyPeriodEndDateDayTextField.getText();
    }

    public String getEdtPopFileNameText() {
        return _populationFileTextField.getText();
    }

    /** sets coordinate type */
    public void setCoordinateType(Parameters.CoordinatesType eCoordinatesType) {
        switch (eCoordinatesType) {
            case CARTESIAN:
                _cartesianRadioButton.setSelected(true);
                break;
            case LATLON:
                _latLongRadioButton.setSelected(true);
                break;
            default:
        }
    }

    /* Toggles collection of checkboxes either all on or all off. */
    private void toggleColumnOutputSettings(List<javax.swing.JCheckBox> check_boxes) {
        boolean all_checked = true;
        all_checked = check_boxes.stream().filter(box -> (box.isEnabled())).map(box -> box.isSelected()).reduce(all_checked, (accumulator, _item) -> accumulator & _item);
        for (javax.swing.JCheckBox box: check_boxes) {
            if (box.isEnabled())
                box.setSelected(!all_checked);
        }
    }
    
    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

        _timePrecisionButtonGroup = new javax.swing.ButtonGroup();
        _coordinatesButtonGroup = new javax.swing.ButtonGroup();
        _analysisTypeButtonGroup = new javax.swing.ButtonGroup();
        _probabilityModelTypeButtonGroup = new javax.swing.ButtonGroup();
        _scanAreasButtonGroup = new javax.swing.ButtonGroup();
        _timeAggregationButtonGroup = new javax.swing.ButtonGroup();
        _tabbedPane = new javax.swing.JTabbedPane();
        _inputTab = new javax.swing.JPanel();
        _caseInputPanel = new javax.swing.JPanel();
        _caseFileLabel = new javax.swing.JLabel();
        _caseFileTextField = new javax.swing.JTextField();
        _caseFileBrowseButton = new javax.swing.JButton();
        _timePrecisionGroup = new javax.swing.JPanel();
        _timePrecisionNone = new javax.swing.JRadioButton();
        _timePrecisionYear = new javax.swing.JRadioButton();
        _timePrecisionMonth = new javax.swing.JRadioButton();
        _timePrecisionDay = new javax.swing.JRadioButton();
        _timePrecisionGeneric = new javax.swing.JRadioButton();
        _studyPeriodGroup = new javax.swing.JPanel();
        _studyPeriodComplete = new javax.swing.JPanel();
        _startDateLabel = new javax.swing.JLabel();
        _startDateYearLabel = new javax.swing.JLabel();
        _studyPeriodStartDateYearTextField = new javax.swing.JTextField();
        _studyPeriodStartDateMonthTextField = new javax.swing.JTextField();
        _startDateMonthLabel = new javax.swing.JLabel();
        _startDateDayLabel = new javax.swing.JLabel();
        _studyPeriodStartDateDayTextField = new javax.swing.JTextField();
        _endDateLabel = new javax.swing.JLabel();
        _studyPeriodEndDateYearTextField = new javax.swing.JTextField();
        _endDateYearLabel = new javax.swing.JLabel();
        _endDateMonthLabel = new javax.swing.JLabel();
        _studyPeriodEndDateMonthTextField = new javax.swing.JTextField();
        _studyPeriodEndDateDayTextField = new javax.swing.JTextField();
        _endDateDayLabel = new javax.swing.JLabel();
        _studyPeriodGeneric = new javax.swing.JPanel();
        _startDateLabel1 = new javax.swing.JLabel();
        _startDateYearLabel1 = new javax.swing.JLabel();
        _studyPeriodStartDateGenericTextField = new javax.swing.JTextField();
        _endDateLabel1 = new javax.swing.JLabel();
        _studyPeriodEndDateGenericTextField = new javax.swing.JTextField();
        _endDateYearLabel1 = new javax.swing.JLabel();
        _controlFileLabel = new javax.swing.JLabel();
        _controlFileTextField = new javax.swing.JTextField();
        _controlFileBrowseButton = new javax.swing.JButton();
        _bernoulliModelHintLabel = new javax.swing.JLabel();
        _populationInputPanel = new javax.swing.JPanel();
        _populationFileLabel = new javax.swing.JLabel();
        _populationFileTextField = new javax.swing.JTextField();
        _populationFileBrowseButton = new javax.swing.JButton();
        _poissionModelHintLabel = new javax.swing.JLabel();
        _geographicalInputPanel = new javax.swing.JPanel();
        _coordinatesFileLabel = new javax.swing.JLabel();
        _coordiantesFileTextField = new javax.swing.JTextField();
        _coordinatesFileBrowseButton = new javax.swing.JButton();
        _coordinateTypeGroup = new javax.swing.JPanel();
        _cartesianRadioButton = new javax.swing.JRadioButton();
        _latLongRadioButton = new javax.swing.JRadioButton();
        _gridFileLabel = new javax.swing.JLabel();
        _gridFileTextField = new javax.swing.JTextField();
        _gridFileBrowseButton = new javax.swing.JButton();
        _gridFileHintLabel = new javax.swing.JLabel();
        _advancedInputButton = new javax.swing.JButton();
        _analysisTab = new javax.swing.JPanel();
        _analysisTypeGroup = new javax.swing.JPanel();
        jLabel1 = new javax.swing.JLabel();
        jLabel2 = new javax.swing.JLabel();
        _retrospectivePurelySpatialRadioButton = new javax.swing.JRadioButton();
        _retrospectivePurelyTemporalRadioButton = new javax.swing.JRadioButton();
        _retrospectiveSpaceTimeRadioButton = new javax.swing.JRadioButton();
        _prospectivePurelyTemporalRadioButton = new javax.swing.JRadioButton();
        _prospectiveSpaceTimeRadioButton = new javax.swing.JRadioButton();
        _spatialVariationRadioButton = new javax.swing.JRadioButton();
        _spatialVariationRadioLabel = new javax.swing.JLabel();
        _retrospectiveSeasonalRadioButton = new javax.swing.JRadioButton();
        _probabilityModelGroup = new javax.swing.JPanel();
        _poissonModelRadioButton = new javax.swing.JRadioButton();
        _bernoulliModelRadioButton = new javax.swing.JRadioButton();
        _spaceTimePermutationModelRadioButton = new javax.swing.JRadioButton();
        _ordinalModelRadioButton = new javax.swing.JRadioButton();
        _exponentialModelRadioButton = new javax.swing.JRadioButton();
        _normalModelRadioButton = new javax.swing.JRadioButton();
        _categoricallModelRadioButton = new javax.swing.JRadioButton();
        _homogeneouspoissonModelRadioButton = new javax.swing.JRadioButton();
        _observableRegionsButton = new javax.swing.JButton();
        jLabel4 = new javax.swing.JLabel();
        jLabel5 = new javax.swing.JLabel();
        _uniformtimeModelRadioButton = new javax.swing.JRadioButton();
        _batchedModelRadioButton = new javax.swing.JRadioButton();
        _scanAreasGroup = new javax.swing.JPanel();
        _highRatesRadioButton = new javax.swing.JRadioButton();
        _lowRatesRadioButton = new javax.swing.JRadioButton();
        _highOrLowRatesRadioButton = new javax.swing.JRadioButton();
        _timeAggregationGroup = new javax.swing.JPanel();
        _timeAggregationUnitsLabel = new javax.swing.JLabel();
        _timeAggregationYearRadioButton = new javax.swing.JRadioButton();
        _timeAggregationMonthRadioButton = new javax.swing.JRadioButton();
        _timeAggregationDayRadioButton = new javax.swing.JRadioButton();
        _timeAggregationLengthLabel = new javax.swing.JLabel();
        _timeAggregationLengthTextField = new javax.swing.JTextField();
        _aggregrationUnitsLabel = new javax.swing.JLabel();
        _advancedAnalysisButton = new javax.swing.JButton();
        _outputTab = new javax.swing.JPanel();
        _additionalOutputFilesGroup = new javax.swing.JPanel();
        _asciiLabel = new JHyperLink("ASCII", Color.BLACK);
        ((JHyperLink)_asciiLabel).addActionListener( new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
                List<javax.swing.JCheckBox> check_boxes = Arrays.asList(
                    _clustersInColumnFormatAsciiCheckBox,
                    _clusterCaseInColumnFormatAsciiCheckBox,
                    _censusAreasReportedClustersAsciiCheckBox,
                    _relativeRiskEstimatesAreaAsciiCheckBox,
                    _simulatedLogLikelihoodRatiosAsciiCheckBox
                );
                toggleColumnOutputSettings(check_boxes);
            }
        });
        _dBaseLabel = new JHyperLink("dBase", Color.BLACK);
        ((JHyperLink)_dBaseLabel).addActionListener( new ActionListener() {
            public void actionPerformed( ActionEvent e ) {
                List<javax.swing.JCheckBox> check_boxes = Arrays.asList(
                    _clustersInColumnFormatDBaseCheckBox,
                    _clusterCaseInColumnFormatDBaseCheckBox,
                    _censusAreasReportedClustersDBaseCheckBox,
                    _relativeRiskEstimatesAreaDBaseCheckBox,
                    _simulatedLogLikelihoodRatiosDBaseCheckBox
                );
                toggleColumnOutputSettings(check_boxes);
            }
        });
        _clustersInColumnFormatLabel = new javax.swing.JLabel();
        _clustersInColumnFormatAsciiCheckBox = new javax.swing.JCheckBox();
        _clustersInColumnFormatDBaseCheckBox = new javax.swing.JCheckBox();
        _clusterCaseInColumnFormatLabel = new javax.swing.JLabel();
        _clusterCaseInColumnFormatAsciiCheckBox = new javax.swing.JCheckBox();
        _clusterCaseInColumnFormatDBaseCheckBox = new javax.swing.JCheckBox();
        _censusAreasReportedClustersLabel = new javax.swing.JLabel();
        _censusAreasReportedClustersAsciiCheckBox = new javax.swing.JCheckBox();
        _censusAreasReportedClustersDBaseCheckBox = new javax.swing.JCheckBox();
        _relativeRiskEstimatesAreaLabel = new javax.swing.JLabel();
        _relativeRiskEstimatesAreaAsciiCheckBox = new javax.swing.JCheckBox();
        _relativeRiskEstimatesAreaDBaseCheckBox = new javax.swing.JCheckBox();
        _simulatedLogLikelihoodRatiosLabel = new javax.swing.JLabel();
        _simulatedLogLikelihoodRatiosAsciiCheckBox = new javax.swing.JCheckBox();
        _simulatedLogLikelihoodRatiosDBaseCheckBox = new javax.swing.JCheckBox();
        _advancedFeaturesOutputButton = new javax.swing.JButton();
        _textOutputFormatGroup = new javax.swing.JPanel();
        _resultsFileLabel = new javax.swing.JLabel();
        _resultsFileTextField = new javax.swing.JTextField();
        _resultsFileBrowseButton = new javax.swing.JButton();
        _geographicalOutputGroup = new javax.swing.JPanel();
        _reportGoogleEarthKML = new javax.swing.JCheckBox();
        _reportShapefile = new javax.swing.JCheckBox();
        _reportGoogleMap = new javax.swing.JCheckBox();
        _reportCartesianGraph = new javax.swing.JCheckBox();

        _timePrecisionButtonGroup.add(_timePrecisionNone);
        _timePrecisionButtonGroup.add(_timePrecisionYear);
        _timePrecisionButtonGroup.add(_timePrecisionMonth);
        _timePrecisionButtonGroup.add(_timePrecisionDay);

        _coordinatesButtonGroup.add(_cartesianRadioButton);
        _coordinatesButtonGroup.add(_latLongRadioButton);

        _analysisTypeButtonGroup.add(_retrospectivePurelySpatialRadioButton);
        _analysisTypeButtonGroup.add(_retrospectivePurelyTemporalRadioButton);
        _analysisTypeButtonGroup.add(_retrospectiveSpaceTimeRadioButton);
        _analysisTypeButtonGroup.add(_retrospectiveSeasonalRadioButton);
        _analysisTypeButtonGroup.add(_spatialVariationRadioButton);
        _analysisTypeButtonGroup.add(_prospectivePurelyTemporalRadioButton);
        _analysisTypeButtonGroup.add(_prospectiveSpaceTimeRadioButton);

        _probabilityModelTypeButtonGroup.add(_poissonModelRadioButton);
        _probabilityModelTypeButtonGroup.add(_bernoulliModelRadioButton);
        _probabilityModelTypeButtonGroup.add(_spaceTimePermutationModelRadioButton);
        _probabilityModelTypeButtonGroup.add(_categoricallModelRadioButton);
        _probabilityModelTypeButtonGroup.add(_ordinalModelRadioButton);
        _probabilityModelTypeButtonGroup.add(_exponentialModelRadioButton);
        _probabilityModelTypeButtonGroup.add(_normalModelRadioButton);
        _probabilityModelTypeButtonGroup.add(_uniformtimeModelRadioButton);
        _probabilityModelTypeButtonGroup.add(_homogeneouspoissonModelRadioButton);
        _probabilityModelTypeButtonGroup.add(_batchedModelRadioButton);

        _scanAreasButtonGroup.add(_highRatesRadioButton);
        _scanAreasButtonGroup.add(_lowRatesRadioButton);
        _scanAreasButtonGroup.add(_highOrLowRatesRadioButton);

        _timeAggregationButtonGroup.add(_timeAggregationYearRadioButton);
        _timeAggregationButtonGroup.add(_timeAggregationMonthRadioButton);
        _timeAggregationButtonGroup.add(_timeAggregationDayRadioButton);

        setClosable(true);
        setDefaultCloseOperation(javax.swing.WindowConstants.DO_NOTHING_ON_CLOSE);
        setIconifiable(true);
        setResizable(true);

        _caseInputPanel.setBorder(javax.swing.BorderFactory.createEtchedBorder());

        _caseFileLabel.setLabelFor(_caseFileTextField);
        _caseFileLabel.setText("Case File:"); // NOI18N

        _caseFileTextField.getDocument().addDocumentListener(new DocumentListener() {
            public void changedUpdate(DocumentEvent e) {}
            public void removeUpdate(DocumentEvent e) {
                _advancedParametersSetting.updateMultipleDataSetsInputs();
            }
            public void insertUpdate(DocumentEvent e) {
                _advancedParametersSetting.updateMultipleDataSetsInputs();
            }
        });

        _caseFileBrowseButton.setText("..."); // NOI18N
        _caseFileBrowseButton.setToolTipText("Open Case File Import Wizard"); // NOI18N
        _caseFileBrowseButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                String key = InputSourceSettings.InputFileType.Case.toString() + "1";
                if (!_input_source_map.containsKey(key)) {
                    _input_source_map.put(key, new InputSourceSettings(InputSourceSettings.InputFileType.Case));
                }
                InputSourceSettings inputSourceSettings = (InputSourceSettings)_input_source_map.get(key);
                // invoke the FileSelectionDialog to guide user through process of selecting the source file.
                FileSelectionDialog selectionDialog = new FileSelectionDialog(SaTScanApplication.getInstance(), inputSourceSettings.getInputFileType(), SaTScanApplication.getInstance().lastBrowseDirectory);
                selectionDialog.browse_inputsource(_caseFileTextField, inputSourceSettings, ParameterSettingsFrame.this);
            }
        });

        _timePrecisionGroup.setBorder(javax.swing.BorderFactory.createTitledBorder("Time Precision"));

        _timePrecisionNone.setText("None"); // NOI18N
        _timePrecisionNone.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        _timePrecisionNone.setMargin(new java.awt.Insets(0, 0, 0, 0));
        _timePrecisionNone.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED)
                onCountTimePrecisionChange();
            }
        });

        _timePrecisionYear.setSelected(true);
        _timePrecisionYear.setText("Year"); // NOI18N
        _timePrecisionYear.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        _timePrecisionYear.setMargin(new java.awt.Insets(0, 0, 0, 0));
        _timePrecisionYear.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED)
                onCountTimePrecisionChange();
            }
        });

        _timePrecisionMonth.setText("Month"); // NOI18N
        _timePrecisionMonth.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        _timePrecisionMonth.setMargin(new java.awt.Insets(0, 0, 0, 0));
        _timePrecisionMonth.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED)
                onCountTimePrecisionChange();
            }
        });

        _timePrecisionDay.setText("Day"); // NOI18N
        _timePrecisionDay.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        _timePrecisionDay.setMargin(new java.awt.Insets(0, 0, 0, 0));
        _timePrecisionDay.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED)
                onCountTimePrecisionChange();
            }
        });

        _timePrecisionButtonGroup.add(_timePrecisionGeneric);
        _timePrecisionGeneric.setText("Generic"); // NOI18N
        _timePrecisionGeneric.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        _timePrecisionGeneric.setMargin(new java.awt.Insets(0, 0, 0, 0));
        _timePrecisionGeneric.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED)
                onCountTimePrecisionChange();
            }
        });

        javax.swing.GroupLayout _timePrecisionGroupLayout = new javax.swing.GroupLayout(_timePrecisionGroup);
        _timePrecisionGroup.setLayout(_timePrecisionGroupLayout);
        _timePrecisionGroupLayout.setHorizontalGroup(
            _timePrecisionGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_timePrecisionGroupLayout.createSequentialGroup()
                .addContainerGap()
                .addGroup(_timePrecisionGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(_timePrecisionGroupLayout.createSequentialGroup()
                        .addComponent(_timePrecisionNone)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                        .addComponent(_timePrecisionYear))
                    .addComponent(_timePrecisionGeneric)
                    .addGroup(_timePrecisionGroupLayout.createSequentialGroup()
                        .addComponent(_timePrecisionMonth)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(_timePrecisionDay)))
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );
        _timePrecisionGroupLayout.setVerticalGroup(
            _timePrecisionGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_timePrecisionGroupLayout.createSequentialGroup()
                .addGroup(_timePrecisionGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(_timePrecisionNone)
                    .addComponent(_timePrecisionYear))
                .addGap(5, 5, 5)
                .addGroup(_timePrecisionGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(_timePrecisionMonth)
                    .addComponent(_timePrecisionDay))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_timePrecisionGeneric)
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );

        _timePrecisionNone.getAccessibleContext().setAccessibleName("time precision none");
        _timePrecisionYear.getAccessibleContext().setAccessibleName("time precision Year");
        _timePrecisionMonth.getAccessibleContext().setAccessibleName("time precision Month");
        _timePrecisionDay.getAccessibleContext().setAccessibleName("time precision Day");
        _timePrecisionGeneric.getAccessibleContext().setAccessibleName("time precision Generic");

        _studyPeriodGroup.setLayout(new java.awt.CardLayout());

        _studyPeriodComplete.setBorder(javax.swing.BorderFactory.createTitledBorder("Study Period"));

        _startDateLabel.setText("Start Date:"); // NOI18N

        _startDateYearLabel.setFont(new java.awt.Font("Tahoma", 0, 10)); // NOI18N
        _startDateYearLabel.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        _startDateYearLabel.setText("Year"); // NOI18N

        _studyPeriodStartDateYearTextField.setText("2000"); // NOI18N

        _studyPeriodStartDateMonthTextField.setText("01"); // NOI18N

        _startDateMonthLabel.setFont(new java.awt.Font("Tahoma", 0, 10)); // NOI18N
        _startDateMonthLabel.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        _startDateMonthLabel.setText("Month"); // NOI18N

        _startDateDayLabel.setFont(new java.awt.Font("Tahoma", 0, 10)); // NOI18N
        _startDateDayLabel.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        _startDateDayLabel.setText("Day"); // NOI18N

        _studyPeriodStartDateDayTextField.setText("01"); // NOI18N

        _endDateLabel.setText("End Date:"); // NOI18N

        _studyPeriodEndDateYearTextField.setText("2000"); // NOI18N

        _endDateYearLabel.setFont(new java.awt.Font("Tahoma", 0, 10)); // NOI18N
        _endDateYearLabel.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        _endDateYearLabel.setText("Year"); // NOI18N

        _endDateMonthLabel.setFont(new java.awt.Font("Tahoma", 0, 10)); // NOI18N
        _endDateMonthLabel.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        _endDateMonthLabel.setText("Month"); // NOI18N

        _studyPeriodEndDateMonthTextField.setText("12"); // NOI18N

        _studyPeriodEndDateDayTextField.setText("31"); // NOI18N

        _endDateDayLabel.setFont(new java.awt.Font("Tahoma", 0, 10)); // NOI18N
        _endDateDayLabel.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        _endDateDayLabel.setText("Day"); // NOI18N

        javax.swing.GroupLayout _studyPeriodCompleteLayout = new javax.swing.GroupLayout(_studyPeriodComplete);
        _studyPeriodComplete.setLayout(_studyPeriodCompleteLayout);
        _studyPeriodCompleteLayout.setHorizontalGroup(
            _studyPeriodCompleteLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_studyPeriodCompleteLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(_startDateLabel)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(_studyPeriodCompleteLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(_startDateYearLabel, javax.swing.GroupLayout.PREFERRED_SIZE, 50, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(_studyPeriodStartDateYearTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 60, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(_studyPeriodCompleteLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(_startDateMonthLabel)
                    .addComponent(_studyPeriodStartDateMonthTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 40, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(_studyPeriodCompleteLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                    .addComponent(_startDateDayLabel, javax.swing.GroupLayout.Alignment.TRAILING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(_studyPeriodStartDateDayTextField, javax.swing.GroupLayout.Alignment.TRAILING, javax.swing.GroupLayout.PREFERRED_SIZE, 40, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addGap(50, 50, 50)
                .addComponent(_endDateLabel)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(_studyPeriodCompleteLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(_endDateYearLabel, javax.swing.GroupLayout.PREFERRED_SIZE, 50, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(_studyPeriodEndDateYearTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 60, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(_studyPeriodCompleteLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(_endDateMonthLabel)
                    .addComponent(_studyPeriodEndDateMonthTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 40, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(_studyPeriodCompleteLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                    .addComponent(_endDateDayLabel, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(_studyPeriodEndDateDayTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 40, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addContainerGap(130, Short.MAX_VALUE))
        );
        _studyPeriodCompleteLayout.setVerticalGroup(
            _studyPeriodCompleteLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_studyPeriodCompleteLayout.createSequentialGroup()
                .addGroup(_studyPeriodCompleteLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(_startDateYearLabel)
                    .addComponent(_startDateMonthLabel)
                    .addComponent(_endDateMonthLabel)
                    .addComponent(_endDateDayLabel)
                    .addComponent(_startDateDayLabel)
                    .addComponent(_endDateYearLabel))
                .addGap(0, 0, 0)
                .addGroup(_studyPeriodCompleteLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(_studyPeriodStartDateYearTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(_studyPeriodStartDateMonthTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(_studyPeriodStartDateDayTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(_endDateLabel)
                    .addComponent(_studyPeriodEndDateYearTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(_studyPeriodEndDateMonthTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(_startDateLabel)
                    .addComponent(_studyPeriodEndDateDayTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );

        _studyPeriodStartDateYearTextField.getAccessibleContext().setAccessibleName("Study Period Start Date Year");
        _studyPeriodStartDateMonthTextField.getAccessibleContext().setAccessibleName("Study Period Start Date Month");
        _studyPeriodStartDateMonthTextField.getAccessibleContext().setAccessibleDescription("");
        _studyPeriodStartDateDayTextField.getAccessibleContext().setAccessibleName("Study Period Start Date Day");
        _studyPeriodEndDateYearTextField.getAccessibleContext().setAccessibleName("Study Period End Date Year");
        _studyPeriodEndDateMonthTextField.getAccessibleContext().setAccessibleName("Study Period End Date Month");
        _studyPeriodEndDateDayTextField.getAccessibleContext().setAccessibleName("Study Period End Date Day");
        _studyPeriodEndDateDayTextField.getAccessibleContext().setAccessibleDescription("");

        _studyPeriodGroup.add(_studyPeriodComplete, "study_complete");

        _studyPeriodGeneric.setBorder(javax.swing.BorderFactory.createTitledBorder("Study Period"));

        _startDateLabel1.setText("Start Time:"); // NOI18N

        _startDateYearLabel1.setFont(new java.awt.Font("Tahoma", 0, 10)); // NOI18N
        _startDateYearLabel1.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        _startDateYearLabel1.setText("Generic Unit"); // NOI18N

        _studyPeriodStartDateGenericTextField.setText("0"); // NOI18N
        _studyPeriodStartDateGenericTextField.getDocument().addUndoableEditListener(new UndoableEditListener() {
            public void undoableEditHappened(UndoableEditEvent evt) {
                undo.addEdit(evt.getEdit());
            }
        });
        _studyPeriodStartDateGenericTextField.addKeyListener(new java.awt.event.KeyAdapter() {
            public void keyTyped(java.awt.event.KeyEvent e) {
                Utils.validateNumericKeyTyped(_studyPeriodStartDateGenericTextField, e, 10);
            }
        });
        _studyPeriodStartDateGenericTextField.addFocusListener(new java.awt.event.FocusAdapter() {
            public void focusLost(java.awt.event.FocusEvent e) {
                if (ParameterSettingsFrame.this.getPrecisionOfTimesControlType() == Parameters.DatePrecisionType.GENERIC) {
                    Utils.validateGenericDateField(_studyPeriodStartDateGenericTextField, undo);
                }
            }
        });

        _endDateLabel1.setText("End Time:"); // NOI18N

        _studyPeriodEndDateGenericTextField.setText("31"); // NOI18N
        _studyPeriodEndDateGenericTextField.getDocument().addUndoableEditListener(new UndoableEditListener() {
            public void undoableEditHappened(UndoableEditEvent evt) {
                undo.addEdit(evt.getEdit());
            }
        });
        _studyPeriodEndDateGenericTextField.addKeyListener(new java.awt.event.KeyAdapter() {
            public void keyTyped(java.awt.event.KeyEvent e) {
                Utils.validateNumericKeyTyped(_studyPeriodEndDateGenericTextField, e, 10);
            }
        });
        _studyPeriodEndDateGenericTextField.addFocusListener(new java.awt.event.FocusAdapter() {
            public void focusLost(java.awt.event.FocusEvent e) {
                if (ParameterSettingsFrame.this.getPrecisionOfTimesControlType() == Parameters.DatePrecisionType.GENERIC) {
                    Utils.validateGenericDateField(_studyPeriodEndDateGenericTextField, undo);
                }
            }
        });

        _endDateYearLabel1.setFont(new java.awt.Font("Tahoma", 0, 10)); // NOI18N
        _endDateYearLabel1.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        _endDateYearLabel1.setText("Generic Unit"); // NOI18N

        javax.swing.GroupLayout _studyPeriodGenericLayout = new javax.swing.GroupLayout(_studyPeriodGeneric);
        _studyPeriodGeneric.setLayout(_studyPeriodGenericLayout);
        _studyPeriodGenericLayout.setHorizontalGroup(
            _studyPeriodGenericLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_studyPeriodGenericLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(_startDateLabel1)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(_studyPeriodGenericLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                    .addComponent(_startDateYearLabel1, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(_studyPeriodStartDateGenericTextField, javax.swing.GroupLayout.DEFAULT_SIZE, 87, Short.MAX_VALUE))
                .addGap(85, 85, 85)
                .addComponent(_endDateLabel1)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(_studyPeriodGenericLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                    .addComponent(_endDateYearLabel1, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(_studyPeriodEndDateGenericTextField, javax.swing.GroupLayout.DEFAULT_SIZE, 80, Short.MAX_VALUE))
                .addContainerGap(234, Short.MAX_VALUE))
        );
        _studyPeriodGenericLayout.setVerticalGroup(
            _studyPeriodGenericLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_studyPeriodGenericLayout.createSequentialGroup()
                .addGroup(_studyPeriodGenericLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(_startDateYearLabel1)
                    .addComponent(_endDateYearLabel1))
                .addGap(0, 0, 0)
                .addGroup(_studyPeriodGenericLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(_studyPeriodStartDateGenericTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(_endDateLabel1)
                    .addComponent(_studyPeriodEndDateGenericTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(_startDateLabel1))
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );

        _studyPeriodStartDateGenericTextField.getAccessibleContext().setAccessibleName("Generic units start time");
        _studyPeriodEndDateGenericTextField.getAccessibleContext().setAccessibleName("Generic units end time");

        _studyPeriodGroup.add(_studyPeriodGeneric, "study_generic");

        _controlFileLabel.setLabelFor(_controlFileTextField);
        _controlFileLabel.setText("Control File:"); // NOI18N

        _controlFileTextField.getDocument().addDocumentListener(new DocumentListener() {
            public void changedUpdate(DocumentEvent e) {}
            public void removeUpdate(DocumentEvent e) {
                _advancedParametersSetting.updateMultipleDataSetsInputs();
            }
            public void insertUpdate(DocumentEvent e) {
                _advancedParametersSetting.updateMultipleDataSetsInputs();
            }
        });

        _controlFileBrowseButton.setText("..."); // NOI18N
        _controlFileBrowseButton.setToolTipText("Open Control File Import Wizard"); // NOI18N
        _controlFileBrowseButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                String key = InputSourceSettings.InputFileType.Control.toString() + "1";
                if (!_input_source_map.containsKey(key)) {
                    _input_source_map.put(key, new InputSourceSettings(InputSourceSettings.InputFileType.Control));
                }
                InputSourceSettings inputSourceSettings = (InputSourceSettings)_input_source_map.get(key);
                // invoke the FileSelectionDialog to guide user through process of selecting the source file.
                FileSelectionDialog selectionDialog = new FileSelectionDialog(SaTScanApplication.getInstance(), inputSourceSettings.getInputFileType(), SaTScanApplication.getInstance().lastBrowseDirectory);
                selectionDialog.browse_inputsource(_controlFileTextField, inputSourceSettings, ParameterSettingsFrame.this);
            }
        });

        _bernoulliModelHintLabel.setText("(Bernoulli Model)"); // NOI18N

        javax.swing.GroupLayout _caseInputPanelLayout = new javax.swing.GroupLayout(_caseInputPanel);
        _caseInputPanel.setLayout(_caseInputPanelLayout);
        _caseInputPanelLayout.setHorizontalGroup(
            _caseInputPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, _caseInputPanelLayout.createSequentialGroup()
                .addContainerGap()
                .addGroup(_caseInputPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING)
                    .addComponent(_studyPeriodGroup, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addGroup(_caseInputPanelLayout.createSequentialGroup()
                        .addGroup(_caseInputPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addGroup(_caseInputPanelLayout.createSequentialGroup()
                                .addGroup(_caseInputPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                                    .addComponent(_caseFileLabel)
                                    .addGroup(_caseInputPanelLayout.createSequentialGroup()
                                        .addComponent(_controlFileLabel)
                                        .addGap(60, 60, 60)
                                        .addComponent(_bernoulliModelHintLabel)))
                                .addGap(0, 0, Short.MAX_VALUE))
                            .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, _caseInputPanelLayout.createSequentialGroup()
                                .addGroup(_caseInputPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING)
                                    .addComponent(_controlFileTextField)
                                    .addComponent(_caseFileTextField))
                                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                .addGroup(_caseInputPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                                    .addComponent(_caseFileBrowseButton, javax.swing.GroupLayout.Alignment.TRAILING, javax.swing.GroupLayout.PREFERRED_SIZE, 25, javax.swing.GroupLayout.PREFERRED_SIZE)
                                    .addComponent(_controlFileBrowseButton, javax.swing.GroupLayout.Alignment.TRAILING, javax.swing.GroupLayout.PREFERRED_SIZE, 25, javax.swing.GroupLayout.PREFERRED_SIZE))))
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(_timePrecisionGroup, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)))
                .addContainerGap())
        );
        _caseInputPanelLayout.setVerticalGroup(
            _caseInputPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_caseInputPanelLayout.createSequentialGroup()
                .addContainerGap()
                .addGroup(_caseInputPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(_caseInputPanelLayout.createSequentialGroup()
                        .addComponent(_caseFileLabel)
                        .addGap(0, 0, 0)
                        .addGroup(_caseInputPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                            .addComponent(_caseFileBrowseButton)
                            .addComponent(_caseFileTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addGroup(_caseInputPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                            .addComponent(_controlFileLabel)
                            .addComponent(_bernoulliModelHintLabel))
                        .addGap(0, 0, 0)
                        .addGroup(_caseInputPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                            .addComponent(_controlFileBrowseButton)
                            .addComponent(_controlFileTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)))
                    .addComponent(_timePrecisionGroup, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_studyPeriodGroup, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .addContainerGap())
        );

        _caseFileBrowseButton.getAccessibleContext().setAccessibleName("");
        _timePrecisionGroup.getAccessibleContext().setAccessibleName("Time Precision Selection");
        _controlFileBrowseButton.getAccessibleContext().setAccessibleName("");

        _populationFileLabel.setLabelFor(_populationFileTextField);
        _populationFileLabel.setText("Population File:"); // NOI18N

        _populationFileTextField.getDocument().addDocumentListener(new DocumentListener() {
            public void changedUpdate(DocumentEvent e) {}
            public void removeUpdate(DocumentEvent e) {
                _advancedParametersSetting.updateMultipleDataSetsInputs();
            }
            public void insertUpdate(DocumentEvent e) {
                _advancedParametersSetting.updateMultipleDataSetsInputs();
            }
        });

        _populationFileBrowseButton.setText("..."); // NOI18N
        _populationFileBrowseButton.setToolTipText("Open Population File Import Wizard"); // NOI18N
        _populationFileBrowseButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                String key = InputSourceSettings.InputFileType.Population.toString() + "1";
                if (!_input_source_map.containsKey(key)) {
                    _input_source_map.put(key, new InputSourceSettings(InputSourceSettings.InputFileType.Population));
                }
                InputSourceSettings inputSourceSettings = (InputSourceSettings)_input_source_map.get(key);
                // invoke the FileSelectionDialog to guide user through process of selecting the source file.
                FileSelectionDialog selectionDialog = new FileSelectionDialog(SaTScanApplication.getInstance(), inputSourceSettings.getInputFileType(), SaTScanApplication.getInstance().lastBrowseDirectory);
                selectionDialog.browse_inputsource(_populationFileTextField, inputSourceSettings, ParameterSettingsFrame.this);
            }
        });

        _poissionModelHintLabel.setText("(Poisson Model)"); // NOI18N

        javax.swing.GroupLayout _populationInputPanelLayout = new javax.swing.GroupLayout(_populationInputPanel);
        _populationInputPanel.setLayout(_populationInputPanelLayout);
        _populationInputPanelLayout.setHorizontalGroup(
            _populationInputPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_populationInputPanelLayout.createSequentialGroup()
                .addContainerGap()
                .addGroup(_populationInputPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(_populationInputPanelLayout.createSequentialGroup()
                        .addComponent(_populationFileTextField)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(_populationFileBrowseButton, javax.swing.GroupLayout.PREFERRED_SIZE, 25, javax.swing.GroupLayout.PREFERRED_SIZE))
                    .addGroup(_populationInputPanelLayout.createSequentialGroup()
                        .addComponent(_populationFileLabel)
                        .addGap(50, 50, 50)
                        .addComponent(_poissionModelHintLabel)))
                .addGap(141, 141, 141))
        );
        _populationInputPanelLayout.setVerticalGroup(
            _populationInputPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_populationInputPanelLayout.createSequentialGroup()
                .addGroup(_populationInputPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(_populationFileLabel)
                    .addComponent(_poissionModelHintLabel))
                .addGap(0, 0, 0)
                .addGroup(_populationInputPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(_populationFileTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(_populationFileBrowseButton))
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );

        _populationFileBrowseButton.getAccessibleContext().setAccessibleName("");

        _geographicalInputPanel.setBorder(javax.swing.BorderFactory.createEtchedBorder());

        _coordinatesFileLabel.setLabelFor(_coordiantesFileTextField);
        _coordinatesFileLabel.setText("Coordinates File:"); // NOI18N

        _coordinatesFileBrowseButton.setText("..."); // NOI18N
        _coordinatesFileBrowseButton.setToolTipText("Open Coordinates File Import Wizard"); // NOI18N
        _coordinatesFileBrowseButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                String key = InputSourceSettings.InputFileType.Coordinates.toString() + "1";
                if (!_input_source_map.containsKey(key)) {
                    _input_source_map.put(key, new InputSourceSettings(InputSourceSettings.InputFileType.Coordinates));
                }
                InputSourceSettings inputSourceSettings = (InputSourceSettings)_input_source_map.get(key);
                // invoke the FileSelectionDialog to guide user through process of selecting the source file.
                FileSelectionDialog selectionDialog = new FileSelectionDialog(SaTScanApplication.getInstance(), inputSourceSettings.getInputFileType(), SaTScanApplication.getInstance().lastBrowseDirectory);
                selectionDialog.browse_inputsource(_coordiantesFileTextField, inputSourceSettings, ParameterSettingsFrame.this);
            }
        });

        _coordinateTypeGroup.setBorder(javax.swing.BorderFactory.createTitledBorder("Coordinates"));

        _cartesianRadioButton.setSelected(true);
        _cartesianRadioButton.setText("Cartesian"); // NOI18N
        _cartesianRadioButton.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        _cartesianRadioButton.setMargin(new java.awt.Insets(0, 0, 0, 0));
        _cartesianRadioButton.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED) {
                    ParameterSettingsFrame.this.enableModelControlForAnalysisType();
                    ParameterSettingsFrame.this.enableSettingsForAnalysisModelCombination();
                    getAdvancedParameterInternalFrame().setSpatialDistanceCaption();
                    getAdvancedParameterInternalFrame().enableSettingsForAnalysisModelCombination();
                }
            }
        });

        _latLongRadioButton.setText("Lat/Long"); // NOI18N
        _latLongRadioButton.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        _latLongRadioButton.setMargin(new java.awt.Insets(0, 0, 0, 0));
        _latLongRadioButton.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED) {
                    ParameterSettingsFrame.this.enableSettingsForAnalysisModelCombination();
                    ParameterSettingsFrame.this.enableModelControlForAnalysisType();
                    getAdvancedParameterInternalFrame().setSpatialDistanceCaption();
                    getAdvancedParameterInternalFrame().enableSettingsForAnalysisModelCombination();
                }
            }
        });

        javax.swing.GroupLayout _coordinateTypeGroupLayout = new javax.swing.GroupLayout(_coordinateTypeGroup);
        _coordinateTypeGroup.setLayout(_coordinateTypeGroupLayout);
        _coordinateTypeGroupLayout.setHorizontalGroup(
            _coordinateTypeGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_coordinateTypeGroupLayout.createSequentialGroup()
                .addContainerGap()
                .addGroup(_coordinateTypeGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(_latLongRadioButton)
                    .addComponent(_cartesianRadioButton))
                .addContainerGap(36, Short.MAX_VALUE))
        );
        _coordinateTypeGroupLayout.setVerticalGroup(
            _coordinateTypeGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_coordinateTypeGroupLayout.createSequentialGroup()
                .addComponent(_cartesianRadioButton)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addComponent(_latLongRadioButton)
                .addContainerGap(22, Short.MAX_VALUE))
        );

        _cartesianRadioButton.getAccessibleContext().setAccessibleName("Cartesian coordinates");
        _latLongRadioButton.getAccessibleContext().setAccessibleName("Latitude/Longitude coordinates");

        _gridFileLabel.setLabelFor(_gridFileTextField);
        _gridFileLabel.setText("Grid File:"); // NOI18N

        _gridFileBrowseButton.setText("..."); // NOI18N
        _gridFileBrowseButton.setToolTipText("Open Grid File Import Wizard"); // NOI18N
        _gridFileBrowseButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                String key = InputSourceSettings.InputFileType.SpecialGrid.toString() + "1";
                if (!_input_source_map.containsKey(key)) {
                    _input_source_map.put(key, new InputSourceSettings(InputSourceSettings.InputFileType.SpecialGrid));
                }
                InputSourceSettings inputSourceSettings = (InputSourceSettings)_input_source_map.get(key);
                // invoke the FileSelectionDialog to guide user through process of selecting the source file.
                FileSelectionDialog selectionDialog = new FileSelectionDialog(SaTScanApplication.getInstance(), inputSourceSettings.getInputFileType(), SaTScanApplication.getInstance().lastBrowseDirectory);
                selectionDialog.browse_inputsource(_gridFileTextField, inputSourceSettings, ParameterSettingsFrame.this);
            }
        });

        _gridFileHintLabel.setText("(optional)"); // NOI18N

        javax.swing.GroupLayout _geographicalInputPanelLayout = new javax.swing.GroupLayout(_geographicalInputPanel);
        _geographicalInputPanel.setLayout(_geographicalInputPanelLayout);
        _geographicalInputPanelLayout.setHorizontalGroup(
            _geographicalInputPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_geographicalInputPanelLayout.createSequentialGroup()
                .addContainerGap()
                .addGroup(_geographicalInputPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(_geographicalInputPanelLayout.createSequentialGroup()
                        .addGroup(_geographicalInputPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addComponent(_coordinatesFileLabel)
                            .addGroup(_geographicalInputPanelLayout.createSequentialGroup()
                                .addComponent(_gridFileLabel)
                                .addGap(83, 83, 83)
                                .addComponent(_gridFileHintLabel)))
                        .addGap(0, 0, Short.MAX_VALUE))
                    .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, _geographicalInputPanelLayout.createSequentialGroup()
                        .addGroup(_geographicalInputPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING)
                            .addComponent(_gridFileTextField)
                            .addComponent(_coordiantesFileTextField))
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addGroup(_geographicalInputPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addComponent(_coordinatesFileBrowseButton, javax.swing.GroupLayout.Alignment.TRAILING, javax.swing.GroupLayout.PREFERRED_SIZE, 25, javax.swing.GroupLayout.PREFERRED_SIZE)
                            .addComponent(_gridFileBrowseButton, javax.swing.GroupLayout.Alignment.TRAILING, javax.swing.GroupLayout.PREFERRED_SIZE, 25, javax.swing.GroupLayout.PREFERRED_SIZE))))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_coordinateTypeGroup, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap())
        );
        _geographicalInputPanelLayout.setVerticalGroup(
            _geographicalInputPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_geographicalInputPanelLayout.createSequentialGroup()
                .addContainerGap()
                .addGroup(_geographicalInputPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(_coordinateTypeGroup, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addGroup(_geographicalInputPanelLayout.createSequentialGroup()
                        .addComponent(_coordinatesFileLabel)
                        .addGap(0, 0, 0)
                        .addGroup(_geographicalInputPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                            .addComponent(_coordiantesFileTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                            .addComponent(_coordinatesFileBrowseButton))
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addGroup(_geographicalInputPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                            .addComponent(_gridFileLabel)
                            .addComponent(_gridFileHintLabel))
                        .addGap(0, 0, 0)
                        .addGroup(_geographicalInputPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                            .addComponent(_gridFileTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                            .addComponent(_gridFileBrowseButton))))
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );

        _coordinatesFileBrowseButton.getAccessibleContext().setAccessibleName("");
        _coordinateTypeGroup.getAccessibleContext().setAccessibleName("Coordinates Type Selection");
        _gridFileBrowseButton.getAccessibleContext().setAccessibleName("");

        _advancedInputButton.setText("Advanced >>"); // NOI18N
        _advancedInputButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                getAdvancedParameterInternalFrame().setVisible(true, AdvancedParameterSettingsFrame.FocusedTabSet.INPUT);
                getAdvancedParameterInternalFrame().requestFocus();
            }
        });

        javax.swing.GroupLayout _inputTabLayout = new javax.swing.GroupLayout(_inputTab);
        _inputTab.setLayout(_inputTabLayout);
        _inputTabLayout.setHorizontalGroup(
            _inputTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, _inputTabLayout.createSequentialGroup()
                .addContainerGap()
                .addGroup(_inputTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING)
                    .addComponent(_caseInputPanel, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(_advancedInputButton)
                    .addComponent(_geographicalInputPanel, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(_populationInputPanel, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                .addContainerGap())
        );
        _inputTabLayout.setVerticalGroup(
            _inputTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_inputTabLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(_caseInputPanel, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_populationInputPanel, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_geographicalInputPanel, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED, 80, Short.MAX_VALUE)
                .addComponent(_advancedInputButton)
                .addContainerGap())
        );

        _advancedInputButton.getAccessibleContext().setAccessibleName("Advanced Input Features");

        _tabbedPane.addTab("Input", _inputTab);
        _inputTab.getAccessibleContext().setAccessibleName("Input tab");

        _analysisTypeGroup.setBorder(javax.swing.BorderFactory.createTitledBorder("Type of Analysis"));

        jLabel1.setText("Retrospective Analyses:"); // NOI18N

        jLabel2.setText("Prospective Analyses:"); // NOI18N

        _retrospectivePurelySpatialRadioButton.setSelected(true);
        _retrospectivePurelySpatialRadioButton.setText("Purely Spatial"); // NOI18N
        _retrospectivePurelySpatialRadioButton.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        _retrospectivePurelySpatialRadioButton.setMargin(new java.awt.Insets(0, 0, 0, 0));
        _retrospectivePurelySpatialRadioButton.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED) {
                    enableModelControlForAnalysisType();
                    enableSettingsForAnalysisModelCombination();
                }
            }
        });

        _retrospectivePurelyTemporalRadioButton.setText("Purely Temporal"); // NOI18N
        _retrospectivePurelyTemporalRadioButton.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        _retrospectivePurelyTemporalRadioButton.setMargin(new java.awt.Insets(0, 0, 0, 0));
        _retrospectivePurelyTemporalRadioButton.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED) {
                    enableModelControlForAnalysisType();
                    enableSettingsForAnalysisModelCombination();
                }
            }
        });

        _retrospectiveSpaceTimeRadioButton.setText("Space-Time"); // NOI18N
        _retrospectiveSpaceTimeRadioButton.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        _retrospectiveSpaceTimeRadioButton.setMargin(new java.awt.Insets(0, 0, 0, 0));
        _retrospectiveSpaceTimeRadioButton.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED) {
                    enableModelControlForAnalysisType();
                    enableSettingsForAnalysisModelCombination();
                }
            }
        });

        _prospectivePurelyTemporalRadioButton.setText("Purely Temporal"); // NOI18N
        _prospectivePurelyTemporalRadioButton.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        _prospectivePurelyTemporalRadioButton.setMargin(new java.awt.Insets(0, 0, 0, 0));
        _prospectivePurelyTemporalRadioButton.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED) {
                    enableModelControlForAnalysisType();
                    enableSettingsForAnalysisModelCombination();
                }
            }
        });

        _prospectiveSpaceTimeRadioButton.setText("Space-Time"); // NOI18N
        _prospectiveSpaceTimeRadioButton.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        _prospectiveSpaceTimeRadioButton.setMargin(new java.awt.Insets(0, 0, 0, 0));
        _prospectiveSpaceTimeRadioButton.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED) {
                    enableModelControlForAnalysisType();
                    enableSettingsForAnalysisModelCombination();
                }
            }
        });

        _analysisTypeButtonGroup.add(_spatialVariationRadioButton);
        _spatialVariationRadioButton.setText("Spatial Variation"); // NOI18N
        _spatialVariationRadioButton.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        _spatialVariationRadioButton.setMargin(new java.awt.Insets(0, 0, 0, 0));
        _spatialVariationRadioButton.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED) {
                    enableModelControlForAnalysisType();
                    enableSettingsForAnalysisModelCombination();
                }
            }
        });

        _spatialVariationRadioLabel.setText("in Temporal Trends");

        _retrospectiveSeasonalRadioButton.setText("Seasonal"); // NOI18N
        _retrospectiveSeasonalRadioButton.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        _retrospectiveSeasonalRadioButton.setMargin(new java.awt.Insets(0, 0, 0, 0));
        _retrospectiveSeasonalRadioButton.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED) {
                    enableModelControlForAnalysisType();
                    enableSettingsForAnalysisModelCombination();
                }
            }
        });

        javax.swing.GroupLayout _analysisTypeGroupLayout = new javax.swing.GroupLayout(_analysisTypeGroup);
        _analysisTypeGroup.setLayout(_analysisTypeGroupLayout);
        _analysisTypeGroupLayout.setHorizontalGroup(
            _analysisTypeGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_analysisTypeGroupLayout.createSequentialGroup()
                .addGroup(_analysisTypeGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(_analysisTypeGroupLayout.createSequentialGroup()
                        .addGap(37, 37, 37)
                        .addComponent(_spatialVariationRadioLabel, javax.swing.GroupLayout.DEFAULT_SIZE, 197, Short.MAX_VALUE))
                    .addGroup(_analysisTypeGroupLayout.createSequentialGroup()
                        .addContainerGap()
                        .addGroup(_analysisTypeGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addComponent(jLabel1)
                            .addGroup(_analysisTypeGroupLayout.createSequentialGroup()
                                .addGap(10, 10, 10)
                                .addGroup(_analysisTypeGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                                    .addComponent(_retrospectivePurelySpatialRadioButton)
                                    .addComponent(_retrospectivePurelyTemporalRadioButton)
                                    .addComponent(_retrospectiveSpaceTimeRadioButton)
                                    .addComponent(_spatialVariationRadioButton)
                                    .addComponent(_retrospectiveSeasonalRadioButton))))
                        .addGap(31, 31, 31)))
                .addContainerGap())
            .addGroup(_analysisTypeGroupLayout.createSequentialGroup()
                .addContainerGap()
                .addGroup(_analysisTypeGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(jLabel2)
                    .addGroup(_analysisTypeGroupLayout.createSequentialGroup()
                        .addGap(10, 10, 10)
                        .addGroup(_analysisTypeGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addComponent(_prospectiveSpaceTimeRadioButton)
                            .addComponent(_prospectivePurelyTemporalRadioButton))))
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );
        _analysisTypeGroupLayout.setVerticalGroup(
            _analysisTypeGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_analysisTypeGroupLayout.createSequentialGroup()
                .addComponent(jLabel1, javax.swing.GroupLayout.PREFERRED_SIZE, 14, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_retrospectivePurelySpatialRadioButton)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addComponent(_retrospectivePurelyTemporalRadioButton)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addComponent(_retrospectiveSpaceTimeRadioButton)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addComponent(_retrospectiveSeasonalRadioButton)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addComponent(_spatialVariationRadioButton, javax.swing.GroupLayout.PREFERRED_SIZE, 22, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addGap(1, 1, 1)
                .addComponent(_spatialVariationRadioLabel)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addComponent(jLabel2)
                .addGap(10, 10, 10)
                .addComponent(_prospectivePurelyTemporalRadioButton)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addComponent(_prospectiveSpaceTimeRadioButton)
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );

        _retrospectivePurelySpatialRadioButton.getAccessibleContext().setAccessibleName("Retrospective Purely Spatial");
        _retrospectivePurelyTemporalRadioButton.getAccessibleContext().setAccessibleName("Retrospective Purely Temporal");
        _retrospectiveSpaceTimeRadioButton.getAccessibleContext().setAccessibleName("Retrospective Space-Time");
        _prospectivePurelyTemporalRadioButton.getAccessibleContext().setAccessibleName("Prospective Purely Temporal");
        _prospectiveSpaceTimeRadioButton.getAccessibleContext().setAccessibleName("Prospective Space-Time");
        _spatialVariationRadioButton.getAccessibleContext().setAccessibleName("Retrospective Spatial Variation in Temporal Trends");
        _retrospectiveSeasonalRadioButton.getAccessibleContext().setAccessibleName("Retrospective Seasonal");

        _probabilityModelGroup.setBorder(javax.swing.BorderFactory.createTitledBorder("Probability Model"));

        _probabilityModelTypeButtonGroup.add(_poissonModelRadioButton);
        _poissonModelRadioButton.setSelected(true);
        _poissonModelRadioButton.setText("Poisson"); // NOI18N
        _poissonModelRadioButton.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        _poissonModelRadioButton.setMargin(new java.awt.Insets(0, 0, 0, 0));
        _poissonModelRadioButton.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED)
                onProbabilityModelTypeClick();
            }
        });

        _probabilityModelTypeButtonGroup.add(_bernoulliModelRadioButton);
        _bernoulliModelRadioButton.setText("Bernoulli"); // NOI18N
        _bernoulliModelRadioButton.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        _bernoulliModelRadioButton.setMargin(new java.awt.Insets(0, 0, 0, 0));
        _bernoulliModelRadioButton.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED)
                onProbabilityModelTypeClick();
            }
        });

        _probabilityModelTypeButtonGroup.add(_spaceTimePermutationModelRadioButton);
        _spaceTimePermutationModelRadioButton.setText("Space-Time Permutation"); // NOI18N
        _spaceTimePermutationModelRadioButton.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        _spaceTimePermutationModelRadioButton.setMargin(new java.awt.Insets(0, 0, 0, 0));
        _spaceTimePermutationModelRadioButton.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED)
                onProbabilityModelTypeClick();
            }
        });

        _probabilityModelTypeButtonGroup.add(_ordinalModelRadioButton);
        _ordinalModelRadioButton.setText("Ordinal"); // NOI18N
        _ordinalModelRadioButton.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        _ordinalModelRadioButton.setMargin(new java.awt.Insets(0, 0, 0, 0));
        _ordinalModelRadioButton.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED)
                onProbabilityModelTypeClick();
            }
        });

        _probabilityModelTypeButtonGroup.add(_exponentialModelRadioButton);
        _exponentialModelRadioButton.setText("Exponential"); // NOI18N
        _exponentialModelRadioButton.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        _exponentialModelRadioButton.setMargin(new java.awt.Insets(0, 0, 0, 0));
        _exponentialModelRadioButton.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED)
                onProbabilityModelTypeClick();
            }
        });

        _probabilityModelTypeButtonGroup.add(_normalModelRadioButton);
        _normalModelRadioButton.setText("Normal"); // NOI18N
        _normalModelRadioButton.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        _normalModelRadioButton.setMargin(new java.awt.Insets(0, 0, 0, 0));
        _normalModelRadioButton.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED)
                onProbabilityModelTypeClick();
            }
        });

        _probabilityModelTypeButtonGroup.add(_categoricallModelRadioButton);
        _categoricallModelRadioButton.setText("Multinomial"); // NOI18N
        _categoricallModelRadioButton.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        _categoricallModelRadioButton.setMargin(new java.awt.Insets(0, 0, 0, 0));
        _categoricallModelRadioButton.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED)
                onProbabilityModelTypeClick();
            }
        });

        _probabilityModelTypeButtonGroup.add(_homogeneouspoissonModelRadioButton);
        _homogeneouspoissonModelRadioButton.setText("Poisson"); // NOI18N
        _homogeneouspoissonModelRadioButton.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        _homogeneouspoissonModelRadioButton.setMargin(new java.awt.Insets(0, 0, 0, 0));
        _homogeneouspoissonModelRadioButton.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED)
                onProbabilityModelTypeClick();
            }
        });

        _observableRegionsButton.setText("_defineRegionsButton");
        _observableRegionsButton.setToolTipText("Define polygons ...");
        _observableRegionsButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                geObservableRegionsParameterInternalFrame().setVisible(true);
            }
        });

        jLabel4.setText("Continuous Scan Statistics:");

        jLabel5.setText("Discrete Scan Statistics:");

        _probabilityModelTypeButtonGroup.add(_uniformtimeModelRadioButton);
        _uniformtimeModelRadioButton.setText("Uniform Time"); // NOI18N
        _uniformtimeModelRadioButton.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        _uniformtimeModelRadioButton.setMargin(new java.awt.Insets(0, 0, 0, 0));
        _uniformtimeModelRadioButton.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED)
                onProbabilityModelTypeClick();
            }
        });

        _probabilityModelTypeButtonGroup.add(_batchedModelRadioButton);
        _batchedModelRadioButton.setText("Batched"); // NOI18N
        _batchedModelRadioButton.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        _batchedModelRadioButton.setMargin(new java.awt.Insets(0, 0, 0, 0));
        _batchedModelRadioButton.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED)
                onProbabilityModelTypeClick();
            }
        });

        javax.swing.GroupLayout _probabilityModelGroupLayout = new javax.swing.GroupLayout(_probabilityModelGroup);
        _probabilityModelGroup.setLayout(_probabilityModelGroupLayout);
        _probabilityModelGroupLayout.setHorizontalGroup(
            _probabilityModelGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_probabilityModelGroupLayout.createSequentialGroup()
                .addContainerGap()
                .addGroup(_probabilityModelGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(_probabilityModelGroupLayout.createSequentialGroup()
                        .addGroup(_probabilityModelGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addComponent(jLabel5)
                            .addComponent(jLabel4))
                        .addGap(0, 0, Short.MAX_VALUE))
                    .addGroup(_probabilityModelGroupLayout.createSequentialGroup()
                        .addGap(10, 10, 10)
                        .addGroup(_probabilityModelGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addComponent(_batchedModelRadioButton, javax.swing.GroupLayout.Alignment.TRAILING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                            .addComponent(_uniformtimeModelRadioButton, javax.swing.GroupLayout.Alignment.TRAILING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                            .addComponent(_normalModelRadioButton, javax.swing.GroupLayout.Alignment.TRAILING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                            .addComponent(_exponentialModelRadioButton, javax.swing.GroupLayout.Alignment.TRAILING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                            .addComponent(_ordinalModelRadioButton, javax.swing.GroupLayout.Alignment.TRAILING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                            .addComponent(_categoricallModelRadioButton, javax.swing.GroupLayout.Alignment.TRAILING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                            .addComponent(_bernoulliModelRadioButton, javax.swing.GroupLayout.Alignment.TRAILING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                            .addComponent(_poissonModelRadioButton, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                            .addComponent(_spaceTimePermutationModelRadioButton, javax.swing.GroupLayout.DEFAULT_SIZE, 186, Short.MAX_VALUE)
                            .addGroup(_probabilityModelGroupLayout.createSequentialGroup()
                                .addComponent(_homogeneouspoissonModelRadioButton, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                .addComponent(_observableRegionsButton, javax.swing.GroupLayout.PREFERRED_SIZE, 21, javax.swing.GroupLayout.PREFERRED_SIZE)))))
                .addContainerGap())
        );
        _probabilityModelGroupLayout.setVerticalGroup(
            _probabilityModelGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_probabilityModelGroupLayout.createSequentialGroup()
                .addComponent(jLabel5, javax.swing.GroupLayout.PREFERRED_SIZE, 14, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_poissonModelRadioButton)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_bernoulliModelRadioButton)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_spaceTimePermutationModelRadioButton)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_categoricallModelRadioButton)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_ordinalModelRadioButton)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_exponentialModelRadioButton)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_normalModelRadioButton)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_batchedModelRadioButton)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_uniformtimeModelRadioButton)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED, 9, Short.MAX_VALUE)
                .addComponent(jLabel4)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(_probabilityModelGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(_homogeneouspoissonModelRadioButton)
                    .addComponent(_observableRegionsButton, javax.swing.GroupLayout.PREFERRED_SIZE, 16, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addGap(18, 18, 18))
        );

        _poissonModelRadioButton.getAccessibleContext().setAccessibleName("Discrete Poisson");
        _bernoulliModelRadioButton.getAccessibleContext().setAccessibleName("Discrete Bernoulli");
        _spaceTimePermutationModelRadioButton.getAccessibleContext().setAccessibleName("Discrete Space-Time Permutation");
        _ordinalModelRadioButton.getAccessibleContext().setAccessibleName("Discrete Ordinal");
        _exponentialModelRadioButton.getAccessibleContext().setAccessibleName("Discrete Exponential");
        _normalModelRadioButton.getAccessibleContext().setAccessibleName("Discrete Normal");
        _categoricallModelRadioButton.getAccessibleContext().setAccessibleName("Discrete Multinomial");
        _homogeneouspoissonModelRadioButton.getAccessibleContext().setAccessibleName("Continuous Poisson");
        _observableRegionsButton.getAccessibleContext().setAccessibleName("");
        _observableRegionsButton.getAccessibleContext().setAccessibleDescription("Define polygons");
        _uniformtimeModelRadioButton.getAccessibleContext().setAccessibleName("Discrete Uniform Time");

        _scanAreasGroup.setBorder(javax.swing.BorderFactory.createTitledBorder("Scan For Areas With:"));

        _highRatesRadioButton.setSelected(true);
        _highRatesRadioButton.setText("High Rates"); // NOI18N
        _highRatesRadioButton.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        _highRatesRadioButton.setMargin(new java.awt.Insets(0, 0, 0, 0));
        _highRatesRadioButton.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                //getAdvancedParameterInternalFrame().enableLimitClustersMinimumCasesGroup(Parameters.AreaRateType.HIGH);
                //getAdvancedParameterInternalFrame().enableLimitClustersByRiskLevelGroup(Parameters.AreaRateType.HIGH);
                if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED) {
                    enableSettingsForAnalysisModelCombination();
                }
            }
        });

        _lowRatesRadioButton.setText("Low Rates"); // NOI18N
        _lowRatesRadioButton.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        _lowRatesRadioButton.setMargin(new java.awt.Insets(0, 0, 0, 0));
        _lowRatesRadioButton.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                //getAdvancedParameterInternalFrame().enableLimitClustersMinimumCasesGroup(Parameters.AreaRateType.LOW);
                //getAdvancedParameterInternalFrame().enableLimitClustersByRiskLevelGroup(Parameters.AreaRateType.LOW);
                if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED) {
                    enableSettingsForAnalysisModelCombination();
                }
            }
        });

        _highOrLowRatesRadioButton.setText("High or Low Rates"); // NOI18N
        _highOrLowRatesRadioButton.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        _highOrLowRatesRadioButton.setMargin(new java.awt.Insets(0, 0, 0, 0));
        _highOrLowRatesRadioButton.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                //getAdvancedParameterInternalFrame().enableLimitClustersMinimumCasesGroup(Parameters.AreaRateType.HIGHANDLOW);
                //getAdvancedParameterInternalFrame().enableLimitClustersByRiskLevelGroup(Parameters.AreaRateType.HIGHANDLOW);
                if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED) {
                    enableSettingsForAnalysisModelCombination();
                }
            }
        });

        javax.swing.GroupLayout _scanAreasGroupLayout = new javax.swing.GroupLayout(_scanAreasGroup);
        _scanAreasGroup.setLayout(_scanAreasGroupLayout);
        _scanAreasGroupLayout.setHorizontalGroup(
            _scanAreasGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_scanAreasGroupLayout.createSequentialGroup()
                .addContainerGap()
                .addGroup(_scanAreasGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(_highRatesRadioButton, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(_lowRatesRadioButton, javax.swing.GroupLayout.Alignment.TRAILING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(_highOrLowRatesRadioButton, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                .addContainerGap())
        );
        _scanAreasGroupLayout.setVerticalGroup(
            _scanAreasGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_scanAreasGroupLayout.createSequentialGroup()
                .addComponent(_highRatesRadioButton, javax.swing.GroupLayout.PREFERRED_SIZE, 15, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addGap(13, 13, 13)
                .addComponent(_lowRatesRadioButton)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addComponent(_highOrLowRatesRadioButton)
                .addGap(0, 33, Short.MAX_VALUE))
        );

        _highRatesRadioButton.getAccessibleContext().setAccessibleName("scan High Rates");
        _lowRatesRadioButton.getAccessibleContext().setAccessibleName("scan Low Rates");
        _highOrLowRatesRadioButton.getAccessibleContext().setAccessibleName("scan High or Low Rates");

        _timeAggregationGroup.setBorder(javax.swing.BorderFactory.createTitledBorder("Time Aggregation"));

        _timeAggregationUnitsLabel.setText("Units:"); // NOI18N

        _timeAggregationYearRadioButton.setSelected(true);
        _timeAggregationYearRadioButton.setText("Year"); // NOI18N
        _timeAggregationYearRadioButton.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        _timeAggregationYearRadioButton.setMargin(new java.awt.Insets(0, 0, 0, 0));
        _timeAggregationYearRadioButton.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED) {
                    getAdvancedParameterInternalFrame().updateMaximumTemporalSizeTextCaptions();
                    _aggregrationUnitsLabel.setText("Years");
                }
            }
        });

        _timeAggregationMonthRadioButton.setText("Month"); // NOI18N
        _timeAggregationMonthRadioButton.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        _timeAggregationMonthRadioButton.setMargin(new java.awt.Insets(0, 0, 0, 0));
        _timeAggregationMonthRadioButton.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED) {
                    getAdvancedParameterInternalFrame().updateMaximumTemporalSizeTextCaptions();
                    _aggregrationUnitsLabel.setText("Months");
                }
            }
        });

        _timeAggregationDayRadioButton.setText("Day"); // NOI18N
        _timeAggregationDayRadioButton.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        _timeAggregationDayRadioButton.setMargin(new java.awt.Insets(0, 0, 0, 0));
        _timeAggregationDayRadioButton.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED) {
                    getAdvancedParameterInternalFrame().updateMaximumTemporalSizeTextCaptions();
                    _aggregrationUnitsLabel.setText("Days");
                }
            }
        });

        _timeAggregationLengthLabel.setText("Length:"); // NOI18N

        _timeAggregationLengthTextField.setText("1"); // NOI18N
        _timeAggregationLengthTextField.addFocusListener(new java.awt.event.FocusAdapter() {
            public void focusLost(java.awt.event.FocusEvent e) {
                while (_timeAggregationLengthTextField.getText().length() == 0)
                if (undo.canUndo()) undo.undo(); else _timeAggregationLengthTextField.setText("1");
            }
        });
        _timeAggregationLengthTextField.addKeyListener(new java.awt.event.KeyAdapter() {
            public void keyTyped(java.awt.event.KeyEvent e) {
                Utils.validatePostiveNumericKeyTyped(_timeAggregationLengthTextField, e, 6);
            }
        });
        _timeAggregationLengthTextField.getDocument().addUndoableEditListener(new UndoableEditListener() {
            public void undoableEditHappened(UndoableEditEvent evt) {
                undo.addEdit(evt.getEdit());
            }
        });

        _aggregrationUnitsLabel.setText("Years"); // NOI18N

        javax.swing.GroupLayout _timeAggregationGroupLayout = new javax.swing.GroupLayout(_timeAggregationGroup);
        _timeAggregationGroup.setLayout(_timeAggregationGroupLayout);
        _timeAggregationGroupLayout.setHorizontalGroup(
            _timeAggregationGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_timeAggregationGroupLayout.createSequentialGroup()
                .addContainerGap()
                .addGroup(_timeAggregationGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(_timeAggregationGroupLayout.createSequentialGroup()
                        .addComponent(_timeAggregationUnitsLabel)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addGroup(_timeAggregationGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addComponent(_timeAggregationDayRadioButton)
                            .addComponent(_timeAggregationMonthRadioButton)
                            .addComponent(_timeAggregationYearRadioButton)))
                    .addGroup(_timeAggregationGroupLayout.createSequentialGroup()
                        .addComponent(_timeAggregationLengthLabel)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(_timeAggregationLengthTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 50, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(_aggregrationUnitsLabel)))
                .addContainerGap(24, Short.MAX_VALUE))
        );
        _timeAggregationGroupLayout.setVerticalGroup(
            _timeAggregationGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_timeAggregationGroupLayout.createSequentialGroup()
                .addGroup(_timeAggregationGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(_timeAggregationUnitsLabel, javax.swing.GroupLayout.PREFERRED_SIZE, 19, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(_timeAggregationYearRadioButton))
                .addGap(10, 10, 10)
                .addComponent(_timeAggregationMonthRadioButton)
                .addGap(10, 10, 10)
                .addComponent(_timeAggregationDayRadioButton)
                .addGap(10, 10, 10)
                .addGroup(_timeAggregationGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(_timeAggregationLengthLabel)
                    .addComponent(_timeAggregationLengthTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(_aggregrationUnitsLabel))
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );

        _timeAggregationYearRadioButton.getAccessibleContext().setAccessibleName("Time Aggregation Year");
        _timeAggregationMonthRadioButton.getAccessibleContext().setAccessibleName("Time Aggregation Month");
        _timeAggregationDayRadioButton.getAccessibleContext().setAccessibleName("Time Aggregation Day");
        _timeAggregationLengthLabel.getAccessibleContext().setAccessibleName("Time Aggregation Length");

        _advancedAnalysisButton.setText("Advanced >>"); // NOI18N
        _advancedAnalysisButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                getAdvancedParameterInternalFrame().setVisible(true, AdvancedParameterSettingsFrame.FocusedTabSet.ANALYSIS);
                getAdvancedParameterInternalFrame().requestFocus();
            }
        });

        javax.swing.GroupLayout _analysisTabLayout = new javax.swing.GroupLayout(_analysisTab);
        _analysisTab.setLayout(_analysisTabLayout);
        _analysisTabLayout.setHorizontalGroup(
            _analysisTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_analysisTabLayout.createSequentialGroup()
                .addGroup(_analysisTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, _analysisTabLayout.createSequentialGroup()
                        .addGap(12, 12, 12)
                        .addComponent(_analysisTypeGroup, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(_probabilityModelGroup, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addGroup(_analysisTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                            .addComponent(_timeAggregationGroup, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                            .addComponent(_scanAreasGroup, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)))
                    .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, _analysisTabLayout.createSequentialGroup()
                        .addContainerGap()
                        .addComponent(_advancedAnalysisButton)))
                .addContainerGap())
        );
        _analysisTabLayout.setVerticalGroup(
            _analysisTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_analysisTabLayout.createSequentialGroup()
                .addContainerGap()
                .addGroup(_analysisTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                    .addGroup(_analysisTabLayout.createSequentialGroup()
                        .addComponent(_scanAreasGroup, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(_timeAggregationGroup, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                    .addComponent(_analysisTypeGroup, javax.swing.GroupLayout.Alignment.TRAILING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(_probabilityModelGroup, javax.swing.GroupLayout.Alignment.TRAILING, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED, 149, Short.MAX_VALUE)
                .addComponent(_advancedAnalysisButton)
                .addContainerGap())
        );

        _analysisTypeGroup.getAccessibleContext().setAccessibleName("Analysis Type Selection");
        _probabilityModelGroup.getAccessibleContext().setAccessibleName("Probability Model Type Selection");
        _scanAreasGroup.getAccessibleContext().setAccessibleName("Scan Area Type Selection");
        _timeAggregationGroup.getAccessibleContext().setAccessibleName("Time Aggregation Type Selection");
        _advancedAnalysisButton.getAccessibleContext().setAccessibleName("Advanced Analysis Features");

        _tabbedPane.addTab("Analysis", _analysisTab);
        _analysisTab.getAccessibleContext().setAccessibleName("Analysis Tab");

        _additionalOutputFilesGroup.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createEtchedBorder(), "Column Output Format"));

        _asciiLabel.setText("ASCII"); // NOI18N

        _dBaseLabel.setText("dBase"); // NOI18N

        _clustersInColumnFormatLabel.setText("Cluster Information"); // NOI18N

        _clustersInColumnFormatAsciiCheckBox.setText("      ");
        _clustersInColumnFormatAsciiCheckBox.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        _clustersInColumnFormatAsciiCheckBox.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        _clustersInColumnFormatAsciiCheckBox.setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
        _clustersInColumnFormatAsciiCheckBox.setMargin(new java.awt.Insets(0, 0, 0, 0));

        _clustersInColumnFormatDBaseCheckBox.setText("      ");
        _clustersInColumnFormatDBaseCheckBox.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        _clustersInColumnFormatDBaseCheckBox.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        _clustersInColumnFormatDBaseCheckBox.setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
        _clustersInColumnFormatDBaseCheckBox.setMargin(new java.awt.Insets(0, 0, 0, 0));

        _clusterCaseInColumnFormatLabel.setText("Stratified Cluster Information"); // NOI18N

        _clusterCaseInColumnFormatAsciiCheckBox.setText("      ");
        _clusterCaseInColumnFormatAsciiCheckBox.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        _clusterCaseInColumnFormatAsciiCheckBox.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        _clusterCaseInColumnFormatAsciiCheckBox.setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
        _clusterCaseInColumnFormatAsciiCheckBox.setMargin(new java.awt.Insets(0, 0, 0, 0));

        _clusterCaseInColumnFormatDBaseCheckBox.setText("      ");
        _clusterCaseInColumnFormatDBaseCheckBox.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        _clusterCaseInColumnFormatDBaseCheckBox.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        _clusterCaseInColumnFormatDBaseCheckBox.setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
        _clusterCaseInColumnFormatDBaseCheckBox.setMargin(new java.awt.Insets(0, 0, 0, 0));

        _censusAreasReportedClustersLabel.setText("Location Information"); // NOI18N

        _censusAreasReportedClustersAsciiCheckBox.setText("      ");
        _censusAreasReportedClustersAsciiCheckBox.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        _censusAreasReportedClustersAsciiCheckBox.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        _censusAreasReportedClustersAsciiCheckBox.setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
        _censusAreasReportedClustersAsciiCheckBox.setMargin(new java.awt.Insets(0, 0, 0, 0));

        _censusAreasReportedClustersDBaseCheckBox.setText("      ");
        _censusAreasReportedClustersDBaseCheckBox.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        _censusAreasReportedClustersDBaseCheckBox.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        _censusAreasReportedClustersDBaseCheckBox.setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
        _censusAreasReportedClustersDBaseCheckBox.setMargin(new java.awt.Insets(0, 0, 0, 0));

        _relativeRiskEstimatesAreaLabel.setText("Risk Estimates for Each Location"); // NOI18N

        _relativeRiskEstimatesAreaAsciiCheckBox.setText("      ");
        _relativeRiskEstimatesAreaAsciiCheckBox.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        _relativeRiskEstimatesAreaAsciiCheckBox.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        _relativeRiskEstimatesAreaAsciiCheckBox.setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
        _relativeRiskEstimatesAreaAsciiCheckBox.setMargin(new java.awt.Insets(0, 0, 0, 0));

        _relativeRiskEstimatesAreaDBaseCheckBox.setText("      ");
        _relativeRiskEstimatesAreaDBaseCheckBox.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        _relativeRiskEstimatesAreaDBaseCheckBox.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        _relativeRiskEstimatesAreaDBaseCheckBox.setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
        _relativeRiskEstimatesAreaDBaseCheckBox.setMargin(new java.awt.Insets(0, 0, 0, 0));

        _simulatedLogLikelihoodRatiosLabel.setText("Simulated Log Likelihood Ratios/Test Statistics"); // NOI18N

        _simulatedLogLikelihoodRatiosAsciiCheckBox.setText("      ");
        _simulatedLogLikelihoodRatiosAsciiCheckBox.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        _simulatedLogLikelihoodRatiosAsciiCheckBox.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        _simulatedLogLikelihoodRatiosAsciiCheckBox.setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
        _simulatedLogLikelihoodRatiosAsciiCheckBox.setMargin(new java.awt.Insets(0, 0, 0, 0));

        _simulatedLogLikelihoodRatiosDBaseCheckBox.setText("      ");
        _simulatedLogLikelihoodRatiosDBaseCheckBox.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        _simulatedLogLikelihoodRatiosDBaseCheckBox.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        _simulatedLogLikelihoodRatiosDBaseCheckBox.setHorizontalTextPosition(javax.swing.SwingConstants.CENTER);
        _simulatedLogLikelihoodRatiosDBaseCheckBox.setMargin(new java.awt.Insets(0, 0, 0, 0));

        javax.swing.GroupLayout _additionalOutputFilesGroupLayout = new javax.swing.GroupLayout(_additionalOutputFilesGroup);
        _additionalOutputFilesGroup.setLayout(_additionalOutputFilesGroupLayout);
        _additionalOutputFilesGroupLayout.setHorizontalGroup(
            _additionalOutputFilesGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_additionalOutputFilesGroupLayout.createSequentialGroup()
                .addContainerGap()
                .addGroup(_additionalOutputFilesGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(_additionalOutputFilesGroupLayout.createSequentialGroup()
                        .addGroup(_additionalOutputFilesGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addComponent(_clustersInColumnFormatLabel)
                            .addComponent(_clusterCaseInColumnFormatLabel))
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED, 294, Short.MAX_VALUE)
                        .addGroup(_additionalOutputFilesGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addComponent(_asciiLabel)
                            .addGroup(_additionalOutputFilesGroupLayout.createSequentialGroup()
                                .addGap(10, 10, 10)
                                .addGroup(_additionalOutputFilesGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                                    .addComponent(_clusterCaseInColumnFormatAsciiCheckBox)
                                    .addComponent(_clustersInColumnFormatAsciiCheckBox)
                                    .addComponent(_censusAreasReportedClustersAsciiCheckBox)
                                    .addComponent(_relativeRiskEstimatesAreaAsciiCheckBox)
                                    .addComponent(_simulatedLogLikelihoodRatiosAsciiCheckBox))))
                        .addGap(59, 59, 59)
                        .addGroup(_additionalOutputFilesGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                            .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, _additionalOutputFilesGroupLayout.createSequentialGroup()
                                .addComponent(_dBaseLabel)
                                .addGap(71, 71, 71))
                            .addGroup(_additionalOutputFilesGroupLayout.createSequentialGroup()
                                .addGap(10, 10, 10)
                                .addGroup(_additionalOutputFilesGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                                    .addComponent(_clusterCaseInColumnFormatDBaseCheckBox)
                                    .addComponent(_clustersInColumnFormatDBaseCheckBox)
                                    .addComponent(_censusAreasReportedClustersDBaseCheckBox)
                                    .addComponent(_relativeRiskEstimatesAreaDBaseCheckBox)
                                    .addComponent(_simulatedLogLikelihoodRatiosDBaseCheckBox))
                                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))))
                    .addGroup(_additionalOutputFilesGroupLayout.createSequentialGroup()
                        .addGroup(_additionalOutputFilesGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addComponent(_censusAreasReportedClustersLabel)
                            .addComponent(_relativeRiskEstimatesAreaLabel)
                            .addComponent(_simulatedLogLikelihoodRatiosLabel))
                        .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))))
        );
        _additionalOutputFilesGroupLayout.setVerticalGroup(
            _additionalOutputFilesGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_additionalOutputFilesGroupLayout.createSequentialGroup()
                .addGroup(_additionalOutputFilesGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(_additionalOutputFilesGroupLayout.createSequentialGroup()
                        .addComponent(_dBaseLabel)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addGroup(_additionalOutputFilesGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addComponent(_clustersInColumnFormatLabel)
                            .addComponent(_clustersInColumnFormatDBaseCheckBox))
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addGroup(_additionalOutputFilesGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addComponent(_clusterCaseInColumnFormatLabel)
                            .addComponent(_clusterCaseInColumnFormatDBaseCheckBox)))
                    .addGroup(_additionalOutputFilesGroupLayout.createSequentialGroup()
                        .addComponent(_asciiLabel)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(_clustersInColumnFormatAsciiCheckBox)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(_clusterCaseInColumnFormatAsciiCheckBox)))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(_additionalOutputFilesGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(_censusAreasReportedClustersLabel)
                    .addComponent(_censusAreasReportedClustersAsciiCheckBox)
                    .addComponent(_censusAreasReportedClustersDBaseCheckBox))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(_additionalOutputFilesGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(_relativeRiskEstimatesAreaLabel)
                    .addComponent(_relativeRiskEstimatesAreaAsciiCheckBox)
                    .addComponent(_relativeRiskEstimatesAreaDBaseCheckBox))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(_additionalOutputFilesGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(_simulatedLogLikelihoodRatiosLabel)
                    .addComponent(_simulatedLogLikelihoodRatiosAsciiCheckBox)
                    .addComponent(_simulatedLogLikelihoodRatiosDBaseCheckBox))
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );

        _clustersInColumnFormatAsciiCheckBox.getAccessibleContext().setAccessibleName("Cluster Information in text format");
        _clustersInColumnFormatDBaseCheckBox.getAccessibleContext().setAccessibleName("Cluster Information in dbase format");
        _clusterCaseInColumnFormatAsciiCheckBox.getAccessibleContext().setAccessibleName("Stratified Cluster Information in text format");
        _clusterCaseInColumnFormatDBaseCheckBox.getAccessibleContext().setAccessibleName("Stratified Cluster Information in dbase format");
        _censusAreasReportedClustersAsciiCheckBox.getAccessibleContext().setAccessibleName("Location Information in text format");
        _censusAreasReportedClustersDBaseCheckBox.getAccessibleContext().setAccessibleName("Location Information in dbase format");
        _relativeRiskEstimatesAreaAsciiCheckBox.getAccessibleContext().setAccessibleName("Risk Estimates for Each Location in text format");
        _relativeRiskEstimatesAreaDBaseCheckBox.getAccessibleContext().setAccessibleName("Risk Estimates for Each Location in dbase format");
        _simulatedLogLikelihoodRatiosAsciiCheckBox.getAccessibleContext().setAccessibleName("Simulated Log Likelihood Ratios/Test Statistics in text format");
        _simulatedLogLikelihoodRatiosDBaseCheckBox.getAccessibleContext().setAccessibleName("Simulated Log Likelihood Ratios/Test Statistics in dbase format");

        _advancedFeaturesOutputButton.setText("Advanced >>"); // NOI18N
        _advancedFeaturesOutputButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                getAdvancedParameterInternalFrame().setVisible(true, AdvancedParameterSettingsFrame.FocusedTabSet.OUTPUT);
                getAdvancedParameterInternalFrame().requestFocus();
            }
        });

        _textOutputFormatGroup.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createEtchedBorder(), "Text Output Format"));

        _resultsFileLabel.setLabelFor(_resultsFileTextField);
        _resultsFileLabel.setText("Main Results File:"); // NOI18N

        _resultsFileBrowseButton.setText("..."); // NOI18N
        _resultsFileBrowseButton.setToolTipText("Browse Results File");
        _resultsFileBrowseButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                List<InputFileFilter> filters = new ArrayList<InputFileFilter>();
                filters.add(new InputFileFilter("txt","Results Files (*.txt)"));
                FileSelectionDialog select = new FileSelectionDialog(SaTScanApplication.getInstance(), "Select Results File", filters, SaTScanApplication.getInstance().lastBrowseDirectory);
                File file = select.browse_saveas();
                if (file != null) {
                    SaTScanApplication.getInstance().lastBrowseDirectory = select.getDirectory();
                    String filename = file.getAbsolutePath();
                    if (new File(filename).getName().lastIndexOf('.') == -1){
                        filename = filename + ".txt";
                    }
                    _resultsFileTextField.setText(filename);
                }
            }
        });

        javax.swing.GroupLayout _textOutputFormatGroupLayout = new javax.swing.GroupLayout(_textOutputFormatGroup);
        _textOutputFormatGroup.setLayout(_textOutputFormatGroupLayout);
        _textOutputFormatGroupLayout.setHorizontalGroup(
            _textOutputFormatGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_textOutputFormatGroupLayout.createSequentialGroup()
                .addContainerGap()
                .addGroup(_textOutputFormatGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(_textOutputFormatGroupLayout.createSequentialGroup()
                        .addComponent(_resultsFileLabel)
                        .addGap(0, 0, Short.MAX_VALUE))
                    .addGroup(_textOutputFormatGroupLayout.createSequentialGroup()
                        .addComponent(_resultsFileTextField)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(_resultsFileBrowseButton, javax.swing.GroupLayout.PREFERRED_SIZE, 25, javax.swing.GroupLayout.PREFERRED_SIZE)))
                .addContainerGap())
        );
        _textOutputFormatGroupLayout.setVerticalGroup(
            _textOutputFormatGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_textOutputFormatGroupLayout.createSequentialGroup()
                .addComponent(_resultsFileLabel)
                .addGap(0, 0, 0)
                .addGroup(_textOutputFormatGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(_resultsFileBrowseButton)
                    .addComponent(_resultsFileTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );

        _resultsFileBrowseButton.getAccessibleContext().setAccessibleName("");

        _geographicalOutputGroup.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createEtchedBorder(), "Geographical Output"));

        _reportGoogleEarthKML.setText("KML file for Google Earth");
        _reportGoogleEarthKML.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                _advancedParametersSetting.enableMapsOutputGroup();
            }
        });

        _reportShapefile.setText("Shapefile for GIS software");

        _reportGoogleMap.setText("HTML file for Google Map");
        _reportGoogleMap.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                _advancedParametersSetting.enableMapsOutputGroup();
            }
        });

        _reportCartesianGraph.setText("HTML file for Cartesian map");
        _reportCartesianGraph.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                _advancedParametersSetting.enableMapsOutputGroup();
            }
        });

        javax.swing.GroupLayout _geographicalOutputGroupLayout = new javax.swing.GroupLayout(_geographicalOutputGroup);
        _geographicalOutputGroup.setLayout(_geographicalOutputGroupLayout);
        _geographicalOutputGroupLayout.setHorizontalGroup(
            _geographicalOutputGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_geographicalOutputGroupLayout.createSequentialGroup()
                .addContainerGap()
                .addGroup(_geographicalOutputGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(_reportCartesianGraph, javax.swing.GroupLayout.DEFAULT_SIZE, 620, Short.MAX_VALUE)
                    .addComponent(_reportShapefile, javax.swing.GroupLayout.DEFAULT_SIZE, 620, Short.MAX_VALUE)
                    .addComponent(_reportGoogleMap, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(_reportGoogleEarthKML, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                .addContainerGap())
        );
        _geographicalOutputGroupLayout.setVerticalGroup(
            _geographicalOutputGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_geographicalOutputGroupLayout.createSequentialGroup()
                .addComponent(_reportGoogleMap)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_reportGoogleEarthKML)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addComponent(_reportShapefile)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addComponent(_reportCartesianGraph)
                .addContainerGap(12, Short.MAX_VALUE))
        );

        javax.swing.GroupLayout _outputTabLayout = new javax.swing.GroupLayout(_outputTab);
        _outputTab.setLayout(_outputTabLayout);
        _outputTabLayout.setHorizontalGroup(
            _outputTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_outputTabLayout.createSequentialGroup()
                .addContainerGap()
                .addGroup(_outputTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(_additionalOutputFilesGroup, javax.swing.GroupLayout.Alignment.TRAILING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, _outputTabLayout.createSequentialGroup()
                        .addGap(0, 0, Short.MAX_VALUE)
                        .addComponent(_advancedFeaturesOutputButton))
                    .addComponent(_textOutputFormatGroup, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(_geographicalOutputGroup, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                .addContainerGap())
        );
        _outputTabLayout.setVerticalGroup(
            _outputTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_outputTabLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(_textOutputFormatGroup, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_geographicalOutputGroup, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_additionalOutputFilesGroup, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED, 76, Short.MAX_VALUE)
                .addComponent(_advancedFeaturesOutputButton)
                .addContainerGap())
        );

        _advancedFeaturesOutputButton.getAccessibleContext().setAccessibleName("Advanced Output Features");

        _tabbedPane.addTab("Output", _outputTab);
        _outputTab.getAccessibleContext().setAccessibleName("Output Tab");

        javax.swing.GroupLayout layout = new javax.swing.GroupLayout(getContentPane());
        getContentPane().setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addComponent(_tabbedPane)
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addComponent(_tabbedPane)
        );

        pack();
    }// </editor-fold>//GEN-END:initComponents

    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JPanel _additionalOutputFilesGroup;
    private javax.swing.JButton _advancedAnalysisButton;
    private javax.swing.JButton _advancedFeaturesOutputButton;
    private javax.swing.JButton _advancedInputButton;
    private javax.swing.JLabel _aggregrationUnitsLabel;
    private javax.swing.JPanel _analysisTab;
    private javax.swing.ButtonGroup _analysisTypeButtonGroup;
    private javax.swing.JPanel _analysisTypeGroup;
    private javax.swing.JLabel _asciiLabel;
    private javax.swing.JRadioButton _batchedModelRadioButton;
    private javax.swing.JLabel _bernoulliModelHintLabel;
    private javax.swing.JRadioButton _bernoulliModelRadioButton;
    private javax.swing.JRadioButton _cartesianRadioButton;
    private javax.swing.JButton _caseFileBrowseButton;
    private javax.swing.JLabel _caseFileLabel;
    public javax.swing.JTextField _caseFileTextField;
    private javax.swing.JPanel _caseInputPanel;
    private javax.swing.JRadioButton _categoricallModelRadioButton;
    private javax.swing.JCheckBox _censusAreasReportedClustersAsciiCheckBox;
    private javax.swing.JCheckBox _censusAreasReportedClustersDBaseCheckBox;
    private javax.swing.JLabel _censusAreasReportedClustersLabel;
    private javax.swing.JCheckBox _clusterCaseInColumnFormatAsciiCheckBox;
    private javax.swing.JCheckBox _clusterCaseInColumnFormatDBaseCheckBox;
    private javax.swing.JLabel _clusterCaseInColumnFormatLabel;
    private javax.swing.JCheckBox _clustersInColumnFormatAsciiCheckBox;
    private javax.swing.JCheckBox _clustersInColumnFormatDBaseCheckBox;
    private javax.swing.JLabel _clustersInColumnFormatLabel;
    private javax.swing.JButton _controlFileBrowseButton;
    private javax.swing.JLabel _controlFileLabel;
    public javax.swing.JTextField _controlFileTextField;
    private javax.swing.JTextField _coordiantesFileTextField;
    private javax.swing.JPanel _coordinateTypeGroup;
    private javax.swing.ButtonGroup _coordinatesButtonGroup;
    private javax.swing.JButton _coordinatesFileBrowseButton;
    private javax.swing.JLabel _coordinatesFileLabel;
    private javax.swing.JLabel _dBaseLabel;
    private javax.swing.JLabel _endDateDayLabel;
    private javax.swing.JLabel _endDateLabel;
    private javax.swing.JLabel _endDateLabel1;
    private javax.swing.JLabel _endDateMonthLabel;
    private javax.swing.JLabel _endDateYearLabel;
    private javax.swing.JLabel _endDateYearLabel1;
    private javax.swing.JRadioButton _exponentialModelRadioButton;
    private javax.swing.JPanel _geographicalInputPanel;
    private javax.swing.JPanel _geographicalOutputGroup;
    private javax.swing.JButton _gridFileBrowseButton;
    private javax.swing.JLabel _gridFileHintLabel;
    private javax.swing.JLabel _gridFileLabel;
    private javax.swing.JTextField _gridFileTextField;
    private javax.swing.JRadioButton _highOrLowRatesRadioButton;
    private javax.swing.JRadioButton _highRatesRadioButton;
    private javax.swing.JRadioButton _homogeneouspoissonModelRadioButton;
    private javax.swing.JPanel _inputTab;
    private javax.swing.JRadioButton _latLongRadioButton;
    private javax.swing.JRadioButton _lowRatesRadioButton;
    private javax.swing.JRadioButton _normalModelRadioButton;
    private javax.swing.JButton _observableRegionsButton;
    private javax.swing.JRadioButton _ordinalModelRadioButton;
    private javax.swing.JPanel _outputTab;
    private javax.swing.JLabel _poissionModelHintLabel;
    private javax.swing.JRadioButton _poissonModelRadioButton;
    private javax.swing.JButton _populationFileBrowseButton;
    private javax.swing.JLabel _populationFileLabel;
    public javax.swing.JTextField _populationFileTextField;
    private javax.swing.JPanel _populationInputPanel;
    private javax.swing.JPanel _probabilityModelGroup;
    private javax.swing.ButtonGroup _probabilityModelTypeButtonGroup;
    private javax.swing.JRadioButton _prospectivePurelyTemporalRadioButton;
    private javax.swing.JRadioButton _prospectiveSpaceTimeRadioButton;
    private javax.swing.JCheckBox _relativeRiskEstimatesAreaAsciiCheckBox;
    private javax.swing.JCheckBox _relativeRiskEstimatesAreaDBaseCheckBox;
    private javax.swing.JLabel _relativeRiskEstimatesAreaLabel;
    private javax.swing.JCheckBox _reportCartesianGraph;
    private javax.swing.JCheckBox _reportGoogleEarthKML;
    private javax.swing.JCheckBox _reportGoogleMap;
    private javax.swing.JCheckBox _reportShapefile;
    private javax.swing.JButton _resultsFileBrowseButton;
    private javax.swing.JLabel _resultsFileLabel;
    private javax.swing.JTextField _resultsFileTextField;
    private javax.swing.JRadioButton _retrospectivePurelySpatialRadioButton;
    private javax.swing.JRadioButton _retrospectivePurelyTemporalRadioButton;
    private javax.swing.JRadioButton _retrospectiveSeasonalRadioButton;
    private javax.swing.JRadioButton _retrospectiveSpaceTimeRadioButton;
    private javax.swing.ButtonGroup _scanAreasButtonGroup;
    private javax.swing.JPanel _scanAreasGroup;
    private javax.swing.JCheckBox _simulatedLogLikelihoodRatiosAsciiCheckBox;
    private javax.swing.JCheckBox _simulatedLogLikelihoodRatiosDBaseCheckBox;
    private javax.swing.JLabel _simulatedLogLikelihoodRatiosLabel;
    private javax.swing.JRadioButton _spaceTimePermutationModelRadioButton;
    private javax.swing.JRadioButton _spatialVariationRadioButton;
    private javax.swing.JLabel _spatialVariationRadioLabel;
    private javax.swing.JLabel _startDateDayLabel;
    private javax.swing.JLabel _startDateLabel;
    private javax.swing.JLabel _startDateLabel1;
    private javax.swing.JLabel _startDateMonthLabel;
    private javax.swing.JLabel _startDateYearLabel;
    private javax.swing.JLabel _startDateYearLabel1;
    private javax.swing.JPanel _studyPeriodComplete;
    private javax.swing.JTextField _studyPeriodEndDateDayTextField;
    private javax.swing.JTextField _studyPeriodEndDateGenericTextField;
    private javax.swing.JTextField _studyPeriodEndDateMonthTextField;
    private javax.swing.JTextField _studyPeriodEndDateYearTextField;
    private javax.swing.JPanel _studyPeriodGeneric;
    private javax.swing.JPanel _studyPeriodGroup;
    private javax.swing.JTextField _studyPeriodStartDateDayTextField;
    private javax.swing.JTextField _studyPeriodStartDateGenericTextField;
    private javax.swing.JTextField _studyPeriodStartDateMonthTextField;
    private javax.swing.JTextField _studyPeriodStartDateYearTextField;
    private javax.swing.JTabbedPane _tabbedPane;
    private javax.swing.JPanel _textOutputFormatGroup;
    private javax.swing.ButtonGroup _timeAggregationButtonGroup;
    private javax.swing.JRadioButton _timeAggregationDayRadioButton;
    private javax.swing.JPanel _timeAggregationGroup;
    private javax.swing.JLabel _timeAggregationLengthLabel;
    public javax.swing.JTextField _timeAggregationLengthTextField;
    private javax.swing.JRadioButton _timeAggregationMonthRadioButton;
    private javax.swing.JLabel _timeAggregationUnitsLabel;
    private javax.swing.JRadioButton _timeAggregationYearRadioButton;
    private javax.swing.ButtonGroup _timePrecisionButtonGroup;
    private javax.swing.JRadioButton _timePrecisionDay;
    private javax.swing.JRadioButton _timePrecisionGeneric;
    private javax.swing.JPanel _timePrecisionGroup;
    private javax.swing.JRadioButton _timePrecisionMonth;
    private javax.swing.JRadioButton _timePrecisionNone;
    private javax.swing.JRadioButton _timePrecisionYear;
    private javax.swing.JRadioButton _uniformtimeModelRadioButton;
    private javax.swing.JLabel jLabel1;
    private javax.swing.JLabel jLabel2;
    private javax.swing.JLabel jLabel4;
    private javax.swing.JLabel jLabel5;
    // End of variables declaration//GEN-END:variables
    public void internalFrameOpened(InternalFrameEvent e) {
    }

    public void internalFrameClosing(InternalFrameEvent e) {
        if ((gbPromptOnExist ? (queryWindowCanClose() ? true : false) : true) == true) {
            // Add parameters settings to history - except skip windows linked to batch frame.
            if (_multipleAnaylsisFrame == null)
                ParameterHistory.getInstance().AddParameterToHistory(_parameters.GetSourceFileName());
            dispose();
        }
    }

    public void internalFrameClosed(InternalFrameEvent e) {
    }

    public void internalFrameIconified(InternalFrameEvent e) {
    }

    public void internalFrameDeiconified(InternalFrameEvent e) {
    }

    public void internalFrameActivated(InternalFrameEvent e) {
    }

    public void internalFrameDeactivated(InternalFrameEvent e) {
    }

    /**
     * Exception class that notes the Component that caused the exceptional situation.
     */
    public class SettingsException extends RuntimeException {

        private static final long serialVersionUID = 1L;
        public final Component focusComponent;

        public SettingsException(Component focusComponent) {
            super();
            this.focusComponent = focusComponent;
        }

        public SettingsException(String arg0, Component focusComponent) {
            super(arg0);
            this.focusComponent = focusComponent;
        }

        public SettingsException(String arg0, Throwable arg1, Component focusComponent) {
            super(arg0, arg1);
            this.focusComponent = focusComponent;
        }

        public SettingsException(Throwable arg0, Component focusComponent) {
            super(arg0);
            this.focusComponent = focusComponent;
        }

        /** recursively searches Container objects contained in 'rootComponent' for for 'searchComponent'. */
        boolean isContainedComponent(Component rootComponent, Component searchComponent) {
            if (rootComponent == searchComponent) {
                return true;
            }
            try {
                if (Class.forName("java.awt.Container").isInstance(rootComponent)) {
                    Container rootContainer = (Container) rootComponent;
                    for (int j = 0; j < rootContainer.getComponentCount(); ++j) {
                        if (isContainedComponent(rootContainer.getComponent(j), searchComponent)) {
                            return true;
                        }
                    }
                }
            } catch (ClassNotFoundException e) {
            }
            return false;
        }

        public void setControlFocus() {
            for (int i = 0; i < _tabbedPane.getTabCount(); ++i) {
                if (isContainedComponent(_tabbedPane.getComponentAt(i), focusComponent)) {
                    _tabbedPane.setSelectedIndex(i);
                    focusComponent.requestFocus();
                    return;
                }
            }
        }
    }
}
