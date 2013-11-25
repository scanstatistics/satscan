//---------------------------------------------------------------------------
#ifndef __MaxWindowLengthAssessor_H
#define __MaxWindowLengthAssessor_H
//---------------------------------------------------------------------------
#include <vector>

class CSaTScanData; /** forward class declaration */

/** Abstract base class interface for determining maximum temporal window length
    in terms of time interval units. */
class AbstractMaxWindowLengthIndicator {
  public:
    AbstractMaxWindowLengthIndicator() {}
    virtual ~AbstractMaxWindowLengthIndicator() {}

    virtual int getNextWindowLength() = 0;
    virtual void reset() {}
    virtual int getMinWindowLength() = 0;
};

/** Returns a fixed value for maximum temporal window length, as initially
    calculated by CSaTScanData object. */
class FixedMaxWindowLengthIndicator : public AbstractMaxWindowLengthIndicator {
  private:
    int _minWindowLength;
    int _maxWindowLength;

  public:
    FixedMaxWindowLengthIndicator(const CSaTScanData & Data);
    virtual ~FixedMaxWindowLengthIndicator() {}

    virtual int getMinWindowLength() {return _minWindowLength;}
    virtual int getNextWindowLength() {return _maxWindowLength;}
};

/** Returns maximum temporal window length for prospective period, as initially
    calculated by CSaTScanData object. The accessing of key method 'GetNextWindowLength()'
    is assumed to be for iterating through an end window range that defines the
    prospective period. */
class ProspectiveMaxWindowLengthIndicator : public AbstractMaxWindowLengthIndicator {
  private:
    int _minWindowLength;
    std::vector<int> gvMaxWindowLengths;
    std::vector<int>::const_iterator gitr;

  public:
    ProspectiveMaxWindowLengthIndicator(const CSaTScanData& Data);
    virtual ~ProspectiveMaxWindowLengthIndicator() {}

    virtual int getMinWindowLength() {return _minWindowLength;}
    virtual int getNextWindowLength() {return *(++gitr);}
    virtual void reset() {gitr = gvMaxWindowLengths.begin();}
};
#endif
