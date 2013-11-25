//---------------------------------------------------------------------------
#include "SaTScan.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#include "MaxWindowLengthIndicator.h"
#include "SaTScanData.h"
#include "SSException.h"

/** constructor */
FixedMaxWindowLengthIndicator::FixedMaxWindowLengthIndicator(const CSaTScanData & dataHub)
 :AbstractMaxWindowLengthIndicator(), _maxWindowLength(dataHub.GetTimeIntervalCut()), _minWindowLength(dataHub.getMinTimeIntervalCut()) {}

/** constructor */
ProspectiveMaxWindowLengthIndicator::ProspectiveMaxWindowLengthIndicator(const CSaTScanData& dataHub)
 :AbstractMaxWindowLengthIndicator(), _minWindowLength(dataHub.getMinTimeIntervalCut()) {
  if (dataHub.GetParameters().GetMaximumTemporalClusterSizeType() == TIMETYPE)
    throw prg_error("For a maximum special cluster size defined as a fixed distance\n"
                    "you should use FixedMaxWindowLengthIndicator.","constructor()");

  // First value in vector is a place holder so that we can call method GetNextWindowLength()
  // to retrieve window length and increment vector iterator in one statement.
  std::vector<int>::const_iterator itr = dataHub.GetProspectiveIntervalCuts().begin(), itr_end = dataHub.GetProspectiveIntervalCuts().end();
  gvMaxWindowLengths.push_back(0);
  for (; itr != itr_end; ++itr)
     gvMaxWindowLengths.push_back(*itr);
  gitr = gvMaxWindowLengths.begin();
}
