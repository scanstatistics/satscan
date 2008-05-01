package org.satscan.gui;

import java.awt.Toolkit;
import java.awt.event.ActionEvent;
import java.awt.event.WindowEvent;
import java.awt.event.WindowFocusListener;
import java.awt.event.WindowListener;
import java.beans.PropertyVetoException;
import java.io.File;
import java.io.IOException;
import java.net.URL;
import java.util.Locale;
import java.util.Vector;
import java.util.logging.Level;
import java.util.logging.Logger;
import javax.help.HelpBroker;
import javax.help.HelpSet;
import javax.swing.AbstractAction;
import javax.swing.Action;
import javax.swing.ImageIcon;
import javax.swing.JFileChooser;
import javax.swing.JInternalFrame;
import javax.swing.JMenuItem;
import javax.swing.JOptionPane;
import javax.swing.UIManager;
import javax.swing.event.InternalFrameEvent;
import javax.swing.event.InternalFrameListener;
import org.satscan.app.AppConstants;
import org.satscan.utils.BareBonesBrowserLaunch;
import org.satscan.utils.FileAccess;
import org.satscan.app.OutputFileRegister;
import org.satscan.app.ParameterHistory;
import org.satscan.app.Parameters;
import org.satscan.gui.utils.InputFileFilter;
import org.satscan.gui.utils.WaitCursor;
/*
 * SaTScanApplication.java
 *
 * Created on December 5, 2007, 11:14 AM
 */

/**
 *
 * @author  Hostovic
 */
public class SaTScanApplication extends javax.swing.JFrame implements WindowFocusListener, WindowListener, InternalFrameListener {

    private static final String _application = System.getProperty("user.dir") + System.getProperty("file.separator") + "SaTScan.jar";

    static { // load the appropriate shared object for VM
        boolean is64BitVM = false;
        try {
            int bits = Integer.getInteger("sun.arch.data.model", 0).intValue();
            if (bits != 0) {
                is64BitVM = bits == 64;
            } else // fallback if sun.arch.data.model isn't available
            {
                is64BitVM = System.getProperty("java.vm.name").toLowerCase().indexOf("64") >= 0;
            }
        } catch (Throwable t) {
        }

        if (is64BitVM) {
            System.loadLibrary("satscan64");
        } else {
            System.loadLibrary("satscan32");
        }
    }
    private static final long serialVersionUID = 1L;
    private final ExecuteSessionAction _executeSessionAction;
    private final ExecuteOptionsAction _executeOptionsAction;
    private final CloseSessionAction _closeSessionAction;
    private final SaveSessionAction _saveSessionAction;
    private final SaveSessionAsAction _saveSessionAsAction;
    private final PrintResultsAction _printResultsAction;
    private JInternalFrame _focusedInternalFrame = null;
    private boolean gbShowStartWindow = true;
    private Vector<JInternalFrame> allOpenFrames = new Vector<JInternalFrame>();
    private static SaTScanApplication _instance;
    public File lastBrowseDirectory = null;

    /**
     * Creates new form SaTScanApplication
     */
    public SaTScanApplication() {
        _instance = this;
        System.out.println(System.getProperties());
        _executeSessionAction = new ExecuteSessionAction();
        _executeOptionsAction = new ExecuteOptionsAction();
        _closeSessionAction = new CloseSessionAction();
        _saveSessionAction = new SaveSessionAction();
        _saveSessionAsAction = new SaveSessionAsAction();
        _printResultsAction = new PrintResultsAction();
        initComponents();
        setTitle(AppConstants.getSoftwareTitle());
        setIconImage(Toolkit.getDefaultToolkit().getImage(getClass().getResource("/SaTScan.gif")));
        enableActions(false, false);
        addWindowFocusListener(this);
        addWindowListener(this);
        refreshOpenList();
        setLocationRelativeTo(null);
    }

    public static SaTScanApplication getInstance() {
        return _instance;
    }

    /**
     * Open new session action.
     */
    public class NewSessionFileAction extends AbstractAction {

        private static final long serialVersionUID = 1L;

        public NewSessionFileAction() {
            super("New Session", new ImageIcon(SaTScanApplication.this.getClass().getResource("/New.gif")));
        }

        public void actionPerformed(ActionEvent e) {
            try {
                openNewParameterSessionWindow("");
            } catch (Throwable t) {
                new ExceptionDialog(SaTScanApplication.this, t).setVisible(true);
            }
        }
    }

