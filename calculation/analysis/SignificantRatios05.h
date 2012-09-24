//**********************************************************************************
#ifndef __SIGNIFICANTRATIOS05_H
#define __SIGNIFICANTRATIOS05_H
//**********************************************************************************
#include "SaTScan.h"

/** Maintains ordered list of significant log likelihood ratios as calculated
    during simulation process. */
class SignificantRatios {
  public:
    typedef std::vector<double> container_t;
    typedef std::pair<bool,double> alpha_t;

  private:
    typedef std::vector<double> container_t;
    typedef std::pair<bool,double> alpha_t;

    container_t  _ratios;
    unsigned int _numReplications;

  public:
    SignificantRatios(unsigned int iNumReplications);

    bool    add(double r);
    alpha_t getAlpha01() const;
    alpha_t getAlpha05() const;
    alpha_t getAlpha001() const;
    alpha_t getAlpha0001() const;
    alpha_t getAlpha00001() const;
    void    initialize();
};
//**********************************************************************************
#endif
