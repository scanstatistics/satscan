package org.satscan.gui;

import java.awt.Color;
import java.awt.Font;
import java.io.File;
import java.io.FileNotFoundException;
import java.io.FileReader;
import java.io.FileWriter;
import java.io.IOException;
import java.io.Reader;
import java.nio.file.Files;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;
import java.util.GregorianCalendar;
import java.util.HashMap;
import java.util.Locale;
import java.util.Map;
import java.util.prefs.Preferences;
import java.util.Properties;
import org.apache.commons.lang3.SystemUtils;
import org.satscan.gui.utils.TextPrompt;
import org.satscan.gui.utils.WaitCursor;

/**
 * @author  Hostovic
 */
public class ApplicationPreferences extends javax.swing.JDialog {
    private static final String CHECK_FREQUENCY_KEY = "check-frequency";
    private static final String CHECK_DATE_KEY = "check-date";
    private static final String KEY_DATE_FORAMT = "yyyy-MM-dd";
    private static final String CHECK_WEEKLY_KEY = "Once a week";
    private static final String CHECK_MONTHLY_KEY = "Once a month";
    private static final String CHECK_EVERYTIME_KEY = "Each time SaTScan starts";
    private static final String CHECK_MANUALLY_KEY = "Manually Only";
    private static final String MAILSERVER_SECTION = "MailServer";
    private static final String MAIL_SERVERNAME_KEY = "mail-servername";
    private static final String MAIL_ADDITIONAL_KEY = "mail-additional";
    private static final String MAIL_FROM_KEY = "mail-from";
    private static final String MAIL_REPLY_KEY = "mail-reply";
    private static String _ini_satscan;
    private Map<String, Properties> _ini_values = new HashMap();

    public native String SendTestMail(String servername, String additional, String from, String reply);    
    
    /** Creates new form ExecutionOptionsDialog */
    public ApplicationPreferences(java.awt.Frame parent) throws FileNotFoundException, IOException {
        super(parent, true);
        initComponents();
        _checkFrequency.setModel(new javax.swing.DefaultComboBoxModel(updateFrequencyChoices()));
        _checkFrequency.setSelectedItem(getUpdateFrequency());
        _file_browse_options.setEnabled(!System.getProperty("os.name").toLowerCase().startsWith("mac"));
        _alternative_browsing.setEnabled(!System.getProperty("os.name").toLowerCase().startsWith("mac"));
        if (_alternative_browsing.isEnabled()) _alternative_browsing.setSelected(SaTScanApplication.getAwtBrowse());        
        setLocationRelativeTo(parent);
        // Read mail server settings from SaTScan ini file - found in user directory.
        _ini_satscan = SystemUtils.getUserHome().getAbsolutePath() + File.separator + ".satscan" + File.separator   + "satscan.ini";
        File file = new File(_ini_satscan);
        if (Files.exists(file.toPath())) {
            _ini_values = parseINI(new FileReader(file));
            for (Map.Entry<String, Properties> entry : _ini_values.entrySet()) {
                if (entry.getKey().equalsIgnoreCase(MAILSERVER_SECTION)) {
                    for (Map.Entry<Object, Object> e : entry.getValue().entrySet()) {
                        if (((String)e.getKey()).equals(MAIL_SERVERNAME_KEY)) {
                            _alerts_smtp.setText((String)e.getValue());
                        } else if (((String)e.getKey()).equals(MAIL_ADDITIONAL_KEY)) {
                            _alerts_curl_additional.setText((String)e.getValue());
                        } else if (((String)e.getKey()).equals(MAIL_FROM_KEY)) {
                            _alerts_from.setText((String)e.getValue());
                        } else if (((String)e.getKey()).equals(MAIL_REPLY_KEY)) {
                            _alerts_reply.setText((String)e.getValue());
                        }
                    }
                }
            }
        }
        enableSendTextMail();
    }

    /* Returns whether user has specified minimum mail server information. */
    public static boolean minimumMailServerDefined() throws FileNotFoundException, IOException {
        boolean hasServerName=false, hasFrom=false;
        String ini_satscan = SystemUtils.getUserHome().getAbsolutePath() + File.separator + ".satscan" + File.separator   + "satscan.ini";
        File file = new File(ini_satscan);
        if (Files.exists(file.toPath())) {
            Map<String, Properties> ini_values = parseINI(new FileReader(file));
            for (Map.Entry<String, Properties> entry : ini_values.entrySet()) {
                if (entry.getKey().equalsIgnoreCase(MAILSERVER_SECTION)) {
                    for (Map.Entry<Object, Object> e : entry.getValue().entrySet()) {
                        if (((String)e.getKey()).equals(MAIL_SERVERNAME_KEY)) {
                            hasServerName = !((String)e.getValue()).isEmpty();
                        } else if (((String)e.getKey()).equals(MAIL_FROM_KEY)) {
                            hasFrom = !((String)e.getValue()).isEmpty();
                        }
                    }
                }
            }
        }
        return hasServerName && hasFrom;
    }
    
