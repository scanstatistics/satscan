package org.satscan.gui;

import java.awt.Color;
import java.awt.Component;
import java.awt.Cursor;
import java.awt.Desktop;
import java.awt.Dimension;
import java.awt.event.MouseEvent;
import java.awt.event.MouseMotionListener;
import java.beans.PropertyVetoException;
import java.io.BufferedReader;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.nio.charset.Charset;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Comparator;
import java.util.Enumeration;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import java.util.zip.ZipEntry;
import java.util.zip.ZipInputStream;
import javax.swing.ImageIcon;
import javax.swing.JEditorPane;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JTree;
import javax.swing.SwingUtilities;
import javax.swing.event.InternalFrameEvent;
import javax.swing.event.InternalFrameListener;
import javax.swing.event.TreeSelectionEvent;
import javax.swing.event.TreeSelectionListener;
import javax.swing.text.BadLocationException;
import javax.swing.text.Element;
import javax.swing.tree.DefaultMutableTreeNode;
import javax.swing.tree.DefaultTreeCellRenderer;
import javax.swing.tree.DefaultTreeModel;
import javax.swing.tree.MutableTreeNode;
import javax.swing.tree.TreeNode;
import javax.swing.tree.TreePath;
import javax.swing.tree.TreeSelectionModel;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;
import org.apache.commons.lang3.tuple.Pair;
import org.satscan.app.AppConstants;
import org.satscan.app.CalculationThread;
import org.satscan.utils.EmailClientLauncher;
import org.satscan.app.Parameters;
import org.satscan.gui.utils.JDocumentRenderer;
import org.satscan.app.OutputFileRegister;
import org.satscan.utils.BareBonesBrowserLaunch;
import org.w3c.dom.NodeList;
import org.xml.sax.SAXException;

/**
 * Analysis execution progress/cancellation and results window.
 * @author  Hostovic
 */
public class AnalysisRunInternalFrame extends javax.swing.JInternalFrame implements InternalFrameListener {

    private boolean gbCancel = false;
    private boolean gbCanClose = false;
    private boolean gbCanPrint;
    private final Parameters _parameters;
    private String gsProgramErrorCallPath = "";
    private final int MAXLINES = 999;
    private boolean _warning_errors_encountered = false;
    Map<DefaultMutableTreeNode, String> _tree_output_map = new HashMap<>();
    Map<String, DefaultMutableTreeNode> _output_tree_map = new HashMap<>();

    /**
     * Creates new form ParameterSettingsFrame
     */
    public AnalysisRunInternalFrame(final Parameters parameters) {
        initComponents();
        /* Remove the drilldown tab initially - we'll add back if analysis completes any drilldowns. */
        _bottom_tabbed_pane.remove(_drilldown_results_tab);
        super.setFrameIcon(new ImageIcon(getClass().getResource("/SaTScan.png")));
        super.addInternalFrameListener(this);
        _parameters = (Parameters)parameters.clone();
        super.setTitle("Running " + (_parameters.GetSourceFileName().equals("") ? "Session" : _parameters.GetSourceFileName()));
        new CalculationThread(this, _parameters).start();
    }

    /* Add drilldown output in relation to outfile file from calling analysis. */
    public void ReportDrilldownResults(String drilldown_resultfile, String parent_resultfile) {
        DefaultMutableTreeNode parent, child;
        if (!_output_tree_map.containsKey(parent_resultfile)) {
            parent = new DefaultMutableTreeNode(parent_resultfile);
            _output_tree_map.put(parent_resultfile, parent);
            _tree_output_map.put(parent, parent_resultfile);
        } else {
            parent = _output_tree_map.get(parent_resultfile);
        }
        if (!_output_tree_map.containsKey(drilldown_resultfile)) {
            child = new DefaultMutableTreeNode(drilldown_resultfile);
            _output_tree_map.put(drilldown_resultfile, child);
            _tree_output_map.put(child, drilldown_resultfile);            
        } else {
            child = _output_tree_map.get(drilldown_resultfile);
        }
        parent.add(child);        
    }
    
    /**
     * Returns whether window can close.
     */
    public boolean GetCanClose() {
        return gbCanClose;
    }

