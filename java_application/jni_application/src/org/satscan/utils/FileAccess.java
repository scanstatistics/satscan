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
import org.apache.commons.lang3.SystemUtils;

public class FileAccess {
    
    public static boolean ValidateFileAccess(String filename, boolean bWrite) {
        boolean bAccessible=false;
        
        try {
            if (bWrite) {
                @SuppressWarnings("unused") FileOutputStream file = new FileOutputStream(filename);
            } else {
                @SuppressWarnings("unused") FileInputStream file = new FileInputStream(filename);
            }
            bAccessible = true;
        } catch (FileNotFoundException e) {} catch (SecurityException e) {}
        
        return bAccessible;
    }    
    
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
        return !(SystemUtils.IS_OS_WINDOWS && !filename.matches("\\A\\p{ASCII}*\\z"));
    }
}
