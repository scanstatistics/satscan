package org.satscan.gui.utils;

import java.io.File;
import javax.swing.filechooser.FileFilter;
import org.satscan.utils.FileAccess;

public class InputFileFilter extends FileFilter {

    public final String filter;
    public final String description;

    public InputFileFilter(String filter, String description) {
        super();
        this.filter = filter;
        this.description = description;
    }
    
    public String getFilter() {
        return this.filter;
    }
    
    public boolean accept(File f) {
        if (f.isDirectory()) {
            return true;
        }
        String extension = FileAccess.getExtension(f);
        if (extension != null) {
            if (extension.equals(filter)) {
                return true;
            } else {
                return false;
            }
        }
        return false;
    }

    public String getDescription() {
        return description;
    }
}
