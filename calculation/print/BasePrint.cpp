//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "BasePrint.h"
#include "SSException.h"
#include <algorithm>
#include <boost/regex.hpp>
#include <iostream>

void ResultsNode::addChild(std::shared_ptr<ResultsNode> child) {
    boost::regex pattern("^.+\\-drilldown\\-((?:C(\\d+)(s|b))+).*$");
    auto it = std::lower_bound(_children.begin(), _children.end(), child, [&](const std::shared_ptr<ResultsNode>& r1, const std::shared_ptr<ResultsNode>& r2) {
        // Extract numeric parts using boost regex for natural sorting
        boost::smatch match_r1, match_r2;
        if (boost::regex_search(r1->_filename, match_r1, pattern) && boost::regex_search(r2->_filename, match_r2, pattern)) {
            //for (size_t t = 0; t < match_a.size(); ++t)
		    //    std::cout << "match_a[" << t << "]: " << match_a[t] << std::endl;
            int num_r1 = std::stoi(match_r1[2].str()), num_r2 = std::stoi(match_r2[2].str());
            if (num_r1 != num_r2) {
                return num_r1 < num_r2;
            } else {
                // If the numbers are equal, compare the suffixes (s or b)
                std::string suffix_r1 = match_r1[3].str(), suffix_r2 = match_r2[3].str();
                return suffix_r1 < suffix_r2; // 'b' comes before 's'
            }
        }
        return r1->_filename < r2->_filename; // fallback to lexicographic comparison
    });
    _children.insert(it, child);
}

/** constructor */
BasePrint::BasePrint(bool bSuppressWarnings) : giMaximumReadErrors(75), gbSuppressWarnings(bSuppressWarnings){
   SetImpliedInputFileType(CASEFILE);
#ifdef _MSC_VER
   gsMessage.resize(MSC_VSNPRINTF_DEFAULT_BUFFER_SIZE);
#else
   gsMessage.resize(1);
#endif
}

/** destructor */
BasePrint::~BasePrint() {}

/** Returns indication of whether maximum number of read errors have been printed
    through this object. */
bool BasePrint::GetMaximumReadErrorsPrinted() const {
  std::map<eInputFileType, int>::const_iterator iter = gInputFileWarningsMap.find(geInputFileType);

  return (iter == gInputFileWarningsMap.end() ? false : iter->second == giMaximumReadErrors);
}

/** Directs message to appropriate output based  upon PrintType. */
void BasePrint::Print(const char * sMessage, PrintType ePrintType) {
   switch (ePrintType) {
     case P_STDOUT    : PrintStandard(sMessage); break;
     case P_NOTICE    : if (!gbSuppressWarnings)
                          PrintNotice(sMessage);
                        break;
     case P_WARNING   : if (!gbSuppressWarnings)
                          PrintWarning(sMessage);
                        break;
     case P_READERROR : PrintReadError(sMessage); break;
     case P_PARAMERROR:                   
     case P_ERROR     :
     default          : PrintError(sMessage);
   };
}

/** Creates formatted output from variable number of parameter arguments and calls class Print() method. */
void BasePrint::Printf(const char * sMessage, PrintType ePrintType, ...) {
  if (!sMessage || sMessage == &gsMessage[0]) return;

  try {
#ifdef _MSC_VER
    va_list varArgs;
    va_start (varArgs, ePrintType);
    vsnprintf(&gsMessage[0], gsMessage.size() - 1, sMessage, varArgs);
    va_end(varArgs);
#else
    va_list varArgs_static;
    va_start(varArgs_static, ePrintType);

    std::va_list arglist_test; 
    macro_va_copy(arglist_test, varArgs_static);
    size_t iStringLength = vsnprintf(&gsMessage[0], gsMessage.size(), sMessage, arglist_test);
    gsMessage.resize(iStringLength + 1);

    std::va_list arglist;
    macro_va_copy(arglist, varArgs_static);
    vsnprintf(&gsMessage[0], iStringLength + 1, sMessage, arglist);

    va_end(varArgs_static);
#endif
  }
  catch (...) {}

  Print(&gsMessage[0], ePrintType);
}

