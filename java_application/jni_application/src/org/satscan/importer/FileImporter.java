package org.satscan.importer;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileWriter;
import java.io.IOException;
import java.util.Map;
import java.util.Vector;
import javax.swing.JProgressBar;
import org.apache.commons.lang3.ObjectUtils;
import org.apache.commons.lang3.StringUtils;
import org.apache.commons.lang3.tuple.Pair;
import org.satscan.app.*;

/**
 * Imports from data source to SaTScan formatted data file.
 */
public class FileImporter {
    private final InputSourceSettings.InputFileType _input_file_type;
    private final File _destinationFile;
    private final ImportDataSource _dataSource;
    private final Vector<ImportVariable> _importVariables;
    private final Map<Integer, Pair<InputSourceSettings.LinelistType, String>> _linelist_field_map;
    private final JProgressBar _progress;
    private boolean _cancelled=false;
    private boolean _permits_blank_values=false;
    
    public FileImporter(
            ImportDataSource dataSource, InputSourceSettings.InputFileType fileType, Vector<ImportVariable> importVariables, 
            Map<Integer, Pair<InputSourceSettings.LinelistType, String>> linelist_field_map,
            InputSourceSettings.SourceDataFileType sourceDataFileType, File destinationFile, JProgressBar progress) {
        _dataSource = dataSource;
        _importVariables = importVariables;
        _linelist_field_map = linelist_field_map;
        _destinationFile = destinationFile;
        _progress = progress;
        _input_file_type = fileType;
        _permits_blank_values = fileType == InputSourceSettings.InputFileType.NETWORK;
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
        // If file type is CASE and user defined line list mappings, add meta rows to the beginning of file.
        if (_input_file_type == InputSourceSettings.InputFileType.Case && !_linelist_field_map.isEmpty()) {
            StringBuilder meta_row = new StringBuilder(), header_row = new StringBuilder();
            // Add the import variables first.
            for (int i=0; i < _importVariables.size(); ++i) {
                if (_importVariables.get(i).isMappedToSourceField()) {
                    meta_row.append(_importVariables.get(i).getLinelistMeta() + " ");
                    String header = _importVariables.get(i).getVariableName();
                    header_row.append(StringUtils.contains(header, " ") ? ("\"" + header + "\"") : header).append(" ");
                }
            }
            // Now add the line list meta and header rows.
            for (Map.Entry<Integer, Pair<InputSourceSettings.LinelistType, String>> entry : _linelist_field_map.entrySet()) {
                switch (entry.getValue().getLeft()) {
                    case EVENT_ID: meta_row.append("<eventid> "); header_row.append("\"Event ID\" "); break;
                    case EVENT_COORD_Y: meta_row.append("<event-latitude> "); header_row.append("latitude "); break;
                    case EVENT_COORD_X: meta_row.append("<event-longitude> "); header_row.append("longitude "); break;
                    default: meta_row.append("<linelist> "); header_row.append("\"" + entry.getValue().getRight() + "\" ");
                }
            }           
            buffer.write(meta_row.toString());
            buffer.newLine();            
            buffer.write(header_row.toString());
            buffer.newLine();            
        }
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
                    if (mappedVariables.get(i).isMappedToSourceField()) {
                        if (iColumn + 1 > values.length)
                            throw new ImportException(String.format("Record %d contains just %d column%s, SaTScan could not read value at column %d.\n",
                                                                    _dataSource.getCurrentRecordNum(), values.length, values.length > 1 ? "s" : "", iColumn +1));
                        value = (String)values[iColumn];
                        if (_dataSource.isColumnDate(iColumn))
                            value = formatDateField(value);
                    } else {
                        value = new String( (String)ObjectUtils.defaultIfNull(mappedVariables.get(i).getDefault(), "") );
                    }
                    value = StringUtils.trimToEmpty(value);
                    if ((StringUtils.isEmpty(value) || StringUtils.isBlank(value)) && !_permits_blank_values) {
                        throw new ImportException(String.format("Record %d contains a 'Source File Variable' that is blank.\nSaTScan does not permit blank variables in data.", _dataSource.getCurrentRecordNum()));
                    } else {
                        if (StringUtils.contains(value, " ")) {
                            value = "\"" + value + "\"";
                        }
                        record.set(mappedVariables.get(i).getVariableIndex(), new String(value));
                    }
                }
                // Build output line from read values.
                StringBuilder output = new StringBuilder();
                for (int i=0; i < record.size(); ++i) {
                    if (record.get(i).length() > 0) {
                        output.append(record.get(i)).append(" ");
                        record.set(i, "");
                    }
                }
                // If file type is CASE and user defined line list mappings, add line list mappings.
                if (_input_file_type == InputSourceSettings.InputFileType.Case && !_linelist_field_map.isEmpty()) {
                    // Now add the line list meta and header rows.
                    for (Map.Entry<Integer, Pair<InputSourceSettings.LinelistType, String>> entry : _linelist_field_map.entrySet()) {
                        int colIdx = entry.getKey() + 2; // These are zero based but need to skip over 2 generated columns.
                        if (colIdx + 1 > values.length)
                            throw new ImportException(String.format(
                                "Record %d contains just %d column%s, SaTScan could not read value at column %d.\n",
                                _dataSource.getCurrentRecordNum(), values.length, values.length > 1 ? "s" : "", colIdx + 1
                            ));
                        value = (String)values[colIdx];
                        output.append(StringUtils.contains(value, " ") ? ("\"" + value + "\"") : value).append(" ");
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
