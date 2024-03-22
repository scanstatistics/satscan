//***************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//***************************************************************************
#include "MultipleAnalyses.h"
#include "Toolkit.h"
#include "newmat.h"
#include "IniParameterFileAccess.h"
#include "AnalysisRun.h"
#include "ParametersValidate.h"
#include "UtilityFunctions.h"
using boost::algorithm::replace_all;

const std::string AnalysisDefinition::_YEAR = "year";
const std::string AnalysisDefinition::_MONTH = "month";
const std::string AnalysisDefinition::_DAY = "day";
const std::string AnalysisDefinition::_GENERIC = "generic";
const std::string AnalysisDefinition::_SUCCESS = "Success";
const std::string AnalysisDefinition::_FAILED = "Failed";

const std::string MultipleAnalyses::_BATCH_SETTINGS_ROOT_TAG = "batch-settings";
const std::string MultipleAnalyses::_ANALYSES_TAG = "analyses";
const std::string MultipleAnalyses::_ANALYSIS_TAG = "analysis";
const std::string MultipleAnalyses::_ANALYSIS_SELECTED_TAG = "<xmlattr>.selected";
const std::string MultipleAnalyses::_DESCRIPTION_TAG = "description";
const std::string MultipleAnalyses::_PARAMETERS_TAG = "parameter-settings";
const std::string MultipleAnalyses::_STUDYPERIOD_TAG = "studyperiod";
const std::string MultipleAnalyses::_STUDYPERIOD_UNITS_TAG = "studyperiod-unit";
const std::string MultipleAnalyses::_LAG_TAG = "lag";
const std::string MultipleAnalyses::_LAG_UNITS_TAG = "lag-unit";
const std::string MultipleAnalyses::_LAST_EXEC_DATE_TAG = "last-execution-date";
const std::string MultipleAnalyses::_LAST_EXEC_STATUS_TAG = "last-execution-status";
const std::string MultipleAnalyses::_LAG_EXEC_MSSG_TAG = "last-execution-warnings-errors";
const std::string MultipleAnalyses::_LAG_EXEC_RESULTS_TAG = "last-results-filename";
const std::string MultipleAnalyses::_DRILLDOWN_TREE_TAG = "drilldown-tree";
const std::string MultipleAnalyses::_DRILLDOWN_NODE_TAG = "drilldown-node";
const std::string MultipleAnalyses::_DRILLDOWN_RESULTS_TAG = "results-filename";
const std::string MultipleAnalyses::_DRILLDOWN_RESULTS_SIGNIFANT_TAG = "results-filename.<xmlattr>.significant";

const std::string MultipleAnalyses::_SUMMARY_EMAIL_SETTINGS_TAG = "summary-email-settings";
const std::string MultipleAnalyses::_SEND_SUMMARY_EMAIL_TAG = "send-summary-email";
const std::string MultipleAnalyses::_SUMMARY_EMAIL_LINKS_TAG = "summary-email-links";
const std::string MultipleAnalyses::_SUMMARY_EMAIL_RECIPIENTS_TAG = "summary-email-recipients";
const std::string MultipleAnalyses::_SUMMARY_EMAIL_PVALUE_CUTOFF_TAG = "summary-email-pvalue-cutoff";
const std::string MultipleAnalyses::_SUMMARY_EMAIL_RECURRENCE_CUTOFF_TAG = "summary-email-recurrence-cutoff";
const std::string MultipleAnalyses::_DEFAULT_RECURRENCE_CUTOFF = "365";
const std::string MultipleAnalyses::_DEFAULT_PVALUE_CUTOFF = "0.05";
const std::string MultipleAnalyses::_EMAIL_TEMP_EXTENSION = ".email.tmp";

bool MultipleAnalyses::addResults(const std::string& resultsname, const std::string& parentname, unsigned int significant, pt::ptree &pt_parent, pt::ptree &pt_child, bool target) {
    if (target && pt_child.data() == parentname) {
        std::string buffer;
        pt::ptree rtree;
        rtree.put(_DRILLDOWN_RESULTS_TAG, resultsname);
        rtree.put(_DRILLDOWN_RESULTS_SIGNIFANT_TAG, printString(buffer, "%u", significant));
        pt_parent.add_child(_DRILLDOWN_NODE_TAG, rtree);
        return true;
    }
    for (pt::ptree::iterator pos = pt_child.begin(); pos != pt_child.end(); ++pos) {
        if (addResults(resultsname, parentname, significant, pt_child, pos->second, pos->first == _DRILLDOWN_RESULTS_TAG))
            return true;
    }
    return false;
}

