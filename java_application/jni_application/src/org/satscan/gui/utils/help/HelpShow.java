/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package org.satscan.gui.utils.help;

import java.net.URL;
import java.util.Locale;
import javax.help.HelpBroker;
import javax.help.HelpSet;
import javax.help.Popup;
import javax.help.SwingHelpUtilities;
import javax.swing.JOptionPane;
import org.satscan.gui.ExceptionDialog;
import org.satscan.gui.SaTScanApplication;
import org.satscan.utils.BareBonesBrowserLaunch;

/**
 *
 * @author hostovic
 */
public class HelpShow {
    public enum ShowType {JavaHelp, JavaHelpPopup, Website};
    /**
     * Shows Java help system, initially directing to section at 'helpID' if not null.
     * Conditionally shows help as popup window.
     * @param helpID
     */
     public static void showHelp(String helpID) {
        final String helpsetName = "help";
        final String defaultID = "Introduction";
        ShowType showType = helpID.contains("http://") ? HelpShow.ShowType.Website : HelpShow.ShowType.JavaHelp;
        
        try {
            if (showType == ShowType.Website) {
                BareBonesBrowserLaunch.openURL(helpID);
            } else {
                SwingHelpUtilities.setContentViewerUI("org.satscan.gui.utils.ExternalLinkContentViewerUI");
                ClassLoader cl = SaTScanApplication.class.getClassLoader();
                URL url = HelpSet.findHelpSet(cl, helpsetName, "", Locale.getDefault());
                if (url == null) {
                    url = HelpSet.findHelpSet(cl, helpsetName, ".hs", Locale.getDefault());
                    if (url == null) {
                        JOptionPane.showMessageDialog(null, "The help system could not be located.", " Help", JOptionPane.WARNING_MESSAGE);
                        return;
                    }
                }                
                if (showType == ShowType.JavaHelpPopup && helpID != null) {
                    Popup popup = SaTScanApplication.getInstance().getHelpPopup();
                    popup.setCurrentID(helpID);
                    popup.setDisplayed(true);
                } else {
                    HelpBroker mainHB = SaTScanApplication.getInstance().getHelpBroker();
                    mainHB.setCurrentID(helpID != null ? helpID : defaultID);
                    if (!mainHB.isDisplayed()) {
                        mainHB.setSize(SaTScanApplication.getInstance().getSize());
                        mainHB.setLocation(SaTScanApplication.getInstance().getLocation());
                        mainHB.setDisplayed(true);
                    }
                }
            }
        } catch (Throwable t) {
            new ExceptionDialog(SaTScanApplication.getInstance(), t).setVisible(true);
        }
    }
     
     public static void showHelp_test(String helpID) {
        final String helpsetName = "help";
        final String defaultID = "Introduction";
        ShowType showType = helpID.contains("http://") ? HelpShow.ShowType.Website : HelpShow.ShowType.JavaHelpPopup;//JavaHelp;
        
        try {
            if (showType == ShowType.Website) {
                BareBonesBrowserLaunch.openURL(helpID);
            } else {
                SwingHelpUtilities.setContentViewerUI("org.satscan.gui.utils.ExternalLinkContentViewerUI");
                ClassLoader cl = SaTScanApplication.class.getClassLoader();
                URL url = HelpSet.findHelpSet(cl, helpsetName, "", Locale.getDefault());
                if (url == null) {
                    url = HelpSet.findHelpSet(cl, helpsetName, ".hs", Locale.getDefault());
                    if (url == null) {
                        JOptionPane.showMessageDialog(null, "The help system could not be located.", " Help", JOptionPane.WARNING_MESSAGE);
                        return;
                    }
                }                
                if (showType == ShowType.JavaHelpPopup && helpID != null) {
                    Popup popup = SaTScanApplication.getInstance().getHelpPopup();
                    popup.setCurrentID(helpID);
                    popup.setDisplayed(true);
                } else {
                    HelpBroker mainHB = SaTScanApplication.getInstance().getHelpBroker();
                    mainHB.setCurrentID(helpID != null ? helpID : defaultID);
                    if (!mainHB.isDisplayed()) {
                        mainHB.setSize(SaTScanApplication.getInstance().getSize());
                        mainHB.setLocation(SaTScanApplication.getInstance().getLocation());
                        mainHB.setDisplayed(true);
                    }
                }
            }
        } catch (Throwable t) {
            new ExceptionDialog(SaTScanApplication.getInstance(), t).setVisible(true);
        }
    }     
}

