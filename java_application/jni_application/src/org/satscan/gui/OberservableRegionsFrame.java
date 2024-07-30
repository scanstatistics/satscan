/*
 * OberservableRegionsFrame.java
 *
 * Created on May 9, 2008, 1:34 PM
 */

package org.satscan.gui;

import java.util.StringTokenizer;
import java.util.Vector;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import javax.swing.DefaultListModel;
import javax.swing.ImageIcon;
import javax.swing.JRootPane;
import javax.swing.event.UndoableEditEvent;
import javax.swing.event.UndoableEditListener;
import javax.swing.undo.UndoManager;
import org.satscan.app.Parameters;
import org.satscan.app.RegionFeaturesException;
import org.satscan.gui.utils.Utils;

/**
 *
 * @author  Hostovic
 */
public class OberservableRegionsFrame extends ModalInternalFrame {
        private final UndoManager undo = new UndoManager();
        private DefaultListModel _regionsListModel = new DefaultListModel();
        private DefaultListModel _inequalitiesListModel = new DefaultListModel();
        private Vector< Vector<String> > _regionInequalities = new Vector< Vector<String> >();
        private Parameters _parameters;
        private final String _defaultInequality = "y >= x + 1";
        private final String _regionLabel = "Polygon";
        private final int _Y = 0;
        private final int _X = 1;
        private final int _plusSignIndex = 0;
        private final int _minusSignIndex = 1;
    
    /** Creates new form OberservableRegionsFrame */
    public OberservableRegionsFrame(final JRootPane rootPane, final ParameterSettingsFrame analysisSettingsWindow, final Parameters parameters) {
        super(rootPane);
        initComponents();
        setFrameIcon(new ImageIcon(getClass().getResource("/SaTScan.png")));
        _regionsListModel.removeAllElements();
        _inequalitiesListModel.removeAllElements();
        enableInequalityEditor();
        SetupInterface(parameters);
        enableRegionEditButtons();
        enableInequalityEditButtons();
        positionRootPaneCenter();
    }
    
    /** Set up interface from parameter settings. */
    private void SetupInterface(final Parameters parameters) {
        _regionsListModel.clear();
        _regionInequalities.clear();
        for (int i=0; i < parameters.GetObservableRegions().size(); ++i) {
           _regionsListModel.addElement(_regionLabel + " " + Integer.toString(i + 1));
           StringTokenizer t = new StringTokenizer(parameters.GetObservableRegions().get(i),",");
           _regionInequalities.add(new Vector<String>());
           while (t.hasMoreTokens()) {
                _regionInequalities.lastElement().add(t.nextToken().trim().toUpperCase());
           }
        }
        if (_regionInequalities.size() > 0) {
            _regionsList.getSelectionModel().setSelectionInterval(0, 0);
        }
    }   

    /** Save parameter settings to Parameters object. */
    public void saveParameterSettings(Parameters parameters) {
        parameters.ClearObservableRegions();
        for (int i=0; i < _regionInequalities.size(); ++i) {
            String inequalitiesString = "";
            for (int j=0; j < _regionInequalities.get(i).size(); ++j) {
                if (j != 0) inequalitiesString += ",";
                inequalitiesString += _regionInequalities.get(i).get(j);
            }
            parameters.AddObservableRegion(inequalitiesString, i, false);
        }
    }
    
    /** Adds new region of collection of regions. */
    private void addRegion() {
        //create new vector of strings for new regions inequalities
        _regionInequalities.add(new Vector<String>());
        //add default inequality to new region
        _regionInequalities.lastElement().add(_defaultInequality.toUpperCase());
        //add label for new region to regions listbox model
        _regionsListModel.addElement(_regionLabel + " " + Integer.toString(_regionsList.getModel().getSize() + 1));
        //add label for new inequality to inequality listbox model
        _inequalitiesListModel.addElement(_defaultInequality.toUpperCase());
        //select new region and inequality in listboxes
        _regionsList.getSelectionModel().setSelectionInterval(0, _regionsListModel.size() - 1);
        _inequalitiesList.getSelectionModel().setSelectionInterval(0, _inequalitiesList.getModel().getSize() - 1);
    }
    