    /**
     * Sets whether window can close.
     */
    public void setCanClose(boolean b) {
        gbCanClose = b;
    }

    /**
     * Enables the email button.
     */
    public void enableEmailButton() {
        _emailButton.setEnabled(true);
    }

    /**
     * Set property that indicates whether printing is enabled.
     */
    public void setPrintEnabled() {
        gbCanPrint = true;
    }

    /**
     * Returns whether user has cancelled analysis.
     */
    synchronized public boolean IsJobCanceled() {
        return gbCancel;
    }

    /**
     * This method returns a new String object, ensuring it ends in newline.
     * The intended use for this method is to append text to the JTextArea
     * controls from originating JNI method. The reason for creating a new
     * object is that the JNI method uses NewStringUTF(JNIENV*, const char*)
     * and many repeated calls exhaust Java heap. The solution appears to be
     * use of method DeleteLocalRef(JNIENV*,jobject) after JNI callbacl using
     * created string. What I'm not sure about here is the behavior between
     * delayed SwingUtilities.invokeLater() call and DeleteLocalRef(JNIENV*,jobject).
     * What does it mean to possibly first call DeleteLocalRef call then invokeLater()
     * on same String object?
     */
    private String getNewInvokeLaterString(final String s) {
        return new String(s + (s.endsWith("\n") ? "" : "\n"));
    }
    
    /**
     * Prints progress string to output textarea.
     */
    synchronized public void PrintProgressWindow(final String ProgressString) {
        final String progress = getNewInvokeLaterString(ProgressString);
        SwingUtilities.invokeLater(new Runnable() {
            public void run() {
                _progressTextArea.append(progress);
            }
        }); 
        
        //Limit number of lines to specified maximum. The JTextArea control is a real
        //memory hog once the number of lines gets large. With default java heap, memory
        //exhausts around the 143,000 line appended. When printing simulations to window, 
        //it's had to believe anyone would be interested in more than MAXLINES lines.
        Element root = _progressTextArea.getDocument().getDefaultRootElement();
        if (root.getElementCount() > MAXLINES) {
           Element firstLine = root.getElement(0);
           try {
             _progressTextArea.getDocument().remove(0, firstLine.getEndOffset());
           } catch(BadLocationException e) {
             gbCanClose = true; 
           }                
        }
    }

    /**
     * Prints warning/error string to output textarea.
     */
    synchronized public void PrintIssuesWindndow(final String ProgressString) {
        _warning_errors_encountered = true;
        final String progress = getNewInvokeLaterString(ProgressString);
        SwingUtilities.invokeLater(new Runnable() {
            public void run() {
                _warningsErrorsTextArea.append(progress);
            }
        });
    }

    private boolean placemarkExist(File file) throws ParserConfigurationException, SAXException, IOException {
        DocumentBuilderFactory dbFactory = DocumentBuilderFactory.newInstance();
        DocumentBuilder dBuilder = dbFactory.newDocumentBuilder();
        org.w3c.dom.Document doc = dBuilder.parse(file);
        doc.getDocumentElement().normalize();
        NodeList nodes = doc.getElementsByTagName("Placemark");
        return nodes.getLength() > 0;
    }
    
