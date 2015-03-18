/*
 * FileSourceWizard.java
 *
 * Created on December 12, 2007, 3:11 PM
 */
package org.satscan.gui;

import java.awt.CardLayout;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Vector;
import java.util.logging.Level;
import java.util.logging.Logger;
import java.util.prefs.Preferences;
import javax.swing.DefaultCellEditor;
import javax.swing.JComboBox;
import javax.swing.JOptionPane;
import javax.swing.JScrollBar;
import javax.swing.SwingWorker;
import javax.swing.event.HyperlinkEvent;
import javax.swing.event.HyperlinkListener;
import javax.swing.event.UndoableEditEvent;
import javax.swing.event.UndoableEditListener;
import javax.swing.table.AbstractTableModel;
import javax.swing.table.DefaultTableModel;
import javax.swing.undo.UndoManager;
import org.satscan.app.AppConstants;
import org.satscan.importer.CSVImportDataSource;
import org.satscan.importer.DBaseImportDataSource;
import org.satscan.importer.FileImporter;
import org.satscan.importer.ImportDataSource;
import org.satscan.importer.ImportException;
import org.satscan.importer.ImportVariable;
import org.satscan.app.Parameters;
import org.satscan.importer.PreviewTableModel;
import org.satscan.app.UnknownEnumException;
import org.satscan.gui.utils.AutofitTableColumns;
import org.satscan.gui.utils.FileSelectionDialog;
import org.satscan.gui.utils.InputFileFilter;
import org.satscan.importer.XLSImportDataSource;
import org.satscan.gui.utils.Utils;
import org.satscan.gui.utils.WaitCursor;
import org.satscan.gui.utils.help.HelpShow;
import org.satscan.importer.DataSourceException;
import org.satscan.importer.InputSourceSettings;
import static org.satscan.importer.InputSourceSettings.InputFileType.Case;
import org.satscan.importer.InputSourceSettings.SourceDataFileType;
import org.satscan.importer.ShapefileDataSource;
import org.satscan.utils.FileAccess;

public class FileSourceWizard extends javax.swing.JDialog implements PropertyChangeListener {

    private Preferences _prefs = Preferences.userNodeForPackage(getClass());
    private static final String _prefLastBackup = new String("import.destination");
    private final String _source_settings_cardname = "source-settings";
    private final String _file_format_cardname = "file-format";
    private final String _data_mapping_cardname = "data-mapping";
    private final String _output_settings_cardname = "output-settings";
    private final String _source_settings_buttons_cardname = "source-settings-buttons";
    private final String _file_format_buttons_cardname = "file-format-buttons";
    private final String _data_mapping_buttons_cardname = "data-mapping-buttons";
    private final String _output_settings_buttons_cardname = "output-settings-buttons";
    public static final String _unassigned_variable = "unassigned";
    private final int _dateVariableColumn = 2;
    private final int _sourceFileLineSample = 200;
    private final UndoManager undo = new UndoManager();
    private Vector<ImportVariable> _import_variables = new Vector<ImportVariable>();
    private final Parameters.ProbabilityModelType _startingModelType;
    private Parameters.CoordinatesType _coordinatesType;
    private String _showing_maincontent_cardname;
    private boolean _errorSamplingSourceFile = true;
    private PreviewTableModel _preview_table_model = null;
    private File _destinationFile = null;
    private File _suggested_import_filename;
    private final InputSourceSettings _input_source_settings;
    private boolean _executed_import=false;
    private boolean _needs_import_save=false;
    private boolean _refresh_related_settings=false;

    /** Creates new form FileSourceWizard */
    public FileSourceWizard(java.awt.Frame parent,
                              final String sourceFile,
                              final String suggested_filename,
                              InputSourceSettings inputSourceSettings,
                              Parameters.ProbabilityModelType modelType,
                              Parameters.CoordinatesType coordinatesType) {
        super(parent, true);
        setSuggestedImportName(sourceFile, suggested_filename, inputSourceSettings.getInputFileType());
        initComponents();
        _source_filename.setText(sourceFile);
        _input_source_settings = new InputSourceSettings(inputSourceSettings);
        if (!_input_source_settings.isSet())
            _input_source_settings.setSourceDataFileType(getSourceFileType(sourceFile));
        _startingModelType = modelType;
        _coordinatesType = coordinatesType;
        _progressBar.setVisible(false);
        configureForDestinationFileType();
        setShowingVariables();
        initializeVariableMappings();
        if (_input_source_settings.getSourceDataFileType() == InputSourceSettings.SourceDataFileType.Excel97_2003 ||
            _input_source_settings.getSourceDataFileType() == InputSourceSettings.SourceDataFileType.Excel) {
            // live read not implemented for Excel97_2003 files.
            _execute_import_now.setSelected(true);
            _save_import_settings.setEnabled(false);
        } else {
            _save_import_settings.setSelected(_input_source_settings.isSet());
        }
        setLocationRelativeTo(parent);
    }

    /** Causes showCard to be the active card. */
    private void bringPanelToFront(String main_cardname, String buttons_cardname) {
        ((CardLayout) _main_content_panel.getLayout()).show(_main_content_panel, main_cardname);
        _showing_maincontent_cardname = main_cardname;
        ((CardLayout) _button_cards_panel.getLayout()).show(_button_cards_panel, buttons_cardname);       
        enableNavigationButtons();
        if (executeButton.isEnabled()) {
            executeButton.requestFocus();
        } else if (nextButtonSource.isEnabled()) {
            nextButtonSource.requestFocus();
        }
    }
    
    /** Checks that required input variables are mapped to a field of the source file. */
    private boolean checkForRequiredVariables() {
        StringBuilder message = new StringBuilder();
        Vector<ImportVariable> missing = new Vector<ImportVariable>();
        VariableMappingTableModel model = (VariableMappingTableModel) _mapping_table.getModel();
        for (ImportVariable variable : _import_variables) {
            if (variable.getIsRequiredField() && 
                model.isShowing(variable.getVariableName()) && 
                (!variable.isMappedToSourceField() || ((String)model.getValueAt(variable.getVariableIndex(), 1)).equals(FileSourceWizard.this._unassigned_variable) )) {
                missing.add(variable);
            }
        }
        // Display a message indicating which variables are not mapped to an source field.
        if (missing.size() > 0) {
            message.append("For the " + getInputFileTypeString());
            message.append(", the following SaTScan Variable(s) are required\nand an Source File Variable must");
            message.append(" be selected for each before import can proceed.\n\nSaTScan Variable(s): ");
            for (int t = 0; t < missing.size(); ++t) {
                message.append(missing.get(t).getVariableName());
                if (t < missing.size() - 1) { message.append(", "); }
            }
            JOptionPane.showMessageDialog(this, message.toString(), "Note", JOptionPane.WARNING_MESSAGE);
            return true;
        }
        // Special case for adjustments file: We need to ensure that either both or neither dates are assigned.
        if (_input_source_settings.getInputFileType() == InputSourceSettings.InputFileType.AdjustmentsByRR) {
            if ((model._visible_variables.get(2).isMappedToSourceField() && !model._visible_variables.get(3).isMappedToSourceField()) ||
                (!model._visible_variables.get(2).isMappedToSourceField() && model._visible_variables.get(3).isMappedToSourceField())) {
                message.append("For the " + getInputFileTypeString());
                message.append(", the dates are required to be selected or omitted as a pair.\n");
                JOptionPane.showMessageDialog(this, message.toString(), "Note", JOptionPane.WARNING_MESSAGE);
                return true;
            }
        }
        return false;
    }
   
    /** Clears field mapping between input variables and field name choices. */
    private void clearSaTScanVariableFieldIndexes() {
        for (ImportVariable variable : _import_variables) {
            variable.setSourceFieldIndex(0);
        }
        ((VariableMappingTableModel) _mapping_table.getModel()).fireTableDataChanged();
    }

    /** Configures the combo-box which changes what variables are displayed. */
    private void configureDisplayVariablesComboBox() {
        _displayVariablesComboBox.removeAllItems();
        switch (_input_source_settings.getInputFileType()) {
            case Case:
                _displayVariablesComboBox.addItem("discrete Poisson model");
                _displayVariablesComboBox.addItem("Bernoulli model");
                _displayVariablesComboBox.addItem("space-time permutation model");
                _displayVariablesComboBox.addItem("multinomial model");
                _displayVariablesComboBox.addItem("ordinal model");
                _displayVariablesComboBox.addItem("exponential model");
                _displayVariablesComboBox.addItem("normal model");
                switch (_startingModelType) {
                    case BERNOULLI            : _displayVariablesComboBox.setSelectedIndex(1); break;
                    case SPACETIMEPERMUTATION : _displayVariablesComboBox.setSelectedIndex(2); break;
                    case CATEGORICAL          : _displayVariablesComboBox.setSelectedIndex(3); break;
                    case ORDINAL              : _displayVariablesComboBox.setSelectedIndex(4); break;
                    case EXPONENTIAL          : _displayVariablesComboBox.setSelectedIndex(5); break;
                    case NORMAL               : _displayVariablesComboBox.setSelectedIndex(6); break;
                    case POISSON              :
                    default                   : _displayVariablesComboBox.setSelectedIndex(0); break;
                } break;
            case Coordinates:
            case SpecialGrid:
                Parameters.CoordinatesType temp = _coordinatesType;
                _displayVariablesComboBox.addItem("Latitude/Longitude Coordinates");
                _displayVariablesComboBox.addItem("Cartesian (x, y) Coordinates");
                _coordinatesType = temp;
                switch (_coordinatesType) {
                    case CARTESIAN : _displayVariablesComboBox.setSelectedIndex(1); break;
                    case LATLON    :
                    default        : _displayVariablesComboBox.setSelectedIndex(0);
                } break;
            case Control: 
                _displayVariablesComboBox.addItem("Bernoulli model");
                _displayVariablesComboBox.setSelectedIndex(0);
                _displayVariablesLabel.setEnabled(false);
                _displayVariablesComboBox.setEnabled(false);
                break;
            case Population:
                _displayVariablesComboBox.addItem("discrete Poisson model");
                _displayVariablesComboBox.setSelectedIndex(0);
                _displayVariablesLabel.setEnabled(false);
                _displayVariablesComboBox.setEnabled(false);
                break;
            default: 
                _displayVariablesLabel.setEnabled(false);
                _displayVariablesComboBox.setEnabled(false);
                
        }
    }

    /** Configures the variables vector based upon the source file type. */
    private void configureForDestinationFileType() {
        switch (_input_source_settings.getInputFileType()) {
            case Case: setCaseFileVariables(); break;
            case Control: setControlFileVariables(); break;
            case Population: setPopulationFileVariables(); break;
            case Coordinates: setGeoFileVariables(); break;
            case SpecialGrid: setGridFileVariables(); break;
            case MaxCirclePopulation: setMaxCirclePopFileVariables(); break;
            case AdjustmentsByRR: setRelativeRisksFileVariables(); break;
            case Neighbors:
            case MetaLocations:
            case AlternativeHypothesis: break;
            default: throw new UnknownEnumException(_input_source_settings.getInputFileType());
        }
        configureDisplayVariablesComboBox();
    }
    
    /** Attempts to create the import file. */
    private void createDestinationInformation() throws IOException {
        try {
            _destinationFile = new File(_outputDirectoryTextField.getText());
            _destinationFile.createNewFile();
        } catch (Exception e) {
            JOptionPane.showMessageDialog(this, "Unable to write to file: '" + _outputDirectoryTextField.getText() + "'.");
        }
    }

    /** Enables navigation buttons based upon active panel and current settings. */
    private void enableNavigationButtons() {
        if (_showing_maincontent_cardname == null) {
            return;
        } else if (_showing_maincontent_cardname.equals(_source_settings_cardname)) {
            // Require a source file to be specified -- we'll verified that it exists and is readable later.
            nextButtonSource.setEnabled(!_source_filename.getText().isEmpty());
        } else if (_showing_maincontent_cardname.equals(_file_format_cardname)) {
            // Require that were able to read file sampling and necessary inputs were specified.
            nextButtonCSV.setEnabled(!_errorSamplingSourceFile && (_otherRadioButton.isSelected() ? _otherFieldSeparatorTextField.getText().length() > 0 : true));
        } else if (_showing_maincontent_cardname.equals(_data_mapping_cardname)) {
            // Require there is at least one record to import and at least one variable is mapped - we'll verify the mappings later.
            if (_source_data_table.getModel().getRowCount() > 0) {
                VariableMappingTableModel model = (VariableMappingTableModel) _mapping_table.getModel();
                for (int i=0; i < model._visible_variables.size() && !nextButtonSource.isEnabled(); ++i) {
                    nextButtonMapping.setEnabled(model._visible_variables.get(i).isMappedToSourceField());
                }
            }
        } else if (_showing_maincontent_cardname.equals(_output_settings_cardname)) {
            if (_execute_import_now.isSelected()) {
                executeButton.setEnabled(_outputDirectoryTextField.getText().length() > 0);
            } else {
                executeButton.setEnabled(true);
            }
        }
    }

