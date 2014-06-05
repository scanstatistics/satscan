/*
 * ImportWizardDialog.java
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
import javax.swing.event.UndoableEditEvent;
import javax.swing.event.UndoableEditListener;
import javax.swing.table.AbstractTableModel;
import javax.swing.table.DefaultTableModel;
import javax.swing.table.TableColumn;
import javax.swing.undo.UndoManager;
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
import org.satscan.importer.DataSourceException;
import org.satscan.importer.InputSourceSettings;
import org.satscan.importer.InputSourceSettings.SourceDataFileType;
import org.satscan.importer.ShapefileDataSource;
import org.satscan.utils.FileAccess;

/**
 *
 * @author  Hostovic
 */
public class ImportWizardDialog extends javax.swing.JDialog implements PropertyChangeListener {

    private Preferences _prefs = Preferences.userNodeForPackage(getClass());
    private static final String _prefLastBackup = new String("import.destination");
    private final String _fileFormatCardName = "File Format";
    private final String _shapeFileOptionsCardName = "Shape File Options";
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
    private final String _sourceFile;
    private final InputSourceSettings.InputFileType _fileType;
    private final Parameters.ProbabilityModelType _startingModelType;
    private Parameters.CoordinatesType _coordinatesType;
    private InputSourceSettings.SourceDataFileType _sourceDataFileType;
    private String _showingCard;
    private boolean _errorSamplingSourceFile = true;
    private PreviewTableModel _previewTableModel = null;
    private boolean _cancelled = true;
    private File _destinationFile = null;
    private File _suggested_import_filename;
    private final InputSourceSettings _initial_inputSourceSettings;

    /** Creates new form ImportWizardDialog */
    public ImportWizardDialog(java.awt.Frame parent, 
                              final String sourceFile, 
                              final String suggested_filename,
                              InputSourceSettings inputSourceSettings, 
                              Parameters.ProbabilityModelType modelType, 
                              Parameters.CoordinatesType coordinatesType) {
        super(parent, true);
        setSuggestedImportName(sourceFile, suggested_filename, inputSourceSettings.getInputFileType());
        initComponents();
        _sourceFile = sourceFile;
        _fileType = inputSourceSettings.getInputFileType();
        _sourceDataFileType = getSourceFileType(_sourceFile);
        _startingModelType = modelType;
        _coordinatesType = coordinatesType;
        _initial_inputSourceSettings = inputSourceSettings;
        _progressBar.setVisible(false);
        configureForDestinationFileType();
        setShowingVariables();
        initializeImportVariableMappings();   
        setLocationRelativeTo(parent);
    }
    
    private void initializeImportVariableMappings() {
        // set initial import variable mappings from input source
        int next_v=0;
        for (int s=0; s < _initial_inputSourceSettings.getFieldMaps().size(); ++s) {
            int s_index = Integer.parseInt(_initial_inputSourceSettings.getFieldMaps().get(s));
            for (int v=0; v < _importVariables.size(); ++v) {
                if (_importVariables.get(v).getTargetFieldIndex() == s) {
                    _importVariables.get(v).setInputFileVariableIndex(s_index);
                }
            }
            //for (int v=next_v; v < _importVariables.size(); ++v) {
            //    if (_importVariables.get(v).getShowing()) {
            //        _importVariables.get(v).setInputFileVariableIndex(s_index);
            //        next_v = v + 1;
            //        break;
            //    }
            //}
        }             
    }
    
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
    
    /*
     * Returns collection of supported file filters.
     */
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
    
    /**
     * Public function used to determine whether user has imported to the date field.
     */
    public boolean getDateFieldImported() {
        boolean bReturn = false;

        switch (_fileType) {
            case Case:
            case Control:
                bReturn = _importVariables.get(_dateVariableColumn).getIsMappedToInputFileVariable();
                break;
            case Population:
                bReturn = true;
                break;
            case Coordinates:
            case SpecialGrid:
            case MaxCirclePopulation:
                bReturn = false;
                break;
            case AdjustmentsByRR:
                bReturn = _importVariables.get(_dateVariableColumn).getIsMappedToInputFileVariable();
                break;
            default:
                throw new UnknownEnumException(_fileType);
        }
        return bReturn;
    }

    /**
     * Returned whether the user completed the import or cancelled operation.
     */
    public boolean getCancelled() {
        return _cancelled;
    }

    /**
     * Returned whether the user choice execute import now.
     */
    public boolean getExecutedImport() {
        return !_cancelled && _execute_import_now.isSelected();
    }    
    
    /**
     * Create import target file.
     */
    private void createDestinationInformation() throws IOException {
        _destinationFile = new File(_outputDirectoryTextField.getText());
        //_destinationFile.
        try {
            _destinationFile.createNewFile();
        } catch (Exception e) {
            JOptionPane.showMessageDialog(this, "Unable to write to file: '" + _outputDirectoryTextField.getText() + "'.");
        }
    }

    /** Returns InputSourceSettings object from user selections in wizard. */
    public InputSourceSettings getInputSourceSettings(InputSourceSettings inputsource) {
        inputsource.setInputFileType(_fileType);
        inputsource.setSourceDataFileType(_sourceDataFileType);
        if (inputsource.getSourceDataFileType() == InputSourceSettings.SourceDataFileType.CSV) {
            inputsource.setDelimiter(Character.toString(getColumnDelimiter()));
            inputsource.setGroup(Character.toString(getGroupMarker()));
            inputsource.setSkiplines(Integer.parseInt(_ignoreRowsTextField.getText()));
            inputsource.setFirstRowHeader(_firstRowColumnHeadersCheckBox.isSelected());
        }
        if (inputsource.getSourceDataFileType() == InputSourceSettings.SourceDataFileType.Shapefile) {
            if (_latlongRadioButton.isSelected()) {
                inputsource.setShapeCoordinatesType(InputSourceSettings.ShapeCoordinatesType.LATLONG_DATA);
            } else if (_utmRadioButton.isSelected()) {
                inputsource.setShapeCoordinatesType(InputSourceSettings.ShapeCoordinatesType.UTM_CONVERSION);
                inputsource.setHemisphere(_hemisphereChoice.getSelectedItem());
                inputsource.setZone(Integer.parseInt(_zoneChoice.getSelectedItem()));
                inputsource.setNorthing(Double.parseDouble(_northing.getText()));
                inputsource.setEasting(Double.parseDouble(_easting.getText()));
            } else {
                inputsource.setShapeCoordinatesType(InputSourceSettings.ShapeCoordinatesType.CARTESIAN_DATA);                
            }
        }
        inputsource.getFieldMaps().clear();
        for (int t=0; t < _importVariables.size(); ++t) {
            if (_importVariables.get(t).getShowing()) {
                if (_importVariables.get(t).getIsMappedToInputFileVariable()) {
                    inputsource.getFieldMaps().add(Integer.toString(_importVariables.get(t).getInputFileVariableIndex()));
                } else {
                    inputsource.getFieldMaps().add("");
                }
            }
        }
        int size = inputsource.getFieldMaps().size() - 1;
        for (int i=inputsource.getFieldMaps().size() - 1; i >=0; --i) {
            if (!inputsource.getFieldMaps().get(i).isEmpty()) 
                break;
            else
                inputsource.getFieldMaps().remove(i);
        }      
        return inputsource;
    }
            
    
    /**
     * Returns import destination filename.
     */
    public String getDestinationFilename() {
        return _destinationFile != null ? _destinationFile.getAbsolutePath() : "";
    }

    /**
     * Returns source file type given source file extension.
     */
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

    private char getGroupMarker() {
        if (_doubleQuotesRadioButton.isSelected()) {
            return '"';
        } else if (_singleQuotesRadioButton.isSelected()) {
            return '\'';
        } else {
            throw new RuntimeException("Unknown group marker.");
        }
    }

