//**********************************************************************************
#ifndef __SIGNIFICANTRATIOS05_H
#define __SIGNIFICANTRATIOS05_H
//**********************************************************************************
#include "SaTScan.h"

/** Maintains ordered list of significant log likelihood ratios as calculated
    during simulation process. */
class CSignificantRatios05 {
  private:
    unsigned int m_nTotalReplications;
    unsigned int m_nRatios;
    double     * m_pRatiosList;
    bool         m_bSuppressed;

  public:
    CSignificantRatios05(bool bSuppress, unsigned int iReplications);
    ~CSignificantRatios05();

    bool        AddRatio(double r);
    void        CalcAlphas();
    double      GetAlpha01() const;
    double      GetAlpha05() const;
    void        Initialize();
};
//**********************************************************************************
#endif
