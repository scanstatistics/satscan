/*
 * Copyright (C) 2010 Information Management Services, Inc.
 */
package org.satscan.gui.utils.help;

import java.awt.BorderLayout;
import java.awt.Color;
import java.awt.Component;
import java.awt.Desktop;
import java.awt.Dimension;
import java.awt.Font;
import java.awt.Frame;
import java.awt.Graphics;
import java.awt.Graphics2D;
import java.awt.IllegalComponentStateException;
import java.awt.LayoutManager;
import java.awt.Point;
import java.awt.Rectangle;
import java.awt.RenderingHints;
import java.awt.Toolkit;
import java.awt.Window;
import java.awt.event.ActionListener;
import java.io.File;
import java.io.IOException;
import java.net.URL;
import java.util.ArrayList;
import java.util.Collection;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import javax.swing.BorderFactory;
import javax.swing.ImageIcon;
import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JComponent;
import javax.swing.JDialog;
import javax.swing.JFrame;
import javax.swing.JLabel;
import javax.swing.JMenuItem;
import javax.swing.JPanel;
import javax.swing.JRadioButton;
import javax.swing.JToggleButton;

import org.apache.commons.lang3.SystemUtils;
import org.satscan.gui.SaTScanApplication;

public final class AppGuiUtils {

    /** A few color constants */
    public static final Color COLOR_COMP_FOCUS_OUT = Color.GRAY;
    public static final Color COLOR_COMP_FOCUS_IN = Color.BLACK;

    /** Cached windows */
    private static Map<String, AppUniqueWindow> _CACHED_WINDOWS = new HashMap<String, AppUniqueWindow>();

    /** Cached windows size and location */
    private static Map<String, String> _WINDOWS_INFO = new HashMap<String, String>();

    /** 
     * Private constructor, no instanciation!
     * <p>
     * Created on Jan 30, 2010 by Fabian
     */
    private AppGuiUtils() {
    }

    // *****************************************************************************************
    //                   UNDER THIS LINE ARE THE WINDOWS MANAGEMENT METHODS
    // *****************************************************************************************

    /** 
     * Tries to get the cached window correpsonding to the requested ID and to show it.
     * <p>
     * Created on Aug 11, 2010 by depryf
     * @param windowId requested Window ID
     * @return the <code>AppUniqueWindow</code> if the window was found and displayed, null otherwise.
     */
    public static synchronized AppUniqueWindow show(String windowId) {
        AppUniqueWindow window = _CACHED_WINDOWS.get(windowId);
        if (window != null) {
            if (window instanceof JFrame) {
                JFrame frame = (JFrame)window;
                if (frame.getExtendedState() == JFrame.ICONIFIED)
                    frame.setExtendedState(JFrame.NORMAL);
                frame.toFront();
            }
            else if (window instanceof JDialog)
                ((JDialog)window).toFront();
            else
                throw new RuntimeException("Unique SEER Windows work only for JFrame and JDialog!");
            return window;
        }
        else
            return null;
    }

    /** 
     * Shows the passed source window.
     * <p>
     * Created on Oct 8, 2010 by depryf
     * @param source source window, cannot be null
     */
    public static synchronized void show(final Window source) {
        if (source instanceof AppUniqueWindow)
            _CACHED_WINDOWS.put(((AppWindow)source).getWindowId(), (AppUniqueWindow)source);

        source.setVisible(true);
    }

    /** 
     * Returns a list of all the cached unique windows ID.
     * <p>
     * Created on May 9, 2011 by depryf
     * @return a list of <code>String</code>, maybe empty but never null
     */
    public static List<String> getCachedUniqueWindowsId() {
        return new ArrayList<String>(_CACHED_WINDOWS.keySet());
    }

    /** 
     * Returns a list of all the cached unique windows.
     * <p>
     * Created on May 9, 2011 by depryf
     * @return a list of <code>AppUniqueWindow</code>, maybe empty but never null
     */
    public static List<AppUniqueWindow> getCachedUniqueWindows() {
        return new ArrayList<AppUniqueWindow>(_CACHED_WINDOWS.values());
    }

    /** 
     * Shows and positions the passed source window.
     * <p>
     * Created on Jan 30, 2010 by Fabian
     * @param source source window, cannot be null
     * @param parent parent component (used to center the source), can be null
     */
    public static synchronized void showAndPosition(final Window source, Component parent) {
        showAndPosition(source, parent, false, false);
    }

