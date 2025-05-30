/*
 * ParameterHistory.java
 *
 * Created on December 11, 2007, 1:51 PM
 *
 * To change this template, choose Tools | Template Manager
 * and open the template in the editor.
 */
package org.satscan.app;

import java.io.File;
import java.util.Vector;
import java.util.prefs.Preferences;

/**
 *
 * @author Hostovic
 */
public class ParameterHistory {

    private static final String _parameterNameProperty = "Parameter";
    private static final int _maxListSize = 10;
    private Preferences _prefs = Preferences.userNodeForPackage(getClass());
    private Vector<File> _parameterHistory = new Vector<File>();
    private static final ParameterHistory _instance = new ParameterHistory();

    /** Creates a new instance of ParameterHistory */
    public ParameterHistory() {
        ReadParametersHistory();
    }

    public static ParameterHistory getInstance() {
        return _instance;
    }

    public Vector<File> getHistoryList() {
        return _parameterHistory;
    }

    public void AddParameterToHistory(String parameterFileName) {
        if (parameterFileName.length() == 0) {
            return;
        }

        File f = new File(parameterFileName);

        if (_parameterHistory.size() == 0) {
            _parameterHistory.add(f);
        } else if (!_parameterHistory.firstElement().equals(f)) {
            for (int i = 1; i < _parameterHistory.size(); ++i) {
                if (_parameterHistory.get(i).equals(f)) {
                    _parameterHistory.remove(_parameterHistory.get(i));
                    break;
                }
            }
            if (_parameterHistory.size() == _maxListSize) {
                _parameterHistory.remove(_parameterHistory.lastElement());
            }
            _parameterHistory.insertElementAt(f, 0);
        }
        WriteParametersHistory();
    }

    /** reads parameter history from ini file */
    public void ReadParametersHistory() {
        String keyName;

        _parameterHistory.removeAllElements();
        for (int i = 0; i < _maxListSize; ++i) {
            keyName = _parameterNameProperty + i;
            String value = _prefs.get(keyName, "");
            if (value.length() > 0) {
                _parameterHistory.add(new File(value));
            }
        }
    }

    /** Writes parameter history to ini file */
    public void WriteParametersHistory() {
        String keyName;

        for (int i = 0; i < _parameterHistory.size(); ++i) {
            keyName = _parameterNameProperty + i;
            String s = _parameterHistory.get(i).getAbsolutePath();
            _prefs.put(keyName, s);
        }
    }
}