    /** Loads analysis results from file into memo control */
    synchronized public void LoadFromFile(final String sFileName, boolean loadTree) {
        if (loadTree && _tree_output_map.size() > 0) {
            /* Drilldown analyses exist, add the dridown result tab.*/
            _bottom_tabbed_pane.add("Drilldown", _drilldown_results_tab);
            /* Find the node that is the primary analysis and set as root. */
            for (DefaultMutableTreeNode node : _tree_output_map.keySet()) {
                if (node.getParent() == null) {
                    sortTree(node);
                    _results_tree.setModel(new DefaultTreeModel(node));
                    _results_tree.setSelectionPath(new TreePath(node.getPath()));
                    break;
                }
            }
        }
        
        SwingUtilities.invokeLater(new Runnable() {
            public void run() {
                _progressTextArea.setText("");
                try {
                    Charset charset = Charset.forName("UTF-8");
                    BufferedReader reader = new BufferedReader(new InputStreamReader(new FileInputStream(sFileName), charset));
                    _progressTextArea.read(reader, null);
                    reader.close();
                    setTitle(sFileName);
                } catch (IOException e) {
                    setTitle("Job Completed");
                    _progressTextArea.append("\nSaTScan completed successfully but was unable to read results from file.\n" +
                            "The results have been written to: \n" + _parameters.GetOutputFileName() + "\n\n");
                }
                _progressTextArea.setCaretPosition(0);
                OutputFileRegister.getInstance().release(_parameters.GetOutputFileName());
            
                try {
                    if (_parameters.getOutputKMLFile() && _parameters.getLaunchMapViewer()) {
                        int extIndex = _parameters.GetOutputFileName().lastIndexOf('.');
                        extIndex = (extIndex == -1 ? _parameters.GetOutputFileName().length() : extIndex);
                        File path = new File(_parameters.GetOutputFileName().substring(0, extIndex) + (_parameters.getCompressClusterKML() ? ".kmz" : ".kml"));
                        
                        // Determine if any clusters were reported -- prevent launching Google Earth when no clusters reported.
                        boolean reallyLaunch = true;
                        if (_parameters.getCompressClusterKML()) {
                            // open kmz and check the primary kml file for placemarks
                            File test = new File(_parameters.GetOutputFileName().substring(0, extIndex) + ".kml");
                            test.createNewFile();
                            //get the zip file content
                            ZipInputStream zis = new ZipInputStream(new FileInputStream(path));
                            //get the zipped file list entry
                            ZipEntry ze = zis.getNextEntry();  
                            while(ze != null) {
                                if (ze.getName().equals(test.getName())) {
                                   byte[] buffer = new byte[1024];                            
                                   FileOutputStream fos = new FileOutputStream(test);
                                   int len;
                                   while ((len = zis.read(buffer)) > 0) {
                                        fos.write(buffer, 0, len);
                                   }
                                   fos.close();
                                   break;
                                }
                                ze = zis.getNextEntry();
                            }
                            reallyLaunch = placemarkExist(test);
                            test.delete();
                        } else {
                            reallyLaunch = placemarkExist(path);
                        }

                        if (reallyLaunch) {                            
                            if (Desktop.isDesktopSupported() && Desktop.getDesktop().isSupported(Desktop.Action.OPEN)) {
                                Desktop.getDesktop().open(path);
                            } else {
                                String kmlFile = "file://localhost/" + path.getAbsolutePath();
                                kmlFile = kmlFile.replace('\\', '/');
                                BareBonesBrowserLaunch.openURL(kmlFile);
                            }                        
                        }
                    }
                } catch (Throwable t) {
                    JOptionPane.showMessageDialog(null, "Unable to launch KML file viewer. If you do not have a viewer, you can download Google Earth from http://www.google.com/earth/download/.");
                }
                
                try {
                    if (_parameters.getOutputCartesianGraph() && _parameters.getLaunchMapViewer()) {
                        int extIndex = _parameters.GetOutputFileName().lastIndexOf('.');
                        extIndex = (extIndex == -1 ? _parameters.GetOutputFileName().length() : extIndex);
                        File path = new File(_parameters.GetOutputFileName().substring(0, extIndex) + ".cluster.html");
                        if (Desktop.isDesktopSupported() && Desktop.getDesktop().isSupported(Desktop.Action.OPEN)) {
                            Desktop.getDesktop().open(path);
                        } else {
                            String htmlFile = "file://localhost/" + path.getAbsolutePath();
                            htmlFile = htmlFile.replace('\\', '/');
                            BareBonesBrowserLaunch.openURL(htmlFile);
                       }                        
                    }
                } catch (Throwable t) {
                    JOptionPane.showMessageDialog(null, "Unable to launch cartesian graph in default browser.");
                }

                try {
                    if (_parameters.getOutputGoogleMapsFile() && _parameters.getLaunchMapViewer()) {
                        int extIndex = _parameters.GetOutputFileName().lastIndexOf('.');
                        extIndex = (extIndex == -1 ? _parameters.GetOutputFileName().length() : extIndex);
                        File path = new File(_parameters.GetOutputFileName().substring(0, extIndex) + ".clustermap.html");
                        if (Desktop.isDesktopSupported() && Desktop.getDesktop().isSupported(Desktop.Action.OPEN)) {
                            Desktop.getDesktop().open(path);
                        } else {
                            String htmlFile = "file://localhost/" + path.getAbsolutePath();
                            htmlFile = htmlFile.replace('\\', '/');
                            BareBonesBrowserLaunch.openURL(htmlFile);
                       }                        
                    }
                } catch (Throwable t) {
                    JOptionPane.showMessageDialog(null, "Unable to launch cartesian graph in default browser.");
                }

            }
        });
    }