    /** 
     * Shows and positions the passed source window.
     * <p>
     * Created on Jan 30, 2010 by Fabian
     * @param source source window, cannot be null
     * @param parent parent component (used to center the source), can be null
     * @param usePreviousSize if true, the cached windows info will be used to try to get the previous size of the source window
     * @param usePreviousLocation if true, the cached windows info will be used to try to get the previous location of the source window
     */
    public static synchronized void showAndPosition(final Window source, Component parent, boolean usePreviousSize, boolean usePreviousLocation) {

        String windowId = source.getClass().getName();
        if (source instanceof AppWindow)
            windowId = ((AppWindow)source).getWindowId();

        String windowInfo = _WINDOWS_INFO.get(windowId);
        if (windowInfo != null && windowInfo.matches("^(max|normal)\\|\\d+\\|\\d+\\|\\d+\\|\\d+(\\|.+)?$")) {
            String[] vals = windowInfo.split("\\|");
            String state = vals[0];
            int w = Integer.valueOf(vals[1]);
            int h = Integer.valueOf(vals[2]);
            int x = Integer.valueOf(vals[3]);
            int y = Integer.valueOf(vals[4]);

            if (usePreviousSize)
                source.setPreferredSize(new Dimension(w, h));
            source.pack();

            if (usePreviousLocation) {
                source.setLocation(x, y);
                if (source instanceof JFrame && "max".equals(state))
                    ((JFrame)source).setExtendedState(Frame.MAXIMIZED_BOTH);
            }
            else
                centerWindow(source, parent);
        }
        else
            centerWindow(source, parent);

        if (source instanceof AppUniqueWindow)
            _CACHED_WINDOWS.put(windowId, (AppUniqueWindow)source);

        source.setVisible(true);
    }

    /** 
     * Centers the passed window on the passed parent; centers on the screen if parent is null.
     * <p>
     * Created on Feb 15, 2010 by depryf
     * @param source window to center
     * @param parent parent to center on
     */
    public static void centerWindow(Window source, Component parent) {
        Point center = new Point();

        if (parent != null)
            center.setLocation(parent.getLocationOnScreen().x + parent.getWidth() / 2, parent.getLocationOnScreen().y + parent.getHeight() / 2);
        else {
            center.setLocation(Toolkit.getDefaultToolkit().getScreenSize().width / 2, Toolkit.getDefaultToolkit().getScreenSize().height / 2);
        }

        source.pack();

        source.setLocation(center.x - source.getWidth() / 2, center.y - source.getHeight() / 2);
    }

    /** 
     * Hides and destroys the passed Window; position and size will be automatically remembered.
     * <p>
     * Created on Apr 8, 2010 by depryf
     * @param source
     */
    public static synchronized void hideAndDestroy(Window source) {
        hideAndDestroy(source, null, false);
    }

    /** 
     * Hides and destroys the passed Window; position and size will be automatically remembered.
     * Passed extra info will be remembed as-well if it is not null.
     * <p>
     * Created on Apr 8, 2010 by depryf
     * @param source
     * @param extraInfo
     */
    public static synchronized void hideAndDestroy(Window source, String extraInfo) {
        hideAndDestroy(source, extraInfo, false);
    }

    /** 
     * Hides and destroys the passed Window; position and size will be remembered if the corresponding parameter is true.
     * Passed extra info will be remembed as-well if it is not null.
     * <p>
     * Created on Apr 8, 2010 by depryf
     * @param source
     * @param extraInfo
     * @param remember
     */
    public static synchronized void hideAndDestroy(Window source, String extraInfo, boolean remember) {
        hide(source, extraInfo, remember);
        source.dispose();
    }

    /** 
     * Hides (but does not destroy) the passed Window; position and size will be automatically remembered.
     * Passed extra info will be remembed as-well if it is not null.
     * <p>
     * Created on Apr 8, 2010 by depryf
     * @param source
     * @param extraInfo
     */
    public static synchronized void hide(Window source, String extraInfo) {
        hide(source, extraInfo, false);
    }

    /** 
     * Hides (but does not destroy) the passed Window; position and size will be remembered if the corresponding parameter is true.
     * Passed extra info will be remembed as-well if it is not null.
     * <p>
     * Created on Apr 8, 2010 by depryf
     * @param source
     * @param extraInfo
     * @param remember
     */
    public static synchronized void hide(Window source, String extraInfo, boolean remember) {

        String windowId = source.getClass().getName();
        if (source instanceof AppWindow)
            windowId = ((AppWindow)source).getWindowId();

        if (remember) {
            String windowInfo = buildWindowInfo(source);
            if (extraInfo != null)
                windowInfo = windowInfo + "|" + extraInfo;
            _WINDOWS_INFO.put(windowId, windowInfo);
        }

        if (source instanceof AppUniqueWindow)
            _CACHED_WINDOWS.remove(windowId);

        source.setVisible(false);
    }

