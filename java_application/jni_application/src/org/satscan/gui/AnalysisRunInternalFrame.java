package org.satscan.gui;

import java.awt.Color;
import java.awt.Component;
import java.awt.Cursor;
import java.awt.Desktop;
import java.awt.Dimension;
import java.awt.event.MouseAdapter;
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
import javax.swing.JMenuItem;
import javax.swing.JOptionPane;
import javax.swing.JPopupMenu;
import javax.swing.JSplitPane;
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
    private boolean _failed_kml_view = false;
    private boolean _failed_maps_view = false;
    private boolean _failed_graph_view = false;
    private final Parameters _parameters;
    private String gsProgramErrorCallPath = "";
    private final int MAXLINES = 999;
    private boolean _warning_errors_encountered = false;
    private Map<FilterTreeNode, String> _tree_output_map = new HashMap<>();
    private Map<String, FilterTreeNode> _output_tree_map = new HashMap<>();
    private Map<FilterTreeNode, Integer> _tree_output_significances = new HashMap<>();
    private boolean _has_insignificant_analyses = false;
    private boolean _viewing_only_significant = true;
    private boolean _first_display_occurred = false;
    private JPopupMenu _launch_output = null;

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
    public void ReportDrilldownResults(String drilldown_resultfile, String parent_resultfile, int significantClusters) {
        FilterTreeNode parent;
        FilterTreeNode child;
        if (!_output_tree_map.containsKey(parent_resultfile)) {
            parent = new FilterTreeNode(parent_resultfile);
            _output_tree_map.put(parent_resultfile, parent);
            _tree_output_map.put(parent, parent_resultfile);
        } else {
            parent = _output_tree_map.get(parent_resultfile);
        }
        if (!_output_tree_map.containsKey(drilldown_resultfile)) {
            child = new FilterTreeNode(drilldown_resultfile);
            _output_tree_map.put(drilldown_resultfile, child);
            _tree_output_map.put(child, drilldown_resultfile);            
        } else {
            child = _output_tree_map.get(drilldown_resultfile);
        }
        parent.add(child);
        _tree_output_significances.put(child, significantClusters);
        _has_insignificant_analyses |= significantClusters == 0;
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

    /** Loads the analysis results and drilldown tree, if there is one. */
    synchronized public void loadAnalysisResults(boolean sortTreeNodes) {
        btnToggleDrilldown.setEnabled(_has_insignificant_analyses);
        OutputFileRegister.getInstance().release(_parameters.GetOutputFileName());
        if (_tree_output_map.size() > 0) {
            /* Drilldown analyses exist, add the drilldown result tab.*/
            _bottom_tabbed_pane.add("Drilldown", _drilldown_results_tab);
            /* Find the node that is the primary analysis and set as root. */
            for (FilterTreeNode node : _tree_output_map.keySet()) {
                if (node.getParent() == null) {
                    if (sortTreeNodes) {
                        // If sorting JTree from root, first turn off filtering so that all nodes sort properly.
                        ((FilterTreeModel)_results_tree.getModel()).activateFilter(false);
                        sortTree(node);
                    }
                    // Reset the TreeModel for JTree and select root node.
                    _results_tree.setModel(new FilterTreeModel(node, false, _viewing_only_significant));
                    _results_tree.setSelectionPath(new TreePath(node.getPath()));
                    break;
                }
            }
            // Add the ability to launch the additional output files for any of the analyses.
            btnLaunchActions.setEnabled(_parameters.getOutputKMLFile() || _parameters.getOutputCartesianGraph() || _parameters.getOutputGoogleMapsFile());
            if (_launch_output == null && btnLaunchActions.isEnabled()) {
                _launch_output = new JPopupMenu();
                btnLaunchActions.add(_launch_output);
                btnLaunchActions.addMouseListener(new MouseAdapter() {
                    public void mousePressed(MouseEvent e) {
                        _launch_output.show(e.getComponent(), e.getX(), e.getY());
                    }
                });            
                if (_parameters.getOutputKMLFile()) {
                    JMenuItem menuItem = new JMenuItem("View Google Earth");
                    menuItem.addActionListener(new java.awt.event.ActionListener() {
                        public void actionPerformed(java.awt.event.ActionEvent evt) {
                            DefaultMutableTreeNode selectedNode = (DefaultMutableTreeNode)_results_tree.getLastSelectedPathComponent(); 
                            launchKMLViewer(_tree_output_map.get(selectedNode), false);
                        }
                    });            
                    _launch_output.add(menuItem);
                }
                if (_parameters.getOutputGoogleMapsFile()) {
                    JMenuItem menuItem = new JMenuItem("View Google Map");
                    menuItem.addActionListener(new java.awt.event.ActionListener() {
                        public void actionPerformed(java.awt.event.ActionEvent evt) {
                            DefaultMutableTreeNode selectedNode = (DefaultMutableTreeNode)_results_tree.getLastSelectedPathComponent(); 
                            launchGoogleMapsViewer(_tree_output_map.get(selectedNode));
                        }
                    });            
                    _launch_output.add(menuItem);
                }
                if (_parameters.getOutputCartesianGraph()) {
                    JMenuItem menuItem = new JMenuItem("View Cartesian Graph");
                    menuItem.addActionListener(new java.awt.event.ActionListener() {
                        public void actionPerformed(java.awt.event.ActionEvent evt) {
                            DefaultMutableTreeNode selectedNode = (DefaultMutableTreeNode)_results_tree.getLastSelectedPathComponent(); 
                            launchCartesianGraphViewer(_tree_output_map.get(selectedNode));
                        }
                    });            
                    _launch_output.add(menuItem);
                }
            }
        } else {
            LoadFromFile(_parameters.GetOutputFileName());
        }
    }
    
    /** Loads analysis results from file into textarea control */
    public void LoadFromFile(final String sFileName) {
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
                            "The results have been written to: \n" + sFileName + "\n\n");
                }
                _progressTextArea.setCaretPosition(0);
                // If requested, attempt to launch a KML viewer.
                if (_parameters.getOutputKMLFile() && _parameters.getLaunchMapViewer()&& !_first_display_occurred && !_failed_kml_view) {
                    launchKMLViewer(sFileName, true);
                }
                // If requested, attempt to launch browser to view cartesian map.
                if (_parameters.getOutputCartesianGraph() && _parameters.getLaunchMapViewer()&& !_first_display_occurred && !_failed_graph_view) {
                    launchCartesianGraphViewer(sFileName);                      
                }
                // If requested, attempt to launch browser to view Google map.
                if (_parameters.getOutputGoogleMapsFile() && _parameters.getLaunchMapViewer()&& !_first_display_occurred && !_failed_maps_view) {
                    launchGoogleMapsViewer(sFileName);
                }
                /* Only automatically launch additional output files if user requests and this is the first time loading results.
                   This prevents causing the automatic launch with each selection of a treenode in the drilldown tree.
                */
                _first_display_occurred = true;
            }
        });
    }

    /* Attempt to launch application which can view KML/KMZ files. */
    private void launchKMLViewer(final String output_filename, boolean verifyClusters) {
        try {
            int extIndex = output_filename.lastIndexOf('.');
            extIndex = (extIndex == -1 ? output_filename.length() : extIndex);
            File path = new File(output_filename.substring(0, extIndex) + (_parameters.getCompressClusterKML() ? ".kmz" : ".kml"));
            // Determine if any clusters were reported -- prevent launching Google Earth when no clusters reported.
            boolean reallyLaunch = true;
            if (_parameters.getCompressClusterKML()) {
                // open kmz and check the primary kml file for placemarks
                File test = new File(output_filename.substring(0, extIndex) + ".kml");
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
                reallyLaunch = verifyClusters ? placemarkExist(test) : true;
                test.delete();
            } else {
                reallyLaunch = verifyClusters ? placemarkExist(path) : true;
            }
            if (reallyLaunch) { 
                launchFileViewer(path);
            }
        } catch (Throwable t) {
            _failed_kml_view = true;
            JOptionPane.showMessageDialog(null, "Unable to launch KML file viewer. If you do not have a viewer, you can download Google Earth from http://www.google.com/earth/download/.");
        }        
    }    
    
    /* Attempt to launch application which can view Cartesian graph html file. */
    private void launchCartesianGraphViewer(final String output_filename) {
        try {
            int extIndex = output_filename.lastIndexOf('.');
            extIndex = (extIndex == -1 ? output_filename.length() : extIndex);
            File path = new File(output_filename.substring(0, extIndex) + ".cluster.html");
            launchFileViewer(path);                      
        } catch (Throwable t) {
            _failed_graph_view = true;
            JOptionPane.showMessageDialog(null, "Unable to launch cartesian graph in default browser.");
        }        
    }    
    
    /* Attempt to launch application which can view Google Maps html file. */
    private void launchGoogleMapsViewer(final String output_filename) {
        try {
            int extIndex = output_filename.lastIndexOf('.');
            extIndex = (extIndex == -1 ? output_filename.length() : extIndex);
            File path = new File(output_filename.substring(0, extIndex) + ".clustermap.html");
            launchFileViewer(path);
        } catch (Throwable t) {
            _failed_maps_view = true;
            JOptionPane.showMessageDialog(null, "Unable to launch cartesian graph in default browser.");
        }        
    }
    
    /* Attempt to launch application which can view html files. */
    private void launchFileViewer(final File path) throws IOException {
        if (Desktop.isDesktopSupported() && Desktop.getDesktop().isSupported(Desktop.Action.OPEN)) {
            Desktop.getDesktop().open(path);
        } else {
            String htmlFile = "file://localhost/" + path.getAbsolutePath();
            htmlFile = htmlFile.replace('\\', '/');
            BareBonesBrowserLaunch.openURL(htmlFile);
        }                        
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
            Pattern r = Pattern.compile("^.+\\-drilldown\\-(((C\\d+)+)\\-(std|bin)){1}.*$");
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
    private static void sortTree(FilterTreeNode root) {
      Enumeration e = root.depthFirstEnumeration();
      while (e.hasMoreElements()) {
        FilterTreeNode thisnode = (FilterTreeNode) e.nextElement();
        if (!thisnode.isLeaf()) {
            List< FilterTreeNode> children = new ArrayList<>();
            Enumeration e_nodes = thisnode.children();
            while (e_nodes.hasMoreElements()) {
                children.add((FilterTreeNode)e_nodes.nextElement());
            } 
            // Sorts tree such that Bernoulli nodes come first in respective tree level.
            Collections.sort(children, new Comparator< DefaultMutableTreeNode>() {
                @Override public int compare(DefaultMutableTreeNode a, DefaultMutableTreeNode b) {
                    Pair <String, Boolean> sa = getNodeLabel(a), sb = getNodeLabel(b);
                    // If both are Bernoulli or standard, just compare cluster number.
                    Pattern r = Pattern.compile("^((C(\\d+))+)\\-(std|bin)$");
                    Matcher m1 = r.matcher(sa.getKey()), m2 = r.matcher(sb.getKey());
                    if (m1.find() && m2.find()) {
                        Integer sai = Integer.parseInt(m1.group(3));
                        Integer sbi = Integer.parseInt(m2.group(3));
                        if (sai == sbi)
                            return sa.getValue() ? -1 : 1; // sort Bernoulli over standard.
                        return sai - sbi;
                    }
                    return sa.getKey().compareToIgnoreCase(sb.getKey());
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
        jPanel1 = new javax.swing.JPanel();
        btnSwitchOrientation = new javax.swing.JButton();
        btnToggleDrilldown = new javax.swing.JButton();
        btnLaunchActions = new javax.swing.JButton();
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
        jSplitPane1.setResizeWeight(0.67);

        _warningsErrorsTextArea.setEditable(false);
        _warningsErrorsTextArea.setMargin(new java.awt.Insets(5, 5, 5, 5));
        jScrollPane2.setViewportView(_warningsErrorsTextArea);

        javax.swing.GroupLayout _warnings_errors_tabLayout = new javax.swing.GroupLayout(_warnings_errors_tab);
        _warnings_errors_tab.setLayout(_warnings_errors_tabLayout);
        _warnings_errors_tabLayout.setHorizontalGroup(
            _warnings_errors_tabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_warnings_errors_tabLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(jScrollPane2, javax.swing.GroupLayout.DEFAULT_SIZE, 666, Short.MAX_VALUE)
                .addContainerGap())
        );
        _warnings_errors_tabLayout.setVerticalGroup(
            _warnings_errors_tabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, _warnings_errors_tabLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(jScrollPane2, javax.swing.GroupLayout.DEFAULT_SIZE, 141, Short.MAX_VALUE))
        );

        _bottom_tabbed_pane.addTab("Warnings/Errors", _warnings_errors_tab);

        jScrollPane1.setViewportView(_results_tree);
        _results_tree.setCellRenderer(new FileTreeCellRenderer());
        _results_tree.setModel(new FilterTreeModel((DefaultMutableTreeNode)_results_tree.getModel().getRoot()));
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
                LoadFromFile(_tree_output_map.get(node));
            }
        });

        btnSwitchOrientation.setIcon(new javax.swing.ImageIcon(getClass().getResource("/Window_App_SplitScreen.png"))); // NOI18N
        btnSwitchOrientation.setToolTipText("Switch Orientation");
        btnSwitchOrientation.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                jSplitPane1.setOrientation(jSplitPane1.getOrientation() == JSplitPane.VERTICAL_SPLIT ? JSplitPane.HORIZONTAL_SPLIT : JSplitPane.VERTICAL_SPLIT);
                System.out.print(" width = " + AnalysisRunInternalFrame.this.getWidth());
                jSplitPane1.setDividerLocation(0.67);
                _bottom_tabbed_pane.setSelectedIndex(1);
            }
        });

        btnToggleDrilldown.setIcon(new javax.swing.ImageIcon(getClass().getResource("/significant.png"))); // NOI18N
        btnToggleDrilldown.setToolTipText("Viewing Only Significant");
        btnToggleDrilldown.setOpaque(false);
        //btnToggleDrilldown.setContentAreaFilled(false);
        //btnToggleDrilldown.setBorderPainted(false);
        btnToggleDrilldown.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                _viewing_only_significant = !_viewing_only_significant;
                if (_viewing_only_significant) {
                    btnToggleDrilldown.setIcon(new javax.swing.ImageIcon(getClass().getResource("/significant.png")));
                    btnToggleDrilldown.setToolTipText("Viewing Only Significant");
                } else {
                    btnToggleDrilldown.setIcon(new javax.swing.ImageIcon(getClass().getResource("/all-results.png")));
                    btnToggleDrilldown.setToolTipText("Viewing All Analyses");
                }
                loadAnalysisResults(false);
                _bottom_tabbed_pane.setSelectedIndex(1);
            }
        });

        btnLaunchActions.setIcon(new javax.swing.ImageIcon(getClass().getResource("/view.png"))); // NOI18N
        btnLaunchActions.setToolTipText("View Other Output");

        javax.swing.GroupLayout jPanel1Layout = new javax.swing.GroupLayout(jPanel1);
        jPanel1.setLayout(jPanel1Layout);
        jPanel1Layout.setHorizontalGroup(
            jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel1Layout.createSequentialGroup()
                .addGap(0, 0, Short.MAX_VALUE)
                .addGroup(jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(btnSwitchOrientation, javax.swing.GroupLayout.Alignment.TRAILING, javax.swing.GroupLayout.PREFERRED_SIZE, 31, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(btnToggleDrilldown, javax.swing.GroupLayout.Alignment.TRAILING, javax.swing.GroupLayout.PREFERRED_SIZE, 31, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(btnLaunchActions, javax.swing.GroupLayout.Alignment.TRAILING, javax.swing.GroupLayout.PREFERRED_SIZE, 31, javax.swing.GroupLayout.PREFERRED_SIZE)))
        );
        jPanel1Layout.setVerticalGroup(
            jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel1Layout.createSequentialGroup()
                .addComponent(btnSwitchOrientation, javax.swing.GroupLayout.PREFERRED_SIZE, 23, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(btnToggleDrilldown, javax.swing.GroupLayout.PREFERRED_SIZE, 23, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(btnLaunchActions, javax.swing.GroupLayout.PREFERRED_SIZE, 23, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addGap(0, 0, Short.MAX_VALUE))
        );

        javax.swing.GroupLayout _drilldown_results_tabLayout = new javax.swing.GroupLayout(_drilldown_results_tab);
        _drilldown_results_tab.setLayout(_drilldown_results_tabLayout);
        _drilldown_results_tabLayout.setHorizontalGroup(
            _drilldown_results_tabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_drilldown_results_tabLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(jScrollPane1, javax.swing.GroupLayout.DEFAULT_SIZE, 629, Short.MAX_VALUE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(jPanel1, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap())
        );
        _drilldown_results_tabLayout.setVerticalGroup(
            _drilldown_results_tabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_drilldown_results_tabLayout.createSequentialGroup()
                .addContainerGap()
                .addGroup(_drilldown_results_tabLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(jScrollPane1, javax.swing.GroupLayout.DEFAULT_SIZE, 125, Short.MAX_VALUE)
                    .addComponent(jPanel1, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)))
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
            .addComponent(jScrollPane3, javax.swing.GroupLayout.DEFAULT_SIZE, 231, Short.MAX_VALUE)
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
            Integer num_significant = _tree_output_significances.get(node);
            if (num_significant == null || num_significant > 0)
                label.setIcon(new javax.swing.ImageIcon(getClass().getResource("/bullet-blue-icon.png")));
            else
                label.setIcon(new javax.swing.ImageIcon(getClass().getResource("/bullet-orange-icon.png")));
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

    /* Specialize class to faciliate filtering which nodes are displayed in JTree. */
    class FilterTreeModel extends DefaultTreeModel {
        protected boolean filterIsActive = false;

        public FilterTreeModel(TreeNode root) {
            this(root, false);
        }

        public FilterTreeModel(TreeNode root, boolean asksAllowsChildren) {
            this(root, false, false);
        }

        public FilterTreeModel(TreeNode root, boolean asksAllowsChildren, boolean filterIsActive) {
            super(root, asksAllowsChildren);
            this.filterIsActive = filterIsActive;
        }

        public void activateFilter(boolean newValue) {
            filterIsActive = newValue;
        }

        public boolean isActivatedFilter() {
            return filterIsActive;
        }

        @Override
        public Object getChild(Object parent, int index) {
            if (filterIsActive && parent instanceof FilterTreeNode)
                return ((FilterTreeNode) parent).getChildAt(index, filterIsActive);
            return ((TreeNode) parent).getChildAt(index);
        }

        @Override
        public int getChildCount(Object parent) {
            if (filterIsActive && parent instanceof FilterTreeNode)
                return ((FilterTreeNode) parent).getChildCount(filterIsActive);
            return ((TreeNode) parent).getChildCount();
        }

    }

    /* Specialize class to faciliate filtering which nodes are displayed in JTree. */
    class FilterTreeNode extends DefaultMutableTreeNode {
        public FilterTreeNode() {
            this(null);
        }

        public FilterTreeNode(Object userObject) {
            this(userObject, true);
        }

        public FilterTreeNode(Object userObject, boolean allowsChildren) {
            super(userObject, allowsChildren);
        }

        /* Returns the child node at index. If currently filtering, gets node from children nodes
           that are signalling visible, otherwise calls parent class to obtain child node. */        
        public TreeNode getChildAt(int index, boolean filterIsActive) {
            if (!filterIsActive) return super.getChildAt(index);
            if (children == null) throw new ArrayIndexOutOfBoundsException("node has no children");
            int realIndex = -1;
            int visibleIndex = -1;
            Enumeration e = children.elements();
            while (e.hasMoreElements()) {
                if (((FilterTreeNode) e.nextElement()).isVisible())
                    visibleIndex++;
                realIndex++;
                if (visibleIndex == index)
                    return (TreeNode) children.elementAt(realIndex);
            }
            throw new ArrayIndexOutOfBoundsException("index unmatched");
        }

        /* Returns the number of children for node. If currently filtering, counts the number of 
           children nodes that are signalling visible, otherwise calls parent class to obtain count. */
        public int getChildCount(boolean filterIsActive) {
            if (!filterIsActive) return super.getChildCount();
            if (children == null) return 0;
            int count = 0;
            Enumeration e = children.elements();
            while (e.hasMoreElements()) {
                if (((FilterTreeNode) e.nextElement()).isVisible()) {
                    count++;
                }
            }
            return count;
        }

        /* Returns whether treenode is visible based upon whether it's the  root or a significant analysis. */
        public boolean isVisible() {            
            Integer num_significant = _tree_output_significances.get(this);
            return (num_significant == null || num_significant > 0);
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
    private javax.swing.JButton btnLaunchActions;
    private javax.swing.JButton btnSwitchOrientation;
    private javax.swing.JButton btnToggleDrilldown;
    private javax.swing.JPanel jPanel1;
    private javax.swing.JScrollPane jScrollPane1;
    private javax.swing.JScrollPane jScrollPane2;
    private javax.swing.JScrollPane jScrollPane3;
    private javax.swing.JSplitPane jSplitPane1;
    // End of variables declaration//GEN-END:variables
}
