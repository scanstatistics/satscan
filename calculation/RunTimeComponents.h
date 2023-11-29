//******************************************************************************
#ifndef __RunTimeComponents_H
#define __RunTimeComponents_H
//******************************************************************************
#include <boost/timer/timer.hpp>
#include <string>

/** Abtract interface for runtime component timer. */
class AbstractRunTimeComponent {
  protected:
    boost::timer::cpu_timer _timer;

  public:
    AbstractRunTimeComponent() { }
    virtual ~AbstractRunTimeComponent() {}

    boost::timer::cpu_times getTimes() { return _timer.elapsed(); }
    void Start() { _timer.resume(); }
    void Stop() { _timer.stop(); }
};

/** Runtime component timer. */
class SerialRunTimeComponent : public AbstractRunTimeComponent {
  public:
    /** Runtime types where time will not overlap. */
    enum Type  {DataRead=1, NeighborCalcuation, RealDataAnalysis, RandomDataGeneration,
                ScanningSimulatedData, PrintingResults, CatchAll};

  protected:
    Type _type;

  public:
    SerialRunTimeComponent(Type eType) : AbstractRunTimeComponent(), _type(eType) {}
    virtual ~SerialRunTimeComponent() {}

    Type GetType() const { return _type; }
};

/** Focused runtime component timer. */
class FocusRunTimeComponent : public AbstractRunTimeComponent {
  public:
    /** Runtime types where time can overlap. */
    enum Type {MeasureListScanningAdding=1, MeasureListRatioCalculation};

  protected:
    SerialRunTimeComponent::Type _serial_type;
    Type _focus_type;

  public:
    FocusRunTimeComponent(SerialRunTimeComponent::Type eSerialType, Type eFocusType)
            :AbstractRunTimeComponent(), _serial_type(eSerialType), _focus_type(eFocusType) {}
    virtual ~FocusRunTimeComponent() {}

    SerialRunTimeComponent::Type GetBelongingSerialType() const {return _serial_type;}
    Type                         GetType() const {return _focus_type;}
};

/** Manages runtime component timers. */
class RunTimeComponentManager {
  typedef std::map<SerialRunTimeComponent::Type, SerialRunTimeComponent>                  srt_container_t;
  typedef std::map<SerialRunTimeComponent::Type, SerialRunTimeComponent>::iterator        srt_itr_t;
  typedef std::map<SerialRunTimeComponent::Type, SerialRunTimeComponent>::const_iterator  const_srt_itr_t;
  typedef std::map<FocusRunTimeComponent::Type, FocusRunTimeComponent>                    frt_container_t;
  typedef std::map<FocusRunTimeComponent::Type, FocusRunTimeComponent>::iterator          frt_itr_t;
  typedef std::map<FocusRunTimeComponent::Type, FocusRunTimeComponent>::const_iterator    const_frt_itr_t;

  private:
    boost::timer::cpu_timer       _timer;
    srt_container_t               _run_time_components;
    srt_itr_t                     _current;
    frt_container_t               _focused_run_time_components;

    SerialRunTimeComponent::Type  GetCorrespondingRunTimeComponent(FocusRunTimeComponent::Type eType) const;
    const char                  * GetLabel(SerialRunTimeComponent::Type eComponent) const;
    const char                  * GetLabel(FocusRunTimeComponent::Type eComponent) const;

  public:
      RunTimeComponentManager() {}

    void                          Print(FILE* fp);
    void                          Initialize();
    void                          StartSerialComponent(SerialRunTimeComponent::Type eComponent);
    void                          StartFocused(FocusRunTimeComponent::Type eComponent);
    void                          StopSerialComponent();
    void                          StopFocused(FocusRunTimeComponent::Type eComponent);
};
//******************************************************************************
#endif