/** Attempts to email summary of analyses last executed through MultipleAnalyses. */
void MultipleAnalyses::emailSummary(BasePrint& print, bool includeUnSelected) {
    // read analyses from xml file.
    if (!read_file(AppToolkit::getToolkit().getMultipleAnalysisFullpath())) {
        print.Printf("The multiple analyses configuration file could not be opened. Use the SaTScan GUI to define multiple analyses.\n", BasePrint::P_WARNING);
        return;
    }
    // Check that user has defined the appropriate email settings.
    if (_send_summary_email) {
        if (AppToolkit::getToolkit().mail_servername.empty()) {
            print.Printf(
                "Warning: The summary email alerts feature requires defining a mail server name (ex. smtp.somecompany.com:25).\n"
                "A summary email alert will not be sent.\n",
                BasePrint::P_WARNING
            );
            return;
        }
        if (AppToolkit::getToolkit().mail_from.empty()) {
            print.Printf(
                "Warning: The summary email alerts feature requires defining an email address as the mail sender (ex. someone@company.com).\n"
                "A summary email alert will not be sent.\n",
                BasePrint::P_WARNING
            );
            return;
        }
        if (_summary_email_recipients.empty()) {
            print.Printf(
                "Warning: The summary email alerts feature requires defining at least one email recipient (ex. someone@company.com).\n"
                "A summary email alert will not be sent.\n",
                BasePrint::P_WARNING
            );
            return;
        }
    }
    std::string filename;
    std::stringstream analysesHTML, analysesPlain, failedAnalyses;
    unsigned int executed = 0, clustersMeetingCutoff = 0;
    std::vector<AnalysisDefinition*> cutoffRuns, failedRuns;
    // Iterate over each defined analysis and execute the analysis.
    for (AnalysesContainer_t::iterator itr = _analysis_defs.begin(); itr != _analysis_defs.end(); ++itr) {
        if (!itr->selected && !includeUnSelected) {
            continue;
        } else if (itr->getFailed()) {
            failedRuns.push_back(&(*itr));
        } else {
            ++executed;
            if (boost::filesystem::exists(printString(filename, "%s%s", itr->getLastResultsFilename().c_str(), _EMAIL_TEMP_EXTENSION).c_str())) {
                std::ifstream summary_tmp(filename.c_str());
                summary_tmp >> clustersMeetingCutoff;
                summary_tmp.close();
                if (clustersMeetingCutoff) cutoffRuns.push_back(&(*itr));
                remove(filename.c_str()); // remove the temporary file now
            }
        }
    }
    // Build email subject and body.
    std::stringstream messageSubjectLine, messageBody;
    std::stringstream messagePlain, messageHTML;
    messageSubjectLine << "SaTScan analyses on <date>: ";
    if (failedRuns.size()) messageSubjectLine << failedRuns.size() << "/" << (failedRuns.size() + executed) << " failed";
    if (executed) {
        messageSubjectLine << (failedRuns.size() ? ", " : "") << executed << "/" << (failedRuns.size() + executed) << " ran, ";
        if (cutoffRuns.size()) messageSubjectLine << cutoffRuns.size() << " with clusters exceeding the alert threshold";
        else messageSubjectLine << "no clusters exceeded the alert threshold";
    }

    messageBody << "Summary results for SaTScan analyses on " << EmailText::DATE_VAR << EmailText::LINEBREAK << EmailText::LINEBREAK;
    messageBody << "Successful executions: " << executed << "/" << (failedRuns.size() + executed) << EmailText::LINEBREAK << (executed == 0 || cutoffRuns.size() ? EmailText::LINEBREAK : "");
    messagePlain << EmailText::getEmailFormattedText(messageBody.str(), "", false);
    messageHTML << EmailText::getEmailFormattedText(messageBody.str(), "", true);
    if (executed) {
        if (cutoffRuns.size()) {
            messageBody.str(""); 
            messageBody << (cutoffRuns.size() == 1 ? "This " : "These ") << cutoffRuns.size() 
                << " analys" << (cutoffRuns.size() == 1 ? "is" : "es") << " had clusters exceeding the alert threshold: " << EmailText::LINEBREAK;
            messagePlain << EmailText::getEmailFormattedText(messageBody.str(), "", false);
            messageHTML << EmailText::getEmailFormattedText(messageBody.str(), "", true);
            for (auto& met : cutoffRuns) {
                messageBody.str(""); messageBody << met->description << ", " << EmailText::SUMMARYLINK_VAR << EmailText::LINEBREAK;
                messagePlain << EmailText::getEmailFormattedText(messageBody.str(), met->getLastResultsFilename(), false);
                messageHTML << EmailText::getEmailFormattedText(messageBody.str(), met->getLastResultsFilename(), true);
            }
        } else {
            messageBody.str(""); messageBody << "No clusters exceeded the alert threshold." << EmailText::LINEBREAK;
            messagePlain << EmailText::getEmailFormattedText(messageBody.str(), "", false);
            messageHTML << EmailText::getEmailFormattedText(messageBody.str(), "", true);
        }
        messagePlain << EmailText::getEmailFormattedText(EmailText::LINEBREAK, "", false);
        messageHTML << EmailText::getEmailFormattedText(EmailText::LINEBREAK, "", true);
    }
    if (failedRuns.size()) {
        messageBody.str(""); messageBody << "Failed  executions: " << failedRuns.size() << "/" << (failedRuns.size() + executed) << EmailText::LINEBREAK;
        for (auto& fail: failedRuns) 
            messageBody << fail->description << EmailText::LINEBREAK;
        messageBody << EmailText::LINEBREAK;
        messagePlain << EmailText::getEmailFormattedText(messageBody.str(), "", false);
        messageHTML << EmailText::getEmailFormattedText(messageBody.str(), "", true);
    }
    messagePlain << EmailText::getEmailFormattedText(EmailText::FOOTER_PAR, "", false);
    messageHTML << EmailText::getEmailFormattedText(EmailText::FOOTER_PAR, "", true);

    std::string recipientsText(_summary_email_recipients);
    std::vector<std::string> recipients;
    csv_string_to_typelist<std::string>(recipientsText.c_str(), recipients);
    // Remove any duplicate recipients.
    std::sort(recipients.begin(), recipients.end());
    recipients.erase(std::unique(recipients.begin(), recipients.end()), recipients.end());
    sendMail( // Send the message.
        AppToolkit::getToolkit().mail_from, recipients, {}, AppToolkit::getToolkit().mail_reply,
        EmailText::getEmailFormattedText(messageSubjectLine.str(), "", false), messagePlain, messageHTML, std::string(""),
        AppToolkit::getToolkit().mail_servername, print, false, AppToolkit::getToolkit().mail_additional
    );
}

