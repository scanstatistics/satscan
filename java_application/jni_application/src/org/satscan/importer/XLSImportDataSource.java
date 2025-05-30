package org.satscan.importer;

import java.io.*;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.logging.Level;
import java.util.logging.Logger;
import org.apache.poi.hssf.usermodel.HSSFWorkbook;
import org.apache.poi.ss.usermodel.BuiltinFormats;
import org.apache.poi.xssf.usermodel.XSSFWorkbook;
import org.apache.poi.ss.usermodel.Cell;
import org.apache.poi.ss.usermodel.DataFormat;
import org.apache.poi.ss.usermodel.DateUtil;
import org.apache.poi.ss.usermodel.Row;
import org.apache.poi.ss.usermodel.Sheet;
import org.apache.poi.ss.usermodel.Workbook;
import org.satscan.utils.FileAccess;

/**
 * XLSImportDataSource acts as a data source for the Importer by reading Excel files.
 */
public class XLSImportDataSource implements ImportDataSource {

    protected Workbook _workbook;
    protected Sheet _sheet;
    protected int _current_row;
    private final String _file_path;
    private int _sheet_index = 0;
    private ArrayList<Object> _column_names;
    private boolean _has_header=false;
    private InputStream _input_stream=null;

    public XLSImportDataSource(File file, boolean hasHeader) {
        _column_names = new ArrayList<>();
        _current_row = 0;
        _has_header = hasHeader;
        try {
            _input_stream = new FileInputStream(file);
            if (FileAccess.getExtension(file).equals("xlsx"))
                _workbook = new XSSFWorkbook(_input_stream);
            else
                _workbook = new HSSFWorkbook(_input_stream);
        } catch (FileNotFoundException e) {
            throw new RuntimeException(e);
        } catch (org.apache.poi.hssf.OldExcelFormatException e) {
            throw new ImportDataSource.UnsupportedException("The Excel file could not be opened.\n" +
            "This error may be caused by opening an unsupported XLS version (Excel 5.0/7.0 format).\n" +
            "To test whether you are importing an unsupported version, re-save the file to versions 97-2003 and try again.", e);
        } catch (IOException e) {
            throw new RuntimeException(e);
        }
        _file_path = file.getAbsolutePath();
        //Set the first sheet to be initial default.
        setSheet(0);
        _column_names.add("Generated Id");
        _column_names.add("One Count");        
        if (_has_header) {
            Object[] row = readRow();
            for (int i=2; i < row.length; ++i)
                _column_names.add(row[i]);
            _current_row = 1;
        } else {
            int sample_count = 0;
            int maxCols = 0;
            Object[] row = readRow();
            while (row != null && sample_count < SAMPLE_ROWS) {
                sample_count++;
                maxCols = Math.max(maxCols, row.length);
                row = readRow();
            }
            for (int i=1; i <= maxCols; ++i) {
                _column_names.add("Column " + i);
            }
            _current_row = _has_header ? 1 : 0; // reset current row
        }
    }

    @Override
    public void close() {        
        try {
            if (_input_stream != null) {_input_stream.close(); _input_stream=null;}
        } catch (IOException ex) {
            Logger.getLogger(XLSImportDataSource.class.getName()).log(Level.SEVERE, null, ex);
        }
    }

    /* Sets what sheet to read from. */
    public void setSheet(int i) {
        _sheet = _workbook.getSheetAt(i);
        _sheet_index = i;
    }

    /* Gets the index of the sheet to read from. */
    public int getSheetIndex() {
        return _sheet_index;
    }

    /**
     * @return An array of strings each of which represent a name
     * of a sheet in the workbook.
     */
    public String[] getSheetList() {
        int sheetCount = _workbook.getNumberOfSheets();
        String[] sheetList = new String[sheetCount];
        for (int i = 0; i < sheetCount; i++) {
            sheetList[i] = _workbook.getSheetName(i);
        }
        return sheetList;
    }

    @Override
    public Object[] readRow() {
        if (_sheet == null) { //If _sheet is null then throw IOException so the caller is alerted to an unexpected error.
            throw new RuntimeException("Null Sheet Reference");
        }

        boolean containedNonEmptyCells = false;
        //returns the next non-empty row or null if none are left
        while (true) {
            if (_current_row <= _sheet.getLastRowNum()) {
                Row row = _sheet.getRow(_current_row);
                _current_row++;
                if (row != null && row.getPhysicalNumberOfCells() > 0) {
                    ArrayList<Object> obj = new ArrayList<>();
                    obj.add("location" + (_current_row));
                    obj.add("1");
                    //Get the number of defined cells in this row
                    int totalDefinedCells = row.getPhysicalNumberOfCells();
                    //Initialize the number of defined cells found so far
                    int definedCellsFound = 0;
                    //set number of cells that exist (defined or not)
                    int lastCellInRow = row.getLastCellNum();
                    //Stop for loop after processing all the defined cells of this row
                    for (int i=0; i < lastCellInRow; i++) {
                        //if still processing defined cells add them accordingly
                        if (definedCellsFound < totalDefinedCells) {
                            Cell cell = row.getCell((short) i);
                            if (cell != null) {
                                //If this cell is defined increment count
                                definedCellsFound++;
                                String cellContents = getCellValue(cell).trim();
                                //keep a flag to see if any of the cells contains real data, not just ""
                                if (!cellContents.equals("")) {
                                    containedNonEmptyCells = true;
                                }
                                obj.add(cellContents);
                            } else { //If the cell is undefined add an empty string
                                obj.add("");
                            }
                        } else {//done with defined cells but there are still some null trailing cells
                            obj.add("");
                        }
                    }
                    //normal return when row has data
                    if (containedNonEmptyCells) {
                        return obj.toArray();
                    }
                }
            } else { //gCurrentRow > _sheet.getLastRowNum()
                //If there are no more rows to read return null.
                return null;
            }
        }
    }