    /**
     * Create a new ParameterSettingsFrame internal frame.
     */
    protected void openNewParameterSessionWindow(String sFilename) {
        WaitCursor waitCursor = new WaitCursor(SaTScanApplication.this);

        try {
            ParameterSettingsFrame frame = new ParameterSettingsFrame(getRootPane(), sFilename);
            frame.addInternalFrameListener(this);
            frame.setVisible(true);
            desktopPane.add(frame);
            try {
                frame.setSelected(true);
            } catch (java.beans.PropertyVetoException e) {
            }
        } finally {
            waitCursor.restore();
        }
    }

    /**
     * Opens an existing session from file.
     */
    public class OpenSessionFileAction extends AbstractAction {

        private static final long serialVersionUID = 1L;

        public OpenSessionFileAction() {
            super("Open Session File", new ImageIcon(SaTScanApplication.this.getClass().getResource("/Open.gif")));
        }

        public void actionPerformed(ActionEvent e) {
            try {
                openParameterSessionWindow();
            } catch (Throwable t) {
                new ExceptionDialog(SaTScanApplication.this, t).setVisible(true);
            }
        }
    }

    /**
     * Create a new ParameterSettingsFrame internal frame, loading control
     * settings from file.
     */
    private void openParameterSessionWindow() {
        //Create a file chooser
        JFileChooser fc = new JFileChooser(lastBrowseDirectory);
        fc.setDialogTitle("Select Parameter File");
        fc.addChoosableFileFilter(new InputFileFilter("txt", "Text Files (*.txt)"));
        fc.addChoosableFileFilter(new InputFileFilter("prm", "Parameter Files (*.prm)"));
        int returnVal = fc.showOpenDialog(SaTScanApplication.this);
        if (returnVal == JFileChooser.APPROVE_OPTION) {
            lastBrowseDirectory = fc.getCurrentDirectory();
            openNewParameterSessionWindow(fc.getSelectedFile().getAbsolutePath());
        }
    }

    /**
     * Save session action; calls ParameterSettingsFrame::WriteSession to
     * write settings to file.
     */
    public class SaveSessionAction extends AbstractAction {

        private static final long serialVersionUID = 1L;

        public SaveSessionAction() {
            super("Save Session", new ImageIcon(SaTScanApplication.this.getClass().getResource("/Save.gif")));
        }

        public void actionPerformed(ActionEvent e) {
            try {
                if (_focusedInternalFrame != null && _focusedInternalFrame instanceof ParameterSettingsFrame) {
                    ((ParameterSettingsFrame) _focusedInternalFrame).WriteSession("");
                }
            } catch (Throwable t) {
                new ExceptionDialog(SaTScanApplication.this, t).setVisible(true);
            }
        }
    }

    /**
     * Save As session action; calls ParameterSettingsFrame::WriteSession to
     * write settings to specified file.
     */
    public class SaveSessionAsAction extends AbstractAction {

        private static final long serialVersionUID = 1L;

        public SaveSessionAsAction() {
            super("Save Session As", new ImageIcon(SaTScanApplication.this.getClass().getResource("/SaveAs.gif")));
        }

        public void actionPerformed(ActionEvent e) {
            try {
                if (_focusedInternalFrame != null && _focusedInternalFrame instanceof ParameterSettingsFrame) {
                    ((ParameterSettingsFrame) _focusedInternalFrame).SaveAs();
                }
            } catch (Throwable t) {
                new ExceptionDialog(SaTScanApplication.this, t).setVisible(true);
            }
        }
    }

    /**
     * Close session action; attmepts to close active session window.
     */
    public class CloseSessionAction extends AbstractAction {

        private static final long serialVersionUID = 1L;

        public CloseSessionAction() {
            super("Close Session");
        }

        public void actionPerformed(ActionEvent e) {
            if (_focusedInternalFrame != null && _focusedInternalFrame instanceof ParameterSettingsFrame) {
                try {
                    _focusedInternalFrame.setClosed(true);
                } catch (PropertyVetoException e1) {
                }
            }
        }
    }

    /**
     * Print results action; attempts to print results from run window.
     */
    public class PrintResultsAction extends AbstractAction {

        private static final long serialVersionUID = 1L;

        public PrintResultsAction() {
            super("Print", new ImageIcon(SaTScanApplication.this.getClass().getResource("/Print.gif")));

        }

        public void actionPerformed(ActionEvent e) {
            try {
                if (_focusedInternalFrame instanceof AnalysisRunInternalFrame) {
                    ((AnalysisRunInternalFrame) _focusedInternalFrame).printWindow();
                }
            //JOptionPane.showMessageDialog(SaTScanApplication.this, "PrintResultsAsAction::actionPerformed() not implemented.", "Note", JOptionPane.INFORMATION_MESSAGE);
            } catch (Throwable t) {
                new ExceptionDialog(SaTScanApplication.this, t).setVisible(true);
            }
        }
    }

