/*
 * Copyright (C) 2010 Information Management Services, Inc.
 */
package org.satscan.gui.utils.help;

import java.awt.Cursor;
import java.awt.Window;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;
import java.net.URL;

import javax.swing.BorderFactory;
import javax.swing.ImageIcon;
import javax.swing.JButton;
import javax.swing.JComponent;

public class AppHelpButton extends JButton {

    private AppHelpDialog _dlg;

    private boolean _showUndecorated, _showingUndecorated;
    
    public AppHelpButton(Window parent, String helpId, String dlgTitle, boolean decorate, String content) {
        this(parent, helpId, dlgTitle, decorate, content, AppGuiUtils.createIcon("help.png"));
    }

    public AppHelpButton(Window parent, String helpId, String dlgTitle, boolean decorate, URL content) {
        this(parent, helpId, dlgTitle, decorate, content, AppGuiUtils.createIcon("help.png"));
    }

    public AppHelpButton(Window parent, String helpId, String dlgTitle, boolean decorate, String content, ImageIcon icon) {
        this(parent, helpId, dlgTitle, icon);
        _dlg = new AppHelpDialog(parent, this, helpId, "Help - " + dlgTitle, icon.getImage(), content, decorate);
        _showUndecorated = !decorate;
        _showingUndecorated = false;
    }

    public AppHelpButton(Window parent, String helpId, String dlgTitle, boolean decorate, URL content, ImageIcon icon) {
        this(parent, helpId, dlgTitle, icon);
        _dlg = new AppHelpDialog(parent, this, helpId, "Help - " + dlgTitle, icon.getImage(), content);
        _showUndecorated = !decorate;
        _showingUndecorated = false;
    }

    protected AppHelpButton(final Window parent, final String helpId, final String dlgTitle, ImageIcon icon) {
        this.setText(null);
        this.setOpaque(false);
        this.setActionCommand("display-help");
        this.setName("display-help-btn");
        this.setToolTipText(null); // the help dialog already behave like a tooltip, having both is just weird
        this.setIcon(icon);
        this.setContentAreaFilled(false);
        this.setFocusPainted(false);
        this.setBorder(BorderFactory.createEmptyBorder(1, 2, 1, 2));

        this.addMouseListener(new MouseAdapter() {
            @Override
            public void mouseEntered(MouseEvent arg0) {
                _dlg.setMouseOnButton(true);
                parent.setCursor(Cursor.getPredefinedCursor(Cursor.HAND_CURSOR));
            }

            @Override
            public void mouseExited(MouseEvent arg0) {
                _dlg.setMouseOnButton(false);
                parent.setCursor(Cursor.getPredefinedCursor(Cursor.DEFAULT_CURSOR));
            }
        });

        this.addActionListener(new ActionListener() {
            @Override
            public void actionPerformed(final ActionEvent e) {
                javax.swing.SwingUtilities.invokeLater(new Runnable() {
                    public void run() {
                        if (AppGuiUtils.show(helpId) == null) {
                            if (_showingUndecorated)
                                _dlg.performClose();
                            else if (!_showUndecorated) {
                                AppGuiUtils.showAndPosition(_dlg, parent);
                            }
                            else {
                                _showingUndecorated = true;
                                _dlg.setRelativeComponent((JComponent)e.getSource(), parent);
                                // there is a bug in Swing where the first call does not size the window properly, but the second call fixes that :-(
                                _dlg.setRelativeComponent((JComponent)e.getSource(), parent);
                                _dlg.setVisible(true);
                            }
                        }
                    }
                });
            }
        });
    }

    public void performWindowClosed() {
        _showingUndecorated = false;
    }
}
