package org.satscan.gui;

import java.awt.Component;
import java.awt.Container;
import java.util.Calendar;
import java.util.GregorianCalendar;
import java.util.Vector;
import javax.swing.DefaultListModel;
import javax.swing.ImageIcon;
import javax.swing.JFileChooser;
import javax.swing.JPanel;
import javax.swing.JRootPane;
import javax.swing.JTextField;
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
import org.satscan.gui.utils.InputFileFilter;
import org.satscan.gui.utils.Utils;
import org.satscan.importer.FileImporter;
import org.satscan.app.UnknownEnumException;

/*
 * ParameterSettingsFrame.java
 *
 * Created on December 5, 2007, 11:07 AM
 */
/**
 *
 * @author  Hostovic
 */
public class AdvancedParameterSettingsFrame extends javax.swing.JInternalFrame {

    public enum FocusedTabSet {
        INPUT, ANALYSIS, OUTPUT
    };
    private JPanel _glass = null;
    private final JRootPane _rootPane;
    private final Component _rootPaneInitialGlass;
    private final UndoManager undo = new UndoManager();
    private final ParameterSettingsFrame _analysisSettingsWindow;
    private DefaultListModel _dataSetsListModel = new DefaultListModel();
    private Vector<String> _caseFilenames = new Vector<String>();
    private Vector<String> _controlFilenames = new Vector<String>();
    private Vector<String> _populationFilenames = new Vector<String>();
    private FocusedTabSet _focusedTabSet = FocusedTabSet.INPUT;
    private final int MAXIMUM_ADDITIONAL_SETS = 11;

    /**
     * Creates new form ParameterSettingsFrame
     */
    public AdvancedParameterSettingsFrame(final JRootPane rootPane, final ParameterSettingsFrame analysisSettingsWindow, final Parameters parameters) {
        initComponents();
        setFrameIcon(new ImageIcon(getClass().getResource("/SaTScan.png")));
        _rootPane = rootPane;
        _analysisSettingsWindow = analysisSettingsWindow;
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
        _glass.add(this);
        SetupInterface(parameters);
    }

