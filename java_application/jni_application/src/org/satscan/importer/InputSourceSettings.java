/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */
package org.satscan.importer;

import java.util.HashMap;
import java.util.LinkedHashMap;
import java.util.Map;
import java.util.Vector;
import org.apache.commons.lang3.tuple.Pair;
import org.satscan.app.UnknownEnumException;

/**
 * 
 * @author hostovic
 */
public class InputSourceSettings implements Cloneable  {
    public enum SourceDataFileType {CSV, dBase, Shapefile, Excel97_2003, Excel};
    public enum InputFileType      {Case, Control, Population, Coordinates, SpecialGrid, MaxCirclePopulation, AdjustmentsByRR, NETWORK, Neighbors, MetaLocations, AlternativeHypothesis};
    public enum LinelistType       {EVENT_ID, EVENT_COORD_X, EVENT_COORD_Y, GENERAL_DATA, CATEGORICAL_DATA, CONTINUOUS_DATA, DISCRETE_DATA};
    
    private SourceDataFileType _source_type=SourceDataFileType.CSV;
    private InputFileType _file_type=InputFileType.Case;
    private int _data_set_index=1;
    private Vector<String> _mappings = new Vector<String>();
    // CVS specific options
    private String _delimiter=",";
    private String _grouper="\"";
    private int _skip_lines=0;
    private boolean _first_row_headers=false;
    private Map<Integer, Pair<LinelistType, String>> _linelist_field_map = new LinkedHashMap<>();
        
    public InputSourceSettings() {}
    public InputSourceSettings(InputFileType filetype) {
        _file_type = filetype;
    }
    
    public InputSourceSettings(InputSourceSettings inputsource) {
        _source_type = inputsource._source_type;
        _file_type = inputsource._file_type;
        _data_set_index = inputsource._data_set_index;
        _mappings = new Vector<String>(inputsource._mappings);
        _delimiter = inputsource._delimiter;
        _grouper = inputsource._grouper;
        _skip_lines = inputsource._skip_lines;
        _first_row_headers = inputsource._first_row_headers;
        for (Map.Entry<Integer, Pair<LinelistType, String>> mapEntry : inputsource.getLinelistFieldMaps().entrySet())
          _linelist_field_map.put(mapEntry.getKey(), mapEntry.getValue());        
    }
    
    public InputSourceSettings clone() throws CloneNotSupportedException {
        InputSourceSettings iss = (InputSourceSettings) super.clone();
        iss._mappings = (Vector<String>)_mappings.clone();
        iss._delimiter = new String(_delimiter);
        iss._grouper = new String(_grouper);
        iss._linelist_field_map = new LinkedHashMap<>();
        for (Map.Entry<Integer, Pair<LinelistType, String>> mapEntry : getLinelistFieldMaps().entrySet())
          iss._linelist_field_map.put(mapEntry.getKey(), mapEntry.getValue());        
        return iss;
    }
    
    public void copy(InputSourceSettings other) {
        _source_type = other._source_type;
        _file_type = other._file_type;
        _data_set_index = other._data_set_index;
        _mappings = other._mappings;
        _delimiter = other._delimiter;
        _grouper = other._grouper;
        _skip_lines = other._skip_lines;
        _first_row_headers = other._first_row_headers;
        _linelist_field_map.clear();
        for (Map.Entry<Integer, Pair<LinelistType, String>> mapEntry : other.getLinelistFieldMaps().entrySet())
          _linelist_field_map.put(mapEntry.getKey(), mapEntry.getValue());
    }
    
    @Override
    public boolean equals(Object _rhs) {
        InputSourceSettings other = (InputSourceSettings)_rhs;
        if (_source_type != other._source_type) return false;
        if (_file_type != other._file_type) return false;
        if (_data_set_index != other._data_set_index) return false;
        if (!_mappings.equals(other._mappings)) return false;
        switch (_source_type) {
            case CSV : 
                if (!_delimiter.equals(other._delimiter)) return false;
                if (!_grouper.equals(other._grouper)) return false;
                if (_skip_lines != other._skip_lines) return false;
                if (_first_row_headers != other._first_row_headers) return false;
                break;
            case Shapefile : 
            case dBase : 
            case Excel :
            case Excel97_2003 : break;
           default: throw new UnknownEnumException(_file_type);
        }
        if (!_linelist_field_map.equals(((InputSourceSettings)_rhs)._linelist_field_map)) return false;
        return true;
    }
    
