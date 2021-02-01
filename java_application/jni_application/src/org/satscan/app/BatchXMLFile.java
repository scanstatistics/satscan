/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package org.satscan.app;

import org.w3c.dom.NodeList;
import org.w3c.dom.Node;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.io.File;
import java.text.ParseException;
import java.util.Collection;
import java.util.logging.Level;
import java.util.logging.Logger;
import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.transform.OutputKeys;
import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.dom.DOMSource;
import javax.xml.transform.stream.StreamResult;
import org.satscan.utils.FileAccess;
import org.w3c.dom.Document;
import org.w3c.dom.Element;

public class BatchXMLFile {

    private static final String ANALYSES = "analyses";
    private static final String ANALYSIS = "analysis";
    private static final String DESCRIPTION = "description";
    private static final String PARAMETER_SETTINGS = "parameter-settings";
    private static final String STUDYPERIOD = "studyperiod";
    private static final String STUDYPERIOD_UNIT = "studyperiod-unit";
    private static final String LAG = "lag";
    private static final String LAG_UNIT = "lag-unit";
    private static final String LAST_EXEC_DATE = "last-execution-date";
    public static final String LAST_EXEC_DATE_FORMAT = "yyyy-MM-dd kk:mm:ss";
    private static final String LAST_EXEC_STATUS = "last-execution-status";
    private static final String LAST_EXEC_MSSG = "last-execution-warnings-errors";
    
    private static final String YEAR = "year";
    private static final String MONTH = "month";
    private static final String GENERIC = "generic";
    private static final String DAY = "day";

    private static final String NEVER = "Never Executed";
    private static final String SUCCESS = "Success";
    private static final String CANCELLED = "Cancelled";
    private static final String FAILED = "Failed";
    
    public static BatchAnalysis.UNITS parseUnit(String val) {
        if (val.equalsIgnoreCase(YEAR))
            return BatchAnalysis.UNITS.YEAR;
        if (val.equalsIgnoreCase(MONTH))
            return BatchAnalysis.UNITS.MONTH;
        if (val.equalsIgnoreCase(GENERIC))
            return BatchAnalysis.UNITS.GENERIC;
        return BatchAnalysis.UNITS.DAY;
    }

    public static String toString(BatchAnalysis.UNITS unit) {
        if (unit.equals(BatchAnalysis.UNITS.YEAR))
            return YEAR;
        if (unit.equals(BatchAnalysis.UNITS.MONTH))
            return MONTH;
        if (unit.equals(BatchAnalysis.UNITS.GENERIC))
            return GENERIC;
        return DAY;
    }
    
    public static BatchAnalysis.STATUS parseStatus(String val) {
        if (val.equalsIgnoreCase(SUCCESS))
            return BatchAnalysis.STATUS.SUCCESS;
        if (val.equalsIgnoreCase(CANCELLED))
            return BatchAnalysis.STATUS.CANCELLED;
        if (val.equalsIgnoreCase(FAILED))
            return BatchAnalysis.STATUS.FAILED;
        if (val.equalsIgnoreCase(NEVER))
            return BatchAnalysis.STATUS.NEVER;
        return BatchAnalysis.STATUS.NEVER;
    }

    public static String toString(BatchAnalysis.STATUS status) {
        try {
            if (status.equals(BatchAnalysis.STATUS.NEVER))
                return NEVER;
            if (status.equals(BatchAnalysis.STATUS.SUCCESS))
                return SUCCESS;
            if (status.equals(BatchAnalysis.STATUS.CANCELLED))
                return CANCELLED;
            if (status.equals(BatchAnalysis.STATUS.FAILED))
                return FAILED;
            throw new Exception("Unrecognized BatchAnalysis.STATUS: " + status);
        } catch (Exception ex) {
            Logger.getLogger(BatchXMLFile.class.getName()).log(Level.SEVERE, null, ex);
        }
        return "";
    }    

    public static java.util.Date parseDate(String val) {
        try {
            if (val.isBlank())
                return null;
            SimpleDateFormat dateFormat = new SimpleDateFormat(LAST_EXEC_DATE_FORMAT);
            return dateFormat.parse(val);
        } catch (ParseException ex) {
            Logger.getLogger(BatchXMLFile.class.getName()).log(Level.SEVERE, null, ex);
        }
        return null;
    }

    public static String toString(java.util.Date date) {
        if (date == null)
            return "";
        SimpleDateFormat dateFormat = new SimpleDateFormat(LAST_EXEC_DATE_FORMAT);
        return dateFormat.format(date);
    } 
    