    /** Returns the field delimiter specified on the csv format panel. */
    private char getColumnDelimiter() {
        if (_commaRadioButton.isSelected()) {
            return ',';
        } else if (_semiColonRadioButton.isSelected()) {
            return ';';
        } else if (_whitespaceRadioButton.isSelected()) {
            return ' ';
        } else if (_otherRadioButton.isSelected()) {
            return _otherFieldSeparatorTextField.getText().toCharArray()[0];
        } else {
            throw new RuntimeException("Unknown column delimiter.");
        }
    }    

    /** Returns the coordinates type selected by user. Only useful with Coordinate and Grid files. */
    public Parameters.CoordinatesType getCoorinatesControlType() {
        switch (_displayVariablesComboBox.getSelectedIndex()) {
            case 1: return Parameters.CoordinatesType.CARTESIAN;
            default: return Parameters.CoordinatesType.LATLON;
        }
    }
    
    /** Used to determine whether user is importing to a date field. */
    public boolean getDateFieldImported() {
        switch (_input_source_settings.getInputFileType()) {
            case Case:
            case Control: return _import_variables.get(_dateVariableColumn).isMappedToSourceField();
            case Population: return _import_variables.get(1).isMappedToSourceField();
            case Coordinates:
            case SpecialGrid:
            case MaxCirclePopulation: 
            case Neighbors:
            case MetaLocations:
            case AlternativeHypothesis: return false;
            case AdjustmentsByRR: return _import_variables.get(_dateVariableColumn).isMappedToSourceField();
            default: throw new UnknownEnumException(_input_source_settings.getInputFileType());
        }
    }
    
    /** Returns import destination filename. */
    public String getDestinationFilename() {
        return _destinationFile == null ? "" : _destinationFile.getAbsolutePath();
    }    
    
    /** Returned whether the user chose to execute import now -- creating an imported file. */
    public boolean getExecutedImport() {
        return _executed_import;
    }    
    
    /** Returned whether the user chose to execute import now -- creating an imported file. */
    public boolean needsSettingsRefresh() {
        return _refresh_related_settings;
    }      
    
    /** Builds html which details the fields expected in the input file. */
    private String getFileExpectedFormatHtml() {
        StringBuilder builder = new StringBuilder();
        builder.append("<html><head><style>th {font-weight:bold;text-align:right;}</style></head><body>");
        builder.append(getFileExpectedFormatParagraphs());
        if (isImportableFileType(_input_source_settings.getInputFileType())) {
            builder.append("<p>If the selected file is not SaTScan formatted (whitespace delimited) or fields are not in the expected order, select the 'Next' button to specify how to read this file.</p>");
        }
        builder.append("</body></html>");
        return builder.toString();
    }
    
    /** Builds html which details the expected format of the input file type. */
    private String getFileExpectedFormatParagraphs() {
        String heplID="";
        StringBuilder builder = new StringBuilder();
        builder.append("<p style=\"margin-top: 0;\">The expected format of the ");
        builder.append(FileSelectionDialog.getFileTypeAsString(_input_source_settings.getInputFileType()).toLowerCase()).append(" file");
        if (_input_source_settings.getInputFileType() == InputSourceSettings.InputFileType.Case) {
            builder.append(", using the ").append(Parameters.GetProbabilityModelTypeAsString(getModelControlType())).append(" probability model");
        } else if (_input_source_settings.getInputFileType() == InputSourceSettings.InputFileType.Coordinates ||
                   _input_source_settings.getInputFileType() == InputSourceSettings.InputFileType.SpecialGrid) {
            builder.append(", using ").append(getCoorinatesControlType() == Parameters.CoordinatesType.LATLON ? "Latitude/Longitude" : "Cartesian").append(" coordinates");
        }
        builder.append(" is:</p><span style=\"margin: 5px 0 0 5px;font-style:italic;font-weight:bold;\">");
        switch (_input_source_settings.getInputFileType()) {
            case Case :
                heplID = AppConstants.CASEFILE_HELPID;
                switch (getModelControlType()) {
                    case POISSON :
                    case SPACETIMEPERMUTATION :
                    case HOMOGENEOUSPOISSON :
                        builder.append("&lt;Location ID&gt;  &lt;Number of Cases&gt;  &lt;Date/Time&gt;  &lt;Covariate 1&gt; ... &lt;Covariate N&gt;");
                        break;
                    case BERNOULLI :
                        builder.append("&lt;Location ID&gt;  &lt;Number of Cases&gt;  &lt;Date/Time&gt;");
                        break;
                    case ORDINAL :
                    case CATEGORICAL :
                        builder.append("&lt;Location ID&gt;  &lt;Number of Cases&gt;  &lt;Date/Time&gt;  &lt;Attribute&gt;");
                        break;
                    case EXPONENTIAL :
                        builder.append("&lt;Location ID&gt;  &lt;Number of Cases&gt;  &lt;Date/Time&gt;  &lt;Attribute&gt;  &lt;Censored&gt;");
                        break;
                    case NORMAL :
                        builder.append("&lt;Location ID&gt;  &lt;Number of Cases&gt;  &lt;Date/Time&gt;  &lt;Attribute&gt;  &lt;Weight&gt;  &lt;Covariate 1&gt; ... &lt;Covariate N&gt;");
                        break;
                    case RANK :
                        builder.append("&lt;Location ID&gt;  &lt;Number of Cases&gt;  &lt;Date/Time&gt;  &lt;Continuous Variable&gt;");
                        break;
                    default: throw new UnknownEnumException(getModelControlType());
                }
                break;
            case Control:
                heplID = AppConstants.CONTROLFILE_HELPID;
                builder.append("&lt;Location ID&gt;  &lt;Controls&gt;  &lt;Date/Time&gt;");
                break;
            case Population:
                heplID = AppConstants.POPULTIONFILE_HELPID;
                heplID = "Population File";
                builder.append("&lt;Location ID&gt;  &lt;Date/Time&gt;  &lt;Population&gt;  &lt;Covariate 1&gt; ... &lt;Covariate N&gt;");
                break;
            case Coordinates:
                heplID = AppConstants.COORDINATESFILE_HELPID;
                if (getCoorinatesControlType() == Parameters.CoordinatesType.CARTESIAN)
                    builder.append("&lt;Location ID&gt;  &lt;X-Coordinate&gt;  &lt;Y-Ccoordinate&gt;  &lt;Z1-Coordinate&gt; ...  &lt;ZN-Coordinate&gt;");
                else
                    builder.append("&lt;Location ID&gt;  &lt;Latitude&gt;  &lt;Longitude&gt;");
                break;
            case SpecialGrid: builder.append("");
                heplID = AppConstants.GRIDFILE_HELPID;
                if (getCoorinatesControlType() == Parameters.CoordinatesType.CARTESIAN)
                    builder.append("&lt;X-Coordinate&gt;  &lt;Y-Coordinate&gt;  &lt;Z1-Coordinate&gt; ... &lt;ZN-Coordinate&gt;");
                else
                    builder.append("&lt;Latitude&gt;  &lt;Longitude&gt;");
                break;
            case MaxCirclePopulation:
                heplID = AppConstants.MAXCIRCLEFILE_HELPID;
                builder.append("&lt;Location ID&gt;  &lt;Population&gt;");
                break;
            case AdjustmentsByRR:
                heplID = AppConstants.ADJUSTMENTSFILE_HELPID;
                builder.append("&lt;Location ID&gt;  &lt;Relative Risk&gt;  &lt;Start Time&gt;  &lt;End Time&gt;");
                break;
            case Neighbors:
                heplID = AppConstants.NONEUCLIDIANFILE_HELPID;
                builder.append("&lt;Location ID 1&gt;  &lt;Location ID 2&gt; &lt;Location ID 3&gt; ... &lt;Location ID N&gt;");
                break;
            case MetaLocations:
                heplID = AppConstants.METALOCATIONSFILE_HELPID;
                builder.append("&lt;Meta Location ID&gt;  (&lt;Location ID&gt; or &lt;Meta Location ID&gt;) ... (&lt;Location ID&gt; or &lt;Meta Location ID&gt;)");
                break;
            case AlternativeHypothesis:
                heplID = AppConstants.ALTERNATIVEHYPOTHESIS_HELPID;
                builder.append("&lt;Location ID&gt;  &lt;Relative Risk&gt;  &lt;Start Time&gt;  &lt;End Time&gt;");
                break;
            default: throw new UnknownEnumException(_input_source_settings.getInputFileType());
        }
        builder.append("&nbsp;&nbsp;</span>");
        //builder.append("<span style=\"padding-left:20px;\">(<a style=\"font-weight:bold;color:black;font-size:smaller;\" href=\"").append(heplID).append("\">More Information</a>)</span>");
        return builder.toString();
    }
    
    /** Returns the field grouping character specified on the csv format panel. */
    private char getGroupMarker() {
        if (_doubleQuotesRadioButton.isSelected()) {
            return '"';
        } else if (_singleQuotesRadioButton.isSelected()) {
            return '\'';
        } else {
            throw new RuntimeException("Unknown group marker.");
        }
    }
    
    /** Return the ImportDataSource object -- based upon the source file type. */
    private ImportDataSource getImportSource() throws FileNotFoundException {
        if (_input_source_settings.getSourceDataFileType() == InputSourceSettings.SourceDataFileType.Shapefile) {
            return new ShapefileDataSource(new File(getSourceFilename()), true);
        } else if (_input_source_settings.getSourceDataFileType() == InputSourceSettings.SourceDataFileType.dBase) {
            return new DBaseImportDataSource(new File(getSourceFilename()), false);
        } else if (_input_source_settings.getSourceDataFileType() == InputSourceSettings.SourceDataFileType.Excel97_2003 ||
                   _input_source_settings.getSourceDataFileType() == InputSourceSettings.SourceDataFileType.Excel) {
            return new XLSImportDataSource(new File(getSourceFilename()));
        } else {
            int skipRows = Integer.parseInt(_ignoreRowsTextField.getText());
            return new CSVImportDataSource(new File(getSourceFilename()), _firstRowColumnHeadersCheckBox.isSelected(), '\n', getColumnDelimiter(), getGroupMarker(), skipRows);
        }
    }
    
    /** Returns InputSourceSettings.InputFileType as string. */
    private String getInputFileTypeString() {
        switch (_input_source_settings.getInputFileType()) {
            case Case: return "Case File";
            case Control: return "Control File";
            case Population: return "Population File";
            case Coordinates: return "Coordinates File";
            case SpecialGrid: return "Grid File";
            case MaxCirclePopulation: return "Max Circle Size File";
            case AdjustmentsByRR: return "Adjustments File";
            case Neighbors: return "Non-Euclidian Neighbors";
            case MetaLocations: return "Meta Locations";
            case AlternativeHypothesis: return "Alternative Hypothesis";
            default: throw new UnknownEnumException(_input_source_settings.getInputFileType());
        }
    }    
    
    /** Returns the base collection of supported file filters. */
    public static ArrayList<InputFileFilter> getInputFilters() {
        // define file filters supported by import wizard
        ArrayList<InputFileFilter> filters = new ArrayList<InputFileFilter>();
        filters.add(new InputFileFilter("csv","Delimited Files (*.csv)"));
        filters.add(new InputFileFilter("xlsx","Excel Files (*.xlsx)"));
        filters.add(new InputFileFilter("xls","Excel 97-2003 Files (*.xls)"));
        filters.add(new InputFileFilter("dbf","dBase Files (*.dbf)"));
        filters.add(new InputFileFilter("txt","Text Files (*.txt)"));
        filters.add(new InputFileFilter("shp","Shape Files (*.shp)"));
        return filters;
    }
    
    /** Returns InputSourceSettings object from user selections in wizard. */
    public final InputSourceSettings getInputSourceSettings() {
        return _input_source_settings;
    }
    