    /**
     * Sends output window text to printer.
     */
    public void printWindow() {
        try {
            if (gbCanPrint) {
                String sPrintText = _progressTextArea.getText() +
                        "\n\n\nWARNINGS / ERRORS\n" +
                        _warningsErrorsTextArea.getText();

                JDocumentRenderer documentRenderer = new JDocumentRenderer();
                documentRenderer.print(new JEditorPane("text/plain", sPrintText));
            }
        } catch (Exception e) {
            JOptionPane.showMessageDialog(this, "Unable to print.", "Operation Failed", JOptionPane.INFORMATION_MESSAGE);
        }
    }

    /**
     * Launches default emial application and creates message detailing error.
     */
    private void launchDefaultClientEmail() {
        StringBuilder messageBody = new StringBuilder();

        messageBody.append("--Job Progress Information--\n");
        messageBody.append(_progressTextArea.getText());
        messageBody.append("\n\n\n--Warnings/Errors Information--\n");
        messageBody.append(_warningsErrorsTextArea.getText());
        if (gsProgramErrorCallPath.length() > 0) {
            messageBody.append("\n\n--Call Path Information--\n\n");
            messageBody.append(gsProgramErrorCallPath);
            messageBody.append("\n\n");
        }
        messageBody.append("\n--End Of Error Message--");

        EmailClientLauncher launcher = new EmailClientLauncher();
        if (!launcher.launchDefaultClientEmail(AppConstants.getTechnicalSupportEmail(), "Automated Error Message", messageBody.toString())) {
            JOptionPane.showMessageDialog(this, "Unable to launch default email application.", "Operation Failed", JOptionPane.INFORMATION_MESSAGE);
        }
    }

    /**
     * triggers TForm::Close() -- if bForce is true, the thread is forced to terminate
     */
    public void forceClose() {
        //ForceThreadTermination();
        OutputFileRegister.getInstance().release(_parameters.GetOutputFileName());
        setCanClose(true);
        dispose();
    }

    /**
     *
     */
    public void setCloseButton() {
        _cancelButton.setText("Close");
    }

    /**
     *
     */
    public void CancelJob() {
        if (getWarningsErrorsEncountered() == false) {
            _progressTextArea.append("Job cancelled. Please review 'Warnings/Errors' window below.");
        } else {
            _progressTextArea.append("Job cancelled.");
        }
        setTitle("Job cancelled");
        setCloseButton();
        gbCancel = true;
        OutputFileRegister.getInstance().release(_parameters.GetOutputFileName());
        setCanClose(true);
    }

    /** */
    public void setProgramErrorCallpathExplicit(String path) {
        gsProgramErrorCallPath = path;
    }

    /** */
    public void setProgramErrorCallpath(StackTraceElement[] stackTrace) {
        StringBuilder trace = new StringBuilder();
        for (int i = 0; i < stackTrace.length; ++i) {
            trace.append(stackTrace[i].getMethodName() + " of " + stackTrace[i].getClassName() + "\n");
        }
        gsProgramErrorCallPath = trace.toString();
    }

    /** */
    public boolean getWarningsErrorsEncountered() {
        return _warning_errors_encountered;
    }

    /* Determines the nodes label to be displayed in jTree and whether node is a Bernoulli analysis. */
    private static Pair <String,Boolean> getNodeLabel(DefaultMutableTreeNode node) {
        Pair <String, Boolean> response;
        File f = new File(node.getUserObject().toString());
        if (node.getParent() == null) {
            response = Pair.of(f.getName(), false);
        } else {
            Pattern r = Pattern.compile("^.+\\-(((C\\d+)+)\\-(std|bin)){1}.*$");
            Matcher m = r.matcher(f.getName());
            if (m.find() && m.groupCount() == 4) {
                response = Pair.of(m.group(1), m.group(4).equals("bin"));
            } else {
                response = Pair.of(f.getName(), false);
            }
        }
        return response;
    }   
    
