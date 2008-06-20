/*
 * Utils.java
 *
 * Created on December 12, 2007, 9:14 AM
 *
 * To change this template, choose Tools | Template Manager
 * and open the template in the editor.
 */
package org.satscan.gui.utils;

import java.util.Calendar;
import java.util.GregorianCalendar;
import javax.swing.JTextField;
import javax.swing.undo.UndoManager;
import org.satscan.app.AppConstants;

/**
 * Utilities class for graphical controls.
 * @author Hostovic
 */
public class Utils {

    /**
     * Validates that key typed is a positive integer or back space; otherwise consumes key.
     */
    public static void validatePostiveNumericKeyTyped(JTextField thisField, java.awt.event.KeyEvent e, int maxFieldLength) {
        if (!Character.isDigit(e.getKeyChar()) && e.getKeyCode() != java.awt.event.KeyEvent.VK_BACK_SPACE) {
            e.consume();
        }
        if (thisField.getSelectedText() == null && thisField.getText().length() >= maxFieldLength) {
            e.consume();
        }
    }

    /**
     * Validates that key typed is a positive integer, period or back space; otherwise consumes key.
     */
    public static void validatePostiveFloatKeyTyped(JTextField thisField, java.awt.event.KeyEvent e, int maxFieldLength) {
        if (!(Character.isDigit(e.getKeyChar()) || e.getKeyChar() == '\b' || e.getKeyChar() == '.')) {
            e.consume();
            return;
        }
        if (thisField.getSelectedText() == null && thisField.getText().length() >= maxFieldLength) {
            e.consume();
        }
    }

    /**
     * Validates that key typed is a positive integer, period, minus or back space; otherwise consumes key.
     */
    public static void validateFloatKeyTyped(JTextField thisField, java.awt.event.KeyEvent e, int maxFieldLength) {
        if (!(Character.isDigit(e.getKeyChar()) || e.getKeyChar() == '\b' || e.getKeyChar() == '-' || e.getKeyChar() == '.')) {
            e.consume();
            return;
        }
        if (thisField.getSelectedText() == null && thisField.getText().length() >= maxFieldLength) {
            e.consume();
        }
    }    
    
    /**
     * validates date controls represented by three passed edit controls - prevents an invalid date
     */
    public static void validateDateControlGroup(JTextField YearControl, JTextField MonthControl, JTextField DayControl, UndoManager undo) {
        GregorianCalendar thisCalender = new GregorianCalendar();

        //first check year
        if (YearControl.getText().length() == 0) {
            if (undo.canUndo()) {
                undo.undo();
            } else {
                YearControl.setText(Integer.toString(AppConstants.MAX_YEAR));
            }
        }//YearControl.Undo();
        else {
            //set year to a valid setting if out of valid range
            if (Integer.parseInt(YearControl.getText()) < AppConstants.MIN_YEAR) {
                YearControl.setText(Integer.toString(AppConstants.MIN_YEAR));
            } else if (Integer.parseInt(YearControl.getText()) > AppConstants.MAX_YEAR) {
                YearControl.setText(Integer.toString(AppConstants.MAX_YEAR));
            }
        }
        thisCalender.set(Calendar.YEAR, Integer.parseInt(YearControl.getText()));
        //now check month
        if (MonthControl.getText().length() == 0) {
            if (undo.canUndo()) {
                undo.undo();
            } else {
                MonthControl.setText(Integer.toString(12));
            }
        }//MonthControl.Undo();
        else {
            //set month to a valid setting if out of valid range
            if (Integer.parseInt(MonthControl.getText()) < 1) {
                MonthControl.setText(Integer.toString(1));
            } else if (Integer.parseInt(MonthControl.getText()) > 12) {
                MonthControl.setText(Integer.toString(12));
            }
        }
        thisCalender.set(Calendar.MONTH, Integer.parseInt(MonthControl.getText()) - 1);
        //now check day
        int iDaysInMonth = thisCalender.getActualMaximum(Calendar.DAY_OF_MONTH);
        if (DayControl.getText().length() == 0) {
            if (undo.canUndo()) {
                undo.undo();
            } else {
                DayControl.setText(Integer.toString(iDaysInMonth));
            }
        }//DayControl.Undo();
        else {
            //set month to a valid setting if out of valid range
            if (Integer.parseInt(DayControl.getText()) < 1) {
                DayControl.setText(Integer.toString(1));
            } else if (Integer.parseInt(DayControl.getText()) > iDaysInMonth) {
                DayControl.setText(Integer.toString(iDaysInMonth));
            }
        }
    }

    /** parses up a date string and places it into the given month, day, year
     * interace text control (TEdit *). Defaults prospective survallience start
     * date to months/days to like study period end date.                       */
    public static void parseDateStringToControls(String sDateString, JTextField Year, JTextField Month, JTextField Day, boolean bEndDate) {
        String[] dateParts = sDateString.split("/");

        try {
            int iYear = 0, iMonth = 0, iDay = 0;
            //set values only if valid, prevent interface from having invalid date when first loaded.
            if (dateParts.length > 0) {
                switch (dateParts.length) {
                    case 1:
                        iYear = Integer.parseInt(dateParts[0]);
                        if (iYear >= AppConstants.MIN_YEAR && iYear <= AppConstants.MAX_YEAR) {
                            Year.setText(dateParts[0]);
                        }
                        break;
                    case 2:
                        iYear = Integer.parseInt(dateParts[0]);
                        iMonth = Integer.parseInt(dateParts[1]);
                        if (iYear >= AppConstants.MIN_YEAR && iYear <= AppConstants.MAX_YEAR && iMonth >= 1 && iMonth <= 12) {
                            Year.setText(dateParts[0]);
                            Month.setText(dateParts[1]);
                            if (bEndDate) {
                                GregorianCalendar thisCalender = new GregorianCalendar();
                                thisCalender.set(Calendar.YEAR, iYear);
                                thisCalender.set(Calendar.MONTH, iMonth - 1);
                                Day.setText(Integer.toString(thisCalender.getActualMaximum(Calendar.DAY_OF_MONTH)));
                            } else {
                                Day.setText("1");
                            }
                        }
                        break;
                    case 3:
                    default:
                        GregorianCalendar thisCalender = new GregorianCalendar();
                        iYear = Integer.parseInt(dateParts[0]);
                        iMonth = Integer.parseInt(dateParts[1]);
                        iDay = Integer.parseInt(dateParts[2]);
                        thisCalender.set(Calendar.YEAR, iYear);
                        thisCalender.set(Calendar.MONTH, iMonth - 1);
                        if (iYear >= AppConstants.MIN_YEAR && iYear <= AppConstants.MAX_YEAR && iMonth >= 1 && iMonth <= 12 &&
                                iDay >= 1 && iDay <= thisCalender.getActualMaximum(Calendar.DAY_OF_MONTH)) {
                            Year.setText(dateParts[0]);
                            Month.setText(dateParts[1]);
                            Day.setText(dateParts[2]);
                        }
                }
            }
        } catch (NumberFormatException e) {
        }
    }
}