void BasePrint::ReportDrilldownResults(const char* drilldown_resultfile, const char* parent_resultfile, unsigned int significantClusters) {
    auto itp = _result_nodes.find(parent_resultfile); // find or create parent
    if (itp == _result_nodes.end())
        itp = _result_nodes.emplace(parent_resultfile, std::make_shared<ResultsNode>(parent_resultfile)).first;
    auto itc = _result_nodes.find(drilldown_resultfile); // find or create drilldown
    if (itc == _result_nodes.end())
        itc = _result_nodes.emplace(drilldown_resultfile, std::make_shared<ResultsNode>(drilldown_resultfile)).first;
    itp->second->addChild(itc->second); // add drilldown as child of parent
    itc->second->setParent(itp->second);
}

// function for printing out input file warning messages, this function will print out MAX_READ_ERRORS
// number of input file messages from each input file type, then will print a warning telling the user to check the
// input file format
// pre : none
// post : increments the counter in the global map for the message type (or starts a new counter if not found) and
//       if the number of messages for that file type is less than the maximum then it just prints as normal
void BasePrint::PrintReadError(const char * sMessage) {
   bool bPrintAsNormal(true);
   std::map<eInputFileType, int>::iterator iter = gInputFileWarningsMap.find(geInputFileType);

   if (iter == gInputFileWarningsMap.end())
      gInputFileWarningsMap.insert(std::make_pair(geInputFileType, 1));
   else {
     iter->second++;
     // print the excessive warning message on the MAX_READ_ERRORS time - else print nothing past -- AJV
     if (iter->second == giMaximumReadErrors) {
       bPrintAsNormal = false;
       std::string message;
       message = "Warning: Suppressing additional errors reading ";
       message += GetImpliedFileTypeString().c_str();
       message += " data.\n";
       PrintError(message.c_str());
     }
     else if(iter->second > giMaximumReadErrors)
       bPrintAsNormal = false;
   }

   if (bPrintAsNormal)
     PrintError(sMessage);
}

void BasePrint::printResultsNodes() const {
    if (_result_nodes.empty()) {
        return;
	}
	// Print the results nodes in a hierarchical manner - first find the root node (that without parent).
    std::shared_ptr<ResultsNode> parent;
    for (const auto& pair : _result_nodes) {
        if (!pair.second->getParent()) {
            parent = pair.second;
            break;
        }
	}
    auto printNode = [&](const std::shared_ptr<ResultsNode>& node, int level, auto& printNodeRef) -> void {
        for (int i = 0; i < level; ++i) std::cout << "  "; // Indentation
        std::cout << node->getFilename() << std::endl;
        for (const auto& child : node->getChildren()) {
            printNodeRef(child, level + 1, printNodeRef);
        }
    };
	printNode(parent, 0, printNode);
}

/* Returns the file name of the input source type used when reporting messages to user. */
const char * BasePrint::getSourceFilenameForType(eInputFileType eType) const {
    switch (eType) {
        case CASEFILE: return "case file";
        case CONTROLFILE: return "control file";
        case POPFILE: return "population file";
        case COORDFILE: return "coordinates file";
        case GRIDFILE: return "grid file";
        case MAXCIRCLEPOPFILE: return "max circle size file";
        case ADJ_BY_RR_FILE: return "adjustments file";
        case LOCATION_NEIGHBORS_FILE: return "location neighbors file";
        case META_LOCATIONS_FILE: return "meta locations file";
        case NETWORK_FILE: return "locations network file";
        case MULTIPLE_LOCATIONS: return "multiple locations file";
        default: throw prg_error("Invalid input file type warning message!", "getSourceFilenameForType()");
    }
}

void BasePrint::SetImpliedInputFileType(eInputFileType eType, bool clearWarningCount) {
    geInputFileType = eType;
    gsInputFileString = getSourceFilenameForType(geInputFileType);
    if (clearWarningCount) {
        auto iter = gInputFileWarningsMap.find(geInputFileType);
        if (iter != gInputFileWarningsMap.end()) iter->second = 0;
    }
}

