/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package org.satscan.gui;

import java.awt.CardLayout;
import java.awt.Dialog;
import java.awt.event.WindowAdapter;
import java.awt.event.WindowEvent;
import java.io.File;
import java.util.ArrayList;
import java.util.List;
import javax.swing.JTextField;
import javax.swing.event.HyperlinkEvent;
import javax.swing.event.HyperlinkListener;
import org.satscan.app.AppConstants;
import org.satscan.app.Parameters;
import org.satscan.app.UnknownEnumException;
import org.satscan.gui.utils.FileSelectionDialog;
import org.satscan.gui.utils.InputFileFilter;
import org.satscan.gui.utils.help.HelpShow;
import org.satscan.importer.InputSourceSettings;
import static org.satscan.importer.InputSourceSettings.InputFileType.AdjustmentsByRR;
import static org.satscan.importer.InputSourceSettings.InputFileType.AlternativeHypothesis;
import static org.satscan.importer.InputSourceSettings.InputFileType.Case;
import static org.satscan.importer.InputSourceSettings.InputFileType.Control;
import static org.satscan.importer.InputSourceSettings.InputFileType.Coordinates;
import static org.satscan.importer.InputSourceSettings.InputFileType.MaxCirclePopulation;
import static org.satscan.importer.InputSourceSettings.InputFileType.MetaLocations;
import static org.satscan.importer.InputSourceSettings.InputFileType.Neighbors;
import static org.satscan.importer.InputSourceSettings.InputFileType.Population;
import static org.satscan.importer.InputSourceSettings.InputFileType.SpecialGrid;
import static org.satscan.importer.InputSourceSettings.SourceDataFileType.Excel;
import static org.satscan.importer.InputSourceSettings.SourceDataFileType.dBase;

/**
 * @author hostovic
 */
public class FileSourceSetting extends javax.swing.JDialog {
    private InputSourceSettings _input_source_settings;
    private JTextField _destination;
    private final ParameterSettingsFrame _settingsFrame;
    private String PRIMARY_CARD = "primary-card";
    private List<InputFileFilter> _filters=new ArrayList<InputFileFilter>();
    private String _browse_title="";
    private boolean _importable_file_type=true;
    
    /**
     * Creates new form FileSourceSetting
     */
    public FileSourceSetting(java.awt.Frame parent, ParameterSettingsFrame settingsFrame, JTextField destination, InputSourceSettings inputSourceSettings) {
        super(parent, false);
        _input_source_settings = inputSourceSettings;
        initComponents();
        String fileTypeName = getFileTypeAsString();
        _browse_title = "Select " + fileTypeName + " File";
        _file_selection_label.setText("Select " + fileTypeName + " File Source:");
        switch (_input_source_settings.getInputFileType()) {
            case Case : 
                _filters = ImportWizardDialog.getInputFilters();                
                _filters.add(new InputFileFilter("cas", fileTypeName + " Files (*.cas)"));
                break;
            case Control : 
                _filters = ImportWizardDialog.getInputFilters();                
                _filters.add(new InputFileFilter("ctl", fileTypeName + " Files (*.ctl)"));
                break;
            case Population : 
                _filters = ImportWizardDialog.getInputFilters();                
                _filters.add(new InputFileFilter("pop", fileTypeName + " Files (*.pop)"));
                break;
            case Coordinates : 
                _filters = ImportWizardDialog.getInputFilters();                
                _filters.add(new InputFileFilter("geo", fileTypeName + " Files (*.geo)"));
                break;
            case SpecialGrid : 
                _filters = ImportWizardDialog.getInputFilters();                
                _filters.add(new InputFileFilter("grd", fileTypeName + " Files (*.grd)"));
                break;
            case MaxCirclePopulation : 
                _filters = ImportWizardDialog.getInputFilters();                
                _filters.add(new InputFileFilter("max", fileTypeName + " iles (*.max)"));
                break;
            case AdjustmentsByRR : 
                _filters = ImportWizardDialog.getInputFilters();                
                _filters.add(new InputFileFilter("adj", fileTypeName + " Files (*.adj)"));
                break;
            case Neighbors:
                _filters.add(new InputFileFilter("nei", fileTypeName + " Files (*.nei)"));
                _launch_import_wizard.setVisible(false);
                _importable_file_type = false;                  
                break;
            case MetaLocations:
                _filters.add(new InputFileFilter("meta", fileTypeName + " Files (*.meta)"));
                _launch_import_wizard.setVisible(false);
                _importable_file_type = false;                    
                break;
            case AlternativeHypothesis:
                _filters.add(new InputFileFilter("adj", fileTypeName + " Files (*.ha)"));
                _launch_import_wizard.setVisible(false);
                _importable_file_type = false;                       
                break;
           default: throw new UnknownEnumException(_input_source_settings.getInputFileType());
        }            
        _destination = destination;
        _source_filename.setText(_destination.getText());
        _settingsFrame = settingsFrame;
        
        _expectedFormatTextPane.setText(getFileExpectedFormatHtml());
        _expectedFormatTextPane.setCaretPosition(0);

        _expectedFormatTextPane.addHyperlinkListener(new HyperlinkListener() {
            public void hyperlinkUpdate(HyperlinkEvent e) {
                if(e.getEventType() == HyperlinkEvent.EventType.ACTIVATED) {
                    HelpShow.showHelp(e.getDescription());
                }
            }
        });        
        
        ((CardLayout) _body_panel.getLayout()).show(_body_panel, PRIMARY_CARD);
        if (!_input_source_settings.isSet()) {
            _expectedFormatTextPane.setText(getFileExpectedFormatHtml());            
            _expectedFormatTextPane.setCaretPosition(0);
            _clear_import_settings.setVisible(false);
        }
        else {
            _expectedFormatTextPane.setText(getMappingsHtml());
            _expectedFormatTextPane.setCaretPosition(0);
        }
        
        addWindowListener(new WindowAdapter() {
            public void windowClosed(WindowEvent e) {
                _destination.setText(_source_filename.getText());
            }
        });               

        setLocationRelativeTo(parent);
    }

