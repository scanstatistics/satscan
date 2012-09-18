//**********************************************************************************
#ifndef __SIGNIFICANTRATIOS05_H
#define __SIGNIFICANTRATIOS05_H
//**********************************************************************************
#include "SaTScan.h"

/** Maintains ordered list of significant log likelihood ratios as calculated
    during simulation process. */
class CSignificantRatios05 {
  private:
    const unsigned int _replications;
    std::vector<double> _ratios_list;

  public:
    CSignificantRatios05(unsigned int replications);

    bool        AddRatio(double r);
    double      GetAlpha01() const;
    double      GetAlpha001() const;
    double      GetAlpha0001() const;
    double      GetAlpha00001() const;
    double      GetAlpha05() const;
    void        Initialize();
};
//**********************************************************************************
#endif