    /** Deletes selected region from of collection of regions. */
    private void deleteRegion() {
        if (!_regionsList.getSelectionModel().isSelectionEmpty()) {
            int ideleteIndex = _regionsList.getSelectionModel().getMinSelectionIndex();
            for (int i=ideleteIndex+1; i < _regionsList.getModel().getSize() ;i++) {
                String s = (String)_regionsListModel.getElementAt(i);
                s = _regionLabel + " " + Integer.toString(i);
                _regionsListModel.setElementAt(s, i);
            }
            // remove collection of inequalities for selected region
           _regionInequalities.remove(ideleteIndex);
           // remove list box name
          _regionsListModel.remove(ideleteIndex);    
          if (_regionsListModel.size() > 0)
            _regionsList.getSelectionModel().setSelectionInterval(0, Math.max(0, ideleteIndex - 1));  
        }
    }
    
    /**
     * Public method to set selected region in region list box.
     */
    public void selectRegionAtIndex(int index) {
        if (index < _regionsList.getModel().getSize())
            _regionsList.getSelectionModel().setSelectionInterval(0, index);
    }
    
    /** Refreshes inequalities listbox for selected region. */
    private void showSelectedRegionInequalities() {
        _inequalitiesListModel.clear();
        if (!_regionsList.getSelectionModel().isSelectionEmpty()) {
            int index = _regionsList.getSelectionModel().getMinSelectionIndex();
           _inequalitiesForRegion.setText("Polygon " + (index + 1) +" Inequalities:");
            for (int i=0; index != -1 && i < _regionInequalities.get(index).size(); ++i) {
                _inequalitiesListModel.addElement(_regionInequalities.get(index).get(i));
            }
            if (!_inequalitiesListModel.isEmpty()) {
                _inequalitiesList.getSelectionModel().setSelectionInterval(0, 0);
            }
        } else {
          _inequalitiesForRegion.setText("Polygon Inequalities:");
        }
    }
    
    /** Enables buttons associated with regions list. */
    private void enableRegionEditButtons() {
        _addRegionButton.setEnabled(true);
        _deleteRegionButton.setEnabled(!_regionsList.getSelectionModel().isSelectionEmpty());
    }

    /** Enables buttons associated with inequalities list. */
    private void enableInequalityEditButtons() {
        _addInequalityButton.setEnabled(!_regionsList.getSelectionModel().isSelectionEmpty());
        _deleteInequalityButton.setEnabled(!_inequalitiesList.getSelectionModel().isSelectionEmpty());        
    }

    /** Enables controls of inequality editor. */
    private void enableInequalityEditor() {
        boolean bEnable = !_inequalitiesList.getSelectionModel().isSelectionEmpty();
        _inequalitiesPanel.setEnabled(bEnable);
        _inequalitiesPanel.setEnabled(bEnable);       
        _ycoefficientTextField.setEnabled(bEnable);
        _variableSelectComboBox.setEnabled(bEnable);
        _inequalitySelectComboBox.setEnabled(bEnable);
        _plusMinusComboBox.setEnabled(bEnable);
        _interceptTextField.setEnabled(bEnable);
        _updateInequalityButton.setEnabled(bEnable);
        _xcoefficientTextField.setEnabled(bEnable && _variableSelectComboBox.getSelectedIndex() != 1);
        _xcoefficientLabel.setEnabled(bEnable && _variableSelectComboBox.getSelectedIndex() != 1);
    }
    
