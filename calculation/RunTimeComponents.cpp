//******************************************************************************
#include "SaTScan.h"
#pragma hdrstop
//******************************************************************************
#include "RunTimeComponents.h"
#include "AsciiPrintFormat.h"
#include "UtilityFunctions.h"

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

/** Initializes run time manager for another analysis. */
void RunTimeComponentManager::Initialize() {
  _run_time_components.clear();
  _current=_run_time_components.end();
  _focused_run_time_components.clear();
  _timer.start();
}

/** Prints details of component runtimes to file stream. */
void RunTimeComponentManager::Print(FILE* fp) {
  boost::timer::cpu_times catch_all;
  std::string buffer;
  AsciiPrintFormat Printer;
  const std::string default_fmt("%us");

  //stop total execution timer
  catch_all = _timer.elapsed();
  Printer.SetMarginsAsRunTimeReportSection();
  fprintf(fp, "\nRUN TIME EVALUATION\n\n");
  //print SerialRunTimeComponent objects
  fprintf(fp, "Run Time Components\n");
  fprintf(fp, "-------------------\n");
  Printer.PrintSectionLabel(fp, "Total Time", false, true);
  Printer.PrintAlignedMarginsDataString(fp, _timer.format(6, default_fmt));
  for (auto& rtc: _run_time_components) {
     printString(buffer, "%d) %s", rtc.second.GetType(), GetLabel(rtc.second.GetType()));
     Printer.PrintSectionLabel(fp, buffer.c_str(), false, true);
     catch_all.system -= rtc.second.getTimes().system;
     catch_all.wall -= rtc.second.getTimes().wall;
     catch_all.user -= rtc.second.getTimes().user;
     Printer.PrintAlignedMarginsDataString(fp, boost::timer::format(rtc.second.getTimes(), 6, default_fmt));
  }
  Printer.PrintSectionLabel(fp, printString(buffer, "%d) %s", SerialRunTimeComponent::CatchAll, GetLabel(SerialRunTimeComponent::CatchAll)).c_str(), false, true);
  Printer.PrintAlignedMarginsDataString(fp, boost::timer::format(catch_all, 6, default_fmt));
  //print FocusRunTimeComponent objects
  if (_focused_run_time_components.size()) {
    fprintf(fp, "\nFocused Components\n");
    fprintf(fp, "------------------\n");
    for (auto& frtc : _focused_run_time_components) {
       printString(buffer, "%s (%d)", GetLabel(frtc.second.GetType()), frtc.second.GetBelongingSerialType());
       Printer.PrintSectionLabel(fp, buffer.c_str(), false, true);
       Printer.PrintAlignedMarginsDataString(fp, boost::timer::format(frtc.second.getTimes(), 6, default_fmt));
    }
  }  
  Printer.PrintSectionSeparatorString(fp);
}

/** Starts timing of SerialRunTimeComponent type, stopping current component, creating
    new SerialRunTimeComponent object if component of type does not already exist. */
void RunTimeComponentManager::StartSerialComponent(SerialRunTimeComponent::Type eComponent) {
  if (_current != _run_time_components.end()) _current->second.Stop();
  _current = _run_time_components.find(eComponent);
  if (_current == _run_time_components.end())
    _current = _run_time_components.insert(_run_time_components.begin(),
                                           std::make_pair(eComponent, SerialRunTimeComponent(eComponent)));
  _current->second.Start();
}

/** Starts timing of FocusRunTimeComponent type, creating new FocusRunTimeComponent object if
    component of type does not already exist. */
void RunTimeComponentManager::StartFocused(FocusRunTimeComponent::Type eComponent) {
  frt_itr_t itr = _focused_run_time_components.find(eComponent);
  if (itr == _focused_run_time_components.end())
    itr = _focused_run_time_components.insert(_focused_run_time_components.begin(),
                                            std::make_pair(eComponent,
                                                           FocusRunTimeComponent(GetCorrespondingRunTimeComponent(eComponent), eComponent)));
  itr->second.Start();
}

/** Stops timing of current SerialRunTimeComponent object. */
void RunTimeComponentManager::StopSerialComponent() {
  if (_current != _run_time_components.end()) _current->second.Stop();
  _current = _run_time_components.end();
}

/** Stops timing of FocusRunTimeComponent object for type. */
void RunTimeComponentManager::StopFocused(FocusRunTimeComponent::Type eComponent) {
  frt_itr_t itr = _focused_run_time_components.find(eComponent);
  if (itr != _focused_run_time_components.end())
    itr->second.Stop();
}

