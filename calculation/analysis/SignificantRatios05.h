//**********************************************************************************
#ifndef __SIGNIFICANTRATIOS05_H
#define __SIGNIFICANTRATIOS05_H
//**********************************************************************************
#include "SaTScan.h"

/** Maintains ordered list of significant log likelihood ratios as calculated
    during simulation process. */
class CSignificantRatios05 {
  private:
    int         m_nTotalReplications;
    int         m_nRatios;
    double    * m_pRatiosList;

  public:
    CSignificantRatios05(int nReplications, BasePrint *pPrintDirection);
    ~CSignificantRatios05();

    bool        AddRatio(double r);
    void        CalcAlphas();
    double      GetAlpha01() const;
    double      GetAlpha05() const;
    void        Initialize();
};
//**********************************************************************************
#endif