    /**
     * Application exist action; closes application.
     */
    public class ExitAction extends AbstractAction {

        private static final long serialVersionUID = 1L;

        public ExitAction() {
            super("Exit");
        }

        public void actionPerformed(ActionEvent e) {
            windowClosing(new WindowEvent(SaTScanApplication.this, WindowEvent.WINDOW_CLOSING));
        }
    }

    /**
     * Session execute action; attmepts to start execution of specified
     * analysis settings.
     */
    public class ExecuteSessionAction extends AbstractAction {

        private static final long serialVersionUID = 1L;

        public ExecuteSessionAction() {
            super("Execute", new ImageIcon(SaTScanApplication.this.getClass().getResource("/Execute.gif")));
        }

        public void actionPerformed(ActionEvent e) {
            try {
                executeAnalysis();
            } catch (Throwable t) {
                new ExceptionDialog(SaTScanApplication.this, t).setVisible(true);
            }
        }
    }

    /**
     * Executes analysis from focused session window.
     */
    protected void executeAnalysis() {
        if (_focusedInternalFrame != null && _focusedInternalFrame instanceof ParameterSettingsFrame) {
            if (((ParameterSettingsFrame) _focusedInternalFrame).ValidateParams()) {
                Parameters parameters = ((ParameterSettingsFrame) _focusedInternalFrame).getParameterSettings();
                if (OutputFileRegister.getInstance().isRegistered(parameters.GetOutputFileName())) {
                    JOptionPane.showMessageDialog(SaTScanApplication.this, "The results file for this analysis is currently being written.\n" +
                            "Please specify another filename or wait for analysis to complete.", "Note", JOptionPane.INFORMATION_MESSAGE);
                } else {
                    AnalysisRunInternalFrame frame = new AnalysisRunInternalFrame(parameters);
                    OutputFileRegister.getInstance().register(parameters.GetOutputFileName());
                    frame.addInternalFrameListener(this);
                    frame.setVisible(true);
                    desktopPane.add(frame);
                    try {
                        frame.setSelected(true);
                    } catch (java.beans.PropertyVetoException e) {
                    }

                }
            }
        }
    }

    /**
     * Execute options action; displays the execution options dialog.
     */
    public class ExecuteOptionsAction extends AbstractAction {

        private static final long serialVersionUID = 1L;

        public ExecuteOptionsAction() {
            super("Execute Options", new ImageIcon(SaTScanApplication.this.getClass().getResource("/Options.gif")));
        }

        public void actionPerformed(ActionEvent e) {
            try {
                if (_focusedInternalFrame instanceof ParameterSettingsFrame) {
                    ((ParameterSettingsFrame) _focusedInternalFrame).showExecOptionsDialog(SaTScanApplication.this);
                }
            } catch (Throwable t) {
                new ExceptionDialog(SaTScanApplication.this, t).setVisible(true);
            }
        }
    }

    /**
     * Help system action, launches the help system.
     * TODO: The current help system is Windows only, will this stay?
     */
    public class HelpSystemAction extends AbstractAction {

        static final String helpsetName = "SaTScan_Help";
        private static final long serialVersionUID = 1L;

        public HelpSystemAction() {
            super("Help System", new ImageIcon(SaTScanApplication.this.getClass().getResource("/Help.gif")));
        }

        public void actionPerformed(ActionEvent e) {
            try {
                ClassLoader cl = SaTScanApplication.class.getClassLoader();
                URL url = HelpSet.findHelpSet(cl, helpsetName, "", Locale.getDefault());
                if (url == null) {
                    url = HelpSet.findHelpSet(cl,helpsetName, ".hs", Locale.getDefault());
                    if (url == null) {
                        JOptionPane.showMessageDialog(null,"The help system could not be located."," Help",JOptionPane.WARNING_MESSAGE);
                        return;
                    }
                }
                HelpSet mainHS = new HelpSet(cl, url);
                HelpBroker mainHB = mainHS.createHelpBroker();
                mainHB.setDisplayed(true);
            } catch (Throwable t) {
                new ExceptionDialog(SaTScanApplication.this, t).setVisible(true);
            }                

            //JOptionPane.showMessageDialog(SaTScanApplication.this, "HelpSystemAction::actionPerformed() not implemented.", "Note", JOptionPane.INFORMATION_MESSAGE);
        }
    }

    /**
     * User guide actions; launches Adbode to view user guide in PDF format.
     */
    public class UserGuideAction extends AbstractAction {