    /** Builds html which details the input source type and variable mappings to source file'variableIdx columns. */
    private String getMappingsHtml() {
        Vector<String> columnNames = getSourceColumnNames();
        StringBuilder builder = new StringBuilder();
        builder.append("<html><head><style>th {font-weight: bold;text-align:right;}</style></head><body>");
        builder.append(getFileExpectedFormatParagraphs());
        builder.append("<p style=\"margin-top: 10px;\"> The ").append(FileSelectionDialog.getFileTypeAsString(_input_source_settings.getInputFileType()).toLowerCase());
        builder.append(" data will be read from the specified file according to the following settings:</p>");
        builder.append("<div style=\"margin-top:5px;\"><table><tr><td valign=\"top\"><table >");
        switch (_input_source_settings.getSourceDataFileType()) {
            case CSV :
                builder.append("<tr><th style=\"white-space:nowrap;\">File Type:</th><td>CSV</td></tr>");
                builder.append("<tr><th>Delimiter:</th><td>");
                if (_input_source_settings.getDelimiter().equals("") || _input_source_settings.getDelimiter().equals(" ")) {
                    builder.append("Whitespace");
                } else if (_input_source_settings.getDelimiter().equals(",")) {
                    builder.append("Comma");
                } else if (_input_source_settings.getDelimiter().equals(";")) {
                    builder.append("Semicolon");
                } else {
                    builder.append(_input_source_settings.getDelimiter());
                }
                builder.append("</td></tr><tr><th style=\"white-space:nowrap;\">Group By:</th><td style=\"white-space:nowrap;\">");
                if (_input_source_settings.getGroup().equals("\"")) {
                    builder.append("Double Quote");
                } else if (_input_source_settings.getGroup().equals("'")) {
                    builder.append("Single Quote");
                } else {
                    builder.append(_input_source_settings.getGroup());
                }
                builder.append("</td></tr><tr><th style=\"white-space:nowrap;\">Lines Skipped:</th><td>" + _input_source_settings.getSkiplines() + "</td></tr>");
                builder.append("<tr><th style=\"white-space:nowrap;\">Column Header:</th><td>" + (_input_source_settings.getFirstRowHeader() ? "Yes" : "No") + "</td></tr>");
                break;
            case dBase : builder.append("<tr><th style=\"white-space:nowrap;\">File Type:</th><td>dBase</td></tr>"); break;
            case Excel97_2003 : builder.append("<tr><th style=\"white-space:nowrap;\">File Type:</th><td>Excel 97-2003</td></tr>"); break;
            case Excel : builder.append("<tr><th style=\"white-space:nowrap;\">File Type:</th><td>Excel</td></tr>"); break;
            case Shapefile : builder.append("<tr><th style=\"white-space:nowrap;\">File Type:</th><td>Shapefile</td></tr>"); break;
        }
        builder.append("</table></td>");
        builder.append("<td valign=\"top\"><table><tr><th valign=\"top\" style=\"white-space:nowrap;\">Field Mapping:</th><td><table>");
        for (int i=0; i < _input_source_settings.getFieldMaps().size(); ++i) {
            ImportVariable variable = getShowingImportVariableAt(i);
            if (variable != null && variable.getSourceFieldIndex() > 0) {
                builder.append("<tr><td>" + variable.getDisplayLabel() + "</td><td> &#8667;</td> <td>");
                int col_idx = Integer.parseInt(_input_source_settings.getFieldMaps().get(i));
                if (_input_source_settings.getFieldMaps().get(i).isEmpty() || col_idx < 1) {
                    builder.append("---");
                } else {
                    if (col_idx <= columnNames.size()) {
                      String name = columnNames.elementAt(col_idx - 1);
                      if (name.length() > 15) {
                          name = name.substring(0, 14) + " ...";
                      }
                      builder.append(name);
                    } else {
                        builder.append("---");
                    }
                }
                builder.append("</td></tr>");
            }
        }
        builder.append("</table></td></tr></table></td></tr></table></div></body></html>");
        return builder.toString();
    }

    /** Returns the probability mapping_model type selected by user. */
    public Parameters.ProbabilityModelType getModelControlType() {
        switch (_displayVariablesComboBox.getSelectedIndex()) {
            case 1: return Parameters.ProbabilityModelType.BERNOULLI;
            case 2: return Parameters.ProbabilityModelType.SPACETIMEPERMUTATION;
            case 3: return Parameters.ProbabilityModelType.CATEGORICAL;
            case 4: return Parameters.ProbabilityModelType.ORDINAL;
            case 5: return Parameters.ProbabilityModelType.EXPONENTIAL;
            case 6: return Parameters.ProbabilityModelType.NORMAL;
            case 0:
            default: return Parameters.ProbabilityModelType.POISSON;
        }
    }    
    
    /** Returned whether the user chose execute import later. */
    public boolean getNeedsImportSourceSave() {
        return _needs_import_save;
    }    
    
    /** Returns the first showing import variable with a target field index == idx. */
    private ImportVariable getShowingImportVariableAt(int idx) {
        for (ImportVariable variable : _import_variables) {
            if (variable.getShowing() && variable.getVariableIndex() == idx)
                return variable;
        } return null;
    }    
    
    /** Attempts to obtain the column names for the source file given InputSourceSettings. */
    private Vector<String> getSourceColumnNames() {
        Vector<String> column_names = new Vector<String>();
        try {
            File file = new File(getSourceFilename());
            if (file.exists()) {
                Object[] names=null;
                if (_input_source_settings.getSourceDataFileType() == InputSourceSettings.SourceDataFileType.Shapefile) {
                    ShapefileDataSource source = new ShapefileDataSource(file, true);
                    names = source.getColumnNames();
                } else if (_input_source_settings.getSourceDataFileType() == InputSourceSettings.SourceDataFileType.dBase) {
                    DBaseImportDataSource source = new DBaseImportDataSource(file, true);
                    names = source.getColumnNames();
                } else if (_input_source_settings.getSourceDataFileType() == InputSourceSettings.SourceDataFileType.CSV) {
                    CSVImportDataSource source = new CSVImportDataSource(file, _input_source_settings.getFirstRowHeader(), '\n', _input_source_settings.getDelimiter().charAt(0), _input_source_settings.getGroup().charAt(0), _input_source_settings.getSkiplines());
                    names = source.getColumnNames();
                } else if (_input_source_settings.getSourceDataFileType() == InputSourceSettings.SourceDataFileType.Excel97_2003 ||
                           _input_source_settings.getSourceDataFileType() == InputSourceSettings.SourceDataFileType.Excel) {
                    XLSImportDataSource source = new XLSImportDataSource(file);
                    names = source.getColumnNames();
                }
                
                for (int i=0; names != null && i < names.length; ++i) {
                    column_names.add((String)names[i]);
                }
            }
        } catch (Exception ex){
            Logger.getLogger(SaTScanApplication.class.getName()).log(Level.SEVERE, null, ex);
        }
        return column_names;
    }
    
    /** Returns source file type given source file extension. */
    static public InputSourceSettings.SourceDataFileType getSourceFileType(final String filename) {
        int pos = filename.lastIndexOf('.');
        if (pos != -1 && filename.substring(pos + 1).equalsIgnoreCase("shp")) {
            return InputSourceSettings.SourceDataFileType.Shapefile;
        } else if (pos != -1 && filename.substring(pos + 1).equalsIgnoreCase("dbf")) {
            return InputSourceSettings.SourceDataFileType.dBase;
        } else if (pos != -1 && filename.substring(pos + 1).equalsIgnoreCase("xls")) {
            return InputSourceSettings.SourceDataFileType.Excel97_2003;
        } else if (pos != -1 && filename.substring(pos + 1).equalsIgnoreCase("xlsx")) {
            return InputSourceSettings.SourceDataFileType.Excel;
        }        
        return InputSourceSettings.SourceDataFileType.CSV;
    }
    
    /** Returns source filename. */
    public String getSourceFilename() {
        return _source_filename.getText();
    }
    
    /** Sets initial import variable mappings from input source. */
    private void initializeVariableMappings() {
        for (int variableIdx=0; variableIdx < _input_source_settings.getFieldMaps().size(); ++variableIdx) {
            String fieldStr = _input_source_settings.getFieldMaps().get(variableIdx).trim();
            if (!fieldStr.isEmpty()) {
                int source_index = Integer.parseInt(fieldStr);
                for (ImportVariable variable : _import_variables) {
                    if (variable.getVariableIndex() == variableIdx) {
                        variable.setSourceFieldIndex(source_index);
                    }
                }
            }
        }
    }

    /** Returns whether InputSourceSettings.InputFileType can be imported. */
    public static boolean isImportableFileType(InputSourceSettings.InputFileType fileType) {
        switch (fileType) {
            case Case:
            case Control:
            case Population:
            case Coordinates:
            case SpecialGrid:
            case MaxCirclePopulation:
            case AdjustmentsByRR: return true;
            case Neighbors:
            case MetaLocations:
            case AlternativeHypothesis: return false;
            default: throw new UnknownEnumException(fileType);
        }
    }
    
    /** Calls appropriate preparation methods then shows panel. */
    private void makeActivePanel(String targetCardName) throws Exception {
        if (targetCardName.equals(_source_settings_cardname)) {
            prepFileSourceOptionsPanel();
            bringPanelToFront(targetCardName, _source_settings_buttons_cardname);
        } else if (targetCardName.equals(_file_format_cardname)) {
            prepFileFormatPanel();
            bringPanelToFront(targetCardName, _file_format_buttons_cardname);
        } else if (targetCardName.equals(_data_mapping_cardname)) {
            prepMappingPanel();
            bringPanelToFront(targetCardName, _data_mapping_buttons_cardname);
        } else if (targetCardName.equals(_output_settings_cardname)) {
            prepOutputSettingsPanel();
            bringPanelToFront(targetCardName, _output_settings_buttons_cardname);
        }        
    }
    
    /** Attempts to advance wizard to the next panel. */
    private void nextPanel() {
        try {
            if (_showing_maincontent_cardname == _source_settings_cardname) {
                /* The user might have changed the filename on the File Options panel.
                 * We need to check the SourceDataFileType for specified filename. */
                if (FileAccess.ValidateFileAccess(getSourceFilename(), false)) {
                    _input_source_settings.setSourceDataFileType(getSourceFileType(getSourceFilename()));
                    if (_input_source_settings.getSourceDataFileType() == SourceDataFileType.CSV) {
                        makeActivePanel(_file_format_cardname);
                    } else {
                        makeActivePanel(_data_mapping_cardname);
                    }
                } else {
                    JOptionPane.showMessageDialog(this, getSourceFilename() + " could not be opened for reading.", "Note", JOptionPane.INFORMATION_MESSAGE);
                }
            } else if (_showing_maincontent_cardname == _file_format_cardname) {
                makeActivePanel(_data_mapping_cardname);
            } else if (_showing_maincontent_cardname == _data_mapping_cardname) {
                if (checkForRequiredVariables()) return;
                makeActivePanel(_output_settings_cardname);
            }
        } catch (org.satscan.importer.ImportDataSource.UnsupportedException e) {
            JOptionPane.showMessageDialog(this, e.getMessage(), "Note", JOptionPane.INFORMATION_MESSAGE);    
        } catch (Throwable t) {
            new ExceptionDialog(FileSourceWizard.this, t).setVisible(true);
        }
    }
    
    /** Preparation for viewing file format panel. */
    private void prepFileFormatPanel() {
        WaitCursor waitCursor = new WaitCursor(this);
        try {
            readDataFileIntoRawDisplayField();
            if (_input_source_settings.isSet()) {
                // define settings given input source settings
                _ignoreRowsTextField.setText(Integer.toString(_input_source_settings.getSkiplines()));
                _firstRowColumnHeadersCheckBox.setSelected(_input_source_settings.getFirstRowHeader());
                if (_input_source_settings.getDelimiter().isEmpty() || _input_source_settings.getDelimiter().equalsIgnoreCase(" ")) {
                    _whitespaceRadioButton.setSelected(true);
                } else if (_input_source_settings.getDelimiter().equalsIgnoreCase(",")) {
                    _commaRadioButton.setSelected(true);
                } else if (_input_source_settings.getDelimiter().equalsIgnoreCase(";")) {
                    _semiColonRadioButton.setSelected(true);
                } else {
                    _otherRadioButton.setSelected(true);
                    _otherFieldSeparatorTextField.setText("" + _input_source_settings.getDelimiter().charAt(0));
                }
                if (_input_source_settings.getGroup().equalsIgnoreCase("'")) {
                    _singleQuotesRadioButton.setSelected(true);
                } else {
                    _doubleQuotesRadioButton.setSelected(true);
                }
            }
            enableNavigationButtons();
        } finally {
            waitCursor.restore();
        }
    }
    
    /** Preparation for viewing of the file source settings options panel. */
    private void prepFileSourceOptionsPanel() {
        WaitCursor waitCursor = new WaitCursor(this);
        try {
            // set the file input fields label for file type
            _file_selection_label.setText(FileSelectionDialog.getFileTypeAsString(_input_source_settings.getInputFileType()) + " File:");
            if (!_input_source_settings.isSet()) {
                _expectedFormatTextPane.setText(getFileExpectedFormatHtml());
                _expectedFormatTextPane.setCaretPosition(0);
                nextButtonSource.setText("Next >");
            } else {
                _expectedFormatTextPane.setText(getMappingsHtml());
                _expectedFormatTextPane.setCaretPosition(0);
                nextButtonSource.setText("Update >");
            }
            for (HyperlinkListener listener :  _expectedFormatTextPane.getHyperlinkListeners()) {
                _expectedFormatTextPane.removeHyperlinkListener(listener);
            }
            _expectedFormatTextPane.addHyperlinkListener(new HyperlinkListener() {
                public void hyperlinkUpdate(HyperlinkEvent e) {
                    if(e.getEventType() == HyperlinkEvent.EventType.ACTIVATED) {
                        HelpShow.showHelp(e.getDescription());
                    }
                }
            });
            clearInputSettigs.setEnabled(_input_source_settings.isSet());
        } finally {
            waitCursor.restore();
        }
    }
    
