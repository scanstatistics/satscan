package org.satscan.importer;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.util.Vector;
import javax.swing.JProgressBar;
import org.apache.commons.lang.StringUtils;
import org.satscan.app.*;

/**
 * Imports from data source to SaTScan formatted data file.
 */
public class FileImporter {
    //private final InputSourceSettings.InputFileType _fileType;
    //private final InputSourceSettings.SourceDataFileType _sourceDataFileType;
    private final File _destinationFile;
    private final ImportDataSource _dataSource;
    private final Vector<ImportVariable> _importVariables;
    private final JProgressBar _progress;
    private boolean _cancelled=false;
    
    public FileImporter(ImportDataSource dataSource, Vector<ImportVariable> importVariables, InputSourceSettings.InputFileType fileType, InputSourceSettings.SourceDataFileType sourceDataFileType, File destinationFile, JProgressBar progress) {
        _dataSource = dataSource;
        _importVariables = importVariables;
        //_fileType = fileType;
        //_sourceDataFileType = sourceDataFileType;
        _destinationFile = destinationFile;
        _progress = progress;
    }
    
    /** This a temporary hack function that formats date fields to sFormat. This is needed because SaTScan
     * expects dates in human readable form such as '12/08/2002' as apposed to raw data form of 20021208. */
    private String formatDateField(String dateString) {
        StringBuilder builder = new StringBuilder();
        builder.append(dateString.substring(0, 4));
        builder.append("/");
        builder.append(dateString.substring(4, 6));
        builder.append("/");
        builder.append(dateString.substring(6, 8));
        return builder.toString();
    }
    
    public boolean getCancelled() {
        return _cancelled;
    }
    
    public void setCancelled() {
        _cancelled = true;
    }
    
    /**
     * Imports file into destination file.
     */
    public void importFile(int skipCount) throws IOException, SecurityException {
        int iColumn=0;
        int iRow=0;
        String value;
        Vector<String> record = new Vector<String>();
        Vector<ImportVariable> mappedVariables = new Vector<ImportVariable>();
        
        //Attempt to open file writer and buffer ...
        FileWriter writer = new FileWriter(_destinationFile);
        BufferedWriter buffer = new BufferedWriter(writer);
        //initialize record and determine which field are actually imported or have a default
        for (int i=0; i < _importVariables.size(); ++i) {
            record.add(new String());
            if (_importVariables.get(i).isMappedToSourceField() || _importVariables.get(i).hasDefault())
                mappedVariables.add(_importVariables.get(i));
        }
        //initialize progress ...
        _progress.setMaximum(_dataSource.getNumRecords());
        //start reading and writing records ...
        Object[] values = _dataSource.readRow();
        while (values != null && !_cancelled) {
            if (iRow >= skipCount) {
                iColumn = 0;
                for (int i=0; i < mappedVariables.size(); ++i) {
                    //get the zero based column index from mapping variables
                    iColumn = mappedVariables.get(i).getSourceFieldIndex() - 1;
                    //retrieve value from current data row or the default 
                    if (_importVariables.get(i).isMappedToSourceField()) {
                        if (iColumn + 1 > values.length)
                            throw new ImportException(String.format("Record %d contains just %d column%s, SaTScan could not read value at column %d.\n",
                                                                    _dataSource.getCurrentRecordNum(), values.length, values.length > 1 ? "s" : "", iColumn +1));
                        value = (String)values[iColumn];
                        if (_dataSource.isColumnDate(iColumn))
                            value = formatDateField(value);
                    } else {
                        value = new String(_importVariables.get(i).getDefault());
                    }
                    value = StringUtils.trimToEmpty(value);
                    if (StringUtils.isEmpty(value) || StringUtils.isBlank(value)) {
                        throw new ImportException(String.format("Record %d contains a 'Source File Variable' that is blank.\nSaTScan does not permit blank variables in data.", _dataSource.getCurrentRecordNum()));
                    } else {
                        if (StringUtils.contains(value, " ")) {
                            value = "\"" + value + "\"";
                        }
                        record.set(mappedVariables.get(i).getVariableIndex(), new String(value));
                    }
                }
                //append string to end of output file and clear input vector at the same time...
                StringBuilder output = new StringBuilder();
                for (int i=0; i < record.size(); ++i) {
                    if (record.get(i).length() > 0) {
                        output.append(record.get(i));
                        output.append(" ");
                        record.set(i, "");
                    }
                }
                buffer.write(output.toString());
                buffer.newLine();
            }
            values = _dataSource.readRow();
            _progress.setValue(++iRow);
        }
        buffer.close();
        writer.close();
    }
}
