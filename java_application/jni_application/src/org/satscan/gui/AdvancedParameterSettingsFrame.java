package org.satscan.gui;

import java.awt.BorderLayout;
import java.awt.CardLayout;
import java.awt.Component;
import java.awt.Container;
import java.awt.Dimension;
import java.awt.GraphicsEnvironment;
import java.awt.Point;
import java.awt.event.ComponentAdapter;
import java.awt.event.ComponentEvent;
import java.io.IOException;
import java.util.ArrayList;
import java.util.GregorianCalendar;
import java.util.logging.Level;
import java.util.logging.Logger;
import javax.swing.DefaultListModel;
import javax.swing.ImageIcon;
import javax.swing.JOptionPane;
import javax.swing.JPanel;
import javax.swing.JRootPane;
import javax.swing.JScrollPane;
import javax.swing.JTextPane;
import javax.swing.event.DocumentEvent;
import javax.swing.event.DocumentListener;
import javax.swing.event.MouseInputAdapter;
import javax.swing.event.UndoableEditEvent;
import javax.swing.event.UndoableEditListener;
import javax.swing.undo.UndoManager;
import org.satscan.app.AdvFeaturesExpection;
import org.satscan.app.AppConstants;
import org.satscan.utils.FileAccess;
import org.satscan.app.Parameters;
import org.satscan.gui.utils.Utils;
import org.satscan.app.UnknownEnumException;
import org.satscan.gui.utils.DateComponentsGroup;
import org.satscan.gui.utils.FileSelectionDialog;
import org.satscan.importer.InputSourceSettings;

/*
 * ParameterSettingsFrame.java
 * Created on December 5, 2007, 11:07 AM
 */
public class AdvancedParameterSettingsFrame extends javax.swing.JInternalFrame {

    public enum FocusedTabSet {INPUT, ANALYSIS, OUTPUT};
    private JPanel _glass = null;
    private final JRootPane _rootPane;
    private final Component _rootPaneInitialGlass;
    private final UndoManager undo = new UndoManager();
    private final ParameterSettingsFrame _settings_window;
    private final DefaultListModel _dataSetsListModel = new DefaultListModel();
    private ArrayList<String> _caseFilenames = new ArrayList<>();
    private ArrayList<String> _controlFilenames = new ArrayList<>();
    private ArrayList<String> _populationFilenames = new ArrayList<>();
    private FocusedTabSet _focusedTabSet = FocusedTabSet.INPUT;
    private final int MAXIMUM_ADDITIONAL_SETS = 19;
    final static String FLEXIBLE_COMPLETE = "flexible_complete";
    final static String FLEXIBLE_GENERIC = "flexible_generic";
    private final DateComponentsGroup _flexStartRangeStartDateComponentsGroup;
    private final DateComponentsGroup _flexStartRangeEndDateComponentsGroup;
    private final DateComponentsGroup _flexEndRangeStartDateComponentsGroup;
    private final DateComponentsGroup _flexEndRangeEndDateComponentsGroup;

    /**
     * Creates new form ParameterSettingsFrame
     */
    public AdvancedParameterSettingsFrame(final JRootPane rootPane, final ParameterSettingsFrame analysisSettingsWindow, final Parameters parameters) {
        initComponents();
        _flexStartRangeStartDateComponentsGroup = new DateComponentsGroup(new UndoManager(), _startRangeStartYearTextField, _startRangeStartMonthTextField, _startRangeStartDayTextField, 2000, 1, 1, false);
        _flexStartRangeEndDateComponentsGroup = new DateComponentsGroup(new UndoManager(), _startRangeEndYearTextField, _startRangeEndMonthTextField, _startRangeEndDayTextField, 2000, 12, 31, true);
        _flexEndRangeStartDateComponentsGroup = new DateComponentsGroup(new UndoManager(), _endRangeStartYearTextField, _endRangeStartMonthTextField, _endRangeStartDayTextField, 2000, 1, 1, false);
        _flexEndRangeEndDateComponentsGroup = new DateComponentsGroup(new UndoManager(), _endRangeEndYearTextField, _endRangeEndMonthTextField, _endRangeEndDayTextField, 2000, 12, 31, true);

        setFrameIcon(new ImageIcon(getClass().getResource("/SaTScan.png")));
        _rootPane = rootPane;
        _settings_window = analysisSettingsWindow;
        _rootPaneInitialGlass = rootPane.getGlassPane();
        // create opaque glass pane
        _glass = new JPanel();
        _glass.setOpaque(false);
        
        // Attach mouse listeners
        MouseInputAdapter adapter = new MouseInputAdapter() {
        };
        _glass.addMouseListener(adapter);
        _glass.addMouseMotionListener(adapter);
        // Add modal internal frame to glass pane
        _glass.add(this, BorderLayout.CENTER);
        _glass.setLayout(new BorderLayout());
        _glass.addComponentListener(new ComponentAdapter(){
            public void componentHidden(ComponentEvent ce){
                _glass.setVisible(AdvancedParameterSettingsFrame.this.isVisible());
            }
        });
        add(popupMenu1);
        setupInterface(parameters);
        pack();
    }

    /**
     * recursively searches Container objects contained in 'rootComponent' for
     * for 'searchComponent'.
     */
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

    /**
     * Sets tab set visible and attempts to set focus on 'focusComponent'.
     */
    public void setVisible(FocusedTabSet focusedTabSet, Component focusComponent) {
        //set tab set visible
        setVisible(true, focusedTabSet);
        //find focus component and request focus
        for (int i = 0; i < jTabbedPane1.getTabCount(); ++i) {
            if (isContainedComponent(jTabbedPane1.getComponentAt(i), focusComponent)) {
                jTabbedPane1.setSelectedIndex(i);
                focusComponent.requestFocus();
                return;
            }
        }
    }

    /**
     *
     */
    public void setVisible(boolean value, FocusedTabSet focusedTabSet) {
        if (value == false) {
            _closeButton.requestFocus();
        } //cause any text controls to loose focus
        super.setVisible(value);
        if (value) {
            startModal(focusedTabSet);
            enableSetDefaultsButton();
        } else {
            stopModal();
            _settings_window.enableAdvancedButtons();
        }
        // show panel in the center of application window
        Dimension appSize = SaTScanApplication.getInstance().getSize();
        Dimension frameSize = this.getSize();
        setLocation((appSize.width - frameSize.width)/2,(appSize.height- frameSize.height)/2);
    }

    /**
     *
     */
    private synchronized void startModal(FocusedTabSet focusedTabSet) {
        if (_glass != null) {
            _rootPane.setGlassPane(_glass);
            _glass.setVisible(true); // Change glass pane to our panel
            
            Dimension windowSize = getSize();
            //_settings_window.getGraphicsConfiguration().get
            GraphicsEnvironment ge = GraphicsEnvironment.getLocalGraphicsEnvironment();
            Point centerPoint = ge.getCenterPoint();
            setLocation(centerPoint.x - windowSize.width, 100);            
        }
        setFocusedTabSet(focusedTabSet);
    }

    /**
     *
     */
    private void setFocusedTabSet(FocusedTabSet focusedTabSet) {
        jTabbedPane1.removeAll();
        if (_focusedTabSet != null) {
            _focusedTabSet = focusedTabSet;
        }
        switch (_focusedTabSet) {
            case ANALYSIS:
                setTitle("Advanced Analysis Features");
                jTabbedPane1.addTab("Spatial Window", null, _spatialWindowTab, null);
                jTabbedPane1.addTab("Temporal Window", null, _temporalWindowTab, null);
                jTabbedPane1.addTab("Cluster Restrictions", null, _cluster_restrictions_tab, null);
                jTabbedPane1.addTab("Space and Time Adjustments", null, _spaceTimeAjustmentsTab, null);
                jTabbedPane1.addTab("Inference", null, _inferenceTab, null);
                jTabbedPane1.addTab("Power Evaluation", null, _powerEvaluationTab, null);
                jTabbedPane1.addTab("Drilldown", null, _drilldown_tab, null);
                jTabbedPane1.addTab("Miscellaneous", null, _miscellaneous_analysis_tab, null);
                break;
            case OUTPUT:
                setTitle("Advanced Output Features");
                jTabbedPane1.addTab("Spatial Output", null, _spatialOutputTab, null);
                jTabbedPane1.addTab("Temporal Output", null, _temporalOutputTab, null);
                jTabbedPane1.addTab("Other Output", null, _otherOutputTab, null);
                jTabbedPane1.addTab("Notifications", null, _notificatons_tab, null);
                break;
            case INPUT:
            default:
                setTitle("Advanced Input Features");
                jTabbedPane1.addTab("Multiple Data Sets", null, _multipleDataSetsTab, null);
                jTabbedPane1.addTab("Data Checking", null, _dataCheckingTab, null);
                jTabbedPane1.addTab("Spatial Neighbors", null, _spatialNeighborsTab, null);
                jTabbedPane1.addTab("Network", null, _network_tab, null);
        }
    }

    /**
     *
     */
    private synchronized void stopModal() {
        if (_glass != null) {
            _glass.setVisible(false);
            //reset root pane glass to original
            _rootPane.setGlassPane(_rootPaneInitialGlass);
        }
    }

    /*
     * enables options in the maps output group
     */
    public void enableMapsOutputGroup() {
        _mapsOutputGroup.setEnabled(_settings_window.anyLaunchableSelections());
        _label_kml_options.setEnabled(_mapsOutputGroup.isEnabled() && _settings_window.getReportingGoogleEarthKML());
        _includeClusterLocationsInKML.setEnabled(_mapsOutputGroup.isEnabled() && _settings_window.getReportingGoogleEarthKML());
        _createCompressedKMZ.setEnabled(_mapsOutputGroup.isEnabled() && _settings_window.getReportingGoogleEarthKML());
        _launch_map_viewer.setEnabled(_mapsOutputGroup.isEnabled() && _settings_window.anyLaunchableSelections());
    }

    /**
     * enabled study period date precision based on time interval unit
     */
    public void enableDatesByTimePrecisionUnits() {
        CardLayout cl_flexible = (CardLayout) (_flexible_window_cards.getLayout());
        switch (_settings_window.getPrecisionOfTimesControlType()) {
            case NONE:
            case DAY:
                enableDates();
                cl_flexible.show(_flexible_window_cards, FLEXIBLE_COMPLETE);
                break;
            case YEAR:
                enableDates();
                cl_flexible.show(_flexible_window_cards, FLEXIBLE_COMPLETE);
                break;
            case MONTH:
                enableDates();
                cl_flexible.show(_flexible_window_cards, FLEXIBLE_COMPLETE);
                break;
            case GENERIC:
                enableDates();
                cl_flexible.show(_flexible_window_cards, FLEXIBLE_GENERIC);
                break;
            default:
                throw new UnknownEnumException(_settings_window.getPrecisionOfTimesControlType());
        }
    }

    /**
     * Enables dates of flexible temporal window group. Enabling is determined 
     * through: - querying the 'precision of time' control contained in the 
     * analysis window - the Enabled property of the TGroupBox of which dates 
     * are contained - the Enabled and Checked properties of the TCheckBox that 
     * indicates whether user wishes to adjust for earlier analyses.
     */
    public void enableDates() {
        boolean enableYears = true, enableMonths = true, enableDays = true,
                enableGroup = _flexibleTemporalWindowDefinitionGroup.isEnabled()
                && _restrictTemporalRangeCheckBox.isEnabled() && _restrictTemporalRangeCheckBox.isSelected();

        switch (_settings_window.getPrecisionOfTimesControlType()) {
            case NONE:
            case DAY:
                enableYears = enableMonths = enableDays = enableGroup;
                break;
            case YEAR:
                enableYears = enableGroup;
                enableMonths = enableDays = false;
                break;
            case MONTH:
                enableYears = enableMonths = enableGroup;
                enableDays = false;
                break;
        }

        //enable generic ranges
        _startRangeStartGenericTextField.setEnabled(enableGroup);
        _startRangeEndGenericTextField.setEnabled(enableGroup);
        _endRangeStartGenericTextField.setEnabled(enableGroup);
        _endRangeEndGenericTextField.setEnabled(enableGroup);

        //enable start range dates
        _startRangeStartYearTextField.setEnabled(enableYears);
        _startRangeStartMonthTextField.setEnabled(enableMonths);
        if (!_startRangeStartMonthTextField.isEnabled() && enableGroup) {
            _flexStartRangeStartDateComponentsGroup.setMonth(1);
        }
        _startRangeStartDayTextField.setEnabled(enableDays);
        if (!_startRangeStartDayTextField.isEnabled() && enableGroup) {
            _flexStartRangeStartDateComponentsGroup.setDay(1);
        }
        _startRangeEndYearTextField.setEnabled(enableYears);
        _startRangeEndMonthTextField.setEnabled(enableMonths);
        if (!_startRangeEndMonthTextField.isEnabled() && enableGroup) {
            _flexStartRangeEndDateComponentsGroup.setMonth(12);
        }
        _startRangeEndDayTextField.setEnabled(enableDays);
        if (!_startRangeEndDayTextField.isEnabled() && enableGroup) {
            _flexStartRangeEndDateComponentsGroup.setDay(31);
        }
        // to be cautious, validate the groups 
        _flexStartRangeStartDateComponentsGroup.validateGroup();
        _flexStartRangeEndDateComponentsGroup.validateGroup();

        //enable end range dates
        _endRangeStartYearTextField.setEnabled(enableYears);
        _endRangeStartMonthTextField.setEnabled(enableMonths);
        if (!_endRangeStartMonthTextField.isEnabled() && enableGroup) {
            _flexEndRangeStartDateComponentsGroup.setMonth(1);
        }
        _endRangeStartDayTextField.setEnabled(enableDays);
        if (!_endRangeStartDayTextField.isEnabled() && enableGroup) {
            _flexEndRangeStartDateComponentsGroup.setDay(1);
        }
        _endRangeEndYearTextField.setEnabled(enableYears);
        _endRangeEndMonthTextField.setEnabled(enableMonths);
        if (!_endRangeEndMonthTextField.isEnabled() && enableGroup) {
            _flexEndRangeEndDateComponentsGroup.setMonth(12);
        }
        _endRangeEndDayTextField.setEnabled(enableDays);
        if (!_endRangeEndDayTextField.isEnabled() && enableGroup) {
            _flexEndRangeEndDateComponentsGroup.setDay(31);
        }
        // to be cautious, validate the groups 
        _flexEndRangeStartDateComponentsGroup.validateGroup();
        _flexEndRangeEndDateComponentsGroup.validateGroup();
    }

    private void enableAdditionalDataSetsGroup(boolean bEnable) {
        //bEnable &= (_performIsotonicScanCheckBox.isEnabled() ? !_performIsotonicScanCheckBox.isSelected() : true);
        bEnable &= _settings_window.getModelControlType() != Parameters.ProbabilityModelType.HOMOGENEOUSPOISSON;
        _additionalDataSetsGroup.setEnabled(bEnable);

        enableDataSetList();
        enableNewButton();
        enableRemoveButton();
        enableInputFileEdits();
        enableDataSetPurposeControls();
        enableLimitClustersMinimumCasesGroup(_settings_window.getAreaScanRateControlType());
        enableLimitClustersByRiskLevelGroup(_settings_window.getAreaScanRateControlType());        
    }

    private void enableEmailAlerts() {
        Parameters.AnalysisType analysis_type = _settings_window.getAnalysisControlType();
        double val = Double.parseDouble(_cutoff_value_email.getText());
        if (analysis_type == Parameters.AnalysisType.PROSPECTIVEPURELYTEMPORAL || analysis_type == Parameters.AnalysisType.PROSPECTIVESPACETIME) {
            _cutoff_email.setText("If recurrence interval greater than or equal to");
            if (val <= 1) _cutoff_value_email.setText(AppConstants.DEFAULT_RECURRENCE_CUTOFF);
            _cutoff_email_label.setText("days, send email with summary results to (csv list):");
        } else {
            _cutoff_email.setText("If p-value less than or equal to");
            if (val > 1) _cutoff_value_email.setText(AppConstants.DEFAULT_PVALUE_CUTOFF);
            _cutoff_email_label.setText("send email with summary results to (csv list):");
        }
        
        boolean emailingSomeone = Utils.selected(_always_sendmail) || Utils.selected(_cutoff_email);
        _always_email_recipients.setEnabled(Utils.selected(_always_sendmail));
        _cutoff_value_email.setEnabled(Utils.selected(_cutoff_email));
        _cutoff_email_label.setEnabled(Utils.selected(_cutoff_email));
        _cutoff_email_recipients.setEnabled(Utils.selected(_cutoff_email));
        _attach_main_results_email.setEnabled(emailingSomeone);
        _create_custom_email_message.setEnabled(emailingSomeone);
        _report_main_results_email.setEnabled(emailingSomeone && !Utils.selected(_create_custom_email_message));
        _custom_email_subject_label.setEnabled(Utils.selected(_create_custom_email_message));
        _custom_email_subject.setEnabled(Utils.selected(_create_custom_email_message));
        _custom_email_tags.setEnabled(Utils.selected(_create_custom_email_message));
        _custom_email_message_label.setEnabled(Utils.selected(_create_custom_email_message));
        _custom_email_message.setEnabled(Utils.selected(_create_custom_email_message));
        
        if (!Utils.selected(_create_custom_email_message)) {
            _custom_email_subject.setText(Parameters.DEFAULT_EMAIL_SUBJECT);
            _custom_email_message.setText(substituteNewlines(Parameters.DEFAULT_EMAIL_MESSAGE));
        }
    }   

    /* Enables controls of the other output group. */
    public void enableOtherOutputGroup() {
        boolean hasLineListData = false;
        for (int idx=1; idx <= _dataSetsListModel.size() + 1 && !hasLineListData; ++idx) {
            String key = InputSourceSettings.InputFileType.Case.toString() + "1";
            if (_settings_window._input_source_map.containsKey(key)) {
                InputSourceSettings inputSourceSettings = (InputSourceSettings)_settings_window._input_source_map.get(key);
                hasLineListData = !inputSourceSettings.getLinelistFieldMaps().isEmpty();
            }
            
        }
        _cluster_lineline_panel.setEnabled(hasLineListData);
        _cluster_lineline_prelabel.setEnabled(hasLineListData);
        _cluster_lineline_value.setEnabled(hasLineListData);
        _cluster_lineline_label.setEnabled(hasLineListData);
        if (hasLineListData) {
            double val = Double.parseDouble(_cluster_lineline_value.getText());
            boolean prospectiveAnalysis = (
                _settings_window.getAnalysisControlType() == Parameters.AnalysisType.PROSPECTIVEPURELYTEMPORAL ||
                _settings_window.getAnalysisControlType() == Parameters.AnalysisType.PROSPECTIVESPACETIME
            );
            if (prospectiveAnalysis) {
                _cluster_lineline_prelabel.setText("Include clusters with recurrence interval greater than or equal to");
                if (val < 1) _cluster_lineline_value.setText(AppConstants.DEFAULT_RECURRENCE_CUTOFF_CSV_LINELIST);
                _cluster_lineline_label.setText("days in line list CSV file.");
            } else {
                _cluster_lineline_prelabel.setText("Include clusters with p-value less than or equal to");
                if (val > 1) _cluster_lineline_value.setText(AppConstants.DEFAULT_PVALUE_CUTOFF_CSV_LINELIST);
                _cluster_lineline_label.setText("in line list CSV file.");
            }        
        }
    }    
    
    /**
     * Enables neighbors file group.
     */
    private void enableNonEucludianNeighborsGroup(boolean bEnable) {
        bEnable &= !(_locations_network.isEnabled() && _locations_network.isSelected());
        _specialNeighborFilesGroup.setEnabled(bEnable);
        _specifiyNeighborsFileCheckBox.setEnabled(bEnable);
        _neighborsFileTextField.setEnabled(bEnable && _specifiyNeighborsFileCheckBox.isSelected());
        _neighborsFileBrowseButton.setEnabled(bEnable && _specifiyNeighborsFileCheckBox.isSelected());

        _specifiyMetaLocationsFileCheckBox.setEnabled(bEnable && _specifiyNeighborsFileCheckBox.isSelected());
        _metaLocationsFileTextField.setEnabled(bEnable && _specifiyMetaLocationsFileCheckBox.isEnabled() && _specifiyMetaLocationsFileCheckBox.isSelected());
        _metaLocationsFileBrowseButton.setEnabled(bEnable && _specifiyMetaLocationsFileCheckBox.isEnabled() && _specifiyMetaLocationsFileCheckBox.isSelected());
    }

    private void enableTemporalStudyPeriodCheckGroup(boolean bEnable) {
        _strictStudyPeriodCheckRadioButton.setEnabled(bEnable);
        _relaxedStudyPeriodCheckRadioButton.setEnabled(bEnable);
    }

    private void enableCoordinatesCheckGroup(boolean bEnable) {
        _strictCoordinatesRadioButton.setEnabled(bEnable);
        _relaxedCoordinatesRadioButton.setEnabled(bEnable);
        _strictCoordinatesLabel.setEnabled(bEnable);
    }

    private void enableMultipleLocationsGroup(boolean bEnable) {
        bEnable &= !_specifiyNeighborsFileCheckBox.isSelected();
        
        _multipleSetsSpatialCoordinatesGroup.setEnabled(bEnable);
        _onePerLocationIdRadioButton.setEnabled(bEnable);
        _atLeastOneRadioButton.setEnabled(bEnable);
        _allLocationsRadioButton.setEnabled(bEnable);
        
        _multiple_locations_file_label.setEnabled(bEnable && (Utils.selected(_atLeastOneRadioButton) || Utils.selected(_allLocationsRadioButton)));
        _multiple_locations_file.setEnabled(bEnable && (Utils.selected(_atLeastOneRadioButton) || Utils.selected(_allLocationsRadioButton)));
        _multiple_locations_file_browse.setEnabled(bEnable && (Utils.selected(_atLeastOneRadioButton) || Utils.selected(_allLocationsRadioButton)));
    }

    /* enables adjustment options controls */
    private void enableRelativeRiskAdjustmentsGroup(boolean bEnable) {
        _knownAdjustmentsGroup.setEnabled(bEnable);
        _adjustForKnownRelativeRisksCheckBox.setEnabled(bEnable);
        _adjustmentsByRelativeRisksFileLabel.setEnabled(bEnable && _adjustForKnownRelativeRisksCheckBox.isSelected());
        _adjustmentsByRelativeRisksFileTextField.setEnabled(bEnable && _adjustForKnownRelativeRisksCheckBox.isSelected());
        _adjustmentsFileBrowseButton.setEnabled(bEnable && _adjustForKnownRelativeRisksCheckBox.isSelected());
    }

    private void enableIterativeScanOptionsGroup(boolean bEnable) {
        _iterativeScanGroup.setEnabled(bEnable);
        _performIterativeScanCheckBox.setEnabled(bEnable);
        _maxIterativeScansLabel.setEnabled(_performIterativeScanCheckBox.isSelected() && bEnable);
        _numIterativeScansTextField.setEnabled(_performIterativeScanCheckBox.isSelected() && bEnable);
        _iterativeCutoffLabel.setEnabled(_performIterativeScanCheckBox.isSelected() && bEnable);
        _iterativeScanCutoffTextField.setEnabled(_performIterativeScanCheckBox.isSelected() && bEnable);
    }

    public void enableLimitClustersMinimumCasesGroup(Parameters.AreaRateType scanrate) {
        boolean enableGroup = true;
        Parameters.ProbabilityModelType modeltype = _settings_window.getModelControlType();
        enableGroup &= !(modeltype == Parameters.ProbabilityModelType.ORDINAL || modeltype == Parameters.ProbabilityModelType.CATEGORICAL);
        enableGroup &= (scanrate == Parameters.AreaRateType.HIGH || scanrate == Parameters.AreaRateType.HIGHANDLOW);
        _minimum_clusters_group.setEnabled(enableGroup);
        _min_cases_label.setEnabled(_minimum_clusters_group.isEnabled());
        _minimum_number_cases_cluster.setEnabled(_minimum_clusters_group.isEnabled());
        _min_cases_label2.setEnabled(_minimum_clusters_group.isEnabled());
    }    
    
    public void enableLimitClustersByRiskLevelGroup(Parameters.AreaRateType scanrate) {
        Parameters.ProbabilityModelType modeltype = _settings_window.getModelControlType();
        boolean enableGroup = !(
            modeltype == Parameters.ProbabilityModelType.ORDINAL ||
            modeltype == Parameters.ProbabilityModelType.CATEGORICAL ||
            modeltype == Parameters.ProbabilityModelType.NORMAL
        );
        _limit_clusters_risk_group.setEnabled(enableGroup);
        switch (_settings_window.getModelControlType()) {
            case EXPONENTIAL: 
                _limit_high_clusters.setText("Restrict short survival clusters to observed/expected greater than or equal to:");
                _limit_low_clusters.setText("Restrict long survival clusters to observed/expected less than or equal to:");
                break;
            case SPACETIMEPERMUTATION:
                _limit_high_clusters.setText("Restrict high rate clusters to observed/expected greater than or equal to:");
                _limit_low_clusters.setText("Restrict low rate clusters to observed/expected less than or equal to:");
                break;
            case POISSON:
            case HOMOGENEOUSPOISSON:
            case BERNOULLI:
            default:
                _limit_high_clusters.setText("Restrict high rate clusters to relative risk greater than or equal to:");
                _limit_low_clusters.setText("Restrict low rate clusters to relative risk less than or equal to:");
        }
        
        _limit_high_clusters.setEnabled(_limit_clusters_risk_group.isEnabled() &&
                                        (scanrate == Parameters.AreaRateType.HIGH || scanrate == Parameters.AreaRateType.HIGHANDLOW));
        _limit_high_clusters_value.setEnabled(_limit_high_clusters.isEnabled() && _limit_high_clusters.isSelected());
        
        _limit_low_clusters.setEnabled(_limit_clusters_risk_group.isEnabled() &&
                                       (scanrate == Parameters.AreaRateType.LOW || scanrate == Parameters.AreaRateType.HIGHANDLOW));
        _limit_low_clusters_value.setEnabled(_limit_low_clusters.isEnabled() && _limit_low_clusters.isSelected());            
    }
    
    private void enablePValueOptionsGroup() {
        boolean bPoisson = _settings_window.getModelControlType() == Parameters.ProbabilityModelType.POISSON,
                bBernoulli = _settings_window.getModelControlType() == Parameters.ProbabilityModelType.BERNOULLI,
                bSTP = _settings_window.getModelControlType() == Parameters.ProbabilityModelType.SPACETIMEPERMUTATION,
                bCategorical = _settings_window.getModelControlType() == Parameters.ProbabilityModelType.CATEGORICAL,
                bOrdinal = _settings_window.getModelControlType() == Parameters.ProbabilityModelType.ORDINAL;
        boolean bPurelySpatial = _settings_window.getAnalysisControlType() == Parameters.AnalysisType.PURELYSPATIAL,
                bSpaceTime = _settings_window.getAnalysisControlType() == Parameters.AnalysisType.PROSPECTIVESPACETIME
                || _settings_window.getAnalysisControlType() == Parameters.AnalysisType.SPACETIME;
        boolean gumbelEnabled = _settings_window.getAreaScanRateControlType() == Parameters.AreaRateType.HIGH && (
            ((bPoisson || bBernoulli || bCategorical || bOrdinal) && bPurelySpatial) ||
            ((bPoisson || bBernoulli || bSTP) && bSpaceTime) 
        );
        _radioGumbelPValues.setEnabled(gumbelEnabled);
        if (_radioGumbelPValues.isEnabled() == false && _radioGumbelPValues.isSelected())
            _radioDefaultPValues.setSelected(true);
        _checkReportGumbel.setEnabled(gumbelEnabled && (_radioEarlyTerminationPValues.isSelected() || _radioStandardPValues.isSelected()));
        if (_checkReportGumbel.isEnabled() == false && _checkReportGumbel.isSelected())
            _checkReportGumbel.setSelected(false);
        _earlyTerminationThreshold.setEnabled(_radioEarlyTerminationPValues.isSelected());
    }

    public void enableSettingsForAnalysisModelCombination() {
        boolean bPoisson = _settings_window.getModelControlType() == Parameters.ProbabilityModelType.POISSON,
                bSpaceTimePermutation = _settings_window.getModelControlType() == Parameters.ProbabilityModelType.SPACETIMEPERMUTATION,
                bExponential = _settings_window.getModelControlType() == Parameters.ProbabilityModelType.EXPONENTIAL,
                bBernoulli = _settings_window.getModelControlType() == Parameters.ProbabilityModelType.BERNOULLI,
                bH_Poisson = _settings_window.getModelControlType() == Parameters.ProbabilityModelType.HOMOGENEOUSPOISSON,
                bUniformTime = _settings_window.getModelControlType() == Parameters.ProbabilityModelType.UNIFORMTIME;

        switch (_settings_window.getAnalysisControlType()) {
            case PURELYSPATIAL:
                enableAdjustmentForTimeTrendOptionsGroup(false, false, false, false);
                enableAdjustmentForSpatialOptionsGroup(false, false);
                enableSpatialOptionsGroup(true, false);
                enableWindowShapeGroup(true);
                enableTemporalOptionsGroup(false, false, false);
                enableClustersReportedOptions(true);
                enableCoordinatesCheckGroup(true);
                enableTemporalStudyPeriodCheckGroup(!bH_Poisson);
                enableNonEucludianNeighborsGroup(!bH_Poisson);
                enableNetworkGroup(!bH_Poisson);
                enableMultipleLocationsGroup(!bH_Poisson);
                enableAdditionalDataSetsGroup(true);
                enableIsotonicScan(bPoisson || bBernoulli);
                enableIterativeScanOptionsGroup(!bH_Poisson);
                enablePValueOptionsGroup();
                enableAdjustDayOfWeek(false);
                enableTemporalGraphsGroup(false);
                enableMiscellaneousAnalysisGroup(bPoisson, false);
                break;
            case PURELYTEMPORAL:
            case SEASONALTEMPORAL:
                enableAdjustmentForTimeTrendOptionsGroup(bPoisson, false, bPoisson, bPoisson);
                enableAdjustmentForSpatialOptionsGroup(false, false);
                enableSpatialOptionsGroup(false, false);
                enableWindowShapeGroup(false);
                enableTemporalOptionsGroup(true, false, true);
                enableClustersReportedOptions(false);
                enableCoordinatesCheckGroup(false);
                enableTemporalStudyPeriodCheckGroup(true);
                enableNonEucludianNeighborsGroup(false);
                enableNetworkGroup(false);
                enableMultipleLocationsGroup(false);
                enableAdditionalDataSetsGroup(true);
                enableIsotonicScan(false);
                enableIterativeScanOptionsGroup(true);
                enablePValueOptionsGroup();
                enableAdjustDayOfWeek(bPoisson || bSpaceTimePermutation);
                enableTemporalGraphsGroup(bPoisson || bSpaceTimePermutation || bBernoulli || bExponential || bUniformTime);
                enableMiscellaneousAnalysisGroup(false, false);
                break;
            case SPACETIME:
                enableAdjustmentForTimeTrendOptionsGroup(bPoisson, bPoisson, bPoisson, bPoisson);
                enableAdjustmentForSpatialOptionsGroup(true, bPoisson);
                enableSpatialOptionsGroup(true, !(bSpaceTimePermutation || Utils.selected(_temporalTrendAdjNonparametric)));
                enableWindowShapeGroup(true);
                enableTemporalOptionsGroup(true, !(bSpaceTimePermutation || bUniformTime), true);
                enableClustersReportedOptions(true);
                enableCoordinatesCheckGroup(true);
                enableTemporalStudyPeriodCheckGroup(true);
                enableNonEucludianNeighborsGroup(true);
                enableNetworkGroup(true);
                enableMultipleLocationsGroup(true);
                enableAdditionalDataSetsGroup(true);
                enableIsotonicScan(false);
                enableIterativeScanOptionsGroup(false);
                enablePValueOptionsGroup();
                enableAdjustDayOfWeek(bPoisson || bSpaceTimePermutation);
                enableTemporalGraphsGroup(bPoisson || bSpaceTimePermutation || bBernoulli || bExponential || bUniformTime);
                enableMiscellaneousAnalysisGroup(false, false);
                break;
            case PROSPECTIVESPACETIME:
                enableAdjustmentForTimeTrendOptionsGroup(
                    bPoisson || (bBernoulli && _settings_window.getAreaScanRateControlType() == Parameters.AreaRateType.HIGH), 
                    bPoisson || (bBernoulli && _settings_window.getAreaScanRateControlType() == Parameters.AreaRateType.HIGH),
                    bPoisson, bPoisson
                );
                enableAdjustmentForSpatialOptionsGroup(true, bPoisson);
                enableSpatialOptionsGroup(true, !(bSpaceTimePermutation || Utils.selected(_temporalTrendAdjNonparametric)));
                enableWindowShapeGroup(true);
                enableTemporalOptionsGroup(true, !(bSpaceTimePermutation || bUniformTime), false);
                enableClustersReportedOptions(true);
                enableCoordinatesCheckGroup(true);
                enableTemporalStudyPeriodCheckGroup(true);
                enableNonEucludianNeighborsGroup(true);
                enableNetworkGroup(true);
                enableMultipleLocationsGroup(true);
                enableAdditionalDataSetsGroup(true);
                enableIsotonicScan(false);
                enableIterativeScanOptionsGroup(false);
                enablePValueOptionsGroup();
                enableAdjustDayOfWeek(bPoisson || bSpaceTimePermutation);
                enableTemporalGraphsGroup(bPoisson || bSpaceTimePermutation || bBernoulli || bExponential || bUniformTime);
                enableMiscellaneousAnalysisGroup(false, true);
                break;
            case PROSPECTIVEPURELYTEMPORAL:
                enableAdjustmentForTimeTrendOptionsGroup(bPoisson, false, bPoisson, bPoisson);
                enableAdjustmentForSpatialOptionsGroup(false, false);
                enableSpatialOptionsGroup(false, false);
                enableWindowShapeGroup(false);
                enableTemporalOptionsGroup(true, false, false);
                enableClustersReportedOptions(false);
                enableCoordinatesCheckGroup(false);
                enableTemporalStudyPeriodCheckGroup(true);
                enableNonEucludianNeighborsGroup(false);
                enableNetworkGroup(false);
                enableMultipleLocationsGroup(false);
                enableAdditionalDataSetsGroup(true);
                enableIsotonicScan(false);
                enableIterativeScanOptionsGroup(true);
                enablePValueOptionsGroup();
                enableAdjustDayOfWeek(bPoisson || bSpaceTimePermutation);
                enableTemporalGraphsGroup(bPoisson || bSpaceTimePermutation || bBernoulli || bExponential || bUniformTime);
                enableMiscellaneousAnalysisGroup(false, true);
                break;
            case SPATIALVARTEMPTREND:
                enableAdjustmentForTimeTrendOptionsGroup(false, false, false, false);
                enableAdjustmentForSpatialOptionsGroup(false, false);
                enableSpatialOptionsGroup(true, false);
                enableTemporalOptionsGroup(false, false, false);
                enableClustersReportedOptions(true);
                enableCoordinatesCheckGroup(true);
                enableTemporalStudyPeriodCheckGroup(true);
                enableNonEucludianNeighborsGroup(true);
                enableNetworkGroup(true);
                enableIterativeScanOptionsGroup(true);
                enableMultipleLocationsGroup(true);
                enableAdditionalDataSetsGroup(false);
                enableIsotonicScan(false);
                enableWindowShapeGroup(true);
                enablePValueOptionsGroup();
                enableAdjustDayOfWeek(bPoisson || bSpaceTimePermutation);
                enableTemporalGraphsGroup(false);                
                enableMiscellaneousAnalysisGroup(false, false);
                break;
        }
        enableLimitClustersMinimumCasesGroup(_settings_window.getAreaScanRateControlType());
        enableLimitClustersByRiskLevelGroup(_settings_window.getAreaScanRateControlType());
        enableClustersReportedGroup();
        enablePowerEvaluationsGroup();
        enableRelativeRiskAdjustmentsGroup(bPoisson);
        updateMonteCarloTextCaptions();
        enableMapsOutputGroup();
        enableDrilldownGroup();
        setSpatialDistanceCaption();
        updateMaximumTemporalSizeTextCaptions();
        enableOtherOutputGroup();
        enableEmailAlerts();
    }

    /* Enables controls on the 'Miscellaneous' tab. */
    public void enableMiscellaneousAnalysisGroup(boolean enableOliveira, boolean enableProspectiveFreq) {
        _oliveiras_f_group.setEnabled(enableOliveira);
        _calculate_oliveiras_f.setEnabled(enableOliveira);
        _number_oliveira_data_sets_label.setEnabled(enableOliveira);
        _number_oliveira_data_sets.setEnabled(enableOliveira);
        _prospective_frequency_group.setEnabled(enableProspectiveFreq);
        _label_prospective_frequency.setEnabled(enableProspectiveFreq);
        _prospective_frequency.setEnabled(enableProspectiveFreq);
    }
        
    public boolean isAdjustingForDayOfWeek() {
        return _adjustDayOfWeek.isEnabled() && _adjustDayOfWeek.isSelected();
    }

    public void enableAdjustDayOfWeek(boolean enable) {
        enable &= !Utils.selected(_temporalTrendAdjNonparametric);
        _adjustDayOfWeek.setEnabled(enable);
        if (_settings_window.getModelControlType() == Parameters.ProbabilityModelType.SPACETIMEPERMUTATION) {
            _adjustDayOfWeek.setText("Adjust for day-of-week by space interaction");
        } else {
            _adjustDayOfWeek.setText("Adjust for day-of-week");
        }
    }

    /**
     * Sets caption of spatial distance radio button based upon coordinates
     * group setting.
     */
    public void setSpatialDistanceCaption() {
        String sRadioCaption = "", sLabelCaption = "";

        if (Utils.selected(_locations_network))
            sRadioCaption = "is a distance of ";
        else
            sRadioCaption = String.format("is %1$s with a", (_circularRadioButton.isSelected() ? "a circle" : "an ellipse"));
        _distancePrefixLabel.setText(sRadioCaption);
        _reportedMaxDistanceLabel.setText(sRadioCaption);
        switch (_settings_window.getCoordinatesType()) {
            case CARTESIAN:
                sLabelCaption = String.format("Cartesian units %1$s", (_circularRadioButton.isSelected() ? "radius" : "minor axis"));
                break;
            case LATLON:
                if (Utils.selected(_locations_network))
                    sLabelCaption = "kilometers";
                else
                    sLabelCaption = String.format("kilometer %1$s", (_circularRadioButton.isSelected() ? "radius" : "minor axis"));
                break;
        }
        _maxRadiusLabel.setText(sLabelCaption);
        _maxReportedRadiusLabel.setText(sLabelCaption);
    }

    /* Updates the text of the maximum temporal cluster size label based on user settings. */
    public void updateMaximumTemporalSizeTextCaptions() {
        Parameters.AnalysisType analysisType = _settings_window.getAnalysisControlType();
        boolean ptAnalysis = (
            analysisType == Parameters.AnalysisType.PURELYTEMPORAL ||
            analysisType == Parameters.AnalysisType.PROSPECTIVEPURELYTEMPORAL ||
            analysisType == Parameters.AnalysisType.SEASONALTEMPORAL
        );
        
        String unitsLabel = "";
        switch (_settings_window.getTimeAggregationControlType()) {
            case YEAR:
                unitsLabel = "years";
                break;
            case MONTH:
                unitsLabel = "months";
                break;
            case DAY:
                unitsLabel = "days";
                break;
            case GENERIC:
                unitsLabel = "units";
                break;
        }
        
        _minTemporalTimeUnitsLabel.setText(unitsLabel);
        
        switch (_settings_window.getModelControlType()) {
            case POISSON:
            case HOMOGENEOUSPOISSON:
            case BERNOULLI:
            case ORDINAL:
            case CATEGORICAL:
            case NORMAL:
            case EXPONENTIAL:
                // Skip for purely temporal analysis or applying spatial adjustment.
                if (!(ptAnalysis || Utils.selected(_spatialAdjustmentsNonparametric))) {
                    _percentageOfStudyPeriodLabel.setText("percent of the study period (<= 90%, default = 50%)");
                    _maxTemporalTimeUnitsLabel.setText(unitsLabel + " (<=90% of the study period)");
                    break;
                }
            case SPACETIMEPERMUTATION:
                _percentageOfStudyPeriodLabel.setText("percent of the study period (<= 50%, default = 50%)");
                _maxTemporalTimeUnitsLabel.setText(unitsLabel + " (<=50% of the study period)");
                break;
        }
    }

    public void updateCriticalValuesTextCaptions() {
        _criticalValuesReplicationsLabel.setText("(" + _montCarloReplicationsTextField.getText() + " replications - value defined on Inference tab)");
    }

    public void updateMonteCarloTextCaptions() {
        switch (getPValueReportingControlType()) {
            case DEFAULT_PVALUE:
            case TERMINATION_PVALUE:
                _labelMonteCarloReplications.setText("Maximum number of replications (0, 9, 999, or value ending in 999):");
                break;
            case STANDARD_PVALUE:
            case GUMBEL_PVALUE:
                _labelMonteCarloReplications.setText("Number of replications (0, 9, 999, or value ending in 999):");
                break;
        }
    }

    /**
     * Enables/disables TListBox that list defined data sets
     */
    private void enableDataSetList() {
        boolean bEnable = _additionalDataSetsGroup.isEnabled() && _dataSetsListModel.getSize() > 0;
        _inputDataSetsList.setEnabled(bEnable);
    }

    private void enableNetworkGroup(boolean enable) {
        enable &= !(_specifiyNeighborsFileCheckBox.isEnabled() && _specifiyNeighborsFileCheckBox.isSelected());
        _network_group.setEnabled(enable);
        _locations_network.setEnabled(enable);
        _network_file_label.setEnabled(enable && _locations_network.isSelected());
        _network_filename.setEnabled(enable && _locations_network.isSelected());
        _browse_network_filename.setEnabled(enable && _locations_network.isSelected());
    }
    
    /**
     * enables or disables the New button on the Input tab
     */
    private void enableNewButton() {
        boolean bEnable = _additionalDataSetsGroup.isEnabled();
        _addDataSetButton.setEnabled(_inputDataSetsList.getModel().getSize() < MAXIMUM_ADDITIONAL_SETS ? bEnable : false);
    }

    /**
     * enables or disables the New button on the Input tab
     */
    private void enableRemoveButton() {
        boolean bEnable = _additionalDataSetsGroup.isEnabled();
        _removeDataSetButton.setEnabled(_inputDataSetsList.getModel().getSize() > 0 ? bEnable : false);
    }

    /**
     * Checks to determine if only default values are set in the dialog Returns
     * true if only default values are set Returns false if user specified a
     * value other than a default
     */
    public boolean getDefaultsSetForInputOptions() {
        boolean bReturn = true;

        bReturn &= Utils.sizeIs(_inputDataSetsList, 0);
        bReturn &= Utils.selected(_multivariateAdjustmentsRadioButton, true);
        bReturn &= Utils.selected(_strictStudyPeriodCheckRadioButton, true);
        bReturn &= Utils.selected(_strictCoordinatesRadioButton, true);
        bReturn &= Utils.selected(_specifiyNeighborsFileCheckBox, false);
        bReturn &= Utils.textIs(_neighborsFileTextField, "");
        bReturn &= Utils.selected(_specifiyMetaLocationsFileCheckBox, false);
        bReturn &= Utils.textIs(_metaLocationsFileTextField, "");
        bReturn &= Utils.selected(_onePerLocationIdRadioButton, true);
        bReturn &= Utils.textIs(_multiple_locations_file, "");
        bReturn &= Utils.selected(_locations_network, false);
        bReturn &= Utils.textIs(_network_filename, "");
        return bReturn;
    }

//	** Checks to determine if only default values are set in the dialog
//	** Returns true if only default values are set on enabled controls
//	** Returns false if user specified a value other than a default
    public boolean getDefaultsSetForAnalysisOptions() {
        boolean bReturn = true;

        // Inference tab
        bReturn &= Utils.selected(_reportCriticalValuesCheckBox, false);
        bReturn &= Utils.selected(_radioDefaultPValues, true);
        bReturn &= Utils.selected(_checkReportGumbel, false);
        bReturn &= Utils.selected(_performIterativeScanCheckBox, false);
        bReturn &= Utils.integerIs(_numIterativeScansTextField, 10);
        bReturn &= Utils.doubleIs(_iterativeScanCutoffTextField, 0.05);
        bReturn &= Utils.integerIs(_montCarloReplicationsTextField, 999);

        // Spatial Window tab
        bReturn &= Utils.doubleIs(_maxSpatialClusterSizeTextField, 50.0);
        bReturn &= Utils.selected(_spatialPopulationFileCheckBox, false);
        bReturn &= Utils.doubleIs(_maxSpatialPercentFileTextField, 50.0);
        bReturn &= Utils.doubleIs(_maxSpatialRadiusTextField, 1.0);
        bReturn &= Utils.selected(_spatialDistanceCheckBox, false);
        bReturn &= Utils.textIs(_maxCirclePopulationFilenameTextField, "");
        bReturn &= Utils.selected(_inclPureTempClustCheckBox, false);
        bReturn &= Utils.selected(_circularRadioButton, true);
        bReturn &= Utils.selectionIs(_nonCompactnessPenaltyComboBox, 1);
        bReturn &= Utils.selected(_onePerLocationIdRadioButton, true);

        // Temporal tab
        bReturn &= Utils.selected(_percentageTemporalRadioButton, true);
        bReturn &= Utils.doubleIs(_maxTemporalClusterSizeTextField, 50.0);
        bReturn &= Utils.integerIs(_maxTemporalClusterSizeUnitsTextField, 1);
        bReturn &= Utils.integerIs(_minTemporalClusterSizeUnitsTextField, 1);
        bReturn &= Utils.selected(_includePureSpacClustCheckBox, false);
        bReturn &= Utils.integerIs(_startRangeStartYearTextField, 1900) || Utils.integerIs(_startRangeStartYearTextField, 2000);
        bReturn &= Utils.integerIs(_startRangeStartMonthTextField, 1);
        bReturn &= Utils.integerIs(_startRangeStartDayTextField, 1);
        bReturn &= Utils.integerIs(_startRangeEndYearTextField, 1900) || Utils.integerIs(_startRangeEndYearTextField, 2000);
        bReturn &= Utils.integerIs(_startRangeEndMonthTextField, 1) || Utils.integerIs(_startRangeEndMonthTextField, 12);
        bReturn &= Utils.integerIs(_startRangeEndDayTextField, 1) || Utils.integerIs(_startRangeEndDayTextField, 31);
        bReturn &= Utils.integerIs(_endRangeStartYearTextField, 1900) || Utils.integerIs(_endRangeStartYearTextField, 2000);
        bReturn &= Utils.integerIs(_endRangeStartMonthTextField, 1) || Utils.integerIs(_endRangeStartMonthTextField, 12);
        bReturn &= Utils.integerIs(_endRangeStartDayTextField, 1) || Utils.integerIs(_endRangeStartDayTextField, 31);
        bReturn &= Utils.integerIs(_endRangeEndYearTextField, 1900) || Utils.integerIs(_endRangeEndYearTextField, 2000);
        bReturn &= Utils.integerIs(_endRangeEndMonthTextField, 12);
        bReturn &= Utils.integerIs(_endRangeEndDayTextField, 31);
        bReturn &= Utils.selected(_restrictTemporalRangeCheckBox, false);

        // Risk tab
        bReturn &= Utils.selected(_adjustForKnownRelativeRisksCheckBox, false);
        bReturn &= Utils.textIs(_adjustmentsByRelativeRisksFileTextField, "");
        bReturn &= Utils.selected(_temporalTrendAdjNone, true);
        bReturn &= Utils.doubleIs(_logLinearTextField, 0.0);
        bReturn &= Utils.selected(_spatialAdjustmentsNone, true);

        // Power Evaluations tab
        bReturn &= Utils.selected(_performPowerEvalautions, false);
        bReturn &= Utils.selected(_partOfRegularAnalysis, true);
        bReturn &= Utils.integerIs(_totalPowerCases, 600);
        bReturn &= Utils.selected(_criticalValuesMonteCarlo, true);
        bReturn &= Utils.selected(_powerEstimationMonteCarlo, true);
        bReturn &= Utils.textIs(_alternativeHypothesisFilename, "");
        bReturn &= Utils.integerIs(_numberPowerReplications, 1000);

        // Cluster Restrictions
        bReturn &= Utils.integerIs(_minimum_number_cases_cluster, 2);
        bReturn &= Utils.selected(_limit_high_clusters, false);
        bReturn &= Utils.doubleIs(_limit_high_clusters_value, 1.0);
        bReturn &= Utils.selected(_limit_low_clusters, false);
        bReturn &= Utils.doubleIs(_limit_low_clusters_value, 1.0);
        
        // Drilldown
        bReturn &= Utils.selected(_mainAnalysisDrilldown, false);
        bReturn &= Utils.selected(_purelySpatialDrilldown, false);
        bReturn &= Utils.doubleIs(_drilldown_restriction_cutoff, 0.05);
        bReturn &= Utils.integerIs(_drilldown_restriction_locations, 2);
        bReturn &= Utils.integerIs(_drilldown_restriction_cases, 10);
        bReturn &= Utils.selected(_drilldown_restriction_dow, false);
        
        // Miscellaneous
        bReturn &= Utils.selected(_calculate_oliveiras_f, false);
        bReturn &= Utils.integerIs(_number_oliveira_data_sets, 1000);
        bReturn &= Utils.selectionIs(_prospective_frequency, 0);
        
        return bReturn;
    }
    
    /* Checks to determine if only default values are set in the dialog
       Returns true if only default values are set
       Returns false if user specified a value other than a default
    */
    public boolean getDefaultsSetForOutputOptions() {
        boolean bReturn = true;

        // Output tab
        bReturn &= Utils.selected(_mostLikelyClustersHierarchically, true);
        bReturn &= Utils.selected(_giniOptimizedClusters, true);
        bReturn &= Utils.selectionIs(_hierarchicalSecondaryClusters, 0);
        bReturn &= Utils.selected(_checkboxReportIndexCoefficients, false);
        bReturn &= Utils.doubleIs(_maxReportedSpatialClusterSizeTextField, 50.0);
        bReturn &= Utils.doubleIs(_maxReportedSpatialPercentFileTextField, 50.0);
        bReturn &= Utils.selected(_reportedSpatialPopulationFileCheckBox, false);
        bReturn &= Utils.doubleIs(_maxReportedSpatialRadiusTextField, 1.0);
        bReturn &= Utils.selected(_reportedSpatialDistanceCheckBox, false);
        bReturn &= Utils.selected(_restrictReportedClustersCheckBox, false);
        bReturn &= Utils.selected(_reportClusterRankCheckBox, false);
        bReturn &= Utils.selected(_printAsciiColumnHeaders, false);
        bReturn &= Utils.textIs(_printTitle, "");
        bReturn &= Utils.selected(_reportTemporalGraph, false);
        bReturn &= Utils.selected(_temporalGraphMostLikely, true);
        bReturn &= Utils.integerIs(_numMostLikelyClustersGraph, 1);
        bReturn &= Utils.doubleIs(_temporalGraphPvalueCutoff, 0.05);                
        bReturn &= Utils.selected(_includeClusterLocationsInKML, true);
        bReturn &= Utils.selected(_createCompressedKMZ, false);
        bReturn &= Utils.selected(_launch_map_viewer, true);
        bReturn &= Utils.doubleIs(_cluster_lineline_value, 0.05); 
        bReturn &= Utils.selected(_always_sendmail, false);
        bReturn &= Utils.textIs(_always_email_recipients, "");
        bReturn &= Utils.selected(_cutoff_email, false);
        bReturn &= Utils.textIs(_cutoff_email_recipients, "");
        bReturn &= Utils.doubleIs(_cutoff_value_email, 0.05); 
        bReturn &= Utils.selected(_attach_main_results_email, false);
        bReturn &= Utils.selected(_report_main_results_email, false);
        bReturn &= Utils.selected(_create_custom_email_message, false);
        bReturn &= Utils.textIs(_custom_email_subject, "");
        bReturn &= Utils.textIs(_custom_email_message, "");

        return bReturn;
    }

    /* Returns the spatial adjustment type based upon selected controls. */
    private Parameters.SpatialAdjustmentType getSpatialAdjustmentType() {
        Parameters.SpatialAdjustmentType eReturn = Parameters.SpatialAdjustmentType.SPATIAL_NOTADJUSTED;
        if (Utils.selected(_spatialAdjustmentsNonparametric)) {
            /* If temporal non-parametric (time stratified) is also selected, then spatial adjustment
               type is non-parametric, not spatially stratfied randomization.
            */
            if (Utils.selected(_temporalTrendAdjNonparametric))
                eReturn = Parameters.SpatialAdjustmentType.SPATIAL_NONPARAMETRIC;
            else
                eReturn = Parameters.SpatialAdjustmentType.SPATIAL_STRATIFIED_RANDOMIZATION;
        }
        return eReturn;        
    }

    private Parameters.CriteriaSecondaryClustersType getCriteriaSecondaryClustersType() {
        Parameters.CriteriaSecondaryClustersType eReturn = null;
        if (_hierarchicalSecondaryClusters.getSelectedIndex() == 0) {
            eReturn = Parameters.CriteriaSecondaryClustersType.NOGEOOVERLAP;
        } else if (_hierarchicalSecondaryClusters.getSelectedIndex() == 1) {
            eReturn = Parameters.CriteriaSecondaryClustersType.NOCENTROIDSINOTHER;
        } else if (_hierarchicalSecondaryClusters.getSelectedIndex() == 2) {
            eReturn = Parameters.CriteriaSecondaryClustersType.NOCENTROIDSINMORELIKE;
        } else if (_hierarchicalSecondaryClusters.getSelectedIndex() == 3) {
            eReturn = Parameters.CriteriaSecondaryClustersType.NOCENTROIDSINLESSLIKE;
        } else if (_hierarchicalSecondaryClusters.getSelectedIndex() == 4) {
            eReturn = Parameters.CriteriaSecondaryClustersType.NOPAIRSINEACHOTHERS;
        } else if (_hierarchicalSecondaryClusters.getSelectedIndex() == 5) {
            eReturn = Parameters.CriteriaSecondaryClustersType.NORESTRICTIONS;
        } else {
            throw new IllegalArgumentException("No secondary clusters option selected.");
        }
        return eReturn;
    }

    private Parameters.GiniIndexReportType getIndexBasedClusterReportType() {
        return Parameters.GiniIndexReportType.OPTIMAL_ONLY;
    }

    private Parameters.NonCompactnessPenaltyType getNonCompactnessPenaltyType() {
        Parameters.NonCompactnessPenaltyType eReturn = null;

        if (_nonCompactnessPenaltyComboBox.getSelectedIndex() == 0) {
            eReturn = Parameters.NonCompactnessPenaltyType.NOPENALTY;
        } else if (_nonCompactnessPenaltyComboBox.getSelectedIndex() == 1) {
            eReturn = Parameters.NonCompactnessPenaltyType.MEDIUMPENALTY;
        } else if (_nonCompactnessPenaltyComboBox.getSelectedIndex() == 2) {
            eReturn = Parameters.NonCompactnessPenaltyType.STRONGPENALTY;
        } else {
            throw new IllegalArgumentException("No non-compactness penalty option selected.");
        }
        return eReturn;
    }

    private Parameters.ProspectiveFrequency getProspectiveFrequencyControlType() {
        Parameters.ProspectiveFrequency eReturn = null;

        if (_prospective_frequency.getSelectedIndex() == 0) {
            eReturn = Parameters.ProspectiveFrequency.SAME_TIMEAGGREGATION;
        } else if (_prospective_frequency.getSelectedIndex() == 1) {
            eReturn = Parameters.ProspectiveFrequency.DAILY;
        } else if (_prospective_frequency.getSelectedIndex() == 2) {
            eReturn = Parameters.ProspectiveFrequency.WEEKLY;
        } else if (_prospective_frequency.getSelectedIndex() == 3) {
            eReturn = Parameters.ProspectiveFrequency.MONTHLY;
        } else if (_prospective_frequency.getSelectedIndex() == 4) {
            eReturn = Parameters.ProspectiveFrequency.QUARTERLY;
        } else if (_prospective_frequency.getSelectedIndex() == 5) {
            eReturn = Parameters.ProspectiveFrequency.YEARLY;
        } else {
            throw new IllegalArgumentException("No prospective frequency option selected.");
        }
        return eReturn;
    }    

    /*
    private Parameters.DatePrecisionType getClusterSignificantRecurrenceControlType() {
        if (!_cluster_significant_ri_type.isEnabled())
            return Parameters.DatePrecisionType.GENERIC;
        Parameters.DatePrecisionType eReturn;
        if (_cluster_significant_ri_type.getSelectedIndex() == 0) {
            eReturn = Parameters.DatePrecisionType.DAY;
        } else if (_cluster_significant_ri_type.getSelectedIndex() == 1) {
            eReturn = Parameters.DatePrecisionType.YEAR;
        } else {
            throw new IllegalArgumentException("No cluster siognificance option selected.");            
        }
        return eReturn;
    }     

    private void setClusterSignificantRecurrenceControlType(Parameters.DatePrecisionType etype) {
        switch (etype) {
            case YEAR: _cluster_significant_ri_type.select(1); break;
            case DAY:
            default: _cluster_significant_ri_type.select(0);
        }
    }*/   

    private Parameters.PowerEvaluationMethodType getPowerEvaluationMethodType() {
        Parameters.PowerEvaluationMethodType eReturn = null;

        if (_partOfRegularAnalysis.isSelected()) {
            eReturn = Parameters.PowerEvaluationMethodType.PE_WITH_ANALYSIS;
        } else if (_powerEvaluationWithCaseFile.isSelected()) {
            eReturn = Parameters.PowerEvaluationMethodType.PE_ONLY_CASEFILE;
        } else if (_powerEvaluationWithSpecifiedCases.isSelected()) {
            eReturn = Parameters.PowerEvaluationMethodType.PE_ONLY_SPECIFIED_CASES;
        } else {
            throw new IllegalArgumentException("No power evaluation option selected.");
        }
        return eReturn;
    }

    private Parameters.CriticalValuesType getCriticalValuesType() {
        Parameters.CriticalValuesType eReturn = null;

        if (_criticalValuesMonteCarlo.isSelected()) {
            eReturn = Parameters.CriticalValuesType.CV_MONTECARLO;
        } else if (_criticalValuesGumbel.isSelected()) {
            eReturn = Parameters.CriticalValuesType.CV_GUMBEL;
        } else {
            throw new IllegalArgumentException("No critical value option selected.");
        }
        return eReturn;
    }

    private Parameters.PowerEstimationType getPowerEstimationType() {
        Parameters.PowerEstimationType eReturn = null;

        if (_powerEstimationMonteCarlo.isSelected()) {
            eReturn = Parameters.PowerEstimationType.PE_MONTECARLO;
        } else if (_powerEstimationGumbel.isSelected()) {
            eReturn = Parameters.PowerEstimationType.PE_GUMBEL;
        } else {
            throw new IllegalArgumentException("No power estimation option selected.");
        }
        return eReturn;
    }

    /**
     * returns MultipleCoordinatesType for controls
     */
    Parameters.MultipleCoordinatesType getMultipleCoordinatesType() {
        Parameters.MultipleCoordinatesType eReturn;

        if (_onePerLocationIdRadioButton.isSelected()) {
            eReturn = Parameters.MultipleCoordinatesType.ONEPERLOCATION;
        } else if (_atLeastOneRadioButton.isSelected()) {
            eReturn = Parameters.MultipleCoordinatesType.ATLEASTONELOCATION;
        } else /* (_allLocationsRadioButton.isSelected()) */ {
            eReturn = Parameters.MultipleCoordinatesType.ALLLOCATIONS;
        }

        return eReturn;
    }

    /**
     * returns geographical coordinates data type for control
     */
    private Parameters.CoordinatesDataCheckingType getCoordinatesDataCheckingTypeFromControl() {
        Parameters.CoordinatesDataCheckingType eReturn = null;

        if (_strictCoordinatesRadioButton.isSelected()) {
            eReturn = Parameters.CoordinatesDataCheckingType.STRICTCOORDINATES;
        } else if (_relaxedCoordinatesRadioButton.isSelected()) {
            eReturn = Parameters.CoordinatesDataCheckingType.RELAXEDCOORDINATES;
        } else {
            throw new IllegalArgumentException("No coordinates data check option selected.");
        }
        return eReturn;
    }

    private String replaceNewlines(final String source) {
        String replaced = source.replace("\n", "<linebreak>");
        return replaced;
    }

    private String substituteNewlines(final String source) {
        String substituted = source.replace("<linebreak>", "\n");
        return substituted;        
    }
    
    /**
     * sets CParameters class with settings in form
     */
    public void saveParameterSettings(Parameters parameters) {
        String sString;

        parameters.SetRiskType(_performIsotonicScanCheckBox.isEnabled() && _performIsotonicScanCheckBox.isSelected() ? Parameters.RiskType.MONOTONERISK.ordinal() : Parameters.RiskType.STANDARDRISK.ordinal());
        parameters.UseLocationNeighborsFile(_specifiyNeighborsFileCheckBox.isSelected());
        parameters.SetLocationNeighborsFileName(_neighborsFileTextField.getText());
        parameters.UseMetaLocationsFile(_specifiyMetaLocationsFileCheckBox.isSelected());
        parameters.SetMetaLocationsFileName(_metaLocationsFileTextField.getText());
        parameters.SetMaxSpatialSizeForType(Parameters.SpatialSizeType.PERCENTOFPOPULATION.ordinal(), Double.parseDouble(_maxSpatialClusterSizeTextField.getText()), false);
        parameters.SetMaxSpatialSizeForType(Parameters.SpatialSizeType.PERCENTOFMAXCIRCLEFILE.ordinal(), Double.parseDouble(_maxSpatialPercentFileTextField.getText()), false);
        parameters.SetRestrictMaxSpatialSizeForType(Parameters.SpatialSizeType.PERCENTOFMAXCIRCLEFILE.ordinal(), _spatialPopulationFileCheckBox.isEnabled() && _spatialPopulationFileCheckBox.isSelected(), false);
        parameters.SetMaxSpatialSizeForType(Parameters.SpatialSizeType.MAXDISTANCE.ordinal(), Double.parseDouble(_maxSpatialRadiusTextField.getText()), false);
        parameters.SetRestrictMaxSpatialSizeForType(Parameters.SpatialSizeType.MAXDISTANCE.ordinal(), _spatialDistanceCheckBox.isEnabled() && _spatialDistanceCheckBox.isSelected(), false);
        parameters.SetRestrictReportedClusters(_restrictReportedClustersCheckBox.isEnabled() && _restrictReportedClustersCheckBox.isSelected());
        parameters.SetMaxSpatialSizeForType(Parameters.SpatialSizeType.PERCENTOFPOPULATION.ordinal(), Double.parseDouble(_maxReportedSpatialClusterSizeTextField.getText()), true);
        parameters.SetMaxSpatialSizeForType(Parameters.SpatialSizeType.PERCENTOFMAXCIRCLEFILE.ordinal(), Double.parseDouble(_maxReportedSpatialPercentFileTextField.getText()), true);
        parameters.SetRestrictMaxSpatialSizeForType(Parameters.SpatialSizeType.PERCENTOFMAXCIRCLEFILE.ordinal(), _reportedSpatialPopulationFileCheckBox.isEnabled() && _reportedSpatialPopulationFileCheckBox.isSelected(), true);
        parameters.SetMaxSpatialSizeForType(Parameters.SpatialSizeType.MAXDISTANCE.ordinal(), Double.parseDouble(_maxReportedSpatialRadiusTextField.getText()), true);
        parameters.SetRestrictMaxSpatialSizeForType(Parameters.SpatialSizeType.MAXDISTANCE.ordinal(), _reportedSpatialDistanceCheckBox.isEnabled() && _reportedSpatialDistanceCheckBox.isSelected(), true);

        parameters.SetIterativeCutOffPValue(Double.parseDouble(_iterativeScanCutoffTextField.getText()));
        parameters.SetIterativeScanning(_performIterativeScanCheckBox.isEnabled() && _performIterativeScanCheckBox.isSelected());
        parameters.SetNumIterativeScans(Integer.parseInt(_numIterativeScansTextField.getText()));
        parameters.SetUseAdjustmentForRelativeRisksFile(_adjustForKnownRelativeRisksCheckBox.isEnabled() && _adjustForKnownRelativeRisksCheckBox.isSelected());
        parameters.SetAdjustmentsByRelativeRisksFilename(_adjustmentsByRelativeRisksFileTextField.getText());
        parameters.SetTimeTrendAdjustmentType(getAdjustmentTimeTrendControlType().ordinal());
        parameters.SetTimeTrendAdjustmentPercentage(Double.parseDouble(_logLinearTextField.getText()));
        parameters.setAdjustForWeeklyTrends(_adjustDayOfWeek.isEnabled() && _adjustDayOfWeek.isSelected());
        parameters.SetSpatialAdjustmentType(getSpatialAdjustmentType().ordinal());
        parameters.SetPValueReportingType(getPValueReportingControlType().ordinal());
        parameters.SetReportGumbelPValue(_checkReportGumbel.isSelected());
        parameters.SetEarlyTermThreshold(Integer.parseInt(_earlyTerminationThreshold.getText()));
        if (_restrictTemporalRangeCheckBox.isEnabled() && _restrictTemporalRangeCheckBox.isSelected()) {
            parameters.SetIncludeClustersType(Parameters.IncludeClustersType.CLUSTERSINRANGE.ordinal());
        } else {
            parameters.SetIncludeClustersType(Parameters.IncludeClustersType.ALLCLUSTERS.ordinal());
        }
        if (parameters.GetPrecisionOfTimesType().equals(Parameters.DatePrecisionType.GENERIC)) {
            parameters.SetStartRangeStartDate(_startRangeStartGenericTextField.getText());
            parameters.SetStartRangeEndDate(_startRangeEndGenericTextField.getText());
            parameters.SetEndRangeStartDate(_endRangeStartGenericTextField.getText());
            parameters.SetEndRangeEndDate(_endRangeEndGenericTextField.getText());
        } else {
            sString = _startRangeStartYearTextField.getText() + "/" + _startRangeStartMonthTextField.getText() + "/" + _startRangeStartDayTextField.getText();
            parameters.SetStartRangeStartDate(sString);
            sString = _startRangeEndYearTextField.getText() + "/" + _startRangeEndMonthTextField.getText() + "/" + _startRangeEndDayTextField.getText();
            parameters.SetStartRangeEndDate(sString);
            sString = _endRangeStartYearTextField.getText() + "/" + _endRangeStartMonthTextField.getText() + "/" + _endRangeStartDayTextField.getText();
            parameters.SetEndRangeStartDate(sString);
            sString = _endRangeEndYearTextField.getText() + "/" + _endRangeEndMonthTextField.getText() + "/" + _endRangeEndDayTextField.getText();
            parameters.SetEndRangeEndDate(sString);
        }
        // save the input files on Input tab
        if (!_additionalDataSetsGroup.isEnabled()) {
            parameters.SetNumDataSets(1);
        } else {
            parameters.SetNumDataSets(1);
            parameters.SetNumDataSets(_dataSetsListModel.size() + 1);
            if (_dataSetsListModel.size() > 0) {
                for (int i = 0; i < _dataSetsListModel.size(); ++i) {
                    parameters.SetCaseFileName(_caseFilenames.get(i), i + 2);
                    parameters.SetControlFileName(_controlFilenames.get(i), i + 2);
                    parameters.SetPopulationFileName(_populationFilenames.get(i), i + 2);
                }
            }
        }
        parameters.SetMultipleDataSetPurposeType(_multivariateAdjustmentsRadioButton.isSelected() ? Parameters.MultipleDataSetPurposeType.MULTIVARIATE.ordinal() : Parameters.MultipleDataSetPurposeType.ADJUSTMENT.ordinal());
        parameters.SetSpatialWindowType(_circularRadioButton.isSelected() ? Parameters.SpatialWindowType.CIRCULAR.ordinal() : Parameters.SpatialWindowType.ELLIPTIC.ordinal());
        parameters.SetNonCompactnessPenalty(getNonCompactnessPenaltyType().ordinal());
        parameters.SetStudyPeriodDataCheckingType(_strictStudyPeriodCheckRadioButton.isSelected() ? Parameters.StudyPeriodDataCheckingType.STRICTBOUNDS.ordinal() : Parameters.StudyPeriodDataCheckingType.RELAXEDBOUNDS.ordinal());
        parameters.SetCoordinatesDataCheckingType(getCoordinatesDataCheckingTypeFromControl().ordinal());
        parameters.SetNumberMonteCarloReplications(Integer.parseInt(_montCarloReplicationsTextField.getText()));
        parameters.SetMaxCirclePopulationFileName(_maxCirclePopulationFilenameTextField.getText());
        parameters.setMinimumTemporalClusterSize(Integer.parseInt(_minTemporalClusterSizeUnitsTextField.getText()));
        parameters.SetMaximumTemporalClusterSize(_percentageTemporalRadioButton.isSelected() ? Double.parseDouble(_maxTemporalClusterSizeTextField.getText()) : Double.parseDouble(_maxTemporalClusterSizeUnitsTextField.getText()));
        parameters.SetMaximumTemporalClusterSizeType(_percentageTemporalRadioButton.isSelected() ? Parameters.TemporalSizeType.PERCENTAGETYPE.ordinal() : Parameters.TemporalSizeType.TIMETYPE.ordinal());
        parameters.SetIncludePurelyTemporalClusters(_inclPureTempClustCheckBox.isEnabled() && _inclPureTempClustCheckBox.isSelected());
        parameters.SetIncludePurelySpatialClusters(_includePureSpacClustCheckBox.isEnabled() && _includePureSpacClustCheckBox.isSelected());
        parameters.SetMultipleCoordinatesType(getMultipleCoordinatesType().ordinal());
        parameters.setMultipleLocationsFile(_multiple_locations_file.getText());
        parameters.SetCriteriaForReportingSecondaryClusters(getCriteriaSecondaryClustersType().ordinal());
        parameters.setGiniIndexReportType(getIndexBasedClusterReportType().ordinal());
        parameters.setReportGiniIndexCoefficents(_checkboxReportIndexCoefficients.isSelected());
        parameters.setReportHierarchicalClusters(_mostLikelyClustersHierarchically.isEnabled() && _mostLikelyClustersHierarchically.isSelected());
        parameters.setReportGiniOptimizedClusters(_giniOptimizedClusters.isEnabled() && _giniOptimizedClusters.isSelected());
        parameters.setIncludeLocationsKML(_includeClusterLocationsInKML.isSelected());
        parameters.setCompressClusterKML(_createCompressedKMZ.isSelected());
        parameters.setLaunchMapViewer(_launch_map_viewer.isSelected());

        // Miscellaneous analysis tab
        parameters.setCalculateOliveirasF(_calculate_oliveiras_f.isEnabled() && _calculate_oliveiras_f.isSelected());
        parameters.setNumRequestedOliveiraSets(Integer.parseInt(_number_oliveira_data_sets.getText()));
        parameters.setProspectiveFrequencyType(getProspectiveFrequencyControlType().ordinal());
        
        // Power Evaluations tab
        parameters.setPerformPowerEvaluation(_powerEvaluationsGroup.isEnabled() && _performPowerEvalautions.isSelected());
        parameters.setPowerEvaluationMethod(getPowerEvaluationMethodType().ordinal());
        parameters.setPowerEvaluationCaseCount(Integer.parseInt((_totalPowerCases.getText().length() > 0 ? _totalPowerCases.getText() : "600")));
        parameters.setNumPowerEvalReplicaPowerStep(Integer.parseInt(_numberPowerReplications.getText()));
        parameters.setPowerEvaluationCriticalValueType(getCriticalValuesType().ordinal());
        parameters.setPowerEstimationType(getPowerEstimationType().ordinal());
        parameters.setPowerEvaluationAltHypothesisFilename(_alternativeHypothesisFilename.getText());

        // Temporal Output tab
        parameters.setOutputTemporalGraphFile(_reportTemporalGraph.isEnabled() && _reportTemporalGraph.isSelected());
        if (_temporalGraphSignificant.isSelected()) {
            parameters.setTemporalGraphReportType(Parameters.TemporalGraphReportType.SIGNIFICANT_ONLY.ordinal());
        } else if (_temporalGraphMostLikelyX.isSelected()) {
            parameters.setTemporalGraphReportType(Parameters.TemporalGraphReportType.X_MCL_ONLY.ordinal());
        } else {
            parameters.setTemporalGraphReportType(Parameters.TemporalGraphReportType.MLC_ONLY.ordinal());            
        }
        parameters.setTemporalGraphMostLikelyCount(Integer.parseInt(_numMostLikelyClustersGraph.getText()));
        if (parameters.GetIsProspectiveAnalysis())
            parameters.setTemporalGraphSignificantCutoff(Double.valueOf(_temporalGraphPvalueCutoff.getText()).intValue());
        else
            parameters.setTemporalGraphSignificantCutoff(Double.parseDouble(_temporalGraphPvalueCutoff.getText()));
        
        // Cluster Restrictions tab
        parameters.setMinimumCasesHighRateClusters(Integer.parseInt(_minimum_number_cases_cluster.getText()));
        parameters.setRiskLimitHighClusters(_limit_high_clusters.isEnabled() && _limit_high_clusters.isSelected());
        parameters.setRiskThresholdHighClusters(Double.parseDouble(_limit_high_clusters_value.getText()));
        parameters.setRiskLimitLowClusters(_limit_low_clusters.isEnabled() && _limit_low_clusters.isSelected());
        parameters.setRiskThresholdLowClusters(Double.parseDouble(_limit_low_clusters_value.getText()));
        
        // Drilldown tab
        parameters.setPerformStandardDrilldown(_mainAnalysisDrilldown.isEnabled() && _mainAnalysisDrilldown.isSelected());
        parameters.setPerformBernoulliDrilldown(_purelySpatialDrilldown.isEnabled() && _purelySpatialDrilldown.isSelected());
        if (parameters.GetIsProspectiveAnalysis())
            parameters.setDrilldownCutoff(Double.valueOf(_drilldown_restriction_cutoff.getText()).intValue());
        else
            parameters.setDrilldownCutoff(Double.parseDouble(_drilldown_restriction_cutoff.getText()));
        parameters.setDrilldownMinimumLocationsCluster(Integer.parseInt(_drilldown_restriction_locations.getText()));
        parameters.setDrilldownMinimumCasesCluster(Integer.parseInt(_drilldown_restriction_cases.getText()));
        parameters.setDrilldownAdjustWeeklyTrends(_drilldown_restriction_dow.isEnabled() && _drilldown_restriction_dow.isSelected());
        
        // Network tab
        parameters.setUseLocationsNetworkFile(_locations_network.isEnabled() && _locations_network.isSelected());
        parameters.setLocationsNetworkFilename(_network_filename.getText());
        
        // Notifications tab
        parameters.setAlwaysEmailSummary(Utils.selected(_always_sendmail));
        parameters.setEmailAlwaysRecipients(_always_email_recipients.getText());
        parameters.setCutoffEmailSummary(Utils.selected(_cutoff_email));
        parameters.setEmailCutoffRecipients(_cutoff_email_recipients.getText());
        if (parameters.GetIsProspectiveAnalysis())
            parameters.setCutoffEmailValue(Double.valueOf(_cutoff_value_email.getText()).intValue());
        else
            parameters.setCutoffEmailValue(Double.parseDouble(_cutoff_value_email.getText()));
        parameters.setEmailAttachResults(Utils.selected(_attach_main_results_email));
        parameters.setEmailIncludeResultsDirectory(Utils.selected(_report_main_results_email));
        parameters.setEmailCustom(Utils.selected(_create_custom_email_message));
        parameters.setEmailCustomSubject(_custom_email_subject.getText());
        parameters.setEmailCustomMessageBody(replaceNewlines(_custom_email_message.getText()));

        // Other Outputs
        parameters.SetReportCriticalValues(_reportCriticalValuesCheckBox.isSelected());
        parameters.setReportClusterRank(_reportClusterRankCheckBox.isSelected());
        parameters.setPrintAsciiHeaders(_printAsciiColumnHeaders.isSelected());
        parameters.SetTitleName(_printTitle.getText());
        if (parameters.GetIsProspectiveAnalysis())
            parameters.setCutoffLineListCSV(Double.valueOf(_cluster_lineline_value.getText()).intValue());
        else
            parameters.setCutoffLineListCSV(Double.parseDouble(_cluster_lineline_value.getText()));
    }

    public boolean isNonEucledianNeighborsSelected() {
        return _specifiyNeighborsFileCheckBox.isEnabled() && _specifiyNeighborsFileCheckBox.isSelected();
    }

    public boolean isNetworkFileSelected() {
        return _locations_network.isEnabled() && _locations_network.isSelected();
    }
    
    public boolean isAdjustedRelativeRisksSelected() {
        return _adjustForKnownRelativeRisksCheckBox.isEnabled() && _adjustForKnownRelativeRisksCheckBox.isSelected();
    }

    /**
     * Returns boolean indication of whether user has selected power evaluation
     * without case file data.
     */
    public boolean isPowerEvaluationNoCaseFile() {
        return _performPowerEvalautions.isEnabled() && _performPowerEvalautions.isSelected() && _powerEvaluationWithSpecifiedCases.isSelected();
    }

    /**
     * returns maximum temporal cluster size type for control
     */
    public Parameters.TemporalSizeType getMaxTemporalClusterSizeControlType() {
        Parameters.TemporalSizeType eReturn = null;

        if (_percentageTemporalRadioButton.isSelected()) {
            eReturn = Parameters.TemporalSizeType.PERCENTAGETYPE;
        }
        if (_timeTemporalRadioButton.isSelected()) {
            eReturn = Parameters.TemporalSizeType.TIMETYPE;
        }
        return eReturn;
    }

    /**
     * returns maximum temporal cluster size from appropriate control
     */
    public double getMaxTemporalClusterSizeFromControl() {
        double dReturn;

        switch (getMaxTemporalClusterSizeControlType()) {
            case TIMETYPE:
                dReturn = Double.parseDouble(_maxTemporalClusterSizeUnitsTextField.getText());
                break;
            case PERCENTAGETYPE:
            default:
                dReturn = Double.parseDouble(_maxTemporalClusterSizeTextField.getText());
        }
        return dReturn;
    }

    /**
     *
     */
    private void validateInputFilesSettings() {
        boolean bAnalysisIsPurelyTemporal = _settings_window.getAnalysisControlType() == Parameters.AnalysisType.PURELYTEMPORAL ||
                                            _settings_window.getAnalysisControlType() == Parameters.AnalysisType.PROSPECTIVEPURELYTEMPORAL ||
                                            _settings_window.getAnalysisControlType() == Parameters.AnalysisType.SEASONALTEMPORAL;
        boolean bFirstDataSetHasPopulationFile = _settings_window.getEdtPopFileNameText().length() > 0;

        if (!_additionalDataSetsGroup.isEnabled()) {
            return;
        }

        for (int i=0; i < _caseFilenames.size(); i++) {
            //Ensure that controls have this dataset display, should we need to
            //show window regarding an error with settings.
            _inputDataSetsList.setSelectedIndex(i);
            //validate the case file for this dataset
            if (_caseFilenames.get(i).length() == 0) {
                throw new AdvFeaturesExpection("Please specify a case file for this additional data set.", FocusedTabSet.INPUT, (Component) _caseFileTextField);
            }
            if (!FileAccess.ValidateFileAccess(_caseFilenames.get(i), false, false)) {
                throw new AdvFeaturesExpection("The case file for this additional data set could not be opened for reading.\n" + "Please confirm that the path and/or file name are valid\n" + "and that you have permissions to read from this directory\nand file.",
                        FocusedTabSet.INPUT, (Component) _caseFileTextField);
            }
            if (!FileAccess.isValidFilename(_caseFilenames.get(i))) {                
                throw new AdvFeaturesExpection(String.format(AppConstants.FILENAME_ASCII_ERROR, _caseFilenames.get(i)), FocusedTabSet.INPUT, (Component) _caseFileTextField);
            }                
            String validationString = _settings_window.validateInputSourceDataFile(_caseFilenames.get(i), InputSourceSettings.InputFileType.Case.toString() + (i + 2), "case");
            if (validationString != null) throw new AdvFeaturesExpection(validationString, FocusedTabSet.INPUT, (Component) _caseFileTextField);            
            //validate the control file for this dataset - Bernoulli model only
            if (_settings_window.getModelControlType() == Parameters.ProbabilityModelType.BERNOULLI) {
                if (_controlFilenames.get(i).length() == 0) {
                    throw new AdvFeaturesExpection("For the Bernoulli model, please specify a control file for this additional data set.", FocusedTabSet.INPUT, (Component) _controlFileTextField);
                }
                if (!FileAccess.ValidateFileAccess(_controlFilenames.get(i), false, false)) {
                    throw new AdvFeaturesExpection("The control file for this additional data set could not be opened for reading.\n" + "Please confirm that the path and/or file name are valid\n" + "and that you have permissions to read from this directory\nand file.",
                            FocusedTabSet.INPUT, (Component) _controlFileTextField);
                }
                if (!FileAccess.isValidFilename(_controlFilenames.get(i))) {                
                    throw new AdvFeaturesExpection(String.format(AppConstants.FILENAME_ASCII_ERROR, _controlFilenames.get(i)), FocusedTabSet.INPUT, (Component) _controlFileTextField);
                }                
                validationString = _settings_window.validateInputSourceDataFile(_controlFilenames.get(i), InputSourceSettings.InputFileType.Control.toString() + (i + 2), "control");
                if (validationString != null) throw new AdvFeaturesExpection(validationString, FocusedTabSet.INPUT, (Component) _controlFileTextField);            
            }
            //validate the population file for this dataset-  Poisson model only
            if (_settings_window.getModelControlType() == Parameters.ProbabilityModelType.POISSON) {
                //For purely temporal analyses, the population file is optional. But if one first
                //dataset does or does not supply a population file; the other dataset must do the same.
                if (bAnalysisIsPurelyTemporal) {
                    if ((_populationFilenames.get(i).length() == 0 && bFirstDataSetHasPopulationFile)
                            || (_populationFilenames.get(i).length() > 0 && !bFirstDataSetHasPopulationFile)) {
                        throw new AdvFeaturesExpection("For the Poisson model with purely temporal analyses, the population file is optional but all data\n" + "sets must either specify a population file or omit it.",
                                FocusedTabSet.INPUT, (Component) _populationFileTextField);
                    } else if (_populationFilenames.get(i).length() > 0 && !FileAccess.ValidateFileAccess(_populationFilenames.get(i), false, false)) {
                        throw new AdvFeaturesExpection("The population file for this additional data set could not be opened for reading.\n" + "Please confirm that the path and/or file name are valid\n" + "and that you have permissions to read from this directory\nand file.",
                                FocusedTabSet.INPUT, (Component) _populationFileTextField);
                    }
                } else if (_populationFilenames.get(i).length() == 0) {
                    throw new AdvFeaturesExpection("For the Poisson model, please specify a population file for this additional data set.", FocusedTabSet.INPUT, (Component) _populationFileTextField);
                } else if (!FileAccess.ValidateFileAccess(_populationFilenames.get(i), false, false)) {
                    throw new AdvFeaturesExpection("The population file for this additional data set could not be opened for reading.\n" + "Please confirm that the path and/or file name are valid\n" + "and that you have permissions to read from this directory\nand file.",
                            FocusedTabSet.INPUT, (Component) _populationFileTextField);
                }
                if (_populationFilenames.get(i).length() > 0) {
                    if (!FileAccess.isValidFilename(_populationFilenames.get(i))) {                
                        throw new AdvFeaturesExpection(String.format(AppConstants.FILENAME_ASCII_ERROR, _populationFilenames.get(i)), FocusedTabSet.INPUT, (Component) _populationFileTextField);
                    }                                    
                    validationString = _settings_window.validateInputSourceDataFile(_populationFilenames.get(i), InputSourceSettings.InputFileType.Population.toString() + (i + 2), "population");
                    if (validationString != null) throw new AdvFeaturesExpection(validationString, FocusedTabSet.INPUT, (Component) _populationFileTextField);
                }
            }
        }  //for loop
        //validate that purpose for multiple data sets is not 'adjustment' if probability model is ordinal
        if ((_settings_window.getModelControlType() == Parameters.ProbabilityModelType.ORDINAL
                || _settings_window.getModelControlType() == Parameters.ProbabilityModelType.CATEGORICAL)
                && _adjustmentByDataSetsRadioButton.isEnabled() && _adjustmentByDataSetsRadioButton.isSelected()) {
            throw new AdvFeaturesExpection("For the ordinal and multinomial probability models with input data defined in multiple data sets,\n"
                    + "the adjustment option has not been implemented.", FocusedTabSet.INPUT, (Component) _adjustmentByDataSetsRadioButton);
        }
    }

    /**
     * validate user settings of the Spatial Neighbors tab.
     */
    private void validateSpatialNeighborsSettings() {
        if (_specialNeighborFilesGroup.isEnabled() && _specifiyNeighborsFileCheckBox.isEnabled() && _specifiyNeighborsFileCheckBox.isSelected()) {
            //validate the case file for this dataset
            if (_neighborsFileTextField.getText().length() == 0) {
                throw new AdvFeaturesExpection("Please specify a neighbors file.", FocusedTabSet.INPUT, (Component) _neighborsFileTextField);
            }
            if (!FileAccess.ValidateFileAccess(_neighborsFileTextField.getText(), false, false)) {
                throw new AdvFeaturesExpection("The neighbors file could not be opened for reading.\n" + "Please confirm that the path and/or file name are valid\n" + "and that you have permissions to read from this directory\nand file.",
                        FocusedTabSet.INPUT, (Component) _neighborsFileTextField);
            }
            if (!FileAccess.isValidFilename(_neighborsFileTextField.getText())) {                
                throw new AdvFeaturesExpection(String.format(AppConstants.FILENAME_ASCII_ERROR, _neighborsFileTextField.getText()), FocusedTabSet.INPUT, (Component) _neighborsFileTextField);
            }                
        }
        if (_specifiyNeighborsFileCheckBox.isEnabled() && _specifiyNeighborsFileCheckBox.isSelected() && _specifiyMetaLocationsFileCheckBox.isSelected()) {
            //validate the case file for this dataset
            if (_metaLocationsFileTextField.getText().length() == 0) {
                throw new AdvFeaturesExpection("Please specify a meta locations file.", FocusedTabSet.INPUT, (Component) _metaLocationsFileTextField);
            }
            if (!FileAccess.ValidateFileAccess(_metaLocationsFileTextField.getText(), false, false)) {
                throw new AdvFeaturesExpection("The meta locations file could not be opened for reading.\n" + "Please confirm that the path and/or file name are valid\n" + "and that you have permissions to read from this directory\nand file.",
                        FocusedTabSet.INPUT, (Component) _metaLocationsFileTextField);
            }
            if (!FileAccess.isValidFilename(_metaLocationsFileTextField.getText())) {                
                throw new AdvFeaturesExpection(String.format(AppConstants.FILENAME_ASCII_ERROR, _metaLocationsFileTextField.getText()), FocusedTabSet.INPUT, (Component) _metaLocationsFileTextField);
            }                
        }
        if (getMultipleCoordinatesType() != Parameters.MultipleCoordinatesType.ONEPERLOCATION && _multiple_locations_file.isEnabled()) {
            if (_multiple_locations_file.getText().length() == 0) {
                throw new AdvFeaturesExpection("Please specify a multiple locations file.", FocusedTabSet.INPUT, (Component) _multiple_locations_file);
            }
            if (!FileAccess.ValidateFileAccess(_multiple_locations_file.getText(), false, false)) {
                throw new AdvFeaturesExpection("The multiple locations file could not be opened for reading.\n" + "Please confirm that the path and/or file name are valid\n" + "and that you have permissions to read from this directory\nand file.",
                        FocusedTabSet.INPUT, (Component) _multiple_locations_file);
            }            
        }
    }

    private void validateSpatialWindowSettings() {
        if (_spatialOptionsGroup.isEnabled() && _spatialPopulationFileCheckBox.isSelected()) {
            if (_maxCirclePopulationFilenameTextField.getText().length() == 0) {
                throw new AdvFeaturesExpection("Please specify a maximum circle size file.", FocusedTabSet.ANALYSIS, (Component) _maxCirclePopulationFilenameTextField);
            }
            if (!FileAccess.ValidateFileAccess(_maxCirclePopulationFilenameTextField.getText(), false, false)) {
                throw new AdvFeaturesExpection("The maximum circle file could not be opened for reading.\n" + "Please confirm that the path and/or file name are valid\n" + "and that you have permissions to read from this directory\nand file.",
                        FocusedTabSet.ANALYSIS, (Component) _maxCirclePopulationFilenameTextField);
            }
            if (!FileAccess.isValidFilename(_maxCirclePopulationFilenameTextField.getText())) {                
                throw new AdvFeaturesExpection(String.format(AppConstants.FILENAME_ASCII_ERROR, _maxCirclePopulationFilenameTextField.getText()), FocusedTabSet.ANALYSIS, (Component) _maxCirclePopulationFilenameTextField);
            }                
        }
        if (_reportedSpatialOptionsGroup.isEnabled() && _restrictReportedClustersCheckBox.isSelected()) {
            if (_maxReportedSpatialClusterSizeTextField.isEnabled() && Double.parseDouble(_maxSpatialClusterSizeTextField.getText()) < Double.parseDouble(_maxReportedSpatialClusterSizeTextField.getText())) {
                throw new AdvFeaturesExpection("The maximum reported spatial cluster size, as percentage of population at risk,\ncan not be greater than the maximum specifed on Spatial Window tab.",
                        FocusedTabSet.OUTPUT, (Component) _maxReportedSpatialClusterSizeTextField);
            }
            if (_reportedSpatialPopulationFileCheckBox.isSelected() && !FileAccess.ValidateFileAccess(_maxCirclePopulationFilenameTextField.getText(), false, false)) {
                throw new AdvFeaturesExpection(
                        "The maximum circle file could not be opened for reading.\n" + "Please confirm that the path and/or file name are valid\n" + 
                        "and that you have permissions to read from this directory\nand file." + "A maximum circle file is required when restricting the maximum\n" + 
                        "reported spatial cluster size by a population defined through a\nmaximum circle file.",
                        FocusedTabSet.ANALYSIS, (Component) _maxCirclePopulationFilenameTextField);
            }
            if (_maxReportedSpatialPercentFileTextField.isEnabled() && Double.parseDouble(_maxSpatialPercentFileTextField.getText()) < Double.parseDouble(_maxReportedSpatialPercentFileTextField.getText())) {
                throw new AdvFeaturesExpection("The maximum reported spatial cluster size, as percentage of population in maximum circle file,\ncan not be greater than the maximum specifed on Spatial Window tab.",
                        FocusedTabSet.OUTPUT, (Component) _maxReportedSpatialPercentFileTextField);
            }
            if (_maxSpatialRadiusTextField.isEnabled() && _maxReportedSpatialRadiusTextField.isEnabled()
                    && Double.parseDouble(_maxSpatialRadiusTextField.getText()) < Double.parseDouble(_maxReportedSpatialRadiusTextField.getText())) {
                throw new AdvFeaturesExpection("The maximum reported spatial cluster size, as as fixed distance,\ncan not be greater than the maximum distance specifed on Spatial Window tab.",
                        FocusedTabSet.OUTPUT, (Component) _maxReportedSpatialRadiusTextField);
            }
        }
        // prevent using the isotonic scan statistic with multiple data sets.
        if (!_caseFilenames.isEmpty()
                && _performIsotonicScanCheckBox.isEnabled() && _performIsotonicScanCheckBox.isSelected()) {
            throw new AdvFeaturesExpection("The isotonic spatial scan statistic is not implemented with multiple data sets.", FocusedTabSet.ANALYSIS, (Component) _performIsotonicScanCheckBox);
        }
    }

    private void validateBorderAnalysisSettings() {
        Parameters.AnalysisType analysis = _settings_window.getAnalysisControlType();
        Parameters.ProbabilityModelType model = _settings_window.getModelControlType();

        if (_calculate_oliveiras_f.isEnabled() && _calculate_oliveiras_f.isSelected()) {                    
            // sanity check -- GUI should already be preventing other combinations
            if (!(analysis == Parameters.AnalysisType.PURELYSPATIAL && model == Parameters.ProbabilityModelType.POISSON))
                throw new AdvFeaturesExpection("The option to calculate Oliveira's is only implemented with purely spatial poisson.", FocusedTabSet.ANALYSIS, (Component) _calculate_oliveiras_f);
            
            int monte_carlos = Integer.parseInt(_montCarloReplicationsTextField.getText());
            int oliveira_sets = Integer.parseInt(_number_oliveira_data_sets.getText());
            if (monte_carlos < 99)
                throw new AdvFeaturesExpection("The Oliveira's F calculation requires at least 999 Monte Carlo replications.", FocusedTabSet.ANALYSIS, (Component)_montCarloReplicationsTextField);
            if (oliveira_sets < 100 || oliveira_sets % 100 > 0) {
                throw new AdvFeaturesExpection("The Oliveira's F calculation requires a minimum of 100 bootstrap replications. The number of bootstrap replications must be a multiple of 100.", FocusedTabSet.ANALYSIS, (Component)_calculate_oliveiras_f);
            }           
            /* We're disabling the gini portion for the time being: https://www.squishlist.com/ims/satscan/66323/ */
            if (_giniOptimizedClusters.isEnabled() && _giniOptimizedClusters.isSelected()) {
                throw new AdvFeaturesExpection("The Oliveira's F calculation cannot be performed in conjunction with the Gini optimized clusters collection.", FocusedTabSet.OUTPUT, (Component)_giniOptimizedClusters);
            }            
            // only permit non-overlapping clusters when reporting hierarchical clusters with border analysis option
            if (_mostLikelyClustersHierarchically.isEnabled() && _mostLikelyClustersHierarchically.isSelected() && getCriteriaSecondaryClustersType() != Parameters.CriteriaSecondaryClustersType.NOGEOOVERLAP) {
                throw new AdvFeaturesExpection("The Oliveira's F calculation can be performed in conjunction with hierarchical clusters only when secondary clusters are not overlapping.", FocusedTabSet.OUTPUT, (Component)_mostLikelyClustersHierarchically);
            }
        }
    }
    
    private void validateAdjustmentSettings() {
        boolean bAnalysisIsPurelyTemporal = _settings_window.getAnalysisControlType() == Parameters.AnalysisType.PURELYTEMPORAL ||
                                            _settings_window.getAnalysisControlType() == Parameters.AnalysisType.PROSPECTIVEPURELYTEMPORAL ||
                                            _settings_window.getAnalysisControlType() == Parameters.AnalysisType.SEASONALTEMPORAL;

        //validate spatial adjustments
        if (Utils.selected(_spatialAdjustmentsNonparametric)) {
            if (_includePureSpacClustCheckBox.isEnabled() && _includePureSpacClustCheckBox.isSelected()) {
                throw new AdvFeaturesExpection("Spatial adjustments can not performed in conjunction\n" + " with the inclusion of purely spatial clusters.", FocusedTabSet.ANALYSIS, (Component) _spatialAdjustmentsGroup);
            }
        }
        //validate temporal adjustments
        if (_temporalTrendAdjGroup.isEnabled() && _settings_window.getModelControlType() == Parameters.ProbabilityModelType.POISSON && bAnalysisIsPurelyTemporal
                && _settings_window.getEdtPopFileNameText().length() == 0 && getAdjustmentTimeTrendControlType() != Parameters.TimeTrendAdjustmentType.TEMPORAL_NOTADJUSTED) {
            throw new AdvFeaturesExpection("Temporal adjustments can not be performed for a purely temporal analysis\n" + "using the Poisson model, when no population file has been specfied.", FocusedTabSet.ANALYSIS, (Component) _temporalTrendAdjGroup);
        }
        //validate spatial/temporal/space-time adjustments
        if (_adjustForKnownRelativeRisksCheckBox.isEnabled() && _adjustForKnownRelativeRisksCheckBox.isSelected()) {
            if (_adjustmentsByRelativeRisksFileTextField.getText().length() == 0) {
                throw new AdvFeaturesExpection("Please specify an adjustments file.", FocusedTabSet.ANALYSIS, (Component) _adjustmentsByRelativeRisksFileTextField);
            }
            if (!FileAccess.ValidateFileAccess(_adjustmentsByRelativeRisksFileTextField.getText(), false, false)) {
                throw new AdvFeaturesExpection("The adjustments file could not be opened for reading.\n" + "Please confirm that the path and/or file name are valid\n" + "and that you have permissions to read from this directory\nand file.",
                        FocusedTabSet.ANALYSIS, (Component) _adjustmentsByRelativeRisksFileTextField);
            }
            if (!FileAccess.isValidFilename(_adjustmentsByRelativeRisksFileTextField.getText())) {                
                throw new AdvFeaturesExpection(String.format(AppConstants.FILENAME_ASCII_ERROR, _adjustmentsByRelativeRisksFileTextField.getText()), FocusedTabSet.ANALYSIS, (Component) _adjustmentsByRelativeRisksFileTextField);
            }                            
        }

        if (isAdjustingForDayOfWeek()) {
            double dStudyPeriodLengthInUnits = _settings_window.CalculateTimeAggregationUnitsInStudyPeriod();
            if (dStudyPeriodLengthInUnits < 14.0) {
                throw new AdvFeaturesExpection("The adjustment for day of week cannot be performed on a period less than 14 days.", FocusedTabSet.ANALYSIS, (Component) _adjustDayOfWeek);
            }
        }
    }

    /* Returns the absolute maximum temporal cluster size give the current analysis settings. */
    private double getAbsoluteMaximumTemporalSizeForSettings() {
        Parameters.AnalysisType analysisType = _settings_window.getAnalysisControlType();
        boolean ptAnalysis = (
            analysisType == Parameters.AnalysisType.PURELYTEMPORAL ||
            analysisType == Parameters.AnalysisType.PROSPECTIVEPURELYTEMPORAL ||
            analysisType == Parameters.AnalysisType.SEASONALTEMPORAL
        );        
        if (_settings_window.getModelControlType() == Parameters.ProbabilityModelType.SPACETIMEPERMUTATION || ptAnalysis || Utils.selected(_spatialAdjustmentsNonparametric))
            return 50.0;
        return 90.0;
    }
    
    /* Validates the temporal cluster size controls with consideration to other user settings. */
    private void validateTemporalClusterSize() {
        if (!_maxTemporalOptionsGroup.isEnabled()) return;
        String sErrorMessage, sPrecisionString;
        double dStudyPeriodLengthInUnits, dMaxTemporalLengthInUnits = 0, absoluteMaximum = getAbsoluteMaximumTemporalSizeForSettings();
        //check whether we are specifiying temporal information
        if (getMaxTemporalClusterSizeControlType() == Parameters.TemporalSizeType.PERCENTAGETYPE) {
            if (_maxTemporalClusterSizeTextField.getText().length() == 0 || Double.parseDouble(_maxTemporalClusterSizeTextField.getText()) == 0) {
                throw new AdvFeaturesExpection("Please specify a maximum temporal cluster size.", FocusedTabSet.ANALYSIS, (Component) _maxTemporalClusterSizeTextField);
            }
            if (Double.parseDouble(_maxTemporalClusterSizeTextField.getText()) > absoluteMaximum) {
                throw new AdvFeaturesExpection(
                    "The maximum temporal cluster size as a percent of the study period can be not greater than " + ((int)absoluteMaximum) + " percent for the current settings.",
                    FocusedTabSet.ANALYSIS, (Component) _maxTemporalClusterSizeTextField
                );
            }
            //validate that the time aggregation length agrees with the study period and maximum temporal cluster size
            dStudyPeriodLengthInUnits = _settings_window.CalculateTimeAggregationUnitsInStudyPeriod();
            dMaxTemporalLengthInUnits = Math.floor(dStudyPeriodLengthInUnits * Double.parseDouble(_maxTemporalClusterSizeTextField.getText()) / 100.0);
            if (dMaxTemporalLengthInUnits < 1) {
                sPrecisionString = _settings_window.getDatePrecisionAsString(_settings_window.getTimeAggregationControlType(), false, false);
                sErrorMessage = "A maximum temporal cluster size as " + _maxTemporalClusterSizeTextField.getText();
                sErrorMessage += " percent of a " + ((int)Math.floor(dStudyPeriodLengthInUnits)) + sPrecisionString + " study period\n";
                sErrorMessage += "results in a maximum temporal cluster size that is less than one time\n" + "aggregation " + sPrecisionString + "\n";
                throw new AdvFeaturesExpection(sErrorMessage, FocusedTabSet.ANALYSIS, (Component) _maxTemporalClusterSizeTextField);
            }
        } else if (getMaxTemporalClusterSizeControlType() == Parameters.TemporalSizeType.TIMETYPE) {
            if (_maxTemporalClusterSizeUnitsTextField.getText().length() == 0 || Integer.parseInt(_maxTemporalClusterSizeUnitsTextField.getText()) == 0) {
                throw new AdvFeaturesExpection("Please specify a maximum temporal cluster size.", FocusedTabSet.ANALYSIS, (Component) _maxTemporalClusterSizeUnitsTextField);
            }
            sPrecisionString = _settings_window.getDatePrecisionAsString(_settings_window.getTimeAggregationControlType(), false, false);
            dStudyPeriodLengthInUnits = _settings_window.CalculateTimeAggregationUnitsInStudyPeriod();
            dMaxTemporalLengthInUnits = Math.floor(dStudyPeriodLengthInUnits * absoluteMaximum / 100.0);
            if (Double.parseDouble(_maxTemporalClusterSizeUnitsTextField.getText()) > dMaxTemporalLengthInUnits) {
                sErrorMessage = "A maximum temporal cluster size of " + Integer.parseInt(_maxTemporalClusterSizeUnitsTextField.getText()) + " ";
                sErrorMessage += sPrecisionString + (Integer.parseInt(_maxTemporalClusterSizeUnitsTextField.getText()) == 1 ? "" : "s");
                sErrorMessage += " exceeds " + ((int)absoluteMaximum) + " percent of a " + ((int)Math.floor(dStudyPeriodLengthInUnits)) + " ";
                sErrorMessage += sPrecisionString + " study period.\n" + "Please set the maximum to at most ";
                sErrorMessage += ((int)Math.floor(dMaxTemporalLengthInUnits)) + " " + sPrecisionString + (dMaxTemporalLengthInUnits == 1 ? "" : "s");
                sErrorMessage += " or extend the study period.";
                throw new AdvFeaturesExpection(sErrorMessage, FocusedTabSet.ANALYSIS, (Component) _maxTemporalClusterSizeUnitsTextField);
            }
            dMaxTemporalLengthInUnits = Integer.parseInt(_maxTemporalClusterSizeUnitsTextField.getText());
        }

        // validate the minimum temporal cluster size setting
        int minTemporalClusterSize = Integer.parseInt(_minTemporalClusterSizeUnitsTextField.getText());
        if (minTemporalClusterSize < 1) {
            sPrecisionString = _settings_window.getDatePrecisionAsString(_settings_window.getTimeAggregationControlType(), false, false);
            sErrorMessage = "The minimum temporal cluster size is 1 " + sPrecisionString + " when time aggregating to " + sPrecisionString + "s.";
            throw new AdvFeaturesExpection(sErrorMessage, FocusedTabSet.ANALYSIS, (Component) _minTemporalClusterSizeUnitsTextField);
        }
        // compare the maximum temporal cluster size to the minimum temporal cluster size
        if (minTemporalClusterSize > dMaxTemporalLengthInUnits) {
            sPrecisionString = _settings_window.getDatePrecisionAsString(_settings_window.getTimeAggregationControlType(), dMaxTemporalLengthInUnits > 1, false);
            sErrorMessage = "The minimum temporal cluster size is greater than the maximum temporal cluster size of " + ((int)Math.floor(dMaxTemporalLengthInUnits)) + " " + sPrecisionString + ".";
            throw new AdvFeaturesExpection(sErrorMessage, FocusedTabSet.ANALYSIS, (Component) _minTemporalClusterSizeUnitsTextField);
        }
    }

    /**
     * validates scanning window range settings - throws exception
     */
    private void validateScanningWindowRanges() {
        if (_restrictTemporalRangeCheckBox.isEnabled() && _restrictTemporalRangeCheckBox.isSelected()) {
            GregorianCalendar StudyPeriodStartDate = _settings_window.getStudyPeriodStartDateAsCalender();
            GregorianCalendar StudyPeriodEndDate = _settings_window.getStudyPeriodEndDateAsCalender();
            GregorianCalendar StartRangeStartDate = ParameterSettingsFrame.getStudyPeriodEndDateAsCalender(_settings_window.getPrecisionOfTimesControlType(),
                    _startRangeStartYearTextField.getText(),
                    _startRangeStartMonthTextField.getText(),
                    _startRangeStartDayTextField.getText(),
                    _startRangeStartGenericTextField.getText());

            GregorianCalendar StartRangeEndDate = ParameterSettingsFrame.getStudyPeriodEndDateAsCalender(_settings_window.getPrecisionOfTimesControlType(),
                    _startRangeEndYearTextField.getText(),
                    _startRangeEndMonthTextField.getText(),
                    _startRangeEndDayTextField.getText(),
                    _startRangeEndGenericTextField.getText());

            GregorianCalendar EndRangeStartDate = ParameterSettingsFrame.getStudyPeriodEndDateAsCalender(_settings_window.getPrecisionOfTimesControlType(),
                    _endRangeStartYearTextField.getText(),
                    _endRangeStartMonthTextField.getText(),
                    _endRangeStartDayTextField.getText(),
                    _endRangeStartGenericTextField.getText());

            GregorianCalendar EndRangeEndDate = ParameterSettingsFrame.getStudyPeriodEndDateAsCalender(_settings_window.getPrecisionOfTimesControlType(),
                    _endRangeEndYearTextField.getText(),
                    _endRangeEndMonthTextField.getText(),
                    _endRangeEndDayTextField.getText(),
                    _endRangeEndGenericTextField.getText());

            //check that scanning ranges are within study period
            if (StartRangeStartDate.before(StudyPeriodStartDate) || StartRangeStartDate.after(StudyPeriodEndDate)) {
                throw new AdvFeaturesExpection("The scanning window start range does not occur within study period.",
                        FocusedTabSet.ANALYSIS, (Component) _startRangeStartYearTextField);
            }
            if (StartRangeEndDate.before(StudyPeriodStartDate) || StartRangeEndDate.after(StudyPeriodEndDate)) {
                throw new AdvFeaturesExpection("The scanning window start range does not occur within study period.",
                        FocusedTabSet.ANALYSIS, (Component) _startRangeEndYearTextField);
            }
            if (StartRangeStartDate.after(StartRangeEndDate)) {
                throw new AdvFeaturesExpection("The scanning window start range dates conflict.",
                        FocusedTabSet.ANALYSIS, (Component) _startRangeStartYearTextField);
            }
            if (EndRangeStartDate.before(StudyPeriodStartDate) || EndRangeStartDate.after(StudyPeriodEndDate)) {
                throw new AdvFeaturesExpection("The scanning window end range does not occur within study period.",
                        FocusedTabSet.ANALYSIS, (Component) _endRangeStartYearTextField);
            }
            if (EndRangeEndDate.before(StudyPeriodStartDate) || EndRangeEndDate.after(StudyPeriodEndDate)) {
                throw new AdvFeaturesExpection("The scanning window end range does not occur within study period.",
                        FocusedTabSet.ANALYSIS, (Component) _endRangeEndYearTextField);
            }
            if (EndRangeStartDate.after(EndRangeEndDate)) {
                throw new AdvFeaturesExpection("The scanning window end range dates conflict.",
                        FocusedTabSet.ANALYSIS, (Component) _endRangeStartYearTextField);
            }
            if (StartRangeStartDate.compareTo(EndRangeEndDate) >= 0) {
                throw new AdvFeaturesExpection("The scanning window start range does not occur before end range.",
                        FocusedTabSet.ANALYSIS, (Component) _startRangeStartYearTextField);
            }
        }
    }

    /**
     * validates temporal window settings - throws exception
     */
    private void validateTemporalWindowSettings() {
        validateTemporalClusterSize();
        validateScanningWindowRanges();
    }

    /**
     * Validates the power evaluations parameters in conjunction with other
     * selected parameters.
     */
    private void validatePowerEvaluationsSettings() {
        if (_performPowerEvalautions.isEnabled() && _performPowerEvalautions.isSelected()) {
            if (_performIterativeScanCheckBox.isEnabled() && _performIterativeScanCheckBox.isSelected()) {
                throw new AdvFeaturesExpection("The power evaluation can not be performed with the iterative scan statistic.\n", FocusedTabSet.ANALYSIS, (Component) _performPowerEvalautions);
            }
            if (_performIsotonicScanCheckBox.isEnabled() && _performIsotonicScanCheckBox.isSelected()) {
                throw new AdvFeaturesExpection("A power evaluation cannot be performed with the isotonic scan.\n", FocusedTabSet.ANALYSIS, (Component) _performPowerEvalautions);
            }
            Parameters.PValueReportingType pvalues = getPValueReportingControlType();
            if (pvalues == Parameters.PValueReportingType.TERMINATION_PVALUE) {
                throw new AdvFeaturesExpection("The power evaluation is not available for the Sequential Standard Monte Carlo p-value reporting.\n", FocusedTabSet.ANALYSIS, (Component) _performPowerEvalautions);
            }
            if (_powerEvaluationWithSpecifiedCases.isSelected()) {
                if (_temporalTrendAdjGroup.isEnabled() && getAdjustmentTimeTrendControlType() != Parameters.TimeTrendAdjustmentType.TEMPORAL_NOTADJUSTED) {
                    throw new AdvFeaturesExpection("A power evaluation cannot be performed when using temporal adjustments without case data.\n", FocusedTabSet.ANALYSIS, (Component) _performPowerEvalautions);
                }
                if (_spatialAdjustmentsGroup.isEnabled() && getSpatialAdjustmentType() != Parameters.SpatialAdjustmentType.SPATIAL_NOTADJUSTED) {
                    throw new AdvFeaturesExpection("A power evaluation cannot be performed when using spatial adjustments without case data.\n", FocusedTabSet.ANALYSIS, (Component) _performPowerEvalautions);
                }
                if (_knownAdjustmentsGroup.isEnabled() && _adjustForKnownRelativeRisksCheckBox.isSelected()) {
                    throw new AdvFeaturesExpection("A power evaluation cannot be performed when using adjustments for known relative\nrisks without case data.\n", FocusedTabSet.ANALYSIS, (Component) _performPowerEvalautions);
                }
                if (Integer.parseInt(_totalPowerCases.getText()) == 0) {
                    throw new AdvFeaturesExpection("The number of power evaluation cases must be greater than zero.\n", FocusedTabSet.ANALYSIS, (Component) _totalPowerCases);
                }
                if (_adjustDayOfWeek.isEnabled() && _adjustDayOfWeek.isSelected()) {
                    throw new AdvFeaturesExpection("A power evaluation cannot be performed when using adjustment for day of week\nunless case data is provided.\n", FocusedTabSet.ANALYSIS, (Component) _performPowerEvalautions);
                }
            }
            int replica = Integer.parseInt(_montCarloReplicationsTextField.getText());
            int replicaPE = Integer.parseInt(_numberPowerReplications.getText());
            if (replica < 999) {
                throw new AdvFeaturesExpection("The minimum number of standard replications in the power evaluation is 999.\n", FocusedTabSet.ANALYSIS, (Component) _montCarloReplicationsTextField);
            }
            if (replicaPE < 100) {
                throw new AdvFeaturesExpection("The minimum number of power replications in the power evaluation is 100.\n", FocusedTabSet.ANALYSIS, (Component) _numberPowerReplications);
            }
            if (replicaPE % 100 != 0) {
                throw new AdvFeaturesExpection("The number of power replications in the power evaluation must be a multiple of 100.\n", FocusedTabSet.ANALYSIS, (Component) _numberPowerReplications);
            }
            if (replicaPE > replica + 1) {
                throw new AdvFeaturesExpection("The number of standard replications must be at most one less than the number of power replications.\n", FocusedTabSet.ANALYSIS, (Component) _montCarloReplicationsTextField);
            }
            if (_alternativeHypothesisFilename.getText().length() == 0) {
                throw new AdvFeaturesExpection("Please specify an alternative hypothesis  filename.", FocusedTabSet.ANALYSIS, (Component) _alternativeHypothesisFilename);
            }
            if (!FileAccess.ValidateFileAccess(_alternativeHypothesisFilename.getText(), false, false)) {
                throw new AdvFeaturesExpection("The alternative hypothesis file could not be opened for reading.\n" + "Please confirm that the path and/or file name are valid\n" + "and that you have permissions to read from this directory\nand file.",
                        FocusedTabSet.ANALYSIS, (Component) _alternativeHypothesisFilename);
            }
            if (!FileAccess.isValidFilename(_alternativeHypothesisFilename.getText())) {                
                throw new AdvFeaturesExpection(String.format(AppConstants.FILENAME_ASCII_ERROR, _alternativeHypothesisFilename.getText()), FocusedTabSet.ANALYSIS, (Component) _alternativeHypothesisFilename);
            }                                        
        }
    }

    /**
     * validates Monte Carlo replications
     */
    private void validateInferenceSettings() {
        //double        dNumReplications/*, dMaxReplications*/;
        int dNumReplications;

        if (_montCarloReplicationsTextField.getText().length() == 0) {
            throw new AdvFeaturesExpection("Please specify a number of Monte Carlo replications.\nChoices are: 0, 9, 999, or value ending in 999.",
                    FocusedTabSet.ANALYSIS, (Component) _montCarloReplicationsTextField);
        }
        dNumReplications = Integer.parseInt(_montCarloReplicationsTextField.getText());
        if (!((dNumReplications == 0 || dNumReplications == 9 || dNumReplications == 19 || (dNumReplications + 1) % 1000 == 0))) {
            throw new AdvFeaturesExpection("Invalid number of Monte Carlo replications.\nChoices are: 0, 9, 999, or value ending in 999.",
                    FocusedTabSet.ANALYSIS, (Component) _montCarloReplicationsTextField);
        }

        int earlyTerminatationThreshold = Integer.parseInt(_earlyTerminationThreshold.getText());
        if (dNumReplications > 0 && getPValueReportingControlType() == Parameters.PValueReportingType.TERMINATION_PVALUE
                && (earlyTerminatationThreshold < 1 || earlyTerminatationThreshold > dNumReplications)) {
            throw new AdvFeaturesExpection("Invalid early termination cutoff.\nThe cutoff may be from 1 to number of specified replications (" + dNumReplications + ").",
                    FocusedTabSet.ANALYSIS, (Component) _earlyTerminationThreshold);
        }
    }

    /** validates parameter settings for the cluster drilldown tab */
    private void validateDrilldownSettings() {
        if ((_purelySpatialDrilldown.isEnabled() && _purelySpatialDrilldown.isSelected()) ||
             (_purelySpatialDrilldown.isEnabled() && _purelySpatialDrilldown.isSelected())) {
            double cutoff = Double.parseDouble(_drilldown_restriction_cutoff.getText());
            if (_settings_window.isProspectiveScan() && cutoff < 1.0)
                throw new AdvFeaturesExpection(
                    "The recurrence interval cutoff for a detected cluster on drilldown must\nbe greater than or equal to one for a prospective scan.", 
                    FocusedTabSet.ANALYSIS, (Component) _drilldown_restriction_cutoff
                );
            if (!_settings_window.isProspectiveScan() && (cutoff < 0.0 || cutoff > 1.0))
                throw new AdvFeaturesExpection(
                    "The p-value cutoff for a detected cluster on drilldown must\nbe between 0 and 1 (inclusive) for a retrospective scan.", 
                    FocusedTabSet.ANALYSIS, (Component) _drilldown_restriction_cutoff
                );
            if (Integer.parseInt(_drilldown_restriction_locations.getText()) < 2)
                throw new AdvFeaturesExpection(
                    "The minimum number of locations in detected cluster for drilldown cannot be less than 2.", 
                    FocusedTabSet.ANALYSIS, (Component) _drilldown_restriction_locations
                );                
            if (Integer.parseInt(_drilldown_restriction_cases.getText()) < 10)
                throw new AdvFeaturesExpection(
                    "The minimum number of cases in detected cluster for drilldown cannot be less than 10.", 
                    FocusedTabSet.ANALYSIS, (Component) _drilldown_restriction_cases
                );                
        }
        
        if (_purelySpatialDrilldown.isEnabled() && _purelySpatialDrilldown.isSelected() &&
            _drilldown_restriction_dow.isEnabled() && _drilldown_restriction_dow.isSelected()) {
            if (isAdjustingForDayOfWeek()) {
                int time_length = Integer.parseInt(_settings_window._timeAggregationLengthTextField.getText());            
                if (!(time_length == 1 && _settings_window.getTimeAggregationControlType() == Parameters.DatePrecisionType.DAY))
                    throw new AdvFeaturesExpection(
                            "The adjustment for weekly trends, in the purely spatial Beroulli drilldown, can only be performed with a time aggregation length of 1 day.", 
                            FocusedTabSet.ANALYSIS, (Component) _drilldown_restriction_dow
                    );
            }        
        }
    }    

    /** Validates parameter settings for the Temporal Output tab */
    private void validateTemporalOutputSettings() {
        if (Utils.selected(_temporalGraphSignificant)) {
            double cutoff = Double.parseDouble(_temporalGraphPvalueCutoff.getText());
            if (_settings_window.isProspectiveScan() && cutoff < 1.0)
                throw new AdvFeaturesExpection(
                    "The recurrence interval cutoff for including clusters in temporal graph must\nbe greater than or equal to one for a prospective scan.", 
                    FocusedTabSet.OUTPUT, (Component) _temporalGraphPvalueCutoff
                );
            if (!_settings_window.isProspectiveScan() && (cutoff < 0.0 || cutoff > 1.0))
                throw new AdvFeaturesExpection(
                    "The p-value cutoff for including clusters in temporal graph must\nbe between 0 and 1 (inclusive) for a retrospective scan.", 
                    FocusedTabSet.OUTPUT, (Component) _temporalGraphPvalueCutoff
                );
        }
    }    
    
    /** Validates parameter settings for the Other Output tab */
    private void validateOtherOutputSettings() {
        if (_cluster_lineline_value.isEnabled()) {
            double cutoff = Double.parseDouble(_cluster_lineline_value.getText());
            if (_settings_window.isProspectiveScan() && cutoff < 1.0)
                throw new AdvFeaturesExpection(
                    "The recurrence interval cutoff for including clusters in line list CSV must be greater than or equal to one for a prospective analysis.", 
                    FocusedTabSet.OUTPUT, (Component) _cluster_lineline_value
                );
            if (!_settings_window.isProspectiveScan() && (cutoff < 0.0 || cutoff > 1.0))
                throw new AdvFeaturesExpection(
                    "The p-value cutoff for including clusters in line list CSV must be between 0 and 1 (inclusive) for a retrospective analysis.", 
                    FocusedTabSet.OUTPUT, (Component) _cluster_lineline_value
                );
        }
    }    
    
    /** validates all the settings in this dialog */
    public void validateParameters() {
        validateInputFilesSettings();
        validateSpatialNeighborsSettings();
        validateSpatialWindowSettings();
        validateAdjustmentSettings();
        validateTemporalWindowSettings();
        validateInferenceSettings();
        validateBorderAnalysisSettings();
        validatePowerEvaluationsSettings();
        validateDrilldownSettings();
        validateTemporalOutputSettings();
        validateOtherOutputSettings();
        validateEmailSettings();
    }

    /* Validates the email settings. */
    private void validateEmailSettings() throws AdvFeaturesExpection {
        if (Utils.selected(_always_sendmail)) {
            try {
                if (!ApplicationPreferences.minimumMailServerDefined()) {
                    throw new AdvFeaturesExpection(
                        """
                        In order to email a results summary, you must define mail server settings.
                        Please see 'Mail Server Settings' in the 'Preferences and Settings' dialog.""",
                        FocusedTabSet.OUTPUT, (Component) _always_sendmail
                    );
                }
                if (Utils.selected(_always_sendmail) && _always_email_recipients.getText().isEmpty()) {
                    throw new AdvFeaturesExpection(
                        "At least one email address is required to receive emails (ex. someone@company.com).",
                        FocusedTabSet.OUTPUT, (Component) _always_email_recipients
                    );
                }
                if (Utils.selected(_cutoff_email) && _cutoff_email_recipients.getText().isEmpty()) {
                    throw new AdvFeaturesExpection(
                        "At least one email address is required to receive emails (ex. someone@company.com).",
                        FocusedTabSet.OUTPUT, (Component) _cutoff_email_recipients
                    );
                }
                if (Utils.selected(_create_custom_email_message)) {
                    if (_custom_email_subject.getText().isEmpty()) {
                        throw new AdvFeaturesExpection(
                            "The email notifications feature requires text for email subject line.",
                            FocusedTabSet.OUTPUT, (Component) _custom_email_subject
                        );
                    }
                    if (_custom_email_message.getText().isEmpty()) {
                        throw new AdvFeaturesExpection(
                            "The email notifications feature requires text for email message.",
                            FocusedTabSet.OUTPUT, (Component) _custom_email_message
                        );
                    }
                }
            } catch (IOException ex) {
                Logger.getLogger(AdvancedParameterSettingsFrame.class.getName()).log(Level.SEVERE, null, ex);
            }
        }
        if (_cutoff_value_email.isEnabled()) {
            double cutoff = Double.parseDouble(_cutoff_value_email.getText());
            if (_settings_window.isProspectiveScan() && cutoff < 1.0)
                throw new AdvFeaturesExpection(
                    "The recurrence interval cutoff for emailing the cluster summary must\nbe greater than or equal to one for a prospective scan.", 
                    FocusedTabSet.OUTPUT, (Component) _cutoff_value_email
                );
            if (!_settings_window.isProspectiveScan() && (cutoff < 0.0 || cutoff > 1.0))
                throw new AdvFeaturesExpection(
                    "The p-value cutoff for emailing the cluster summary must\nbe between 0 and 1 (inclusive) for a retrospective scan.", 
                    FocusedTabSet.OUTPUT, (Component) _cutoff_value_email
                );
        }        
    }    
    
    /**
     * enables input tab case/control/pop files edit boxes
     */
    private void enableInputFileEdits() {
        boolean bEnable = _additionalDataSetsGroup.isEnabled() && _inputDataSetsList.getModel().getSize() > 0;

        _caseFileTextField.setEnabled(bEnable);
        _controlFileTextField.setEnabled(bEnable);
        _populationFileTextField.setEnabled(bEnable);
        _caseFileBrowseButton.setEnabled(bEnable);
        _controlFileBrowseButton.setEnabled(bEnable);
        _populationFileBrowseButton.setEnabled(bEnable);
        _caseFileLabel.setEnabled(bEnable);
        _controlFileLabel.setEnabled(bEnable);
        _populationFileLabel.setEnabled(bEnable);
    }

    /**
     * enables 'Set Defaults' button
     */
    private void enableSetDefaultsButton() {
        // update enable/disable of Set Defaults button
        switch (_focusedTabSet) {
            case INPUT:
                _setDefaultButton.setEnabled(!getDefaultsSetForInputOptions());
                break;
            case ANALYSIS:
                _setDefaultButton.setEnabled(!getDefaultsSetForAnalysisOptions());
                break;
            case OUTPUT:
                _setDefaultButton.setEnabled(!getDefaultsSetForOutputOptions());
                break;
        }
    }

    /**
     * Enables/disables controls that indicate purpose of additional data sets.
     */
    private void enableDataSetPurposeControls() {
        boolean bEnable = _additionalDataSetsGroup.isEnabled() && _dataSetsListModel.getSize() > 0;
        _multivariateAdjustmentsRadioButton.setEnabled(bEnable);
        _adjustmentByDataSetsRadioButton.setEnabled(bEnable);
        _multipleDataSetPurposeLabel.setEnabled(bEnable);
    }

    /**
     * Sets default values for Input related tab and respective controls
     */
    private void setDefaultsForInputTab() {
        // clear all visual components
        _caseFileTextField.setText("");
        _controlFileTextField.setText("");
        _populationFileTextField.setText("");
        _dataSetsListModel.removeAllElements();
        enableDataSetList();
        enableDataSetPurposeControls();

        // clear the non-visual components
        _caseFilenames.clear();
        _controlFilenames.clear();
        _populationFilenames.clear();
        enableNewButton();
        enableRemoveButton();
        enableInputFileEdits();
        _multivariateAdjustmentsRadioButton.setSelected(true);

        //data checking
        _strictStudyPeriodCheckRadioButton.setSelected(true);
        _strictCoordinatesRadioButton.setSelected(true);

        //neighbors file
        _specifiyNeighborsFileCheckBox.setSelected(false);
        _neighborsFileTextField.setText("");
        _specifiyMetaLocationsFileCheckBox.setSelected(false);
        _metaLocationsFileTextField.setText("");
        _multiple_locations_file.setText("");
        
        // network file
        _locations_network.setSelected(false);
        _network_filename.setText("");
    }

    /**
     * Sets maximum temporal cluster size control for passed type
     */
    private void setMaxTemporalClusterSizeTypeControl(Parameters.TemporalSizeType eTemporalSizeType) {
        switch (eTemporalSizeType) {
            case TIMETYPE:
                _timeTemporalRadioButton.setSelected(true);
                break;
            case PERCENTAGETYPE:
            default:
                _percentageTemporalRadioButton.setSelected(true);
        }
    }

    /**
     * Sets default values for Analysis related tabs and their respective
     * controls pulled these default values from the CParameter class
     */
    private void setDefaultsForAnalysisTabs() {
        // Inference tab
        _radioDefaultPValues.setSelected(true);
        _earlyTerminationThreshold.setText("50");
        _checkReportGumbel.setSelected(false);
        _reportCriticalValuesCheckBox.setSelected(false);
        _performIterativeScanCheckBox.setSelected(false);
        _numIterativeScansTextField.setText("10");
        _iterativeScanCutoffTextField.setText("0.05");
        _montCarloReplicationsTextField.setText("999");

        // Spatial Window tab
        _maxSpatialClusterSizeTextField.setText("50");
        _maxSpatialPercentFileTextField.setText("50");
        _maxSpatialRadiusTextField.setText("1");
        _spatialPopulationFileCheckBox.setSelected(false);
        _spatialDistanceCheckBox.setSelected(false);
        _maxCirclePopulationFilenameTextField.setText("");
        _inclPureTempClustCheckBox.setSelected(false);
        _circularRadioButton.setSelected(true);
        _nonCompactnessPenaltyComboBox.select(1);
        setSpatialDistanceCaption();

        _onePerLocationIdRadioButton.setSelected(true);

        // Temporal tab
        _minTemporalClusterSizeUnitsTextField.setText("1");
        setMaxTemporalClusterSizeTypeControl(Parameters.TemporalSizeType.PERCENTAGETYPE);
        _maxTemporalClusterSizeTextField.setText("50");
        _maxTemporalClusterSizeUnitsTextField.setText("1");
        _includePureSpacClustCheckBox.setSelected(false);
        Utils.parseDateStringToControls("2000/01/01", _startRangeStartYearTextField, _startRangeStartMonthTextField, _startRangeStartDayTextField, true);
        Utils.parseDateStringToControls("2000/12/31", _startRangeEndYearTextField, _startRangeEndMonthTextField, _startRangeEndDayTextField, false);
        Utils.parseDateStringToControls("2000/01/01", _endRangeStartYearTextField, _endRangeStartMonthTextField, _endRangeStartDayTextField, true);
        Utils.parseDateStringToControls("2000/12/31", _endRangeEndYearTextField, _endRangeEndMonthTextField, _endRangeEndDayTextField, false);
        _restrictTemporalRangeCheckBox.setSelected(false);

        // Risk tab
        _adjustForKnownRelativeRisksCheckBox.setSelected(false);
        _adjustmentsByRelativeRisksFileTextField.setText("");
        setTemporalTrendAdjustmentControl(Parameters.TimeTrendAdjustmentType.TEMPORAL_NOTADJUSTED);
        _spatialAdjustmentsNone.setSelected(true);
        _logLinearTextField.setText("0");
        _adjustDayOfWeek.setSelected(false);

        // Power Evaluations tab
        _performPowerEvalautions.setSelected(false);
        _partOfRegularAnalysis.setSelected(true);
        _totalPowerCases.setText("600");
        _criticalValuesMonteCarlo.setSelected(true);
        _powerEstimationMonteCarlo.setSelected(true);
        _alternativeHypothesisFilename.setText("");
        _numberPowerReplications.setText("1000");
        
        // Cluster Restrictions
        _minimum_number_cases_cluster.setText("2");
        _limit_high_clusters.setSelected(false);
        _limit_high_clusters_value.setText("1.0");
        _limit_low_clusters.setSelected(false);
        _limit_low_clusters_value.setText("1.0");

        // Drilldown tab
        _mainAnalysisDrilldown.setSelected(false);
        _purelySpatialDrilldown.setSelected(false);
        _drilldown_restriction_cutoff.setText("0.05");
        _drilldown_restriction_locations.setText("2");
        _drilldown_restriction_cases.setText("10");
        _drilldown_restriction_dow.setSelected(false);
        
        // Miscellaneous
        _calculate_oliveiras_f.setSelected(false);
        _number_oliveira_data_sets.setText("1000");
        _prospective_frequency.select(0);      
    }

    /**
     * Sets default values for Output related tab and respective controls pulled
     * these default values from the CParameter class
     */
    private void setDefaultsForOutputTab() {
        _mostLikelyClustersHierarchically.setSelected(true);
        _giniOptimizedClusters.setSelected(true);
        _hierarchicalSecondaryClusters.select(0);
        _checkboxReportIndexCoefficients.setSelected(false);
        _restrictReportedClustersCheckBox.setSelected(false);
        _reportedSpatialPopulationFileCheckBox.setSelected(false);
        _reportedSpatialDistanceCheckBox.setSelected(false);
        _maxReportedSpatialClusterSizeTextField.setText("50");
        _maxReportedSpatialPercentFileTextField.setText("50");
        _maxReportedSpatialRadiusTextField.setText("1");
        _reportClusterRankCheckBox.setSelected(false);
        _printAsciiColumnHeaders.setSelected(false);
        _printTitle.setText("");
        _reportTemporalGraph.setSelected(false);
        _temporalGraphMostLikely.setSelected(true);
        _numMostLikelyClustersGraph.setText("1");
        _temporalGraphPvalueCutoff.setText("0.05");
        _includeClusterLocationsInKML.setSelected(true);
        _createCompressedKMZ.setSelected(false);
        _launch_map_viewer.setSelected(true);
        _calculate_oliveiras_f.setSelected(false);
        _number_oliveira_data_sets.setText("1000");        
        _always_sendmail.setSelected(false);
        _cluster_lineline_value.setText("0.05");
        _always_sendmail.setSelected(false);
        _always_email_recipients.setText("");
        _cutoff_email.setSelected(false);
        _cutoff_value_email.setText("0.05");
        _cutoff_email_recipients.setText("");
        _attach_main_results_email.setSelected(false);
        _report_main_results_email.setSelected(false);
        _create_custom_email_message.setSelected(false);
        _custom_email_subject.setText("");
        _custom_email_message.setText("");
    }

    /**
     * Resets advanced settings to default values
     */
    private void setDefaultsClick() {
        switch (_focusedTabSet) {
            case INPUT:
                setDefaultsForInputTab();
                break;
            case ANALYSIS:
                setDefaultsForAnalysisTabs();
                break;
            case OUTPUT:
                setDefaultsForOutputTab();
                break;
        }
        enableSetDefaultsButton();
    }

    /**
     * enables or disables the spatial options group control
     */
    private void enableSpatialOptionsGroup(boolean bEnable, boolean bEnableIncludePurelyTemporal) {
        _spatialOptionsGroup.setEnabled(bEnable);
        boolean bEnablePopPercentage = true;
        _maxSpatialClusterSizeTextField.setEnabled(bEnable && bEnablePopPercentage);
        _percentageOfPopulationLabel.setEnabled(bEnable && bEnablePopPercentage);

        boolean bEnablePopulationFile = bEnable && _settings_window.getModelControlType() != Parameters.ProbabilityModelType.HOMOGENEOUSPOISSON;
        _spatialPopulationFileCheckBox.setEnabled(bEnablePopulationFile);
        _maxSpatialPercentFileTextField.setEnabled(bEnablePopulationFile && _spatialPopulationFileCheckBox.isSelected());
        _percentageOfPopFileLabel.setEnabled(bEnablePopulationFile);
        _maxCirclePopulationFilenameTextField.setEnabled(bEnablePopulationFile);
        _maxCirclePopFileBrowseButton.setEnabled(bEnablePopulationFile);

        _spatialDistanceCheckBox.setEnabled(bEnable && !_specifiyNeighborsFileCheckBox.isSelected());
        _maxSpatialRadiusTextField.setEnabled(Utils.selected(_spatialDistanceCheckBox));
        _distancePrefixLabel.setEnabled(Utils.selected(_spatialDistanceCheckBox));
        _maxRadiusLabel.setEnabled(Utils.selected(_spatialDistanceCheckBox));

        _inclPureTempClustCheckBox.setEnabled(bEnable && bEnableIncludePurelyTemporal);
        enableReportedSpatialOptionsGroup(bEnable);
    }

    /**
     * enables or disables the window shape options group control
     */
    private void enableWindowShapeGroup(boolean bEnable) {
        bEnable &= !(_specifiyNeighborsFileCheckBox.isEnabled() && _specifiyNeighborsFileCheckBox.isSelected());
        bEnable &= !(_locations_network.isEnabled() && _locations_network.isSelected());
        _spatialWindowShapeGroup.setEnabled(bEnable);
        _circularRadioButton.setEnabled(bEnable);
        _ellipticRadioButton.setEnabled(bEnable
                && _settings_window.getCoordinatesType() == Parameters.CoordinatesType.CARTESIAN
                && _settings_window.getModelControlType() != Parameters.ProbabilityModelType.HOMOGENEOUSPOISSON
                && (_performIsotonicScanCheckBox.isEnabled() ? !_performIsotonicScanCheckBox.isSelected() : true));
        _nonCompactnessPenaltyLabel.setEnabled(_ellipticRadioButton.isEnabled() && _ellipticRadioButton.isSelected());
        _nonCompactnessPenaltyComboBox.setEnabled(_ellipticRadioButton.isEnabled() && _ellipticRadioButton.isSelected());
        if (!_ellipticRadioButton.isEnabled() && _ellipticRadioButton.isSelected() && _circularRadioButton.isEnabled()) {
            _circularRadioButton.setSelected(true);
        }
        setSpatialDistanceCaption();
    }

    /**
     * enables or disables the spatial options group control
     */
    private void enableReportedSpatialOptionsGroup(boolean bEnable) {
        _reportedSpatialOptionsGroup.setEnabled(bEnable);
        _restrictReportedClustersCheckBox.setEnabled(bEnable);

        boolean bEnablePopPercentage = true;
        _maxReportedSpatialClusterSizeTextField.setEnabled(Utils.selected(_restrictReportedClustersCheckBox) && bEnablePopPercentage);
        _reportedPercentOfPopulationLabel.setEnabled(Utils.selected(_restrictReportedClustersCheckBox) && bEnablePopPercentage);
        boolean bEnablePopulationFile = bEnable && _settings_window.getModelControlType() != Parameters.ProbabilityModelType.HOMOGENEOUSPOISSON;
        _reportedSpatialPopulationFileCheckBox.setEnabled(bEnablePopulationFile && Utils.selected(_restrictReportedClustersCheckBox));
        _maxReportedSpatialPercentFileTextField.setEnabled(Utils.selected(_reportedSpatialPopulationFileCheckBox) && Utils.selected(_restrictReportedClustersCheckBox));
        _reportedPercentageOfPopFileLabel.setEnabled(Utils.selected(_reportedSpatialPopulationFileCheckBox) && Utils.selected(_restrictReportedClustersCheckBox));

        _reportedSpatialDistanceCheckBox.setEnabled(bEnable && !Utils.selected(_specifiyNeighborsFileCheckBox) && Utils.selected(_restrictReportedClustersCheckBox));
        _reportedMaxDistanceLabel.setEnabled(Utils.selected(_reportedSpatialDistanceCheckBox) && Utils.selected(_restrictReportedClustersCheckBox));
        _maxReportedSpatialRadiusTextField.setEnabled(Utils.selected(_reportedSpatialDistanceCheckBox) && Utils.selected(_restrictReportedClustersCheckBox) && Utils.selected(_reportedSpatialDistanceCheckBox));
        _maxReportedRadiusLabel.setEnabled(Utils.selected(_reportedSpatialDistanceCheckBox) && Utils.selected(_restrictReportedClustersCheckBox));
    }

    /**
     * returns p-value reporting option type control index
     */
    private Parameters.PValueReportingType getPValueReportingControlType() {
        Parameters.PValueReportingType eReturn = null;

        if (_radioDefaultPValues.isSelected()) {
            eReturn = Parameters.PValueReportingType.DEFAULT_PVALUE;
        } else if (_radioEarlyTerminationPValues.isSelected()) {
            eReturn = Parameters.PValueReportingType.TERMINATION_PVALUE;
        } else if (_radioStandardPValues.isSelected()) {
            eReturn = Parameters.PValueReportingType.STANDARD_PVALUE;
        } else if (_radioGumbelPValues.isSelected()) {
            eReturn = Parameters.PValueReportingType.GUMBEL_PVALUE;
        }
        return eReturn;
    }
    
    /**
     * sets p-value reporting option type control index
     */
    private void setPValueReportingControlType(Parameters.PValueReportingType ePValueReportingType) {
        switch (ePValueReportingType) {
            case DEFAULT_PVALUE:
                _radioDefaultPValues.setSelected(true);
                break;
            case TERMINATION_PVALUE:
                _radioEarlyTerminationPValues.setSelected(true);
                break;
            case STANDARD_PVALUE:
                _radioStandardPValues.setSelected(true);
                break;
            case GUMBEL_PVALUE:
                _radioGumbelPValues.setSelected(true);
                break;
        }
    }

    /* returns adjustment for time trend type from control selection */
    private Parameters.TimeTrendAdjustmentType getAdjustmentTimeTrendControlType() {
        Parameters.TimeTrendAdjustmentType eReturn = Parameters.TimeTrendAdjustmentType.TEMPORAL_NOTADJUSTED;
        if (Utils.selected(_temporalTrendAdjNonparametric)) {
            eReturn = Parameters.TimeTrendAdjustmentType.TEMPORAL_STRATIFIED_RANDOMIZATION;
        } else if (Utils.selected(_temporalTrendAdjLogLinear)) {
            eReturn = Parameters.TimeTrendAdjustmentType.LOGLINEAR_PERC;
        } else if (Utils.selected(_temporalTrendAdjLogLinearCalc)) {
            eReturn = Parameters.TimeTrendAdjustmentType.CALCULATED_LOGLINEAR_PERC;
        } else if (Utils.selected(_temporalTrendAdjQuadCalc)) {
            eReturn = Parameters.TimeTrendAdjustmentType.CALCULATED_QUADRATIC;
        }
        return eReturn;
    }

    /* Sets time trend adjustment control's index */
    private void setTemporalTrendAdjustmentControl(Parameters.TimeTrendAdjustmentType eTimeTrendAdjustmentType) {
        switch (eTimeTrendAdjustmentType) {
            case LOGLINEAR_PERC:
                _temporalTrendAdjLogLinear.setSelected(true);
                break;
            case CALCULATED_LOGLINEAR_PERC:
                _temporalTrendAdjLogLinearCalc.setSelected(true);
                break;
            case CALCULATED_QUADRATIC:
                _temporalTrendAdjQuadCalc.setSelected(true);
                break;
            case TEMPORAL_NONPARAMETRIC: // Non-parametric isn't an option in the GUI.
            case TEMPORAL_STRATIFIED_RANDOMIZATION:
                _temporalTrendAdjNonparametric.setSelected(true);
                break;
            case TEMPORAL_NOTADJUSTED:
            default:
                _temporalTrendAdjNone.setSelected(true);
        }
    }

    /**
     * enables or disables the temporal time trend adjustment control group
     */
    private void enableAdjustmentForTimeTrendOptionsGroup(boolean bEnable, boolean bNonparametric, boolean bLogYearPercentage, boolean bCalculatedLog) {
        Parameters.TimeTrendAdjustmentType eTimeTrendAdjustmentType = getAdjustmentTimeTrendControlType();

        // trump control enables
        bNonparametric &= bEnable && !Utils.selected(_adjustDayOfWeek);
        bLogYearPercentage &= bEnable;
        bCalculatedLog &= bEnable;

        _temporalTrendAdjGroup.setEnabled(bEnable);
        _temporalTrendAdjNonparametric.setEnabled(bNonparametric);
        if (!bNonparametric && eTimeTrendAdjustmentType == Parameters.TimeTrendAdjustmentType.TEMPORAL_STRATIFIED_RANDOMIZATION) {
            setTemporalTrendAdjustmentControl(Parameters.TimeTrendAdjustmentType.TEMPORAL_NOTADJUSTED);
        }

        _temporalTrendAdjLogLinear.setEnabled(bLogYearPercentage);
        _logLinearLabel.setEnabled(bLogYearPercentage);
        _logLinearTextField.setEnabled(bLogYearPercentage && eTimeTrendAdjustmentType == Parameters.TimeTrendAdjustmentType.LOGLINEAR_PERC);
        if (!bLogYearPercentage && eTimeTrendAdjustmentType == Parameters.TimeTrendAdjustmentType.LOGLINEAR_PERC) {
            setTemporalTrendAdjustmentControl(Parameters.TimeTrendAdjustmentType.TEMPORAL_NOTADJUSTED);
        }

        _temporalTrendAdjLogLinearCalc.setEnabled(bCalculatedLog);
        if (!bCalculatedLog && eTimeTrendAdjustmentType == Parameters.TimeTrendAdjustmentType.CALCULATED_LOGLINEAR_PERC) {
            setTemporalTrendAdjustmentControl(Parameters.TimeTrendAdjustmentType.TEMPORAL_NOTADJUSTED);
        }

        _temporalTrendAdjQuadCalc.setEnabled(bCalculatedLog);
        if (!bCalculatedLog && eTimeTrendAdjustmentType == Parameters.TimeTrendAdjustmentType.CALCULATED_QUADRATIC) {
            setTemporalTrendAdjustmentControl(Parameters.TimeTrendAdjustmentType.TEMPORAL_NOTADJUSTED);
        }
    }

    private void enableAdjustmentForSpatialOptionsGroup(boolean bEnable, boolean bEnableNonparametric) {
        _spatialAdjustmentsGroup.setEnabled(bEnable);
        bEnableNonparametric &= bEnable;
        _spatialAdjustmentsNonparametric.setEnabled(bEnableNonparametric);
        if (bEnable && !bEnableNonparametric && _spatialAdjustmentsNonparametric.isSelected()) {
            _spatialAdjustmentsNone.setSelected(true);
        }
    }

    /**
     * enables temporal options controls
     */
    private void enableTemporalRanges(boolean bEnable, boolean bEnableRanges) {
        _flexibleTemporalWindowDefinitionGroup.setEnabled(bEnable && bEnableRanges);
        _restrictTemporalRangeCheckBox.setEnabled(bEnable && bEnableRanges);
        _startWindowRangeLabel.setEnabled(bEnable && bEnableRanges && _restrictTemporalRangeCheckBox.isSelected());
        _startRangeToLabel.setEnabled(bEnable && bEnableRanges && _restrictTemporalRangeCheckBox.isSelected());
        _endWindowRangeLabel.setEnabled(bEnable && bEnableRanges && _restrictTemporalRangeCheckBox.isSelected());
        _endRangeToLabel.setEnabled(bEnable && bEnableRanges && _restrictTemporalRangeCheckBox.isSelected());

        _startGenericWindowRangeLabel.setEnabled(bEnable && bEnableRanges && _restrictTemporalRangeCheckBox.isSelected());
        _startGenericRangeToLabel.setEnabled(bEnable && bEnableRanges && _restrictTemporalRangeCheckBox.isSelected());
        _endGenericWindowRangeLabel.setEnabled(bEnable && bEnableRanges && _restrictTemporalRangeCheckBox.isSelected());
        _endGenericRangeToLabel.setEnabled(bEnable && bEnableRanges && _restrictTemporalRangeCheckBox.isSelected());
    }

    /** enables controls of 'Temporal Graphs' groups */
    private void enableTemporalGraphsGroup(boolean enable) {
        _graphOutputGroup.setEnabled(enable);
        _reportTemporalGraph.setEnabled(_graphOutputGroup.isEnabled());
        _temporalGraphMostLikely.setEnabled(enable && _reportTemporalGraph.isSelected());
        _temporalGraphMostLikelyX.setEnabled(enable && _reportTemporalGraph.isSelected());
        _numMostLikelyClustersGraph.setEnabled(enable && _reportTemporalGraph.isSelected());
        _numMostLikelyClustersGraphLabel.setEnabled(enable && _reportTemporalGraph.isSelected());
        _temporalGraphSignificant.setEnabled(enable && _reportTemporalGraph.isSelected());
        _temporalGraphPvalueCutoff.setEnabled(enable && _reportTemporalGraph.isSelected());
        Parameters.AnalysisType analysis_type = _settings_window.getAnalysisControlType();
        double val = Double.parseDouble(_temporalGraphPvalueCutoff.getText());
        if (analysis_type == Parameters.AnalysisType.PROSPECTIVEPURELYTEMPORAL || analysis_type == Parameters.AnalysisType.PROSPECTIVESPACETIME) {
            if (val <= 1) _temporalGraphPvalueCutoff.setText(AppConstants.DEFAULT_RECURRENCE_CUTOFF);
            _temporalGraphSignificant.setText("All clusters, one graph for each, with a recurrence interval greater than or equal:");
        } else {
            _temporalGraphSignificant.setText("All clusters, one graph for each, with p-value less than or equal:");
            if (val > 1) _temporalGraphPvalueCutoff.setText(AppConstants.DEFAULT_PVALUE_CUTOFF);
        }        
    }

    /**
     * enables or disables the temporal options group control
     */
    private void enableTemporalOptionsGroup(boolean bEnable, boolean bEnableIncludePurelySpatial, boolean bEnableRanges) {
        _maxTemporalOptionsGroup.setEnabled(bEnable);
        _percentageTemporalRadioButton.setEnabled(bEnable);
        _maxTemporalClusterSizeTextField.setEnabled(bEnable && _percentageTemporalRadioButton.isSelected());
        _percentageOfStudyPeriodLabel.setEnabled(bEnable);
        _timeTemporalRadioButton.setEnabled(bEnable);
        _maxTemporalClusterSizeUnitsTextField.setEnabled(bEnable && _timeTemporalRadioButton.isSelected());
        _maxTemporalTimeUnitsLabel.setEnabled(bEnable);
        _minTemporalOptionsGroup.setEnabled(bEnable);
        _minTemporalClusterSizeUnitsTextField.setEnabled(bEnable);
        _minTemporalTimeUnitsLabel.setEnabled(bEnable);
        _includePureSpacClustCheckBox.setEnabled(bEnable && bEnableIncludePurelySpatial);
        enableTemporalRanges(bEnable, bEnableRanges);
    }

    /**
     * Enables isotonic scan feature controls.
     */
    private void enableIsotonicScan(boolean bEnable) {
        _performIsotonicScanCheckBox.setEnabled(bEnable);
    }

    /**
     * Enables controls on the clusters reported tab.
     */
    private void enableClustersReportedOptions(boolean bEnable) {
        _hierarchicalSecondaryClusters.setEnabled(bEnable && _mostLikelyClustersHierarchically.isEnabled() && _mostLikelyClustersHierarchically.isSelected());
        _hierarchicalLabel.setEnabled(_hierarchicalSecondaryClusters.isEnabled());
        // enable the advanced settings for index based clusters reported group
        _checkboxReportIndexCoefficients.setEnabled(_giniOptimizedClusters.isEnabled() && _giniOptimizedClusters.isSelected());
    }

    /**
     * Set appropriate control for maximum spatial cluster size type.
     */
    private void setMaxSpatialClusterSizeControl(Parameters.SpatialSizeType eSpatialSizeType, double dMaxSize) {
        switch (eSpatialSizeType) {
            case MAXDISTANCE:
                _maxSpatialRadiusTextField.setText(dMaxSize <= 0 ? "1" : Double.toString(dMaxSize));
                break;
            case PERCENTOFMAXCIRCLEFILE:
                _maxSpatialPercentFileTextField.setText(dMaxSize <= 0 || dMaxSize > 50 ? "50" : Double.toString(dMaxSize));
                break;
            case PERCENTOFPOPULATION:
            default:
                _maxSpatialClusterSizeTextField.setText(dMaxSize <= 0 || dMaxSize > 50 ? "50" : Double.toString(dMaxSize));
                break;
        }
    }

    void setMultipleCoordinatesType(Parameters.MultipleCoordinatesType eType) {
        switch (eType) {
            case ATLEASTONELOCATION:
                _atLeastOneRadioButton.setSelected(true);
                break;
            case ALLLOCATIONS:
                _allLocationsRadioButton.setSelected(true);
                break;
            case ONEPERLOCATION:
            default:
                _onePerLocationIdRadioButton.setSelected(true);
        }
    }

    /**
     * Set appropriate control for maximum spatial cluster size type.
     */
    private void setMaxTemporalClusterSizeControl(double dMaxSize) {
        Double d = Double.valueOf(dMaxSize);
        double dMaxValue = _settings_window.getModelControlType() == Parameters.ProbabilityModelType.SPACETIMEPERMUTATION ? 50.0 : 90.0;

        switch (getMaxTemporalClusterSizeControlType()) {
            case TIMETYPE:
                _maxTemporalClusterSizeUnitsTextField.setText(dMaxSize <= 0 ? "1" : Integer.toString(d.intValue()));
                break;
            case PERCENTAGETYPE:
            default:
                _maxTemporalClusterSizeTextField.setText(dMaxSize <= 0 || dMaxSize > dMaxValue ? Double.toString(dMaxValue) : Double.toString(dMaxSize));
                break;
        }
    }

    /** Sets spatial adjustment control type */
    private void setSpatialAdjustmentTypeControl(Parameters.SpatialAdjustmentType eSpatialAdjustmentType) {
        switch (eSpatialAdjustmentType) {
            case SPATIAL_NONPARAMETRIC:
            case SPATIAL_STRATIFIED_RANDOMIZATION:
                _spatialAdjustmentsNonparametric.setSelected(true);
                break;
            case SPATIAL_NOTADJUSTED:
            default:
                _spatialAdjustmentsNone.setSelected(true);
        }
    }

    /**
     * Sets study period data checking control's index
     */
    private void setStudyPeriodDataCheckingControl(Parameters.StudyPeriodDataCheckingType eStudyPeriodDataCheckingType) {
        switch (eStudyPeriodDataCheckingType) {
            case RELAXEDBOUNDS:
                _relaxedStudyPeriodCheckRadioButton.setSelected(true);
                break;
            case STRICTBOUNDS:
            default:
                _strictStudyPeriodCheckRadioButton.setSelected(true);
                break;
        }
    }

    private void setCoordinatesDataCheckingControl(Parameters.CoordinatesDataCheckingType eCoordinatesDataCheckingType) {
        switch (eCoordinatesDataCheckingType) {
            case STRICTCOORDINATES:
                _strictCoordinatesRadioButton.setSelected(true);
                break;
            case RELAXEDCOORDINATES:
            default:
                _relaxedCoordinatesRadioButton.setSelected(true);
                break;
        }
    }

    /**
     * Set appropriate control for maximum spatial cluster size type.
     */
    private void setMaxReportedSpatialClusterSizeControl(Parameters.SpatialSizeType eSpatialSizeType, double dMaxSize) {
        switch (eSpatialSizeType) {
            case MAXDISTANCE:
                _maxReportedSpatialRadiusTextField.setText(dMaxSize <= 0 ? "1" : Double.toString(dMaxSize));
                break;
            case PERCENTOFMAXCIRCLEFILE:
                _maxReportedSpatialPercentFileTextField.setText(dMaxSize <= 0 || dMaxSize > 50 ? "50" : Double.toString(dMaxSize));
                break;
            case PERCENTOFPOPULATION:
            default:
                _maxReportedSpatialClusterSizeTextField.setText(dMaxSize <= 0 || dMaxSize > 50 ? "50" : Double.toString(dMaxSize));
                break;
        }
    }

    private void setupInterface(final Parameters parameters) {
        // Spatial Window tab
        setMaxSpatialClusterSizeControl(Parameters.SpatialSizeType.PERCENTOFPOPULATION, parameters.GetMaxSpatialSizeForType(Parameters.SpatialSizeType.PERCENTOFPOPULATION.ordinal(), false));
        setMaxSpatialClusterSizeControl(Parameters.SpatialSizeType.PERCENTOFMAXCIRCLEFILE, parameters.GetMaxSpatialSizeForType(Parameters.SpatialSizeType.PERCENTOFMAXCIRCLEFILE.ordinal(), false));
        setMaxSpatialClusterSizeControl(Parameters.SpatialSizeType.MAXDISTANCE, parameters.GetMaxSpatialSizeForType(Parameters.SpatialSizeType.MAXDISTANCE.ordinal(), false));
        _spatialPopulationFileCheckBox.setSelected(parameters.GetRestrictMaxSpatialSizeForType(Parameters.SpatialSizeType.PERCENTOFMAXCIRCLEFILE.ordinal(), false));
        _spatialDistanceCheckBox.setSelected(parameters.GetRestrictMaxSpatialSizeForType(Parameters.SpatialSizeType.MAXDISTANCE.ordinal(), false));

        _maxCirclePopulationFilenameTextField.setText(parameters.GetMaxCirclePopulationFileName());
        _inclPureTempClustCheckBox.setSelected(parameters.GetIncludePurelyTemporalClusters());

        _circularRadioButton.setSelected(parameters.GetSpatialWindowType() == Parameters.SpatialWindowType.CIRCULAR);
        _ellipticRadioButton.setSelected(parameters.GetSpatialWindowType() == Parameters.SpatialWindowType.ELLIPTIC);
        _nonCompactnessPenaltyComboBox.select(parameters.GetNonCompactnessPenaltyType().ordinal());
        setSpatialDistanceCaption();
        _performIsotonicScanCheckBox.setSelected(parameters.GetRiskType() == Parameters.RiskType.MONOTONERISK);

        // Multiple Coordinates Tab
        setMultipleCoordinatesType(parameters.GetMultipleCoordinatesType());
        _multiple_locations_file.setText(parameters.getMultipleLocationsFile());

        // Temporal tab
        _minTemporalClusterSizeUnitsTextField.setText(Integer.toString(parameters.getMinimumTemporalClusterSize()));
        setMaxTemporalClusterSizeTypeControl(parameters.GetMaximumTemporalClusterSizeType());
        setMaxTemporalClusterSizeControl(parameters.GetMaximumTemporalClusterSize());
        _includePureSpacClustCheckBox.setSelected(parameters.GetIncludePurelySpatialClusters());
        _restrictTemporalRangeCheckBox.setSelected(parameters.GetIncludeClustersType() == Parameters.IncludeClustersType.CLUSTERSINRANGE);
        if (parameters.GetPrecisionOfTimesType().equals(Parameters.DatePrecisionType.GENERIC)) {
            Utils.parseDateStringToControl(parameters.GetStartRangeStartDate(), _startRangeStartGenericTextField);
            Utils.parseDateStringToControl(parameters.GetStartRangeEndDate(), _startRangeEndGenericTextField);
            Utils.parseDateStringToControl(parameters.GetEndRangeStartDate(), _endRangeStartGenericTextField);
            Utils.parseDateStringToControl(parameters.GetEndRangeEndDate(), _endRangeEndGenericTextField);
        } else {
            Utils.parseDateStringToControls(parameters.GetStartRangeStartDate(), _startRangeStartYearTextField, _startRangeStartMonthTextField, _startRangeStartDayTextField, false);
            Utils.parseDateStringToControls(parameters.GetStartRangeEndDate(), _startRangeEndYearTextField, _startRangeEndMonthTextField, _startRangeEndDayTextField, false);
            Utils.parseDateStringToControls(parameters.GetEndRangeStartDate(), _endRangeStartYearTextField, _endRangeStartMonthTextField, _endRangeStartDayTextField, true);
            Utils.parseDateStringToControls(parameters.GetEndRangeEndDate(), _endRangeEndYearTextField, _endRangeEndMonthTextField, _endRangeEndDayTextField, true);
        }
        // Space-Time Adjustments tab
        setTemporalTrendAdjustmentControl(parameters.GetTimeTrendAdjustmentType());
        _logLinearTextField.setText(parameters.GetTimeTrendAdjustmentPercentage() <= -100 ? "0" : Double.toString(parameters.GetTimeTrendAdjustmentPercentage()));
        _adjustDayOfWeek.setSelected(parameters.getAdjustForWeeklyTrends());
        setSpatialAdjustmentTypeControl(parameters.GetSpatialAdjustmentType());
        _adjustForKnownRelativeRisksCheckBox.setSelected(parameters.UseAdjustmentForRelativeRisksFile());
        _adjustmentsByRelativeRisksFileTextField.setText(parameters.GetAdjustmentsByRelativeRisksFilename());

        // Inference tab
        setPValueReportingControlType(parameters.GetPValueReportingType());
        _checkReportGumbel.setSelected(parameters.GetReportGumbelPValue());
        _earlyTerminationThreshold.setText(Integer.toString(parameters.GetEarlyTermThreshold()));
                
        _performIterativeScanCheckBox.setSelected(parameters.GetIsIterativeScanning());
        _numIterativeScansTextField.setText(parameters.GetNumIterativeScansRequested() < 1 || parameters.GetNumIterativeScansRequested() > Parameters.MAXIMUM_ITERATIVE_ANALYSES ? "10" : Integer.toString(parameters.GetNumIterativeScansRequested()));
        _iterativeScanCutoffTextField.setText(parameters.GetIterativeCutOffPValue() <= 0 || parameters.GetIterativeCutOffPValue() > 1 ? "0.05" : Double.toString(parameters.GetIterativeCutOffPValue()));
        _montCarloReplicationsTextField.setText(Integer.toString(parameters.GetNumReplicationsRequested()));

        // Miscellaneous analysis tab
        _calculate_oliveiras_f.setSelected(parameters.getCalculateOliveirasF());
        _number_oliveira_data_sets.setText(Integer.toString(parameters.getNumRequestedOliveiraSets()));
        _prospective_frequency.select(parameters.getProspectiveFrequencyType().ordinal());
        
        // Spatial Clusters tab
        _mostLikelyClustersHierarchically.setSelected(parameters.getReportHierarchicalClusters());
        _giniOptimizedClusters.setSelected(parameters.getReportGiniOptimizedClusters());
        _restrictReportedClustersCheckBox.setSelected(parameters.GetRestrictingMaximumReportedGeoClusterSize());
        _hierarchicalSecondaryClusters.select(parameters.GetCriteriaSecondClustersType().ordinal());
        _checkboxReportIndexCoefficients.setSelected(parameters.getReportGiniIndexCoefficents());
        setMaxReportedSpatialClusterSizeControl(Parameters.SpatialSizeType.PERCENTOFPOPULATION, parameters.GetMaxSpatialSizeForType(Parameters.SpatialSizeType.PERCENTOFPOPULATION.ordinal(), true));
        setMaxReportedSpatialClusterSizeControl(Parameters.SpatialSizeType.PERCENTOFMAXCIRCLEFILE, parameters.GetMaxSpatialSizeForType(Parameters.SpatialSizeType.PERCENTOFMAXCIRCLEFILE.ordinal(), true));
        setMaxReportedSpatialClusterSizeControl(Parameters.SpatialSizeType.MAXDISTANCE, parameters.GetMaxSpatialSizeForType(Parameters.SpatialSizeType.MAXDISTANCE.ordinal(), true));

        // Spatial Output tab
        _reportedSpatialPopulationFileCheckBox.setSelected(parameters.GetRestrictMaxSpatialSizeForType(Parameters.SpatialSizeType.PERCENTOFMAXCIRCLEFILE.ordinal(), true));
        _reportedSpatialDistanceCheckBox.setSelected(parameters.GetRestrictMaxSpatialSizeForType(Parameters.SpatialSizeType.MAXDISTANCE.ordinal(), true));
        _includeClusterLocationsInKML.setSelected(parameters.getIncludeLocationsKML());
        _createCompressedKMZ.setSelected(parameters.getCompressClusterKML());
        _launch_map_viewer.setSelected(parameters.getLaunchMapViewer());

        // Multiple Data Sets tab
        enableAdditionalDataSetsGroup(false);
        for (int i = 1; i < parameters.GetNumDataSets(); i++) { // multiple data sets
            _dataSetsListModel.addElement("Data Set " + Integer.toString(i + 1));
            _caseFilenames.add(parameters.GetCaseFileName(i + 1));
            _controlFilenames.add(parameters.GetControlFileName(i + 1));
            _populationFilenames.add(parameters.GetPopulationFileName(i + 1));
        }
        if (_dataSetsListModel.size() > 0) {
            _inputDataSetsList.setSelectedIndex(0);
        }
        _multivariateAdjustmentsRadioButton.setSelected(parameters.GetMultipleDataSetPurposeType() == Parameters.MultipleDataSetPurposeType.MULTIVARIATE);
        _adjustmentByDataSetsRadioButton.setSelected(parameters.GetMultipleDataSetPurposeType() == Parameters.MultipleDataSetPurposeType.ADJUSTMENT);

        // Data Checking Tab
        setStudyPeriodDataCheckingControl(parameters.GetStudyPeriodDataCheckingType());
        setCoordinatesDataCheckingControl(parameters.GetCoordinatesDataCheckingType());

        // Neighbors Tab
        _specifiyNeighborsFileCheckBox.setSelected(parameters.UseLocationNeighborsFile());
        _neighborsFileTextField.setText(parameters.GetLocationNeighborsFileName());
        _specifiyMetaLocationsFileCheckBox.setSelected(parameters.UseMetaLocationsFile());
        _metaLocationsFileTextField.setText(parameters.GetMetaLocationsFileName());

        // Power Evaluations tab
        _performPowerEvalautions.setSelected(parameters.getPerformPowerEvaluation());
        _partOfRegularAnalysis.setSelected(parameters.getPowerEvaluationMethod() == Parameters.PowerEvaluationMethodType.PE_ONLY_CASEFILE);
        _powerEvaluationWithCaseFile.setSelected(parameters.getPowerEvaluationMethod() == Parameters.PowerEvaluationMethodType.PE_ONLY_CASEFILE);
        _powerEvaluationWithSpecifiedCases.setSelected(parameters.getPowerEvaluationMethod() == Parameters.PowerEvaluationMethodType.PE_ONLY_SPECIFIED_CASES);
        _totalPowerCases.setText(Integer.toString(parameters.getPowerEvaluationCaseCount()));
        _numberPowerReplications.setText(Integer.toString(parameters.getNumPowerEvalReplicaPowerStep()));
        _criticalValuesMonteCarlo.setSelected(parameters.getPowerEvaluationCriticalValueType() == Parameters.CriticalValuesType.CV_MONTECARLO);
        _criticalValuesGumbel.setSelected(parameters.getPowerEvaluationCriticalValueType() == Parameters.CriticalValuesType.CV_GUMBEL);
        _powerEstimationMonteCarlo.setSelected(parameters.getPowerEstimationType() == Parameters.PowerEstimationType.PE_MONTECARLO);
        _powerEstimationGumbel.setSelected(parameters.getPowerEstimationType() == Parameters.PowerEstimationType.PE_GUMBEL);
        _alternativeHypothesisFilename.setText(parameters.getPowerEvaluationAltHypothesisFilename());

        // Temporal Output tab
        _reportTemporalGraph.setSelected(parameters.getOutputTemporalGraphFile());
        _temporalGraphMostLikely.setSelected(parameters.getTemporalGraphReportType() == Parameters.TemporalGraphReportType.MLC_ONLY);
        _temporalGraphMostLikelyX.setSelected(parameters.getTemporalGraphReportType() == Parameters.TemporalGraphReportType.X_MCL_ONLY);
        _numMostLikelyClustersGraph.setText(Integer.toString(parameters.getTemporalGraphMostLikelyCount()));
        _temporalGraphSignificant.setSelected(parameters.getTemporalGraphReportType() == Parameters.TemporalGraphReportType.SIGNIFICANT_ONLY);
        updateCriticalValuesTextCaptions();
        
        // Cluster Restrictions tab
        _minimum_number_cases_cluster.setText(Integer.toString(parameters.getMinimumCasesHighRateClusters()));
        _limit_high_clusters.setSelected(parameters.getRiskLimitHighClusters());
        _limit_high_clusters_value.setText(Double.toString(parameters.getRiskThresholdHighClusters()));
        _limit_low_clusters.setSelected(parameters.getRiskLimitLowClusters());
        _limit_low_clusters_value.setText(Double.toString(parameters.getRiskThresholdLowClusters()));
     
        // Drilldown tab
        _mainAnalysisDrilldown.setSelected(parameters.getPerformStandardDrilldown());
        _purelySpatialDrilldown.setSelected(parameters.getPerformBernoulliDrilldown());
        _drilldown_restriction_locations.setText(Integer.toString(parameters.getDrilldownMinimumLocationsCluster()));
        _drilldown_restriction_cases.setText(Integer.toString(parameters.getDrilldownMinimumCasesCluster()));
        _drilldown_restriction_dow.setSelected(parameters.getDrilldownAdjustWeeklyTrends());
        
        // Network tab
        _locations_network.setSelected(parameters.getUseLocationsNetworkFile());
        _network_filename.setText(parameters.getLocationsNetworkFilename());
        
        // Notifications tab
        _always_sendmail.setSelected(parameters.getAlwaysEmailSummary());
        _always_email_recipients.setText(parameters.getEmailAlwaysRecipients());
        _cutoff_email.setSelected(parameters.getCutoffEmailSummary());
        _cutoff_email_recipients.setText(parameters.getEmailCutoffRecipients());  
        _attach_main_results_email.setSelected(parameters.getEmailAttachResults());
        _report_main_results_email.setSelected(parameters.getEmailIncludeResultsDirectory());
        _create_custom_email_message.setSelected(parameters.getEmailCustom());
        _custom_email_subject.setText(parameters.getEmailCustomSubject());
        _custom_email_message.setText(substituteNewlines(parameters.getEmailCustomMessageBody()));
        
        // Other Output Tab
        _reportCriticalValuesCheckBox.setSelected(parameters.GetReportCriticalValues());    
        _reportClusterRankCheckBox.setSelected(parameters.getReportClusterRank());
        _printAsciiColumnHeaders.setSelected(parameters.getPrintAsciiHeaders());
        _printTitle.setText(parameters.GetTitleName());
    }

    /** Some controls can't be set from the Parameters object until after the ParameterSettingsFrame object
     *  has been loaded. There can be issues with some of the listeners being called before analysis type
     *  controls have been set.
     */
    public void setupInterfaceFinalize(final Parameters parameters) {
        Parameters.AnalysisType analysisType = this._settings_window.getAnalysisControlType();
        if (analysisType == Parameters.AnalysisType.PROSPECTIVEPURELYTEMPORAL ||
            analysisType == Parameters.AnalysisType.PROSPECTIVESPACETIME) {
            _temporalGraphPvalueCutoff.setText(Integer.toString(Double.valueOf(parameters.getTemporalGraphSignificantCutoff()).intValue()));
            _drilldown_restriction_cutoff.setText(Integer.toString(Double.valueOf(parameters.getDrilldownCutoff()).intValue()));
            _cutoff_value_email.setText(Integer.toString(Double.valueOf(parameters.getCutoffEmailValue()).intValue()));
            _cluster_lineline_value.setText(Integer.toString(Double.valueOf(parameters.getCutoffLineListCSV()).intValue()));
        } else {
            _temporalGraphPvalueCutoff.setText(Double.toString(parameters.getTemporalGraphSignificantCutoff()));        
            _drilldown_restriction_cutoff.setText(Double.toString(parameters.getDrilldownCutoff()));
            _cutoff_value_email.setText(Double.toString(parameters.getCutoffEmailValue()));
            _cluster_lineline_value.setText(Double.toString(parameters.getCutoffLineListCSV()));
        }
    }    
    
    /**
     * Enabled the power evaluations group based upon current settings.
     */
    private void enablePowerEvaluationsGroup() {
        Parameters.AnalysisType eAnalysisType = _settings_window.getAnalysisControlType();
        Parameters.ProbabilityModelType eModelType = _settings_window.getModelControlType();

        boolean bEnableGroup = eModelType == Parameters.ProbabilityModelType.POISSON && eAnalysisType != Parameters.AnalysisType.SPATIALVARTEMPTREND;
        _powerEvaluationsGroup.setEnabled(bEnableGroup);
        _performPowerEvalautions.setEnabled(bEnableGroup);
        _partOfRegularAnalysis.setEnabled(bEnableGroup && _performPowerEvalautions.isSelected());
        _powerEvaluationWithCaseFile.setEnabled(bEnableGroup && _performPowerEvalautions.isSelected());
        _powerEvaluationWithSpecifiedCases.setEnabled(bEnableGroup && _performPowerEvalautions.isSelected());
        _totalPowerCases.setEnabled(bEnableGroup && _performPowerEvalautions.isSelected() && _powerEvaluationWithSpecifiedCases.isSelected());
        _powerEvaluationWithSpecifiedCasesLabel.setEnabled(bEnableGroup && _performPowerEvalautions.isSelected() && _powerEvaluationWithSpecifiedCases.isSelected());
        _criticalValuesTypeLabel.setEnabled(bEnableGroup && _performPowerEvalautions.isSelected());
        _criticalValuesMonteCarlo.setEnabled(bEnableGroup && _performPowerEvalautions.isSelected());
        _criticalValuesGumbel.setEnabled(bEnableGroup && _performPowerEvalautions.isSelected());
        _criticalValuesReplicationsLabel.setEnabled(bEnableGroup && _performPowerEvalautions.isSelected());
        _powerEstimationMonteCarlo.setEnabled(bEnableGroup && _performPowerEvalautions.isSelected());
        _powerEstimationGumbel.setEnabled(bEnableGroup && _performPowerEvalautions.isSelected());
        _powerEstimationTypeLabel.setEnabled(bEnableGroup && _performPowerEvalautions.isSelected());
        _alternativeHypothesisFilenameLabel.setEnabled(bEnableGroup && _performPowerEvalautions.isSelected());
        _alternativeHypothesisFilename.setEnabled(bEnableGroup && _performPowerEvalautions.isSelected());
        _alternativeHypothesisFilenameButton.setEnabled(bEnableGroup && _performPowerEvalautions.isSelected());
        _numberPowerReplicationsLabel.setEnabled(bEnableGroup && _performPowerEvalautions.isSelected());
        _numberPowerReplications.setEnabled(bEnableGroup && _performPowerEvalautions.isSelected());
    }

    /**
     * Enables clusters reported group based upon other primary settings, such
     * as analysis type and probability model.
     */
    private void enableClustersReportedGroup() {
        Parameters.AnalysisType eAnalysisType = _settings_window.getAnalysisControlType();
        Parameters.ProbabilityModelType eModelType = _settings_window.getModelControlType();

        boolean bEnableGroup = !(eAnalysisType == Parameters.AnalysisType.PURELYTEMPORAL || 
                                 eAnalysisType == Parameters.AnalysisType.PROSPECTIVEPURELYTEMPORAL ||
                                 eAnalysisType == Parameters.AnalysisType.SEASONALTEMPORAL);
        _clustersReportedGroup.setEnabled(bEnableGroup);
        _mostLikelyClustersHierarchically.setEnabled(bEnableGroup);
        boolean enableIndexBased = eAnalysisType == Parameters.AnalysisType.PURELYSPATIAL;
        enableIndexBased &= eModelType == Parameters.ProbabilityModelType.POISSON || eModelType == Parameters.ProbabilityModelType.BERNOULLI;
        enableIndexBased &= !(_multivariateAdjustmentsRadioButton.isEnabled() && _multivariateAdjustmentsRadioButton.isSelected());
        enableIndexBased &= !(_performIsotonicScanCheckBox.isEnabled() && _performIsotonicScanCheckBox.isSelected());
        enableIndexBased &= !(_performIterativeScanCheckBox.isEnabled() && _performIterativeScanCheckBox.isSelected());
        _giniOptimizedClusters.setEnabled(bEnableGroup && enableIndexBased);
        enableClustersReportedOptions(bEnableGroup);
    }

    /**
     * Enables clusters reported group based upon other primary settings, such
     * as analysis type and probability model.
     */
    private void enableDrilldownGroup() {
        Parameters.AnalysisType eAnalysisType = _settings_window.getAnalysisControlType();
        Parameters.ProbabilityModelType eModelType = _settings_window.getModelControlType();
        
        boolean bEnableGroup = (
            eAnalysisType == Parameters.AnalysisType.SPACETIME || 
            eAnalysisType == Parameters.AnalysisType.PROSPECTIVESPACETIME ||
            eAnalysisType == Parameters.AnalysisType.PURELYSPATIAL ||
            eAnalysisType == Parameters.AnalysisType.SPATIALVARTEMPTREND
        );
        // This feature is not implemented when using meta locations.
        bEnableGroup &= !(
            _specifiyNeighborsFileCheckBox.isEnabled() && _specifiyNeighborsFileCheckBox.isSelected() &&
            _specifiyMetaLocationsFileCheckBox.isEnabled() && _specifiyMetaLocationsFileCheckBox.isSelected()
        );
        _drilldown_restrictions_group.setEnabled(bEnableGroup);
        _mainAnalysisDrilldown.setEnabled(bEnableGroup);
        _purelySpatialDrilldown.setEnabled(
            bEnableGroup &&
            (eAnalysisType == Parameters.AnalysisType.SPACETIME || eAnalysisType == Parameters.AnalysisType.PROSPECTIVESPACETIME)  &&
            (eModelType == Parameters.ProbabilityModelType.POISSON || eModelType == Parameters.ProbabilityModelType.SPACETIMEPERMUTATION)
        );
        boolean mainSelected = _mainAnalysisDrilldown.isEnabled() && _mainAnalysisDrilldown.isSelected();
        boolean bernoulliSelected = _purelySpatialDrilldown.isEnabled() && _purelySpatialDrilldown.isSelected();
        boolean drilldownSelected = mainSelected || bernoulliSelected;
        _drilldown_restrictions.setEnabled(bEnableGroup && drilldownSelected);
        _drilldown_restriction_cutoff_label.setEnabled(bEnableGroup && drilldownSelected);
        _drilldown_restriction_cutoff.setEnabled(drilldownSelected);
        _drilldown_restriction_locations_label.setEnabled(bEnableGroup && drilldownSelected);
        _drilldown_restriction_locations.setEnabled(bEnableGroup && drilldownSelected);
        _drilldown_restriction_cases_label.setEnabled(bEnableGroup && drilldownSelected);
        _drilldown_restriction_cases.setEnabled(bEnableGroup && drilldownSelected);
        // Adjustment by day of week is permitted for only purely spatia Bernoulli and one data set.
        _drilldown_restriction_dow.setEnabled(bEnableGroup && bernoulliSelected && _inputDataSetsList.getModel().getSize() == 0);
        
        
        double val = Double.parseDouble(_drilldown_restriction_cutoff.getText());
        if (_settings_window.isProspectiveScan()) {
            if (val <= 1) _drilldown_restriction_cutoff.setText(AppConstants.DEFAULT_RECURRENCE_CUTOFF);
            _drilldown_restriction_cutoff_label.setText("Recurrence Interval Greater Than Or Equal:");
        } else {
            _drilldown_restriction_cutoff_label.setText("P-Value Less Than Or Equal:");
            if (val > 1) _drilldown_restriction_cutoff.setText(AppConstants.DEFAULT_PVALUE_CUTOFF);
        }         
    }
    
    /* Adds listeners to a JTextField intended for p-value/recurrence interval cutoff input. */
    private void initCutoffJTextField(javax.swing.JTextField cutoff_field, String default_ri, String default_pvalue) {
        UndoManager undome = new UndoManager();
        cutoff_field.addKeyListener(new java.awt.event.KeyAdapter() {
            @Override
            public void keyTyped(java.awt.event.KeyEvent e) {
                if (_settings_window.isProspectiveScan())
                    Utils.validatePostiveNumericKeyTyped(cutoff_field, e, 10);
                else 
                    Utils.validatePostiveFloatKeyTyped(cutoff_field, e, 20);
            }
        });
        cutoff_field.addFocusListener(new java.awt.event.FocusAdapter() {
            @Override
            public void focusLost(java.awt.event.FocusEvent e) {
                if (_settings_window.isProspectiveScan()) {
                    while (cutoff_field.getText().length() == 0 ||
                        Double.parseDouble(cutoff_field.getText()) < 1)
                        if (undome.canUndo()) undome.undo(); else cutoff_field.setText(default_ri);
                } else {
                    while (cutoff_field.getText().length() == 0 ||
                            Double.parseDouble(cutoff_field.getText()) <= 0 ||
                            Double.parseDouble(cutoff_field.getText()) > 1)
                        if (undome.canUndo()) undome.undo(); else cutoff_field.setText(default_pvalue);
                }
                enableSetDefaultsButton();
            }
        });
        cutoff_field.getDocument().addUndoableEditListener((UndoableEditEvent evt) -> {
            undome.addEdit(evt.getEdit());
        });
    }
    
    /**
     * This method is called from within the constructor to initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is always
     * regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

        _multipleSetButtonGroup = new javax.swing.ButtonGroup();
        _studyPeriodCheckButtonGroup = new javax.swing.ButtonGroup();
        _geographicalCoordinatesCheckButtonGroup = new javax.swing.ButtonGroup();
        _multipleSetsSpatialCoordinatesButtonGroup = new javax.swing.ButtonGroup();
        _spatialWindowShapeButtonGroup = new javax.swing.ButtonGroup();
        _temporalOptionsButtonGroup = new javax.swing.ButtonGroup();
        _spatialAdjustmentsButtonGroup = new javax.swing.ButtonGroup();
        _temporalTrendAdjButtonGroup = new javax.swing.ButtonGroup();
        _pValueButtonGroup = new javax.swing.ButtonGroup();
        _powerEvaluationMethodButtonGroup = new javax.swing.ButtonGroup();
        _criticalValuesButtonGroup = new javax.swing.ButtonGroup();
        _powerEstimationButtonGroup = new javax.swing.ButtonGroup();
        _temporalGraphButtonGroup = new javax.swing.ButtonGroup();
        popupMenu1 = new java.awt.PopupMenu();
        jTabbedPane1 = new javax.swing.JTabbedPane();
        _multipleDataSetsTab = new javax.swing.JPanel();
        _additionalDataSetsGroup = new javax.swing.JPanel();
        _dataSetsGroup = new javax.swing.JPanel();
        jScrollPane1 = new javax.swing.JScrollPane();
        _inputDataSetsList = new javax.swing.JList(_dataSetsListModel);
        _addDataSetButton = new javax.swing.JButton();
        _removeDataSetButton = new javax.swing.JButton();
        _fileInputGroup = new javax.swing.JPanel();
        _caseFileLabel = new javax.swing.JLabel();
        _caseFileTextField = new javax.swing.JTextField();
        _caseFileBrowseButton = new javax.swing.JButton();
        _controlFileLabel = new javax.swing.JLabel();
        _controlFileTextField = new javax.swing.JTextField();
        _controlFileBrowseButton = new javax.swing.JButton();
        _populationFileLabel = new javax.swing.JLabel();
        _populationFileTextField = new javax.swing.JTextField();
        _populationFileBrowseButton = new javax.swing.JButton();
        _multipleSetPurposeGroup = new javax.swing.JPanel();
        _multipleDataSetPurposeLabel = new javax.swing.JLabel();
        _multivariateAdjustmentsRadioButton = new javax.swing.JRadioButton();
        _adjustmentByDataSetsRadioButton = new javax.swing.JRadioButton();
        _dataCheckingTab = new javax.swing.JPanel();
        _studyPeriodCheckGroup = new javax.swing.JPanel();
        _strictStudyPeriodCheckRadioButton = new javax.swing.JRadioButton();
        _relaxedStudyPeriodCheckRadioButton = new javax.swing.JRadioButton();
        _geographicalCoordinatesCheckGroup = new javax.swing.JPanel();
        _strictCoordinatesRadioButton = new javax.swing.JRadioButton();
        _relaxedCoordinatesRadioButton = new javax.swing.JRadioButton();
        _strictCoordinatesLabel = new javax.swing.JLabel();
        _spatialNeighborsTab = new javax.swing.JPanel();
        _specialNeighborFilesGroup = new javax.swing.JPanel();
        _specifiyNeighborsFileCheckBox = new javax.swing.JCheckBox();
        _neighborsFileTextField = new javax.swing.JTextField();
        _neighborsFileBrowseButton = new javax.swing.JButton();
        _specifiyMetaLocationsFileCheckBox = new javax.swing.JCheckBox();
        _metaLocationsFileTextField = new javax.swing.JTextField();
        _metaLocationsFileBrowseButton = new javax.swing.JButton();
        _multipleSetsSpatialCoordinatesGroup = new javax.swing.JPanel();
        _onePerLocationIdRadioButton = new javax.swing.JRadioButton();
        _atLeastOneRadioButton = new javax.swing.JRadioButton();
        _allLocationsRadioButton = new javax.swing.JRadioButton();
        _multiple_locations_file = new javax.swing.JTextField();
        _multiple_locations_file_browse = new javax.swing.JButton();
        _multiple_locations_file_label = new javax.swing.JLabel();
        _spatialWindowTab = new javax.swing.JPanel();
        _spatialOptionsGroup = new javax.swing.JPanel();
        _maxSpatialClusterSizeTextField = new javax.swing.JTextField();
        _percentageOfPopulationLabel = new javax.swing.JLabel();
        _spatialPopulationFileCheckBox = new javax.swing.JCheckBox();
        _maxSpatialPercentFileTextField = new javax.swing.JTextField();
        _percentageOfPopFileLabel = new javax.swing.JLabel();
        _maxCirclePopulationFilenameTextField = new javax.swing.JTextField();
        _maxCirclePopFileBrowseButton = new javax.swing.JButton();
        _spatialDistanceCheckBox = new javax.swing.JCheckBox();
        _distancePrefixLabel = new javax.swing.JLabel();
        _maxSpatialRadiusTextField = new javax.swing.JTextField();
        _maxRadiusLabel = new javax.swing.JLabel();
        _inclPureTempClustCheckBox = new javax.swing.JCheckBox();
        _spatialWindowShapeGroup = new javax.swing.JPanel();
        _circularRadioButton = new javax.swing.JRadioButton();
        _ellipticRadioButton = new javax.swing.JRadioButton();
        _nonCompactnessPenaltyLabel = new javax.swing.JLabel();
        _nonCompactnessPenaltyComboBox = new java.awt.Choice();
        _performIsotonicScanCheckBox = new javax.swing.JCheckBox();
        _temporalWindowTab = new javax.swing.JPanel();
        _maxTemporalOptionsGroup = new javax.swing.JPanel();
        _percentageTemporalRadioButton = new javax.swing.JRadioButton();
        _timeTemporalRadioButton = new javax.swing.JRadioButton();
        _maxTemporalClusterSizeTextField = new javax.swing.JTextField();
        _percentageOfStudyPeriodLabel = new javax.swing.JLabel();
        _maxTemporalClusterSizeUnitsTextField = new javax.swing.JTextField();
        _maxTemporalTimeUnitsLabel = new javax.swing.JLabel();
        _includePureSpacClustCheckBox = new javax.swing.JCheckBox();
        _flexibleTemporalWindowDefinitionGroup = new javax.swing.JPanel();
        _restrictTemporalRangeCheckBox = new javax.swing.JCheckBox();
        _flexible_window_cards = new javax.swing.JPanel();
        _windowCompletePanel = new javax.swing.JPanel();
        _startWindowRangeLabel = new javax.swing.JLabel();
        _startRangeStartYearTextField = new javax.swing.JTextField();
        _startRangeStartMonthTextField = new javax.swing.JTextField();
        _startRangeStartDayTextField = new javax.swing.JTextField();
        _startRangeToLabel = new javax.swing.JLabel();
        _startRangeEndYearTextField = new javax.swing.JTextField();
        _startRangeEndMonthTextField = new javax.swing.JTextField();
        _startRangeEndDayTextField = new javax.swing.JTextField();
        _endRangeEndDayTextField = new javax.swing.JTextField();
        _endRangeEndMonthTextField = new javax.swing.JTextField();
        _endRangeEndYearTextField = new javax.swing.JTextField();
        _endRangeToLabel = new javax.swing.JLabel();
        _endRangeStartDayTextField = new javax.swing.JTextField();
        _endRangeStartMonthTextField = new javax.swing.JTextField();
        _endRangeStartYearTextField = new javax.swing.JTextField();
        _endWindowRangeLabel = new javax.swing.JLabel();
        _windowGenericPanel = new javax.swing.JPanel();
        _startGenericWindowRangeLabel = new javax.swing.JLabel();
        _startRangeStartGenericTextField = new javax.swing.JTextField();
        _startGenericRangeToLabel = new javax.swing.JLabel();
        _startRangeEndGenericTextField = new javax.swing.JTextField();
        _endRangeEndGenericTextField = new javax.swing.JTextField();
        _endGenericRangeToLabel = new javax.swing.JLabel();
        _endRangeStartGenericTextField = new javax.swing.JTextField();
        _endGenericWindowRangeLabel = new javax.swing.JLabel();
        _minTemporalOptionsGroup = new javax.swing.JPanel();
        _minTemporalClusterSizeUnitsTextField = new javax.swing.JTextField();
        _minTemporalTimeUnitsLabel = new javax.swing.JLabel();
        _spaceTimeAjustmentsTab = new javax.swing.JPanel();
        _temporalTrendAdjGroup = new javax.swing.JPanel();
        _temporalTrendAdjNone = new javax.swing.JRadioButton();
        _temporalTrendAdjNonparametric = new javax.swing.JRadioButton();
        _temporalTrendAdjLogLinear = new javax.swing.JRadioButton();
        _temporalTrendAdjLogLinearCalc = new javax.swing.JRadioButton();
        _logLinearTextField = new javax.swing.JTextField();
        _logLinearLabel = new javax.swing.JLabel();
        _temporalTrendAdjQuadCalc = new javax.swing.JRadioButton();
        _spatialAdjustmentsGroup = new javax.swing.JPanel();
        _spatialAdjustmentsNone = new javax.swing.JRadioButton();
        _spatialAdjustmentsNonparametric = new javax.swing.JRadioButton();
        _knownAdjustmentsGroup = new javax.swing.JPanel();
        _adjustForKnownRelativeRisksCheckBox = new javax.swing.JCheckBox();
        _adjustmentsByRelativeRisksFileTextField = new javax.swing.JTextField();
        _adjustmentsFileBrowseButton = new javax.swing.JButton();
        _adjustmentsByRelativeRisksFileLabel = new javax.swing.JLabel();
        _adjustDayOfWeek = new javax.swing.JCheckBox();
        _inferenceTab = new javax.swing.JPanel();
        _pValueOptionsGroup = new javax.swing.JPanel();
        _radioDefaultPValues = new javax.swing.JRadioButton();
        _radioEarlyTerminationPValues = new javax.swing.JRadioButton();
        _radioGumbelPValues = new javax.swing.JRadioButton();
        _radioStandardPValues = new javax.swing.JRadioButton();
        _checkReportGumbel = new javax.swing.JCheckBox();
        _earlyTerminationThreshold = new javax.swing.JTextField();
        _iterativeScanGroup = new javax.swing.JPanel();
        _performIterativeScanCheckBox = new javax.swing.JCheckBox();
        _maxIterativeScansLabel = new javax.swing.JLabel();
        _numIterativeScansTextField = new javax.swing.JTextField();
        _iterativeCutoffLabel = new javax.swing.JLabel();
        _iterativeScanCutoffTextField = new javax.swing.JTextField();
        _monteCarloGroup = new javax.swing.JPanel();
        _labelMonteCarloReplications = new javax.swing.JLabel();
        _montCarloReplicationsTextField = new javax.swing.JTextField();
        _spatialOutputTab = new javax.swing.JPanel();
        _mapsOutputGroup = new javax.swing.JPanel();
        _includeClusterLocationsInKML = new javax.swing.JCheckBox();
        _createCompressedKMZ = new javax.swing.JCheckBox();
        _launch_map_viewer = new javax.swing.JCheckBox();
        _label_kml_options = new javax.swing.JLabel();
        _clustersReportedGroup = new javax.swing.JPanel();
        _hierarchicalSecondaryClusters = new java.awt.Choice();
        _hierarchicalLabel = new javax.swing.JLabel();
        _checkboxReportIndexCoefficients = new javax.swing.JCheckBox();
        _mostLikelyClustersHierarchically = new javax.swing.JCheckBox();
        _giniOptimizedClusters = new javax.swing.JCheckBox();
        _reportedSpatialOptionsGroup = new javax.swing.JPanel();
        _restrictReportedClustersCheckBox = new javax.swing.JCheckBox();
        _maxReportedSpatialClusterSizeTextField = new javax.swing.JTextField();
        _reportedPercentOfPopulationLabel = new javax.swing.JLabel();
        _reportedSpatialPopulationFileCheckBox = new javax.swing.JCheckBox();
        _maxReportedSpatialPercentFileTextField = new javax.swing.JTextField();
        _reportedPercentageOfPopFileLabel = new javax.swing.JLabel();
        _reportedSpatialDistanceCheckBox = new javax.swing.JCheckBox();
        _reportedMaxDistanceLabel = new javax.swing.JLabel();
        _maxReportedSpatialRadiusTextField = new javax.swing.JTextField();
        _maxReportedRadiusLabel = new javax.swing.JLabel();
        _otherOutputTab = new javax.swing.JPanel();
        _reportCriticalValuesGroup = new javax.swing.JPanel();
        _reportCriticalValuesCheckBox = new javax.swing.JCheckBox();
        _reportClusterRankGroup = new javax.swing.JPanel();
        _reportClusterRankCheckBox = new javax.swing.JCheckBox();
        _additionalOutputFiles = new javax.swing.JPanel();
        _printAsciiColumnHeaders = new javax.swing.JCheckBox();
        _userDefinedRunTitle = new javax.swing.JPanel();
        _printTitle = new javax.swing.JTextField();
        _cluster_lineline_panel = new javax.swing.JPanel();
        _cluster_lineline_value = new javax.swing.JTextField();
        initCutoffJTextField(_cluster_lineline_value, AppConstants.DEFAULT_RECURRENCE_CUTOFF_CSV_LINELIST, AppConstants.DEFAULT_PVALUE_CUTOFF_CSV_LINELIST);
        _cluster_lineline_label = new javax.swing.JLabel();
        _cluster_lineline_prelabel = new javax.swing.JLabel();
        _powerEvaluationTab = new javax.swing.JPanel();
        _powerEvaluationsGroup = new javax.swing.JPanel();
        _partOfRegularAnalysis = new javax.swing.JRadioButton();
        _powerEvaluationWithCaseFile = new javax.swing.JRadioButton();
        _powerEvaluationWithSpecifiedCases = new javax.swing.JRadioButton();
        _totalPowerCases = new javax.swing.JTextField();
        _powerEvaluationWithSpecifiedCasesLabel = new javax.swing.JLabel();
        _alternativeHypothesisFilenameLabel = new javax.swing.JLabel();
        _alternativeHypothesisFilename = new javax.swing.JTextField();
        _alternativeHypothesisFilenameButton = new javax.swing.JButton();
        _numberPowerReplicationsLabel = new javax.swing.JLabel();
        _numberPowerReplications = new javax.swing.JTextField();
        _criticalValuesTypeLabel = new javax.swing.JLabel();
        _criticalValuesMonteCarlo = new javax.swing.JRadioButton();
        _criticalValuesGumbel = new javax.swing.JRadioButton();
        _criticalValuesReplicationsLabel = new javax.swing.JLabel();
        _powerEstimationTypeLabel = new javax.swing.JLabel();
        _powerEstimationMonteCarlo = new javax.swing.JRadioButton();
        _powerEstimationGumbel = new javax.swing.JRadioButton();
        _performPowerEvalautions = new javax.swing.JCheckBox();
        _temporalOutputTab = new javax.swing.JPanel();
        _graphOutputGroup = new javax.swing.JPanel();
        _reportTemporalGraph = new javax.swing.JCheckBox();
        _temporalGraphMostLikely = new javax.swing.JRadioButton();
        _temporalGraphMostLikelyX = new javax.swing.JRadioButton();
        _numMostLikelyClustersGraph = new javax.swing.JTextField();
        _numMostLikelyClustersGraphLabel = new javax.swing.JLabel();
        _temporalGraphSignificant = new javax.swing.JRadioButton();
        _temporalGraphPvalueCutoff = new javax.swing.JTextField();
        _miscellaneous_analysis_tab = new javax.swing.JPanel();
        _oliveiras_f_group = new javax.swing.JPanel();
        _calculate_oliveiras_f = new javax.swing.JCheckBox();
        _number_oliveira_data_sets_label = new javax.swing.JLabel();
        _number_oliveira_data_sets = new javax.swing.JTextField();
        _prospective_frequency_group = new javax.swing.JPanel();
        _label_prospective_frequency = new javax.swing.JLabel();
        _prospective_frequency = new java.awt.Choice();
        _cluster_restrictions_tab = new javax.swing.JPanel();
        _limit_clusters_risk_group = new javax.swing.JPanel();
        _limit_low_clusters = new javax.swing.JCheckBox();
        _limit_low_clusters_value = new javax.swing.JTextField();
        _limit_high_clusters = new javax.swing.JCheckBox();
        _limit_high_clusters_value = new javax.swing.JTextField();
        _minimum_clusters_group = new javax.swing.JPanel();
        _min_cases_label = new javax.swing.JLabel();
        _minimum_number_cases_cluster = new javax.swing.JTextField();
        _min_cases_label2 = new javax.swing.JLabel();
        _drilldown_tab = new javax.swing.JPanel();
        _drilldown_restrictions_group = new javax.swing.JPanel();
        _mainAnalysisDrilldown = new javax.swing.JCheckBox();
        _purelySpatialDrilldown = new javax.swing.JCheckBox();
        _drilldown_restriction_locations_label = new javax.swing.JLabel();
        _drilldown_restriction_locations = new javax.swing.JTextField();
        _drilldown_restriction_cases_label = new javax.swing.JLabel();
        _drilldown_restriction_cases = new javax.swing.JTextField();
        _drilldown_restriction_cutoff_label = new javax.swing.JLabel();
        _drilldown_restriction_cutoff = new javax.swing.JTextField();
        _drilldown_restrictions = new javax.swing.JLabel();
        _drilldown_restriction_dow = new javax.swing.JCheckBox();
        _network_tab = new javax.swing.JPanel();
        _network_group = new javax.swing.JPanel();
        _locations_network = new javax.swing.JCheckBox();
        _network_file_label = new javax.swing.JLabel();
        _network_filename = new javax.swing.JTextField();
        _browse_network_filename = new javax.swing.JButton();
        _notificatons_tab = new javax.swing.JPanel();
        _panel_email_notifications = new javax.swing.JPanel();
        _always_sendmail = new javax.swing.JCheckBox();
        _always_sendmail.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                enableEmailAlerts();
            }
        });
        _always_email_recipients = new javax.swing.JTextField();
        _cutoff_email_recipients = new javax.swing.JTextField();
        _attach_main_results_email = new javax.swing.JCheckBox();
        _cutoff_email = new javax.swing.JCheckBox();
        _cutoff_email.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                enableEmailAlerts();
            }
        });
        _report_main_results_email = new javax.swing.JCheckBox();
        _custom_email_subject_label = new javax.swing.JLabel();
        _custom_email_subject = new javax.swing.JTextField();
        _custom_email_tags = new javax.swing.JButton();
        _custom_email_tags.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                JTextPane textPane = new JTextPane();
                textPane.setEditable(false);
                //textPane.setBackground(new java.awt.Color(240, 240, 240));
                textPane.setContentType("text/html"); // NOI18N
                textPane.setText("""
                    <html><head><style>body {font-size;15px;} div.header {margin-top:5px;font-size;15px;color:#2C3E50;font-weight:bold;} p.tag-row {margin: 0 0 0 0;} span.tag {color: ##333;font-weight: bold;} span.info {color:#34495e;font-style:normal;}</style></head><body>
                    <p style="margin-top:0px;font-weight:bold;color:#1B2631;">Use these tags in your custom message and they will be substituted at the time the email is sent.</p>
                    <div class="header">Output Variables:</div>
                    <p class="tag-row"><span class="tag">&lt;date&gt;</span><span class="info"> - Today's date.</span></p>
                    <p class="tag-row"><span class="tag">&lt;results-directory&gt;</span><span class="info"> - The directory to which the analysis result files were written.</span></p>
                    <p class="tag-row"><span class="tag">&lt;results-filename&gt;</span><span class="info"> - The name and path of the main results file.</span></p>
                    <p class="tag-row"><span class="tag">&lt;results-name&gt;</span><span class="info"> - The name of the main results file, without an extension.</span></p>
                    <div class="header">Output Paragraphs:</div>
                    <p class="tag-row"><span class="tag">&lt;summary-paragraph&gt;</span><span class="info"> - Summary information about the most likely cluster and potentially secondary clusters.</span></p>
                    <p class="tag-row"><span class="tag">&lt;linelist-paragraph&gt;</span><span class="info"> - For prospective analyses, summary information about new and ongoing signals when reading line list data with individual IDs.</span></p>
                    <p class="tag-row"><span class="tag">&lt;location-paragraph&gt;</span><span class="info"> - Location of the main results file and the directory to which other result files were written.</span></p>
                    <p class="tag-row"><span class="tag">&lt;footer-paragraph&gt;</span><span class="info"> - Message indicating that this email is part of an automated process.</span></p>
                    </body></html>
                    """);
                    JOptionPane.showMessageDialog(null, new JScrollPane(textPane, JScrollPane.VERTICAL_SCROLLBAR_NEVER, JScrollPane.HORIZONTAL_SCROLLBAR_NEVER), "Email Message Tags", JOptionPane.PLAIN_MESSAGE);
                }
            });
            jScrollPane3 = new javax.swing.JScrollPane();
            _custom_email_message = new javax.swing.JTextArea();
            _custom_email_message_label = new javax.swing.JLabel();
            _create_custom_email_message = new javax.swing.JCheckBox();
            _create_custom_email_message.addItemListener(new java.awt.event.ItemListener() {
                public void itemStateChanged(java.awt.event.ItemEvent e) {
                    enableEmailAlerts();
                }
            });
            _cutoff_value_email = new javax.swing.JTextField();
            _cutoff_email_label = new javax.swing.JLabel();
            _closeButton = new javax.swing.JButton();
            _setDefaultButton = new javax.swing.JButton();

            _multipleSetButtonGroup.add(_multivariateAdjustmentsRadioButton);
            _multipleSetButtonGroup.add(_adjustmentByDataSetsRadioButton);

            _studyPeriodCheckButtonGroup.add(_strictStudyPeriodCheckRadioButton);
            _studyPeriodCheckButtonGroup.add(_relaxedStudyPeriodCheckRadioButton);

            _geographicalCoordinatesCheckButtonGroup.add(_strictCoordinatesRadioButton);
            _geographicalCoordinatesCheckButtonGroup.add(_relaxedCoordinatesRadioButton);

            _multipleSetsSpatialCoordinatesButtonGroup.add(_onePerLocationIdRadioButton);
            _multipleSetsSpatialCoordinatesButtonGroup.add(_atLeastOneRadioButton);
            _multipleSetsSpatialCoordinatesButtonGroup.add(_allLocationsRadioButton);

            _spatialWindowShapeButtonGroup.add(_circularRadioButton);
            _spatialWindowShapeButtonGroup.add(_ellipticRadioButton);

            _temporalOptionsButtonGroup.add(_percentageTemporalRadioButton);
            _temporalOptionsButtonGroup.add(_timeTemporalRadioButton);

            _spatialAdjustmentsButtonGroup.add(_spatialAdjustmentsNone);
            _spatialAdjustmentsButtonGroup.add(_spatialAdjustmentsNonparametric);

            _temporalTrendAdjButtonGroup.add(_temporalTrendAdjNone);
            _temporalTrendAdjButtonGroup.add(_temporalTrendAdjNonparametric);
            _temporalTrendAdjButtonGroup.add(_temporalTrendAdjLogLinear);
            _temporalTrendAdjButtonGroup.add(_temporalTrendAdjLogLinearCalc);
            _temporalTrendAdjButtonGroup.add(_temporalTrendAdjQuadCalc);

            _pValueButtonGroup.add(_radioDefaultPValues);
            _pValueButtonGroup.add(_radioStandardPValues);
            _pValueButtonGroup.add(_radioEarlyTerminationPValues);
            _pValueButtonGroup.add(_radioGumbelPValues);

            popupMenu1.setLabel("popupMenu1");

            setDefaultCloseOperation(javax.swing.WindowConstants.HIDE_ON_CLOSE);
            setResizable(true);

            _additionalDataSetsGroup.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createEtchedBorder(), "Additional Input Data Sets"));

            _inputDataSetsList.setSelectionMode(javax.swing.ListSelectionModel.SINGLE_SELECTION);
            _inputDataSetsList.addListSelectionListener(new javax.swing.event.ListSelectionListener() {
                public void valueChanged(javax.swing.event.ListSelectionEvent e) {
                    enableInputFileEdits();
                    if (_dataSetsListModel.getSize() > 0 && _inputDataSetsList.getSelectedIndex() != -1) {
                        _caseFileTextField.setText(AdvancedParameterSettingsFrame.this._caseFilenames.get(_inputDataSetsList.getSelectedIndex()));
                        _controlFileTextField.setText(AdvancedParameterSettingsFrame.this._controlFilenames.get(_inputDataSetsList.getSelectedIndex()));
                        _populationFileTextField.setText(AdvancedParameterSettingsFrame.this._populationFilenames.get(_inputDataSetsList.getSelectedIndex()));
                    }
                    enableSetDefaultsButton();
                }
            });
            jScrollPane1.setViewportView(_inputDataSetsList);
            _inputDataSetsList.getAccessibleContext().setAccessibleName("Additional data sets selection");

            _addDataSetButton.setText("Add"); // NOI18N
            _addDataSetButton.addActionListener(new java.awt.event.ActionListener() {
                public void actionPerformed(java.awt.event.ActionEvent e) {
                    _dataSetsListModel.addElement("Data Set " + Integer.toString(_inputDataSetsList.getModel().getSize() + 2));

                    // enable and clear the edit boxes
                    _caseFilenames.add("");
                    _controlFilenames.add("");
                    _populationFilenames.add("");
                    _inputDataSetsList.setSelectedIndex(_dataSetsListModel.getSize() - 1);
                    _inputDataSetsList.ensureIndexIsVisible(_dataSetsListModel.getSize() - 1);
                    _caseFileTextField.setText("");
                    _controlFileTextField.setText("");
                    _populationFileTextField.setText("");
                    _caseFileTextField.requestFocusInWindow();
                    enableSettingsForAnalysisModelCombination();
                }
            });

            _removeDataSetButton.setText("Remove"); // NOI18N
            _removeDataSetButton.addActionListener(new java.awt.event.ActionListener() {
                public void actionPerformed(java.awt.event.ActionEvent e) {
                    int iDeleteIndex = _inputDataSetsList.getSelectedIndex();

                    ArrayList<InputSourceSettings.InputFileType> filetypes = new ArrayList<>();
                    filetypes.add(InputSourceSettings.InputFileType.Case);
                    filetypes.add(InputSourceSettings.InputFileType.Control);
                    filetypes.add(InputSourceSettings.InputFileType.Population);
                    for (InputSourceSettings.InputFileType type : filetypes) {
                        String key = type.toString() + Integer.toString(iDeleteIndex + 2);
                        // remove input source settings for file type
                        if (_settings_window._input_source_map.containsKey(key)) {
                            _settings_window._input_source_map.remove(key);
                        }
                        // update input source settings keys for greater indexes
                        for (int i=iDeleteIndex+1; i < _inputDataSetsList.getModel().getSize() ;i++) {
                            key = type.toString() + Integer.toString(i + 2);
                            if (_settings_window._input_source_map.containsKey(key)) {
                                String newKey = type.toString() + Integer.toString(i - 1 + 2);
                                _settings_window._input_source_map.put(newKey, _settings_window._input_source_map.remove(key));
                            }
                        }
                    }

                    // update remaining list box names
                    for (int i=iDeleteIndex+1; i < _inputDataSetsList.getModel().getSize() ;i++) {
                        String s = (String)_dataSetsListModel.getElementAt(i);
                        s =	"Data Set " + Integer.toString(i + 1);
                        _dataSetsListModel.setElementAt(s, i);
                    }
                    // remove list box name
                    _dataSetsListModel.remove(iDeleteIndex);

                    // remove files
                    _caseFileTextField.setText("");
                    _caseFilenames.remove(iDeleteIndex);
                    _controlFileTextField.setText("");
                    _controlFilenames.remove(iDeleteIndex);
                    _populationFileTextField.setText("");
                    _populationFilenames.remove(iDeleteIndex);
                    if (_inputDataSetsList.getModel().getSize() > 0) {
                        _inputDataSetsList.setSelectedIndex(0);
                        _inputDataSetsList.ensureIndexIsVisible(0);
                    }
                    enableSettingsForAnalysisModelCombination();
                }
            });

            javax.swing.GroupLayout _dataSetsGroupLayout = new javax.swing.GroupLayout(_dataSetsGroup);
            _dataSetsGroup.setLayout(_dataSetsGroupLayout);
            _dataSetsGroupLayout.setHorizontalGroup(
                _dataSetsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addComponent(jScrollPane1, javax.swing.GroupLayout.DEFAULT_SIZE, 140, Short.MAX_VALUE)
                .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, _dataSetsGroupLayout.createSequentialGroup()
                    .addComponent(_addDataSetButton, javax.swing.GroupLayout.PREFERRED_SIZE, 63, Short.MAX_VALUE)
                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                    .addComponent(_removeDataSetButton))
            );
            _dataSetsGroupLayout.setVerticalGroup(
                _dataSetsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_dataSetsGroupLayout.createSequentialGroup()
                    .addComponent(jScrollPane1, javax.swing.GroupLayout.PREFERRED_SIZE, 100, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                    .addGroup(_dataSetsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                        .addComponent(_removeDataSetButton)
                        .addComponent(_addDataSetButton))
                    .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
            );

            _addDataSetButton.getAccessibleContext().setAccessibleName("Add data set");
            _removeDataSetButton.getAccessibleContext().setAccessibleName("Remove selected data set");

            _caseFileLabel.setLabelFor(_caseFileTextField);
            _caseFileLabel.setText("Case File:"); // NOI18N

            _caseFileTextField.getDocument().addDocumentListener(new DocumentListener() {
                public void changedUpdate(DocumentEvent e) {
                }
                public void removeUpdate(DocumentEvent e) {
                    if (_inputDataSetsList.getSelectedIndex() != -1)
                    _caseFilenames.set(_inputDataSetsList.getSelectedIndex(), _caseFileTextField.getText());
                }
                public void insertUpdate(DocumentEvent e) {
                    if (_inputDataSetsList.getSelectedIndex() != -1)
                    _caseFilenames.set(_inputDataSetsList.getSelectedIndex(), _caseFileTextField.getText());
                }
            });

            _caseFileBrowseButton.setText("..."); // NOI18N
            _caseFileBrowseButton.setToolTipText("Open Case File Import Wizard"); // NOI18N
            _caseFileBrowseButton.addActionListener(new java.awt.event.ActionListener() {
                public void actionPerformed(java.awt.event.ActionEvent e) {
                    String key = InputSourceSettings.InputFileType.Case.toString() + Integer.toString(_inputDataSetsList.getSelectedIndex() + 2);
                    if (!_settings_window._input_source_map.containsKey(key)) {
                        _settings_window._input_source_map.put(key, new InputSourceSettings(InputSourceSettings.InputFileType.Case));
                    }
                    InputSourceSettings inputSourceSettings = (InputSourceSettings)_settings_window._input_source_map.get(key);
                    inputSourceSettings.setDataSetIndex(_inputDataSetsList.getSelectedIndex() + 2);
                    // invoke the FileSelectionDialog to guide user through process of selecting the source file.
                    FileSelectionDialog selectionDialog = new FileSelectionDialog(SaTScanApplication.getInstance(), inputSourceSettings.getInputFileType(), SaTScanApplication.getInstance().lastBrowseDirectory);
                    selectionDialog.browse_inputsource(_caseFileTextField, inputSourceSettings, _settings_window);
                }
            });

            _controlFileLabel.setLabelFor(_controlFileTextField);
            _controlFileLabel.setText("Control File:"); // NOI18N

            _controlFileTextField.getDocument().addDocumentListener(new DocumentListener() {
                public void changedUpdate(DocumentEvent e) {
                }
                public void removeUpdate(DocumentEvent e) {
                    if (_inputDataSetsList.getSelectedIndex() != -1)
                    _controlFilenames.set(_inputDataSetsList.getSelectedIndex(), _controlFileTextField.getText());
                }
                public void insertUpdate(DocumentEvent e) {
                    if (_inputDataSetsList.getSelectedIndex() != -1)
                    _controlFilenames.set(_inputDataSetsList.getSelectedIndex(), _controlFileTextField.getText());
                }
            });

            _controlFileBrowseButton.setText("..."); // NOI18N
            _controlFileBrowseButton.setToolTipText("Open Control File Import Wizard"); // NOI18N
            _controlFileBrowseButton.addActionListener(new java.awt.event.ActionListener() {
                public void actionPerformed(java.awt.event.ActionEvent e) {
                    String key = InputSourceSettings.InputFileType.Control.toString() + Integer.toString(_inputDataSetsList.getSelectedIndex() + 2);
                    if (!_settings_window._input_source_map.containsKey(key)) {
                        _settings_window._input_source_map.put(key, new InputSourceSettings(InputSourceSettings.InputFileType.Control));
                    }
                    InputSourceSettings inputSourceSettings = (InputSourceSettings)_settings_window._input_source_map.get(key);
                    inputSourceSettings.setDataSetIndex(_inputDataSetsList.getSelectedIndex() + 2);
                    // invoke the FileSelectionDialog to guide user through process of selecting the source file.
                    FileSelectionDialog selectionDialog = new FileSelectionDialog(SaTScanApplication.getInstance(), inputSourceSettings.getInputFileType(), SaTScanApplication.getInstance().lastBrowseDirectory);
                    selectionDialog.browse_inputsource(_controlFileTextField, inputSourceSettings, _settings_window);
                }
            });

            _populationFileLabel.setLabelFor(_populationFileTextField);
            _populationFileLabel.setText("Population File:"); // NOI18N

            _populationFileTextField.getDocument().addDocumentListener(new DocumentListener() {
                public void changedUpdate(DocumentEvent e) {
                }
                public void removeUpdate(DocumentEvent e) {
                    if (_inputDataSetsList.getSelectedIndex() != -1)
                    _populationFilenames.set(_inputDataSetsList.getSelectedIndex(), _populationFileTextField.getText());
                }
                public void insertUpdate(DocumentEvent e) {
                    if (_inputDataSetsList.getSelectedIndex() != -1)
                    _populationFilenames.set(_inputDataSetsList.getSelectedIndex(), _populationFileTextField.getText());
                }
            });

            _populationFileBrowseButton.setText("..."); // NOI18N
            _populationFileBrowseButton.setToolTipText("Open Population File Import Wizard"); // NOI18N
            _populationFileBrowseButton.addActionListener(new java.awt.event.ActionListener() {
                public void actionPerformed(java.awt.event.ActionEvent e) {
                    String key = InputSourceSettings.InputFileType.Population.toString() + Integer.toString(_inputDataSetsList.getSelectedIndex() + 2);
                    if (!_settings_window._input_source_map.containsKey(key)) {
                        _settings_window._input_source_map.put(key, new InputSourceSettings(InputSourceSettings.InputFileType.Population));
                    }
                    InputSourceSettings inputSourceSettings = (InputSourceSettings)_settings_window._input_source_map.get(key);
                    inputSourceSettings.setDataSetIndex(_inputDataSetsList.getSelectedIndex() + 2);
                    // invoke the FileSelectionDialog to guide user through process of selecting the source file.
                    FileSelectionDialog selectionDialog = new FileSelectionDialog(SaTScanApplication.getInstance(), inputSourceSettings.getInputFileType(), SaTScanApplication.getInstance().lastBrowseDirectory);
                    selectionDialog.browse_inputsource(_populationFileTextField, inputSourceSettings, _settings_window);
                }
            });

            javax.swing.GroupLayout _fileInputGroupLayout = new javax.swing.GroupLayout(_fileInputGroup);
            _fileInputGroup.setLayout(_fileInputGroupLayout);
            _fileInputGroupLayout.setHorizontalGroup(
                _fileInputGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_fileInputGroupLayout.createSequentialGroup()
                    .addContainerGap()
                    .addGroup(_fileInputGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                        .addGroup(_fileInputGroupLayout.createSequentialGroup()
                            .addComponent(_populationFileTextField)
                            .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                            .addComponent(_populationFileBrowseButton, javax.swing.GroupLayout.PREFERRED_SIZE, 25, javax.swing.GroupLayout.PREFERRED_SIZE))
                        .addGroup(_fileInputGroupLayout.createSequentialGroup()
                            .addGroup(_fileInputGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                                .addComponent(_caseFileLabel)
                                .addComponent(_controlFileLabel)
                                .addComponent(_populationFileLabel))
                            .addGap(0, 389, Short.MAX_VALUE))
                        .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, _fileInputGroupLayout.createSequentialGroup()
                            .addGroup(_fileInputGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING)
                                .addComponent(_controlFileTextField)
                                .addComponent(_caseFileTextField))
                            .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                            .addGroup(_fileInputGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                                .addComponent(_caseFileBrowseButton, javax.swing.GroupLayout.Alignment.TRAILING, javax.swing.GroupLayout.PREFERRED_SIZE, 25, javax.swing.GroupLayout.PREFERRED_SIZE)
                                .addComponent(_controlFileBrowseButton, javax.swing.GroupLayout.Alignment.TRAILING, javax.swing.GroupLayout.PREFERRED_SIZE, 25, javax.swing.GroupLayout.PREFERRED_SIZE))))
                    .addContainerGap())
            );
            _fileInputGroupLayout.setVerticalGroup(
                _fileInputGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_fileInputGroupLayout.createSequentialGroup()
                    .addComponent(_caseFileLabel)
                    .addGap(0, 0, 0)
                    .addGroup(_fileInputGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                        .addComponent(_caseFileBrowseButton)
                        .addComponent(_caseFileTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                    .addComponent(_controlFileLabel)
                    .addGap(0, 0, 0)
                    .addGroup(_fileInputGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                        .addComponent(_controlFileBrowseButton)
                        .addComponent(_controlFileTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                    .addComponent(_populationFileLabel)
                    .addGap(0, 0, 0)
                    .addGroup(_fileInputGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                        .addComponent(_populationFileBrowseButton)
                        .addComponent(_populationFileTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                    .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
            );

            _caseFileTextField.getAccessibleContext().setAccessibleName("Case File");
            _caseFileBrowseButton.getAccessibleContext().setAccessibleName("");
            _controlFileLabel.getAccessibleContext().setAccessibleName("Control File");
            _controlFileBrowseButton.getAccessibleContext().setAccessibleName("");
            _populationFileLabel.getAccessibleContext().setAccessibleName("Population File");
            _populationFileTextField.getAccessibleContext().setAccessibleName("Population File");
            _populationFileBrowseButton.getAccessibleContext().setAccessibleName("");

            _multipleDataSetPurposeLabel.setText("Purpose of Multiple Data Sets:"); // NOI18N

            _multivariateAdjustmentsRadioButton.setSelected(true);
            _multivariateAdjustmentsRadioButton.setText("Multivariate Analysis (clusters in one or more data sets)"); // NOI18N
            _multivariateAdjustmentsRadioButton.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
            _multivariateAdjustmentsRadioButton.setMargin(new java.awt.Insets(0, 0, 0, 0));
            _multivariateAdjustmentsRadioButton.addItemListener(new java.awt.event.ItemListener() {
                public void itemStateChanged(java.awt.event.ItemEvent e) {
                    if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED) {
                        enableSetDefaultsButton();
                        enableClustersReportedGroup();
                    }
                }
            });

            _adjustmentByDataSetsRadioButton.setText("Adjustment (clusters in all data sets simultaneously)"); // NOI18N
            _adjustmentByDataSetsRadioButton.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
            _adjustmentByDataSetsRadioButton.setMargin(new java.awt.Insets(0, 0, 0, 0));
            _adjustmentByDataSetsRadioButton.addItemListener(new java.awt.event.ItemListener() {
                public void itemStateChanged(java.awt.event.ItemEvent e) {
                    if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED) {
                        enableSetDefaultsButton();
                        enableClustersReportedGroup();
                    }
                }
            });

            javax.swing.GroupLayout _multipleSetPurposeGroupLayout = new javax.swing.GroupLayout(_multipleSetPurposeGroup);
            _multipleSetPurposeGroup.setLayout(_multipleSetPurposeGroupLayout);
            _multipleSetPurposeGroupLayout.setHorizontalGroup(
                _multipleSetPurposeGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_multipleSetPurposeGroupLayout.createSequentialGroup()
                    .addContainerGap()
                    .addGroup(_multipleSetPurposeGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                        .addComponent(_multivariateAdjustmentsRadioButton, javax.swing.GroupLayout.Alignment.TRAILING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                        .addGroup(_multipleSetPurposeGroupLayout.createSequentialGroup()
                            .addComponent(_multipleDataSetPurposeLabel)
                            .addGap(0, 0, Short.MAX_VALUE))
                        .addComponent(_adjustmentByDataSetsRadioButton, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                    .addContainerGap())
            );
            _multipleSetPurposeGroupLayout.setVerticalGroup(
                _multipleSetPurposeGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_multipleSetPurposeGroupLayout.createSequentialGroup()
                    .addGap(0, 0, 0)
                    .addComponent(_multipleDataSetPurposeLabel)
                    .addGap(10, 10, 10)
                    .addComponent(_multivariateAdjustmentsRadioButton)
                    .addGap(10, 10, 10)
                    .addComponent(_adjustmentByDataSetsRadioButton)
                    .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
            );

            javax.swing.GroupLayout _additionalDataSetsGroupLayout = new javax.swing.GroupLayout(_additionalDataSetsGroup);
            _additionalDataSetsGroup.setLayout(_additionalDataSetsGroupLayout);
            _additionalDataSetsGroupLayout.setHorizontalGroup(
                _additionalDataSetsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_additionalDataSetsGroupLayout.createSequentialGroup()
                    .addContainerGap()
                    .addGroup(_additionalDataSetsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                        .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, _additionalDataSetsGroupLayout.createSequentialGroup()
                            .addComponent(_dataSetsGroup, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                            .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                            .addComponent(_fileInputGroup, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                        .addComponent(_multipleSetPurposeGroup, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                    .addGap(2, 2, 2))
            );
            _additionalDataSetsGroupLayout.setVerticalGroup(
                _additionalDataSetsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_additionalDataSetsGroupLayout.createSequentialGroup()
                    .addGroup(_additionalDataSetsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                        .addComponent(_dataSetsGroup, 0, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                        .addComponent(_fileInputGroup, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                    .addComponent(_multipleSetPurposeGroup, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addContainerGap())
            );

            javax.swing.GroupLayout _multipleDataSetsTabLayout = new javax.swing.GroupLayout(_multipleDataSetsTab);
            _multipleDataSetsTab.setLayout(_multipleDataSetsTabLayout);
            _multipleDataSetsTabLayout.setHorizontalGroup(
                _multipleDataSetsTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_multipleDataSetsTabLayout.createSequentialGroup()
                    .addContainerGap()
                    .addComponent(_additionalDataSetsGroup, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addContainerGap())
            );
            _multipleDataSetsTabLayout.setVerticalGroup(
                _multipleDataSetsTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_multipleDataSetsTabLayout.createSequentialGroup()
                    .addContainerGap()
                    .addComponent(_additionalDataSetsGroup, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addContainerGap(220, Short.MAX_VALUE))
            );

            jTabbedPane1.addTab("Multiple Data Sets", _multipleDataSetsTab);
            _multipleDataSetsTab.getAccessibleContext().setAccessibleName("Multiple Data Sets tab");

            _studyPeriodCheckGroup.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createEtchedBorder(), "Temporal Data Check"));

            _strictStudyPeriodCheckRadioButton.setSelected(true);
            _strictStudyPeriodCheckRadioButton.setText("Check to ensure that all cases and controls are within the specified temporal study period."); // NOI18N
            _strictStudyPeriodCheckRadioButton.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
            _strictStudyPeriodCheckRadioButton.setMargin(new java.awt.Insets(0, 0, 0, 0));
            _strictStudyPeriodCheckRadioButton.addItemListener(new java.awt.event.ItemListener() {
                public void itemStateChanged(java.awt.event.ItemEvent e) {
                    if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED) enableSetDefaultsButton();
                }
            });

            _relaxedStudyPeriodCheckRadioButton.setText("Ignore cases and controls that are outside the specified temporal study period."); // NOI18N
            _relaxedStudyPeriodCheckRadioButton.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
            _relaxedStudyPeriodCheckRadioButton.setMargin(new java.awt.Insets(0, 0, 0, 0));
            _relaxedStudyPeriodCheckRadioButton.addItemListener(new java.awt.event.ItemListener() {
                public void itemStateChanged(java.awt.event.ItemEvent e) {
                    if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED) enableSetDefaultsButton();
                }
            });

            javax.swing.GroupLayout _studyPeriodCheckGroupLayout = new javax.swing.GroupLayout(_studyPeriodCheckGroup);
            _studyPeriodCheckGroup.setLayout(_studyPeriodCheckGroupLayout);
            _studyPeriodCheckGroupLayout.setHorizontalGroup(
                _studyPeriodCheckGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_studyPeriodCheckGroupLayout.createSequentialGroup()
                    .addContainerGap()
                    .addGroup(_studyPeriodCheckGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                        .addComponent(_strictStudyPeriodCheckRadioButton, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                        .addComponent(_relaxedStudyPeriodCheckRadioButton, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                    .addContainerGap())
            );
            _studyPeriodCheckGroupLayout.setVerticalGroup(
                _studyPeriodCheckGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_studyPeriodCheckGroupLayout.createSequentialGroup()
                    .addContainerGap()
                    .addComponent(_strictStudyPeriodCheckRadioButton)
                    .addGap(10, 10, 10)
                    .addComponent(_relaxedStudyPeriodCheckRadioButton)
                    .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
            );

            _geographicalCoordinatesCheckGroup.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createEtchedBorder(), "Geographical Data Check"));

            _strictCoordinatesRadioButton.setSelected(true);
            _strictCoordinatesRadioButton.setText("Check to ensure that all observations (cases, controls and populations) are within the specified"); // NOI18N
            _strictCoordinatesRadioButton.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
            _strictCoordinatesRadioButton.setMargin(new java.awt.Insets(0, 0, 0, 0));
            _strictCoordinatesRadioButton.addItemListener(new java.awt.event.ItemListener() {
                public void itemStateChanged(java.awt.event.ItemEvent e) {
                    if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED) enableSetDefaultsButton();
                }
            });

            _relaxedCoordinatesRadioButton.setText("Ignore observations that are outside the specified geographical area."); // NOI18N
            _relaxedCoordinatesRadioButton.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
            _relaxedCoordinatesRadioButton.setMargin(new java.awt.Insets(0, 0, 0, 0));
            _relaxedCoordinatesRadioButton.addItemListener(new java.awt.event.ItemListener() {
                public void itemStateChanged(java.awt.event.ItemEvent e) {
                    if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED) enableSetDefaultsButton();
                }
            });

            _strictCoordinatesLabel.setText(" geographical area."); // NOI18N

            javax.swing.GroupLayout _geographicalCoordinatesCheckGroupLayout = new javax.swing.GroupLayout(_geographicalCoordinatesCheckGroup);
            _geographicalCoordinatesCheckGroup.setLayout(_geographicalCoordinatesCheckGroupLayout);
            _geographicalCoordinatesCheckGroupLayout.setHorizontalGroup(
                _geographicalCoordinatesCheckGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_geographicalCoordinatesCheckGroupLayout.createSequentialGroup()
                    .addContainerGap()
                    .addGroup(_geographicalCoordinatesCheckGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                        .addGroup(_geographicalCoordinatesCheckGroupLayout.createSequentialGroup()
                            .addGap(17, 17, 17)
                            .addComponent(_strictCoordinatesLabel, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                        .addComponent(_relaxedCoordinatesRadioButton, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                        .addComponent(_strictCoordinatesRadioButton, javax.swing.GroupLayout.DEFAULT_SIZE, 620, Short.MAX_VALUE))
                    .addContainerGap())
            );
            _geographicalCoordinatesCheckGroupLayout.setVerticalGroup(
                _geographicalCoordinatesCheckGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_geographicalCoordinatesCheckGroupLayout.createSequentialGroup()
                    .addContainerGap()
                    .addComponent(_strictCoordinatesRadioButton)
                    .addGap(0, 0, 0)
                    .addComponent(_strictCoordinatesLabel)
                    .addGap(10, 10, 10)
                    .addComponent(_relaxedCoordinatesRadioButton)
                    .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
            );

            _strictCoordinatesRadioButton.getAccessibleContext().setAccessibleName("Check to ensure that all observations (cases, controls and populations) are within the specified geographical area.");

            javax.swing.GroupLayout _dataCheckingTabLayout = new javax.swing.GroupLayout(_dataCheckingTab);
            _dataCheckingTab.setLayout(_dataCheckingTabLayout);
            _dataCheckingTabLayout.setHorizontalGroup(
                _dataCheckingTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, _dataCheckingTabLayout.createSequentialGroup()
                    .addContainerGap()
                    .addGroup(_dataCheckingTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING)
                        .addComponent(_geographicalCoordinatesCheckGroup, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                        .addComponent(_studyPeriodCheckGroup, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                    .addContainerGap())
            );
            _dataCheckingTabLayout.setVerticalGroup(
                _dataCheckingTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_dataCheckingTabLayout.createSequentialGroup()
                    .addContainerGap()
                    .addComponent(_studyPeriodCheckGroup, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                    .addComponent(_geographicalCoordinatesCheckGroup, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addContainerGap(283, Short.MAX_VALUE))
            );

            jTabbedPane1.addTab("Data Checking", _dataCheckingTab);
            _dataCheckingTab.getAccessibleContext().setAccessibleName("Data Checking tab");

            _specialNeighborFilesGroup.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createEtchedBorder(), "Non-Euclidean Neighbors"));

            _specifiyNeighborsFileCheckBox.setText("Specify neighbors through a non-Euclidean neighbors file"); // NOI18N
            _specifiyNeighborsFileCheckBox.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
            _specifiyNeighborsFileCheckBox.setMargin(new java.awt.Insets(0, 0, 0, 0));
            _specifiyNeighborsFileCheckBox.addItemListener(new java.awt.event.ItemListener() {
                public void itemStateChanged(java.awt.event.ItemEvent e) {
                    enableSettingsForAnalysisModelCombination();
                    enableSetDefaultsButton();
                }
            });

            _neighborsFileTextField.addActionListener(new java.awt.event.ActionListener() {
                public void actionPerformed(java.awt.event.ActionEvent e) {
                    enableSetDefaultsButton();
                }
            });

            _neighborsFileBrowseButton.setText("..."); // NOI18N
            _neighborsFileBrowseButton.setToolTipText("Open Neighbors File Import Wizard"); // NOI18N
            _neighborsFileBrowseButton.addActionListener(new java.awt.event.ActionListener() {
                public void actionPerformed(java.awt.event.ActionEvent e) {
                    InputSourceSettings inputSourceSettings = new InputSourceSettings(InputSourceSettings.InputFileType.Neighbors);
                    // invoke the FileSelectionDialog to guide user through process of selecting the source file.
                    FileSelectionDialog selectionDialog = new FileSelectionDialog(SaTScanApplication.getInstance(), inputSourceSettings.getInputFileType(), SaTScanApplication.getInstance().lastBrowseDirectory);
                    selectionDialog.browse_inputsource(_neighborsFileTextField, inputSourceSettings, _settings_window);
                }
            });

            _specifiyMetaLocationsFileCheckBox.setText("Specify a meta location file"); // NOI18N
            _specifiyMetaLocationsFileCheckBox.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
            _specifiyMetaLocationsFileCheckBox.setMargin(new java.awt.Insets(0, 0, 0, 0));
            _specifiyMetaLocationsFileCheckBox.addItemListener(new java.awt.event.ItemListener() {
                public void itemStateChanged(java.awt.event.ItemEvent e) {
                    enableSettingsForAnalysisModelCombination();
                    enableSetDefaultsButton();
                }
            });

            _metaLocationsFileTextField.addActionListener(new java.awt.event.ActionListener() {
                public void actionPerformed(java.awt.event.ActionEvent e) {
                    enableSetDefaultsButton();
                }
            });

            _metaLocationsFileBrowseButton.setText("..."); // NOI18N
            _metaLocationsFileBrowseButton.setToolTipText("Open Meta Locations File Import Wizard"); // NOI18N
            _metaLocationsFileBrowseButton.addActionListener(new java.awt.event.ActionListener() {
                public void actionPerformed(java.awt.event.ActionEvent e) {
                    InputSourceSettings inputSourceSettings = new InputSourceSettings(InputSourceSettings.InputFileType.MetaLocations);
                    // invoke the FileSelectionDialog to guide user through process of selecting the source file.
                    FileSelectionDialog selectionDialog = new FileSelectionDialog(SaTScanApplication.getInstance(), inputSourceSettings.getInputFileType(), SaTScanApplication.getInstance().lastBrowseDirectory);
                    selectionDialog.browse_inputsource(_metaLocationsFileTextField, inputSourceSettings, _settings_window);
                }
            });

            javax.swing.GroupLayout _specialNeighborFilesGroupLayout = new javax.swing.GroupLayout(_specialNeighborFilesGroup);
            _specialNeighborFilesGroup.setLayout(_specialNeighborFilesGroupLayout);
            _specialNeighborFilesGroupLayout.setHorizontalGroup(
                _specialNeighborFilesGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_specialNeighborFilesGroupLayout.createSequentialGroup()
                    .addContainerGap()
                    .addGroup(_specialNeighborFilesGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                        .addComponent(_specifiyNeighborsFileCheckBox, javax.swing.GroupLayout.DEFAULT_SIZE, 620, Short.MAX_VALUE)
                        .addComponent(_specifiyMetaLocationsFileCheckBox, javax.swing.GroupLayout.DEFAULT_SIZE, 620, Short.MAX_VALUE)
                        .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, _specialNeighborFilesGroupLayout.createSequentialGroup()
                            .addGap(17, 17, 17)
                            .addGroup(_specialNeighborFilesGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                                .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, _specialNeighborFilesGroupLayout.createSequentialGroup()
                                    .addComponent(_metaLocationsFileTextField, javax.swing.GroupLayout.DEFAULT_SIZE, 572, Short.MAX_VALUE)
                                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                    .addComponent(_metaLocationsFileBrowseButton, javax.swing.GroupLayout.PREFERRED_SIZE, 25, javax.swing.GroupLayout.PREFERRED_SIZE))
                                .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, _specialNeighborFilesGroupLayout.createSequentialGroup()
                                    .addComponent(_neighborsFileTextField, javax.swing.GroupLayout.DEFAULT_SIZE, 572, Short.MAX_VALUE)
                                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                    .addComponent(_neighborsFileBrowseButton, javax.swing.GroupLayout.PREFERRED_SIZE, 25, javax.swing.GroupLayout.PREFERRED_SIZE)))))
                    .addContainerGap())
            );
            _specialNeighborFilesGroupLayout.setVerticalGroup(
                _specialNeighborFilesGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_specialNeighborFilesGroupLayout.createSequentialGroup()
                    .addContainerGap()
                    .addComponent(_specifiyNeighborsFileCheckBox)
                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                    .addGroup(_specialNeighborFilesGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                        .addComponent(_neighborsFileBrowseButton)
                        .addComponent(_neighborsFileTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                    .addComponent(_specifiyMetaLocationsFileCheckBox)
                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                    .addGroup(_specialNeighborFilesGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                        .addComponent(_metaLocationsFileBrowseButton)
                        .addComponent(_metaLocationsFileTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                    .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
            );

            _neighborsFileTextField.getAccessibleContext().setAccessibleName("non-euclidean neighbors file path");
            _neighborsFileBrowseButton.getAccessibleContext().setAccessibleName("");
            _metaLocationsFileTextField.getAccessibleContext().setAccessibleName("meta locations files path");
            _metaLocationsFileBrowseButton.getAccessibleContext().setAccessibleName("");

            _multipleSetsSpatialCoordinatesGroup.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createEtchedBorder(), "Observations with Multiple Locations"));

            _onePerLocationIdRadioButton.setSelected(true);
            _onePerLocationIdRadioButton.setText("One location per observation."); // NOI18N
            _onePerLocationIdRadioButton.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
            _onePerLocationIdRadioButton.setMargin(new java.awt.Insets(0, 0, 0, 0));
            _onePerLocationIdRadioButton.addItemListener(new java.awt.event.ItemListener() {
                public void itemStateChanged(java.awt.event.ItemEvent e) {
                    if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED) {
                        enableMultipleLocationsGroup(_multipleSetsSpatialCoordinatesGroup.isEnabled());
                        enableSetDefaultsButton();
                    }
                }
            });

            _atLeastOneRadioButton.setText("Include observation if at least one of its locations is in the window."); // NOI18N
            _atLeastOneRadioButton.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
            _atLeastOneRadioButton.setMargin(new java.awt.Insets(0, 0, 0, 0));
            _atLeastOneRadioButton.addItemListener(new java.awt.event.ItemListener() {
                public void itemStateChanged(java.awt.event.ItemEvent e) {
                    if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED) {
                        enableMultipleLocationsGroup(_multipleSetsSpatialCoordinatesGroup.isEnabled());
                        enableSetDefaultsButton();
                    }
                }
            });

            _allLocationsRadioButton.setText("Include observation only if all its locations are in the window."); // NOI18N
            _allLocationsRadioButton.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
            _allLocationsRadioButton.setMargin(new java.awt.Insets(0, 0, 0, 0));
            _allLocationsRadioButton.addItemListener(new java.awt.event.ItemListener() {
                public void itemStateChanged(java.awt.event.ItemEvent e) {
                    if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED) {
                        enableMultipleLocationsGroup(_multipleSetsSpatialCoordinatesGroup.isEnabled());
                        enableSetDefaultsButton();
                    }
                }
            });

            _neighborsFileTextField.addActionListener(new java.awt.event.ActionListener() {
                public void actionPerformed(java.awt.event.ActionEvent e) {
                    enableSetDefaultsButton();
                }
            });

            _multiple_locations_file_browse.setText("..."); // NOI18N
            _multiple_locations_file_browse.setToolTipText("Open Multiple Locations Per Observation File Import Wizard"); // NOI18N
            _multiple_locations_file_browse.addActionListener(new java.awt.event.ActionListener() {
                public void actionPerformed(java.awt.event.ActionEvent e) {
                    String key = InputSourceSettings.InputFileType.Multiple_Locations.toString() + "1";
                    if (!_settings_window._input_source_map.containsKey(key)) {
                        _settings_window._input_source_map.put(key, new InputSourceSettings(InputSourceSettings.InputFileType.Multiple_Locations));
                    }
                    InputSourceSettings inputSourceSettings = (InputSourceSettings)_settings_window._input_source_map.get(key);
                    // invoke the FileSelectionDialog to guide user through process of selecting the source file.
                    FileSelectionDialog selectionDialog = new FileSelectionDialog(SaTScanApplication.getInstance(), inputSourceSettings.getInputFileType(), SaTScanApplication.getInstance().lastBrowseDirectory);
                    selectionDialog.browse_inputsource(_multiple_locations_file, inputSourceSettings, _settings_window);
                }
            });

            _multiple_locations_file_label.setLabelFor(_multiple_locations_file);
            _multiple_locations_file_label.setText("Multiple Locations Per Observation File:");

            javax.swing.GroupLayout _multipleSetsSpatialCoordinatesGroupLayout = new javax.swing.GroupLayout(_multipleSetsSpatialCoordinatesGroup);
            _multipleSetsSpatialCoordinatesGroup.setLayout(_multipleSetsSpatialCoordinatesGroupLayout);
            _multipleSetsSpatialCoordinatesGroupLayout.setHorizontalGroup(
                _multipleSetsSpatialCoordinatesGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_multipleSetsSpatialCoordinatesGroupLayout.createSequentialGroup()
                    .addContainerGap()
                    .addGroup(_multipleSetsSpatialCoordinatesGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                        .addComponent(_atLeastOneRadioButton, javax.swing.GroupLayout.DEFAULT_SIZE, 620, Short.MAX_VALUE)
                        .addComponent(_onePerLocationIdRadioButton, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                        .addComponent(_allLocationsRadioButton, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                        .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, _multipleSetsSpatialCoordinatesGroupLayout.createSequentialGroup()
                            .addGroup(_multipleSetsSpatialCoordinatesGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                                .addComponent(_multiple_locations_file, javax.swing.GroupLayout.DEFAULT_SIZE, 589, Short.MAX_VALUE)
                                .addComponent(_multiple_locations_file_label, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                            .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                            .addComponent(_multiple_locations_file_browse, javax.swing.GroupLayout.PREFERRED_SIZE, 25, javax.swing.GroupLayout.PREFERRED_SIZE)))
                    .addContainerGap())
            );
            _multipleSetsSpatialCoordinatesGroupLayout.setVerticalGroup(
                _multipleSetsSpatialCoordinatesGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_multipleSetsSpatialCoordinatesGroupLayout.createSequentialGroup()
                    .addContainerGap()
                    .addComponent(_onePerLocationIdRadioButton)
                    .addGap(10, 10, 10)
                    .addComponent(_atLeastOneRadioButton)
                    .addGap(10, 10, 10)
                    .addComponent(_allLocationsRadioButton)
                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                    .addComponent(_multiple_locations_file_label)
                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addGroup(_multipleSetsSpatialCoordinatesGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                        .addComponent(_multiple_locations_file_browse)
                        .addComponent(_multiple_locations_file, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                    .addContainerGap())
            );

            _multiple_locations_file_browse.getAccessibleContext().setAccessibleName("");

            javax.swing.GroupLayout _spatialNeighborsTabLayout = new javax.swing.GroupLayout(_spatialNeighborsTab);
            _spatialNeighborsTab.setLayout(_spatialNeighborsTabLayout);
            _spatialNeighborsTabLayout.setHorizontalGroup(
                _spatialNeighborsTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_spatialNeighborsTabLayout.createSequentialGroup()
                    .addContainerGap()
                    .addGroup(_spatialNeighborsTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                        .addComponent(_specialNeighborFilesGroup, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                        .addComponent(_multipleSetsSpatialCoordinatesGroup, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                    .addContainerGap())
            );
            _spatialNeighborsTabLayout.setVerticalGroup(
                _spatialNeighborsTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_spatialNeighborsTabLayout.createSequentialGroup()
                    .addContainerGap()
                    .addComponent(_specialNeighborFilesGroup, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                    .addComponent(_multipleSetsSpatialCoordinatesGroup, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addContainerGap(160, Short.MAX_VALUE))
            );

            jTabbedPane1.addTab("Spatial Neighbors", _spatialNeighborsTab);
            _spatialNeighborsTab.getAccessibleContext().setAccessibleName("Spatial Neighbors tab");

            _spatialOptionsGroup.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createEtchedBorder(), "Maximum Spatial Cluster Size"));

            _maxSpatialClusterSizeTextField.setText("50"); // NOI18N
            _maxSpatialClusterSizeTextField.addKeyListener(new java.awt.event.KeyAdapter() {
                public void keyTyped(java.awt.event.KeyEvent e) {
                    Utils.validatePostiveFloatKeyTyped(_maxSpatialClusterSizeTextField, e, 5);
                }
            });
            _maxSpatialClusterSizeTextField.addFocusListener(new java.awt.event.FocusAdapter() {
                public void focusLost(java.awt.event.FocusEvent e) {
                    while (_maxSpatialClusterSizeTextField.getText().length() == 0 ||
                        Double.parseDouble(_maxSpatialClusterSizeTextField.getText()) == 0 ||
                        Double.parseDouble(_maxSpatialClusterSizeTextField.getText()) > 50.0) {
                        if (undo.canUndo()) undo.undo(); else _maxSpatialClusterSizeTextField.setText("50");
                    }
                    enableSetDefaultsButton();
                }
            });
            _maxSpatialClusterSizeTextField.getDocument().addUndoableEditListener(new UndoableEditListener() {
                public void undoableEditHappened(UndoableEditEvent evt) {
                    undo.addEdit(evt.getEdit());
                }
            });

            _percentageOfPopulationLabel.setLabelFor(_maxSpatialClusterSizeTextField);
            _percentageOfPopulationLabel.setText("percent of the population at risk (<= 50%, default = 50%)"); // NOI18N

            _spatialPopulationFileCheckBox.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
            _spatialPopulationFileCheckBox.setMargin(new java.awt.Insets(0, 0, 0, 0));
            _spatialPopulationFileCheckBox.addItemListener(new java.awt.event.ItemListener() {
                public void itemStateChanged(java.awt.event.ItemEvent e) {
                    enableSpatialOptionsGroup(_spatialOptionsGroup.isEnabled(), _inclPureTempClustCheckBox.isEnabled());
                    enableSetDefaultsButton();
                }
            });

            _maxSpatialPercentFileTextField.setText("50"); // NOI18N
            _maxSpatialPercentFileTextField.addKeyListener(new java.awt.event.KeyAdapter() {
                public void keyTyped(java.awt.event.KeyEvent e) {
                    Utils.validatePostiveFloatKeyTyped(_maxSpatialPercentFileTextField, e, 5);
                }
            });
            _maxSpatialPercentFileTextField.addFocusListener(new java.awt.event.FocusAdapter() {
                public void focusLost(java.awt.event.FocusEvent e) {
                    while (_maxSpatialPercentFileTextField.getText().length() == 0 ||
                        Double.parseDouble(_maxSpatialPercentFileTextField.getText()) == 0 ||
                        Double.parseDouble(_maxSpatialPercentFileTextField.getText()) > 50.0) {
                        if (undo.canUndo()) undo.undo(); else _maxSpatialPercentFileTextField.setText("50");
                    }
                    enableSetDefaultsButton();
                }
            });
            _maxSpatialPercentFileTextField.getDocument().addUndoableEditListener(new UndoableEditListener() {
                public void undoableEditHappened(UndoableEditEvent evt) {
                    undo.addEdit(evt.getEdit());
                }
            });

            _percentageOfPopFileLabel.setLabelFor(_maxSpatialPercentFileTextField);
            _percentageOfPopFileLabel.setText("percent of the population defined in the max circle size file (<= 50%)"); // NOI18N

            _maxCirclePopFileBrowseButton.setText("..."); // NOI18N
            _maxCirclePopFileBrowseButton.setToolTipText("Open Maximum Circle File Import Wizard"); // NOI18N
            _maxCirclePopFileBrowseButton.addActionListener(new java.awt.event.ActionListener() {
                public void actionPerformed(java.awt.event.ActionEvent e) {
                    String key = InputSourceSettings.InputFileType.MaxCirclePopulation.toString() + "1";
                    if (!_settings_window._input_source_map.containsKey(key)) {
                        _settings_window._input_source_map.put(key, new InputSourceSettings(InputSourceSettings.InputFileType.MaxCirclePopulation));
                    }
                    InputSourceSettings inputSourceSettings = (InputSourceSettings)_settings_window._input_source_map.get(key);
                    // invoke the FileSelectionDialog to guide user through process of selecting the source file.
                    FileSelectionDialog selectionDialog = new FileSelectionDialog(SaTScanApplication.getInstance(), inputSourceSettings.getInputFileType(), SaTScanApplication.getInstance().lastBrowseDirectory);
                    selectionDialog.browse_inputsource(_maxCirclePopulationFilenameTextField, inputSourceSettings, _settings_window);
                }
            });

            _spatialDistanceCheckBox.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
            _spatialDistanceCheckBox.setMargin(new java.awt.Insets(0, 0, 0, 0));
            _spatialDistanceCheckBox.addItemListener(new java.awt.event.ItemListener() {
                public void itemStateChanged(java.awt.event.ItemEvent e) {
                    enableSpatialOptionsGroup(_spatialOptionsGroup.isEnabled(), _inclPureTempClustCheckBox.isEnabled());
                    enableSetDefaultsButton();
                }
            });

            _distancePrefixLabel.setText("is an ellipse with a"); // NOI18N

            _maxSpatialRadiusTextField.setText("1"); // NOI18N
            _maxSpatialRadiusTextField.addKeyListener(new java.awt.event.KeyAdapter() {
                public void keyTyped(java.awt.event.KeyEvent e) {
                    Utils.validatePostiveFloatKeyTyped(_maxSpatialRadiusTextField, e, 20);
                }
            });
            _maxSpatialRadiusTextField.addFocusListener(new java.awt.event.FocusAdapter() {
                public void focusLost(java.awt.event.FocusEvent e) {
                    while (_maxSpatialRadiusTextField.getText().length() == 0 || Double.parseDouble(_maxSpatialRadiusTextField.getText()) == 0) {
                        if (undo.canUndo()) undo.undo(); else _maxSpatialRadiusTextField.setText("1");
                    }
                    enableSetDefaultsButton();
                }
            });
            _maxSpatialRadiusTextField.getDocument().addUndoableEditListener(new UndoableEditListener() {
                public void undoableEditHappened(UndoableEditEvent evt) {
                    undo.addEdit(evt.getEdit());
                }
            });

            _maxRadiusLabel.setText("kilometer radius"); // NOI18N

            javax.swing.GroupLayout _spatialOptionsGroupLayout = new javax.swing.GroupLayout(_spatialOptionsGroup);
            _spatialOptionsGroup.setLayout(_spatialOptionsGroupLayout);
            _spatialOptionsGroupLayout.setHorizontalGroup(
                _spatialOptionsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_spatialOptionsGroupLayout.createSequentialGroup()
                    .addGroup(_spatialOptionsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                        .addGroup(_spatialOptionsGroupLayout.createSequentialGroup()
                            .addContainerGap()
                            .addGroup(_spatialOptionsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                                .addGroup(_spatialOptionsGroupLayout.createSequentialGroup()
                                    .addGap(10, 10, 10)
                                    .addComponent(_spatialPopulationFileCheckBox)
                                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                    .addComponent(_maxSpatialPercentFileTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 55, javax.swing.GroupLayout.PREFERRED_SIZE)
                                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                                    .addComponent(_percentageOfPopFileLabel, javax.swing.GroupLayout.DEFAULT_SIZE, 526, Short.MAX_VALUE))
                                .addGroup(_spatialOptionsGroupLayout.createSequentialGroup()
                                    .addComponent(_maxSpatialClusterSizeTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 55, javax.swing.GroupLayout.PREFERRED_SIZE)
                                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                    .addComponent(_percentageOfPopulationLabel, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))))
                        .addGroup(_spatialOptionsGroupLayout.createSequentialGroup()
                            .addGap(19, 19, 19)
                            .addGroup(_spatialOptionsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                                .addGroup(_spatialOptionsGroupLayout.createSequentialGroup()
                                    .addComponent(_spatialDistanceCheckBox)
                                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                    .addComponent(_distancePrefixLabel)
                                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                    .addComponent(_maxSpatialRadiusTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 72, javax.swing.GroupLayout.PREFERRED_SIZE)
                                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                    .addComponent(_maxRadiusLabel, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                                .addGroup(_spatialOptionsGroupLayout.createSequentialGroup()
                                    .addComponent(_maxCirclePopulationFilenameTextField)
                                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                    .addComponent(_maxCirclePopFileBrowseButton, javax.swing.GroupLayout.PREFERRED_SIZE, 25, javax.swing.GroupLayout.PREFERRED_SIZE)))))
                    .addContainerGap())
            );
            _spatialOptionsGroupLayout.setVerticalGroup(
                _spatialOptionsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_spatialOptionsGroupLayout.createSequentialGroup()
                    .addContainerGap()
                    .addGroup(_spatialOptionsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                        .addComponent(_maxSpatialClusterSizeTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addComponent(_percentageOfPopulationLabel))
                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                    .addGroup(_spatialOptionsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                        .addComponent(_percentageOfPopFileLabel)
                        .addGroup(_spatialOptionsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING)
                            .addComponent(_spatialPopulationFileCheckBox)
                            .addComponent(_maxSpatialPercentFileTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)))
                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                    .addGroup(_spatialOptionsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                        .addComponent(_maxCirclePopulationFilenameTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addComponent(_maxCirclePopFileBrowseButton))
                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                    .addGroup(_spatialOptionsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING)
                        .addComponent(_spatialDistanceCheckBox)
                        .addGroup(_spatialOptionsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                            .addComponent(_distancePrefixLabel)
                            .addComponent(_maxSpatialRadiusTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                            .addComponent(_maxRadiusLabel)))
                    .addContainerGap(21, Short.MAX_VALUE))
            );

            _spatialPopulationFileCheckBox.getAccessibleContext().setAccessibleName("selection for spatial maximum by max circle size file");
            _maxCirclePopulationFilenameTextField.getAccessibleContext().setAccessibleName("file path of maximum circle size file");
            _maxCirclePopFileBrowseButton.getAccessibleContext().setAccessibleName("");
            _spatialDistanceCheckBox.getAccessibleContext().setAccessibleName("selection for spatial maximum by distance");
            _maxSpatialRadiusTextField.getAccessibleContext().setAccessibleName("maximum radius by distance");

            _inclPureTempClustCheckBox.setText("Include Purely Temporal Clusters (Spatial Size = 100%)"); // NOI18N
            _inclPureTempClustCheckBox.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
            _inclPureTempClustCheckBox.setMargin(new java.awt.Insets(0, 0, 0, 0));
            _inclPureTempClustCheckBox.addItemListener(new java.awt.event.ItemListener() {
                public void itemStateChanged(java.awt.event.ItemEvent e) {
                    enableSetDefaultsButton();
                }
            });

            _spatialWindowShapeGroup.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createEtchedBorder(), "Spatial Window Shape"));

            _circularRadioButton.setSelected(true);
            _circularRadioButton.setText("Circular"); // NOI18N
            _circularRadioButton.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
            _circularRadioButton.setMargin(new java.awt.Insets(0, 0, 0, 0));
            _circularRadioButton.addItemListener(new java.awt.event.ItemListener() {
                public void itemStateChanged(java.awt.event.ItemEvent e) {
                    if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED) {
                        enableWindowShapeGroup(_spatialWindowShapeGroup.isEnabled());
                        enableSetDefaultsButton();
                    }
                }
            });

            _ellipticRadioButton.setText("Elliptic"); // NOI18N
            _ellipticRadioButton.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
            _ellipticRadioButton.setMargin(new java.awt.Insets(0, 0, 0, 0));
            _ellipticRadioButton.addItemListener(new java.awt.event.ItemListener() {
                public void itemStateChanged(java.awt.event.ItemEvent e) {
                    if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED) {
                        enableWindowShapeGroup(_spatialWindowShapeGroup.isEnabled());
                        enableSetDefaultsButton();
                    }
                }
            });

            _nonCompactnessPenaltyLabel.setLabelFor(_nonCompactnessPenaltyComboBox);
            _nonCompactnessPenaltyLabel.setText("Non-Compactness Penalty:"); // NOI18N

            _nonCompactnessPenaltyComboBox.addItemListener(new java.awt.event.ItemListener() {
                public void itemStateChanged(java.awt.event.ItemEvent e) {
                    enableSetDefaultsButton();
                }
            });
            _nonCompactnessPenaltyComboBox.add("None");
            _nonCompactnessPenaltyComboBox.add("Medium");
            _nonCompactnessPenaltyComboBox.add("Strong");

            javax.swing.GroupLayout _spatialWindowShapeGroupLayout = new javax.swing.GroupLayout(_spatialWindowShapeGroup);
            _spatialWindowShapeGroup.setLayout(_spatialWindowShapeGroupLayout);
            _spatialWindowShapeGroupLayout.setHorizontalGroup(
                _spatialWindowShapeGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_spatialWindowShapeGroupLayout.createSequentialGroup()
                    .addContainerGap()
                    .addGroup(_spatialWindowShapeGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                        .addComponent(_circularRadioButton)
                        .addGroup(_spatialWindowShapeGroupLayout.createSequentialGroup()
                            .addComponent(_ellipticRadioButton)
                            .addGap(86, 86, 86)
                            .addComponent(_nonCompactnessPenaltyLabel)
                            .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                            .addComponent(_nonCompactnessPenaltyComboBox, javax.swing.GroupLayout.PREFERRED_SIZE, 111, javax.swing.GroupLayout.PREFERRED_SIZE)))
                    .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
            );
            _spatialWindowShapeGroupLayout.setVerticalGroup(
                _spatialWindowShapeGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_spatialWindowShapeGroupLayout.createSequentialGroup()
                    .addContainerGap()
                    .addComponent(_circularRadioButton)
                    .addGap(10, 10, 10)
                    .addGroup(_spatialWindowShapeGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                        .addComponent(_ellipticRadioButton)
                        .addComponent(_nonCompactnessPenaltyLabel))
                    .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, _spatialWindowShapeGroupLayout.createSequentialGroup()
                    .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(_nonCompactnessPenaltyComboBox, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addContainerGap())
            );

            _circularRadioButton.getAccessibleContext().setAccessibleName("Circular window shape");
            _ellipticRadioButton.getAccessibleContext().setAccessibleName("Elliptic window shape");
            _nonCompactnessPenaltyComboBox.getAccessibleContext().setAccessibleName("Non-Compactness Penalty");

            _performIsotonicScanCheckBox.setText("Use Isotonic Spatial Scan Statistic"); // NOI18N
            _performIsotonicScanCheckBox.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
            _performIsotonicScanCheckBox.setMargin(new java.awt.Insets(0, 0, 0, 0));
            _performIsotonicScanCheckBox.addItemListener(new java.awt.event.ItemListener() {
                public void itemStateChanged(java.awt.event.ItemEvent e) {
                    enableSetDefaultsButton();
                    enableSettingsForAnalysisModelCombination();
                }
            });

            javax.swing.GroupLayout _spatialWindowTabLayout = new javax.swing.GroupLayout(_spatialWindowTab);
            _spatialWindowTab.setLayout(_spatialWindowTabLayout);
            _spatialWindowTabLayout.setHorizontalGroup(
                _spatialWindowTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_spatialWindowTabLayout.createSequentialGroup()
                    .addContainerGap()
                    .addGroup(_spatialWindowTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                        .addComponent(_spatialWindowShapeGroup, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                        .addComponent(_spatialOptionsGroup, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                        .addComponent(_inclPureTempClustCheckBox, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                        .addComponent(_performIsotonicScanCheckBox, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                    .addContainerGap())
            );
            _spatialWindowTabLayout.setVerticalGroup(
                _spatialWindowTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_spatialWindowTabLayout.createSequentialGroup()
                    .addContainerGap()
                    .addComponent(_spatialOptionsGroup, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                    .addComponent(_inclPureTempClustCheckBox)
                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                    .addComponent(_spatialWindowShapeGroup, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                    .addComponent(_performIsotonicScanCheckBox)
                    .addContainerGap(175, Short.MAX_VALUE))
            );

            jTabbedPane1.addTab("Spatial Window", _spatialWindowTab);
            _spatialWindowTab.getAccessibleContext().setAccessibleName("Spatial Window tab");

            _maxTemporalOptionsGroup.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createEtchedBorder(), "Maximum Temporal Cluster Size"));

            _percentageTemporalRadioButton.setSelected(true);
            _percentageTemporalRadioButton.setText("is"); // NOI18N
            _percentageTemporalRadioButton.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
            _percentageTemporalRadioButton.setMargin(new java.awt.Insets(0, 0, 0, 0));
            _percentageTemporalRadioButton.addItemListener(new java.awt.event.ItemListener() {
                public void itemStateChanged(java.awt.event.ItemEvent e) {
                    if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED) {
                        //cause enabling to be refreshed based upon clicked radio button
                        enableTemporalOptionsGroup(_maxTemporalOptionsGroup.isEnabled(), _includePureSpacClustCheckBox.isEnabled(), _restrictTemporalRangeCheckBox.isEnabled());
                        enableSetDefaultsButton();
                    }
                }
            });

            _timeTemporalRadioButton.setText("is"); // NOI18N
            _timeTemporalRadioButton.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
            _timeTemporalRadioButton.setMargin(new java.awt.Insets(0, 0, 0, 0));
            _timeTemporalRadioButton.addItemListener(new java.awt.event.ItemListener() {
                public void itemStateChanged(java.awt.event.ItemEvent e) {
                    if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED) {
                        //cause enabling to be refreshed based upon clicked radio button
                        enableTemporalOptionsGroup(_maxTemporalOptionsGroup.isEnabled(), _includePureSpacClustCheckBox.isEnabled(), _restrictTemporalRangeCheckBox.isEnabled());
                        enableSetDefaultsButton();
                    }
                }
            });

            _maxTemporalClusterSizeTextField.setText("50"); // NOI18N
            _maxTemporalClusterSizeTextField.addKeyListener(new java.awt.event.KeyAdapter() {
                public void keyTyped(java.awt.event.KeyEvent e) {
                    Utils.validatePostiveFloatKeyTyped(_maxTemporalClusterSizeTextField, e, 5);
                }
            });
            _maxTemporalClusterSizeTextField.addFocusListener(new java.awt.event.FocusAdapter() {
                public void focusLost(java.awt.event.FocusEvent e) {
                    double dMaxValue = getAbsoluteMaximumTemporalSizeForSettings();
                    while (_maxTemporalClusterSizeTextField.getText().length() == 0 ||
                        Double.parseDouble(_maxTemporalClusterSizeTextField.getText()) == 0 ||
                        Double.parseDouble(_maxTemporalClusterSizeTextField.getText()) > dMaxValue) {
                        if (undo.canUndo()) undo.undo(); else _maxTemporalClusterSizeTextField.setText(Double.toString(dMaxValue));
                    }
                    enableSetDefaultsButton();
                }
            });
            _maxTemporalClusterSizeTextField.getDocument().addUndoableEditListener(new UndoableEditListener() {
                public void undoableEditHappened(UndoableEditEvent evt) {
                    undo.addEdit(evt.getEdit());
                }
            });

            _percentageOfStudyPeriodLabel.setLabelFor(_maxTemporalClusterSizeTextField);
            _percentageOfStudyPeriodLabel.setText("percent of the study period (<= 90%, default = 50%)"); // NOI18N

            _maxTemporalClusterSizeUnitsTextField.setText("1"); // NOI18N
            _maxTemporalClusterSizeUnitsTextField.addKeyListener(new java.awt.event.KeyAdapter() {
                public void keyTyped(java.awt.event.KeyEvent e) {
                    Utils.validatePostiveNumericKeyTyped(_maxTemporalClusterSizeUnitsTextField, e, 6);
                }
            });
            _maxTemporalClusterSizeUnitsTextField.addFocusListener(new java.awt.event.FocusAdapter() {
                public void focusLost(java.awt.event.FocusEvent e) {
                    while (_maxTemporalClusterSizeUnitsTextField.getText().length() == 0 || Double.parseDouble(_maxTemporalClusterSizeUnitsTextField.getText()) == 0) {
                        if (undo.canUndo()) undo.undo(); else _maxTemporalClusterSizeUnitsTextField.setText("1");
                    }
                    enableSetDefaultsButton();
                }
            });
            _maxTemporalClusterSizeUnitsTextField.getDocument().addUndoableEditListener(new UndoableEditListener() {
                public void undoableEditHappened(UndoableEditEvent evt) {
                    undo.addEdit(evt.getEdit());
                }
            });

            _maxTemporalTimeUnitsLabel.setLabelFor(_maxTemporalClusterSizeUnitsTextField);
            _maxTemporalTimeUnitsLabel.setText("years (<=90% of the study period)"); // NOI18N

            _includePureSpacClustCheckBox.setText("Include Purely Spatial Clusters (Temporal Size = 100%)"); // NOI18N
            _includePureSpacClustCheckBox.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
            _includePureSpacClustCheckBox.setMargin(new java.awt.Insets(0, 0, 0, 0));
            _includePureSpacClustCheckBox.addItemListener(new java.awt.event.ItemListener() {
                public void itemStateChanged(java.awt.event.ItemEvent e) {
                    enableSetDefaultsButton();
                }
            });

            javax.swing.GroupLayout _maxTemporalOptionsGroupLayout = new javax.swing.GroupLayout(_maxTemporalOptionsGroup);
            _maxTemporalOptionsGroup.setLayout(_maxTemporalOptionsGroupLayout);
            _maxTemporalOptionsGroupLayout.setHorizontalGroup(
                _maxTemporalOptionsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_maxTemporalOptionsGroupLayout.createSequentialGroup()
                    .addContainerGap()
                    .addGroup(_maxTemporalOptionsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                        .addGroup(_maxTemporalOptionsGroupLayout.createSequentialGroup()
                            .addComponent(_percentageTemporalRadioButton)
                            .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                            .addComponent(_maxTemporalClusterSizeTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 60, javax.swing.GroupLayout.PREFERRED_SIZE)
                            .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                            .addComponent(_percentageOfStudyPeriodLabel, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                        .addGroup(_maxTemporalOptionsGroupLayout.createSequentialGroup()
                            .addComponent(_timeTemporalRadioButton)
                            .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                            .addComponent(_maxTemporalClusterSizeUnitsTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 60, javax.swing.GroupLayout.PREFERRED_SIZE)
                            .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                            .addComponent(_maxTemporalTimeUnitsLabel, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                        .addComponent(_includePureSpacClustCheckBox, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                    .addContainerGap())
            );
            _maxTemporalOptionsGroupLayout.setVerticalGroup(
                _maxTemporalOptionsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_maxTemporalOptionsGroupLayout.createSequentialGroup()
                    .addContainerGap()
                    .addGroup(_maxTemporalOptionsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                        .addComponent(_percentageTemporalRadioButton)
                        .addComponent(_maxTemporalClusterSizeTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addComponent(_percentageOfStudyPeriodLabel))
                    .addGap(10, 10, 10)
                    .addGroup(_maxTemporalOptionsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                        .addComponent(_timeTemporalRadioButton)
                        .addComponent(_maxTemporalClusterSizeUnitsTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addComponent(_maxTemporalTimeUnitsLabel))
                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                    .addComponent(_includePureSpacClustCheckBox)
                    .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
            );

            _percentageTemporalRadioButton.getAccessibleContext().setAccessibleName("selection of maximum temporal by percentage of population");
            _timeTemporalRadioButton.getAccessibleContext().setAccessibleName("selection of maximum temporal by time units");
            _percentageOfStudyPeriodLabel.getAccessibleContext().setAccessibleName("percent of the study period (<= 90, default = 50)");
            _maxTemporalClusterSizeUnitsTextField.getAccessibleContext().setAccessibleName("years (<=90 of the study period)");

            _flexibleTemporalWindowDefinitionGroup.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createEtchedBorder(), "Flexible Temporal Window Definition"));

            _restrictTemporalRangeCheckBox.setText("Include only windows with:"); // NOI18N
            _restrictTemporalRangeCheckBox.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
            _restrictTemporalRangeCheckBox.setMargin(new java.awt.Insets(0, 0, 0, 0));
            _restrictTemporalRangeCheckBox.addItemListener(new java.awt.event.ItemListener() {
                public void itemStateChanged(java.awt.event.ItemEvent e) {
                    enableReportedSpatialOptionsGroup(_spatialOptionsGroup.isEnabled());
                    enableDatesByTimePrecisionUnits();
                    enableTemporalOptionsGroup(_maxTemporalOptionsGroup.isEnabled(), _includePureSpacClustCheckBox.isEnabled(), _restrictTemporalRangeCheckBox.isEnabled());
                    enableSetDefaultsButton();
                }
            });

            _flexible_window_cards.setLayout(new java.awt.CardLayout());

            _startWindowRangeLabel.setText("Start time in range:"); // NOI18N

            _startRangeStartYearTextField.setText("2000"); // NOI18N

            _startRangeStartMonthTextField.setText("01"); // NOI18N

            _startRangeStartDayTextField.setText("01"); // NOI18N

            _startRangeToLabel.setText("to"); // NOI18N

            _startRangeEndYearTextField.setText("2000"); // NOI18N

            _startRangeEndMonthTextField.setText("01"); // NOI18N

            _startRangeEndDayTextField.setText("01"); // NOI18N

            _endRangeEndDayTextField.setText("31"); // NOI18N

            _endRangeEndMonthTextField.setText("12"); // NOI18N

            _endRangeEndYearTextField.setText("2000"); // NOI18N

            _endRangeToLabel.setText("to"); // NOI18N

            _endRangeStartDayTextField.setText("31"); // NOI18N

            _endRangeStartMonthTextField.setText("12"); // NOI18N

            _endRangeStartYearTextField.setText("2000"); // NOI18N

            _endWindowRangeLabel.setText("End time in range:"); // NOI18N

            javax.swing.GroupLayout _windowCompletePanelLayout = new javax.swing.GroupLayout(_windowCompletePanel);
            _windowCompletePanel.setLayout(_windowCompletePanelLayout);
            _windowCompletePanelLayout.setHorizontalGroup(
                _windowCompletePanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_windowCompletePanelLayout.createSequentialGroup()
                    .addContainerGap()
                    .addGroup(_windowCompletePanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                        .addComponent(_startWindowRangeLabel)
                        .addComponent(_endWindowRangeLabel))
                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                    .addGroup(_windowCompletePanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                        .addComponent(_endRangeStartYearTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 60, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addComponent(_startRangeStartYearTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 60, javax.swing.GroupLayout.PREFERRED_SIZE))
                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                    .addGroup(_windowCompletePanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                        .addComponent(_endRangeStartMonthTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 40, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addComponent(_startRangeStartMonthTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 40, javax.swing.GroupLayout.PREFERRED_SIZE))
                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                    .addGroup(_windowCompletePanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                        .addComponent(_endRangeStartDayTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 40, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addComponent(_startRangeStartDayTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 40, javax.swing.GroupLayout.PREFERRED_SIZE))
                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                    .addGroup(_windowCompletePanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                        .addComponent(_startRangeToLabel)
                        .addComponent(_endRangeToLabel, javax.swing.GroupLayout.Alignment.TRAILING))
                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addGroup(_windowCompletePanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                        .addComponent(_endRangeEndYearTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 60, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addComponent(_startRangeEndYearTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 60, javax.swing.GroupLayout.PREFERRED_SIZE))
                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                    .addGroup(_windowCompletePanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                        .addGroup(_windowCompletePanelLayout.createSequentialGroup()
                            .addComponent(_startRangeEndMonthTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 40, javax.swing.GroupLayout.PREFERRED_SIZE)
                            .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                            .addComponent(_startRangeEndDayTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 40, javax.swing.GroupLayout.PREFERRED_SIZE))
                        .addGroup(_windowCompletePanelLayout.createSequentialGroup()
                            .addComponent(_endRangeEndMonthTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 40, javax.swing.GroupLayout.PREFERRED_SIZE)
                            .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                            .addComponent(_endRangeEndDayTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 40, javax.swing.GroupLayout.PREFERRED_SIZE)))
                    .addContainerGap(227, Short.MAX_VALUE))
            );
            _windowCompletePanelLayout.setVerticalGroup(
                _windowCompletePanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_windowCompletePanelLayout.createSequentialGroup()
                    .addContainerGap()
                    .addGroup(_windowCompletePanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                        .addComponent(_startWindowRangeLabel)
                        .addComponent(_startRangeStartYearTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addComponent(_startRangeStartMonthTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addComponent(_startRangeStartDayTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addComponent(_startRangeToLabel)
                        .addComponent(_startRangeEndYearTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addComponent(_startRangeEndMonthTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addComponent(_startRangeEndDayTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                    .addGroup(_windowCompletePanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                        .addComponent(_endWindowRangeLabel)
                        .addComponent(_endRangeStartYearTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addComponent(_endRangeStartMonthTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addComponent(_endRangeStartDayTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addComponent(_endRangeToLabel)
                        .addComponent(_endRangeEndYearTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addComponent(_endRangeEndMonthTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addComponent(_endRangeEndDayTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                    .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
            );

            _startRangeStartYearTextField.getAccessibleContext().setAccessibleName("start range, begin year");
            _startRangeStartMonthTextField.getAccessibleContext().setAccessibleName("start range, begin month");
            _startRangeStartDayTextField.getAccessibleContext().setAccessibleName("start range, begin day");
            _startRangeEndYearTextField.getAccessibleContext().setAccessibleName("start range, end year");
            _startRangeEndMonthTextField.getAccessibleContext().setAccessibleName("start range, end month");
            _startRangeEndDayTextField.getAccessibleContext().setAccessibleName("start range, end day");
            _endRangeEndDayTextField.getAccessibleContext().setAccessibleName("end range, end day");
            _endRangeEndMonthTextField.getAccessibleContext().setAccessibleName("end range, end month");
            _endRangeEndYearTextField.getAccessibleContext().setAccessibleName("end range, end year");
            _endRangeStartDayTextField.getAccessibleContext().setAccessibleName("end range, begin day");
            _endRangeStartMonthTextField.getAccessibleContext().setAccessibleName("end range, begin month");
            _endRangeStartYearTextField.getAccessibleContext().setAccessibleName("end range, begin year");

            _flexible_window_cards.add(_windowCompletePanel, "flexible_complete");

            _startGenericWindowRangeLabel.setText("Start time in range:"); // NOI18N

            _startRangeStartGenericTextField.setText("0"); // NOI18N
            _startRangeStartGenericTextField.getDocument().addUndoableEditListener(new UndoableEditListener() {
                public void undoableEditHappened(UndoableEditEvent evt) {
                    undo.addEdit(evt.getEdit());
                }
            });
            _startRangeStartGenericTextField.addKeyListener(new java.awt.event.KeyAdapter() {
                public void keyTyped(java.awt.event.KeyEvent e) {
                    Utils.validateNumericKeyTyped(_startRangeStartGenericTextField, e, 10);
                }
            });

            _startGenericRangeToLabel.setText("to"); // NOI18N

            _startRangeEndGenericTextField.setText("31"); // NOI18N
            _startRangeEndGenericTextField.getDocument().addUndoableEditListener(new UndoableEditListener() {
                public void undoableEditHappened(UndoableEditEvent evt) {
                    undo.addEdit(evt.getEdit());
                }
            });
            _startRangeEndGenericTextField.addKeyListener(new java.awt.event.KeyAdapter() {
                public void keyTyped(java.awt.event.KeyEvent e) {
                    Utils.validatePostiveNumericKeyTyped(_startRangeEndGenericTextField, e, 10);
                }
            });

            _endRangeEndGenericTextField.setText("31"); // NOI18N
            _endRangeEndGenericTextField.getDocument().addUndoableEditListener(new UndoableEditListener() {
                public void undoableEditHappened(UndoableEditEvent evt) {
                    undo.addEdit(evt.getEdit());
                }
            });
            _endRangeEndGenericTextField.addKeyListener(new java.awt.event.KeyAdapter() {
                public void keyTyped(java.awt.event.KeyEvent e) {
                    Utils.validatePostiveNumericKeyTyped(_endRangeEndGenericTextField, e, 10);
                }
            });

            _endGenericRangeToLabel.setText("to"); // NOI18N

            _endRangeStartGenericTextField.setText("0"); // NOI18N
            _endRangeStartGenericTextField.getDocument().addUndoableEditListener(new UndoableEditListener() {
                public void undoableEditHappened(UndoableEditEvent evt) {
                    undo.addEdit(evt.getEdit());
                }
            });
            _endRangeStartGenericTextField.addKeyListener(new java.awt.event.KeyAdapter() {
                public void keyTyped(java.awt.event.KeyEvent e) {
                    Utils.validatePostiveNumericKeyTyped(_endRangeStartGenericTextField, e, 10);
                }
            });

            _endGenericWindowRangeLabel.setText("End time in range:"); // NOI18N

            javax.swing.GroupLayout _windowGenericPanelLayout = new javax.swing.GroupLayout(_windowGenericPanel);
            _windowGenericPanel.setLayout(_windowGenericPanelLayout);
            _windowGenericPanelLayout.setHorizontalGroup(
                _windowGenericPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_windowGenericPanelLayout.createSequentialGroup()
                    .addContainerGap()
                    .addGroup(_windowGenericPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                        .addComponent(_startGenericWindowRangeLabel)
                        .addComponent(_endGenericWindowRangeLabel))
                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                    .addGroup(_windowGenericPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                        .addComponent(_endRangeStartGenericTextField, javax.swing.GroupLayout.Alignment.TRAILING)
                        .addComponent(_startRangeStartGenericTextField, javax.swing.GroupLayout.Alignment.TRAILING, javax.swing.GroupLayout.DEFAULT_SIZE, 114, Short.MAX_VALUE))
                    .addGap(18, 18, 18)
                    .addGroup(_windowGenericPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                        .addGroup(_windowGenericPanelLayout.createSequentialGroup()
                            .addComponent(_startGenericRangeToLabel)
                            .addGap(10, 10, 10)
                            .addComponent(_startRangeEndGenericTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 114, javax.swing.GroupLayout.PREFERRED_SIZE))
                        .addGroup(_windowGenericPanelLayout.createSequentialGroup()
                            .addComponent(_endGenericRangeToLabel)
                            .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                            .addComponent(_endRangeEndGenericTextField)))
                    .addContainerGap(247, Short.MAX_VALUE))
            );
            _windowGenericPanelLayout.setVerticalGroup(
                _windowGenericPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_windowGenericPanelLayout.createSequentialGroup()
                    .addContainerGap()
                    .addGroup(_windowGenericPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                        .addComponent(_startGenericWindowRangeLabel)
                        .addComponent(_startRangeStartGenericTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addComponent(_startGenericRangeToLabel)
                        .addComponent(_startRangeEndGenericTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                    .addGroup(_windowGenericPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                        .addComponent(_endGenericWindowRangeLabel)
                        .addComponent(_endRangeStartGenericTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addComponent(_endRangeEndGenericTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addComponent(_endGenericRangeToLabel))
                    .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
            );

            _flexible_window_cards.add(_windowGenericPanel, "flexible_generic");

            javax.swing.GroupLayout _flexibleTemporalWindowDefinitionGroupLayout = new javax.swing.GroupLayout(_flexibleTemporalWindowDefinitionGroup);
            _flexibleTemporalWindowDefinitionGroup.setLayout(_flexibleTemporalWindowDefinitionGroupLayout);
            _flexibleTemporalWindowDefinitionGroupLayout.setHorizontalGroup(
                _flexibleTemporalWindowDefinitionGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_flexibleTemporalWindowDefinitionGroupLayout.createSequentialGroup()
                    .addContainerGap()
                    .addGroup(_flexibleTemporalWindowDefinitionGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                        .addComponent(_flexible_window_cards, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                        .addComponent(_restrictTemporalRangeCheckBox, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                    .addContainerGap())
            );
            _flexibleTemporalWindowDefinitionGroupLayout.setVerticalGroup(
                _flexibleTemporalWindowDefinitionGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_flexibleTemporalWindowDefinitionGroupLayout.createSequentialGroup()
                    .addContainerGap()
                    .addComponent(_restrictTemporalRangeCheckBox)
                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                    .addComponent(_flexible_window_cards, javax.swing.GroupLayout.PREFERRED_SIZE, 74, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
            );

            _restrictTemporalRangeCheckBox.getAccessibleContext().setAccessibleName("Include only windows within specified range");

            _minTemporalOptionsGroup.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createEtchedBorder(), "Minimum Temporal Cluster Size"));

            _minTemporalClusterSizeUnitsTextField.setText("1"); // NOI18N
            _minTemporalClusterSizeUnitsTextField.addKeyListener(new java.awt.event.KeyAdapter() {
                public void keyTyped(java.awt.event.KeyEvent e) {
                    Utils.validatePostiveNumericKeyTyped(_minTemporalClusterSizeUnitsTextField, e, 6);
                }
            });
            _minTemporalClusterSizeUnitsTextField.addFocusListener(new java.awt.event.FocusAdapter() {
                public void focusLost(java.awt.event.FocusEvent e) {
                    while (_minTemporalClusterSizeUnitsTextField.getText().length() == 0 || Integer.parseInt(_minTemporalClusterSizeUnitsTextField.getText()) == 0) {
                        if (undo.canUndo()) undo.undo(); else _minTemporalClusterSizeUnitsTextField.setText("1");
                    }
                    enableSetDefaultsButton();
                }
            });
            _minTemporalClusterSizeUnitsTextField.getDocument().addUndoableEditListener(new UndoableEditListener() {
                public void undoableEditHappened(UndoableEditEvent evt) {
                    undo.addEdit(evt.getEdit());
                }
            });

            _minTemporalTimeUnitsLabel.setText("years"); // NOI18N

            javax.swing.GroupLayout _minTemporalOptionsGroupLayout = new javax.swing.GroupLayout(_minTemporalOptionsGroup);
            _minTemporalOptionsGroup.setLayout(_minTemporalOptionsGroupLayout);
            _minTemporalOptionsGroupLayout.setHorizontalGroup(
                _minTemporalOptionsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_minTemporalOptionsGroupLayout.createSequentialGroup()
                    .addContainerGap()
                    .addComponent(_minTemporalClusterSizeUnitsTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 60, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                    .addComponent(_minTemporalTimeUnitsLabel, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addContainerGap())
            );
            _minTemporalOptionsGroupLayout.setVerticalGroup(
                _minTemporalOptionsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, _minTemporalOptionsGroupLayout.createSequentialGroup()
                    .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addGroup(_minTemporalOptionsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                        .addComponent(_minTemporalClusterSizeUnitsTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addComponent(_minTemporalTimeUnitsLabel))
                    .addContainerGap())
            );

            _minTemporalClusterSizeUnitsTextField.getAccessibleContext().setAccessibleName("minimum temporal cluster size in time units");

            javax.swing.GroupLayout _temporalWindowTabLayout = new javax.swing.GroupLayout(_temporalWindowTab);
            _temporalWindowTab.setLayout(_temporalWindowTabLayout);
            _temporalWindowTabLayout.setHorizontalGroup(
                _temporalWindowTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_temporalWindowTabLayout.createSequentialGroup()
                    .addContainerGap()
                    .addGroup(_temporalWindowTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                        .addComponent(_maxTemporalOptionsGroup, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                        .addComponent(_flexibleTemporalWindowDefinitionGroup, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                        .addComponent(_minTemporalOptionsGroup, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                    .addContainerGap())
            );
            _temporalWindowTabLayout.setVerticalGroup(
                _temporalWindowTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_temporalWindowTabLayout.createSequentialGroup()
                    .addContainerGap()
                    .addComponent(_maxTemporalOptionsGroup, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addGap(11, 11, 11)
                    .addComponent(_minTemporalOptionsGroup, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                    .addComponent(_flexibleTemporalWindowDefinitionGroup, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addContainerGap(134, Short.MAX_VALUE))
            );

            jTabbedPane1.addTab("Temporal Window", _temporalWindowTab);
            _temporalWindowTab.getAccessibleContext().setAccessibleName("Temporal Window tab");

            _temporalTrendAdjGroup.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createEtchedBorder(), "Temporal Trend Adjustments"));

            _temporalTrendAdjNone.setSelected(true);
            _temporalTrendAdjNone.setText("None"); // NOI18N
            _temporalTrendAdjNone.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
            _temporalTrendAdjNone.setMargin(new java.awt.Insets(0, 0, 0, 0));
            _temporalTrendAdjNone.addItemListener(new java.awt.event.ItemListener() {
                public void itemStateChanged(java.awt.event.ItemEvent e) {
                    enableSetDefaultsButton();
                    enableSettingsForAnalysisModelCombination();
                }
            });

            _temporalTrendAdjNonparametric.setText("Nonparametric"); // NOI18N
            _temporalTrendAdjNonparametric.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
            _temporalTrendAdjNonparametric.setMargin(new java.awt.Insets(0, 0, 0, 0));
            _temporalTrendAdjNonparametric.addItemListener(new java.awt.event.ItemListener() {
                public void itemStateChanged(java.awt.event.ItemEvent e) {
                    if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED) {
                        _spatialAdjustmentsNonparametric.setEnabled(true);
                        enableSetDefaultsButton();
                        enableSettingsForAnalysisModelCombination();
                    }
                }
            });

            _temporalTrendAdjLogLinear.setText("Log linear trend with"); // NOI18N
            _temporalTrendAdjLogLinear.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
            _temporalTrendAdjLogLinear.setMargin(new java.awt.Insets(0, 0, 0, 0));
            _temporalTrendAdjLogLinear.addItemListener(new java.awt.event.ItemListener() {
                public void itemStateChanged(java.awt.event.ItemEvent e) {
                    if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED) {
                        _logLinearTextField.setEnabled(true);
                        enableSetDefaultsButton();
                        enableSettingsForAnalysisModelCombination();
                    }
                }
            });

            _temporalTrendAdjLogLinearCalc.setText("Log linear with automatically calculated trend"); // NOI18N
            _temporalTrendAdjLogLinearCalc.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
            _temporalTrendAdjLogLinearCalc.setMargin(new java.awt.Insets(0, 0, 0, 0));
            _temporalTrendAdjLogLinearCalc.addItemListener(new java.awt.event.ItemListener() {
                public void itemStateChanged(java.awt.event.ItemEvent e) {
                    enableSetDefaultsButton();
                    enableSettingsForAnalysisModelCombination();
                }
            });

            _logLinearTextField.setText("0"); // NOI18N
            _logLinearTextField.getDocument().addUndoableEditListener(new UndoableEditListener() {
                public void undoableEditHappened(UndoableEditEvent evt) {
                    undo.addEdit(evt.getEdit());
                }
            });
            _logLinearTextField.addKeyListener(new java.awt.event.KeyAdapter() {
                public void keyTyped(java.awt.event.KeyEvent e) {
                    Utils.validateFloatKeyTyped(_logLinearTextField, e, 5);
                }
            });
            _logLinearTextField.addFocusListener(new java.awt.event.FocusAdapter() {
                public void focusLost(java.awt.event.FocusEvent e) {
                    while (_logLinearTextField.getText().length() == 0 || Double.parseDouble(_logLinearTextField.getText()) < -100) {
                        if (undo.canUndo()) undo.undo(); else _logLinearTextField.setText("0");
                    }
                    enableSetDefaultsButton();
                }
            });

            _logLinearLabel.setText("%  per year"); // NOI18N

            _temporalTrendAdjQuadCalc.setText("Log quadratic with automatically calculated trend"); // NOI18N
            _temporalTrendAdjQuadCalc.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
            _temporalTrendAdjQuadCalc.setMargin(new java.awt.Insets(0, 0, 0, 0));
            _temporalTrendAdjQuadCalc.addItemListener(new java.awt.event.ItemListener() {
                public void itemStateChanged(java.awt.event.ItemEvent e) {
                    enableSetDefaultsButton();
                    enableSettingsForAnalysisModelCombination();
                }
            });

            javax.swing.GroupLayout _temporalTrendAdjGroupLayout = new javax.swing.GroupLayout(_temporalTrendAdjGroup);
            _temporalTrendAdjGroup.setLayout(_temporalTrendAdjGroupLayout);
            _temporalTrendAdjGroupLayout.setHorizontalGroup(
                _temporalTrendAdjGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_temporalTrendAdjGroupLayout.createSequentialGroup()
                    .addContainerGap()
                    .addGroup(_temporalTrendAdjGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                        .addGroup(_temporalTrendAdjGroupLayout.createSequentialGroup()
                            .addGroup(_temporalTrendAdjGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                                .addComponent(_temporalTrendAdjLogLinearCalc, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                                .addComponent(_temporalTrendAdjNone, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                                .addGroup(_temporalTrendAdjGroupLayout.createSequentialGroup()
                                    .addComponent(_temporalTrendAdjLogLinear)
                                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                    .addComponent(_logLinearTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 70, javax.swing.GroupLayout.PREFERRED_SIZE)
                                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                    .addComponent(_logLinearLabel, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                                .addComponent(_temporalTrendAdjNonparametric, javax.swing.GroupLayout.DEFAULT_SIZE, 628, Short.MAX_VALUE))
                            .addGap(2, 2, 2))
                        .addGroup(_temporalTrendAdjGroupLayout.createSequentialGroup()
                            .addComponent(_temporalTrendAdjQuadCalc, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                            .addContainerGap())))
            );
            _temporalTrendAdjGroupLayout.setVerticalGroup(
                _temporalTrendAdjGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, _temporalTrendAdjGroupLayout.createSequentialGroup()
                    .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(_temporalTrendAdjNone)
                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                    .addComponent(_temporalTrendAdjNonparametric)
                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                    .addGroup(_temporalTrendAdjGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                        .addComponent(_temporalTrendAdjLogLinear)
                        .addComponent(_logLinearTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addComponent(_logLinearLabel))
                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                    .addComponent(_temporalTrendAdjLogLinearCalc)
                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                    .addComponent(_temporalTrendAdjQuadCalc)
                    .addContainerGap())
            );

            _temporalTrendAdjNone.getAccessibleContext().setAccessibleName("no temporal adjustments");
            _temporalTrendAdjNonparametric.getAccessibleContext().setAccessibleName("Nonparametric temporal adjustment");
            _temporalTrendAdjLogLinear.getAccessibleContext().setAccessibleName("Log linear trend adjustment");
            _logLinearTextField.getAccessibleContext().setAccessibleName("log linear trend adjustment percentage");

            _spatialAdjustmentsGroup.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createEtchedBorder(), "Spatial Adjustments"));

            _spatialAdjustmentsNone.setSelected(true);
            _spatialAdjustmentsNone.setText("None"); // NOI18N
            _spatialAdjustmentsNone.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
            _spatialAdjustmentsNone.setMargin(new java.awt.Insets(0, 0, 0, 0));
            _spatialAdjustmentsNone.addItemListener(new java.awt.event.ItemListener() {
                public void itemStateChanged(java.awt.event.ItemEvent e) {
                    enableSettingsForAnalysisModelCombination();
                    enableSetDefaultsButton();
                }
            });

            _spatialAdjustmentsNonparametric.setText("Nonparametric"); // NOI18N
            _spatialAdjustmentsNonparametric.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
            _spatialAdjustmentsNonparametric.setMargin(new java.awt.Insets(0, 0, 0, 0));
            _spatialAdjustmentsNonparametric.addItemListener(new java.awt.event.ItemListener() {
                public void itemStateChanged(java.awt.event.ItemEvent e) {
                    enableSetDefaultsButton();
                    enableSettingsForAnalysisModelCombination();
                }
            });

            javax.swing.GroupLayout _spatialAdjustmentsGroupLayout = new javax.swing.GroupLayout(_spatialAdjustmentsGroup);
            _spatialAdjustmentsGroup.setLayout(_spatialAdjustmentsGroupLayout);
            _spatialAdjustmentsGroupLayout.setHorizontalGroup(
                _spatialAdjustmentsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_spatialAdjustmentsGroupLayout.createSequentialGroup()
                    .addContainerGap()
                    .addGroup(_spatialAdjustmentsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                        .addComponent(_spatialAdjustmentsNone, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                        .addComponent(_spatialAdjustmentsNonparametric, javax.swing.GroupLayout.DEFAULT_SIZE, 628, Short.MAX_VALUE))
                    .addGap(2, 2, 2))
            );
            _spatialAdjustmentsGroupLayout.setVerticalGroup(
                _spatialAdjustmentsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_spatialAdjustmentsGroupLayout.createSequentialGroup()
                    .addContainerGap()
                    .addComponent(_spatialAdjustmentsNone)
                    .addGap(10, 10, 10)
                    .addComponent(_spatialAdjustmentsNonparametric)
                    .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
            );

            _spatialAdjustmentsNone.getAccessibleContext().setAccessibleName("no spatial adjustment");
            _spatialAdjustmentsNonparametric.getAccessibleContext().setAccessibleName("Nonparametric spatial adjustment");

            _knownAdjustmentsGroup.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createEtchedBorder(), "Temporal, Spatial and/or Space-Time Adjustments"));

            _adjustForKnownRelativeRisksCheckBox.setText("Adjust for known relative risks"); // NOI18N
            _adjustForKnownRelativeRisksCheckBox.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
            _adjustForKnownRelativeRisksCheckBox.setMargin(new java.awt.Insets(0, 0, 0, 0));
            _adjustForKnownRelativeRisksCheckBox.addItemListener(new java.awt.event.ItemListener() {
                public void itemStateChanged(java.awt.event.ItemEvent e) {
                    boolean  bEnabled = _knownAdjustmentsGroup.isEnabled() && e.getStateChange() == java.awt.event.ItemEvent.SELECTED;

                    _adjustmentsByRelativeRisksFileLabel.setEnabled(bEnabled);
                    _adjustmentsByRelativeRisksFileTextField.setEnabled(bEnabled);
                    _adjustmentsFileBrowseButton.setEnabled(bEnabled);
                    enableSetDefaultsButton();
                }
            });

            _adjustmentsByRelativeRisksFileTextField.addFocusListener(new java.awt.event.FocusAdapter() {
                public void focusLost(java.awt.event.FocusEvent e) {
                    enableSetDefaultsButton();
                }
            });

            _adjustmentsFileBrowseButton.setText("..."); // NOI18N
            _adjustmentsFileBrowseButton.setToolTipText("Open Adjustments File Import Wizard"); // NOI18N
            _adjustmentsFileBrowseButton.addActionListener(new java.awt.event.ActionListener() {
                public void actionPerformed(java.awt.event.ActionEvent e) {
                    String key = InputSourceSettings.InputFileType.AdjustmentsByRR.toString() + "1";
                    if (!_settings_window._input_source_map.containsKey(key)) {
                        _settings_window._input_source_map.put(key, new InputSourceSettings(InputSourceSettings.InputFileType.AdjustmentsByRR));
                    }
                    InputSourceSettings inputSourceSettings = (InputSourceSettings)_settings_window._input_source_map.get(key);
                    // invoke the FileSelectionDialog to guide user through process of selecting the source file.
                    FileSelectionDialog selectionDialog = new FileSelectionDialog(SaTScanApplication.getInstance(), inputSourceSettings.getInputFileType(), SaTScanApplication.getInstance().lastBrowseDirectory);
                    selectionDialog.browse_inputsource(_adjustmentsByRelativeRisksFileTextField, inputSourceSettings, _settings_window);
                }
            });

            _adjustmentsByRelativeRisksFileLabel.setLabelFor(_adjustmentsByRelativeRisksFileTextField);
            _adjustmentsByRelativeRisksFileLabel.setText("Adjustments File:"); // NOI18N

            javax.swing.GroupLayout _knownAdjustmentsGroupLayout = new javax.swing.GroupLayout(_knownAdjustmentsGroup);
            _knownAdjustmentsGroup.setLayout(_knownAdjustmentsGroupLayout);
            _knownAdjustmentsGroupLayout.setHorizontalGroup(
                _knownAdjustmentsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_knownAdjustmentsGroupLayout.createSequentialGroup()
                    .addContainerGap()
                    .addGroup(_knownAdjustmentsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                        .addGroup(_knownAdjustmentsGroupLayout.createSequentialGroup()
                            .addGap(17, 17, 17)
                            .addComponent(_adjustmentsByRelativeRisksFileLabel)
                            .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                            .addComponent(_adjustmentsByRelativeRisksFileTextField)
                            .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                            .addComponent(_adjustmentsFileBrowseButton, javax.swing.GroupLayout.PREFERRED_SIZE, 25, javax.swing.GroupLayout.PREFERRED_SIZE))
                        .addComponent(_adjustForKnownRelativeRisksCheckBox, javax.swing.GroupLayout.DEFAULT_SIZE, 620, Short.MAX_VALUE))
                    .addContainerGap())
            );
            _knownAdjustmentsGroupLayout.setVerticalGroup(
                _knownAdjustmentsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_knownAdjustmentsGroupLayout.createSequentialGroup()
                    .addContainerGap()
                    .addComponent(_adjustForKnownRelativeRisksCheckBox)
                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                    .addGroup(_knownAdjustmentsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                        .addComponent(_adjustmentsByRelativeRisksFileLabel)
                        .addComponent(_adjustmentsByRelativeRisksFileTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addComponent(_adjustmentsFileBrowseButton))
                    .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
            );

            _adjustmentsFileBrowseButton.getAccessibleContext().setAccessibleName("");

            _adjustDayOfWeek.setText("Adjust for day-of-week ");
            _adjustDayOfWeek.addItemListener(new java.awt.event.ItemListener() {
                public void itemStateChanged(java.awt.event.ItemEvent e) {
                    enableSetDefaultsButton();
                    enableSettingsForAnalysisModelCombination();
                }
            });

            javax.swing.GroupLayout _spaceTimeAjustmentsTabLayout = new javax.swing.GroupLayout(_spaceTimeAjustmentsTab);
            _spaceTimeAjustmentsTab.setLayout(_spaceTimeAjustmentsTabLayout);
            _spaceTimeAjustmentsTabLayout.setHorizontalGroup(
                _spaceTimeAjustmentsTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_spaceTimeAjustmentsTabLayout.createSequentialGroup()
                    .addContainerGap()
                    .addGroup(_spaceTimeAjustmentsTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                        .addComponent(_spatialAdjustmentsGroup, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                        .addComponent(_temporalTrendAdjGroup, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                        .addComponent(_knownAdjustmentsGroup, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                        .addComponent(_adjustDayOfWeek, javax.swing.GroupLayout.Alignment.TRAILING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                    .addContainerGap())
            );
            _spaceTimeAjustmentsTabLayout.setVerticalGroup(
                _spaceTimeAjustmentsTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_spaceTimeAjustmentsTabLayout.createSequentialGroup()
                    .addContainerGap()
                    .addComponent(_temporalTrendAdjGroup, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                    .addComponent(_adjustDayOfWeek)
                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                    .addComponent(_spatialAdjustmentsGroup, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                    .addComponent(_knownAdjustmentsGroup, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addContainerGap(94, Short.MAX_VALUE))
            );

            _adjustDayOfWeek.getAccessibleContext().setAccessibleName("Adjust for day of week");

            jTabbedPane1.addTab("Space and Time Adjustments", _spaceTimeAjustmentsTab);
            _spaceTimeAjustmentsTab.getAccessibleContext().setAccessibleName("Space and Time Adjustments tab");

            _pValueOptionsGroup.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createEtchedBorder(), "P-Value"));

            _radioDefaultPValues.setSelected(true);
            _radioDefaultPValues.setText("Default");
            _radioDefaultPValues.addItemListener(new java.awt.event.ItemListener() {
                public void itemStateChanged(java.awt.event.ItemEvent e) {
                    if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED) {
                        enablePValueOptionsGroup();
                        enableSetDefaultsButton();
                        updateMonteCarloTextCaptions();
                    }
                }
            });

            _radioEarlyTerminationPValues.setText("Sequential Monte Carlo       Early termination cutoff:");
            _radioEarlyTerminationPValues.addItemListener(new java.awt.event.ItemListener() {
                public void itemStateChanged(java.awt.event.ItemEvent e) {
                    if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED) {
                        enablePValueOptionsGroup();
                        enableSetDefaultsButton();
                        updateMonteCarloTextCaptions();
                    }
                }
            });

            _radioGumbelPValues.setText("Gumbel Approximation");
            _radioGumbelPValues.addItemListener(new java.awt.event.ItemListener() {
                public void itemStateChanged(java.awt.event.ItemEvent e) {
                    if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED) {
                        enablePValueOptionsGroup();
                        enableSetDefaultsButton();
                        updateMonteCarloTextCaptions();
                    }
                }
            });

            _radioStandardPValues.setText("Standard Monte Carlo");
            _radioStandardPValues.addItemListener(new java.awt.event.ItemListener() {
                public void itemStateChanged(java.awt.event.ItemEvent e) {
                    if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED) {
                        enablePValueOptionsGroup();
                        enableSetDefaultsButton();
                        updateMonteCarloTextCaptions();
                    }
                }
            });

            _checkReportGumbel.setText("Also report Gumbel based p-values");
            _checkReportGumbel.setHorizontalAlignment(javax.swing.SwingConstants.RIGHT);

            _earlyTerminationThreshold.setText("50");
            _earlyTerminationThreshold.addFocusListener(new java.awt.event.FocusAdapter() {
                public void focusLost(java.awt.event.FocusEvent e) {
                    while (_earlyTerminationThreshold.getText().length() == 0 || Integer.parseInt(_earlyTerminationThreshold.getText()) < 1)
                    if (undo.canUndo()) undo.undo(); else _earlyTerminationThreshold.setText("50");
                    enableSetDefaultsButton();
                }
            });
            _earlyTerminationThreshold.addKeyListener(new java.awt.event.KeyAdapter() {
                public void keyTyped(java.awt.event.KeyEvent e) {
                    Utils.validatePostiveNumericKeyTyped(_earlyTerminationThreshold, e, 10);
                }
            });
            _earlyTerminationThreshold.getDocument().addUndoableEditListener(new UndoableEditListener() {
                public void undoableEditHappened(UndoableEditEvent evt) {
                    undo.addEdit(evt.getEdit());
                }
            });

            javax.swing.GroupLayout _pValueOptionsGroupLayout = new javax.swing.GroupLayout(_pValueOptionsGroup);
            _pValueOptionsGroup.setLayout(_pValueOptionsGroupLayout);
            _pValueOptionsGroupLayout.setHorizontalGroup(
                _pValueOptionsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_pValueOptionsGroupLayout.createSequentialGroup()
                    .addContainerGap()
                    .addGroup(_pValueOptionsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                        .addComponent(_radioDefaultPValues, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                        .addComponent(_radioStandardPValues, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                        .addGroup(_pValueOptionsGroupLayout.createSequentialGroup()
                            .addComponent(_radioGumbelPValues, javax.swing.GroupLayout.PREFERRED_SIZE, 342, javax.swing.GroupLayout.PREFERRED_SIZE)
                            .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED, 5, Short.MAX_VALUE)
                            .addComponent(_checkReportGumbel, javax.swing.GroupLayout.PREFERRED_SIZE, 281, javax.swing.GroupLayout.PREFERRED_SIZE))
                        .addGroup(_pValueOptionsGroupLayout.createSequentialGroup()
                            .addComponent(_radioEarlyTerminationPValues)
                            .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                            .addComponent(_earlyTerminationThreshold, javax.swing.GroupLayout.PREFERRED_SIZE, 73, javax.swing.GroupLayout.PREFERRED_SIZE)
                            .addGap(0, 0, Short.MAX_VALUE)))
                    .addContainerGap())
            );
            _pValueOptionsGroupLayout.setVerticalGroup(
                _pValueOptionsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_pValueOptionsGroupLayout.createSequentialGroup()
                    .addComponent(_radioDefaultPValues)
                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                    .addComponent(_radioStandardPValues)
                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                    .addGroup(_pValueOptionsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                        .addComponent(_radioEarlyTerminationPValues)
                        .addComponent(_earlyTerminationThreshold, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                    .addGroup(_pValueOptionsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                        .addComponent(_radioGumbelPValues)
                        .addComponent(_checkReportGumbel))
                    .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
            );

            _radioDefaultPValues.getAccessibleContext().setAccessibleName("Default combination");
            _earlyTerminationThreshold.getAccessibleContext().setAccessibleName("early termination cutoff");

            _iterativeScanGroup.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createEtchedBorder(), "Iterative Scan Statistic"));

            _performIterativeScanCheckBox.setText("Adjusting for More Likely Clusters"); // NOI18N
            _performIterativeScanCheckBox.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
            _performIterativeScanCheckBox.setMargin(new java.awt.Insets(0, 0, 0, 0));
            _performIterativeScanCheckBox.addItemListener(new java.awt.event.ItemListener() {
                public void itemStateChanged(java.awt.event.ItemEvent e) {
                    enableSettingsForAnalysisModelCombination();
                    enableSetDefaultsButton();
                }
            });

            _maxIterativeScansLabel.setLabelFor(_numIterativeScansTextField);
            _maxIterativeScansLabel.setText("Maximum number of iterations:"); // NOI18N

            _numIterativeScansTextField.setText("10"); // NOI18N
            _numIterativeScansTextField.addKeyListener(new java.awt.event.KeyAdapter() {
                public void keyTyped(java.awt.event.KeyEvent e) {
                    Utils.validatePostiveNumericKeyTyped(_numIterativeScansTextField, e, 5);
                }
            });
            _numIterativeScansTextField.addFocusListener(new java.awt.event.FocusAdapter() {
                public void focusLost(java.awt.event.FocusEvent e) {
                    while (_numIterativeScansTextField.getText().length() == 0 ||
                        Integer.parseInt(_numIterativeScansTextField.getText()) < 1 ||
                        Integer.parseInt(_numIterativeScansTextField.getText()) > Parameters.MAXIMUM_ITERATIVE_ANALYSES) {
                        if (undo.canUndo()) undo.undo(); else _numIterativeScansTextField.setText("10");
                    }
                    enableSetDefaultsButton();
                }
            });
            _numIterativeScansTextField.getDocument().addUndoableEditListener(new UndoableEditListener() {
                public void undoableEditHappened(UndoableEditEvent evt) {
                    undo.addEdit(evt.getEdit());
                }
            });

            _iterativeCutoffLabel.setLabelFor(_iterativeScanCutoffTextField);
            _iterativeCutoffLabel.setText("Stop when the p-value is greater than:"); // NOI18N

            _iterativeScanCutoffTextField.setText("0.05"); // NOI18N
            _iterativeScanCutoffTextField.addKeyListener(new java.awt.event.KeyAdapter() {
                public void keyTyped(java.awt.event.KeyEvent e) {
                    Utils.validatePostiveFloatKeyTyped(_iterativeScanCutoffTextField, e, 20);
                }
            });
            _iterativeScanCutoffTextField.addFocusListener(new java.awt.event.FocusAdapter() {
                public void focusLost(java.awt.event.FocusEvent e) {
                    while (_iterativeScanCutoffTextField.getText().length() == 0 ||
                        Double.parseDouble(_iterativeScanCutoffTextField.getText()) <= 0 ||
                        Double.parseDouble(_iterativeScanCutoffTextField.getText()) > 1)
                    if (undo.canUndo()) undo.undo(); else _iterativeScanCutoffTextField.setText(".05");
                    enableSetDefaultsButton();
                }
            });
            _iterativeScanCutoffTextField.getDocument().addUndoableEditListener(new UndoableEditListener() {
                public void undoableEditHappened(UndoableEditEvent evt) {
                    undo.addEdit(evt.getEdit());
                }
            });

            javax.swing.GroupLayout _iterativeScanGroupLayout = new javax.swing.GroupLayout(_iterativeScanGroup);
            _iterativeScanGroup.setLayout(_iterativeScanGroupLayout);
            _iterativeScanGroupLayout.setHorizontalGroup(
                _iterativeScanGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_iterativeScanGroupLayout.createSequentialGroup()
                    .addContainerGap()
                    .addGroup(_iterativeScanGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                        .addGroup(_iterativeScanGroupLayout.createSequentialGroup()
                            .addGap(17, 17, 17)
                            .addComponent(_maxIterativeScansLabel)
                            .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                            .addComponent(_numIterativeScansTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 70, javax.swing.GroupLayout.PREFERRED_SIZE)
                            .addGap(18, 18, 18)
                            .addComponent(_iterativeCutoffLabel)
                            .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                            .addComponent(_iterativeScanCutoffTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 70, javax.swing.GroupLayout.PREFERRED_SIZE))
                        .addComponent(_performIterativeScanCheckBox, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                    .addContainerGap())
            );
            _iterativeScanGroupLayout.setVerticalGroup(
                _iterativeScanGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_iterativeScanGroupLayout.createSequentialGroup()
                    .addComponent(_performIterativeScanCheckBox)
                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                    .addGroup(_iterativeScanGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                        .addComponent(_maxIterativeScansLabel)
                        .addComponent(_numIterativeScansTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addComponent(_iterativeCutoffLabel)
                        .addComponent(_iterativeScanCutoffTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                    .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
            );

            _monteCarloGroup.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createEtchedBorder(), "Monte Carlo Replications"));

            _labelMonteCarloReplications.setLabelFor(_montCarloReplicationsTextField);
            _labelMonteCarloReplications.setText("Number of replications (0, 9, 999, or value ending in 999):"); // NOI18N

            _montCarloReplicationsTextField.setText("999"); // NOI18N
            _montCarloReplicationsTextField.addFocusListener(new java.awt.event.FocusAdapter() {
                public void focusLost(java.awt.event.FocusEvent e) {
                    while (_montCarloReplicationsTextField.getText().length() == 0)
                    if (undo.canUndo()) undo.undo(); else _montCarloReplicationsTextField.setText("999");
                    enableSetDefaultsButton();
                    updateCriticalValuesTextCaptions();
                }
            });
            _montCarloReplicationsTextField.addKeyListener(new java.awt.event.KeyAdapter() {
                public void keyTyped(java.awt.event.KeyEvent e) {
                    Utils.validatePostiveNumericKeyTyped(_montCarloReplicationsTextField, e, 10);
                }
            });
            _montCarloReplicationsTextField.getDocument().addUndoableEditListener(new UndoableEditListener() {
                public void undoableEditHappened(UndoableEditEvent evt) {
                    undo.addEdit(evt.getEdit());
                }
            });

            javax.swing.GroupLayout _monteCarloGroupLayout = new javax.swing.GroupLayout(_monteCarloGroup);
            _monteCarloGroup.setLayout(_monteCarloGroupLayout);
            _monteCarloGroupLayout.setHorizontalGroup(
                _monteCarloGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_monteCarloGroupLayout.createSequentialGroup()
                    .addContainerGap()
                    .addComponent(_labelMonteCarloReplications)
                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                    .addComponent(_montCarloReplicationsTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 80, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
            );
            _monteCarloGroupLayout.setVerticalGroup(
                _monteCarloGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_monteCarloGroupLayout.createSequentialGroup()
                    .addGroup(_monteCarloGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                        .addComponent(_labelMonteCarloReplications)
                        .addComponent(_montCarloReplicationsTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                    .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
            );

            javax.swing.GroupLayout _inferenceTabLayout = new javax.swing.GroupLayout(_inferenceTab);
            _inferenceTab.setLayout(_inferenceTabLayout);
            _inferenceTabLayout.setHorizontalGroup(
                _inferenceTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_inferenceTabLayout.createSequentialGroup()
                    .addContainerGap()
                    .addGroup(_inferenceTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                        .addComponent(_pValueOptionsGroup, javax.swing.GroupLayout.Alignment.TRAILING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                        .addComponent(_monteCarloGroup, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                        .addComponent(_iterativeScanGroup, javax.swing.GroupLayout.Alignment.TRAILING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                    .addContainerGap())
            );
            _inferenceTabLayout.setVerticalGroup(
                _inferenceTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_inferenceTabLayout.createSequentialGroup()
                    .addContainerGap()
                    .addComponent(_pValueOptionsGroup, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                    .addComponent(_monteCarloGroup, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                    .addComponent(_iterativeScanGroup, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addContainerGap(196, Short.MAX_VALUE))
            );

            _pValueOptionsGroup.getAccessibleContext().setAccessibleName("P-Value options");

            jTabbedPane1.addTab("Inference", _inferenceTab);
            _inferenceTab.getAccessibleContext().setAccessibleName("Inference tab");

            _mapsOutputGroup.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createEtchedBorder(), "Maps"));

            _includeClusterLocationsInKML.setText("Include All Location IDs in the Clusters");
            _includeClusterLocationsInKML.addItemListener(new java.awt.event.ItemListener() {
                public void itemStateChanged(java.awt.event.ItemEvent e) {
                    enableSetDefaultsButton();
                }
            });

            _createCompressedKMZ.setText("Create compressed KMZ file");
            _createCompressedKMZ.addItemListener(new java.awt.event.ItemListener() {
                public void itemStateChanged(java.awt.event.ItemEvent e) {
                    enableSetDefaultsButton();
                }
            });

            _launch_map_viewer.setText("Automatically launch requested maps");
            _launch_map_viewer.addItemListener(new java.awt.event.ItemListener() {
                public void itemStateChanged(java.awt.event.ItemEvent e) {
                    enableSetDefaultsButton();
                }
            });

            _label_kml_options.setText("KML Options:");

            javax.swing.GroupLayout _mapsOutputGroupLayout = new javax.swing.GroupLayout(_mapsOutputGroup);
            _mapsOutputGroup.setLayout(_mapsOutputGroupLayout);
            _mapsOutputGroupLayout.setHorizontalGroup(
                _mapsOutputGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, _mapsOutputGroupLayout.createSequentialGroup()
                    .addContainerGap()
                    .addComponent(_launch_map_viewer, javax.swing.GroupLayout.PREFERRED_SIZE, 279, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addGap(9, 9, 9)
                    .addGroup(_mapsOutputGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                        .addGroup(_mapsOutputGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                            .addComponent(_createCompressedKMZ, javax.swing.GroupLayout.Alignment.TRAILING, javax.swing.GroupLayout.DEFAULT_SIZE, 332, Short.MAX_VALUE)
                            .addComponent(_label_kml_options, javax.swing.GroupLayout.Alignment.TRAILING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                        .addComponent(_includeClusterLocationsInKML, javax.swing.GroupLayout.PREFERRED_SIZE, 332, javax.swing.GroupLayout.PREFERRED_SIZE))
                    .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
            );
            _mapsOutputGroupLayout.setVerticalGroup(
                _mapsOutputGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, _mapsOutputGroupLayout.createSequentialGroup()
                    .addGap(0, 0, Short.MAX_VALUE)
                    .addComponent(_label_kml_options)
                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                    .addComponent(_createCompressedKMZ)
                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                    .addComponent(_includeClusterLocationsInKML)
                    .addGap(5, 5, 5))
                .addGroup(_mapsOutputGroupLayout.createSequentialGroup()
                    .addComponent(_launch_map_viewer)
                    .addContainerGap())
            );

            _clustersReportedGroup.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createEtchedBorder(), "Criteria for Reporting Secondary Clusters"));

            _hierarchicalSecondaryClusters.addItemListener(new java.awt.event.ItemListener() {
                public void itemStateChanged(java.awt.event.ItemEvent e) {
                    enableSetDefaultsButton();
                }
            });
            _hierarchicalSecondaryClusters.add("No Geographical Overlap");
            _hierarchicalSecondaryClusters.add("No Cluster Centers in Other Clusters");
            _hierarchicalSecondaryClusters.add("No Cluster Centers in More Likely Clusters");
            _hierarchicalSecondaryClusters.add("No Cluster Centers in Less Likely Clusters");
            _hierarchicalSecondaryClusters.add("No Pairs of Centers Both in Each Others Clusters");
            _hierarchicalSecondaryClusters.add("No Restrictions = Most Likely Cluster for Each Grid Point");

            _hierarchicalLabel.setLabelFor(_hierarchicalSecondaryClusters);
            _hierarchicalLabel.setText("Criteria for Reporting Hierarchical Clusters");

            _checkboxReportIndexCoefficients.setText("Report Gini indexes in results file");
            _checkboxReportIndexCoefficients.addItemListener(new java.awt.event.ItemListener() {
                public void itemStateChanged(java.awt.event.ItemEvent e) {
                    enableSetDefaultsButton();
                }
            });

            _mostLikelyClustersHierarchically.setText("Most Likely Clusters, Hierarchically");
            _mostLikelyClustersHierarchically.addItemListener(new java.awt.event.ItemListener() {
                public void itemStateChanged(java.awt.event.ItemEvent e) {
                    enableClustersReportedOptions(_clustersReportedGroup.isEnabled());
                    enableSetDefaultsButton();
                }
            });

            _giniOptimizedClusters.setText("Gini Optimized Cluster Collection");
            _giniOptimizedClusters.addItemListener(new java.awt.event.ItemListener() {
                public void itemStateChanged(java.awt.event.ItemEvent e) {
                    enableClustersReportedOptions(_clustersReportedGroup.isEnabled());
                    enableSetDefaultsButton();
                }
            });

            javax.swing.GroupLayout _clustersReportedGroupLayout = new javax.swing.GroupLayout(_clustersReportedGroup);
            _clustersReportedGroup.setLayout(_clustersReportedGroupLayout);
            _clustersReportedGroupLayout.setHorizontalGroup(
                _clustersReportedGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_clustersReportedGroupLayout.createSequentialGroup()
                    .addContainerGap()
                    .addGroup(_clustersReportedGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                        .addComponent(_mostLikelyClustersHierarchically, javax.swing.GroupLayout.DEFAULT_SIZE, 243, Short.MAX_VALUE)
                        .addComponent(_giniOptimizedClusters, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                    .addGroup(_clustersReportedGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING)
                        .addComponent(_checkboxReportIndexCoefficients, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                        .addGroup(javax.swing.GroupLayout.Alignment.LEADING, _clustersReportedGroupLayout.createSequentialGroup()
                            .addGroup(_clustersReportedGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING)
                                .addComponent(_hierarchicalSecondaryClusters, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.PREFERRED_SIZE, 248, javax.swing.GroupLayout.PREFERRED_SIZE)
                                .addComponent(_hierarchicalLabel, javax.swing.GroupLayout.Alignment.LEADING))
                            .addGap(0, 0, Short.MAX_VALUE)))
                    .addContainerGap())
            );
            _clustersReportedGroupLayout.setVerticalGroup(
                _clustersReportedGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_clustersReportedGroupLayout.createSequentialGroup()
                    .addGroup(_clustersReportedGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                        .addComponent(_mostLikelyClustersHierarchically)
                        .addComponent(_hierarchicalLabel))
                    .addGap(0, 0, 0)
                    .addComponent(_hierarchicalSecondaryClusters, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                    .addGroup(_clustersReportedGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                        .addComponent(_giniOptimizedClusters)
                        .addComponent(_checkboxReportIndexCoefficients))
                    .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
            );

            _hierarchicalSecondaryClusters.getAccessibleContext().setAccessibleName("Criteria for Reporting Hierarchical Clusters");
            _hierarchicalLabel.getAccessibleContext().setAccessibleDescription("");

            _reportedSpatialOptionsGroup.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createEtchedBorder(), "Maximum Reported Spatial Cluster Size"));

            _restrictReportedClustersCheckBox.setText("Report only clusters smaller than:"); // NOI18N
            _restrictReportedClustersCheckBox.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
            _restrictReportedClustersCheckBox.setMargin(new java.awt.Insets(0, 0, 0, 0));
            _restrictReportedClustersCheckBox.addItemListener(new java.awt.event.ItemListener() {
                public void itemStateChanged(java.awt.event.ItemEvent e) {
                    enableReportedSpatialOptionsGroup(_spatialOptionsGroup.isEnabled());
                    enableSetDefaultsButton();
                }
            });

            _maxReportedSpatialClusterSizeTextField.setText("50"); // NOI18N
            _maxReportedSpatialClusterSizeTextField.addKeyListener(new java.awt.event.KeyAdapter() {
                public void keyTyped(java.awt.event.KeyEvent e) {
                    Utils.validatePostiveFloatKeyTyped(_maxReportedSpatialClusterSizeTextField, e, 5);
                }
            });
            _maxReportedSpatialClusterSizeTextField.addFocusListener(new java.awt.event.FocusAdapter() {
                public void focusLost(java.awt.event.FocusEvent e) {
                    while (_maxReportedSpatialClusterSizeTextField.getText().length() == 0 || Double.parseDouble(_maxReportedSpatialClusterSizeTextField.getText()) == 0) {
                        if (undo.canUndo()) undo.undo(); else _maxReportedSpatialClusterSizeTextField.setText("50");
                    }
                    enableSetDefaultsButton();
                }
            });
            _maxReportedSpatialClusterSizeTextField.getDocument().addUndoableEditListener(new UndoableEditListener() {
                public void undoableEditHappened(UndoableEditEvent evt) {
                    undo.addEdit(evt.getEdit());
                }
            });

            _reportedPercentOfPopulationLabel.setLabelFor(_maxReportedSpatialClusterSizeTextField);
            _reportedPercentOfPopulationLabel.setText("percent of the population at risk (<= 50%, default = 50%)"); // NOI18N

            _reportedSpatialPopulationFileCheckBox.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
            _reportedSpatialPopulationFileCheckBox.setMargin(new java.awt.Insets(0, 0, 0, 0));
            _reportedSpatialPopulationFileCheckBox.addItemListener(new java.awt.event.ItemListener() {
                public void itemStateChanged(java.awt.event.ItemEvent e) {
                    enableReportedSpatialOptionsGroup(_spatialOptionsGroup.isEnabled());
                    enableSetDefaultsButton();
                }
            });

            _maxReportedSpatialPercentFileTextField.setText("50"); // NOI18N
            _maxReportedSpatialPercentFileTextField.addKeyListener(new java.awt.event.KeyAdapter() {
                public void keyTyped(java.awt.event.KeyEvent e) {
                    Utils.validatePostiveFloatKeyTyped(_maxReportedSpatialPercentFileTextField, e, 5);
                }
            });
            _maxReportedSpatialPercentFileTextField.addFocusListener(new java.awt.event.FocusAdapter() {
                public void focusLost(java.awt.event.FocusEvent e) {
                    while (_maxReportedSpatialPercentFileTextField.getText().length() == 0 || Double.parseDouble(_maxReportedSpatialPercentFileTextField.getText()) == 0) {
                        if (undo.canUndo()) undo.undo(); else _maxReportedSpatialPercentFileTextField.setText("50");
                    }
                    enableSetDefaultsButton();
                }
            });
            _maxReportedSpatialPercentFileTextField.getDocument().addUndoableEditListener(new UndoableEditListener() {
                public void undoableEditHappened(UndoableEditEvent evt) {
                    undo.addEdit(evt.getEdit());
                }
            });

            _reportedPercentageOfPopFileLabel.setLabelFor(_maxReportedSpatialPercentFileTextField);
            _reportedPercentageOfPopFileLabel.setText("percent of the population defined in the max circle size file (<= 50%)"); // NOI18N

            _reportedSpatialDistanceCheckBox.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
            _reportedSpatialDistanceCheckBox.setMargin(new java.awt.Insets(0, 0, 0, 0));
            _reportedSpatialDistanceCheckBox.addItemListener(new java.awt.event.ItemListener() {
                public void itemStateChanged(java.awt.event.ItemEvent e) {
                    enableReportedSpatialOptionsGroup(_spatialOptionsGroup.isEnabled());
                    enableSetDefaultsButton();
                }
            });

            _reportedMaxDistanceLabel.setText("is an ellipse with a"); // NOI18N

            _maxReportedSpatialRadiusTextField.setText("1"); // NOI18N
            _maxReportedSpatialRadiusTextField.addKeyListener(new java.awt.event.KeyAdapter() {
                public void keyTyped(java.awt.event.KeyEvent e) {
                    Utils.validatePostiveFloatKeyTyped(_maxReportedSpatialRadiusTextField, e, 20);
                }
            });
            _maxReportedSpatialRadiusTextField.addFocusListener(new java.awt.event.FocusAdapter() {
                public void focusLost(java.awt.event.FocusEvent e) {
                    while (_maxReportedSpatialRadiusTextField.getText().length() == 0 || Double.parseDouble(_maxReportedSpatialRadiusTextField.getText()) == 0) {
                        if (undo.canUndo()) undo.undo(); else _maxReportedSpatialRadiusTextField.setText("1");
                    }
                    enableSetDefaultsButton();
                }
            });
            _maxReportedSpatialRadiusTextField.getDocument().addUndoableEditListener(new UndoableEditListener() {
                public void undoableEditHappened(UndoableEditEvent evt) {
                    undo.addEdit(evt.getEdit());
                }
            });

            _maxReportedRadiusLabel.setLabelFor(_maxReportedSpatialRadiusTextField);
            _maxReportedRadiusLabel.setText("kilometer radius"); // NOI18N

            javax.swing.GroupLayout _reportedSpatialOptionsGroupLayout = new javax.swing.GroupLayout(_reportedSpatialOptionsGroup);
            _reportedSpatialOptionsGroup.setLayout(_reportedSpatialOptionsGroupLayout);
            _reportedSpatialOptionsGroupLayout.setHorizontalGroup(
                _reportedSpatialOptionsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, _reportedSpatialOptionsGroupLayout.createSequentialGroup()
                    .addGroup(_reportedSpatialOptionsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING)
                        .addGroup(_reportedSpatialOptionsGroupLayout.createSequentialGroup()
                            .addContainerGap()
                            .addComponent(_restrictReportedClustersCheckBox, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                        .addGroup(_reportedSpatialOptionsGroupLayout.createSequentialGroup()
                            .addGap(27, 27, 27)
                            .addGroup(_reportedSpatialOptionsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                                .addGroup(_reportedSpatialOptionsGroupLayout.createSequentialGroup()
                                    .addComponent(_reportedSpatialPopulationFileCheckBox)
                                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                    .addComponent(_maxReportedSpatialPercentFileTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 70, javax.swing.GroupLayout.PREFERRED_SIZE)
                                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                    .addComponent(_reportedPercentageOfPopFileLabel, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                                .addGroup(_reportedSpatialOptionsGroupLayout.createSequentialGroup()
                                    .addComponent(_maxReportedSpatialClusterSizeTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 70, javax.swing.GroupLayout.PREFERRED_SIZE)
                                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                    .addComponent(_reportedPercentOfPopulationLabel, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                                .addGroup(_reportedSpatialOptionsGroupLayout.createSequentialGroup()
                                    .addComponent(_reportedSpatialDistanceCheckBox)
                                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                    .addComponent(_reportedMaxDistanceLabel)
                                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                    .addComponent(_maxReportedSpatialRadiusTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 70, javax.swing.GroupLayout.PREFERRED_SIZE)
                                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                    .addComponent(_maxReportedRadiusLabel, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)))))
                    .addContainerGap())
            );
            _reportedSpatialOptionsGroupLayout.setVerticalGroup(
                _reportedSpatialOptionsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_reportedSpatialOptionsGroupLayout.createSequentialGroup()
                    .addContainerGap()
                    .addComponent(_restrictReportedClustersCheckBox)
                    .addGap(10, 10, 10)
                    .addGroup(_reportedSpatialOptionsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                        .addComponent(_maxReportedSpatialClusterSizeTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addComponent(_reportedPercentOfPopulationLabel))
                    .addGap(10, 10, 10)
                    .addGroup(_reportedSpatialOptionsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                        .addComponent(_reportedSpatialPopulationFileCheckBox)
                        .addComponent(_maxReportedSpatialPercentFileTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addComponent(_reportedPercentageOfPopFileLabel))
                    .addGap(10, 10, 10)
                    .addGroup(_reportedSpatialOptionsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                        .addComponent(_reportedSpatialDistanceCheckBox)
                        .addGroup(_reportedSpatialOptionsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                            .addComponent(_reportedMaxDistanceLabel)
                            .addComponent(_maxReportedSpatialRadiusTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                            .addComponent(_maxReportedRadiusLabel)))
                    .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
            );

            _reportedSpatialPopulationFileCheckBox.getAccessibleContext().setAccessibleName("selection for spatial maximum by population in maximum circle size file");
            _reportedSpatialDistanceCheckBox.getAccessibleContext().setAccessibleName("selection for spatial maximum by distance");

            javax.swing.GroupLayout _spatialOutputTabLayout = new javax.swing.GroupLayout(_spatialOutputTab);
            _spatialOutputTab.setLayout(_spatialOutputTabLayout);
            _spatialOutputTabLayout.setHorizontalGroup(
                _spatialOutputTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_spatialOutputTabLayout.createSequentialGroup()
                    .addContainerGap()
                    .addGroup(_spatialOutputTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                        .addComponent(_mapsOutputGroup, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                        .addComponent(_clustersReportedGroup, javax.swing.GroupLayout.Alignment.TRAILING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                        .addComponent(_reportedSpatialOptionsGroup, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                    .addContainerGap())
            );
            _spatialOutputTabLayout.setVerticalGroup(
                _spatialOutputTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_spatialOutputTabLayout.createSequentialGroup()
                    .addContainerGap()
                    .addComponent(_mapsOutputGroup, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                    .addComponent(_clustersReportedGroup, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                    .addComponent(_reportedSpatialOptionsGroup, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addContainerGap(98, Short.MAX_VALUE))
            );

            jTabbedPane1.addTab("Spatial Output", _spatialOutputTab);
            _spatialOutputTab.getAccessibleContext().setAccessibleName("Spatial Output tab");

            _reportCriticalValuesGroup.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createEtchedBorder(), "Critical Values"));

            _reportCriticalValuesCheckBox.setText("Report critical values for an observed cluster to be significant"); // NOI18N
            _reportCriticalValuesCheckBox.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
            _reportCriticalValuesCheckBox.setMargin(new java.awt.Insets(0, 0, 0, 0));
            _reportCriticalValuesCheckBox.addItemListener(new java.awt.event.ItemListener() {
                public void itemStateChanged(java.awt.event.ItemEvent e) {
                    enableSetDefaultsButton();
                }
            });

            javax.swing.GroupLayout _reportCriticalValuesGroupLayout = new javax.swing.GroupLayout(_reportCriticalValuesGroup);
            _reportCriticalValuesGroup.setLayout(_reportCriticalValuesGroupLayout);
            _reportCriticalValuesGroupLayout.setHorizontalGroup(
                _reportCriticalValuesGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_reportCriticalValuesGroupLayout.createSequentialGroup()
                    .addContainerGap()
                    .addComponent(_reportCriticalValuesCheckBox, javax.swing.GroupLayout.DEFAULT_SIZE, 669, Short.MAX_VALUE)
                    .addContainerGap())
            );
            _reportCriticalValuesGroupLayout.setVerticalGroup(
                _reportCriticalValuesGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_reportCriticalValuesGroupLayout.createSequentialGroup()
                    .addContainerGap()
                    .addComponent(_reportCriticalValuesCheckBox)
                    .addContainerGap(14, Short.MAX_VALUE))
            );

            _reportClusterRankGroup.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createEtchedBorder(), "Monte Carlo Rank"));

            _reportClusterRankCheckBox.setText("Report Monte Carlo Rank"); // NOI18N
            _reportClusterRankCheckBox.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
            _reportClusterRankCheckBox.setMargin(new java.awt.Insets(0, 0, 0, 0));
            _reportCriticalValuesCheckBox.addItemListener(new java.awt.event.ItemListener() {
                public void itemStateChanged(java.awt.event.ItemEvent e) {
                    enableSetDefaultsButton();
                }
            });

            javax.swing.GroupLayout _reportClusterRankGroupLayout = new javax.swing.GroupLayout(_reportClusterRankGroup);
            _reportClusterRankGroup.setLayout(_reportClusterRankGroupLayout);
            _reportClusterRankGroupLayout.setHorizontalGroup(
                _reportClusterRankGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_reportClusterRankGroupLayout.createSequentialGroup()
                    .addContainerGap()
                    .addComponent(_reportClusterRankCheckBox, javax.swing.GroupLayout.DEFAULT_SIZE, 620, Short.MAX_VALUE)
                    .addContainerGap())
            );
            _reportClusterRankGroupLayout.setVerticalGroup(
                _reportClusterRankGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_reportClusterRankGroupLayout.createSequentialGroup()
                    .addContainerGap()
                    .addComponent(_reportClusterRankCheckBox)
                    .addContainerGap(14, Short.MAX_VALUE))
            );

            _additionalOutputFiles.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createEtchedBorder(), "Column Headers"));

            _printAsciiColumnHeaders.setText("Print column headers in ASCII output files"); // NOI18N
            _printAsciiColumnHeaders.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
            _printAsciiColumnHeaders.setMargin(new java.awt.Insets(0, 0, 0, 0));
            _reportCriticalValuesCheckBox.addItemListener(new java.awt.event.ItemListener() {
                public void itemStateChanged(java.awt.event.ItemEvent e) {
                    enableSetDefaultsButton();
                }
            });

            javax.swing.GroupLayout _additionalOutputFilesLayout = new javax.swing.GroupLayout(_additionalOutputFiles);
            _additionalOutputFiles.setLayout(_additionalOutputFilesLayout);
            _additionalOutputFilesLayout.setHorizontalGroup(
                _additionalOutputFilesLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_additionalOutputFilesLayout.createSequentialGroup()
                    .addContainerGap()
                    .addComponent(_printAsciiColumnHeaders, javax.swing.GroupLayout.DEFAULT_SIZE, 620, Short.MAX_VALUE)
                    .addContainerGap())
            );
            _additionalOutputFilesLayout.setVerticalGroup(
                _additionalOutputFilesLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_additionalOutputFilesLayout.createSequentialGroup()
                    .addContainerGap()
                    .addComponent(_printAsciiColumnHeaders)
                    .addContainerGap(14, Short.MAX_VALUE))
            );

            _userDefinedRunTitle.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createEtchedBorder(), "Title for Results File"));

            javax.swing.GroupLayout _userDefinedRunTitleLayout = new javax.swing.GroupLayout(_userDefinedRunTitle);
            _userDefinedRunTitle.setLayout(_userDefinedRunTitleLayout);
            _userDefinedRunTitleLayout.setHorizontalGroup(
                _userDefinedRunTitleLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_userDefinedRunTitleLayout.createSequentialGroup()
                    .addContainerGap()
                    .addComponent(_printTitle)
                    .addContainerGap())
            );
            _userDefinedRunTitleLayout.setVerticalGroup(
                _userDefinedRunTitleLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, _userDefinedRunTitleLayout.createSequentialGroup()
                    .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(_printTitle, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addContainerGap())
            );

            _printTitle.getAccessibleContext().setAccessibleName("title for results files");

            _cluster_lineline_panel.setBorder(javax.swing.BorderFactory.createTitledBorder("Cluster Line List:"));

            _cluster_lineline_value.setText("1");

            _cluster_lineline_label.setText("in line list CSV file.");

            _cluster_lineline_prelabel.setText("Include clusters with p-value less than or equal to");

            javax.swing.GroupLayout _cluster_lineline_panelLayout = new javax.swing.GroupLayout(_cluster_lineline_panel);
            _cluster_lineline_panel.setLayout(_cluster_lineline_panelLayout);
            _cluster_lineline_panelLayout.setHorizontalGroup(
                _cluster_lineline_panelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_cluster_lineline_panelLayout.createSequentialGroup()
                    .addContainerGap()
                    .addComponent(_cluster_lineline_prelabel)
                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                    .addComponent(_cluster_lineline_value, javax.swing.GroupLayout.PREFERRED_SIZE, 80, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                    .addComponent(_cluster_lineline_label, javax.swing.GroupLayout.PREFERRED_SIZE, 211, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
            );
            _cluster_lineline_panelLayout.setVerticalGroup(
                _cluster_lineline_panelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_cluster_lineline_panelLayout.createSequentialGroup()
                    .addContainerGap()
                    .addGroup(_cluster_lineline_panelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                        .addComponent(_cluster_lineline_value, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addComponent(_cluster_lineline_label)
                        .addComponent(_cluster_lineline_prelabel))
                    .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
            );

            _cluster_lineline_value.getAccessibleContext().setAccessibleName("Include clusters in line list CSV file meeting cutoff");

            javax.swing.GroupLayout _otherOutputTabLayout = new javax.swing.GroupLayout(_otherOutputTab);
            _otherOutputTab.setLayout(_otherOutputTabLayout);
            _otherOutputTabLayout.setHorizontalGroup(
                _otherOutputTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, _otherOutputTabLayout.createSequentialGroup()
                    .addContainerGap()
                    .addGroup(_otherOutputTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING)
                        .addComponent(_cluster_lineline_panel, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                        .addComponent(_reportCriticalValuesGroup, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                        .addComponent(_reportClusterRankGroup, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                        .addComponent(_additionalOutputFiles, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                        .addComponent(_userDefinedRunTitle, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                    .addContainerGap())
            );
            _otherOutputTabLayout.setVerticalGroup(
                _otherOutputTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, _otherOutputTabLayout.createSequentialGroup()
                    .addContainerGap()
                    .addComponent(_reportCriticalValuesGroup, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                    .addComponent(_reportClusterRankGroup, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                    .addComponent(_additionalOutputFiles, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                    .addComponent(_userDefinedRunTitle, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                    .addComponent(_cluster_lineline_panel, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addContainerGap(144, Short.MAX_VALUE))
            );

            jTabbedPane1.addTab("Other Output", _otherOutputTab);
            _otherOutputTab.getAccessibleContext().setAccessibleName("Other Output tab");

            _powerEvaluationsGroup.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createEtchedBorder(), "Statistical Power Evaluation"));

            _powerEvaluationMethodButtonGroup.add(_partOfRegularAnalysis);
            _partOfRegularAnalysis.setSelected(true);
            _partOfRegularAnalysis.setText("As Part of Regular Analysis");
            _partOfRegularAnalysis.addItemListener(new java.awt.event.ItemListener() {
                public void itemStateChanged(java.awt.event.ItemEvent e) {
                    enablePowerEvaluationsGroup();
                    enableSetDefaultsButton();
                }
            });

            _powerEvaluationMethodButtonGroup.add(_powerEvaluationWithCaseFile);
            _powerEvaluationWithCaseFile.setText("Power Evaluation Only, Use Total Cases From Case File");
            _powerEvaluationWithCaseFile.addItemListener(new java.awt.event.ItemListener() {
                public void itemStateChanged(java.awt.event.ItemEvent e) {
                    enablePowerEvaluationsGroup();
                    enableSetDefaultsButton();
                }
            });

            _powerEvaluationMethodButtonGroup.add(_powerEvaluationWithSpecifiedCases);
            _powerEvaluationWithSpecifiedCases.setText("Power Evaluation Only, Use ");
            _powerEvaluationWithSpecifiedCases.addItemListener(new java.awt.event.ItemListener() {
                public void itemStateChanged(java.awt.event.ItemEvent e) {
                    enablePowerEvaluationsGroup();
                    enableSetDefaultsButton();
                }
            });

            _totalPowerCases.setText("600");
            _totalPowerCases.addFocusListener(new java.awt.event.FocusAdapter() {
                public void focusLost(java.awt.event.FocusEvent e) {
                    while (_totalPowerCases.getText().length() == 0)
                    if (undo.canUndo()) undo.undo(); else _totalPowerCases.setText("600");
                    enableSetDefaultsButton();
                }
            });
            _totalPowerCases.addKeyListener(new java.awt.event.KeyAdapter() {
                public void keyTyped(java.awt.event.KeyEvent e) {
                    Utils.validatePostiveNumericKeyTyped(_totalPowerCases, e, 10);
                }
            });
            _totalPowerCases.getDocument().addUndoableEditListener(new UndoableEditListener() {
                public void undoableEditHappened(UndoableEditEvent evt) {
                    undo.addEdit(evt.getEdit());
                }
            });

            _powerEvaluationWithSpecifiedCasesLabel.setText("Total Cases");

            _alternativeHypothesisFilenameLabel.setLabelFor(_alternativeHypothesisFilename);
            _alternativeHypothesisFilenameLabel.setText("Alternative Hypothesis File:"); // NOI18N

            _alternativeHypothesisFilename.addFocusListener(new java.awt.event.FocusAdapter() {
                public void focusLost(java.awt.event.FocusEvent e) {
                    enableSetDefaultsButton();
                }
            });

            _alternativeHypothesisFilenameButton.setText("..."); // NOI18N
            _alternativeHypothesisFilenameButton.setToolTipText("Open Alternative Hypothesis File Import Wizard"); // NOI18N
            _alternativeHypothesisFilenameButton.addActionListener(new java.awt.event.ActionListener() {
                public void actionPerformed(java.awt.event.ActionEvent e) {
                    InputSourceSettings inputSourceSettings = new InputSourceSettings(InputSourceSettings.InputFileType.AlternativeHypothesis);
                    // invoke the FileSelectionDialog to guide user through process of selecting the source file.
                    FileSelectionDialog selectionDialog = new FileSelectionDialog(SaTScanApplication.getInstance(), inputSourceSettings.getInputFileType(), SaTScanApplication.getInstance().lastBrowseDirectory);
                    selectionDialog.browse_inputsource(_alternativeHypothesisFilename, inputSourceSettings, _settings_window);
                }
            });

            _numberPowerReplicationsLabel.setLabelFor(_numberPowerReplications);
            _numberPowerReplicationsLabel.setText("Number of replications (100, 1000 or multiple of 100):"); // NOI18N

            _numberPowerReplications.setText("1000"); // NOI18N
            _numberPowerReplications.addFocusListener(new java.awt.event.FocusAdapter() {
                public void focusLost(java.awt.event.FocusEvent e) {
                    while (_numberPowerReplications.getText().length() == 0)
                    if (undo.canUndo()) undo.undo(); else _numberPowerReplications.setText("1000");
                    enableSetDefaultsButton();
                }
            });
            _numberPowerReplications.addKeyListener(new java.awt.event.KeyAdapter() {
                public void keyTyped(java.awt.event.KeyEvent e) {
                    Utils.validatePostiveNumericKeyTyped(_numberPowerReplications, e, 10);
                }
            });
            _numberPowerReplications.getDocument().addUndoableEditListener(new UndoableEditListener() {
                public void undoableEditHappened(UndoableEditEvent evt) {
                    undo.addEdit(evt.getEdit());
                }
            });

            _criticalValuesTypeLabel.setText("Critical Values");

            _criticalValuesButtonGroup.add(_criticalValuesMonteCarlo);
            _criticalValuesMonteCarlo.setText("Monte Carlo");
            _criticalValuesMonteCarlo.addItemListener(new java.awt.event.ItemListener() {
                public void itemStateChanged(java.awt.event.ItemEvent e) {
                    enableSetDefaultsButton();
                }
            });

            _criticalValuesButtonGroup.add(_criticalValuesGumbel);
            _criticalValuesGumbel.setText("Gumbel");
            _criticalValuesGumbel.addItemListener(new java.awt.event.ItemListener() {
                public void itemStateChanged(java.awt.event.ItemEvent e) {
                    enableSetDefaultsButton();
                }
            });

            _criticalValuesReplicationsLabel.setFont(new java.awt.Font("Tahoma", 2, 11)); // NOI18N
            _criticalValuesReplicationsLabel.setText("(999 replications - value defined on Inference tab)");

            _powerEstimationTypeLabel.setText("Power Estimation");

            _powerEstimationButtonGroup.add(_powerEstimationMonteCarlo);
            _powerEstimationMonteCarlo.setText("Monte Carlo");
            _powerEstimationMonteCarlo.addItemListener(new java.awt.event.ItemListener() {
                public void itemStateChanged(java.awt.event.ItemEvent e) {
                    enableSetDefaultsButton();
                }
            });

            _powerEstimationButtonGroup.add(_powerEstimationGumbel);
            _powerEstimationGumbel.setText("Gumbel");
            _powerEstimationGumbel.addItemListener(new java.awt.event.ItemListener() {
                public void itemStateChanged(java.awt.event.ItemEvent e) {
                    enableSetDefaultsButton();
                }
            });

            _performPowerEvalautions.setText("Perform Power Evaluations");
            _performPowerEvalautions.addItemListener(new java.awt.event.ItemListener() {
                public void itemStateChanged(java.awt.event.ItemEvent e) {
                    enablePowerEvaluationsGroup();
                    enableSetDefaultsButton();
                }
            });

            javax.swing.GroupLayout _powerEvaluationsGroupLayout = new javax.swing.GroupLayout(_powerEvaluationsGroup);
            _powerEvaluationsGroup.setLayout(_powerEvaluationsGroupLayout);
            _powerEvaluationsGroupLayout.setHorizontalGroup(
                _powerEvaluationsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_powerEvaluationsGroupLayout.createSequentialGroup()
                    .addContainerGap()
                    .addGroup(_powerEvaluationsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                        .addGroup(_powerEvaluationsGroupLayout.createSequentialGroup()
                            .addComponent(_powerEstimationTypeLabel, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                            .addContainerGap())
                        .addGroup(_powerEvaluationsGroupLayout.createSequentialGroup()
                            .addGroup(_powerEvaluationsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING)
                                .addGroup(javax.swing.GroupLayout.Alignment.LEADING, _powerEvaluationsGroupLayout.createSequentialGroup()
                                    .addGap(10, 10, 10)
                                    .addGroup(_powerEvaluationsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                                        .addGroup(_powerEvaluationsGroupLayout.createSequentialGroup()
                                            .addComponent(_powerEstimationMonteCarlo, javax.swing.GroupLayout.PREFERRED_SIZE, 122, javax.swing.GroupLayout.PREFERRED_SIZE)
                                            .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                                            .addComponent(_powerEstimationGumbel, javax.swing.GroupLayout.PREFERRED_SIZE, 129, javax.swing.GroupLayout.PREFERRED_SIZE))
                                        .addGroup(_powerEvaluationsGroupLayout.createSequentialGroup()
                                            .addComponent(_numberPowerReplicationsLabel)
                                            .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                            .addComponent(_numberPowerReplications, javax.swing.GroupLayout.PREFERRED_SIZE, 75, javax.swing.GroupLayout.PREFERRED_SIZE)))
                                    .addGap(0, 0, Short.MAX_VALUE))
                                .addGroup(_powerEvaluationsGroupLayout.createSequentialGroup()
                                    .addGroup(_powerEvaluationsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING)
                                        .addComponent(_alternativeHypothesisFilenameLabel, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                                        .addComponent(_alternativeHypothesisFilename))
                                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                    .addComponent(_alternativeHypothesisFilenameButton, javax.swing.GroupLayout.PREFERRED_SIZE, 25, javax.swing.GroupLayout.PREFERRED_SIZE)))
                            .addContainerGap())
                        .addGroup(_powerEvaluationsGroupLayout.createSequentialGroup()
                            .addGroup(_powerEvaluationsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING)
                                .addComponent(_performPowerEvalautions, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                                .addGroup(javax.swing.GroupLayout.Alignment.LEADING, _powerEvaluationsGroupLayout.createSequentialGroup()
                                    .addComponent(_partOfRegularAnalysis, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                                    .addGap(11, 11, 11))
                                .addGroup(_powerEvaluationsGroupLayout.createSequentialGroup()
                                    .addComponent(_powerEvaluationWithCaseFile, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                                    .addGap(11, 11, 11))
                                .addGroup(javax.swing.GroupLayout.Alignment.LEADING, _powerEvaluationsGroupLayout.createSequentialGroup()
                                    .addGap(10, 10, 10)
                                    .addGroup(_powerEvaluationsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                                        .addComponent(_criticalValuesReplicationsLabel, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                                        .addGroup(_powerEvaluationsGroupLayout.createSequentialGroup()
                                            .addComponent(_criticalValuesMonteCarlo, javax.swing.GroupLayout.PREFERRED_SIZE, 122, javax.swing.GroupLayout.PREFERRED_SIZE)
                                            .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                                            .addComponent(_criticalValuesGumbel, javax.swing.GroupLayout.PREFERRED_SIZE, 133, javax.swing.GroupLayout.PREFERRED_SIZE))))
                                .addGroup(_powerEvaluationsGroupLayout.createSequentialGroup()
                                    .addGroup(_powerEvaluationsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING)
                                        .addComponent(_criticalValuesTypeLabel, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                                        .addGroup(javax.swing.GroupLayout.Alignment.LEADING, _powerEvaluationsGroupLayout.createSequentialGroup()
                                            .addComponent(_powerEvaluationWithSpecifiedCases)
                                            .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                            .addComponent(_totalPowerCases, javax.swing.GroupLayout.PREFERRED_SIZE, 78, javax.swing.GroupLayout.PREFERRED_SIZE)
                                            .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                            .addComponent(_powerEvaluationWithSpecifiedCasesLabel, javax.swing.GroupLayout.DEFAULT_SIZE, 366, Short.MAX_VALUE)))
                                    .addGap(17, 17, 17)))
                            .addGap(2, 2, 2))))
            );
            _powerEvaluationsGroupLayout.setVerticalGroup(
                _powerEvaluationsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_powerEvaluationsGroupLayout.createSequentialGroup()
                    .addComponent(_performPowerEvalautions)
                    .addGap(5, 5, 5)
                    .addComponent(_partOfRegularAnalysis)
                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                    .addComponent(_powerEvaluationWithCaseFile)
                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                    .addGroup(_powerEvaluationsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                        .addComponent(_powerEvaluationWithSpecifiedCases)
                        .addComponent(_totalPowerCases, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addComponent(_powerEvaluationWithSpecifiedCasesLabel))
                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                    .addComponent(_criticalValuesTypeLabel)
                    .addGap(2, 2, 2)
                    .addGroup(_powerEvaluationsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                        .addComponent(_criticalValuesMonteCarlo)
                        .addComponent(_criticalValuesGumbel))
                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                    .addComponent(_criticalValuesReplicationsLabel)
                    .addGap(11, 11, 11)
                    .addComponent(_powerEstimationTypeLabel)
                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                    .addGroup(_powerEvaluationsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                        .addComponent(_powerEstimationMonteCarlo)
                        .addComponent(_powerEstimationGumbel))
                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                    .addGroup(_powerEvaluationsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                        .addComponent(_numberPowerReplicationsLabel)
                        .addComponent(_numberPowerReplications, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(_alternativeHypothesisFilenameLabel)
                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                    .addGroup(_powerEvaluationsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                        .addComponent(_alternativeHypothesisFilename, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addComponent(_alternativeHypothesisFilenameButton))
                    .addContainerGap())
            );

            _powerEvaluationWithSpecifiedCases.getAccessibleContext().setAccessibleName("Power Evaluation Only, Use total cases");
            _totalPowerCases.getAccessibleContext().setAccessibleName("total cases in power evaluation");
            _alternativeHypothesisFilenameButton.getAccessibleContext().setAccessibleName("");
            _criticalValuesMonteCarlo.getAccessibleContext().setAccessibleName("Monte Carlo critical values");
            _criticalValuesGumbel.getAccessibleContext().setAccessibleName("Gumbel critical values");
            _powerEstimationMonteCarlo.getAccessibleContext().setAccessibleName("Monte Carlo power estimation");
            _powerEstimationGumbel.getAccessibleContext().setAccessibleName("Gumbel power estimation");

            javax.swing.GroupLayout _powerEvaluationTabLayout = new javax.swing.GroupLayout(_powerEvaluationTab);
            _powerEvaluationTab.setLayout(_powerEvaluationTabLayout);
            _powerEvaluationTabLayout.setHorizontalGroup(
                _powerEvaluationTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_powerEvaluationTabLayout.createSequentialGroup()
                    .addContainerGap()
                    .addComponent(_powerEvaluationsGroup, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addContainerGap())
            );
            _powerEvaluationTabLayout.setVerticalGroup(
                _powerEvaluationTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_powerEvaluationTabLayout.createSequentialGroup()
                    .addContainerGap()
                    .addComponent(_powerEvaluationsGroup, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addContainerGap(122, Short.MAX_VALUE))
            );

            jTabbedPane1.addTab("Power Evaluation", _powerEvaluationTab);
            _powerEvaluationTab.getAccessibleContext().setAccessibleName("Power Evaluation tab");

            _graphOutputGroup.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createEtchedBorder(), "Temporal Graphs"));

            _reportTemporalGraph.setText("Produce Temporal Graphs");
            _reportTemporalGraph.addItemListener(new java.awt.event.ItemListener() {
                public void itemStateChanged(java.awt.event.ItemEvent e) {
                    enableTemporalGraphsGroup(_graphOutputGroup.isEnabled());
                    enableSetDefaultsButton();
                }
            });

            _temporalGraphButtonGroup.add(_temporalGraphMostLikely);
            _temporalGraphMostLikely.setSelected(true);
            _temporalGraphMostLikely.setText("Most likely cluster only");
            _temporalGraphMostLikely.addItemListener(new java.awt.event.ItemListener() {
                public void itemStateChanged(java.awt.event.ItemEvent e) {
                    if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED) {
                        enableTemporalGraphsGroup(_graphOutputGroup.isEnabled());
                        enableSetDefaultsButton();
                    }
                }
            });

            _temporalGraphButtonGroup.add(_temporalGraphMostLikelyX);

            _numMostLikelyClustersGraph.setText("1"); // NOI18N
            _numMostLikelyClustersGraph.addKeyListener(new java.awt.event.KeyAdapter() {
                public void keyTyped(java.awt.event.KeyEvent e) {
                    Utils.validatePostiveFloatKeyTyped(_numMostLikelyClustersGraph, e, 5);
                }
            });
            _numMostLikelyClustersGraph.addFocusListener(new java.awt.event.FocusAdapter() {
                public void focusLost(java.awt.event.FocusEvent e) {
                    while (_numMostLikelyClustersGraph.getText().length() == 0 || Integer.parseInt(_numMostLikelyClustersGraph.getText()) == 0) {
                        if (undo.canUndo()) undo.undo(); else _numMostLikelyClustersGraph.setText("1");
                    }
                    enableSetDefaultsButton();
                }
            });
            _numMostLikelyClustersGraph.getDocument().addUndoableEditListener(new UndoableEditListener() {
                public void undoableEditHappened(UndoableEditEvent evt) {
                    undo.addEdit(evt.getEdit());
                }
            });
            _numMostLikelyClustersGraph.addFocusListener(new java.awt.event.FocusAdapter() {
                public void focusGained(java.awt.event.FocusEvent evt) {
                    _numMostLikelyClustersGraphFocusGained(evt);
                }
            });

            _numMostLikelyClustersGraphLabel.setLabelFor(_numMostLikelyClustersGraph);
            _numMostLikelyClustersGraphLabel.setText("most likely clusters, one graph for each");
            _numMostLikelyClustersGraphLabel.addMouseListener(new java.awt.event.MouseAdapter() {
                public void mouseClicked(java.awt.event.MouseEvent evt) {
                    _numMostLikelyClustersGraphLabelMouseClicked(evt);
                }
            });

            _temporalGraphButtonGroup.add(_temporalGraphSignificant);
            _temporalGraphSignificant.setText("All clusters, one graph for each, with p-value less than:");

            _temporalGraphPvalueCutoff.setText("0.05"); // NOI18N
            _temporalGraphPvalueCutoff.setPreferredSize(new java.awt.Dimension(75, 22));
            initCutoffJTextField(_temporalGraphPvalueCutoff, AppConstants.DEFAULT_RECURRENCE_CUTOFF, AppConstants.DEFAULT_PVALUE_CUTOFF);
            _temporalGraphPvalueCutoff.addFocusListener(new java.awt.event.FocusAdapter() {
                public void focusGained(java.awt.event.FocusEvent evt) {
                    _temporalGraphPvalueCutoffFocusGained(evt);
                }
            });

            javax.swing.GroupLayout _graphOutputGroupLayout = new javax.swing.GroupLayout(_graphOutputGroup);
            _graphOutputGroup.setLayout(_graphOutputGroupLayout);
            _graphOutputGroupLayout.setHorizontalGroup(
                _graphOutputGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_graphOutputGroupLayout.createSequentialGroup()
                    .addGroup(_graphOutputGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                        .addGroup(_graphOutputGroupLayout.createSequentialGroup()
                            .addContainerGap()
                            .addComponent(_reportTemporalGraph, javax.swing.GroupLayout.DEFAULT_SIZE, 624, Short.MAX_VALUE))
                        .addGroup(_graphOutputGroupLayout.createSequentialGroup()
                            .addGap(27, 27, 27)
                            .addGroup(_graphOutputGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                                .addGroup(_graphOutputGroupLayout.createSequentialGroup()
                                    .addComponent(_temporalGraphSignificant)
                                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                                    .addComponent(_temporalGraphPvalueCutoff, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                                    .addGap(0, 0, Short.MAX_VALUE))
                                .addComponent(_temporalGraphMostLikely, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                                .addGroup(_graphOutputGroupLayout.createSequentialGroup()
                                    .addComponent(_temporalGraphMostLikelyX)
                                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                    .addComponent(_numMostLikelyClustersGraph, javax.swing.GroupLayout.PREFERRED_SIZE, 70, javax.swing.GroupLayout.PREFERRED_SIZE)
                                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                    .addComponent(_numMostLikelyClustersGraphLabel, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)))))
                    .addContainerGap())
            );
            _graphOutputGroupLayout.setVerticalGroup(
                _graphOutputGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_graphOutputGroupLayout.createSequentialGroup()
                    .addComponent(_reportTemporalGraph)
                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                    .addComponent(_temporalGraphMostLikely)
                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                    .addGroup(_graphOutputGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING)
                        .addComponent(_temporalGraphMostLikelyX)
                        .addGroup(_graphOutputGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                            .addComponent(_numMostLikelyClustersGraph, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                            .addComponent(_numMostLikelyClustersGraphLabel)))
                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                    .addGroup(_graphOutputGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                        .addComponent(_temporalGraphSignificant)
                        .addComponent(_temporalGraphPvalueCutoff, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                    .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
            );

            _temporalGraphMostLikelyX.getAccessibleContext().setAccessibleName("select one graph for each cluster");
            _temporalGraphPvalueCutoff.getAccessibleContext().setAccessibleName("All clusters, one graph for each cluster meeting cutoff");

            javax.swing.GroupLayout _temporalOutputTabLayout = new javax.swing.GroupLayout(_temporalOutputTab);
            _temporalOutputTab.setLayout(_temporalOutputTabLayout);
            _temporalOutputTabLayout.setHorizontalGroup(
                _temporalOutputTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_temporalOutputTabLayout.createSequentialGroup()
                    .addContainerGap()
                    .addComponent(_graphOutputGroup, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addContainerGap())
            );
            _temporalOutputTabLayout.setVerticalGroup(
                _temporalOutputTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_temporalOutputTabLayout.createSequentialGroup()
                    .addContainerGap()
                    .addComponent(_graphOutputGroup, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addContainerGap(318, Short.MAX_VALUE))
            );

            jTabbedPane1.addTab("Temporal Output", _temporalOutputTab);
            _temporalOutputTab.getAccessibleContext().setAccessibleName("Temporal Output tab");

            _oliveiras_f_group.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createEtchedBorder(), "Oliveira's F"));

            _calculate_oliveiras_f.setText("Calculate Oliveira's F for each location (increases computing time)");

            _number_oliveira_data_sets_label.setLabelFor(_number_oliveira_data_sets);
            _number_oliveira_data_sets_label.setText("Number of bootstrap replications (minimum 100, multiple of 100):"); // NOI18N

            _number_oliveira_data_sets.setText("1000"); // NOI18N
            _montCarloReplicationsTextField.addFocusListener(new java.awt.event.FocusAdapter() {
                public void focusLost(java.awt.event.FocusEvent e) {
                    while (_montCarloReplicationsTextField.getText().length() == 0)
                    if (undo.canUndo()) undo.undo(); else _montCarloReplicationsTextField.setText("999");
                    enableSetDefaultsButton();
                    updateCriticalValuesTextCaptions();
                }
            });
            _montCarloReplicationsTextField.addKeyListener(new java.awt.event.KeyAdapter() {
                public void keyTyped(java.awt.event.KeyEvent e) {
                    Utils.validatePostiveNumericKeyTyped(_montCarloReplicationsTextField, e, 10);
                }
            });
            _montCarloReplicationsTextField.getDocument().addUndoableEditListener(new UndoableEditListener() {
                public void undoableEditHappened(UndoableEditEvent evt) {
                    undo.addEdit(evt.getEdit());
                }
            });

            javax.swing.GroupLayout _oliveiras_f_groupLayout = new javax.swing.GroupLayout(_oliveiras_f_group);
            _oliveiras_f_group.setLayout(_oliveiras_f_groupLayout);
            _oliveiras_f_groupLayout.setHorizontalGroup(
                _oliveiras_f_groupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_oliveiras_f_groupLayout.createSequentialGroup()
                    .addGap(10, 10, 10)
                    .addGroup(_oliveiras_f_groupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                        .addComponent(_calculate_oliveiras_f, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                        .addGroup(_oliveiras_f_groupLayout.createSequentialGroup()
                            .addGap(21, 21, 21)
                            .addComponent(_number_oliveira_data_sets_label)
                            .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                            .addComponent(_number_oliveira_data_sets, javax.swing.GroupLayout.PREFERRED_SIZE, 80, javax.swing.GroupLayout.PREFERRED_SIZE)
                            .addGap(0, 203, Short.MAX_VALUE)))
                    .addContainerGap())
            );
            _oliveiras_f_groupLayout.setVerticalGroup(
                _oliveiras_f_groupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_oliveiras_f_groupLayout.createSequentialGroup()
                    .addComponent(_calculate_oliveiras_f)
                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                    .addGroup(_oliveiras_f_groupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                        .addComponent(_number_oliveira_data_sets_label)
                        .addComponent(_number_oliveira_data_sets, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                    .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
            );

            _prospective_frequency_group.setBorder(javax.swing.BorderFactory.createTitledBorder("Prospective Analyses"));

            _label_prospective_frequency.setLabelFor(_prospective_frequency);
            _label_prospective_frequency.setText("How frequently are analyses performed?");

            _prospective_frequency.addItemListener(new java.awt.event.ItemListener() {
                public void itemStateChanged(java.awt.event.ItemEvent e) {
                    enableSetDefaultsButton();
                }
            });
            _prospective_frequency.add("Same as Time Aggregation");
            _prospective_frequency.add("Daily");
            _prospective_frequency.add("Weekly");
            _prospective_frequency.add("Monthly");
            _prospective_frequency.add("Quarterly");
            _prospective_frequency.add("Yearly");

            javax.swing.GroupLayout _prospective_frequency_groupLayout = new javax.swing.GroupLayout(_prospective_frequency_group);
            _prospective_frequency_group.setLayout(_prospective_frequency_groupLayout);
            _prospective_frequency_groupLayout.setHorizontalGroup(
                _prospective_frequency_groupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_prospective_frequency_groupLayout.createSequentialGroup()
                    .addContainerGap()
                    .addComponent(_label_prospective_frequency, javax.swing.GroupLayout.PREFERRED_SIZE, 268, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                    .addComponent(_prospective_frequency, javax.swing.GroupLayout.PREFERRED_SIZE, 190, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
            );
            _prospective_frequency_groupLayout.setVerticalGroup(
                _prospective_frequency_groupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_prospective_frequency_groupLayout.createSequentialGroup()
                    .addGroup(_prospective_frequency_groupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                        .addComponent(_prospective_frequency, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                        .addComponent(_label_prospective_frequency, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                    .addGap(0, 14, Short.MAX_VALUE))
            );

            _prospective_frequency.getAccessibleContext().setAccessibleName("How frequently are analyses performed?");

            javax.swing.GroupLayout _miscellaneous_analysis_tabLayout = new javax.swing.GroupLayout(_miscellaneous_analysis_tab);
            _miscellaneous_analysis_tab.setLayout(_miscellaneous_analysis_tabLayout);
            _miscellaneous_analysis_tabLayout.setHorizontalGroup(
                _miscellaneous_analysis_tabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, _miscellaneous_analysis_tabLayout.createSequentialGroup()
                    .addContainerGap()
                    .addGroup(_miscellaneous_analysis_tabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING)
                        .addComponent(_prospective_frequency_group, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                        .addComponent(_oliveiras_f_group, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                    .addContainerGap())
            );
            _miscellaneous_analysis_tabLayout.setVerticalGroup(
                _miscellaneous_analysis_tabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_miscellaneous_analysis_tabLayout.createSequentialGroup()
                    .addContainerGap()
                    .addComponent(_oliveiras_f_group, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                    .addComponent(_prospective_frequency_group, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addContainerGap(328, Short.MAX_VALUE))
            );

            jTabbedPane1.addTab("Miscellaneous", null, _miscellaneous_analysis_tab, "");
            _miscellaneous_analysis_tab.getAccessibleContext().setAccessibleName("Miscellaneous tab");

            _limit_clusters_risk_group.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createEtchedBorder(), "Boscoe’s Limit on Clusters by Risk Level"));

            _limit_low_clusters.setText("Restrict low rate clusters to relative risk less than or equal to:");
            _limit_low_clusters.addItemListener(new java.awt.event.ItemListener() {
                public void itemStateChanged(java.awt.event.ItemEvent e) {
                    enableLimitClustersByRiskLevelGroup(_settings_window.getAreaScanRateControlType());
                    enableSetDefaultsButton();
                }
            });

            _limit_low_clusters_value.setText("0.25");
            _limit_low_clusters_value.addKeyListener(new java.awt.event.KeyAdapter() {
                public void keyTyped(java.awt.event.KeyEvent e) {
                    Utils.validatePostiveFloatKeyTyped(_limit_low_clusters_value, e, 20);
                }
            });
            _limit_low_clusters_value.addFocusListener(new java.awt.event.FocusAdapter() {
                public void focusLost(java.awt.event.FocusEvent e) {
                    while (_limit_low_clusters_value.getText().length() == 0 ||
                        Double.parseDouble(_limit_low_clusters_value.getText()) < 0 ||
                        Double.parseDouble(_limit_low_clusters_value.getText()) > 1)
                    if (undo.canUndo()) undo.undo(); else _limit_low_clusters_value.setText("1.0");
                    enableSetDefaultsButton();
                }
            });
            _limit_low_clusters_value.getDocument().addUndoableEditListener(new UndoableEditListener() {
                public void undoableEditHappened(UndoableEditEvent evt) {
                    undo.addEdit(evt.getEdit());
                }
            });

            _limit_high_clusters.setText("Restrict high rate clusters to relative risk greater than or equal to:");
            _limit_high_clusters.addItemListener(new java.awt.event.ItemListener() {
                public void itemStateChanged(java.awt.event.ItemEvent e) {
                    enableLimitClustersByRiskLevelGroup(_settings_window.getAreaScanRateControlType());
                    enableSetDefaultsButton();
                }
            });

            _limit_high_clusters_value.setText("1.875");
            _limit_high_clusters_value.addKeyListener(new java.awt.event.KeyAdapter() {
                public void keyTyped(java.awt.event.KeyEvent e) {
                    Utils.validatePostiveFloatKeyTyped(_limit_high_clusters_value, e, 20);
                }
            });
            _limit_high_clusters_value.addFocusListener(new java.awt.event.FocusAdapter() {
                public void focusLost(java.awt.event.FocusEvent e) {
                    while (_limit_high_clusters_value.getText().length() == 0 ||
                        Double.parseDouble(_limit_high_clusters_value.getText()) < 1.0)
                    if (undo.canUndo()) undo.undo(); else _limit_high_clusters_value.setText("1.0");
                    enableSetDefaultsButton();
                }
            });
            _limit_high_clusters_value.getDocument().addUndoableEditListener(new UndoableEditListener() {
                public void undoableEditHappened(UndoableEditEvent evt) {
                    undo.addEdit(evt.getEdit());
                }
            });

            javax.swing.GroupLayout _limit_clusters_risk_groupLayout = new javax.swing.GroupLayout(_limit_clusters_risk_group);
            _limit_clusters_risk_group.setLayout(_limit_clusters_risk_groupLayout);
            _limit_clusters_risk_groupLayout.setHorizontalGroup(
                _limit_clusters_risk_groupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_limit_clusters_risk_groupLayout.createSequentialGroup()
                    .addContainerGap()
                    .addGroup(_limit_clusters_risk_groupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                        .addComponent(_limit_low_clusters, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                        .addComponent(_limit_high_clusters, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                    .addGroup(_limit_clusters_risk_groupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                        .addComponent(_limit_low_clusters_value)
                        .addComponent(_limit_high_clusters_value, javax.swing.GroupLayout.DEFAULT_SIZE, 101, Short.MAX_VALUE))
                    .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
            );
            _limit_clusters_risk_groupLayout.setVerticalGroup(
                _limit_clusters_risk_groupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_limit_clusters_risk_groupLayout.createSequentialGroup()
                    .addGap(5, 5, 5)
                    .addGroup(_limit_clusters_risk_groupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                        .addComponent(_limit_high_clusters)
                        .addComponent(_limit_high_clusters_value, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                    .addGroup(_limit_clusters_risk_groupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                        .addComponent(_limit_low_clusters)
                        .addComponent(_limit_low_clusters_value, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                    .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
            );

            _limit_low_clusters_value.getAccessibleContext().setAccessibleName("Restrict low rate clusters to relative risk less than or equal to:");
            _limit_high_clusters_value.getAccessibleContext().setAccessibleName("Restrict high rate clusters to relative risk greater than or equal to:");

            _minimum_clusters_group.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createEtchedBorder(), "Minimum Number of Cases"));

            _min_cases_label.setText("Restrict high rate clusters to have at least"); // NOI18N

            _minimum_number_cases_cluster.setText("2"); // NOI18N
            _minimum_number_cases_cluster.addKeyListener(new java.awt.event.KeyAdapter() {
                public void keyTyped(java.awt.event.KeyEvent e) {
                    Utils.validatePostiveNumericKeyTyped(_minimum_number_cases_cluster, e, 5);
                }
            });
            _minimum_number_cases_cluster.addFocusListener(new java.awt.event.FocusAdapter() {
                public void focusLost(java.awt.event.FocusEvent e) {
                    while (_minimum_number_cases_cluster.getText().length() == 0 ||
                        Integer.parseInt(_minimum_number_cases_cluster.getText()) < 2) {
                        if (undo.canUndo()) undo.undo(); else _minimum_number_cases_cluster.setText("2");
                    }
                    enableSetDefaultsButton();
                }
            });
            _minimum_number_cases_cluster.getDocument().addUndoableEditListener(new UndoableEditListener() {
                public void undoableEditHappened(UndoableEditEvent evt) {
                    undo.addEdit(evt.getEdit());
                }
            });

            _min_cases_label2.setText("cases.");

            javax.swing.GroupLayout _minimum_clusters_groupLayout = new javax.swing.GroupLayout(_minimum_clusters_group);
            _minimum_clusters_group.setLayout(_minimum_clusters_groupLayout);
            _minimum_clusters_groupLayout.setHorizontalGroup(
                _minimum_clusters_groupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_minimum_clusters_groupLayout.createSequentialGroup()
                    .addContainerGap()
                    .addComponent(_min_cases_label)
                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                    .addComponent(_minimum_number_cases_cluster, javax.swing.GroupLayout.PREFERRED_SIZE, 70, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                    .addComponent(_min_cases_label2, javax.swing.GroupLayout.PREFERRED_SIZE, 291, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addContainerGap())
            );
            _minimum_clusters_groupLayout.setVerticalGroup(
                _minimum_clusters_groupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_minimum_clusters_groupLayout.createSequentialGroup()
                    .addGroup(_minimum_clusters_groupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                        .addComponent(_min_cases_label)
                        .addComponent(_minimum_number_cases_cluster, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addComponent(_min_cases_label2))
                    .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
            );

            _minimum_number_cases_cluster.getAccessibleContext().setAccessibleName("Restrict high rate clusters to have at least X cases");

            javax.swing.GroupLayout _cluster_restrictions_tabLayout = new javax.swing.GroupLayout(_cluster_restrictions_tab);
            _cluster_restrictions_tab.setLayout(_cluster_restrictions_tabLayout);
            _cluster_restrictions_tabLayout.setHorizontalGroup(
                _cluster_restrictions_tabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_cluster_restrictions_tabLayout.createSequentialGroup()
                    .addContainerGap()
                    .addGroup(_cluster_restrictions_tabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                        .addComponent(_limit_clusters_risk_group, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                        .addComponent(_minimum_clusters_group, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                    .addContainerGap())
            );
            _cluster_restrictions_tabLayout.setVerticalGroup(
                _cluster_restrictions_tabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_cluster_restrictions_tabLayout.createSequentialGroup()
                    .addGap(5, 5, 5)
                    .addComponent(_minimum_clusters_group, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                    .addComponent(_limit_clusters_risk_group, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addContainerGap(315, Short.MAX_VALUE))
            );

            jTabbedPane1.addTab("Cluster Restrictions", _cluster_restrictions_tab);
            _cluster_restrictions_tab.getAccessibleContext().setAccessibleName("Cluster Restrictions tab");

            _drilldown_restrictions_group.setBorder(javax.swing.BorderFactory.createTitledBorder("Cluster Drilldown"));

            _mainAnalysisDrilldown.setText("Same Design as Main Analysis");
            _mainAnalysisDrilldown.addActionListener(new java.awt.event.ActionListener() {
                public void actionPerformed(java.awt.event.ActionEvent e) {
                    enableDrilldownGroup();
                }
            });

            _purelySpatialDrilldown.setText("Purely Spatial Bernoulli");
            _purelySpatialDrilldown.addActionListener(new java.awt.event.ActionListener() {
                public void actionPerformed(java.awt.event.ActionEvent e) {
                    enableDrilldownGroup();
                }
            });

            _drilldown_restriction_locations_label.setText("Minimum Locations:");

            _drilldown_restriction_locations.setText("2");
            _drilldown_restriction_locations.addFocusListener(new java.awt.event.FocusAdapter() {
                public void focusLost(java.awt.event.FocusEvent e) {
                    while (_drilldown_restriction_locations.getText().length() == 0 ||
                        Double.parseDouble(_drilldown_restriction_locations.getText()) < 2
                    )
                    if (undo.canUndo()) undo.undo(); else _drilldown_restriction_locations.setText("2");
                    enableSetDefaultsButton();
                }
            });
            _drilldown_restriction_locations.addKeyListener(new java.awt.event.KeyAdapter() {
                public void keyTyped(java.awt.event.KeyEvent e) {
                    Utils.validatePostiveNumericKeyTyped(_drilldown_restriction_locations, e, 10);
                }
            });
            _drilldown_restriction_locations.getDocument().addUndoableEditListener(new UndoableEditListener() {
                public void undoableEditHappened(UndoableEditEvent evt) {
                    undo.addEdit(evt.getEdit());
                }
            });

            _drilldown_restriction_cases_label.setText("Minimum Cases:");

            _drilldown_restriction_cases.setText("10");
            _drilldown_restriction_cases.addFocusListener(new java.awt.event.FocusAdapter() {
                public void focusLost(java.awt.event.FocusEvent e) {
                    while (_drilldown_restriction_cases.getText().length() == 0 ||
                        Double.parseDouble(_drilldown_restriction_cases.getText()) < 10
                    )
                    if (undo.canUndo()) undo.undo(); else _drilldown_restriction_cases.setText("10");
                    enableSetDefaultsButton();
                }
            });
            _drilldown_restriction_cases.addKeyListener(new java.awt.event.KeyAdapter() {
                public void keyTyped(java.awt.event.KeyEvent e) {
                    Utils.validatePostiveNumericKeyTyped(_totalPowerCases, e, 10);
                }
            });
            _drilldown_restriction_cases.getDocument().addUndoableEditListener(new UndoableEditListener() {
                public void undoableEditHappened(UndoableEditEvent evt) {
                    undo.addEdit(evt.getEdit());
                }
            });

            _drilldown_restriction_cutoff_label.setText("P-Value Less Than Or Equal:"); // NOI18N

            _drilldown_restriction_cutoff.setText("0.05"); // NOI18N
            initCutoffJTextField(_drilldown_restriction_cutoff, AppConstants.DEFAULT_RECURRENCE_CUTOFF, AppConstants.DEFAULT_PVALUE_CUTOFF);

            _drilldown_restrictions.setText("Drilldown for Clusters with:");

            _drilldown_restriction_dow.setText("With Day of Week Adjustment");

            javax.swing.GroupLayout _drilldown_restrictions_groupLayout = new javax.swing.GroupLayout(_drilldown_restrictions_group);
            _drilldown_restrictions_group.setLayout(_drilldown_restrictions_groupLayout);
            _drilldown_restrictions_groupLayout.setHorizontalGroup(
                _drilldown_restrictions_groupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_drilldown_restrictions_groupLayout.createSequentialGroup()
                    .addContainerGap()
                    .addGroup(_drilldown_restrictions_groupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                        .addComponent(_mainAnalysisDrilldown, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                        .addGroup(_drilldown_restrictions_groupLayout.createSequentialGroup()
                            .addGroup(_drilldown_restrictions_groupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                                .addGroup(_drilldown_restrictions_groupLayout.createSequentialGroup()
                                    .addGap(10, 10, 10)
                                    .addGroup(_drilldown_restrictions_groupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                                        .addComponent(_drilldown_restriction_cutoff_label, javax.swing.GroupLayout.DEFAULT_SIZE, 191, Short.MAX_VALUE)
                                        .addComponent(_drilldown_restriction_locations_label, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                                        .addComponent(_drilldown_restriction_cases_label, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                    .addGroup(_drilldown_restrictions_groupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING, false)
                                        .addComponent(_drilldown_restriction_locations, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, 70, Short.MAX_VALUE)
                                        .addComponent(_drilldown_restriction_cutoff, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, 70, Short.MAX_VALUE)
                                        .addComponent(_drilldown_restriction_cases, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, 70, Short.MAX_VALUE)))
                                .addComponent(_drilldown_restrictions, javax.swing.GroupLayout.PREFERRED_SIZE, 349, javax.swing.GroupLayout.PREFERRED_SIZE)
                                .addGroup(_drilldown_restrictions_groupLayout.createSequentialGroup()
                                    .addComponent(_purelySpatialDrilldown, javax.swing.GroupLayout.PREFERRED_SIZE, 222, javax.swing.GroupLayout.PREFERRED_SIZE)
                                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                    .addComponent(_drilldown_restriction_dow, javax.swing.GroupLayout.PREFERRED_SIZE, 241, javax.swing.GroupLayout.PREFERRED_SIZE)))
                            .addGap(0, 161, Short.MAX_VALUE)))
                    .addContainerGap())
            );
            _drilldown_restrictions_groupLayout.setVerticalGroup(
                _drilldown_restrictions_groupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_drilldown_restrictions_groupLayout.createSequentialGroup()
                    .addContainerGap()
                    .addComponent(_mainAnalysisDrilldown)
                    .addGap(6, 6, 6)
                    .addGroup(_drilldown_restrictions_groupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                        .addComponent(_purelySpatialDrilldown, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                        .addComponent(_drilldown_restriction_dow, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                    .addGap(18, 18, 18)
                    .addComponent(_drilldown_restrictions)
                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                    .addGroup(_drilldown_restrictions_groupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                        .addComponent(_drilldown_restriction_cutoff_label)
                        .addComponent(_drilldown_restriction_cutoff, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                    .addGroup(_drilldown_restrictions_groupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                        .addComponent(_drilldown_restriction_locations_label)
                        .addComponent(_drilldown_restriction_locations, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                    .addGap(7, 7, 7)
                    .addGroup(_drilldown_restrictions_groupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                        .addComponent(_drilldown_restriction_cases_label, javax.swing.GroupLayout.PREFERRED_SIZE, 12, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addComponent(_drilldown_restriction_cases, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                    .addContainerGap(80, Short.MAX_VALUE))
            );

            _drilldown_restriction_locations.getAccessibleContext().setAccessibleName("Drilldown for Clusters having at least X locations");
            _drilldown_restriction_cases.getAccessibleContext().setAccessibleName("Drilldown for Clusters having at least X cases");
            _drilldown_restriction_cutoff.getAccessibleContext().setAccessibleName("Drilldown for Clusters meeting cutoff");

            javax.swing.GroupLayout _drilldown_tabLayout = new javax.swing.GroupLayout(_drilldown_tab);
            _drilldown_tab.setLayout(_drilldown_tabLayout);
            _drilldown_tabLayout.setHorizontalGroup(
                _drilldown_tabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_drilldown_tabLayout.createSequentialGroup()
                    .addContainerGap()
                    .addComponent(_drilldown_restrictions_group, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addContainerGap())
            );
            _drilldown_tabLayout.setVerticalGroup(
                _drilldown_tabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_drilldown_tabLayout.createSequentialGroup()
                    .addContainerGap()
                    .addComponent(_drilldown_restrictions_group, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addContainerGap(195, Short.MAX_VALUE))
            );

            jTabbedPane1.addTab("Drilldown", _drilldown_tab);
            _drilldown_tab.getAccessibleContext().setAccessibleName("Drilldown tab");

            _network_group.setBorder(javax.swing.BorderFactory.createTitledBorder("Locations Network"));

            _locations_network.setText("Specify locations through a network file");
            _locations_network.addItemListener(new java.awt.event.ItemListener() {
                public void itemStateChanged(java.awt.event.ItemEvent e) {
                    enableSettingsForAnalysisModelCombination();
                    enableSetDefaultsButton();
                }
            });

            _network_file_label.setLabelFor(_network_filename);
            _network_file_label.setText("Network File:"); // NOI18N

            _adjustmentsByRelativeRisksFileTextField.addFocusListener(new java.awt.event.FocusAdapter() {
                public void focusLost(java.awt.event.FocusEvent e) {
                    enableSetDefaultsButton();
                }
            });

            _browse_network_filename.setText("..."); // NOI18N
            _browse_network_filename.setToolTipText("Open Adjustments File Import Wizard"); // NOI18N
            _browse_network_filename.addActionListener(new java.awt.event.ActionListener() {
                public void actionPerformed(java.awt.event.ActionEvent e) {
                    String key = InputSourceSettings.InputFileType.NETWORK.toString() + "1";
                    if (!_settings_window._input_source_map.containsKey(key)) {
                        _settings_window._input_source_map.put(key, new InputSourceSettings(InputSourceSettings.InputFileType.NETWORK));
                    }
                    InputSourceSettings inputSourceSettings = (InputSourceSettings)_settings_window._input_source_map.get(key);
                    // invoke the FileSelectionDialog to guide user through process of selecting the source file.
                    FileSelectionDialog selectionDialog = new FileSelectionDialog(SaTScanApplication.getInstance(), inputSourceSettings.getInputFileType(), SaTScanApplication.getInstance().lastBrowseDirectory);
                    selectionDialog.browse_inputsource(_network_filename, inputSourceSettings, _settings_window);
                }
            });

            javax.swing.GroupLayout _network_groupLayout = new javax.swing.GroupLayout(_network_group);
            _network_group.setLayout(_network_groupLayout);
            _network_groupLayout.setHorizontalGroup(
                _network_groupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_network_groupLayout.createSequentialGroup()
                    .addContainerGap()
                    .addGroup(_network_groupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                        .addComponent(_locations_network, javax.swing.GroupLayout.DEFAULT_SIZE, 624, Short.MAX_VALUE)
                        .addGroup(_network_groupLayout.createSequentialGroup()
                            .addGap(14, 14, 14)
                            .addComponent(_network_file_label)
                            .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                            .addComponent(_network_filename)
                            .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                            .addComponent(_browse_network_filename, javax.swing.GroupLayout.PREFERRED_SIZE, 25, javax.swing.GroupLayout.PREFERRED_SIZE)))
                    .addContainerGap())
            );
            _network_groupLayout.setVerticalGroup(
                _network_groupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_network_groupLayout.createSequentialGroup()
                    .addContainerGap()
                    .addComponent(_locations_network)
                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                    .addGroup(_network_groupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                        .addComponent(_network_file_label)
                        .addComponent(_network_filename, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addComponent(_browse_network_filename))
                    .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
            );

            _browse_network_filename.getAccessibleContext().setAccessibleName("");

            javax.swing.GroupLayout _network_tabLayout = new javax.swing.GroupLayout(_network_tab);
            _network_tab.setLayout(_network_tabLayout);
            _network_tabLayout.setHorizontalGroup(
                _network_tabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_network_tabLayout.createSequentialGroup()
                    .addContainerGap()
                    .addComponent(_network_group, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addContainerGap())
            );
            _network_tabLayout.setVerticalGroup(
                _network_tabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_network_tabLayout.createSequentialGroup()
                    .addContainerGap()
                    .addComponent(_network_group, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addContainerGap(385, Short.MAX_VALUE))
            );

            jTabbedPane1.addTab("Network", _network_tab);
            _network_tab.getAccessibleContext().setAccessibleName("Network tab");

            _panel_email_notifications.setBorder(javax.swing.BorderFactory.createTitledBorder("Email Alerts"));

            _always_sendmail.setText("Always send email with summary results to (csv list):");

            _attach_main_results_email.setText("Attach main results file to email alerts");

            _cutoff_email.setText("If p-value less than or equal to");

            _report_main_results_email.setText("Include main results file and output directory in email");

            _custom_email_subject_label.setText("Subject");

            _custom_email_tags.setText("Message Tags");

            _custom_email_message.setColumns(20);
            _custom_email_message.setLineWrap(true);
            _custom_email_message.setRows(5);
            jScrollPane3.setViewportView(_custom_email_message);
            _custom_email_message.getAccessibleContext().setAccessibleName("email message body");

            _custom_email_message_label.setText("Message");

            _create_custom_email_message.setText("Create a custom email message (optional):");

            _cutoff_value_email.setText("0.05");
            _cutoff_value_email.setMinimumSize(new java.awt.Dimension(75, 22));
            _cutoff_value_email.setPreferredSize(new java.awt.Dimension(75, 22));
            initCutoffJTextField(_cutoff_value_email, AppConstants.DEFAULT_RECURRENCE_CUTOFF, AppConstants.DEFAULT_PVALUE_CUTOFF);

            _cutoff_email_label.setText(" send email with summary results to (csv list):");

            javax.swing.GroupLayout _panel_email_notificationsLayout = new javax.swing.GroupLayout(_panel_email_notifications);
            _panel_email_notifications.setLayout(_panel_email_notificationsLayout);
            _panel_email_notificationsLayout.setHorizontalGroup(
                _panel_email_notificationsLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_panel_email_notificationsLayout.createSequentialGroup()
                    .addContainerGap()
                    .addGroup(_panel_email_notificationsLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                        .addComponent(_attach_main_results_email, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                        .addComponent(_report_main_results_email, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                        .addGroup(_panel_email_notificationsLayout.createSequentialGroup()
                            .addGroup(_panel_email_notificationsLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                                .addComponent(_always_sendmail, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                                .addComponent(_create_custom_email_message, javax.swing.GroupLayout.Alignment.TRAILING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                                .addGroup(_panel_email_notificationsLayout.createSequentialGroup()
                                    .addGap(27, 27, 27)
                                    .addComponent(_custom_email_subject_label)
                                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                    .addComponent(_custom_email_subject, javax.swing.GroupLayout.PREFERRED_SIZE, 466, javax.swing.GroupLayout.PREFERRED_SIZE)
                                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                                    .addComponent(_custom_email_tags, javax.swing.GroupLayout.PREFERRED_SIZE, 127, javax.swing.GroupLayout.PREFERRED_SIZE))
                                .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, _panel_email_notificationsLayout.createSequentialGroup()
                                    .addGap(21, 21, 21)
                                    .addGroup(_panel_email_notificationsLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                                        .addComponent(_cutoff_email_recipients, javax.swing.GroupLayout.Alignment.TRAILING)
                                        .addComponent(_always_email_recipients)))
                                .addGroup(_panel_email_notificationsLayout.createSequentialGroup()
                                    .addComponent(_cutoff_email)
                                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                    .addComponent(_cutoff_value_email, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                    .addComponent(_cutoff_email_label, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                                .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, _panel_email_notificationsLayout.createSequentialGroup()
                                    .addGap(20, 20, 20)
                                    .addComponent(_custom_email_message_label)
                                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                    .addComponent(jScrollPane3)))
                            .addContainerGap())))
            );
            _panel_email_notificationsLayout.setVerticalGroup(
                _panel_email_notificationsLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, _panel_email_notificationsLayout.createSequentialGroup()
                    .addGap(7, 7, 7)
                    .addComponent(_always_sendmail)
                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                    .addComponent(_always_email_recipients, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addGap(11, 11, 11)
                    .addGroup(_panel_email_notificationsLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                        .addComponent(_cutoff_email)
                        .addComponent(_cutoff_value_email, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addComponent(_cutoff_email_label))
                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                    .addComponent(_cutoff_email_recipients, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                    .addComponent(_attach_main_results_email)
                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                    .addComponent(_report_main_results_email)
                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                    .addComponent(_create_custom_email_message)
                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                    .addGroup(_panel_email_notificationsLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                        .addComponent(_custom_email_tags, javax.swing.GroupLayout.Alignment.TRAILING, javax.swing.GroupLayout.PREFERRED_SIZE, 22, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addGroup(_panel_email_notificationsLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                            .addComponent(_custom_email_subject, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                            .addComponent(_custom_email_subject_label)))
                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                    .addGroup(_panel_email_notificationsLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                        .addComponent(_custom_email_message_label)
                        .addComponent(jScrollPane3, javax.swing.GroupLayout.PREFERRED_SIZE, 182, javax.swing.GroupLayout.PREFERRED_SIZE))
                    .addGap(24, 24, 24))
            );

            _always_email_recipients.getAccessibleContext().setAccessibleName("");
            _always_email_recipients.getAccessibleContext().setAccessibleDescription("Always send email with summary results to email list");
            _cutoff_email_recipients.getAccessibleContext().setAccessibleName("");
            _cutoff_email_recipients.getAccessibleContext().setAccessibleDescription("Send email with summary results to email list if analysis meets cutoff");
            _custom_email_subject.getAccessibleContext().setAccessibleName("email message subject");
            _custom_email_tags.getAccessibleContext().setAccessibleName("Message Body Tags");
            _cutoff_value_email.getAccessibleContext().setAccessibleName("send email summary cutoff");

            javax.swing.GroupLayout _notificatons_tabLayout = new javax.swing.GroupLayout(_notificatons_tab);
            _notificatons_tab.setLayout(_notificatons_tabLayout);
            _notificatons_tabLayout.setHorizontalGroup(
                _notificatons_tabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_notificatons_tabLayout.createSequentialGroup()
                    .addContainerGap()
                    .addComponent(_panel_email_notifications, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addContainerGap())
            );
            _notificatons_tabLayout.setVerticalGroup(
                _notificatons_tabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(_notificatons_tabLayout.createSequentialGroup()
                    .addContainerGap()
                    .addComponent(_panel_email_notifications, javax.swing.GroupLayout.PREFERRED_SIZE, 463, Short.MAX_VALUE)
                    .addContainerGap())
            );

            jTabbedPane1.addTab("Notifications", _notificatons_tab);
            _notificatons_tab.getAccessibleContext().setAccessibleName("Notifications tab");

            _closeButton.setText("Close"); // NOI18N
            _closeButton.addActionListener(new java.awt.event.ActionListener() {
                public void actionPerformed(java.awt.event.ActionEvent e) {
                    setVisible(false, null);
                }
            });

            _setDefaultButton.setText("Set Defaults"); // NOI18N
            _setDefaultButton.addActionListener(new java.awt.event.ActionListener() {
                public void actionPerformed(java.awt.event.ActionEvent e) {
                    setDefaultsClick();
                }
            });

            javax.swing.GroupLayout layout = new javax.swing.GroupLayout(getContentPane());
            getContentPane().setLayout(layout);
            layout.setHorizontalGroup(
                layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, layout.createSequentialGroup()
                    .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(_setDefaultButton, javax.swing.GroupLayout.PREFERRED_SIZE, 120, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                    .addComponent(_closeButton, javax.swing.GroupLayout.PREFERRED_SIZE, 105, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addContainerGap())
                .addComponent(jTabbedPane1)
            );
            layout.setVerticalGroup(
                layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, layout.createSequentialGroup()
                    .addComponent(jTabbedPane1)
                    .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                    .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                        .addComponent(_setDefaultButton)
                        .addComponent(_closeButton))
                    .addContainerGap())
            );

            pack();
        }// </editor-fold>//GEN-END:initComponents

    private void _numMostLikelyClustersGraphFocusGained(java.awt.event.FocusEvent evt) {//GEN-FIRST:event__numMostLikelyClustersGraphFocusGained
        _temporalGraphMostLikelyX.setSelected(true);
    }//GEN-LAST:event__numMostLikelyClustersGraphFocusGained

    private void _temporalGraphPvalueCutoffFocusGained(java.awt.event.FocusEvent evt) {//GEN-FIRST:event__temporalGraphPvalueCutoffFocusGained
        _temporalGraphSignificant.setSelected(true);
    }//GEN-LAST:event__temporalGraphPvalueCutoffFocusGained

    private void _numMostLikelyClustersGraphLabelMouseClicked(java.awt.event.MouseEvent evt) {//GEN-FIRST:event__numMostLikelyClustersGraphLabelMouseClicked
        if (_numMostLikelyClustersGraphLabel.isEnabled())
            _temporalGraphMostLikelyX.setSelected(true);
    }//GEN-LAST:event__numMostLikelyClustersGraphLabelMouseClicked

    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JButton _addDataSetButton;
    private javax.swing.JPanel _additionalDataSetsGroup;
    private javax.swing.JPanel _additionalOutputFiles;
    private javax.swing.JCheckBox _adjustDayOfWeek;
    private javax.swing.JCheckBox _adjustForKnownRelativeRisksCheckBox;
    private javax.swing.JRadioButton _adjustmentByDataSetsRadioButton;
    private javax.swing.JLabel _adjustmentsByRelativeRisksFileLabel;
    private javax.swing.JTextField _adjustmentsByRelativeRisksFileTextField;
    private javax.swing.JButton _adjustmentsFileBrowseButton;
    private javax.swing.JRadioButton _allLocationsRadioButton;
    private javax.swing.JTextField _alternativeHypothesisFilename;
    private javax.swing.JButton _alternativeHypothesisFilenameButton;
    private javax.swing.JLabel _alternativeHypothesisFilenameLabel;
    private javax.swing.JTextField _always_email_recipients;
    private javax.swing.JCheckBox _always_sendmail;
    private javax.swing.JRadioButton _atLeastOneRadioButton;
    private javax.swing.JCheckBox _attach_main_results_email;
    private javax.swing.JButton _browse_network_filename;
    protected javax.swing.JCheckBox _calculate_oliveiras_f;
    private javax.swing.JButton _caseFileBrowseButton;
    private javax.swing.JLabel _caseFileLabel;
    private javax.swing.JTextField _caseFileTextField;
    private javax.swing.JCheckBox _checkReportGumbel;
    private javax.swing.JCheckBox _checkboxReportIndexCoefficients;
    private javax.swing.JRadioButton _circularRadioButton;
    private javax.swing.JButton _closeButton;
    private javax.swing.JLabel _cluster_lineline_label;
    private javax.swing.JPanel _cluster_lineline_panel;
    private javax.swing.JLabel _cluster_lineline_prelabel;
    private javax.swing.JTextField _cluster_lineline_value;
    private javax.swing.JPanel _cluster_restrictions_tab;
    private javax.swing.JPanel _clustersReportedGroup;
    private javax.swing.JButton _controlFileBrowseButton;
    private javax.swing.JLabel _controlFileLabel;
    private javax.swing.JTextField _controlFileTextField;
    private javax.swing.JCheckBox _createCompressedKMZ;
    private javax.swing.JCheckBox _create_custom_email_message;
    private javax.swing.ButtonGroup _criticalValuesButtonGroup;
    private javax.swing.JRadioButton _criticalValuesGumbel;
    private javax.swing.JRadioButton _criticalValuesMonteCarlo;
    private javax.swing.JLabel _criticalValuesReplicationsLabel;
    private javax.swing.JLabel _criticalValuesTypeLabel;
    private javax.swing.JTextArea _custom_email_message;
    private javax.swing.JLabel _custom_email_message_label;
    private javax.swing.JTextField _custom_email_subject;
    private javax.swing.JLabel _custom_email_subject_label;
    private javax.swing.JButton _custom_email_tags;
    private javax.swing.JCheckBox _cutoff_email;
    private javax.swing.JLabel _cutoff_email_label;
    private javax.swing.JTextField _cutoff_email_recipients;
    private javax.swing.JTextField _cutoff_value_email;
    private javax.swing.JPanel _dataCheckingTab;
    private javax.swing.JPanel _dataSetsGroup;
    private javax.swing.JLabel _distancePrefixLabel;
    private javax.swing.JTextField _drilldown_restriction_cases;
    private javax.swing.JLabel _drilldown_restriction_cases_label;
    private javax.swing.JTextField _drilldown_restriction_cutoff;
    private javax.swing.JLabel _drilldown_restriction_cutoff_label;
    private javax.swing.JCheckBox _drilldown_restriction_dow;
    private javax.swing.JTextField _drilldown_restriction_locations;
    private javax.swing.JLabel _drilldown_restriction_locations_label;
    private javax.swing.JLabel _drilldown_restrictions;
    private javax.swing.JPanel _drilldown_restrictions_group;
    private javax.swing.JPanel _drilldown_tab;
    private javax.swing.JTextField _earlyTerminationThreshold;
    private javax.swing.JRadioButton _ellipticRadioButton;
    private javax.swing.JLabel _endGenericRangeToLabel;
    private javax.swing.JLabel _endGenericWindowRangeLabel;
    private javax.swing.JTextField _endRangeEndDayTextField;
    private javax.swing.JTextField _endRangeEndGenericTextField;
    private javax.swing.JTextField _endRangeEndMonthTextField;
    private javax.swing.JTextField _endRangeEndYearTextField;
    private javax.swing.JTextField _endRangeStartDayTextField;
    private javax.swing.JTextField _endRangeStartGenericTextField;
    private javax.swing.JTextField _endRangeStartMonthTextField;
    private javax.swing.JTextField _endRangeStartYearTextField;
    private javax.swing.JLabel _endRangeToLabel;
    private javax.swing.JLabel _endWindowRangeLabel;
    private javax.swing.JPanel _fileInputGroup;
    private javax.swing.JPanel _flexibleTemporalWindowDefinitionGroup;
    private javax.swing.JPanel _flexible_window_cards;
    private javax.swing.ButtonGroup _geographicalCoordinatesCheckButtonGroup;
    private javax.swing.JPanel _geographicalCoordinatesCheckGroup;
    private javax.swing.JCheckBox _giniOptimizedClusters;
    private javax.swing.JPanel _graphOutputGroup;
    private javax.swing.JLabel _hierarchicalLabel;
    private java.awt.Choice _hierarchicalSecondaryClusters;
    private javax.swing.JCheckBox _inclPureTempClustCheckBox;
    private javax.swing.JCheckBox _includeClusterLocationsInKML;
    private javax.swing.JCheckBox _includePureSpacClustCheckBox;
    private javax.swing.JPanel _inferenceTab;
    private javax.swing.JList _inputDataSetsList;
    private javax.swing.JLabel _iterativeCutoffLabel;
    private javax.swing.JTextField _iterativeScanCutoffTextField;
    private javax.swing.JPanel _iterativeScanGroup;
    private javax.swing.JPanel _knownAdjustmentsGroup;
    private javax.swing.JLabel _labelMonteCarloReplications;
    private javax.swing.JLabel _label_kml_options;
    private javax.swing.JLabel _label_prospective_frequency;
    private javax.swing.JCheckBox _launch_map_viewer;
    private javax.swing.JPanel _limit_clusters_risk_group;
    private javax.swing.JCheckBox _limit_high_clusters;
    private javax.swing.JTextField _limit_high_clusters_value;
    private javax.swing.JCheckBox _limit_low_clusters;
    private javax.swing.JTextField _limit_low_clusters_value;
    private javax.swing.JCheckBox _locations_network;
    private javax.swing.JLabel _logLinearLabel;
    private javax.swing.JTextField _logLinearTextField;
    private javax.swing.JCheckBox _mainAnalysisDrilldown;
    private javax.swing.JPanel _mapsOutputGroup;
    private javax.swing.JButton _maxCirclePopFileBrowseButton;
    private javax.swing.JTextField _maxCirclePopulationFilenameTextField;
    private javax.swing.JLabel _maxIterativeScansLabel;
    private javax.swing.JLabel _maxRadiusLabel;
    private javax.swing.JLabel _maxReportedRadiusLabel;
    private javax.swing.JTextField _maxReportedSpatialClusterSizeTextField;
    private javax.swing.JTextField _maxReportedSpatialPercentFileTextField;
    private javax.swing.JTextField _maxReportedSpatialRadiusTextField;
    private javax.swing.JTextField _maxSpatialClusterSizeTextField;
    private javax.swing.JTextField _maxSpatialPercentFileTextField;
    private javax.swing.JTextField _maxSpatialRadiusTextField;
    private javax.swing.JTextField _maxTemporalClusterSizeTextField;
    private javax.swing.JTextField _maxTemporalClusterSizeUnitsTextField;
    private javax.swing.JPanel _maxTemporalOptionsGroup;
    private javax.swing.JLabel _maxTemporalTimeUnitsLabel;
    private javax.swing.JButton _metaLocationsFileBrowseButton;
    private javax.swing.JTextField _metaLocationsFileTextField;
    private javax.swing.JTextField _minTemporalClusterSizeUnitsTextField;
    private javax.swing.JPanel _minTemporalOptionsGroup;
    private javax.swing.JLabel _minTemporalTimeUnitsLabel;
    private javax.swing.JLabel _min_cases_label;
    private javax.swing.JLabel _min_cases_label2;
    private javax.swing.JPanel _minimum_clusters_group;
    private javax.swing.JTextField _minimum_number_cases_cluster;
    private javax.swing.JPanel _miscellaneous_analysis_tab;
    private javax.swing.JTextField _montCarloReplicationsTextField;
    private javax.swing.JPanel _monteCarloGroup;
    private javax.swing.JCheckBox _mostLikelyClustersHierarchically;
    private javax.swing.JLabel _multipleDataSetPurposeLabel;
    private javax.swing.JPanel _multipleDataSetsTab;
    private javax.swing.ButtonGroup _multipleSetButtonGroup;
    private javax.swing.JPanel _multipleSetPurposeGroup;
    private javax.swing.ButtonGroup _multipleSetsSpatialCoordinatesButtonGroup;
    private javax.swing.JPanel _multipleSetsSpatialCoordinatesGroup;
    private javax.swing.JTextField _multiple_locations_file;
    private javax.swing.JButton _multiple_locations_file_browse;
    private javax.swing.JLabel _multiple_locations_file_label;
    private javax.swing.JRadioButton _multivariateAdjustmentsRadioButton;
    private javax.swing.JButton _neighborsFileBrowseButton;
    private javax.swing.JTextField _neighborsFileTextField;
    private javax.swing.JLabel _network_file_label;
    private javax.swing.JTextField _network_filename;
    private javax.swing.JPanel _network_group;
    private javax.swing.JPanel _network_tab;
    private java.awt.Choice _nonCompactnessPenaltyComboBox;
    private javax.swing.JLabel _nonCompactnessPenaltyLabel;
    private javax.swing.JPanel _notificatons_tab;
    private javax.swing.JTextField _numIterativeScansTextField;
    private javax.swing.JTextField _numMostLikelyClustersGraph;
    private javax.swing.JLabel _numMostLikelyClustersGraphLabel;
    private javax.swing.JTextField _numberPowerReplications;
    private javax.swing.JLabel _numberPowerReplicationsLabel;
    private javax.swing.JTextField _number_oliveira_data_sets;
    private javax.swing.JLabel _number_oliveira_data_sets_label;
    private javax.swing.JPanel _oliveiras_f_group;
    private javax.swing.JRadioButton _onePerLocationIdRadioButton;
    private javax.swing.JPanel _otherOutputTab;
    private javax.swing.ButtonGroup _pValueButtonGroup;
    private javax.swing.JPanel _pValueOptionsGroup;
    private javax.swing.JPanel _panel_email_notifications;
    private javax.swing.JRadioButton _partOfRegularAnalysis;
    private javax.swing.JLabel _percentageOfPopFileLabel;
    private javax.swing.JLabel _percentageOfPopulationLabel;
    private javax.swing.JLabel _percentageOfStudyPeriodLabel;
    private javax.swing.JRadioButton _percentageTemporalRadioButton;
    private javax.swing.JCheckBox _performIsotonicScanCheckBox;
    private javax.swing.JCheckBox _performIterativeScanCheckBox;
    private javax.swing.JCheckBox _performPowerEvalautions;
    private javax.swing.JButton _populationFileBrowseButton;
    private javax.swing.JLabel _populationFileLabel;
    private javax.swing.JTextField _populationFileTextField;
    private javax.swing.ButtonGroup _powerEstimationButtonGroup;
    private javax.swing.JRadioButton _powerEstimationGumbel;
    private javax.swing.JRadioButton _powerEstimationMonteCarlo;
    private javax.swing.JLabel _powerEstimationTypeLabel;
    private javax.swing.ButtonGroup _powerEvaluationMethodButtonGroup;
    private javax.swing.JPanel _powerEvaluationTab;
    private javax.swing.JRadioButton _powerEvaluationWithCaseFile;
    private javax.swing.JRadioButton _powerEvaluationWithSpecifiedCases;
    private javax.swing.JLabel _powerEvaluationWithSpecifiedCasesLabel;
    private javax.swing.JPanel _powerEvaluationsGroup;
    private javax.swing.JCheckBox _printAsciiColumnHeaders;
    private javax.swing.JTextField _printTitle;
    private java.awt.Choice _prospective_frequency;
    private javax.swing.JPanel _prospective_frequency_group;
    private javax.swing.JCheckBox _purelySpatialDrilldown;
    private javax.swing.JRadioButton _radioDefaultPValues;
    private javax.swing.JRadioButton _radioEarlyTerminationPValues;
    private javax.swing.JRadioButton _radioGumbelPValues;
    private javax.swing.JRadioButton _radioStandardPValues;
    private javax.swing.JRadioButton _relaxedCoordinatesRadioButton;
    private javax.swing.JRadioButton _relaxedStudyPeriodCheckRadioButton;
    private javax.swing.JButton _removeDataSetButton;
    private javax.swing.JCheckBox _reportClusterRankCheckBox;
    private javax.swing.JPanel _reportClusterRankGroup;
    private javax.swing.JCheckBox _reportCriticalValuesCheckBox;
    private javax.swing.JPanel _reportCriticalValuesGroup;
    private javax.swing.JCheckBox _reportTemporalGraph;
    private javax.swing.JCheckBox _report_main_results_email;
    private javax.swing.JLabel _reportedMaxDistanceLabel;
    private javax.swing.JLabel _reportedPercentOfPopulationLabel;
    private javax.swing.JLabel _reportedPercentageOfPopFileLabel;
    private javax.swing.JCheckBox _reportedSpatialDistanceCheckBox;
    private javax.swing.JPanel _reportedSpatialOptionsGroup;
    private javax.swing.JCheckBox _reportedSpatialPopulationFileCheckBox;
    private javax.swing.JCheckBox _restrictReportedClustersCheckBox;
    private javax.swing.JCheckBox _restrictTemporalRangeCheckBox;
    private javax.swing.JButton _setDefaultButton;
    private javax.swing.JPanel _spaceTimeAjustmentsTab;
    private javax.swing.ButtonGroup _spatialAdjustmentsButtonGroup;
    private javax.swing.JPanel _spatialAdjustmentsGroup;
    private javax.swing.JRadioButton _spatialAdjustmentsNone;
    private javax.swing.JRadioButton _spatialAdjustmentsNonparametric;
    private javax.swing.JCheckBox _spatialDistanceCheckBox;
    private javax.swing.JPanel _spatialNeighborsTab;
    private javax.swing.JPanel _spatialOptionsGroup;
    private javax.swing.JPanel _spatialOutputTab;
    private javax.swing.JCheckBox _spatialPopulationFileCheckBox;
    private javax.swing.ButtonGroup _spatialWindowShapeButtonGroup;
    private javax.swing.JPanel _spatialWindowShapeGroup;
    private javax.swing.JPanel _spatialWindowTab;
    private javax.swing.JPanel _specialNeighborFilesGroup;
    private javax.swing.JCheckBox _specifiyMetaLocationsFileCheckBox;
    private javax.swing.JCheckBox _specifiyNeighborsFileCheckBox;
    private javax.swing.JLabel _startGenericRangeToLabel;
    private javax.swing.JLabel _startGenericWindowRangeLabel;
    private javax.swing.JTextField _startRangeEndDayTextField;
    private javax.swing.JTextField _startRangeEndGenericTextField;
    private javax.swing.JTextField _startRangeEndMonthTextField;
    private javax.swing.JTextField _startRangeEndYearTextField;
    private javax.swing.JTextField _startRangeStartDayTextField;
    private javax.swing.JTextField _startRangeStartGenericTextField;
    private javax.swing.JTextField _startRangeStartMonthTextField;
    private javax.swing.JTextField _startRangeStartYearTextField;
    private javax.swing.JLabel _startRangeToLabel;
    private javax.swing.JLabel _startWindowRangeLabel;
    private javax.swing.JLabel _strictCoordinatesLabel;
    private javax.swing.JRadioButton _strictCoordinatesRadioButton;
    private javax.swing.JRadioButton _strictStudyPeriodCheckRadioButton;
    private javax.swing.ButtonGroup _studyPeriodCheckButtonGroup;
    private javax.swing.JPanel _studyPeriodCheckGroup;
    private javax.swing.ButtonGroup _temporalGraphButtonGroup;
    private javax.swing.JRadioButton _temporalGraphMostLikely;
    private javax.swing.JRadioButton _temporalGraphMostLikelyX;
    private javax.swing.JTextField _temporalGraphPvalueCutoff;
    private javax.swing.JRadioButton _temporalGraphSignificant;
    private javax.swing.ButtonGroup _temporalOptionsButtonGroup;
    private javax.swing.JPanel _temporalOutputTab;
    private javax.swing.ButtonGroup _temporalTrendAdjButtonGroup;
    private javax.swing.JPanel _temporalTrendAdjGroup;
    private javax.swing.JRadioButton _temporalTrendAdjLogLinear;
    private javax.swing.JRadioButton _temporalTrendAdjLogLinearCalc;
    private javax.swing.JRadioButton _temporalTrendAdjNone;
    private javax.swing.JRadioButton _temporalTrendAdjNonparametric;
    private javax.swing.JRadioButton _temporalTrendAdjQuadCalc;
    private javax.swing.JPanel _temporalWindowTab;
    private javax.swing.JRadioButton _timeTemporalRadioButton;
    private javax.swing.JTextField _totalPowerCases;
    private javax.swing.JPanel _userDefinedRunTitle;
    private javax.swing.JPanel _windowCompletePanel;
    private javax.swing.JPanel _windowGenericPanel;
    private javax.swing.JScrollPane jScrollPane1;
    private javax.swing.JScrollPane jScrollPane3;
    private javax.swing.JTabbedPane jTabbedPane1;
    private java.awt.PopupMenu popupMenu1;
    // End of variables declaration//GEN-END:variables
}
