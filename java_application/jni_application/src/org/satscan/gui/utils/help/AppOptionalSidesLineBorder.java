/*
 * Copyright (C) 2010 Information Management Services, Inc.
 */
package org.satscan.gui.utils.help;

import java.awt.Color;
import java.awt.Component;
import java.awt.Graphics;

import javax.swing.border.LineBorder;

public class AppOptionalSidesLineBorder extends LineBorder {

    protected boolean _top, _left, _bottom, _right;

    public AppOptionalSidesLineBorder(Color c, boolean top, boolean left, boolean bottom, boolean right) {
        super(c);

        _top = top;
        _left = left;
        _bottom = bottom;
        _right = right;
    }

    @Override
    public void paintBorder(Component c, Graphics g, int x, int y, int width, int height) {
        Color oldColor = g.getColor();
        g.setColor(lineColor);

        if (_top)
            g.drawLine(x, y, width - 1, y);
        if (_left)
            g.drawLine(x, y, x, height - 1);
        if (_bottom)
            g.drawLine(x, height - 1, width - 1, height - 1);
        if (_right)
            g.drawLine(width - 1, y, width - 1, height - 1);

        g.setColor(oldColor);
    }
}