    /* Sorts tree such that Bernoulli (bin) analyses group together at tree level first. */
    private static void sortTree(DefaultMutableTreeNode root) {
      Enumeration e = root.depthFirstEnumeration();
      while (e.hasMoreElements()) {
        DefaultMutableTreeNode thisnode = (DefaultMutableTreeNode) e.nextElement();
        if (!thisnode.isLeaf()) {
            List< DefaultMutableTreeNode> children = new ArrayList<>();
            Enumeration e_nodes = thisnode.children();
            while (e_nodes.hasMoreElements()) {
              children.add((DefaultMutableTreeNode)e_nodes.nextElement());
            } 
            // Sorts tree such that Bernoulli nodes come first in respective tree level.
            Collections.sort(children, new Comparator< DefaultMutableTreeNode>() {
                @Override public int compare(DefaultMutableTreeNode a, DefaultMutableTreeNode b) {
                    Pair <String, Boolean> sa = getNodeLabel(a), sb = getNodeLabel(b);
                    // If both are Bernoulli or standard, just compare strings.
                    if ((sa.getValue() && sb.getValue()) || (!sa.getValue() && !sb.getValue()))
                        return sa.getKey().compareToIgnoreCase(sb.getKey());
                    // Otherwise sort Bernoulli over standard.
                    return sa.getValue() ? -1 : 1;
                }
            });
            thisnode.removeAllChildren();
            for (MutableTreeNode node: children) {
              thisnode.add(node);
            }            
        }
      }
    }

    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

        jSplitPane1 = new javax.swing.JSplitPane();
        _bottom_panel = new javax.swing.JPanel();
        _bottom_tabbed_pane = new javax.swing.JTabbedPane();
        _warnings_errors_tab = new javax.swing.JPanel();
        jScrollPane2 = new javax.swing.JScrollPane();
        _warningsErrorsTextArea = new javax.swing.JTextArea();
        _drilldown_results_tab = new javax.swing.JPanel();
        jScrollPane1 = new javax.swing.JScrollPane();
        _results_tree = new javax.swing.JTree();
        _cancelButton = new javax.swing.JButton();
        _emailButton = new javax.swing.JButton();
        _primary_panel = new javax.swing.JPanel();
        jScrollPane3 = new javax.swing.JScrollPane();
        _progressTextArea = new javax.swing.JTextArea();

        setClosable(true);
        setDefaultCloseOperation(javax.swing.WindowConstants.DO_NOTHING_ON_CLOSE);
        setIconifiable(true);
        setMaximizable(true);
        setResizable(true);

        jSplitPane1.setBorder(null);
        jSplitPane1.setDividerLocation(251);
        jSplitPane1.setOrientation(javax.swing.JSplitPane.VERTICAL_SPLIT);
        jSplitPane1.setResizeWeight(1.0);

        _warningsErrorsTextArea.setEditable(false);
        _warningsErrorsTextArea.setMargin(new java.awt.Insets(5, 5, 5, 5));
        jScrollPane2.setViewportView(_warningsErrorsTextArea);

        javax.swing.GroupLayout _warnings_errors_tabLayout = new javax.swing.GroupLayout(_warnings_errors_tab);
        _warnings_errors_tab.setLayout(_warnings_errors_tabLayout);
        _warnings_errors_tabLayout.setHorizontalGroup(
            _warnings_errors_tabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addComponent(jScrollPane2, javax.swing.GroupLayout.DEFAULT_SIZE, 686, Short.MAX_VALUE)
        );
        _warnings_errors_tabLayout.setVerticalGroup(
            _warnings_errors_tabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addComponent(jScrollPane2, javax.swing.GroupLayout.DEFAULT_SIZE, 120, Short.MAX_VALUE)
        );

        _bottom_tabbed_pane.addTab("Warnings/Errors", _warnings_errors_tab);