    /** Preparation for viewing the mapping panel. */
    private void prepMappingPanel() throws Exception {
        WaitCursor waitCursor = new WaitCursor(this);
        try {
            previewSource();
            // clear any variables that have source field mappings which are greater than the number of choices
            for (ImportVariable variable: _import_variables) {
                if (variable.getSourceFieldIndex() >  _preview_table_model.getDataSourceColumnNameCount())
                    variable.setSourceFieldIndex(-1);
            }
            setMappingTableComboCells();
            setInputSourceSettings();
            enableNavigationButtons();
        } finally {
            waitCursor.restore();
        }
    }
    
    /** Preparation for viewing the output settings panel. */
    private void prepOutputSettingsPanel() {
        if (_input_source_settings.getSourceDataFileType() == InputSourceSettings.SourceDataFileType.Excel97_2003 ||
            _input_source_settings.getSourceDataFileType() == InputSourceSettings.SourceDataFileType.Excel) {
            // We don't have code yet that allows reading from Excel97_2003 in the C++ code (calculation engine).
            _execute_import_now.setSelected(true);
            _save_import_settings.setEnabled(false);
        }
        _source_data_table.setModel(new DefaultTableModel());
        setInputSourceSettings();
    }
    
    /** Opening source as specified by file type. */
    private void previewSource() throws Exception {
        //set the import tables mapping_model to default until we have an instance of the native mapping_model avaiable
        _source_data_table.setModel(new DefaultTableModel());

        boolean show_generatedId=false;
        boolean show_oneCount=false;
        switch (_input_source_settings.getInputFileType()) {
            case Case: show_generatedId=true; show_oneCount=true; break;
            case Control: show_generatedId=true; show_oneCount=true; break;
            case Population: show_generatedId=true; show_oneCount=false; break;
            case Coordinates: show_generatedId=true; show_oneCount=false; break;
            case SpecialGrid: show_generatedId=false; show_oneCount=false; break;
            case MaxCirclePopulation: show_generatedId=true; show_oneCount=false; break;
            case AdjustmentsByRR: show_generatedId=true; show_oneCount=false; break;
            case Neighbors: show_generatedId=true; show_oneCount=false; break;
            case MetaLocations: show_generatedId=false; show_oneCount=false; break;
            case AlternativeHypothesis: show_generatedId=true; show_oneCount=false; break;
            default: throw new UnknownEnumException(_input_source_settings.getInputFileType());
        }        
        //create the table mapping_model
        File file = new File(getSourceFilename());
        if (file.exists()) {
            if (_input_source_settings.getSourceDataFileType() == InputSourceSettings.SourceDataFileType.Shapefile) {
                String supportedType = ShapefileDataSource.isSupportedShapeType(getSourceFilename());
                if (supportedType.length() > 0) {
                    throw new DataSourceException(supportedType);
                }            
                _preview_table_model = new PreviewTableModel(new ShapefileDataSource(file, true), show_generatedId, show_oneCount);
            } else if (_input_source_settings.getSourceDataFileType() == InputSourceSettings.SourceDataFileType.dBase) {
                _preview_table_model = new PreviewTableModel(new DBaseImportDataSource(new File(getSourceFilename()), true), show_generatedId, show_oneCount);
            } else if (_input_source_settings.getSourceDataFileType() == InputSourceSettings.SourceDataFileType.Excel97_2003 ||
                       _input_source_settings.getSourceDataFileType() == InputSourceSettings.SourceDataFileType.Excel) {
                _preview_table_model = new PreviewTableModel(new XLSImportDataSource(new File(getSourceFilename())), show_generatedId, show_oneCount);
            } else {
                int skipRows = Integer.parseInt(_ignoreRowsTextField.getText());
                _preview_table_model = new PreviewTableModel(new CSVImportDataSource(file, _firstRowColumnHeadersCheckBox.isSelected(), '\n', getColumnDelimiter(), getGroupMarker(), skipRows), show_generatedId, show_oneCount);
            }
        }
        //now assign mapping_model to table object
        if (_preview_table_model != null) {
            _source_data_table.setModel(_preview_table_model);
        }

        int widthTotal = 0;
        //calculate the column widths to fit header/data
        Vector<Integer> colWidths = new Vector<Integer>();
        for (int c=0; c < _source_data_table.getColumnCount(); ++c) {
            colWidths.add(AutofitTableColumns.getMaxColumnWidth(_source_data_table, c, true, 20));
            widthTotal += colWidths.lastElement();
        }
        int additional = Math.max(0, _importTableScrollPane.getViewport().getSize().width - widthTotal - 20/*scrollbar width?*/)/colWidths.size();
        for (int c=0; c < colWidths.size(); ++c) {
            _source_data_table.getColumnModel().getColumn(c).setMinWidth(colWidths.elementAt(c) + additional);
        }
    }
    
    /** Attempts to reverse wizard to the previous panel. */    
    public void previousPanel() {
        try {
            if (_showing_maincontent_cardname == _file_format_cardname) {
               makeActivePanel(_source_settings_cardname);
            } else if (_showing_maincontent_cardname == _data_mapping_cardname) {
                switch (_input_source_settings.getSourceDataFileType()) {
                    case CSV: makeActivePanel(_file_format_cardname); break;
                    default : makeActivePanel(_source_settings_cardname);
                }
            } else if (_showing_maincontent_cardname == _output_settings_cardname) {
                makeActivePanel(_data_mapping_cardname);
            }
        } catch (Throwable t) {
            new ExceptionDialog(FileSourceWizard.this, t).setVisible(true);
        }
    }
    
    /**Invoked when task'variableIdx progress property changes. */
    public void propertyChange(PropertyChangeEvent evt) {
        if (evt.getNewValue() instanceof Integer) {
            _progressBar.setValue(((Integer) evt.getNewValue()).intValue());
        }
    }

    /** Reads in a sample of data csv file into a memo field to help user to determine structure of source file. */
    private void readDataFileIntoRawDisplayField() {
        _fileContentsTextArea.setText("");
        _errorSamplingSourceFile = false;

        //Attempt to open source file reader...
        FileReader fileSample = null;
        try {
            fileSample = new FileReader(getSourceFilename());
        } catch (FileNotFoundException e) {
            _fileContentsTextArea.append("* Unable to view source file. *");
            _fileContentsTextArea.append("* File does not exist. *");
            _errorSamplingSourceFile = true;
        } catch (SecurityException e) {
            _fileContentsTextArea.append("* Unable to view source file. *");
            _fileContentsTextArea.append("* File permissions deny read access. *");
            _errorSamplingSourceFile = true;
        }
        //Attempt to read the first X lines of file and add to sample text area
        try {
            BufferedReader buffer = new BufferedReader(fileSample);
            for (int i = 0; i < _sourceFileLineSample; ++i) {
                String line = null;
                line = buffer.readLine();
                if (line == null) {
                    break;
                }
                _fileContentsTextArea.append(line + "\n");
            }
            buffer.close();
            fileSample.close();
        } catch (IOException e) {
            _fileContentsTextArea.removeAll();
            _fileContentsTextArea.append("* Unable to view source file. *");
            _errorSamplingSourceFile = true;
        }
        //Indicate whether the source file had any data...
        if (_fileContentsTextArea.getLineCount() == 0) {
            _fileContentsTextArea.append("* Source file contains no data. *");
            _errorSamplingSourceFile = true;
        } else {
            _errorSamplingSourceFile = false;
        }
        JScrollBar vbar = jScrollPane1.getVerticalScrollBar();
        vbar.setValue(vbar.getMinimum());
        _fileContentsTextArea.setCaretPosition(0);
    }
    
    /** Setup field descriptors for case file. */
    private void setCaseFileVariables() {
        _import_variables.clear();
        _import_variables.addElement(new ImportVariable("Location ID", 0, true, null, null));
        _import_variables.addElement(new ImportVariable("Number of Cases", 1, true, null, null));
        _import_variables.addElement(new ImportVariable("Date/Time", 2, false, null, null));
        _import_variables.addElement(new ImportVariable("Attribute (value)", 3, true, null, null));
        _import_variables.addElement(new ImportVariable("Censored", 4, false, null, null));
        _import_variables.addElement(new ImportVariable("Weight", 4, false, null, null));
        _import_variables.addElement(new ImportVariable("Covariate1", 5, false, null, null));
        _import_variables.addElement(new ImportVariable("Covariate2", 6, false, null, null));
        _import_variables.addElement(new ImportVariable("Covariate3", 7, false, null, null));
        _import_variables.addElement(new ImportVariable("Covariate4", 8, false, null, null));
        _import_variables.addElement(new ImportVariable("Covariate5", 9, false, null, null));
        _import_variables.addElement(new ImportVariable("Covariate6", 10, false, null, null));
        _import_variables.addElement(new ImportVariable("Covariate7", 11, false, null, null));
        _import_variables.addElement(new ImportVariable("Covariate8", 12, false, null, null));
        _import_variables.addElement(new ImportVariable("Covariate9", 13, false, null, null));
        _import_variables.addElement(new ImportVariable("Covariate10", 14, false, null, null));
    }
    
    /** Setup field descriptors for control file. */
    private void setControlFileVariables() {
        _import_variables.clear();
        _import_variables.addElement(new ImportVariable("Location ID", 0, true, null, null));
        _import_variables.addElement(new ImportVariable("Number of Controls", 1, true, null, null));
        _import_variables.addElement(new ImportVariable("Date/Time", 2, false, null, null));
    }
    
    /** Setup field descriptors for coordinates file. */
    private void setGeoFileVariables() {
        _import_variables.clear();
        _import_variables.addElement(new ImportVariable("Location ID", 0, true, null, null));
        _import_variables.addElement(new ImportVariable("Latitude", 1, true, "y-axis", null));
        _import_variables.addElement(new ImportVariable("Longitude", 2, true, "x-axis", null));
        _import_variables.addElement(new ImportVariable("X", 1, true, null, null));
        _import_variables.addElement(new ImportVariable("Y", 2, true, null, null));
        _import_variables.addElement(new ImportVariable("Z1", 3, false, null, null));
        _import_variables.addElement(new ImportVariable("Z2", 4, false, null, null));
        _import_variables.addElement(new ImportVariable("Z3", 5, false, null, null));
        _import_variables.addElement(new ImportVariable("Z4", 6, false, null, null));
        _import_variables.addElement(new ImportVariable("Z5", 7, false, null, null));
        _import_variables.addElement(new ImportVariable("Z6", 8, false, null, null));
        _import_variables.addElement(new ImportVariable("Z7", 9, false, null, null));
        _import_variables.addElement(new ImportVariable("Z8", 10, false, null, null));
    }
    
    /** Setup field descriptors for special grid file. */
    private void setGridFileVariables() {
        _import_variables.clear();
        _import_variables.addElement(new ImportVariable("Latitude", 0, true, "y-axis", null));
        _import_variables.addElement(new ImportVariable("Longitude", 1, true, "x-axis", null));
        _import_variables.addElement(new ImportVariable("X", 0, true, null, null));
        _import_variables.addElement(new ImportVariable("Y", 1, true, null, null));
        _import_variables.addElement(new ImportVariable("Z1", 2, false, null, null));
        _import_variables.addElement(new ImportVariable("Z2", 3, false, null, null));
        _import_variables.addElement(new ImportVariable("Z3", 4, false, null, null));
        _import_variables.addElement(new ImportVariable("Z4", 5, false, null, null));
        _import_variables.addElement(new ImportVariable("Z5", 6, false, null, null));
        _import_variables.addElement(new ImportVariable("Z6", 7, false, null, null));
        _import_variables.addElement(new ImportVariable("Z7", 8, false, null, null));
        _import_variables.addElement(new ImportVariable("Z8", 9, false, null, null));
    }
    
    /** Sets InputSourceSettings object from user selections in wizard. */
    private void setInputSourceSettings() {
        if (_input_source_settings.getSourceDataFileType() == InputSourceSettings.SourceDataFileType.CSV) {
            // update CSV options
            _input_source_settings.setDelimiter(Character.toString(getColumnDelimiter()));
            _input_source_settings.setGroup(Character.toString(getGroupMarker()));
            _input_source_settings.setSkiplines(Integer.parseInt(_ignoreRowsTextField.getText()));
            _input_source_settings.setFirstRowHeader(_firstRowColumnHeadersCheckBox.isSelected());
        }
        // update variable to source field mappings
        _input_source_settings.getFieldMaps().clear();
        for (ImportVariable variable: _import_variables) {
            if (variable.getShowing()) {
                if (variable.isMappedToSourceField() || variable.hasDefault()) {
                    _input_source_settings.getFieldMaps().add(Integer.toString(variable.getSourceFieldIndex()));
                } else {
                    _input_source_settings.getFieldMaps().add("");
                }
            }
        }
        // remove any mappings that are blank -- from end to first none blank
        for (int i=_input_source_settings.getFieldMaps().size() - 1; i >= 0; --i) {
            if (!_input_source_settings.getFieldMaps().get(i).isEmpty()) break;
            else _input_source_settings.getFieldMaps().remove(i);
        }
    }
    
