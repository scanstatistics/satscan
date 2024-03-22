
package org.satscan.gui;

import java.awt.Component;
import java.awt.Cursor;
import java.awt.Dimension;
import java.awt.Graphics;
import java.awt.Point;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.beans.PropertyChangeEvent;
import java.beans.PropertyChangeListener;
import java.beans.PropertyVetoException;
import java.nio.file.attribute.FileTime;
import java.text.SimpleDateFormat;
import java.time.LocalDate;
import java.time.LocalDateTime;
import java.time.ZoneId;
import java.time.format.DateTimeFormatter;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.Date;
import java.util.Iterator;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.logging.Level;
import java.util.logging.Logger;
import java.util.stream.Collectors;
import javax.swing.DefaultCellEditor;
import javax.swing.Icon;
import javax.swing.ImageIcon;
import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JInternalFrame;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JRootPane;
import javax.swing.JTable;
import javax.swing.JTextField;
import javax.swing.SwingUtilities;
import javax.swing.SwingWorker;
import javax.swing.event.InternalFrameEvent;
import javax.swing.event.InternalFrameListener;
import javax.swing.event.TableModelEvent;
import javax.swing.event.TableModelListener;
import javax.swing.table.DefaultTableCellRenderer;
import javax.swing.table.DefaultTableModel;
import javax.swing.table.JTableHeader;
import javax.swing.table.TableCellEditor;
import javax.swing.table.TableCellRenderer;
import javax.swing.table.TableColumn;
import javax.swing.table.TableColumnModel;
import javax.swing.table.TableModel;
import org.apache.commons.lang3.tuple.ImmutablePair;
import org.satscan.app.BatchAnalysis;
import org.satscan.app.BatchXMLFile;
import org.satscan.app.CalculationThread;
import org.satscan.app.OutputFileRegister;
import org.satscan.app.Parameters;
import org.satscan.gui.utils.WaitCursor;
import org.apache.commons.vfs2.FileChangeEvent;
import org.apache.commons.vfs2.FileListener;
import org.apache.commons.vfs2.FileSystemException;
import org.apache.commons.vfs2.VFS;
import org.apache.commons.vfs2.impl.DefaultFileMonitor;

/**
 * BatchAnalysisFrame defines the internal frame which manages the creation, editing, execution, and display of
 * parameter settings. The key difference from normal parameter setting files is that these parameter settings
 * are stored in an XML file, not separate files, along with run results information. This allows the user to
 * execute multiple analyses in sequence either from the GUI or from the command-line application.
 */
public class BatchAnalysisFrame extends javax.swing.JInternalFrame implements InternalFrameListener {
    private final JRootPane _root_pane;
    private ArrayList<BatchAnalysis> _batch_analyses = new ArrayList();
    /* Map collection of analyses to open parameter setting windows. */
    private final LinkedHashMap<BatchAnalysis, ParameterSettingsFrame> _open_settings_frames = new LinkedHashMap();
    /* Full path of the xml file which stores analysis. */
    private final String _saved_filename = System.getProperty("user.home") + System.getProperty("file.separator") + ".satscan" + System.getProperty("file.separator") + "batch-settings.xml";
    public static final String DATE_FORMAT = "yyyy-MM-dd hh:mm aa";
    private static final String DEFAULT_STUDY_PERIOD = "Per Settings";
    private static final String DEFAULT_LAG = "No Lag";
    private final int SELECT_IDX = 0;
    private final int DESCRIPTION_IDX = 1;
    private final int ANALYSIS_IDX = 2;
    private final int MODEL_IDX = 3;
    private final int STUDYPERIOD_IDX = 4;
    private final int LAG_IDX = 5;
    private final int STATUS_IDX = 6;
    private boolean _table_initiallized = false;
    private final String _lag_helptext = "If specified, the Study Period End Date is replaced with a value relative to today’s date. For example, a Lag Time of 3 days would set the Study Period End Date equal to today’s date minus 3 days. If the No Lag box is checked, the Study Period End Date defaults to the date specified in the parameter settings.";
    private final String _study_period_helptext = "Study Length is the length of time between the Study Period Start Date and the Study Period End Date (inclusive). If specified, the Study Period Start Date is replaced with a value relative to the Study Period End Date. If the No Offset box is checked, the Study Period Start Date defaults to the date specified in the parameter settings.";
    private boolean _batch_executing = false;
    private FileTime _xml_last_modified;
    private DefaultFileMonitor _file_monitor;
    private BatchXMLFile.SummaryMailSettings _email_settings=new BatchXMLFile.SummaryMailSettings();
      
    /* Creates new form AnalysisBatchFrame */
    public BatchAnalysisFrame(final JRootPane root_pane) {
            initComponents();
            setFrameIcon(new ImageIcon(getClass().getResource("/SaTScan.png")));
            _root_pane = rootPane;
            addInternalFrameListener(this);
            pack();
            readBatchAnalysesFromFile();
            enableButtons();
            _xml_last_modified = org.satscan.app.BatchXMLFile.last_modified(_saved_filename);
            // Define file listener monitor to take action when the storage XML file is updated outside of this GUI.
            try {
                _file_monitor = new DefaultFileMonitor(new FileListener() {
                    private boolean assumedExternalUpdate() {
                        if (_xml_last_modified == null) return true;
                        LocalDateTime three_seconds_ago = LocalDateTime.now().minusSeconds(3);
                        LocalDateTime last_active_write = LocalDateTime.ofInstant(_xml_last_modified.toInstant(), ZoneId.systemDefault());
                        // Assume this listener wasn't triggered by the writeBatchAnalysesFromFile() method if stored file timestamp
                        // isn't in the last 3 seconds - implying that the file was updated through other means, for instance the command-line application.
                        return last_active_write.isBefore(three_seconds_ago);
                    }
                    @Override
                    public void fileCreated(FileChangeEvent fce) throws Exception { /* nop */ }
                    @Override
                    public void fileDeleted(FileChangeEvent fce) throws Exception { /* nop */ }
                    @Override
                    public void fileChanged(FileChangeEvent fce) throws Exception {
                        if (assumedExternalUpdate()) {
                            // We're maintaining the XML file in sync with changes to the JTable model,
                            // so we can merge the results from the file with existing BatchAnalysis objects.
                            // Doing this ensures that references to those objects remain valid in the open frames list
                            // and assuming this file was not edited manually, it's the only attributes that would be updated.
                            org.satscan.app.BatchXMLFile.readResults(_saved_filename, _batch_analyses);
                            ((DefaultTableModel)_analyses_table.getModel()).fireTableDataChanged();
                        }
                    }
                });
                _file_monitor.addFile(VFS.getManager().resolveFile(_saved_filename));
                _file_monitor.start();
            } catch (FileSystemException ex) { _file_monitor = null; }            
            _add_analysis.requestFocusInWindow();
    }
    
