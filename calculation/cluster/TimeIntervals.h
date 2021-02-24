//*****************************************************************************
#ifndef __TIMEINTERVALS_H
#define __TIMEINTERVALS_H
//*****************************************************************************
#include "SaTScan.h"
#include "Parameters.h"
#include "cluster.h"
#include "AbstractClusterData.h"
#include "MaxWindowLengthIndicator.h"
#include "IntermediateClustersContainer.h"

class CMeasureList; /** forward class declaration */
class CCluster; /** forward class declaration */
class CSaTScanData; /** forward class declaration */

/** Abstract base class which defines methods of iterating through temporal
    windows, evaluating the strength of a clustering.*/
class CTimeIntervals {
    private:
        void Setup(IncludeClustersType eIncludeClustersType);

    protected:
        IntervalRange_t _interval_range;  /* (startrange_start, startrange_end, endrange_start, endrange_end) */
        IntervalRange_t _init_interval_range;  /* (startrange_start, startrange_end, endrange_start, endrange_end) */
        const CSaTScanData & gDataHub; /** data hub */
        const GInfo & _gInfo;
        AbstractLikelihoodCalculator & gLikelihoodCalculator; /** log likelihood calculator */
        std::auto_ptr<AbstractMaxWindowLengthIndicator> gpMaxWindowLengthIndicator; /** indicates maximum temporal window length */
        int giNumIntervals; /* number of total time intervals */
        int giMaxWindowLength; /* maximum window length */

    public:
        CTimeIntervals(const CSaTScanData& DataHub, AbstractLikelihoodCalculator& Calculator, IncludeClustersType eIncludeClustersType);
        virtual ~CTimeIntervals() {}

        virtual void CompareClusterSet(CCluster& Running, CClusterSet& ClusterSet) = 0;
        virtual void CompareMeasures(AbstractTemporalClusterData& StreamData, CMeasureList& MeasureList) = 0;
        virtual double ComputeMaximizingValue(AbstractTemporalClusterData& ClusterData) = 0;
        void resetIntervalRange() {_interval_range = _init_interval_range; }
        void setIntervalRange(tract_t centerpoint);
};

#define STARTRANGE_STARTDATE _interval_range.get<0>()
#define STARTRANGE_ENDDATE _interval_range.get<1>()
#define ENDRANGE_STARTDATE _interval_range.get<2>()
#define ENDRANGE_ENDDATE _interval_range.get<3>()

class AbstractWindowIterator {
    public:
        typedef std::pair<int, int> WindowInterval_t;

    protected:
        AbstractMaxWindowLengthIndicator& _max_indicator;
        const IntervalRange_t& _interval_range;
        const int _window_length_max;
        std::auto_ptr<WindowInterval_t> _current_interval;

    public:
        AbstractWindowIterator(AbstractMaxWindowLengthIndicator& max_indicator, const IntervalRange_t& interval_range, int max_window_length)
            : _max_indicator(max_indicator), _interval_range(interval_range), _window_length_max(max_window_length) {
            _max_indicator.reset();
        }
        virtual ~AbstractWindowIterator() {}

        virtual const WindowInterval_t * nextInterval() = 0;
        virtual void reset() = 0;
};

/* 
    Window iterator for scanning windows of cluster. This iterator implements for the followinf for double for loop:
    iMaxEndWindow = std::min(ENDRANGE_ENDDATE, STARTRANGE_ENDDATE + giMaxWindowLength);
    for (iWindowEnd=ENDRANGE_STARTDATE; iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
        iMinWindowStart = std::max(iWindowEnd - gpMaxWindowLengthIndicator->getNextWindowLength(), STARTRANGE_STARTDATE);
        iWindowStart = std::min(STARTRANGE_ENDDATE + 1, iWindowEnd - gpMaxWindowLengthIndicator->getMinWindowLength());
        for (; iWindowStart >= iMinWindowStart; --iWindowStart) {
            ...
        }
    }
*/
class ClusterWindowIterator : public AbstractWindowIterator {
    protected:
        bool _initialized;
        int  _window_start, _window_start_min, _window_end, _window_end_max;

    public:
        ClusterWindowIterator(AbstractMaxWindowLengthIndicator& max_indicator, const IntervalRange_t& interval_range, int max_window_length)
            : AbstractWindowIterator(max_indicator, interval_range, max_window_length), _initialized(false) {
        }
        virtual ~ClusterWindowIterator() {}

