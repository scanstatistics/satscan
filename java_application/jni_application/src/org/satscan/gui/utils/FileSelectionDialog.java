/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package org.satscan.gui.utils;

import java.awt.Component;
import java.awt.FileDialog;
import java.awt.Frame;
import java.io.File;
import java.io.FilenameFilter;
import java.util.ArrayList;
import java.util.List;
import javax.swing.JFileChooser;
import javax.swing.JTextField;
import org.satscan.app.Parameters;
import org.satscan.app.UnknownEnumException;
import org.satscan.gui.FileSourceWizard;
import org.satscan.gui.ParameterSettingsFrame;
import org.satscan.gui.SaTScanApplication;
import org.satscan.importer.InputSourceSettings;

/**
 *
 * @author hostovic
 */
public class FileSelectionDialog {
    private JFileChooser _file_chooser=null;
    private FileDialog _file_dialog=null;
    private File _lastBrowseDirectory;
    private Component _parent;
    
    public FileSelectionDialog(final Component parent, final String title, final List<InputFileFilter> filters, final File lastBrowseDirectory) {
        setup(parent, title, filters, lastBrowseDirectory);
    }

    public FileSelectionDialog(final Component parent, final InputSourceSettings.InputFileType fileType, final File lastBrowseDirectory) {
        String browse_title;
        List<InputFileFilter> filters = new ArrayList<InputFileFilter>();
        
        browse_title = "Select " + getFileTypeAsString(fileType) +" File";
        switch (fileType) {
            case Case :
                filters = FileSourceWizard.getInputFilters();                
                filters.add(new InputFileFilter("cas", "Case Files (*.cas)"));
                break;
            case Control : 
                filters = FileSourceWizard.getInputFilters();                
                filters.add(new InputFileFilter("ctl", "Control Files (*.ctl)"));
                break;
            case Population : 
                filters = FileSourceWizard.getInputFilters();                
                filters.add(new InputFileFilter("pop", "Population Files (*.pop)"));
                break;
            case Coordinates : 
                filters = FileSourceWizard.getInputFilters();                
                filters.add(new InputFileFilter("geo", "Coordinates Files (*.geo)"));
                break;
            case SpecialGrid : 
                filters = FileSourceWizard.getInputFilters();                
                filters.add(new InputFileFilter("grd", "Grid Files (*.grd)"));
                break;
            case MaxCirclePopulation : 
                filters = FileSourceWizard.getInputFilters();                
                filters.add(new InputFileFilter("max", "Maximum Circle Files (*.max)"));
                break;
            case AdjustmentsByRR : 
                filters = FileSourceWizard.getInputFilters();                
                filters.add(new InputFileFilter("adj", "Adjustments Files (*.adj)"));
                break;
            case Neighbors:
                filters.add(new InputFileFilter("nei", "Non-Euclidian Neighbors Files (*.nei)"));
                break;
            case MetaLocations:
                filters.add(new InputFileFilter("meta", "Meta Locations Files (*.meta)"));
                break;
            case AlternativeHypothesis:
                filters.add(new InputFileFilter("adj", "Alternative Hypothesis Files (*.ha)"));
                break;
           default: throw new UnknownEnumException(fileType);
        }            
        setup(parent, browse_title, filters, lastBrowseDirectory);
    }    
    
    public FileSelectionDialog(final Component parent, final String title, final File lastBrowseDirectory) {
        _parent = parent;
        _lastBrowseDirectory = lastBrowseDirectory;
        if (System.getProperty("os.name").toLowerCase().startsWith("mac")) {
            System.setProperty("apple.awt.fileDialogForDirectories", "true");
            _file_dialog = new FileDialog((Frame)parent, title);
            _file_dialog.setDirectory(lastBrowseDirectory.getAbsolutePath());
        } else {
            _file_chooser = new JFileChooser(lastBrowseDirectory);
            _file_chooser.setDialogTitle(title);
            _file_chooser.setFileSelectionMode(JFileChooser.DIRECTORIES_ONLY);
        }
    }    
    
    /* Returns file type as text string. */
    public static String getFileTypeAsString(InputSourceSettings.InputFileType fileType) {
        switch (fileType) {
            case Case : return "Case";
            case Control: return "Control";
            case Population: return "Population";
            case Coordinates: return "Coordinates";              
            case SpecialGrid: return "Grid";
            case MaxCirclePopulation: return "Maximum Circle";
            case AdjustmentsByRR: return "Adjustments";
            case Neighbors: return "Non-Euclidian Neighbors";
            case MetaLocations: return "Meta Locations";
            case AlternativeHypothesis: return "Alternative Hypothesis";
            default: throw new UnknownEnumException(fileType);
        }                
    }    
    
    public void setup(final Component parent, final String title, final List<InputFileFilter> filters, final File lastBrowseDirectory) {
        _parent = parent;
        _lastBrowseDirectory = lastBrowseDirectory;
        if (System.getProperty("os.name").toLowerCase().startsWith("mac")) {
            System.setProperty("apple.awt.fileDialogForDirectories", "false");
            _file_dialog = new FileDialog((Frame)parent, title);
            _file_dialog.setDirectory(lastBrowseDirectory.getAbsolutePath());
            _file_dialog.setFilenameFilter(new FilenameFilter(){
                    @Override
                    public boolean accept(File dir, String name) {
                        for (int f=0; f < filters.size(); f++) {                        
                            if (name.endsWith("." + filters.get(f).getFilter())) {
                                return true;
                            }
                        }
                        return false;
                    }
            });                            
        } else {
            _file_chooser = new JFileChooser(lastBrowseDirectory);
            _file_chooser.setDialogTitle(title);
            for (int f=0; f < filters.size(); f++) {
                _file_chooser.addChoosableFileFilter(filters.get(f));
            }
        }
    }
    