    /* Enables side buttons based on table and selection. */
    private void enableButtons() {
        if (!_batch_executing) {
            ArrayList<ImmutablePair<BatchAnalysis, Integer>> selected = getSelectedAnalysis();
            _add_analysis.setEnabled(true);
            _remove_analysis.setEnabled(!selected.isEmpty());
            _modify_analysis.setEnabled(selected.size() == 1);
            _duplicate_analysis.setEnabled(selected.size() == 1);
            _execute_selected.setEnabled(!selected.isEmpty() && _open_settings_frames.isEmpty());
            _moveUp.setEnabled(selected.size() == 1 && selected.get(0).right > 0);
            _moveDown.setEnabled(selected.size() == 1 && selected.get(0).right < _analyses_table.getModel().getRowCount() - 1);
            _summary_email.setEnabled(true);
        }
    }
    
    /* Disables all side buttons. */
    private void disableButtons() {
        for (Component c: _actions_panel.getComponents()) {
            c.setEnabled(false);
        }
    }
    
    public void redisplay() {
        setVisible(true);
        enableButtons();
        toFront();
    }    
    
    /* Returns date as string in defined format or empty string. */
    public static String dateToString(java.util.Date date) {
        if (date == null)
            return "";
        SimpleDateFormat dateFormat = new SimpleDateFormat(DATE_FORMAT);
        return dateFormat.format(date);
    }     
    
    /* Returns display string for StudyPeriodOffset object. */
    static private String getOffsetString(BatchAnalysis.StudyPeriodOffset offset, String defaultString) {
        if (offset == null)
            return defaultString;
        return offset.getOffset() + " " + BatchXMLFile.toString(offset.getUnits()) + (offset.getOffset() == 1 ? "" : "s");
    }
    
    /* Returns display string for BatchAnalysis object status. */
    static private String getStatusString(BatchAnalysis obj) {
        StringBuilder status = new StringBuilder(BatchXMLFile.toString(obj.getLastExecutedStatus()));
        if (obj.getLastExecutedStatus() != BatchAnalysis.STATUS.NEVER)
            status.append(" @ ").append(dateToString(obj.getLastExecutedDate())); 
        return status.toString();
    }    
    
    private Object[] getRowForBatchAnalysis(BatchAnalysis obj) {
        return new Object[]{ 
            obj.getSelected(), obj.getDescription(), obj.getParameters().GetAnalysisTypeAsString(true),
            obj.getParameters().GetModelTypeAsString(true), obj.getStudyPeriodLength(), obj.getLag(), obj
        };        
    }
    
    /* Reads all batch analysis settings from storage file. */
    private void readBatchAnalysesFromFile() {
        DefaultTableModel model = (DefaultTableModel) _analyses_table.getModel();
        model.setRowCount(0);
        _email_settings = org.satscan.app.BatchXMLFile.readSummaryMailSettings(_saved_filename);
        _batch_analyses = org.satscan.app.BatchXMLFile.readAnalyses(_saved_filename);
        for (BatchAnalysis batchAnalysis: _batch_analyses)
            model.addRow(getRowForBatchAnalysis(batchAnalysis));
        _table_initiallized = true;
    }
    
    /* Writes all batch analysis settings to storage file. */
    private void writeFile() {
        org.satscan.app.BatchXMLFile.write(_saved_filename, _batch_analyses, _email_settings);
        // store the date last modified, we'll use this information with file listener
        _xml_last_modified = org.satscan.app.BatchXMLFile.last_modified(_saved_filename);
    }
    
    /* Returns the selected BatchAnalysis or null. */
    private ArrayList<ImmutablePair<BatchAnalysis, Integer>> getSelectedAnalysis() {
        ArrayList<ImmutablePair<BatchAnalysis, Integer>> selected = new ArrayList();
        DefaultTableModel model = (DefaultTableModel) _analyses_table.getModel();
        for (int i=0; i < model.getRowCount(); i++) {
            Boolean obj = (Boolean)model.getValueAt(i, SELECT_IDX);
            if (obj)
                selected.add(new ImmutablePair<>(_batch_analyses.get(i), i));
        }
        return selected;
    }
    
    /* Ensures that none of the BatchAnalysis are selected. */
    private void removeSelection(BatchAnalysis exclude) {
        DefaultTableModel model = (DefaultTableModel) _analyses_table.getModel();
        for (int i=0; i < model.getRowCount(); i++) {
            BatchAnalysis batch = _batch_analyses.get(i);
            if (batch == exclude)
                continue;
            model.setValueAt(Boolean.FALSE, i, SELECT_IDX);
        }
        enableButtons();
    }
    
    /* Creates new multiple analysis from existing ParameterSettingsFrame object. */
    public void addParametersFromSessionWindow(final ParameterSettingsFrame other_frame) {
        WaitCursor waitCursor = new WaitCursor(BatchAnalysisFrame.this);
        try {
            BatchAnalysis copyAnalysis = new BatchAnalysis();
            copyAnalysis.setDescription("New Analysis");
            copyAnalysis.setLastExecutedDate(null);
            copyAnalysis.setLastExecutedStatus(BatchAnalysis.STATUS.NEVER);
            copyAnalysis.setParameters(other_frame.getParameterSettings());
            copyAnalysis.setSelected(true);
            _batch_analyses.add(copyAnalysis);
            // Add record to table.
            DefaultTableModel model = (DefaultTableModel) _analyses_table.getModel();
            model.addRow(getRowForBatchAnalysis(copyAnalysis));
            // Remove selection from any other records in table - to focus on new record.
            //removeSelection(copyAnalysis);
            // Show settngs window for new reocrd.
            ParameterSettingsFrame frame = new ParameterSettingsFrame(SaTScanApplication.getInstance().getRootPane(), BatchAnalysisFrame.this, copyAnalysis.getParameters());
            frame.setTitle(copyAnalysis.getDescription());
            frame.setIconifiable(false);
            frame.addInternalFrameListener(BatchAnalysisFrame.this);
            frame.setVisible(true);
            SaTScanApplication.getInstance().AddFrame(frame);
            _open_settings_frames.put(copyAnalysis, frame);
            frame.setSelected(true);
            enableButtons();
        } catch (PropertyVetoException t) {
            new ExceptionDialog(SaTScanApplication.getInstance(), t).setVisible(true);
        } finally {
            waitCursor.restore();
        }        
    }    
    
    /* Queries whether this window can close */
    public boolean queryCanClose() {
        if (_open_settings_frames.isEmpty()) {
            return true;
        } else {
            Iterator<BatchAnalysis> itr = _open_settings_frames.keySet().iterator();
            while (itr.hasNext()) {
                BatchAnalysis analysis = itr.next();
                ParameterSettingsFrame settings_frame = _open_settings_frames.get(analysis);
                settings_frame.toFront();
                if (!settings_frame.queryWindowCanClose())
                    return false; // skip attepting to close this batch frame                    
                // Re-assign parameters class associated with this nalysis panel.
                analysis.setParameters(settings_frame.getParameterSettings());                
                itr.remove();                
                settings_frame.dispose();
            }
            /* Now write the all parameter settings back to file -- maybe this only needs to be done on batch frame close or run? */
            writeFile();            
        }
        return true;
    }
    
