// SigRatios05.h

#ifndef __SIGRATIOS05_H
#define __SIGRATIOS05_H

class CSignificantRatios05
{
  public:
    CSignificantRatios05(int nReplications);
    ~CSignificantRatios05();

    void   Initialize();
    bool   AddRatio(double r);
    void   CalcAlphas();
    double GetAlpha01();
    double GetAlpha05();

  private:
    int     m_nTotalReplications;
    int     m_nRatios;
    double* m_pRatiosList;

};

#endif
