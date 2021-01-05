/*
 * FileAccess.java
 *
 * Created on December 11, 2007, 3:57 PM
 *
 * To change this template, choose Tools | Template Manager
 * and open the template in the editor.
 */

package org.satscan.utils;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.util.logging.Level;
import java.util.logging.Logger;
import org.apache.commons.lang3.SystemUtils;

public class FileAccess {
    
    public static boolean ValidateFileAccess(String filename, boolean bWrite, boolean temporaryFile) {
        boolean bAccessible=false;
        
        try {
            String test_filename = getFormatSubstitutedFilename(filename);
            File file = new File(test_filename + (temporaryFile ? ".writetestjava" : ""));
            if (bWrite) {
                @SuppressWarnings("unused") FileOutputStream filestream = new FileOutputStream(file);
                filestream.close();
            } else {
                @SuppressWarnings("unused") FileInputStream filestream = new FileInputStream(file);
                filestream.close();
            }
            bAccessible = true;
            if (temporaryFile) 
                file.delete();
        } catch (FileNotFoundException e) {} catch (SecurityException e) {} catch (IOException ex) {
            Logger.getLogger(FileAccess.class.getName()).log(Level.SEVERE, null, ex);
        }
        
        return bAccessible;
    }    
    
    /**
     * Native method to obtain filename with format substitutions. 
     */
    native static public final String getFormatSubstitutedFilename(final String filename);
    
    /* Get the extension of a file. */
    public static String getExtension(File f) {
        String ext = null;
        String s = f.getName();
        int i = s.lastIndexOf('.');
        if (i > 0 && i < s.length() - 1) {
            ext = s.substring(i + 1).toLowerCase();
        }
        return ext;
    }
    
    /* Checks that filename is valid -- in particular, that fullpath only has ascii characters
       for the Windows platform. There is a problem with JNI conversion.
       https://www.squishlist.com/ims/satscan/66273/
    */
    public static boolean isValidFilename(String filename) {
        String test = getFormatSubstitutedFilename(filename);
        return !(SystemUtils.IS_OS_WINDOWS && !test.matches("\\A\\p{ASCII}*\\z"));
    }
}
