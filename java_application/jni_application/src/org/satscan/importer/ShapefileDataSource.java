/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package org.satscan.importer;

import java.io.File;
import java.util.Vector;
import org.satscan.utils.FileAccess;

/**
 *
 * @author hostovic
 */
public class ShapefileDataSource implements ImportDataSource {
    private File _source_file;
    private DBaseImportDataSource _dbase_data_source=null;
    private long _current_row_number = 0;
    
    public ShapefileDataSource(File file, boolean formatDates) {
        try {
            _source_file = file;
            // check for existance of dBase file
            String dBaseFilename = file.getAbsolutePath();
            int lastDot = dBaseFilename.lastIndexOf(".");
            if (lastDot != -1) {
                 dBaseFilename = dBaseFilename.substring(0, lastDot) + ".dbf";
            } else {
                dBaseFilename = dBaseFilename + ".dbf";
            } 
            if (FileAccess.ValidateFileAccess(dBaseFilename, false)) {
                _dbase_data_source = new DBaseImportDataSource(new File(dBaseFilename), formatDates);
            }
        } catch (Throwable e) {}   
    }
     
    public Object[] getColumnNames() {
        Vector<Object> names = new Vector<Object>();
        names.add("Generated Id");
        names.add("One Count");
        names.add("Latitude / Y");
        names.add("Longitude / X");
        if (_dbase_data_source != null) {
           names.addAll(java.util.Arrays.asList(_dbase_data_source.getColumnNames()));
        }
        return names.toArray();
    }   
    
    /**
     * Returns whether column at index is date field.
     */
    @Override
    public boolean isColumnDate(int iColumn) {
        // first 4 columns are always the X coordinate, Y coordinate, one count and generated id
        if (iColumn >= 0 && iColumn <= 3)
            return false;
        return _dbase_data_source == null ? false : _dbase_data_source.isColumnDate(iColumn - 4);
    }

    /**
     * Native method that will return the number of shapes in file.
     */
    private native int getNumberOfShapes(String filename);    
    
    /**
     * Native method which returns file shape type is supported.
     */
    public static native String isSupportedShapeType(String filename);    
    
    @Override
    public long getCurrentRecordNum() {
        return _current_row_number;
    }

    @Override
    public int getNumRecords() {
        return getNumberOfShapes(_source_file.getAbsolutePath());
    }

    /*
     * native call to get the longitude / latitude of shape.
     */
    public native double[] getCoordinates(String filename, long shapeIdx);

    /* Retrieves coordinates for shape at shapeIdx. */
    public double[] getCoordinates(long shapeIdx) {
        return getCoordinates(_source_file.getAbsolutePath(), shapeIdx);
    }
    
    @Override
    public Object[] readRow() {
        ++_current_row_number;
        if (_current_row_number > getNumRecords())
            return null;
        Vector<Object> values = new Vector<Object>();
        values.add("location" + _current_row_number);
        values.add("1");
        double[] coordinates = getCoordinates(_current_row_number - 1);
        if (coordinates == null) {
            values.add("");
            values.add("");
        } else {
            for (int i=0; i < coordinates.length; ++i)
                values.add(Double.toString(coordinates[i]));
        }
        if (_dbase_data_source != null) {
            Object[] dbase_values = _dbase_data_source.readRow();
            if (dbase_values != null)
                values.addAll(java.util.Arrays.asList(dbase_values));
        }
        return values.toArray();
    }
}
