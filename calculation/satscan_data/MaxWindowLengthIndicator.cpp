//---------------------------------------------------------------------------
#include "SaTScan.h"
#pragma hdrstop
//---------------------------------------------------------------------------
#include "MaxWindowLengthIndicator.h"
#include "SaTScanData.h"

/** constructor */
FixedMaxWindowLengthIndicator::FixedMaxWindowLengthIndicator(const CSaTScanData & Data)
                             :AbstractMaxWindowLengthIndicator(), giMaxWindowLength(Data.GetTimeIntervalCut()) {}

/** destructor */
FixedMaxWindowLengthIndicator::~FixedMaxWindowLengthIndicator() {}


/** constructor */
ProspectiveMaxWindowLengthIndicator::ProspectiveMaxWindowLengthIndicator(const CSaTScanData& Data)
                                   :AbstractMaxWindowLengthIndicator() {
  std::vector<int>::const_iterator itr = Data.GetProspectiveIntervalCuts().begin(),
                                   itr_end = Data.GetProspectiveIntervalCuts().end();

  if (Data.GetParameters().GetMaximumTemporalClusterSizeType() == TIMETYPE)
    ZdGenerateException("For a maximum special cluster size defined as a fixed distance\n"
                        "you should use FixedMaxWindowLengthIndicator.","constructor()");

  // First value in vector is a place holder so that we can call method GetNextWindowLength()
  // to retrieve window length and increment vector iterator in one statement.
  gvMaxWindowLengths.push_back(0);
  for (; itr != itr_end; ++itr)
     gvMaxWindowLengths.push_back(*itr);
  gitr = gvMaxWindowLengths.begin();
}

/** destructor */
ProspectiveMaxWindowLengthIndicator::~ProspectiveMaxWindowLengthIndicator() {}