        _results_tree.setModel(null);
        jScrollPane1.setViewportView(_results_tree);
        _results_tree.setCellRenderer(new FileTreeCellRenderer());
        _results_tree.getSelectionModel().setSelectionMode(TreeSelectionModel.SINGLE_TREE_SELECTION);
        _results_tree.setBorder(javax.swing.BorderFactory.createEmptyBorder(5, 5, 5, 5));
        _results_tree.addMouseMotionListener(new MouseMotionListener() {
            @Override
            public void mouseDragged(MouseEvent e) { }
            @Override
            public void mouseMoved(MouseEvent e) {
                TreePath tp = ((JTree)e.getSource()).getPathForLocation(e.getX(), e.getY());
                if(tp != null) {
                    ((JTree)e.getSource()).setCursor(new Cursor(Cursor.HAND_CURSOR));
                } else {
                    ((JTree)e.getSource()).setCursor(new Cursor(Cursor.DEFAULT_CURSOR));
                }
            }
        });
        _results_tree.addTreeSelectionListener(new TreeSelectionListener() {
            public void valueChanged(TreeSelectionEvent e) {
                DefaultMutableTreeNode node = (DefaultMutableTreeNode)_results_tree.getLastSelectedPathComponent();
                if (node == null) return;
                LoadFromFile(_tree_output_map.get(node), false);
            }
        });

        javax.swing.GroupLayout _drilldown_results_tabLayout = new javax.swing.GroupLayout(_drilldown_results_tab);
        _drilldown_results_tab.setLayout(_drilldown_results_tabLayout);
        _drilldown_results_tabLayout.setHorizontalGroup(
            _drilldown_results_tabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGap(0, 686, Short.MAX_VALUE)
            .addGroup(_drilldown_results_tabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addComponent(jScrollPane1, javax.swing.GroupLayout.DEFAULT_SIZE, 686, Short.MAX_VALUE))
        );
        _drilldown_results_tabLayout.setVerticalGroup(
            _drilldown_results_tabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGap(0, 120, Short.MAX_VALUE)
            .addGroup(_drilldown_results_tabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                .addComponent(jScrollPane1, javax.swing.GroupLayout.Alignment.TRAILING, javax.swing.GroupLayout.DEFAULT_SIZE, 120, Short.MAX_VALUE))
        );

        _bottom_tabbed_pane.addTab("Drilldown", _drilldown_results_tab);

