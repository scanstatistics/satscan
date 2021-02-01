package org.satscan.app;

import org.satscan.gui.AnalysisRunInternalFrame;

/**
 * Thread class for executing analysis through call to Java Native Interface.
 */
public class CalculationThread extends Thread {

    private final AnalysisRunInternalFrame _analysis_run_frame;
    final private Parameters _parameters;
    String _program_error_callpath = "";

    public native int RunAnalysis(Parameters jparameters);

    public CalculationThread(AnalysisRunInternalFrame analysis_run_frame, final Parameters parameters) {
        super(parameters.GetOutputFileName());
        _analysis_run_frame = analysis_run_frame;
        _parameters = parameters;
    }

    synchronized private boolean IsCancelled() {
        return _analysis_run_frame.userCancelled();
    }

    synchronized private void PrintStandard(java.lang.String line) {
        _analysis_run_frame.PrintProgressWindow(line);
    }

    synchronized public void PrintError(String line) {
        _analysis_run_frame.PrintIssuesWindndow(line);
    }

    synchronized public void PrintWarning(String line) {
        _analysis_run_frame.PrintIssuesWindndow(line);
    }

    synchronized public void PrintNotice(String line) {
        _analysis_run_frame.PrintIssuesWindndow(line);
    }

    synchronized public void ReportDrilldownResults(String drilldown_resultfile, String parent_resultfile, int significantClusters) {
        _analysis_run_frame.ReportDrilldownResults(drilldown_resultfile, parent_resultfile, significantClusters);
    }
    
    synchronized public void setCallpath(String call_path) {
        _program_error_callpath = call_path;
    }

    /** Starts thread execution -- makes call to native code through JNI function. */
    @Override
    public void run() {
        try {
            if (RunAnalysis(_parameters) == 0) {
                // Analysis execution terminated normally.
                if (_analysis_run_frame.userCancelled()) {
                    // Analysis cancelled by user -- acknowledge that engine has terminated.
                    _analysis_run_frame.setTitle("Job cancelled");
                    _analysis_run_frame.PrintProgressWindow("Job cancelled by user.");
                } else {
                    // Cause analysis run framer to load results of a successful analysis.
                    _analysis_run_frame.loadAnalysisResults(true);
                }
            } else {
                // Analysis execution terminated with some kind of error.
                _analysis_run_frame.setExceptionalTermination(_program_error_callpath);
            }
            // If no warnings or errors reported at this point, so that to user.
            if (!_analysis_run_frame.issuesReported())
                _analysis_run_frame.PrintIssuesWindndow("No Warnings or Errors.");
            _analysis_run_frame.setPrintEnabled();
            _analysis_run_frame.setCanClose(true);
            _analysis_run_frame.setCloseButton();
        } catch (Throwable t) {
            _analysis_run_frame.setExceptionOccurred(t.getMessage(), t.getStackTrace());
        }
    }
}
