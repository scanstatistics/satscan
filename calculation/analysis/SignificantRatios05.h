//**********************************************************************************
#ifndef __SIGNIFICANTRATIOS05_H
#define __SIGNIFICANTRATIOS05_H
//**********************************************************************************
#include "SaTScan.h"

class CSignificantRatios05
{
  public:
    CSignificantRatios05(int nReplications, BasePrint *pPrintDirection);
    ~CSignificantRatios05();

    void   Initialize();
    bool   AddRatio(double r);
    void   CalcAlphas();
    const double GetAlpha01() const;
    const double GetAlpha05() const;

  private:
    int     m_nTotalReplications;
    int     m_nRatios;
    double* m_pRatiosList;
};
//**********************************************************************************
#endif