        _cancelButton.setText("Cancel");
        _cancelButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                if (_cancelButton.getText().equals("Close")) {
                    try {
                        AnalysisRunInternalFrame.this.setClosed(true);
                    } catch (PropertyVetoException e1) {
                        // TODO Auto-generated catch block
                        e1.printStackTrace();
                    }
                } else {
                    PrintProgressWindow("Cancelling job, please wait...");
                    gbCancel = true;
                }
                // gRegistry.Release(gsOutputFileName);
            }
        });

        _emailButton.setText("Email");
        _emailButton.setEnabled(false);
        _emailButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                launchDefaultClientEmail();
            }
        });

        javax.swing.GroupLayout _bottom_panelLayout = new javax.swing.GroupLayout(_bottom_panel);
        _bottom_panel.setLayout(_bottom_panelLayout);
        _bottom_panelLayout.setHorizontalGroup(
            _bottom_panelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, _bottom_panelLayout.createSequentialGroup()
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .addComponent(_emailButton, javax.swing.GroupLayout.PREFERRED_SIZE, 76, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_cancelButton, javax.swing.GroupLayout.PREFERRED_SIZE, 76, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap())
            .addComponent(_bottom_tabbed_pane)
        );
        _bottom_panelLayout.setVerticalGroup(
            _bottom_panelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_bottom_panelLayout.createSequentialGroup()
                .addComponent(_bottom_tabbed_pane)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(_bottom_panelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(_cancelButton)
                    .addComponent(_emailButton)))
        );

        jSplitPane1.setRightComponent(_bottom_panel);

        _progressTextArea.setEditable(false);
        _progressTextArea.setColumns(20);
        _progressTextArea.setRows(5);
        _progressTextArea.setMargin(new java.awt.Insets(5, 5, 5, 5));
        jScrollPane3.setViewportView(_progressTextArea);

        javax.swing.GroupLayout _primary_panelLayout = new javax.swing.GroupLayout(_primary_panel);
        _primary_panel.setLayout(_primary_panelLayout);
        _primary_panelLayout.setHorizontalGroup(
            _primary_panelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addComponent(jScrollPane3, javax.swing.GroupLayout.DEFAULT_SIZE, 691, Short.MAX_VALUE)
        );
        _primary_panelLayout.setVerticalGroup(
            _primary_panelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addComponent(jScrollPane3, javax.swing.GroupLayout.DEFAULT_SIZE, 251, Short.MAX_VALUE)
        );

        jSplitPane1.setLeftComponent(_primary_panel);

        javax.swing.GroupLayout layout = new javax.swing.GroupLayout(getContentPane());
        getContentPane().setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(layout.createSequentialGroup()
                .addContainerGap()
                .addComponent(jSplitPane1)
                .addContainerGap())
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(layout.createSequentialGroup()
                .addContainerGap()
                .addComponent(jSplitPane1)
                .addContainerGap())
        );

        pack();
    }// </editor-fold>//GEN-END:initComponents
    @Override
    public void internalFrameOpened(InternalFrameEvent e) {
    }

    @Override
    public void internalFrameClosing(InternalFrameEvent e) {
        if (GetCanClose()) {
            OutputFileRegister.getInstance().release(_parameters.GetOutputFileName());
            dispose();
        }
    }

    @Override
    public void internalFrameClosed(InternalFrameEvent e) {
    }

    @Override
    public void internalFrameIconified(InternalFrameEvent e) {
    }

    @Override
    public void internalFrameDeiconified(InternalFrameEvent e) {
    }

    @Override
    public void internalFrameActivated(InternalFrameEvent e) {
    }

    @Override
    public void internalFrameDeactivated(InternalFrameEvent e) {
    }
   
    /** A TreeCellRenderer for displaying filename only of analysis output files. */
    class FileTreeCellRenderer extends DefaultTreeCellRenderer {
        private JLabel label;

        FileTreeCellRenderer() {
            label = new JLabel();
            label.setOpaque(true);
            super.setTextSelectionColor(Color.white);
            super.setBackgroundSelectionColor(new Color(229, 228, 226));
            super.setBorderSelectionColor(Color.black);
        }

        @Override
        public Component getTreeCellRendererComponent(
            JTree tree,
            Object value,
            boolean selected,
            boolean expanded,
            boolean leaf,
            int row,
            boolean hasFocus) {
            DefaultMutableTreeNode node = (DefaultMutableTreeNode)value;
            label.setIcon(new javax.swing.ImageIcon(getClass().getResource("/bullet-blue-icon.png")));
            label.setText(getNodeLabel(node).getKey());
            label.setToolTipText(node.getUserObject().toString());
            if (selected) {
                label.setBackground(backgroundSelectionColor);
            } else {
                label.setBackground(backgroundNonSelectionColor);
            }
            int width = 10 + label.getIcon().getIconWidth();
            width += SwingUtilities.computeStringWidth(label.getFontMetrics(label.getFont()), label.getText());
            label.setPreferredSize( new Dimension(width, (int)label.getPreferredSize().getHeight()) );            
            return label;
        }
    }    
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JPanel _bottom_panel;
    private javax.swing.JTabbedPane _bottom_tabbed_pane;
    private javax.swing.JButton _cancelButton;
    private javax.swing.JPanel _drilldown_results_tab;
    private javax.swing.JButton _emailButton;
    private javax.swing.JPanel _primary_panel;
    private javax.swing.JTextArea _progressTextArea;
    private javax.swing.JTree _results_tree;
    private javax.swing.JTextArea _warningsErrorsTextArea;
    private javax.swing.JPanel _warnings_errors_tab;
    private javax.swing.JScrollPane jScrollPane1;
    private javax.swing.JScrollPane jScrollPane2;
    private javax.swing.JScrollPane jScrollPane3;
    private javax.swing.JSplitPane jSplitPane1;
    // End of variables declaration//GEN-END:variables
}