    /* Concise ini file reader. */
    public static Map<String, Properties> parseINI(Reader reader) throws IOException {
        Map<String, Properties> result = new HashMap();
        new Properties() {
            private Properties section;
            
            @Override
            public Object put(Object key, Object value) {
                String header = (((String) key) + " " + value).trim();
                if (header.startsWith("[") && header.endsWith("]"))
                    return result.put(header.substring(1, header.length() - 1), section = new Properties());
                else
                    return section.put(key, value);
            }

        }.load(reader);
        return result;
    }      
    
    /*
     * Returns application update frequency choices
     */
    public static String[] updateFrequencyChoices() {
        return new String[] { CHECK_EVERYTIME_KEY, CHECK_WEEKLY_KEY, CHECK_MONTHLY_KEY, CHECK_MANUALLY_KEY };
    }
    
    /*
     * Sets the application update frequency setting. 
     */
    public static void setUpdateFrequency(String choice) {
        Preferences prefs = Preferences.userNodeForPackage(SaTScanApplication.class);
        prefs.put(CHECK_FREQUENCY_KEY, choice);        
    }
    
    /*
     * Sets the application update frequency setting. 
     */
    public static String getUpdateFrequency() {
        Preferences prefs = Preferences.userNodeForPackage(SaTScanApplication.class);
        return prefs.get(CHECK_FREQUENCY_KEY, CHECK_MONTHLY_KEY);
    }
   
    /*
     * Sets the type of file selection dialog to use.
     */
    public void setAlternativeBrowsing(boolean selected) {
        if (_alternative_browsing.isEnabled()) {
            setPreference(SaTScanApplication.FILE_BROWSE_KEY,Boolean.toString(selected));
            SaTScanApplication.setAwtBrowse(selected);
        }        
    }    
    
    public static String getPreference(String key) {
        Preferences prefs = Preferences.userNodeForPackage(SaTScanApplication.class);
        return prefs.get(key, "");
    }    
    
    public static void setPreference(String key, String value) {
        Preferences prefs = Preferences.userNodeForPackage(SaTScanApplication.class);
        prefs.put(key, value);        
    }    
    
    /*
     * Sets last time checked as now.
     */
    public static Date lastCheckedNow() {
        SimpleDateFormat sdf = new SimpleDateFormat(KEY_DATE_FORAMT, Locale.ENGLISH);
        Date date = new Date();
        Preferences.userNodeForPackage(SaTScanApplication.class).put(CHECK_DATE_KEY, sdf.format(date));
        return date;
    }
    
    /*
     * Returns whether user preferences indicate that we should check for preferences now.
     */
    public static boolean shouldCheckUpdate() {
        try {
            Preferences prefs = Preferences.userNodeForPackage(SaTScanApplication.class);
            String frequency = prefs.get(CHECK_FREQUENCY_KEY, CHECK_WEEKLY_KEY);
            if (frequency.equalsIgnoreCase(CHECK_MANUALLY_KEY)) {
                return false;
            } else if (frequency.equalsIgnoreCase(CHECK_EVERYTIME_KEY)) {
                    return true;
            } else {
                try {
                    Date lastCheck = new SimpleDateFormat(KEY_DATE_FORAMT, Locale.ENGLISH).parse(prefs.get(CHECK_DATE_KEY, ""));
                    Calendar calendar = new GregorianCalendar();
                    if (frequency.equalsIgnoreCase(CHECK_MONTHLY_KEY)) {
                        calendar.add(Calendar.MONTH, -1);
                    } else { // default CHECK_WEEKLY_KEY
                        calendar.add(Calendar.DAY_OF_MONTH, -7);
                    }       
                    return lastCheck.before(calendar.getTime());
                } catch (ParseException ex) { // last check not set or bad string
                    return true;
                }
            }
        } finally {
            lastCheckedNow();
        }
    }
    
    private void enableSendTextMail() {
        _send_test_mail.setEnabled(
           !_alerts_smtp.getText().isEmpty() && !_alerts_from.getText().isEmpty()
        );
    }
    
    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