/* Reads multiple analysis definitions from xml filee, executes each analysis then writes results to file. */
int MultipleAnalyses::execute(BasePrint& print, bool includeUnSelected) {
    boost::posix_time::ptime localTime = boost::posix_time::second_clock::local_time();
    // read analyses from xml file.
    if (!read_file(AppToolkit::getToolkit().getMultipleAnalysisFullpath())) {
        print.Printf("Error: The multiple analyses configuration file could not be opened. Use the SaTScan GUI to define multiple analyses.\n", BasePrint::P_ERROR);
        return 1;
    }
    if (_analysis_defs.empty()) {
        print.Printf("Error: No analyses defined in multiple analyses configuration file. Use SaTScan GUI to define multiple analyses.\n", BasePrint::P_ERROR);
        return 1;
    }
    print.Printf("%u analyses defined in multiple analyses configuration file.\n", BasePrint::P_STDOUT, _analysis_defs.size());
    // Iterate over each defined analysis and execute the analysis.
    for (AnalysesContainer_t::iterator itr=_analysis_defs.begin(); itr != _analysis_defs.end(); ++itr) {
        if (!itr->selected && !includeUnSelected) {
            print.Printf("Analysis '%s' is not selected for execution. Skipping.\n", BasePrint::P_STDOUT, itr->description.c_str());
            continue;
        }
        print.Printf("Executing analysis '%s'.\n", BasePrint::P_STDOUT, itr->description.c_str());
        PrintProxy proxyPrint(*itr, print); // Create print direction proxy so was can record certain actions.
        try {
            CParameters parameters; // Read parameters settings in CParameters object.
            std::stringstream readstream;
            readstream >> std::noskipws; // Make sure stream doesn't skip whitespace.
            readstream << itr->parameters;
            if (IniParameterFileAccess(parameters, print).Read(readstream)) {
                parameters.setTimestamp(localTime);
                // set study period based on study period length and lag settings.
                boost::gregorian::date enddate = itr->getDate(localTime.date(), itr->getLagOffset(), false);
                if (itr->getLagOffset().first != NONE)
                    parameters.SetStudyPeriodEndDate(gregorianToString(enddate).c_str());
                else // otherwise define enddate per paramemter settings - for possible study period offset
                    enddate = gregorianFromString(parameters.GetStudyPeriodEndDate());
                // base the study period start as an offset from the lag period.
                boost::gregorian::date startdate = itr->getDate(enddate, itr->getGetStudyPeriodOffset(), true);
                if (itr->getGetStudyPeriodOffset().first != NONE)
                    parameters.SetStudyPeriodStartDate(gregorianToString(startdate).c_str());
                if (!ParametersValidate(parameters).Validate(proxyPrint))
                    throw resolvable_error("The parameter settings prevent SaTScan from continuing this analysis. Please review above message(s) and modify parameter settings accordingly.");
                // Set the last results filename -- which might have date format substitutions.
                itr->lastresultsfilename = parameters.GetOutputFileName();
                //create analysis runner object and execute analysis
                time_t RunTime; time(&RunTime);
                parameters.setCreateEmailSummaryFile(_send_summary_email);
                parameters.setEmailSummaryValue(std::stod(parameters.GetIsProspectiveAnalysis() ? _summary_email_recurrence_cutoff : _summary_email_pvalue_cutoff));
                AnalysisRunner(parameters, RunTime, proxyPrint).run();
                //report completion
                print.Printf("\nSaTScan completed successfully.\nThe results have been written to: \n  %s\n\n", BasePrint::P_STDOUT, parameters.GetOutputFileName().c_str());
                itr->signalSuccess();
            }
        } catch (resolvable_error & x) {
            proxyPrint.getWarnErrors() << printString(itr->lastexecutionwarningserrors, "%s\nUse '--help' to get help with program options.\n", x.what());
            itr->signalFailure();
        } catch (prg_exception& x) {
            proxyPrint.getWarnErrors() << printString(
                itr->lastexecutionwarningserrors, 
                "Job cancelled due to an unexpected program error.\n\nPlease contact technical support with the following information:\n%s\n%s\n", 
                x.what(), x.trace()
            );
            itr->signalFailure();
        } catch (std::bad_alloc &x) {
            proxyPrint.getWarnErrors() << printString(
                itr->lastexecutionwarningserrors,
                "SaTScan is unable to perform analysis due to insufficient memory.\nPlease see 'Memory Requirements' in user guide for suggested solutions.\n"
            );
        } catch (std::exception& x) {
            proxyPrint.getWarnErrors() << printString(
                itr->lastexecutionwarningserrors, 
                "Job cancelled due to an unexpected program error.\n\nPlease contact technical support with the following information:\n%s\n", 
                x.what()
            );
            itr->signalFailure();
        } catch (BaseException& x) {
            proxyPrint.getWarnErrors() << printString(
                itr->lastexecutionwarningserrors, 
                "Job cancelled due to an unexpected program error.\n\nPlease contact technical support with the following information:\n%s\n",
                x.what()
            );
            itr->signalFailure();
        } catch (...) {
            proxyPrint.getWarnErrors() << printString(
                itr->lastexecutionwarningserrors, 
                "Job cancelled due to an unexpected program error. Please contact technical support with the following information:\n"
                "Unknown program error encountered."
            );
            itr->signalFailure();
        }
        // Record warnings and errors now.
        itr->lastexecutionwarningserrors = proxyPrint.getWarnErrors().str();
    }
    // Write results to file.
    write_file(AppToolkit::getToolkit().getMultipleAnalysisFullpath());
    // Now email summary results if requested.
    if (_send_summary_email)
        emailSummary(print, includeUnSelected);
    return 0;
}

