//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "RunTimeComponents.h"
#include "AsciiPrintFormat.h"

/** constructor */
RunTimeComponentManager::RunTimeComponentManager() {}

/** destructor */
RunTimeComponentManager::~RunTimeComponentManager() {}

/** Returns corresponding SerialRunTimeComponent type which FocusRunTimeComponent
    type belongs to. */
SerialRunTimeComponent::Type RunTimeComponentManager::GetCorrespondingRunTimeComponent(FocusRunTimeComponent::Type eType) const {
  switch (eType) {
    case FocusRunTimeComponent::MeasureListScanningAdding   : return SerialRunTimeComponent::ScanningSimulatedData;
    case FocusRunTimeComponent::MeasureListRatioCalculation : return SerialRunTimeComponent::ScanningSimulatedData;
  }
  return SerialRunTimeComponent::CatchAll;
}

/** Returns label associated with SerialRunTimeComponent type. */
const char * RunTimeComponentManager::GetLabel(SerialRunTimeComponent::Type eComponent) const {
  switch (eComponent) {
    case SerialRunTimeComponent::DataRead              : return "Data Read";
    case SerialRunTimeComponent::NeighborCalcuation    : return "Neighbor Calculation";
    case SerialRunTimeComponent::RealDataAnalysis      : return "Real Data Analysis";
    case SerialRunTimeComponent::RandomDataGeneration  : return "Randomized Data Creation";
    case SerialRunTimeComponent::ScanningSimulatedData : return "Scanning Simulated Data";
    case SerialRunTimeComponent::PrintingResults       : return "Printing Results";
    case SerialRunTimeComponent::CatchAll              : return "Catch All"; 
  }
  return "?";
}

/** Returns label associated with FocusRunTimeComponent type. */
const char * RunTimeComponentManager::GetLabel(FocusRunTimeComponent::Type eComponent) const {
  switch (eComponent) {
    case FocusRunTimeComponent::MeasureListScanningAdding   : return "MeasureList - Scan/Add";
    case FocusRunTimeComponent::MeasureListRatioCalculation : return "MeasureList - Ratio Calculation";
  }
  return "?";
}

/** Returns formated string which indicates total run time. */
ZdString & RunTimeComponentManager::GetTimeString(double dTimeInSeconds, ZdString& sTimeString) const {
  short                 uwDays, uwHours, uwMinutes;
  ZdString              sBuffer;

  sTimeString.Clear();

//  uwDays = static_cast<short>(dTimeInSeconds / 86400);
//  dTimeInSeconds -= uwDays * 86400;
//  uwHours = static_cast<short>(dTimeInSeconds / 3600);
//  dTimeInSeconds -= uwHours * 3600;
//  uwMinutes = static_cast<short>(dTimeInSeconds / 60);
//  dTimeInSeconds -= uwMinutes * 60;

//  if (uwDays)
//    sTimeString << uwDays << " d ";
//  if (uwHours)
//    sTimeString << uwHours << " h ";
//  if (uwMinutes)
//    sTimeString << uwMinutes << " m ";

  sBuffer.printf("%g", std::max(dTimeInSeconds, 0.0));
  sTimeString << sBuffer << " s ";

  return sTimeString;
}

/** Initializes run time manager for another analysis. */
void RunTimeComponentManager::Initialize() {
  gtRunTimeComponents.clear();
  gtCurrent=gtRunTimeComponents.end();
  gtFocusedRunTimeComponents.clear();
  gTimer.restart();
}