    /**
     * make a date string out of the excel representation (i.e. M/D/YYYY [HH:MM])
     * @param cellValue excel representation
     * @return a string formatted date
     */
    private String formatDate(double cellValue) {
        //create date and set calendar to be used in setting dateValue
        Calendar cal = Calendar.getInstance();
        cal.setTime(DateUtil.getJavaDate(cellValue));
        StringBuilder dateValue = new StringBuilder();
        dateValue.append(cal.get(Calendar.YEAR));
        dateValue.append("/");
        int month = cal.get(Calendar.MONTH) + 1;
        if (month < 10) {
            dateValue.append("0");
        }
        dateValue.append(month);
        dateValue.append("/");
        int day = cal.get(Calendar.DAY_OF_MONTH);
        if (day < 10) {
            dateValue.append("0");
        }
        dateValue.append(day);
        return dateValue.toString();
    }

    /* Attempts to convert numeric cell value. */
    private String formatNumericCell(Cell cell, double cellValue) {
        boolean isPoiKnownDate = DateUtil.isCellDateFormatted(cell) || DateUtil.isInternalDateFormat(cell.getCellStyle().getDataFormat());
        if (isPoiKnownDate) {
            return formatDate(cellValue);
        } else {
            //try to see if this is a user defined date format (unknown by POI's HSSFDateUtil)
            short formatIndex = cell.getCellStyle().getDataFormat();
            try {
                BuiltinFormats.getBuiltinFormat(formatIndex);
            } catch (ArrayIndexOutOfBoundsException e) { // user defined format
                DataFormat dataFormat = _workbook.createDataFormat();
                String format = dataFormat.getFormat(formatIndex);
                boolean isValid = DateUtil.isValidExcelDate(cellValue);
                //see if there are any date formatting strings in the format
                boolean hasDateFormating = format.matches(".*[mM]{2,}.*|.*[dD]{2,}.*|.*[yY]{2,}.*|.*[hH]{2,}.*|.*[sS]{2,}.*|.*[qQ]{2,}.*|.*[nN]{2,}.*|.*[wW]{2,}.*");
                if (isValid && hasDateFormating) {
                    return formatDate(cellValue);
                }
            }
            //if this is not a date process as a number
            Double number = cellValue;
            //if this is an int create the string as such
            if (number.doubleValue() == number.intValue()) {
                return (Integer.toString(number.intValue()));
            }
            return (number.toString());
        }        
    }
    
    /**
     * The only types of cell supported are:<br>
     * NUMERIC, STRING, FORMULA, and BLANK.
     *
     * @param cell the cell to have its value converted to a String.
     * @return the String representation of the value in the cell. If the
     * cell is blank or unsupported an empty String is returned.
     */
    private String getCellValue(Cell cell) {
        try {
            switch (cell.getCellType()) {
                case ERROR:
                    return cell.getStringCellValue();
                case NUMERIC:
                    return formatNumericCell(cell, cell.getNumericCellValue());
                case BOOLEAN:
                    return cell.getBooleanCellValue() + "";
                case STRING:
                    return cell.getStringCellValue();
                case FORMULA:
                    try { // First try as numeric then fallback to string.
                        return formatNumericCell(cell, cell.getNumericCellValue());
                    } catch (Throwable e) {
                        return cell.getStringCellValue();
                    }
                case BLANK:
                default: return "";
            }
        } catch (Throwable e) {
            return "##cell-read-failure##";
        }
    }

    @Override
    public boolean isColumnDate(int iColumn) {
        return false;
    }

    @Override
    public long getCurrentRecordNum() {
        return _current_row;
    }

    @Override
    public int getNumRecords() {
        return _sheet.getLastRowNum();
    }

    /** Returns column names, if any.*/
    @Override
    public Object[] getColumnNames() {
        return _column_names.toArray();
    }
    
    @Override
    public int getColumnIndex(String name) {
        for (int i=0; i < _column_names.size(); ++i) {
            String column_name = (String)_column_names.get(i);
            if (column_name.equals(name)) {
                return i + 1;
            }
        } return 0;
    }  
}