    /** Updates selected inequality with inequality editor controls. */
    private void updateInequality() {
        StringBuilder inequality = new StringBuilder();
        if (Double.parseDouble(_ycoefficientTextField.getText()) != 1)
            inequality.append(_ycoefficientTextField.getText());
        inequality.append(((String)_variableSelectComboBox.getSelectedItem()).toLowerCase()).append(" ");
        inequality.append(((String)_inequalitySelectComboBox.getSelectedItem())).append(" ");
        boolean hasX = false;
        if (_xcoefficientTextField.isEnabled() && Double.parseDouble(_xcoefficientTextField.getText()) != 0) {
            if (Double.parseDouble(_xcoefficientTextField.getText()) != 1)
               inequality.append(_xcoefficientTextField.getText());
            inequality.append("x "); 
            hasX = true;
        }
        if (hasX == false) {
            if (_plusMinusComboBox.getSelectedIndex() == _minusSignIndex)
                inequality.append("-");
            inequality.append(_interceptTextField.getText()); 
        }
        else if (Double.parseDouble(_interceptTextField.getText()) != 0) {
            inequality.append(((String)_plusMinusComboBox.getSelectedItem())).append(" ");
            inequality.append(_interceptTextField.getText()); 
        }
                
        //update regions inequalities
        int selectedRegionIndex = _regionsList.getSelectionModel().getMinSelectionIndex();
        int selectedInequalityIndex = _inequalitiesList.getSelectionModel().getMinSelectionIndex();
        if (selectedInequalityIndex == -1) {
            //append inequality to regions collection
            _regionInequalities.get(selectedRegionIndex).add(inequality.toString().toUpperCase());
            _inequalitiesListModel.addElement(inequality.toString().toUpperCase());
        } else {
            //update inequality in regions collection
            _regionInequalities.get(selectedRegionIndex).set(selectedInequalityIndex, inequality.toString().toUpperCase());
            _inequalitiesListModel.setElementAt(inequality.toString().toUpperCase(), selectedInequalityIndex);
        }
    }
    
    /** Adds new inequality to selected region. */
    private void addNewInequalityToRegion() {
        if (!_regionsList.getSelectionModel().isSelectionEmpty()) {
            int selectionIndex = _regionsList.getSelectionModel().getMinSelectionIndex();
            //add default inequality to region vector for selected region
            _regionInequalities.get(selectionIndex).add(_defaultInequality.toUpperCase());
            //add default inequality to inequality listbox
            _inequalitiesListModel.addElement(_defaultInequality.toUpperCase()); 
            //set new inequality as selected
            _inequalitiesList.getSelectionModel().setSelectionInterval(0, _inequalitiesListModel.size() - 1);
        }
    }

    /** Deletes inequality from selected region. */
    private void deleteInequalityFromRegion() {
        if (!_inequalitiesList.getSelectionModel().isSelectionEmpty()) {
            int selectedRegionIndex = _regionsList.getSelectionModel().getMinSelectionIndex();
            int selectedInequalityIndex = _inequalitiesList.getSelectionModel().getMinSelectionIndex();
            //remove inequality from listbox
            _inequalitiesListModel.remove(selectedInequalityIndex);
            //remove inequality from regions vector
            _regionInequalities.get(selectedRegionIndex).remove(selectedInequalityIndex);
            //set previous inequality as selected in listbox
            if (_inequalitiesList.getModel().getSize() > 0)
                _inequalitiesList.getSelectionModel().setSelectionInterval(0, Math.max(0, selectedInequalityIndex - 1));
        }
    }
    