    /* Returns file type as text string. */
    private String getFileTypeAsString() {
        switch (_input_source_settings.getInputFileType()) {
            case Case : return new String("Case");
            case Control: return new String("Control");
            case Population: return new String("Population");
            case Coordinates: return new String("Coordinates");              
            case SpecialGrid: return new String("Grid");
            case MaxCirclePopulation: return new String("Maximum Circle");
            case AdjustmentsByRR: return new String("Adjustments");
            case Neighbors: return new String("Non-Euclidian Neighbors");
            case MetaLocations: return new String("Meta Locations");
            case AlternativeHypothesis: return new String("Alternative Hypothesis");
            default: throw new UnknownEnumException(_input_source_settings.getInputFileType());
        }                
    }
    
    private String getFileExpectedFormatParagraphs() {
        String heplID="";
        StringBuilder builder = new StringBuilder();
        builder.append("<p style=\"margin-top: 0;\">The expected format of the ");
        builder.append(getFileTypeAsString().toLowerCase());
        builder.append(" file with the ").append(Parameters.GetProbabilityModelTypeAsString(_settingsFrame.getModelControlType()));
        builder.append(" probability model is:</p>").append("<span style=\"margin: 5px 0 0 5px;font-style:italic;font-weight:bold;\">");     
        switch (_input_source_settings.getInputFileType()) {
            case Case :
                heplID = AppConstants.CASEFILE_HELPID;
                switch (_settingsFrame.getModelControlType()) {
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
                    default: throw new UnknownEnumException(_settingsFrame.getModelControlType());
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
                if (_settingsFrame.getCoordinatesType() == Parameters.CoordinatesType.CARTESIAN)
                    builder.append("&lt;Location ID&gt;  &lt;X-Coordinate&gt;  &lt;Y-Ccoordinate&gt;  &lt;Z1-Coordinate&gt; ...  &lt;ZN-Coordinate&gt;");
                else
                    builder.append("&lt;Location ID&gt;  &lt;Latitude&gt;  &lt;Longitude&gt;");
                break;                
            case SpecialGrid: builder.append("");
                heplID = AppConstants.GRIDFILE_HELPID;
                if (_settingsFrame.getCoordinatesType() == Parameters.CoordinatesType.CARTESIAN)
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
        builder.append("&nbsp;&nbsp;</span><span style=\"padding-left:20px;\">(<a style=\"font-weight:bold;color:black;font-size:smaller;\" href=\"").append(heplID).append("\">More Information</a>)</span>");
        return builder.toString();
    }
    
    private String getFileExpectedFormatHtml() {
        StringBuilder builder = new StringBuilder();
        builder.append("<html><head><style>th {font-weight:bold;text-align:right;}</style></head><body>");
        builder.append(getFileExpectedFormatParagraphs());
        if (_importable_file_type) {
            builder.append("<p>If the selected file is not SaTScan formatted, or fields are not in the expected order, you can use the import wizard to specify how this file should be read.</p>");
        }
        builder.append("</body></html>");
        return builder.toString();
    }

    private String getMappingsHtml() {
        StringBuilder builder = new StringBuilder();
        builder.append("<html><head><style>th {font-weight: bold;text-align:right;}</style></head><body>"); 
        builder.append(getFileExpectedFormatParagraphs());
        builder.append("<p style=\"margin-top: 10px;\">The ").append(getFileTypeAsString().toLowerCase());
        builder.append(" file has been set to read from the specified source file at the time of analysis. The file will be read according to the following settings:</p>");
        builder.append("<div style=\"margin-top: 5px;\"><table><tr><td valign=\"top\"><table >");        
        switch (_input_source_settings.getSourceDataFileType()) {
            case CSV : 
                builder.append("<tr><th>File Type:</th><td>CSV</td></tr>");
                builder.append("<tr><th>Field Delimiter Character:</th><td>" + _input_source_settings.getDelimiter() + "</td></tr>");
                builder.append("<tr><th>Field Group Character:</th><td>" + _input_source_settings.getGroup() + "</td></tr>");
                builder.append("<tr><th>Lines Skipped:</th><td>" + _input_source_settings.getSkiplines() + "</td></tr>");
                builder.append("<tr><th>Header First Column:</th><td>" + (_input_source_settings.getFirstRowHeader() ? "Yes" : "No") + "</td></tr>");
                break;
            case dBase : builder.append("<tr><th>File Type:</th><td>dBase</td></tr>"); break;
            case Excel : builder.append("<tr><th>File Type:</th><td>Excel</td></tr>"); break;
            case Shapefile : 
                builder.append("<tr><th>File Type:</th><td>Shapefile</td></tr>");
                if (_input_source_settings.getShapeCoordinatesType() == InputSourceSettings.ShapeCoordinatesType.LATLONG_DATA) {
                    builder.append("<tr><th>Coordinates:</th><td>Latitude / Longitude</td></tr>");                
                } else if (_input_source_settings.getShapeCoordinatesType() == InputSourceSettings.ShapeCoordinatesType.UTM_CONVERSION) {
                    builder.append("<tr><th>Coordinates:</th><td>UTM</td></tr>");
                    builder.append("<tr><th>Hemisphere:</th><td>").append(_input_source_settings.getHemisphere()).append("</td></tr>");
                    builder.append("<tr><th>Zone:</th><td>").append(Integer.toString(_input_source_settings.getZone())).append("</td></tr>");
                    builder.append("<tr><th>Northing:</th><td>").append(Double.toString(_input_source_settings.getNorthing())).append("</td></tr>");
                    builder.append("<tr><th>Easting:</th><td>").append(Double.toString(_input_source_settings.getEasting())).append("</td></tr>");
                } else if (_input_source_settings.getShapeCoordinatesType() == InputSourceSettings.ShapeCoordinatesType.UTM_CONVERSION) {
                    builder.append("<tr><th>Coordinates:</th><td>Cartesian</td></tr>");
                }
                break;
        }
        builder.append("</table></td>");        
        builder.append("<td valign=\"top\"><table><tr><th valign=\"top\">Field Mapping:</th><td><table>");        
        for (int i=0; i < _input_source_settings.getFieldMaps().size(); ++i) {
            builder.append("<tr><td>Field " + (i + 1) + "</td><td> &#8667;</td> <td>");
            if (_input_source_settings.getFieldMaps().get(i).isEmpty()) {
                builder.append("---");
            } else {
                builder.append("Data Column " + (_input_source_settings.getFieldMaps().get(i)));
            }
            builder.append("</td></tr>");
        }          
        builder.append("</table></td></tr></table></td></tr></table></div></body></html>");
        return builder.toString();
    }
    
    /** Returns a InputSourceSettings object -- could be null if user choice to import now.
     */
    public InputSourceSettings getInputSourceSettings() {
        return _input_source_settings;
    }    
    
    /** Modally shows import dialog. */
    public void launchImporter(String sFileName) {
        ImportWizardDialog wizard = new ImportWizardDialog(SaTScanApplication.getInstance(), 
                                                           sFileName, 
                                                           _settingsFrame.getParameters().GetSourceFileName(), 
                                                           _input_source_settings,
                                                           _settingsFrame.getModelControlType(), 
                                                           _settingsFrame.getCoordinatesType());
        wizard.setVisible(true);
        if (!wizard.getCancelled()) {
            _expectedFormatTextPane.setText(getFileExpectedFormatHtml());
            _expectedFormatTextPane.setCaretPosition(0);            
            if (wizard.getExecutedImport()) {
                _input_source_settings.getFieldMaps().clear(); 
                _source_filename.setText(wizard.getDestinationFilename());
                _destination.setText(wizard.getDestinationFilename());
            } else {
                // user choice to read source at time of analysis
                wizard.getInputSourceSettings(_input_source_settings);
                _expectedFormatTextPane.setText(getMappingsHtml());
                _expectedFormatTextPane.setCaretPosition(0);
                _destination.setText(_source_filename.getText());
            }
            // update parameter settings back in the settings window to reflect users selection in wizard
            switch (_input_source_settings.getInputFileType()) {
                case Case:
                    _settingsFrame.setPrecisionOfTimesControl(wizard.getDateFieldImported() ? (_settingsFrame.getPrecisionOfTimesControlType() == Parameters.DatePrecisionType.NONE ? Parameters.DatePrecisionType.YEAR : _settingsFrame.getPrecisionOfTimesControlType()) : Parameters.DatePrecisionType.NONE);
                    _settingsFrame.setModelControl(wizard.getModelControlType());
                    break;
                case Control:
                    _settingsFrame.setPrecisionOfTimesControl(wizard.getDateFieldImported() ? (_settingsFrame.getPrecisionOfTimesControlType() == Parameters.DatePrecisionType.NONE ? Parameters.DatePrecisionType.YEAR : _settingsFrame.getPrecisionOfTimesControlType()) : Parameters.DatePrecisionType.NONE);
                    _settingsFrame.setModelControl(Parameters.ProbabilityModelType.BERNOULLI);
                    break;
                case Population: _settingsFrame.setModelControl(Parameters.ProbabilityModelType.POISSON); break;
                case Coordinates: _settingsFrame.setCoordinateType(wizard.getCoorinatesControlType()); break;
                case SpecialGrid: _settingsFrame.setCoordinateType(wizard.getCoorinatesControlType()); break;
                case MaxCirclePopulation: 
                case AdjustmentsByRR:
                case Neighbors:
                case MetaLocations:
                case AlternativeHypothesis: break;
                default: throw new UnknownEnumException(_input_source_settings.getInputFileType());
            }
        }
    }
    
    
    /**
     * This method is called from within the constructor to initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is always
     * regenerated by the Form Editor.
     */
    @SuppressWarnings("unchecked")
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

        _file_selection_label = new javax.swing.JLabel();
        _source_filename = new javax.swing.JTextField();
        _browse_source = new javax.swing.JButton();
        _body_panel = new javax.swing.JPanel();
        _primary_card = new javax.swing.JPanel();
        _expectedFormatScrollPane = new javax.swing.JScrollPane();
        _expectedFormatTextPane = new javax.swing.JTextPane();
        _launch_import_wizard = new javax.swing.JButton();
        _close = new javax.swing.JButton();
        _clear_import_settings = new javax.swing.JButton();

        setDefaultCloseOperation(javax.swing.WindowConstants.DISPOSE_ON_CLOSE);
        setTitle("Source File Settings");

        _file_selection_label.setText("Select Case File Source:");

        _browse_source.setText("..."); // NOI18N
        _browse_source.setToolTipText("Browse for source file ..."); // NOI18N
        _browse_source.setPreferredSize(new java.awt.Dimension(45, 20));
        _browse_source.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                FileSelectionDialog select = new FileSelectionDialog(SaTScanApplication.getInstance(), _browse_title, _filters, SaTScanApplication.getInstance().lastBrowseDirectory);
                File file = select.browse_load(true);
                if (file != null) {
                    SaTScanApplication.getInstance().lastBrowseDirectory = select.getDirectory();
                    _source_filename.setText(file.getAbsolutePath());
                }
            }
        });

        _body_panel.setLayout(new java.awt.CardLayout());

        _expectedFormatScrollPane.setBorder(null);
        _expectedFormatScrollPane.setHorizontalScrollBarPolicy(javax.swing.ScrollPaneConstants.HORIZONTAL_SCROLLBAR_NEVER);

        _expectedFormatTextPane.setEditable(false);
        _expectedFormatTextPane.setBackground(new java.awt.Color(240, 240, 240));
        _expectedFormatTextPane.setContentType("text/html"); // NOI18N
        _expectedFormatTextPane.setText("<html>\r\n  <head>\r</head>\r\n  <body>\r\n    <p style=\"margin-top: 0;\">\r\rThe expected format of the case file, for the current settings, is:</p>\r\n    <p style=\"margin: 5px 0 0 5px;font-style:italic;font-size:1.01em;font-weight:bold;\">&lt;indentifier&gt;  &lt;count&gt; &lt;date&gt; &lt;covariate 1&gt; ... &lt;covariate N&gt;</p>\n  </body>\r\n</html>\r\n");
        _expectedFormatScrollPane.setViewportView(_expectedFormatTextPane);

        _launch_import_wizard.setText("Launch Import Wizard");
        _launch_import_wizard.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                try {
                    if (_source_filename.getText().trim().isEmpty()) {
                        FileSelectionDialog select = new FileSelectionDialog(SaTScanApplication.getInstance(), _browse_title, _filters, SaTScanApplication.getInstance().lastBrowseDirectory);
                        File file = select.browse_load(true);
                        if (file != null) {
                            SaTScanApplication.getInstance().lastBrowseDirectory = select.getDirectory();
                            _source_filename.setText(file.getAbsolutePath());
                        }
                    }
                    if (!_source_filename.getText().trim().isEmpty()) {
                        launchImporter(_source_filename.getText().trim());
                    }
                } catch (Throwable t) {
                    new ExceptionDialog(FileSourceSetting.this, t).setVisible(true);
                }
            }
        });

        _close.setText("Close");
        _close.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                dispose();
            }
        });

        _clear_import_settings.setText("Clear Import Settings");
        _clear_import_settings.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                try {
                    _input_source_settings.getFieldMaps().clear();
                    _expectedFormatTextPane.setText(getFileExpectedFormatHtml());
                    _expectedFormatTextPane.setCaretPosition(0);
                } catch (Throwable t) {
                    new ExceptionDialog(SaTScanApplication.getInstance(), t).setVisible(true);
                }
            }
        });

        javax.swing.GroupLayout _primary_cardLayout = new javax.swing.GroupLayout(_primary_card);
        _primary_card.setLayout(_primary_cardLayout);
        _primary_cardLayout.setHorizontalGroup(
            _primary_cardLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_primary_cardLayout.createSequentialGroup()
                .addGroup(_primary_cardLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(_primary_cardLayout.createSequentialGroup()
                        .addComponent(_launch_import_wizard, javax.swing.GroupLayout.PREFERRED_SIZE, 170, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(_clear_import_settings)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                        .addComponent(_close))
                    .addComponent(_expectedFormatScrollPane, javax.swing.GroupLayout.DEFAULT_SIZE, 635, Short.MAX_VALUE))
                .addContainerGap())
        );
        _primary_cardLayout.setVerticalGroup(
            _primary_cardLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, _primary_cardLayout.createSequentialGroup()
                .addComponent(_expectedFormatScrollPane, javax.swing.GroupLayout.DEFAULT_SIZE, 211, Short.MAX_VALUE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(_primary_cardLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(_launch_import_wizard)
                    .addComponent(_close)
                    .addComponent(_clear_import_settings)))
        );

        _body_panel.add(_primary_card, "primary-card");

        javax.swing.GroupLayout layout = new javax.swing.GroupLayout(getContentPane());
        getContentPane().setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(_body_panel, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(_file_selection_label, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addGroup(layout.createSequentialGroup()
                        .addComponent(_source_filename)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(_browse_source, javax.swing.GroupLayout.PREFERRED_SIZE, 25, javax.swing.GroupLayout.PREFERRED_SIZE)))
                .addContainerGap())
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(layout.createSequentialGroup()
                .addContainerGap()
                .addComponent(_file_selection_label)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(_source_filename, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(_browse_source, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_body_panel, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .addContainerGap())
        );

        pack();
    }// </editor-fold>//GEN-END:initComponents

    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JPanel _body_panel;
    private javax.swing.JButton _browse_source;
    private javax.swing.JButton _clear_import_settings;
    private javax.swing.JButton _close;
    private javax.swing.JScrollPane _expectedFormatScrollPane;
    private javax.swing.JTextPane _expectedFormatTextPane;
    private javax.swing.JLabel _file_selection_label;
    private javax.swing.JButton _launch_import_wizard;
    private javax.swing.JPanel _primary_card;
    private javax.swing.JTextField _source_filename;
    // End of variables declaration//GEN-END:variables
}