/** Prints details of component runtimes to file stream. */
void RunTimeComponentManager::Print(FILE* fp) {
  double                dTotalExecutionTime;
  ZdString              sBuffer;
  AsciiPrintFormat      Printer;

  //stop total execution timer
  dTotalExecutionTime = gTimer.elapsed();
  Printer.SetMarginsAsRunTimeReportSection();
  fprintf(fp, "\nRUN TIME EVALUATION\n\n");
  //print SerialRunTimeComponent objects
  fprintf(fp, "Run Time Components\n");
  fprintf(fp, "-------------------\n");
  Printer.PrintSectionLabel(fp, "Total Time", false, true);
  Printer.PrintAlignedMarginsDataString(fp, GetTimeString(dTotalExecutionTime, sBuffer));
  for (const_srt_itr_t itr=gtRunTimeComponents.begin(); itr != gtRunTimeComponents.end(); ++itr) {
     sBuffer.printf("%d) %s", itr->second.GetType(), GetLabel(itr->second.GetType()));
     Printer.PrintSectionLabel(fp, sBuffer.GetCString(), false, true);
     dTotalExecutionTime -= itr->second.GetTotalTime();
     Printer.PrintAlignedMarginsDataString(fp, GetTimeString(itr->second.GetTotalTime(), sBuffer));
  }
  sBuffer.printf("%d) %s", SerialRunTimeComponent::CatchAll, GetLabel(SerialRunTimeComponent::CatchAll));
  Printer.PrintSectionLabel(fp, sBuffer.GetCString(), false, true);
  Printer.PrintAlignedMarginsDataString(fp, GetTimeString(dTotalExecutionTime, sBuffer));
  //print FocusRunTimeComponent objects
  if (gtFocusedRunTimeComponents.size()) {
    fprintf(fp, "\nFocused Components\n");
    fprintf(fp, "------------------\n");
    for (const_frt_itr_t itr=gtFocusedRunTimeComponents.begin(); itr != gtFocusedRunTimeComponents.end(); ++itr) {
       sBuffer.printf("%s (%d)", GetLabel(itr->second.GetType()), itr->second.GetBelongingSerialType());
       Printer.PrintSectionLabel(fp, sBuffer.GetCString(), false, true);
       Printer.PrintAlignedMarginsDataString(fp, GetTimeString(itr->second.GetTotalTime(), sBuffer));
    }
  }  
  Printer.PrintSectionSeparatorString(fp);
}

/** Starts timing of SerialRunTimeComponent type, stopping current component, creating
    new SerialRunTimeComponent object if component of type does not already exist. */
void RunTimeComponentManager::StartSerialComponent(SerialRunTimeComponent::Type eComponent) {
  if (gtCurrent != gtRunTimeComponents.end()) gtCurrent->second.Stop();
  gtCurrent = gtRunTimeComponents.find(eComponent);
  if (gtCurrent == gtRunTimeComponents.end())
    gtCurrent = gtRunTimeComponents.insert(gtRunTimeComponents.begin(),
                                           std::make_pair(eComponent, SerialRunTimeComponent(eComponent)));
  gtCurrent->second.Start();
}

/** Starts timing of FocusRunTimeComponent type, creating new FocusRunTimeComponent object if
    component of type does not already exist. */
void RunTimeComponentManager::StartFocused(FocusRunTimeComponent::Type eComponent) {
  frt_itr_t itr = gtFocusedRunTimeComponents.find(eComponent);
  if (itr == gtFocusedRunTimeComponents.end())
    itr = gtFocusedRunTimeComponents.insert(gtFocusedRunTimeComponents.begin(),
                                            std::make_pair(eComponent,
                                                           FocusRunTimeComponent(GetCorrespondingRunTimeComponent(eComponent), eComponent)));
  itr->second.Start();
}

/** Stops timing of current SerialRunTimeComponent object. */
void RunTimeComponentManager::StopSerialComponent() {
  if (gtCurrent != gtRunTimeComponents.end()) gtCurrent->second.Stop();
  gtCurrent = gtRunTimeComponents.end();
}

/** Stops timing of FocusRunTimeComponent object for type. */
void RunTimeComponentManager::StopFocused(FocusRunTimeComponent::Type eComponent) {
  frt_itr_t itr = gtFocusedRunTimeComponents.find(eComponent);
  if (itr != gtFocusedRunTimeComponents.end())
    itr->second.Stop();
}