/* Adds result property as a child to correct parent in property tree.  Note that this process relies on in-order calls from root down to leafs. 
   The boost property tree doesn't make additions like this easy since trees don't know there parents (at least not through exposed functions),
   so we need to scan tree to find existing parent tree with 'parent_resultfile' then create child tree under that node.
*/
void PrintProxy::ReportDrilldownResults(const char * drilldown_resultfile, const char * parent_resultfile, unsigned int significantClusters) {
    /* Check for empty property tree and add parent as needed. */
    if (_analysisDef._drilldown_tree.empty()) {
        pt::ptree rtree;
        rtree.put(MultipleAnalyses::_DRILLDOWN_RESULTS_TAG, parent_resultfile);
        rtree.put(MultipleAnalyses::_DRILLDOWN_RESULTS_SIGNIFANT_TAG, "1");
        _analysisDef._drilldown_tree.add_child(MultipleAnalyses::_DRILLDOWN_NODE_TAG, rtree);
    }
    /* Recursively search for parent tree then add new sub tree for this result. */
    if (!MultipleAnalyses::addResults(drilldown_resultfile, parent_resultfile, significantClusters, _analysisDef._drilldown_tree, _analysisDef._drilldown_tree))
        throw prg_error("Unable to find parent subtree for parent '%s' and child '%s'.", parent_resultfile, drilldown_resultfile);
}
