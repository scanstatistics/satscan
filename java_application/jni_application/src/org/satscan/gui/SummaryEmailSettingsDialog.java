package org.satscan.gui;

import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import javax.swing.event.UndoableEditEvent;
import javax.swing.event.UndoableEditListener;
import javax.swing.undo.UndoManager;
import org.satscan.app.AppConstants;
import org.satscan.app.BatchXMLFile;
import org.satscan.gui.utils.Utils;

/** Dialog used to manage settings of the summary email feature. */
public class SummaryEmailSettingsDialog extends javax.swing.JDialog implements ActionListener {
    private final UndoManager undo = new UndoManager();
    BatchXMLFile.SummaryMailSettings _email_settings;
    public boolean _updated = false;

    /** Creates new form SummaryEmailSettingsDialog */
    public SummaryEmailSettingsDialog(java.awt.Frame parent, BatchXMLFile.SummaryMailSettings email_settings) {
        super(parent, true);
        initComponents();
        _ok.addActionListener(this);
        _cancel.addActionListener(this);
        _email_settings = email_settings;
        _send_summary.setSelected(_email_settings._send_summary_email);
        _recipients.setText(_email_settings._summary_email_recipients);
        _recipients.setCaretPosition(0);
        _cutoff_value_ri.setText(_email_settings._summary_email_recurrence_cutoff.toString());
        enableControls();
    }

    private void enableControls() {
        _recipients.setEnabled(Utils.selected(_send_summary));
        _cutoff_value_label.setEnabled(Utils.selected(_send_summary));
        _cutoff_value_ri.setEnabled(Utils.selected(_send_summary));
        _cutoff_value_label_pval.setEnabled(Utils.selected(_send_summary));
    }
    
    /**
     * This method is called from within the constructor to initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is always
     * regenerated by the Form Editor.
     */
    @SuppressWarnings("unchecked")
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

        _send_summary = new javax.swing.JCheckBox();
        _recipients = new javax.swing.JTextField();
        _ok = new javax.swing.JButton();
        _cancel = new javax.swing.JButton();
        _cutoff_value_label = new javax.swing.JLabel();
        _cutoff_value_ri = new javax.swing.JTextField();
        _cutoff_value_label_pval = new javax.swing.JLabel();

        setDefaultCloseOperation(javax.swing.WindowConstants.DISPOSE_ON_CLOSE);
        setTitle("Summary Email Alerts");
        setResizable(false);

        _send_summary.setText("Send email alerts with summary results to recipients (csv list)");
        _send_summary.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                enableControls();
            }
        });

        _ok.setText("Ok");

        _cancel.setText("Cancel");

        _cutoff_value_label.setText("For prospective analyses, report when at least one cluster has a recurrence interval >=");

        _cutoff_value_ri.setText("365");
        _cutoff_value_ri.setMinimumSize(new java.awt.Dimension(75, 22));
        _cutoff_value_ri.setPreferredSize(new java.awt.Dimension(75, 22));
        _cutoff_value_ri.addKeyListener(new java.awt.event.KeyAdapter() {
            public void keyTyped(java.awt.event.KeyEvent e) {
                Utils.validatePostiveNumericKeyTyped(_cutoff_value_ri, e, 10);
            }
        });
        _cutoff_value_ri.addFocusListener(new java.awt.event.FocusAdapter() {
            public void focusLost(java.awt.event.FocusEvent e) {
                while (_cutoff_value_ri.getText().length() == 0)
                if (undo.canUndo()) undo.undo(); else _cutoff_value_ri.setText(AppConstants.DEFAULT_RECURRENCE_CUTOFF);
            }
        });
        _cutoff_value_ri.getDocument().addUndoableEditListener(new UndoableEditListener() {
            public void undoableEditHappened(UndoableEditEvent evt) {
                undo.addEdit(evt.getEdit());
            }
        });

        _cutoff_value_label_pval.setText("days.");

        javax.swing.GroupLayout layout = new javax.swing.GroupLayout(getContentPane());
        getContentPane().setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, layout.createSequentialGroup()
                .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING)
                    .addGroup(javax.swing.GroupLayout.Alignment.LEADING, layout.createSequentialGroup()
                        .addContainerGap()
                        .addComponent(_send_summary, javax.swing.GroupLayout.DEFAULT_SIZE, 470, Short.MAX_VALUE))
                    .addGroup(javax.swing.GroupLayout.Alignment.LEADING, layout.createSequentialGroup()
                        .addGap(14, 14, 14)
                        .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addComponent(_cutoff_value_label, javax.swing.GroupLayout.Alignment.TRAILING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                            .addComponent(_recipients, javax.swing.GroupLayout.Alignment.TRAILING)
                            .addGroup(layout.createSequentialGroup()
                                .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING)
                                    .addGroup(layout.createSequentialGroup()
                                        .addComponent(_cutoff_value_ri, javax.swing.GroupLayout.PREFERRED_SIZE, 90, javax.swing.GroupLayout.PREFERRED_SIZE)
                                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                        .addComponent(_cutoff_value_label_pval))
                                    .addGroup(layout.createSequentialGroup()
                                        .addComponent(_ok, javax.swing.GroupLayout.PREFERRED_SIZE, 68, javax.swing.GroupLayout.PREFERRED_SIZE)
                                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                        .addComponent(_cancel, javax.swing.GroupLayout.PREFERRED_SIZE, 68, javax.swing.GroupLayout.PREFERRED_SIZE)))
                                .addGap(0, 0, Short.MAX_VALUE)))))
                .addContainerGap())
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(layout.createSequentialGroup()
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .addComponent(_send_summary)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_recipients, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_cutoff_value_label)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(_cutoff_value_ri, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(_cutoff_value_label_pval))
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                    .addComponent(_ok)
                    .addComponent(_cancel))
                .addGap(22, 22, 22))
        );

        _cutoff_value_ri.getAccessibleContext().setAccessibleName("");
        _cutoff_value_ri.getAccessibleContext().setAccessibleDescription("Recurrance interval cutoff with prospective analyses.");

        pack();
    }// </editor-fold>//GEN-END:initComponents

    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JButton _cancel;
    private javax.swing.JLabel _cutoff_value_label;
    private javax.swing.JLabel _cutoff_value_label_pval;
    private javax.swing.JTextField _cutoff_value_ri;
    private javax.swing.JButton _ok;
    private javax.swing.JTextField _recipients;
    private javax.swing.JCheckBox _send_summary;
    // End of variables declaration//GEN-END:variables

    @Override
    public void actionPerformed(ActionEvent e) {
        if ("Ok".equals( e.getActionCommand() ) ) {
            if (_send_summary.isSelected()) {
                _email_settings._send_summary_email = _send_summary.isSelected();
                _email_settings._summary_email_recipients = _recipients.getText();
                _email_settings._summary_email_recurrence_cutoff = Integer.valueOf(_cutoff_value_ri.getText());
            }
            _updated = true;
        }
        setVisible(false);    
    }
}