    /** Assigns the field name choices in the drop-down menu of the variable mapping table. */
    private void setMappingTableComboCells() {
        VariableMappingTableModel mapping_model = (VariableMappingTableModel) _mapping_table.getModel();
        // assign the combo-box choices from column names of source data table
        mapping_model._combo_box.removeAllItems();
        mapping_model._combo_box.addItem(_unassigned_variable);
        for (int i=0; i < _source_data_table.getModel().getColumnCount(); ++i) {
            mapping_model._combo_box.addItem(((PreviewTableModel)_source_data_table.getModel()).getNonSuffixedColumnName(i));
        }
        // re-assign the cell editor for the second column of mapping table
        _mapping_table.getColumnModel().getColumn(1).setCellEditor(new DefaultCellEditor(mapping_model._combo_box));
        mapping_model.fireTableDataChanged();
    }
    
    /** Setup field descriptors for maximum circle population file. */
    private void setMaxCirclePopFileVariables() {
        _import_variables.clear();
        _import_variables.addElement(new ImportVariable("Location ID", 0, true, null, null));
        _import_variables.addElement(new ImportVariable("Population", 1, true, null, null));
    }
    
    /** Setup field descriptors for population file. */
    private void setPopulationFileVariables() {
        _import_variables.clear();
        _import_variables.addElement(new ImportVariable("Location ID", 0, true, null, null));
        ImportVariable variable = new ImportVariable("Date/Time", 1, false, null, "unspecified");
        /* Set the variable index to below the one-based variables -- variables less than one
         * are considered special and are not actually a data source column option.
         * In this case, the population date will be set to 'unspecified'. */
        variable.setSourceFieldIndex(0);
        _import_variables.addElement(variable);
        _import_variables.addElement(new ImportVariable("Population", 2, true, null, null));
        _import_variables.addElement(new ImportVariable("Covariate1", 3, false, null, null));
        _import_variables.addElement(new ImportVariable("Covariate2", 4, false, null, null));
        _import_variables.addElement(new ImportVariable("Covariate3", 5, false, null, null));
        _import_variables.addElement(new ImportVariable("Covariate4", 6, false, null, null));
        _import_variables.addElement(new ImportVariable("Covariate5", 7, false, null, null));
        _import_variables.addElement(new ImportVariable("Covariate6", 8, false, null, null));
        _import_variables.addElement(new ImportVariable("Covariate7", 9, false, null, null));
        _import_variables.addElement(new ImportVariable("Covariate8", 10, false, null, null));
        _import_variables.addElement(new ImportVariable("Covariate9", 11, false, null, null));
        _import_variables.addElement(new ImportVariable("Covariate10", 12, false, null, null));
    }
    
    /** Setup field descriptors for relative risks file. */
    private void setRelativeRisksFileVariables() {
        _import_variables.clear();
        _import_variables.addElement(new ImportVariable("Location ID", 0, true, null, null));
        _import_variables.addElement(new ImportVariable("Relative Risk", 1, true, null, null));
        _import_variables.addElement(new ImportVariable("Start Date", 2, false, null, null));
        _import_variables.addElement(new ImportVariable("End Date", 3, false, null, null));
    }
    
    /** Shows/hides variables based upon destination file type and mapping_model/coordinates type. */
    private void setShowingVariables() {
        VariableMappingTableModel model = (VariableMappingTableModel) _mapping_table.getModel();
        Parameters.ProbabilityModelType _modelType = getModelControlType();
        Parameters.CoordinatesType _coordindatesType = getCoorinatesControlType();

        switch (_input_source_settings.getInputFileType()) {
            case Case:
                model.hideAll();
                for (int t = 0; t < _import_variables.size(); ++t) {
                    _import_variables.get(t).setShowing(false);
                }
                for (int t = 0; t < _import_variables.size(); ++t) {
                    if (t >= 1 && t <= 2) {//show '# cases' and 'date time'  variables
                        _import_variables.get(t).setShowing(true);
                        model.setShowing(_import_variables.get(t));
                    } else if (t >= 6 && t <= 15) {//show 'covariate' variables for Poisson,space-time permutation and normal models only
                        _import_variables.get(t).setShowing(_modelType == Parameters.ProbabilityModelType.POISSON ||
                                                           _modelType == Parameters.ProbabilityModelType.SPACETIMEPERMUTATION ||
                                                           _modelType == Parameters.ProbabilityModelType.NORMAL);
                        model.setShowing(_import_variables.get(t));
                    } else if (t == 3) { //show 'attribute' variable for ordinal, exponential, normal and rank models only
                        _import_variables.get(t).setShowing(_modelType == Parameters.ProbabilityModelType.ORDINAL ||
                                                           _modelType == Parameters.ProbabilityModelType.CATEGORICAL ||
                                                           _modelType == Parameters.ProbabilityModelType.EXPONENTIAL ||
                                                           _modelType == Parameters.ProbabilityModelType.NORMAL ||
                                                           _modelType == Parameters.ProbabilityModelType.RANK);
                        model.setShowing(_import_variables.get(t));
                    } else if (t == 4) { //show 'censored' variable for exponential mapping_model only
                        _import_variables.get(t).setShowing(_modelType == Parameters.ProbabilityModelType.EXPONENTIAL);
                        model.setShowing(_import_variables.get(t));
                    } else if (t == 5) { //show 'weight' variable for normal mapping_model only
                        _import_variables.get(t).setShowing(_modelType == Parameters.ProbabilityModelType.NORMAL);
                        model.setShowing(_import_variables.get(t));
                    } else { //default - show variable
                        _import_variables.get(t).setShowing(true);
                        model.setShowing(_import_variables.get(t));
                    }
                } break;
            case Coordinates:
                for (int t = 0; t < _import_variables.size(); ++t) {
                    if (t == 1 || t == 2) { //show 'lat/long' variables for lat/long system
                        _import_variables.get(t).setShowing(_coordindatesType == Parameters.CoordinatesType.LATLON);
                        model.setShowing(_import_variables.get(t));
                    } else if (t >= 3) { //show 'X/Y/Zn' variables for Cartesian system
                        _import_variables.get(t).setShowing(_coordindatesType == Parameters.CoordinatesType.CARTESIAN);
                        model.setShowing(_import_variables.get(t));
                    } else {
                        _import_variables.get(t).setShowing(true);
                        model.setShowing(_import_variables.get(t));
                    }
                } break;
            case SpecialGrid:
                for (int t = 0; t < _import_variables.size(); ++t) {
                    if (t == 0 || t == 1) { //show 'lat/long' variables for lat/long system
                        _import_variables.get(t).setShowing(_coordindatesType == Parameters.CoordinatesType.LATLON);
                        model.setShowing(_import_variables.get(t));
                    } else { //show 'X/Y/Zn' variables for Cartesian system
                        _import_variables.get(t).setShowing(_coordindatesType == Parameters.CoordinatesType.CARTESIAN);
                        model.setShowing(_import_variables.get(t));
                    }
                } break;
            default:
                for (ImportVariable variable : _import_variables) {
                    variable.setShowing(true);
                    model.setShowing(variable);
                }
        }
        // ensure that the input variable index values are sequential
        int showingIdx=0;
        for (ImportVariable variable : _import_variables) {
            if (variable.getShowing()) {
                variable.setVariableIndex(showingIdx);
                showingIdx++;
            }
        }
        model.fireTableDataChanged();
    }
    
    /** Returns a suggested filename for the import file based on the source filename and InputSourceSettings.InputFileType. */
    private void setSuggestedImportName(final String sourceFile, final String suggested_filename, final InputSourceSettings.InputFileType filetype) {
        String defaultName = "";
        String extension = "";
        switch (filetype) {
            case Case: defaultName = "Cases"; extension =  ".cas"; break;
            case Control: defaultName = "Controls"; extension =  ".ctl"; break;
            case Population: defaultName = "Population"; extension =  ".pop";  break;
            case Coordinates: defaultName = "Coordintes"; extension =  ".geo";  break;
            case SpecialGrid: defaultName = "Grid"; extension =  ".grd";  break;
            case MaxCirclePopulation: defaultName = "MaximumSizeCircle"; extension =  ".max"; break;
            case AdjustmentsByRR: defaultName = "Adjustments"; extension =  ".adj"; break;
            case Neighbors: defaultName = "NonEuclidianNeighbors"; extension =  ".nei"; break;
            case MetaLocations: defaultName = "MetaLocations"; extension =  ".meta"; break;
            case AlternativeHypothesis: defaultName = "AlternativeHypothesis"; extension =  ".ha"; break;
            default: throw new UnknownEnumException(filetype);
        }
        if (suggested_filename.trim().isEmpty()) {
            _suggested_import_filename = new File(_prefs.get(_prefLastBackup, System.getProperty("user.home")) + System.getProperty("file.separator") + defaultName + extension);
        } else {
            int lastDot = suggested_filename.trim().lastIndexOf(".");
            if (lastDot != -1)
                _suggested_import_filename = new File(suggested_filename.trim().substring(0, lastDot) + extension);
            else
                _suggested_import_filename = new File(suggested_filename.trim() + extension);
        }
        if (_suggested_import_filename.getAbsolutePath().equals(sourceFile)) {
            // we shouldn't suggest writing to the source file
            int lastDot = sourceFile.trim().lastIndexOf(".");
            _suggested_import_filename = new File(sourceFile.trim().substring(0, lastDot) + "(2)" + extension);
        }
    }

    /** Prepares the dialog for display and make visible. */
    @Override
    public void setVisible(boolean value) {
        try {
            if (value) {
                // Add the panels we're showing for file type.
                _main_content_panel.removeAll();
                _main_content_panel.add(_fileSourceSettingsPanel, _source_settings_cardname);
                if (!isImportableFileType(_input_source_settings.getInputFileType())) {
                    // Some input files do not support importing. Only show the start panel.
                    clearInputSettigs.setVisible(false);
                    nextButtonSource.setVisible(false);
                } else {
                    _main_content_panel.add(_fileFormatPanel, _file_format_cardname);
                    _main_content_panel.add(_dataMappingPanel, _data_mapping_cardname);
                    _main_content_panel.add(_outputSettingsPanel, _output_settings_cardname);
                }
                makeActivePanel(_source_settings_cardname);
            }
            getRootPane().setDefaultButton( acceptButton );
            _source_filename.requestFocusInWindow();
            super.setVisible(value);
        } catch (DataSourceException e) {
            JOptionPane.showMessageDialog(this, e.getMessage(), "Note", JOptionPane.INFORMATION_MESSAGE);
        } catch (Exception e) {
            new ExceptionDialog(this, e).setVisible(true);
        }
    }