        private static final long serialVersionUID = 1L;

        public UserGuideAction() {
            super("User Guide", new ImageIcon(SaTScanApplication.this.getClass().getResource("/UserGuide.gif")));
        }

        public void actionPerformed(ActionEvent e) {
            try {
                File path = new File("SaTScan_Users_Guide.pdf");
                String userGuide = "file://localhost/" + path.getAbsolutePath();
                userGuide = userGuide.replace('\\', '/');
                System.out.println(userGuide);
                BareBonesBrowserLaunch.openURL(userGuide);
            } catch (Throwable t) {
                new ExceptionDialog(SaTScanApplication.this, t).setVisible(true);
            }
        }
    }

    /**
     * Check version action; checks whether a new version of SaTScan is available.
     * TODO: download and updated process not implemented yet.
     */
    public class CheckNewVersionAction extends AbstractAction {

        private static final long serialVersionUID = 1L;

        public CheckNewVersionAction() {
            super("Check for New Version", new ImageIcon(SaTScanApplication.this.getClass().getResource("/Update.gif")));
        }

        public void actionPerformed(ActionEvent e) {
            try {
                UpdateCheckDialog updateCheck = new UpdateCheckDialog(SaTScanApplication.this);
                updateCheck.setVisible(true);
                if (updateCheck.getRestartRequired()) {
                    if (getAnalysesRunning()) {
                        JOptionPane.showMessageDialog(SaTScanApplication.this, "SaTScan can not update will analyses are executing. " +
                                "Please cancel or wait for analyses then close SaTScan.", "Error", JOptionPane.INFORMATION_MESSAGE);
                        return;
                    }
                    //trigger windowClosing event manually ...
                    windowClosing(new WindowEvent(SaTScanApplication.this, WindowEvent.WINDOW_CLOSING));
                }
            } catch (Throwable t) {
                new ExceptionDialog(SaTScanApplication.this, t).setVisible(true);
            }
        }
    }

    /**
     * returns whether there are actively running analyses
     */
    private boolean getAnalysesRunning() {
        boolean bReturn = false;

        for (int i = 0; i < allOpenFrames.size() && !bReturn; i++) {
            if (allOpenFrames.get(i) instanceof AnalysisRunInternalFrame) {
                bReturn = !((AnalysisRunInternalFrame) allOpenFrames.get(i)).GetCanClose();
            }
        }
        return bReturn;
    }

    /**
     * Suggested citation action; displays the suggested citation dialog.
     */
    public class SuggestedCitationAction extends AbstractAction {

        private static final long serialVersionUID = 1L;

        public SuggestedCitationAction() {
            super("Suggested Citation");
        }

        public void actionPerformed(ActionEvent e) {
            try {
                URL n = SaTScanApplication.this.getClass().getResource("/suggested_citation.html");
                new SuggestedCitationDialog(SaTScanApplication.this, n).setVisible(true);
            } catch (Throwable t) {
                new ExceptionDialog(SaTScanApplication.this, t).setVisible(true);
            }
        }
    }

    /**
     * About dialog action; displays the about dialog.
     */
    public class AboutAction extends AbstractAction {

        private static final long serialVersionUID = 1L;

        public AboutAction() {
            super("About SaTScan");
        }

        public void actionPerformed(ActionEvent e) {
            try {
                AboutDialog aboutDialog = new AboutDialog(SaTScanApplication.this);
                aboutDialog.setVisible(true);
            } catch (Throwable t) {
                new ExceptionDialog(SaTScanApplication.this, t).setVisible(true);
            }
        }
    }

    /**
     * Enables/disables various actions based upon whether they are related to
     * the setting windows or the execution/results window.
     */
    private void enableActions(boolean enableSettingActions, boolean enableResultsActions) {
        _executeSessionAction.setEnabled(enableSettingActions);
        _executeOptionsAction.setEnabled(enableSettingActions);
        _closeSessionAction.setEnabled(enableSettingActions);
        _saveSessionAction.setEnabled(enableSettingActions);
        _saveSessionAsAction.setEnabled(enableSettingActions);
        _printResultsAction.setEnabled(enableResultsActions);
    }