    /**
     * Sets which panels and the order of showing panels.
     */
    private void setPanelsToShow(InputSourceSettings.SourceDataFileType eType) {
        _basePanel.removeAll();
        if (eType == InputSourceSettings.SourceDataFileType.SPACE_DELIMITED || eType == InputSourceSettings.SourceDataFileType.CSV) {
            _basePanel.add(_fileFormatPanel, _fileFormatCardName);
        } else if (eType == InputSourceSettings.SourceDataFileType.Shapefile) {
            _basePanel.add(_shapeFileOptionsPanel, _shapeFileOptionsCardName);
        }        
        _basePanel.add(_dataMappingPanel, _dataMappingCardName);
        _basePanel.add(_outputSettingsPanel, _outPutSettingsCardName);
        //reset class variable to that which we are showing
        _sourceDataFileType = eType;
    }

    /**
     * Updates which file format options panel is showing.
     */
    private void showFileTypeFormatOptionsPanel(InputSourceSettings.SourceDataFileType eFileType) {
        CardLayout cl = (CardLayout) (_sourceFileTypeOptions.getLayout());
        switch (eFileType) {
            case CSV:
                cl.show(_sourceFileTypeOptions, _fileFormatDelimitedCardName);
                _sourceDataFileType = InputSourceSettings.SourceDataFileType.CSV;
                break;
            default:
                throw new UnknownEnumException(eFileType);
        }
    }

    /**
     * Returns the coordinates type selected by user. This function
     * will only behavior as expected when FileImporter.InputFileType
     * is Coordinates or SpecialGrid.
     */
    public Parameters.CoordinatesType getCoorinatesControlType() {
        switch (_displayVariablesComboBox.getSelectedIndex()) {
            case 1:
                return Parameters.CoordinatesType.CARTESIAN;
            default:
                return Parameters.CoordinatesType.LATLON;
        }
    }

    /**
     * Returns the probablilty model type selected by user.
     */
    public Parameters.ProbabilityModelType getModelControlType() {
        switch (_displayVariablesComboBox.getSelectedIndex()) {
            case 1:
                return Parameters.ProbabilityModelType.BERNOULLI;
            case 2:
                return Parameters.ProbabilityModelType.SPACETIMEPERMUTATION;
            case 3:
                return Parameters.ProbabilityModelType.CATEGORICAL;
            case 4:
                return Parameters.ProbabilityModelType.ORDINAL;
            case 5:
                return Parameters.ProbabilityModelType.EXPONENTIAL;
            case 6:
                return Parameters.ProbabilityModelType.NORMAL;
            case 0:
            default:
                return Parameters.ProbabilityModelType.POISSON;
        }
    }

    /**
     * Shows/hides variables based upon destination file type and model/coordinates type.
     */
    private void setShowingVariables() {
        VariableMappingTableModel model = (VariableMappingTableModel) _fieldMapTable.getModel();
        Parameters.ProbabilityModelType _modelType = getModelControlType();
        Parameters.CoordinatesType _coordindatesType = getCoorinatesControlType();

        switch (_fileType) {
            case Case:
                model.hideAll();
                for (int t = 0; t < _importVariables.size(); ++t) {
                    _importVariables.get(t).setShowing(false);
                }
                for (int t = 0; t < _importVariables.size(); ++t) {
                    if (t >= 1 && t <= 2) {//show '# cases' and 'date time'  variables
                        _importVariables.get(t).setShowing(true);
                        model.setShowing(_importVariables.get(t));                   
                    } else if (t >= 6 && t <= 15) //show 'covariate' variables for Poisson,space-time permutation and normal models only
                    {
                        _importVariables.get(t).setShowing(_modelType == Parameters.ProbabilityModelType.POISSON ||
                                                           _modelType == Parameters.ProbabilityModelType.SPACETIMEPERMUTATION ||
                                                           _modelType == Parameters.ProbabilityModelType.NORMAL);
                        model.setShowing(_importVariables.get(t));
                    } else if (t == 3) //show 'attribute' variable for ordinal, exponential, normal and rank models only
                    {
                        _importVariables.get(t).setShowing(_modelType == Parameters.ProbabilityModelType.ORDINAL ||
                                                           _modelType == Parameters.ProbabilityModelType.CATEGORICAL ||
                                                           _modelType == Parameters.ProbabilityModelType.EXPONENTIAL ||
                                                           _modelType == Parameters.ProbabilityModelType.NORMAL ||
                                                           _modelType == Parameters.ProbabilityModelType.RANK);
                        model.setShowing(_importVariables.get(t));
                    } else if (t == 4) //show 'censored' variable for exponential model only
                    {
                        _importVariables.get(t).setShowing(_modelType == Parameters.ProbabilityModelType.EXPONENTIAL);
                        model.setShowing(_importVariables.get(t));
                    } else if (t == 5) //show 'weight' variable for normal model only
                    {
                        _importVariables.get(t).setShowing(_modelType == Parameters.ProbabilityModelType.NORMAL);
                        model.setShowing(_importVariables.get(t));
                    } else //default - show variable
                    {
                        _importVariables.get(t).setShowing(true);
                        model.setShowing(_importVariables.get(t));
                    }
                }
                break;
            case Coordinates:
                for (int t = 0; t < _importVariables.size(); ++t) {
                    if (t == 1 || t == 2) //show 'lat/long' variables for lat/long system
                    {
                        _importVariables.get(t).setShowing(_coordindatesType == Parameters.CoordinatesType.LATLON);
                        model.setShowing(_importVariables.get(t));
                    } else if (t >= 3) //show 'X/Y/Zn' variables for Cartesian system
                    {
                        _importVariables.get(t).setShowing(_coordindatesType == Parameters.CoordinatesType.CARTESIAN);
                        model.setShowing(_importVariables.get(t));
                    } else {
                        _importVariables.get(t).setShowing(true);
                        model.setShowing(_importVariables.get(t));
                    }
                }
                break;
            case SpecialGrid:
                for (int t = 0; t < _importVariables.size(); ++t) {
                    if (t == 0 || t == 1) //show 'lat/long' variables for lat/long system
                    {
                        _importVariables.get(t).setShowing(_coordindatesType == Parameters.CoordinatesType.LATLON);
                        model.setShowing(_importVariables.get(t));
                    } else //show 'X/Y/Zn' variables for Cartesian system
                    {
                        _importVariables.get(t).setShowing(_coordindatesType == Parameters.CoordinatesType.CARTESIAN);
                        model.setShowing(_importVariables.get(t));
                    }
                }
                break;
            default:
                for (int t = 0; t < _importVariables.size(); ++t) {
                    _importVariables.get(t).setShowing(true);
                    model.setShowing(_importVariables.get(t));
                }
        }
        model.fireTableDataChanged();
        
        
        
    }

