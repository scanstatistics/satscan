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
import java.util.logging.Level;
import java.util.logging.Logger;
import java.util.prefs.Preferences;
import java.util.Properties;
import javax.swing.JOptionPane;
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

        parallelProcessorsGroup = new javax.swing.JPanel();
        _checkFrequency = new javax.swing.JComboBox();
        jLabel1 = new javax.swing.JLabel();
        cancelButton = new javax.swing.JButton();
        okButton = new javax.swing.JButton();
        jPanel1 = new javax.swing.JPanel();
        _alert_label_smtp = new javax.swing.JLabel();
        _alerts_smtp = new javax.swing.JTextField();
        TextPrompt tpSMTP = new TextPrompt("ex. smtp.somecompany.com:25", _alerts_smtp);
        tpSMTP.setForeground( Color.BLUE );
        tpSMTP.changeAlpha(0.5f);
        tpSMTP.changeStyle(Font.BOLD + Font.ITALIC);
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
        _alert_label_add_curl = new javax.swing.JLabel();
        _alerts_curl_additional = new javax.swing.JTextField();
        TextPrompt tpAddCurl = new TextPrompt("ex. --user <user:password> --ssl-reqd", _alerts_curl_additional);
        tpAddCurl.setForeground( Color.BLUE );
        tpAddCurl.changeAlpha(0.5f);
        tpAddCurl.changeStyle(Font.BOLD + Font.ITALIC);
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

        parallelProcessorsGroup.setBorder(javax.swing.BorderFactory.createTitledBorder(javax.swing.BorderFactory.createEtchedBorder(), "Software Updates"));

        jLabel1.setText("Automatically check for updates:");

        javax.swing.GroupLayout parallelProcessorsGroupLayout = new javax.swing.GroupLayout(parallelProcessorsGroup);
        parallelProcessorsGroup.setLayout(parallelProcessorsGroupLayout);
        parallelProcessorsGroupLayout.setHorizontalGroup(
            parallelProcessorsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(parallelProcessorsGroupLayout.createSequentialGroup()
                .addContainerGap()
                .addGroup(parallelProcessorsGroupLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(_checkFrequency, 0, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(jLabel1, javax.swing.GroupLayout.DEFAULT_SIZE, 415, Short.MAX_VALUE))
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

        jPanel1.setBorder(javax.swing.BorderFactory.createTitledBorder("Mail Server Settings"));

        _alert_label_smtp.setText("Server Name:");

        _alerts_smtp.addFocusListener(new java.awt.event.FocusAdapter() {
            public void focusLost(java.awt.event.FocusEvent e) {
                enableSendTextMail();
            }
        });

        _alert_label_from.setText("'From' Address:");

        _alerts_from.addFocusListener(new java.awt.event.FocusAdapter() {
            public void focusLost(java.awt.event.FocusEvent e) {
                enableSendTextMail();
            }
        });

        _alert_label_reply.setText("'Reply' Address:");

        _alert_label_add_curl.setText("Additional curl Parameters:");

        _send_test_mail.setText("Send Test Email");
        _send_test_mail.setToolTipText("Send test email to 'from' address");

        _send_test_mail_response.setEditable(false);
        _send_test_mail_response.setColumns(20);
        _send_test_mail_response.setRows(5);
        jScrollPane1.setViewportView(_send_test_mail_response);

        javax.swing.GroupLayout jPanel1Layout = new javax.swing.GroupLayout(jPanel1);
        jPanel1.setLayout(jPanel1Layout);
        jPanel1Layout.setHorizontalGroup(
            jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel1Layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(_alert_label_from, javax.swing.GroupLayout.Alignment.TRAILING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(_alerts_curl_additional)
                    .addComponent(_alert_label_add_curl, javax.swing.GroupLayout.DEFAULT_SIZE, 415, Short.MAX_VALUE)
                    .addComponent(_alert_label_reply, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(_alerts_reply, javax.swing.GroupLayout.Alignment.TRAILING)
                    .addComponent(_alert_label_smtp, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(_alerts_smtp)
                    .addComponent(_alerts_from)
                    .addComponent(jScrollPane1)
                    .addGroup(jPanel1Layout.createSequentialGroup()
                        .addComponent(_send_test_mail, javax.swing.GroupLayout.PREFERRED_SIZE, 193, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addGap(0, 0, Short.MAX_VALUE)))
                .addContainerGap())
        );
        jPanel1Layout.setVerticalGroup(
            jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel1Layout.createSequentialGroup()
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
                .addComponent(jScrollPane1)
                .addContainerGap())
        );

        javax.swing.GroupLayout layout = new javax.swing.GroupLayout(getContentPane());
        getContentPane().setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(parallelProcessorsGroup, javax.swing.GroupLayout.Alignment.TRAILING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, layout.createSequentialGroup()
                        .addGap(0, 0, Short.MAX_VALUE)
                        .addComponent(okButton, javax.swing.GroupLayout.PREFERRED_SIZE, 65, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(cancelButton))
                    .addComponent(jPanel1, javax.swing.GroupLayout.Alignment.TRAILING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                .addContainerGap())
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(layout.createSequentialGroup()
                .addContainerGap()
                .addComponent(parallelProcessorsGroup, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(jPanel1, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .addGap(18, 18, 18)
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
    private javax.swing.JComboBox _checkFrequency;
    private javax.swing.JButton _send_test_mail;
    private javax.swing.JTextArea _send_test_mail_response;
    private javax.swing.JButton cancelButton;
    private javax.swing.JLabel jLabel1;
    private javax.swing.JPanel jPanel1;
    private javax.swing.JScrollPane jScrollPane1;
    private javax.swing.JButton okButton;
    private javax.swing.JPanel parallelProcessorsGroup;
    // End of variables declaration//GEN-END:variables
}
