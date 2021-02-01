/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package org.satscan.app;

/**
 *
 * @author hostovic
 */
public class BatchAnalysis implements Cloneable {   
    public enum UNITS { YEAR, MONTH, DAY, GENERIC }
    public enum STATUS { NEVER, SUCCESS, CANCELLED, FAILED  }
    public static class StudyPeriodOffset implements Cloneable {
        private int _offset = 365;
        private BatchAnalysis.UNITS _units = UNITS.DAY;

        public StudyPeriodOffset() {}
        
        public StudyPeriodOffset(int offset, BatchAnalysis.UNITS units) {
            _offset = offset;
            _units = units;
        }
        
        public int getOffset() {
            return _offset;
        }

        public BatchAnalysis.UNITS getUnits() {
            return _units;
        }

        public void setOffset(int i) {
            _offset = i;
        }

        public void setUnits(BatchAnalysis.UNITS u) {
            _units = u;
        }   
        
        @Override
        public Object clone() throws CloneNotSupportedException {
            return (StudyPeriodOffset) super.clone();
        }
    }    
    private String _description;
    private Parameters _parameters;
    private StudyPeriodOffset _lag = null;
    private StudyPeriodOffset _study_length = null;
    private java.util.Date _last_executed_date = null;
    private STATUS _last_executed_status=STATUS.NEVER;
    private String _last_executed_mssg=null;
    
    public BatchAnalysis() {
        super();
        _description = "New Analysis";
        _parameters = new Parameters();
        
    }
    
    public BatchAnalysis(String description, Parameters parameters, StudyPeriodOffset study_length, StudyPeriodOffset lag) {
        super();
        _description = description;
        _parameters = parameters;
        _lag = lag;
        _study_length = study_length;
    } 
        
    @Override
    public Object clone() throws CloneNotSupportedException {
        return (BatchAnalysis) super.clone();
    }    
    
    static public String getDescription(final Parameters parameters) {
        StringBuilder description = new StringBuilder();
        description.append(parameters.GetAnalysisTypeAsString());
        description.append(", ").append(parameters.GetModelTypeAsString());
        Parameters.AnalysisType analysisType =parameters.GetAnalysisType();
        if (!(analysisType == Parameters.AnalysisType.PURELYSPATIAL || analysisType == Parameters.AnalysisType.SPATIALVARTEMPTREND)) {
            if (parameters.GetMaximumTemporalClusterSizeType() == Parameters.TemporalSizeType.TIMETYPE) {
                description.append(", ").append(parameters.GetMaximumTemporalClusterSize());
                switch (parameters.GetTimeAggregationUnitsType()) {
                    case YEAR : description.append(" year").append((parameters.GetMaximumTemporalClusterSize() == 1.0 ? "" : "s")); break;
                    case MONTH: description.append(" month").append((parameters.GetMaximumTemporalClusterSize() == 1.0 ? "" : "s")); break;
                    case DAY:  description.append(" day").append((parameters.GetMaximumTemporalClusterSize() == 1.0 ? "" : "s")); break;
                    case GENERIC: description.append(" generic unit").append((parameters.GetMaximumTemporalClusterSize() == 1.0 ? "" : "s")); break;
                    case NONE: 
                    default: 
                }
            } else {
                description.append(", ").append(parameters.GetMaximumTemporalClusterSize()).append("%");
            }
            description.append(" max temporal");
        }
        if (analysisType == Parameters.AnalysisType.PURELYSPATIAL || analysisType == Parameters.AnalysisType.SPATIALVARTEMPTREND ||
            analysisType == Parameters.AnalysisType.SPACETIME || analysisType == Parameters.AnalysisType.PROSPECTIVESPACETIME) {
            description.append(", ").append(parameters.GetMaxSpatialSizeForType(Parameters.SpatialSizeType.PERCENTOFPOPULATION.ordinal(), false) + "%");
            if (parameters.GetRestrictMaxSpatialSizeForType(Parameters.SpatialSizeType.PERCENTOFMAXCIRCLEFILE.ordinal(), false)) {
                description.append(", ").append(parameters.GetMaxSpatialSizeForType(Parameters.SpatialSizeType.PERCENTOFMAXCIRCLEFILE.ordinal(), false));
            }
            if (parameters.GetRestrictMaxSpatialSizeForType(Parameters.SpatialSizeType.MAXDISTANCE.ordinal(), false)) {
                description.append(", ").append(parameters.GetMaxSpatialSizeForType(Parameters.SpatialSizeType.MAXDISTANCE.ordinal(), false)).append(" radius");
            }
            description.append(" max spatial");
        }
        return description.toString();
    }
    
    public String getDescription() {
        return _description;
    }

    public void setDescription(final String s) {
        _description = new String(s);
    }    
    
    public Parameters getParameters() {
        return _parameters;
    }

    public void setParameters(final Parameters p) {
        _parameters = (Parameters)p.clone();
    }
    
    public StudyPeriodOffset getStudyPeriodLength() {
        return _study_length;
    }

    public void setStudyPeriodLength(StudyPeriodOffset offset) {
        _study_length = offset;
    }    
    
    public StudyPeriodOffset getLag() {
        return _lag;
    }

    public void setLag(StudyPeriodOffset offset) {
        _lag = offset;
    }    
    
    public java.util.Date getLastExecutedDate() {
        return _last_executed_date;
    }

    public void setLastExecutedDate(java.util.Date d) {
        _last_executed_date = d;
    }
    
    public STATUS getLastExecutedStatus() {
        return _last_executed_status;
    }

    public void setLastExecutedStatus(STATUS s) {
        _last_executed_status = s;
    }    

    public String getLastExecutedMessage() {
        return _last_executed_mssg;
    }

    public void setLastExecutedMessage(String s) {
        _last_executed_mssg = s;
    }    
}