    /**
     * This method is called from within the constructor to
     * configureForDestinationFileType the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

        _fieldSeparatorButtonGroup = new javax.swing.ButtonGroup();
        _groupIndicatorButtonGroup = new javax.swing.ButtonGroup();
        _import_operation_buttonGroup = new javax.swing.ButtonGroup();
        _projectionButtonGroup = new javax.swing.ButtonGroup();
        _dialog_base_panel = new javax.swing.JPanel();
        _main_content_panel = new javax.swing.JPanel();
        _fileSourceSettingsPanel = new javax.swing.JPanel();
        _file_selection_label = new javax.swing.JLabel();
        _source_filename = new javax.swing.JTextField();
        _browse_source = new javax.swing.JButton();
        _expectedFormatScrollPane = new javax.swing.JScrollPane();
        _expectedFormatTextPane = new javax.swing.JTextPane();
        _fileFormatPanel = new javax.swing.JPanel();
        jLabel1 = new javax.swing.JLabel();
        jScrollPane1 = new javax.swing.JScrollPane();
        _fileContentsTextArea = new javax.swing.JTextArea();
        jLabel4 = new javax.swing.JLabel();
        _ignoreRowsTextField = new javax.swing.JTextField();
        jLabel5 = new javax.swing.JLabel();
        _firstRowColumnHeadersCheckBox = new javax.swing.JCheckBox();
        fieldSeparatorGroup = new javax.swing.JPanel();
        _commaRadioButton = new javax.swing.JRadioButton();
        _semiColonRadioButton = new javax.swing.JRadioButton();
        _whitespaceRadioButton = new javax.swing.JRadioButton();
        _otherRadioButton = new javax.swing.JRadioButton();
        _otherFieldSeparatorTextField = new javax.swing.JTextField();
        _groupIndiocatorGroup = new javax.swing.JPanel();
        _doubleQuotesRadioButton = new javax.swing.JRadioButton();
        _singleQuotesRadioButton = new javax.swing.JRadioButton();
        _dataMappingPanel = new javax.swing.JPanel();
        jSplitPane1 = new javax.swing.JSplitPane();
        _dataMappingTopPanel = new javax.swing.JPanel();
        _displayVariablesLabel = new javax.swing.JLabel();
        _displayVariablesComboBox = new javax.swing.JComboBox();
        jScrollPane2 = new javax.swing.JScrollPane();
        _mapping_table = new javax.swing.JTable();
        _clearSelectionButton = new javax.swing.JButton();
        _dataMappingBottomPanel = new javax.swing.JPanel();
        _importTableScrollPane = new javax.swing.JScrollPane();
        _source_data_table = new javax.swing.JTable();
        jLabel2 = new javax.swing.JLabel();
        _outputSettingsPanel = new javax.swing.JPanel();
        _execute_import_now = new javax.swing.JRadioButton();
        _outputDirectoryTextField = new javax.swing.JTextField();
        _changeSaveDirectoryButton = new javax.swing.JButton();
        _save_import_settings = new javax.swing.JRadioButton();
        _progressBar = new javax.swing.JProgressBar();
        _button_cards_panel = new javax.swing.JPanel();
        _file_source_buttons_panel = new javax.swing.JPanel();
        acceptButton = new javax.swing.JButton();
        nextButtonSource = new javax.swing.JButton();
        clearInputSettigs = new javax.swing.JButton();
        _file_format_buttons_panel = new javax.swing.JPanel();
        previousButtonCSV = new javax.swing.JButton();
        nextButtonCSV = new javax.swing.JButton();
        _data_mapping_buttons_panel = new javax.swing.JPanel();
        previousButtonMapping = new javax.swing.JButton();
        nextButtonMapping = new javax.swing.JButton();
        _output_settings_buttons_panel = new javax.swing.JPanel();
        previousButtonOutSettings = new javax.swing.JButton();
        executeButton = new javax.swing.JButton();
        cancelButton = new javax.swing.JButton();

        _fieldSeparatorButtonGroup.add(_commaRadioButton);
        _fieldSeparatorButtonGroup.add(_semiColonRadioButton);
        _fieldSeparatorButtonGroup.add(_whitespaceRadioButton);
        _fieldSeparatorButtonGroup.add(_otherRadioButton);

        _groupIndicatorButtonGroup.add(_doubleQuotesRadioButton);
        _groupIndicatorButtonGroup.add(_singleQuotesRadioButton);

        setDefaultCloseOperation(javax.swing.WindowConstants.DISPOSE_ON_CLOSE);
        setTitle("Import File Wizard"); // NOI18N
        setModal(true);
        setPreferredSize(new java.awt.Dimension(550, 450));

        _dialog_base_panel.setLayout(new javax.swing.BoxLayout(_dialog_base_panel, javax.swing.BoxLayout.Y_AXIS));

        _main_content_panel.setPreferredSize(new java.awt.Dimension(580, 500));
        _main_content_panel.setLayout(new java.awt.CardLayout());

        _file_selection_label.setFont(new java.awt.Font("Tahoma", 0, 12)); // NOI18N
        _file_selection_label.setText("Case File:");

        _browse_source.setText("..."); // NOI18N
        _browse_source.setToolTipText("Browse for source file ..."); // NOI18N
        _browse_source.setPreferredSize(new java.awt.Dimension(45, 20));
        _browse_source.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                FileSelectionDialog selectionDialog = new FileSelectionDialog(SaTScanApplication.getInstance(), _input_source_settings.getInputFileType(), SaTScanApplication.getInstance().lastBrowseDirectory);
                File file = selectionDialog.browse_load(true);
                if (file != null) {
                    SaTScanApplication.getInstance().lastBrowseDirectory = selectionDialog.getDirectory();
                    _source_filename.setText(file.getAbsolutePath());
                }
            }
        });

        _expectedFormatScrollPane.setBorder(null);
        _expectedFormatScrollPane.setHorizontalScrollBarPolicy(javax.swing.ScrollPaneConstants.HORIZONTAL_SCROLLBAR_NEVER);
        _expectedFormatScrollPane.setBorder(null);

        _expectedFormatTextPane.setEditable(false);
        _expectedFormatTextPane.setBackground(new java.awt.Color(240, 240, 240));
        _expectedFormatTextPane.setContentType("text/html"); // NOI18N
        _expectedFormatTextPane.setText("<html>\r\n  <head>\r</head>\r\n  <body>\r\n    <p style=\"margin-top: 0;\">\r\rThe expected format of the case file, for the current settings, is:</p>\r\n    <p style=\"margin: 5px 0 0 5px;font-style:italic;font-size:1.01em;font-weight:bold;\">&lt;indentifier&gt;  &lt;count&gt; &lt;date&gt; &lt;covariate 1&gt; ... &lt;covariate N&gt;</p>\n  </body>\r\n</html>\r\n");
        _expectedFormatScrollPane.setViewportView(_expectedFormatTextPane);

        javax.swing.GroupLayout _fileSourceSettingsPanelLayout = new javax.swing.GroupLayout(_fileSourceSettingsPanel);
        _fileSourceSettingsPanel.setLayout(_fileSourceSettingsPanelLayout);
        _fileSourceSettingsPanelLayout.setHorizontalGroup(
            _fileSourceSettingsPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_fileSourceSettingsPanelLayout.createSequentialGroup()
                .addContainerGap()
                .addGroup(_fileSourceSettingsPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(_file_selection_label, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addGroup(_fileSourceSettingsPanelLayout.createSequentialGroup()
                        .addComponent(_source_filename)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(_browse_source, javax.swing.GroupLayout.PREFERRED_SIZE, 25, javax.swing.GroupLayout.PREFERRED_SIZE))
                    .addComponent(_expectedFormatScrollPane, javax.swing.GroupLayout.DEFAULT_SIZE, 530, Short.MAX_VALUE))
                .addContainerGap())
        );
        _fileSourceSettingsPanelLayout.setVerticalGroup(
            _fileSourceSettingsPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_fileSourceSettingsPanelLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(_file_selection_label)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(_fileSourceSettingsPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(_source_filename, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(_browse_source, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_expectedFormatScrollPane, javax.swing.GroupLayout.DEFAULT_SIZE, 322, Short.MAX_VALUE)
                .addContainerGap())
        );

        _main_content_panel.add(_fileSourceSettingsPanel, "source-settings");

        jLabel1.setText("Sampling of File Contents:"); // NOI18N

        _fileContentsTextArea.setEditable(false);
        _fileContentsTextArea.setColumns(20);
        _fileContentsTextArea.setRows(5);
        jScrollPane1.setViewportView(_fileContentsTextArea);

        jLabel4.setText("Ignore first"); // NOI18N

        _ignoreRowsTextField.setText("0"); // NOI18N
        _ignoreRowsTextField.addFocusListener(new java.awt.event.FocusAdapter() {
            public void focusLost(java.awt.event.FocusEvent e) {
                while (_ignoreRowsTextField.getText().length() == 0)
                if (undo.canUndo()) undo.undo(); else _ignoreRowsTextField.setText("0");
            }
        });
        _ignoreRowsTextField.addKeyListener(new java.awt.event.KeyAdapter() {
            public void keyTyped(java.awt.event.KeyEvent e) {
                Utils.validatePostiveNumericKeyTyped(_ignoreRowsTextField, e, 5);
            }
        });
        _ignoreRowsTextField.getDocument().addUndoableEditListener(new UndoableEditListener() {
            public void undoableEditHappened(UndoableEditEvent evt) {
                undo.addEdit(evt.getEdit());
            }
        });

        jLabel5.setText("rows"); // NOI18N

        _firstRowColumnHeadersCheckBox.setText("First row is column name"); // NOI18N
        _firstRowColumnHeadersCheckBox.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        _firstRowColumnHeadersCheckBox.setMargin(new java.awt.Insets(0, 0, 0, 0));

        fieldSeparatorGroup.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createEtchedBorder(), "Field Separator"));

        _commaRadioButton.setSelected(true);
        _commaRadioButton.setText("Comma"); // NOI18N
        _commaRadioButton.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        _commaRadioButton.setMargin(new java.awt.Insets(0, 0, 0, 0));

        _semiColonRadioButton.setText("Semicolon"); // NOI18N
        _semiColonRadioButton.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        _semiColonRadioButton.setMargin(new java.awt.Insets(0, 0, 0, 0));

        _whitespaceRadioButton.setText("Whitespace"); // NOI18N
        _whitespaceRadioButton.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        _whitespaceRadioButton.setMargin(new java.awt.Insets(0, 0, 0, 0));

        _otherRadioButton.setText("Other"); // NOI18N
        _otherRadioButton.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        _otherRadioButton.setMargin(new java.awt.Insets(0, 0, 0, 0));

        _otherFieldSeparatorTextField.addKeyListener(new java.awt.event.KeyAdapter() {
            public void keyTyped(java.awt.event.KeyEvent e) {
                //Utils.validatePostiveNumericKeyTyped(_otherFieldSeparatorTextField, e, 1);
                enableNavigationButtons();
            }
        });
        _otherFieldSeparatorTextField.getDocument().addUndoableEditListener(new UndoableEditListener() {
            public void undoableEditHappened(UndoableEditEvent evt) {
                undo.addEdit(evt.getEdit());
                enableNavigationButtons();
            }
        });

        javax.swing.GroupLayout fieldSeparatorGroupLayout = new javax.swing.GroupLayout(fieldSeparatorGroup);
        fieldSeparatorGroup.setLayout(fieldSeparatorGroupLayout);
        fieldSeparatorGroupLayout.setHorizontalGroup(
            fieldSeparatorGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(fieldSeparatorGroupLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(_commaRadioButton)
                .addGap(20, 20, 20)
                .addComponent(_semiColonRadioButton)
                .addGap(20, 20, 20)
                .addComponent(_whitespaceRadioButton)
                .addGap(20, 20, 20)
                .addComponent(_otherRadioButton)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_otherFieldSeparatorTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 20, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap(186, Short.MAX_VALUE))
        );
        fieldSeparatorGroupLayout.setVerticalGroup(
            fieldSeparatorGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(fieldSeparatorGroupLayout.createSequentialGroup()
                .addGroup(fieldSeparatorGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(_commaRadioButton)
                    .addComponent(_semiColonRadioButton)
                    .addComponent(_whitespaceRadioButton)
                    .addComponent(_otherRadioButton)
                    .addComponent(_otherFieldSeparatorTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );

        _groupIndiocatorGroup.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createEtchedBorder(), "Group Indicator"));

        _doubleQuotesRadioButton.setSelected(true);
        _doubleQuotesRadioButton.setText("Double Quotes"); // NOI18N
        _doubleQuotesRadioButton.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        _doubleQuotesRadioButton.setMargin(new java.awt.Insets(0, 0, 0, 0));

        _singleQuotesRadioButton.setText("Single Quotes"); // NOI18N
        _singleQuotesRadioButton.setBorder(javax.swing.BorderFactory.createEmptyBorder(0, 0, 0, 0));
        _singleQuotesRadioButton.setMargin(new java.awt.Insets(0, 0, 0, 0));

        javax.swing.GroupLayout _groupIndiocatorGroupLayout = new javax.swing.GroupLayout(_groupIndiocatorGroup);
        _groupIndiocatorGroup.setLayout(_groupIndiocatorGroupLayout);
        _groupIndiocatorGroupLayout.setHorizontalGroup(
            _groupIndiocatorGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_groupIndiocatorGroupLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(_doubleQuotesRadioButton)
                .addGap(20, 20, 20)
                .addComponent(_singleQuotesRadioButton)
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );
        _groupIndiocatorGroupLayout.setVerticalGroup(
            _groupIndiocatorGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_groupIndiocatorGroupLayout.createSequentialGroup()
                .addGroup(_groupIndiocatorGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(_doubleQuotesRadioButton)
                    .addComponent(_singleQuotesRadioButton))
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );

        javax.swing.GroupLayout _fileFormatPanelLayout = new javax.swing.GroupLayout(_fileFormatPanel);
        _fileFormatPanel.setLayout(_fileFormatPanelLayout);
        _fileFormatPanelLayout.setHorizontalGroup(
            _fileFormatPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, _fileFormatPanelLayout.createSequentialGroup()
                .addContainerGap()
                .addGroup(_fileFormatPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING)
                    .addComponent(_groupIndiocatorGroup, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addGroup(javax.swing.GroupLayout.Alignment.LEADING, _fileFormatPanelLayout.createSequentialGroup()
                        .addComponent(jLabel1)
                        .addGap(0, 0, Short.MAX_VALUE))
                    .addComponent(jScrollPane1)
                    .addGroup(javax.swing.GroupLayout.Alignment.LEADING, _fileFormatPanelLayout.createSequentialGroup()
                        .addComponent(jLabel4)
                        .addGap(5, 5, 5)
                        .addComponent(_ignoreRowsTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 42, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addGap(5, 5, 5)
                        .addComponent(jLabel5)
                        .addGap(18, 18, 18)
                        .addComponent(_firstRowColumnHeadersCheckBox, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                    .addComponent(fieldSeparatorGroup, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                .addContainerGap())
        );
        _fileFormatPanelLayout.setVerticalGroup(
            _fileFormatPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_fileFormatPanelLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(jLabel1)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(jScrollPane1, javax.swing.GroupLayout.PREFERRED_SIZE, 137, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(_fileFormatPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(jLabel4)
                    .addComponent(jLabel5)
                    .addComponent(_ignoreRowsTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(_firstRowColumnHeadersCheckBox))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addComponent(fieldSeparatorGroup, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addComponent(_groupIndiocatorGroup, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap(66, Short.MAX_VALUE))
        );

        _main_content_panel.add(_fileFormatPanel, "file-format");

        jSplitPane1.setBorder(null);
        jSplitPane1.setDividerLocation(170);
        jSplitPane1.setOrientation(javax.swing.JSplitPane.VERTICAL_SPLIT);
        jSplitPane1.setBorder(null);

        _displayVariablesLabel.setText("Display SaTScan Variables For:"); // NOI18N

        _displayVariablesComboBox.setModel(new javax.swing.DefaultComboBoxModel(new String[] { "Item 1", "Item 2", "Item 3", "Item 4" }));
        _displayVariablesComboBox.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                if (_input_source_settings.getInputFileType() == InputSourceSettings.InputFileType.Control || _input_source_settings.getInputFileType() == InputSourceSettings.InputFileType.Population) {
                    // skip this listener for the control and popuation files
                    return;
                }
                if (_input_source_settings.getInputFileType() == InputSourceSettings.InputFileType.Coordinates || _input_source_settings.getInputFileType() == InputSourceSettings.InputFileType.SpecialGrid) {
                    if (_displayVariablesComboBox.getSelectedIndex() == 0) {
                        _coordinatesType = Parameters.CoordinatesType.LATLON;
                    } else {
                        _coordinatesType = Parameters.CoordinatesType.CARTESIAN;
                    }
                }
                setShowingVariables();
                enableNavigationButtons();
            }
        });

        _mapping_table.setRowSelectionAllowed(false);
        _mapping_table.setModel(new VariableMappingTableModel(_import_variables));
        jScrollPane2.setViewportView(_mapping_table);

        _clearSelectionButton.setText("Clear"); // NOI18N
        _clearSelectionButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                clearSaTScanVariableFieldIndexes();
            }
        });

        javax.swing.GroupLayout _dataMappingTopPanelLayout = new javax.swing.GroupLayout(_dataMappingTopPanel);
        _dataMappingTopPanel.setLayout(_dataMappingTopPanelLayout);
        _dataMappingTopPanelLayout.setHorizontalGroup(
            _dataMappingTopPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, _dataMappingTopPanelLayout.createSequentialGroup()
                .addGroup(_dataMappingTopPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING)
                    .addGroup(_dataMappingTopPanelLayout.createSequentialGroup()
                        .addComponent(_displayVariablesLabel)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(_displayVariablesComboBox, 0, 316, Short.MAX_VALUE))
                    .addComponent(jScrollPane2, javax.swing.GroupLayout.DEFAULT_SIZE, 467, Short.MAX_VALUE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_clearSelectionButton))
        );
        _dataMappingTopPanelLayout.setVerticalGroup(
            _dataMappingTopPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_dataMappingTopPanelLayout.createSequentialGroup()
                .addGroup(_dataMappingTopPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(_displayVariablesLabel)
                    .addComponent(_displayVariablesComboBox, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(_dataMappingTopPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(_clearSelectionButton)
                    .addComponent(jScrollPane2, javax.swing.GroupLayout.DEFAULT_SIZE, 133, Short.MAX_VALUE))
                .addContainerGap())
        );

        jSplitPane1.setTopComponent(_dataMappingTopPanel);

        _dataMappingBottomPanel.setToolTipText("");

        _source_data_table.setModel(new javax.swing.table.DefaultTableModel(
            new Object [][] {
                {null, null, null, null},
                {null, null, null, null},
                {null, null, null, null},
                {null, null, null, null}
            },
            new String [] {
                "Title 1", "Title 2", "Title 3", "Title 4"
            }
        ));
        _source_data_table.setToolTipText("Displaying at most 50 rows from data file.");
        _source_data_table.setAutoResizeMode(javax.swing.JTable.AUTO_RESIZE_OFF);
        _source_data_table.setRowSelectionAllowed(false);
        _importTableScrollPane.setViewportView(_source_data_table);

        javax.swing.GroupLayout _dataMappingBottomPanelLayout = new javax.swing.GroupLayout(_dataMappingBottomPanel);
        _dataMappingBottomPanel.setLayout(_dataMappingBottomPanelLayout);
        _dataMappingBottomPanelLayout.setHorizontalGroup(
            _dataMappingBottomPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addComponent(_importTableScrollPane, javax.swing.GroupLayout.DEFAULT_SIZE, 530, Short.MAX_VALUE)
        );
        _dataMappingBottomPanelLayout.setVerticalGroup(
            _dataMappingBottomPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addComponent(_importTableScrollPane, javax.swing.GroupLayout.DEFAULT_SIZE, 185, Short.MAX_VALUE)
        );

        jSplitPane1.setRightComponent(_dataMappingBottomPanel);

        jLabel2.setText("# = Column is not actually defined in file but can be used as SaTScan variable.");

        javax.swing.GroupLayout _dataMappingPanelLayout = new javax.swing.GroupLayout(_dataMappingPanel);
        _dataMappingPanel.setLayout(_dataMappingPanelLayout);
        _dataMappingPanelLayout.setHorizontalGroup(
            _dataMappingPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_dataMappingPanelLayout.createSequentialGroup()
                .addContainerGap()
                .addGroup(_dataMappingPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(jSplitPane1)
                    .addComponent(jLabel2, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                .addContainerGap())
        );
        _dataMappingPanelLayout.setVerticalGroup(
            _dataMappingPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_dataMappingPanelLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(jSplitPane1)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(jLabel2))
        );

        _main_content_panel.add(_dataMappingPanel, "data-mapping");

        _import_operation_buttonGroup.add(_execute_import_now);
        _execute_import_now.setSelected(true);
        _execute_import_now.setText("Save imported input file as:");
        _execute_import_now.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED)
                executeButton.setText("Import");
            }
        });

        _outputDirectoryTextField.setText(_suggested_import_filename.getAbsolutePath());

        _changeSaveDirectoryButton.setText("Change"); // NOI18N
        _changeSaveDirectoryButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                FileSelectionDialog select = new FileSelectionDialog(org.satscan.gui.SaTScanApplication.getInstance(), "Select directory to save imported file", org.satscan.gui.SaTScanApplication.getInstance().lastBrowseDirectory);
                File file = select.browse_saveas();
                if (file != null) {
                    org.satscan.gui.SaTScanApplication.getInstance().lastBrowseDirectory = select.getDirectory();
                    _outputDirectoryTextField.setText(file.getAbsolutePath());
                    _prefs.put(_prefLastBackup, select.getDirectory().getAbsolutePath());
                }
            }
        });

        _import_operation_buttonGroup.add(_save_import_settings);
        _save_import_settings.setText("Save these settings and read directly from file source when running the analysis.");
        _save_import_settings.setVerticalAlignment(javax.swing.SwingConstants.TOP);
        _save_import_settings.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED)
                executeButton.setText("Ok");
            }
        });

        javax.swing.GroupLayout _outputSettingsPanelLayout = new javax.swing.GroupLayout(_outputSettingsPanel);
        _outputSettingsPanel.setLayout(_outputSettingsPanelLayout);
        _outputSettingsPanelLayout.setHorizontalGroup(
            _outputSettingsPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_outputSettingsPanelLayout.createSequentialGroup()
                .addContainerGap()
                .addGroup(_outputSettingsPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(_progressBar, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addGroup(_outputSettingsPanelLayout.createSequentialGroup()
                        .addGap(21, 21, 21)
                        .addComponent(_outputDirectoryTextField)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(_changeSaveDirectoryButton))
                    .addComponent(_execute_import_now, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(_save_import_settings, javax.swing.GroupLayout.DEFAULT_SIZE, 530, Short.MAX_VALUE))
                .addContainerGap())
        );
        _outputSettingsPanelLayout.setVerticalGroup(
            _outputSettingsPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_outputSettingsPanelLayout.createSequentialGroup()
                .addGap(27, 27, 27)
                .addComponent(_execute_import_now)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(_outputSettingsPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(_outputDirectoryTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(_changeSaveDirectoryButton))
                .addGap(18, 18, 18)
                .addComponent(_save_import_settings, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .addGap(234, 234, 234)
                .addComponent(_progressBar, javax.swing.GroupLayout.PREFERRED_SIZE, 30, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap())
        );

        _main_content_panel.add(_outputSettingsPanel, "output-settings");

        _dialog_base_panel.add(_main_content_panel);

        _button_cards_panel.setLayout(new java.awt.CardLayout());

        acceptButton.setText("Ok"); // NOI18N
        acceptButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                // set close status = Closed?
                setVisible(false);
            }
        });

        nextButtonSource.setText("Next >"); // NOI18N
        nextButtonSource.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                nextPanel();
            }
        });

        clearInputSettigs.setText("Clear Import"); // NOI18N
        clearInputSettigs.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                _input_source_settings.reset();
                clearInputSettigs.setEnabled(_input_source_settings.isSet());
                clearSaTScanVariableFieldIndexes();
                _execute_import_now.setSelected(true);
                _needs_import_save = true;
                _expectedFormatTextPane.setText(getFileExpectedFormatHtml());
                _expectedFormatTextPane.setCaretPosition(0);
                nextButtonSource.setText("Next >");
                acceptButton.requestFocus();
            }
        });

        javax.swing.GroupLayout _file_source_buttons_panelLayout = new javax.swing.GroupLayout(_file_source_buttons_panel);
        _file_source_buttons_panel.setLayout(_file_source_buttons_panelLayout);
        _file_source_buttons_panelLayout.setHorizontalGroup(
            _file_source_buttons_panelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_file_source_buttons_panelLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(acceptButton, javax.swing.GroupLayout.PREFERRED_SIZE, 100, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED, 212, Short.MAX_VALUE)
                .addComponent(clearInputSettigs, javax.swing.GroupLayout.PREFERRED_SIZE, 112, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(nextButtonSource, javax.swing.GroupLayout.PREFERRED_SIZE, 100, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap())
        );
        _file_source_buttons_panelLayout.setVerticalGroup(
            _file_source_buttons_panelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, _file_source_buttons_panelLayout.createSequentialGroup()
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .addGroup(_file_source_buttons_panelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(acceptButton)
                    .addComponent(nextButtonSource)
                    .addComponent(clearInputSettigs))
                .addContainerGap())
        );

        _button_cards_panel.add(_file_source_buttons_panel, "source-settings-buttons");

        _file_format_buttons_panel.setPreferredSize(new java.awt.Dimension(438, 40));

        previousButtonCSV.setText("< Previous"); // NOI18N
        previousButtonCSV.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                previousPanel();
            }
        });

        nextButtonCSV.setText("Next >"); // NOI18N
        nextButtonCSV.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                nextPanel();
            }
        });

        javax.swing.GroupLayout _file_format_buttons_panelLayout = new javax.swing.GroupLayout(_file_format_buttons_panel);
        _file_format_buttons_panel.setLayout(_file_format_buttons_panelLayout);
        _file_format_buttons_panelLayout.setHorizontalGroup(
            _file_format_buttons_panelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_file_format_buttons_panelLayout.createSequentialGroup()
                .addContainerGap(334, Short.MAX_VALUE)
                .addComponent(previousButtonCSV, javax.swing.GroupLayout.PREFERRED_SIZE, 100, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(nextButtonCSV, javax.swing.GroupLayout.PREFERRED_SIZE, 100, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap())
        );
        _file_format_buttons_panelLayout.setVerticalGroup(
            _file_format_buttons_panelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, _file_format_buttons_panelLayout.createSequentialGroup()
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .addGroup(_file_format_buttons_panelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(nextButtonCSV)
                    .addComponent(previousButtonCSV))
                .addContainerGap())
        );

        _button_cards_panel.add(_file_format_buttons_panel, "file-format-buttons");

        previousButtonMapping.setText("< Previous"); // NOI18N
        previousButtonMapping.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                previousPanel();
            }
        });

        nextButtonMapping.setText("Next >"); // NOI18N
        nextButtonMapping.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                nextPanel();
            }
        });

        javax.swing.GroupLayout _data_mapping_buttons_panelLayout = new javax.swing.GroupLayout(_data_mapping_buttons_panel);
        _data_mapping_buttons_panel.setLayout(_data_mapping_buttons_panelLayout);
        _data_mapping_buttons_panelLayout.setHorizontalGroup(
            _data_mapping_buttons_panelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, _data_mapping_buttons_panelLayout.createSequentialGroup()
                .addContainerGap(334, Short.MAX_VALUE)
                .addComponent(previousButtonMapping, javax.swing.GroupLayout.PREFERRED_SIZE, 100, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(nextButtonMapping, javax.swing.GroupLayout.PREFERRED_SIZE, 100, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap())
        );
        _data_mapping_buttons_panelLayout.setVerticalGroup(
            _data_mapping_buttons_panelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_data_mapping_buttons_panelLayout.createSequentialGroup()
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .addGroup(_data_mapping_buttons_panelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(nextButtonMapping)
                    .addComponent(previousButtonMapping))
                .addContainerGap())
        );

        _button_cards_panel.add(_data_mapping_buttons_panel, "data-mapping-buttons");

        previousButtonOutSettings.setText("< Previous"); // NOI18N
        previousButtonOutSettings.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                previousPanel();
            }
        });

        executeButton.setText("Import"); // NOI18N
        executeButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                try {
                    if (_execute_import_now.isSelected()) {
                        try {
                            createDestinationInformation();
                        } catch (IOException i) {
                            JOptionPane.showMessageDialog(FileSourceWizard.this,
                                "The import wizard encountered an error attempting to create the import file.\n" +
                                "This is most likely occuring because write permissions are not granted for\n" +
                                "specified directory. Please check path or review user permissions for specified directory.\n", "Note", JOptionPane.WARNING_MESSAGE);
                            return;
                        }
                        ImportTask task = new ImportTask();
                        task.addPropertyChangeListener(FileSourceWizard.this);
                        task.execute();
                    } else {
                        _needs_import_save = true;
                        _refresh_related_settings = true;
                        setVisible(false);
                    }
                } catch (Throwable t) {
                    new ExceptionDialog(FileSourceWizard.this, t).setVisible(true);
                }
            }
        });

        cancelButton.setText("Cancel");
        cancelButton.setEnabled(false);

        javax.swing.GroupLayout _output_settings_buttons_panelLayout = new javax.swing.GroupLayout(_output_settings_buttons_panel);
        _output_settings_buttons_panel.setLayout(_output_settings_buttons_panelLayout);
        _output_settings_buttons_panelLayout.setHorizontalGroup(
            _output_settings_buttons_panelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, _output_settings_buttons_panelLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(cancelButton, javax.swing.GroupLayout.PREFERRED_SIZE, 85, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED, 246, Short.MAX_VALUE)
                .addComponent(previousButtonOutSettings, javax.swing.GroupLayout.PREFERRED_SIZE, 93, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(executeButton, javax.swing.GroupLayout.PREFERRED_SIZE, 100, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap())
        );
        _output_settings_buttons_panelLayout.setVerticalGroup(
            _output_settings_buttons_panelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, _output_settings_buttons_panelLayout.createSequentialGroup()
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .addGroup(_output_settings_buttons_panelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(previousButtonOutSettings)
                    .addComponent(executeButton)
                    .addComponent(cancelButton))
                .addContainerGap())
        );

        _button_cards_panel.add(_output_settings_buttons_panel, "output-settings-buttons");

        _dialog_base_panel.add(_button_cards_panel);

        javax.swing.GroupLayout layout = new javax.swing.GroupLayout(getContentPane());
        getContentPane().setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addComponent(_dialog_base_panel, javax.swing.GroupLayout.DEFAULT_SIZE, 550, Short.MAX_VALUE)
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addComponent(_dialog_base_panel, javax.swing.GroupLayout.DEFAULT_SIZE, 436, Short.MAX_VALUE)
        );

        pack();
    }// </editor-fold>//GEN-END:initComponents
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JButton _browse_source;
    private javax.swing.JPanel _button_cards_panel;
    private javax.swing.JButton _changeSaveDirectoryButton;
    private javax.swing.JButton _clearSelectionButton;
    private javax.swing.JRadioButton _commaRadioButton;
    private javax.swing.JPanel _dataMappingBottomPanel;
    private javax.swing.JPanel _dataMappingPanel;
    private javax.swing.JPanel _dataMappingTopPanel;
    private javax.swing.JPanel _data_mapping_buttons_panel;
    private javax.swing.JPanel _dialog_base_panel;
    private javax.swing.JComboBox _displayVariablesComboBox;
    private javax.swing.JLabel _displayVariablesLabel;
    private javax.swing.JRadioButton _doubleQuotesRadioButton;
    private javax.swing.JRadioButton _execute_import_now;
    private javax.swing.JScrollPane _expectedFormatScrollPane;
    private javax.swing.JTextPane _expectedFormatTextPane;
    private javax.swing.ButtonGroup _fieldSeparatorButtonGroup;
    private javax.swing.JTextArea _fileContentsTextArea;
    private javax.swing.JPanel _fileFormatPanel;
    private javax.swing.JPanel _fileSourceSettingsPanel;
    private javax.swing.JPanel _file_format_buttons_panel;
    private javax.swing.JLabel _file_selection_label;
    private javax.swing.JPanel _file_source_buttons_panel;
    private javax.swing.JCheckBox _firstRowColumnHeadersCheckBox;
    private javax.swing.ButtonGroup _groupIndicatorButtonGroup;
    private javax.swing.JPanel _groupIndiocatorGroup;
    private javax.swing.JTextField _ignoreRowsTextField;
    private javax.swing.JScrollPane _importTableScrollPane;
    private javax.swing.ButtonGroup _import_operation_buttonGroup;
    private javax.swing.JPanel _main_content_panel;
    private javax.swing.JTable _mapping_table;
    private javax.swing.JTextField _otherFieldSeparatorTextField;
    private javax.swing.JRadioButton _otherRadioButton;
    private javax.swing.JTextField _outputDirectoryTextField;
    private javax.swing.JPanel _outputSettingsPanel;
    private javax.swing.JPanel _output_settings_buttons_panel;
    private javax.swing.JProgressBar _progressBar;
    private javax.swing.ButtonGroup _projectionButtonGroup;
    private javax.swing.JRadioButton _save_import_settings;
    private javax.swing.JRadioButton _semiColonRadioButton;
    private javax.swing.JRadioButton _singleQuotesRadioButton;
    private javax.swing.JTable _source_data_table;
    private javax.swing.JTextField _source_filename;
    private javax.swing.JRadioButton _whitespaceRadioButton;
    private javax.swing.JButton acceptButton;
    private javax.swing.JButton cancelButton;
    private javax.swing.JButton clearInputSettigs;
    private javax.swing.JButton executeButton;
    private javax.swing.JPanel fieldSeparatorGroup;
    private javax.swing.JLabel jLabel1;
    private javax.swing.JLabel jLabel2;
    private javax.swing.JLabel jLabel4;
    private javax.swing.JLabel jLabel5;
    private javax.swing.JScrollPane jScrollPane1;
    private javax.swing.JScrollPane jScrollPane2;
    private javax.swing.JSplitPane jSplitPane1;
    private javax.swing.JButton nextButtonCSV;
    private javax.swing.JButton nextButtonMapping;
    private javax.swing.JButton nextButtonSource;
    private javax.swing.JButton previousButtonCSV;
    private javax.swing.JButton previousButtonMapping;
    private javax.swing.JButton previousButtonOutSettings;
    // End of variables declaration//GEN-END:variables
    
    /** Table mapping_model used to assign import variables to source file fields.  */
    class VariableMappingTableModel extends AbstractTableModel {

