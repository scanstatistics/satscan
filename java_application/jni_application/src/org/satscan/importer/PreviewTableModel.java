package org.satscan.importer;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Vector;
import javax.swing.table.AbstractTableModel;
import org.apache.commons.lang.ArrayUtils;
import org.satscan.gui.FileSourceWizard;

/**
 * Extends AbstractTableModel to implement a preview table model.
 */
public class PreviewTableModel extends AbstractTableModel {

    private static final long serialVersionUID = 1L;
    public static final int DEFAULT_PREVIEW_LENGTH = 50;
    private int _previewLength = DEFAULT_PREVIEW_LENGTH;
    protected ArrayList<Object[]> _previewData = new ArrayList<Object[]>();
    protected int _maxFieldCount = 0;
    protected final ImportDataSource _data_source;
    Vector<Object> _column_names = new Vector<Object>();
    protected boolean _show_generatedId=true;
    protected boolean _show_oneCount=true;
    protected String _non_datasource_column_suffix=" #";

    /** Constructs a new PreviewTableModel object. */
    public PreviewTableModel(ImportDataSource data_source, boolean show_generatedId, boolean show_oneCount) {
        super();
        _data_source = data_source;
        _show_generatedId = show_generatedId;
        _show_oneCount = show_oneCount;
        _column_names = new Vector(Arrays.asList(_data_source.getColumnNames()));
        if (!_show_oneCount) _column_names.remove(1);
        if (!_show_generatedId) _column_names.remove(0);
        for (int i=0; i < getPreviewLength(); ++i) {
            Object[] values = _data_source.readRow();
            if (values != null) {
                addRow(values);
            }
        }        
    }

    /** Returns the number of columns in table. */
    public int getColumnCount() {
        return _maxFieldCount;
    }

    /** Returns the maximum number of rows displayed by preview. */
    public int getPreviewLength() {
        return _previewLength;
    }

    /** Set the maximum number of rows displayed by preview. */
    public void setPreviewLength(int previewLength) {
        _previewLength = previewLength;
    }

    /** Returns the number of rows in table excluding header row (if defined). */
    public int getRowCount() {
        return _previewData.size();
    }

    /** Returns object at table row/column. */
    public Object getValueAt(int row, int col) {
        Object[] rowData = _previewData.get(row);
        if (col < rowData.length) {
            return rowData[col];
        } else {
            return "";
        }
    }

    public String getNonSuffixedColumnName(int idx) {
        if (idx < _column_names.size()) {
           return (String)_column_names.elementAt(idx);
        } else {
           return FileSourceWizard._unassigned_variable; 
        }        
    }
    
    /** Returns the name of column at index. If a header row is not defined, returns "Column x" as name. */
    @Override
    public String getColumnName(int idx) {
        String name = getNonSuffixedColumnName(idx);
        if ((idx == 0 && _show_generatedId) || (idx == 0 && !_show_generatedId && _show_oneCount) || (idx == 1 && _show_generatedId && _show_oneCount))
            return name + _non_datasource_column_suffix;
        return name;
    }

    /* Returns the number of columns in data source. */
    public int getDataSourceColumnNameCount() {
        return _data_source.getColumnNames().length;
    }
    
    /** Returns the data source column name at index. */
    public String getDataSourceColumnName(int colIdx) {
        Object[] names = _data_source.getColumnNames();
        return colIdx < names.length ? (String)_data_source.getColumnNames()[colIdx] : "";
    }
    
    /** Returns the data source column index that matches name. */
    public int getDataSourceColumnIndex(String name) {
        return _data_source.getColumnIndex(name);
    }
    
    /* Adds row to table cache and fires table date change event. */
    public void addRow(Object[] row) {
        if (row != null) {
            if (!_show_oneCount) row = ArrayUtils.remove(row, 1);
            if (!_show_generatedId) row = ArrayUtils.remove(row, 0);            
            _previewData.add(row);
            if (row.length > _maxFieldCount) {
                _maxFieldCount = row.length;
            }
            fireTableDataChanged();
        }
    }
}
