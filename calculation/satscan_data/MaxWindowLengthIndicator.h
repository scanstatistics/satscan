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

    virtual int                         GetNextWindowLength() = 0;
    virtual void                        Reset() {}
};

/** Returns a fixed value for maximum temporal window length, as initially
    calculated by CSaTScanData object. */
class FixedMaxWindowLengthIndicator : public AbstractMaxWindowLengthIndicator {
  private:
    int                                 giMaxWindowLength;

  public:
    FixedMaxWindowLengthIndicator(const CSaTScanData & Data);
    virtual ~FixedMaxWindowLengthIndicator();

    virtual int                         GetNextWindowLength() {return giMaxWindowLength;}
};

/** Returns maximum temporal window length for prospective period, as initially
    calculated by CSaTScanData object. The accessing of key method 'GetNextWindowLength()'
    is assumed to be for iterating through an end window range that defines the
    prospective period. */
class ProspectiveMaxWindowLengthIndicator : public AbstractMaxWindowLengthIndicator {
  private:
    std::vector<int>                    gvMaxWindowLengths;
    std::vector<int>::const_iterator    gitr;

  public:
    ProspectiveMaxWindowLengthIndicator(const CSaTScanData& Data);
    virtual ~ProspectiveMaxWindowLengthIndicator();

    virtual int                         GetNextWindowLength() {return *(++gitr);}
    virtual void                        Reset() {gitr = gvMaxWindowLengths.begin();}
};
#endif
