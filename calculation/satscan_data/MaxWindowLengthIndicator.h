//---------------------------------------------------------------------------
#ifndef __MaxWindowLengthAssessor_H
#define __MaxWindowLengthAssessor_H
//---------------------------------------------------------------------------
#include <vector>

class CSaTScanData;

class AbstractMaxWindowLengthIndicator {
  public:
    AbstractMaxWindowLengthIndicator() {}
    virtual ~AbstractMaxWindowLengthIndicator() {}

    virtual int                         GetNextWindowLength() = 0;
    virtual void                        Reset() {}
};

class FixedMaxWindowLengthIndicator : public AbstractMaxWindowLengthIndicator {
  private:
    int                                 giMaxWindowLength;

  public:
    FixedMaxWindowLengthIndicator(const CSaTScanData & Data);
    virtual ~FixedMaxWindowLengthIndicator();

    virtual int                         GetNextWindowLength() {return giMaxWindowLength;}
};

class ProspectiveMaxWindowLengthIndicator : public AbstractMaxWindowLengthIndicator {
  private:
    std::vector<int>                    gvMaxWindowLengths;
    std::vector<int>::const_iterator    gitr;

  public:
    ProspectiveMaxWindowLengthIndicator(const CSaTScanData & Data);
    virtual ~ProspectiveMaxWindowLengthIndicator();

    virtual int                         GetNextWindowLength() {++gitr; return *(gitr);}
    virtual void                        Reset() {gitr = gvMaxWindowLengths.begin();}
};
#endif