    /** Attempt to close parameter settings windows. Returns false if any windows would
     * not close, otherwise true. */
    private boolean CloseParameterSettingsWindows() {
        //first check that there isn't an advanced parameters window open -- model problems otherwise
        for (int i = allOpenFrames.size() - 1; i >= 0; i--) {
            if (allOpenFrames.get(i) instanceof ParameterSettingsFrame) {
                if (!((ParameterSettingsFrame) allOpenFrames.get(i)).isAdvancedParametersWindowVisible()) {
                    ((ParameterSettingsFrame) allOpenFrames.get(i)).setAdvancedParametersWindowVisible(false);
                    if (!((ParameterSettingsFrame) allOpenFrames.get(i)).QueryWindowCanClose()) {
                        return false;
                    } else {
                        ((ParameterSettingsFrame) allOpenFrames.get(i)).dispose();
                    }
                }
            }
        }

        for (int i = allOpenFrames.size() - 1; i >= 0; i--) {
            if (allOpenFrames.get(i) instanceof ParameterSettingsFrame) {
                if (!((ParameterSettingsFrame) allOpenFrames.get(i)).QueryWindowCanClose()) {
                    return false;
                } else {
                    ((ParameterSettingsFrame) allOpenFrames.get(i)).dispose();
                }
            }
        }
        return true;
    }

    /** closes all running analysis child windows -- this method is primarily used when closing the
     * application and the user has indicated to close regardless of executing analyses. */
    private void CloseRunningAnalysesWindows() {
        for (int i = 0; i < allOpenFrames.size(); i++) {
            if (allOpenFrames.get(i) instanceof AnalysisRunInternalFrame) {
                ((AnalysisRunInternalFrame) allOpenFrames.get(i)).forceClose();
            }
        }
    }

    /**
     * About dialog action; displays the about dialog.
     */
    public class ReopenAction extends AbstractAction {

        private static final long serialVersionUID = 1L;
        private final File _file;

        public ReopenAction(File file) {
            super(file.getName());
            try {
                putValue(Action.NAME, file.getCanonicalPath().toString());
            } catch (IOException e) {
            }
            _file = file;
        }

        public void actionPerformed(ActionEvent e) {
            try {
                openNewParameterSessionWindow(_file.getAbsolutePath());
            } catch (Throwable t) {
                new ExceptionDialog(SaTScanApplication.this, t).setVisible(true);
            }
        }
    }