    public void ThrowOrdinalIndexException(int iInvalidOrdinal, Enum[] e) {
        throw new RuntimeException("Ordinal index " + iInvalidOrdinal + " out of range [" +  e[0].ordinal() + "," +  e[e.length - 1].ordinal() + "].");
    }    
    
    public SourceDataFileType getSourceDataFileType() {return _source_type;}
    public void setSourceDataFileType(int iOrdinal) {
        try { setSourceDataFileType(SourceDataFileType.values()[iOrdinal]);
        } catch (ArrayIndexOutOfBoundsException e) { 
            ThrowOrdinalIndexException(iOrdinal, SourceDataFileType.values()); 
        }
    }
    public void setSourceDataFileType(SourceDataFileType e) {_source_type = e;}
    
    public InputFileType getInputFileType() {return _file_type;}
    public void setInputFileType(int iOrdinal) {
        try { setInputFileType(InputFileType.values()[iOrdinal]);
        } catch (ArrayIndexOutOfBoundsException e) { 
            ThrowOrdinalIndexException(iOrdinal, InputFileType.values()); 
        }
    }        
    public void setInputFileType(InputFileType e) {_file_type = e;}
    public int getDataSetIndex() {return _data_set_index;}
    public void setDataSetIndex(int i) {_data_set_index = i;}

    public Vector<String> getFieldMaps() {return _mappings;}
    public void addFieldMapping(String s) {_mappings.addElement(s);}    
    public void setFieldMaps(Vector<String> v) {_mappings = new Vector<String>(v);}
        
    public Map<Integer, Pair<LinelistType, String>> getLinelistFieldMaps() {return _linelist_field_map;}
    public void addLinelistFieldMapping(int column, int iOrdinal, String label) {
        try { 
            _linelist_field_map.put(column, Pair.of(LinelistType.values()[iOrdinal], label));
        } catch (ArrayIndexOutOfBoundsException e) { ThrowOrdinalIndexException(iOrdinal, LinelistType.values()); }     
    }
    public void setLinelsitFieldMaps(Map<Integer, Pair<LinelistType, String>> v) {_linelist_field_map = new HashMap(v);}    
    public String getLinelistFieldMapsStr() {
        StringBuilder builder = new StringBuilder();
        for (Map.Entry<Integer, Pair<LinelistType, String>> mapEntry : _linelist_field_map.entrySet()) {
            builder.append(mapEntry.getKey()).append(":").append("\"").append(mapEntry.getValue().getLeft().ordinal()).append("\"");
            builder.append(":").append(mapEntry.getValue().getRight()).append(",");
        }
        String llmapStr = builder.toString();
        if (llmapStr.endsWith(","))
            llmapStr = llmapStr.substring(0, llmapStr.length() - 1);
        return llmapStr;
    }
    public Pair<Boolean, Boolean> hasLinelistEventIdAndPlotInfo() {
        boolean event=false, eventx=false, eventy=false;
        for (Map.Entry<Integer, Pair<LinelistType, String>> mapEntry : _linelist_field_map.entrySet()) {
            switch (mapEntry.getValue().getLeft()) {
                case EVENT_ID: event = true; break;
                case EVENT_COORD_X: eventx = true; break;
                case EVENT_COORD_Y: eventy = true; break;
            }
        }
        return Pair.of(event, eventx && eventy);
    }
    
    public String getDelimiter() {return _delimiter;}
    public void setDelimiter(String s) {_delimiter = s;}
    
    public String getGroup() {return _grouper;}
    public void setGroup(String s) {_grouper = s;}
    
    public int getSkiplines() {return _skip_lines;}
    public void setSkiplines(int i) {_skip_lines = i;}
    
    public boolean getFirstRowHeader() {return _first_row_headers;}
    public void setFirstRowHeader(boolean b) {_first_row_headers = b;}
    
    public boolean isSet() {return _mappings.size() > 0;}
    public void reset() {
        _source_type=SourceDataFileType.CSV;
        _mappings.clear();
        _delimiter=",";
        _grouper="\"";
        _skip_lines=0;
        _first_row_headers=false;
        _linelist_field_map.clear();
    }
}