    /*
     * Browses for the input source file ...
     */
    public void browse_inputsource(JTextField inputSourceFilename, InputSourceSettings inputSourceSettings, ParameterSettingsFrame settingsFrame) {
        String filename = null;
        
        // If the input source filename is blank, display the file browse dialog.
        if (inputSourceFilename.getText().isEmpty()) {
           File file = browse_load(true);
           if (file != null) {
             filename = file.getAbsolutePath();
             SaTScanApplication.getInstance().lastBrowseDirectory = getDirectory();
           }
        } else {
            filename = inputSourceFilename.getText();
        }
        
        // If we have a filename at this point, display the file source wizard.
        if (filename != null) {
            FileSourceWizard wizard = new FileSourceWizard(SaTScanApplication.getInstance(), 
                                                           filename, 
                                                           settingsFrame.getParameters().GetSourceFileName(), 
                                                           inputSourceSettings,
                                                           settingsFrame.getModelControlType(), 
                                                           settingsFrame.getCoordinatesType());
            wizard.setVisible(true);
            if (wizard.getExecutedImport()) {
                inputSourceSettings.reset();
                inputSourceFilename.setText(wizard.getDestinationFilename());
            } else {
                if (wizard.getNeedsImportSourceSave())
                    inputSourceSettings.copy(wizard.getInputSourceSettings());
                inputSourceFilename.setText(wizard.getSourceFilename());
            }
            if (wizard.getExecutedImport() || wizard.getNeedsImportSourceSave()) {
                // update parameter settings back in the settings window to reflect users selection in wizard
                switch (inputSourceSettings.getInputFileType()) {
                    case Case:
                        settingsFrame.setPrecisionOfTimesControl(wizard.getDateFieldImported() ? (settingsFrame.getPrecisionOfTimesControlType() == Parameters.DatePrecisionType.NONE ? Parameters.DatePrecisionType.YEAR : settingsFrame.getPrecisionOfTimesControlType()) : Parameters.DatePrecisionType.NONE);
                        settingsFrame.setModelControl(wizard.getModelControlType());
                        break;
                    case Control:
                        settingsFrame.setPrecisionOfTimesControl(wizard.getDateFieldImported() ? (settingsFrame.getPrecisionOfTimesControlType() == Parameters.DatePrecisionType.NONE ? Parameters.DatePrecisionType.YEAR : settingsFrame.getPrecisionOfTimesControlType()) : Parameters.DatePrecisionType.NONE);
                        settingsFrame.setModelControl(Parameters.ProbabilityModelType.BERNOULLI);
                        break;
                    case Population: settingsFrame.setModelControl(Parameters.ProbabilityModelType.POISSON); break;
                    case Coordinates: settingsFrame.setCoordinateType(wizard.getCoorinatesControlType()); break;
                    case SpecialGrid: settingsFrame.setCoordinateType(wizard.getCoorinatesControlType()); break;
                    case MaxCirclePopulation: 
                    case AdjustmentsByRR:
                    case Neighbors:
                    case MetaLocations:
                    case AlternativeHypothesis: break;
                    default: throw new UnknownEnumException(inputSourceSettings.getInputFileType());
                }                
            }
        }        
    }    
    
    public File browse_load(boolean require_exits) {
        File file = null;
        if (_file_dialog != null) {
            _file_dialog.setVisible(true);
            _file_dialog.setMode(FileDialog.LOAD);
            if (_file_dialog.getFile() != null) {
                _lastBrowseDirectory = new File(_file_dialog.getDirectory());
                file = new File(_file_dialog.getDirectory() +  _file_dialog.getFile());                
            }
        } else {
            int returnVal = _file_chooser.showOpenDialog(_parent);
            if (returnVal == JFileChooser.APPROVE_OPTION) {
                _lastBrowseDirectory = _file_chooser.getCurrentDirectory();
                file = new File(_file_chooser.getSelectedFile().getAbsolutePath());
            }            
        }
        return file == null ? null : (require_exits && !file.exists() ? null : file);
    }

    public File browse_saveas() {
        File file = null;
        if (_file_dialog != null) {
            _file_dialog.setMode(FileDialog.SAVE);
            _file_dialog.setVisible(true);
            if (_file_dialog.getFile() != null) {
                _lastBrowseDirectory = new File(_file_dialog.getDirectory());
                file = new File(_file_dialog.getDirectory() +  _file_dialog.getFile());                
            }
        } else {
            int returnVal = _file_chooser.showOpenDialog(_parent);
            if (returnVal == JFileChooser.APPROVE_OPTION) {
                _lastBrowseDirectory = _file_chooser.getCurrentDirectory();
                file = new File(_file_chooser.getSelectedFile().getAbsolutePath());
            }            
        }
        return file;
    }
    
    public File getDirectory() {
        return _lastBrowseDirectory;
    }
}