    /** Opens source file as shapefile. */
    private void previewSourceAsShapeFile() throws Exception {
        try {
            String supportedType = ShapefileDataSource.isSupportedShapeType(_sourceFile);
            if (supportedType.length() > 0) {
                throw new DataSourceException(supportedType);
            }                
            //String supportedProjection = ShapefileDataSource.isSupportedProjection(_sourceFile);
            //if (supportedProjection.length() > 0) {
            //    throw new DataSourceException(supportedProjection);
            //}            
            ShapefileDataSource source = new ShapefileDataSource(new File(_sourceFile), true, 
                                                                 _utmRadioButton.isSelected(),
                                                                 _hemisphereChoice.getSelectedItem(),
                                                                 Integer.parseInt(_zoneChoice.getSelectedItem()),
                                                                 Double.parseDouble(_northing.getText()),
                                                                 Double.parseDouble(_easting.getText()));
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

    private ImportDataSource getImportSource() throws FileNotFoundException {
        if (_sourceDataFileType == InputSourceSettings.SourceDataFileType.Shapefile) {
            return new ShapefileDataSource(new File(_sourceFile), true, 
                                           _utmRadioButton.isSelected(),
                                           _hemisphereChoice.getSelectedItem(),
                                           Integer.parseInt(_zoneChoice.getSelectedItem()),
                                           Double.parseDouble(_northing.getText()),
                                           Double.parseDouble(_easting.getText()));            
        } else if (_sourceDataFileType == InputSourceSettings.SourceDataFileType.dBase) {
            return new DBaseImportDataSource(new File(_sourceFile), false);
        } else if (_sourceDataFileType == InputSourceSettings.SourceDataFileType.Excel) {
            return new XLSImportDataSource(new File(_sourceFile), true);
        } else {
            return new CSVImportDataSource(new File(_sourceFile), _firstRowColumnHeadersCheckBox.isSelected(),
                    '\n', getColumnDelimiter(), getGroupMarker());
        } 
    }

    /** Opening source as specified by file type. */
    private void previewSource() throws Exception {
        //set the import tables model to default until we have an instance of the native model avaiable
        _importTableDataTable.setModel(new DefaultTableModel());

        //create the table model
        if (_sourceDataFileType == InputSourceSettings.SourceDataFileType.Shapefile) {
            previewSourceAsShapeFile();
        } else if (_sourceDataFileType == InputSourceSettings.SourceDataFileType.dBase) {
            previewSourceAsDBaseFile();
        } else if (_sourceDataFileType == InputSourceSettings.SourceDataFileType.Excel) {
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

    /**
     * Enables navigation button based upon active panel and settings.
     */
    private void enableNavigationButtons() {
        previousButton.setEnabled(false);
        nextButton.setEnabled(false);
        executeButton.setEnabled(false);

        if (_showingCard == null) {
            return;
        } else if (_showingCard.equals(_fileFormatCardName)) {
            nextButton.setEnabled(!_errorSamplingSourceFile &&
                                  (_otherRadioButton.isSelected() ? _otherFieldSeparatorTextField.getText().length() > 0 : true));
        } else if (_showingCard.equals(this._shapeFileOptionsCardName)) {
            // TODO -- probably need something here eventually 
            nextButton.setEnabled(true);
        } else if (_showingCard.equals(_dataMappingCardName)) {
            previousButton.setEnabled(_sourceDataFileType != InputSourceSettings.SourceDataFileType.dBase &&
                                      _sourceDataFileType != InputSourceSettings.SourceDataFileType.Excel);
            if (_importTableDataTable.getModel().getRowCount() > 0) {
                VariableMappingTableModel model = (VariableMappingTableModel) _fieldMapTable.getModel();
                for (int i = 0; i < model.variables_visible.size() && !nextButton.isEnabled(); ++i) {
                    nextButton.setEnabled(model.variables_visible.get(i).getIsMappedToInputFileVariable());
                }
            }
        } else if (_showingCard.equals(_outPutSettingsCardName)) {
            previousButton.setEnabled(true);
            executeButton.setEnabled(_outputDirectoryTextField.getText().length() > 0);
        }
    //else nop
    }

    /**
     * Preparation for viewing file format panel.
     */
    private void prepFileFormatPanel() {
        WaitCursor waitCursor = new WaitCursor(this);
        try {
            readDataFileIntoRawDisplayField();
            showFileTypeFormatOptionsPanel(InputSourceSettings.SourceDataFileType.CSV);
            if (_initial_inputSourceSettings.isSet()) {
                // define settings given input source settings
                _ignoreRowsTextField.setText(Integer.toString(_initial_inputSourceSettings.getSkiplines()));
                _firstRowColumnHeadersCheckBox.setSelected(_initial_inputSourceSettings.getFirstRowHeader());
                if (_initial_inputSourceSettings.getDelimiter().isEmpty() || _initial_inputSourceSettings.getDelimiter().equalsIgnoreCase(" ")) {
                    _whitespaceRadioButton.setSelected(true);
                } else if (_initial_inputSourceSettings.getDelimiter().equalsIgnoreCase(",")) {
                    _commaRadioButton.setSelected(true);
                } else if (_initial_inputSourceSettings.getDelimiter().equalsIgnoreCase(";")) {
                    _semiColonRadioButton.setSelected(true);
                } else {
                    _otherRadioButton.setSelected(true);
                    _otherFieldSeparatorTextField.setText("" + _initial_inputSourceSettings.getDelimiter().charAt(0));
                }
                if (_initial_inputSourceSettings.getGroup().equalsIgnoreCase("'")) {
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

    /**
     * Preparation for viewing of the shapefile options panel.
     */
    private void prepShapefileOptionsPanel() {
        WaitCursor waitCursor = new WaitCursor(this);
        try {
            _textAreashapefileCoordinatesSampling.removeAll();
            String supportedType = ShapefileDataSource.isSupportedShapeType(_sourceFile);
            if (supportedType.length() > 0) {
                _textAreashapefileCoordinatesSampling.append(supportedType);
            }                
            ShapefileDataSource source = new ShapefileDataSource(new File(_sourceFile), true);
            for (int r=0; r < Math.min(10, source.getNumRecords()); ++r) {
                double[] coordinates = source.getCoordinates(r);
                _textAreashapefileCoordinatesSampling.append(Double.toString(coordinates[0]) + ", " + Double.toString(coordinates[1]) + "\n");
            }
            JScrollBar vbar = _ScrollPaneShapefileCoordinatesSampling.getVerticalScrollBar();
            _textAreashapefileCoordinatesSampling.setCaretPosition(0);
            vbar.setValue(vbar.getMinimum());
            readProjectionFileContents();
            if (_initial_inputSourceSettings.isSet()) {
                switch (_initial_inputSourceSettings.getShapeCoordinatesType()) {
                    case LATLONG_DATA : _latlongRadioButton.setSelected(true); break;
                    case UTM_CONVERSION : 
                        _utmRadioButton.setSelected(true);
                        _hemisphereChoice.select(_initial_inputSourceSettings.getHemisphere());
                        _zoneChoice.select(Integer.toString(_initial_inputSourceSettings.getZone()));
                        _northing.setText(Double.toString(_initial_inputSourceSettings.getNorthing()));
                        _easting.setText(Double.toString(_initial_inputSourceSettings.getEasting()));
                    break;
                    case CARTESIAN_DATA : _othercoordinatesRadioButton.setSelected(true); break;
                    default: throw new UnknownEnumException(_initial_inputSourceSettings.getShapeCoordinatesType());
                }                
            }
            enableNavigationButtons();
        } finally {
            waitCursor.restore();
        }
    }
    
    /**
     * Reads in a sample of data file into a memo field to help user
     * to determine structure of source file.
     */
    private void readProjectionFileContents() {
        jTextArea1.removeAll();

        //Attempt to open source file reader...
        FileReader fileSample = null;
        try {
            String projectionFilename = new String(_sourceFile);
            int lastDot = projectionFilename.lastIndexOf(".");
            if (lastDot != -1) {
                 projectionFilename = projectionFilename.substring(0, lastDot) + ".prj";
            } else {
                projectionFilename = projectionFilename + ".prj";
            } 
            fileSample = new FileReader(projectionFilename);
        } catch (FileNotFoundException e) {
            jTextArea1.setText("* Unable to view projection file contents. File does not exist. *");
            return;
        } catch (SecurityException e) {
            jTextArea1.setText("* Unable to view projection file contents. File permissions deny read access. *");
            return;
        }
        try {
            BufferedReader buffer = new BufferedReader(fileSample);
            for (int i = 0; i < 50 /* should be one line, but just in case */; ++i) {
                String line = null;
                line = buffer.readLine();
                if (line == null) {
                    break;
                }
                jTextArea1.append(line + "\n");
            }
            buffer.close();
            fileSample.close();
        } catch (IOException e) {
            jTextArea1.removeAll();
            jTextArea1.append("* Unable to view projection file contents. *");
            return;
        }
        //Indicate whether the source file had any data...
        if (jTextArea1.getLineCount() == 0) {
            jTextArea1.append("* Source file contains no data. *");
        }
        jTextArea1.setCaretPosition(0);
        JScrollBar vbar = jScrollPane3.getVerticalScrollBar();
        vbar.setValue(vbar.getMinimum());
    }
    
    /**
     * Clears and reassigns values of VariableMappingTableModel combobox column.
     */
    private void setMappingTableComboCells() {
        TableColumn selectionColumn = _fieldMapTable.getColumnModel().getColumn(1);
        VariableMappingTableModel model = (VariableMappingTableModel) _fieldMapTable.getModel();
        //if (model.comboBox.getItemCount() == 0) {
        model.comboBox.removeAllItems();
        model.comboBox.addItem(_unAssignedVariableName);
        for (int i = 0; i < _importTableDataTable.getModel().getColumnCount(); ++i) {
            model.comboBox.addItem(_importTableDataTable.getModel().getColumnName(i));
        }
        selectionColumn.setCellEditor(new DefaultCellEditor(model.comboBox));
        model.fireTableDataChanged();
        // SAH -- remove? clearSaTScanVariableFieldIndexes();
    }

    /** Clears field mapping selections. */
    private void clearSaTScanVariableFieldIndexes() {
        for (int t = 0; t < _importVariables.size(); ++t) {
            _importVariables.get(t).setInputFileVariableIndex(0);
        }
        VariableMappingTableModel model = (VariableMappingTableModel) _fieldMapTable.getModel();
        model.fireTableDataChanged();
    }

    /**
     * Returns the destination file type as string.
     */
    private String getInputFileTypeString() {
        switch (_fileType) {
            case Case:
                return "Case File";
            case Control:
                return "Control File";
            case Population:
                return "Population File";
            case Coordinates:
                return "Coordinates File";
            case SpecialGrid:
                return "Grid File";
            case MaxCirclePopulation:
                return "Max Circle Size File";
            case AdjustmentsByRR:
                return "Adjustments File";
            default:
                throw new UnknownEnumException(_fileType);
        }
    }

    /**
     * Validates that required SaTScan Variables has been specified with an input
     * file field to import from. Displays message if variables are missings.
     */
    private boolean checkForRequiredVariables() {
        StringBuilder message = new StringBuilder();
        Vector<ImportVariable> missing = new Vector<ImportVariable>();
        VariableMappingTableModel model = (VariableMappingTableModel) _fieldMapTable.getModel();

        for (int t = 0; t < _importVariables.size(); ++t) {
            ImportVariable variable = _importVariables.get(t);
            if (model.isShowing(variable.getVariableName()) && !variable.getIsMappedToInputFileVariable() && variable.getIsRequiredField()) {
                missing.add(variable);
            }
        }

        if (missing.size() > 0) {
            message.append("For the " + getInputFileTypeString());
            message.append(", the following SaTScan Variable(s) are required\nand an Input File Variable must");
            message.append(" be selected for each before import can proceed.\n\nSaTScan Variable(s): ");
            for (int t = 0; t < missing.size(); ++t) {
                message.append(missing.get(t).getVariableName());
                if (t < missing.size() - 1) {
                    message.append(", ");
                }
            }
            JOptionPane.showMessageDialog(this, message.toString(), "Note", JOptionPane.WARNING_MESSAGE);
            return true;
        }

        // we need to ensure that either both or neither dates are assigned for adjustment file
        if (_fileType == InputSourceSettings.InputFileType.AdjustmentsByRR) {
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

    /**
     * Preparation for viewing mapping panel.
     */
    private void prepMappingPanel() throws Exception {
        WaitCursor waitCursor = new WaitCursor(this);
        try {
            previewSource();
            setMappingTableComboCells();
            // SAH setShowingVariables();
            enableNavigationButtons();
        } finally {
            waitCursor.restore();
        }
    }

    /**
     * Causes showCard to be the active card.
     */
    private void bringPanelToFront(String showCard) {
        ((CardLayout) _basePanel.getLayout()).show(_basePanel, showCard);
        _showingCard = showCard;
        enableNavigationButtons();
        if (executeButton.isEnabled()) {
            executeButton.requestFocus();
        } else if (nextButton.isEnabled()) {
            nextButton.requestFocus();
        }
    }

    private void prepOutputSettingsPanel() {
        //reset import table model
        _importTableDataTable.setModel(new DefaultTableModel());
    }

    /**
     * Calls appropriate preparation methods then shows panel.
     */
    private void makeActivePanel(String targetCardName) throws Exception {
        if (targetCardName.equals(_fileFormatCardName)) {
            prepFileFormatPanel();
        } else if (targetCardName.equals(_shapeFileOptionsCardName)) {
            prepShapefileOptionsPanel();
        } else if (targetCardName.equals(_dataMappingCardName)) {
            prepMappingPanel();
        } else if (targetCardName.equals(_outPutSettingsCardName)) {
            prepOutputSettingsPanel();
        }
        bringPanelToFront(targetCardName);
    }

    /**
     * Reads in a sample of data file into a memo field to help user
     * to determine structure of source file.
     */
    private void readDataFileIntoRawDisplayField() {
        _fileContentsTextArea.removeAll();
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
    }

    /**
     * Configure the combo box that changes what variables are displayed.
     */
    private void configureDisplayVariablesComboBox() {
        if (_fileType == InputSourceSettings.InputFileType.Case) {
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
            }
        } else if (_fileType == InputSourceSettings.InputFileType.Coordinates || _fileType == InputSourceSettings.InputFileType.SpecialGrid) {
            _displayVariablesComboBox.removeAllItems();
            _displayVariablesComboBox.addItem("Latitude/Longitude Coordinates");
            _displayVariablesComboBox.addItem("Cartesian (x, y) Coordinates");
            switch (_coordinatesType) {
                case CARTESIAN : _displayVariablesComboBox.setSelectedIndex(1); break;
                case LATLON    :   
                default        : _displayVariablesComboBox.setSelectedIndex(0); 
            }
        } else {
            _displayVariablesLabel.setEnabled(false);
            _displayVariablesComboBox.setEnabled(false);
            _displayVariablesComboBox.removeAllItems();            
        }
    }

    /**
     * Configures the variables vector based upon the target file type.
     */
    private void configureForDestinationFileType() {
        switch (_fileType) {
            case Case:
                setCaseFileVariables();
                break;
            case Control:
                setControlFileVariables();
                break;
            case Population:
                setPopFileVariables();
                break;
            case Coordinates:
                setGeoFileVariables();
                break;
            case SpecialGrid:
                setGridFileVariables();
                break;
            case MaxCirclePopulation:
                setMaxCirclePopFileVariables();
                break;
            case AdjustmentsByRR:
                setRelativeRisksFileVariables();
                break;
            default:
                throw new UnknownEnumException(_fileType);
        }
        configureDisplayVariablesComboBox();
    }

    /**
     * Prepares the dialog for the initial
     */
    @Override
    public void setVisible(boolean value) {
        try {
            if (value) {
                if (_sourceDataFileType == InputSourceSettings.SourceDataFileType.dBase || 
                    _sourceDataFileType == InputSourceSettings.SourceDataFileType.Excel ||
                    _sourceDataFileType == InputSourceSettings.SourceDataFileType.Shapefile) {
                    try {
                        setPanelsToShow(_sourceDataFileType);
                        if (_sourceDataFileType == InputSourceSettings.SourceDataFileType.Shapefile) {
                            makeActivePanel(this._shapeFileOptionsCardName);                            
                        } else {
                            makeActivePanel(_dataMappingCardName);
                        }
                        super.setVisible(value);
                        return;
                    } catch (DataSourceException e) {
                      throw e;  
                    } catch (Throwable e) { // try opening as default delimited ascii file
                        _sourceDataFileType = InputSourceSettings.SourceDataFileType.CSV;
                    }
                }
                setPanelsToShow(_sourceDataFileType);
                makeActivePanel(_fileFormatCardName);
            }
            super.setVisible(value);
        } catch (DataSourceException e) {
            JOptionPane.showMessageDialog(this, e.getMessage(), "Note", JOptionPane.INFORMATION_MESSAGE);            
        } catch (Exception e) {
            new ExceptionDialog(this, e).setVisible(true);
        }
    }

    /**
     * Setup field descriptors for case file.
     */
    private void setCaseFileVariables() {
        _importVariables.clear();
        _importVariables.addElement(new ImportVariable("Location ID", 0, true, null));
        _importVariables.addElement(new ImportVariable("Number of Cases", 1, true, null));
        _importVariables.addElement(new ImportVariable("Date/Time", 2, false, null));
        _importVariables.addElement(new ImportVariable("Attribute (value)", 3, true, null));
        _importVariables.addElement(new ImportVariable("Censored", 4, false, null));
        _importVariables.addElement(new ImportVariable("Weight", 4, false, null));
        _importVariables.addElement(new ImportVariable("Covariate1", 5, false, null));
        _importVariables.addElement(new ImportVariable("Covariate2", 6, false, null));
        _importVariables.addElement(new ImportVariable("Covariate3", 7, false, null));
        _importVariables.addElement(new ImportVariable("Covariate4", 8, false, null));
        _importVariables.addElement(new ImportVariable("Covariate5", 9, false, null));
        _importVariables.addElement(new ImportVariable("Covariate6", 10, false, null));
        _importVariables.addElement(new ImportVariable("Covariate7", 11, false, null));
        _importVariables.addElement(new ImportVariable("Covariate8", 12, false, null));
        _importVariables.addElement(new ImportVariable("Covariate9", 13, false, null));
        _importVariables.addElement(new ImportVariable("Covariate10", 14, false, null));    
    }

    /**
     * Setup field descriptors for control file.
     */
    private void setControlFileVariables() {
        _importVariables.clear();
        _importVariables.addElement(new ImportVariable("Location ID", 0, true, null));
        _importVariables.addElement(new ImportVariable("Number of Controls", 1, true, null));
        _importVariables.addElement(new ImportVariable("Date/Time", 2, false, null));
    }

    /**
     * Setup field descriptors for coordinates file.
     */
    private void setGeoFileVariables() {
        _importVariables.clear();
        _importVariables.addElement(new ImportVariable("Location ID", 0, true, null));
        _importVariables.addElement(new ImportVariable("Latitude", 1, true, "y-axis"));
        _importVariables.addElement(new ImportVariable("Longitude", 2, true, "x-axis"));
        _importVariables.addElement(new ImportVariable("X", 1, true, null));
        _importVariables.addElement(new ImportVariable("Y", 2, true, null));
        _importVariables.addElement(new ImportVariable("Z1", 3, false, null));
        _importVariables.addElement(new ImportVariable("Z2", 4, false, null));
        _importVariables.addElement(new ImportVariable("Z3", 5, false, null));
        _importVariables.addElement(new ImportVariable("Z4", 6, false, null));
        _importVariables.addElement(new ImportVariable("Z5", 7, false, null));
        _importVariables.addElement(new ImportVariable("Z6", 8, false, null));
        _importVariables.addElement(new ImportVariable("Z7", 9, false, null));
        _importVariables.addElement(new ImportVariable("Z8", 10, false, null));
    }

    /**
     * Setup field descriptors for special grid file.
     */
    private void setGridFileVariables() {
        _importVariables.clear();
        _importVariables.addElement(new ImportVariable("Latitude", 0, true, "y-axis"));
        _importVariables.addElement(new ImportVariable("Longitude", 1, true, "x-axis"));
        _importVariables.addElement(new ImportVariable("X", 0, true, null));
        _importVariables.addElement(new ImportVariable("Y", 1, true, null));
        _importVariables.addElement(new ImportVariable("Z1", 2, false, null));
        _importVariables.addElement(new ImportVariable("Z2", 3, false, null));
        _importVariables.addElement(new ImportVariable("Z3", 4, false, null));
        _importVariables.addElement(new ImportVariable("Z4", 5, false, null));
        _importVariables.addElement(new ImportVariable("Z5", 6, false, null));
        _importVariables.addElement(new ImportVariable("Z6", 7, false, null));
        _importVariables.addElement(new ImportVariable("Z7", 8, false, null));
        _importVariables.addElement(new ImportVariable("Z8", 9, false, null));
    }

    /**
     * Setup field descriptors for maximum circle population file.
     */
    private void setMaxCirclePopFileVariables() {
        _importVariables.clear();
        _importVariables.addElement(new ImportVariable("Location ID", 0, true, null));
        _importVariables.addElement(new ImportVariable("Population", 1, true, null));
    }

    /**
     * Setup field descriptors for population file.
     */
    private void setPopFileVariables() {
        _importVariables.clear();
        _importVariables.addElement(new ImportVariable("Location ID", 0, true, null));
        _importVariables.addElement(new ImportVariable("Date/Time", 1, true, null));
        _importVariables.addElement(new ImportVariable("Population", 2, true, null));
        _importVariables.addElement(new ImportVariable("Covariate1", 3, false, null));
        _importVariables.addElement(new ImportVariable("Covariate2", 4, false, null));
        _importVariables.addElement(new ImportVariable("Covariate3", 5, false, null));
        _importVariables.addElement(new ImportVariable("Covariate4", 6, false, null));
        _importVariables.addElement(new ImportVariable("Covariate5", 7, false, null));
        _importVariables.addElement(new ImportVariable("Covariate6", 8, false, null));
        _importVariables.addElement(new ImportVariable("Covariate7", 9, false, null));
        _importVariables.addElement(new ImportVariable("Covariate8", 10, false, null));
        _importVariables.addElement(new ImportVariable("Covariate9", 11, false, null));
        _importVariables.addElement(new ImportVariable("Covariate10", 12, false, null));
    }

    /**
     * Setup field descriptors for relative risks file.
     */
    private void setRelativeRisksFileVariables() {
        _importVariables.clear();
        _importVariables.addElement(new ImportVariable("Location ID", 0, true, null));
        _importVariables.addElement(new ImportVariable("Relative Risk", 1, true, null));
        _importVariables.addElement(new ImportVariable("Start Date", 2, false, null));
        _importVariables.addElement(new ImportVariable("End Date", 3, false, null));
    }

    /**
     * Invoked when task's progress property changes.
     */
    public void propertyChange(PropertyChangeEvent evt) {
        if (evt.getNewValue() instanceof Integer) {
            _progressBar.setValue(((Integer) evt.getNewValue()).intValue());
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
        cancelButton = new javax.swing.JButton();
        nextButton = new javax.swing.JButton();
        previousButton = new javax.swing.JButton();
        executeButton = new javax.swing.JButton();
        _basePanel = new javax.swing.JPanel();
        _fileFormatPanel = new javax.swing.JPanel();
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
        _dataMappingPanel = new javax.swing.JPanel();
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
        _outputSettingsPanel = new javax.swing.JPanel();
        _outputDirectoryTextField = new javax.swing.JTextField();
        _changeSaveDirectoryButton = new javax.swing.JButton();
        _progressBar = new javax.swing.JProgressBar();
        _execute_import_now = new javax.swing.JRadioButton();
        _save_import_settings = new javax.swing.JRadioButton();
        _shapeFileOptionsPanel = new javax.swing.JPanel();
        jLabel2 = new javax.swing.JLabel();
        _ScrollPaneShapefileCoordinatesSampling = new javax.swing.JScrollPane();
        _textAreashapefileCoordinatesSampling = new javax.swing.JTextArea();
        jLabel3 = new javax.swing.JLabel();
        jScrollPane3 = new javax.swing.JScrollPane();
        jTextArea1 = new javax.swing.JTextArea();
        jScrollPane4 = new javax.swing.JScrollPane();
        jTextPane1 = new javax.swing.JTextPane();
        _latlongRadioButton = new javax.swing.JRadioButton();
        _utmRadioButton = new javax.swing.JRadioButton();
        jLabel6 = new javax.swing.JLabel();
        jLabel7 = new javax.swing.JLabel();
        _hemisphereChoice = new java.awt.Choice();
        jLabel8 = new javax.swing.JLabel();
        _zoneChoice = new java.awt.Choice();
        jLabel9 = new javax.swing.JLabel();
        _northing = new javax.swing.JTextField();
        jLabel10 = new javax.swing.JLabel();
        _easting = new javax.swing.JTextField();
        _othercoordinatesRadioButton = new javax.swing.JRadioButton();
        jLabel11 = new javax.swing.JLabel();

        _fieldSeparatorButtonGroup.add(_commaRadioButton);
        _fieldSeparatorButtonGroup.add(_semiColonRadioButton);
        _fieldSeparatorButtonGroup.add(_whitespaceRadioButton);
        _fieldSeparatorButtonGroup.add(_otherRadioButton);

        _groupIndicatorButtonGroup.add(_doubleQuotesRadioButton);
        _groupIndicatorButtonGroup.add(_singleQuotesRadioButton);

        setDefaultCloseOperation(javax.swing.WindowConstants.DISPOSE_ON_CLOSE);
        setTitle("Import Wizard"); // NOI18N
        setModal(true);

        cancelButton.setText("Cancel"); // NOI18N
        cancelButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                _cancelled = true;
                setVisible(false);
            }
        });

        nextButton.setText("Next >"); // NOI18N
        nextButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                try {
                    if (_showingCard == _fileFormatCardName || _showingCard == _shapeFileOptionsCardName)
                    makeActivePanel(_dataMappingCardName);
                    else if (_showingCard == _dataMappingCardName) {
                        if (checkForRequiredVariables()) return;
                        makeActivePanel(_outPutSettingsCardName);
                    }
                    //else nop
                } catch (Throwable t) {
                    new ExceptionDialog(ImportWizardDialog.this, t).setVisible(true);
                }
            }
        });

        previousButton.setText("< Previous"); // NOI18N
        previousButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                try {
                    if (_showingCard == _dataMappingCardName) {
                        if (_sourceDataFileType == SourceDataFileType.Shapefile)
                        makeActivePanel(_shapeFileOptionsCardName);
                        else
                        makeActivePanel(_fileFormatCardName);
                    } else if (_showingCard == _outPutSettingsCardName)
                    makeActivePanel(_dataMappingCardName);
                    //else nop
                } catch (Throwable t) {
                    new ExceptionDialog(ImportWizardDialog.this, t).setVisible(true);
                }
            }
        });

        executeButton.setText("Execute"); // NOI18N
        executeButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                try {
                    if (_execute_import_now.isSelected()) {
                        try {
                            createDestinationInformation();
                        } catch (IOException i) {
                            JOptionPane.showMessageDialog(ImportWizardDialog.this,
                                "The import wizard encountered an error attempting to create the import file.\n" +
                                "This is most likely occuring because write permissions are not granted for\n" +
                                "specified directory. Please check path or review user permissions for specified directory.\n", "Note", JOptionPane.WARNING_MESSAGE);
                            return;
                        }
                        ImportTask task = new ImportTask();
                        task.addPropertyChangeListener(ImportWizardDialog.this);
                        task.execute();
                    } else {
                        _cancelled = false;
                        setVisible(false);
                    }
                } catch (Throwable t) {
                    new ExceptionDialog(ImportWizardDialog.this, t).setVisible(true);
                }
            }
        });

        _basePanel.setLayout(new java.awt.CardLayout());

        jLabel1.setText("File Contents:"); // NOI18N

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
                .addContainerGap(238, Short.MAX_VALUE))
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
                .addContainerGap(368, Short.MAX_VALUE))
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
            .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, _cSVDefsPanelLayout.createSequentialGroup()
                .addContainerGap()
                .addGroup(_cSVDefsPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING)
                    .addComponent(_groupIndiocatorGroup, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(fieldSeparatorGroup, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                .addContainerGap())
        );
        _cSVDefsPanelLayout.setVerticalGroup(
            _cSVDefsPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_cSVDefsPanelLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(fieldSeparatorGroup, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_groupIndiocatorGroup, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap(41, Short.MAX_VALUE))
        );

        _sourceFileTypeOptions.add(_cSVDefsPanel, "cvsPanel");

        javax.swing.GroupLayout _fileFormatPanelLayout = new javax.swing.GroupLayout(_fileFormatPanel);
        _fileFormatPanel.setLayout(_fileFormatPanelLayout);
        _fileFormatPanelLayout.setHorizontalGroup(
            _fileFormatPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_fileFormatPanelLayout.createSequentialGroup()
                .addComponent(jLabel1)
                .addContainerGap())
            .addComponent(_sourceFileTypeOptions, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
            .addComponent(jScrollPane1)
            .addGroup(_fileFormatPanelLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(jLabel4)
                .addGap(5, 5, 5)
                .addComponent(_ignoreRowsTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 42, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addGap(5, 5, 5)
                .addComponent(jLabel5)
                .addGap(18, 18, 18)
                .addComponent(_firstRowColumnHeadersCheckBox, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .addGap(164, 164, 164))
        );
        _fileFormatPanelLayout.setVerticalGroup(
            _fileFormatPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, _fileFormatPanelLayout.createSequentialGroup()
                .addComponent(jLabel1)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(jScrollPane1, javax.swing.GroupLayout.DEFAULT_SIZE, 266, Short.MAX_VALUE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(_fileFormatPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(jLabel4)
                    .addComponent(jLabel5)
                    .addComponent(_ignoreRowsTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(_firstRowColumnHeadersCheckBox))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_sourceFileTypeOptions, javax.swing.GroupLayout.PREFERRED_SIZE, 167, javax.swing.GroupLayout.PREFERRED_SIZE))
        );

        _basePanel.add(_fileFormatPanel, "File Format");

        jSplitPane1.setDividerLocation(250);
        jSplitPane1.setOrientation(javax.swing.JSplitPane.VERTICAL_SPLIT);

        _displayVariablesLabel.setText("Display SaTScan Variables For:"); // NOI18N

        _displayVariablesComboBox.setModel(new javax.swing.DefaultComboBoxModel(new String[] { "Item 1", "Item 2", "Item 3", "Item 4" }));
        _displayVariablesComboBox.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                if (_fileType == InputSourceSettings.InputFileType.Coordinates || _fileType == InputSourceSettings.InputFileType.SpecialGrid) {
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
                        .addComponent(_displayVariablesComboBox, 0, 386, Short.MAX_VALUE))
                    .addComponent(jScrollPane2, javax.swing.GroupLayout.DEFAULT_SIZE, 537, Short.MAX_VALUE))
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
                    .addComponent(jScrollPane2, javax.swing.GroupLayout.DEFAULT_SIZE, 212, Short.MAX_VALUE))
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
            .addComponent(_importTableScrollPane, javax.swing.GroupLayout.DEFAULT_SIZE, 600, Short.MAX_VALUE)
        );
        _dataMappingBottomPanelLayout.setVerticalGroup(
            _dataMappingBottomPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_dataMappingBottomPanelLayout.createSequentialGroup()
                .addComponent(_importTableScrollPane, javax.swing.GroupLayout.DEFAULT_SIZE, 218, Short.MAX_VALUE)
                .addContainerGap())
        );

        jSplitPane1.setRightComponent(_dataMappingBottomPanel);

        javax.swing.GroupLayout _dataMappingPanelLayout = new javax.swing.GroupLayout(_dataMappingPanel);
        _dataMappingPanel.setLayout(_dataMappingPanelLayout);
        _dataMappingPanelLayout.setHorizontalGroup(
            _dataMappingPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addComponent(jSplitPane1)
        );
        _dataMappingPanelLayout.setVerticalGroup(
            _dataMappingPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addComponent(jSplitPane1)
        );

        _basePanel.add(_dataMappingPanel, "Mapping Panel");

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
        _execute_import_now.setText("Save imported input file:");
        _execute_import_now.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED)
                executeButton.setText("Execute");
            }
        });

        _import_operation_buttonGroup.add(_save_import_settings);
        _save_import_settings.setText("Read directly from data source when running the analysis.");
        _save_import_settings.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED)
                executeButton.setText("Done");
            }
        });

        javax.swing.GroupLayout _outputSettingsPanelLayout = new javax.swing.GroupLayout(_outputSettingsPanel);
        _outputSettingsPanel.setLayout(_outputSettingsPanelLayout);
        _outputSettingsPanelLayout.setHorizontalGroup(
            _outputSettingsPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_outputSettingsPanelLayout.createSequentialGroup()
                .addGroup(_outputSettingsPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(_execute_import_now, javax.swing.GroupLayout.Alignment.TRAILING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addGroup(_outputSettingsPanelLayout.createSequentialGroup()
                        .addContainerGap()
                        .addComponent(_progressBar, javax.swing.GroupLayout.DEFAULT_SIZE, 582, Short.MAX_VALUE))
                    .addGroup(_outputSettingsPanelLayout.createSequentialGroup()
                        .addGap(21, 21, 21)
                        .addComponent(_outputDirectoryTextField)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(_changeSaveDirectoryButton))
                    .addComponent(_save_import_settings, javax.swing.GroupLayout.Alignment.TRAILING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                .addContainerGap())
        );
        _outputSettingsPanelLayout.setVerticalGroup(
            _outputSettingsPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_outputSettingsPanelLayout.createSequentialGroup()
                .addComponent(_execute_import_now)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(_outputSettingsPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING)
                    .addComponent(_outputDirectoryTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(_changeSaveDirectoryButton))
                .addGap(18, 18, 18)
                .addComponent(_save_import_settings)
                .addGap(376, 376, 376)
                .addComponent(_progressBar, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );

        _basePanel.add(_outputSettingsPanel, "Output Settings");

        jLabel2.setText("Shapefile Coordinates Sampling:");

        _textAreashapefileCoordinatesSampling.setColumns(20);
        _textAreashapefileCoordinatesSampling.setRows(5);
        _ScrollPaneShapefileCoordinatesSampling.setViewportView(_textAreashapefileCoordinatesSampling);

        jLabel3.setText("Projection File Contents:");

        jScrollPane3.setHorizontalScrollBarPolicy(javax.swing.ScrollPaneConstants.HORIZONTAL_SCROLLBAR_NEVER);

        jTextArea1.setColumns(20);
        jTextArea1.setRows(2);
        jTextArea1.setWrapStyleWord(true);
        jTextArea1.setLineWrap(true);
        jScrollPane3.setViewportView(jTextArea1);

        jScrollPane4.setBorder(null);

        jTextPane1.setEditable(false);
        jTextPane1.setBackground(new java.awt.Color(240, 240, 240));
        jTextPane1.setContentType("text/html"); // NOI18N
        jTextPane1.setText("<html>\r\n  <head>\r\n\r\n  </head>\r\n  <body>\r\n    <p style=\"margin-top: 0\">\r\n      \rPlease use the coordinates sampling, projection file contents and your knowledge of shapefiles to select best option below:\n    </p>\r\n  </body>\r\n</html>\r\n");
        jScrollPane4.setViewportView(jTextPane1);

        _projectionButtonGroup.add(_latlongRadioButton);
        _latlongRadioButton.setSelected(true);
        _latlongRadioButton.setText("Coordinates are already in a latitude / longitude coordinates system.");
        _latlongRadioButton.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED) {
                    _coordinatesType = Parameters.CoordinatesType.LATLON;
                    switch (_coordinatesType) {
                        case CARTESIAN : _displayVariablesComboBox.setSelectedIndex(1); break;
                        case LATLON    :
                        default        : _displayVariablesComboBox.setSelectedIndex(0);
                    }
                }
            }
        });

        _projectionButtonGroup.add(_utmRadioButton);
        _utmRadioButton.setText("Coordinates are in a UTM coordinate system.");
        _utmRadioButton.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED) {
                    _coordinatesType = Parameters.CoordinatesType.LATLON;
                    switch (_coordinatesType) {
                        case CARTESIAN : _displayVariablesComboBox.setSelectedIndex(1); break;
                        case LATLON    :
                        default        : _displayVariablesComboBox.setSelectedIndex(0);
                    }
                }
            }
        });

        jLabel6.setText("Convert the coordinates to latitude / longitude knowing the following is true:");

        jLabel7.setText("Hemisphere:");

        _hemisphereChoice.add("N");
        _hemisphereChoice.add("S");

        jLabel8.setText("Zone:");

        for (int i=1; i <= 60; ++i) {
            _zoneChoice.add(Integer.toString(i));
        }

        jLabel9.setText("Northing:");

        _northing.setText("0");
        _northing.addKeyListener(new java.awt.event.KeyAdapter() {
            public void keyTyped(java.awt.event.KeyEvent e) {
                Utils.validatePostiveFloatKeyTyped(_northing, e, 20);
            }
        });
        _northing.addFocusListener(new java.awt.event.FocusAdapter() {
            public void focusLost(java.awt.event.FocusEvent e) {
                while (_northing.getText().length() == 0) {
                    if (undo.canUndo()) undo.undo(); else _northing.setText("0");
                }
            }
        });
        _northing.getDocument().addUndoableEditListener(new UndoableEditListener() {
            public void undoableEditHappened(UndoableEditEvent evt) {
                undo.addEdit(evt.getEdit());
            }
        });

        jLabel10.setText("Easting:");

        _easting.setText("500000");
        _easting.addKeyListener(new java.awt.event.KeyAdapter() {
            public void keyTyped(java.awt.event.KeyEvent e) {
                Utils.validatePostiveFloatKeyTyped(_easting, e, 20);
            }
        });
        _easting.addFocusListener(new java.awt.event.FocusAdapter() {
            public void focusLost(java.awt.event.FocusEvent e) {
                while (_easting.getText().length() == 0) {
                    if (undo.canUndo()) undo.undo(); else _easting.setText("500000");
                }
            }
        });
        _easting.getDocument().addUndoableEditListener(new UndoableEditListener() {
            public void undoableEditHappened(UndoableEditEvent evt) {
                undo.addEdit(evt.getEdit());
            }
        });

        _projectionButtonGroup.add(_othercoordinatesRadioButton);
        _othercoordinatesRadioButton.setText("Coordinates are in another coordinate system and should be considered Cartesian values.");
        _othercoordinatesRadioButton.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                if (e.getStateChange() == java.awt.event.ItemEvent.SELECTED) {
                    _coordinatesType = Parameters.CoordinatesType.CARTESIAN;
                    switch (_coordinatesType) {
                        case CARTESIAN : _displayVariablesComboBox.setSelectedIndex(1); break;
                        case LATLON    :
                        default        : _displayVariablesComboBox.setSelectedIndex(0);
                    }
                }
            }
        });

        jLabel11.setText("(Note: If using this data as input for the coordinates or grid files, the coordinates type should be set to Cartesian.)");

        javax.swing.GroupLayout _shapeFileOptionsPanelLayout = new javax.swing.GroupLayout(_shapeFileOptionsPanel);
        _shapeFileOptionsPanel.setLayout(_shapeFileOptionsPanelLayout);
        _shapeFileOptionsPanelLayout.setHorizontalGroup(
            _shapeFileOptionsPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addComponent(_ScrollPaneShapefileCoordinatesSampling)
            .addComponent(jLabel3, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
            .addComponent(jScrollPane3)
            .addComponent(jScrollPane4, javax.swing.GroupLayout.DEFAULT_SIZE, 602, Short.MAX_VALUE)
            .addComponent(_latlongRadioButton, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
            .addGroup(_shapeFileOptionsPanelLayout.createSequentialGroup()
                .addGroup(_shapeFileOptionsPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(jLabel2, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(_utmRadioButton, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addGroup(_shapeFileOptionsPanelLayout.createSequentialGroup()
                        .addGap(21, 21, 21)
                        .addGroup(_shapeFileOptionsPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addGroup(_shapeFileOptionsPanelLayout.createSequentialGroup()
                                .addComponent(jLabel7)
                                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                .addComponent(_hemisphereChoice, javax.swing.GroupLayout.PREFERRED_SIZE, 40, javax.swing.GroupLayout.PREFERRED_SIZE)
                                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                .addComponent(jLabel8)
                                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                .addComponent(_zoneChoice, javax.swing.GroupLayout.PREFERRED_SIZE, 50, javax.swing.GroupLayout.PREFERRED_SIZE)
                                .addGap(25, 25, 25)
                                .addComponent(jLabel9, javax.swing.GroupLayout.PREFERRED_SIZE, 55, javax.swing.GroupLayout.PREFERRED_SIZE)
                                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                .addComponent(_northing, javax.swing.GroupLayout.PREFERRED_SIZE, 65, javax.swing.GroupLayout.PREFERRED_SIZE)
                                .addGap(18, 18, 18)
                                .addComponent(jLabel10, javax.swing.GroupLayout.PREFERRED_SIZE, 55, javax.swing.GroupLayout.PREFERRED_SIZE)
                                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                .addComponent(_easting, javax.swing.GroupLayout.PREFERRED_SIZE, 65, javax.swing.GroupLayout.PREFERRED_SIZE)
                                .addGap(0, 0, Short.MAX_VALUE))
                            .addComponent(jLabel6, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)))
                    .addGroup(_shapeFileOptionsPanelLayout.createSequentialGroup()
                        .addContainerGap()
                        .addGroup(_shapeFileOptionsPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addGroup(_shapeFileOptionsPanelLayout.createSequentialGroup()
                                .addGap(21, 21, 21)
                                .addComponent(jLabel11, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                            .addComponent(_othercoordinatesRadioButton, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))))
                .addContainerGap())
        );
        _shapeFileOptionsPanelLayout.setVerticalGroup(
            _shapeFileOptionsPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_shapeFileOptionsPanelLayout.createSequentialGroup()
                .addComponent(jLabel2)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_ScrollPaneShapefileCoordinatesSampling, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addComponent(jLabel3)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(jScrollPane3, javax.swing.GroupLayout.PREFERRED_SIZE, 56, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(jScrollPane4, javax.swing.GroupLayout.PREFERRED_SIZE, 40, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_latlongRadioButton)
                .addGap(18, 18, 18)
                .addComponent(_utmRadioButton)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(jLabel6)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(_shapeFileOptionsPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(jLabel7)
                    .addComponent(_hemisphereChoice, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(jLabel8)
                    .addComponent(_zoneChoice, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addGroup(_shapeFileOptionsPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                        .addComponent(jLabel9)
                        .addComponent(_northing, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addComponent(jLabel10)
                        .addComponent(_easting, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)))
                .addGap(18, 18, 18)
                .addComponent(_othercoordinatesRadioButton)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(jLabel11)
                .addContainerGap(67, Short.MAX_VALUE))
        );

        _basePanel.add(_shapeFileOptionsPanel, "card5");

        javax.swing.GroupLayout layout = new javax.swing.GroupLayout(getContentPane());
        getContentPane().setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, layout.createSequentialGroup()
                        .addComponent(previousButton, javax.swing.GroupLayout.PREFERRED_SIZE, 100, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(nextButton, javax.swing.GroupLayout.PREFERRED_SIZE, 100, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(executeButton, javax.swing.GroupLayout.PREFERRED_SIZE, 100, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(cancelButton, javax.swing.GroupLayout.PREFERRED_SIZE, 100, javax.swing.GroupLayout.PREFERRED_SIZE))
                    .addComponent(_basePanel, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                .addContainerGap())
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, layout.createSequentialGroup()
                .addContainerGap()
                .addComponent(_basePanel, javax.swing.GroupLayout.PREFERRED_SIZE, 485, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(cancelButton)
                    .addComponent(executeButton)
                    .addComponent(nextButton)
                    .addComponent(previousButton))
                .addGap(5, 5, 5))
        );

        pack();
    }// </editor-fold>//GEN-END:initComponents
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JScrollPane _ScrollPaneShapefileCoordinatesSampling;
    private javax.swing.JPanel _basePanel;
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
    private javax.swing.JTextField _easting;
    private javax.swing.JRadioButton _execute_import_now;
    private javax.swing.JTable _fieldMapTable;
    private javax.swing.ButtonGroup _fieldSeparatorButtonGroup;
    private javax.swing.JTextArea _fileContentsTextArea;
    private javax.swing.JPanel _fileFormatPanel;
    private javax.swing.JCheckBox _firstRowColumnHeadersCheckBox;
    private javax.swing.ButtonGroup _groupIndicatorButtonGroup;
    private javax.swing.JPanel _groupIndiocatorGroup;
    private java.awt.Choice _hemisphereChoice;
    private javax.swing.JTextField _ignoreRowsTextField;
    private javax.swing.JTable _importTableDataTable;
    private javax.swing.JScrollPane _importTableScrollPane;
    private javax.swing.ButtonGroup _import_operation_buttonGroup;
    private javax.swing.JRadioButton _latlongRadioButton;
    private javax.swing.JTextField _northing;
    private javax.swing.JTextField _otherFieldSeparatorTextField;
    private javax.swing.JRadioButton _otherRadioButton;
    private javax.swing.JRadioButton _othercoordinatesRadioButton;
    private javax.swing.JTextField _outputDirectoryTextField;
    private javax.swing.JPanel _outputSettingsPanel;
    private javax.swing.JProgressBar _progressBar;
    private javax.swing.ButtonGroup _projectionButtonGroup;
    private javax.swing.JRadioButton _save_import_settings;
    private javax.swing.JRadioButton _semiColonRadioButton;
    private javax.swing.JPanel _shapeFileOptionsPanel;
    private javax.swing.JRadioButton _singleQuotesRadioButton;
    private javax.swing.JPanel _sourceFileTypeOptions;
    private javax.swing.JTextArea _textAreashapefileCoordinatesSampling;
    private javax.swing.JRadioButton _utmRadioButton;
    private javax.swing.JRadioButton _whitespaceRadioButton;
    private java.awt.Choice _zoneChoice;
    private javax.swing.JButton cancelButton;
    private javax.swing.JButton executeButton;
    private javax.swing.JPanel fieldSeparatorGroup;
    private javax.swing.JLabel jLabel1;
    private javax.swing.JLabel jLabel10;
    private javax.swing.JLabel jLabel11;
    private javax.swing.JLabel jLabel2;
    private javax.swing.JLabel jLabel3;
    private javax.swing.JLabel jLabel4;
    private javax.swing.JLabel jLabel5;
    private javax.swing.JLabel jLabel6;
    private javax.swing.JLabel jLabel7;
    private javax.swing.JLabel jLabel8;
    private javax.swing.JLabel jLabel9;
    private javax.swing.JScrollPane jScrollPane1;
    private javax.swing.JScrollPane jScrollPane2;
    private javax.swing.JScrollPane jScrollPane3;
    private javax.swing.JScrollPane jScrollPane4;
    private javax.swing.JSplitPane jSplitPane1;
    private javax.swing.JTextArea jTextArea1;
    private javax.swing.JTextPane jTextPane1;
    private javax.swing.JButton nextButton;
    private javax.swing.JButton previousButton;
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

        private WaitCursor waitCursor = new WaitCursor(ImportWizardDialog.this);
        private FileImporter _importer;

        @Override
        public Void doInBackground() {
            try {
                cancelButton.addActionListener(this);
                previousButton.setEnabled(false);
                executeButton.setEnabled(false);
                _progressBar.setVisible(true);
                _progressBar.setValue(0);
                VariableMappingTableModel model = (VariableMappingTableModel) _fieldMapTable.getModel();
                _importer = new FileImporter(getImportSource(),
                        model.variables_static,
                        _fileType,
                        _sourceDataFileType,
                        _destinationFile,
                        _progressBar);
                _importer.importFile(Integer.parseInt(_ignoreRowsTextField.getText()));
                _cancelled = _importer.getCancelled();
                setVisible(false);
            } catch (IOException e) {
                JOptionPane.showMessageDialog(ImportWizardDialog.this,
                        "The import wizard encountered an error attempting to create the import file.\n" +
                        "This is most likely occuring because write permissions are not granted for\n" +
                        "specified directory. Please check path or review user permissions for specified directory.\n", "Note", JOptionPane.WARNING_MESSAGE);
            } catch (SecurityException e) {
                JOptionPane.showMessageDialog(ImportWizardDialog.this,
                        "The import wizard encountered an error attempting to create the import file.\n" +
                        "This is most likely occuring because write permissions are not granted for\n" +
                        "specified directory. Please check path or review user permissions for specified directory.\n", "Note", JOptionPane.WARNING_MESSAGE);
            } catch (ImportException e) {
                JOptionPane.showMessageDialog(ImportWizardDialog.this, e.getMessage(), "Note", JOptionPane.ERROR_MESSAGE);
            } catch (Throwable t) {
                new ExceptionDialog(ImportWizardDialog.this, t).setVisible(true);
            } finally {
                cancelButton.removeActionListener(this);
            }
            return null;
        }

        /*
         * Executed in event dispatching thread
         */
        @Override
        public void done() {
            executeButton.setEnabled(true);
            previousButton.setEnabled(true);
            waitCursor.restore();
        }

        public void actionPerformed(ActionEvent e) {
            _importer.setCancelled();
        }
    }
}