    /** 
     * Builds the string representation (winfow info) of the passed Window to keep track of its position and size.
     * <p>
     * Created on Apr 8, 2010 by depryf
     * @param source
     * @return
     */
    public static String buildWindowInfo(Window source) {
        StringBuffer buf = new StringBuffer();

        // format is 'state|w|h|locX|locY'
        if (source instanceof JFrame && ((JFrame)source).getExtendedState() == Frame.MAXIMIZED_BOTH) {
            Dimension screenSize = Toolkit.getDefaultToolkit().getScreenSize();
            buf.append("max|").append(screenSize.width).append("|").append(screenSize.height).append("|0|0");
        }
        else {
            buf.append("normal|").append(source.getWidth()).append("|").append(source.getHeight()).append("|");
            Point loc = null;
            try {
                loc = source.getLocationOnScreen();
            }
            catch (IllegalComponentStateException e) {
                Dimension dim = Toolkit.getDefaultToolkit().getScreenSize();
                loc = new Point((dim.width / 2) - (source.getWidth() / 2), (dim.height / 2) - (source.getHeight() / 2));
            }
            buf.append(loc.x).append("|").append(loc.y);
        }

        return buf.toString();
    }

    /** 
     * Adds the passed window info to the cached windows info.
     * <p>
     * Created on Apr 8, 2010 by depryf
     * @param source
     * @param info
     */
    public static void addWindowInfo(Window source, String info) {
        String windowId = source.getClass().getName();
        if (source instanceof AppWindow)
            windowId = ((AppWindow)source).getWindowId();

        synchronized (AppGuiUtils.class) {
            _WINDOWS_INFO.put(windowId, info);
        }
    }

    /** 
     * Returns the winfow info of the passed Window, if any.
     * <p>
     * Created on Apr 8, 2010 by depryf
     * @param source
     * @return
     */
    public static String getWindowInfo(Window source) {
        String windowId = source.getClass().getName();
        if (source instanceof AppWindow)
            windowId = ((AppWindow)source).getWindowId();
        return _WINDOWS_INFO.get(windowId);
    }

    /** 
     * Returns all the window infos currently cached.
     * <p>
     * Created on Apr 8, 2010 by depryf
     * @return
     */
    public static Map<String, String> getAllWindowsInfo() {
        return Collections.unmodifiableMap(_WINDOWS_INFO);
    }

    /** 
     * Resets the windows info caching.
     * <p>
     * Created on May 7, 2011 by depryf
     */
    public static synchronized void resetCachedWindowsInfo() {
        _WINDOWS_INFO.clear();
    }

    /** 
     * Opens folder.
     * <p>
     * Created on Jan 6, 2012 by murphyr
     * @param dir
     */
    public static void openDirectory(File dir) throws IOException {
        Desktop.getDesktop().open(dir);
    }

    /** 
     * Opens folder and if the OS is windows, selects the file.
     * <p>
     * Created on Jan 6, 2012 by murphyr
     * @param dir
     * @param fileToSelect
     */
    public static void openDirectory(File dir, String fileToSelect) throws IOException {
        if (SystemUtils.IS_OS_WINDOWS)
            Runtime.getRuntime().exec("Explorer /select," + dir.getAbsolutePath() + "\\" + fileToSelect);
        else
            Desktop.getDesktop().open(dir);
    }

    // *****************************************************************************************
    //                    UNDER THIS LINE ARE THE 'CREATE' METHODS
    // *****************************************************************************************

    public static ImageIcon createIcon(String icon) {
        return createIcon(icon, "/");
    }

    public static ImageIcon createIcon(String icon, String path) {
        //return new ImageIcon(Thread.currentThread().getContextClassLoader().getResource(path + icon));
        ClassLoader loader = Thread.currentThread().getContextClassLoader();
        //URL url = loader.getResource(path + icon);

        URL url = SaTScanApplication.getInstance().getClass().getResource("/help.png");
        return new ImageIcon(url);
    }

    public static JPanel createPanel() {
        return createPanel(new BorderLayout());
    }

    public static JPanel createPanel(LayoutManager layout) {
        JPanel panel = new JPanel(layout);
        panel.setBorder(BorderFactory.createEmptyBorder());
        panel.setOpaque(false);
        return panel;
    }

    public static JPanel createSeparation(final Color topColor, final Color bottomColor) {
        return new JPanel() {
            public void paintComponent(Graphics graphics) {
                Graphics2D g = (Graphics2D)graphics;

                Rectangle bounds = getBounds();
                g.setColor(topColor);
                g.drawLine(10, bounds.height / 2, bounds.width - 10, bounds.height / 2);
                g.setColor(bottomColor);
                g.drawLine(11, bounds.height / 2 + 1, bounds.width - 10, bounds.height / 2 + 1);
            }
        };
    }