    /** refreshes 'reopen' menu item to reflect possibly updated history list */
    public void refreshOpenList() {
        _reopenSessionMenu.removeAll();
        for (int i = 0; i < ParameterHistory.getInstance().getHistoryList().size(); ++i) {
            JMenuItem item = new JMenuItem();
            item.setIcon(null);
            item.setAction(new ReopenAction(ParameterHistory.getInstance().getHistoryList().get(i)));
            _reopenSessionMenu.add(item);
        }
    }

    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc=" Generated Code ">//GEN-BEGIN:initComponents
    private void initComponents() {
        desktopPane = new javax.swing.JDesktopPane();
        _ToolBar = new javax.swing.JToolBar();
        _newSessionToolButton = new javax.swing.JButton();
        _openSessionToolButton = new javax.swing.JButton();
        _saveSessionToolButton = new javax.swing.JButton();
        _executeSessionToolButton = new javax.swing.JButton();
        _printToolButton = new javax.swing.JButton();
        _versionUpdateToolButton = new javax.swing.JButton();
        helpSystemToolButton = new javax.swing.JButton();
        menuBar = new javax.swing.JMenuBar();
        _fileMenu = new javax.swing.JMenu();
        _newSessionMenuItem = new javax.swing.JMenuItem();
        _openSessionMenuItem = new javax.swing.JMenuItem();
        _reopenSessionMenu = new javax.swing.JMenu();
        _closeSessionMenuItem = new javax.swing.JMenuItem();
        _fileMenuSeparator1 = new javax.swing.JSeparator();
        _saveSessionMenuItem = new javax.swing.JMenuItem();
        _saveSessionAsMenuItem = new javax.swing.JMenuItem();
        _fileMenuSeparator2 = new javax.swing.JSeparator();
        _printMenuItem = new javax.swing.JMenuItem();
        _fileMenuSeparator3 = new javax.swing.JSeparator();
        _exitMenuItem = new javax.swing.JMenuItem();
        _sessionMenu = new javax.swing.JMenu();
        _executeSessionMenuItem = new javax.swing.JMenuItem();
        _executeOptionsMenuItem = new javax.swing.JMenuItem();
        _helpMenu = new javax.swing.JMenu();
        _helpContentMenuItem = new javax.swing.JMenuItem();
        _userGuideMenuItem = new javax.swing.JMenuItem();
        _helpMenuSeparator1 = new javax.swing.JSeparator();
        _chechVersionMenuItem = new javax.swing.JMenuItem();
        _helpMenuSeparator2 = new javax.swing.JSeparator();
        _suggestedCitationMenuItem = new javax.swing.JMenuItem();
        _helpMenuSeparator3 = new javax.swing.JSeparator();
        _aboutMenuItem = new javax.swing.JMenuItem();

        setDefaultCloseOperation(javax.swing.WindowConstants.DO_NOTHING_ON_CLOSE);
        setLocationByPlatform(true);

        _ToolBar.setFloatable(false);
        _newSessionToolButton.setAction(new NewSessionFileAction());
        _newSessionToolButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/New.gif")));
        _newSessionToolButton.setToolTipText("New Session");
        _newSessionToolButton.setHideActionText(true);
        _ToolBar.add(_newSessionToolButton);

        _openSessionToolButton.setAction(new OpenSessionFileAction());
        _openSessionToolButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/Open.gif")));
        _openSessionToolButton.setToolTipText("Open Session");
        _openSessionToolButton.setHideActionText(true);
        _ToolBar.add(_openSessionToolButton);

        _saveSessionToolButton.setAction(_saveSessionAction);
        _saveSessionToolButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/Save.gif")));
        _saveSessionToolButton.setToolTipText("Save Session");
        _saveSessionToolButton.setHideActionText(true);
        _ToolBar.add(_saveSessionToolButton);

        _executeSessionToolButton.setAction(_executeSessionAction);
        _executeSessionToolButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/Execute.gif")));
        _executeSessionToolButton.setToolTipText("Execute Session");
        _executeSessionToolButton.setHideActionText(true);
        _ToolBar.add(_executeSessionToolButton);

        _printToolButton.setAction(_printResultsAction);
        _printToolButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/Print.gif")));
        _printToolButton.setToolTipText("Print Analysis Results");
        _printToolButton.setHideActionText(true);
        _ToolBar.add(_printToolButton);

        _versionUpdateToolButton.setAction(new CheckNewVersionAction());
        _versionUpdateToolButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/Update.gif")));
        _versionUpdateToolButton.setToolTipText("Check for New Version");
        _versionUpdateToolButton.setHideActionText(true);
        _ToolBar.add(_versionUpdateToolButton);

        helpSystemToolButton.setAction(new HelpSystemAction());
        helpSystemToolButton.setIcon(new javax.swing.ImageIcon(getClass().getResource("/Help.gif")));
        helpSystemToolButton.setToolTipText("Help");
        helpSystemToolButton.setHideActionText(true);
        _ToolBar.add(helpSystemToolButton);

        _fileMenu.setText("File");
        _newSessionMenuItem.setAction(new NewSessionFileAction());
        _newSessionMenuItem.setAccelerator(javax.swing.KeyStroke.getKeyStroke(java.awt.event.KeyEvent.VK_N, java.awt.event.InputEvent.CTRL_MASK));
        _newSessionMenuItem.setText("New Session");
        _newSessionMenuItem.setIcon(null);
        _fileMenu.add(_newSessionMenuItem);

        _openSessionMenuItem.setAction(new OpenSessionFileAction());
        _openSessionMenuItem.setAccelerator(javax.swing.KeyStroke.getKeyStroke(java.awt.event.KeyEvent.VK_O, java.awt.event.InputEvent.CTRL_MASK));
        _openSessionMenuItem.setText("Open Session File");
        _openSessionMenuItem.setIcon(null);
        _fileMenu.add(_openSessionMenuItem);

        _reopenSessionMenu.setText("Reopen Session File");
        _reopenSessionMenu.setIcon(null);
        _fileMenu.add(_reopenSessionMenu);

        _closeSessionMenuItem.setAction(_closeSessionAction);
        _closeSessionMenuItem.setIcon(null);
        _fileMenu.add(_closeSessionMenuItem);

        _fileMenu.add(_fileMenuSeparator1);

        _saveSessionMenuItem.setAction(_saveSessionAction);
        _saveSessionMenuItem.setIcon(null);
        _fileMenu.add(_saveSessionMenuItem);

        _saveSessionAsMenuItem.setAction(_saveSessionAsAction);
        _saveSessionAsMenuItem.setIcon(null);
        _fileMenu.add(_saveSessionAsMenuItem);

        _fileMenu.add(_fileMenuSeparator2);

        _printMenuItem.setAction(_printResultsAction);
        _printMenuItem.setIcon(null);
        _fileMenu.add(_printMenuItem);

        _fileMenu.add(_fileMenuSeparator3);

        _exitMenuItem.setAction(new ExitAction());
        _exitMenuItem.setText("Exit");
        _exitMenuItem.setIcon(null);
        _fileMenu.add(_exitMenuItem);

        menuBar.add(_fileMenu);

        _sessionMenu.setText("Session");
        _executeSessionMenuItem.setAction(_executeSessionAction);
        _executeSessionMenuItem.setIcon(null);
        _sessionMenu.add(_executeSessionMenuItem);

        _executeOptionsMenuItem.setAction(_executeOptionsAction);
        _executeOptionsMenuItem.setIcon(null);
        _sessionMenu.add(_executeOptionsMenuItem);

        menuBar.add(_sessionMenu);

        _helpMenu.setText("Help");
        _helpContentMenuItem.setAction(new HelpSystemAction());
        _helpContentMenuItem.setAccelerator(javax.swing.KeyStroke.getKeyStroke(java.awt.event.KeyEvent.VK_F1, 0));
        _helpContentMenuItem.setText("Help Contents");
        _helpContentMenuItem.setIcon(null);
        _helpMenu.add(_helpContentMenuItem);

        _userGuideMenuItem.setAction(new UserGuideAction());
        _userGuideMenuItem.setText("User Guide");
        _userGuideMenuItem.setIcon(null);
        _helpMenu.add(_userGuideMenuItem);

        _helpMenu.add(_helpMenuSeparator1);

        _chechVersionMenuItem.setAction(new CheckNewVersionAction());
        _chechVersionMenuItem.setText("Check for New Version");
        _chechVersionMenuItem.setIcon(null);
        _helpMenu.add(_chechVersionMenuItem);

        _helpMenu.add(_helpMenuSeparator2);

        _suggestedCitationMenuItem.setAction(new SuggestedCitationAction());
        _suggestedCitationMenuItem.setText("Suggested Citation");
        _suggestedCitationMenuItem.setIcon(null);
        _helpMenu.add(_suggestedCitationMenuItem);

        _helpMenu.add(_helpMenuSeparator3);

        _aboutMenuItem.setAction(new AboutAction());
        _aboutMenuItem.setText("About SaTScan");
        _aboutMenuItem.setIcon(null);
        _helpMenu.add(_aboutMenuItem);

        menuBar.add(_helpMenu);

        setJMenuBar(menuBar);

        javax.swing.GroupLayout layout = new javax.swing.GroupLayout(getContentPane());
        getContentPane().setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addComponent(_ToolBar, javax.swing.GroupLayout.DEFAULT_SIZE, 768, Short.MAX_VALUE)
            .addComponent(desktopPane, javax.swing.GroupLayout.DEFAULT_SIZE, 768, Short.MAX_VALUE)
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(layout.createSequentialGroup()
                .addComponent(_ToolBar, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(desktopPane, javax.swing.GroupLayout.DEFAULT_SIZE, 555, Short.MAX_VALUE)
                .addContainerGap())
        );
        pack();
    }// </editor-fold>//GEN-END:initComponents
    /**
     * @param args the command line arguments
     */
    public static void main(String args[]) {
        java.awt.EventQueue.invokeLater(new Runnable() {

            public void run() {
                try {
                    UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
                } catch (Exception e) {
                }
                new SaTScanApplication().setVisible(true);
            }
        });
    }