    /** Assigns controls of inequality editor from selected item in equalities list. */
    private void assignInequalityEditor() {
        if (_inequalitiesList.getSelectionModel().getMinSelectionIndex() != -1) {
            Pattern reYFull = Pattern.compile("^(-?\\d+\\.\\d+|-?\\d+)?y\\s*(<=|>=)\\s*(-?\\d+\\.\\d+|-?\\d+)?x\\s*(\\+|\\-)?\\s*(\\d+\\.\\d+|\\d+)*$", Pattern.CASE_INSENSITIVE);
            Pattern reYConst = Pattern.compile("^(-?\\d+\\.\\d+|-?\\d+)?y\\s*(<=|>=)\\s*(-?\\d+\\.\\d+|-?\\d+)+$", Pattern.CASE_INSENSITIVE);
            Pattern reXConst = Pattern.compile("^(-?\\d+\\.\\d+|-?\\d+)?x\\s*(<=|>=)\\s*(-?\\d+\\.\\d+|-?\\d+)+$", Pattern.CASE_INSENSITIVE);
            String value = (String)_inequalitiesList.getModel().getElementAt(_inequalitiesList.getSelectionModel().getMinSelectionIndex());
            Matcher reYFullMatcher = reYFull.matcher(value);
            Matcher reYConstMatcher = reYConst.matcher(value);
            Matcher reXConstMatcher = reXConst.matcher(value);

            //determine which pattern this inequality matches
            if (reYFullMatcher.matches()) {
                // get Y coefficient
                _variableSelectComboBox.select(0);
                _ycoefficientTextField.setText(reYFullMatcher.group(1) != null ? reYFullMatcher.group(1) : "1");
                // get equality sign
                _inequalitySelectComboBox.select(reYFullMatcher.group(2).equals(">=") ? 0 : 1);
                // get X coefficient
                _xcoefficientTextField.setText(reYFullMatcher.group(3) != null ? reYFullMatcher.group(3) : "1");
                // get operator
                if (reYFullMatcher.group(4) != null) {
                    _plusMinusComboBox.select(reYFullMatcher.group(4).equals("+") ? 0 : 1);
                }
                // get constant
                _interceptTextField.setText(reYFullMatcher.group(5) != null ? reYFullMatcher.group(5) : "0");
            } else if (reYConstMatcher.matches()) {
                // get Y coefficient
                _variableSelectComboBox.select(0);
                _ycoefficientTextField.setText(reYConstMatcher.group(1) != null ? reYConstMatcher.group(1) : "1");
                // get equality sign
                _inequalitySelectComboBox.select(reYConstMatcher.group(2).equals(">=") ? 0 : 1);
                _xcoefficientTextField.setText("0");
                double inter = Double.parseDouble(reYConstMatcher.group(3));
                _plusMinusComboBox.select(inter >= 0 ? 0 : 1);
                // get constant
                _interceptTextField.setText(reYConstMatcher.group(3).replace("-", ""));
            } else if (reXConstMatcher.matches()) {
                // get X coefficient
                _variableSelectComboBox.select(1);
                _ycoefficientTextField.setText(reXConstMatcher.group(1) != null ? reXConstMatcher.group(1) : "1");
                // get equality sign
                double inter = Double.parseDouble(reXConstMatcher.group(3));
                _plusMinusComboBox.select(inter >= 0 ? 0 : 1);
                _inequalitySelectComboBox.select(reXConstMatcher.group(2).equals(">=") ? 0 : 1);
                _xcoefficientTextField.setText("0");
                // get constant
                _interceptTextField.setText(reXConstMatcher.group(3).replace("-", ""));
            } else {/*throw new RuntimeException("Inequality could not be matched: " + value );*/}
        }
    }

    /** Returns whether any regions are defined.  */
    public boolean getHasRegionsDefined() {
        return _regionInequalities.size() > 0;
    }    
    
    public native String _getRegionIsValid( String region );    
    public native String _getRegionIsValidRegionCollection( Vector<String> regions);    
    
