//**********************************************************************************
#ifndef __SIGNIFICANTRATIOS05_H
#define __SIGNIFICANTRATIOS05_H
//**********************************************************************************
#include "SaTScan.h"

/** Maintains ordered list of significant log likelihood ratios as calculated
    during simulation process. */
class CSignificantRatios05 {
  private:
    std::vector<double>         gvRatiosList;

  public:
    CSignificantRatios05(unsigned int iNumReplications);
    ~CSignificantRatios05();

    bool        AddRatio(double r);
    double      GetAlpha01() const;
    double      GetAlpha05() const;
    void        Initialize();
};
//**********************************************************************************
#endif