        private static final long serialVersionUID = 1L;
        private String[] _column_names = {"SaTScan Variable", "Source File Variable"};
        private final Vector<ImportVariable> _static_variables;
        private Vector<ImportVariable> _visible_variables;
        public JComboBox _combo_box = new JComboBox();

        public VariableMappingTableModel(Vector<ImportVariable> variables) {
            _static_variables = variables;
            _visible_variables = (Vector)_static_variables.clone();
        }

        public int getColumnCount() {
            return _column_names.length;
        }

        public int getRowCount() {
            return _visible_variables.size();
        }

        @Override
        public Class getColumnClass(int c) {
            return getValueAt(0, c).getClass();
        }

        @Override
        public String getColumnName(int col) {
            return _column_names[col];
        }

        public Object getValueAt(int row, int col) {
            if (col == 0) {
                return _visible_variables.get(row).getDisplayLabel();
            } else {
                if (_visible_variables.get(row).getSourceFieldIndex() < 1) return _unassigned_variable;
                String name = _preview_table_model.getDataSourceColumnName(_visible_variables.get(row).getSourceFieldIndex() - 1);
                return name == null || name.isEmpty() ? _unassigned_variable : name;
            }
        }

        @Override
        public boolean isCellEditable(int row, int col) {
            return col == 0 ? false : true;
        }

