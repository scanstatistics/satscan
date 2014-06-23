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

/**
 * @author  Hostovic
 */
public class FileSourceWizard extends javax.swing.JDialog implements PropertyChangeListener {

    private Preferences _prefs = Preferences.userNodeForPackage(getClass());
    private static final String _prefLastBackup = new String("import.destination");
    private final String _startCardName = "Source Settings";
    private final String _fileFormatCardName = "File Format";
    private final String _dataMappingCardName = "Mapping Panel";
    private final String _outPutSettingsCardName = "Output Settings";
    private final String _fileFormatDelimitedCardName = "cvsPanel";
    private final String _fileFormatFixedColumnCardName = "fixedPanel";
    private final String _unAssignedVariableName = "unassigned";
    private final String _importFilePrefix = "import";
    private final int _dateVariableColumn = 2;
    private final int _sourceFileLineSample = 200;
    private final UndoManager undo = new UndoManager();
    private Vector<ImportVariable> _importVariables = new Vector<ImportVariable>();
    private String _sourceFile;
    private final Parameters.ProbabilityModelType _startingModelType;
    private Parameters.CoordinatesType _coordinatesType;
    private String _showingCard;
    private boolean _errorSamplingSourceFile = true;
    private PreviewTableModel _previewTableModel = null;
    private File _destinationFile = null;
    private File _suggested_import_filename;
    private final InputSourceSettings _inputSourceSettings;
    private boolean _executed_import=false;
    private boolean _needs_import_save=false;

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
        _inputSourceSettings = new InputSourceSettings(inputSourceSettings);
        _inputSourceSettings.setSourceDataFileType(getSourceFileType(sourceFile));
        _sourceFile = sourceFile;
        _startingModelType = modelType;
        _coordinatesType = coordinatesType;
        _progressBar.setVisible(false);
        configureForDestinationFileType();
        setShowingVariables();
        initializeImportVariableMappings();
        if (_inputSourceSettings.getSourceDataFileType() == InputSourceSettings.SourceDataFileType.Excel) {
            // live read not implemented for Excel files.
            _execute_import_now.setSelected(true);
            _save_import_settings.setEnabled(false);
        } else {
            _save_import_settings.setSelected(_inputSourceSettings.isSet());
        }
        setLocationRelativeTo(parent);
    }

    /** Causes showCard to be the active card. */
    private void bringPanelToFront(String showCard) {
        ((CardLayout) _basePanel.getLayout()).show(_basePanel, showCard);
        _showingCard = showCard;
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
        VariableMappingTableModel model = (VariableMappingTableModel) _fieldMapTable.getModel();
        for (ImportVariable variable : _importVariables) {
            if (variable.getIsRequiredField() && model.isShowing(variable.getVariableName()) && !variable.getIsMappedToInputFileVariable()) {
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
        if (_inputSourceSettings.getInputFileType() == InputSourceSettings.InputFileType.AdjustmentsByRR) {
            if ((model.variables_visible.get(2).getIsMappedToInputFileVariable() && !model.variables_visible.get(3).getIsMappedToInputFileVariable()) ||
                (!model.variables_visible.get(2).getIsMappedToInputFileVariable() && model.variables_visible.get(3).getIsMappedToInputFileVariable())) {
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
        for (ImportVariable variable : _importVariables) {
            variable.setInputFileVariableIndex(0);
        }
        ((VariableMappingTableModel) _fieldMapTable.getModel()).fireTableDataChanged();
    }

    /** Configures the combo-box which changes what variables are displayed. */
    private void configureDisplayVariablesComboBox() {
        switch (_inputSourceSettings.getInputFileType()) {
            case Case:
                _displayVariablesComboBox.removeAllItems();
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
                _displayVariablesComboBox.removeAllItems();
                _displayVariablesComboBox.addItem("Latitude/Longitude Coordinates");
                _displayVariablesComboBox.addItem("Cartesian (x, y) Coordinates");
                _coordinatesType = temp;
                switch (_coordinatesType) {
                    case CARTESIAN : _displayVariablesComboBox.setSelectedIndex(1); break;
                    case LATLON    :
                    default        : _displayVariablesComboBox.setSelectedIndex(0);
                } break;
            default: 
                _displayVariablesLabel.setEnabled(false);
                _displayVariablesComboBox.setEnabled(false);
                _displayVariablesComboBox.removeAllItems();
        }
    }

    /** Configures the variables vector based upon the source file type. */
    private void configureForDestinationFileType() {
        switch (_inputSourceSettings.getInputFileType()) {
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
            default: throw new UnknownEnumException(_inputSourceSettings.getInputFileType());
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
        if (_showingCard == null) {
            return;
        } else if (_showingCard.equals(_startCardName)) {
            // Require a source file to be specified -- we'll verified that it exists and is readable later.
            nextButtonSource.setEnabled(!_source_filename.getText().isEmpty());
        } else if (_showingCard.equals(_fileFormatCardName)) {
            // Require that were able to read file sampling and necessary inputs were specified.
            nextButtonCSV.setEnabled(!_errorSamplingSourceFile && (_otherRadioButton.isSelected() ? _otherFieldSeparatorTextField.getText().length() > 0 : true));
        } else if (_showingCard.equals(_dataMappingCardName)) {
            // Require there is at least one record to import and at least one variable is mapped - we'll verify the mappings later.
            if (_importTableDataTable.getModel().getRowCount() > 0) {
                VariableMappingTableModel model = (VariableMappingTableModel) _fieldMapTable.getModel();
                for (int i=0; i < model.variables_visible.size() && !nextButtonSource.isEnabled(); ++i) {
                    nextButtonMapping.setEnabled(model.variables_visible.get(i).getIsMappedToInputFileVariable());
                }
            }
        } else if (_showingCard.equals(_outPutSettingsCardName)) {
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
        switch (_inputSourceSettings.getInputFileType()) {
            case Case:
            case Control: return _importVariables.get(_dateVariableColumn).getIsMappedToInputFileVariable();
            case Population: return _importVariables.get(1).getIsMappedToInputFileVariable();
            case Coordinates:
            case SpecialGrid:
            case MaxCirclePopulation: 
            case Neighbors:
            case MetaLocations:
            case AlternativeHypothesis: return false;
            case AdjustmentsByRR: return _importVariables.get(_dateVariableColumn).getIsMappedToInputFileVariable();
            default: throw new UnknownEnumException(_inputSourceSettings.getInputFileType());
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
    
    /** Builds html which details the fields expected in the input file. */
    private String getFileExpectedFormatHtml() {
        StringBuilder builder = new StringBuilder();
        builder.append("<html><head><style>th {font-weight:bold;text-align:right;}</style></head><body>");
        builder.append(getFileExpectedFormatParagraphs());
        if (isImportableFileType(_inputSourceSettings.getInputFileType())) {
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
        builder.append(FileSelectionDialog.getFileTypeAsString(_inputSourceSettings.getInputFileType()).toLowerCase()).append(" file");
        if (_inputSourceSettings.getInputFileType() == InputSourceSettings.InputFileType.Case) {
            builder.append(", using the ").append(Parameters.GetProbabilityModelTypeAsString(getModelControlType())).append(" probability model");
        } else if (_inputSourceSettings.getInputFileType() == InputSourceSettings.InputFileType.Coordinates ||
                   _inputSourceSettings.getInputFileType() == InputSourceSettings.InputFileType.SpecialGrid) {
            builder.append(", using ").append(getCoorinatesControlType() == Parameters.CoordinatesType.LATLON ? "Latitude/Longitude" : "Cartesian").append(" coordinates");
        }
        builder.append(" is:</p><span style=\"margin: 5px 0 0 5px;font-style:italic;font-weight:bold;\">");
        switch (_inputSourceSettings.getInputFileType()) {
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
            default: throw new UnknownEnumException(_inputSourceSettings.getInputFileType());
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
    
    /** Return the ImportSource object -- based upon the source file type. */
    private ImportDataSource getImportSource() throws FileNotFoundException {
        if (_inputSourceSettings.getSourceDataFileType() == InputSourceSettings.SourceDataFileType.Shapefile) {
            return new ShapefileDataSource(new File(_sourceFile), true);
        } else if (_inputSourceSettings.getSourceDataFileType() == InputSourceSettings.SourceDataFileType.dBase) {
            return new DBaseImportDataSource(new File(_sourceFile), false);
        } else if (_inputSourceSettings.getSourceDataFileType() == InputSourceSettings.SourceDataFileType.Excel) {
            return new XLSImportDataSource(new File(_sourceFile), true);
        } else {
            return new CSVImportDataSource(new File(_sourceFile), _firstRowColumnHeadersCheckBox.isSelected(), '\n', getColumnDelimiter(), getGroupMarker());
        }
    }
    
    /** Returns InputSourceSettings.InputFileType as string. */
    private String getInputFileTypeString() {
        switch (_inputSourceSettings.getInputFileType()) {
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
            default: throw new UnknownEnumException(_inputSourceSettings.getInputFileType());
        }
    }    
    
    /** Returns the base collection of supported file filters. */
    public static ArrayList<InputFileFilter> getInputFilters() {
        // define file filters supported by import wizard
        ArrayList<InputFileFilter> filters = new ArrayList<InputFileFilter>();
        filters.add(new InputFileFilter("csv","Delimited Files (*.csv)"));
        filters.add(new InputFileFilter("xls","Excel Files (*.xls)"));
        filters.add(new InputFileFilter("dbf","dBase Files (*.dbf)"));
        filters.add(new InputFileFilter("txt","Text Files (*.txt)"));
        filters.add(new InputFileFilter("shp","Shape Files (*.shp)"));
        return filters;
    }
    
    /** Returns InputSourceSettings object from user selections in wizard. */
    public final InputSourceSettings getInputSourceSettings() {
        return _inputSourceSettings;
    }
    
    /** Builds html which details the input source type and variable mappings to source file's columns. */
    private String getMappingsHtml() {
        Vector<String> columnNames = getSourceColumnNames();
        StringBuilder builder = new StringBuilder();
        builder.append("<html><head><style>th {font-weight: bold;text-align:right;}</style></head><body>");
        builder.append(getFileExpectedFormatParagraphs());
        builder.append("<p style=\"margin-top: 10px;\"> The ").append(FileSelectionDialog.getFileTypeAsString(_inputSourceSettings.getInputFileType()).toLowerCase());
        builder.append(" data will be read from the specified file according to the following settings:</p>");
        builder.append("<div style=\"margin-top:5px;\"><table><tr><td valign=\"top\"><table >");
        switch (_inputSourceSettings.getSourceDataFileType()) {
            case CSV :
                builder.append("<tr><th>File Type:</th><td>CSV</td></tr>");
                builder.append("<tr><th>Delimiter:</th><td>");
                if (_inputSourceSettings.getDelimiter().equals("") || _inputSourceSettings.getDelimiter().equals(" ")) {
                    builder.append("Whitespace");
                } else if (_inputSourceSettings.getDelimiter().equals(",")) {
                    builder.append("Comma");
                } else if (_inputSourceSettings.getDelimiter().equals(";")) {
                    builder.append("Semicolon");
                } else {
                    builder.append(_inputSourceSettings.getDelimiter());
                }
                builder.append("</td></tr><tr><th>Group By:</th><td style=\"white-space:nowrap;\">");
                if (_inputSourceSettings.getGroup().equals("\"")) {
                    builder.append("Double Quote");
                } else if (_inputSourceSettings.getGroup().equals("'")) {
                    builder.append("Single Quote");
                } else {
                    builder.append(_inputSourceSettings.getGroup());
                }
                builder.append("</td></tr><tr><th>Lines Skipped:</th><td>" + _inputSourceSettings.getSkiplines() + "</td></tr>");
                builder.append("<tr><th>Column Header:</th><td>" + (_inputSourceSettings.getFirstRowHeader() ? "Yes" : "No") + "</td></tr>");
                break;
            case dBase : builder.append("<tr><th>File Type:</th><td>dBase</td></tr>"); break;
            case Excel : builder.append("<tr><th>File Type:</th><td>Excel</td></tr>"); break;
            case Shapefile : builder.append("<tr><th>File Type:</th><td>Shapefile</td></tr>"); break;
        }
        builder.append("</table></td>");
        builder.append("<td valign=\"top\"><table><tr><th valign=\"top\">Field Mapping:</th><td><table>");
        for (int i=0; i < _inputSourceSettings.getFieldMaps().size(); ++i) {
            ImportVariable variable = getShowingImportVariableAt(i);
            if (variable != null && variable.getInputFileVariableIndex() > 0) {
                builder.append("<tr><td>" + variable.getVariableDisplayName() + "</td><td> &#8667;</td> <td>");
                if (_inputSourceSettings.getFieldMaps().get(i).isEmpty() || Integer.parseInt(_inputSourceSettings.getFieldMaps().get(i)) < 1) {
                    builder.append("---");
                } else {
                    if (i < columnNames.size()) {
                      String name = columnNames.elementAt(i);
                      if (name.length() > 15) {
                          name = name.substring(0, 14) + " ...";
                      }
                      builder.append(columnNames.elementAt(Integer.parseInt(_inputSourceSettings.getFieldMaps().get(i)) - 1));
                    } else {
                        builder.append("Column " + (_inputSourceSettings.getFieldMaps().get(i)));
                    }
                }
                builder.append("</td></tr>");
            }
        }
        builder.append("</table></td></tr></table></td></tr></table></div></body></html>");
        return builder.toString();
    }

    /** Builds html which details the input source type and variable mappings to source file's columns. */
    private String getMappingsHtml_alternative() {
        Vector<String> columnNames = getSourceColumnNames();
        StringBuilder builder = new StringBuilder();
        builder.append("<html><head><style>th {font-weight: bold;text-align:right;}</style></head><body>");
        builder.append(getFileExpectedFormatParagraphs());
        builder.append("<p style=\"margin-top: 10px;\"> The ").append(FileSelectionDialog.getFileTypeAsString(_inputSourceSettings.getInputFileType()).toLowerCase());
        builder.append(" data will be read from the specified file according to the following settings:</p>");
        builder.append("<div style=\"margin-top:5px;\"><table >");
        switch (_inputSourceSettings.getSourceDataFileType()) {
            case CSV :
                builder.append("<tr><th>File Type:</th><td>CSV (");
                builder.append("Delimiter=");
                if (_inputSourceSettings.getDelimiter().equals("") || _inputSourceSettings.getDelimiter().equals(" ")) {
                    builder.append("Whitespace");
                } else if (_inputSourceSettings.getDelimiter().equals(",")) {
                    builder.append("Comma");
                } else if (_inputSourceSettings.getDelimiter().equals(";")) {
                    builder.append("Semicolon");
                } else {
                    builder.append(_inputSourceSettings.getDelimiter());
                }
                builder.append(", Group=");
                if (_inputSourceSettings.getGroup().equals("\"")) {
                    builder.append("Double Quote");
                } else if (_inputSourceSettings.getGroup().equals("'")) {
                    builder.append("Single Quote");
                } else {
                    builder.append(_inputSourceSettings.getGroup());
                }
                builder.append(", Lines Skipped=" + _inputSourceSettings.getSkiplines());
                builder.append(", Column Header=" + (_inputSourceSettings.getFirstRowHeader() ? "Yes" : "No") + ")</td></tr>");
                break;
            case dBase : builder.append("<tr><th>File Type:</th><td>dBase</td></tr>"); break;
            case Excel : builder.append("<tr><th>File Type:</th><td>Excel</td></tr>"); break;
            case Shapefile : builder.append("<tr><th>File Type:</th><td>Shapefile</td></tr>"); break;
        }
        builder.append("<tr><th valign=\"top\">Field Mapping:</th><td><table>");
        for (int i=0; i < _inputSourceSettings.getFieldMaps().size(); ++i) {
            ImportVariable variable = getShowingImportVariableAt(i);
            if (variable != null && variable.getInputFileVariableIndex() > 0) {
                builder.append("<tr><td>" + variable.getVariableDisplayName() + "</td><td> &#8667;</td> <td>");
                if (_inputSourceSettings.getFieldMaps().get(i).isEmpty() || Integer.parseInt(_inputSourceSettings.getFieldMaps().get(i)) < 1) {
                    builder.append("---");
                } else {
                    if (i < columnNames.size()) {
                      String name = columnNames.elementAt(i);
                      if (name.length() > 15) {
                          name = name.substring(0, 14) + " ...";
                      }
                      builder.append(columnNames.elementAt(Integer.parseInt(_inputSourceSettings.getFieldMaps().get(i)) - 1));
                    } else {
                        builder.append("Column " + (_inputSourceSettings.getFieldMaps().get(i)));
                    }
                }
            }
        }
        builder.append("</table></td></tr></table></div></body></html>");
        return builder.toString();
    }
    
    /** Returns the probability model type selected by user. */
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
        for (ImportVariable variable : _importVariables) {
            if (variable.getShowing() && variable.getTargetFieldIndex() == idx)
                return variable;
        } return null;
    }    
    
    /** Attempts to obtain the column names for the source file given InputSourceSettings. */
    private Vector<String> getSourceColumnNames() {
        Vector<String> column_names = new Vector<String>();
        try {
            File file = new File(_sourceFile);
            if (file.exists()) {
                Object[] names=null;
                if (_inputSourceSettings.getSourceDataFileType() == InputSourceSettings.SourceDataFileType.Shapefile) {
                    ShapefileDataSource source = new ShapefileDataSource(file, true);
                    names = source.getColumnNames();
                } else if (_inputSourceSettings.getSourceDataFileType() == InputSourceSettings.SourceDataFileType.dBase) {
                    DBaseImportDataSource source = new DBaseImportDataSource(file, true);
                    names = source.getColumnNames();
                } else if (_inputSourceSettings.getSourceDataFileType() == InputSourceSettings.SourceDataFileType.CSV && this._inputSourceSettings.getFirstRowHeader()) {
                    CSVImportDataSource source = new CSVImportDataSource(file, false, '\n', _inputSourceSettings.getDelimiter().charAt(0), _inputSourceSettings.getGroup().charAt(0));
                    for (int i=0; i < _inputSourceSettings.getSkiplines(); ++i) {
                        Object[] values = source.readRow();
                    }
                    names = source.readRow();
                }
                for (int i=0; names != null && i < names.length; ++i) {
                    column_names.add((String)names[i]);
                }
            }
        } catch (Exception e){}
        return column_names;
    }
    
    /** Returns source file type given source file extension. */
    private InputSourceSettings.SourceDataFileType getSourceFileType(final String filename) {
        int pos = filename.lastIndexOf('.');
        if (pos != -1 && filename.substring(pos + 1).equalsIgnoreCase("shp")) {
            return InputSourceSettings.SourceDataFileType.Shapefile;
        } else if (pos != -1 && filename.substring(pos + 1).equalsIgnoreCase("dbf")) {
            return InputSourceSettings.SourceDataFileType.dBase;
        } else if (pos != -1 && (filename.substring(pos + 1).equalsIgnoreCase("xls") || filename.substring(pos + 1).equalsIgnoreCase("xlsx"))) {
            return InputSourceSettings.SourceDataFileType.Excel;
        }
        return InputSourceSettings.SourceDataFileType.CSV;
    }
    
    /** Returns source filename. */
    public String getSourceFilename() {
        return _sourceFile;
    }
    
    /** Sets initial import variable mappings from input source. */
    private void initializeImportVariableMappings() {
        for (int s=0; s < _inputSourceSettings.getFieldMaps().size(); ++s) {
            // translate field map index into input file variable index
            String targetfieldStr = _inputSourceSettings.getFieldMaps().get(s).trim();
            if (!targetfieldStr.isEmpty()) {
                int s_index = Integer.parseInt(_inputSourceSettings.getFieldMaps().get(s));
                for (ImportVariable variable : _importVariables) {
                    if (variable.getTargetFieldIndex() == s) {
                        variable.setInputFileVariableIndex(s_index);
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
        if (targetCardName.equals(_startCardName)) {
            prepFileSourceOptionsPanel();
        } else if (targetCardName.equals(_fileFormatCardName)) {
            prepFileFormatPanel();
        } else if (targetCardName.equals(_dataMappingCardName)) {
            prepMappingPanel();
        } else if (targetCardName.equals(_outPutSettingsCardName)) {
            prepOutputSettingsPanel();
        }
        bringPanelToFront(targetCardName);
    }
    
    /** Attempts to advance wizard to the next panel. */
    private void nextPanel() {
        try {
            if (_showingCard == _startCardName) {
                /* The user might have changed the filename on the File Options panel.
                 * We need to check the SourceDataFileType for specified filename. */
                _sourceFile = _source_filename.getText();
                if (FileAccess.ValidateFileAccess(_sourceFile, false)) {
                    _inputSourceSettings.setSourceDataFileType(getSourceFileType(_sourceFile));
                    if (_inputSourceSettings.getSourceDataFileType() == SourceDataFileType.CSV) {
                        makeActivePanel(_fileFormatCardName);
                    } else {
                        makeActivePanel(_dataMappingCardName);
                    }
                } else {
                    JOptionPane.showMessageDialog(this, _sourceFile + " could not be opened for reading.", "Note", JOptionPane.INFORMATION_MESSAGE);
                }
            } else if (_showingCard == _fileFormatCardName) {
                makeActivePanel(_dataMappingCardName);
            } else if (_showingCard == _dataMappingCardName) {
                if (checkForRequiredVariables()) return;
                makeActivePanel(_outPutSettingsCardName);
            }
        } catch (Throwable t) {
            new ExceptionDialog(FileSourceWizard.this, t).setVisible(true);
        }
    }
    
    /** Preparation for viewing file format panel. */
    private void prepFileFormatPanel() {
        WaitCursor waitCursor = new WaitCursor(this);
        try {
            readDataFileIntoRawDisplayField();
            if (_inputSourceSettings.isSet()) {
                // define settings given input source settings
                _ignoreRowsTextField.setText(Integer.toString(_inputSourceSettings.getSkiplines()));
                _firstRowColumnHeadersCheckBox.setSelected(_inputSourceSettings.getFirstRowHeader());
                if (_inputSourceSettings.getDelimiter().isEmpty() || _inputSourceSettings.getDelimiter().equalsIgnoreCase(" ")) {
                    _whitespaceRadioButton.setSelected(true);
                } else if (_inputSourceSettings.getDelimiter().equalsIgnoreCase(",")) {
                    _commaRadioButton.setSelected(true);
                } else if (_inputSourceSettings.getDelimiter().equalsIgnoreCase(";")) {
                    _semiColonRadioButton.setSelected(true);
                } else {
                    _otherRadioButton.setSelected(true);
                    _otherFieldSeparatorTextField.setText("" + _inputSourceSettings.getDelimiter().charAt(0));
                }
                if (_inputSourceSettings.getGroup().equalsIgnoreCase("'")) {
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
            _file_selection_label.setText(FileSelectionDialog.getFileTypeAsString(_inputSourceSettings.getInputFileType()) + " File:");
            _source_filename.setText(_sourceFile);
            if (!_inputSourceSettings.isSet()) {
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
            clearInputSettigs.setEnabled(_inputSourceSettings.isSet());
        } finally {
            waitCursor.restore();
        }
    }
    
    /** Preparation for viewing the mapping panel. */
    private void prepMappingPanel() throws Exception {
        WaitCursor waitCursor = new WaitCursor(this);
        try {
            previewSource();
            setMappingTableComboCells();
            enableNavigationButtons();
        } finally {
            waitCursor.restore();
        }
    }
    
    /** Preparation for viewing the output settings panel. */
    private void prepOutputSettingsPanel() {
        if (_inputSourceSettings.getSourceDataFileType() == InputSourceSettings.SourceDataFileType.Excel) {
            // We don't have code yet that allows reading from Excel in the C++ code (caculation engine).
            _execute_import_now.setSelected(true);
            _save_import_settings.setEnabled(false);
        }
        _importTableDataTable.setModel(new DefaultTableModel());
        setInputSourceSettings();
    }
    
    /** Opening source as specified by file type. */
    private void previewSource() throws Exception {
        //set the import tables model to default until we have an instance of the native model avaiable
        _importTableDataTable.setModel(new DefaultTableModel());

        //create the table model
        if (_inputSourceSettings.getSourceDataFileType() == InputSourceSettings.SourceDataFileType.Shapefile) {
            previewSourceAsShapeFile();
        } else if (_inputSourceSettings.getSourceDataFileType() == InputSourceSettings.SourceDataFileType.dBase) {
            previewSourceAsDBaseFile();
        } else if (_inputSourceSettings.getSourceDataFileType() == InputSourceSettings.SourceDataFileType.Excel) {
            previewSourceAsExcelFile();
        } else {
            previewSourceAsCSVFile();
        }
        //now assign model to table object
        if (_previewTableModel != null) {
            _importTableDataTable.setModel(_previewTableModel);
        }

        int widthTotal = 0;
        //calculate the column widths to fit header/data
        Vector<Integer> colWidths = new Vector<Integer>();
        for (int c=0; c < _importTableDataTable.getColumnCount(); ++c) {
            colWidths.add(AutofitTableColumns.getMaxColumnWidth(_importTableDataTable, c, true, 20));
            widthTotal += colWidths.lastElement();
        }
        int additional = Math.max(0, _importTableScrollPane.getViewport().getSize().width - widthTotal - 20/*scrollbar width?*/)/colWidths.size();
        for (int c=0; c < colWidths.size(); ++c) {
            _importTableDataTable.getColumnModel().getColumn(c).setMinWidth(colWidths.elementAt(c) + additional);
        }
    }
    
    /** Opens source file as character delimited source file. */
    private void previewSourceAsCSVFile() throws FileNotFoundException {
        File file = new File(_sourceFile);
        CSVImportDataSource source = new CSVImportDataSource(file, false, '\n', getColumnDelimiter(), getGroupMarker());
        _previewTableModel = new PreviewTableModel(_firstRowColumnHeadersCheckBox.isSelected());
        int skipRows = Integer.parseInt(_ignoreRowsTextField.getText());
        for (int i = 0; i < _previewTableModel.getPreviewLength() + skipRows; ++i) {
            Object[] values = source.readRow();
            if (values != null && i >= skipRows) {
                _previewTableModel.addRow(values);
            }
        }
    }
    
    /** Opens source file as dBase file. */
    private void previewSourceAsDBaseFile() throws Exception {
        try {
            DBaseImportDataSource source = new DBaseImportDataSource(new File(_sourceFile), true);
            _previewTableModel = new PreviewTableModel(true);
            _previewTableModel.addRow(source.getColumnNames());
            for (int i = 0; i < _previewTableModel.getPreviewLength(); ++i) {
                Object[] values = source.readRow();
                if (values != null) {
                    _previewTableModel.addRow(values);
                }
            }
        } catch (Exception e) {
            throw e;
        }
    }
    
    /** Opens source file as dBase file. */
    private void previewSourceAsExcelFile() throws Exception {
        try {
            XLSImportDataSource source = new XLSImportDataSource(new File(_sourceFile), false);
            _previewTableModel = new PreviewTableModel(true);
            for (int i = 0; i < _previewTableModel.getPreviewLength(); ++i) {
                Object[] values = source.readRow();
                if (values != null) {
                    _previewTableModel.addRow(values);
                }
            }
        } catch (Exception e) {
            throw e;
        }
    }
    
    /** Opens source file as shapefile. */
    private void previewSourceAsShapeFile() throws Exception {
        try {
            _previewTableModel = new PreviewTableModel(true);
            File file = new File(_sourceFile);
            if (file.exists()) {
                String supportedType = ShapefileDataSource.isSupportedShapeType(_sourceFile);
                if (supportedType.length() > 0) {
                    throw new DataSourceException(supportedType);
                }
                ShapefileDataSource source = new ShapefileDataSource(file, true);
                _previewTableModel.addRow(source.getColumnNames());
                for (int i = 0; i < _previewTableModel.getPreviewLength(); ++i) {
                    Object[] values = source.readRow();
                    if (values != null) {
                        _previewTableModel.addRow(values);
                    }
                }
            }
        } catch (Exception e) {
            throw e;
        }
    }
    
    /** Attempts to reverse wizard to the previous panel. */    
    public void previousPanel() {
        try {
            if (_showingCard == _fileFormatCardName) {
               makeActivePanel(_startCardName);
            } else if (_showingCard == _dataMappingCardName) {
                switch (_inputSourceSettings.getSourceDataFileType()) {
                    case CSV: makeActivePanel(_fileFormatCardName); break;
                    default : makeActivePanel(_startCardName);
                }
            } else if (_showingCard == _outPutSettingsCardName) {
                makeActivePanel(_dataMappingCardName);
            }
        } catch (Throwable t) {
            new ExceptionDialog(FileSourceWizard.this, t).setVisible(true);
        }
    }
    
    /**Invoked when task's progress property changes. */
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
            fileSample = new FileReader(_sourceFile);
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
        _importVariables.clear();
        _importVariables.addElement(new ImportVariable("Location ID", 0, true, null, null));
        _importVariables.addElement(new ImportVariable("Number of Cases", 1, true, null, null));
        _importVariables.addElement(new ImportVariable("Date/Time", 2, false, null, null));
        _importVariables.addElement(new ImportVariable("Attribute (value)", 3, true, null, null));
        _importVariables.addElement(new ImportVariable("Censored", 4, false, null, null));
        _importVariables.addElement(new ImportVariable("Weight", 4, false, null, null));
        _importVariables.addElement(new ImportVariable("Covariate1", 5, false, null, null));
        _importVariables.addElement(new ImportVariable("Covariate2", 6, false, null, null));
        _importVariables.addElement(new ImportVariable("Covariate3", 7, false, null, null));
        _importVariables.addElement(new ImportVariable("Covariate4", 8, false, null, null));
        _importVariables.addElement(new ImportVariable("Covariate5", 9, false, null, null));
        _importVariables.addElement(new ImportVariable("Covariate6", 10, false, null, null));
        _importVariables.addElement(new ImportVariable("Covariate7", 11, false, null, null));
        _importVariables.addElement(new ImportVariable("Covariate8", 12, false, null, null));
        _importVariables.addElement(new ImportVariable("Covariate9", 13, false, null, null));
        _importVariables.addElement(new ImportVariable("Covariate10", 14, false, null, null));
    }
    
    /** Setup field descriptors for control file. */
    private void setControlFileVariables() {
        _importVariables.clear();
        _importVariables.addElement(new ImportVariable("Location ID", 0, true, null, null));
        _importVariables.addElement(new ImportVariable("Number of Controls", 1, true, null, null));
        _importVariables.addElement(new ImportVariable("Date/Time", 2, false, null, null));
    }
    
    /** Setup field descriptors for coordinates file. */
    private void setGeoFileVariables() {
        _importVariables.clear();
        _importVariables.addElement(new ImportVariable("Location ID", 0, true, null, null));
        _importVariables.addElement(new ImportVariable("Latitude", 1, true, "y-axis", null));
        _importVariables.addElement(new ImportVariable("Longitude", 2, true, "x-axis", null));
        _importVariables.addElement(new ImportVariable("X", 1, true, null, null));
        _importVariables.addElement(new ImportVariable("Y", 2, true, null, null));
        _importVariables.addElement(new ImportVariable("Z1", 3, false, null, null));
        _importVariables.addElement(new ImportVariable("Z2", 4, false, null, null));
        _importVariables.addElement(new ImportVariable("Z3", 5, false, null, null));
        _importVariables.addElement(new ImportVariable("Z4", 6, false, null, null));
        _importVariables.addElement(new ImportVariable("Z5", 7, false, null, null));
        _importVariables.addElement(new ImportVariable("Z6", 8, false, null, null));
        _importVariables.addElement(new ImportVariable("Z7", 9, false, null, null));
        _importVariables.addElement(new ImportVariable("Z8", 10, false, null, null));
    }
    
    /** Setup field descriptors for special grid file. */
    private void setGridFileVariables() {
        _importVariables.clear();
        _importVariables.addElement(new ImportVariable("Latitude", 0, true, "y-axis", null));
        _importVariables.addElement(new ImportVariable("Longitude", 1, true, "x-axis", null));
        _importVariables.addElement(new ImportVariable("X", 0, true, null, null));
        _importVariables.addElement(new ImportVariable("Y", 1, true, null, null));
        _importVariables.addElement(new ImportVariable("Z1", 2, false, null, null));
        _importVariables.addElement(new ImportVariable("Z2", 3, false, null, null));
        _importVariables.addElement(new ImportVariable("Z3", 4, false, null, null));
        _importVariables.addElement(new ImportVariable("Z4", 5, false, null, null));
        _importVariables.addElement(new ImportVariable("Z5", 6, false, null, null));
        _importVariables.addElement(new ImportVariable("Z6", 7, false, null, null));
        _importVariables.addElement(new ImportVariable("Z7", 8, false, null, null));
        _importVariables.addElement(new ImportVariable("Z8", 9, false, null, null));
    }
    
    /** Sets InputSourceSettings object from user selections in wizard. */
    private void setInputSourceSettings() {
        if (_inputSourceSettings.getSourceDataFileType() == InputSourceSettings.SourceDataFileType.CSV) {
            _inputSourceSettings.setDelimiter(Character.toString(getColumnDelimiter()));
            _inputSourceSettings.setGroup(Character.toString(getGroupMarker()));
            _inputSourceSettings.setSkiplines(Integer.parseInt(_ignoreRowsTextField.getText()));
            _inputSourceSettings.setFirstRowHeader(_firstRowColumnHeadersCheckBox.isSelected());
        }
        _inputSourceSettings.getFieldMaps().clear();
        for (int t=0; t < _importVariables.size(); ++t) {
            if (_importVariables.get(t).getShowing()) {
                if (_importVariables.get(t).getIsMappedToInputFileVariable() || _importVariables.get(t).getDefault() != null) {
                    _inputSourceSettings.getFieldMaps().add(Integer.toString(_importVariables.get(t).getInputFileVariableIndex()));
                } else {
                    _inputSourceSettings.getFieldMaps().add("");
                }
            }
        }
        for (int i=_inputSourceSettings.getFieldMaps().size() - 1; i >= 0; --i) {
            if (!_inputSourceSettings.getFieldMaps().get(i).isEmpty()) break;
            else _inputSourceSettings.getFieldMaps().remove(i);
        }
    }
    
    /** Assigns the field name choices in the drop-down menu of the variable mapping table. */
    private void setMappingTableComboCells() {
        VariableMappingTableModel model = (VariableMappingTableModel) _fieldMapTable.getModel();
        model.comboBox.removeAllItems();
        model.comboBox.addItem(_unAssignedVariableName);
        for (int i = 0; i < _importTableDataTable.getModel().getColumnCount(); ++i) {
            model.comboBox.addItem(_importTableDataTable.getModel().getColumnName(i));
        }
        _fieldMapTable.getColumnModel().getColumn(1).setCellEditor(new DefaultCellEditor(model.comboBox));
        model.fireTableDataChanged();
    }
    
    /** Setup field descriptors for maximum circle population file. */
    private void setMaxCirclePopFileVariables() {
        _importVariables.clear();
        _importVariables.addElement(new ImportVariable("Location ID", 0, true, null, null));
        _importVariables.addElement(new ImportVariable("Population", 1, true, null, null));
    }
    
    /** Setup field descriptors for population file. */
    private void setPopulationFileVariables() {
        _importVariables.clear();
        _importVariables.addElement(new ImportVariable("Location ID", 0, true, null, null));
        ImportVariable variable = new ImportVariable("Date/Time", 1, false, null, "unspecified");
        /* Set the variable index to below the one-based variables -- variables less than one
         * are considered special and are not actually a data source column option.
         * In this case, the population date will be set to 'unspecified'. */
        variable.setInputFileVariableIndex(0);
        _importVariables.addElement(variable);
        _importVariables.addElement(new ImportVariable("Population", 2, true, null, null));
        _importVariables.addElement(new ImportVariable("Covariate1", 3, false, null, null));
        _importVariables.addElement(new ImportVariable("Covariate2", 4, false, null, null));
        _importVariables.addElement(new ImportVariable("Covariate3", 5, false, null, null));
        _importVariables.addElement(new ImportVariable("Covariate4", 6, false, null, null));
        _importVariables.addElement(new ImportVariable("Covariate5", 7, false, null, null));
        _importVariables.addElement(new ImportVariable("Covariate6", 8, false, null, null));
        _importVariables.addElement(new ImportVariable("Covariate7", 9, false, null, null));
        _importVariables.addElement(new ImportVariable("Covariate8", 10, false, null, null));
        _importVariables.addElement(new ImportVariable("Covariate9", 11, false, null, null));
        _importVariables.addElement(new ImportVariable("Covariate10", 12, false, null, null));
    }
    
    /** Setup field descriptors for relative risks file. */
    private void setRelativeRisksFileVariables() {
        _importVariables.clear();
        _importVariables.addElement(new ImportVariable("Location ID", 0, true, null, null));
        _importVariables.addElement(new ImportVariable("Relative Risk", 1, true, null, null));
        _importVariables.addElement(new ImportVariable("Start Date", 2, false, null, null));
        _importVariables.addElement(new ImportVariable("End Date", 3, false, null, null));
    }
    
    /** Shows/hides variables based upon destination file type and model/coordinates type. */
    private void setShowingVariables() {
        VariableMappingTableModel model = (VariableMappingTableModel) _fieldMapTable.getModel();
        Parameters.ProbabilityModelType _modelType = getModelControlType();
        Parameters.CoordinatesType _coordindatesType = getCoorinatesControlType();

        switch (_inputSourceSettings.getInputFileType()) {
            case Case:
                model.hideAll();
                for (int t = 0; t < _importVariables.size(); ++t) {
                    _importVariables.get(t).setShowing(false);
                }
                for (int t = 0; t < _importVariables.size(); ++t) {
                    if (t >= 1 && t <= 2) {//show '# cases' and 'date time'  variables
                        _importVariables.get(t).setShowing(true);
                        model.setShowing(_importVariables.get(t));
                    } else if (t >= 6 && t <= 15) {//show 'covariate' variables for Poisson,space-time permutation and normal models only
                        _importVariables.get(t).setShowing(_modelType == Parameters.ProbabilityModelType.POISSON ||
                                                           _modelType == Parameters.ProbabilityModelType.SPACETIMEPERMUTATION ||
                                                           _modelType == Parameters.ProbabilityModelType.NORMAL);
                        model.setShowing(_importVariables.get(t));
                    } else if (t == 3) { //show 'attribute' variable for ordinal, exponential, normal and rank models only
                        _importVariables.get(t).setShowing(_modelType == Parameters.ProbabilityModelType.ORDINAL ||
                                                           _modelType == Parameters.ProbabilityModelType.CATEGORICAL ||
                                                           _modelType == Parameters.ProbabilityModelType.EXPONENTIAL ||
                                                           _modelType == Parameters.ProbabilityModelType.NORMAL ||
                                                           _modelType == Parameters.ProbabilityModelType.RANK);
                        model.setShowing(_importVariables.get(t));
                    } else if (t == 4) { //show 'censored' variable for exponential model only
                        _importVariables.get(t).setShowing(_modelType == Parameters.ProbabilityModelType.EXPONENTIAL);
                        model.setShowing(_importVariables.get(t));
                    } else if (t == 5) { //show 'weight' variable for normal model only
                        _importVariables.get(t).setShowing(_modelType == Parameters.ProbabilityModelType.NORMAL);
                        model.setShowing(_importVariables.get(t));
                    } else { //default - show variable
                        _importVariables.get(t).setShowing(true);
                        model.setShowing(_importVariables.get(t));
                    }
                } break;
            case Coordinates:
                for (int t = 0; t < _importVariables.size(); ++t) {
                    if (t == 1 || t == 2) { //show 'lat/long' variables for lat/long system
                        _importVariables.get(t).setShowing(_coordindatesType == Parameters.CoordinatesType.LATLON);
                        model.setShowing(_importVariables.get(t));
                    } else if (t >= 3) { //show 'X/Y/Zn' variables for Cartesian system
                        _importVariables.get(t).setShowing(_coordindatesType == Parameters.CoordinatesType.CARTESIAN);
                        model.setShowing(_importVariables.get(t));
                    } else {
                        _importVariables.get(t).setShowing(true);
                        model.setShowing(_importVariables.get(t));
                    }
                } break;
            case SpecialGrid:
                for (int t = 0; t < _importVariables.size(); ++t) {
                    if (t == 0 || t == 1) { //show 'lat/long' variables for lat/long system
                        _importVariables.get(t).setShowing(_coordindatesType == Parameters.CoordinatesType.LATLON);
                        model.setShowing(_importVariables.get(t));
                    } else { //show 'X/Y/Zn' variables for Cartesian system
                        _importVariables.get(t).setShowing(_coordindatesType == Parameters.CoordinatesType.CARTESIAN);
                        model.setShowing(_importVariables.get(t));
                    }
                } break;
            default:
                for (ImportVariable variable : _importVariables) {
                    variable.setShowing(true);
                    model.setShowing(variable);
                }
        }
        // ensure that the input variable index values are sequential
        int showingIdx=0;
        for (ImportVariable variable : _importVariables) {
            if (variable.getShowing()) {
                variable.setTargetFieldIndex(showingIdx);
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
                _basePanel.removeAll();
                _basePanel.add(_fileSourceSettingsPanel, _startCardName);
                if (!isImportableFileType(_inputSourceSettings.getInputFileType())) {
                    // Some input files do not support importing. Only show the start panel.
                    nextButtonSource.setVisible(false);
                } else {
                    _basePanel.add(_fileFormatPanel, _fileFormatCardName);
                    _basePanel.add(_dataMappingPanel, _dataMappingCardName);
                    _basePanel.add(_outputSettingsPanel, _outPutSettingsCardName);
                }
                makeActivePanel(_startCardName);
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
        _basePanel = new javax.swing.JPanel();
        _fileFormatPanel = new javax.swing.JPanel();
        jPanel1 = new javax.swing.JPanel();
        jLabel1 = new javax.swing.JLabel();
        jScrollPane1 = new javax.swing.JScrollPane();
        _fileContentsTextArea = new javax.swing.JTextArea();
        jLabel4 = new javax.swing.JLabel();
        _ignoreRowsTextField = new javax.swing.JTextField();
        jLabel5 = new javax.swing.JLabel();
        _firstRowColumnHeadersCheckBox = new javax.swing.JCheckBox();
        _sourceFileTypeOptions = new javax.swing.JPanel();
        _cSVDefsPanel = new javax.swing.JPanel();
        fieldSeparatorGroup = new javax.swing.JPanel();
        _commaRadioButton = new javax.swing.JRadioButton();
        _semiColonRadioButton = new javax.swing.JRadioButton();
        _whitespaceRadioButton = new javax.swing.JRadioButton();
        _otherRadioButton = new javax.swing.JRadioButton();
        _otherFieldSeparatorTextField = new javax.swing.JTextField();
        _groupIndiocatorGroup = new javax.swing.JPanel();
        _doubleQuotesRadioButton = new javax.swing.JRadioButton();
        _singleQuotesRadioButton = new javax.swing.JRadioButton();
        _fileFormatButtonPanel = new javax.swing.JPanel();
        previousButtonCSV = new javax.swing.JButton();
        nextButtonCSV = new javax.swing.JButton();
        _dataMappingPanel = new javax.swing.JPanel();
        jPanel3 = new javax.swing.JPanel();
        jSplitPane1 = new javax.swing.JSplitPane();
        _dataMappingTopPanel = new javax.swing.JPanel();
        _displayVariablesLabel = new javax.swing.JLabel();
        _displayVariablesComboBox = new javax.swing.JComboBox();
        jScrollPane2 = new javax.swing.JScrollPane();
        _fieldMapTable = new javax.swing.JTable();
        _clearSelectionButton = new javax.swing.JButton();
        _dataMappingBottomPanel = new javax.swing.JPanel();
        _importTableScrollPane = new javax.swing.JScrollPane();
        _importTableDataTable = new javax.swing.JTable();
        _fileSourceButtonPanel4 = new javax.swing.JPanel();
        previousButtonMapping = new javax.swing.JButton();
        nextButtonMapping = new javax.swing.JButton();
        _outputSettingsPanel = new javax.swing.JPanel();
        _fileSourceButtonPanel5 = new javax.swing.JPanel();
        previousButtonOutSettings = new javax.swing.JButton();
        executeButton = new javax.swing.JButton();
        cancelButton = new javax.swing.JButton();
        jPanel4 = new javax.swing.JPanel();
        _outputDirectoryTextField = new javax.swing.JTextField();
        _changeSaveDirectoryButton = new javax.swing.JButton();
        _progressBar = new javax.swing.JProgressBar();
        _execute_import_now = new javax.swing.JRadioButton();
        _save_import_settings = new javax.swing.JRadioButton();
        _fileSourceSettingsPanel = new javax.swing.JPanel();
        _fileSourceButtonPanel = new javax.swing.JPanel();
        acceptButton = new javax.swing.JButton();
        nextButtonSource = new javax.swing.JButton();
        clearInputSettigs = new javax.swing.JButton();
        jPanel2 = new javax.swing.JPanel();
        _file_selection_label = new javax.swing.JLabel();
        _source_filename = new javax.swing.JTextField();
        _expectedFormatScrollPane = new javax.swing.JScrollPane();
        _expectedFormatTextPane = new javax.swing.JTextPane();
        _browse_source = new javax.swing.JButton();

        _fieldSeparatorButtonGroup.add(_commaRadioButton);
        _fieldSeparatorButtonGroup.add(_semiColonRadioButton);
        _fieldSeparatorButtonGroup.add(_whitespaceRadioButton);
        _fieldSeparatorButtonGroup.add(_otherRadioButton);

        _groupIndicatorButtonGroup.add(_doubleQuotesRadioButton);
        _groupIndicatorButtonGroup.add(_singleQuotesRadioButton);

        setDefaultCloseOperation(javax.swing.WindowConstants.DISPOSE_ON_CLOSE);
        setTitle("File Wizard"); // NOI18N
        setModal(true);
        setResizable(false);

        _basePanel.setLayout(new java.awt.CardLayout());

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

        _sourceFileTypeOptions.setLayout(new java.awt.CardLayout());

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
                .addContainerGap(223, Short.MAX_VALUE))
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

        javax.swing.GroupLayout _cSVDefsPanelLayout = new javax.swing.GroupLayout(_cSVDefsPanel);
        _cSVDefsPanel.setLayout(_cSVDefsPanelLayout);
        _cSVDefsPanelLayout.setHorizontalGroup(
            _cSVDefsPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addComponent(fieldSeparatorGroup, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
            .addComponent(_groupIndiocatorGroup, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
        );
        _cSVDefsPanelLayout.setVerticalGroup(
            _cSVDefsPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_cSVDefsPanelLayout.createSequentialGroup()
                .addComponent(fieldSeparatorGroup, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_groupIndiocatorGroup, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap(20, Short.MAX_VALUE))
        );

        _sourceFileTypeOptions.add(_cSVDefsPanel, "cvsPanel");

        javax.swing.GroupLayout jPanel1Layout = new javax.swing.GroupLayout(jPanel1);
        jPanel1.setLayout(jPanel1Layout);
        jPanel1Layout.setHorizontalGroup(
            jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel1Layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(_sourceFileTypeOptions, javax.swing.GroupLayout.PREFERRED_SIZE, 0, Short.MAX_VALUE)
                    .addGroup(jPanel1Layout.createSequentialGroup()
                        .addComponent(jLabel4)
                        .addGap(5, 5, 5)
                        .addComponent(_ignoreRowsTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 42, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addGap(5, 5, 5)
                        .addComponent(jLabel5)
                        .addGap(18, 18, 18)
                        .addComponent(_firstRowColumnHeadersCheckBox, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                        .addGap(164, 164, 164))
                    .addComponent(jLabel1)
                    .addComponent(jScrollPane1, javax.swing.GroupLayout.Alignment.TRAILING))
                .addContainerGap())
        );
        jPanel1Layout.setVerticalGroup(
            jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel1Layout.createSequentialGroup()
                .addContainerGap()
                .addComponent(jLabel1)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(jScrollPane1, javax.swing.GroupLayout.PREFERRED_SIZE, 137, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .addGroup(jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(jLabel4)
                    .addComponent(jLabel5)
                    .addComponent(_ignoreRowsTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(_firstRowColumnHeadersCheckBox))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_sourceFileTypeOptions, javax.swing.GroupLayout.PREFERRED_SIZE, 135, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addGap(61, 61, 61))
        );

        _fileFormatButtonPanel.setPreferredSize(new java.awt.Dimension(438, 40));

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

        javax.swing.GroupLayout _fileFormatButtonPanelLayout = new javax.swing.GroupLayout(_fileFormatButtonPanel);
        _fileFormatButtonPanel.setLayout(_fileFormatButtonPanelLayout);
        _fileFormatButtonPanelLayout.setHorizontalGroup(
            _fileFormatButtonPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_fileFormatButtonPanelLayout.createSequentialGroup()
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .addComponent(previousButtonCSV, javax.swing.GroupLayout.PREFERRED_SIZE, 100, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(nextButtonCSV, javax.swing.GroupLayout.PREFERRED_SIZE, 100, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap())
        );
        _fileFormatButtonPanelLayout.setVerticalGroup(
            _fileFormatButtonPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_fileFormatButtonPanelLayout.createSequentialGroup()
                .addGroup(_fileFormatButtonPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(previousButtonCSV)
                    .addComponent(nextButtonCSV))
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );

        javax.swing.GroupLayout _fileFormatPanelLayout = new javax.swing.GroupLayout(_fileFormatPanel);
        _fileFormatPanel.setLayout(_fileFormatPanelLayout);
        _fileFormatPanelLayout.setHorizontalGroup(
            _fileFormatPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addComponent(jPanel1, javax.swing.GroupLayout.Alignment.TRAILING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
            .addComponent(_fileFormatButtonPanel, javax.swing.GroupLayout.DEFAULT_SIZE, 587, Short.MAX_VALUE)
        );
        _fileFormatPanelLayout.setVerticalGroup(
            _fileFormatPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_fileFormatPanelLayout.createSequentialGroup()
                .addComponent(jPanel1, javax.swing.GroupLayout.PREFERRED_SIZE, 342, Short.MAX_VALUE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_fileFormatButtonPanel, javax.swing.GroupLayout.PREFERRED_SIZE, 34, javax.swing.GroupLayout.PREFERRED_SIZE))
        );

        _basePanel.add(_fileFormatPanel, "File Format");

        jSplitPane1.setDividerLocation(175);
        jSplitPane1.setOrientation(javax.swing.JSplitPane.VERTICAL_SPLIT);
        jSplitPane1.setBorder(null);

        _displayVariablesLabel.setText("Display SaTScan Variables For:"); // NOI18N

        _displayVariablesComboBox.setModel(new javax.swing.DefaultComboBoxModel(new String[] { "Item 1", "Item 2", "Item 3", "Item 4" }));
        _displayVariablesComboBox.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                if (_inputSourceSettings.getInputFileType() == InputSourceSettings.InputFileType.Coordinates || _inputSourceSettings.getInputFileType() == InputSourceSettings.InputFileType.SpecialGrid) {
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

        _fieldMapTable.setRowSelectionAllowed(false);
        _fieldMapTable.setModel(new VariableMappingTableModel(_importVariables));
        jScrollPane2.setViewportView(_fieldMapTable);

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
                        .addComponent(_displayVariablesComboBox, 0, 353, Short.MAX_VALUE))
                    .addComponent(jScrollPane2, javax.swing.GroupLayout.DEFAULT_SIZE, 502, Short.MAX_VALUE))
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
                    .addComponent(jScrollPane2, javax.swing.GroupLayout.DEFAULT_SIZE, 137, Short.MAX_VALUE))
                .addContainerGap())
        );

        jSplitPane1.setTopComponent(_dataMappingTopPanel);

        _importTableDataTable.setModel(new javax.swing.table.DefaultTableModel(
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
        _importTableDataTable.setAutoResizeMode(javax.swing.JTable.AUTO_RESIZE_OFF);
        _importTableDataTable.setRowSelectionAllowed(false);
        _importTableScrollPane.setViewportView(_importTableDataTable);

        javax.swing.GroupLayout _dataMappingBottomPanelLayout = new javax.swing.GroupLayout(_dataMappingBottomPanel);
        _dataMappingBottomPanel.setLayout(_dataMappingBottomPanelLayout);
        _dataMappingBottomPanelLayout.setHorizontalGroup(
            _dataMappingBottomPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addComponent(_importTableScrollPane, javax.swing.GroupLayout.DEFAULT_SIZE, 565, Short.MAX_VALUE)
        );
        _dataMappingBottomPanelLayout.setVerticalGroup(
            _dataMappingBottomPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addComponent(_importTableScrollPane, javax.swing.GroupLayout.DEFAULT_SIZE, 139, Short.MAX_VALUE)
        );

        jSplitPane1.setRightComponent(_dataMappingBottomPanel);

        javax.swing.GroupLayout jPanel3Layout = new javax.swing.GroupLayout(jPanel3);
        jPanel3.setLayout(jPanel3Layout);
        jPanel3Layout.setHorizontalGroup(
            jPanel3Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel3Layout.createSequentialGroup()
                .addContainerGap()
                .addComponent(jSplitPane1, javax.swing.GroupLayout.PREFERRED_SIZE, 0, Short.MAX_VALUE)
                .addContainerGap())
        );
        jPanel3Layout.setVerticalGroup(
            jPanel3Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel3Layout.createSequentialGroup()
                .addContainerGap()
                .addComponent(jSplitPane1)
                .addContainerGap())
        );

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

        javax.swing.GroupLayout _fileSourceButtonPanel4Layout = new javax.swing.GroupLayout(_fileSourceButtonPanel4);
        _fileSourceButtonPanel4.setLayout(_fileSourceButtonPanel4Layout);
        _fileSourceButtonPanel4Layout.setHorizontalGroup(
            _fileSourceButtonPanel4Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, _fileSourceButtonPanel4Layout.createSequentialGroup()
                .addContainerGap(371, Short.MAX_VALUE)
                .addComponent(previousButtonMapping, javax.swing.GroupLayout.PREFERRED_SIZE, 100, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(nextButtonMapping, javax.swing.GroupLayout.PREFERRED_SIZE, 100, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap())
        );
        _fileSourceButtonPanel4Layout.setVerticalGroup(
            _fileSourceButtonPanel4Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_fileSourceButtonPanel4Layout.createSequentialGroup()
                .addGroup(_fileSourceButtonPanel4Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(previousButtonMapping)
                    .addComponent(nextButtonMapping))
                .addGap(0, 11, Short.MAX_VALUE))
        );

        javax.swing.GroupLayout _dataMappingPanelLayout = new javax.swing.GroupLayout(_dataMappingPanel);
        _dataMappingPanel.setLayout(_dataMappingPanelLayout);
        _dataMappingPanelLayout.setHorizontalGroup(
            _dataMappingPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addComponent(jPanel3, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
            .addComponent(_fileSourceButtonPanel4, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
        );
        _dataMappingPanelLayout.setVerticalGroup(
            _dataMappingPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_dataMappingPanelLayout.createSequentialGroup()
                .addComponent(jPanel3, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_fileSourceButtonPanel4, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
        );

        _basePanel.add(_dataMappingPanel, "Mapping Panel");

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
                        setVisible(false);
                    }
                } catch (Throwable t) {
                    new ExceptionDialog(FileSourceWizard.this, t).setVisible(true);
                }
            }
        });

        cancelButton.setText("Cancel");

        javax.swing.GroupLayout _fileSourceButtonPanel5Layout = new javax.swing.GroupLayout(_fileSourceButtonPanel5);
        _fileSourceButtonPanel5.setLayout(_fileSourceButtonPanel5Layout);
        _fileSourceButtonPanel5Layout.setHorizontalGroup(
            _fileSourceButtonPanel5Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, _fileSourceButtonPanel5Layout.createSequentialGroup()
                .addContainerGap()
                .addComponent(cancelButton, javax.swing.GroupLayout.PREFERRED_SIZE, 85, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .addComponent(previousButtonOutSettings, javax.swing.GroupLayout.PREFERRED_SIZE, 100, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(executeButton, javax.swing.GroupLayout.PREFERRED_SIZE, 100, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap())
        );
        _fileSourceButtonPanel5Layout.setVerticalGroup(
            _fileSourceButtonPanel5Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_fileSourceButtonPanel5Layout.createSequentialGroup()
                .addGroup(_fileSourceButtonPanel5Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(previousButtonOutSettings)
                    .addComponent(executeButton)
                    .addComponent(cancelButton))
                .addGap(0, 11, Short.MAX_VALUE))
        );

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

        _import_operation_buttonGroup.add(_execute_import_now);
        _execute_import_now.setSelected(true);
        _execute_import_now.setText("Save imported input file as:");
        _execute_import_now.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED)
                executeButton.setText("Import");
            }
        });

        _import_operation_buttonGroup.add(_save_import_settings);
        _save_import_settings.setText("Save these settings and read directly from file source when running the analysis.");
        _save_import_settings.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED)
                executeButton.setText("Ok");
            }
        });

        javax.swing.GroupLayout jPanel4Layout = new javax.swing.GroupLayout(jPanel4);
        jPanel4.setLayout(jPanel4Layout);
        jPanel4Layout.setHorizontalGroup(
            jPanel4Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel4Layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(jPanel4Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(jPanel4Layout.createSequentialGroup()
                        .addGap(15, 15, 15)
                        .addComponent(_outputDirectoryTextField)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(_changeSaveDirectoryButton))
                    .addComponent(_save_import_settings, javax.swing.GroupLayout.Alignment.TRAILING, javax.swing.GroupLayout.DEFAULT_SIZE, 566, Short.MAX_VALUE)
                    .addComponent(_execute_import_now, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(_progressBar, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                .addContainerGap())
        );
        jPanel4Layout.setVerticalGroup(
            jPanel4Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel4Layout.createSequentialGroup()
                .addGap(22, 22, 22)
                .addComponent(_execute_import_now)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel4Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING)
                    .addComponent(_outputDirectoryTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(_changeSaveDirectoryButton))
                .addGap(18, 18, 18)
                .addComponent(_save_import_settings)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED, 206, Short.MAX_VALUE)
                .addComponent(_progressBar, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap())
        );

        javax.swing.GroupLayout _outputSettingsPanelLayout = new javax.swing.GroupLayout(_outputSettingsPanel);
        _outputSettingsPanel.setLayout(_outputSettingsPanelLayout);
        _outputSettingsPanelLayout.setHorizontalGroup(
            _outputSettingsPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_outputSettingsPanelLayout.createSequentialGroup()
                .addComponent(jPanel4, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addGap(0, 1, Short.MAX_VALUE))
            .addComponent(_fileSourceButtonPanel5, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
        );
        _outputSettingsPanelLayout.setVerticalGroup(
            _outputSettingsPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_outputSettingsPanelLayout.createSequentialGroup()
                .addComponent(jPanel4, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_fileSourceButtonPanel5, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
        );

        _basePanel.add(_outputSettingsPanel, "Output Settings");

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
                _inputSourceSettings.reset();
                clearInputSettigs.setEnabled(_inputSourceSettings.isSet());
                clearSaTScanVariableFieldIndexes();
                _execute_import_now.setSelected(true);
                _needs_import_save = true;
                _expectedFormatTextPane.setText(getFileExpectedFormatHtml());
                _expectedFormatTextPane.setCaretPosition(0);
                nextButtonSource.setText("Next >");
                acceptButton.requestFocus();
            }
        });

        javax.swing.GroupLayout _fileSourceButtonPanelLayout = new javax.swing.GroupLayout(_fileSourceButtonPanel);
        _fileSourceButtonPanel.setLayout(_fileSourceButtonPanelLayout);
        _fileSourceButtonPanelLayout.setHorizontalGroup(
            _fileSourceButtonPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_fileSourceButtonPanelLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(acceptButton, javax.swing.GroupLayout.PREFERRED_SIZE, 100, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED, 249, Short.MAX_VALUE)
                .addComponent(clearInputSettigs, javax.swing.GroupLayout.PREFERRED_SIZE, 112, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(nextButtonSource, javax.swing.GroupLayout.PREFERRED_SIZE, 100, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap())
        );
        _fileSourceButtonPanelLayout.setVerticalGroup(
            _fileSourceButtonPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_fileSourceButtonPanelLayout.createSequentialGroup()
                .addGroup(_fileSourceButtonPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(acceptButton)
                    .addComponent(nextButtonSource)
                    .addComponent(clearInputSettigs))
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );

        _file_selection_label.setFont(new java.awt.Font("Tahoma", 0, 12)); // NOI18N
        _file_selection_label.setText("Case File:");

        _expectedFormatScrollPane.setHorizontalScrollBarPolicy(javax.swing.ScrollPaneConstants.HORIZONTAL_SCROLLBAR_NEVER);
        _expectedFormatScrollPane.setBorder(null);

        _expectedFormatTextPane.setEditable(false);
        _expectedFormatTextPane.setBackground(new java.awt.Color(240, 240, 240));
        _expectedFormatTextPane.setContentType("text/html"); // NOI18N
        _expectedFormatTextPane.setText("<html>\r\n  <head>\r</head>\r\n  <body>\r\n    <p style=\"margin-top: 0;\">\r\rThe expected format of the case file, for the current settings, is:</p>\r\n    <p style=\"margin: 5px 0 0 5px;font-style:italic;font-size:1.01em;font-weight:bold;\">&lt;indentifier&gt;  &lt;count&gt; &lt;date&gt; &lt;covariate 1&gt; ... &lt;covariate N&gt;</p>\n  </body>\r\n</html>\r\n");
        _expectedFormatScrollPane.setViewportView(_expectedFormatTextPane);

        _browse_source.setText("..."); // NOI18N
        _browse_source.setToolTipText("Browse for source file ..."); // NOI18N
        _browse_source.setPreferredSize(new java.awt.Dimension(45, 20));
        _browse_source.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                FileSelectionDialog selectionDialog = new FileSelectionDialog(SaTScanApplication.getInstance(), _inputSourceSettings.getInputFileType(), SaTScanApplication.getInstance().lastBrowseDirectory);
                File file = selectionDialog.browse_load(true);
                if (file != null) {
                    SaTScanApplication.getInstance().lastBrowseDirectory = selectionDialog.getDirectory();
                    _source_filename.setText(file.getAbsolutePath());
                }
            }
        });

        javax.swing.GroupLayout jPanel2Layout = new javax.swing.GroupLayout(jPanel2);
        jPanel2.setLayout(jPanel2Layout);
        jPanel2Layout.setHorizontalGroup(
            jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel2Layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(_expectedFormatScrollPane, javax.swing.GroupLayout.Alignment.TRAILING)
                    .addGroup(jPanel2Layout.createSequentialGroup()
                        .addComponent(_source_filename)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(_browse_source, javax.swing.GroupLayout.PREFERRED_SIZE, 25, javax.swing.GroupLayout.PREFERRED_SIZE))
                    .addComponent(_file_selection_label, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                .addContainerGap())
        );
        jPanel2Layout.setVerticalGroup(
            jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel2Layout.createSequentialGroup()
                .addContainerGap()
                .addComponent(_file_selection_label)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(_source_filename, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(_browse_source, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_expectedFormatScrollPane, javax.swing.GroupLayout.DEFAULT_SIZE, 273, Short.MAX_VALUE)
                .addContainerGap())
        );

        javax.swing.GroupLayout _fileSourceSettingsPanelLayout = new javax.swing.GroupLayout(_fileSourceSettingsPanel);
        _fileSourceSettingsPanel.setLayout(_fileSourceSettingsPanelLayout);
        _fileSourceSettingsPanelLayout.setHorizontalGroup(
            _fileSourceSettingsPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addComponent(jPanel2, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
            .addComponent(_fileSourceButtonPanel, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
        );
        _fileSourceSettingsPanelLayout.setVerticalGroup(
            _fileSourceSettingsPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, _fileSourceSettingsPanelLayout.createSequentialGroup()
                .addComponent(jPanel2, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_fileSourceButtonPanel, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
        );

        _basePanel.add(_fileSourceSettingsPanel, "Source Settings");

        javax.swing.GroupLayout layout = new javax.swing.GroupLayout(getContentPane());
        getContentPane().setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addComponent(_basePanel, javax.swing.GroupLayout.PREFERRED_SIZE, 587, javax.swing.GroupLayout.PREFERRED_SIZE)
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addComponent(_basePanel, javax.swing.GroupLayout.PREFERRED_SIZE, 382, javax.swing.GroupLayout.PREFERRED_SIZE)
        );

        pack();
    }// </editor-fold>//GEN-END:initComponents
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JPanel _basePanel;
    private javax.swing.JButton _browse_source;
    private javax.swing.JPanel _cSVDefsPanel;
    private javax.swing.JButton _changeSaveDirectoryButton;
    private javax.swing.JButton _clearSelectionButton;
    private javax.swing.JRadioButton _commaRadioButton;
    private javax.swing.JPanel _dataMappingBottomPanel;
    private javax.swing.JPanel _dataMappingPanel;
    private javax.swing.JPanel _dataMappingTopPanel;
    private javax.swing.JComboBox _displayVariablesComboBox;
    private javax.swing.JLabel _displayVariablesLabel;
    private javax.swing.JRadioButton _doubleQuotesRadioButton;
    private javax.swing.JRadioButton _execute_import_now;
    private javax.swing.JScrollPane _expectedFormatScrollPane;
    private javax.swing.JTextPane _expectedFormatTextPane;
    private javax.swing.JTable _fieldMapTable;
    private javax.swing.ButtonGroup _fieldSeparatorButtonGroup;
    private javax.swing.JTextArea _fileContentsTextArea;
    private javax.swing.JPanel _fileFormatButtonPanel;
    private javax.swing.JPanel _fileFormatPanel;
    private javax.swing.JPanel _fileSourceButtonPanel;
    private javax.swing.JPanel _fileSourceButtonPanel4;
    private javax.swing.JPanel _fileSourceButtonPanel5;
    private javax.swing.JPanel _fileSourceSettingsPanel;
    private javax.swing.JLabel _file_selection_label;
    private javax.swing.JCheckBox _firstRowColumnHeadersCheckBox;
    private javax.swing.ButtonGroup _groupIndicatorButtonGroup;
    private javax.swing.JPanel _groupIndiocatorGroup;
    private javax.swing.JTextField _ignoreRowsTextField;
    private javax.swing.JTable _importTableDataTable;
    private javax.swing.JScrollPane _importTableScrollPane;
    private javax.swing.ButtonGroup _import_operation_buttonGroup;
    private javax.swing.JTextField _otherFieldSeparatorTextField;
    private javax.swing.JRadioButton _otherRadioButton;
    private javax.swing.JTextField _outputDirectoryTextField;
    private javax.swing.JPanel _outputSettingsPanel;
    private javax.swing.JProgressBar _progressBar;
    private javax.swing.ButtonGroup _projectionButtonGroup;
    private javax.swing.JRadioButton _save_import_settings;
    private javax.swing.JRadioButton _semiColonRadioButton;
    private javax.swing.JRadioButton _singleQuotesRadioButton;
    private javax.swing.JPanel _sourceFileTypeOptions;
    private javax.swing.JTextField _source_filename;
    private javax.swing.JRadioButton _whitespaceRadioButton;
    private javax.swing.JButton acceptButton;
    private javax.swing.JButton cancelButton;
    private javax.swing.JButton clearInputSettigs;
    private javax.swing.JButton executeButton;
    private javax.swing.JPanel fieldSeparatorGroup;
    private javax.swing.JLabel jLabel1;
    private javax.swing.JLabel jLabel4;
    private javax.swing.JLabel jLabel5;
    private javax.swing.JPanel jPanel1;
    private javax.swing.JPanel jPanel2;
    private javax.swing.JPanel jPanel3;
    private javax.swing.JPanel jPanel4;
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
    /**
     *
     */
    class VariableMappingTableModel extends AbstractTableModel {

        private static final long serialVersionUID = 1L;
        private String[] columnNames = {"SaTScan Variable", "Source File Variable"};
        private final Vector<ImportVariable> variables_static;
        private Vector<ImportVariable> variables_visible;
        public JComboBox comboBox = new JComboBox();

        public VariableMappingTableModel(Vector<ImportVariable> variables) {
            this.variables_static = variables;
            variables_visible = (Vector) this.variables_static.clone();
        }

        public int getColumnCount() {
            return columnNames.length;
        }

        public int getRowCount() {
            return variables_visible.size();
        }

        @Override
        public Class getColumnClass(int c) {
            return getValueAt(0, c).getClass();
        }

        @Override
        public String getColumnName(int col) {
            return columnNames[col];
        }

        public Object getValueAt(int row, int col) {
            if (col == 0) {
                return variables_visible.get(row).getVariableDisplayName();
            } else {
                Object obj = comboBox.getItemAt(variables_visible.get(row).getInputFileVariableIndex());
                if (obj != null) {
                    return obj;
                } else {
                    return "";
                }
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
            for (int i = 0; i < variables_visible.size(); ++i) {
                if (variableName.equals(variables_visible.get(i).getVariableName())) {
                    return true;
                }
            }
            return false;
        }

        public void showVariable(final String variableName) {
            //first search to see if aleady showing
            for (int i = 0; i < variables_visible.size(); ++i) {
                if (variableName.equals(variables_visible.get(i).getVariableName())) {
                    return; //already showing
                }
            }
            //find index in variables vector
            for (int i = 0; i < variables_static.size(); ++i) {
                if (variableName.equals(variables_static.get(i).getVariableName())) {
                    variables_visible.add(variables_static.get(i));
                    return; //already showing
                }
            }

        }

        public void hideVariable(final String variableName) {
            for (int i = 0; i < variables_visible.size(); ++i) {
                if (variableName.equals(variables_visible.get(i).getVariableName())) {
                    variables_visible.remove(i);
                    return;
                }
            }
        }

        public void hideAll() {
            variables_visible.clear();
        }

        @Override
        public void setValueAt(Object value, int row, int col) {
            if (value == null) {
                return;
            }
            for (int i = 0; i < comboBox.getItemCount(); ++i) {
                if (comboBox.getItemAt(i).equals(value)) {
                    variables_visible.get(row).setInputFileVariableIndex(i);
                    enableNavigationButtons();
                    fireTableCellUpdated(row, col);
                    return;
                }
            }
        }
    }

    /*
     * Import worker; executed in background thread.
     */
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
                VariableMappingTableModel model = (VariableMappingTableModel) _fieldMapTable.getModel();
                _importer = new FileImporter(getImportSource(),
                        model.variables_static,
                        _inputSourceSettings.getInputFileType(),
                        _inputSourceSettings.getSourceDataFileType(),
                        _destinationFile,
                        _progressBar);
                _importer.importFile(Integer.parseInt(_ignoreRowsTextField.getText()));
                if (!_importer.getCancelled()) {
                    _executed_import = true;
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