    /**
     * When the main window first shows, displays the start window.
     */
    public void windowGainedFocus(WindowEvent e) {
        if (gbShowStartWindow) {
            try {
                StartDialog startDialog = new StartDialog(SaTScanApplication.this);
                startDialog.setVisible(true);
                gbShowStartWindow = false;
                switch (startDialog.GetOpenType()) {
                    case NEW:
                        openNewParameterSessionWindow("");
                        break;
                    case SAVED:
                        openParameterSessionWindow();
                        break;
                    case LAST:
                        if (FileAccess.ValidateFileAccess(ParameterHistory.getInstance().getHistoryList().get(0).getAbsolutePath(), false)) {
                            openNewParameterSessionWindow(ParameterHistory.getInstance().getHistoryList().get(0).getAbsolutePath());
                        }
                        break;
                    case CANCEL:
                    default:
                        break;
                }
            } catch (Throwable t) {
                new ExceptionDialog(SaTScanApplication.this, t).setVisible(true);
            }
        }
    }

    public void windowLostFocus(WindowEvent e) {
    }

    public void windowOpened(WindowEvent e) {
    }

    /**
     * form close event -- checks whether there are actively running analyses and
     * prompts user as to whether to continue closing accordingly. The ForceClose()
     * method is used to ensure that all child windows will close.
     */
    public void windowClosing(WindowEvent e) {
        if (getAnalysesRunning() &&
                JOptionPane.showConfirmDialog(this, "There are analyses currently executing. Are you sure you want to exit SaTScan?", "Warning", JOptionPane.YES_NO_OPTION) == JOptionPane.NO_OPTION) {
            return;
        }
        if (!CloseParameterSettingsWindows()) {
            return;
        }
        CloseRunningAnalysesWindows();
        if (UpdateCheckDialog._runUpdateOnTerminate) {
            try {
                //launch updater application and close
                String[] commandline = new String[]{"java", "-jar", UpdateCheckDialog._updaterFilename.getName(), UpdateCheckDialog._updateArchiveName.getName(), _application};
                Runtime.getRuntime().exec(commandline, null, UpdateCheckDialog.getDownloadTempDirectory());
            } catch (IOException ex) {
                Logger.getLogger(SaTScanApplication.class.getName()).log(Level.SEVERE, null, ex);
            }
        }
        this.dispose();
        System.exit(0);
    }

