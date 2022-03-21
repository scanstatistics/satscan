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
import java.io.FileOutputStream;
import java.nio.file.FileSystems;
import java.nio.file.Files;
import java.nio.file.Path;
import org.apache.commons.lang3.SystemUtils;

public class FileAccess {
    
    public static boolean ValidateFileAccess(String filename, boolean bWrite, boolean na) {
        boolean bAccessible=false;
        try {
            String test_filename = getFormatSubstitutedFilename(filename);
            Path fpath = FileSystems.getDefault().getPath(test_filename);
            if (bWrite) {
                if (Files.exists(fpath)) { // File exists - test whether it is writable.
                    bAccessible = Files.isWritable(fpath);
                } else { // File doesn't exist, create temporary file then test whether it is writable.
                    @SuppressWarnings("unused") 
                    FileOutputStream file = new FileOutputStream(test_filename);
                    file.close();
                    bAccessible = true;
                    Files.deleteIfExists(fpath);
                }
            } else {
                bAccessible = Files.isReadable(fpath);
            }
        } catch (java.io.IOException | SecurityException e) {}
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