    /**
     * recursively searches Container objects contained in 'rootComponent' for for 'searchComponent'.
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
            _analysisSettingsWindow.enableAdvancedButtons();
        }
    }

    /**
     *
     */
    private synchronized void startModal(FocusedTabSet focusedTabSet) {
        if (_glass != null) {
            _rootPane.setGlassPane(_glass);
            _glass.setVisible(true); // Change glass pane to our panel
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
                jTabbedPane1.addTab("Space and Time Adjustments", null, _spaceTimeAjustmentsTab, null);
                jTabbedPane1.addTab("Inference", null, _inferenceTab, null);
                break;
            case OUTPUT:
                setTitle("Advanced Output Features");
                jTabbedPane1.addTab("Clusters Reported", null, _clustersReportedTab, null);
                break;
            case INPUT:
            default:
                setTitle("Advanced Input Features");
                jTabbedPane1.addTab("Multiple Data Sets", null, _multipleDataSetsTab, null);
                jTabbedPane1.addTab("Data Checking", null, _dataCheckingTab, null);
                jTabbedPane1.addTab("Non-Eucledian Neighbors", null, _nonEucledianNeighborsTab, null);
                jTabbedPane1.addTab("Mutliple Coordinates Per Location", null, _multipleCoordinatesTab, null);
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

    /** Modally shows import dialog. */
    public void LaunchImporter(String sFileName, FileImporter.InputFileType eFileType) {
        //try {
        ImportWizardDialog wizard = new ImportWizardDialog(SaTScanApplication.getInstance(), sFileName, eFileType, _analysisSettingsWindow.getModelControlType(), _analysisSettingsWindow.getCoordinatesType());
        wizard.setVisible(true);
        if (!wizard.getCancelled()) {
            switch (eFileType) {  // set parameters
                case Case:
                    _caseFileTextField.setText(wizard.getDestinationFilename());
                    break;
                case Control:
                    _controlFileTextField.setText(wizard.getDestinationFilename());
                    break;
                case Population:
                    _populationFileTextField.setText(wizard.getDestinationFilename());
                    break;
                case MaxCirclePopulation:
                    _maxCirclePopulationFilenameTextField.setText(wizard.getDestinationFilename());
                    break;
                case AdjustmentsByRR:
                    _adjustmentsByRelativeRisksFileTextField.setText(wizard.getDestinationFilename());
                    break;
                default:
                    throw new UnknownEnumException(eFileType);
            }
        }
    }

    /** Enables dates of flexible temporal window and prospective surveillance groups.
     * Enabling is determined through:
     * - querying the 'precision of time' control contained in the analysis window
     * - the Enabled property of the TGroupBox of which dates are contained
     * - the Enabled and Checked properties of the TCheckBox that indicates whether
     * user wishes to adjust for earlier analyses. */
    public void EnableDatesByTimePrecisionUnits() {
        boolean bYears = true, bMonths = true, bDays = true,
                bEnable = _flexibleTemporalWindowDefinitionGroup.isEnabled() &&
                _restrictTemporalRangeCheckBox.isEnabled() && _restrictTemporalRangeCheckBox.isSelected();

        switch (_analysisSettingsWindow.getPrecisionOfTimesControlType()) {
            case NONE:
            case DAY:
                bYears = bMonths = bDays = bEnable;
                break;
            case YEAR:
                bYears = bEnable;
                bMonths = bDays = false;
                break;
            case MONTH:
                bYears = bMonths = bEnable;
                bDays = false;
                break;
        }

        //enable start range dates
        _startRangeStartYearTextField.setEnabled(bYears);
        _startRangeStartMonthTextField.setEnabled(bMonths);
        if (!_startRangeStartMonthTextField.isEnabled() && bEnable) {
            _startRangeStartMonthTextField.setText("1");
        }
        _startRangeStartDayTextField.setEnabled(bDays);
        if (!_startRangeStartDayTextField.isEnabled() && bEnable) {
            _startRangeStartDayTextField.setText("1");
        }
        _startRangeEndYearTextField.setEnabled(bYears);
        _startRangeEndMonthTextField.setEnabled(bMonths);
        if (!_startRangeEndMonthTextField.isEnabled() && bEnable) {
            _startRangeEndMonthTextField.setText("12");
        }
        _startRangeEndDayTextField.setEnabled(bDays);
        if (!_startRangeEndDayTextField.isEnabled() && bEnable) {
            GregorianCalendar thisCalender = new GregorianCalendar();
            thisCalender.set(Calendar.YEAR, Integer.parseInt(_startRangeEndYearTextField.getText()));
            thisCalender.set(Calendar.MONTH, Integer.parseInt(_startRangeEndMonthTextField.getText()) - 1);
            _startRangeEndDayTextField.setText(Integer.toString(thisCalender.getActualMaximum(Calendar.DAY_OF_MONTH)));
        }
        //enable end range dates
        _endRangeStartYearTextField.setEnabled(bYears);
        _endRangeStartMonthTextField.setEnabled(bMonths);
        if (!_endRangeStartMonthTextField.isEnabled() && bEnable) {
            _endRangeStartMonthTextField.setText("1");
        }
        _endRangeStartDayTextField.setEnabled(bDays);
        if (!_endRangeStartDayTextField.isEnabled() && bEnable) {
            _endRangeStartDayTextField.setText("1");
        }
        _endRangeEndYearTextField.setEnabled(bYears);
        _endRangeEndMonthTextField.setEnabled(bMonths);
        if (!_endRangeEndMonthTextField.isEnabled() && bEnable) {
            _endRangeEndMonthTextField.setText("12");
        }
        _endRangeEndDayTextField.setEnabled(bDays);
        if (!_endRangeEndDayTextField.isEnabled() && bEnable) {
            GregorianCalendar thisCalender = new GregorianCalendar();
            thisCalender.set(Calendar.YEAR, Integer.parseInt(_endRangeEndYearTextField.getText()));
            thisCalender.set(Calendar.MONTH, Integer.parseInt(_endRangeEndMonthTextField.getText()) - 1);
            _endRangeEndDayTextField.setText(Integer.toString(thisCalender.getActualMaximum(Calendar.DAY_OF_MONTH)));
        }
        //enable date contained in prospective surveillance group
        EnableProspectiveStartDate();
    }

    private void EnableAdditionalDataSetsGroup(boolean bEnable) {
        bEnable &= (_performIsotonicScanCheckBox.isEnabled() ? !_performIsotonicScanCheckBox.isSelected() : true);
        bEnable &= _analysisSettingsWindow.getModelControlType() != Parameters.ProbabilityModelType.HOMOGENEOUSPOISSON;
        _additionalDataSetsGroup.setEnabled(bEnable);

        EnableDataSetList(_additionalDataSetsGroup.isEnabled());
        EnableNewButton();
        EnableRemoveButton();
        EnableInputFileEdits(_additionalDataSetsGroup.isEnabled());
        _caseFileLabel.setEnabled(_additionalDataSetsGroup.isEnabled());
        _controlFileLabel.setEnabled(_additionalDataSetsGroup.isEnabled());
        _populationFileLabel.setEnabled(_additionalDataSetsGroup.isEnabled());
        EnableDataSetPurposeControls(_additionalDataSetsGroup.isEnabled());
    }

    /** Enables neighbors file group. */
    private void EnableNeighborsFileGroup(boolean bEnable) {
        _specialNeighborFilesGroup.setEnabled(bEnable);
        _specifiyNeighborsFileCheckBox.setEnabled(bEnable);
        _neighborsFileTextField.setEnabled(bEnable && _specifiyNeighborsFileCheckBox.isSelected());
        _neighborsFileBrowseButton.setEnabled(bEnable && _specifiyNeighborsFileCheckBox.isSelected());

        _specifiyMetaLocationsFileCheckBox.setEnabled(bEnable && _specifiyNeighborsFileCheckBox.isSelected());
        _metaLocationsFileTextField.setEnabled(bEnable && _specifiyMetaLocationsFileCheckBox.isEnabled() && _specifiyMetaLocationsFileCheckBox.isSelected());
        _metaLocationsFileBrowseButton.setEnabled(bEnable && _specifiyMetaLocationsFileCheckBox.isEnabled() && _specifiyMetaLocationsFileCheckBox.isSelected());
    }

    private void EnableCoordinatesCheckGroup(boolean bEnable) {
        _strictCoordinatesRadioButton.setEnabled(bEnable);
        _relaxedCoordinatesRadioButton.setEnabled(bEnable);
        _strictCoordinatesLabel.setEnabled(bEnable);
        _relaxedCoordinatesLabel.setEnabled(bEnable);
    }

    private void EnableMultipleLocationsGroup(boolean bEnable) {
        bEnable &= !_specifiyNeighborsFileCheckBox.isSelected();
        _multipleSetsSpatialCoordinatesGroup.setEnabled(bEnable);
        _onePerLocationIdRadioButton.setEnabled(bEnable);
        _atLeastOneRadioButton.setEnabled(bEnable);
        _allLocationsRadioButton.setEnabled(bEnable);
    }

    /** returns spatial adjustment type from control index  */
    private Parameters.SpatialAdjustmentType getAdjustmentSpatialControlType() {
        Parameters.SpatialAdjustmentType eReturn = null;

        if (_spatialAdjustmentsNoneRadioButton.isSelected()) {
            eReturn = Parameters.SpatialAdjustmentType.NO_SPATIAL_ADJUSTMENT;
        }
        if (_spatialAdjustmentsSpatialStratifiedRadioButton.isSelected()) {
            eReturn = Parameters.SpatialAdjustmentType.SPATIALLY_STRATIFIED_RANDOMIZATION;
        }
        return eReturn;
    }

    /** enables adjustment options controls */
    private void EnableAdjustmentsGroup(boolean bEnable) {
        _knownAdjustmentsGroup.setEnabled(bEnable);
        _adjustForKnownRelativeRisksCheckBox.setEnabled(bEnable);
        _adjustmentsByRelativeRisksFileLabel.setEnabled(bEnable && _adjustForKnownRelativeRisksCheckBox.isSelected());
        _adjustmentsByRelativeRisksFileTextField.setEnabled(bEnable && _adjustForKnownRelativeRisksCheckBox.isSelected());
        _adjustmentsFileBrowseButton.setEnabled(bEnable && _adjustForKnownRelativeRisksCheckBox.isSelected());
        _adjustmentsFileImportButton.setEnabled(bEnable && _adjustForKnownRelativeRisksCheckBox.isSelected());
    }

    private void EnableIterativeScanOptionsGroup(boolean bEnable) {
        _iterativeScanGroup.setEnabled(bEnable);
        _performIterativeScanCheckBox.setEnabled(bEnable);
        _maxIterativeScansLabel.setEnabled(_performIterativeScanCheckBox.isSelected() && bEnable);
        _numIterativeScansTextField.setEnabled(_performIterativeScanCheckBox.isSelected() && bEnable);
        _iterativeCutoffLabel.setEnabled(_performIterativeScanCheckBox.isSelected() && bEnable);
        _iterativeScanCutoffTextField.setEnabled(_performIterativeScanCheckBox.isSelected() && bEnable);
    }

    public void EnableSettingsForAnalysisModelCombination() {
        boolean bPoisson = _analysisSettingsWindow.getModelControlType() == Parameters.ProbabilityModelType.POISSON,
                bSpaceTimePermutation = _analysisSettingsWindow.getModelControlType() == Parameters.ProbabilityModelType.SPACETIMEPERMUTATION,
                bExponential = _analysisSettingsWindow.getModelControlType() == Parameters.ProbabilityModelType.EXPONENTIAL,
                bBernoulli = _analysisSettingsWindow.getModelControlType() == Parameters.ProbabilityModelType.BERNOULLI;

        switch (_analysisSettingsWindow.getAnalysisControlType()) {
            case PURELYSPATIAL:
                EnableAdjustmentForTimeTrendOptionsGroup(false, false, false, false);
                EnableAdjustmentForSpatialOptionsGroup(false, false);
                enableSpatialOptionsGroup(true, false);
                EnableWindowShapeGroup(true);
                EnableTemporalOptionsGroup(false, false, false);
                EnableProspectiveSurveillanceGroup(false);
                EnableOutputOptions(true);
                EnableNeighborsFileGroup(true);
                EnableCoordinatesCheckGroup(true);
                EnableMultipleLocationsGroup(true);
                EnableAdditionalDataSetsGroup(true);
                EnableIsotonicScan(bPoisson || bBernoulli);
                EnableIterativeScanOptionsGroup(true);
                break;
            case PURELYTEMPORAL:
                EnableAdjustmentForTimeTrendOptionsGroup(bPoisson, false, bPoisson, bPoisson);
                EnableAdjustmentForSpatialOptionsGroup(false, false);
                enableSpatialOptionsGroup(false, false);
                EnableWindowShapeGroup(false);
                EnableTemporalOptionsGroup(true, false, true);
                EnableProspectiveSurveillanceGroup(false);
                EnableOutputOptions(false);
                EnableNeighborsFileGroup(false);
                EnableCoordinatesCheckGroup(false);
                EnableMultipleLocationsGroup(false);
                EnableAdditionalDataSetsGroup(true);
                EnableIsotonicScan(false);
                EnableIterativeScanOptionsGroup(true);
                break;
            case SPACETIME:
                EnableAdjustmentForTimeTrendOptionsGroup(bPoisson,
                        bPoisson && getAdjustmentSpatialControlType() != Parameters.SpatialAdjustmentType.SPATIALLY_STRATIFIED_RANDOMIZATION,
                        bPoisson, bPoisson);
                EnableAdjustmentForSpatialOptionsGroup(bPoisson, getAdjustmentTimeTrendControlType() != Parameters.TimeTrendAdjustmentType.STRATIFIED_RANDOMIZATION);
                enableSpatialOptionsGroup(true, !bSpaceTimePermutation);
                EnableWindowShapeGroup(true);
                EnableTemporalOptionsGroup(true, !bSpaceTimePermutation, true);
                EnableProspectiveSurveillanceGroup(false);
                EnableOutputOptions(true);
                EnableNeighborsFileGroup(true);
                EnableCoordinatesCheckGroup(true);
                EnableMultipleLocationsGroup(true);
                EnableAdditionalDataSetsGroup(true);
                EnableIsotonicScan(false);
                EnableIterativeScanOptionsGroup(false);
                break;
            case PROSPECTIVESPACETIME:
                EnableAdjustmentForTimeTrendOptionsGroup(bPoisson,
                        bPoisson && getAdjustmentSpatialControlType() != Parameters.SpatialAdjustmentType.SPATIALLY_STRATIFIED_RANDOMIZATION,
                        bPoisson, bPoisson);
                EnableAdjustmentForSpatialOptionsGroup(bPoisson, getAdjustmentTimeTrendControlType() != Parameters.TimeTrendAdjustmentType.STRATIFIED_RANDOMIZATION);
                enableSpatialOptionsGroup(true, !bSpaceTimePermutation);
                EnableWindowShapeGroup(true);
                EnableTemporalOptionsGroup(true, !bSpaceTimePermutation, false);
                EnableProspectiveSurveillanceGroup(true);
                EnableOutputOptions(true);
                EnableNeighborsFileGroup(true);
                EnableCoordinatesCheckGroup(true);
                EnableMultipleLocationsGroup(true);
                EnableAdditionalDataSetsGroup(true);
                EnableIsotonicScan(false);
                EnableIterativeScanOptionsGroup(false);
                break;
            case PROSPECTIVEPURELYTEMPORAL:
                EnableAdjustmentForTimeTrendOptionsGroup(bPoisson, false, bPoisson, bPoisson);
                EnableAdjustmentForSpatialOptionsGroup(false, false);
                enableSpatialOptionsGroup(false, false);
                EnableWindowShapeGroup(false);
                EnableTemporalOptionsGroup(true, false, false);
                EnableProspectiveSurveillanceGroup(true);
                EnableOutputOptions(false);
                EnableNeighborsFileGroup(false);
                EnableCoordinatesCheckGroup(false);
                EnableMultipleLocationsGroup(false);
                EnableAdditionalDataSetsGroup(true);
                EnableIsotonicScan(false);
                EnableIterativeScanOptionsGroup(true);
                break;
            case SPATIALVARTEMPTREND:
                EnableAdjustmentForTimeTrendOptionsGroup(false, false, false, false);
                EnableAdjustmentForSpatialOptionsGroup(false, false);
                enableSpatialOptionsGroup(true, false);
                EnableTemporalOptionsGroup(false, false, false);
                EnableProspectiveSurveillanceGroup(false);
                EnableOutputOptions(true);
                EnableNeighborsFileGroup(true);
                EnableCoordinatesCheckGroup(true);
                EnableIterativeScanOptionsGroup(true);
                EnableMultipleLocationsGroup(true);
                EnableAdditionalDataSetsGroup(false);
                EnableIsotonicScan(false);
                EnableWindowShapeGroup(true);
                break;
        }
        EnableAdjustmentsGroup(bPoisson);
    }

    /** Sets caption of spatial distance radio button based upon coordinates group setting. */
    public void SetSpatialDistanceCaption() {
        String sRadioCaption = "", sLabelCaption = "";

        sRadioCaption = String.format("is %1$s with a", (_circularRadioButton.isSelected() ? "a circle" : "an ellipse"));
        _distancePrefixLabel.setText(sRadioCaption);
        //getJEdtMaxSpatialRadius().setLocation(new java.awt.Point(getJRdoCircular().isSelected() ? 159 : 168, getJEdtMaxSpatialRadius().getLocation().y));
        //jLblMaxRadius.setLocation(new java.awt.Point(getJRdoCircular().isSelected() ? 227 : 236 , jLblMaxRadius.getLocation().y));
        sRadioCaption = String.format("%1$s with a", (_circularRadioButton.isSelected() ? "a circle" : "an ellipse"));
        _reportedMaxDistanceLabel.setText(sRadioCaption);
        //getJEdtMaxReportedSpatialRadius().setLocation(new java.awt.Point(getJRdoCircular().isSelected() ? 170 : 178 , getJEdtMaxReportedSpatialRadius().getLocation().y));
        //_maxRadiusLabel.setLocation(new java.awt.Point(getJRdoCircular().isSelected() ? 238 : 245, jLblMaxReportedRadius.getLocation().y));
        switch (_analysisSettingsWindow.getCoordinatesType()) {
            case CARTESIAN:
                sLabelCaption = String.format("Cartesian units %1$s", (_circularRadioButton.isSelected() ? "radius" : "minor axis"));
                break;
            case LATLON:
                sLabelCaption = String.format("kilometer %1$s", (_circularRadioButton.isSelected() ? "radius" : "minor axis"));
                break;
        }
        _maxRadiusLabel.setText(sLabelCaption);
        _maxReportedRadiusLabel.setText(sLabelCaption);
    }

    public void UpdateMaxiumSpatialTextCaptions() {
        switch (_analysisSettingsWindow.getModelControlType()) {
            case POISSON:
            case HOMOGENEOUSPOISSON:    
            case BERNOULLI:
            case ORDINAL:
            case CATEGORICAL:  
            case WEIGHTEDNORMAL:    
            case NORMAL:
            case EXPONENTIAL:
                _percentageOfPopulationLabel.setText("percent of the study period (<= 90%, default = 50%)");
                break;
            case SPACETIMEPERMUTATION:
                _percentageOfPopulationLabel.setText("percent of the study period (<= 50%, default = 50%)");
                break;
        }
    }

    public void UpdateMaxiumTemporalTextCaptions() {
        switch (_analysisSettingsWindow.getTimeAggregationControlType()) {
            case YEAR:
                _maxTemporalTimeUnitsLabel.setText("years");
                break;
            case MONTH:
                _maxTemporalTimeUnitsLabel.setText("months");
                break;
            case DAY:
                _maxTemporalTimeUnitsLabel.setText("days");
                break;
        }
    }

    /**
     * Enables/disables TListBox that list defined data sets
     */
    private void EnableDataSetList(boolean bEnable) {
        _inputDataSetsList.setEnabled(bEnable && _additionalDataSetsGroup.isEnabled());
    }

    /**
     * enables or disables the New button on the Input tab
     */
    private void EnableNewButton() {
         boolean bEnable = _additionalDataSetsGroup.isEnabled();
        _addDataSetButton.setEnabled(_inputDataSetsList.getModel().getSize() < MAXIMUM_ADDITIONAL_SETS ? bEnable : false);
    }

    /**
     * enables or disables the New button on the Input tab
     */
    private void EnableRemoveButton() {
        boolean bEnable = _additionalDataSetsGroup.isEnabled();
        _removeDataSetButton.setEnabled(_inputDataSetsList.getModel().getSize() > 0 ? bEnable : false);
    }

    /** Checks to determine if only default values are set in the dialog
     *  Returns true if only default values are set
     *  Returns false if user specified a value other than a default
     */
    public boolean getDefaultsSetForInputOptions() {
        boolean bReturn = true;

        bReturn &= (_inputDataSetsList.getModel().getSize() == 0);
        bReturn &= (_multivariateAdjustmentsRadioButton.isSelected());
        bReturn &= (_strictStudyPeriodCheckRadioButton.isSelected());
        bReturn &= (_strictCoordinatesRadioButton.isSelected());
        bReturn &= (_specifiyNeighborsFileCheckBox.isSelected() == false);
        bReturn &= (_neighborsFileTextField.getText().length() == 0);
        bReturn &= (_specifiyMetaLocationsFileCheckBox.isSelected() == false);
        bReturn &= (_metaLocationsFileTextField.getText().length() == 0);

        return bReturn;
    }

//	** Checks to determine if only default values are set in the dialog
//	** Returns true if only default values are set on enabled controls
//	** Returns false if user specified a value other than a default
    public boolean getDefaultsSetForAnalysisOptions() {
        boolean bReturn = true;

        // Inference tab
        bReturn &= (_adjustForEarlierAnalysesCheckBox.isSelected() == false);
        bReturn &= (_reportCriticalValuesCheckBox.isSelected() == false);
        bReturn &= (_terminateEarlyCheckBox.isSelected() == false);
        bReturn &= (Integer.parseInt(_prospectiveStartDateYearTextField.getText()) == 1900 || Integer.parseInt(_prospectiveStartDateYearTextField.getText()) == 2000);
        bReturn &= (Integer.parseInt(_prospectiveStartDateMonthTextField.getText()) == 12);
        bReturn &= (Integer.parseInt(_prospectiveStartDateDayTextField.getText()) == 31);
        bReturn &= (_performIterativeScanCheckBox.isSelected() == false);
        bReturn &= (Integer.parseInt(_numIterativeScansTextField.getText()) == 10);
        bReturn &= (Double.parseDouble(_iterativeScanCutoffTextField.getText()) == 0.05);

        // Spatial Window tab
        bReturn &= (Double.parseDouble(_maxSpatialClusterSizeTextField.getText()) == 50);
        bReturn &= (_spatialPopulationFileCheckBox.isSelected() == false);
        bReturn &= (Double.parseDouble(_maxSpatialPercentFileTextField.getText()) == 50);
        bReturn &= (Double.parseDouble(_maxSpatialRadiusTextField.getText()) == 1);
        bReturn &= (_spatialDistanceCheckBox.isSelected() == false);
        bReturn &= (_maxCirclePopulationFilenameTextField.getText().equals(""));
        bReturn &= (_inclPureTempClustCheckBox.isSelected() == false);
        bReturn &= (_circularRadioButton.isSelected() == true);
        bReturn &= (_nonCompactnessPenaltyComboBox.getSelectedIndex() == 1);
        bReturn &= (_onePerLocationIdRadioButton.isSelected() == true);

        // Temporal tab
        bReturn &= (_percentageTemporalRadioButton.isSelected() == true);
        bReturn &= (Double.parseDouble(_maxTemporalClusterSizeTextField.getText()) == 50);
        bReturn &= (Integer.parseInt(_maxTemporalClusterSizeUnitsTextField.getText()) == 1);
        bReturn &= (_includePureSpacClustCheckBox.isSelected() == false);
        bReturn &= (Integer.parseInt(_startRangeStartYearTextField.getText()) == 1900 || Integer.parseInt(_startRangeStartYearTextField.getText()) == 2000);
        bReturn &= (Integer.parseInt(_startRangeStartMonthTextField.getText()) == 1);
        bReturn &= (Integer.parseInt(_startRangeStartDayTextField.getText()) == 1);
        bReturn &= (Integer.parseInt(_startRangeEndYearTextField.getText()) == 1900 || Integer.parseInt(_startRangeEndYearTextField.getText()) == 2000);
        bReturn &= (Integer.parseInt(_startRangeEndMonthTextField.getText()) == 1 || Integer.parseInt(_startRangeEndMonthTextField.getText()) == 12);
        bReturn &= (Integer.parseInt(_startRangeEndDayTextField.getText()) == 1 || Integer.parseInt(_startRangeEndDayTextField.getText()) == 31);
        bReturn &= (Integer.parseInt(_endRangeStartYearTextField.getText()) == 1900 || Integer.parseInt(_endRangeStartYearTextField.getText()) == 2000);
        bReturn &= (Integer.parseInt(_endRangeStartMonthTextField.getText()) == 1 || Integer.parseInt(_endRangeStartMonthTextField.getText()) == 12);
        bReturn &= (Integer.parseInt(_endRangeStartDayTextField.getText()) == 1 || Integer.parseInt(_endRangeStartDayTextField.getText()) == 31);
        bReturn &= (Integer.parseInt(_endRangeEndYearTextField.getText()) == 1900 || Integer.parseInt(_endRangeEndYearTextField.getText()) == 2000);
        bReturn &= (Integer.parseInt(_endRangeEndMonthTextField.getText()) == 12);
        bReturn &= (Integer.parseInt(_endRangeEndDayTextField.getText()) == 31);
        bReturn &= (_restrictTemporalRangeCheckBox.isSelected() == false);

        // Risk tab
        bReturn &= (_adjustForKnownRelativeRisksCheckBox.isSelected() == false);
        bReturn &= (_adjustmentsByRelativeRisksFileTextField.getText().equals(""));
        bReturn &= (_temporalTrendAdjNoneRadioButton.isSelected() == true);
        bReturn &= (Double.parseDouble(_logLinearTextField.getText()) == 0);
        bReturn &= (_spatialAdjustmentsNoneRadioButton.isSelected() == true);

        return bReturn;
    }
    //	** Checks to determine if only default values are set in the dialog
    //	** Returns true if only default values are set
    //	** Returns false if user specified a value other than a default
    public boolean getDefaultsSetForOutputOptions() {
        boolean bReturn = true;

        // Output tab
        bReturn &= (_criteriaSecClustersNoGeoLapRadioButton.isSelected() == true);
        bReturn &= (Double.parseDouble(_maxReportedSpatialClusterSizeTextField.getText()) == 50);
        bReturn &= (Double.parseDouble(_maxReportedSpatialPercentFileTextField.getText()) == 50);
        bReturn &= (_reportedSpatialPopulationFileCheckBox.isSelected() == false);
        bReturn &= (Double.parseDouble(_maxReportedSpatialRadiusTextField.getText()) == 1);
        bReturn &= (_reportedSpatialDistanceCheckBox.isSelected() == false);
        bReturn &= (_restrictReportedClustersCheckBox.isSelected() == false);

        return bReturn;
    }

    private Parameters.SpatialAdjustmentType getSpatialAdjustmentType() {
        Parameters.SpatialAdjustmentType eReturn = null;

        if (_spatialAdjustmentsNoneRadioButton.isSelected()) {
            eReturn = Parameters.SpatialAdjustmentType.NO_SPATIAL_ADJUSTMENT;
        }
        if (_spatialAdjustmentsSpatialStratifiedRadioButton.isSelected()) {
            eReturn = Parameters.SpatialAdjustmentType.SPATIALLY_STRATIFIED_RANDOMIZATION;
        }
        return eReturn;
    }

    private Parameters.CriteriaSecondaryClustersType getCriteriaSecondaryClustersType() {
        Parameters.CriteriaSecondaryClustersType eReturn = null;

        if (_criteriaSecClustersNoGeoLapRadioButton.isSelected()) {
            eReturn = Parameters.CriteriaSecondaryClustersType.NOGEOOVERLAP;
        }
        if (_criteriaSecClustersNoCentersRadioButton.isSelected()) {
            eReturn = Parameters.CriteriaSecondaryClustersType.NOCENTROIDSINOTHER;
        }
        if (_criteriaSecClustersNoCentersMoreRadioButton.isSelected()) {
            eReturn = Parameters.CriteriaSecondaryClustersType.NOCENTROIDSINMORELIKE;
        }
        if (_criteriaSecClustersNoCentersLessRadioButton.isSelected()) {
            eReturn = Parameters.CriteriaSecondaryClustersType.NOCENTROIDSINLESSLIKE;
        }
        if (_criteriaSecClustersNoCentersEachRadioButton.isSelected()) {
            eReturn = Parameters.CriteriaSecondaryClustersType.NOPAIRSINEACHOTHERS;
        }
        if (_criteriaSecClustersNoRestrictionsRadioButton.isSelected()) {
            eReturn = Parameters.CriteriaSecondaryClustersType.NORESTRICTIONS;
        }
        return eReturn;
    }

    private Parameters.NonCompactnessPenaltyType getNonCompactnessPenaltyType() {
        Parameters.NonCompactnessPenaltyType eReturn = null;

        if (_nonCompactnessPenaltyComboBox.getSelectedIndex() == 0) {
            eReturn = Parameters.NonCompactnessPenaltyType.NOPENALTY;
        }
        if (_nonCompactnessPenaltyComboBox.getSelectedIndex() == 1) {
            eReturn = Parameters.NonCompactnessPenaltyType.MEDIUMPENALTY;
        }
        if (_nonCompactnessPenaltyComboBox.getSelectedIndex() == 2) {
            eReturn = Parameters.NonCompactnessPenaltyType.STRONGPENALTY;
        }
        return eReturn;
    }

    /** returns MultipleCoordinatesType for controls */
    Parameters.MultipleCoordinatesType getMultipleCoordinatesType() {
        Parameters.MultipleCoordinatesType eReturn;

        if (_onePerLocationIdRadioButton.isSelected()) {
            eReturn = Parameters.MultipleCoordinatesType.ONEPERLOCATION;
        } else if (_atLeastOneRadioButton.isSelected()) {
            eReturn = Parameters.MultipleCoordinatesType.ATLEASTONELOCATION;
        } else // (_allLocationsRadioButton.isSelected())
        {
            eReturn = Parameters.MultipleCoordinatesType.ALLLOCATIONS;
        }

        return eReturn;
    }

    /** returns geographical coordinates data type for control */
    private Parameters.CoordinatesDataCheckingType getCoordinatesDataCheckingTypeFromControl() {
        Parameters.CoordinatesDataCheckingType eReturn = null;

        if (_strictCoordinatesRadioButton.isSelected()) {
            eReturn = Parameters.CoordinatesDataCheckingType.STRICTCOORDINATES;
        } else if (_relaxedCoordinatesRadioButton.isSelected()) {
            eReturn = Parameters.CoordinatesDataCheckingType.RELAXEDCOORDINATES;
        }
        //else
        //    ZdGenerateException("Unknown geographical coordinates.", "GetCoordinatesDataCheckingTypeFromControl()");

        return eReturn;
    }

    /** sets CParameters class with settings in form */
    public void SaveParameterSettings(Parameters parameters) {
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
        parameters.SetTimeTrendAdjustmentType(_temporalTrendAdjGroup.isEnabled() ? getAdjustmentTimeTrendControlType().ordinal() : Parameters.TimeTrendAdjustmentType.NOTADJUSTED.ordinal());
        parameters.SetTimeTrendAdjustmentPercentage(Double.parseDouble(_logLinearTextField.getText()));
        parameters.SetSpatialAdjustmentType(_spatialAdjustmentsGroup.isEnabled() ? getSpatialAdjustmentType().ordinal() : Parameters.SpatialAdjustmentType.NO_SPATIAL_ADJUSTMENT.ordinal());
        parameters.SetTerminateSimulationsEarly(_terminateEarlyCheckBox.isSelected());
        parameters.SetReportCriticalValues(_reportCriticalValuesCheckBox.isSelected());
        if (_restrictTemporalRangeCheckBox.isEnabled() && _restrictTemporalRangeCheckBox.isSelected()) {
            parameters.SetIncludeClustersType(Parameters.IncludeClustersType.CLUSTERSINRANGE.ordinal());
        } else {
            parameters.SetIncludeClustersType(Parameters.IncludeClustersType.ALLCLUSTERS.ordinal());
        }
        sString = _startRangeStartYearTextField.getText() + "/" + _startRangeStartMonthTextField.getText() + "/" + _startRangeStartDayTextField.getText();
        parameters.SetStartRangeStartDate(sString);
        sString = _startRangeEndYearTextField.getText() + "/" + _startRangeEndMonthTextField.getText() + "/" + _startRangeEndDayTextField.getText();
        parameters.SetStartRangeEndDate(sString);
        sString = _endRangeStartYearTextField.getText() + "/" + _endRangeStartMonthTextField.getText() + "/" + _endRangeStartDayTextField.getText();
        parameters.SetEndRangeStartDate(sString);
        sString = _endRangeEndYearTextField.getText() + "/" + _endRangeEndMonthTextField.getText() + "/" + _endRangeEndDayTextField.getText();
        parameters.SetEndRangeEndDate(sString);
        parameters.SetCriteriaForReportingSecondaryClusters(getCriteriaSecondaryClustersType().ordinal());

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
        parameters.SetAdjustForEarlierAnalyses(_adjustForEarlierAnalysesCheckBox.isEnabled() && _adjustForEarlierAnalysesCheckBox.isSelected());
        if (_adjustForEarlierAnalysesCheckBox.isEnabled() && !_adjustForEarlierAnalysesCheckBox.isSelected()) {
            sString = _analysisSettingsWindow.getEdtStudyPeriodEndDateYearText() + "/" + _analysisSettingsWindow.getEdtStudyPeriodEndDateMonthText() + "/" + _analysisSettingsWindow.getEdtStudyPeriodEndDateDayText();
        } else {
            sString = _prospectiveStartDateYearTextField.getText() + "/" + _prospectiveStartDateMonthTextField.getText() + "/" + _prospectiveStartDateDayTextField.getText();
        }
        parameters.SetProspectiveStartDate(sString);
        parameters.SetMaxCirclePopulationFileName(_maxCirclePopulationFilenameTextField.getText());
        parameters.SetMaximumTemporalClusterSize(_percentageTemporalRadioButton.isSelected() ? Double.parseDouble(_maxTemporalClusterSizeTextField.getText()) : Double.parseDouble(_maxTemporalClusterSizeUnitsTextField.getText()));
        parameters.SetMaximumTemporalClusterSizeType(_percentageTemporalRadioButton.isSelected() ? Parameters.TemporalSizeType.PERCENTAGETYPE.ordinal() : Parameters.TemporalSizeType.TIMETYPE.ordinal());
        parameters.SetIncludePurelyTemporalClusters(_inclPureTempClustCheckBox.isEnabled() && _inclPureTempClustCheckBox.isSelected());
        parameters.SetIncludePurelySpatialClusters(_includePureSpacClustCheckBox.isEnabled() && _includePureSpacClustCheckBox.isSelected());
        parameters.SetMultipleCoordinatesType(getMultipleCoordinatesType().ordinal());
    }

    public boolean isNonEucledianNeighborsSelected() {
        return _specifiyNeighborsFileCheckBox.isEnabled() && _specifiyNeighborsFileCheckBox.isSelected();
    }

    /** returns maximum temporal cluster size type for control */
    public Parameters.TemporalSizeType GetMaxTemporalClusterSizeControlType() {
        Parameters.TemporalSizeType eReturn = null;

        if (_percentageTemporalRadioButton.isSelected()) {
            eReturn = Parameters.TemporalSizeType.PERCENTAGETYPE;
        }
        if (_timeTemporalRadioButton.isSelected()) {
            eReturn = Parameters.TemporalSizeType.TIMETYPE;
        }
        return eReturn;
    }

    /** returns maximum temporal cluster size from appropriate control */
    public double GetMaxTemporalClusterSizeFromControl() {
        double dReturn;

        switch (GetMaxTemporalClusterSizeControlType()) {
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
    private void ValidateInputFiles() {
        boolean bAnalysisIsPurelyTemporal = _analysisSettingsWindow.getAnalysisControlType() == Parameters.AnalysisType.PURELYTEMPORAL ||
                _analysisSettingsWindow.getAnalysisControlType() == Parameters.AnalysisType.PROSPECTIVEPURELYTEMPORAL;
        boolean bFirstDataSetHasPopulationFile = _analysisSettingsWindow.getEdtPopFileNameText().length() > 0;

        for (int i = 0; i < _caseFilenames.size(); i++) {
            //Ensure that controls have this dataset display, should we need to
            //show window regarding an error with settings.
            _inputDataSetsList.setSelectedIndex(i);
            //validate the case file for this dataset
            if (_caseFilenames.get(i).length() == 0) {
                throw new AdvFeaturesExpection("Please specify a case file for this additional data set.", FocusedTabSet.INPUT, (Component) _caseFileTextField);
            }
            if (!FileAccess.ValidateFileAccess(_caseFilenames.get(i), false)) {
                throw new AdvFeaturesExpection("The case file for this additional data set could not be opened for reading.\n" + "Please confirm that the path and/or file name are valid\n" + "and that you have permissions to read from this directory\nand file.",
                        FocusedTabSet.INPUT, (Component) _caseFileTextField);
            }
            //validate the control file for this dataset - Bernoulli model only
            if (_analysisSettingsWindow.getModelControlType() == Parameters.ProbabilityModelType.BERNOULLI) {
                if (_controlFilenames.get(i).length() == 0) {
                    throw new AdvFeaturesExpection("For the Bernoulli model, please specify a control file for this additional data set.", FocusedTabSet.INPUT, (Component) _controlFileTextField);
                }
                if (!FileAccess.ValidateFileAccess(_controlFilenames.get(i), false)) {
                    throw new AdvFeaturesExpection("The control file for this additional data set could not be opened for reading.\n" + "Please confirm that the path and/or file name are valid\n" + "and that you have permissions to read from this directory\nand file.",
                            FocusedTabSet.INPUT, (Component) _controlFileTextField);
                }
            }
            //validate the population file for this dataset-  Poisson model only
            if (_analysisSettingsWindow.getModelControlType() == Parameters.ProbabilityModelType.POISSON) {
                //For purely temporal analyses, the population file is optional. But if one first
                //dataset does or does not supply a population file; the other dataset must do the same.
                if (bAnalysisIsPurelyTemporal) {
                    if ((_populationFilenames.get(i).length() == 0 && bFirstDataSetHasPopulationFile) ||
                            (_populationFilenames.get(i).length() > 0 && !bFirstDataSetHasPopulationFile)) {
                        throw new AdvFeaturesExpection("For the Poisson model with purely temporal analyses, the population file is optional but all data\n" + "sets must either specify a population file or omit it.",
                                FocusedTabSet.INPUT, (Component) _populationFileTextField);
                    } else if (_populationFilenames.get(i).length() > 0 && !FileAccess.ValidateFileAccess(_populationFilenames.get(i), false)) {
                        throw new AdvFeaturesExpection("The population file for this additional data set could not be opened for reading.\n" + "Please confirm that the path and/or file name are valid\n" + "and that you have permissions to read from this directory\nand file.",
                                FocusedTabSet.INPUT, (Component) _populationFileTextField);
                    }
                } else if (_populationFilenames.get(i).length() == 0) {
                    throw new AdvFeaturesExpection("For the Poisson model, please specify a population file for this additional data set.", FocusedTabSet.INPUT, (Component) _populationFileTextField);
                } else if (!FileAccess.ValidateFileAccess(_populationFilenames.get(i), false)) {
                    throw new AdvFeaturesExpection("The population file for this additional data set could not be opened for reading.\n" + "Please confirm that the path and/or file name are valid\n" + "and that you have permissions to read from this directory\nand file.",
                            FocusedTabSet.INPUT, (Component) _populationFileTextField);
                }
            }
        }  //for loop

        //validate that purpose for multiple data sets is not 'adjustment' if probability model is ordinal
        if ((_analysisSettingsWindow.getModelControlType() == Parameters.ProbabilityModelType.ORDINAL ||
             _analysisSettingsWindow.getModelControlType() == Parameters.ProbabilityModelType.CATEGORICAL)
            && _adjustmentByDataSetsRadioButton.isEnabled() && _adjustmentByDataSetsRadioButton.isSelected()) {
            throw new AdvFeaturesExpection("For the ordinal and categorical probability models with input data defined in multiple data sets,\n" + 
                                            "the adjustment option has not been implemented.", FocusedTabSet.INPUT, (Component) _adjustmentByDataSetsRadioButton);
        }
    }

    /** Validate user settings of the Neighbors File tab. */
    private void ValidateNeighborsFileSettings() {
        if (_specialNeighborFilesGroup.isEnabled() && _specifiyNeighborsFileCheckBox.isEnabled() && _specifiyNeighborsFileCheckBox.isSelected()) {
            //validate the case file for this dataset
            if (_neighborsFileTextField.getText().length() == 0) {
                throw new AdvFeaturesExpection("Please specify a neighbors file.", FocusedTabSet.INPUT, (Component) _neighborsFileTextField);
            }
            if (!FileAccess.ValidateFileAccess(_neighborsFileTextField.getText(), false)) {
                throw new AdvFeaturesExpection("The neighbors file could not be opened for reading.\n" + "Please confirm that the path and/or file name are valid\n" + "and that you have permissions to read from this directory\nand file.",
                        FocusedTabSet.INPUT, (Component) _neighborsFileTextField);
            }
        }
        if (_specifiyNeighborsFileCheckBox.isEnabled() && _specifiyNeighborsFileCheckBox.isSelected() && _specifiyMetaLocationsFileCheckBox.isSelected()) {
            //validate the case file for this dataset
            if (_metaLocationsFileTextField.getText().length() == 0) {
                throw new AdvFeaturesExpection("Please specify a meta locations file.", FocusedTabSet.INPUT, (Component) _metaLocationsFileTextField);
            }
            if (!FileAccess.ValidateFileAccess(_metaLocationsFileTextField.getText(), false)) {
                throw new AdvFeaturesExpection("The meta locations file could not be opened for reading.\n" + "Please confirm that the path and/or file name are valid\n" + "and that you have permissions to read from this directory\nand file.",
                        FocusedTabSet.INPUT, (Component) _metaLocationsFileTextField);
            }
        }
    }

    private void ValidateSpatialClusterSize() {
        if (_spatialOptionsGroup.isEnabled() && _spatialPopulationFileCheckBox.isSelected() && _maxCirclePopulationFilenameTextField.getText().length() == 0) {
            throw new AdvFeaturesExpection("Please specify a maximum circle size file.", FocusedTabSet.ANALYSIS, (Component) _maxCirclePopulationFilenameTextField);
        }

        if (_spatialOptionsGroup.isEnabled() && _spatialPopulationFileCheckBox.isSelected() && !FileAccess.ValidateFileAccess(_maxCirclePopulationFilenameTextField.getText(), false)) {
            throw new AdvFeaturesExpection("The maximum circle file could not be opened for reading.\n" + "Please confirm that the path and/or file name are valid\n" + "and that you have permissions to read from this directory\nand file.",
                    FocusedTabSet.ANALYSIS, (Component) _maxCirclePopulationFilenameTextField);
        }

        if (_reportedSpatialOptionsGroup.isEnabled() && _restrictReportedClustersCheckBox.isSelected() &&
                _reportedSpatialPopulationFileCheckBox.isSelected() && !FileAccess.ValidateFileAccess(_maxCirclePopulationFilenameTextField.getText(), false)) {
            throw new AdvFeaturesExpection("The maximum circle file could not be opened for reading.\n" + "Please confirm that the path and/or file name are valid\n" + "and that you have permissions to read from this directory\nand file." + "A maximum circle file is required when restricting the maximum\n" + "reported spatial cluster size by a population defined through a\nmaximum circle file.",
                    FocusedTabSet.ANALYSIS, (Component) _maxCirclePopulationFilenameTextField);
        }
        //When analysis type is prospective space-time, adjusting for earlier analyses - max spatial size must be defined
        //as percentage of max circle population or as a distance.
        if (_analysisSettingsWindow.getAnalysisControlType() == Parameters.AnalysisType.PROSPECTIVESPACETIME &&
                _adjustForEarlierAnalysesCheckBox.isEnabled() && _adjustForEarlierAnalysesCheckBox.isSelected()) {
            if (!_spatialPopulationFileCheckBox.isSelected() && _spatialPopulationFileCheckBox.isEnabled() && _specifiyNeighborsFileCheckBox.isSelected()) {
                throw new AdvFeaturesExpection("For a prospective space-time analysis adjusting for ealier analyses and defining\n" + "non-eucledian neigbors, the maximum spatial cluster size must be defined as a\n" + "percentage of the population as defined in a max circle size file.\n",
                        FocusedTabSet.ANALYSIS, (Component) _spatialPopulationFileCheckBox);
            } else if (!_spatialPopulationFileCheckBox.isSelected() && !_spatialDistanceCheckBox.isSelected()) {
                throw new AdvFeaturesExpection("For a prospective space-time analysis adjusting for ealier analyses, the maximum spatial\n" + "cluster size must be defined as a percentage of the population as defined in a max\n" + "circle size file.\n" + "Alternatively you may choose to specify the maximum as a fixed radius, in which case a\n" + "max circle size file is not required.\n",
                        FocusedTabSet.ANALYSIS, (Component) _spatialPopulationFileCheckBox);
            }
        }
    }

    private void ValidateAdjustmentSettings() {
        boolean bAnalysisIsPurelyTemporal = _analysisSettingsWindow.getAnalysisControlType() == Parameters.AnalysisType.PURELYTEMPORAL ||
                _analysisSettingsWindow.getAnalysisControlType() == Parameters.AnalysisType.PROSPECTIVEPURELYTEMPORAL;

        //validate spatial adjustments
        if (_spatialAdjustmentsGroup.isEnabled() && _spatialAdjustmentsSpatialStratifiedRadioButton.isSelected()) {
            if (_includePureSpacClustCheckBox.isEnabled() && _includePureSpacClustCheckBox.isSelected()) {
                throw new AdvFeaturesExpection("Spatial adjustments can not performed in conjunction\n" + " with the inclusion of purely spatial clusters.", FocusedTabSet.ANALYSIS, (Component) _spatialAdjustmentsGroup);
            }
            if (_temporalTrendAdjGroup.isEnabled() && getAdjustmentTimeTrendControlType() == Parameters.TimeTrendAdjustmentType.NOTADJUSTED) {
                throw new AdvFeaturesExpection("Spatial adjustments can not performed in conjunction\n" + "with the nonparametric temporal adjustment.", FocusedTabSet.ANALYSIS, (Component) _spatialAdjustmentsGroup);
            }
        }
        //validate temporal adjustments
        if (_temporalTrendAdjGroup.isEnabled() && _analysisSettingsWindow.getModelControlType() == Parameters.ProbabilityModelType.POISSON && bAnalysisIsPurelyTemporal &&
                _analysisSettingsWindow.getEdtPopFileNameText().length() == 0 && getAdjustmentTimeTrendControlType() != Parameters.TimeTrendAdjustmentType.NOTADJUSTED) {
            throw new AdvFeaturesExpection("Temporal adjustments can not be performed for a purely temporal analysis\n" + "using the Poisson model, when no population file has been specfied.", FocusedTabSet.ANALYSIS, (Component) _temporalTrendAdjGroup);
        }
        //validate spatial/temporal/space-time adjustments
        if (_adjustForKnownRelativeRisksCheckBox.isEnabled() && _adjustForKnownRelativeRisksCheckBox.isSelected()) {
            if (_adjustmentsByRelativeRisksFileTextField.getText().length() == 0) {
                throw new AdvFeaturesExpection("Please specify an adjustments file.", FocusedTabSet.ANALYSIS, (Component) _adjustmentsByRelativeRisksFileTextField);
            }
            if (!FileAccess.ValidateFileAccess(_adjustmentsByRelativeRisksFileTextField.getText(), false)) {
                throw new AdvFeaturesExpection("The adjustments file could not be opened for reading.\n" + "Please confirm that the path and/or file name are valid\n" + "and that you have permissions to read from this directory\nand file.",
                        FocusedTabSet.ANALYSIS, (Component) _adjustmentsByRelativeRisksFileTextField);
            }
        }
    }

    private void ValidateTemporalClusterSize() {
        String sErrorMessage, sPrecisionString;
        double dStudyPeriodLengthInUnits, dMaxTemporalLengthInUnits;

        //check whether we are specifiying temporal information
        if (!_temporalOptionsGroup.isEnabled()) {
            return;
        }

        if (GetMaxTemporalClusterSizeControlType() == Parameters.TemporalSizeType.PERCENTAGETYPE) {
            if (_maxTemporalClusterSizeTextField.getText().length() == 0 || Double.parseDouble(_maxTemporalClusterSizeTextField.getText()) == 0) {
                throw new AdvFeaturesExpection("Please specify a maximum temporal cluster size.", FocusedTabSet.ANALYSIS, (Component) _maxTemporalClusterSizeTextField);
            }
            //check maximum temporal cluster size(as percentage of population) is less than maximum for given probability model
            if (Double.parseDouble(_maxTemporalClusterSizeTextField.getText()) > (_analysisSettingsWindow.getModelControlType() == Parameters.ProbabilityModelType.SPACETIMEPERMUTATION ? 50 : 90)) {
                sErrorMessage = "For the " + Parameters.GetProbabilityModelTypeAsString(_analysisSettingsWindow.getModelControlType()) + " model, the maximum temporal cluster size as a percent of the study period is " + (_analysisSettingsWindow.getModelControlType() == Parameters.ProbabilityModelType.SPACETIMEPERMUTATION ? 50 : 90) + " percent.";
                throw new AdvFeaturesExpection(sErrorMessage, FocusedTabSet.ANALYSIS, (Component) _maxTemporalClusterSizeTextField);
            }
            //validate that the time aggregation length agrees with the study period and maximum temporal cluster size
            dStudyPeriodLengthInUnits = _analysisSettingsWindow.CalculateTimeAggregationUnitsInStudyPeriod();
            dMaxTemporalLengthInUnits = Math.floor(dStudyPeriodLengthInUnits * Double.parseDouble(_maxTemporalClusterSizeTextField.getText()) / 100.0);
            if (dMaxTemporalLengthInUnits < 1) {
                sPrecisionString = _analysisSettingsWindow.GetDatePrecisionAsString(_analysisSettingsWindow.getTimeAggregationControlType(), false, false);
                sErrorMessage = "A maximum temporal cluster size as " + Double.parseDouble(_maxTemporalClusterSizeTextField.getText()) + " percent of a " + Math.floor(dStudyPeriodLengthInUnits) + sPrecisionString + " study period\n" + "results in a maximum temporal cluster size that is less than one time\n" + "aggregation " + sPrecisionString + "\n";
                throw new AdvFeaturesExpection(sErrorMessage, FocusedTabSet.ANALYSIS, (Component) _maxTemporalClusterSizeTextField);
            }
        } else if (GetMaxTemporalClusterSizeControlType() == Parameters.TemporalSizeType.TIMETYPE) {
            if (_maxTemporalClusterSizeUnitsTextField.getText().length() == 0 || Integer.parseInt(_maxTemporalClusterSizeUnitsTextField.getText()) == 0) {
                throw new AdvFeaturesExpection("Please specify a maximum temporal cluster size.", FocusedTabSet.ANALYSIS, (Component) _maxTemporalClusterSizeUnitsTextField);
            }
            sPrecisionString = _analysisSettingsWindow.GetDatePrecisionAsString(_analysisSettingsWindow.getTimeAggregationControlType(), false, false);
            dStudyPeriodLengthInUnits = _analysisSettingsWindow.CalculateTimeAggregationUnitsInStudyPeriod();
            dMaxTemporalLengthInUnits = Math.floor(dStudyPeriodLengthInUnits * (_analysisSettingsWindow.getModelControlType() == Parameters.ProbabilityModelType.SPACETIMEPERMUTATION ? 50 : 90) / 100.0);
            if (Double.parseDouble(_maxTemporalClusterSizeUnitsTextField.getText()) > dMaxTemporalLengthInUnits) {
                sErrorMessage = "A maximum temporal cluster size of " + Integer.parseInt(_maxTemporalClusterSizeUnitsTextField.getText()) + sPrecisionString + (Integer.parseInt(_maxTemporalClusterSizeUnitsTextField.getText()) == 1 ? "" : "s") + " exceeds " + (_analysisSettingsWindow.getModelControlType() == Parameters.ProbabilityModelType.SPACETIMEPERMUTATION ? 50 : 90) + " percent of a " + Math.floor(dStudyPeriodLengthInUnits) + " " + sPrecisionString + " study period.\n" + "Note that current settings limit the maximum to " + Math.floor(dMaxTemporalLengthInUnits) + " " + sPrecisionString + (dMaxTemporalLengthInUnits == 1 ? "" : "s") + ".";
                throw new AdvFeaturesExpection(sErrorMessage, FocusedTabSet.ANALYSIS, (Component) _maxTemporalClusterSizeUnitsTextField);
            }
        }
    //    else
    // ZdException::GenerateNotification("Unknown temporal percentage type: %d.","ValidateTemporalClusterSize()", GetMaxTemporalClusterSizeControlType());
    }

    /** validates scanning window range settings - throws exception */
    private void ValidateScanningWindowRanges() {
        if (_restrictTemporalRangeCheckBox.isEnabled() && _restrictTemporalRangeCheckBox.isSelected()) {
            GregorianCalendar StudyPeriodStartDate = _analysisSettingsWindow.getStudyPeriodStartDateAsCalender();
            GregorianCalendar StudyPeriodEndDate = _analysisSettingsWindow.getStudyPeriodEndDateAsCalender();
            GregorianCalendar StartRangeStartDate = new GregorianCalendar();
            StartRangeStartDate.set(Calendar.YEAR, Integer.parseInt(_startRangeStartYearTextField.getText()));
            StartRangeStartDate.set(Calendar.MONTH, Integer.parseInt(_startRangeStartMonthTextField.getText()) - 1);
            StartRangeStartDate.set(Calendar.DAY_OF_MONTH, Integer.parseInt(_startRangeStartDayTextField.getText()));
            GregorianCalendar StartRangeEndDate = new GregorianCalendar();
            StartRangeEndDate.set(Calendar.YEAR, Integer.parseInt(_startRangeEndYearTextField.getText()));
            StartRangeEndDate.set(Calendar.MONTH, Integer.parseInt(_startRangeEndMonthTextField.getText()) - 1);
            StartRangeEndDate.set(Calendar.DAY_OF_MONTH, Integer.parseInt(_startRangeEndDayTextField.getText()));
            GregorianCalendar EndRangeStartDate = new GregorianCalendar();
            EndRangeStartDate.set(Calendar.YEAR, Integer.parseInt(_endRangeStartYearTextField.getText()));
            EndRangeStartDate.set(Calendar.MONTH, Integer.parseInt(_endRangeStartMonthTextField.getText()) - 1);
            EndRangeStartDate.set(Calendar.DAY_OF_MONTH, Integer.parseInt(_endRangeStartDayTextField.getText()));
            GregorianCalendar EndRangeEndDate = new GregorianCalendar();
            EndRangeEndDate.set(Calendar.YEAR, Integer.parseInt(_endRangeEndYearTextField.getText()));
            EndRangeEndDate.set(Calendar.MONTH, Integer.parseInt(_endRangeEndMonthTextField.getText()) - 1);
            EndRangeEndDate.set(Calendar.DAY_OF_MONTH, Integer.parseInt(_endRangeEndDayTextField.getText()));

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

    /** Specific prospective space-time date check
     * Must be between the start and end dates of the analysis */
    private void ValidateProspDateRange() {
        if (!(_adjustForEarlierAnalysesCheckBox.isEnabled() && _adjustForEarlierAnalysesCheckBox.isSelected())) {
            return;
        }

        if (_prospectiveStartDateYearTextField.getText().length() == 0) {
            throw new AdvFeaturesExpection("Please specify a prospective start year.",
                    FocusedTabSet.ANALYSIS, (Component) _prospectiveStartDateYearTextField);
        }
        if (_prospectiveStartDateMonthTextField.getText().length() == 0) {
            throw new AdvFeaturesExpection("Please specify a prospective start month.",
                    FocusedTabSet.ANALYSIS, (Component) _prospectiveStartDateMonthTextField);
        }
        if (_prospectiveStartDateDayTextField.getText().length() == 0) {
            throw new AdvFeaturesExpection("Please specify a prospective start day.",
                    FocusedTabSet.ANALYSIS, (Component) _prospectiveStartDateDayTextField);
        }
        GregorianCalendar StudyPeriodStartDate = _analysisSettingsWindow.getStudyPeriodStartDateAsCalender();
        GregorianCalendar StudyPeriodEndDate = _analysisSettingsWindow.getStudyPeriodEndDateAsCalender();
        GregorianCalendar ProspectiveStartDate = new GregorianCalendar();
        ProspectiveStartDate.set(Calendar.YEAR, Integer.parseInt(_prospectiveStartDateYearTextField.getText()));
        ProspectiveStartDate.set(Calendar.MONTH, Integer.parseInt(_prospectiveStartDateMonthTextField.getText()) - 1);
        ProspectiveStartDate.set(Calendar.DAY_OF_MONTH, Integer.parseInt(_prospectiveStartDateDayTextField.getText()));
        if (ProspectiveStartDate.before(StudyPeriodStartDate) || ProspectiveStartDate.after(StudyPeriodEndDate)) {
            throw new AdvFeaturesExpection("The prospective start date must be between the study period start and end dates.",
                    FocusedTabSet.ANALYSIS, (Component) _prospectiveStartDateYearTextField);
        }
    }

    /** validates temporal window settings - throws exception */
    private void ValidateTemporalWindowSettings() {
        ValidateTemporalClusterSize();
        ValidateScanningWindowRanges();
    }

    /** validates reported clusters limiting control setting - throws exception */
    private void ValidateReportedSpatialClusterSize() {
        if (!(_restrictReportedClustersCheckBox.isEnabled() && _restrictReportedClustersCheckBox.isSelected())) {
            return;
        }
        //When analysis type is prospective space-time, adjusting for earlier analyses - max spatial size must be defined
        //as percentage of max circle population or as a distance.
        if (_analysisSettingsWindow.getAnalysisControlType() == Parameters.AnalysisType.PROSPECTIVESPACETIME &&
                _adjustForEarlierAnalysesCheckBox.isEnabled() && _adjustForEarlierAnalysesCheckBox.isSelected()) {
            if (!_reportedSpatialPopulationFileCheckBox.isSelected() && _specifiyNeighborsFileCheckBox.isEnabled() && _specifiyNeighborsFileCheckBox.isSelected()) {
                throw new AdvFeaturesExpection("For a prospective space-time analysis adjusting for ealier analyses and defining\n" + "non-eculedian neighbors, the maximum reported spatial cluster size must be\n" + "defined as a percentage of the population as defined in a max circle size file.\n",
                        FocusedTabSet.OUTPUT, (Component) _reportedSpatialPopulationFileCheckBox);

            } else if (!_reportedSpatialPopulationFileCheckBox.isSelected() && !_reportedSpatialDistanceCheckBox.isSelected()) {
                throw new AdvFeaturesExpection("For a prospective space-time analysis adjusting for ealier analyses, the maximum spatial\n" + "cluster size must be defined as a percentage of the population as defined in a max\n" + "circle size file.\n" + "Alternatively you may choose to specify the maximum as a fixed radius, in which case a\n" + "max circle size file is not required.\n", FocusedTabSet.OUTPUT, (Component) _reportedSpatialPopulationFileCheckBox);
            }
        }
    }

    /** validates all the settings in this dialog */
    public void Validate() {
        ValidateInputFiles();
        ValidateNeighborsFileSettings();
        ValidateSpatialClusterSize();
        ValidateAdjustmentSettings();
        ValidateTemporalWindowSettings();
        ValidateProspDateRange();
        ValidateReportedSpatialClusterSize();
    }

    /**
     * enables input tab case/control/pop files edit boxes
     */
    private void EnableInputFileEdits(boolean bEnable) {
        bEnable &= _additionalDataSetsGroup.isEnabled();
        bEnable &= _inputDataSetsList.getModel().getSize() > 0;

        _caseFileTextField.setEnabled(bEnable);
        _controlFileTextField.setEnabled(bEnable);
        _populationFileTextField.setEnabled(bEnable);
        _caseFileBrowseButton.setEnabled(bEnable);
        _caseFileImportButton.setEnabled(bEnable);
        _controlFileBrowseButton.setEnabled(bEnable);
        _controlFileImportButton.setEnabled(bEnable);
        _populationFileBrowseButton.setEnabled(bEnable);
        _populationFileImportButton.setEnabled(bEnable);
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
    private void EnableDataSetPurposeControls(boolean bEnable) {
        _additionalDataSetsGroup.setEnabled(bEnable);
        _multivariateAdjustmentsRadioButton.setEnabled(bEnable);
        _adjustmentByDataSetsRadioButton.setEnabled(bEnable);
        _multipleDataSetPurposeLabel.setEnabled(bEnable);
    }

    /**
     * Sets default values for Input related tab and respective controls
     */
    private void SetDefaultsForInputTab() {
        // clear all visual components
        _caseFileTextField.setText("");
        _controlFileTextField.setText("");
        _populationFileTextField.setText("");
        _dataSetsListModel.removeAllElements();
        EnableDataSetList(_inputDataSetsList.getModel().getSize() > 0);
        EnableDataSetPurposeControls(_inputDataSetsList.getModel().getSize() > 0);

        // clear the non-visual components
        _caseFilenames.clear();
        _controlFilenames.clear();
        _populationFilenames.clear();
        EnableNewButton();
        EnableRemoveButton();
        EnableInputFileEdits(false);
        _multivariateAdjustmentsRadioButton.setSelected(true);

        //data checking
        _strictStudyPeriodCheckRadioButton.setSelected(true);
        _strictCoordinatesRadioButton.setSelected(true);

        //neighbors file
        _specifiyNeighborsFileCheckBox.setSelected(false);
        _neighborsFileTextField.setText("");
        _specifiyMetaLocationsFileCheckBox.setSelected(false);
        _metaLocationsFileTextField.setText("");
    }

    /** Sets maximum temporal cluster size control for passed type */
    private void SetMaxTemporalClusterSizeTypeControl(Parameters.TemporalSizeType eTemporalSizeType) {
        switch (eTemporalSizeType) {
            case TIMETYPE:
                _timeTemporalRadioButton.setSelected(true);
                break;
            case PERCENTAGETYPE:
            default:
                _percentageTemporalRadioButton.setSelected(true);
        }
    }

    /** Sets default values for Analysis related tabs and their respective controls
     * pulled these default values from the CParameter class
     */
    private void SetDefaultsForAnalysisTabs() {
        // Inference tab
        _adjustForEarlierAnalysesCheckBox.setSelected(false);
        _terminateEarlyCheckBox.setSelected(false);
        _prospectiveStartDateYearTextField.setText("2000");
        _prospectiveStartDateMonthTextField.setText("12");
        _prospectiveStartDateDayTextField.setText("31");
        _reportCriticalValuesCheckBox.setSelected(false);
        _performIterativeScanCheckBox.setSelected(false);
        _numIterativeScansTextField.setText("10");
        _iterativeScanCutoffTextField.setText(".05");

        // Spatial Window tab
        _maxSpatialClusterSizeTextField.setText("50");
        _maxSpatialPercentFileTextField.setText("50");
        _maxSpatialRadiusTextField.setText("1");
        _spatialPopulationFileCheckBox.setSelected(false);
        _spatialDistanceCheckBox.setSelected(false);
        _maxCirclePopulationFilenameTextField.setText("");
        _inclPureTempClustCheckBox.setSelected(false);
        _circularRadioButton.setSelected(true);
        _nonCompactnessPenaltyComboBox.setSelectedIndex(1);
        SetSpatialDistanceCaption();


        _onePerLocationIdRadioButton.setSelected(true);

        // Temporal tab
        SetMaxTemporalClusterSizeTypeControl(Parameters.TemporalSizeType.PERCENTAGETYPE);
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
        SetTemporalTrendAdjustmentControl(Parameters.TimeTrendAdjustmentType.NOTADJUSTED);
        _spatialAdjustmentsNoneRadioButton.setSelected(true);
        _logLinearTextField.setText("0");
    }

    /** Sets default values for Output related tab and respective controls
     *   pulled these default values from the CParameter class
     */
    private void SetDefaultsForOutputTab() {
        _criteriaSecClustersNoGeoLapRadioButton.setSelected(true);
        _restrictReportedClustersCheckBox.setSelected(false);
        _reportedSpatialPopulationFileCheckBox.setSelected(false);
        _reportedSpatialDistanceCheckBox.setSelected(false);
        _maxReportedSpatialClusterSizeTextField.setText("50");
        _maxReportedSpatialPercentFileTextField.setText("50");
        _maxReportedSpatialRadiusTextField.setText("1");
    }

    /**
     * Resets advanced settings to default values
     */
    private void SetDefaultsClick() {
        switch (_focusedTabSet) {
            case INPUT:
                SetDefaultsForInputTab();
                break;
            case ANALYSIS:
                SetDefaultsForAnalysisTabs();
                break;
            case OUTPUT:
                SetDefaultsForOutputTab();
                break;
        }
        enableSetDefaultsButton();
    }

    /**
     * enables or disables the spatial options group control
     */
    private void enableSpatialOptionsGroup(boolean bEnable, boolean bEnableIncludePurelyTemporal) {
        _spatialOptionsGroup.setEnabled(bEnable);
        boolean bEnablePopPercentage = !(_analysisSettingsWindow.getAnalysisControlType() == Parameters.AnalysisType.PROSPECTIVESPACETIME &&
                _adjustForEarlierAnalysesCheckBox.isEnabled() && _adjustForEarlierAnalysesCheckBox.isSelected());
        _maxSpatialClusterSizeTextField.setEnabled(bEnable && bEnablePopPercentage);
        _percentageOfPopulationLabel.setEnabled(bEnable && bEnablePopPercentage);
        
        boolean bEnablePopulationFile = bEnable && _analysisSettingsWindow.getModelControlType() != Parameters.ProbabilityModelType.HOMOGENEOUSPOISSON;
        _spatialPopulationFileCheckBox.setEnabled(bEnablePopulationFile);
        _maxSpatialPercentFileTextField.setEnabled(bEnablePopulationFile && _spatialPopulationFileCheckBox.isSelected());
        _percentageOfPopFileLabel.setEnabled(bEnablePopulationFile);
        _maxCirclePopulationFilenameTextField.setEnabled(bEnablePopulationFile);
        _maxCirclePopFileBrowseButton.setEnabled(bEnablePopulationFile);
        _maxCirclePopFileImportButton.setEnabled(bEnablePopulationFile);

        _spatialDistanceCheckBox.setEnabled(bEnable && !_specifiyNeighborsFileCheckBox.isSelected());
        _maxSpatialRadiusTextField.setEnabled(_spatialDistanceCheckBox.isEnabled() && _spatialDistanceCheckBox.isSelected());
        _distancePrefixLabel.setEnabled(_spatialDistanceCheckBox.isEnabled());
        _maxRadiusLabel.setEnabled(_spatialDistanceCheckBox.isEnabled());

        _inclPureTempClustCheckBox.setEnabled(bEnable && bEnableIncludePurelyTemporal);
        enableReportedSpatialOptionsGroup(bEnable);
    }

    /**
     * enables or disables the window shape options group control
     */
    private void EnableWindowShapeGroup(boolean bEnable) {
        bEnable &= !_specifiyNeighborsFileCheckBox.isSelected();
        _spatialWindowShapeGroup.setEnabled(bEnable);
        _circularRadioButton.setEnabled(bEnable);
        _ellipticRadioButton.setEnabled(bEnable && 
                                        _analysisSettingsWindow.getCoordinatesType() == Parameters.CoordinatesType.CARTESIAN &&
                                        _analysisSettingsWindow.getModelControlType() != Parameters.ProbabilityModelType.HOMOGENEOUSPOISSON);
        _nonCompactnessPenaltyLabel.setEnabled(_ellipticRadioButton.isEnabled() && _ellipticRadioButton.isSelected());
        _nonCompactnessPenaltyComboBox.setEnabled(_ellipticRadioButton.isEnabled() && _ellipticRadioButton.isSelected());
        if (!_ellipticRadioButton.isEnabled() && _ellipticRadioButton.isSelected() && _circularRadioButton.isEnabled()) {
            _circularRadioButton.setSelected(true);
        }
        SetSpatialDistanceCaption();
    }

    /** enables or disables the spatial options group control */
    private void enableReportedSpatialOptionsGroup(boolean bEnable) {
        _reportedSpatialOptionsGroup.setEnabled(bEnable);
        _restrictReportedClustersCheckBox.setEnabled(bEnable);

        boolean bEnablePopPercentage = !(_analysisSettingsWindow.getAnalysisControlType() == Parameters.AnalysisType.PROSPECTIVESPACETIME &&
                _adjustForEarlierAnalysesCheckBox.isEnabled() && _adjustForEarlierAnalysesCheckBox.isSelected());
        _maxReportedSpatialClusterSizeTextField.setEnabled(bEnable && _restrictReportedClustersCheckBox.isSelected() && bEnablePopPercentage);
        _reportedPercentOfPopulationLabel.setEnabled(bEnable && _restrictReportedClustersCheckBox.isSelected() && bEnablePopPercentage);
        boolean bEnablePopulationFile = bEnable && _analysisSettingsWindow.getModelControlType() != Parameters.ProbabilityModelType.HOMOGENEOUSPOISSON;
        _reportedSpatialPopulationFileCheckBox.setEnabled(bEnablePopulationFile && _restrictReportedClustersCheckBox.isSelected());
        _maxReportedSpatialPercentFileTextField.setEnabled(bEnablePopulationFile && _restrictReportedClustersCheckBox.isSelected() && _reportedSpatialPopulationFileCheckBox.isSelected());
        _reportedPercentageOfPopFileLabel.setEnabled(bEnablePopulationFile && _restrictReportedClustersCheckBox.isSelected());

        _reportedSpatialDistanceCheckBox.setEnabled(bEnable && !_specifiyNeighborsFileCheckBox.isSelected() && _restrictReportedClustersCheckBox.isSelected());
        _reportedMaxDistanceLabel.setEnabled(_reportedSpatialDistanceCheckBox.isEnabled() && _restrictReportedClustersCheckBox.isSelected());
        _maxReportedSpatialRadiusTextField.setEnabled(_reportedSpatialDistanceCheckBox.isEnabled() && _restrictReportedClustersCheckBox.isSelected() && _reportedSpatialDistanceCheckBox.isSelected());
        _maxReportedRadiusLabel.setEnabled(_reportedSpatialDistanceCheckBox.isEnabled() && _restrictReportedClustersCheckBox.isSelected());
    }

    /** returns adjustment for time trend type for control index */
    private Parameters.TimeTrendAdjustmentType getAdjustmentTimeTrendControlType() {
        Parameters.TimeTrendAdjustmentType eReturn = null;

        if (_temporalTrendAdjNoneRadioButton.isSelected()) {
            eReturn = Parameters.TimeTrendAdjustmentType.NOTADJUSTED;
        }
        if (_temporalTrendAdjTimeStratifiedRadioButton.isSelected()) {
            eReturn = Parameters.TimeTrendAdjustmentType.STRATIFIED_RANDOMIZATION;
        }
        if (_temporalTrendAdjLogLinearRadioButton.isSelected()) {
            eReturn = Parameters.TimeTrendAdjustmentType.LOGLINEAR_PERC;
        }
        if (_temporalTrendAdjLogLinearCalcRadioButton.isSelected()) {
            eReturn = Parameters.TimeTrendAdjustmentType.CALCULATED_LOGLINEAR_PERC;
        }
        return eReturn;
    }

    /** Sets time trend adjustment control's index */
    private void SetTemporalTrendAdjustmentControl(Parameters.TimeTrendAdjustmentType eTimeTrendAdjustmentType) {
        switch (eTimeTrendAdjustmentType) {
            case NOTADJUSTED:
                _temporalTrendAdjNoneRadioButton.setSelected(true);
                break;
            case NONPARAMETRIC:
                _temporalTrendAdjTimeStratifiedRadioButton.setSelected(true);
                break;
            case LOGLINEAR_PERC:
                _temporalTrendAdjLogLinearRadioButton.setSelected(true);
                break;
            case CALCULATED_LOGLINEAR_PERC:
                _temporalTrendAdjLogLinearCalcRadioButton.setSelected(true);
                break;
            case STRATIFIED_RANDOMIZATION:
                _temporalTrendAdjTimeStratifiedRadioButton.setSelected(true);
                break;
        }
    }

    /** enables or disables the temporal time trend adjustment control group */
    private void EnableAdjustmentForTimeTrendOptionsGroup(boolean bEnable, boolean bTimeStratified, boolean bLogYearPercentage, boolean bCalculatedLog) {
        Parameters.TimeTrendAdjustmentType eTimeTrendAdjustmentType = getAdjustmentTimeTrendControlType();

        // trump control enables
        bTimeStratified &= bEnable;
        bLogYearPercentage &= bEnable;
        bCalculatedLog &= bEnable;

        _temporalTrendAdjGroup.setEnabled(bEnable);
        _temporalTrendAdjTimeStratifiedRadioButton.setEnabled(bTimeStratified);
        if (bEnable && !bTimeStratified && eTimeTrendAdjustmentType == Parameters.TimeTrendAdjustmentType.STRATIFIED_RANDOMIZATION) {
            SetTemporalTrendAdjustmentControl(Parameters.TimeTrendAdjustmentType.NOTADJUSTED);
        }

        _temporalTrendAdjLogLinearRadioButton.setEnabled(bLogYearPercentage);
        _logLinearLabel.setEnabled(bLogYearPercentage);
        _logLinearTextField.setEnabled(bLogYearPercentage && eTimeTrendAdjustmentType == Parameters.TimeTrendAdjustmentType.LOGLINEAR_PERC);
        if (bEnable && !bLogYearPercentage && eTimeTrendAdjustmentType == Parameters.TimeTrendAdjustmentType.LOGLINEAR_PERC) {
            SetTemporalTrendAdjustmentControl(Parameters.TimeTrendAdjustmentType.NOTADJUSTED);
        }

        _temporalTrendAdjLogLinearCalcRadioButton.setEnabled(bCalculatedLog);
        if (bEnable && !bCalculatedLog && eTimeTrendAdjustmentType == Parameters.TimeTrendAdjustmentType.CALCULATED_LOGLINEAR_PERC) {
            SetTemporalTrendAdjustmentControl(Parameters.TimeTrendAdjustmentType.NOTADJUSTED);
        }
    }

    private void EnableAdjustmentForSpatialOptionsGroup(boolean bEnable, boolean bEnableStratified) {
        _spatialAdjustmentsGroup.setEnabled(bEnable);
        bEnableStratified &= bEnable;
        _spatialAdjustmentsSpatialStratifiedRadioButton.setEnabled(bEnableStratified);
        if (bEnable && !bEnableStratified && _spatialAdjustmentsSpatialStratifiedRadioButton.isSelected()) {
            _spatialAdjustmentsNoneRadioButton.setSelected(true);
        }
    }

    /** enables temporal options controls */
    private void EnableTemporalRanges(boolean bEnable, boolean bEnableRanges) {
        _flexibleTemporalWindowDefinitionGroup.setEnabled(bEnable && bEnableRanges);
        _restrictTemporalRangeCheckBox.setEnabled(bEnable && bEnableRanges);
        _startWindowRangeLabel.setEnabled(bEnable && bEnableRanges && _restrictTemporalRangeCheckBox.isSelected());
        _startRangeToLabel.setEnabled(bEnable && bEnableRanges && _restrictTemporalRangeCheckBox.isSelected());
        _endWindowRangeLabel.setEnabled(bEnable && bEnableRanges && _restrictTemporalRangeCheckBox.isSelected());
        _endRangeToLabel.setEnabled(bEnable && bEnableRanges && _restrictTemporalRangeCheckBox.isSelected());
    }

    /** enables or disables the temporal options group control */
    private void EnableTemporalOptionsGroup(boolean bEnable, boolean bEnableIncludePurelySpatial, boolean bEnableRanges) {
        _temporalOptionsGroup.setEnabled(bEnable);
        _percentageTemporalRadioButton.setEnabled(bEnable);
        _maxTemporalClusterSizeTextField.setEnabled(bEnable && _percentageTemporalRadioButton.isSelected());
        _percentageOfStudyPeriodLabel.setEnabled(bEnable);
        _timeTemporalRadioButton.setEnabled(bEnable);
        _maxTemporalClusterSizeUnitsTextField.setEnabled(bEnable && _timeTemporalRadioButton.isSelected());
        _maxTemporalTimeUnitsLabel.setEnabled(bEnable);
        _includePureSpacClustCheckBox.setEnabled(bEnable && bEnableIncludePurelySpatial);
        EnableTemporalRanges(bEnable, bEnableRanges);
    }

    /** Enables isotonic scan feature controls. */
    private void EnableIsotonicScan(boolean bEnable) {
        _performIsotonicScanCheckBox.setEnabled(bEnable);
    }

    /** enabled prospective start date controls */
    private void EnableProspectiveStartDate() {
        boolean bYears = true, bMonths = true, bDays = true,
                bEnable = _prospectiveSurveillanceGroup.isEnabled() &&
                _adjustForEarlierAnalysesCheckBox.isEnabled() && _adjustForEarlierAnalysesCheckBox.isSelected();

        switch (_analysisSettingsWindow.getPrecisionOfTimesControlType()) {
            case NONE:
            case DAY:
                bYears = bMonths = bDays = bEnable;
                break;
            case YEAR:
                bYears = bEnable;
                bMonths = bDays = false;
                break;
            case MONTH:
                bYears = bMonths = bEnable;
                bDays = false;
                break;
        }

        _prospectiveStartYearLabel.setEnabled(bYears);
        _prospectiveStartDateYearTextField.setEnabled(bYears);
        _prospectiveStartMonthLabel.setEnabled(bMonths);
        _prospectiveStartDateMonthTextField.setEnabled(bMonths);
        if (!bMonths && bEnable) {
            _prospectiveStartDateMonthTextField.setText(_analysisSettingsWindow.getEdtStudyPeriodEndDateMonthText());
        }
        _prospectiveStartDayLabel.setEnabled(bDays);
        _prospectiveStartDateDayTextField.setEnabled(bDays);
        if (!bDays && bEnable) {
            GregorianCalendar thisCalender = new GregorianCalendar();
            thisCalender.set(Calendar.YEAR, Integer.parseInt(_prospectiveStartDateYearTextField.getText()));
            thisCalender.set(Calendar.MONTH, Integer.parseInt(_prospectiveStartDateMonthTextField.getText()) - 1);
            _prospectiveStartDateDayTextField.setText(Integer.toString(thisCalender.getActualMaximum(Calendar.DAY_OF_MONTH)));
        }
    }

    /** enables or disables the prospective start date group control */
    private void EnableProspectiveSurveillanceGroup(boolean bEnable) {
        _prospectiveSurveillanceGroup.setEnabled(bEnable);
        _adjustForEarlierAnalysesCheckBox.setEnabled(bEnable);
        EnableProspectiveStartDate();
    }

    /** enables adjustment options controls */
    private void EnableOutputOptions(boolean bEnable) {
        bEnable &= !_specifiyNeighborsFileCheckBox.isSelected();
        _criteriaSecClustersGroup.setEnabled(bEnable);
        _criteriaSecClustersNoGeoLapRadioButton.setEnabled(bEnable);
        _criteriaSecClustersNoCentersRadioButton.setEnabled(bEnable);
        _criteriaSecClustersNoCentersMoreRadioButton.setEnabled(bEnable);
        _criteriaSecClustersNoCentersLessRadioButton.setEnabled(bEnable);
        _criteriaSecClustersNoCentersEachRadioButton.setEnabled(bEnable);
        _criteriaSecClustersNoRestrictionsRadioButton.setEnabled(bEnable);
    }

    /** Set appropriate control for maximum spatial cluster size type. */
    private void SetMaxSpatialClusterSizeControl(Parameters.SpatialSizeType eSpatialSizeType, double dMaxSize) {
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

    /** Set appropriate control for maximum spatial cluster size type. */
    private void SetMaxTemporalClusterSizeControl(double dMaxSize) {
        Double d = new Double(dMaxSize);

        switch (GetMaxTemporalClusterSizeControlType()) {
            case TIMETYPE:
                _maxTemporalClusterSizeUnitsTextField.setText(dMaxSize <= 0 ? "1" : Integer.toString(d.intValue()));
                break;
            case PERCENTAGETYPE:
            default:
                _maxTemporalClusterSizeTextField.setText(dMaxSize <= 0 || dMaxSize > 50 ? "50" : Double.toString(dMaxSize));
                break;
        }
    }

    /** Sets spatial adjustment control type */
    private void SetSpatialAdjustmentTypeControl(Parameters.SpatialAdjustmentType eSpatialAdjustmentType) {
        switch (eSpatialAdjustmentType) {
            case NO_SPATIAL_ADJUSTMENT:
                _spatialAdjustmentsNoneRadioButton.setSelected(true);
                break;
            case SPATIALLY_STRATIFIED_RANDOMIZATION:
            default:
                _spatialAdjustmentsSpatialStratifiedRadioButton.setSelected(true);
        }
    }

    /** Sets secondary cluster control type */
    private void SetSecondaryClustersTypeControl(Parameters.CriteriaSecondaryClustersType eCriteriaSecondaryClustersType) {
        switch (eCriteriaSecondaryClustersType) {
            case NOGEOOVERLAP:
                _criteriaSecClustersNoGeoLapRadioButton.setSelected(true);
                break;
            case NOCENTROIDSINOTHER:
                _criteriaSecClustersNoCentersRadioButton.setSelected(true);
                break;
            case NOCENTROIDSINMORELIKE:
                _criteriaSecClustersNoCentersMoreRadioButton.setSelected(true);
                break;
            case NOCENTROIDSINLESSLIKE:
                _criteriaSecClustersNoCentersLessRadioButton.setSelected(true);
                break;
            case NOPAIRSINEACHOTHERS:
                _criteriaSecClustersNoCentersEachRadioButton.setSelected(true);
                break;
            case NORESTRICTIONS:
                _criteriaSecClustersNoRestrictionsRadioButton.setSelected(true);
                break;
        }
    }

    /** Sets study period data checking control's index */
    private void SetStudyPeriodDataCheckingControl(Parameters.StudyPeriodDataCheckingType eStudyPeriodDataCheckingType) {
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

    private void SetCoordinatesDataCheckingControl(Parameters.CoordinatesDataCheckingType eCoordinatesDataCheckingType) {
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

    /** parses up a date string and places it into the given month, day, year
     * interace text control (TEdit *). Defaults prospective survallience start
     * date to months/days to like study period end date.                       */
    private void ParseProspectiveDate(String sDateString, JTextField Year, JTextField Month, JTextField Day) {
        String[] dateParts = sDateString.split("/");
        GregorianCalendar thisCalender = new GregorianCalendar();
        int iStudyPeriodDay = 0;

        try {
            int iYear = 0, iMonth = 0, iDay = 0;
            //set values only if valid, prevent interface from having invalid date when first loaded.
            if (dateParts.length > 0) {
                switch (dateParts.length) {
                    case 1:
                        iYear = Integer.parseInt(dateParts[0]);
                        if (iYear >= AppConstants.MIN_YEAR && iYear <= AppConstants.MAX_YEAR) {
                            Year.setText(dateParts[0]);
                        }
                        thisCalender.set(Calendar.YEAR, iYear);
                        thisCalender.set(Calendar.MONTH, Integer.parseInt(_analysisSettingsWindow.getEdtStudyPeriodEndDateMonthText()) - 1);
                        Month.setText(_analysisSettingsWindow.getEdtStudyPeriodEndDateMonthText());
                        iStudyPeriodDay = Integer.parseInt(_analysisSettingsWindow.getEdtStudyPeriodEndDateDayText());
                        if (iStudyPeriodDay < thisCalender.getActualMaximum(Calendar.DAY_OF_MONTH)) {
                            Day.setText(_analysisSettingsWindow.getEdtStudyPeriodEndDateDayText());
                        } else {
                            Day.setText(Integer.toString(thisCalender.getActualMaximum(Calendar.DAY_OF_MONTH)));
                        }
                        break;
                    case 2:
                        iYear = Integer.parseInt(dateParts[0]);
                        iMonth = Integer.parseInt(dateParts[1]);
                        if (iYear >= AppConstants.MIN_YEAR && iYear <= AppConstants.MAX_YEAR && iMonth >= 1 && iMonth <= 12) {
                            Year.setText(dateParts[0]);
                            Month.setText(dateParts[1]);
                            thisCalender.set(Calendar.YEAR, iYear);
                            thisCalender.set(Calendar.MONTH, iMonth - 1);
                            iStudyPeriodDay = Integer.parseInt(_analysisSettingsWindow.getEdtStudyPeriodEndDateDayText());
                            if (iStudyPeriodDay < thisCalender.getActualMaximum(Calendar.DAY_OF_MONTH)) {
                                Day.setText(_analysisSettingsWindow.getEdtStudyPeriodEndDateDayText());
                            } else {
                                Day.setText(Integer.toString(thisCalender.getActualMaximum(Calendar.DAY_OF_MONTH)));
                            }
                        }
                        break;
                    case 3:
                    default:
                        iYear = Integer.parseInt(dateParts[0]);
                        iMonth = Integer.parseInt(dateParts[1]);
                        iDay = Integer.parseInt(dateParts[2]);
                        thisCalender.set(Calendar.YEAR, iYear);
                        thisCalender.set(Calendar.MONTH, iMonth - 1);
                        if (iYear >= AppConstants.MIN_YEAR && iYear <= AppConstants.MAX_YEAR && iMonth >= 1 && iMonth <= 12 &&
                                iDay >= 1 && iDay <= thisCalender.getActualMaximum(Calendar.DAY_OF_MONTH)) {
                            Year.setText(dateParts[0]);
                            Month.setText(dateParts[1]);
                            Day.setText(dateParts[2]);
                        }
                        break;
                }
            }
        } catch (NumberFormatException e) {
            return; //leave current values if parse error occurs
        }
    }

    /** Set appropriate control for maximum spatial cluster size type. */
    private void SetMaxReportedSpatialClusterSizeControl(Parameters.SpatialSizeType eSpatialSizeType, double dMaxSize) {
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

    private void SetupInterface(final Parameters parameters) {
        // Spatial Window tab
        SetMaxSpatialClusterSizeControl(Parameters.SpatialSizeType.PERCENTOFPOPULATION, parameters.GetMaxSpatialSizeForType(Parameters.SpatialSizeType.PERCENTOFPOPULATION.ordinal(), false));
        SetMaxSpatialClusterSizeControl(Parameters.SpatialSizeType.PERCENTOFMAXCIRCLEFILE, parameters.GetMaxSpatialSizeForType(Parameters.SpatialSizeType.PERCENTOFMAXCIRCLEFILE.ordinal(), false));
        SetMaxSpatialClusterSizeControl(Parameters.SpatialSizeType.MAXDISTANCE, parameters.GetMaxSpatialSizeForType(Parameters.SpatialSizeType.MAXDISTANCE.ordinal(), false));
        _spatialPopulationFileCheckBox.setSelected(parameters.GetRestrictMaxSpatialSizeForType(Parameters.SpatialSizeType.PERCENTOFMAXCIRCLEFILE.ordinal(), false));
        _spatialDistanceCheckBox.setSelected(parameters.GetRestrictMaxSpatialSizeForType(Parameters.SpatialSizeType.MAXDISTANCE.ordinal(), false));
        
        _maxCirclePopulationFilenameTextField.setText(parameters.GetMaxCirclePopulationFileName());
        _inclPureTempClustCheckBox.setSelected(parameters.GetIncludePurelyTemporalClusters());
        
        _circularRadioButton.setSelected(parameters.GetSpatialWindowType() == Parameters.SpatialWindowType.CIRCULAR);
        _ellipticRadioButton.setSelected(parameters.GetSpatialWindowType() == Parameters.SpatialWindowType.ELLIPTIC);
        _nonCompactnessPenaltyComboBox.setSelectedIndex(parameters.GetNonCompactnessPenaltyType().ordinal());
        SetSpatialDistanceCaption();
        _performIsotonicScanCheckBox.setSelected(parameters.GetRiskType() == Parameters.RiskType.MONOTONERISK);

        // Multiple Coordinates Tab
       setMultipleCoordinatesType(parameters.GetMultipleCoordinatesType());
 
        // Temporal tab
        SetMaxTemporalClusterSizeTypeControl(parameters.GetMaximumTemporalClusterSizeType());
        SetMaxTemporalClusterSizeControl(parameters.GetMaximumTemporalClusterSize());
        _includePureSpacClustCheckBox.setSelected(parameters.GetIncludePurelySpatialClusters());
        _restrictTemporalRangeCheckBox.setSelected(parameters.GetIncludeClustersType() == Parameters.IncludeClustersType.CLUSTERSINRANGE);
        Utils.parseDateStringToControls(parameters.GetStartRangeStartDate(), _startRangeStartYearTextField, _startRangeStartMonthTextField, _startRangeStartDayTextField, false);
        Utils.parseDateStringToControls(parameters.GetStartRangeEndDate(), _startRangeEndYearTextField, _startRangeEndMonthTextField, _startRangeEndDayTextField, false);
        Utils.parseDateStringToControls(parameters.GetEndRangeStartDate(), _endRangeStartYearTextField, _endRangeStartMonthTextField, _endRangeStartDayTextField, true);
        Utils.parseDateStringToControls(parameters.GetEndRangeEndDate(), _endRangeEndYearTextField, _endRangeEndMonthTextField, _endRangeEndDayTextField, true);

        // Risk tab
        SetSpatialAdjustmentTypeControl(parameters.GetSpatialAdjustmentType());
        _adjustForKnownRelativeRisksCheckBox.setSelected(parameters.UseAdjustmentForRelativeRisksFile());
        _adjustmentsByRelativeRisksFileTextField.setText(parameters.GetAdjustmentsByRelativeRisksFilename());
        SetTemporalTrendAdjustmentControl(parameters.GetTimeTrendAdjustmentType());
        _logLinearTextField.setText(parameters.GetTimeTrendAdjustmentPercentage() <= -100 ? "0" : Double.toString(parameters.GetTimeTrendAdjustmentPercentage()));

        // Inference tab
        _terminateEarlyCheckBox.setSelected(parameters.GetTerminateSimulationsEarly());
        _reportCriticalValuesCheckBox.setSelected(parameters.GetReportCriticalValues());
        _adjustForEarlierAnalysesCheckBox.setSelected(parameters.GetAdjustForEarlierAnalyses());
        ParseProspectiveDate(parameters.GetProspectiveStartDate(), _prospectiveStartDateYearTextField, _prospectiveStartDateMonthTextField, _prospectiveStartDateDayTextField);
        _performIterativeScanCheckBox.setSelected(parameters.GetIsIterativeScanning());
        _numIterativeScansTextField.setText(parameters.GetNumIterativeScansRequested() < 1 || parameters.GetNumIterativeScansRequested() > Parameters.MAXIMUM_ITERATIVE_ANALYSES ? "10" : Integer.toString(parameters.GetNumIterativeScansRequested()));
        _iterativeScanCutoffTextField.setText(parameters.GetIterativeCutOffPValue() <= 0 || parameters.GetIterativeCutOffPValue() > 1 ? "0.05" : Double.toString(parameters.GetIterativeCutOffPValue()));

        // Output tab
        _restrictReportedClustersCheckBox.setSelected(parameters.GetRestrictingMaximumReportedGeoClusterSize());
        SetSecondaryClustersTypeControl(parameters.GetCriteriaSecondClustersType());
        SetMaxReportedSpatialClusterSizeControl(Parameters.SpatialSizeType.PERCENTOFPOPULATION, parameters.GetMaxSpatialSizeForType(Parameters.SpatialSizeType.PERCENTOFPOPULATION.ordinal(), true));
        SetMaxReportedSpatialClusterSizeControl(Parameters.SpatialSizeType.PERCENTOFMAXCIRCLEFILE, parameters.GetMaxSpatialSizeForType(Parameters.SpatialSizeType.PERCENTOFMAXCIRCLEFILE.ordinal(), true));
        SetMaxReportedSpatialClusterSizeControl(Parameters.SpatialSizeType.MAXDISTANCE, parameters.GetMaxSpatialSizeForType(Parameters.SpatialSizeType.MAXDISTANCE.ordinal(), true));
        _reportedSpatialPopulationFileCheckBox.setSelected(parameters.GetRestrictMaxSpatialSizeForType(Parameters.SpatialSizeType.PERCENTOFMAXCIRCLEFILE.ordinal(), true));
        _reportedSpatialDistanceCheckBox.setSelected(parameters.GetRestrictMaxSpatialSizeForType(Parameters.SpatialSizeType.MAXDISTANCE.ordinal(), true));

        // Input tab
        EnableAdditionalDataSetsGroup(false);
        EnableInputFileEdits(false);
        for (int i = 1; i < parameters.GetNumDataSets(); i++) { // multiple data sets
            _dataSetsListModel.addElement("Data Set " + Integer.toString(i + 1));
            _caseFilenames.addElement(parameters.GetCaseFileName(i + 1));
            _controlFilenames.addElement(parameters.GetControlFileName(i + 1));
            _populationFilenames.addElement(parameters.GetPopulationFileName(i + 1));
        }
        if (_dataSetsListModel.size() > 0) {
            _inputDataSetsList.setSelectedIndex(0);
        }
        _multivariateAdjustmentsRadioButton.setSelected(parameters.GetMultipleDataSetPurposeType() == Parameters.MultipleDataSetPurposeType.MULTIVARIATE);
        _adjustmentByDataSetsRadioButton.setSelected(parameters.GetMultipleDataSetPurposeType() == Parameters.MultipleDataSetPurposeType.ADJUSTMENT);

        // Data Checking Tab
        SetStudyPeriodDataCheckingControl(parameters.GetStudyPeriodDataCheckingType());
        SetCoordinatesDataCheckingControl(parameters.GetCoordinatesDataCheckingType());
        
        // Neighbors Tab
        _specifiyNeighborsFileCheckBox.setSelected(parameters.UseLocationNeighborsFile());
        _neighborsFileTextField.setText(parameters.GetLocationNeighborsFileName());
        _specifiyMetaLocationsFileCheckBox.setSelected(parameters.UseMetaLocationsFile());
        _metaLocationsFileTextField.setText(parameters.GetMetaLocationsFileName());        
    }

    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
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
        _criteriaSecClustersButtonGroup = new javax.swing.ButtonGroup();
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
        _caseFileImportButton = new javax.swing.JButton();
        _controlFileLabel = new javax.swing.JLabel();
        _controlFileTextField = new javax.swing.JTextField();
        _controlFileBrowseButton = new javax.swing.JButton();
        _controlFileImportButton = new javax.swing.JButton();
        _populationFileLabel = new javax.swing.JLabel();
        _populationFileTextField = new javax.swing.JTextField();
        _populationFileBrowseButton = new javax.swing.JButton();
        _populationFileImportButton = new javax.swing.JButton();
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
        _relaxedCoordinatesLabel = new javax.swing.JLabel();
        _nonEucledianNeighborsTab = new javax.swing.JPanel();
        _specialNeighborFilesGroup = new javax.swing.JPanel();
        _specifiyNeighborsFileCheckBox = new javax.swing.JCheckBox();
        _neighborsFileTextField = new javax.swing.JTextField();
        _neighborsFileBrowseButton = new javax.swing.JButton();
        _specifiyMetaLocationsFileCheckBox = new javax.swing.JCheckBox();
        _metaLocationsFileTextField = new javax.swing.JTextField();
        _metaLocationsFileBrowseButton = new javax.swing.JButton();
        _multipleCoordinatesTab = new javax.swing.JPanel();
        _multipleSetsSpatialCoordinatesGroup = new javax.swing.JPanel();
        _onePerLocationIdRadioButton = new javax.swing.JRadioButton();
        _atLeastOneRadioButton = new javax.swing.JRadioButton();
        _allLocationsRadioButton = new javax.swing.JRadioButton();
        _spatialWindowTab = new javax.swing.JPanel();
        _spatialOptionsGroup = new javax.swing.JPanel();
        _maxSpatialClusterSizeTextField = new javax.swing.JTextField();
        _percentageOfPopulationLabel = new javax.swing.JLabel();
        _spatialPopulationFileCheckBox = new javax.swing.JCheckBox();
        _maxSpatialPercentFileTextField = new javax.swing.JTextField();
        _percentageOfPopFileLabel = new javax.swing.JLabel();
        _maxCirclePopulationFilenameTextField = new javax.swing.JTextField();
        _maxCirclePopFileBrowseButton = new javax.swing.JButton();
        _maxCirclePopFileImportButton = new javax.swing.JButton();
        _spatialDistanceCheckBox = new javax.swing.JCheckBox();
        _distancePrefixLabel = new javax.swing.JLabel();
        _maxSpatialRadiusTextField = new javax.swing.JTextField();
        _maxRadiusLabel = new javax.swing.JLabel();
        _inclPureTempClustCheckBox = new javax.swing.JCheckBox();
        _spatialWindowShapeGroup = new javax.swing.JPanel();
        _circularRadioButton = new javax.swing.JRadioButton();
        _ellipticRadioButton = new javax.swing.JRadioButton();
        _nonCompactnessPenaltyLabel = new javax.swing.JLabel();
        _nonCompactnessPenaltyComboBox = new javax.swing.JComboBox();
        _performIsotonicScanCheckBox = new javax.swing.JCheckBox();
        _temporalWindowTab = new javax.swing.JPanel();
        _temporalOptionsGroup = new javax.swing.JPanel();
        _percentageTemporalRadioButton = new javax.swing.JRadioButton();
        _timeTemporalRadioButton = new javax.swing.JRadioButton();
        _maxTemporalClusterSizeTextField = new javax.swing.JTextField();
        _percentageOfStudyPeriodLabel = new javax.swing.JLabel();
        _maxTemporalClusterSizeUnitsTextField = new javax.swing.JTextField();
        _maxTemporalTimeUnitsLabel = new javax.swing.JLabel();
        _includePureSpacClustCheckBox = new javax.swing.JCheckBox();
        _flexibleTemporalWindowDefinitionGroup = new javax.swing.JPanel();
        _restrictTemporalRangeCheckBox = new javax.swing.JCheckBox();
        _startWindowRangeLabel = new javax.swing.JLabel();
        _endWindowRangeLabel = new javax.swing.JLabel();
        _startRangeStartYearTextField = new javax.swing.JTextField();
        _endRangeStartYearTextField = new javax.swing.JTextField();
        _startRangeStartMonthTextField = new javax.swing.JTextField();
        _endRangeStartMonthTextField = new javax.swing.JTextField();
        _startRangeStartDayTextField = new javax.swing.JTextField();
        _endRangeStartDayTextField = new javax.swing.JTextField();
        _startRangeToLabel = new javax.swing.JLabel();
        _endRangeToLabel = new javax.swing.JLabel();
        _startRangeEndYearTextField = new javax.swing.JTextField();
        _endRangeEndYearTextField = new javax.swing.JTextField();
        _startRangeEndMonthTextField = new javax.swing.JTextField();
        _startRangeEndDayTextField = new javax.swing.JTextField();
        _endRangeEndMonthTextField = new javax.swing.JTextField();
        _endRangeEndDayTextField = new javax.swing.JTextField();
        _spaceTimeAjustmentsTab = new javax.swing.JPanel();
        _temporalTrendAdjGroup = new javax.swing.JPanel();
        _temporalTrendAdjNoneRadioButton = new javax.swing.JRadioButton();
        _temporalTrendAdjTimeStratifiedRadioButton = new javax.swing.JRadioButton();
        _temporalTrendAdjLogLinearRadioButton = new javax.swing.JRadioButton();
        _temporalTrendAdjLogLinearCalcRadioButton = new javax.swing.JRadioButton();
        _logLinearTextField = new javax.swing.JTextField();
        _logLinearLabel = new javax.swing.JLabel();
        _spatialAdjustmentsGroup = new javax.swing.JPanel();
        _spatialAdjustmentsNoneRadioButton = new javax.swing.JRadioButton();
        _spatialAdjustmentsSpatialStratifiedRadioButton = new javax.swing.JRadioButton();
        _knownAdjustmentsGroup = new javax.swing.JPanel();
        _adjustForKnownRelativeRisksCheckBox = new javax.swing.JCheckBox();
        _adjustmentsByRelativeRisksFileTextField = new javax.swing.JTextField();
        _adjustmentsFileBrowseButton = new javax.swing.JButton();
        _adjustmentsFileImportButton = new javax.swing.JButton();
        _adjustmentsByRelativeRisksFileLabel = new javax.swing.JLabel();
        _inferenceTab = new javax.swing.JPanel();
        _earlyTerminationGroup = new javax.swing.JPanel();
        _terminateEarlyCheckBox = new javax.swing.JCheckBox();
        _prospectiveSurveillanceGroup = new javax.swing.JPanel();
        _adjustForEarlierAnalysesCheckBox = new javax.swing.JCheckBox();
        _prospectiveStartDateYearTextField = new javax.swing.JTextField();
        _prospectiveStartDateMonthTextField = new javax.swing.JTextField();
        _prospectiveStartDateDayTextField = new javax.swing.JTextField();
        _prospectiveStartYearLabel = new javax.swing.JLabel();
        _prospectiveStartMonthLabel = new javax.swing.JLabel();
        _prospectiveStartDayLabel = new javax.swing.JLabel();
        _reportCriticalValuesGroup = new javax.swing.JPanel();
        _reportCriticalValuesCheckBox = new javax.swing.JCheckBox();
        _iterativeScanGroup = new javax.swing.JPanel();
        _performIterativeScanCheckBox = new javax.swing.JCheckBox();
        _maxIterativeScansLabel = new javax.swing.JLabel();
        _numIterativeScansTextField = new javax.swing.JTextField();
        _iterativeCutoffLabel = new javax.swing.JLabel();
        _iterativeScanCutoffTextField = new javax.swing.JTextField();
        _clustersReportedTab = new javax.swing.JPanel();
        _criteriaSecClustersGroup = new javax.swing.JPanel();
        _criteriaSecClustersNoGeoLapRadioButton = new javax.swing.JRadioButton();
        _criteriaSecClustersNoCentersRadioButton = new javax.swing.JRadioButton();
        _criteriaSecClustersNoCentersMoreRadioButton = new javax.swing.JRadioButton();
        _criteriaSecClustersNoCentersLessRadioButton = new javax.swing.JRadioButton();
        _criteriaSecClustersNoCentersEachRadioButton = new javax.swing.JRadioButton();
        _criteriaSecClustersNoRestrictionsRadioButton = new javax.swing.JRadioButton();
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

        _spatialAdjustmentsButtonGroup.add(_spatialAdjustmentsNoneRadioButton);
        _spatialAdjustmentsButtonGroup.add(_spatialAdjustmentsSpatialStratifiedRadioButton);

        _temporalTrendAdjButtonGroup.add(this._temporalTrendAdjNoneRadioButton);
        _temporalTrendAdjButtonGroup.add(this._temporalTrendAdjTimeStratifiedRadioButton);
        _temporalTrendAdjButtonGroup.add(this._temporalTrendAdjLogLinearRadioButton);
        _temporalTrendAdjButtonGroup.add(this._temporalTrendAdjLogLinearCalcRadioButton);

        _criteriaSecClustersButtonGroup.add(_criteriaSecClustersNoCentersEachRadioButton);
        _criteriaSecClustersButtonGroup.add(_criteriaSecClustersNoCentersLessRadioButton);
        _criteriaSecClustersButtonGroup.add(_criteriaSecClustersNoCentersMoreRadioButton);
        _criteriaSecClustersButtonGroup.add(_criteriaSecClustersNoCentersRadioButton);
        _criteriaSecClustersButtonGroup.add(_criteriaSecClustersNoGeoLapRadioButton);
        _criteriaSecClustersButtonGroup.add(_criteriaSecClustersNoRestrictionsRadioButton);

        setDefaultCloseOperation(javax.swing.WindowConstants.HIDE_ON_CLOSE);

        _additionalDataSetsGroup.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createEtchedBorder(), "Additional Input Data Sets", javax.swing.border.TitledBorder.DEFAULT_JUSTIFICATION, javax.swing.border.TitledBorder.DEFAULT_POSITION, new java.awt.Font("Tahoma", 0, 11), new java.awt.Color(0, 0, 0)));

        _inputDataSetsList.setSelectionMode(javax.swing.ListSelectionModel.SINGLE_SELECTION);
        _inputDataSetsList.addListSelectionListener(new javax.swing.event.ListSelectionListener() {
            public void valueChanged(javax.swing.event.ListSelectionEvent e) {
                EnableInputFileEdits(_dataSetsListModel.getSize() > 0);
                if (_dataSetsListModel.getSize() > 0 && _inputDataSetsList.getSelectedIndex() != -1) {
                    _caseFileTextField.setText(AdvancedParameterSettingsFrame.this._caseFilenames.elementAt(_inputDataSetsList.getSelectedIndex()));
                    _controlFileTextField.setText(AdvancedParameterSettingsFrame.this._controlFilenames.elementAt(_inputDataSetsList.getSelectedIndex()));
                    _populationFileTextField.setText(AdvancedParameterSettingsFrame.this._populationFilenames.elementAt(_inputDataSetsList.getSelectedIndex()));
                }
                enableSetDefaultsButton();
            }
        });
        jScrollPane1.setViewportView(_inputDataSetsList);

        _addDataSetButton.setText("Add"); // NOI18N
        _addDataSetButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                EnableDataSetList(true);
                EnableDataSetPurposeControls(true);
                _dataSetsListModel.addElement("Data Set " + Integer.toString(_inputDataSetsList.getModel().getSize() + 1));

                // enable and clear the edit boxes
                EnableInputFileEdits(true);
                _caseFilenames.addElement("");
                _controlFilenames.addElement("");
                _populationFilenames.addElement("");
                _inputDataSetsList.setSelectedIndex(_dataSetsListModel.getSize() - 1);
                _inputDataSetsList.ensureIndexIsVisible(_dataSetsListModel.getSize() - 1);
                _caseFileTextField.setText("");
                _controlFileTextField.setText("");
                _populationFileTextField.setText("");
                _caseFileTextField.requestFocusInWindow();
                EnableNewButton();
                EnableRemoveButton();
                enableSetDefaultsButton();
            }
        });

        _removeDataSetButton.setText("Remove"); // NOI18N
        _removeDataSetButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                // update remaining list box names
                for (int i=_inputDataSetsList.getSelectedIndex()+1; i < _inputDataSetsList.getModel().getSize() ;i++) {
                    String s = (String)_dataSetsListModel.getElementAt(i);
                    s =	"Data Set " + Integer.toString(i);
                    _dataSetsListModel.setElementAt(s, i);
                }
                int iDeleteIndex = _inputDataSetsList.getSelectedIndex();
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
                EnableDataSetList(_dataSetsListModel.getSize() > 0);
                EnableDataSetPurposeControls(_dataSetsListModel.getSize() > 0);
                EnableInputFileEdits(_dataSetsListModel.getSize() > 0);
                EnableNewButton();
                EnableRemoveButton();
                enableSetDefaultsButton();
            }
        });

        javax.swing.GroupLayout _dataSetsGroupLayout = new javax.swing.GroupLayout(_dataSetsGroup);
        _dataSetsGroup.setLayout(_dataSetsGroupLayout);
        _dataSetsGroupLayout.setHorizontalGroup(
            _dataSetsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addComponent(jScrollPane1, javax.swing.GroupLayout.DEFAULT_SIZE, 140, Short.MAX_VALUE)
            .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, _dataSetsGroupLayout.createSequentialGroup()
                .addComponent(_addDataSetButton, javax.swing.GroupLayout.DEFAULT_SIZE, 63, Short.MAX_VALUE)
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
                .addContainerGap(5, Short.MAX_VALUE))
        );

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
        _caseFileBrowseButton.setToolTipText("Browse for case file ..."); // NOI18N
        _caseFileBrowseButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                JFileChooser fc = new JFileChooser(SaTScanApplication.getInstance().lastBrowseDirectory);
                fc.setDialogTitle("Select Case File");
                fc.addChoosableFileFilter(new InputFileFilter("txt","Text Files (*.txt)"));
                fc.addChoosableFileFilter(new InputFileFilter("cas","Case Files (*.cas)"));
                int returnVal = fc.showOpenDialog(AdvancedParameterSettingsFrame.this);
                if (returnVal == JFileChooser.APPROVE_OPTION) {
                    SaTScanApplication.getInstance().lastBrowseDirectory = fc.getCurrentDirectory();
                    _caseFileTextField.setText(fc.getSelectedFile().getAbsolutePath());
                }
            }
        });

        _caseFileImportButton.setText("...");
        _caseFileImportButton.setToolTipText("Import case file ..."); // NOI18N
        _caseFileImportButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                try {
                    JFileChooser fc = new JFileChooser(SaTScanApplication.getInstance().lastBrowseDirectory);
                    fc.setDialogTitle("Select Case File Import Source");
                    fc.addChoosableFileFilter(new InputFileFilter("dbf","dBase Files (*.dbf)"));
                    fc.addChoosableFileFilter(new InputFileFilter("csv","Delimited Files (*.csv)"));
                    fc.addChoosableFileFilter(new InputFileFilter("xls","Excel Files (*.xls)"));
                    fc.addChoosableFileFilter(new InputFileFilter("txt","Text Files (*.txt)"));
                    fc.addChoosableFileFilter(new InputFileFilter("cas","Case Files (*.cas)"));
                    int returnVal = fc.showOpenDialog(AdvancedParameterSettingsFrame.this);
                    if (returnVal == JFileChooser.APPROVE_OPTION) {
                        SaTScanApplication.getInstance().lastBrowseDirectory = fc.getCurrentDirectory();
                        LaunchImporter(fc.getSelectedFile().getAbsolutePath(), FileImporter.InputFileType.Case);
                    }
                } catch (Throwable t) {
                    new ExceptionDialog(SaTScanApplication.getInstance(), t).setVisible(true);
                }
            }
        });

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
        _controlFileBrowseButton.setToolTipText("Browse for control file ..."); // NOI18N
        _controlFileBrowseButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                JFileChooser fc = new JFileChooser(SaTScanApplication.getInstance().lastBrowseDirectory);
                fc.setDialogTitle("Select Control File");
                fc.addChoosableFileFilter(new InputFileFilter("txt","Text Files (*.txt)"));
                fc.addChoosableFileFilter(new InputFileFilter("ctl","Control Files (*.ctl)"));
                int returnVal = fc.showOpenDialog(AdvancedParameterSettingsFrame.this);
                if (returnVal == JFileChooser.APPROVE_OPTION) {
                    SaTScanApplication.getInstance().lastBrowseDirectory = fc.getCurrentDirectory();
                    _controlFileTextField.setText(fc.getSelectedFile().getAbsolutePath());
                }
            }
        });

        _controlFileImportButton.setText("..."); // NOI18N
        _controlFileImportButton.setToolTipText("Import control file ..."); // NOI18N
        _controlFileImportButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                try {
                    JFileChooser fc = new JFileChooser(SaTScanApplication.getInstance().lastBrowseDirectory);
                    fc.setDialogTitle("Select Control File Import Source");
                    fc.addChoosableFileFilter(new InputFileFilter("dbf","dBase Files (*.dbf)"));
                    fc.addChoosableFileFilter(new InputFileFilter("csv","Delimited Files (*.csv)"));
                    fc.addChoosableFileFilter(new InputFileFilter("xls","Excel Files (*.xls)"));
                    fc.addChoosableFileFilter(new InputFileFilter("txt","Text Files (*.txt)"));
                    fc.addChoosableFileFilter(new InputFileFilter("ctl","Control Files (*.ctl)"));
                    int returnVal = fc.showOpenDialog(AdvancedParameterSettingsFrame.this);
                    if (returnVal == JFileChooser.APPROVE_OPTION) {
                        SaTScanApplication.getInstance().lastBrowseDirectory = fc.getCurrentDirectory();
                        LaunchImporter(fc.getSelectedFile().getAbsolutePath(), FileImporter.InputFileType.Control);
                    }
                } catch (Throwable t) {
                    new ExceptionDialog(SaTScanApplication.getInstance(), t).setVisible(true);
                }
            }
        });

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
        _populationFileBrowseButton.setToolTipText("Browse for population file ..."); // NOI18N
        _populationFileBrowseButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                JFileChooser fc = new JFileChooser(SaTScanApplication.getInstance().lastBrowseDirectory);
                fc.setDialogTitle("Select Population File");
                fc.addChoosableFileFilter(new InputFileFilter("txt","Text Files (*.txt)"));
                fc.addChoosableFileFilter(new InputFileFilter("pop","Population Files (*.pop)"));
                int returnVal = fc.showOpenDialog(AdvancedParameterSettingsFrame.this);
                if (returnVal == JFileChooser.APPROVE_OPTION) {
                    SaTScanApplication.getInstance().lastBrowseDirectory = fc.getCurrentDirectory();
                    _populationFileTextField.setText(fc.getSelectedFile().getAbsolutePath());
                }
            }
        });

        _populationFileImportButton.setText("..."); // NOI18N
        _populationFileImportButton.setToolTipText("Import population file ..."); // NOI18N
        _populationFileImportButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                try {
                    JFileChooser fc = new JFileChooser(SaTScanApplication.getInstance().lastBrowseDirectory);
                    fc.setDialogTitle("Select Source Population Import File");
                    fc.addChoosableFileFilter(new InputFileFilter("dbf","dBase Files (*.dbf)"));
                    fc.addChoosableFileFilter(new InputFileFilter("csv","Delimited Files (*.csv)"));
                    fc.addChoosableFileFilter(new InputFileFilter("xls","Excel Files (*.xls)"));
                    fc.addChoosableFileFilter(new InputFileFilter("txt","Text Files (*.txt)"));
                    fc.addChoosableFileFilter(new InputFileFilter("pop","Population Files (*.pop)"));
                    int returnVal = fc.showOpenDialog(AdvancedParameterSettingsFrame.this);
                    if (returnVal == JFileChooser.APPROVE_OPTION) {
                        SaTScanApplication.getInstance().lastBrowseDirectory = fc.getCurrentDirectory();
                        LaunchImporter(fc.getSelectedFile().getAbsolutePath(), FileImporter.InputFileType.Population);
                    }
                } catch (Throwable t) {
                    new ExceptionDialog(SaTScanApplication.getInstance(), t).setVisible(true);
                }
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
                        .addComponent(_caseFileTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 307, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(_caseFileBrowseButton, javax.swing.GroupLayout.PREFERRED_SIZE, 25, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addGap(6, 6, 6)
                        .addComponent(_caseFileImportButton, javax.swing.GroupLayout.PREFERRED_SIZE, 25, javax.swing.GroupLayout.PREFERRED_SIZE))
                    .addGroup(_fileInputGroupLayout.createSequentialGroup()
                        .addComponent(_controlFileTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 307, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(_controlFileBrowseButton, javax.swing.GroupLayout.PREFERRED_SIZE, 25, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(_controlFileImportButton, javax.swing.GroupLayout.PREFERRED_SIZE, 25, javax.swing.GroupLayout.PREFERRED_SIZE))
                    .addGroup(_fileInputGroupLayout.createSequentialGroup()
                        .addComponent(_populationFileTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 307, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(_populationFileBrowseButton, javax.swing.GroupLayout.PREFERRED_SIZE, 25, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(_populationFileImportButton, javax.swing.GroupLayout.PREFERRED_SIZE, 25, javax.swing.GroupLayout.PREFERRED_SIZE))
                    .addComponent(_caseFileLabel)
                    .addComponent(_controlFileLabel)
                    .addComponent(_populationFileLabel))
                .addContainerGap(10, Short.MAX_VALUE))
        );
        _fileInputGroupLayout.setVerticalGroup(
            _fileInputGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_fileInputGroupLayout.createSequentialGroup()
                .addComponent(_caseFileLabel)
                .addGap(0, 0, 0)
                .addGroup(_fileInputGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(_caseFileBrowseButton)
                    .addComponent(_caseFileImportButton)
                    .addComponent(_caseFileTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_controlFileLabel)
                .addGap(0, 0, 0)
                .addGroup(_fileInputGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(_controlFileBrowseButton)
                    .addComponent(_controlFileImportButton)
                    .addComponent(_controlFileTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_populationFileLabel)
                .addGap(0, 0, 0)
                .addGroup(_fileInputGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(_populationFileBrowseButton)
                    .addComponent(_populationFileImportButton)
                    .addComponent(_populationFileTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );

        _multipleDataSetPurposeLabel.setText("Purpose of Multiple Data Sets:"); // NOI18N

        _multivariateAdjustmentsRadioButton.setSelected(true);
        _multivariateAdjustmentsRadioButton.setText("Multivariate Analysis (clusters in one or more data sets)"); // NOI18N
        _multivariateAdjustmentsRadioButton.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        _multivariateAdjustmentsRadioButton.setMargin(new java.awt.Insets(0, 0, 0, 0));
        _multivariateAdjustmentsRadioButton.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED) enableSetDefaultsButton();
            }
        });

        _adjustmentByDataSetsRadioButton.setText("Adjustment (clusters in all data sets simultaneously)"); // NOI18N
        _adjustmentByDataSetsRadioButton.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        _adjustmentByDataSetsRadioButton.setMargin(new java.awt.Insets(0, 0, 0, 0));
        _adjustmentByDataSetsRadioButton.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED) enableSetDefaultsButton();
            }
        });

        javax.swing.GroupLayout _multipleSetPurposeGroupLayout = new javax.swing.GroupLayout(_multipleSetPurposeGroup);
        _multipleSetPurposeGroup.setLayout(_multipleSetPurposeGroupLayout);
        _multipleSetPurposeGroupLayout.setHorizontalGroup(
            _multipleSetPurposeGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_multipleSetPurposeGroupLayout.createSequentialGroup()
                .addContainerGap()
                .addGroup(_multipleSetPurposeGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(_multipleDataSetPurposeLabel)
                    .addGroup(_multipleSetPurposeGroupLayout.createSequentialGroup()
                        .addGap(10, 10, 10)
                        .addGroup(_multipleSetPurposeGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addComponent(_adjustmentByDataSetsRadioButton)
                            .addComponent(_multivariateAdjustmentsRadioButton))))
                .addContainerGap(232, Short.MAX_VALUE))
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
                .addContainerGap())
        );
        _additionalDataSetsGroupLayout.setVerticalGroup(
            _additionalDataSetsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_additionalDataSetsGroupLayout.createSequentialGroup()
                .addGroup(_additionalDataSetsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                    .addComponent(_dataSetsGroup, 0, 134, Short.MAX_VALUE)
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
                .addContainerGap(104, Short.MAX_VALUE))
        );

        jTabbedPane1.addTab("Multiple Data Sets", _multipleDataSetsTab);

        _studyPeriodCheckGroup.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createEtchedBorder(), "Study Period Check", javax.swing.border.TitledBorder.DEFAULT_JUSTIFICATION, javax.swing.border.TitledBorder.DEFAULT_POSITION, new java.awt.Font("Tahoma", 0, 11), new java.awt.Color(0, 0, 0)));

        _strictStudyPeriodCheckRadioButton.setSelected(true);
        _strictStudyPeriodCheckRadioButton.setText("Check to ensure that cases and controls are within the Study Period."); // NOI18N
        _strictStudyPeriodCheckRadioButton.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        _strictStudyPeriodCheckRadioButton.setMargin(new java.awt.Insets(0, 0, 0, 0));
        _strictStudyPeriodCheckRadioButton.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED) enableSetDefaultsButton();
            }
        });

        _relaxedStudyPeriodCheckRadioButton.setText("Ignore cases and controls that are outside the Study Period."); // NOI18N
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
                    .addComponent(_strictStudyPeriodCheckRadioButton)
                    .addComponent(_relaxedStudyPeriodCheckRadioButton))
                .addContainerGap(198, Short.MAX_VALUE))
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

        _geographicalCoordinatesCheckGroup.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createEtchedBorder(), "Geographical Coordinates Check", javax.swing.border.TitledBorder.DEFAULT_JUSTIFICATION, javax.swing.border.TitledBorder.DEFAULT_POSITION, new java.awt.Font("Tahoma", 0, 11), new java.awt.Color(0, 0, 0)));

        _strictCoordinatesRadioButton.setSelected(true);
        _strictCoordinatesRadioButton.setText("Check to ensure that all locations in the case, control and population files are present"); // NOI18N
        _strictCoordinatesRadioButton.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        _strictCoordinatesRadioButton.setMargin(new java.awt.Insets(0, 0, 0, 0));
        _strictCoordinatesRadioButton.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED) enableSetDefaultsButton();
            }
        });

        _relaxedCoordinatesRadioButton.setText("Ignore data in the case, control and population files that do not correspond to a location ID"); // NOI18N
        _relaxedCoordinatesRadioButton.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        _relaxedCoordinatesRadioButton.setMargin(new java.awt.Insets(0, 0, 0, 0));
        _relaxedCoordinatesRadioButton.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED) enableSetDefaultsButton();
            }
        });

        _strictCoordinatesLabel.setText(" in the coordinates file."); // NOI18N

        _relaxedCoordinatesLabel.setText(" listed in the coordinates file."); // NOI18N

        javax.swing.GroupLayout _geographicalCoordinatesCheckGroupLayout = new javax.swing.GroupLayout(_geographicalCoordinatesCheckGroup);
        _geographicalCoordinatesCheckGroup.setLayout(_geographicalCoordinatesCheckGroupLayout);
        _geographicalCoordinatesCheckGroupLayout.setHorizontalGroup(
            _geographicalCoordinatesCheckGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_geographicalCoordinatesCheckGroupLayout.createSequentialGroup()
                .addContainerGap()
                .addGroup(_geographicalCoordinatesCheckGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(_geographicalCoordinatesCheckGroupLayout.createSequentialGroup()
                        .addGap(17, 17, 17)
                        .addComponent(_strictCoordinatesLabel, javax.swing.GroupLayout.DEFAULT_SIZE, 518, Short.MAX_VALUE))
                    .addComponent(_relaxedCoordinatesRadioButton, javax.swing.GroupLayout.DEFAULT_SIZE, 535, Short.MAX_VALUE)
                    .addComponent(_strictCoordinatesRadioButton, javax.swing.GroupLayout.DEFAULT_SIZE, 535, Short.MAX_VALUE)
                    .addGroup(_geographicalCoordinatesCheckGroupLayout.createSequentialGroup()
                        .addGap(17, 17, 17)
                        .addComponent(_relaxedCoordinatesLabel, javax.swing.GroupLayout.DEFAULT_SIZE, 518, Short.MAX_VALUE)))
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
                .addGap(0, 0, 0)
                .addComponent(_relaxedCoordinatesLabel)
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );

        javax.swing.GroupLayout _dataCheckingTabLayout = new javax.swing.GroupLayout(_dataCheckingTab);
        _dataCheckingTab.setLayout(_dataCheckingTabLayout);
        _dataCheckingTabLayout.setHorizontalGroup(
            _dataCheckingTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_dataCheckingTabLayout.createSequentialGroup()
                .addContainerGap()
                .addGroup(_dataCheckingTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(_studyPeriodCheckGroup, javax.swing.GroupLayout.Alignment.TRAILING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(_geographicalCoordinatesCheckGroup, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                .addContainerGap())
        );
        _dataCheckingTabLayout.setVerticalGroup(
            _dataCheckingTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_dataCheckingTabLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(_studyPeriodCheckGroup, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_geographicalCoordinatesCheckGroup, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap(146, Short.MAX_VALUE))
        );

        jTabbedPane1.addTab("Data Checking", _dataCheckingTab);

        _specialNeighborFilesGroup.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createEtchedBorder(), "Special Neighbor Files", javax.swing.border.TitledBorder.DEFAULT_JUSTIFICATION, javax.swing.border.TitledBorder.DEFAULT_POSITION, new java.awt.Font("Tahoma", 0, 11), new java.awt.Color(0, 0, 0)));

        _specifiyNeighborsFileCheckBox.setText("Specify neighbors through a user defined file"); // NOI18N
        _specifiyNeighborsFileCheckBox.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        _specifiyNeighborsFileCheckBox.setMargin(new java.awt.Insets(0, 0, 0, 0));
        _specifiyNeighborsFileCheckBox.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                EnableSettingsForAnalysisModelCombination();
                enableSetDefaultsButton();
            }
        });

        _neighborsFileTextField.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                enableSetDefaultsButton();
            }
        });

        _neighborsFileBrowseButton.setText("..."); // NOI18N
        _neighborsFileBrowseButton.setToolTipText("Browse for neighbors file ..."); // NOI18N
        _neighborsFileBrowseButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                JFileChooser fc = new JFileChooser(SaTScanApplication.getInstance().lastBrowseDirectory);
                fc.setDialogTitle("Select Neighbors File");
                fc.addChoosableFileFilter(new InputFileFilter("txt","Text Files (*.txt)"));
                fc.addChoosableFileFilter(new InputFileFilter("nei","Neighbors Files (*.nei)"));
                int returnVal = fc.showOpenDialog(AdvancedParameterSettingsFrame.this);
                if (returnVal == JFileChooser.APPROVE_OPTION) {
                    SaTScanApplication.getInstance().lastBrowseDirectory = fc.getCurrentDirectory();
                    _neighborsFileTextField.setText(fc.getSelectedFile().getAbsolutePath());
                }
            }
        });

        _specifiyMetaLocationsFileCheckBox.setText("Specify meta locations through a user defined file"); // NOI18N
        _specifiyMetaLocationsFileCheckBox.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        _specifiyMetaLocationsFileCheckBox.setMargin(new java.awt.Insets(0, 0, 0, 0));
        _specifiyMetaLocationsFileCheckBox.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                EnableSettingsForAnalysisModelCombination();
                enableSetDefaultsButton();
            }
        });

        _metaLocationsFileTextField.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                enableSetDefaultsButton();
            }
        });

        _metaLocationsFileBrowseButton.setText("..."); // NOI18N
        _metaLocationsFileBrowseButton.setToolTipText("Browse for meta locations file ..."); // NOI18N
        _metaLocationsFileBrowseButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                JFileChooser fc = new JFileChooser(SaTScanApplication.getInstance().lastBrowseDirectory);
                fc.setDialogTitle("Select Meta Locations File");
                fc.addChoosableFileFilter(new InputFileFilter("txt","Text Files (*.txt)"));
                fc.addChoosableFileFilter(new InputFileFilter("meta","Meta Locations Files (*.meta)"));
                int returnVal = fc.showOpenDialog(AdvancedParameterSettingsFrame.this);
                if (returnVal == JFileChooser.APPROVE_OPTION) {
                    SaTScanApplication.getInstance().lastBrowseDirectory = fc.getCurrentDirectory();
                    _metaLocationsFileTextField.setText(fc.getSelectedFile().getAbsolutePath());
                }
            }
        });

        javax.swing.GroupLayout _specialNeighborFilesGroupLayout = new javax.swing.GroupLayout(_specialNeighborFilesGroup);
        _specialNeighborFilesGroup.setLayout(_specialNeighborFilesGroupLayout);
        _specialNeighborFilesGroupLayout.setHorizontalGroup(
            _specialNeighborFilesGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_specialNeighborFilesGroupLayout.createSequentialGroup()
                .addContainerGap()
                .addGroup(_specialNeighborFilesGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, _specialNeighborFilesGroupLayout.createSequentialGroup()
                        .addGap(17, 17, 17)
                        .addComponent(_metaLocationsFileTextField, javax.swing.GroupLayout.DEFAULT_SIZE, 487, Short.MAX_VALUE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(_metaLocationsFileBrowseButton, javax.swing.GroupLayout.PREFERRED_SIZE, 25, javax.swing.GroupLayout.PREFERRED_SIZE))
                    .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, _specialNeighborFilesGroupLayout.createSequentialGroup()
                        .addGap(17, 17, 17)
                        .addComponent(_neighborsFileTextField, javax.swing.GroupLayout.DEFAULT_SIZE, 487, Short.MAX_VALUE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(_neighborsFileBrowseButton, javax.swing.GroupLayout.PREFERRED_SIZE, 25, javax.swing.GroupLayout.PREFERRED_SIZE))
                    .addComponent(_specifiyNeighborsFileCheckBox, javax.swing.GroupLayout.DEFAULT_SIZE, 535, Short.MAX_VALUE)
                    .addComponent(_specifiyMetaLocationsFileCheckBox, javax.swing.GroupLayout.DEFAULT_SIZE, 535, Short.MAX_VALUE))
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

        javax.swing.GroupLayout _nonEucledianNeighborsTabLayout = new javax.swing.GroupLayout(_nonEucledianNeighborsTab);
        _nonEucledianNeighborsTab.setLayout(_nonEucledianNeighborsTabLayout);
        _nonEucledianNeighborsTabLayout.setHorizontalGroup(
            _nonEucledianNeighborsTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_nonEucledianNeighborsTabLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(_specialNeighborFilesGroup, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .addContainerGap())
        );
        _nonEucledianNeighborsTabLayout.setVerticalGroup(
            _nonEucledianNeighborsTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_nonEucledianNeighborsTabLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(_specialNeighborFilesGroup, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap(214, Short.MAX_VALUE))
        );

        jTabbedPane1.addTab("Non-Eucledian Neighbors", _nonEucledianNeighborsTab);

        _multipleSetsSpatialCoordinatesGroup.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createEtchedBorder(), "Multiple Sets of Spatial Coordinates per Location ID", javax.swing.border.TitledBorder.DEFAULT_JUSTIFICATION, javax.swing.border.TitledBorder.DEFAULT_POSITION, new java.awt.Font("Tahoma", 0, 11), new java.awt.Color(0, 0, 0)));

        _onePerLocationIdRadioButton.setSelected(true);
        _onePerLocationIdRadioButton.setText("Allow only of set of coordinates per location ID."); // NOI18N
        _onePerLocationIdRadioButton.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        _onePerLocationIdRadioButton.setMargin(new java.awt.Insets(0, 0, 0, 0));
        _onePerLocationIdRadioButton.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED) enableSetDefaultsButton();
            }
        });

        _atLeastOneRadioButton.setText("Include location ID in the scanning window if at least on set of coordinates is included."); // NOI18N
        _atLeastOneRadioButton.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        _atLeastOneRadioButton.setMargin(new java.awt.Insets(0, 0, 0, 0));
        _atLeastOneRadioButton.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED) enableSetDefaultsButton();
            }
        });

        _allLocationsRadioButton.setText("Include location ID in the scanning window if all sets of coordinates are included."); // NOI18N
        _allLocationsRadioButton.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        _allLocationsRadioButton.setMargin(new java.awt.Insets(0, 0, 0, 0));
        _allLocationsRadioButton.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED) enableSetDefaultsButton();
            }
        });

        javax.swing.GroupLayout _multipleSetsSpatialCoordinatesGroupLayout = new javax.swing.GroupLayout(_multipleSetsSpatialCoordinatesGroup);
        _multipleSetsSpatialCoordinatesGroup.setLayout(_multipleSetsSpatialCoordinatesGroupLayout);
        _multipleSetsSpatialCoordinatesGroupLayout.setHorizontalGroup(
            _multipleSetsSpatialCoordinatesGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_multipleSetsSpatialCoordinatesGroupLayout.createSequentialGroup()
                .addContainerGap()
                .addGroup(_multipleSetsSpatialCoordinatesGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(_atLeastOneRadioButton, javax.swing.GroupLayout.DEFAULT_SIZE, 535, Short.MAX_VALUE)
                    .addComponent(_onePerLocationIdRadioButton, javax.swing.GroupLayout.DEFAULT_SIZE, 535, Short.MAX_VALUE)
                    .addComponent(_allLocationsRadioButton, javax.swing.GroupLayout.DEFAULT_SIZE, 535, Short.MAX_VALUE))
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
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );

        javax.swing.GroupLayout _multipleCoordinatesTabLayout = new javax.swing.GroupLayout(_multipleCoordinatesTab);
        _multipleCoordinatesTab.setLayout(_multipleCoordinatesTabLayout);
        _multipleCoordinatesTabLayout.setHorizontalGroup(
            _multipleCoordinatesTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_multipleCoordinatesTabLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(_multipleSetsSpatialCoordinatesGroup, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .addContainerGap())
        );
        _multipleCoordinatesTabLayout.setVerticalGroup(
            _multipleCoordinatesTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_multipleCoordinatesTabLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(_multipleSetsSpatialCoordinatesGroup, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap(243, Short.MAX_VALUE))
        );

        jTabbedPane1.addTab("Mutliple Coordinates Per Location", _multipleCoordinatesTab);

        _spatialOptionsGroup.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createEtchedBorder(), "Maximum Spatial Cluster Size", javax.swing.border.TitledBorder.DEFAULT_JUSTIFICATION, javax.swing.border.TitledBorder.DEFAULT_POSITION, new java.awt.Font("Tahoma", 0, 11), new java.awt.Color(0, 0, 0)));

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

        _percentageOfPopulationLabel.setText("percent of the study period (<= 50%, default = 50%)"); // NOI18N

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

        _percentageOfPopFileLabel.setText("percent of the population defined in the max circle size file (<= 50%)"); // NOI18N

        _maxCirclePopFileBrowseButton.setText("..."); // NOI18N
        _maxCirclePopFileBrowseButton.setToolTipText("Browse for max circle size file ..."); // NOI18N
        _maxCirclePopFileBrowseButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                JFileChooser fc = new JFileChooser(SaTScanApplication.getInstance().lastBrowseDirectory);
                fc.setDialogTitle("Select Max Circle Size File");
                fc.addChoosableFileFilter(new InputFileFilter("txt","Text Files (*.txt)"));
                fc.addChoosableFileFilter(new InputFileFilter("max","Max Circle Size files (*.max)"));
                int returnVal = fc.showOpenDialog(AdvancedParameterSettingsFrame.this);
                if (returnVal == JFileChooser.APPROVE_OPTION) {
                    SaTScanApplication.getInstance().lastBrowseDirectory = fc.getCurrentDirectory();
                    _maxCirclePopulationFilenameTextField.setText(fc.getSelectedFile().getAbsolutePath());
                }
            }
        });

        _maxCirclePopFileImportButton.setText("..."); // NOI18N
        _maxCirclePopFileImportButton.setToolTipText("Import max circle size file ..."); // NOI18N
        _maxCirclePopFileImportButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                try {
                    JFileChooser fc = new JFileChooser(SaTScanApplication.getInstance().lastBrowseDirectory);
                    fc.setDialogTitle("Select Source Max Circle Import File");
                    fc.addChoosableFileFilter(new InputFileFilter("dbf","dBase Files (*.dbf)"));
                    fc.addChoosableFileFilter(new InputFileFilter("csv","Delimited Files (*.csv)"));
                    fc.addChoosableFileFilter(new InputFileFilter("xls","Excel Files (*.xls)"));
                    fc.addChoosableFileFilter(new InputFileFilter("txt","Text Files (*.txt)"));
                    fc.addChoosableFileFilter(new InputFileFilter("max","Max Circle Files (*.max)"));
                    int returnVal = fc.showOpenDialog(AdvancedParameterSettingsFrame.this);
                    if (returnVal == JFileChooser.APPROVE_OPTION) {
                        SaTScanApplication.getInstance().lastBrowseDirectory = fc.getCurrentDirectory();
                        LaunchImporter(fc.getSelectedFile().getAbsolutePath(), FileImporter.InputFileType.MaxCirclePopulation);
                    }
                } catch (Throwable t) {
                    new ExceptionDialog(SaTScanApplication.getInstance(), t).setVisible(true);
                }
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
                Utils.validatePostiveNumericKeyTyped(_maxSpatialRadiusTextField, e, 20);
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
                .addContainerGap()
                .addComponent(_maxSpatialClusterSizeTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 45, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(_spatialOptionsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(_percentageOfPopulationLabel, javax.swing.GroupLayout.DEFAULT_SIZE, 484, Short.MAX_VALUE)
                    .addGroup(_spatialOptionsGroupLayout.createSequentialGroup()
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addGroup(_spatialOptionsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addGroup(_spatialOptionsGroupLayout.createSequentialGroup()
                                .addComponent(_spatialDistanceCheckBox)
                                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                .addComponent(_distancePrefixLabel)
                                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                .addComponent(_maxSpatialRadiusTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 72, javax.swing.GroupLayout.PREFERRED_SIZE)
                                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                .addComponent(_maxRadiusLabel, javax.swing.GroupLayout.DEFAULT_SIZE, 301, Short.MAX_VALUE))
                            .addGroup(_spatialOptionsGroupLayout.createSequentialGroup()
                                .addComponent(_spatialPopulationFileCheckBox)
                                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                .addGroup(_spatialOptionsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                                    .addGroup(_spatialOptionsGroupLayout.createSequentialGroup()
                                        .addComponent(_maxCirclePopulationFilenameTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 375, javax.swing.GroupLayout.PREFERRED_SIZE)
                                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                        .addComponent(_maxCirclePopFileBrowseButton, javax.swing.GroupLayout.PREFERRED_SIZE, 25, javax.swing.GroupLayout.PREFERRED_SIZE)
                                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                        .addComponent(_maxCirclePopFileImportButton, javax.swing.GroupLayout.PREFERRED_SIZE, 25, javax.swing.GroupLayout.PREFERRED_SIZE))
                                    .addGroup(_spatialOptionsGroupLayout.createSequentialGroup()
                                        .addComponent(_maxSpatialPercentFileTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 45, javax.swing.GroupLayout.PREFERRED_SIZE)
                                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                        .addComponent(_percentageOfPopFileLabel, javax.swing.GroupLayout.DEFAULT_SIZE, 416, Short.MAX_VALUE)))))))
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
                .addGroup(_spatialOptionsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING)
                    .addComponent(_spatialPopulationFileCheckBox)
                    .addGroup(_spatialOptionsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                        .addComponent(_maxSpatialPercentFileTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addComponent(_percentageOfPopFileLabel)))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(_spatialOptionsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(_maxCirclePopulationFilenameTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(_maxCirclePopFileBrowseButton)
                    .addComponent(_maxCirclePopFileImportButton))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(_spatialOptionsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING)
                    .addComponent(_spatialDistanceCheckBox)
                    .addGroup(_spatialOptionsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                        .addComponent(_distancePrefixLabel)
                        .addComponent(_maxSpatialRadiusTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addComponent(_maxRadiusLabel)))
                .addContainerGap(21, Short.MAX_VALUE))
        );

        _inclPureTempClustCheckBox.setText("Include Purely Temporal Clusters (Spatial Size = 100%)"); // NOI18N
        _inclPureTempClustCheckBox.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        _inclPureTempClustCheckBox.setMargin(new java.awt.Insets(0, 0, 0, 0));
        _inclPureTempClustCheckBox.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                enableSetDefaultsButton();
            }
        });

        _spatialWindowShapeGroup.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createEtchedBorder(), "Spatial Window Shape", javax.swing.border.TitledBorder.DEFAULT_JUSTIFICATION, javax.swing.border.TitledBorder.DEFAULT_POSITION, new java.awt.Font("Tahoma", 0, 11), new java.awt.Color(0, 0, 0)));

        _circularRadioButton.setSelected(true);
        _circularRadioButton.setText("Circular"); // NOI18N
        _circularRadioButton.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        _circularRadioButton.setMargin(new java.awt.Insets(0, 0, 0, 0));
        _circularRadioButton.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED) {
                    EnableWindowShapeGroup(_spatialWindowShapeGroup.isEnabled());
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
                    EnableWindowShapeGroup(_spatialWindowShapeGroup.isEnabled());
                    enableSetDefaultsButton();
                }
            }
        });

        _nonCompactnessPenaltyLabel.setText("Non-compactness penalty:"); // NOI18N

        _nonCompactnessPenaltyComboBox.setModel(new javax.swing.DefaultComboBoxModel(new String[] { "None", "Medium", "Strong" }));
        _nonCompactnessPenaltyComboBox.setLightWeightPopupEnabled(false);
        _nonCompactnessPenaltyComboBox.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                enableSetDefaultsButton();
            }
        });

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
                        .addComponent(_nonCompactnessPenaltyComboBox, javax.swing.GroupLayout.PREFERRED_SIZE, 148, javax.swing.GroupLayout.PREFERRED_SIZE)))
                .addContainerGap(132, Short.MAX_VALUE))
        );
        _spatialWindowShapeGroupLayout.setVerticalGroup(
            _spatialWindowShapeGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_spatialWindowShapeGroupLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(_circularRadioButton)
                .addGap(10, 10, 10)
                .addGroup(_spatialWindowShapeGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(_ellipticRadioButton)
                    .addComponent(_nonCompactnessPenaltyLabel)
                    .addComponent(_nonCompactnessPenaltyComboBox, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );

        _performIsotonicScanCheckBox.setText("Use Isotonic Spatial Scan Statistic"); // NOI18N
        _performIsotonicScanCheckBox.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        _performIsotonicScanCheckBox.setMargin(new java.awt.Insets(0, 0, 0, 0));
        _performIsotonicScanCheckBox.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                enableSetDefaultsButton();
                EnableSettingsForAnalysisModelCombination();
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
                    .addComponent(_inclPureTempClustCheckBox, javax.swing.GroupLayout.DEFAULT_SIZE, 567, Short.MAX_VALUE)
                    .addComponent(_performIsotonicScanCheckBox, javax.swing.GroupLayout.DEFAULT_SIZE, 567, Short.MAX_VALUE))
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
                .addContainerGap(56, Short.MAX_VALUE))
        );

        jTabbedPane1.addTab("Spatial Window", _spatialWindowTab);

        _temporalOptionsGroup.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createEtchedBorder(), "Maximum Temporal Cluster Size", javax.swing.border.TitledBorder.DEFAULT_JUSTIFICATION, javax.swing.border.TitledBorder.DEFAULT_POSITION, new java.awt.Font("Tahoma", 0, 11), new java.awt.Color(0, 0, 0)));

        _percentageTemporalRadioButton.setSelected(true);
        _percentageTemporalRadioButton.setText("is"); // NOI18N
        _percentageTemporalRadioButton.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        _percentageTemporalRadioButton.setMargin(new java.awt.Insets(0, 0, 0, 0));
        _percentageTemporalRadioButton.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED) {
                    //cause enabling to be refreshed based upon clicked radio button
                    EnableTemporalOptionsGroup(_temporalOptionsGroup.isEnabled(), _includePureSpacClustCheckBox.isEnabled(), _restrictTemporalRangeCheckBox.isEnabled());
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
                    EnableTemporalOptionsGroup(_temporalOptionsGroup.isEnabled(), _includePureSpacClustCheckBox.isEnabled(), _restrictTemporalRangeCheckBox.isEnabled());
                    enableSetDefaultsButton();
                }
            }
        });

        _maxTemporalClusterSizeTextField.setText("50"); // NOI18N
        _maxTemporalClusterSizeTextField.addKeyListener(new java.awt.event.KeyAdapter() {
            public void keyTyped(java.awt.event.KeyEvent e) {
                Utils.validatePostiveNumericKeyTyped(_maxTemporalClusterSizeTextField, e, 5);
            }
        });
        _maxTemporalClusterSizeTextField.addFocusListener(new java.awt.event.FocusAdapter() {
            public void focusLost(java.awt.event.FocusEvent e) {
                double dMaxValue = _analysisSettingsWindow.getModelControlType() == Parameters.ProbabilityModelType.SPACETIMEPERMUTATION ? 50.0 : 90.0;
                while (_maxTemporalClusterSizeTextField.getText().length() == 0 ||
                    Double.parseDouble(_maxTemporalClusterSizeTextField.getText()) == 0 ||
                    Double.parseDouble(_maxTemporalClusterSizeTextField.getText()) > dMaxValue) {
                    if (undo.canUndo()) undo.undo(); else _maxTemporalClusterSizeTextField.setText("50");
                }
                enableSetDefaultsButton();
            }
        });
        _maxTemporalClusterSizeTextField.getDocument().addUndoableEditListener(new UndoableEditListener() {
            public void undoableEditHappened(UndoableEditEvent evt) {
                undo.addEdit(evt.getEdit());
            }
        });

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

        _maxTemporalTimeUnitsLabel.setText("years"); // NOI18N

        javax.swing.GroupLayout _temporalOptionsGroupLayout = new javax.swing.GroupLayout(_temporalOptionsGroup);
        _temporalOptionsGroup.setLayout(_temporalOptionsGroupLayout);
        _temporalOptionsGroupLayout.setHorizontalGroup(
            _temporalOptionsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_temporalOptionsGroupLayout.createSequentialGroup()
                .addContainerGap()
                .addGroup(_temporalOptionsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(_temporalOptionsGroupLayout.createSequentialGroup()
                        .addComponent(_percentageTemporalRadioButton)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(_maxTemporalClusterSizeTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 45, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(_percentageOfStudyPeriodLabel))
                    .addGroup(_temporalOptionsGroupLayout.createSequentialGroup()
                        .addComponent(_timeTemporalRadioButton)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(_maxTemporalClusterSizeUnitsTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 45, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(_maxTemporalTimeUnitsLabel)))
                .addContainerGap(202, Short.MAX_VALUE))
        );
        _temporalOptionsGroupLayout.setVerticalGroup(
            _temporalOptionsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_temporalOptionsGroupLayout.createSequentialGroup()
                .addContainerGap()
                .addGroup(_temporalOptionsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(_percentageTemporalRadioButton)
                    .addComponent(_maxTemporalClusterSizeTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(_percentageOfStudyPeriodLabel))
                .addGap(10, 10, 10)
                .addGroup(_temporalOptionsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(_timeTemporalRadioButton)
                    .addComponent(_maxTemporalClusterSizeUnitsTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(_maxTemporalTimeUnitsLabel))
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );

        _includePureSpacClustCheckBox.setText("Include Purely Spatial Clusters (Temporal Size = 100%)"); // NOI18N
        _includePureSpacClustCheckBox.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        _includePureSpacClustCheckBox.setMargin(new java.awt.Insets(0, 0, 0, 0));
        _includePureSpacClustCheckBox.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                enableSetDefaultsButton();
            }
        });

        _flexibleTemporalWindowDefinitionGroup.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createEtchedBorder(), "Flexible Temporal Window Definition", javax.swing.border.TitledBorder.DEFAULT_JUSTIFICATION, javax.swing.border.TitledBorder.DEFAULT_POSITION, new java.awt.Font("Tahoma", 0, 11), new java.awt.Color(0, 0, 0)));

        _restrictTemporalRangeCheckBox.setText("Include only windows with:"); // NOI18N
        _restrictTemporalRangeCheckBox.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        _restrictTemporalRangeCheckBox.setMargin(new java.awt.Insets(0, 0, 0, 0));
        _restrictTemporalRangeCheckBox.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                enableReportedSpatialOptionsGroup(_spatialOptionsGroup.isEnabled());
                EnableDatesByTimePrecisionUnits();
                enableSetDefaultsButton();
            }
        });

        _startWindowRangeLabel.setText("Start time in range:"); // NOI18N

        _endWindowRangeLabel.setText("End time in range:"); // NOI18N

        _startRangeStartYearTextField.setText("2000"); // NOI18N
        _startRangeStartYearTextField.getDocument().addUndoableEditListener(new UndoableEditListener() {
            public void undoableEditHappened(UndoableEditEvent evt) {
                undo.addEdit(evt.getEdit());
            }
        });
        _startRangeStartYearTextField.addKeyListener(new java.awt.event.KeyAdapter() {
            public void keyTyped(java.awt.event.KeyEvent e) {
                Utils.validatePostiveNumericKeyTyped(_startRangeStartYearTextField, e, 4);
            }
        });
        _startRangeStartYearTextField.addFocusListener(new java.awt.event.FocusAdapter() {
            public void focusLost(java.awt.event.FocusEvent e) {
                Utils.validateDateControlGroup(_startRangeStartYearTextField, _startRangeStartMonthTextField, _startRangeStartDayTextField, undo);
                enableSetDefaultsButton();
            }
        });

        _endRangeStartYearTextField.setText("2000"); // NOI18N
        _endRangeStartYearTextField.getDocument().addUndoableEditListener(new UndoableEditListener() {
            public void undoableEditHappened(UndoableEditEvent evt) {
                undo.addEdit(evt.getEdit());
            }
        });
        _endRangeStartYearTextField.addKeyListener(new java.awt.event.KeyAdapter() {
            public void keyTyped(java.awt.event.KeyEvent e) {
                Utils.validatePostiveNumericKeyTyped(_endRangeStartYearTextField, e, 4);
            }
        });
        _endRangeStartYearTextField.addFocusListener(new java.awt.event.FocusAdapter() {
            public void focusLost(java.awt.event.FocusEvent e) {
                Utils.validateDateControlGroup(_endRangeStartYearTextField, _endRangeStartMonthTextField, _endRangeStartDayTextField, undo);
                enableSetDefaultsButton();
            }
        });

        _startRangeStartMonthTextField.setText("01"); // NOI18N
        _startRangeStartMonthTextField.getDocument().addUndoableEditListener(new UndoableEditListener() {
            public void undoableEditHappened(UndoableEditEvent evt) {
                undo.addEdit(evt.getEdit());
            }
        });
        _startRangeStartMonthTextField.addKeyListener(new java.awt.event.KeyAdapter() {
            public void keyTyped(java.awt.event.KeyEvent e) {
                Utils.validatePostiveNumericKeyTyped(_startRangeStartMonthTextField, e, 2);
            }
        });
        _startRangeStartMonthTextField.addFocusListener(new java.awt.event.FocusAdapter() {
            public void focusLost(java.awt.event.FocusEvent e) {
                Utils.validateDateControlGroup(_startRangeStartYearTextField, _startRangeStartMonthTextField, _startRangeStartDayTextField, undo);
                enableSetDefaultsButton();
            }
        });

        _endRangeStartMonthTextField.setText("12"); // NOI18N
        _endRangeStartMonthTextField.getDocument().addUndoableEditListener(new UndoableEditListener() {
            public void undoableEditHappened(UndoableEditEvent evt) {
                undo.addEdit(evt.getEdit());
            }
        });
        _endRangeStartMonthTextField.addKeyListener(new java.awt.event.KeyAdapter() {
            public void keyTyped(java.awt.event.KeyEvent e) {
                Utils.validatePostiveNumericKeyTyped(_endRangeStartMonthTextField, e, 2);
            }
        });
        _endRangeStartMonthTextField.addFocusListener(new java.awt.event.FocusAdapter() {
            public void focusLost(java.awt.event.FocusEvent e) {
                Utils.validateDateControlGroup(_endRangeStartYearTextField, _endRangeStartMonthTextField, _endRangeStartDayTextField, undo);
                enableSetDefaultsButton();
            }
        });

        _startRangeStartDayTextField.setText("01"); // NOI18N
        _startRangeStartDayTextField.getDocument().addUndoableEditListener(new UndoableEditListener() {
            public void undoableEditHappened(UndoableEditEvent evt) {
                undo.addEdit(evt.getEdit());
            }
        });
        _startRangeStartDayTextField.addKeyListener(new java.awt.event.KeyAdapter() {
            public void keyTyped(java.awt.event.KeyEvent e) {
                Utils.validatePostiveNumericKeyTyped(_startRangeStartDayTextField, e, 2);
            }
        });
        _startRangeStartDayTextField.addFocusListener(new java.awt.event.FocusAdapter() {
            public void focusLost(java.awt.event.FocusEvent e) {
                Utils.validateDateControlGroup(_startRangeStartYearTextField, _startRangeStartMonthTextField, _startRangeStartDayTextField, undo);
                enableSetDefaultsButton();
            }
        });

        _endRangeStartDayTextField.setText("31"); // NOI18N
        _endRangeStartDayTextField.getDocument().addUndoableEditListener(new UndoableEditListener() {
            public void undoableEditHappened(UndoableEditEvent evt) {
                undo.addEdit(evt.getEdit());
            }
        });
        _endRangeStartDayTextField.addKeyListener(new java.awt.event.KeyAdapter() {
            public void keyTyped(java.awt.event.KeyEvent e) {
                Utils.validatePostiveNumericKeyTyped(_endRangeStartDayTextField, e, 2);
            }
        });
        _endRangeStartDayTextField.addFocusListener(new java.awt.event.FocusAdapter() {
            public void focusLost(java.awt.event.FocusEvent e) {
                Utils.validateDateControlGroup(_endRangeStartYearTextField, _endRangeStartMonthTextField, _endRangeStartDayTextField, undo);
                enableSetDefaultsButton();
            }
        });

        _startRangeToLabel.setText("to"); // NOI18N

        _endRangeToLabel.setText("to"); // NOI18N

        _startRangeEndYearTextField.setText("2000"); // NOI18N
        _startRangeEndYearTextField.getDocument().addUndoableEditListener(new UndoableEditListener() {
            public void undoableEditHappened(UndoableEditEvent evt) {
                undo.addEdit(evt.getEdit());
            }
        });
        _startRangeEndYearTextField.addKeyListener(new java.awt.event.KeyAdapter() {
            public void keyTyped(java.awt.event.KeyEvent e) {
                Utils.validatePostiveNumericKeyTyped(_startRangeEndYearTextField, e, 4);
            }
        });
        _startRangeEndYearTextField.addFocusListener(new java.awt.event.FocusAdapter() {
            public void focusLost(java.awt.event.FocusEvent e) {
                Utils.validateDateControlGroup(_startRangeEndYearTextField, _startRangeEndMonthTextField, _startRangeEndDayTextField, undo);
                enableSetDefaultsButton();
            }
        });

        _endRangeEndYearTextField.setText("2000"); // NOI18N
        _endRangeEndYearTextField.getDocument().addUndoableEditListener(new UndoableEditListener() {
            public void undoableEditHappened(UndoableEditEvent evt) {
                undo.addEdit(evt.getEdit());
            }
        });
        _endRangeEndYearTextField.addKeyListener(new java.awt.event.KeyAdapter() {
            public void keyTyped(java.awt.event.KeyEvent e) {
                Utils.validatePostiveNumericKeyTyped(_endRangeEndYearTextField, e, 4);
            }
        });
        _endRangeEndYearTextField.addFocusListener(new java.awt.event.FocusAdapter() {
            public void focusLost(java.awt.event.FocusEvent e) {
                Utils.validateDateControlGroup(_endRangeEndYearTextField, _endRangeEndMonthTextField, _endRangeEndDayTextField, undo);
                enableSetDefaultsButton();
            }
        });

        _startRangeEndMonthTextField.setText("01"); // NOI18N
        _startRangeEndMonthTextField.getDocument().addUndoableEditListener(new UndoableEditListener() {
            public void undoableEditHappened(UndoableEditEvent evt) {
                undo.addEdit(evt.getEdit());
            }
        });
        _startRangeEndMonthTextField.addKeyListener(new java.awt.event.KeyAdapter() {
            public void keyTyped(java.awt.event.KeyEvent e) {
                Utils.validatePostiveNumericKeyTyped(_startRangeEndMonthTextField, e, 2);
            }
        });
        _startRangeEndMonthTextField.addFocusListener(new java.awt.event.FocusAdapter() {
            public void focusLost(java.awt.event.FocusEvent e) {
                Utils.validateDateControlGroup(_startRangeEndYearTextField, _startRangeEndMonthTextField, _startRangeEndDayTextField, undo);
                enableSetDefaultsButton();
            }
        });

        _startRangeEndDayTextField.setText("01"); // NOI18N
        _startRangeEndDayTextField.getDocument().addUndoableEditListener(new UndoableEditListener() {
            public void undoableEditHappened(UndoableEditEvent evt) {
                undo.addEdit(evt.getEdit());
            }
        });
        _startRangeEndDayTextField.addKeyListener(new java.awt.event.KeyAdapter() {
            public void keyTyped(java.awt.event.KeyEvent e) {
                Utils.validatePostiveNumericKeyTyped(_startRangeEndDayTextField, e, 2);
            }
        });
        _startRangeEndDayTextField.addFocusListener(new java.awt.event.FocusAdapter() {
            public void focusLost(java.awt.event.FocusEvent e) {
                Utils.validateDateControlGroup(_startRangeEndYearTextField, _startRangeEndMonthTextField, _startRangeEndDayTextField, undo);
                enableSetDefaultsButton();
            }
        });

        _endRangeEndMonthTextField.setText("12"); // NOI18N
        _endRangeEndMonthTextField.getDocument().addUndoableEditListener(new UndoableEditListener() {
            public void undoableEditHappened(UndoableEditEvent evt) {
                undo.addEdit(evt.getEdit());
            }
        });
        _endRangeEndMonthTextField.addKeyListener(new java.awt.event.KeyAdapter() {
            public void keyTyped(java.awt.event.KeyEvent e) {
                Utils.validatePostiveNumericKeyTyped(_endRangeEndMonthTextField, e, 2);
            }
        });
        _endRangeEndMonthTextField.addFocusListener(new java.awt.event.FocusAdapter() {
            public void focusLost(java.awt.event.FocusEvent e) {
                Utils.validateDateControlGroup(_endRangeEndYearTextField, _endRangeEndMonthTextField, _endRangeEndDayTextField, undo);
                enableSetDefaultsButton();
            }
        });

        _endRangeEndDayTextField.setText("31"); // NOI18N
        _endRangeEndDayTextField.getDocument().addUndoableEditListener(new UndoableEditListener() {
            public void undoableEditHappened(UndoableEditEvent evt) {
                undo.addEdit(evt.getEdit());
            }
        });
        _endRangeEndDayTextField.addKeyListener(new java.awt.event.KeyAdapter() {
            public void keyTyped(java.awt.event.KeyEvent e) {
                Utils.validatePostiveNumericKeyTyped(_endRangeEndDayTextField, e, 2);
            }
        });
        _endRangeEndDayTextField.addFocusListener(new java.awt.event.FocusAdapter() {
            public void focusLost(java.awt.event.FocusEvent e) {
                Utils.validateDateControlGroup(_endRangeEndYearTextField, _endRangeEndMonthTextField, _endRangeEndDayTextField, undo);
                enableSetDefaultsButton();
            }
        });

        javax.swing.GroupLayout _flexibleTemporalWindowDefinitionGroupLayout = new javax.swing.GroupLayout(_flexibleTemporalWindowDefinitionGroup);
        _flexibleTemporalWindowDefinitionGroup.setLayout(_flexibleTemporalWindowDefinitionGroupLayout);
        _flexibleTemporalWindowDefinitionGroupLayout.setHorizontalGroup(
            _flexibleTemporalWindowDefinitionGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_flexibleTemporalWindowDefinitionGroupLayout.createSequentialGroup()
                .addContainerGap()
                .addGroup(_flexibleTemporalWindowDefinitionGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(_restrictTemporalRangeCheckBox)
                    .addGroup(_flexibleTemporalWindowDefinitionGroupLayout.createSequentialGroup()
                        .addGap(17, 17, 17)
                        .addGroup(_flexibleTemporalWindowDefinitionGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addComponent(_startWindowRangeLabel)
                            .addComponent(_endWindowRangeLabel))
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addGroup(_flexibleTemporalWindowDefinitionGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addGroup(_flexibleTemporalWindowDefinitionGroupLayout.createSequentialGroup()
                                .addComponent(_startRangeStartYearTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 45, javax.swing.GroupLayout.PREFERRED_SIZE)
                                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                .addComponent(_startRangeStartMonthTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 30, javax.swing.GroupLayout.PREFERRED_SIZE)
                                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                .addComponent(_startRangeStartDayTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 30, javax.swing.GroupLayout.PREFERRED_SIZE)
                                .addGap(10, 10, 10)
                                .addComponent(_startRangeToLabel)
                                .addGap(10, 10, 10)
                                .addComponent(_startRangeEndYearTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 45, javax.swing.GroupLayout.PREFERRED_SIZE))
                            .addGroup(_flexibleTemporalWindowDefinitionGroupLayout.createSequentialGroup()
                                .addComponent(_endRangeStartYearTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 45, javax.swing.GroupLayout.PREFERRED_SIZE)
                                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                .addComponent(_endRangeStartMonthTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 30, javax.swing.GroupLayout.PREFERRED_SIZE)
                                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                .addComponent(_endRangeStartDayTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 30, javax.swing.GroupLayout.PREFERRED_SIZE)
                                .addGap(10, 10, 10)
                                .addComponent(_endRangeToLabel)
                                .addGap(10, 10, 10)
                                .addComponent(_endRangeEndYearTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 45, javax.swing.GroupLayout.PREFERRED_SIZE)))
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addGroup(_flexibleTemporalWindowDefinitionGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addGroup(_flexibleTemporalWindowDefinitionGroupLayout.createSequentialGroup()
                                .addComponent(_startRangeEndMonthTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 30, javax.swing.GroupLayout.PREFERRED_SIZE)
                                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                .addComponent(_startRangeEndDayTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 30, javax.swing.GroupLayout.PREFERRED_SIZE))
                            .addGroup(_flexibleTemporalWindowDefinitionGroupLayout.createSequentialGroup()
                                .addComponent(_endRangeEndMonthTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 30, javax.swing.GroupLayout.PREFERRED_SIZE)
                                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                .addComponent(_endRangeEndDayTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 30, javax.swing.GroupLayout.PREFERRED_SIZE)))))
                .addContainerGap(167, Short.MAX_VALUE))
        );
        _flexibleTemporalWindowDefinitionGroupLayout.setVerticalGroup(
            _flexibleTemporalWindowDefinitionGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_flexibleTemporalWindowDefinitionGroupLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(_restrictTemporalRangeCheckBox)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(_flexibleTemporalWindowDefinitionGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(_startWindowRangeLabel)
                    .addComponent(_startRangeStartYearTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(_startRangeStartMonthTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(_startRangeStartDayTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(_startRangeToLabel)
                    .addComponent(_startRangeEndYearTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(_startRangeEndMonthTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(_startRangeEndDayTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(_flexibleTemporalWindowDefinitionGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
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

        javax.swing.GroupLayout _temporalWindowTabLayout = new javax.swing.GroupLayout(_temporalWindowTab);
        _temporalWindowTab.setLayout(_temporalWindowTabLayout);
        _temporalWindowTabLayout.setHorizontalGroup(
            _temporalWindowTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, _temporalWindowTabLayout.createSequentialGroup()
                .addContainerGap()
                .addGroup(_temporalWindowTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING)
                    .addComponent(_flexibleTemporalWindowDefinitionGroup, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(_temporalOptionsGroup, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(_includePureSpacClustCheckBox, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, 567, Short.MAX_VALUE))
                .addContainerGap())
        );
        _temporalWindowTabLayout.setVerticalGroup(
            _temporalWindowTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_temporalWindowTabLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(_temporalOptionsGroup, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_includePureSpacClustCheckBox)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_flexibleTemporalWindowDefinitionGroup, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap(116, Short.MAX_VALUE))
        );

        jTabbedPane1.addTab("Temporal Window", _temporalWindowTab);

        _temporalTrendAdjGroup.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createEtchedBorder(), "Temporal Adjustments", javax.swing.border.TitledBorder.DEFAULT_JUSTIFICATION, javax.swing.border.TitledBorder.DEFAULT_POSITION, new java.awt.Font("Tahoma", 0, 11), new java.awt.Color(0, 0, 0)));

        _temporalTrendAdjNoneRadioButton.setSelected(true);
        _temporalTrendAdjNoneRadioButton.setText("None"); // NOI18N
        _temporalTrendAdjNoneRadioButton.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        _temporalTrendAdjNoneRadioButton.setMargin(new java.awt.Insets(0, 0, 0, 0));
        _temporalTrendAdjNoneRadioButton.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                enableSetDefaultsButton();
                EnableSettingsForAnalysisModelCombination();
            }
        });

        _temporalTrendAdjTimeStratifiedRadioButton.setText("Nonparametric, with time stratified randomization"); // NOI18N
        _temporalTrendAdjTimeStratifiedRadioButton.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        _temporalTrendAdjTimeStratifiedRadioButton.setMargin(new java.awt.Insets(0, 0, 0, 0));
        _temporalTrendAdjTimeStratifiedRadioButton.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED) {
                    _spatialAdjustmentsSpatialStratifiedRadioButton.setEnabled(true);
                    enableSetDefaultsButton();
                    EnableSettingsForAnalysisModelCombination();
                }
            }
        });

        _temporalTrendAdjLogLinearRadioButton.setText("Log linear trend with"); // NOI18N
        _temporalTrendAdjLogLinearRadioButton.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        _temporalTrendAdjLogLinearRadioButton.setMargin(new java.awt.Insets(0, 0, 0, 0));
        _temporalTrendAdjLogLinearRadioButton.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED) {
                    _logLinearTextField.setEnabled(true);
                    enableSetDefaultsButton();
                    EnableSettingsForAnalysisModelCombination();
                }
            }
        });

        _temporalTrendAdjLogLinearCalcRadioButton.setText("Log linear with automatically calculated trend"); // NOI18N
        _temporalTrendAdjLogLinearCalcRadioButton.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        _temporalTrendAdjLogLinearCalcRadioButton.setMargin(new java.awt.Insets(0, 0, 0, 0));
        _temporalTrendAdjLogLinearCalcRadioButton.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                enableSetDefaultsButton();
                EnableSettingsForAnalysisModelCombination();
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
                Utils.validatePostiveFloatKeyTyped(_logLinearTextField, e, 5);
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

        javax.swing.GroupLayout _temporalTrendAdjGroupLayout = new javax.swing.GroupLayout(_temporalTrendAdjGroup);
        _temporalTrendAdjGroup.setLayout(_temporalTrendAdjGroupLayout);
        _temporalTrendAdjGroupLayout.setHorizontalGroup(
            _temporalTrendAdjGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_temporalTrendAdjGroupLayout.createSequentialGroup()
                .addContainerGap()
                .addGroup(_temporalTrendAdjGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(_temporalTrendAdjLogLinearCalcRadioButton, javax.swing.GroupLayout.DEFAULT_SIZE, 535, Short.MAX_VALUE)
                    .addComponent(_temporalTrendAdjNoneRadioButton, javax.swing.GroupLayout.DEFAULT_SIZE, 535, Short.MAX_VALUE)
                    .addGroup(_temporalTrendAdjGroupLayout.createSequentialGroup()
                        .addComponent(_temporalTrendAdjLogLinearRadioButton)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(_logLinearTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 61, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(_logLinearLabel, javax.swing.GroupLayout.DEFAULT_SIZE, 349, Short.MAX_VALUE))
                    .addComponent(_temporalTrendAdjTimeStratifiedRadioButton, javax.swing.GroupLayout.DEFAULT_SIZE, 535, Short.MAX_VALUE))
                .addContainerGap())
        );
        _temporalTrendAdjGroupLayout.setVerticalGroup(
            _temporalTrendAdjGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_temporalTrendAdjGroupLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(_temporalTrendAdjNoneRadioButton)
                .addGap(10, 10, 10)
                .addComponent(_temporalTrendAdjTimeStratifiedRadioButton)
                .addGap(10, 10, 10)
                .addGroup(_temporalTrendAdjGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(_temporalTrendAdjLogLinearRadioButton)
                    .addComponent(_logLinearTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(_logLinearLabel))
                .addGap(10, 10, 10)
                .addComponent(_temporalTrendAdjLogLinearCalcRadioButton)
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );

        _spatialAdjustmentsGroup.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createEtchedBorder(), "Spatial Adjustments", javax.swing.border.TitledBorder.DEFAULT_JUSTIFICATION, javax.swing.border.TitledBorder.DEFAULT_POSITION, new java.awt.Font("Tahoma", 0, 11), new java.awt.Color(0, 0, 0)));

        _spatialAdjustmentsNoneRadioButton.setSelected(true);
        _spatialAdjustmentsNoneRadioButton.setText("None"); // NOI18N
        _spatialAdjustmentsNoneRadioButton.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        _spatialAdjustmentsNoneRadioButton.setMargin(new java.awt.Insets(0, 0, 0, 0));
        _spatialAdjustmentsNoneRadioButton.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                EnableSettingsForAnalysisModelCombination();
                enableSetDefaultsButton();
            }
        });

        _spatialAdjustmentsSpatialStratifiedRadioButton.setText("Nonparametric, with spatial stratified randomization"); // NOI18N
        _spatialAdjustmentsSpatialStratifiedRadioButton.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        _spatialAdjustmentsSpatialStratifiedRadioButton.setMargin(new java.awt.Insets(0, 0, 0, 0));
        _spatialAdjustmentsSpatialStratifiedRadioButton.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                enableSetDefaultsButton();
                EnableSettingsForAnalysisModelCombination();
            }
        });

        javax.swing.GroupLayout _spatialAdjustmentsGroupLayout = new javax.swing.GroupLayout(_spatialAdjustmentsGroup);
        _spatialAdjustmentsGroup.setLayout(_spatialAdjustmentsGroupLayout);
        _spatialAdjustmentsGroupLayout.setHorizontalGroup(
            _spatialAdjustmentsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_spatialAdjustmentsGroupLayout.createSequentialGroup()
                .addContainerGap()
                .addGroup(_spatialAdjustmentsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(_spatialAdjustmentsNoneRadioButton, javax.swing.GroupLayout.DEFAULT_SIZE, 535, Short.MAX_VALUE)
                    .addComponent(_spatialAdjustmentsSpatialStratifiedRadioButton, javax.swing.GroupLayout.DEFAULT_SIZE, 535, Short.MAX_VALUE))
                .addContainerGap())
        );
        _spatialAdjustmentsGroupLayout.setVerticalGroup(
            _spatialAdjustmentsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_spatialAdjustmentsGroupLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(_spatialAdjustmentsNoneRadioButton)
                .addGap(10, 10, 10)
                .addComponent(_spatialAdjustmentsSpatialStratifiedRadioButton)
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );

        _knownAdjustmentsGroup.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createEtchedBorder(), "Temporal, Spatial and/or Space-Time Adjustments", javax.swing.border.TitledBorder.DEFAULT_JUSTIFICATION, javax.swing.border.TitledBorder.DEFAULT_POSITION, new java.awt.Font("Tahoma", 0, 11), new java.awt.Color(0, 0, 0)));

        _adjustForKnownRelativeRisksCheckBox.setText("Adjust for known relative risks"); // NOI18N
        _adjustForKnownRelativeRisksCheckBox.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        _adjustForKnownRelativeRisksCheckBox.setMargin(new java.awt.Insets(0, 0, 0, 0));
        _adjustForKnownRelativeRisksCheckBox.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                boolean  bEnabled = _knownAdjustmentsGroup.isEnabled() && e.getStateChange() == java.awt.event.ItemEvent.SELECTED;

                _adjustmentsByRelativeRisksFileLabel.setEnabled(bEnabled);
                _adjustmentsByRelativeRisksFileTextField.setEnabled(bEnabled);
                _adjustmentsFileBrowseButton.setEnabled(bEnabled);
                _adjustmentsFileImportButton.setEnabled(bEnabled);
                enableSetDefaultsButton();
            }
        });

        _adjustmentsFileBrowseButton.setText("..."); // NOI18N
        _adjustmentsFileBrowseButton.setToolTipText("Browse for adjustments file ..."); // NOI18N
        _adjustmentsFileBrowseButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                JFileChooser fc = new JFileChooser(SaTScanApplication.getInstance().lastBrowseDirectory);
                fc.setDialogTitle("Select Adjustments File");
                fc.addChoosableFileFilter(new InputFileFilter("txt","Text Files (*.txt)"));
                fc.addChoosableFileFilter(new InputFileFilter("adj","Adjustments Files (*.adj)"));
                int returnVal = fc.showOpenDialog(AdvancedParameterSettingsFrame.this);
                if (returnVal == JFileChooser.APPROVE_OPTION) {
                    SaTScanApplication.getInstance().lastBrowseDirectory = fc.getCurrentDirectory();
                    _adjustmentsByRelativeRisksFileTextField.setText(fc.getSelectedFile().getAbsolutePath());
                }
            }
        });

        _adjustmentsFileImportButton.setText("..."); // NOI18N
        _adjustmentsFileImportButton.setToolTipText("Import adjustments file ..."); // NOI18N
        _adjustmentsFileImportButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                try {
                    JFileChooser fc = new JFileChooser(SaTScanApplication.getInstance().lastBrowseDirectory);
                    fc.setDialogTitle("Select Source Adjustments Import File");
                    fc.addChoosableFileFilter(new InputFileFilter("dbf","dBase Files (*.dbf)"));
                    fc.addChoosableFileFilter(new InputFileFilter("csv","Delimited Files (*.csv)"));
                    fc.addChoosableFileFilter(new InputFileFilter("xls","Excel Files (*.xls)"));
                    fc.addChoosableFileFilter(new InputFileFilter("txt","Text Files (*.txt)"));
                    fc.addChoosableFileFilter(new InputFileFilter("adj","Adjustments Files (*.adj)"));
                    int returnVal = fc.showOpenDialog(AdvancedParameterSettingsFrame.this);
                    if (returnVal == JFileChooser.APPROVE_OPTION) {
                        SaTScanApplication.getInstance().lastBrowseDirectory = fc.getCurrentDirectory();
                        LaunchImporter(fc.getSelectedFile().getAbsolutePath(), FileImporter.InputFileType.AdjustmentsByRR);
                    }
                } catch (Throwable t) {
                    new ExceptionDialog(SaTScanApplication.getInstance(), t).setVisible(true);
                }
            }
        });

        _adjustmentsByRelativeRisksFileLabel.setText("Adjustments File:"); // NOI18N

        javax.swing.GroupLayout _knownAdjustmentsGroupLayout = new javax.swing.GroupLayout(_knownAdjustmentsGroup);
        _knownAdjustmentsGroup.setLayout(_knownAdjustmentsGroupLayout);
        _knownAdjustmentsGroupLayout.setHorizontalGroup(
            _knownAdjustmentsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_knownAdjustmentsGroupLayout.createSequentialGroup()
                .addGroup(_knownAdjustmentsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(_knownAdjustmentsGroupLayout.createSequentialGroup()
                        .addGap(27, 27, 27)
                        .addComponent(_adjustmentsByRelativeRisksFileLabel)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(_adjustmentsByRelativeRisksFileTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 307, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(_adjustmentsFileBrowseButton, javax.swing.GroupLayout.PREFERRED_SIZE, 25, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(_adjustmentsFileImportButton, javax.swing.GroupLayout.PREFERRED_SIZE, 25, javax.swing.GroupLayout.PREFERRED_SIZE))
                    .addGroup(_knownAdjustmentsGroupLayout.createSequentialGroup()
                        .addContainerGap()
                        .addComponent(_adjustForKnownRelativeRisksCheckBox, javax.swing.GroupLayout.DEFAULT_SIZE, 535, Short.MAX_VALUE)))
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
                    .addComponent(_adjustmentsFileBrowseButton)
                    .addComponent(_adjustmentsFileImportButton))
                .addContainerGap(20, Short.MAX_VALUE))
        );

        javax.swing.GroupLayout _spaceTimeAjustmentsTabLayout = new javax.swing.GroupLayout(_spaceTimeAjustmentsTab);
        _spaceTimeAjustmentsTab.setLayout(_spaceTimeAjustmentsTabLayout);
        _spaceTimeAjustmentsTabLayout.setHorizontalGroup(
            _spaceTimeAjustmentsTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_spaceTimeAjustmentsTabLayout.createSequentialGroup()
                .addContainerGap()
                .addGroup(_spaceTimeAjustmentsTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(_spatialAdjustmentsGroup, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(_temporalTrendAdjGroup, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(_knownAdjustmentsGroup, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                .addContainerGap())
        );
        _spaceTimeAjustmentsTabLayout.setVerticalGroup(
            _spaceTimeAjustmentsTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_spaceTimeAjustmentsTabLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(_temporalTrendAdjGroup, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_spatialAdjustmentsGroup, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_knownAdjustmentsGroup, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap(12, Short.MAX_VALUE))
        );

        jTabbedPane1.addTab("Space and Time Adjustments", _spaceTimeAjustmentsTab);

        _earlyTerminationGroup.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createEtchedBorder(), "Early Termination", javax.swing.border.TitledBorder.DEFAULT_JUSTIFICATION, javax.swing.border.TitledBorder.DEFAULT_POSITION, new java.awt.Font("Tahoma", 0, 11), new java.awt.Color(0, 0, 0)));

        _terminateEarlyCheckBox.setText("Terminate the analysis early for large p-values"); // NOI18N
        _terminateEarlyCheckBox.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        _terminateEarlyCheckBox.setMargin(new java.awt.Insets(0, 0, 0, 0));
        _terminateEarlyCheckBox.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                enableSetDefaultsButton();
            }
        });

        javax.swing.GroupLayout _earlyTerminationGroupLayout = new javax.swing.GroupLayout(_earlyTerminationGroup);
        _earlyTerminationGroup.setLayout(_earlyTerminationGroupLayout);
        _earlyTerminationGroupLayout.setHorizontalGroup(
            _earlyTerminationGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_earlyTerminationGroupLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(_terminateEarlyCheckBox, javax.swing.GroupLayout.DEFAULT_SIZE, 535, Short.MAX_VALUE)
                .addContainerGap())
        );
        _earlyTerminationGroupLayout.setVerticalGroup(
            _earlyTerminationGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_earlyTerminationGroupLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(_terminateEarlyCheckBox)
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );

        _prospectiveSurveillanceGroup.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createEtchedBorder(), "Prospective Surveillance", javax.swing.border.TitledBorder.DEFAULT_JUSTIFICATION, javax.swing.border.TitledBorder.DEFAULT_POSITION, new java.awt.Font("Tahoma", 0, 11), new java.awt.Color(0, 0, 0)));

        _adjustForEarlierAnalysesCheckBox.setText("Adjust for earlier analyses performed since:"); // NOI18N
        _adjustForEarlierAnalysesCheckBox.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        _adjustForEarlierAnalysesCheckBox.setMargin(new java.awt.Insets(0, 0, 0, 0));
        _adjustForEarlierAnalysesCheckBox.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                EnableSettingsForAnalysisModelCombination();
                enableSetDefaultsButton();
            }
        });

        _prospectiveStartDateYearTextField.setText("2000"); // NOI18N
        _prospectiveStartDateYearTextField.getDocument().addUndoableEditListener(new UndoableEditListener() {
            public void undoableEditHappened(UndoableEditEvent evt) {
                undo.addEdit(evt.getEdit());
            }
        });
        _prospectiveStartDateYearTextField.addKeyListener(new java.awt.event.KeyAdapter() {
            public void keyTyped(java.awt.event.KeyEvent e) {
                Utils.validatePostiveNumericKeyTyped(_prospectiveStartDateYearTextField, e, 4);
            }
        });
        _prospectiveStartDateYearTextField.addFocusListener(new java.awt.event.FocusAdapter() {
            public void focusLost(java.awt.event.FocusEvent e) {
                Utils.validateDateControlGroup(_prospectiveStartDateYearTextField, _prospectiveStartDateMonthTextField, _prospectiveStartDateDayTextField, undo);
                enableSetDefaultsButton();
            }
        });

        _prospectiveStartDateMonthTextField.setText("12"); // NOI18N
        _prospectiveStartDateMonthTextField.getDocument().addUndoableEditListener(new UndoableEditListener() {
            public void undoableEditHappened(UndoableEditEvent evt) {
                undo.addEdit(evt.getEdit());
            }
        });
        _prospectiveStartDateMonthTextField.addKeyListener(new java.awt.event.KeyAdapter() {
            public void keyTyped(java.awt.event.KeyEvent e) {
                Utils.validatePostiveNumericKeyTyped(_prospectiveStartDateMonthTextField, e, 2);
            }
        });
        _prospectiveStartDateMonthTextField.addFocusListener(new java.awt.event.FocusAdapter() {
            public void focusLost(java.awt.event.FocusEvent e) {
                Utils.validateDateControlGroup(_prospectiveStartDateYearTextField, _prospectiveStartDateMonthTextField, _prospectiveStartDateDayTextField, undo);
                enableSetDefaultsButton();
            }
        });

        _prospectiveStartDateDayTextField.setText("31"); // NOI18N
        _prospectiveStartDateDayTextField.getDocument().addUndoableEditListener(new UndoableEditListener() {
            public void undoableEditHappened(UndoableEditEvent evt) {
                undo.addEdit(evt.getEdit());
            }
        });
        _prospectiveStartDateDayTextField.addKeyListener(new java.awt.event.KeyAdapter() {
            public void keyTyped(java.awt.event.KeyEvent e) {
                Utils.validatePostiveNumericKeyTyped(_prospectiveStartDateDayTextField, e, 2);
            }
        });
        _prospectiveStartDateDayTextField.addFocusListener(new java.awt.event.FocusAdapter() {
            public void focusLost(java.awt.event.FocusEvent e) {
                Utils.validateDateControlGroup(_prospectiveStartDateYearTextField, _prospectiveStartDateMonthTextField, _prospectiveStartDateDayTextField, undo);
                enableSetDefaultsButton();
            }
        });

        _prospectiveStartYearLabel.setFont(new java.awt.Font("Tahoma", 0, 10));
        _prospectiveStartYearLabel.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        _prospectiveStartYearLabel.setText("Year"); // NOI18N

        _prospectiveStartMonthLabel.setFont(new java.awt.Font("Tahoma", 0, 10));
        _prospectiveStartMonthLabel.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        _prospectiveStartMonthLabel.setText("Month"); // NOI18N

        _prospectiveStartDayLabel.setFont(new java.awt.Font("Tahoma", 0, 10));
        _prospectiveStartDayLabel.setHorizontalAlignment(javax.swing.SwingConstants.CENTER);
        _prospectiveStartDayLabel.setText("Day"); // NOI18N

        javax.swing.GroupLayout _prospectiveSurveillanceGroupLayout = new javax.swing.GroupLayout(_prospectiveSurveillanceGroup);
        _prospectiveSurveillanceGroup.setLayout(_prospectiveSurveillanceGroupLayout);
        _prospectiveSurveillanceGroupLayout.setHorizontalGroup(
            _prospectiveSurveillanceGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_prospectiveSurveillanceGroupLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(_adjustForEarlierAnalysesCheckBox)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(_prospectiveSurveillanceGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                    .addComponent(_prospectiveStartYearLabel, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(_prospectiveStartDateYearTextField, javax.swing.GroupLayout.DEFAULT_SIZE, 40, Short.MAX_VALUE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(_prospectiveSurveillanceGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(_prospectiveStartDateMonthTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 30, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(_prospectiveStartMonthLabel))
                .addGap(2, 2, 2)
                .addGroup(_prospectiveSurveillanceGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                    .addComponent(_prospectiveStartDayLabel, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(_prospectiveStartDateDayTextField, javax.swing.GroupLayout.DEFAULT_SIZE, 30, Short.MAX_VALUE))
                .addContainerGap(204, Short.MAX_VALUE))
        );
        _prospectiveSurveillanceGroupLayout.setVerticalGroup(
            _prospectiveSurveillanceGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_prospectiveSurveillanceGroupLayout.createSequentialGroup()
                .addGroup(_prospectiveSurveillanceGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(_prospectiveStartMonthLabel)
                    .addComponent(_prospectiveStartDayLabel)
                    .addComponent(_prospectiveStartYearLabel))
                .addGap(0, 0, 0)
                .addGroup(_prospectiveSurveillanceGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(_prospectiveStartDateYearTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(_prospectiveStartDateMonthTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(_prospectiveStartDateDayTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(_adjustForEarlierAnalysesCheckBox))
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );

        _reportCriticalValuesGroup.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createEtchedBorder(), "Critical Values", javax.swing.border.TitledBorder.DEFAULT_JUSTIFICATION, javax.swing.border.TitledBorder.DEFAULT_POSITION, new java.awt.Font("Tahoma", 0, 11), new java.awt.Color(0, 0, 0)));

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
                .addComponent(_reportCriticalValuesCheckBox, javax.swing.GroupLayout.DEFAULT_SIZE, 535, Short.MAX_VALUE)
                .addContainerGap())
        );
        _reportCriticalValuesGroupLayout.setVerticalGroup(
            _reportCriticalValuesGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_reportCriticalValuesGroupLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(_reportCriticalValuesCheckBox)
                .addContainerGap(14, Short.MAX_VALUE))
        );

        _iterativeScanGroup.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createEtchedBorder(), "Iterative Scan", javax.swing.border.TitledBorder.DEFAULT_JUSTIFICATION, javax.swing.border.TitledBorder.DEFAULT_POSITION, new java.awt.Font("Tahoma", 0, 11), new java.awt.Color(0, 0, 0)));

        _performIterativeScanCheckBox.setText("Adjusting for More Likely Clusters"); // NOI18N
        _performIterativeScanCheckBox.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        _performIterativeScanCheckBox.setMargin(new java.awt.Insets(0, 0, 0, 0));
        _performIterativeScanCheckBox.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                EnableSettingsForAnalysisModelCombination();
                enableSetDefaultsButton();
            }
        });

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

        _iterativeCutoffLabel.setText("Stop when p-value greater:"); // NOI18N

        _iterativeScanCutoffTextField.setText(".05"); // NOI18N
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
                .addGroup(_iterativeScanGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(_iterativeScanGroupLayout.createSequentialGroup()
                        .addGap(27, 27, 27)
                        .addGroup(_iterativeScanGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addComponent(_maxIterativeScansLabel)
                            .addComponent(_iterativeCutoffLabel))
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addGroup(_iterativeScanGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                            .addComponent(_iterativeScanCutoffTextField)
                            .addComponent(_numIterativeScansTextField, javax.swing.GroupLayout.DEFAULT_SIZE, 59, Short.MAX_VALUE)))
                    .addGroup(_iterativeScanGroupLayout.createSequentialGroup()
                        .addContainerGap()
                        .addComponent(_performIterativeScanCheckBox, javax.swing.GroupLayout.DEFAULT_SIZE, 535, Short.MAX_VALUE)))
                .addContainerGap())
        );
        _iterativeScanGroupLayout.setVerticalGroup(
            _iterativeScanGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_iterativeScanGroupLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(_performIterativeScanCheckBox)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(_iterativeScanGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(_maxIterativeScansLabel)
                    .addComponent(_numIterativeScansTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .addGroup(_iterativeScanGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(_iterativeCutoffLabel)
                    .addComponent(_iterativeScanCutoffTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addContainerGap())
        );

        javax.swing.GroupLayout _inferenceTabLayout = new javax.swing.GroupLayout(_inferenceTab);
        _inferenceTab.setLayout(_inferenceTabLayout);
        _inferenceTabLayout.setHorizontalGroup(
            _inferenceTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_inferenceTabLayout.createSequentialGroup()
                .addContainerGap()
                .addGroup(_inferenceTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(_prospectiveSurveillanceGroup, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(_earlyTerminationGroup, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(_reportCriticalValuesGroup, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(_iterativeScanGroup, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                .addContainerGap())
        );
        _inferenceTabLayout.setVerticalGroup(
            _inferenceTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_inferenceTabLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(_earlyTerminationGroup, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_prospectiveSurveillanceGroup, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_reportCriticalValuesGroup, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_iterativeScanGroup, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap(24, Short.MAX_VALUE))
        );

        jTabbedPane1.addTab("Inference", _inferenceTab);

        _criteriaSecClustersGroup.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createEtchedBorder(), "Criteria for Reporting Secondary Clusters", javax.swing.border.TitledBorder.DEFAULT_JUSTIFICATION, javax.swing.border.TitledBorder.DEFAULT_POSITION, new java.awt.Font("Tahoma", 0, 11), new java.awt.Color(0, 0, 0)));

        _criteriaSecClustersNoGeoLapRadioButton.setSelected(true);
        _criteriaSecClustersNoGeoLapRadioButton.setText("No Geographical Overlap"); // NOI18N
        _criteriaSecClustersNoGeoLapRadioButton.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        _criteriaSecClustersNoGeoLapRadioButton.setMargin(new java.awt.Insets(0, 0, 0, 0));
        _criteriaSecClustersNoGeoLapRadioButton.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED) enableSetDefaultsButton();
            }
        });

        _criteriaSecClustersNoCentersRadioButton.setText("No Cluster Centers in Other Clusters"); // NOI18N
        _criteriaSecClustersNoCentersRadioButton.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        _criteriaSecClustersNoCentersRadioButton.setMargin(new java.awt.Insets(0, 0, 0, 0));
        _criteriaSecClustersNoCentersRadioButton.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED) enableSetDefaultsButton();
            }
        });

        _criteriaSecClustersNoCentersMoreRadioButton.setText("No Cluster Centers in More Likely Clusters"); // NOI18N
        _criteriaSecClustersNoCentersMoreRadioButton.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        _criteriaSecClustersNoCentersMoreRadioButton.setMargin(new java.awt.Insets(0, 0, 0, 0));
        _criteriaSecClustersNoCentersMoreRadioButton.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED) enableSetDefaultsButton();
            }
        });

        _criteriaSecClustersNoCentersLessRadioButton.setText("No Cluster Centers in Less Likely Clusters"); // NOI18N
        _criteriaSecClustersNoCentersLessRadioButton.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        _criteriaSecClustersNoCentersLessRadioButton.setMargin(new java.awt.Insets(0, 0, 0, 0));
        _criteriaSecClustersNoCentersLessRadioButton.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED) enableSetDefaultsButton();
            }
        });

        _criteriaSecClustersNoCentersEachRadioButton.setText("No Pairs of Centers Both in Each Others Clusters"); // NOI18N
        _criteriaSecClustersNoCentersEachRadioButton.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        _criteriaSecClustersNoCentersEachRadioButton.setMargin(new java.awt.Insets(0, 0, 0, 0));
        _criteriaSecClustersNoCentersEachRadioButton.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED) enableSetDefaultsButton();
            }
        });

        _criteriaSecClustersNoRestrictionsRadioButton.setText("No Restrictions = Most Likely Cluster for Each Grid Point"); // NOI18N
        _criteriaSecClustersNoRestrictionsRadioButton.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        _criteriaSecClustersNoRestrictionsRadioButton.setMargin(new java.awt.Insets(0, 0, 0, 0));
        _criteriaSecClustersNoRestrictionsRadioButton.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED) enableSetDefaultsButton();
            }
        });

        javax.swing.GroupLayout _criteriaSecClustersGroupLayout = new javax.swing.GroupLayout(_criteriaSecClustersGroup);
        _criteriaSecClustersGroup.setLayout(_criteriaSecClustersGroupLayout);
        _criteriaSecClustersGroupLayout.setHorizontalGroup(
            _criteriaSecClustersGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_criteriaSecClustersGroupLayout.createSequentialGroup()
                .addContainerGap()
                .addGroup(_criteriaSecClustersGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(_criteriaSecClustersNoGeoLapRadioButton)
                    .addComponent(_criteriaSecClustersNoCentersRadioButton)
                    .addComponent(_criteriaSecClustersNoCentersMoreRadioButton)
                    .addComponent(_criteriaSecClustersNoCentersLessRadioButton)
                    .addComponent(_criteriaSecClustersNoCentersEachRadioButton)
                    .addComponent(_criteriaSecClustersNoRestrictionsRadioButton))
                .addContainerGap(260, Short.MAX_VALUE))
        );
        _criteriaSecClustersGroupLayout.setVerticalGroup(
            _criteriaSecClustersGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_criteriaSecClustersGroupLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(_criteriaSecClustersNoGeoLapRadioButton)
                .addGap(10, 10, 10)
                .addComponent(_criteriaSecClustersNoCentersRadioButton)
                .addGap(10, 10, 10)
                .addComponent(_criteriaSecClustersNoCentersMoreRadioButton)
                .addGap(10, 10, 10)
                .addComponent(_criteriaSecClustersNoCentersLessRadioButton)
                .addGap(10, 10, 10)
                .addComponent(_criteriaSecClustersNoCentersEachRadioButton)
                .addGap(10, 10, 10)
                .addComponent(_criteriaSecClustersNoRestrictionsRadioButton)
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );

        _reportedSpatialOptionsGroup.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createEtchedBorder(), "Maximum Reported Spatial Cluster Size", javax.swing.border.TitledBorder.DEFAULT_JUSTIFICATION, javax.swing.border.TitledBorder.DEFAULT_POSITION, new java.awt.Font("Tahoma", 0, 11), new java.awt.Color(0, 0, 0)));

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
                Utils.validatePostiveNumericKeyTyped(_maxReportedSpatialRadiusTextField, e, 20);
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

        _maxReportedRadiusLabel.setText("kilometer radius"); // NOI18N

        javax.swing.GroupLayout _reportedSpatialOptionsGroupLayout = new javax.swing.GroupLayout(_reportedSpatialOptionsGroup);
        _reportedSpatialOptionsGroup.setLayout(_reportedSpatialOptionsGroupLayout);
        _reportedSpatialOptionsGroupLayout.setHorizontalGroup(
            _reportedSpatialOptionsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_reportedSpatialOptionsGroupLayout.createSequentialGroup()
                .addContainerGap()
                .addGroup(_reportedSpatialOptionsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(_restrictReportedClustersCheckBox)
                    .addGroup(_reportedSpatialOptionsGroupLayout.createSequentialGroup()
                        .addGap(17, 17, 17)
                        .addGroup(_reportedSpatialOptionsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addGroup(_reportedSpatialOptionsGroupLayout.createSequentialGroup()
                                .addComponent(_reportedSpatialPopulationFileCheckBox)
                                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                .addComponent(_maxReportedSpatialPercentFileTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 45, javax.swing.GroupLayout.PREFERRED_SIZE)
                                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                .addComponent(_reportedPercentageOfPopFileLabel, javax.swing.GroupLayout.DEFAULT_SIZE, 450, Short.MAX_VALUE))
                            .addGroup(_reportedSpatialOptionsGroupLayout.createSequentialGroup()
                                .addComponent(_maxReportedSpatialClusterSizeTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 45, javax.swing.GroupLayout.PREFERRED_SIZE)
                                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                .addComponent(_reportedPercentOfPopulationLabel, javax.swing.GroupLayout.DEFAULT_SIZE, 469, Short.MAX_VALUE))
                            .addGroup(_reportedSpatialOptionsGroupLayout.createSequentialGroup()
                                .addComponent(_reportedSpatialDistanceCheckBox)
                                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                .addComponent(_reportedMaxDistanceLabel)
                                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                .addComponent(_maxReportedSpatialRadiusTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 68, javax.swing.GroupLayout.PREFERRED_SIZE)
                                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                .addComponent(_maxReportedRadiusLabel)))))
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

        javax.swing.GroupLayout _clustersReportedTabLayout = new javax.swing.GroupLayout(_clustersReportedTab);
        _clustersReportedTab.setLayout(_clustersReportedTabLayout);
        _clustersReportedTabLayout.setHorizontalGroup(
            _clustersReportedTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, _clustersReportedTabLayout.createSequentialGroup()
                .addContainerGap()
                .addGroup(_clustersReportedTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING)
                    .addComponent(_reportedSpatialOptionsGroup, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(_criteriaSecClustersGroup, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                .addContainerGap())
        );
        _clustersReportedTabLayout.setVerticalGroup(
            _clustersReportedTabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_clustersReportedTabLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(_criteriaSecClustersGroup, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_reportedSpatialOptionsGroup, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );

        jTabbedPane1.addTab("Clusters Reported", _clustersReportedTab);

        _closeButton.setText("Close"); // NOI18N
        _closeButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                setVisible(false, null);
            }
        });

        _setDefaultButton.setText("Set Defaults"); // NOI18N
        _setDefaultButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                SetDefaultsClick();
            }
        });

        javax.swing.GroupLayout layout = new javax.swing.GroupLayout(getContentPane());
        getContentPane().setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(layout.createSequentialGroup()
                .addGap(411, 411, 411)
                .addComponent(_setDefaultButton)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_closeButton, javax.swing.GroupLayout.DEFAULT_SIZE, 74, Short.MAX_VALUE)
                .addContainerGap())
            .addComponent(jTabbedPane1, javax.swing.GroupLayout.DEFAULT_SIZE, 592, Short.MAX_VALUE)
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, layout.createSequentialGroup()
                .addComponent(jTabbedPane1, javax.swing.GroupLayout.PREFERRED_SIZE, 411, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED, 30, Short.MAX_VALUE)
                .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(_setDefaultButton)
                    .addComponent(_closeButton))
                .addContainerGap())
        );

        pack();
    }// </editor-fold>//GEN-END:initComponents
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JButton _addDataSetButton;
    private javax.swing.JPanel _additionalDataSetsGroup;
    private javax.swing.JCheckBox _adjustForEarlierAnalysesCheckBox;
    private javax.swing.JCheckBox _adjustForKnownRelativeRisksCheckBox;
    private javax.swing.JRadioButton _adjustmentByDataSetsRadioButton;
    private javax.swing.JLabel _adjustmentsByRelativeRisksFileLabel;
    private javax.swing.JTextField _adjustmentsByRelativeRisksFileTextField;
    private javax.swing.JButton _adjustmentsFileBrowseButton;
    private javax.swing.JButton _adjustmentsFileImportButton;
    private javax.swing.JRadioButton _allLocationsRadioButton;
    private javax.swing.JRadioButton _atLeastOneRadioButton;
    private javax.swing.JButton _caseFileBrowseButton;
    private javax.swing.JButton _caseFileImportButton;
    private javax.swing.JLabel _caseFileLabel;
    private javax.swing.JTextField _caseFileTextField;
    private javax.swing.JRadioButton _circularRadioButton;
    private javax.swing.JButton _closeButton;
    private javax.swing.JPanel _clustersReportedTab;
    private javax.swing.JButton _controlFileBrowseButton;
    private javax.swing.JButton _controlFileImportButton;
    private javax.swing.JLabel _controlFileLabel;
    private javax.swing.JTextField _controlFileTextField;
    private javax.swing.ButtonGroup _criteriaSecClustersButtonGroup;
    private javax.swing.JPanel _criteriaSecClustersGroup;
    private javax.swing.JRadioButton _criteriaSecClustersNoCentersEachRadioButton;
    private javax.swing.JRadioButton _criteriaSecClustersNoCentersLessRadioButton;
    private javax.swing.JRadioButton _criteriaSecClustersNoCentersMoreRadioButton;
    private javax.swing.JRadioButton _criteriaSecClustersNoCentersRadioButton;
    private javax.swing.JRadioButton _criteriaSecClustersNoGeoLapRadioButton;
    private javax.swing.JRadioButton _criteriaSecClustersNoRestrictionsRadioButton;
    private javax.swing.JPanel _dataCheckingTab;
    private javax.swing.JPanel _dataSetsGroup;
    private javax.swing.JLabel _distancePrefixLabel;
    private javax.swing.JPanel _earlyTerminationGroup;
    private javax.swing.JRadioButton _ellipticRadioButton;
    private javax.swing.JTextField _endRangeEndDayTextField;
    private javax.swing.JTextField _endRangeEndMonthTextField;
    private javax.swing.JTextField _endRangeEndYearTextField;
    private javax.swing.JTextField _endRangeStartDayTextField;
    private javax.swing.JTextField _endRangeStartMonthTextField;
    private javax.swing.JTextField _endRangeStartYearTextField;
    private javax.swing.JLabel _endRangeToLabel;
    private javax.swing.JLabel _endWindowRangeLabel;
    private javax.swing.JPanel _fileInputGroup;
    private javax.swing.JPanel _flexibleTemporalWindowDefinitionGroup;
    private javax.swing.ButtonGroup _geographicalCoordinatesCheckButtonGroup;
    private javax.swing.JPanel _geographicalCoordinatesCheckGroup;
    private javax.swing.JCheckBox _inclPureTempClustCheckBox;
    private javax.swing.JCheckBox _includePureSpacClustCheckBox;
    private javax.swing.JPanel _inferenceTab;
    private javax.swing.JList _inputDataSetsList;
    private javax.swing.JLabel _iterativeCutoffLabel;
    private javax.swing.JTextField _iterativeScanCutoffTextField;
    private javax.swing.JPanel _iterativeScanGroup;
    private javax.swing.JPanel _knownAdjustmentsGroup;
    private javax.swing.JLabel _logLinearLabel;
    private javax.swing.JTextField _logLinearTextField;
    private javax.swing.JButton _maxCirclePopFileBrowseButton;
    private javax.swing.JButton _maxCirclePopFileImportButton;
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
    private javax.swing.JLabel _maxTemporalTimeUnitsLabel;
    private javax.swing.JButton _metaLocationsFileBrowseButton;
    private javax.swing.JTextField _metaLocationsFileTextField;
    private javax.swing.JPanel _multipleCoordinatesTab;
    private javax.swing.JLabel _multipleDataSetPurposeLabel;
    private javax.swing.JPanel _multipleDataSetsTab;
    private javax.swing.ButtonGroup _multipleSetButtonGroup;
    private javax.swing.JPanel _multipleSetPurposeGroup;
    private javax.swing.ButtonGroup _multipleSetsSpatialCoordinatesButtonGroup;
    private javax.swing.JPanel _multipleSetsSpatialCoordinatesGroup;
    private javax.swing.JRadioButton _multivariateAdjustmentsRadioButton;
    private javax.swing.JButton _neighborsFileBrowseButton;
    private javax.swing.JTextField _neighborsFileTextField;
    private javax.swing.JComboBox _nonCompactnessPenaltyComboBox;
    private javax.swing.JLabel _nonCompactnessPenaltyLabel;
    private javax.swing.JPanel _nonEucledianNeighborsTab;
    private javax.swing.JTextField _numIterativeScansTextField;
    private javax.swing.JRadioButton _onePerLocationIdRadioButton;
    private javax.swing.JLabel _percentageOfPopFileLabel;
    private javax.swing.JLabel _percentageOfPopulationLabel;
    private javax.swing.JLabel _percentageOfStudyPeriodLabel;
    private javax.swing.JRadioButton _percentageTemporalRadioButton;
    private javax.swing.JCheckBox _performIsotonicScanCheckBox;
    private javax.swing.JCheckBox _performIterativeScanCheckBox;
    private javax.swing.JButton _populationFileBrowseButton;
    private javax.swing.JButton _populationFileImportButton;
    private javax.swing.JLabel _populationFileLabel;
    private javax.swing.JTextField _populationFileTextField;
    private javax.swing.JTextField _prospectiveStartDateDayTextField;
    private javax.swing.JTextField _prospectiveStartDateMonthTextField;
    private javax.swing.JTextField _prospectiveStartDateYearTextField;
    private javax.swing.JLabel _prospectiveStartDayLabel;
    private javax.swing.JLabel _prospectiveStartMonthLabel;
    private javax.swing.JLabel _prospectiveStartYearLabel;
    private javax.swing.JPanel _prospectiveSurveillanceGroup;
    private javax.swing.JLabel _relaxedCoordinatesLabel;
    private javax.swing.JRadioButton _relaxedCoordinatesRadioButton;
    private javax.swing.JRadioButton _relaxedStudyPeriodCheckRadioButton;
    private javax.swing.JButton _removeDataSetButton;
    private javax.swing.JCheckBox _reportCriticalValuesCheckBox;
    private javax.swing.JPanel _reportCriticalValuesGroup;
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
    private javax.swing.JRadioButton _spatialAdjustmentsNoneRadioButton;
    private javax.swing.JRadioButton _spatialAdjustmentsSpatialStratifiedRadioButton;
    private javax.swing.JCheckBox _spatialDistanceCheckBox;
    private javax.swing.JPanel _spatialOptionsGroup;
    private javax.swing.JCheckBox _spatialPopulationFileCheckBox;
    private javax.swing.ButtonGroup _spatialWindowShapeButtonGroup;
    private javax.swing.JPanel _spatialWindowShapeGroup;
    private javax.swing.JPanel _spatialWindowTab;
    private javax.swing.JPanel _specialNeighborFilesGroup;
    private javax.swing.JCheckBox _specifiyMetaLocationsFileCheckBox;
    private javax.swing.JCheckBox _specifiyNeighborsFileCheckBox;
    private javax.swing.JTextField _startRangeEndDayTextField;
    private javax.swing.JTextField _startRangeEndMonthTextField;
    private javax.swing.JTextField _startRangeEndYearTextField;
    private javax.swing.JTextField _startRangeStartDayTextField;
    private javax.swing.JTextField _startRangeStartMonthTextField;
    private javax.swing.JTextField _startRangeStartYearTextField;
    private javax.swing.JLabel _startRangeToLabel;
    private javax.swing.JLabel _startWindowRangeLabel;
    private javax.swing.JLabel _strictCoordinatesLabel;
    private javax.swing.JRadioButton _strictCoordinatesRadioButton;
    private javax.swing.JRadioButton _strictStudyPeriodCheckRadioButton;
    private javax.swing.ButtonGroup _studyPeriodCheckButtonGroup;
    private javax.swing.JPanel _studyPeriodCheckGroup;
    private javax.swing.ButtonGroup _temporalOptionsButtonGroup;
    private javax.swing.JPanel _temporalOptionsGroup;
    private javax.swing.ButtonGroup _temporalTrendAdjButtonGroup;
    private javax.swing.JPanel _temporalTrendAdjGroup;
    private javax.swing.JRadioButton _temporalTrendAdjLogLinearCalcRadioButton;
    private javax.swing.JRadioButton _temporalTrendAdjLogLinearRadioButton;
    private javax.swing.JRadioButton _temporalTrendAdjNoneRadioButton;
    private javax.swing.JRadioButton _temporalTrendAdjTimeStratifiedRadioButton;
    private javax.swing.JPanel _temporalWindowTab;
    private javax.swing.JCheckBox _terminateEarlyCheckBox;
    private javax.swing.JRadioButton _timeTemporalRadioButton;
    private javax.swing.JScrollPane jScrollPane1;
    private javax.swing.JTabbedPane jTabbedPane1;
    // End of variables declaration//GEN-END:variables
}
