package org.satscan.importer;

import java.io.*;
import java.util.*;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 * CSVImportDataSource acts as a data source for the Importer by reading
 * from a text file.  The parameters which dictate how a file is
 * interpreted are the delimiters for: row, column, and group.
 *
 *
 * @author watsond
 */
public class CSVImportDataSource implements ImportDataSource {

    protected File _sourceFile;
    protected PushbackInputStream _pushbackStream;
    protected char _rowDelimiter='\n';
    protected char _colDelimiter=',';
    protected char _groupDelimiter='"';
    private int _skip=0;
    private int _currentRowNumber=0;
    private int _totalRows=0;
    private boolean _hasHeader=false;
    private ArrayList<Object> _column_names;

    public CSVImportDataSource(File file, boolean hasHeader, char rowDelimiter, char colDelimiter, char groupDelimiter, int skip) throws FileNotFoundException {
        _column_names = new ArrayList<Object>();
        _sourceFile = file;
        _totalRows = countLines(_sourceFile);
        _pushbackStream = new PushbackInputStream(new FileInputStream(_sourceFile));
        _hasHeader = hasHeader;
        _rowDelimiter = rowDelimiter;
        _colDelimiter = colDelimiter;
        _groupDelimiter = groupDelimiter;
        _skip = skip;
        _column_names.add("Generated Id");
        _column_names.add("One Count");        
        if (_hasHeader) {
            Object[] row = readRow();
            for (int i=2; i < row.length; ++i)
                _column_names.add(row[i]);
            _skip += 1;
        } else {
            Object[] row = readRow();
            for (int i=2; i < row.length; ++i) {
                _column_names.add("Column " + (i - 1));
            }
            reset();            
        }
    }

    /** Returns number of records in file. */
    public int getNumRecords() {
        return _totalRows;
    }

    /** Returns column names, if any.*/
    public Object[] getColumnNames() {
        return _column_names.toArray();
    }
    
    public int getColumnIndex(String name) {
        for (int i=0; i < _column_names.size(); ++i) {
            String column_name = (String)_column_names.get(i);
            if (column_name.equals(name)) {
                return i + 1;
            }
        } return 0;
    }    
    
    public static int countLines(File file) {
        int lineCount = 0;
        try {
            InputStream test = new FileInputStream(file);
            Reader reader = new InputStreamReader(test);

            char[] buffer = new char[4096];
            for (int charsRead = reader.read(buffer); charsRead >= 0; charsRead = reader.read(buffer)) {
                for (int charIndex = 0; charIndex < charsRead; charIndex++) {
                    if (buffer[charIndex] == '\n') {
                        lineCount++;
                    } else if (buffer[charIndex] == '\r') {
                        lineCount++;
                        if (charIndex + 1 < charsRead && buffer[charIndex + 1] == '\n')
                            charIndex++;
                    }
                }
            }
            reader.close();
            test.close();
        } catch (Exception e) {
            throw new RuntimeException(e.getMessage(), e);
        }
        return lineCount;
    }

    public void close() {        
        try {
            if (_pushbackStream != null) {_pushbackStream.close(); _pushbackStream=null;}
        } catch (IOException ex) {
            Logger.getLogger(XLSImportDataSource.class.getName()).log(Level.SEVERE, null, ex);
        }
    }

    public String getAbsolutePath() {
        return _sourceFile.getAbsolutePath();
    }

    /* (non-Javadoc)
     * @see bsi.client.table.importer.ImportDataSource#readRow()
     */
    public Object[] readRow() {
        String line = null;

        try {
            while (_currentRowNumber < _skip) {
                line = readLine();
                if (line == null) break;
                _currentRowNumber++;
            }
            line = readLine();
        } catch (IOException ex) {
            ex.printStackTrace();
        }
        if (line == null) {
            return null;
        }
        ArrayList row = ImportUtils.parseLine(line, Character.toString(_colDelimiter), Character.toString(_groupDelimiter));
        row.add(0, "1");
        row.add(0, "location" + (_currentRowNumber + 1 - _skip));
        _currentRowNumber++;
        return row.toArray();
    }

    /* (non-Javadoc)
     * @see bsi.client.table.importer.ImportDataSource#reset()
     */
    public void reset() {
        try {
            if (_pushbackStream != null) {
                _pushbackStream.close();
            }
            _pushbackStream = new PushbackInputStream(new FileInputStream(_sourceFile));
        } catch (IOException e) {
            _pushbackStream = null;
        }
        _currentRowNumber = 0;
    }

    /* (non-Javadoc)
     * @see bsi.client.table.importer.ImportDataSource#getCurrentRow()
     */
    public int getCurrentRow() {
        return _currentRowNumber;
    }

    /**
     * @return a String representing an unparsed version of the next row.
     * @throws IOException if there is a read error or if the EOF is reached.
     */
    private String readLine() throws IOException {
        //if gStream is null no line can be read
        if (_pushbackStream == null) {
            throw new IOException("Null Stream");
        }

        StringBuilder line = new StringBuilder();
        int c = _pushbackStream.read();
        if (c < 0) {
            return null;
        }//throw new EOFException();
        while (c >= 0) {
            if (c == _rowDelimiter) {
                break;
            }
            if (c == '\r') {
                // peek at next character, to see if it is delimiter
                c = _pushbackStream.read();
                if (c != _rowDelimiter) {
                    _pushbackStream.unread(c);
                }
                break;
            } else {
                line.append((char) c);
            }
            c = _pushbackStream.read();
        }
        return line.toString();
    }

    public boolean isColumnDate(int iColumn) {
        return false;
    }

    public long getCurrentRecordNum() {
        return _currentRowNumber;
    }
}