        public void setShowing(final ImportVariable variable) {
            if (variable.getShowing()) {
                showVariable(variable.getVariableName());
            } else {
                hideVariable(variable.getVariableName());
            }
        }

        public boolean isShowing(final String variableName) {
            for (int i = 0; i < _visible_variables.size(); ++i) {
                if (variableName.equals(_visible_variables.get(i).getVariableName())) {
                    return true;
                }
            }
            return false;
        }

        public void showVariable(final String variableName) {
            //first search to see if aleady showing
            for (int i = 0; i < _visible_variables.size(); ++i) {
                if (variableName.equals(_visible_variables.get(i).getVariableName())) {
                    return; //already showing
                }
            }
            //find index in variables vector
            for (int i = 0; i < _static_variables.size(); ++i) {
                if (variableName.equals(_static_variables.get(i).getVariableName())) {
                    _visible_variables.add(_static_variables.get(i));
                    return; //already showing
                }
            }

        }

        public void hideVariable(final String variableName) {
            for (int i=0; i < _visible_variables.size(); ++i) {
                if (variableName.equals(_visible_variables.get(i).getVariableName())) {
                    _visible_variables.remove(i);
                    return;
                }
            }
        }

        public void hideAll() {
            _visible_variables.clear();
        }

        @Override
        public void setValueAt(Object value, int row, int col) {
            if (value == null) {
                return;
            }
            for (int i=0; i < _combo_box.getItemCount(); ++i) {
                if (_combo_box.getItemAt(i).equals(value)) {
                    _visible_variables.get(row).setSourceFieldIndex(_preview_table_model.getDataSourceColumnIndex((String)value));
                    enableNavigationButtons();
                    fireTableCellUpdated(row, col);
                    return;
                }
            }
        }
    }

    /** Import worker; executed in background thread. */
    class ImportTask extends SwingWorker<Void, Void> implements ActionListener {

        private WaitCursor waitCursor = new WaitCursor(FileSourceWizard.this);
        private FileImporter _importer;

        @Override
        public Void doInBackground() {
            try {
                cancelButton.setEnabled(true);
                cancelButton.addActionListener(this);
                previousButtonOutSettings.setEnabled(false);
                acceptButton.setEnabled(false);
                executeButton.setEnabled(false);
                _progressBar.setVisible(true);
                _progressBar.setValue(0);
                VariableMappingTableModel model = (VariableMappingTableModel) _mapping_table.getModel();
                _importer = new FileImporter(getImportSource(),
                        model._static_variables,
                        _input_source_settings.getInputFileType(),
                        _input_source_settings.getSourceDataFileType(),
                        _destinationFile,
                        _progressBar);
                _importer.importFile(0);
                if (!_importer.getCancelled()) {
                    _executed_import = true;
                    _refresh_related_settings = true;
                    setVisible(false);
                }
            } catch (IOException e) {
                JOptionPane.showMessageDialog(FileSourceWizard.this,
                        "The import wizard encountered an error attempting to create the import file.\n" +
                        "This is most likely occuring because write permissions are not granted for\n" +
                        "specified directory. Please check path or review user permissions for specified directory.\n", "Note", JOptionPane.WARNING_MESSAGE);
            } catch (SecurityException e) {
                JOptionPane.showMessageDialog(FileSourceWizard.this,
                        "The import wizard encountered an error attempting to create the import file.\n" +
                        "This is most likely occuring because write permissions are not granted for\n" +
                        "specified directory. Please check path or review user permissions for specified directory.\n", "Note", JOptionPane.WARNING_MESSAGE);
            } catch (ImportException e) {
                JOptionPane.showMessageDialog(FileSourceWizard.this, e.getMessage(), "Note", JOptionPane.ERROR_MESSAGE);
            } catch (Throwable t) {
                new ExceptionDialog(FileSourceWizard.this, t).setVisible(true);
            } finally {
                cancelButton.removeActionListener(this);
                cancelButton.setEnabled(false);
                previousButtonOutSettings.setEnabled(true);
                _progressBar.setVisible(false);
            }
            return null;
        }

        /*
         * Executed in event dispatching thread
         */
        @Override
        public void done() {
            executeButton.setEnabled(true);
            acceptButton.setEnabled(true);
            waitCursor.restore();
        }

        public void actionPerformed(ActionEvent e) {
            _importer.setCancelled();
        }
    }
}