    public static JButton createButton(String text, String action, String tooltip, ActionListener listener) {
        JButton btn = new JButton(text);
        btn.setOpaque(false);
        btn.setActionCommand(action);
        btn.setName(action + "-btn");
        btn.setToolTipText(tooltip);
        btn.addActionListener(listener);
        return btn;
    }

    public static JButton createToolbarButton(String icon, String action, String tooltip, ActionListener listener) {
        JButton btn = createButton(null, action, tooltip, listener);
        btn.setIcon(createIcon(icon));
        btn.setFocusPainted(false);
        return btn;
    }

    public static JToggleButton createToolbarToggleButton(String icon, String action, String tooltip, ActionListener listener) {
        JToggleButton btn = new JToggleButton();
        btn.setIcon(createIcon(icon));
        btn.setFocusPainted(false);
        btn.setOpaque(false);
        btn.setActionCommand(action);
        btn.setName(action + "-btn");
        btn.setToolTipText(tooltip);
        btn.addActionListener(listener);
        return btn;
    }

    public static JLabel createLabel(String text) {
        return createLabel(text, Font.PLAIN);
    }

    public static JLabel createLabel(String text, int style) {
        return createLabel(text, style, Color.BLACK);
    }

    public static JLabel createLabel(String text, int style, Color color) {
        JLabel lbl = new JLabel(text);
        lbl.setOpaque(false);
        lbl.setFont(lbl.getFont().deriveFont(style));
        lbl.setForeground(color);
        return lbl;
    }

    public static JLabel createLabel(String text, int style, int size, Color color) {
        JLabel lbl = new JLabel(text);
        lbl.setOpaque(false);
        lbl.setFont(lbl.getFont().deriveFont(style, size));
        lbl.setForeground(color);
        return lbl;
    }

    public static JLabel createLabel(String text, Font font, Color color) {
        JLabel lbl = new JLabel(text);
        lbl.setOpaque(false);
        lbl.setFont(font);
        lbl.setForeground(color);
        return lbl;
    }

    public static JCheckBox createCheckBox(String text, String action, ActionListener listener) {
        return createCheckBox(text, action, Font.PLAIN, listener);
    }

    public static JCheckBox createCheckBox(String text, String action, int style, ActionListener listener) {
        JCheckBox box = new JCheckBox(text);
        box.setOpaque(false);
        box.setActionCommand(action);
        box.setName(action);
        box.setFont(box.getFont().deriveFont(style));
        if (listener != null)
            box.addActionListener(listener);

        return box;
    }

    public static JRadioButton createRadioButton(String text, String action, ActionListener listener) {
        return createRadioButton(text, action, Font.PLAIN, listener);
    }

    public static JRadioButton createRadioButton(String text, String action, int style, ActionListener listener) {
        JRadioButton btn = new JRadioButton(text);
        btn.setOpaque(false);
        btn.setActionCommand(action);
        btn.setName(action);
        btn.setFont(btn.getFont().deriveFont(style));
        if (listener != null)
            btn.addActionListener(listener);

        return btn;
    }

    public static JMenuItem createMenuItem(String label, String action, ActionListener listener) {
        JMenuItem item = new JMenuItem(label);
        item.setActionCommand(action);
        if (listener != null)
            item.addActionListener(listener);

        item.setName(action);

        return item;
    }

    public static void synchronizedComponentsWidth(JComponent... components) {
        List<JComponent> comps = new ArrayList<JComponent>();
        for (JComponent comp : components)
            comps.add(comp);

        synchronizedComponentsWidth(comps);
    }

    public static void synchronizedComponentsWidth(Collection<? extends JComponent> comps) {
        if (comps.isEmpty())
            return;

        int maxWidth = 0;
        for (JComponent comp : comps)
            if (comp.isVisible())
                maxWidth = Math.max(maxWidth, comp.getPreferredSize().width);

        Dimension dim = new Dimension(maxWidth, comps.iterator().next().getPreferredSize().height);
        for (JComponent comp : comps) {
            comp.setPreferredSize(dim);
            comp.setMaximumSize(dim);
        }
    }

    public static boolean getAntialias(Graphics g) {
        if (g instanceof Graphics2D)
            return RenderingHints.VALUE_ANTIALIAS_ON.equals(((Graphics2D)g).getRenderingHint(RenderingHints.KEY_ANTIALIASING));
        else
            return false;
    }

    public static void setAntialias(Graphics g, boolean state) {
        if (g instanceof Graphics2D)
            ((Graphics2D)g).setRenderingHint(RenderingHints.KEY_ANTIALIASING, state ? RenderingHints.VALUE_ANTIALIAS_ON : RenderingHints.VALUE_ANTIALIAS_OFF);
    }
}