        cancelButton = new javax.swing.JButton();
        okButton = new javax.swing.JButton();
        jTabbedPane1 = new javax.swing.JTabbedPane();
        jPanel2 = new javax.swing.JPanel();
        parallelProcessorsGroup = new javax.swing.JPanel();
        _checkFrequency = new javax.swing.JComboBox();
        jLabel1 = new javax.swing.JLabel();
        _alternative_browsing = new javax.swing.JCheckBox();
        jScrollPane2 = new javax.swing.JScrollPane();
        _file_browse_options = new javax.swing.JTextPane();
        jPanel3 = new javax.swing.JPanel();
        _alert_label_smtp = new javax.swing.JLabel();
        _alerts_smtp = new javax.swing.JTextField();
        TextPrompt tpSMTP = new TextPrompt("ex. smtp.somecompany.com:25", _alerts_smtp);
        tpSMTP.setForeground( Color.BLUE );
        tpSMTP.changeAlpha(0.5f);
        tpSMTP.changeStyle(Font.BOLD + Font.ITALIC);
        _alert_label_add_curl = new javax.swing.JLabel();
        _alerts_curl_additional = new javax.swing.JTextField();
        TextPrompt tpAddCurl = new TextPrompt("ex. --user <user:password> --ssl-reqd", _alerts_curl_additional);
        tpAddCurl.setForeground( Color.BLUE );
        tpAddCurl.changeAlpha(0.5f);
        tpAddCurl.changeStyle(Font.BOLD + Font.ITALIC);
        _alert_label_from = new javax.swing.JLabel();
        _alerts_from = new javax.swing.JTextField();
        TextPrompt tpFrom = new TextPrompt("ex. sender@domain.com", _alerts_from); 
        tpFrom.setForeground( Color.BLUE );
        tpFrom.changeAlpha(0.5f);
        tpFrom.changeStyle(Font.BOLD + Font.ITALIC);
        _alert_label_reply = new javax.swing.JLabel();
        _alerts_reply = new javax.swing.JTextField();
        TextPrompt tpReply = new TextPrompt("ex. sender@domain.com", _alerts_reply); 
        tpReply.setForeground( Color.BLUE );
        tpReply.changeAlpha(0.5f);
        tpReply.changeStyle(Font.BOLD + Font.ITALIC);
        _send_test_mail = new javax.swing.JButton();
        _send_test_mail.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                WaitCursor waitCursor = new WaitCursor(ApplicationPreferences.this);
                try {
                    _send_test_mail_response.setText("");
                    String response = SendTestMail(
                        _alerts_smtp.getText(), _alerts_curl_additional.getText(),
                        _alerts_from.getText(), _alerts_reply.getText()
                    );
                    _send_test_mail_response.append(response);
                    _send_test_mail_response.setCaretPosition(0);
                } catch (Throwable t) {
                    new ExceptionDialog(ApplicationPreferences.this, t).setVisible(true);
                } finally {
                    waitCursor.restore();
                }
            }
        });
        jScrollPane1 = new javax.swing.JScrollPane();
        _send_test_mail_response = new javax.swing.JTextArea();

        setTitle("Preferences and Settings");
        setModal(true);

        cancelButton.setText("Cancel");
        cancelButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                setVisible(false);
            }
        });

        okButton.setText("Ok");
        okButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent event) {
                WaitCursor waitCursor = new WaitCursor(ApplicationPreferences.this);
                try {
                    setUpdateFrequency(_checkFrequency.getSelectedItem().toString());
                    setAlternativeBrowsing(_alternative_browsing.isSelected());
                    FileWriter fw = new FileWriter(_ini_satscan);
                    // Write already defined section - other than that of mail server.
                    for (Map.Entry<String, Properties> entry : _ini_values.entrySet()) {
                        String sectionName = "[" + entry.getKey() + "]";
                        if (!entry.getKey().equals(MAILSERVER_SECTION)) {
                            fw.write(sectionName + System.lineSeparator());
                            for (Map.Entry<Object, Object> e : entry.getValue().entrySet()) {
                                fw.write((String)e.getKey() + "=" + (String)e.getValue() + System.lineSeparator());
                            }
                            fw.write(System.lineSeparator());
                        }
                    }
                    // Now write the mail server section.
                    fw.write("[" + MAILSERVER_SECTION + "]" + System.lineSeparator());
                    fw.write(MAIL_SERVERNAME_KEY + "=" + _alerts_smtp.getText() + System.lineSeparator());
                    fw.write(MAIL_ADDITIONAL_KEY + "=" + _alerts_curl_additional.getText() + System.lineSeparator());
                    fw.write(MAIL_FROM_KEY + "=" + _alerts_from.getText() + System.lineSeparator());
                    fw.write(MAIL_REPLY_KEY + "=" + _alerts_reply.getText() + System.lineSeparator());
                    fw.write(System.lineSeparator());
                    fw.close();
                    setVisible(false);
                } catch (Throwable t) {
                    new ExceptionDialog(ApplicationPreferences.this, t).setVisible(true);
                } finally {
                    waitCursor.restore();
                }
            }
        });

        parallelProcessorsGroup.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createEtchedBorder(), "Software Updates"));

        jLabel1.setLabelFor(_checkFrequency);
        jLabel1.setText("Automatically check for updates:");

        javax.swing.GroupLayout parallelProcessorsGroupLayout = new javax.swing.GroupLayout(parallelProcessorsGroup);
        parallelProcessorsGroup.setLayout(parallelProcessorsGroupLayout);
        parallelProcessorsGroupLayout.setHorizontalGroup(
            parallelProcessorsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(parallelProcessorsGroupLayout.createSequentialGroup()
                .addContainerGap()
                .addGroup(parallelProcessorsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(_checkFrequency, 0, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(jLabel1, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                .addContainerGap())
        );
        parallelProcessorsGroupLayout.setVerticalGroup(
            parallelProcessorsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(parallelProcessorsGroupLayout.createSequentialGroup()
                .addComponent(jLabel1)
                .addGap(11, 11, 11)
                .addComponent(_checkFrequency, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addGap(0, 9, Short.MAX_VALUE))
        );

        _alternative_browsing.setText("Use Alternative File Browsing Dialog");

        jScrollPane2.setBorder(null);
        jScrollPane2.setHorizontalScrollBarPolicy(javax.swing.ScrollPaneConstants.HORIZONTAL_SCROLLBAR_NEVER);

        _file_browse_options.setEditable(false);
        _file_browse_options.setBackground(new java.awt.Color(240, 240, 240));
        _file_browse_options.setBorder(null);
        _file_browse_options.setText("In some instances the dialog used to browse for parameter, input, and output files takes an excessive amount of time to browse the file system. Selecting the above option causes this application to use an alternative dialog which should help resolve the excessive file browsing time, although providing a slightly less rich experience.");
        _file_browse_options.setMargin(new java.awt.Insets(10, 10, 10, 10));
        jScrollPane2.setViewportView(_file_browse_options);
        _file_browse_options.getAccessibleContext().setAccessibleName("");

        javax.swing.GroupLayout jPanel2Layout = new javax.swing.GroupLayout(jPanel2);
        jPanel2.setLayout(jPanel2Layout);
        jPanel2Layout.setHorizontalGroup(
            jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel2Layout.createSequentialGroup()
                .addGroup(jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(parallelProcessorsGroup, javax.swing.GroupLayout.Alignment.TRAILING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addGroup(jPanel2Layout.createSequentialGroup()
                        .addContainerGap()
                        .addGroup(jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addComponent(jScrollPane2, javax.swing.GroupLayout.PREFERRED_SIZE, 0, Short.MAX_VALUE)
                            .addComponent(_alternative_browsing, javax.swing.GroupLayout.DEFAULT_SIZE, 355, Short.MAX_VALUE))))
                .addContainerGap())
        );
        jPanel2Layout.setVerticalGroup(
            jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel2Layout.createSequentialGroup()
                .addContainerGap()
                .addComponent(parallelProcessorsGroup, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_alternative_browsing)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addComponent(jScrollPane2, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap(94, Short.MAX_VALUE))
        );

        _alternative_browsing.getAccessibleContext().setAccessibleDescription("In some instances the dialog used to browse for parameter, input, and output files takes an excessive amount of time to browse the file system. Selecting the above option causes this application to use an alternative dialog which should help resolve the excessive file browsing time, although providing a slightly less rich experience.");

        jTabbedPane1.addTab("General", jPanel2);
        jPanel2.getAccessibleContext().setAccessibleName("General Preferences tab");

        _alert_label_smtp.setLabelFor(_alerts_smtp);
        _alert_label_smtp.setText("Server Name:");

        _alerts_smtp.addFocusListener(new java.awt.event.FocusAdapter() {
            public void focusLost(java.awt.event.FocusEvent e) {
                enableSendTextMail();
            }
        });

        _alert_label_add_curl.setLabelFor(_alerts_curl_additional);
        _alert_label_add_curl.setText("Additional curl Parameters:");

        _alert_label_from.setLabelFor(_alert_label_from);
        _alert_label_from.setText("'From' Address:");

        _alerts_from.addFocusListener(new java.awt.event.FocusAdapter() {
            public void focusLost(java.awt.event.FocusEvent e) {
                enableSendTextMail();
            }
        });

        _alert_label_reply.setLabelFor(_alerts_reply);
        _alert_label_reply.setText("'Reply' Address:");

        _send_test_mail.setText("Send Test Email");
        _send_test_mail.setToolTipText("Send test email to 'from' address");

        _send_test_mail_response.setEditable(false);
        _send_test_mail_response.setColumns(20);
        _send_test_mail_response.setRows(5);
        jScrollPane1.setViewportView(_send_test_mail_response);
        _send_test_mail_response.getAccessibleContext().setAccessibleName("Send test mail response. ");

        javax.swing.GroupLayout jPanel3Layout = new javax.swing.GroupLayout(jPanel3);
        jPanel3.setLayout(jPanel3Layout);
        jPanel3Layout.setHorizontalGroup(
            jPanel3Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel3Layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(jPanel3Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(jPanel3Layout.createSequentialGroup()
                        .addComponent(_send_test_mail, javax.swing.GroupLayout.PREFERRED_SIZE, 193, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addGap(0, 162, Short.MAX_VALUE))
                    .addComponent(_alerts_smtp)
                    .addComponent(_alert_label_smtp, javax.swing.GroupLayout.Alignment.TRAILING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(_alerts_curl_additional)
                    .addComponent(_alert_label_add_curl, javax.swing.GroupLayout.Alignment.TRAILING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(_alerts_from)
                    .addComponent(_alert_label_from, javax.swing.GroupLayout.Alignment.TRAILING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(jScrollPane1)
                    .addComponent(_alerts_reply)
                    .addComponent(_alert_label_reply, javax.swing.GroupLayout.Alignment.TRAILING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                .addContainerGap())
        );
        jPanel3Layout.setVerticalGroup(
            jPanel3Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, jPanel3Layout.createSequentialGroup()
                .addComponent(_alert_label_smtp)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_alerts_smtp, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_alert_label_add_curl)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_alerts_curl_additional, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_alert_label_from)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_alerts_from, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_alert_label_reply)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_alerts_reply, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_send_test_mail)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(jScrollPane1, javax.swing.GroupLayout.DEFAULT_SIZE, 61, Short.MAX_VALUE)
                .addContainerGap())
        );

        _alerts_smtp.getAccessibleContext().setAccessibleName("mail server name");
        _alerts_curl_additional.getAccessibleContext().setAccessibleName("additional curl parameters");
        _alerts_from.getAccessibleContext().setAccessibleName("mail 'from' address");
        _alerts_reply.getAccessibleContext().setAccessibleName("mail 'reply' address");

        jTabbedPane1.addTab("Mail Server Settings", jPanel3);
        jPanel3.getAccessibleContext().setAccessibleName("Mail Server Settings tab");

        javax.swing.GroupLayout layout = new javax.swing.GroupLayout(getContentPane());
        getContentPane().setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(layout.createSequentialGroup()
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .addComponent(okButton, javax.swing.GroupLayout.PREFERRED_SIZE, 65, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(cancelButton)
                .addContainerGap())
            .addComponent(jTabbedPane1)
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(layout.createSequentialGroup()
                .addComponent(jTabbedPane1)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(cancelButton)
                    .addComponent(okButton))
                .addContainerGap())
        );

        pack();
    }// </editor-fold>//GEN-END:initComponents

    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JLabel _alert_label_add_curl;
    private javax.swing.JLabel _alert_label_from;
    private javax.swing.JLabel _alert_label_reply;
    private javax.swing.JLabel _alert_label_smtp;
    private javax.swing.JTextField _alerts_curl_additional;
    private javax.swing.JTextField _alerts_from;
    private javax.swing.JTextField _alerts_reply;
    private javax.swing.JTextField _alerts_smtp;
    private javax.swing.JCheckBox _alternative_browsing;
    private javax.swing.JComboBox _checkFrequency;
    private javax.swing.JTextPane _file_browse_options;
    private javax.swing.JButton _send_test_mail;
    private javax.swing.JTextArea _send_test_mail_response;
    private javax.swing.JButton cancelButton;
    private javax.swing.JLabel jLabel1;
    private javax.swing.JPanel jPanel2;
    private javax.swing.JPanel jPanel3;
    private javax.swing.JScrollPane jScrollPane1;
    private javax.swing.JScrollPane jScrollPane2;
    private javax.swing.JTabbedPane jTabbedPane1;
    private javax.swing.JButton okButton;
    private javax.swing.JPanel parallelProcessorsGroup;
    // End of variables declaration//GEN-END:variables
}