    public static void setJTableColumnsWidth(JTable table, int tablePreferredWidth, double... percentages) {
        double total = 0;
        for (int i = 0; i < table.getColumnModel().getColumnCount(); i++) {
            total += percentages[i];
        }
 
        for (int i = 0; i < table.getColumnModel().getColumnCount(); i++) {
            TableColumn column = table.getColumnModel().getColumn(i);
            column.setPreferredWidth((int)(tablePreferredWidth * (percentages[i] / total)));
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

        _actionitems_panel = new javax.swing.JPanel();
        _actions_panel = new javax.swing.JPanel();
        _add_analysis = new javax.swing.JButton();
        _remove_analysis = new javax.swing.JButton();
        _modify_analysis = new javax.swing.JButton();
        _execute_selected = new javax.swing.JButton();
        _duplicate_analysis = new javax.swing.JButton();
        jSeparator2 = new javax.swing.JSeparator();
        _execute_progress = new javax.swing.JProgressBar();
        jSeparator3 = new javax.swing.JSeparator();
        _moveUp = new javax.swing.JButton();
        _moveDown = new javax.swing.JButton();
        _summary_email = new javax.swing.JButton();
        _analysesScrollpane = new javax.swing.JScrollPane();
        _analyses_table = new javax.swing.JTable();

        setClosable(true);
        setDefaultCloseOperation(javax.swing.WindowConstants.DO_NOTHING_ON_CLOSE);
        setResizable(true);
        setTitle("Multiple Analyses");
        setPreferredSize(new java.awt.Dimension(900, 400));

        _actions_panel.setBorder(javax.swing.BorderFactory.createEtchedBorder());

        _add_analysis.setText("New Analysis");
        _add_analysis.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                WaitCursor waitCursor = new WaitCursor(BatchAnalysisFrame.this);
                // Create new BatchAnalysis object and add to collection.
                BatchAnalysis newAnalysis = new BatchAnalysis();
                newAnalysis.setSelected(true);
                _batch_analyses.add(newAnalysis);
                // Add record to table.
                DefaultTableModel model = (DefaultTableModel) _analyses_table.getModel();
                model.addRow(getRowForBatchAnalysis(newAnalysis));
                // Remove selection from any other records in table - to focus on new record.
                removeSelection(newAnalysis);
                // Show settngs window for new reocrd.
                try {
                    ParameterSettingsFrame frame = new ParameterSettingsFrame(SaTScanApplication.getInstance().getRootPane(), BatchAnalysisFrame.this, newAnalysis.getParameters());
                    frame.setTitle(newAnalysis.getDescription());
                    frame.setIconifiable(false);
                    frame.addInternalFrameListener(BatchAnalysisFrame.this);
                    frame.setVisible(true);
                    SaTScanApplication.getInstance().AddFrame(frame);
                    frame.addInternalFrameListener(SaTScanApplication.getInstance());
                    _open_settings_frames.put(newAnalysis, frame);
                    frame.setSelected(true);
                } catch (Throwable t) {
                    new ExceptionDialog(SaTScanApplication.getInstance(), t).setVisible(true);
                } finally {
                    waitCursor.restore();
                }
            }
        });

        _remove_analysis.setText("Remove Selected");
        _remove_analysis.setEnabled(false);
        _remove_analysis.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                DefaultTableModel model = (DefaultTableModel) _analyses_table.getModel();
                ArrayList<ImmutablePair<BatchAnalysis, Integer>> selectedAnalyses = getSelectedAnalysis();
                if (JOptionPane.showInternalConfirmDialog(
                    BatchAnalysisFrame.this, 
                    "Remove " + selectedAnalyses.size() + " selected analy" + (selectedAnalyses.size() == 1 ? "sis" : "ses") + "?", "Remove?", 
                    JOptionPane.OK_CANCEL_OPTION
                ) == JOptionPane.OK_OPTION) {
                    Collections.sort(selectedAnalyses, Comparator.comparing(p -> -p.getRight()));
                    for (ImmutablePair<BatchAnalysis, Integer> selectedAnalysis : selectedAnalyses) {
                        ParameterSettingsFrame settings_frame = _open_settings_frames.get(selectedAnalysis.left);
                        if (settings_frame != null) {                
                            // There is an open settings window for it - bring to focus verses removing.
                            settings_frame.toFront();
                            return;
                        } else {
                            model.removeRow(selectedAnalysis.right);
                            _batch_analyses.remove(selectedAnalysis.left);
                        }
                    }
                    writeFile();
                    enableButtons();
                }
            }
        });

        _modify_analysis.setText("Modify Selected");
        _modify_analysis.setEnabled(false);
        _modify_analysis.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                WaitCursor waitCursor = new WaitCursor(BatchAnalysisFrame.this);
                try {
                    ImmutablePair<BatchAnalysis, Integer> selected = getSelectedAnalysis().get(0);
                    ParameterSettingsFrame settings_frame = _open_settings_frames.get(selected.left);
                    if (settings_frame != null) {
                        settings_frame.toFront();
                        return;
                    }
                    ParameterSettingsFrame parameters_frame = new ParameterSettingsFrame(SaTScanApplication.getInstance().getRootPane(), BatchAnalysisFrame.this, selected.left.getParameters());
                    parameters_frame.setIconifiable(false);
                    parameters_frame.addInternalFrameListener(BatchAnalysisFrame.this);
                    parameters_frame.setVisible(true);
                    SaTScanApplication.getInstance().AddFrame(parameters_frame);
                    parameters_frame.addInternalFrameListener(SaTScanApplication.getInstance());
                    _open_settings_frames.put(selected.left, parameters_frame);
                    parameters_frame.setSelected(true);
                    enableButtons();
                } catch (Throwable t) {
                    new ExceptionDialog(SaTScanApplication.getInstance(), t).setVisible(true);
                } finally {
                    waitCursor.restore();
                }
            }
        });

        _execute_selected.setText("Execute Selected");
        _execute_selected.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                if (_email_settings._send_summary_email) {
                    try {
                        if (!ApplicationPreferences.minimumMailServerDefined()) {
                            JOptionPane.showInternalMessageDialog(BatchAnalysisFrame.this,
                                """
                                In order to email a summary email alerts, you must define mail server settings.
                                Please see 'Mail Server Settings' in the 'Preferences and Settings' dialog."""
                            );
                            return;
                        }
                        if (_email_settings._summary_email_recipients.trim().isEmpty()) {
                            JOptionPane.showInternalMessageDialog(BatchAnalysisFrame.this,
                                "At least one email address is required to receive summary email alerts."
                            );
                            return;
                        }
                    } catch (Throwable t) {
                        new ExceptionDialog(SaTScanApplication.getInstance(), t).setVisible(true);
                        return;
                    }
                }
                _execute_progress.setValue(0);
                _execute_progress.setString("");
                _execute_progress.setVisible(true);
                ExecuteAnalysisTask task = new ExecuteAnalysisTask();
                task.addPropertyChangeListener(new PropertyChangeListener() {
                    public  void propertyChange(PropertyChangeEvent evt) {
                        if ("progress".equals(evt.getPropertyName())) {
                            _execute_progress.setValue((Integer)evt.getNewValue());
                        }
                    }
                });
                task.execute();
            }
        });

        _duplicate_analysis.setText("Create Duplicate");
        _duplicate_analysis.setEnabled(false);
        _duplicate_analysis.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                WaitCursor waitCursor = new WaitCursor(BatchAnalysisFrame.this);
                try {
                    ImmutablePair<BatchAnalysis, Integer> selected = getSelectedAnalysis().get(0);
                    BatchAnalysis duplicated = (BatchAnalysis)selected.left.clone();
                    duplicated.setDescription("Copy - " + duplicated.getDescription());
                    duplicated.setLastExecutedDate(null);
                    duplicated.setLastExecutedStatus(BatchAnalysis.STATUS.NEVER);
                    duplicated.setDrilldownRoot(null);
                    duplicated.setSelected(true);
                    _batch_analyses.add(selected.right + 1, duplicated);
                    // Add record to table.
                    DefaultTableModel model = (DefaultTableModel) _analyses_table.getModel();
                    model.insertRow(selected.right + 1, getRowForBatchAnalysis(duplicated));
                    // Remove selection from any other records in table - to focus on new record.
                    removeSelection(duplicated);
                    // Show settngs window for new reocrd.
                    ParameterSettingsFrame frame = new ParameterSettingsFrame(SaTScanApplication.getInstance().getRootPane(), BatchAnalysisFrame.this, duplicated.getParameters());
                    frame.setTitle(duplicated.getDescription());
                    frame.setIconifiable(false);
                    frame.addInternalFrameListener(BatchAnalysisFrame.this);
                    frame.setVisible(true);
                    SaTScanApplication.getInstance().AddFrame(frame);
                    frame.addInternalFrameListener(SaTScanApplication.getInstance());
                    _open_settings_frames.put(duplicated, frame);
                    frame.setSelected(true);
                    _duplicate_analysis.requestFocusInWindow();
                } catch (Throwable t) {
                    new ExceptionDialog(SaTScanApplication.getInstance(), t).setVisible(true);
                } finally {
                    waitCursor.restore();
                }
            }
        });

        _execute_progress.setStringPainted(true);
        _execute_progress.setVisible(false);

        _moveUp.setIcon(new javax.swing.ImageIcon(getClass().getResource("/arrow-up.png"))); // NOI18N
        _moveUp.setMaximumSize(new java.awt.Dimension(49, 23));
        _moveUp.setMinimumSize(new java.awt.Dimension(49, 23));
        _moveUp.setPreferredSize(new java.awt.Dimension(49, 23));
        _moveUp.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                _moveUpActionPerformed(evt);
            }
        });

        _moveDown.setIcon(new javax.swing.ImageIcon(getClass().getResource("/arrow-down.png"))); // NOI18N
        _moveDown.setMaximumSize(new java.awt.Dimension(49, 23));
        _moveDown.setMinimumSize(new java.awt.Dimension(49, 23));
        _moveDown.setPreferredSize(new java.awt.Dimension(49, 23));
        _moveDown.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                _moveDownActionPerformed(evt);
            }
        });

        _summary_email.setText("Email Alerts");
        _summary_email.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                try {
                    SummaryEmailSettingsDialog settings = new SummaryEmailSettingsDialog(SaTScanApplication.getInstance(), _email_settings);
                    settings.setLocationRelativeTo(SaTScanApplication.getInstance());
                    settings.setVisible(true);
                    if (settings._updated) writeFile();
                } catch (Throwable t) {
                    new ExceptionDialog(SaTScanApplication.getInstance(), t).setVisible(true);
                }
            }
        });

        javax.swing.GroupLayout _actions_panelLayout = new javax.swing.GroupLayout(_actions_panel);
        _actions_panel.setLayout(_actions_panelLayout);
        _actions_panelLayout.setHorizontalGroup(
            _actions_panelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_actions_panelLayout.createSequentialGroup()
                .addContainerGap()
                .addGroup(_actions_panelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(_modify_analysis, javax.swing.GroupLayout.Alignment.TRAILING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(_add_analysis, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(_remove_analysis, javax.swing.GroupLayout.Alignment.TRAILING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(_execute_selected, javax.swing.GroupLayout.Alignment.TRAILING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(_duplicate_analysis, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(jSeparator2)
                    .addComponent(_execute_progress, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(jSeparator3)
                    .addGroup(_actions_panelLayout.createSequentialGroup()
                        .addComponent(_moveUp, javax.swing.GroupLayout.PREFERRED_SIZE, 68, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                        .addComponent(_moveDown, javax.swing.GroupLayout.PREFERRED_SIZE, 68, javax.swing.GroupLayout.PREFERRED_SIZE))
                    .addComponent(_summary_email, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                .addContainerGap())
        );
        _actions_panelLayout.setVerticalGroup(
            _actions_panelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_actions_panelLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(_add_analysis)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_modify_analysis)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_duplicate_analysis)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(jSeparator3, javax.swing.GroupLayout.PREFERRED_SIZE, 5, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(_actions_panelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(_moveUp, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(_moveDown, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_remove_analysis)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(jSeparator2, javax.swing.GroupLayout.PREFERRED_SIZE, 5, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_summary_email)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED, 94, Short.MAX_VALUE)
                .addComponent(_execute_progress, javax.swing.GroupLayout.PREFERRED_SIZE, 23, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_execute_selected)
                .addContainerGap())
        );

        _analyses_table.setModel(new javax.swing.table.DefaultTableModel(
            new Object [][] {},
            new String [] { "", "Description", "Analysis", "Model", "Study Length", "Lag", "Status" }){
            Class[] types = new Class [] {
                java.lang.Boolean.class, String.class, String.class, String.class, BatchAnalysis.StudyPeriodOffset.class, BatchAnalysis.StudyPeriodOffset.class, BatchAnalysis.class
            };
            boolean[] canEdit = new boolean [] { true, true, false, false, true, true, false };
            public Class getColumnClass(int columnIndex) {
                return types [columnIndex];
            }
            public boolean isCellEditable(int rowIndex, int columnIndex) {
                return !_batch_executing && canEdit[columnIndex];
            }
        });
        _analyses_table.putClientProperty("terminateEditOnFocusLost", Boolean.TRUE);
        _analyses_table.getTableHeader().setOpaque(false);
        // Set custom header renderer for the row selection header.
        TableColumn column = _analyses_table.getColumnModel().getColumn(SELECT_IDX);
        column.setHeaderRenderer(new CheckBoxHeaderRenderer());
        column.setHeaderValue(Boolean.FALSE);
        // Add listeners for user selection on table header and rows.
        CheckBoxHeaderHandler handler = new CheckBoxHeaderHandler(_analyses_table, SELECT_IDX);
        _analyses_table.getModel().addTableModelListener(handler);
        _analyses_table.getTableHeader().addMouseListener(handler);

        _analyses_table.setRowHeight(35);
        _analyses_table.setIntercellSpacing(new Dimension(10,1));
        _analyses_table.setDefaultRenderer(Parameters.class, new ParametersRenderer());
        _analyses_table.setDefaultRenderer(BatchAnalysis.StudyPeriodOffset.class, new StudyPeriodOffsetRenderer());
        _analyses_table.setDefaultRenderer(BatchAnalysis.class, new BatchStatusRenderer());
        _analyses_table.setDefaultEditor(BatchAnalysis.StudyPeriodOffset.class, new StudyPeriodOffsetEditor());
        _analyses_table.setDefaultEditor(String.class, new StringEditor());
        ((DefaultCellEditor)_analyses_table.getDefaultEditor(BatchAnalysis.StudyPeriodOffset.class)).setClickCountToStart(1);
        ((DefaultCellEditor)_analyses_table.getDefaultEditor(String.class)).setClickCountToStart(1);
        _analyses_table.setRowSelectionAllowed(false);
        _analyses_table.getTableHeader().setReorderingAllowed(false);
        setJTableColumnsWidth(_analyses_table, _analyses_table.getPreferredSize().width, 1, 30, 16, 8, 10, 6, 29);
        _analyses_table.getModel().addTableModelListener(new TableModelListener() {
            @Override
            public void tableChanged(TableModelEvent e) {
                enableButtons();
                if (_table_initiallized) {
                    int row = e.getFirstRow();
                    int column = e.getColumn();
                    if (column == SELECT_IDX) {
                        DefaultTableModel model = (DefaultTableModel)e.getSource();
                        boolean curr_value = _batch_analyses.get(row).getSelected();
                        _batch_analyses.get(row).setSelected((boolean)model.getValueAt(row, column));
                        if (curr_value != _batch_analyses.get(row).getSelected())
                        writeFile();
                    } else if (column == DESCRIPTION_IDX) {
                        DefaultTableModel model = (DefaultTableModel)e.getSource();
                        String curr_value = _batch_analyses.get(row).getDescription();
                        _batch_analyses.get(row).setDescription((String)model.getValueAt(row, column));
                        if (!curr_value.equals(_batch_analyses.get(row).getDescription()))
                        writeFile();
                    }
                }
            }
        });
        _analysesScrollpane.setViewportView(_analyses_table);

        javax.swing.GroupLayout _actionitems_panelLayout = new javax.swing.GroupLayout(_actionitems_panel);
        _actionitems_panel.setLayout(_actionitems_panelLayout);
        _actionitems_panelLayout.setHorizontalGroup(
            _actionitems_panelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_actionitems_panelLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(_actions_panel, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_analysesScrollpane, javax.swing.GroupLayout.DEFAULT_SIZE, 787, Short.MAX_VALUE)
                .addContainerGap())
        );
        _actionitems_panelLayout.setVerticalGroup(
            _actionitems_panelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, _actionitems_panelLayout.createSequentialGroup()
                .addContainerGap()
                .addGroup(_actionitems_panelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING)
                    .addComponent(_analysesScrollpane, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, 348, Short.MAX_VALUE)
                    .addComponent(_actions_panel, javax.swing.GroupLayout.Alignment.LEADING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                .addContainerGap())
        );

        javax.swing.GroupLayout layout = new javax.swing.GroupLayout(getContentPane());
        getContentPane().setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addComponent(_actionitems_panel, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addComponent(_actionitems_panel, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
        );

        pack();
    }// </editor-fold>//GEN-END:initComponents

    private void _moveUpActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event__moveUpActionPerformed
        ImmutablePair<BatchAnalysis, Integer> selected = getSelectedAnalysis().get(0);
        if(selected.right > 0){
            // move rows from the index to index into the position index -1
            DefaultTableModel model = (DefaultTableModel)_analyses_table.getModel();
            model.moveRow(selected.right, selected.right, selected.right - 1);
            // set selection to the new position
            _analyses_table.setRowSelectionInterval(selected.right - 1, selected.right - 1);            
            Collections.swap(_batch_analyses, selected.right, selected.right - 1);
            writeFile();
            _moveUp.setEnabled(selected.right - 1 > 0);
            if (_moveUp.isEnabled()) _moveUp.requestFocusInWindow();
        }
    }//GEN-LAST:event__moveUpActionPerformed

    private void _moveDownActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event__moveDownActionPerformed
        ImmutablePair<BatchAnalysis, Integer> selected = getSelectedAnalysis().get(0);
        if(selected.right < _analyses_table.getModel().getRowCount() - 1){
            // move rows from the index to index into the position index -1
            DefaultTableModel model = (DefaultTableModel)_analyses_table.getModel();
            model.moveRow(selected.right, selected.right, selected.right + 1);
            // set selection to the new position
            _analyses_table.setRowSelectionInterval(selected.right + 1, selected.right + 1);
            Collections.swap(_batch_analyses, selected.right, selected.right + 1);
            writeFile();
            _moveDown.setEnabled(selected.right + 1 < _analyses_table.getModel().getRowCount() - 1);
            if (_moveDown.isEnabled()) _moveDown.requestFocusInWindow();
        }
    }//GEN-LAST:event__moveDownActionPerformed

    @Override
    public void internalFrameOpened(InternalFrameEvent e) {
        //throw new UnsupportedOperationException("Not supported yet."); //To change body of generated methods, choose Tools | Templates.
    }

    @Override public void internalFrameClosing(InternalFrameEvent e) {
        JInternalFrame this_frame = e.getInternalFrame();
        if (this_frame instanceof BatchAnalysisFrame && queryCanClose()) {
            this.setVisible(false); 
        }
    }

    @Override public void internalFrameClosed(InternalFrameEvent e) {
        JInternalFrame this_frame = e.getInternalFrame();
        if (this_frame instanceof ParameterSettingsFrame) {
            ParameterSettingsFrame settings_frame = (ParameterSettingsFrame)this_frame;
            // Search for this frame in collection of open windows.
            for (Map.Entry<BatchAnalysis, ParameterSettingsFrame> entry : _open_settings_frames.entrySet()) {
                BatchAnalysis batchAnaylsis = entry.getKey();
                if (settings_frame == entry.getValue()) {
                    _open_settings_frames.remove(batchAnaylsis);
                    // Re-assign parameters class associated with this nalysis panel.
                    batchAnaylsis.setParameters(settings_frame.getParameterSettings());
                    /* Now write the all parameter settings back to file -- maybe this only needs to be done on batch frame close or run? */
                    writeFile();
                    // Fresh table cell to reflect any updates.
                    int idx = _batch_analyses.indexOf(batchAnaylsis);
                    _analyses_table.getModel().setValueAt(batchAnaylsis.getParameters().GetAnalysisTypeAsString(true), idx, ANALYSIS_IDX);
                    _analyses_table.getModel().setValueAt(
                        batchAnaylsis.getParameters().GetProbabilityModelTypeAsString(batchAnaylsis.getParameters().GetProbabilityModelType(), true), idx, MODEL_IDX
                    );
                    break;
                }
            }
        }
    }

    @Override public void internalFrameIconified(InternalFrameEvent e) {
        //throw new UnsupportedOperationException("Not supported yet."); //To change body of generated methods, choose Tools | Templates.
    }

    @Override public void internalFrameDeiconified(InternalFrameEvent e) {
        //throw new UnsupportedOperationException("Not supported yet."); //To change body of generated methods, choose Tools | Templates.
    }

    @Override public void internalFrameActivated(InternalFrameEvent e) {
        //throw new UnsupportedOperationException("Not supported yet."); //To change body of generated methods, choose Tools | Templates.
    }

    @Override public void internalFrameDeactivated(InternalFrameEvent e) {
        //throw new UnsupportedOperationException("Not supported yet."); //To change body of generated methods, choose Tools | Templates.
    }

    // Icon class implementation that ...
    class ComponentIcon implements Icon {
        private final Component cmp;

        protected ComponentIcon(Component cmp) {
            this.cmp = cmp;
        }

        @Override public void paintIcon(Component c, Graphics g, int x, int y) {
            SwingUtilities.paintComponent(g, cmp, c.getParent(), x + 3, y, getIconWidth(), getIconHeight());
        }

        @Override public int getIconWidth() {
            return cmp.getPreferredSize().width;
        }

        @Override public int getIconHeight() {
            return cmp.getPreferredSize().height;
        }
    }    
    
    /* Header renderer for column which is the row selection checkbox. */
    class CheckBoxHeaderRenderer implements TableCellRenderer {
        private final JCheckBox check = new JCheckBox("");
        private final JLabel label = new JLabel("", JLabel.CENTER);

        @Override public Component getTableCellRendererComponent(JTable table, Object value, boolean isSelected, boolean hasFocus, int row, int column) {
            Boolean val = (Boolean) value;
            if (val == null) {
                check.setSelected(true);
                check.setEnabled(false);            
            } else if (!val) {
                check.setSelected(false);
                check.setEnabled(true);            
            } else {
                check.setSelected(true);
                check.setEnabled(true);            
            }
            check.setOpaque(false);
            check.setFont(table.getFont());
            TableCellRenderer r = table.getTableHeader().getDefaultRenderer();
            JLabel l = (JLabel) r.getTableCellRendererComponent(table, value, isSelected, hasFocus, row, column);
        
            label.setIcon(new ComponentIcon(check));
            l.setIcon(new ComponentIcon(label));
            l.setText(null); // XXX: Nimbus???
            return l;
        }
    }    

    /* Header listener class for table model actions and mouse actions. */
    class CheckBoxHeaderHandler extends MouseAdapter implements TableModelListener {
        private final JTable table;
        private final int targetColumnIndex;

        protected CheckBoxHeaderHandler(JTable table, int index) {
            super();
            this.table = table;
            this.targetColumnIndex = index;
        }

        @Override public void tableChanged(TableModelEvent e) {
            if (e.getType() == TableModelEvent.UPDATE && e.getColumn() == targetColumnIndex) {
                int vci = table.convertColumnIndexToView(targetColumnIndex);
                TableColumn column = table.getColumnModel().getColumn(vci);
                Object status = column.getHeaderValue();
                TableModel m = table.getModel();
                if (m instanceof DefaultTableModel && fireUpdateEvent((DefaultTableModel) m, column, status)) {
                    JTableHeader h = table.getTableHeader();
                    h.repaint(h.getHeaderRect(vci));
                }
            }
        }

        private boolean fireUpdateEvent(DefaultTableModel m, TableColumn column, Object status) {
            if (status == null) {
                List<Boolean> l = ((java.util.Vector<?>) m.getDataVector()).stream()
                .map(v -> (Boolean) ((java.util.Vector<?>) v).get(targetColumnIndex)).distinct().collect(Collectors.toList());
                boolean isOnlyOneSelected = l.size() == 1;
                if (isOnlyOneSelected) {
                    column.setHeaderValue(l.get(0) ? Boolean.TRUE : Boolean.FALSE);
                    return true;
                } else {
                    return false;
                }
            } else {
                column.setHeaderValue(null);
                return true;
            }
        }

        @Override public void mouseClicked(MouseEvent e) {
            if (_batch_executing) return;
            JTableHeader header = (JTableHeader) e.getComponent();
            JTable tbl = header.getTable();
            TableColumnModel columnModel = tbl.getColumnModel();
            TableModel m = tbl.getModel();
            int vci = columnModel.getColumnIndexAtX(e.getX());
            int mci = tbl.convertColumnIndexToModel(vci);
            if (mci == targetColumnIndex && m.getRowCount() > 0) {
                TableColumn column = columnModel.getColumn(vci);
                Object v = column.getHeaderValue();
                boolean b = (Boolean)v == Boolean.FALSE;
                for (int i = 0; i < m.getRowCount(); i++) {
                    m.setValueAt(b, i, mci);
                }
                column.setHeaderValue(b ? Boolean.TRUE : Boolean.FALSE);
            }
        }
    }    
    
    class ParametersRenderer extends DefaultTableCellRenderer {
        @Override public Component getTableCellRendererComponent(JTable table, Object value, boolean isSelected, boolean hasFocus, int row, int column) {
            super.getTableCellRendererComponent(table, value, isSelected, false, row, column);
            Parameters parameters = (Parameters)value;
            setText(BatchAnalysis.getDescription(parameters));
            return this;
        }
    }    
    
    /* TableCellRenderer specific to StudyPeriodOffset class -- displays durection and units together. */
    class StudyPeriodOffsetRenderer extends DefaultTableCellRenderer {
        @Override public Component getTableCellRendererComponent(JTable table, Object value, boolean isSelected, boolean hasFocus, int row, int column) {
            super.getTableCellRendererComponent(table, value, isSelected, false, row, column);
            setText(getOffsetString((BatchAnalysis.StudyPeriodOffset)value, column == LAG_IDX ? DEFAULT_LAG : DEFAULT_STUDY_PERIOD));
            return this;
        }
    }    
    
    /* TableCellRenderer specific to BatchAnalysis class -- displays analysis status. */
    class BatchStatusRenderer extends DefaultTableCellRenderer {
        
        public BatchStatusRenderer() {
            super();
            // Add mouse motion listener - to change cursor on cell hover.
            _analyses_table.addMouseMotionListener(new java.awt.event.MouseMotionListener() {
                @Override public void mouseMoved(java.awt.event.MouseEvent evt) {
                    if (_analyses_table.columnAtPoint(evt.getPoint()) == STATUS_IDX){
                        int rowIdx = _analyses_table.rowAtPoint(evt.getPoint());
                        if (rowIdx >= 0 && _batch_analyses.get(rowIdx).getLastExecutedStatus() != BatchAnalysis.STATUS.NEVER) {
                            _analyses_table.setCursor(Cursor.getPredefinedCursor(Cursor.HAND_CURSOR)); 
                            return;
                        }
                    }
                    _analyses_table.setCursor(Cursor.getPredefinedCursor(Cursor.DEFAULT_CURSOR));                    
                }
                @Override public void mouseDragged(MouseEvent e) {}
            });
            // Add mouse listener - to launcher results viewer
            _analyses_table.addMouseListener(new java.awt.event.MouseAdapter() {        
                @Override public void mouseEntered(java.awt.event.MouseEvent evt) {}
                @Override public void mouseExited(java.awt.event.MouseEvent evt) {}
                @Override public void mousePressed(java.awt.event.MouseEvent evt) {
                    int row = _analyses_table.rowAtPoint(evt.getPoint());
                    int col = _analyses_table.columnAtPoint(evt.getPoint());
                    if (row >= 0 && col == STATUS_IDX) { 
                        BatchAnalysis ba = _batch_analyses.get(row);
                        if (ba.getLastExecutedStatus() == BatchAnalysis.STATUS.SUCCESS || ba.getLastExecutedStatus() == BatchAnalysis.STATUS.FAILED) {
                            AnalysisRunInternalFrame frame = new AnalysisRunInternalFrame(ba);
                            frame.disableAdditionalOutputAutoLaunch(true);
                            // Add application as listener, set visiable and add frame to application document.
                            frame.addInternalFrameListener(SaTScanApplication.getInstance());
                            frame.setVisible(true);
                            SaTScanApplication.getInstance().AddFrame(frame);
                            try {
                                frame.setSelected(true);
                            } catch (java.beans.PropertyVetoException x) {}
                        }
                    }                
                }                                    
            });               
        }
        
        @Override public Component getTableCellRendererComponent(JTable table, Object value, boolean isSelected, boolean hasFocus, int row, int column) {
            super.getTableCellRendererComponent(table, value, isSelected, false, row, column);
            if (_batch_analyses.get(row).getLastExecutedStatus() != BatchAnalysis.STATUS.NEVER)
                setText("<html><a href=\"\">" + getStatusString(_batch_analyses.get(row)) + "</a></html>");
            else 
                setText(getStatusString(_batch_analyses.get(row)));
            return this;
        }
    } 
    
    /* TableCellEditor specific to String class -- prevents empty/blank  string. */
    public class StringEditor extends DefaultCellEditor implements TableCellEditor {

        public StringEditor() {
            super(new JTextField());
        }
        
        @Override public boolean stopCellEditing() {
            if (((JTextField)editorComponent).getText().isBlank())
                return false;
            fireEditingStopped();
            return true;
        }        
    }    
    
    
    /* TableCellEditor specific to StudyPeriodOffset class -- allowing user to set both durection and units at same time. */
    public class StudyPeriodOffsetEditor extends DefaultCellEditor implements TableCellEditor {
        private final StudyPeriodOffsetDialog _editor_dialog = new StudyPeriodOffsetDialog(SaTScanApplication.getInstance());
        private BatchAnalysis.StudyPeriodOffset currentOffset = null;
        private final JButton editorComponent;

        public StudyPeriodOffsetEditor() {
            super(new JTextField());
            editorComponent = new JButton();
            editorComponent.setBorderPainted(false);
            editorComponent.setContentAreaFilled(false);
            editorComponent.setFocusable(false); // Make sure focus goes back to the table when the dialog is closed.
        }

        //Implement the one CellEditor method that AbstractCellEditor doesn't.
        @Override
        public Object getCellEditorValue() {
            return currentOffset;
        }

        @Override
        public Component getTableCellEditorComponent(JTable table, Object value, boolean isSelected, int row, int column) {
            currentOffset = (BatchAnalysis.StudyPeriodOffset)value;
            SwingUtilities.invokeLater(() -> {
                _editor_dialog.setStudyPeriodOffset(
                    (column == LAG_IDX ? "Edit Lag Time" : "Edit Study Length"), 
                    (column == LAG_IDX ? "No Lag" : "No Offset"), 
                    (column == LAG_IDX ? _lag_helptext : _study_period_helptext), 
                    currentOffset,
                    (column == LAG_IDX ? 0 : 1)
                );
                Point p = editorComponent.getLocationOnScreen();
                _editor_dialog.setLocation(p.x, p.y + editorComponent.getSize().height);
                _editor_dialog.setVisible(true);
                if (_editor_dialog.updatedOffset()) {
                    BatchAnalysis ba = _batch_analyses.get(row);
                    if (column == STUDYPERIOD_IDX)
                        ba.setStudyPeriodLength(_editor_dialog.getStudyPeriodOffset());
                    else if (column == LAG_IDX)
                        ba.setLag(_editor_dialog.getStudyPeriodOffset());
                    currentOffset = _editor_dialog.getStudyPeriodOffset();
                    writeFile();
                }
                fireEditingStopped();
            });
            editorComponent.setText(getOffsetString(currentOffset, column == LAG_IDX ? DEFAULT_LAG : DEFAULT_STUDY_PERIOD));
            return editorComponent;
        }
    }

    private native int SendSummaryEmail();
    
    /* Background task runner which executes selected analyese. */
    class ExecuteAnalysisTask extends SwingWorker<Void, BatchAnalysis> {
        private final WaitCursor waitCursor = new WaitCursor(BatchAnalysisFrame.this);
        public static final String DATE_FORMAT = "yyyy/MM/dd";
        private static final int SLEEP_MAXIMUM = 100;
        private double _num_records;
        private double _num_executed = 0;
        // Store execution start date to ensure that all analyses use todays's date.
        private final LocalDate _start_date = LocalDate.now();

        private LocalDate getLocalDate(LocalDate initial, BatchAnalysis.StudyPeriodOffset offset, boolean isStudyPeriod) {
            if (offset == null)
                return initial;
            switch (offset.getUnits()) {
                case YEAR -> {
                    return initial.minusYears(offset.getOffset());
                }
                case MONTH -> { 
                    return initial.minusMonths(offset.getOffset());
                }
                case DAY, GENERIC -> {
                    // special case with study period - since end date is included
                    return initial.minusDays(offset.getOffset() - (isStudyPeriod ? 1 : 0));
                }
            }
            return initial;
        }
        
        private void executeAnalysis(BatchAnalysis analysis) {
            AnalysisRunInternalFrame frame = null;
            try {
                    // Create a new analysis runner frame.
                    // Clone the paramters so we can modifiy the study period dates.
                    Parameters run_parameters = (Parameters)analysis.getParameters().clone();
                    run_parameters.setCreateEmailSummaryFile(_email_settings._send_summary_email);
                    if (run_parameters.GetIsProspectiveAnalysis())
                        run_parameters.setEmailSummaryValue(_email_settings._summary_email_recurrence_cutoff.doubleValue());
                    else
                        run_parameters.setEmailSummaryValue(_email_settings._summary_email_pvalue_cutoff.doubleValue());                        
                    // set study period based on study period length and lag settings.
                    LocalDate enddate = getLocalDate(_start_date, analysis.getLag(), false);
                    if (analysis.getLag() != null)
                        run_parameters.SetStudyPeriodEndDate(enddate.format(DateTimeFormatter.ofPattern(DATE_FORMAT)));
                    else // otherwise define enddate per paramemter settings - for possible study period offset
                        enddate = LocalDate.parse(run_parameters.GetStudyPeriodEndDate(), DateTimeFormatter.ofPattern("yyyy/M/d"));
                    // base the study period start as an offset from the lag period.
                    LocalDate startdate = getLocalDate(enddate, analysis.getStudyPeriodLength(), true);
                    if (analysis.getStudyPeriodLength() != null)
                        run_parameters.SetStudyPeriodStartDate(startdate.format(DateTimeFormatter.ofPattern(DATE_FORMAT)));
                    
                    frame = new AnalysisRunInternalFrame(run_parameters);
                    frame.disableAdditionalOutputAutoLaunch(true);
                    frame.setTitle("Running " + analysis.getDescription());
                    // Register this analysis as executing -- in case user attempts to close application.
                    OutputFileRegister.getInstance().register(run_parameters.GetOutputFileName());
                    // Add application as listener, set visiable and add frame to application document.
                    frame.addInternalFrameListener(SaTScanApplication.getInstance());
                    frame.setVisible(true);
                    SaTScanApplication.getInstance().AddFrame(frame);
                    frame.setSelected(true);
                    CalculationThread thread = frame.getCalculationThread();
                    thread.start(); // Start thread executing.
                    thread.join(); // Wait on thread to finish execution.
                    int sleep_iterations = 0;
                    while (frame.messageWorkersPending() && sleep_iterations <= SLEEP_MAXIMUM){
                        Thread.sleep(200);
                        ++sleep_iterations;
                    }
                    // Update attributes of BatchAnalysis to reflect this execution.
                    if (frame.isSuccessful()) {
                        analysis.setLastExecutedStatus(BatchAnalysis.STATUS.SUCCESS);
                        analysis.setLastResultsFilename(run_parameters.GetOutputFileName());
                    } else if (frame.userCancelled()) {
                        analysis.setLastExecutedStatus(BatchAnalysis.STATUS.CANCELLED);
                        // Assume that if user cancelled current analysis, they wish to abort any further analyses.
                        cancel(true);
                    } else if (frame.exceptionOccurred())
                        analysis.setLastExecutedStatus(BatchAnalysis.STATUS.FAILED);
                    else 
                        Logger.getLogger(ExecuteAnalysisTask.class.getName()).log(Level.SEVERE, "Unknown execution status.");
                    analysis.setLastExecutedDate(new Date());
                    analysis.setLastExecutedMessage(frame.getWarningsErrorsText());
                    frame.setDrilldownRoot(analysis);
                    frame.setClosed(true); // Close analysis run frame.
            } catch (java.lang.InterruptedException ie) {
                Logger.getLogger(ExecuteAnalysisTask.class.getName()).log(Level.SEVERE, null, ie);
            } catch (PropertyVetoException e) {
                Logger.getLogger(ExecuteAnalysisTask.class.getName()).log(Level.SEVERE, "Execution failed with error: {0}", e.toString());
                JOptionPane.showInternalMessageDialog(BatchAnalysisFrame.this, e.getMessage());
                //throw new RuntimeException(e.getMessage(), e);
            } finally {
                if (frame != null) try {
                    frame.setClosed(true);
                } catch (PropertyVetoException ex) {
                    Logger.getLogger(ExecuteAnalysisTask.class.getName()).log(Level.SEVERE, null, ex);
                }
            }
        }

        @Override
        protected Void doInBackground() throws Exception {
            try {
                _batch_executing = true;
                disableButtons();
                ArrayList<ImmutablePair<BatchAnalysis, Integer>> selectedAnalyses = getSelectedAnalysis();
                _num_records = selectedAnalyses.size();
                DefaultTableModel model = (DefaultTableModel) _analyses_table.getModel();
                for (ImmutablePair<BatchAnalysis, Integer> selected : selectedAnalyses) { 
                    // Test for user cancellation at the beginning of each analyses.
                    if (isCancelled())
                        break;
                    executeAnalysis(selected.left);
                    // Update Status column in table to reflect execution results.
                    model.setValueAt(selected.left, selected.right, STATUS_IDX);
                    //publish(selected.left);
                    ++_num_executed;
                    setProgress((int)(100.0 * (_num_executed / _num_records)));
                    _execute_progress.setString((int)_num_executed + " of " + (int)_num_records);
                }
                setProgress(100);
                // Write results to xml file -- recording analysis status and execution time.
                writeFile();
                // send summary email, if requested
                if (_email_settings._send_summary_email) {
                    if (BatchAnalysisFrame.this.SendSummaryEmail() != 0)
                        JOptionPane.showInternalMessageDialog(
                            BatchAnalysisFrame.this, "The summary email could not be sent."
                        );
                }
            } catch (Exception e) {
                Logger.getLogger(ExecuteAnalysisTask.class.getName()).log(Level.SEVERE, null, e);
                throw new RuntimeException(e.getMessage(), e);
            }
            return null;
        }

        @Override
        public void done() {
            waitCursor.restore();
            _execute_progress.setVisible(false);
            _batch_executing = false;
            enableButtons();
        }
    }
    
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JPanel _actionitems_panel;
    private javax.swing.JPanel _actions_panel;
    private javax.swing.JButton _add_analysis;
    private javax.swing.JScrollPane _analysesScrollpane;
    private javax.swing.JTable _analyses_table;
    private javax.swing.JButton _duplicate_analysis;
    private javax.swing.JProgressBar _execute_progress;
    private javax.swing.JButton _execute_selected;
    private javax.swing.JButton _modify_analysis;
    private javax.swing.JButton _moveDown;
    private javax.swing.JButton _moveUp;
    private javax.swing.JButton _remove_analysis;
    private javax.swing.JButton _summary_email;
    private javax.swing.JSeparator jSeparator2;
    private javax.swing.JSeparator jSeparator3;
    // End of variables declaration//GEN-END:variables
}