    /** Validates all the settings in this dialog. */
    public void Validate() {
        Vector<String> regions = new Vector<String>();
        
        for (int i=0; i < _regionInequalities.size(); ++i) {
            String inequalitiesString = "";
            for (int j=0; j < _regionInequalities.get(i).size(); ++j) {
                if (j != 0) inequalitiesString += ",";
                inequalitiesString += _regionInequalities.get(i).get(j);
            }
            //check that region defines validate polygon
            String response = _getRegionIsValid(inequalitiesString);
            if (!response.equals("Ok"))
               throw new RegionFeaturesException(response, i);

            regions.add(inequalitiesString);
        }        
        //check that region defines validate polygon
        if (regions.size() == 0)
           throw new RegionFeaturesException("At least one polygon must be defined for a Homogeneous Poisson model.");        
            
        String response = _getRegionIsValidRegionCollection(regions);
        if (!response.equals("Ok"))
           throw new RegionFeaturesException(response);        
    }   
    
    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
    // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
    private void initComponents() {

        _closeButton = new javax.swing.JButton();
        _regionsPanel = new javax.swing.JPanel();
        jPanel1 = new javax.swing.JPanel();
        jLabel1 = new javax.swing.JLabel();
        jScrollPane1 = new javax.swing.JScrollPane();
        _regionsList = new javax.swing.JList(_regionsListModel);
        _addRegionButton = new javax.swing.JButton();
        _deleteRegionButton = new javax.swing.JButton();
        jSeparator1 = new javax.swing.JSeparator();
        jPanel2 = new javax.swing.JPanel();
        _inequalitiesForRegion = new javax.swing.JLabel();
        jScrollPane2 = new javax.swing.JScrollPane();
        _inequalitiesList = new javax.swing.JList(_inequalitiesListModel);
        _addInequalityButton = new javax.swing.JButton();
        _deleteInequalityButton = new javax.swing.JButton();
        _inequalitiesPanel = new javax.swing.JPanel();
        _ycoefficientTextField = new javax.swing.JTextField();
        _ycoefficientTextField.addKeyListener(new java.awt.event.KeyAdapter() {
            public void keyTyped(java.awt.event.KeyEvent e) {
                Utils.validateFloatKeyTyped(_ycoefficientTextField, e, 10);
            }
        });
        _ycoefficientTextField.addFocusListener(new java.awt.event.FocusAdapter() {
            public void focusLost(java.awt.event.FocusEvent e) {
                while (_ycoefficientTextField.getText().length() == 0 ||
                    Double.parseDouble(_ycoefficientTextField.getText()) == 0) {
                    if (undo.canUndo()) undo.undo(); else _ycoefficientTextField.setText("1");
                }
            }
        });
        _ycoefficientTextField.getDocument().addUndoableEditListener(new UndoableEditListener() {
            public void undoableEditHappened(UndoableEditEvent evt) {
                undo.addEdit(evt.getEdit());
            }
        });
        _xcoefficientTextField = new javax.swing.JTextField();
        _xcoefficientTextField.addKeyListener(new java.awt.event.KeyAdapter() {
            public void keyTyped(java.awt.event.KeyEvent e) {
                Utils.validateFloatKeyTyped(_xcoefficientTextField, e, 10);
            }
        });
        _xcoefficientTextField.addFocusListener(new java.awt.event.FocusAdapter() {
            public void focusLost(java.awt.event.FocusEvent e) {
                while (_xcoefficientTextField.getText().length() == 0) {
                    if (undo.canUndo()) undo.undo(); else _xcoefficientTextField.setText("1");
                }
            }
        });
        _xcoefficientTextField.getDocument().addUndoableEditListener(new UndoableEditListener() {
            public void undoableEditHappened(UndoableEditEvent evt) {
                undo.addEdit(evt.getEdit());
            }
        });
        _xcoefficientLabel = new javax.swing.JLabel();
        _interceptTextField = new javax.swing.JTextField();
        _interceptTextField.addKeyListener(new java.awt.event.KeyAdapter() {
            public void keyTyped(java.awt.event.KeyEvent e) {
                Utils.validatePostiveFloatKeyTyped(_interceptTextField, e, 10);
            }
        });
        _interceptTextField.addFocusListener(new java.awt.event.FocusAdapter() {
            public void focusLost(java.awt.event.FocusEvent e) {
                while (_interceptTextField.getText().length() == 0) {
                    if (undo.canUndo()) undo.undo(); else _interceptTextField.setText("1");
                }
            }
        });
        _interceptTextField.getDocument().addUndoableEditListener(new UndoableEditListener() {
            public void undoableEditHappened(UndoableEditEvent evt) {
                undo.addEdit(evt.getEdit());
            }
        });
        _updateInequalityButton = new javax.swing.JButton();
        _variableSelectComboBox = new java.awt.Choice();
        _inequalitySelectComboBox = new java.awt.Choice();
        _plusMinusComboBox = new java.awt.Choice();

        setTitle("Polygons");

        _closeButton.setText("Close"); // NOI18N
        _closeButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                setVisible(false);
            }
        });
        _closeButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                _closeButtonActionPerformed(evt);
            }
        });

        _regionsPanel.setBorder(javax.swing.BorderFactory.createTitledBorder("Bounding Regions:"));

        jLabel1.setLabelFor(_regionsList);
        jLabel1.setText("Polygons:");

        _regionsList.setSelectionMode(javax.swing.ListSelectionModel.SINGLE_SELECTION);
        _regionsList.addListSelectionListener(new javax.swing.event.ListSelectionListener() {
            public void valueChanged(javax.swing.event.ListSelectionEvent e) {
                enableRegionEditButtons();
                showSelectedRegionInequalities();
                enableInequalityEditor();
            }
        });
        jScrollPane1.setViewportView(_regionsList);
        _regionsList.getAccessibleContext().setAccessibleName("Polygons");
        _regionsList.getAccessibleContext().setAccessibleDescription("");

        _addRegionButton.setText("Add");
        _addRegionButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                addRegion();
            }
        });

        _deleteRegionButton.setText("Delete");
        _deleteRegionButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                deleteRegion();
            }
        });

        jSeparator1.setOrientation(javax.swing.SwingConstants.VERTICAL);

        javax.swing.GroupLayout jPanel1Layout = new javax.swing.GroupLayout(jPanel1);
        jPanel1.setLayout(jPanel1Layout);
        jPanel1Layout.setHorizontalGroup(
            jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel1Layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(jPanel1Layout.createSequentialGroup()
                        .addComponent(jScrollPane1, javax.swing.GroupLayout.PREFERRED_SIZE, 115, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addGroup(jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                            .addComponent(_addRegionButton, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                            .addComponent(_deleteRegionButton, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(jSeparator1))
                    .addComponent(jLabel1))
                .addContainerGap())
        );
        jPanel1Layout.setVerticalGroup(
            jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, jPanel1Layout.createSequentialGroup()
                .addGroup(jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.TRAILING)
                    .addGroup(jPanel1Layout.createSequentialGroup()
                        .addGap(11, 11, 11)
                        .addComponent(jSeparator1, javax.swing.GroupLayout.DEFAULT_SIZE, 150, Short.MAX_VALUE))
                    .addGroup(jPanel1Layout.createSequentialGroup()
                        .addComponent(jLabel1)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addGroup(jPanel1Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                            .addGroup(jPanel1Layout.createSequentialGroup()
                                .addComponent(_addRegionButton)
                                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                                .addComponent(_deleteRegionButton)
                                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED, 89, Short.MAX_VALUE))
                            .addComponent(jScrollPane1, javax.swing.GroupLayout.DEFAULT_SIZE, 141, Short.MAX_VALUE))))
                .addContainerGap())
        );

        _addRegionButton.getAccessibleContext().setAccessibleName("Add Polygon");
        _deleteRegionButton.getAccessibleContext().setAccessibleName("Delete Polygon");

        _inequalitiesForRegion.setLabelFor(_inequalitiesList);
        _inequalitiesForRegion.setText("Region 1 Inequalities:");

        _inequalitiesList.setSelectionMode(javax.swing.ListSelectionModel.SINGLE_SELECTION);
        _inequalitiesList.addListSelectionListener(new javax.swing.event.ListSelectionListener() {
            public void valueChanged(javax.swing.event.ListSelectionEvent e) {
                enableInequalityEditButtons();
                assignInequalityEditor();
                enableInequalityEditor();
            }
        });
        jScrollPane2.setViewportView(_inequalitiesList);

        _addInequalityButton.setText("Add");
        _addInequalityButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                addNewInequalityToRegion();
            }
        });

        _deleteInequalityButton.setText("Delete");
        _deleteInequalityButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                deleteInequalityFromRegion();
            }
        });

        javax.swing.GroupLayout jPanel2Layout = new javax.swing.GroupLayout(jPanel2);
        jPanel2.setLayout(jPanel2Layout);
        jPanel2Layout.setHorizontalGroup(
            jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel2Layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, jPanel2Layout.createSequentialGroup()
                        .addComponent(jScrollPane2, javax.swing.GroupLayout.DEFAULT_SIZE, 193, Short.MAX_VALUE)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addGroup(jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING, false)
                            .addComponent(_addInequalityButton, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                            .addComponent(_deleteInequalityButton)))
                    .addComponent(_inequalitiesForRegion))
                .addContainerGap())
        );
        jPanel2Layout.setVerticalGroup(
            jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(jPanel2Layout.createSequentialGroup()
                .addComponent(_inequalitiesForRegion)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addGroup(jPanel2Layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(jPanel2Layout.createSequentialGroup()
                        .addComponent(_addInequalityButton)
                        .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                        .addComponent(_deleteInequalityButton))
                    .addComponent(jScrollPane2, javax.swing.GroupLayout.DEFAULT_SIZE, 130, Short.MAX_VALUE))
                .addContainerGap())
        );

        _addInequalityButton.getAccessibleContext().setAccessibleName("Add Inequality");
        _deleteInequalityButton.getAccessibleContext().setAccessibleName("Delete Inequality");

        javax.swing.GroupLayout _regionsPanelLayout = new javax.swing.GroupLayout(_regionsPanel);
        _regionsPanel.setLayout(_regionsPanelLayout);
        _regionsPanelLayout.setHorizontalGroup(
            _regionsPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_regionsPanelLayout.createSequentialGroup()
                .addComponent(jPanel1, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(jPanel2, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .addContainerGap())
        );
        _regionsPanelLayout.setVerticalGroup(
            _regionsPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addComponent(jPanel1, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
            .addComponent(jPanel2, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
        );

        _inequalitiesPanel.setBorder(javax.swing.BorderFactory.createTitledBorder("Inequality Editor"));

        _ycoefficientTextField.setText("1");

        _xcoefficientTextField.setText("1");

        _xcoefficientLabel.setText("X");

        _interceptTextField.setText("1");

        _updateInequalityButton.setText("Update");
        _updateInequalityButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent e) {
                updateInequality();
                /*EnableDataSetList(true);
                EnableDataSetPurposeControls(true);
                _dataSetsListModel.addElement("Data Set " + Integer.toString(_inputDataSetsList.getModel().getSize() + 1));

                // enable and clear the edit boxes
                EnableInputFileEdits(true);
                _caseFilenames.addElement("");
                _controlFilenames.addElement("");
                _populationFilenames.addElement("");
                _inputDataSetsList.setSelectedIndex(_dataSetsListModel.getSize() - 1);
                _inputDataSetsList.ensureIndexIsVisible(_dataSetsListModel.getSize() - 1);
                _caseFileTextField.setText("");
                _controlFileTextField.setText("");
                _populationFileTextField.setText("");
                _caseFileTextField.requestFocusInWindow();
                EnableNewButton();
                EnableRemoveButton();
                enableSetDefaultsButton();*/
            }
        });
        _updateInequalityButton.addActionListener(new java.awt.event.ActionListener() {
            public void actionPerformed(java.awt.event.ActionEvent evt) {
                _updateInequalityButtonActionPerformed(evt);
            }
        });

        _variableSelectComboBox.addItemListener(new java.awt.event.ItemListener() {
            public void itemStateChanged(java.awt.event.ItemEvent e) {
                _xcoefficientTextField.setEnabled(_variableSelectComboBox.getSelectedIndex() != 1);
                _xcoefficientLabel.setEnabled(_variableSelectComboBox.getSelectedIndex() != 1);
            }
        });
        _variableSelectComboBox.add("Y");
        _variableSelectComboBox.add("X");

        _inequalitySelectComboBox.add(">=");
        _inequalitySelectComboBox.add("<=");

        _plusMinusComboBox.add("+");
        _plusMinusComboBox.add("-");

        javax.swing.GroupLayout _inequalitiesPanelLayout = new javax.swing.GroupLayout(_inequalitiesPanel);
        _inequalitiesPanel.setLayout(_inequalitiesPanelLayout);
        _inequalitiesPanelLayout.setHorizontalGroup(
            _inequalitiesPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_inequalitiesPanelLayout.createSequentialGroup()
                .addContainerGap()
                .addComponent(_ycoefficientTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 50, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_variableSelectComboBox, javax.swing.GroupLayout.PREFERRED_SIZE, 60, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_inequalitySelectComboBox, javax.swing.GroupLayout.PREFERRED_SIZE, 60, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addGap(25, 25, 25)
                .addComponent(_xcoefficientTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 50, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_xcoefficientLabel)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_plusMinusComboBox, javax.swing.GroupLayout.PREFERRED_SIZE, 50, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_interceptTextField, javax.swing.GroupLayout.PREFERRED_SIZE, 50, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.UNRELATED)
                .addComponent(_updateInequalityButton, javax.swing.GroupLayout.PREFERRED_SIZE, 81, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addContainerGap(14, Short.MAX_VALUE))
        );
        _inequalitiesPanelLayout.setVerticalGroup(
            _inequalitiesPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(_inequalitiesPanelLayout.createSequentialGroup()
                .addComponent(_inequalitySelectComboBox, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                .addGap(40, 40, 40))
            .addGroup(_inequalitiesPanelLayout.createSequentialGroup()
                .addGroup(_inequalitiesPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addGroup(_inequalitiesPanelLayout.createParallelGroup(javax.swing.GroupLayout.Alignment.BASELINE)
                        .addComponent(_ycoefficientTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addComponent(_xcoefficientTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addComponent(_xcoefficientLabel)
                        .addComponent(_interceptTextField, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                        .addComponent(_updateInequalityButton))
                    .addComponent(_plusMinusComboBox, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                    .addComponent(_variableSelectComboBox, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addContainerGap(javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE))
        );

        _ycoefficientTextField.getAccessibleContext().setAccessibleName("Inequality Coeficient");
        _xcoefficientTextField.getAccessibleContext().setAccessibleDescription("X coeficient");
        _interceptTextField.getAccessibleContext().setAccessibleDescription("Intercept value");
        _variableSelectComboBox.getAccessibleContext().setAccessibleDescription("Variable selection");
        _inequalitySelectComboBox.getAccessibleContext().setAccessibleDescription("Inequality selection");
        _plusMinusComboBox.getAccessibleContext().setAccessibleDescription("Operator selection");

        javax.swing.GroupLayout layout = new javax.swing.GroupLayout(getContentPane());
        getContentPane().setLayout(layout);
        layout.setHorizontalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(layout.createSequentialGroup()
                .addContainerGap()
                .addGroup(layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
                    .addComponent(_inequalitiesPanel, javax.swing.GroupLayout.Alignment.TRAILING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(_regionsPanel, javax.swing.GroupLayout.Alignment.TRAILING, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, Short.MAX_VALUE)
                    .addComponent(_closeButton, javax.swing.GroupLayout.Alignment.TRAILING, javax.swing.GroupLayout.PREFERRED_SIZE, 101, javax.swing.GroupLayout.PREFERRED_SIZE))
                .addContainerGap())
        );
        layout.setVerticalGroup(
            layout.createParallelGroup(javax.swing.GroupLayout.Alignment.LEADING)
            .addGroup(javax.swing.GroupLayout.Alignment.TRAILING, layout.createSequentialGroup()
                .addContainerGap()
                .addComponent(_regionsPanel, javax.swing.GroupLayout.PREFERRED_SIZE, javax.swing.GroupLayout.DEFAULT_SIZE, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED)
                .addComponent(_inequalitiesPanel, javax.swing.GroupLayout.PREFERRED_SIZE, 63, javax.swing.GroupLayout.PREFERRED_SIZE)
                .addPreferredGap(javax.swing.LayoutStyle.ComponentPlacement.RELATED, 48, Short.MAX_VALUE)
                .addComponent(_closeButton)
                .addContainerGap())
        );

        pack();
    }// </editor-fold>//GEN-END:initComponents

    private void _closeButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event__closeButtonActionPerformed
        // TODO add your handling code here:
    }//GEN-LAST:event__closeButtonActionPerformed

    private void _updateInequalityButtonActionPerformed(java.awt.event.ActionEvent evt) {//GEN-FIRST:event__updateInequalityButtonActionPerformed
}//GEN-LAST:event__updateInequalityButtonActionPerformed
    
    
    // Variables declaration - do not modify//GEN-BEGIN:variables
    private javax.swing.JButton _addInequalityButton;
    private javax.swing.JButton _addRegionButton;
    private javax.swing.JButton _closeButton;
    private javax.swing.JButton _deleteInequalityButton;
    private javax.swing.JButton _deleteRegionButton;
    private javax.swing.JLabel _inequalitiesForRegion;
    private javax.swing.JList _inequalitiesList;
    private javax.swing.JPanel _inequalitiesPanel;
    private java.awt.Choice _inequalitySelectComboBox;
    private javax.swing.JTextField _interceptTextField;
    private java.awt.Choice _plusMinusComboBox;
    private javax.swing.JList _regionsList;
    private javax.swing.JPanel _regionsPanel;
    private javax.swing.JButton _updateInequalityButton;
    private java.awt.Choice _variableSelectComboBox;
    private javax.swing.JLabel _xcoefficientLabel;
    private javax.swing.JTextField _xcoefficientTextField;
    private javax.swing.JTextField _ycoefficientTextField;
    private javax.swing.JLabel jLabel1;
    private javax.swing.JPanel jPanel1;
    private javax.swing.JPanel jPanel2;
    private javax.swing.JScrollPane jScrollPane1;
    private javax.swing.JScrollPane jScrollPane2;
    private javax.swing.JSeparator jSeparator1;
    // End of variables declaration//GEN-END:variables
    
}
