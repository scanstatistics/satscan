/*
 * EmailClientLauncher.java
 *
 * Created on December 10, 2007, 4:02 PM
 *
 * To change this template, choose Tools | Template Manager
 * and open the template in the editor.
 */
package org.satscan.utils;

import java.awt.Desktop;
import java.net.URI;

/**
 *
 * @author Hostovic
 */
public class EmailClientLauncher {

    private static int MAX_BODY_LEN = 1500;

    /** Creates a new instance of EmailClientLauncher */
    public EmailClientLauncher() {
    }

    /**
     * Launches default emial application and creates message detailing error.
     */
    public boolean launchDefaultClientEmail(String mailTo, String subject, String body) {
        try {
            URI uriMailTo = null;
            String _text = body;
            //There appears to be a limitation either in the Java API or with Outlook (assuming most users use Outlook)
            //with passing body text; empirically noted that limit is in the region of > 1500 characters ...
            if (_text != null) {
                _text = _text.substring(0, Math.min(MAX_BODY_LEN, _text.length()));
            }
            StringBuilder uriText = new StringBuilder();
            uriText.append(mailTo);
            if (subject != null && subject.length() > 0) {
                uriText.append("?SUBJECT=" + subject);
            }
            if (_text != null && _text.length() > 0) {
                uriText.append("&BODY=" + _text);
            }
            uriMailTo = new URI("mailto", uriText.toString(), null);
            Desktop.getDesktop().mail(uriMailTo);
        } catch (Exception e) {
            return false;
        }
        return true;
    }
}
