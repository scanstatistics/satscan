/*
 * Copyright (C) 2010 Information Management Services, Inc.
 */
package org.satscan.gui.utils.help;

/** 
 * This interface adds special SEER*Abs functionality to all the windows used in the application.
 * <p>
 * Created on Jan 30, 2010 by depryf
 * @author depryf
 */
public interface AppWindow {

    /** 
     * Handles the passed shortcut key (CTRL modifier is assumed, except for Escape).
     * <p>
     * Note that Enter, unlike Escape, should not be used globally through this mechanism. The reason is that Enter can 
     * already have some action bound to it when the focus is on some component (for example on a button). Then it becomes 
     * very confusing if Enter is bound to the "Ok" action through this mechanism but the focus is on the Cancel button!
     * <p>
     * Created on Oct 29, 2008 by depryf
     * @param key requested shortcut key (CTRL modifier is assumed)
     * @return true if the key has been consumed, false otherwise
     */
    public boolean handleShortcut(int key);
    
    /** 
     * Returns this window's unique identifier. Most of the time the identifier will be based on the window name and therefore
     * those names should not be changed.
     * <p>
     * Created on Jan 30, 2010 by depryf
     * @return unique identifier for this window, never null
     */
    public String getWindowId();
}
