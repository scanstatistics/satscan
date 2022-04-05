/*
 * ImportVariable.java
 *
 * Created on December 14, 2007, 3:45 PM
 *
 * To change this template, choose Tools | Template Manager
 * and open the template in the editor.
 */
package org.satscan.importer;

/**
 * Import variable.
 */
public class ImportVariable implements Cloneable {

    private String _variable_name="";
    private int _variable_index=-1;
    private boolean _variable_required=true;
    private int _source_field_index=0;
    private String _help_text=null;
    private boolean _showing=true;
    private String _default=null;
    private String _linelist_meta=null;

    public ImportVariable() {}

    public ImportVariable(String display_name, int variable_idx, boolean required) {
        this(display_name, variable_idx, required, null, null, null);
    }    
    
    public ImportVariable(String display_name, int variable_idx, boolean required, String help_text, String default_value, String linelist_meta) {
        _variable_required = required;
        _variable_name = display_name;
        _variable_index = variable_idx;
        if (!_variable_required) {
            _help_text = "optional";
        } else if (_help_text == null) {
            _help_text = help_text;
        }
        _default = default_value;
        _linelist_meta = linelist_meta;
    }

    @Override
    public Object clone() {
        try {
            ImportVariable newObject = (ImportVariable) super.clone();
            newObject._variable_name = new String(_variable_name);
            newObject._help_text = new String(_help_text);
            newObject._default = _default == null ? _default : new String(_default);
            return newObject;
        } catch (CloneNotSupportedException e) {
            throw new InternalError("But we are Cloneable!!!");
        }
    }

    public boolean getShowing() {
        return _showing;
    }

    public boolean setShowing(boolean b) {
        _showing = b;
        return _showing;
    }
    
    public final String getDefault() {
        return _default;
    }
    
    public final boolean hasDefault() {
        return _default != null;
    }    
    
    public void setSourceFieldIndex(int idx) {
        _source_field_index = idx;
    }

    public int getSourceFieldIndex() {
        return _source_field_index;
    }

    public boolean isMappedToSourceField() {
        return _source_field_index > 0;
    }

    public boolean getIsRequiredField() {
        return _variable_required;
    }

    public void setVariableIndex(int idx) {
        _variable_index = idx;
    }
        
    public int getVariableIndex() {
        return _variable_index;
    }

    public String getDisplayLabel() {
        StringBuilder builder = new StringBuilder();
        builder.append(_variable_name);
        if (_help_text != null && _help_text.length() > 0) {
            builder.append(" (").append(_help_text).append(")");
        }
        return builder.toString();
    }

    public final String getVariableName() {
        return _variable_name;
    }
    
    public final String getLinelistMeta() {
        return _linelist_meta;
    }
}