    public static ArrayList<BatchAnalysis> read(String filename) {
        ArrayList<BatchAnalysis> _batch_analyses = new ArrayList();
        try {
            // Test to see if file exists.
            if (!FileAccess.ValidateFileAccess(filename, false, false))
                return _batch_analyses;
            
            File fXmlFile = new File(filename);
            DocumentBuilderFactory dbFactory = DocumentBuilderFactory.newInstance();
            DocumentBuilder dBuilder = dbFactory.newDocumentBuilder();
            Document doc = dBuilder.parse(fXmlFile);
            
            //optional, but recommended
            //read this - http://stackoverflow.com/questions/13786607/normalization-in-dom-parsing-with-java-how-does-it-work
            doc.getDocumentElement().normalize();
            System.out.println("Root element :" + doc.getDocumentElement().getNodeName());
            NodeList nList = doc.getElementsByTagName(ANALYSIS);
            System.out.println("----------------------------");

            for (int temp=0; temp < nList.getLength(); temp++) {
                Node nNode = nList.item(temp);
                if (nNode.getNodeType() == Node.ELEMENT_NODE) {
                    Element eElement = (Element) nNode;
                    
                    String param_settings = eElement.getElementsByTagName(PARAMETER_SETTINGS).item(0).getTextContent();
                    Parameters parameters = new Parameters();
                    parameters.ReadFromStringStream(param_settings);
                    BatchAnalysis.StudyPeriodOffset study_length = null;
                    if (!eElement.getElementsByTagName(STUDYPERIOD).item(0).getTextContent().isBlank() && 
                        !eElement.getElementsByTagName(STUDYPERIOD_UNIT).item(0).getTextContent().isBlank()) {
                        study_length = new BatchAnalysis.StudyPeriodOffset(
                            Integer.parseInt(eElement.getElementsByTagName(STUDYPERIOD).item(0).getTextContent()),
                            parseUnit(eElement.getElementsByTagName(STUDYPERIOD_UNIT).item(0).getTextContent())
                        );
                    }
                    BatchAnalysis.StudyPeriodOffset lag = null;
                    if (!eElement.getElementsByTagName(LAG).item(0).getTextContent().isBlank() && 
                        !eElement.getElementsByTagName(LAG_UNIT).item(0).getTextContent().isBlank()) {
                        lag = new BatchAnalysis.StudyPeriodOffset(
                            Integer.parseInt(eElement.getElementsByTagName(LAG).item(0).getTextContent()),
                            parseUnit(eElement.getElementsByTagName(LAG_UNIT).item(0).getTextContent())
                        );
                    }                    
                    BatchAnalysis batchAnalysis = new BatchAnalysis(
                        eElement.getElementsByTagName(DESCRIPTION).item(0).getTextContent(), parameters, study_length, lag
                    );                    
                    batchAnalysis.setLastExecutedDate(parseDate(eElement.getElementsByTagName(LAST_EXEC_DATE).item(0).getTextContent()));
                    batchAnalysis.setLastExecutedStatus(parseStatus(eElement.getElementsByTagName(LAST_EXEC_STATUS).item(0).getTextContent()));
                    batchAnalysis.setLastExecutedMessage(eElement.getElementsByTagName(LAST_EXEC_MSSG).item(0).getTextContent());
                    _batch_analyses.add(batchAnalysis);
                }
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
        return _batch_analyses;
    }
    
    public static void write(String filename, Collection<BatchAnalysis> batchAnalyses) {
        try {
            DocumentBuilderFactory docFactory = DocumentBuilderFactory.newInstance();
            DocumentBuilder docBuilder = docFactory.newDocumentBuilder();

            // root elements
            Document doc = docBuilder.newDocument();
            Element rootElement = doc.createElement(ANALYSES);
            doc.appendChild(rootElement);

            for (BatchAnalysis batchAnalysis: batchAnalyses) {
                Element analysis = doc.createElement(ANALYSIS);
                rootElement.appendChild(analysis);

                Element description = doc.createElement(DESCRIPTION);
                description.appendChild(doc.createTextNode(batchAnalysis.getDescription()));
                analysis.appendChild(description);                                
                
                Element settings = doc.createElement(PARAMETER_SETTINGS);
                String parameters = batchAnalysis.getParameters().WriteToStringStream();
                settings.appendChild(doc.createTextNode(parameters));
                analysis.appendChild(settings);                
                
                Element studyperiod = doc.createElement(STUDYPERIOD);
                if (batchAnalysis.getStudyPeriodLength() != null)
                    studyperiod.appendChild(doc.createTextNode(Integer.toString(batchAnalysis.getStudyPeriodLength().getOffset())));
                analysis.appendChild(studyperiod);                

                Element baseline_unit = doc.createElement(STUDYPERIOD_UNIT);
                if (batchAnalysis.getStudyPeriodLength() != null)
                    baseline_unit.appendChild(doc.createTextNode(toString(batchAnalysis.getStudyPeriodLength().getUnits())));
                analysis.appendChild(baseline_unit);                

                Element lag = doc.createElement(LAG);
                if (batchAnalysis.getLag() != null)
                    lag.appendChild(doc.createTextNode(Integer.toString(batchAnalysis.getLag().getOffset())));
                analysis.appendChild(lag);                

                Element lag_unit = doc.createElement(LAG_UNIT);
                if (batchAnalysis.getLag() != null)
                    lag_unit.appendChild(doc.createTextNode(toString(batchAnalysis.getLag().getUnits())));
                analysis.appendChild(lag_unit);                

                Element last_exec_date = doc.createElement(LAST_EXEC_DATE);
                last_exec_date.appendChild(doc.createTextNode(toString(batchAnalysis.getLastExecutedDate())));
                analysis.appendChild(last_exec_date);                

                Element last_exec_status = doc.createElement(LAST_EXEC_STATUS);
                last_exec_status.appendChild(doc.createTextNode(toString(batchAnalysis.getLastExecutedStatus())));
                analysis.appendChild(last_exec_status);                

                Element last_exec_mssg = doc.createElement(LAST_EXEC_MSSG);
                last_exec_mssg.appendChild(doc.createTextNode(batchAnalysis.getLastExecutedMessage()));
                analysis.appendChild(last_exec_mssg);                
            }

            // write the content into xml file
            TransformerFactory transformerFactory = TransformerFactory.newInstance();
            Transformer transformer = transformerFactory.newTransformer();
            transformer.setOutputProperty(OutputKeys.INDENT, "yes");
            DOMSource source = new DOMSource(doc);
            StreamResult result = new StreamResult(new File(filename));
            transformer.transform(source, result);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}