        virtual inline const WindowInterval_t * nextInterval() {
            if (!_initialized) {
                _initialized = true;
                _window_end = ENDRANGE_STARTDATE;
                _window_end_max = std::min(ENDRANGE_ENDDATE, STARTRANGE_ENDDATE + _window_length_max);
                while (_window_end <= _window_end_max) {
                    _window_start = std::min(STARTRANGE_ENDDATE + 1, _window_end - _max_indicator.getMinWindowLength());
                    _window_start_min = std::max(_window_end - _max_indicator.getNextWindowLength(), STARTRANGE_STARTDATE);
                    if (_window_start >= _window_start_min) {
                        _current_interval.reset(new WindowInterval_t());
                        _current_interval->first = _window_start;
                        _current_interval->second = _window_end;
                        return _current_interval.get();
                    }
                    ++_window_end;
                }
                return _current_interval.get();
            }

            --_window_start;
            if (_window_start >= _window_start_min) {
                _current_interval->first = _window_start;
                return _current_interval.get();
            } else {
                ++_window_end;
                while (_window_end <= _window_end_max) {
                    _window_start_min = std::max(_window_end - _max_indicator.getNextWindowLength(), STARTRANGE_STARTDATE);
                    _window_start = std::min(STARTRANGE_ENDDATE + 1, _window_end - _max_indicator.getMinWindowLength());
                    if (_window_start >= _window_start_min) {
                        _current_interval->first = _window_start;
                        _current_interval->second = _window_end;
                        return _current_interval.get();
                    }
                    ++_window_end;
                }
                _current_interval.reset(0); // no more windows
            }
            return _current_interval.get();
        }

        virtual void reset() { _initialized = false; }
};

/* 
    Window iterator for scanning windows of measure or maximizing. This iterator implements for the followinf for double for loop:
    iMaxEndWindow = std::min(ENDRANGE_ENDDATE, STARTRANGE_ENDDATE + giMaxWindowLength);
    for (iWindowEnd=ENDRANGE_STARTDATE; iWindowEnd <= iMaxEndWindow; ++iWindowEnd) {
        iWindowStart = std::max(iWindowEnd - gpMaxWindowLengthIndicator->getNextWindowLength(), STARTRANGE_STARTDATE);
        iMaxStartWindow = std::min(STARTRANGE_ENDDATE + 1, iWindowEnd - gpMaxWindowLengthIndicator->getMinWindowLength() + 1);
        for (; iWindowStart < iMaxStartWindow; ++iWindowStart)
            ...
        }
*/
class MeasureWindowIterator : public AbstractWindowIterator {
    protected:
        bool _initialized;
        int  _window_start, _window_start_max, _window_end, _window_end_max;

    public:
        MeasureWindowIterator(AbstractMaxWindowLengthIndicator& max_indicator, const IntervalRange_t& interval_range, int max_window_length)
            : AbstractWindowIterator(max_indicator, interval_range, max_window_length), _initialized(false) {
        }
        virtual ~MeasureWindowIterator() {}

        virtual inline const WindowInterval_t * nextInterval() {
            if (!_initialized) {
                _initialized = true;
                _window_end = ENDRANGE_STARTDATE;
                _window_end_max = std::min(ENDRANGE_ENDDATE, STARTRANGE_ENDDATE + _window_length_max);
                while (_window_end <= _window_end_max) {
                    _window_start = std::max(_window_end - _max_indicator.getNextWindowLength(), STARTRANGE_STARTDATE);
                    _window_start_max = std::min(STARTRANGE_ENDDATE + 1, _window_end - _max_indicator.getMinWindowLength() + 1);
                    if (_window_start < _window_start_max) {
                        _current_interval.reset(new WindowInterval_t());
                        _current_interval->first = _window_start;
                        _current_interval->second = _window_end;
                        return _current_interval.get();
                    }
                    ++_window_end;
                }
                return _current_interval.get();
            }

            ++_window_start;
            if (_window_start < _window_start_max) {
                _current_interval->first = _window_start;
                return _current_interval.get();
            } else {
                ++_window_end;
                while (_window_end <= _window_end_max) {
                    _window_start = std::max(_window_end - _max_indicator.getNextWindowLength(), STARTRANGE_STARTDATE);
                    _window_start_max = std::min(STARTRANGE_ENDDATE + 1, _window_end - _max_indicator.getMinWindowLength() + 1);
                    if (_window_start < _window_start_max) {
                        _current_interval->first = _window_start;
                        _current_interval->second = _window_end;
                        return _current_interval.get();
                    }
                    ++_window_end;
                }
                _current_interval.reset(0); // no more windows
            }
            return _current_interval.get();
        }

        virtual void reset() { _initialized = false; }
};
#endif
