//******************************************************************************
#ifndef __RunTimeComponents_H
#define __RunTimeComponents_H
//******************************************************************************
#include <boost/timer.hpp>
#include <string>

/** Abtract interface for runtime component timer. */
class AbstractRunTimeComponent {
  protected:
    boost::timer                gTimer;
    double                      gdTotalTime;

  public:
    AbstractRunTimeComponent() : gdTotalTime(0) {}
    virtual ~AbstractRunTimeComponent() {}

    double                      GetTotalTime() const {return gdTotalTime;}
    void                        Reset() {gdTotalTime=0;}
    void                        Start() {gTimer.restart();}
    void                        Stop() {gdTotalTime += gTimer.elapsed();}
};

/** Runtime component timer. */
class SerialRunTimeComponent : public AbstractRunTimeComponent {
  public:
    /** Runtime types where time will not overlap. */
    enum Type  {DataRead=1, NeighborCalcuation, RealDataAnalysis, RandomDataGeneration,
                ScanningSimulatedData, PrintingResults, CatchAll};

  protected:
    Type        geType;

  public:
    SerialRunTimeComponent(Type eType) : AbstractRunTimeComponent(), geType(eType) {}
    virtual ~SerialRunTimeComponent() {}

    Type        GetType() const {return geType;}
};

/** Focused runtime component timer. */
class FocusRunTimeComponent : public AbstractRunTimeComponent {
  public:
    /** Runtime types where time can overlap. */
    enum Type  {MeasureListScanningAdding=1, MeasureListRatioCalculation};

  protected:
    SerialRunTimeComponent::Type  geSerialType;
    Type                          geFocusType;

  public:
    FocusRunTimeComponent(SerialRunTimeComponent::Type eSerialType, Type eFocusType)
            :AbstractRunTimeComponent(), geSerialType(eSerialType), geFocusType(eFocusType) {}
    virtual ~FocusRunTimeComponent() {}

    SerialRunTimeComponent::Type  GetBelongingSerialType() const {return geSerialType;}
    Type                          GetType() const {return geFocusType;}
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
    boost::timer                  gTimer;
    srt_container_t               gtRunTimeComponents;
    srt_itr_t                     gtCurrent;
    frt_container_t               gtFocusedRunTimeComponents;

    SerialRunTimeComponent::Type  GetCorrespondingRunTimeComponent(FocusRunTimeComponent::Type eType) const;
    const char                  * GetLabel(SerialRunTimeComponent::Type eComponent) const;
    const char                  * GetLabel(FocusRunTimeComponent::Type eComponent) const;
    ZdString                    & GetTimeString(double dTimeInSeconds, ZdString& sTimeString) const;

  public:
    RunTimeComponentManager();
    ~RunTimeComponentManager();

    void                          Print(FILE* fp);
    void                          Initialize();
    void                          StartSerialComponent(SerialRunTimeComponent::Type eComponent);
    void                          StartFocused(FocusRunTimeComponent::Type eComponent);
    void                          StopSerialComponent();
    void                          StopFocused(FocusRunTimeComponent::Type eComponent);
};
//******************************************************************************
#endif
