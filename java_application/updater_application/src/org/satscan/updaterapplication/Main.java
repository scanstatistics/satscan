/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package org.satscan.updaterapplication;

import javax.swing.JOptionPane;
import javax.swing.UIManager;

/**
 *
 * @author Scott
 */
public class Main {

    private static String VER_ID_OPTION_STRING = "-ver_id";

    /**
     * @param args the command line arguments
     */
    public static void main(String[] args) {
        try {
            UIManager.setLookAndFeel(UIManager.getSystemLookAndFeelClassName());
            if (args.length < 2) {
                throw new RuntimeException("Invalid argument count.\nUsage: 'archive filename' 'launch application' [-ver_id[=val]]");
            }
            //check and show End User License Agreement if not "unrequested":
            boolean hasVersionOption = false;
            for (int i = 0; i < args.length && !hasVersionOption; ++i) {
                hasVersionOption = args[i].startsWith(VER_ID_OPTION_STRING);
            }
            //run the update
            new InstallerFrame(args[0], args[1]).installUpdate(!hasVersionOption);
        } catch (Throwable t) {
            JOptionPane.showMessageDialog(null, String.format("SaTScan update was aborted due to an error while reading updates.\nPlease email SaTScan with the following information:\n\n%s.", t.getMessage()), "SaTScan Update Aborted", JOptionPane.ERROR_MESSAGE);
            System.exit(0);
        }
    }
}