    public void windowClosed(WindowEvent e) {
    }

    public void windowIconified(WindowEvent e) {
    }

    public void windowDeiconified(WindowEvent e) {
    }

    public void windowActivated(WindowEvent e) {
    }

    public void windowDeactivated(WindowEvent e) {
    }

    /**
     * Adds frame to collection of internal frames.
     */
    public void internalFrameOpened(InternalFrameEvent e) {
        allOpenFrames.addElement(e.getInternalFrame());
    }

    public void internalFrameClosing(InternalFrameEvent e) {
    }

    /**
     * Responds to the start window closing event by invoking user response.
     */
    public void internalFrameClosed(InternalFrameEvent e) {
        allOpenFrames.removeElement(e.getInternalFrame());
        if (e.getInternalFrame() instanceof ParameterSettingsFrame) {
            refreshOpenList();
        }
    }

    public void internalFrameIconified(InternalFrameEvent e) {
    }

    public void internalFrameDeiconified(InternalFrameEvent e) {
    }

    /**
     * Responds to the activation of an internal frame; enabling various actions.
     * The focused internal frame is noted for reference.
     */
    public void internalFrameActivated(InternalFrameEvent e) {
        _focusedInternalFrame = e.getInternalFrame();
        enableActions((_focusedInternalFrame instanceof ParameterSettingsFrame), (_focusedInternalFrame instanceof AnalysisRunInternalFrame));
    }

    /**
     * Responds to the activation of an internal frame; enabling various actions.
     */
    public void internalFrameDeactivated(InternalFrameEvent e) {
        if (_focusedInternalFrame == e.getInternalFrame()) {
            _focusedInternalFrame = null;
        }
        enableActions(false, false);
    }
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JToolBar _ToolBar;
    private javax.swing.JMenuItem _aboutMenuItem;
    private javax.swing.JMenuItem _chechVersionMenuItem;
    private javax.swing.JMenuItem _closeSessionMenuItem;
    private javax.swing.JMenuItem _executeOptionsMenuItem;
    private javax.swing.JMenuItem _executeSessionMenuItem;
    private javax.swing.JButton _executeSessionToolButton;
    private javax.swing.JMenuItem _exitMenuItem;
    private javax.swing.JMenu _fileMenu;
    private javax.swing.JSeparator _fileMenuSeparator1;
    private javax.swing.JSeparator _fileMenuSeparator2;
    private javax.swing.JSeparator _fileMenuSeparator3;
    private javax.swing.JMenuItem _helpContentMenuItem;
    private javax.swing.JMenu _helpMenu;
    private javax.swing.JSeparator _helpMenuSeparator1;
    private javax.swing.JSeparator _helpMenuSeparator2;
    private javax.swing.JSeparator _helpMenuSeparator3;
    private javax.swing.JMenuItem _newSessionMenuItem;
    private javax.swing.JButton _newSessionToolButton;
    private javax.swing.JMenuItem _openSessionMenuItem;
    private javax.swing.JButton _openSessionToolButton;
    private javax.swing.JMenuItem _printMenuItem;
    private javax.swing.JButton _printToolButton;
    private javax.swing.JMenu _reopenSessionMenu;
    private javax.swing.JMenuItem _saveSessionAsMenuItem;
    private javax.swing.JMenuItem _saveSessionMenuItem;
    private javax.swing.JButton _saveSessionToolButton;
    private javax.swing.JMenu _sessionMenu;
    private javax.swing.JMenuItem _suggestedCitationMenuItem;
    private javax.swing.JMenuItem _userGuideMenuItem;
    private javax.swing.JButton _versionUpdateToolButton;
    private javax.swing.JDesktopPane desktopPane;
    private javax.swing.JButton helpSystemToolButton;
    private javax.swing.JMenuBar menuBar;
    // End of variables declaration//GEN-END:variables
}
